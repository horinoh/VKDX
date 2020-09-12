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

	//!< VK�̏ꍇ�C�~���[�^�u���T���v���ƒʏ�̃T���v���͊�{�I�ɓ������́A�f�X�N���v�^�Z�b�g���C�A�E�g�̎w�肪�قȂ邾��
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
			.unnormalizedCoordinates = VK_FALSE // addressing VK_FALSE:���K��[0.0-1.0], VK_TRUE:�摜�̃f�B�����V����
		};
		VERIFY_SUCCEEDED(vkCreateSampler(Device, &SCI, GetAllocationCallbacks(), &Samplers.back()));
	}
#endif

	virtual void CreateDescriptorSetLayout() override {
		DescriptorSetLayouts.emplace_back(VkDescriptorSetLayout());
#ifdef USE_IMMUTABLE_SAMPLER
		//!< �C�~���[�^�u���T���v�����g���ꍇ
		//!< �u�Z�b�g���C�A�E�g�ɉi���I�Ƀo�C���h����ύX�ł��Ȃ��v
		//!< (�R���o�C���h�C���[�W�T���v���[��ύX����ꍇ�́A�C���[�W�r���[�ւ̕ύX�͔��f����邪�T���v���ւ̕ύX�͖�������邱�ƂɂȂ�)
		assert(!Samplers.empty() && "");
		const std::array ISs = { Samplers[0] };
		VKExt::CreateDescriptorSetLayout(DescriptorSetLayouts.back(), 0, {
			{ 0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, static_cast<uint32_t>(ISs.size()), VK_SHADER_STAGE_FRAGMENT_BIT, ISs.data() }
		});
#else
		//!< �ʏ�̃T���v�����g���ꍇ
		VKExt::CreateDescriptorSetLayout(DescriptorSetLayouts.back(), 0, {
				{ 0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr },
			});
#endif
	}
	virtual void CreatePipelineLayout() override {
		assert(!DescriptorSetLayouts.empty() && "");
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
				{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1 }
		});
	}
	virtual void AllocateDescriptorSet() override {
		assert(!DescriptorSetLayouts.empty() && "");
		const std::array DSLs = { DescriptorSetLayouts[0] };
		assert(!DescriptorPools.empty() && "");
		const VkDescriptorSetAllocateInfo DSAI = {
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
			.pNext = nullptr,
			.descriptorPool = DescriptorPools[0],
			.descriptorSetCount = static_cast<uint32_t>(size(DSLs)), .pSetLayouts = data(DSLs)
		};
		DescriptorSets.emplace_back(VkDescriptorSet());
		VERIFY_SUCCEEDED(vkAllocateDescriptorSets(Device, &DSAI, &DescriptorSets.back()));
	}

	//!< VK�̏ꍇ�C�~���[�^�u���T���v���ƒʏ�̃T���v���͊�{�I�ɓ������́A�f�X�N���v�^�Z�b�g���C�A�E�g�̎w�肪�قȂ邾��
#ifndef USE_IMMUTABLE_SAMPLER
	virtual void CreateSampler() override {
		Samplers.emplace_back(VkSampler());
		const VkSamplerCreateInfo SCI = {
			.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
			.pNext = nullptr,
			.flags = 0,
			.magFilter = VK_FILTER_NEAREST, .minFilter = VK_FILTER_NEAREST, .mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST, //!< ��C�~���[�^�u���T���v���̏ꍇ������ NEAREST �ɂ��Ă���
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
		assert(!DescriptorSetLayouts.empty() && "");
		VK::CreateDescriptorUpdateTemplate(DescriptorUpdateTemplates.back(), {
			{
				0, 0,
				_countof(DescriptorUpdateInfo::DII), VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
				offsetof(DescriptorUpdateInfo, DII), sizeof(DescriptorUpdateInfo)
			},
		}, DescriptorSetLayouts[0]);
	}
	virtual void UpdateDescriptorSet() override {
#ifdef USE_IMMUTABLE_SAMPLER
		assert(!Samplers.empty() && "");
#endif
		const DescriptorUpdateInfo DUI = {
#ifdef USE_IMMUTABLE_SAMPLER
			{ VK_NULL_HANDLE, ImageViews[0], VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL },
#else
			{ Samplers[0], ImageViews[0], VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL },
#endif
		};
		assert(!DescriptorSets.empty() && "");
		assert(!DescriptorUpdateTemplates.empty() && "");
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