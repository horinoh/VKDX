#pragma once

#include "resource.h"

#pragma region Code
#include "../VKExt.h"

class ComputeVK : public VKExt
{
private:
	using Super = VKExt;
public:
	ComputeVK() : Super() {}
	virtual ~ComputeVK() {}

protected:

	//!< #VK_TODO コマンドバッファを作成
	//!< #VK_TODO 出力テクスチャ用のimage2Dを用意しないとならない

	virtual void CreatePipeline() override { Super::CreatePipeline_Compute(); }
	virtual void CreateIndirectBuffer() override { CreateIndirectBuffer_Dispatch(32, 1, 1); }

	virtual void CreateDescriptorSetLayoutBindings(std::vector<VkDescriptorSetLayoutBinding>& DescriptorSetLayoutBindings) const override {
		CreateDescriptorSetLayoutBindings_1SI(DescriptorSetLayoutBindings, VK_SHADER_STAGE_COMPUTE_BIT);
	}
	virtual void CreateDescriptorPoolSizes(std::vector<VkDescriptorPoolSize>& DescriptorPoolSizes) const override {
		CreateDescriptorPoolSizes_1SI(DescriptorPoolSizes);
	}
	virtual void CreateWriteDescriptorSets(std::vector<VkWriteDescriptorSet>& WriteDescriptorSets, const std::vector<VkDescriptorBufferInfo>& DescriptorBufferInfos, const std::vector<VkDescriptorImageInfo>& DescriptorImageInfos, const std::vector<VkBufferView>& BufferViews) const override {
		CreateWriteDescriptorSets_1SI(WriteDescriptorSets, DescriptorImageInfos);
	}
	virtual void UpdateDescriptorSet() override {
		UpdateDescriptorSet_1SI();
	}

	virtual void CreateShader(std::vector<VkShaderModule>& SM, std::vector<VkPipelineShaderStageCreateInfo>& CreateInfo) const override {
		CreateShader_Cs(SM, CreateInfo);
	}
	virtual void PopulateCommandBuffer(const size_t i) override;

	virtual void Draw() override { Dispatch(); }

private:
};
#pragma endregion