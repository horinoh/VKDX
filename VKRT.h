#pragma once

#include "VKExt.h"

class VKRT : public VKExt
{
private:
	using Super = VKExt;
protected:
	//!< #TIPS VK�C���X�^���X�쐬���� "VK_LAYER_RENDERDOC_Capture" ���g�p����ƁA���b�V���V�F�[�_�[�⃌�C�g���[�V���O�Ɠ����Ɏg�p�����ꍇ�AvkCreateDevice() �ŃR�P��悤�ɂȂ�̂Œ��� (If we use "VK_LAYER_RENDERDOC_Capture" with mesh shader or raytracing, vkCreateDevice() failed)
	virtual void CreateDevice(HWND hWnd, HINSTANCE hInstance, [[maybe_unused]] void* pNext, [[maybe_unused]] const std::vector<const char*>& AddExtensions) override {
		if (HasMeshShaderSupport(GetCurrentPhysicalDevice())) {
#ifdef _DEBUG
			uint32_t APIVersion; VERIFY_SUCCEEDED(vkEnumerateInstanceVersion(&APIVersion)); assert(APIVersion >= VK_MAKE_VERSION(1, 2, 162) && "RayTracing require 1.2.162 or later");
#endif
			VkPhysicalDeviceBufferDeviceAddressFeatures PDBDAF = { .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES, .pNext = nullptr, .bufferDeviceAddress = VK_TRUE };
			VkPhysicalDeviceRayTracingPipelineFeaturesKHR PDRTPF = { .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR, .pNext = &PDBDAF, .rayTracingPipeline = VK_TRUE, .rayTracingPipelineTraceRaysIndirect = VK_TRUE };
			VkPhysicalDeviceAccelerationStructureFeaturesKHR PDASF = { .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR, .pNext = &PDRTPF, .accelerationStructure = VK_TRUE };
			Super::CreateDevice(hWnd, hInstance, &PDASF, {
				VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME,
				VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME,
				//!< VK_KHR_acceleration_structure
				VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME,
				VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME,
				VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME,
				//!< VK_KHR_ray_tracing_pipeline
				VK_KHR_SPIRV_1_4_EXTENSION_NAME,
				//!< VK_KHR_spirv_1_4
				VK_KHR_SHADER_FLOAT_CONTROLS_EXTENSION_NAME,
				});
		}
		else {
			Super::CreateDevice(hWnd, hInstance);
		}
	}
	virtual void CreateSwapchain() override { VK::CreateSwapchain(GetCurrentPhysicalDevice(), Surface, GetClientRectWidth(), GetClientRectHeight(), VK_IMAGE_USAGE_TRANSFER_DST_BIT); }
	virtual void CreateTexture() override {
		if (!HasRayTracingSupport(GetCurrentPhysicalDevice())) { return; }
		//!< �X���b�v�`�F�C���Ɠ����J���[�t�H�[�}�b�g�ɂ��Ă����A(���C�A�E�g�͕ύX������߂����肷��̂ŁA�߂��郌�C�A�E�g�ɂ��Ă���(�����ł� TRANSFER_SRC_OPTIMAL))
		StorageTextures.emplace_back().Create(Device, GetCurrentPhysicalDeviceMemoryProperties(), ColorFormat, VkExtent3D({ .width = static_cast<uint32_t>(GetClientRectWidth()), .height = static_cast<uint32_t>(GetClientRectHeight()), .depth = 1 }))
			.SubmitSetLayoutCommand(CommandBuffers[0], GraphicsQueue, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
	}
	virtual void CreatePipelineLayout() override {
		if (!HasRayTracingSupport(GetCurrentPhysicalDevice())) { return; }
		CreateDescriptorSetLayout(DescriptorSetLayouts.emplace_back(), 0, {
			VkDescriptorSetLayoutBinding({.binding = 0, .descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, .descriptorCount = 1, .stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR, .pImmutableSamplers = nullptr }),
			VkDescriptorSetLayoutBinding({.binding = 1, .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, .descriptorCount = 1, .stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR, .pImmutableSamplers = nullptr }),
			});
		VK::CreatePipelineLayout(PipelineLayouts.emplace_back(), DescriptorSetLayouts, {});
	}
	virtual void CreateDescriptor() override {
		VKExt::CreateDescriptorPool(DescriptorPools.emplace_back(), VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT, {
			VkDescriptorPoolSize({.type = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, .descriptorCount = 1 }),
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

		const std::array ASs = { TLASs[0].AccelerationStructure };
		const auto WDSAS = VkWriteDescriptorSetAccelerationStructureKHR({ .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_KHR, .pNext = nullptr, .accelerationStructureCount = static_cast<uint32_t>(size(ASs)), .pAccelerationStructures = data(ASs) });
		const auto DII = VkDescriptorImageInfo({ .sampler = VK_NULL_HANDLE, .imageView = StorageTextures[0].View, .imageLayout = VK_IMAGE_LAYOUT_GENERAL });
		const std::array WDSs = {
			VkWriteDescriptorSet({
				.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
				.pNext = &WDSAS, //!< pNext �� VkWriteDescriptorSetAccelerationStructureKHR ���w�肷�邱��
				.dstSet = DescriptorSets[0],
				.dstBinding = 0, .dstArrayElement = 0,
				.descriptorCount = 1, .descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR,
				.pImageInfo = nullptr, .pBufferInfo = nullptr, .pTexelBufferView = nullptr
			}),
			VkWriteDescriptorSet({
				.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
				.pNext = nullptr,
				.dstSet = DescriptorSets[0],
				.dstBinding = 1, .dstArrayElement = 0,
				.descriptorCount = 1, .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
				.pImageInfo = &DII, .pBufferInfo = nullptr, .pTexelBufferView = nullptr
			})
		};
		constexpr std::array<VkCopyDescriptorSet, 0> CDSs = {};
		vkUpdateDescriptorSets(Device, static_cast<uint32_t>(size(WDSs)), data(WDSs), static_cast<uint32_t>(size(CDSs)), data(CDSs));

		//!< #VK_TODO vkUpdateDescriptorSetWithTemplate() �� AccelerationStructure �Ή�
#if 0
		VK::CreateDescriptorUpdateTemplate(DescriptorUpdateTemplates.emplace_back(), {
			VkDescriptorUpdateTemplateEntry({
				.dstBinding = 0, .dstArrayElement = 0,
				.descriptorCount = _countof(DescriptorUpdateInfo::DescriptorASInfos), .descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR,
				.offset = offsetof(DescriptorUpdateInfo, DescriptorASInfos), .stride = sizeof(DescriptorUpdateInfo)
			}),
			VkDescriptorUpdateTemplateEntry({
				.dstBinding = 1, .dstArrayElement = 0,
				.descriptorCount = _countof(DescriptorUpdateInfo::DescriptorImageInfos), .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
				.offset = offsetof(DescriptorUpdateInfo, DescriptorImageInfos), .stride = sizeof(DescriptorUpdateInfo)
			}),
			}, DescriptorSetLayouts[0]);

		const DescriptorUpdateInfo DUI = {
			VkDescriptorBufferInfo({.buffer = TLASs[0].Buffer, .offset = 0, .range = VK_WHOLE_SIZE }), //!< TLAS
			VkDescriptorImageInfo({.sampler = VK_NULL_HANDLE, .imageView = StorageTextures[0].View, .imageLayout = VK_IMAGE_LAYOUT_GENERAL }), //!< StorageImage
		};
		vkUpdateDescriptorSetWithTemplate(Device, DescriptorSets[0], DescriptorUpdateTemplates[0], &DUI);
#endif
	}
	virtual void PopulateBeginRenderTargetCommand(const size_t i) {
		const auto CB = CommandBuffers[i];
		constexpr auto ISR = VkImageSubresourceRange({ .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .baseMipLevel = 0, .levelCount = 1, .baseArrayLayer = 0, .layerCount = 1 });
		const std::array IMBs = {
			//!< RenderTarget : TRANSFER_SRC_OPTIMAL -> GENERAL
			VkImageMemoryBarrier({
				.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
				.pNext = nullptr,
				.srcAccessMask = 0, .dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT,
				.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, .newLayout = VK_IMAGE_LAYOUT_GENERAL,
				.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED, .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
				.image = StorageTextures[0].Image,
				.subresourceRange = ISR
			})
		};
		vkCmdPipelineBarrier(CB, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, nullptr, 0, nullptr, static_cast<uint32_t>(size(IMBs)), data(IMBs));
	}
	virtual void PopulateEndRenderTargetCommand(const size_t i) {
		const auto CB = CommandBuffers[i];
		constexpr auto ISR = VkImageSubresourceRange({ .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .baseMipLevel = 0, .levelCount = 1, .baseArrayLayer = 0, .layerCount = 1 });
		{
			const std::array IMBs = {
				//!< RenderTarget : GENERAL -> TRANSFER_SRC_OPTIMAL
				VkImageMemoryBarrier({
					.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
					.pNext = nullptr,
					.srcAccessMask = 0, .dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT,
					.oldLayout = VK_IMAGE_LAYOUT_GENERAL, .newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
					.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED, .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
					.image = StorageTextures[0].Image,
					.subresourceRange = ISR
				}),
				//!< Swapchain : PRESENT_SRC_KHR -> TRANSFER_DST_OPTIMAL
				VkImageMemoryBarrier({
					.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
					.pNext = nullptr,
					.srcAccessMask = 0, .dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
					.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED, .newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
					.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED, .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
					.image = SwapchainImages[i],
					.subresourceRange = ISR
				}),
			};
			vkCmdPipelineBarrier(CB, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, nullptr, 0, nullptr, static_cast<uint32_t>(size(IMBs)), data(IMBs));
		}

		const std::array ICs = {
			VkImageCopy({
				.srcSubresource = VkImageSubresourceLayers({.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .mipLevel = 0, .baseArrayLayer = 0, .layerCount = 1 }),
				.srcOffset = VkOffset3D({.x = 0, .y = 0, .z = 0}),
				.dstSubresource = VkImageSubresourceLayers({.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .mipLevel = 0, .baseArrayLayer = 0, .layerCount = 1 }),
				.dstOffset = VkOffset3D({.x = 0, .y = 0, .z = 0}),
				.extent = VkExtent3D({.width = static_cast<uint32_t>(GetClientRectWidth()), .height = static_cast<uint32_t>(GetClientRectHeight()), .depth = 1 }),
			}),
		};
		vkCmdCopyImage(CB, StorageTextures[0].Image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, SwapchainImages[i], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, static_cast<uint32_t>(size(ICs)), data(ICs));

		{
			const std::array IMBs = {
				//!< Swapchain : TRANSFER_DST_OPTIMAL -> PRESENT_SRC_KHR, 
				VkImageMemoryBarrier({
					.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
					.pNext = nullptr,
					.srcAccessMask = 0, .dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
					.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, .newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
					.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED, .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
					.image = SwapchainImages[i],
					.subresourceRange = ISR
				}),
			};
			vkCmdPipelineBarrier(CB, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, nullptr, 0, nullptr, static_cast<uint32_t>(size(IMBs)), data(IMBs));
		}
	}
};