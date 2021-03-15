#pragma once

#include "DDSTextureLoader.h"

#include "DXExt.h"

class DXImage : public DXExt
{
private:
	using Super = DXExt;

protected:
	class DDSTexture : public Texture
	{
	private:
		using Super = Texture;
		std::vector<D3D12_SUBRESOURCE_DATA> SRDs;
		std::vector<D3D12_PLACED_SUBRESOURCE_FOOTPRINT> PSFs;
	public:
		D3D12_SHADER_RESOURCE_VIEW_DESC SRV;
		void Create(ID3D12Device* Dev, std::wstring_view Path) {
			assert(std::filesystem::exists(Path) && "");
			assert(Path.ends_with(TEXT(".dds")) && "");
			std::unique_ptr<uint8_t[]> DDSData;
			VERIFY_SUCCEEDED(DirectX::LoadDDSTextureFromFile(Dev, data(Path), COM_PTR_PUT(Resource), DDSData, SRDs));

			const auto RD = Resource->GetDesc();
			SRV = RD.DepthOrArraySize == 1 ?
				D3D12_SHADER_RESOURCE_VIEW_DESC({ .Format = RD.Format, .ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D, .Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING, .Texture2D = D3D12_TEX2D_SRV({.MostDetailedMip = 0, .MipLevels = RD.MipLevels, .PlaneSlice = 0, .ResourceMinLODClamp = 0.0f }) }) :
				D3D12_SHADER_RESOURCE_VIEW_DESC({ .Format = RD.Format, .ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY, .Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING, .Texture2DArray = D3D12_TEX2D_ARRAY_SRV({.MostDetailedMip = 0, .MipLevels = RD.MipLevels, .FirstArraySlice = 0, .ArraySize = RD.DepthOrArraySize, .PlaneSlice = 0, .ResourceMinLODClamp = 0.0f }) });
		}
		void PopulateCopyCommand(ID3D12GraphicsCommandList* GCL, const std::vector<D3D12_PLACED_SUBRESOURCE_FOOTPRINT>& PSFs0, const D3D12_RESOURCE_STATES RS, ID3D12Resource* Upload) {
			DX::PopulateCommandList_CopyTextureRegion(GCL, Upload, COM_PTR_GET(Resource), PSFs0, RS);
		}
		void ExecuteCopyCommand(ID3D12Device* Dev, ID3D12CommandAllocator* CA, ID3D12GraphicsCommandList* GCL, ID3D12CommandQueue* CQ, ID3D12Fence* Fnc, const D3D12_RESOURCE_STATES RS) {
			//!< フットプリントの取得 (Acquire footprint)
			PSFs.resize(size(SRDs));
			std::vector<UINT> NumRows(size(SRDs));
			std::vector<UINT64> RowSizeInBytes(size(SRDs));
			UINT64 TotalBytes = 0;
			const auto RD = Resource->GetDesc();
			Dev->GetCopyableFootprints(&RD, 0, static_cast<const UINT>(size(SRDs)), 0, data(PSFs), data(NumRows), data(RowSizeInBytes), &TotalBytes);

			COM_PTR<ID3D12Resource> Upload;
			DX::CreateBufferResource(COM_PTR_PUT(Upload), Dev, SRDs, PSFs, NumRows, RowSizeInBytes, TotalBytes);

			VERIFY_SUCCEEDED(GCL->Reset(CA, nullptr)); {
				PopulateCopyCommand(GCL, PSFs, RS, COM_PTR_GET(Upload));
			} VERIFY_SUCCEEDED(GCL->Close());
			DX::ExecuteAndWait(CQ, GCL, Fnc);
		}
	};

	virtual void LoadImage(ID3D12Resource** Resource, const D3D12_RESOURCE_STATES RS, std::wstring_view Path) {
		assert(std::filesystem::exists(Path) && "");
		assert(Path.ends_with(TEXT(".dds")) && "");
		LoadImage_DDS(Resource, RS, Path);
	}
	void LoadImage_DDS(ID3D12Resource** Resource, const D3D12_RESOURCE_STATES RS, std::wstring_view Path);

	std::vector<DDSTexture> DDSTextures;
};