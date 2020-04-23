#include "DXImage.h"

#pragma comment(lib, "DirectXTK12.lib")

void DXImage::LoadImage(ID3D12Resource** Resource, const std::wstring& Path, const D3D12_RESOURCE_STATES ResourceState)
{
	LoadImage_DDS(Resource, Path, ResourceState);

#ifdef DEBUG_STDOUT
	std::wcout << "\t" << "ImageFile = " << Path.c_str() << std::endl;
#endif
	LOG_OK();
}

void DXImage::LoadImage_DDS(ID3D12Resource** Resource, const std::wstring& Path, const D3D12_RESOURCE_STATES ResourceState)
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
