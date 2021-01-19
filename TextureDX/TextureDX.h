#pragma once

#include "resource.h"

#pragma region Code
#include "../DXImage.h"

class TextureDX : public DXImage
{
private:
	using Super = DXImage;
public:
	TextureDX() : Super() {}
	virtual ~TextureDX() {}

protected:
	virtual void CreateBottomLevel() override { CreateIndirectBuffer_Draw(4, 1); }

	virtual void CreateTexture() override {
		ImageResources.emplace_back(COM_PTR<ID3D12Resource>());
		std::wstring Path;
		if (FindDirectory("DDS", Path)) {
			LoadImage(COM_PTR_PUT(ImageResources.back()), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, Path + TEXT("\\PavingStones050_2K-JPG\\PavingStones050_2K_Color.dds"));
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
	}

#ifdef USE_STATIC_SAMPLER
	virtual void CreateStaticSampler() override {
		StaticSamplerDescs.emplace_back(D3D12_STATIC_SAMPLER_DESC({
			.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR,
			.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP, .AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP, .AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP,
			.MipLODBias = 0.0f,
			.MaxAnisotropy = 0,
			.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER,
			.BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE,
			.MinLOD = 0.0f, .MaxLOD = 1.0f,
			.ShaderRegister = 0, .RegisterSpace = 0, .ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL //!< register(t0, space0)
		}));
	}
#endif

	virtual void CreateRootSignature() override {
		COM_PTR<ID3DBlob> Blob;
#ifdef USE_HLSL_ROOTSIGNATRUE 
#ifdef USE_STATIC_SAMPLER
		GetRootSignaturePartFromShader(Blob, data(GetBasePath() + TEXT(".rs.cso")));
#else
		GetRootSignaturePartFromShader(Blob, data(GetBasePath() + TEXT("_s.rs.cso")));
#endif
#else
		const std::array DRs_Srv = {
			D3D12_DESCRIPTOR_RANGE({
				.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 
				.NumDescriptors = 1, 
				.BaseShaderRegister = 0, .RegisterSpace = 0,
				.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND
				})
		};
#ifdef USE_STATIC_SAMPLER
		assert(!empty(StaticSamplerDescs) && "");
#else
		const std::array DRs_Smp = {
			D3D12_DESCRIPTOR_RANGE({ 
				.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER,
				.NumDescriptors = 1, 
				.BaseShaderRegister = 0, .RegisterSpace = 0,
				.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND 
				}) 
		};
#endif
		DX::SerializeRootSignature(Blob, {
			//!< SRV
			D3D12_ROOT_PARAMETER({
				.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE,
				.DescriptorTable = D3D12_ROOT_DESCRIPTOR_TABLE({.NumDescriptorRanges = static_cast<uint32_t>(size(DRs_Srv)), .pDescriptorRanges = data(DRs_Srv) }),
				.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL
					}),
#ifndef USE_STATIC_SAMPLER
			//!< Sampler
			D3D12_ROOT_PARAMETER({
				.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE,
				.DescriptorTable = D3D12_ROOT_DESCRIPTOR_TABLE({.NumDescriptorRanges = static_cast<uint32_t>(size(DRs_Smp)), .pDescriptorRanges = data(DRs_Smp) }),
				.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL
				})
#endif
			}, {
#ifdef USE_STATIC_SAMPLER
				StaticSamplerDescs[0], //!< StaticSampler
#endif
			},
			D3D12_ROOT_SIGNATURE_FLAG_NONE | SHADER_ROOT_ACCESS_PS);
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
			const D3D12_DESCRIPTOR_HEAP_DESC DHD = { .Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, .NumDescriptors = 1, .Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE, .NodeMask = 0 }; //!< SRV
			VERIFY_SUCCEEDED(Device->CreateDescriptorHeap(&DHD, COM_PTR_UUIDOF_PUTVOID(CbvSrvUavDescriptorHeaps.back())));
		}
#ifndef USE_STATIC_SAMPLER
		{
			SamplerDescriptorHeaps.emplace_back(COM_PTR<ID3D12DescriptorHeap>());
			const D3D12_DESCRIPTOR_HEAP_DESC DHD = { .Type = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER, .NumDescriptors = 1, .Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE, .NodeMask = 0 }; //!< Sampler
			VERIFY_SUCCEEDED(Device->CreateDescriptorHeap(&DHD, COM_PTR_UUIDOF_PUTVOID(SamplerDescriptorHeaps.back())));
		}
#endif
	}
#ifndef USE_STATIC_SAMPLER
	virtual void CreateSampler() override {
		assert(!empty(SamplerDescriptorHeaps) && "");
		const D3D12_SAMPLER_DESC SD = {
			.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT, //!< ひと目でわかるように、非スタティックサンプラの場合敢えて POINT にしている
			.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP, .AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP, .AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP,
			.MipLODBias = 0.0f,
			.MaxAnisotropy = 0,
			.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER,
			.BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE,
			.MinLOD = 0.0f, .MaxLOD = 1.0f,
		};
		Device->CreateSampler(&SD, SamplerDescriptorHeaps[0]->GetCPUDescriptorHandleForHeapStart());
	}
#endif
	virtual void CreateDescriptorView() override {
		assert(!empty(ImageResources) && "");
		const auto& DH = CbvSrvUavDescriptorHeaps[0];
		auto CDH = DH->GetCPUDescriptorHandleForHeapStart();
#if 1
		Device->CreateShaderResourceView(COM_PTR_GET(ImageResources[0]), &ShaderResourceViewDescs[0], CDH); CDH.ptr += Device->GetDescriptorHandleIncrementSize(DH->GetDesc().Type);
#else
		//!< リソースと同じフォーマットとディメンションで最初のミップマップとスライスをターゲットするような場合にはnullptrを指定できる
		Device->CreateShaderResourceView(COM_PTR_GET(ImageResources[0]), nullptr, CDH); CDH.ptr += Device->GetDescriptorHandleIncrementSize(DH->GetDesc().Type);
#endif
	}

	virtual void PopulateCommandList(const size_t i) override;
};
#pragma endregion