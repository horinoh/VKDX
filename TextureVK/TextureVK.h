#pragma once

#include "resource.h"

#pragma region Code
#include "../VKImage.h"

class TextureVK : public VKImage
{
private:
	using Super = VKImage;
public:
	TextureVK() : Super() {}
	virtual ~TextureVK() {}

protected:
	virtual void CreateIndirectBuffer() override { CreateIndirectBuffer_Draw(4); }
	virtual void CreatePipelineLayout() override {
		DescriptorSetLayouts.resize(1);
		auto& DSL = DescriptorSetLayouts[0];
		CreateDescriptorSetLayout_1CIS(DSL, VK_SHADER_STAGE_FRAGMENT_BIT);
		CreatePipelineLayout_1DSL(DSL);
	}

	virtual void CreateDescriptorPool() override { CreateDescriptorPool_1CIS(); }
	virtual void UpdateDescriptorSet() override { UpdateDescriptorSet_1CIS(); }

	virtual void CreateTexture() override {
#if 1
		LoadImage(&Image, &ImageDeviceMemory, &ImageView, "UV.dds");
#else
		//LoadImage(&Image, &ImageDeviceMemory, &ImageView, "..\\Intermediate\\Image\\kueken7_a8_unorm.dds"); //!< #VK_TODO
		//LoadImage(&Image, &ImageDeviceMemory, &ImageView, "..\\Intermediate\\Image\\kueken7_bgr8_srgb.dds"); //!< #VK_TODO ... gli“à‚Å‚±‚¯‚é
		//LoadImage(&Image, &ImageDeviceMemory, &ImageView, "..\\Intermediate\\Image\\kueken7_bgr8_unorm.dds"); //!< #VK_TODO
		//LoadImage(&Image, &ImageDeviceMemory, &ImageView, "..\\Intermediate\\Image\\kueken7_bgra8_srgb.dds"); //!< #VK_TODO
		//LoadImage(&Image, &ImageDeviceMemory, &ImageView, "..\\Intermediate\\Image\\kueken7_bgra8_unorm.dds"); //!< OK
		//LoadImage(&Image, &ImageDeviceMemory, &ImageView, "..\\Intermediate\\Image\\kueken7_bgrx8_srgb.dds"); //!< OK 
		LoadImage(&Image, &ImageDeviceMemory, &ImageView, "..\\Intermediate\\Image\\kueken7_bgrx8_unorm.dds");
		//LoadImage(&Image, &ImageDeviceMemory, &ImageView, "..\\Intermediate\\Image\\kueken7_l8_unorm.dds");
		//LoadImage(&Image, &ImageDeviceMemory, &ImageView, "..\\Intermediate\\Image\\kueken7_r_ati1n_unorm.dds");
		//LoadImage(&Image, &ImageDeviceMemory, &ImageView, "..\\Intermediate\\Image\\kueken7_r5g6b5_unorm.dds");
		//LoadImage(&Image, &ImageDeviceMemory, &ImageView, "..\\Intermediate\\Image\\kueken7_r8_sint.dds");
		//LoadImage(&Image, &ImageDeviceMemory, &ImageView, "..\\Intermediate\\Image\\kueken7_r8_uint.dds");
		//LoadImage(&Image, &ImageDeviceMemory, &ImageView, "..\\Intermediate\\Image\\kueken7_r16_unorm.dds");
		//LoadImage(&Image, &ImageDeviceMemory, &ImageView, "..\\Intermediate\\Image\\kueken7_rg_ati2n_unorm.dds");
		//LoadImage(&Image, &ImageDeviceMemory, &ImageView, "..\\Intermediate\\Image\\kueken7_rg11b10_ufloat.dds");
		//LoadImage(&Image, &ImageDeviceMemory, &ImageView, "..\\Intermediate\\Image\\kueken7_rgb_atc_unorm.dds");
		//LoadImage(&Image, &ImageDeviceMemory, &ImageView, "..\\Intermediate\\Image\\kueken7_rgb_etc1_unorm.dds");
		//LoadImage(&Image, &ImageDeviceMemory, &ImageView, "..\\Intermediate\\Image\\kueken7_rgb_etc2_srgb.dds");
		//LoadImage(&Image, &ImageDeviceMemory, &ImageView, "..\\Intermediate\\Image\\kueken7_rgb_etc2_unorm.dds");
		//LoadImage(&Image, &ImageDeviceMemory, &ImageView, "..\\Intermediate\\Image\\kueken7_rgb_pvrtc_2bpp_unorm.dds");
		//LoadImage(&Image, &ImageDeviceMemory, &ImageView, "..\\Intermediate\\Image\\kueken7_rgb_pvrtc_4bpp_unorm.dds");
		//LoadImage(&Image, &ImageDeviceMemory, &ImageView, "..\\Intermediate\\Image\\kueken7_rgb5a1_unorm.dds");
		//LoadImage(&Image, &ImageDeviceMemory, &ImageView, "..\\Intermediate\\Image\\kueken7_rgb5a1_unorm_.dds");
		//LoadImage(&Image, &ImageDeviceMemory, &ImageView, "..\\Intermediate\\Image\\kueken7_rgb9e5_ufloat.dds");
		//LoadImage(&Image, &ImageDeviceMemory, &ImageView, "..\\Intermediate\\Image\\kueken7_rgb10a2_unorm.dds");
		//LoadImage(&Image, &ImageDeviceMemory, &ImageView, "..\\Intermediate\\Image\\kueken7_rgb10a2u.dds");
		//LoadImage(&Image, &ImageDeviceMemory, &ImageView, "..\\Intermediate\\Image\\kueken7_rgba_astc4x4_srgb.dds");
		//LoadImage(&Image, &ImageDeviceMemory, &ImageView, "..\\Intermediate\\Image\\kueken7_rgba_astc8x8_unorm.dds");
		//LoadImage(&Image, &ImageDeviceMemory, &ImageView, "..\\Intermediate\\Image\\kueken7_rgba_atc_explicit_unorm.dds");
		//LoadImage(&Image, &ImageDeviceMemory, &ImageView, "..\\Intermediate\\Image\\kueken7_rgba_atc_interpolate_unorm.dds");
		//LoadImage(&Image, &ImageDeviceMemory, &ImageView, "..\\Intermediate\\Image\\kueken7_rgba_dxt1_srgb.dds");
		//LoadImage(&Image, &ImageDeviceMemory, &ImageView, "..\\Intermediate\\Image\\kueken7_rgba_dxt1_unorm.dds");
		//LoadImage(&Image, &ImageDeviceMemory, &ImageView, "..\\Intermediate\\Image\\kueken7_rgba_dxt5_srgb.dds");
		//LoadImage(&Image, &ImageDeviceMemory, &ImageView, "..\\Intermediate\\Image\\kueken7_rgba_dxt5_unorm.dds");
		//LoadImage(&Image, &ImageDeviceMemory, &ImageView, "..\\Intermediate\\Image\\kueken7_rgba_dxt5_unorm1.dds");
		//LoadImage(&Image, &ImageDeviceMemory, &ImageView, "..\\Intermediate\\Image\\kueken7_rgba_dxt5_unorm2.dds");
		//LoadImage(&Image, &ImageDeviceMemory, &ImageView, "..\\Intermediate\\Image\\kueken7_rgba_pvrtc2_4bpp_unorm.dds");
		//LoadImage(&Image, &ImageDeviceMemory, &ImageView, "..\\Intermediate\\Image\\kueken7_rgba4_unorm.dds");
		//LoadImage(&Image, &ImageDeviceMemory, &ImageView, "..\\Intermediate\\Image\\kueken7_rgba8_snorm.dds");
		//LoadImage(&Image, &ImageDeviceMemory, &ImageView, "..\\Intermediate\\Image\\kueken7_rgba8_srgb.dds");
		//LoadImage(&Image, &ImageDeviceMemory, &ImageView, "..\\Intermediate\\Image\\kueken7_rgba8_unorm.dds");
		//LoadImage(&Image, &ImageDeviceMemory, &ImageView, "..\\Intermediate\\Image\\kueken7_rgba16_sfloat.dds");
		//LoadImage(&Image, &ImageDeviceMemory, &ImageView, "..\\Intermediate\\Image\\kueken8_bgr8_unorm.dds");
		//LoadImage(&Image, &ImageDeviceMemory, &ImageView, "..\\Intermediate\\Image\\kueken8_rgba_dxt1_unorm.dds");
		//LoadImage(&Image, &ImageDeviceMemory, &ImageView, "..\\Intermediate\\Image\\kueken8_rgba8_srgb.dds");
#endif
	}
	virtual void CreateSampler(VkSampler* Sampler, const float MaxLOD = (std::numeric_limits<float>::max)()) const override {
		CreateSampler_LR(Sampler, MaxLOD);
	}

	virtual void CreatePipeline() override { CreatePipeline_VsFs(); }
	virtual void PopulateCommandBuffer(const size_t i) override;
};
#pragma endregion