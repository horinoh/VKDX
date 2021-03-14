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
		void PopulateCopyCommand(ID3D12GraphicsCommandList* GCL, const std::vector<D3D12_PLACED_SUBRESOURCE_FOOTPRINT>& PSFs, const D3D12_RESOURCE_STATES RS, ID3D12Resource* Upload) {
			DX::PopulateCommandList_CopyTextureRegion(GCL, Upload, COM_PTR_GET(Resource), PSFs, RS);
		}
		void ExecuteCopyCommand(ID3D12Device* Dev, ID3D12CommandAllocator* CA, ID3D12GraphicsCommandList* GCL, ID3D12CommandQueue* CQ, ID3D12Fence* Fnc, const D3D12_RESOURCE_STATES RS) {
			COM_PTR<ID3D12Resource> Upload;
#if 0
			std::vector<D3D12_PLACED_SUBRESOURCE_FOOTPRINT> PSFs;
			DX::CreateBufferResource(COM_PTR_PUT(Upload), Dev, PSFs, SRDs, Resource->GetDesc());
#else
			//!< フットプリントの取得 (Acquire footprint)
			std::vector<D3D12_PLACED_SUBRESOURCE_FOOTPRINT> PSFs(size(SRDs));
			std::vector<UINT> NumRows(size(SRDs));
			std::vector<UINT64> RowSizeInBytes(size(SRDs));
			UINT64 Size = 0;
			{
				const auto RD = Resource->GetDesc();
				Dev->GetCopyableFootprints(&RD, 0, static_cast<const UINT>(size(SRDs)), 0, data(PSFs), data(NumRows), data(RowSizeInBytes), &Size);
			}

			{
				const D3D12_RESOURCE_DESC RD = {
					.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER,
					.Alignment = 0,
					.Width = Size, .Height = 1,
					.DepthOrArraySize = 1, .MipLevels = 1,
					.Format = DXGI_FORMAT_UNKNOWN,
					.SampleDesc = DXGI_SAMPLE_DESC({.Count = 1, .Quality = 0 }),
					.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR,
					.Flags = D3D12_RESOURCE_FLAG_NONE
				};
				constexpr D3D12_HEAP_PROPERTIES HP = {
					.Type = D3D12_HEAP_TYPE_UPLOAD,
					.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
					.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN,
					.CreationNodeMask = 0, .VisibleNodeMask = 0
				};
				VERIFY_SUCCEEDED(Dev->CreateCommittedResource(&HP, D3D12_HEAP_FLAG_NONE, &RD, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(COM_PTR_PUT(Upload))));
				
				if (nullptr != Upload) [[likely]] {
					BYTE * Data;
					VERIFY_SUCCEEDED(Upload->Map(0, nullptr, reinterpret_cast<void**>(&Data))); {
						for (auto i = 0; i < size(PSFs); ++i) {
							const auto NR = NumRows[i];
							const auto RSIB = RowSizeInBytes[i];
							const D3D12_MEMCPY_DEST MCD = {
								.pData = Data + PSFs[i].Offset,
								.RowPitch = PSFs[i].Footprint.RowPitch,
								.SlicePitch = static_cast<SIZE_T>(PSFs[i].Footprint.RowPitch) * NR
							};
							const auto& SRD = SRDs[i];
							for (UINT j = 0; j < PSFs[i].Footprint.Depth; ++j) {
								auto Dst = reinterpret_cast<BYTE*>(MCD.pData) + MCD.SlicePitch * j;
								const auto Src = reinterpret_cast<const BYTE*>(SRD.pData) + SRD.SlicePitch * j;
								for (UINT k = 0; k < NR; ++k) {
									memcpy(Dst + MCD.RowPitch * k, Src + SRD.RowPitch * k, RSIB);
								}
							}
						}
					} Upload->Unmap(0, nullptr);
				}
			}
#endif
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