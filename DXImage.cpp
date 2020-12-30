#include "DXImage.h"

#pragma comment(lib, "DirectXTK12.lib")

void DXImage::LoadImage_DDS(ID3D12Resource** Resource, const D3D12_RESOURCE_STATES RS, std::wstring_view Path)
{
	const auto CA = COM_PTR_GET(CommandAllocators[0]);
	const auto CL = COM_PTR_GET(GraphicsCommandLists[0]);

	//!< リソースの作成、サブリソースデータを取得 (Create resource, and acquire subresource data)
	std::unique_ptr<uint8_t[]> DDSData; //!< 未使用 (Not used)
	std::vector<D3D12_SUBRESOURCE_DATA> SRDs;
	VERIFY_SUCCEEDED(DirectX::LoadDDSTextureFromFile(COM_PTR_GET(Device), data(Path), Resource, DDSData, SRDs));

	//!< フットプリントの取得 (Acquire footprint)
	UINT64 TotalBytes = 0;
	std::vector<D3D12_PLACED_SUBRESOURCE_FOOTPRINT> PSF(size(SRDs));
	std::vector<UINT> NumRows(size(SRDs));
	std::vector<UINT64> RowBytes(size(SRDs));
	const auto RD = (*Resource)->GetDesc();
	Device->GetCopyableFootprints(&RD,
		0, static_cast<const UINT>(size(SRDs)),
		0, data(PSF),
		data(NumRows), data(RowBytes), &TotalBytes);

	COM_PTR<ID3D12Resource> UploadResource;
	CreateBufferResource(COM_PTR_PUT(UploadResource), TotalBytes, D3D12_HEAP_TYPE_UPLOAD);
	CopyToUploadResource(COM_PTR_GET(UploadResource), PSF, NumRows, RowBytes, SRDs);

	//!< #DX_TODO ミップマップの生成 Mipmap
	//if(ResourceDesc.MipLevels != static_cast<const UINT16>(size(SubresourceData))) {
	//	UploadResource.GenerateMips(*Resource);
	//}

	VERIFY_SUCCEEDED(CL->Reset(CA, nullptr)); {
		if (D3D12_RESOURCE_DIMENSION_BUFFER == (*Resource)->GetDesc().Dimension) [[unlikely]] {
			PopulateCommandList_CopyBufferRegion(CL, COM_PTR_GET(UploadResource), *Resource, PSF, RS);
		}
		else {
			PopulateCommandList_CopyTextureRegion(CL, COM_PTR_GET(UploadResource), *Resource, PSF, RS);
		}
	} VERIFY_SUCCEEDED(CL->Close());

	ExecuteAndWait(COM_PTR_GET(CommandQueue), static_cast<ID3D12CommandList*>(CL));
}
