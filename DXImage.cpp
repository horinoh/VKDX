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
	VERIFY_SUCCEEDED(DirectX::LoadDDSTextureFromFile(Device.Get(), Path.c_str(), Resource, DDSData, SubresourceData));
	const auto NumSubresource = static_cast<UINT>(SubresourceData.size());

	//!< サイズを求める
	UINT64 Size = 0;
	D3D12_RESOURCE_DESC Desc = (*Resource)->GetDesc();
	ID3D12Device* pDevice;
	(*Resource)->GetDevice(__uuidof(*pDevice), reinterpret_cast<void**>(&pDevice));
	pDevice->GetCopyableFootprints(&Desc, 0, NumSubresource, 0, nullptr, nullptr, nullptr, &Size);
	pDevice->Release();

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
		Size, 1,
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

	//todo
	D3D12_PLACED_SUBRESOURCE_FOOTPRINT* PlacedSubresourceFootprint = nullptr;//todo
	auto RowSizesInBytes = reinterpret_cast<UINT64*>(PlacedSubresourceFootprint + NumSubresource);
	auto NumRows = reinterpret_cast<UINT*>(RowSizesInBytes + NumSubresource);

	BYTE* Data;
	VERIFY_SUCCEEDED(UploadResource->Map(0, nullptr, reinterpret_cast<void**>(&Data))); {
		//for (UINT i = 0; i < NumSubresource; ++i) {
		//	if (RowSizesInBytes[i] <= (SIZE_T)-1) {
		//		const D3D12_MEMCPY_DEST MemcpyDest = {
		//			Data + PlacedSubresourceFootprint[i].Offset,
		//			PlacedSubresourceFootprint[i].Footprint.RowPitch,
		//			PlacedSubresourceFootprint[i].Footprint.RowPitch * NumRows[i]
		//		};

		//		//MemcpySubresource(&MemcpyDest, &SubresourceData[i], (SIZE_T)RowSizesInBytes[i], NumRows[i], PlacedSubresourceFootprint[i].Footprint.Depth);
		//		for (UINT z = 0; z < PlacedSubresourceFootprint[i].Footprint.Depth; ++z) {
		//			BYTE* pDestSlice = reinterpret_cast<BYTE*>(MemcpyDest.pData) + MemcpyDest.SlicePitch * z;
		//			const BYTE* pSrcSlice = reinterpret_cast<const BYTE*>(MemcpyDest.pData) + SubresourceData[i].SlicePitch * z;
		//			for (UINT y = 0; y < NumRows[i]; ++y) {
		//				memcpy(pDestSlice + MemcpyDest.RowPitch * y, pSrcSlice + SubresourceData[i].RowPitch * y, RowSizesInBytes[i]);
		//			}
		//		}
		//	}
		//}
	} UploadResource->Unmap(0, nullptr);
	
	VERIFY_SUCCEEDED(CommandList->Reset(CommandAllocator, nullptr)); {
		BarrierTransition(CommandList, *Resource, D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST); {
			//const D3D12_BOX Box = {
			//	static_cast<UINT>(PlacedSubresourceFootprint[0].Offset),
			//	0,
			//	0,
			//	static_cast<UINT>(PlacedSubresourceFootprint[0].Offset) + PlacedSubresourceFootprint[0].Footprint.Width,
			//	1,
			//	1
			//};
			//CommandList->CopyBufferRegion(*Resource, 0, UploadResource.Get(), PlacedSubresourceFootprint[0].Offset, PlacedSubresourceFootprint[0].Footprint.Width);
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

	//BYTE* Data;
	//VERIFY_SUCCEEDED(UploadResource->Map(0, nullptr, reinterpret_cast<void**>(&Data))); {
	//	memcpy(Data, Source, Size);
	//} UploadResource->Unmap(0, nullptr);

	//!< コピーコマンドを発行

	//VERIFY_SUCCEEDED(CommandList->Reset(CommandAllocator, nullptr)); {
	//	BarrierTransition(CommandList, *Resource, D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST); {
	//		CommandList->CopyBufferRegion(*Resource, 0, UploadResource.Get(), 0, Size);
	//	} BarrierTransition(CommandList, *Resource, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	//} VERIFY_SUCCEEDED(CommandList->Close());

	//std::vector<ID3D12CommandList*> CommandLists = { CommandList };
	//CommandQueue->ExecuteCommandLists(static_cast<UINT>(CommandLists.size()), CommandLists.data());

	//WaitForFence();
}
