#include "stdafx.h"

#include "DXImage.h"

#pragma comment(lib, "DirectXTK12.lib")

void DXImage::LoadDDS(const std::wstring& Path)
{
	const auto CommandList = GraphicsCommandLists[0].Get();
	const auto CommandAllocator = CommandAllocators[0].Get();
	ID3D12Resource** Resource = ImageResource.GetAddressOf();

	std::unique_ptr<uint8_t[]> DDSData;
	std::vector<D3D12_SUBRESOURCE_DATA> SubresourceData;
	/**
	D3D12_SUBRESOURCE_DATA = {
		const void *pData;
		LONG_PTR RowPitch;
		LONG_PTR SlicePitch;
	}
	*/
	/** 
	DirectX::LoadDDSTextureFromFile() を使用
	Resource が作成される
	DDSData, SubresourceData へデータが格納される
	*/
	VERIFY_SUCCEEDED(DirectX::LoadDDSTextureFromFile(Device.Get(), Path.c_str(), Resource, DDSData, SubresourceData));
	const auto SubresourceCount = static_cast<UINT>(SubresourceData.size());

	/**
	D3D12_PLACED_SUBRESOURCE_FOOTPRINT = {
	UINT64 Offset;
	D3D12_SUBRESOURCE_FOOTPRINT Footprint = {
	DXGI_FORMAT Format;
	UINT Width;
	UINT Height;
	UINT Depth;
	UINT RowPitch;
	}
	}
	*/
	UINT64 TotalSize = 0;
	std::vector<D3D12_PLACED_SUBRESOURCE_FOOTPRINT> PlacedSubresourceFootprints(SubresourceCount);
	std::vector<UINT> NumRows(SubresourceCount);
	std::vector<UINT64> RowSizes(SubresourceCount);
	{
		D3D12_RESOURCE_DESC Desc = (*Resource)->GetDesc();
		ID3D12Device* pDevice;
		(*Resource)->GetDevice(__uuidof(*pDevice), reinterpret_cast<void**>(&pDevice));
		//!< PlacedSubresourceFootprints, NumRows, RowSizes へデータを格納、また TotalSize も返す
		pDevice->GetCopyableFootprints(&Desc, 0, SubresourceCount, 0, PlacedSubresourceFootprints.data(), NumRows.data(), RowSizes.data(), &TotalSize);
		pDevice->Release();
	}

	//!< アップロードリソースの作成
	const D3D12_HEAP_PROPERTIES UploadHeapProperties = {
		D3D12_HEAP_TYPE_UPLOAD, //!< UPLOAD にすること
		D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
		D3D12_MEMORY_POOL_UNKNOWN,
		1,
		1
	};
	const DXGI_SAMPLE_DESC SampleDesc = { 1, 0 };
	const D3D12_RESOURCE_DESC ResourceDesc = {
		D3D12_RESOURCE_DIMENSION_BUFFER,
		0,
		TotalSize, 1,
		1,
		1,
		DXGI_FORMAT_UNKNOWN,
		SampleDesc,
		D3D12_TEXTURE_LAYOUT_ROW_MAJOR,
		D3D12_RESOURCE_FLAG_NONE
	};
	Microsoft::WRL::ComPtr<ID3D12Resource> UploadResource;
	VERIFY_SUCCEEDED(Device->CreateCommittedResource(&UploadHeapProperties,
		D3D12_HEAP_FLAG_NONE,
		&ResourceDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ, //!< UPLOAD では GENERIC_READ にすること
		nullptr,
		IID_PPV_ARGS(UploadResource.GetAddressOf())));

	BYTE* Data;
	VERIFY_SUCCEEDED(UploadResource->Map(0, nullptr, reinterpret_cast<void**>(&Data))); {
		for (UINT i = 0; i < SubresourceCount; ++i) {
			const auto& PSF = PlacedSubresourceFootprints[i];
			const auto& SD = SubresourceData[i];

			const auto SliceCount = PSF.Footprint.Depth;
			const auto RowCount = NumRows[i];
			const auto RowSize = RowSizes[i];

			const D3D12_MEMCPY_DEST MemcpyDest = {
				Data + PSF.Offset,
				PSF.Footprint.RowPitch,
				PSF.Footprint.RowPitch * RowCount
			};
			for (UINT z = 0; z < SliceCount; ++z) {
				auto Dst = reinterpret_cast<BYTE*>(MemcpyDest.pData) + MemcpyDest.SlicePitch * z;
				const auto Src = reinterpret_cast<const BYTE*>(SD.pData) + SD.SlicePitch * z;
				for (UINT y = 0; y < RowCount; ++y) {
					memcpy(Dst + MemcpyDest.RowPitch * y, Src + SD.RowPitch * y, RowSize);
				}
			}
		}
	} UploadResource->Unmap(0, nullptr);
	
	VERIFY_SUCCEEDED(CommandList->Reset(CommandAllocator, nullptr)); {
		BarrierTransition(CommandList, *Resource, D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST); {
#if 1
			for (UINT i = 0; i < SubresourceCount; ++i) {
				const auto& PSF = PlacedSubresourceFootprints[i];
				const D3D12_TEXTURE_COPY_LOCATION Dst = {
					*Resource,
					D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX,
					i
				};
				const D3D12_TEXTURE_COPY_LOCATION Src = {
					UploadResource.Get(),
					D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT,
					PSF
				};
				CommandList->CopyTextureRegion(&Dst, 0, 0, 0, &Src, nullptr);
			}
#else
			//!< 転送先(*Resource)が D3D12_RESOURCE_DIMENSION_BUFFER の場合はこちら
			const auto& PSF = PlacedSubresourceFootprints[0];
			const D3D12_BOX Box = {
				static_cast<UINT>(PSF.Offset),
				0,
				0,
				static_cast<UINT>(PSF.Offset) + PSF.Footprint.Width,
				1,
				1
			};
			CommandList->CopyBufferRegion(*Resource, 0, UploadResource.Get(), PSF.Offset, PSF.Footprint.Width);
#endif
		} BarrierTransition(CommandList, *Resource, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	} VERIFY_SUCCEEDED(CommandList->Close());

	std::vector<ID3D12CommandList*> CommandLists = { CommandList };
	CommandQueue->ExecuteCommandLists(static_cast<UINT>(CommandLists.size()), CommandLists.data());

	WaitForFence();

#if 0
	// If it's missing mips, let's generate them
	if (generateMipsIfMissing && subresources.size() != (*texture)->GetDesc().MipLevels)
	{
		resourceUpload.GenerateMips(*texture);
	}
#else
	// todo
#endif

	auto CpuDescriptorHandle(ImageDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
	const auto IncrementSize = Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV); //!< ここでは必要ないが一応
	//!< デスクリプタ(ビュー)の作成。リソース上でのオフセットを指定して作成している、結果が変数に返るわけではない
	Device->CreateShaderResourceView(ImageResource.Get(), nullptr, CpuDescriptorHandle);
	CpuDescriptorHandle.ptr += IncrementSize; //!< ここでは必要ないが一応
}
