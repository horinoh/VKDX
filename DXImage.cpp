#include "stdafx.h"

#include "DXImage.h"

#pragma comment(lib, "DirectXTK12.lib")

void DXImage::LoadImage_DDS(ID3D12Resource** Resource, ID3D12DescriptorHeap* DescriptorHeap, const std::wstring& Path, const D3D12_RESOURCE_STATES ResourceState)
{
	std::unique_ptr<uint8_t[]> DDSData;
	std::vector<D3D12_SUBRESOURCE_DATA> SubresourceData;
	VERIFY_SUCCEEDED(DirectX::LoadDDSTextureFromFile(Device.Get(), Path.c_str(), Resource, DDSData, SubresourceData));

	//!< フットプリントの取得 Acquire footprint
	UINT64 Size = 0;
	std::vector<D3D12_PLACED_SUBRESOURCE_FOOTPRINT> PlacedSubresourceFootprints(SubresourceData.size());
	std::vector<UINT> NumRows(SubresourceData.size());
	std::vector<UINT64> RowSizes(SubresourceData.size());
	const auto ResourceDesc = (*Resource)->GetDesc();
	Device->GetCopyableFootprints(&ResourceDesc, 0, static_cast<const UINT>(SubresourceData.size()), 0, PlacedSubresourceFootprints.data(), NumRows.data(), RowSizes.data(), &Size);

	Microsoft::WRL::ComPtr<ID3D12Resource> UploadResource;
	CreateUploadResource(UploadResource.GetAddressOf(), Size);
	CopyToUploadResource(UploadResource.Get(), SubresourceData, PlacedSubresourceFootprints, NumRows, RowSizes);

	//!< #DX_TODO ミップマップの生成
	//if(ResourceDesc.MipLevels != static_cast<const UINT16>(SubresourceData.size())) {
	//	UploadResource.GenerateMips(*Resource);
	//}

	const auto CommandList = GraphicsCommandLists[0].Get();
	const auto CommandAllocator = CommandAllocators[0].Get();
	ExecuteCopyTexture(CommandAllocator, CommandList, UploadResource.Get(), *Resource, PlacedSubresourceFootprints, ResourceState);

	//!< ビューを作成
	const auto CDH = GetCPUDescriptorHandle(DescriptorHeap, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	Device->CreateShaderResourceView(*Resource, nullptr, CDH);

	//!< サンプラを作成
	CreateSampler(D3D12_SHADER_VISIBILITY_PIXEL, static_cast<const FLOAT>((*Resource)->GetDesc().MipLevels));
}
