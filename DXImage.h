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
	protected:
		using Super = Texture;
		std::unique_ptr<uint8_t[]> DDSData;
		std::vector<D3D12_SUBRESOURCE_DATA> SRDs;
	public:
		D3D12_SHADER_RESOURCE_VIEW_DESC SRV;
		DDSTexture& Create(ID3D12Device* Dev, std::wstring_view Path) {
			assert(std::filesystem::exists(Path) && "");
			assert(Path.ends_with(TEXT(".dds")) && "");
			VERIFY_SUCCEEDED(DirectX::LoadDDSTextureFromFile(Dev, data(Path), COM_PTR_PUT(Resource), DDSData, SRDs));
			//!< ここでは DepthOrArraySize が複数の場合 TEXTURE2DARRAY として扱う為、TEXTURECUBE や TEXTURE3D として扱いたい場合等は後から SRV を明示的に上書きする必要がある
			const auto RD = Resource->GetDesc();
			SRV = RD.DepthOrArraySize == 1 ?
				D3D12_SHADER_RESOURCE_VIEW_DESC({ .Format = RD.Format, .ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D, .Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING, .Texture2D = D3D12_TEX2D_SRV({.MostDetailedMip = 0, .MipLevels = RD.MipLevels, .PlaneSlice = 0, .ResourceMinLODClamp = 0.0f }) }) :
				D3D12_SHADER_RESOURCE_VIEW_DESC({ .Format = RD.Format, .ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY, .Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING, .Texture2DArray = D3D12_TEX2D_ARRAY_SRV({.MostDetailedMip = 0, .MipLevels = RD.MipLevels, .FirstArraySlice = 0, .ArraySize = RD.DepthOrArraySize, .PlaneSlice = 0, .ResourceMinLODClamp = 0.0f }) });
			return *this;
		}
		void CreateUploadResourceAndFootprint(ID3D12Device* Dev, ID3D12Resource** Upload, std::vector<D3D12_PLACED_SUBRESOURCE_FOOTPRINT>& PSFs) {
			PSFs.resize(size(SRDs));
			std::vector<UINT> NumRows(size(SRDs));
			std::vector<UINT64> RowSizeInBytes(size(SRDs));
			UINT64 TotalBytes = 0;
			const auto RD = Resource->GetDesc();
			Dev->GetCopyableFootprints(&RD, 0, static_cast<const UINT>(size(SRDs)), 0, data(PSFs), data(NumRows), data(RowSizeInBytes), &TotalBytes);
			DX::CreateBufferResource(Upload, Dev, SRDs, PSFs, NumRows, RowSizeInBytes, TotalBytes);
		}
		void PopulateCopyCommand(ID3D12GraphicsCommandList* GCL, const std::vector<D3D12_PLACED_SUBRESOURCE_FOOTPRINT>& PSFs, const D3D12_RESOURCE_STATES RS, ID3D12Resource* Upload) {
			DX::PopulateCommandList_CopyTextureRegion(GCL, Upload, COM_PTR_GET(Resource), PSFs, RS);
		}
		void ExecuteCopyCommand(ID3D12Device* Dev, ID3D12CommandAllocator* CA, ID3D12GraphicsCommandList* GCL, ID3D12CommandQueue* CQ, ID3D12Fence* Fnc, const D3D12_RESOURCE_STATES RS) {
			COM_PTR<ID3D12Resource> Upload;
			std::vector<D3D12_PLACED_SUBRESOURCE_FOOTPRINT> PSFs;
			CreateUploadResourceAndFootprint(Dev, COM_PTR_PUT(Upload), PSFs);
			VERIFY_SUCCEEDED(GCL->Reset(CA, nullptr)); {
				PopulateCopyCommand(GCL, PSFs, RS, COM_PTR_GET(Upload));
			} VERIFY_SUCCEEDED(GCL->Close());
			DX::ExecuteAndWait(CQ, GCL, Fnc);
			Release();
		}
		void Release() {
			SRDs.clear();
			DDSData.reset();
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