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
	virtual void CreateDescriptorSetLayout() override {
		DescriptorSetLayouts.resize(1);
#if 0
		VKExt::CreateDescriptorSetLayout(DescriptorSetLayouts[0],
			{
				{ 0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr },
			});
#else
		assert(!ImmutableSamplers.empty() && "");
		//!< ImmutableSampler(DX の STATIC_SAMPLER_DESC 相当と思われる)を使う場合 
		//!< セットレイアウトに永続的にバインドされ変更できない
		//!< コンバインドイメージサンプラーの変更の場合は、イメージビューへの変更は反映されるがサンプラへの変更は無視される
		const std::array<VkSampler, 1> ISs = { ImmutableSamplers[0] };
		VKExt::CreateDescriptorSetLayout(DescriptorSetLayouts[0], {
				{ 0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, static_cast<uint32_t>(ISs.size()), VK_SHADER_STAGE_FRAGMENT_BIT, ISs.data() }
			});
#endif
	}
	virtual void CreatePipelineLayout() override {
		assert(!DescriptorSetLayouts.empty() && "");
		PipelineLayouts.resize(1);
		VKExt::CreatePipelineLayout(PipelineLayouts[0], {
				DescriptorSetLayouts[0] 
			}, {});
	}

	virtual void CreateDescriptorPool() override { 
		DescriptorPools.resize(1);
		VKExt::CreateDescriptorPool(DescriptorPools[0], /*VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT*/0, {
				{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1 } 
			});
	}
	virtual void AllocateDescriptorSet() override {
		assert(!DescriptorPools.empty() && "");
		assert(!DescriptorSetLayouts.empty() && "");
		std::vector<VkDescriptorSet> DSs;
		VKExt::AllocateDescriptorSet(DSs, DescriptorPools[0], {
				DescriptorSetLayouts[0]
			});
		std::copy(DSs.begin(), DSs.end(), std::back_inserter(DescriptorSets));
	}
	virtual void UpdateDescriptorSet() override {
		assert(!DescriptorSets.empty() && "");
		//assert(!Samplers.empty() && "");
		assert(VK_NULL_HANDLE != ImageView && "");
		const std::array<VkDescriptorImageInfo, 1> DIIs = {
			//!< ImmutableSampler使用のためサンプラはどうせ変更できない(無視される)
			{ /*Samplers[0]*/VK_NULL_HANDLE, ImageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL }
		};

		VKExt::UpdateDescriptorSet(
			{
				{
					VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
					nullptr,
					DescriptorSets[0], 0, 0,
					static_cast<uint32_t>(DIIs.size()), VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, DIIs.data(), nullptr, nullptr
				}
			},
			{});
	}

	virtual void CreateTexture() override {
#if 1
		LoadImage(&Image, &ImageDeviceMemory, &ImageView, "UV.dds");
#else
		//LoadImage(&Image, &ImageDeviceMemory, &ImageView, "..\\Intermediate\\Image\\kueken7_a8_unorm.dds"); //!< #VK_TODO
		//LoadImage(&Image, &ImageDeviceMemory, &ImageView, "..\\Intermediate\\Image\\kueken7_bgr8_srgb.dds"); //!< #VK_TODO ... gli内でこける
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
	virtual void CreateImmutableSampler() override {
		ImmutableSamplers.resize(1);
		const VkSamplerCreateInfo SCI = {
			VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
			nullptr,
			0,
			VK_FILTER_LINEAR, VK_FILTER_LINEAR, VK_SAMPLER_MIPMAP_MODE_LINEAR, // min, mag, mip
			VK_SAMPLER_ADDRESS_MODE_REPEAT, VK_SAMPLER_ADDRESS_MODE_REPEAT, VK_SAMPLER_ADDRESS_MODE_REPEAT, // u, v, w
			0.0f, // lod bias
			VK_FALSE, 1.0f, // anisotropy
			VK_FALSE, VK_COMPARE_OP_NEVER, // compare
			0.0f, 1.0f, // min, maxlod
			VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE, // border
			VK_FALSE // addressing VK_FALSE:正規化[0.0-1.0], VK_TRUE:画像のディメンション
		};
		VERIFY_SUCCEEDED(vkCreateSampler(Device, &SCI, GetAllocationCallbacks(), &ImmutableSamplers[0]));
	}
	virtual void CreateShaderModule() override { CreateShaderModle_VsFs(); }
	virtual void CreatePipeline() override { CreatePipeline_VsFs(); }
	virtual void PopulateCommandBuffer(const size_t i) override;
};
#pragma endregion