#pragma once

#include <fstream>

#include "DDSTextureLoader.h"
#include "WICTextureLoader.h"

#include "DXRT.h"

class DXImage : public DXExt
{
private:
	using Super = DXExt;

public:
	static bool IsDDS(const std::filesystem::path& Path) {
		std::ifstream In(data(Path.string()), std::ios::in | std::ios::binary);
		if (!In.fail()) {
			std::array<uint32_t, 2> Header = { 0, 0 };
			In.read(reinterpret_cast<char*>(data(Header)), sizeof(Header));
			In.close();
			return 0x20534444 == Header[0] && 124 == Header[1];
		}
		return false;
	}

public:
	class XTKTexture : public Texture
	{
	protected:
		using Super = Texture;
		std::unique_ptr<uint8_t[]> Data;
		std::vector<D3D12_SUBRESOURCE_DATA> SRDs;
	public:
		D3D12_SHADER_RESOURCE_VIEW_DESC SRV;
		XTKTexture& Create(ID3D12Device* Dev, const std::filesystem::path& Path) {
			assert(std::filesystem::exists(Path) && "");
			bool IsCubeMap = false;
			if (IsDDS(Path)) {
				DirectX::DDS_ALPHA_MODE AlphaMode;
				VERIFY_SUCCEEDED(DirectX::LoadDDSTextureFromFile(Dev, data(Path.wstring()), COM_PTR_PUT(Resource), Data, SRDs, 0, &AlphaMode, &IsCubeMap));
			} else {
				VERIFY_SUCCEEDED(DirectX::LoadWICTextureFromFile(Dev, data(Path.wstring()), COM_PTR_PUT(Resource), Data, SRDs.emplace_back(), 0));
			}
			const auto RD = Resource->GetDesc();
			SRV = IsCubeMap ? D3D12_SHADER_RESOURCE_VIEW_DESC({ .Format = RD.Format, .ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE, .Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING, .TextureCube = D3D12_TEXCUBE_SRV({.MostDetailedMip = 0, .MipLevels = RD.MipLevels, .ResourceMinLODClamp = 0.0f }) }) :
				(RD.DepthOrArraySize == 1 ?
					D3D12_SHADER_RESOURCE_VIEW_DESC({ .Format = RD.Format, .ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D, .Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING, .Texture2D = D3D12_TEX2D_SRV({.MostDetailedMip = 0, .MipLevels = RD.MipLevels, .PlaneSlice = 0, .ResourceMinLODClamp = 0.0f }) }) :
					D3D12_SHADER_RESOURCE_VIEW_DESC({ .Format = RD.Format, .ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY, .Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING, .Texture2DArray = D3D12_TEX2D_ARRAY_SRV({.MostDetailedMip = 0, .MipLevels = RD.MipLevels, .FirstArraySlice = 0, .ArraySize = RD.DepthOrArraySize, .PlaneSlice = 0, .ResourceMinLODClamp = 0.0f }) })
					);
			return *this;
		}
		void PopulateCopyCommand(ID3D12GraphicsCommandList* GCL, const std::vector<D3D12_PLACED_SUBRESOURCE_FOOTPRINT>& PSFs, const D3D12_RESOURCE_STATES RS, ID3D12Resource* Upload) {
			DX::PopulateCopyTextureRegionCommand(GCL, Upload, COM_PTR_GET(Resource), PSFs, RS);
		}
		void ExecuteCopyCommand(ID3D12Device* Dev, ID3D12CommandAllocator* CA, ID3D12GraphicsCommandList* GCL, ID3D12CommandQueue* CQ, ID3D12Fence* Fnc, const D3D12_RESOURCE_STATES RS) {
			std::vector<D3D12_PLACED_SUBRESOURCE_FOOTPRINT> PSFs(size(SRDs));
			COM_PTR<ID3D12Resource> Upload;
			{
				std::vector<UINT> NumRows(size(SRDs));
				std::vector<UINT64> RowSizeInBytes(size(SRDs));
				UINT64 TotalBytes = 0;
				const auto RD = Resource->GetDesc();
				Dev->GetCopyableFootprints(&RD, 0, static_cast<const UINT>(size(SRDs)), 0, data(PSFs), data(NumRows), data(RowSizeInBytes), &TotalBytes);
				DX::CreateBufferResource(COM_PTR_PUT(Upload), Dev, SRDs, PSFs, NumRows, RowSizeInBytes, TotalBytes);
			}
			VERIFY_SUCCEEDED(GCL->Reset(CA, nullptr)); {
				PopulateCopyCommand(GCL, PSFs, RS, COM_PTR_GET(Upload));
			} VERIFY_SUCCEEDED(GCL->Close());
			DX::ExecuteAndWait(CQ, GCL, Fnc);
			Release();
		}
		void Release() {
			SRDs.clear();
			Data.reset();
		}
	};

protected:
	std::vector<XTKTexture> XTKTextures;
};

class DXImageDepth : public DXExtDepth
{
private:
	using Super = DXExtDepth;
protected:
	std::vector<DXImage::XTKTexture> XTKTextures;
};

class DXImageRT : public DXRT
{
private:
	using Super = DXRT;
protected:
	std::vector<DXImage::XTKTexture> XTKTextures;
};