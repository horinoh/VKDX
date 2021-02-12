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

	virtual void CreateGeometry() override { 
		constexpr VkDispatchIndirectCommand DIC = { .x = 32, .y = 1, .z = 1 };
		IndirectBuffers.emplace_back().Create(Device, GetCurrentPhysicalDeviceMemoryProperties(), DIC, CommandBuffers[0], GraphicsQueue);
	}

	virtual void CreatePipelineLayout() override {
		CreateDescriptorSetLayout(DescriptorSetLayouts.emplace_back(), 0, {
			{ 0, /*VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER*/VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr }
		});

		VK::CreatePipelineLayout(PipelineLayouts.emplace_back(), DescriptorSetLayouts, {});
	}

#pragma region DESCRIPTOR
	virtual void CreateDescriptorSet() override {
		VK::CreateDescriptorPool(DescriptorPools.emplace_back(), 0, {
			{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1 }
		});

		const std::array DSLs = { DescriptorSetLayouts[0] };
		const VkDescriptorSetAllocateInfo DSAI = {
			VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
			nullptr,
			DescriptorPools[0],
			static_cast<uint32_t>(size(DSLs)), data(DSLs)
		};
		VERIFY_SUCCEEDED(vkAllocateDescriptorSets(Device, &DSAI, &DescriptorSets.emplace_back()));
	}
	//virtual void CreateDescriptorPool() override {
	//	DescriptorPools.resize(1);
	//	VKExt::CreateDescriptorPool(DescriptorPools[0], 0, {
	//			{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1 } 
	//	});
	//}
	//virtual void AllocateDescriptorSet() override {
	//	assert(!empty(DescriptorSetLayouts) && "");
	//	const std::array<VkDescriptorSetLayout, 1> DSLs = { DescriptorSetLayouts[0] };
	//	assert(!empty(DescriptorPools) && "");
	//	const VkDescriptorSetAllocateInfo DSAI = {
	//		VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
	//		nullptr,
	//		DescriptorPools[0],
	//		static_cast<uint32_t>(size(DSLs)), data(DSLs)
	//	};
	//	DescriptorSets.resize(1);
	//	VERIFY_SUCCEEDED(vkAllocateDescriptorSets(Device, &DSAI, &DescriptorSets[0]));
	//}
	virtual void UpdateDescriptorSet() override {
		VK::CreateDescriptorUpdateTemplate(DescriptorUpdateTemplates.emplace_back(), {
			VkDescriptorUpdateTemplateEntry({
				.dstBinding = 0, .dstArrayElement = 0,
				.descriptorCount = _countof(DescriptorUpdateInfo::DII), .descriptorType = /*VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER*/VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
				.offset = offsetof(DescriptorUpdateInfo, DII), .stride = sizeof(DescriptorUpdateInfo)
			}),
		}, DescriptorSetLayouts[0]);

		const DescriptorUpdateInfo DUI = {
			VkDescriptorImageInfo({ .sampler = VK_NULL_HANDLE, .imageView = ImageViews[0], .imageLayout = VK_IMAGE_LAYOUT_GENERAL })
		};
		vkUpdateDescriptorSetWithTemplate(Device, DescriptorSets[0], DescriptorUpdateTemplates[0], &DUI);
	}
#pragma endregion //!< DESCRIPTOR

	virtual void CreateTexture() override {
		const auto Format = VK_FORMAT_R8G8B8A8_UINT;

		{
			Images.push_back(Image());
			const auto Type = VK_IMAGE_TYPE_2D;
			const VkExtent3D Extent3D = { 800, 600, 1 };
			const auto Faces = 1;
			const auto Layers = 1 * Faces;
			const auto Levels = 1;
			//!< 参考 : VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT
			CreateImage(&Images.back().Image, 0, Type, Format, Extent3D, Levels, Layers, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_STORAGE_BIT);

			AllocateDeviceMemory(&Images.back().DeviceMemory, Images.back().Image, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
			VERIFY_SUCCEEDED(vkBindImageMemory(Device, Images.back().Image, Images.back().DeviceMemory, 0));

			ImageViews.push_back(VkImageView());
			const VkComponentMapping CompMap = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };
			const VkImageSubresourceRange ISR = {
				.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
				.baseMipLevel = 0, .levelCount = VK_REMAINING_MIP_LEVELS,
				.baseArrayLayer = 0, .layerCount = VK_REMAINING_ARRAY_LAYERS
			};
			CreateImageView(&ImageViews.back(), Images.back().Image, VK_IMAGE_VIEW_TYPE_2D, Format, CompMap, ISR);
		}
	}
	
	virtual void CreateShaderModule() override { CreateShaderModle_Cs(); }
	virtual void CreatePipeline() override { CreatePipeline_Cs(); }
	virtual void PopulateCommandBuffer(const size_t i) override;
	virtual void Draw() override { Dispatch(); }

private:
	struct DescriptorUpdateInfo
	{
		VkDescriptorImageInfo DII[1];
	}; 
};
#pragma endregion