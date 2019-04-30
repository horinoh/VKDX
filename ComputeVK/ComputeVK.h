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

	virtual void CreateTexture() override {
		const auto Format = VK_FORMAT_R8G8B8A8_UINT;

		{
			const auto Type = VK_IMAGE_TYPE_2D;

			const VkExtent3D Extent3D = { 800, 600, 1 };

			const auto Faces = 1;
			const auto Layers = 1 * Faces;
			const auto Levels = 1;

			//!< 参考 : VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT
			CreateImage(&Image, 0, Type, Format, Extent3D, Levels, Layers, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_STORAGE_BIT);
			CreateDeviceLocalMemory(&ImageDeviceMemory, Image);
			BindDeviceMemory(Image, ImageDeviceMemory);
		}

		{
			const auto Type = VK_IMAGE_VIEW_TYPE_2D;
			const VkComponentMapping CompMap = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };

			CreateImageView(&ImageView, Image, Type, Format, CompMap, ImageSubresourceRange_ColorAll);
		}
	}

	virtual void PopulateCommandBuffer(const size_t i) override;

	virtual void Draw() override { Dispatch(); }

private:
};
#pragma endregion