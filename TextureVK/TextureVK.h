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
	virtual void CreateDescriptorSetLayoutBindings(std::vector<VkDescriptorSetLayoutBinding>& DescriptorSetLayoutBindings) const override {
		CreateDescriptorSetLayoutBindings_1CIS(DescriptorSetLayoutBindings, VK_SHADER_STAGE_FRAGMENT_BIT);
	}
	virtual void CreateDescriptorPoolSizes(std::vector<VkDescriptorPoolSize>& DescriptorPoolSizes) const override {
		CreateDescriptorPoolSizes_1CIS(DescriptorPoolSizes); 
	}
	virtual void CreateWriteDescriptorSets(std::vector<VkWriteDescriptorSet>& WriteDescriptorSets, VkDescriptorImageInfo* DescriptorImageInfo, VkDescriptorBufferInfo* DescriptorBufferInfo, VkBufferView* BufferView) const override {
		CreaateWriteDescriptorSets_1CIS(WriteDescriptorSets, DescriptorImageInfo, DescriptorBufferInfo, BufferView);
	}

	virtual void CreateShader(std::vector<VkShaderModule>& ShaderModules, std::vector<VkPipelineShaderStageCreateInfo>& PipelineShaderStageCreateInfos) const override {
		CreateShader_VsPs(ShaderModules, PipelineShaderStageCreateInfos);
	}
	
	virtual void CreateTexture() override {
		LoadImage(&Image, &ImageDeviceMemory, &ImageView, "UV.dds");
	}
	virtual void CreateSampler(const float MaxLOD = (std::numeric_limits<float>::max)()) override {
		CreateSampler_LinearRepeat(MaxLOD);
	}

	virtual void CreatePipeline() override { CreateGraphicsPipeline(); }
	virtual void PopulateCommandBuffer(const VkCommandBuffer CommandBuffer) override;
};
#pragma endregion