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

	//!< #VK_TODO 出力テクスチャ用のimage2Dを用意しないとならない

	virtual void AllocateCommandBuffer() override {
		const VkCommandPoolCreateInfo CPCI = {
			.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
			.pNext = nullptr,
			.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
			.queueFamilyIndex = ComputeQueueFamilyIndex
		};
		VERIFY_SUCCEEDED(vkCreateCommandPool(Device, &CPCI, GetAllocationCallbacks(), &CommandPools.emplace_back()));

		const VkCommandBufferAllocateInfo CBAI = {
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
			.pNext = nullptr,
			.commandPool = CommandPools.back(),
			.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
			.commandBufferCount = 1
		};
		VERIFY_SUCCEEDED(vkAllocateCommandBuffers(Device, &CBAI, &CommandBuffers.emplace_back()));
	}

	virtual void CreateGeometry() override { 
		const auto PDMP = GetCurrentPhysicalDeviceMemoryProperties();
		constexpr VkDispatchIndirectCommand DIC = { .x = 32, .y = 1, .z = 1 };
		IndirectBuffers.emplace_back().Create(Device, PDMP, DIC).SubmitCopyCommand(Device, PDMP, CommandBuffers[0], GraphicsQueue, sizeof(DIC), &DIC);
	}
	virtual void CreateTexture() override {
		//!< VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT : コンピュートシェーダでストレージターゲットとなり、フラグメントシェーダでサンプルされる #VK_TODO
		RenderTextures.emplace_back().Create(Device, GetCurrentPhysicalDeviceMemoryProperties(), VK_FORMAT_R8G8B8A8_UINT, VkExtent3D({ .width = 800, .height = 600, .depth = 1 }));
	}
	virtual void CreatePipelineLayout() override {
		CreateDescriptorSetLayout(DescriptorSetLayouts.emplace_back(), 0, {
			{ 0, /*VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER*/VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr }
		});

		VK::CreatePipelineLayout(PipelineLayouts.emplace_back(), DescriptorSetLayouts, {});
	}
	virtual void CreatePipeline() override {
		const auto ShaderPath = GetBasePath();
		const std::array SMs = {
			VK::CreateShaderModule(data(ShaderPath + TEXT(".comp.spv"))),
		};
	}
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
	virtual void UpdateDescriptorSet() override {
		VK::CreateDescriptorUpdateTemplate(DescriptorUpdateTemplates.emplace_back(), {
			VkDescriptorUpdateTemplateEntry({
				.dstBinding = 0, .dstArrayElement = 0,
				.descriptorCount = _countof(DescriptorUpdateInfo::DII), .descriptorType = /*VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER*/VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
				.offset = offsetof(DescriptorUpdateInfo, DII), .stride = sizeof(DescriptorUpdateInfo)
			}),
		}, DescriptorSetLayouts[0]);

		const DescriptorUpdateInfo DUI = {
			VkDescriptorImageInfo({ .sampler = VK_NULL_HANDLE, .imageView = RenderTextures[0].View, .imageLayout = VK_IMAGE_LAYOUT_GENERAL })
		};
		vkUpdateDescriptorSetWithTemplate(Device, DescriptorSets[0], DescriptorUpdateTemplates[0], &DUI);
	}
	
	virtual void PopulateCommandBuffer(const size_t i) override;

	virtual void Draw() override { Dispatch(); }

private:
	struct DescriptorUpdateInfo
	{
		VkDescriptorImageInfo DII[1];
	}; 
};
#pragma endregion