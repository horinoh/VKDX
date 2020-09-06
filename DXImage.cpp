#include "DXImage.h"

#pragma comment(lib, "DirectXTK12.lib")

//void DXImage::LoadImage(ID3D12Resource** Resource, const std::wstring& Path, const D3D12_RESOURCE_STATES ResourceState)
//{
//	LoadImage_DDS(Resource, Path, ResourceState);
//
//#ifdef DEBUG_STDOUT
//	std::wcout << "\t" << "ImageFile = " << Path.c_str() << std::endl;
//#endif
//	LOG_OK();
//}

void DXImage::LoadImage_DDS(ID3D12Resource** Resource, const D3D12_RESOURCE_STATES ResourceState, const std::wstring& Path)
{
	const auto CA = COM_PTR_GET(CommandAllocators[0]);
	const auto CL = COM_PTR_GET(GraphicsCommandLists[0]);

	//!< リソースの作成、サブリソースデータを取得 (Create resource, and acquire sub resource data)
	std::unique_ptr<uint8_t[]> DDSData; //!< 未使用 (Not used)
	std::vector<D3D12_SUBRESOURCE_DATA> Subresource;
	VERIFY_SUCCEEDED(DirectX::LoadDDSTextureFromFile(COM_PTR_GET(Device), Path.c_str(), Resource, DDSData, Subresource));

	//!< フットプリントの取得 (Acquire footprint)
	UINT64 TotalBytes = 0;
	std::vector<D3D12_PLACED_SUBRESOURCE_FOOTPRINT> PSF(Subresource.size());
	std::vector<UINT> NumRows(Subresource.size());
	std::vector<UINT64> RowBytes(Subresource.size());
	const auto RD = (*Resource)->GetDesc();
	Device->GetCopyableFootprints(&RD,
		0, static_cast<const UINT>(Subresource.size()),
		0, PSF.data(),
		NumRows.data(), RowBytes.data(), &TotalBytes);

	COM_PTR<ID3D12Resource> UploadResource;
	CreateUploadResource(COM_PTR_PUT(UploadResource), TotalBytes);
	CopyToUploadResource(COM_PTR_GET(UploadResource), PSF, NumRows, RowBytes, Subresource);

	//!< #DX_TODO ミップマップの生成 Mipmap
	//if(ResourceDesc.MipLevels != static_cast<const UINT16>(SubresourceData.size())) {
	//	UploadResource.GenerateMips(*Resource);
	//}

	ExecuteCopyTexture(*Resource, CA, CL, PSF, ResourceState, COM_PTR_GET(UploadResource));
}

void DXImage::CreateColorImage(ID3D12Resource** Resource, const D3D12_RESOURCE_STATES RS, const UINT32 Color)
{
	static const UINT64 Width = 4;
	static const UINT Height = 4;

	const D3D12_HEAP_PROPERTIES HP = {
		D3D12_HEAP_TYPE_CUSTOM,
		D3D12_CPU_PAGE_PROPERTY_WRITE_BACK,
		D3D12_MEMORY_POOL_L0, //D3D12_MEMORY_POOL_UNKNOWN
		0, 0
	};
	const D3D12_RESOURCE_DESC RD = {
		D3D12_RESOURCE_DIMENSION_TEXTURE2D,
		0,
		Width, Height, 1,
		1, 
		DXGI_FORMAT_R8G8B8A8_UNORM,
		{ 1, 0 },
		D3D12_TEXTURE_LAYOUT_UNKNOWN,
		D3D12_RESOURCE_FLAG_NONE
	};

	VERIFY_SUCCEEDED(Device->CreateCommittedResource(&HP, D3D12_HEAP_FLAG_NONE, &RD, RS, nullptr, IID_PPV_ARGS(Resource)));

	std::array<UINT32, Width * Height> Whites;
	std::fill(begin(Whites), end(Whites), Color);

	VERIFY_SUCCEEDED((*Resource)->WriteToSubresource(0, nullptr, Whites.data(), Width * Height, static_cast<UINT>(Whites.size())));
}
