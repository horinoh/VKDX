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
	virtual void CreateSwapchain() override { VK::CreateSwapchain(GetCurrentPhysicalDevice(), Surface, GetClientRectWidth(), GetClientRectHeight(), VK_IMAGE_USAGE_TRANSFER_DST_BIT); }
	virtual void CreateGeometry() override { 
		const auto PDMP = GetCurrentPhysicalDeviceMemoryProperties();
		constexpr VkDispatchIndirectCommand DIC = { .x = 32, .y = 1, .z = 1 };
		IndirectBuffers.emplace_back().Create(Device, PDMP, DIC).SubmitCopyCommand(Device, PDMP, CommandBuffers[0], GraphicsQueue, sizeof(DIC), &DIC);
	}
	virtual void CreateTexture() override {
		if (!HasRayTracingSupport(GetCurrentPhysicalDevice())) { return; }
		//!< スワップチェインと同じカラーフォーマットにしておく、(レイアウトは変更したり戻したりするので、戻せるレイアウトにしておく(ここでは TRANSFER_SRC_OPTIMAL))
		StorageTextures.emplace_back().Create(Device, GetCurrentPhysicalDeviceMemoryProperties(), ColorFormat, VkExtent3D({ .width = static_cast<uint32_t>(GetClientRectWidth()), .height = static_cast<uint32_t>(GetClientRectHeight()), .depth = 1 }))
			.SubmitSetLayoutCommand(CommandBuffers[0], GraphicsQueue, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
	}
	virtual void CreatePipelineLayout() override {
		CreateDescriptorSetLayout(DescriptorSetLayouts.emplace_back(), 0, {
			VkDescriptorSetLayoutBinding({.binding = 0, .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, .descriptorCount = 1, .stageFlags = VK_SHADER_STAGE_COMPUTE_BIT, .pImmutableSamplers = nullptr }),
		});
		VK::CreatePipelineLayout(PipelineLayouts.emplace_back(), DescriptorSetLayouts, {});
	}
	virtual void CreatePipeline() override {
		const auto SM = VK::CreateShaderModule(data(GetBasePath() + TEXT(".comp.spv")));
		
		const std::array CPCIs = {
			VkComputePipelineCreateInfo({
				.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
				.pNext = nullptr,
	#ifdef _DEBUG
				.flags = VK_PIPELINE_CREATE_DISABLE_OPTIMIZATION_BIT,
	#else
				.flags = 0,
	#endif
				.stage = VkPipelineShaderStageCreateInfo({
					.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
					.pNext = nullptr, 
					.flags = 0, 
					.stage = VK_SHADER_STAGE_COMPUTE_BIT,
					.module = SM,
					.pName = "main", 
					.pSpecializationInfo = nullptr 
				}),
				.layout = PipelineLayouts[0],
				.basePipelineHandle = VK_NULL_HANDLE, .basePipelineIndex  = -1
			}),
		};
		VERIFY_SUCCEEDED(vkCreateComputePipelines(Device, VK_NULL_HANDLE, static_cast<uint32_t>(size(CPCIs)), data(CPCIs), GetAllocationCallbacks(), &Pipelines.emplace_back()));

		vkDestroyShaderModule(Device, SM, GetAllocationCallbacks()); 
	}
	virtual void CreateDescriptor() override {
		VKExt::CreateDescriptorPool(DescriptorPools.emplace_back(), VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT, {
			VkDescriptorPoolSize({.type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, .descriptorCount = 1 })
		});

		const std::array DSLs = { DescriptorSetLayouts[0] };
		const VkDescriptorSetAllocateInfo DSAI = {
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
			.pNext = nullptr,
			.descriptorPool = DescriptorPools[0],
			.descriptorSetCount = static_cast<uint32_t>(size(DSLs)), .pSetLayouts = data(DSLs)
		};
		VERIFY_SUCCEEDED(vkAllocateDescriptorSets(Device, &DSAI, &DescriptorSets.emplace_back()));

		const auto DII = VkDescriptorImageInfo({ .sampler = VK_NULL_HANDLE, .imageView = StorageTextures[0].View, .imageLayout = VK_IMAGE_LAYOUT_GENERAL });
		VkDescriptorUpdateTemplate DUT;
		VK::CreateDescriptorUpdateTemplate(DUT, {
			VkDescriptorUpdateTemplateEntry({
				.dstBinding = 0, .dstArrayElement = 0,
				.descriptorCount = 1, .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
				.offset = 0, .stride = sizeof(DII)
			}),
		}, DescriptorSetLayouts[0]);
		vkUpdateDescriptorSetWithTemplate(Device, DescriptorSets[0], DUT, &DII);

		vkDestroyDescriptorUpdateTemplate(Device, DUT, GetAllocationCallbacks());
	}
	
	virtual void PopulateCommandBuffer(const size_t i) override {
		{
			const auto CCB = ComputeCommandBuffers[0];
			constexpr VkCommandBufferBeginInfo CBBI = {
				.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
				.pNext = nullptr,
				.flags = 0,
				.pInheritanceInfo = nullptr
			};
			VERIFY_SUCCEEDED(vkBeginCommandBuffer(CCB, &CBBI)); {
				vkCmdBindPipeline(CCB, VK_PIPELINE_BIND_POINT_COMPUTE, Pipelines[0]);

				const std::array DSs = { DescriptorSets[0] };
				constexpr std::array<uint32_t, 0> DynamicOffsets = {};
				vkCmdBindDescriptorSets(CCB, VK_PIPELINE_BIND_POINT_COMPUTE, PipelineLayouts[0], 0, static_cast<uint32_t>(size(DSs)), data(DSs), static_cast<uint32_t>(size(DynamicOffsets)), data(DynamicOffsets));

				vkCmdDispatchIndirect(CCB, IndirectBuffers[0].Buffer, 0);
			} VERIFY_SUCCEEDED(vkEndCommandBuffer(CCB));
		}
		{
			const auto CB = CommandBuffers[i];
			constexpr VkCommandBufferBeginInfo CBBI = {
				.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
				.pNext = nullptr,
				.flags = 0,
				.pInheritanceInfo = nullptr
			};
			const auto RT = StorageTextures[0].Image;
			VERIFY_SUCCEEDED(vkBeginCommandBuffer(CB, &CBBI)); {
				PopulateBeginRenderTargetCommand(CB, RT); {
				} PopulateEndRenderTargetCommand(CB, RT, SwapchainImages[i], static_cast<uint32_t>(GetClientRectWidth()), static_cast<uint32_t>(GetClientRectHeight()));
			} VERIFY_SUCCEEDED(vkEndCommandBuffer(CB));
		}
	}
};
#pragma endregion