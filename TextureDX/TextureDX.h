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
	virtual void CreateIndirectBuffer() override { CreateIndirectBuffer_Draw(4, 1); }

#ifdef USE_STATIC_SAMPLER
	virtual void CreateStaticSampler() override {
		StaticSamplerDescs.push_back({
			D3D12_FILTER_MIN_MAG_MIP_LINEAR,
			D3D12_TEXTURE_ADDRESS_MODE_WRAP, D3D12_TEXTURE_ADDRESS_MODE_WRAP, D3D12_TEXTURE_ADDRESS_MODE_WRAP,
			0.0f,
			0,
			D3D12_COMPARISON_FUNC_NEVER,
			D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE,
			0.0f, 1.0f,
			0, 0, D3D12_SHADER_VISIBILITY_PIXEL //!< register(t0, space0), PIXEL
		});
	}
#endif
	virtual void CreateRootSignature() override {
		COM_PTR<ID3DBlob> Blob;
#ifdef USE_HLSL_ROOTSIGNATRUE 
#ifdef USE_STATIC_SAMPLER
		GetRootSignaturePartFromShader(Blob, (GetBasePath() + TEXT(".rs.cso")).data());
#else
		GetRootSignaturePartFromShader(Blob, (GetBasePath() + TEXT("_s.rs.cso")).data());
#endif
#else
		const std::array<D3D12_DESCRIPTOR_RANGE, 1> DRs_Srv = {
			{ D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND } //!< register(t0, space0)
		};
#ifdef USE_STATIC_SAMPLER
		assert(!StaticSamplerDescs.empty() && "");
#else
		const std::array<D3D12_DESCRIPTOR_RANGE, 1> DRs_Smp = {
			{ D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND } //!< register(s0, space0)
		};
#endif
		DX::SerializeRootSignature(Blob, {
				{ D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE, { static_cast<uint32_t>(DRs_Srv.size()), DRs_Srv.data() }, D3D12_SHADER_VISIBILITY_PIXEL }, //!< SRV
#ifndef USE_STATIC_SAMPLER
				{ D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE, { static_cast<uint32_t>(DRs_Smp.size()), DRs_Smp.data() }, D3D12_SHADER_VISIBILITY_PIXEL } //!< Sampler
#endif
			}, {
#ifdef USE_STATIC_SAMPLER
				StaticSamplerDescs[0], //!< StaticSampler
#endif
			}, 
			D3D12_ROOT_SIGNATURE_FLAG_NONE
			| D3D12_ROOT_SIGNATURE_FLAG_DENY_VERTEX_SHADER_ROOT_ACCESS
			| D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS
			| D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS
			| D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS
			//| D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS
		);
#endif
		RootSignatures.resize(1);
		VERIFY_SUCCEEDED(Device->CreateRootSignature(0, Blob->GetBufferPointer(), Blob->GetBufferSize(), COM_PTR_UUIDOF_PUTVOID(RootSignatures[0])));
		LOG_OK();
	}

	virtual void CreateTexture() override {
		ImageResources.resize(1);
		std::wstring Path;
		if (FindDirectory("DDS", Path)) {
			LoadImage(COM_PTR_PUT(ImageResources[0]), Path + TEXT("\\PavingStones050_2K-JPG\\PavingStones050_2K_Color.dds"), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
		}
	}

	virtual void CreateDescriptorHeap() override {
		{
			CbvSrvUavDescriptorHeaps.resize(1);
			const D3D12_DESCRIPTOR_HEAP_DESC DHD = { D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 1, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE, 0 }; //!< SRV
			VERIFY_SUCCEEDED(Device->CreateDescriptorHeap(&DHD, COM_PTR_UUIDOF_PUTVOID(CbvSrvUavDescriptorHeaps[0])));
		}
#ifndef USE_STATIC_SAMPLER
		{
			SamplerDescriptorHeaps.resize(1);
			const D3D12_DESCRIPTOR_HEAP_DESC DHD = { D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER, 1, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE, 0 }; //!< Sampler
			VERIFY_SUCCEEDED(Device->CreateDescriptorHeap(&DHD, COM_PTR_UUIDOF_PUTVOID(SamplerDescriptorHeaps[0])));
		}
#endif
	}
	virtual void CreateDescriptorView() override {
		assert(!ImageResources.empty() && "");
		const auto& DH = CbvSrvUavDescriptorHeaps[0];
		auto CDH = DH->GetCPUDescriptorHandleForHeapStart();
		Device->CreateShaderResourceView(COM_PTR_GET(ImageResources[0]), nullptr, CDH); CDH.ptr += Device->GetDescriptorHandleIncrementSize(DH->GetDesc().Type);
	}

#ifndef USE_STATIC_SAMPLER
	virtual void CreateSampler() override {
		assert(!SamplerDescriptorHeaps.empty() && "");
		const D3D12_SAMPLER_DESC SD = {
			D3D12_FILTER_MIN_MAG_MIP_POINT, //!< ひと目でわかるように、非スタティックサンプラの場合敢えて POINT にしている
			D3D12_TEXTURE_ADDRESS_MODE_WRAP, D3D12_TEXTURE_ADDRESS_MODE_WRAP, D3D12_TEXTURE_ADDRESS_MODE_WRAP,
			0.0f,
			0,
			D3D12_COMPARISON_FUNC_NEVER,
			D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE,
			0.0f, 1.0f,
		};
		Device->CreateSampler(&SD, SamplerDescriptorHeaps[0]->GetCPUDescriptorHandleForHeapStart());
	}
#endif

	virtual void CreateShaderBlobs() override { CreateShaderBlob_VsPs(); }
	virtual void CreatePipelineStates() override { CreatePipelineState_VsPs(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE, FALSE); }
	virtual void PopulateCommandList(const size_t i) override;
};
#pragma endregion