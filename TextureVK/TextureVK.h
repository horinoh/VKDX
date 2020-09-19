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
	virtual void CreateIndirectBuffer() override { CreateIndirectBuffer_Draw(4, 1); }

	virtual void CreateTexture() override {
		std::wstring Path;
		if (FindDirectory("DDS", Path)) {
			Images.emplace_back(Image());
			const auto GLITexture = LoadImage(&Images.back().Image, &Images.back().DeviceMemory, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, ToString(Path + TEXT("\\PavingStones050_2K-JPG\\PavingStones050_2K_Color.dds")));
			ImageViews.emplace_back(VkImageView());
			CreateImageView(&ImageViews.back(), Images.back().Image, GLITexture);
		}
	}

	//!< VKの場合イミュータブルサンプラと通常のサンプラは基本的に同じもの、デスクリプタセットレイアウトの指定が異なるだけ
#ifdef USE_IMMUTABLE_SAMPLER
	virtual void CreateImmutableSampler() override {
		Samplers.emplace_back(VkSampler());
		const VkSamplerCreateInfo SCI = {
			.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
			.pNext = nullptr,
			.flags = 0,
			.magFilter = VK_FILTER_LINEAR, .minFilter = VK_FILTER_LINEAR, .mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
			.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT, .addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT, .addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT,
			.mipLodBias = 0.0f,
			.anisotropyEnable = VK_FALSE, .maxAnisotropy = 1.0f,
			.compareEnable = VK_FALSE, .compareOp = VK_COMPARE_OP_NEVER,
			.minLod = 0.0f, .maxLod = 1.0f,
			.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE,
			.unnormalizedCoordinates = VK_FALSE // addressing VK_FALSE:正規化[0.0-1.0], VK_TRUE:画像のディメンション
		};
		VERIFY_SUCCEEDED(vkCreateSampler(Device, &SCI, GetAllocationCallbacks(), &Samplers.back()));
	}
#endif

	virtual void CreateDescriptorSetLayout() override {
		DescriptorSetLayouts.emplace_back(VkDescriptorSetLayout());
#ifdef USE_IMMUTABLE_SAMPLER
		//!< イミュータブルサンプラを使う場合
		//!< 「セットレイアウトに永続的にバインドされ変更できない」
		//!< (コンバインドイメージサンプラーを変更する場合は、イメージビューへの変更は反映されるがサンプラへの変更は無視されることになる)
		assert(!empty(Samplers) && "");
		const std::array ISs = { Samplers[0] };
		VKExt::CreateDescriptorSetLayout(DescriptorSetLayouts.back(), 0, {
			VkDescriptorSetLayoutBinding({ .binding = 0, .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, .descriptorCount = static_cast<uint32_t>(ISs.size()), .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT, .pImmutableSamplers = ISs.data() })
		});
#else
		//!< 通常のサンプラを使う場合
		VKExt::CreateDescriptorSetLayout(DescriptorSetLayouts.back(), 0, {
				VkDescriptorSetLayoutBinding({ .binding = 0, .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, .descriptorCount = 1, .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT, .pImmutableSamplers = nullptr }),
			});
#endif
	}
	virtual void CreatePipelineLayout() override {
		assert(!empty(DescriptorSetLayouts) && "");
		PipelineLayouts.emplace_back(VkPipelineLayout());
		VKExt::CreatePipelineLayout(PipelineLayouts.back(), {
				DescriptorSetLayouts[0] 
			}, {});
	}
	virtual void CreateRenderPass() { VK::CreateRenderPass(VK_ATTACHMENT_LOAD_OP_DONT_CARE, false); }
	virtual void CreateShaderModules() override { CreateShaderModle_VsFs(); }
	virtual void CreatePipelines() override { CreatePipeline_VsFs(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP, 0, VK_FALSE); }

	virtual void CreateDescriptorPool() override {
		DescriptorPools.emplace_back(VkDescriptorPool());
		VKExt::CreateDescriptorPool(DescriptorPools.back(), 0, {
			VkDescriptorPoolSize({ .type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, .descriptorCount = 1 })
		});
	}
	virtual void AllocateDescriptorSet() override {
		assert(!empty(DescriptorSetLayouts) && "");
		const std::array DSLs = { DescriptorSetLayouts[0] };
		assert(!empty(DescriptorPools) && "");
		const VkDescriptorSetAllocateInfo DSAI = {
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
			.pNext = nullptr,
			.descriptorPool = DescriptorPools[0],
			.descriptorSetCount = static_cast<uint32_t>(size(DSLs)), .pSetLayouts = data(DSLs)
		};
		DescriptorSets.emplace_back(VkDescriptorSet());
		VERIFY_SUCCEEDED(vkAllocateDescriptorSets(Device, &DSAI, &DescriptorSets.back()));
	}

	//!< VKの場合イミュータブルサンプラと通常のサンプラは基本的に同じもの、デスクリプタセットレイアウトの指定が異なるだけ
#ifndef USE_IMMUTABLE_SAMPLER
	virtual void CreateSampler() override {
		Samplers.emplace_back(VkSampler());
		const VkSamplerCreateInfo SCI = {
			.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
			.pNext = nullptr,
			.flags = 0,
			.magFilter = VK_FILTER_NEAREST, .minFilter = VK_FILTER_NEAREST, .mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST, //!< 非イミュータブルサンプラの場合敢えて NEAREST にしている
			.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT, .addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT, .addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT,
			.mipLodBias = 0.0f,
			.anisotropyEnable = VK_FALSE, .maxAnisotropy = 1.0f,
			.compareEnable = VK_FALSE, .compareOp = VK_COMPARE_OP_NEVER,
			.minLod = 0.0f, .maxLod = 1.0f,
			.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE,
			.unnormalizedCoordinates = VK_FALSE
		};
		VERIFY_SUCCEEDED(vkCreateSampler(Device, &SCI, GetAllocationCallbacks(), &Samplers.back()));
	}
#endif

	virtual void CreateDescriptorUpdateTemplate() override {
		DescriptorUpdateTemplates.emplace_back(VkDescriptorUpdateTemplate());
		assert(!empty(DescriptorSetLayouts) && "");
		VK::CreateDescriptorUpdateTemplate(DescriptorUpdateTemplates.back(), {
			VkDescriptorUpdateTemplateEntry({
				.dstBinding = 0, .dstArrayElement = 0,
				.descriptorCount = _countof(DescriptorUpdateInfo::DII), .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
				.offset = offsetof(DescriptorUpdateInfo, DII), .stride = sizeof(DescriptorUpdateInfo)
			}),
		}, DescriptorSetLayouts[0]);
	}
	virtual void UpdateDescriptorSet() override {
#ifdef USE_IMMUTABLE_SAMPLER
		assert(!empty(Samplers) && "");
#endif
		const DescriptorUpdateInfo DUI = {
#ifdef USE_IMMUTABLE_SAMPLER
			VkDescriptorImageInfo({ .sampler = VK_NULL_HANDLE, .imageView = ImageViews[0], .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL }),
#else
			VkDescriptorImageInfo({ .sampler = Samplers[0], .imageView = ImageViews[0], .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL }),
#endif
		};
		assert(!empty(DescriptorSets) && "");
		assert(!empty(DescriptorUpdateTemplates) && "");
		vkUpdateDescriptorSetWithTemplate(Device, DescriptorSets[0], DescriptorUpdateTemplates[0], &DUI);
	}

	virtual void PopulateCommandBuffer(const size_t i) override;

private:
	struct DescriptorUpdateInfo
	{
		VkDescriptorImageInfo DII[1];
	};
};
#pragma endregion