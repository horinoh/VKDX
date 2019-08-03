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

	virtual void CreateDescriptorSetLayout() override {
		DescriptorSetLayouts.resize(1);
		VKExt::CreateDescriptorSetLayout(DescriptorSetLayouts[0], {
				{ 0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr } 
			});
	}
	virtual void CreatePipelineLayout() override {
		assert(!DescriptorSetLayouts.empty() && "");
		VKExt::CreatePipelineLayout(PipelineLayout, DescriptorSetLayouts[0]);
	}

	virtual void CreateDescriptorPool() override {
		DescriptorPools.resize(1);
		VKExt::CreateDescriptorPool(DescriptorPools[0], {
				{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1 } 
			});
	}
	virtual void CreateDescriptorSet() override {
		assert(!DescriptorPools.empty() && "");
		assert(!DescriptorSetLayouts.empty() && "");
		DescriptorSets.resize(1);
		VKExt::CreateDescriptorSet(DescriptorSets[0], DescriptorPools[0], DescriptorSetLayouts[0]);
	}
	virtual void UpdateDescriptorSet() override {
		assert(!DescriptorSets.empty() && "");
		assert(VK_NULL_HANDLE != ImageView && "");
		UpdateDescriptorSet_1SI(DescriptorSets[0], ImageView);
	}

	//virtual void CreateShader(std::vector<VkShaderModule>& SM, std::vector<VkPipelineShaderStageCreateInfo>& CreateInfo) const override {
	//	CreateShader_Cs(SM, CreateInfo);
	//}

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
			CreateDeviceMemory(&ImageDeviceMemory, Image, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
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