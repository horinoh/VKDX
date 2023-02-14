#pragma once

#include "DDSTextureLoader.h"
#include "WICTextureLoader.h"

#include "DXRT.h"

class DXImage : public DXExt
{
private:
	using Super = DXExt;
public:
	class DDSTexture : public Texture
	{
	protected:
		using Super = Texture;
		std::unique_ptr<uint8_t[]> DDSData;
		std::vector<D3D12_SUBRESOURCE_DATA> SRDs;
	public:
		D3D12_SHADER_RESOURCE_VIEW_DESC SRV;
		DDSTexture& Create(ID3D12Device* Dev, const std::filesystem::path& Path) {
			assert(std::filesystem::exists(Path) && "");
			assert(Path.extension() == TEXT(".dds") && "");
			DirectX::DDS_ALPHA_MODE AlphaMode;
			bool IsCubeMap;
			VERIFY_SUCCEEDED(DirectX::LoadDDSTextureFromFile(Dev, data(Path.wstring()), COM_PTR_PUT(Resource), DDSData, SRDs, 0, &AlphaMode, &IsCubeMap));
			//!< ˆµ‚¢‚½‚¢ê‡“™‚ÍŒã‚©‚ç SRV ‚ð–¾Ž¦“I‚Éã‘‚«‚·‚é
			const auto RD = Resource->GetDesc();
			SRV = IsCubeMap ? D3D12_SHADER_RESOURCE_VIEW_DESC({ .Format = RD.Format, .ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE, .Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING, .TextureCube = D3D12_TEXCUBE_SRV({.MostDetailedMip = 0, .MipLevels = RD.MipLevels, .ResourceMinLODClamp = 0.0f }) }) :
				(RD.DepthOrArraySize == 1 ?
				D3D12_SHADER_RESOURCE_VIEW_DESC({ .Format = RD.Format, .ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D, .Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING, .Texture2D = D3D12_TEX2D_SRV({.MostDetailedMip = 0, .MipLevels = RD.MipLevels, .PlaneSlice = 0, .ResourceMinLODClamp = 0.0f }) }) :
				D3D12_SHADER_RESOURCE_VIEW_DESC({ .Format = RD.Format, .ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY, .Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING, .Texture2DArray = D3D12_TEX2D_ARRAY_SRV({.MostDetailedMip = 0, .MipLevels = RD.MipLevels, .FirstArraySlice = 0, .ArraySize = RD.DepthOrArraySize, .PlaneSlice = 0, .ResourceMinLODClamp = 0.0f }) })
					);
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
			DX::PopulateCopyTextureRegionCommand(GCL, Upload, COM_PTR_GET(Resource), PSFs, RS);
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

	class PNGTexture : public Texture
	{
	protected:
		using Super = Texture;
		std::unique_ptr<uint8_t[]> PNGData;
		D3D12_SUBRESOURCE_DATA SRD;
	public:
		D3D12_SHADER_RESOURCE_VIEW_DESC SRV;
		PNGTexture& Create(ID3D12Device* Dev, const std::filesystem::path& Path) {
			assert(std::filesystem::exists(Path) && "");
			assert(Path.extension() == TEXT(".png") && "");
			VERIFY_SUCCEEDED(DirectX::LoadWICTextureFromFile(Dev, data(Path.wstring()), COM_PTR_PUT(Resource), PNGData, SRD, 0));
			//!< ˆµ‚¢‚½‚¢ê‡“™‚ÍŒã‚©‚ç SRV ‚ð–¾Ž¦“I‚Éã‘‚«‚·‚é
			const auto RD = Resource->GetDesc();
			SRV = (RD.DepthOrArraySize == 1 ?
					D3D12_SHADER_RESOURCE_VIEW_DESC({ .Format = RD.Format, .ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D, .Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING, .Texture2D = D3D12_TEX2D_SRV({.MostDetailedMip = 0, .MipLevels = RD.MipLevels, .PlaneSlice = 0, .ResourceMinLODClamp = 0.0f }) }) :
					D3D12_SHADER_RESOURCE_VIEW_DESC({ .Format = RD.Format, .ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY, .Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING, .Texture2DArray = D3D12_TEX2D_ARRAY_SRV({.MostDetailedMip = 0, .MipLevels = RD.MipLevels, .FirstArraySlice = 0, .ArraySize = RD.DepthOrArraySize, .PlaneSlice = 0, .ResourceMinLODClamp = 0.0f }) })
					);
			return *this;
		}
		void CreateUploadResourceAndFootprint(ID3D12Device* Dev, ID3D12Resource** Upload, std::vector<D3D12_PLACED_SUBRESOURCE_FOOTPRINT>& PSFs) {
			PSFs.resize(1);
			std::vector<UINT> NumRows(1);
			std::vector<UINT64> RowSizeInBytes(1);
			UINT64 TotalBytes = 0;
			const auto RD = Resource->GetDesc();
			Dev->GetCopyableFootprints(&RD, 0, 1, 0, data(PSFs), data(NumRows), data(RowSizeInBytes), &TotalBytes);
			DX::CreateBufferResource(Upload, Dev, { SRD }, PSFs, NumRows, RowSizeInBytes, TotalBytes);
		}
		void PopulateCopyCommand(ID3D12GraphicsCommandList* GCL, const std::vector<D3D12_PLACED_SUBRESOURCE_FOOTPRINT>& PSFs, const D3D12_RESOURCE_STATES RS, ID3D12Resource* Upload) {
			DX::PopulateCopyTextureRegionCommand(GCL, Upload, COM_PTR_GET(Resource), PSFs, RS);
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
			PNGData.reset();
		}
	};

protected:
	std::vector<DDSTexture> DDSTextures;
};

class DXImageDepth : public DXExtDepth
{
private:
	using Super = DXExtDepth;
protected:
	std::vector<DXImage::DDSTexture> DDSTextures;
};

class DXImageRT : public DXRT
{
private:
	using Super = DXRT;
protected:
	std::vector<DXImage::DDSTexture> DDSTextures;
};