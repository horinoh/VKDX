#include "stdafx.h"

#include "DXImage.h"

#pragma comment(lib, "DirectXTK12.lib")

void DXImage::LoadImage_DDS(ID3D12Resource** Resource, ID3D12DescriptorHeap* DescriptorHeap, const std::wstring& Path, const D3D12_RESOURCE_STATES ResourceState)
{
	std::unique_ptr<uint8_t[]> DDSData;
	std::vector<D3D12_SUBRESOURCE_DATA> SubresourceData;
	VERIFY_SUCCEEDED(DirectX::LoadDDSTextureFromFile(Device.Get(), Path.c_str(), Resource, DDSData, SubresourceData));

	const auto ResourceDesc = (*Resource)->GetDesc();

	//!< フットプリントの取得
	UINT64 TotalSize = 0;
	std::vector<D3D12_PLACED_SUBRESOURCE_FOOTPRINT> PlacedSubresourceFootprints(SubresourceData.size());
	std::vector<UINT> NumRows(SubresourceData.size());
	std::vector<UINT64> RowSizes(SubresourceData.size());
	Device->GetCopyableFootprints(&ResourceDesc, 0, static_cast<const UINT>(SubresourceData.size()), 0, PlacedSubresourceFootprints.data(), NumRows.data(), RowSizes.data(), &TotalSize);

	//!< アップロードリソースの作成
	Microsoft::WRL::ComPtr<ID3D12Resource> UploadResource;
	CreateUploadResource(UploadResource.GetAddressOf(), SubresourceData, TotalSize, PlacedSubresourceFootprints, NumRows, RowSizes);
	
	//!< #TODO ミップマップの生成
	if(ResourceDesc.MipLevels != static_cast<const UINT16>(SubresourceData.size())) {
		//UploadResource.GenerateMips(*Resource);
	}

	//!< コピーコマンドの発行
	const auto CommandList = GraphicsCommandLists[0].Get();
	const auto CommandAllocator = CommandAllocators[0].Get();
	VERIFY_SUCCEEDED(CommandList->Reset(CommandAllocator, nullptr)); {
		if (D3D12_RESOURCE_DIMENSION_BUFFER == ResourceDesc.Dimension) {
			PopulateCopyBufferCommand(CommandList, UploadResource.Get(), *Resource, PlacedSubresourceFootprints, ResourceState);
		}
		else {
			PopulateCopyTextureCommand(CommandList, UploadResource.Get(), *Resource, PlacedSubresourceFootprints, ResourceState);
		}
	} VERIFY_SUCCEEDED(CommandList->Close());

	//ExecuteCommandListAndWaitForFence(CommandList);
	const std::vector<ID3D12CommandList*> CommandLists = { CommandList };
	CommandQueue->ExecuteCommandLists(static_cast<UINT>(CommandLists.size()), CommandLists.data());
	WaitForFence();

	//!< ビューを作成
	Device->CreateShaderResourceView(*Resource, nullptr, GetCPUDescriptorHandle(DescriptorHeap, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));

	//!< サンプラを作成
	CreateSampler(D3D12_SHADER_VISIBILITY_PIXEL, static_cast<const FLOAT>((*Resource)->GetDesc().MipLevels));
}
