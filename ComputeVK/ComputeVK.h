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
		VKExt::CreateDescriptorSetLayout(DescriptorSetLayouts[0], {
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

#ifdef USE_DESCRIPTOR_UPDATE_TEMPLATE
	virtual void CreateDescriptorUpdateTemplate() override {
		const std::array<VkDescriptorUpdateTemplateEntry, 1> DUTEs = {
			{
				0, 0,
				_countof(DescriptorUpdateInfo::DescriptorImageInfos), /*VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER*/VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
				offsetof(DescriptorUpdateInfo, DescriptorImageInfos), sizeof(DescriptorUpdateInfo)
			}
		};

		assert(!DescriptorSetLayouts.empty() && "");
		const VkDescriptorUpdateTemplateCreateInfo DUTCI = {
			VK_STRUCTURE_TYPE_DESCRIPTOR_UPDATE_TEMPLATE_CREATE_INFO,
			nullptr,
			0,
			static_cast<uint32_t>(DUTEs.size()), DUTEs.data(),
			VK_DESCRIPTOR_UPDATE_TEMPLATE_TYPE_DESCRIPTOR_SET,
			DescriptorSetLayouts[0],
			VK_PIPELINE_BIND_POINT_COMPUTE, VK_NULL_HANDLE, 0
		};
		DescriptorUpdateTemplates.resize(1);
		VERIFY_SUCCEEDED(vkCreateDescriptorUpdateTemplate(Device, &DUTCI, GetAllocationCallbacks(), &DescriptorUpdateTemplates[0]));
	}
#endif

	virtual void CreateDescriptorPool() override {
		DescriptorPools.resize(1);
		VKExt::CreateDescriptorPool(DescriptorPools[0], /*VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT*/0, {
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
		for (auto& i : DescriptorSets) {
			VERIFY_SUCCEEDED(vkAllocateDescriptorSets(Device, &DSAI, &i));
		}
	}
	virtual void UpdateDescriptorSet() override {
		assert(VK_NULL_HANDLE != ImageView && "");
		const DescriptorUpdateInfo DUI = {
			{ VK_NULL_HANDLE, ImageView, VK_IMAGE_LAYOUT_GENERAL }
		};

		assert(!DescriptorSets.empty() && "");
#ifdef USE_DESCRIPTOR_UPDATE_TEMPLATE
		assert(!DescriptorUpdateTemplates.empty() && "");
		vkUpdateDescriptorSetWithTemplate(Device, DescriptorSets[0], DescriptorUpdateTemplates[0], &DUI);
#else
		VKExt::UpdateDescriptorSet(
			{
				{
					VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
					nullptr,
					DescriptorSets[0], 0, 0,
					_countof(DescriptorUpdateInfo::DescriptorImageInfos), VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, DUI.DescriptorImageInfos, nullptr, nullptr
				}
},
			{});
#endif
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
			AllocateImageMemory(&ImageDeviceMemory, Image, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
			VERIFY_SUCCEEDED(vkBindImageMemory(Device, Image, ImageDeviceMemory, 0));
		}

		{
			const auto Type = VK_IMAGE_VIEW_TYPE_2D;
			const VkComponentMapping CompMap = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };

			CreateImageView(&ImageView, Image, Type, Format, CompMap, ImageSubresourceRange_ColorAll);
		}
	}
	
	virtual void CreateShaderModule() override { CreateShaderModle_Cs(); }
	virtual void CreatePipeline() override { CreatePipeline_Cs(); }
	virtual void PopulateCommandBuffer(const size_t i) override;
	virtual void Draw() override { Dispatch(); }

private:
	struct DescriptorUpdateInfo
	{
		VkDescriptorImageInfo DescriptorImageInfos[1];
	}; 
};
#pragma endregion