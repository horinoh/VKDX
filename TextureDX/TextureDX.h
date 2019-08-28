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
	virtual void CreateIndirectBuffer() override { CreateIndirectBuffer_Draw(4); }

	virtual void CreateRootSignature() override {
#ifdef USE_WINRT
		winrt::com_ptr<ID3DBlob> Blob;
#elif defined(USE_WRL)
		Microsoft::WRL::ComPtr<ID3DBlob> Blob;
#endif
#ifdef ROOTSIGNATRUE_FROM_SHADER
		GetRootSignaturePartFromShader(Blob);
#else
#if 0
		const std::array<D3D12_DESCRIPTOR_RANGE, 2> DRs = { {
			{ D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND },
			{ D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND}
		} };
		DX::SerializeRootSignature(Blob, {
				{ D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE, { static_cast<uint32_t>(DRs.size()), DRs.data() }, D3D12_SHADER_VISIBILITY_PIXEL }
			}, {}, D3D12_ROOT_SIGNATURE_FLAG_NONE);
#else
		const std::array<D3D12_DESCRIPTOR_RANGE, 1> DRs = { {
			{ D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND },
		} };

		assert(!StaticSamplerDescs.empty() && "");
		DX::SerializeRootSignature(Blob, {
				{ D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE, { static_cast<uint32_t>(DRs.size()), DRs.data() }, D3D12_SHADER_VISIBILITY_PIXEL }
			}, {
				StaticSamplerDescs[0],
			}, D3D12_ROOT_SIGNATURE_FLAG_NONE);
#endif
#endif
		DX::CreateRootSignature(RootSignature, Blob);
		LOG_OK();
	}
	virtual void CreateDescriptorHeap() override {
		DX::CreateDescriptorHeap(ImageDescriptorHeap,
			{ D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 1, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE, 0 }
		);

#ifdef USE_WINRT
		[&](const D3D12_DESCRIPTOR_HEAP_TYPE Type, winrt::com_ptr<ID3D12Resource> Resource, winrt::com_ptr<ID3D12DescriptorHeap> DH) {
			const auto CDH = GetCPUDescriptorHandle(DH.get(), Type);
			Device->CreateShaderResourceView(Resource.get(), nullptr, CDH);
		}
#elif defined(USE_WRL)
		[&](const D3D12_DESCRIPTOR_HEAP_TYPE Type, Microsoft::WRL::ComPtr<ID3D12Resource> Resource, Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> DH) {
			const auto CDH = GetCPUDescriptorHandle(DH.Get(), Type);
			Device->CreateShaderResourceView(Resource.Get(), nullptr, CDH);
		}
#endif
		(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, ImageResource, ImageDescriptorHeap);
	}

	virtual void CreateTexture() override {
#if 1
#ifdef USE_WINRT
		LoadImage(ImageResource.put(), TEXT("UV.dds"), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
#elif defined(USE_WRL)
		LoadImage(ImageResource.GetAddressOf(), TEXT("UV.dds"), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
#endif
#else
		//LoadImage(ImageResource.GetAddressOf(), TEXT("..\\Intermediate\\Image\\kueken7_a8_unorm.dds"), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
		//LoadImage(ImageResource.GetAddressOf(), TEXT("..\\Intermediate\\Image\\kueken7_bgr8_srgb.dds"), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE); //!< #DX_TODO
		//LoadImage(ImageResource.GetAddressOf(), TEXT("..\\Intermediate\\Image\\kueken7_bgr8_unorm.dds"), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE); //!< #DX_TODO
		//LoadImage(ImageResource.GetAddressOf(), TEXT("..\\Intermediate\\Image\\kueken7_bgra8_srgb.dds"), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE); //!< OK
		//LoadImage(ImageResource.GetAddressOf(), TEXT("..\\Intermediate\\Image\\kueken7_bgra8_unorm.dds"), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE); //!< OK
		//LoadImage(ImageResource.GetAddressOf(), TEXT("..\\Intermediate\\Image\\kueken7_bgrx8_srgb.dds"), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE); //!< #DX_TODO
		//LoadImage(ImageResource.GetAddressOf(), TEXT("..\\Intermediate\\Image\\kueken7_bgrx8_unorm.dds"), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE); //!< #DX_TODO
		//LoadImage(ImageResource.GetAddressOf(), TEXT("..\\Intermediate\\Image\\kueken7_l8_unorm.dds"), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE); //!< OK
		//LoadImage(ImageResource.GetAddressOf(), TEXT("..\\Intermediate\\Image\\kueken7_r_ati1n_unorm.dds"), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE); //!< OK
		//LoadImage(ImageResource.GetAddressOf(), TEXT("..\\Intermediate\\Image\\kueken7_r5g6b5_unorm.dds"), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE); //!< #DX_TODO
		//LoadImage(ImageResource.GetAddressOf(), TEXT("..\\Intermediate\\Image\\kueken7_r8_sint.dds"), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
		//LoadImage(ImageResource.GetAddressOf(), TEXT("..\\Intermediate\\Image\\kueken7_r8_uint.dds"), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
		//LoadImage(ImageResource.GetAddressOf(), TEXT("..\\Intermediate\\Image\\kueken7_r16_unorm.dds"), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
		//LoadImage(ImageResource.GetAddressOf(), TEXT("..\\Intermediate\\Image\\kueken7_rg_ati2n_unorm.dds"), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE); //!< OK
		//LoadImage(ImageResource.GetAddressOf(), TEXT("..\\Intermediate\\Image\\kueken7_rg11b10_ufloat.dds"), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE); //!< #DX_TODO
		//LoadImage(ImageResource.GetAddressOf(), TEXT("..\\Intermediate\\Image\\kueken7_rgb_atc_unorm.dds"), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE); //!< #DX_TODO
		//LoadImage(ImageResource.GetAddressOf(), TEXT("..\\Intermediate\\Image\\kueken7_rgb_etc1_unorm.dds"), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE); //!< #DX_TODO
		//LoadImage(ImageResource.GetAddressOf(), TEXT("..\\Intermediate\\Image\\kueken7_rgb_etc2_srgb.dds"), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE); //!< #DX_TODO
		//LoadImage(ImageResource.GetAddressOf(), TEXT("..\\Intermediate\\Image\\kueken7_rgb_etc2_unorm.dds"), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE); //!< #DX_TODO
		//LoadImage(ImageResource.GetAddressOf(), TEXT("..\\Intermediate\\Image\\kueken7_rgb_pvrtc_2bpp_unorm.dds"), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE); //!< #DX_TODO
		//LoadImage(ImageResource.GetAddressOf(), TEXT("..\\Intermediate\\Image\\kueken7_rgb_pvrtc_4bpp_unorm.dds"), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE); //!< #DX_TODO
		//LoadImage(ImageResource.GetAddressOf(), TEXT("..\\Intermediate\\Image\\kueken7_rgb5a1_unorm.dds"), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE); //!< #DX_TODO
		//LoadImage(ImageResource.GetAddressOf(), TEXT("..\\Intermediate\\Image\\kueken7_rgb5a1_unorm_.dds"), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE); //!< #DX_TODO
		//LoadImage(ImageResource.GetAddressOf(), TEXT("..\\Intermediate\\Image\\kueken7_rgb9e5_ufloat.dds"), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE); //!< OK
		//LoadImage(ImageResource.GetAddressOf(), TEXT("..\\Intermediate\\Image\\kueken7_rgb10a2_unorm.dds"), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE); //!< #DX_TODO
		//LoadImage(ImageResource.GetAddressOf(), TEXT("..\\Intermediate\\Image\\kueken7_rgb10a2u.dds"), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE); //!< #DX_TODO
		//LoadImage(ImageResource.GetAddressOf(), TEXT("..\\Intermediate\\Image\\kueken7_rgba_astc4x4_srgb.dds"), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE); //!< #DX_TODO
		//LoadImage(ImageResource.GetAddressOf(), TEXT("..\\Intermediate\\Image\\kueken7_rgba_astc8x8_unorm.dds"), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE); //!< #DX_TODO
		//LoadImage(ImageResource.GetAddressOf(), TEXT("..\\Intermediate\\Image\\kueken7_rgba_atc_explicit_unorm.dds"), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE); //!< #DX_TODO
		//LoadImage(ImageResource.GetAddressOf(), TEXT("..\\Intermediate\\Image\\kueken7_rgba_atc_interpolate_unorm.dds"), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE); //!< #DX_TODO
		//LoadImage(ImageResource.GetAddressOf(), TEXT("..\\Intermediate\\Image\\kueken7_rgba_dxt1_srgb.dds"), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE); //!< #DX_TODO
		//LoadImage(ImageResource.GetAddressOf(), TEXT("..\\Intermediate\\Image\\kueken7_rgba_dxt1_unorm.dds"), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE); //!< #DX_TODO
		//LoadImage(ImageResource.GetAddressOf(), TEXT("..\\Intermediate\\Image\\kueken7_rgba_dxt5_srgb.dds"), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE); //!< #DX_TODO
		//LoadImage(ImageResource.GetAddressOf(), TEXT("..\\Intermediate\\Image\\kueken7_rgba_dxt5_unorm.dds"), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE); //!< #DX_TODO
		//LoadImage(ImageResource.GetAddressOf(), TEXT("..\\Intermediate\\Image\\kueken7_rgba_dxt5_unorm1.dds"), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE); //!< #DX_TODO
		//LoadImage(ImageResource.GetAddressOf(), TEXT("..\\Intermediate\\Image\\kueken7_rgba_dxt5_unorm2.dds"), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE); //!< #DX_TODO
		//LoadImage(ImageResource.GetAddressOf(), TEXT("..\\Intermediate\\Image\\kueken7_rgba_pvrtc2_4bpp_unorm.dds"), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE); //!< #DX_TODO
		//LoadImage(ImageResource.GetAddressOf(), TEXT("..\\Intermediate\\Image\\kueken7_rgba4_unorm.dds"), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE); //!< #DX_TODO
		//LoadImage(ImageResource.GetAddressOf(), TEXT("..\\Intermediate\\Image\\kueken7_rgba8_snorm.dds"), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE); //!< #DX_TODO
		//LoadImage(ImageResource.GetAddressOf(), TEXT("..\\Intermediate\\Image\\kueken7_rgba8_srgb.dds"), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE); //!< #DX_TODO
		//LoadImage(ImageResource.GetAddressOf(), TEXT("..\\Intermediate\\Image\\kueken7_rgba8_unorm.dds"), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE); //!< #DX_TODO
		//LoadImage(ImageResource.GetAddressOf(), TEXT("..\\Intermediate\\Image\\kueken7_rgba16_sfloat.dds"), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE); //!< #DX_TODO
		//LoadImage(ImageResource.GetAddressOf(), TEXT("..\\Intermediate\\Image\\kueken8_bgr8_unorm.dds"), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE); //!< #DX_TODO
		//LoadImage(ImageResource.GetAddressOf(), TEXT("..\\Intermediate\\Image\\kueken8_rgba_dxt1_unorm.dds"), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE); //!< #DX_TODO
		//LoadImage(ImageResource.GetAddressOf(), TEXT("..\\Intermediate\\Image\\kueken8_rgba8_srgb.dds"), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE); //!< #DX_TODO
#endif
	}
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
	virtual void CreateSampler() override {
		SamplerDescriptorHeaps.resize(1);
		const D3D12_DESCRIPTOR_HEAP_DESC DHD = {
			D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER,
			1, 
			D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE,
			0
		};
#ifdef USE_WINRT
		VERIFY_SUCCEEDED(Device->CreateDescriptorHeap(&DHD, __uuidof(SamplerDescriptorHeaps[0]), SamplerDescriptorHeaps[0].put_void()));
#elif defined(USE_WRL)
		VERIFY_SUCCEEDED(Device->CreateDescriptorHeap(&DHD, IID_PPV_ARGS(SamplerDescriptorHeaps[0].GetAddressOf())));
#endif

		const D3D12_SAMPLER_DESC SD = {
			//D3D12_FILTER_MIN_MAG_MIP_LINEAR,
			D3D12_FILTER_MIN_MAG_MIP_POINT,
			D3D12_TEXTURE_ADDRESS_MODE_WRAP, D3D12_TEXTURE_ADDRESS_MODE_WRAP, D3D12_TEXTURE_ADDRESS_MODE_WRAP,
			0.0f,
			0,
			D3D12_COMPARISON_FUNC_NEVER,
			D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE,
			0.0f, 1.0f,
		};
		Device->CreateSampler(&SD, SamplerDescriptorHeaps[0]->GetCPUDescriptorHandleForHeapStart());
	}
	virtual void CreateShaderBlob() override { CreateShaderBlob_VsPs(); }
	virtual void CreatePipelineState() override { CreatePipelineState_VsPs(); }
	virtual void PopulateCommandList(const size_t i) override;
};
#pragma endregion