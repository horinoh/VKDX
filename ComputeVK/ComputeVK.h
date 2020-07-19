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

	virtual void CreateIndirectBuffer() override { CreateIndirectBuffer_Dispatch(32, 1, 1); }

	virtual void CreateDescriptorSetLayout() override {
		DescriptorSetLayouts.resize(1);
		VKExt::CreateDescriptorSetLayout(DescriptorSetLayouts[0], 0, {
				{ 0, /*VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER*/VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr }
			});
	}
	virtual void CreatePipelineLayout() override {
		assert(!DescriptorSetLayouts.empty() && "");
		PipelineLayouts.resize(1);
		VKExt::CreatePipelineLayout(PipelineLayouts[0], {
				DescriptorSetLayouts[0] 
			}, {});
	}

#pragma region DESCRIPTOR
	virtual void CreateDescriptorPool() override {
		DescriptorPools.resize(1);
		VKExt::CreateDescriptorPool(DescriptorPools[0], 0, {
				{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1 } 
			});
	}
	virtual void AllocateDescriptorSet() override {
		assert(!DescriptorSetLayouts.empty() && "");
		const std::array<VkDescriptorSetLayout, 1> DSLs = { DescriptorSetLayouts[0] };
		assert(!DescriptorPools.empty() && "");
		const VkDescriptorSetAllocateInfo DSAI = {
			VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
			nullptr,
			DescriptorPools[0],
			static_cast<uint32_t>(DSLs.size()), DSLs.data()
		};
		DescriptorSets.resize(1);
		VERIFY_SUCCEEDED(vkAllocateDescriptorSets(Device, &DSAI, &DescriptorSets[0]));
	}
	virtual void CreateDescriptorUpdateTemplate() override {
		DescriptorUpdateTemplates.resize(1);
		assert(!DescriptorSetLayouts.empty() && "");
		VK::CreateDescriptorUpdateTemplate(DescriptorUpdateTemplates[0], {
			{
				0, 0,
				_countof(DescriptorUpdateInfo::DII), /*VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER*/VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
				offsetof(DescriptorUpdateInfo, DII), sizeof(DescriptorUpdateInfo)
			},
		}, DescriptorSetLayouts[0]);
	}
	virtual void UpdateDescriptorSet() override {
		assert(!_ImageViews.empty() && "");
		const DescriptorUpdateInfo DUI = {
			{ VK_NULL_HANDLE, _ImageViews[0], VK_IMAGE_LAYOUT_GENERAL }
		};
		assert(!DescriptorSets.empty() && "");
		assert(!DescriptorUpdateTemplates.empty() && "");
		vkUpdateDescriptorSetWithTemplate(Device, DescriptorSets[0], DescriptorUpdateTemplates[0], &DUI);
	}
#pragma endregion //!< DESCRIPTOR

	virtual void CreateTexture() override {
		const auto Format = VK_FORMAT_R8G8B8A8_UINT;

		{
			_Images.resize(1);
			const auto Type = VK_IMAGE_TYPE_2D;

			const VkExtent3D Extent3D = { 800, 600, 1 };

			const auto Faces = 1;
			const auto Layers = 1 * Faces;
			const auto Levels = 1;

			//!< 参考 : VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT
			CreateImage(&_Images[0], 0, Type, Format, Extent3D, Levels, Layers, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_STORAGE_BIT);
#if 0
			AllocateImageMemory(&ImageDeviceMemory, Image, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
			VERIFY_SUCCEEDED(vkBindImageMemory(Device, Images[0], ImageDeviceMemory, 0));
#else
			uint32_t HeapIndex;
			VkDeviceSize Offset;
			SuballocateImageMemory(HeapIndex, Offset, _Images[0], VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
#endif
		}

		{
			_ImageViews.resize(1);
			const auto Type = VK_IMAGE_VIEW_TYPE_2D;
			const VkComponentMapping CompMap = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };

			CreateImageView(&_ImageViews[0], _Images[0], Type, Format, CompMap, ImageSubresourceRange_ColorAll);
		}
	}
	
	virtual void CreateShaderModules() override { CreateShaderModle_Cs(); }
	virtual void CreatePipelines() override { Pipelines.resize(1); CreatePipeline_Cs(Pipelines[0]); }
	virtual void PopulateCommandBuffer(const size_t i) override;
	virtual void Draw() override { Dispatch(); }

private:
	struct DescriptorUpdateInfo
	{
		VkDescriptorImageInfo DII[1];
	}; 
};
#pragma endregion