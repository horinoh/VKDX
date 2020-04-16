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
			{ D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND }
		};
#ifdef USE_STATIC_SAMPLER
		assert(!StaticSamplerDescs.empty() && "");
		DX::SerializeRootSignature(Blob, {
				{ D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE, { static_cast<uint32_t>(DRs_Srv.size()), DRs_Srv.data() }, D3D12_SHADER_VISIBILITY_PIXEL }
			}, {
				StaticSamplerDescs[0],
			}, 
			D3D12_ROOT_SIGNATURE_FLAG_NONE
			| D3D12_ROOT_SIGNATURE_FLAG_DENY_VERTEX_SHADER_ROOT_ACCESS
			| D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS
			| D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS
			| D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS
			//| D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS
		);
#else
		const std::array<D3D12_DESCRIPTOR_RANGE, 1> DRs_Smp = { 
			{ D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND }
		};
		DX::SerializeRootSignature(Blob, {
				{ D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE, { static_cast<uint32_t>(DRs_Srv.size()), DRs_Srv.data() }, D3D12_SHADER_VISIBILITY_PIXEL },
				{ D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE, { static_cast<uint32_t>(DRs_Smp.size()), DRs_Smp.data() }, D3D12_SHADER_VISIBILITY_PIXEL }
			}, {
			},
			D3D12_ROOT_SIGNATURE_FLAG_NONE
			| D3D12_ROOT_SIGNATURE_FLAG_DENY_VERTEX_SHADER_ROOT_ACCESS
			| D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS
			| D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS
			| D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS
			//| D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS
			);
#endif
#endif
		RootSignatures.resize(1);
		VERIFY_SUCCEEDED(Device->CreateRootSignature(0, Blob->GetBufferPointer(), Blob->GetBufferSize(), COM_PTR_UUIDOF_PUTVOID(RootSignatures[0])));
		LOG_OK();
	}

	virtual void CreateDescriptorHeap() override {
		const D3D12_DESCRIPTOR_HEAP_DESC DHD = { D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 1, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE, 0 };
		VERIFY_SUCCEEDED(Device->CreateDescriptorHeap(&DHD, COM_PTR_UUIDOF_PUTVOID(ImageDescriptorHeap)));
#ifndef USE_STATIC_SAMPLER
		SamplerDescriptorHeaps.resize(1);
		const D3D12_DESCRIPTOR_HEAP_DESC DHD_Smp = { D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER, 1, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE, 0 };
		VERIFY_SUCCEEDED(Device->CreateDescriptorHeap(&DHD_Smp, COM_PTR_UUIDOF_PUTVOID(SamplerDescriptorHeaps[0])));
#endif
	}
	virtual void CreateDescriptorView() override {
		Device->CreateShaderResourceView(COM_PTR_GET(ImageResource), nullptr, GetCPUDescriptorHandle(COM_PTR_GET(ImageDescriptorHeap), 0));
	}

	virtual void CreateTexture() override {
#if 1
		LoadImage(COM_PTR_PUT(ImageResource), TEXT("UV.dds"), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
#else
		//LoadImage(COM_PTR_PUT(ImageResource), TEXT("..\\Intermediate\\Image\\kueken7_a8_unorm.dds"), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
		//LoadImage(COM_PTR_PUT(ImageResource), TEXT("..\\Intermediate\\Image\\kueken7_bgr8_srgb.dds"), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE); //!< #DX_TODO
		//LoadImage(COM_PTR_PUT(ImageResource), TEXT("..\\Intermediate\\Image\\kueken7_bgr8_unorm.dds"), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE); //!< #DX_TODO
		//LoadImage(COM_PTR_PUT(ImageResource), TEXT("..\\Intermediate\\Image\\kueken7_bgra8_srgb.dds"), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE); //!< OK
		//LoadImage(COM_PTR_PUT(ImageResource), TEXT("..\\Intermediate\\Image\\kueken7_bgra8_unorm.dds"), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE); //!< OK
		//LoadImage(COM_PTR_PUT(ImageResource), TEXT("..\\Intermediate\\Image\\kueken7_bgrx8_srgb.dds"), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE); //!< #DX_TODO
		//LoadImage(COM_PTR_PUT(ImageResource), TEXT("..\\Intermediate\\Image\\kueken7_bgrx8_unorm.dds"), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE); //!< #DX_TODO
		//LoadImage(COM_PTR_PUT(ImageResource), TEXT("..\\Intermediate\\Image\\kueken7_l8_unorm.dds"), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE); //!< OK
		//LoadImage(COM_PTR_PUT(ImageResource), TEXT("..\\Intermediate\\Image\\kueken7_r_ati1n_unorm.dds"), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE); //!< OK
		//LoadImage(COM_PTR_PUT(ImageResource), TEXT("..\\Intermediate\\Image\\kueken7_r5g6b5_unorm.dds"), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE); //!< #DX_TODO
		//LoadImage(COM_PTR_PUT(ImageResource), TEXT("..\\Intermediate\\Image\\kueken7_r8_sint.dds"), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
		//LoadImage(COM_PTR_PUT(ImageResource), TEXT("..\\Intermediate\\Image\\kueken7_r8_uint.dds"), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
		//LoadImage(COM_PTR_PUT(ImageResource), TEXT("..\\Intermediate\\Image\\kueken7_r16_unorm.dds"), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
		//LoadImage(COM_PTR_PUT(ImageResource), TEXT("..\\Intermediate\\Image\\kueken7_rg_ati2n_unorm.dds"), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE); //!< OK
		//LoadImage(COM_PTR_PUT(ImageResource), TEXT("..\\Intermediate\\Image\\kueken7_rg11b10_ufloat.dds"), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE); //!< #DX_TODO
		//LoadImage(COM_PTR_PUT(ImageResource), TEXT("..\\Intermediate\\Image\\kueken7_rgb_atc_unorm.dds"), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE); //!< #DX_TODO
		//LoadImage(COM_PTR_PUT(ImageResource), TEXT("..\\Intermediate\\Image\\kueken7_rgb_etc1_unorm.dds"), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE); //!< #DX_TODO
		//LoadImage(COM_PTR_PUT(ImageResource), TEXT("..\\Intermediate\\Image\\kueken7_rgb_etc2_srgb.dds"), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE); //!< #DX_TODO
		//LoadImage(COM_PTR_PUT(ImageResource), TEXT("..\\Intermediate\\Image\\kueken7_rgb_etc2_unorm.dds"), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE); //!< #DX_TODO
		//LoadImage(COM_PTR_PUT(ImageResource), TEXT("..\\Intermediate\\Image\\kueken7_rgb_pvrtc_2bpp_unorm.dds"), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE); //!< #DX_TODO
		//LoadImage(COM_PTR_PUT(ImageResource), TEXT("..\\Intermediate\\Image\\kueken7_rgb_pvrtc_4bpp_unorm.dds"), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE); //!< #DX_TODO
		//LoadImage(COM_PTR_PUT(ImageResource), TEXT("..\\Intermediate\\Image\\kueken7_rgb5a1_unorm.dds"), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE); //!< #DX_TODO
		//LoadImage(COM_PTR_PUT(ImageResource), TEXT("..\\Intermediate\\Image\\kueken7_rgb5a1_unorm_.dds"), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE); //!< #DX_TODO
		//LoadImage(COM_PTR_PUT(ImageResource), TEXT("..\\Intermediate\\Image\\kueken7_rgb9e5_ufloat.dds"), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE); //!< OK
		//LoadImage(COM_PTR_PUT(ImageResource), TEXT("..\\Intermediate\\Image\\kueken7_rgb10a2_unorm.dds"), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE); //!< #DX_TODO
		//LoadImage(COM_PTR_PUT(ImageResource), TEXT("..\\Intermediate\\Image\\kueken7_rgb10a2u.dds"), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE); //!< #DX_TODO
		//LoadImage(COM_PTR_PUT(ImageResource), TEXT("..\\Intermediate\\Image\\kueken7_rgba_astc4x4_srgb.dds"), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE); //!< #DX_TODO
		//LoadImage(COM_PTR_PUT(ImageResource), TEXT("..\\Intermediate\\Image\\kueken7_rgba_astc8x8_unorm.dds"), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE); //!< #DX_TODO
		//LoadImage(COM_PTR_PUT(ImageResource), TEXT("..\\Intermediate\\Image\\kueken7_rgba_atc_explicit_unorm.dds"), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE); //!< #DX_TODO
		//LoadImage(COM_PTR_PUT(ImageResource), TEXT("..\\Intermediate\\Image\\kueken7_rgba_atc_interpolate_unorm.dds"), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE); //!< #DX_TODO
		//LoadImage(COM_PTR_PUT(ImageResource), TEXT("..\\Intermediate\\Image\\kueken7_rgba_dxt1_srgb.dds"), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE); //!< #DX_TODO
		//LoadImage(COM_PTR_PUT(ImageResource), TEXT("..\\Intermediate\\Image\\kueken7_rgba_dxt1_unorm.dds"), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE); //!< #DX_TODO
		//LoadImage(COM_PTR_PUT(ImageResource), TEXT("..\\Intermediate\\Image\\kueken7_rgba_dxt5_srgb.dds"), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE); //!< #DX_TODO
		//LoadImage(COM_PTR_PUT(ImageResource), TEXT("..\\Intermediate\\Image\\kueken7_rgba_dxt5_unorm.dds"), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE); //!< #DX_TODO
		//LoadImage(COM_PTR_PUT(ImageResource), TEXT("..\\Intermediate\\Image\\kueken7_rgba_dxt5_unorm1.dds"), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE); //!< #DX_TODO
		//LoadImage(COM_PTR_PUT(ImageResource), TEXT("..\\Intermediate\\Image\\kueken7_rgba_dxt5_unorm2.dds"), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE); //!< #DX_TODO
		//LoadImage(COM_PTR_PUT(ImageResource), TEXT("..\\Intermediate\\Image\\kueken7_rgba_pvrtc2_4bpp_unorm.dds"), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE); //!< #DX_TODO
		//LoadImage(COM_PTR_PUT(ImageResource), TEXT("..\\Intermediate\\Image\\kueken7_rgba4_unorm.dds"), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE); //!< #DX_TODO
		//LoadImage(COM_PTR_PUT(ImageResource), TEXT("..\\Intermediate\\Image\\kueken7_rgba8_snorm.dds"), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE); //!< #DX_TODO
		//LoadImage(COM_PTR_PUT(ImageResource), TEXT("..\\Intermediate\\Image\\kueken7_rgba8_srgb.dds"), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE); //!< #DX_TODO
		//LoadImage(COM_PTR_PUT(ImageResource), TEXT("..\\Intermediate\\Image\\kueken7_rgba8_unorm.dds"), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE); //!< #DX_TODO
		//LoadImage(COM_PTR_PUT(ImageResource), TEXT("..\\Intermediate\\Image\\kueken7_rgba16_sfloat.dds"), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE); //!< #DX_TODO
		//LoadImage(COM_PTR_PUT(ImageResource), TEXT("..\\Intermediate\\Image\\kueken8_bgr8_unorm.dds"), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE); //!< #DX_TODO
		//LoadImage(COM_PTR_PUT(ImageResource), TEXT("..\\Intermediate\\Image\\kueken8_rgba_dxt1_unorm.dds"), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE); //!< #DX_TODO
		//LoadImage(COM_PTR_PUT(ImageResource), TEXT("..\\Intermediate\\Image\\kueken8_rgba8_srgb.dds"), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE); //!< #DX_TODO
#endif
	}
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
			0, 0, D3D12_SHADER_VISIBILITY_PIXEL
		});
	}
#else
	virtual void CreateSampler() override {
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
	virtual void CreatePipelineStates() override { CreatePipelineState_VsPs(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE); }
	virtual void PopulateCommandList(const size_t i) override;
};
#pragma endregion