#include "DXImage.h"

#pragma comment(lib, "DirectXTK12.lib")

void DXImage::LoadImage_DDS(ID3D12Resource** Resource, const D3D12_RESOURCE_STATES RS, std::wstring_view Path)
{
	const auto CA = COM_PTR_GET(CommandAllocators[0]);
	const auto GCL = COM_PTR_GET(GraphicsCommandLists[0]);

	//!< リソースの作成、サブリソースデータを取得 (Create resource, and acquire subresource data)
	std::unique_ptr<uint8_t[]> DDSData; //!< 未使用 (Not used)
	std::vector<D3D12_SUBRESOURCE_DATA> SRDs;
	VERIFY_SUCCEEDED(DirectX::LoadDDSTextureFromFile(COM_PTR_GET(Device), data(Path), Resource, DDSData, SRDs));

	//!< フットプリントの取得 (Acquire footprint)
	std::vector<D3D12_PLACED_SUBRESOURCE_FOOTPRINT> PSFs(size(SRDs));
	std::vector<UINT> NumRows(size(SRDs));
	std::vector<UINT64> RowSizeInBytes(size(SRDs));
	UINT64 TotalBytes = 0;
	const auto RD = (*Resource)->GetDesc();
	Device->GetCopyableFootprints(&RD, 0, static_cast<const UINT>(size(SRDs)), 0, data(PSFs), data(NumRows), data(RowSizeInBytes), &TotalBytes);

	COM_PTR<ID3D12Resource> UploadResource;
	CreateBufferResource(COM_PTR_PUT(UploadResource), COM_PTR_GET(Device), SRDs, PSFs, NumRows, RowSizeInBytes, TotalBytes);

	VERIFY_SUCCEEDED(GCL->Reset(CA, nullptr)); {
		assert(D3D12_RESOURCE_DIMENSION_BUFFER != (*Resource)->GetDesc().Dimension && "");
		PopulateCommandList_CopyTextureRegion(GCL, COM_PTR_GET(UploadResource), *Resource, PSFs, RS);
	} VERIFY_SUCCEEDED(GCL->Close());

	ExecuteAndWait(COM_PTR_GET(GraphicsCommandQueue), static_cast<ID3D12CommandList*>(GCL), COM_PTR_GET(Fence));
}
