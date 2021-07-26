#pragma once

#include "resource.h"

#pragma region Code
#include "../DXExt.h"

//!< RayGeneration <- 1
//!< Miss
//!< HitGroup <- 2
//!<	ClosestHit
//!<	AnyHit
//!<	Intersection
//!< HitGroup...

//!< ShaderRecord
//!<	Shader Identifier -> 1
//!<	Local Root Argument
//!< ShaderRecord
//!<	Shader Identifier -> 2 
//!<	Local Root Argument
//!< ShaderRecord...  

class RayTracingDX : public DXExt
{
private:
	using Super = DXExt;
public:
	RayTracingDX() : Super() {}
	virtual ~RayTracingDX() {}

#pragma region RAYTRACING
	virtual void CreateGeometry() override;
	virtual void CreateTexture() override {
		if (!HasRaytracingSupport(COM_PTR_GET(Device))) { return; }
		UnorderedAccessTextures.emplace_back().Create(COM_PTR_GET(Device), 1280, 720, 1, DXGI_FORMAT_R8G8B8A8_UNORM);
	}
	virtual void CreateRootSignature() override {
		if (!HasRaytracingSupport(COM_PTR_GET(Device))) { return; }

		//!< グローバルルートシグネチャ (Global root signature)
		{
			COM_PTR<ID3DBlob> Blob;
#ifdef USE_HLSL_ROOTSIGNATRUE
			GetRootSignaturePartFromShader(Blob, data(GetBasePath() + TEXT(".grs.cso")));
#else
			constexpr std::array DRs_Tlas = {
				//!< register(t0, space0)
				D3D12_DESCRIPTOR_RANGE({.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV, .NumDescriptors = 1, .BaseShaderRegister = 0, .RegisterSpace = 0, .OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND })
			};
			constexpr std::array DRs_Uav = {
				//!< register(u0, space0)
				D3D12_DESCRIPTOR_RANGE({.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV, .NumDescriptors = 1, .BaseShaderRegister = 0, .RegisterSpace = 0, .OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND })
			};
			DX::SerializeRootSignature(Blob, {
				//!< TLAS
				D3D12_ROOT_PARAMETER({
					.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE, 
					.DescriptorTable = D3D12_ROOT_DESCRIPTOR_TABLE({.NumDescriptorRanges = static_cast<UINT>(size(DRs_Tlas)), .pDescriptorRanges = data(DRs_Tlas) }),
					.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL 
				}),
				//!< UAV0
				D3D12_ROOT_PARAMETER({
					.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE, 
					.DescriptorTable = D3D12_ROOT_DESCRIPTOR_TABLE({.NumDescriptorRanges = static_cast<UINT>(size(DRs_Uav)), .pDescriptorRanges = data(DRs_Uav) }),
					.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL
				}),
				}, {}, D3D12_ROOT_SIGNATURE_FLAG_NONE);
#endif
			VERIFY_SUCCEEDED(Device->CreateRootSignature(0, Blob->GetBufferPointer(), Blob->GetBufferSize(), COM_PTR_UUIDOF_PUTVOID(RootSignatures.emplace_back())));
		}

		//!< ローカルルートシグネチャ (Local root signature)
#if 0
		{
			COM_PTR<ID3DBlob> Blob;
#ifdef USE_HLSL_ROOTSIGNATRUE
			GetRootSignaturePartFromShader(Blob, data(GetBasePath() + TEXT(".lrs.cso")));
#else
			DX::SerializeRootSignature(Blob, {}, {}, D3D12_ROOT_SIGNATURE_FLAG_LOCAL_ROOT_SIGNATURE);
#endif
			VERIFY_SUCCEEDED(Device->CreateRootSignature(0, Blob->GetBufferPointer(), Blob->GetBufferSize(), COM_PTR_UUIDOF_PUTVOID(RootSignatures.emplace_back())));
		}
#endif
	}
	virtual void CreatePipelineState() override;
	virtual void CreateDescriptorHeap() override {
	}
	virtual void CreateDescriptorView() override {
		const auto DH = CbvSrvUavDescriptorHeaps[0];
		auto CDH = DH->GetCPUDescriptorHandleForHeapStart();
		//Device->CreateUnorderedAccessView(COM_PTR_GET(UnorderedAccessTextures[0].Resource), &UnorderedAccessTextures[0].UAV, CDH); CDH.ptr += Device->GetDescriptorHandleIncrementSize(DH->GetDesc().Type);
		//Device->CreateShaderResourceView(COM_PTR_GET(TLASs[0].Resource), &TLASs[0].SRV, CDH); CDH.ptr += Device->GetDescriptorHandleIncrementSize(DH->GetDesc().Type);
	}
	virtual void PopulateCommandList(const size_t i) override;
#pragma endregion
};
#pragma endregion