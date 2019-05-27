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
#ifdef USE_WINRT
		VERIFY_SUCCEEDED(DirectX::LoadDDSTextureFromFile(Device.get(), Path.c_str(), Resource, DDSData, Subresource));
#elif defined(USE_WRL)
		VERIFY_SUCCEEDED(DirectX::LoadDDSTextureFromFile(Device.Get(), Path.c_str(), Resource, DDSData, Subresource));
#endif
		//!< フットプリントの取得 Acquire footprint
		UINT64 TotalBytes = 0;
		std::vector<D3D12_PLACED_SUBRESOURCE_FOOTPRINT> Footprint(Subresource.size());
		std::vector<UINT> NumRows(Subresource.size());
		std::vector<UINT64> RowBytes(Subresource.size());
		const auto RD = (*Resource)->GetDesc();
		Device->GetCopyableFootprints(&RD, 
			0, static_cast<const UINT>(Subresource.size()), 
			0, Footprint.data(), 
			NumRows.data(), RowBytes.data(), &TotalBytes);

#ifdef USE_WINRT
		winrt::com_ptr<ID3D12Resource> UploadResource;
		CreateUploadResource(UploadResource.put(), TotalBytes);
		CopyToUploadResource(UploadResource.get(), Subresource, Footprint, NumRows, RowBytes); 
#elif defined(USE_WRL)
		Microsoft::WRL::ComPtr<ID3D12Resource> UploadResource;
		CreateUploadResource(UploadResource.GetAddressOf(), TotalBytes);
		CopyToUploadResource(UploadResource.Get(), Subresource, Footprint, NumRows, RowBytes); 
#endif

		//!< #DX_TODO ミップマップの生成 Mipmap
		//if(ResourceDesc.MipLevels != static_cast<const UINT16>(SubresourceData.size())) {
		//	UploadResource.GenerateMips(*Resource);
		//}

#ifdef USE_WINRT
		ExecuteCopyTexture(CA, CL, UploadResource.get(), *Resource, Footprint, ResourceState);
#elif defined(USE_WRL)
		ExecuteCopyTexture(CA, CL, UploadResource.Get(), *Resource, Footprint, ResourceState);
#endif
	}
#ifdef USE_WINRT
	(Resource, Path, ResourceState, CommandAllocators[0].get(), GraphicsCommandLists[0].get());
#elif defined(USE_WRL)
	(Resource, Path, ResourceState, CommandAllocators[0].Get(), GraphicsCommandLists[0].Get());
#endif

#if 1
	//!< ルートシグネチャに登録して使う 複数のルートシグネチャでサンプラを使い回すような場合スタティックサンプラにしておくと良い
	//!< デスクリプタヒープ等は不要
	D3D12_STATIC_SAMPLER_DESC SSD;
	CreateStaticSamplerDesc(SSD, D3D12_SHADER_VISIBILITY_PIXEL, static_cast<const FLOAT>((*Resource)->GetDesc().MipLevels));
	StaticSamplerDescs.push_back(SSD);
#else
	//!< #DX_TODO スタティックサンプラを使用しない
	const D3D12_DESCRIPTOR_HEAP_DESC DHD = {
			D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER,
			1,
			D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE,
			0
	};
#ifdef USE_WINRT
	VERIFY_SUCCEEDED(Device->CreateDescriptorHeap(&DHD, __uuidof(SamplerDescriptorHeap), SamplerDescriptorHeap.put_void()));
#elif defined(USE_WRL)
	VERIFY_SUCCEEDED(Device->CreateDescriptorHeap(&DHD, IID_PPV_ARGS(SamplerDescriptorHeap)));
#endif

	D3D12_SAMPLER_DESC SD = {
		D3D12_FILTER_MIN_MAG_MIP_LINEAR,
		D3D12_TEXTURE_ADDRESS_MODE_WRAP, D3D12_TEXTURE_ADDRESS_MODE_WRAP, D3D12_TEXTURE_ADDRESS_MODE_WRAP,
		0.0f,
		0,
		D3D12_COMPARISON_FUNC_NEVER,
		D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE, 
		0.0f, static_cast<const FLOAT>((*Resource)->GetDesc().MipLevels),
	};
	Device->CreateSampler(&SD, SamplerDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
#endif
}
