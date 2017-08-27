#pragma once

#include "resource.h"

#pragma region Code
#include "../DXImage.h"

#define USE_DRAW_INDIRECT

class TextureDX : public DXImage
{
private:
	using Super = DXImage;
public:
	TextureDX() : Super() {}
	virtual ~TextureDX() {}

protected:
#ifdef USE_DRAW_INDIRECT
	virtual void CreateIndirectBuffer() override { CreateIndirectBuffer_4Vertices(); }
#endif

	virtual void CreateDescriptorRanges(std::vector<D3D12_DESCRIPTOR_RANGE>& DescriptorRanges) const override { 
		CreateDescriptorRanges_1SRV(DescriptorRanges); 
	}
	virtual void CreateRootParameters(std::vector<D3D12_ROOT_PARAMETER>& RootParameters, const std::vector<D3D12_DESCRIPTOR_RANGE>& DescriptorRanges) const override { 
		CreateRootParameters_1SRV(RootParameters, DescriptorRanges, D3D12_SHADER_VISIBILITY_PIXEL);
	}
	virtual void CreateStaticSamplerDescs(std::vector<D3D12_STATIC_SAMPLER_DESC>& StaticSamplerDescs) const override {
		//!< LoadImageResource_DDS() 内で作成したサンプラを使用する (DDS から取得した MaxLOD を使用、D3D12_FLOAT32_MAXでも良いのだが念のため)
		StaticSamplerDescs.push_back(StaticSamplerDesc);
	}

	virtual void CreateShader(std::vector<Microsoft::WRL::ComPtr<ID3DBlob>>& ShaderBlobs, std::array<D3D12_SHADER_BYTECODE, 5>& ShaderBytecodes) const override {
		CreateShader_VsPs(ShaderBlobs, ShaderBytecodes);
	}

	virtual void CreateTexture() override {
#if 1
		LoadImage(ImageResource.GetAddressOf(), ImageDescriptorHeap.GetAddressOf(), L"UV.dds", D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
#else
		//LoadImage(ImageResource.GetAddressOf(), ImageDescriptorHeap.GetAddressOf(), L"..\\Intermediate\\Image\\kueken7_a8_unorm.dds", D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
		//LoadImage(ImageResource.GetAddressOf(), ImageDescriptorHeap.GetAddressOf(), L"..\\Intermediate\\Image\\kueken7_bgr8_srgb.dds", D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE); //!< #DX_TODO
		//LoadImage(ImageResource.GetAddressOf(), ImageDescriptorHeap.GetAddressOf(), L"..\\Intermediate\\Image\\kueken7_bgr8_unorm.dds", D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE); //!< #DX_TODO
		//LoadImage(ImageResource.GetAddressOf(), ImageDescriptorHeap.GetAddressOf(), L"..\\Intermediate\\Image\\kueken7_bgra8_srgb.dds", D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE); //!< OK
		//LoadImage(ImageResource.GetAddressOf(), ImageDescriptorHeap.GetAddressOf(), L"..\\Intermediate\\Image\\kueken7_bgra8_unorm.dds", D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE); //!< OK
		//LoadImage(ImageResource.GetAddressOf(), ImageDescriptorHeap.GetAddressOf(), L"..\\Intermediate\\Image\\kueken7_bgrx8_srgb.dds", D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE); //!< #DX_TODO
		//LoadImage(ImageResource.GetAddressOf(), ImageDescriptorHeap.GetAddressOf(), L"..\\Intermediate\\Image\\kueken7_bgrx8_unorm.dds", D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE); //!< #DX_TODO
		//LoadImage(ImageResource.GetAddressOf(), ImageDescriptorHeap.GetAddressOf(), L"..\\Intermediate\\Image\\kueken7_l8_unorm.dds", D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE); //!< OK
		//LoadImage(ImageResource.GetAddressOf(), ImageDescriptorHeap.GetAddressOf(), L"..\\Intermediate\\Image\\kueken7_r_ati1n_unorm.dds", D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE); //!< OK
		//LoadImage(ImageResource.GetAddressOf(), ImageDescriptorHeap.GetAddressOf(), L"..\\Intermediate\\Image\\kueken7_r5g6b5_unorm.dds", D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE); //!< #DX_TODO
		//LoadImage(ImageResource.GetAddressOf(), ImageDescriptorHeap.GetAddressOf(), L"..\\Intermediate\\Image\\kueken7_r8_sint.dds", D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
		//LoadImage(ImageResource.GetAddressOf(), ImageDescriptorHeap.GetAddressOf(), L"..\\Intermediate\\Image\\kueken7_r8_uint.dds", D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
		//LoadImage(ImageResource.GetAddressOf(), ImageDescriptorHeap.GetAddressOf(), L"..\\Intermediate\\Image\\kueken7_r16_unorm.dds", D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
		//LoadImage(ImageResource.GetAddressOf(), ImageDescriptorHeap.GetAddressOf(), L"..\\Intermediate\\Image\\kueken7_rg_ati2n_unorm.dds", D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE); //!< OK
		//LoadImage(ImageResource.GetAddressOf(), ImageDescriptorHeap.GetAddressOf(), L"..\\Intermediate\\Image\\kueken7_rg11b10_ufloat.dds", D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE); //!< #DX_TODO
		//LoadImage(ImageResource.GetAddressOf(), ImageDescriptorHeap.GetAddressOf(), L"..\\Intermediate\\Image\\kueken7_rgb_atc_unorm.dds", D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE); //!< #DX_TODO
		//LoadImage(ImageResource.GetAddressOf(), ImageDescriptorHeap.GetAddressOf(), L"..\\Intermediate\\Image\\kueken7_rgb_etc1_unorm.dds", D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE); //!< #DX_TODO
		//LoadImage(ImageResource.GetAddressOf(), ImageDescriptorHeap.GetAddressOf(), L"..\\Intermediate\\Image\\kueken7_rgb_etc2_srgb.dds", D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE); //!< #DX_TODO
		//LoadImage(ImageResource.GetAddressOf(), ImageDescriptorHeap.GetAddressOf(), L"..\\Intermediate\\Image\\kueken7_rgb_etc2_unorm.dds", D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE); //!< #DX_TODO
		//LoadImage(ImageResource.GetAddressOf(), ImageDescriptorHeap.GetAddressOf(), L"..\\Intermediate\\Image\\kueken7_rgb_pvrtc_2bpp_unorm.dds", D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE); //!< #DX_TODO
		//LoadImage(ImageResource.GetAddressOf(), ImageDescriptorHeap.GetAddressOf(), L"..\\Intermediate\\Image\\kueken7_rgb_pvrtc_4bpp_unorm.dds", D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE); //!< #DX_TODO
		//LoadImage(ImageResource.GetAddressOf(), ImageDescriptorHeap.GetAddressOf(), L"..\\Intermediate\\Image\\kueken7_rgb5a1_unorm.dds", D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE); //!< #DX_TODO
		//LoadImage(ImageResource.GetAddressOf(), ImageDescriptorHeap.GetAddressOf(), L"..\\Intermediate\\Image\\kueken7_rgb5a1_unorm_.dds", D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE); //!< #DX_TODO
		//LoadImage(ImageResource.GetAddressOf(), ImageDescriptorHeap.GetAddressOf(), L"..\\Intermediate\\Image\\kueken7_rgb9e5_ufloat.dds", D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE); //!< OK
		//LoadImage(ImageResource.GetAddressOf(), ImageDescriptorHeap.GetAddressOf(), L"..\\Intermediate\\Image\\kueken7_rgb10a2_unorm.dds", D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE); //!< #DX_TODO
		//LoadImage(ImageResource.GetAddressOf(), ImageDescriptorHeap.GetAddressOf(), L"..\\Intermediate\\Image\\kueken7_rgb10a2u.dds", D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE); //!< #DX_TODO
		//LoadImage(ImageResource.GetAddressOf(), ImageDescriptorHeap.GetAddressOf(), L"..\\Intermediate\\Image\\kueken7_rgba_astc4x4_srgb.dds", D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE); //!< #DX_TODO
		//LoadImage(ImageResource.GetAddressOf(), ImageDescriptorHeap.GetAddressOf(), L"..\\Intermediate\\Image\\kueken7_rgba_astc8x8_unorm.dds", D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE); //!< #DX_TODO
		//LoadImage(ImageResource.GetAddressOf(), ImageDescriptorHeap.GetAddressOf(), L"..\\Intermediate\\Image\\kueken7_rgba_atc_explicit_unorm.dds", D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE); //!< #DX_TODO
		//LoadImage(ImageResource.GetAddressOf(), ImageDescriptorHeap.GetAddressOf(), L"..\\Intermediate\\Image\\kueken7_rgba_atc_interpolate_unorm.dds", D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE); //!< #DX_TODO
		//LoadImage(ImageResource.GetAddressOf(), ImageDescriptorHeap.GetAddressOf(), L"..\\Intermediate\\Image\\kueken7_rgba_dxt1_srgb.dds", D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE); //!< #DX_TODO
		//LoadImage(ImageResource.GetAddressOf(), ImageDescriptorHeap.GetAddressOf(), L"..\\Intermediate\\Image\\kueken7_rgba_dxt1_unorm.dds", D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE); //!< #DX_TODO
		//LoadImage(ImageResource.GetAddressOf(), ImageDescriptorHeap.GetAddressOf(), L"..\\Intermediate\\Image\\kueken7_rgba_dxt5_srgb.dds", D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE); //!< #DX_TODO
		//LoadImage(ImageResource.GetAddressOf(), ImageDescriptorHeap.GetAddressOf(), L"..\\Intermediate\\Image\\kueken7_rgba_dxt5_unorm.dds", D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE); //!< #DX_TODO
		//LoadImage(ImageResource.GetAddressOf(), ImageDescriptorHeap.GetAddressOf(), L"..\\Intermediate\\Image\\kueken7_rgba_dxt5_unorm1.dds", D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE); //!< #DX_TODO
		//LoadImage(ImageResource.GetAddressOf(), ImageDescriptorHeap.GetAddressOf(), L"..\\Intermediate\\Image\\kueken7_rgba_dxt5_unorm2.dds", D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE); //!< #DX_TODO
		//LoadImage(ImageResource.GetAddressOf(), ImageDescriptorHeap.GetAddressOf(), L"..\\Intermediate\\Image\\kueken7_rgba_pvrtc2_4bpp_unorm.dds", D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE); //!< #DX_TODO
		//LoadImage(ImageResource.GetAddressOf(), ImageDescriptorHeap.GetAddressOf(), L"..\\Intermediate\\Image\\kueken7_rgba4_unorm.dds", D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE); //!< #DX_TODO
		//LoadImage(ImageResource.GetAddressOf(), ImageDescriptorHeap.GetAddressOf(), L"..\\Intermediate\\Image\\kueken7_rgba8_snorm.dds", D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE); //!< #DX_TODO
		//LoadImage(ImageResource.GetAddressOf(), ImageDescriptorHeap.GetAddressOf(), L"..\\Intermediate\\Image\\kueken7_rgba8_srgb.dds", D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE); //!< #DX_TODO
		//LoadImage(ImageResource.GetAddressOf(), ImageDescriptorHeap.GetAddressOf(), L"..\\Intermediate\\Image\\kueken7_rgba8_unorm.dds", D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE); //!< #DX_TODO
		//LoadImage(ImageResource.GetAddressOf(), ImageDescriptorHeap.GetAddressOf(), L"..\\Intermediate\\Image\\kueken7_rgba16_sfloat.dds", D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE); //!< #DX_TODO
		//LoadImage(ImageResource.GetAddressOf(), ImageDescriptorHeap.GetAddressOf(), L"..\\Intermediate\\Image\\kueken8_bgr8_unorm.dds", D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE); //!< #DX_TODO
		//LoadImage(ImageResource.GetAddressOf(), ImageDescriptorHeap.GetAddressOf(), L"..\\Intermediate\\Image\\kueken8_rgba_dxt1_unorm.dds", D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE); //!< #DX_TODO
		//LoadImage(ImageResource.GetAddressOf(), ImageDescriptorHeap.GetAddressOf(), L"..\\Intermediate\\Image\\kueken8_rgba8_srgb.dds", D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE); //!< #DX_TODO
#endif
	}
	virtual void CreateSampler(const D3D12_SHADER_VISIBILITY ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL, const FLOAT MaxLOD = (std::numeric_limits<FLOAT>::max)()) override {
		CreateSampler_LinearWrap(ShaderVisibility, MaxLOD);
	}

	virtual void CreatePipelineState() override { CreateGraphicsPipelineState(); }
	//virtual void PopulateCommandList(ID3D12GraphicsCommandList* CommandList, ID3D12Resource* SwapChainResource, const D3D12_CPU_DESCRIPTOR_HANDLE& DescriptorHandle) override;
	virtual void PopulateCommandList(ID3D12GraphicsCommandList* CommandList, ID3D12Resource* SwapChainResource, const D3D12_CPU_DESCRIPTOR_HANDLE& DescriptorHandle, const DirectX::XMVECTORF32& Color) override;
};
#pragma endregion