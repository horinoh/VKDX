#pragma once

#include "resource.h"

#pragma region Code
#include "../DXExt.h"
#include "../Leap.h"

class LeapDX : public DXExt, public Leap
{
private:
	using Super = DXExt;
public:
	LeapDX() : Super() {}
	virtual ~LeapDX() {}

protected:
	virtual void OnTimer(HWND hWnd, HINSTANCE hInstance) override {
		Super::OnTimer(hWnd, hInstance);
#ifdef USE_LEAP
		InterpolatedTrackingEvent();
#endif
	}
#ifdef USE_LEAP
	virtual void OnTrackingEvent(const LEAP_TRACKING_EVENT* TE) override {
		Leap::OnTrackingEvent(TE);
	}
	virtual void OnImageEvent(const LEAP_IMAGE_EVENT* IE) override {
		Leap::OnImageEvent(IE);
	}
#endif

	virtual void CreateIndirectBuffer() override { CreateIndirectBuffer_Draw(4, 1); }
	virtual void CreateTexture() override {
#ifdef USE_LEAP
#if false
		//!< Leapイメージ
		{
		}
#else
		CreateTextureArray1x1({ 0xff0000ff, 0xff00ff00 });
#endif

		//!< ディストーションマップ
		{
			const auto Layers = static_cast<UINT32>(size(DistortionMatrices));
			constexpr auto LayerSize = sizeof(DistortionMatrices[0]);
			constexpr auto PitchSize = sizeof(DistortionMatrices[0].matrix[0]);

			ImageResources.emplace_back(COM_PTR<ID3D12Resource>());
			const auto RD = D3D12_RESOURCE_DESC({
				.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D,
				.Alignment = 0,
				.Width = LEAP_DISTORTION_MATRIX_N, .Height = LEAP_DISTORTION_MATRIX_N, .DepthOrArraySize = static_cast<UINT16>(Layers), .MipLevels = 1,
				.Format = DXGI_FORMAT_R32G32_FLOAT,
				.SampleDesc = DXGI_SAMPLE_DESC({.Count = 1, .Quality = 0 }),
				.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN,
				.Flags = D3D12_RESOURCE_FLAG_NONE
				});
			const D3D12_HEAP_PROPERTIES HP = {
				.Type = D3D12_HEAP_TYPE_DEFAULT,
				.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
				.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN,
				.CreationNodeMask = 0, .VisibleNodeMask = 0
			};
			VERIFY_SUCCEEDED(Device->CreateCommittedResource(&HP, D3D12_HEAP_FLAG_NONE, &RD, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, COM_PTR_UUIDOF_PUTVOID(ImageResources.back())));

			{
				const auto CA = COM_PTR_GET(CommandAllocators[0]);
				const auto CL = COM_PTR_GET(GraphicsCommandLists[0]);

				size_t AlignedSize = 0;
				for (UINT32 i = 0; i < Layers; ++i) {
					AlignedSize = RoundUp(i * LayerSize, D3D12_TEXTURE_DATA_PLACEMENT_ALIGNMENT);
					AlignedSize += LayerSize;
				}
				std::vector<std::byte> AlignedData(AlignedSize, std::byte());
				for (UINT32 i = 0; i < Layers; ++i) {
					*reinterpret_cast<LEAP_DISTORTION_MATRIX*>(&AlignedData[RoundUp(i * LayerSize, D3D12_TEXTURE_DATA_PLACEMENT_ALIGNMENT)]) = DistortionMatrices[i];
				}

				COM_PTR<ID3D12Resource> UploadResource;
				CreateBufferResource(COM_PTR_PUT(UploadResource), size(AlignedData), D3D12_HEAP_TYPE_UPLOAD);
				CopyToUploadResource(COM_PTR_GET(UploadResource), size(AlignedData), data(AlignedData));

				std::vector<D3D12_PLACED_SUBRESOURCE_FOOTPRINT> PSFs;
				for (UINT32 i = 0; i < Layers; ++i) {
					PSFs.emplace_back(D3D12_PLACED_SUBRESOURCE_FOOTPRINT({
						.Offset = RoundUp(i * LayerSize, D3D12_TEXTURE_DATA_PLACEMENT_ALIGNMENT),
						.Footprint = D3D12_SUBRESOURCE_FOOTPRINT({.Format = RD.Format, .Width = static_cast<UINT>(RD.Width), .Height = RD.Height, .Depth = 1, .RowPitch = static_cast<UINT>(RoundUp(PitchSize, D3D12_TEXTURE_DATA_PITCH_ALIGNMENT)) })
						}));
				}
				VERIFY_SUCCEEDED(CL->Reset(CA, nullptr)); {
					PopulateCommandList_CopyTextureRegion(CL, COM_PTR_GET(UploadResource), COM_PTR_GET(ImageResources.back()), PSFs, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
				} VERIFY_SUCCEEDED(CL->Close());

				ExecuteAndWait(COM_PTR_GET(CommandQueue), static_cast<ID3D12CommandList*>(CL));
			}

			ShaderResourceViewDescs.emplace_back(D3D12_SHADER_RESOURCE_VIEW_DESC({
				.Format = ImageResources.back()->GetDesc().Format,
				.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D,
				.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING,
				.Texture2D = D3D12_TEX2D_SRV({
					.MostDetailedMip = 0,
					.MipLevels = ImageResources.back()->GetDesc().MipLevels,
					.PlaneSlice = 0,
					.ResourceMinLODClamp = 0.0f
					})
				}));
		}
#else
		//!< ABRG
		CreateTextureArray1x1({ 0xff0000ff, 0xff00ff00 });
#pragma region SecondTexture
		CreateTextureArray1x1({ 0xffff0000, 0xff00ffff });
#pragma endregion
#endif
	}
	virtual void CreateStaticSampler() override {
		//!< https://developer.leapmotion.com/documentation/v4/images.html
		//!< フィルタを LINEAR、ラップモードを CLAMP
		StaticSamplerDescs.emplace_back(D3D12_STATIC_SAMPLER_DESC({
			.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR,
			.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP, .AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP, .AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
			.MipLODBias = 0.0f,
			.MaxAnisotropy = 0,
			.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER,
			.BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE,
			.MinLOD = 0.0f, .MaxLOD = 1.0f,
			.ShaderRegister = 0, .RegisterSpace = 0, .ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL
			}));
	}

	virtual void CreateRootSignature() override {
		COM_PTR<ID3DBlob> Blob;
#ifdef USE_HLSL_ROOTSIGNATRUE 
		GetRootSignaturePartFromShader(Blob, data(GetBasePath() + TEXT(".rs.cso")));
#else
		const std::array DRs = {
			D3D12_DESCRIPTOR_RANGE({
				.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
#pragma region SecondTexture
				.NumDescriptors = 2,
#pragma endregion
				.BaseShaderRegister = 0, .RegisterSpace = 0,
				.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND
				})
		};
		assert(!empty(StaticSamplerDescs) && "");
		DX::SerializeRootSignature(Blob, {
			D3D12_ROOT_PARAMETER({
				.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE,
				.DescriptorTable = D3D12_ROOT_DESCRIPTOR_TABLE({.NumDescriptorRanges = static_cast<uint32_t>(size(DRs)), .pDescriptorRanges = data(DRs) }),
				.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL
					}),
			}, {
				StaticSamplerDescs[0],
			},
			D3D12_ROOT_SIGNATURE_FLAG_NONE
			| D3D12_ROOT_SIGNATURE_FLAG_DENY_VERTEX_SHADER_ROOT_ACCESS
			| D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS
			| D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS
			| D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS
		);
#endif
		RootSignatures.emplace_back(COM_PTR<ID3D12RootSignature>());
		VERIFY_SUCCEEDED(Device->CreateRootSignature(0, Blob->GetBufferPointer(), Blob->GetBufferSize(), COM_PTR_UUIDOF_PUTVOID(RootSignatures.back())));
		LOG_OK();
	}

	virtual void CreateShaderBlobs() override { CreateShaderBlob_VsPs(); }

	virtual void CreatePipelineStates() override { CreatePipelineState_VsPs(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE, FALSE); }

	virtual void CreateDescriptorHeap() override {
		{
			CbvSrvUavDescriptorHeaps.emplace_back(COM_PTR<ID3D12DescriptorHeap>());
			const D3D12_DESCRIPTOR_HEAP_DESC DHD = { 
				.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
#pragma region SecondTexture
				.NumDescriptors = 2,
#pragma endregion
				.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE, .NodeMask = 0 };
			VERIFY_SUCCEEDED(Device->CreateDescriptorHeap(&DHD, COM_PTR_UUIDOF_PUTVOID(CbvSrvUavDescriptorHeaps.back())));
		}
	}
	virtual void CreateDescriptorView() override {
		assert(!empty(ImageResources) && "");
		const auto& DH = CbvSrvUavDescriptorHeaps[0];
		auto CDH = DH->GetCPUDescriptorHandleForHeapStart();
		Device->CreateShaderResourceView(COM_PTR_GET(ImageResources[0]), &ShaderResourceViewDescs[0], CDH); CDH.ptr += Device->GetDescriptorHandleIncrementSize(DH->GetDesc().Type);
#pragma region SecondTexture
		Device->CreateShaderResourceView(COM_PTR_GET(ImageResources[1]), &ShaderResourceViewDescs[1], CDH); CDH.ptr += Device->GetDescriptorHandleIncrementSize(DH->GetDesc().Type); 
#pragma endregion
	}

	virtual void PopulateCommandList(const size_t i) override;
};
#pragma endregion