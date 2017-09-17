#include "stdafx.h"

#include "DXImage.h"

#pragma comment(lib, "DirectXTK12.lib")

void DXImage::LoadImage(ID3D12Resource** Resource, const std::wstring& Path, const D3D12_RESOURCE_STATES ResourceState /*= D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE*/)
{
	LoadImage_DDS(Resource, Path, ResourceState);

#ifdef DEBUG_STDOUT
	std::wcout << "\t" << "ImageFile = " << Path.c_str() << std::endl;
#endif

#ifdef DEBUG_STDOUT
	std::cout << "LoadImage" << COUT_OK << std::endl << std::endl;
#endif
}

void DXImage::LoadImage_DDS(ID3D12Resource** Resource, const std::wstring& Path, const D3D12_RESOURCE_STATES ResourceState)
{
	[&](ID3D12Resource** Resource, const std::wstring& Path, const D3D12_RESOURCE_STATES ResourceState, ID3D12CommandAllocator* CA, ID3D12GraphicsCommandList* CL) {
		//!< サブリソースデータを取得 Acquire sub resource data
		std::unique_ptr<uint8_t[]> DDSData; //!< 未使用 Not used
		std::vector<D3D12_SUBRESOURCE_DATA> Subresource;
		VERIFY_SUCCEEDED(DirectX::LoadDDSTextureFromFile(Device.Get(), Path.c_str(), Resource, DDSData, Subresource));

		//!< フットプリントの取得 Acquire footprint
		UINT64 TotalBytes = 0;
		std::vector<D3D12_PLACED_SUBRESOURCE_FOOTPRINT> Footprint(Subresource.size());
		std::vector<UINT> NumRows(Subresource.size());
		std::vector<UINT64> RowBytes(Subresource.size());
		const auto ResourceDesc = (*Resource)->GetDesc();
		Device->GetCopyableFootprints(&ResourceDesc, 
			0, static_cast<const UINT>(Subresource.size()), 
			0, Footprint.data(), 
			NumRows.data(), RowBytes.data(), &TotalBytes);

		Microsoft::WRL::ComPtr<ID3D12Resource> UploadResource;
		CreateUploadResource(UploadResource.GetAddressOf(), TotalBytes);
		CopyToUploadResource(UploadResource.Get(), Subresource, Footprint, NumRows, RowBytes);

		//!< #DX_TODO ミップマップの生成 Mipmap
		//if(ResourceDesc.MipLevels != static_cast<const UINT16>(SubresourceData.size())) {
		//	UploadResource.GenerateMips(*Resource);
		//}

		ExecuteCopyTexture(CA, CL, UploadResource.Get(), *Resource, Footprint, ResourceState);
	}(Resource, Path, ResourceState, CommandAllocators[0].Get(), GraphicsCommandLists[0].Get());

	//!< スタティックサンプラデスクリプタを作成 Create static sampler descritor
	D3D12_STATIC_SAMPLER_DESC SSD;
	CreateStaticSamplerDesc(SSD, D3D12_SHADER_VISIBILITY_PIXEL, static_cast<const FLOAT>((*Resource)->GetDesc().MipLevels));
	StaticSamplerDescs.push_back(SSD);
}
