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

#ifdef USE_WINRT
	virtual void SerializeRootSignature(winrt::com_ptr<ID3DBlob>& RSBlob) override;
#elif defined(USE_WRL)
	virtual void SerializeRootSignature(Microsoft::WRL::ComPtr<ID3DBlob>& RSBlob) override;
#endif

	virtual void CreateDescriptorHeap() override {
		CreateDescriptorHeap_1SRV();
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
	virtual void CreateShaderBlob() override { CreateShaderBlob_VsPs(); }
	virtual void CreatePipelineState() override { CreatePipelineState_VsPs(); }
	virtual void PopulateCommandList(const size_t i) override;
};
#pragma endregion