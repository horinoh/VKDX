#pragma once

#include "resource.h"

#pragma region Code
#include "../VKExt.h"
#include "../Leap.h"

class LeapVK : public VKExt, public Leap
{
private:
	using Super = VKExt;
public:
	LeapVK() : Super(), Leap() {}
	virtual ~LeapVK() {}

protected:
protected:
	virtual void OnTimer(HWND hWnd, HINSTANCE hInstance) override {
		Super::OnTimer(hWnd, hInstance);
#ifdef USE_LEAP
		InterpolatedTrackingEvent();
#else
		for (auto i = 0; i < 5; ++i) {
			for (auto j = 0; j < 4; ++j) {
				Tracking.Hands[0][i][j] = glm::vec4(i * 0.2f + 0.1f, 0.5f, (j + 1) * 0.2f - 0.5f, 1.0f);
			}
		}
		for (auto i = 0; i < 5; ++i) {
			for (auto j = 0; j < 4; ++j) {
				Tracking.Hands[1][i][j] = glm::vec4(-i * 0.2f - 0.1f, 0.5f, (j + 1) * 0.2f - 0.5f, 1.0f);
			}
		}
#endif
#pragma region UB
		CopyToHostVisibleDeviceMemory(UniformBuffers[GetCurrentBackBufferIndex()].DeviceMemory, 0, sizeof(Tracking), &Tracking);
#pragma endregion
	}

	virtual void CreateGeometry() override {
		const auto PDMP = GetCurrentPhysicalDeviceMemoryProperties();
		constexpr VkDrawIndirectCommand DIC = { .vertexCount = 4, .instanceCount = 1, .firstVertex = 0, .firstInstance = 0 };
		IndirectBuffers.emplace_back().Create(Device, PDMP, DIC).SubmitCopyCommand(Device, PDMP, CommandBuffers[0], GraphicsQueue, sizeof(DIC), &DIC);
	}
#pragma region UB
	virtual void CreateUniformBuffer() override {
		for (size_t i = 0; i < size(SwapchainImages); ++i) {
			UniformBuffers.emplace_back().Create(Device, GetCurrentPhysicalDeviceMemoryProperties(), sizeof(Tracking));
		}
	}
#pragma endregion
	virtual void CreateTexture() override {
#ifdef USE_LEAP		
		//!< Leap イメージ
		{
			const auto Layers = static_cast<uint32_t>(size(ImageData));
			const auto Extent = VkExtent3D({ .width = ImageProperties[0].width, .height = ImageProperties[0].height, .depth = 1 });
			const auto Format = VK_FORMAT_R8_UNORM;

			Images.emplace_back();
			CreateImage(&Images.back().Image, 0, VK_IMAGE_TYPE_2D, Format, Extent, 1, Layers, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);
			AllocateDeviceMemory(&Images.back().DeviceMemory, Images.back().Image, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
			VERIFY_SUCCEEDED(vkBindImageMemory(Device, Images.back().Image, Images.back().DeviceMemory, 0));
			const VkImageViewCreateInfo IVCI = {
				.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
				.pNext = nullptr,
				.flags = 0,
				.image = Images.back().Image,
				.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY,
				.format = Format,
				.components = VkComponentMapping({.r = VK_COMPONENT_SWIZZLE_R, .g = VK_COMPONENT_SWIZZLE_G, .b = VK_COMPONENT_SWIZZLE_B, .a = VK_COMPONENT_SWIZZLE_A }),
				.subresourceRange = VkImageSubresourceRange({.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .baseMipLevel = 0, .levelCount = VK_REMAINING_MIP_LEVELS, .baseArrayLayer = 0, .layerCount = VK_REMAINING_ARRAY_LAYERS }),
			};
			VERIFY_SUCCEEDED(vkCreateImageView(Device, &IVCI, GetAllocationCallbacks(), &ImageViews.emplace_back()));

			UpdateLeapImage();
		}
		//!< ディストーションマップ
		{
			const auto Layers = static_cast<uint32_t>(size(DistortionMatrices));
			constexpr auto Extent = VkExtent3D({ .width = LEAP_DISTORTION_MATRIX_N, .height = LEAP_DISTORTION_MATRIX_N, .depth = 1 });
			const auto Format = VK_FORMAT_R32G32_SFLOAT;

			Images.emplace_back();
			CreateImage(&Images.back().Image, 0, VK_IMAGE_TYPE_2D, Format, Extent, 1, Layers, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);
			AllocateDeviceMemory(&Images.back().DeviceMemory, Images.back().Image, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
			VERIFY_SUCCEEDED(vkBindImageMemory(Device, Images.back().Image, Images.back().DeviceMemory, 0));
			const VkImageViewCreateInfo IVCI = {
				.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
				.pNext = nullptr,
				.flags = 0,
				.image = Images.back().Image,
				.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY,
				.format = Format,
				.components = VkComponentMapping({.r = VK_COMPONENT_SWIZZLE_R, .g = VK_COMPONENT_SWIZZLE_G, .b = VK_COMPONENT_SWIZZLE_B, .a = VK_COMPONENT_SWIZZLE_A }),
				.subresourceRange = VkImageSubresourceRange({.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .baseMipLevel = 0, .levelCount = VK_REMAINING_MIP_LEVELS, .baseArrayLayer = 0, .layerCount = VK_REMAINING_ARRAY_LAYERS }),
			};
			VERIFY_SUCCEEDED(vkCreateImageView(Device, &IVCI, GetAllocationCallbacks(), &ImageViews.emplace_back()));

			UpdateDistortionImage();
		}
#else
		CreateTextureArray1x1({ 0xff0000ff, 0xff00ff00 });
		CreateTextureArray1x1({ 0xffff0000, 0xff00ffff });
#endif
	}
	virtual void CreateImmutableSampler() override {
		//!< https://developer.leapmotion.com/documentation/v4/images.html
		//!< フィルタを LINEAR、ラップモードを CLAMP
		const VkSamplerCreateInfo SCI = {
			.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
			.pNext = nullptr,
			.flags = 0,
			.magFilter = VK_FILTER_LINEAR, .minFilter = VK_FILTER_LINEAR, .mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
			.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, .addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, .addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
			.mipLodBias = 0.0f,
			.anisotropyEnable = VK_FALSE, .maxAnisotropy = 1.0f,
			.compareEnable = VK_FALSE, .compareOp = VK_COMPARE_OP_NEVER,
			.minLod = 0.0f, .maxLod = 1.0f,
			.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE,
			.unnormalizedCoordinates = VK_FALSE
		};
		VERIFY_SUCCEEDED(vkCreateSampler(Device, &SCI, GetAllocationCallbacks(), &Samplers.emplace_back()));
	}
	virtual void CreatePipelineLayout() override {
		const std::array ISs = { Samplers[0] };
		CreateDescriptorSetLayout(DescriptorSetLayouts.emplace_back(), 0, {
			VkDescriptorSetLayoutBinding({.binding = 0, .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, .descriptorCount = static_cast<uint32_t>(size(ISs)), .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT, .pImmutableSamplers = data(ISs) }),
#pragma region SECOND_TEXTURE
			VkDescriptorSetLayoutBinding({.binding = 1, .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, .descriptorCount = static_cast<uint32_t>(size(ISs)), .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT, .pImmutableSamplers = data(ISs) }),
#pragma endregion
#pragma region UB
			VkDescriptorSetLayoutBinding({.binding = 2, .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, .descriptorCount = 1, .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT, .pImmutableSamplers = nullptr }),
#pragma endregion
		});
		VK::CreatePipelineLayout(PipelineLayouts.emplace_back(), DescriptorSetLayouts, {});
	}
	virtual void CreatePipeline() override { 
		const auto ShaderPath = GetBasePath();
		const std::array SMs = {
			VK::CreateShaderModule(data(ShaderPath + TEXT(".vert.spv"))),
			VK::CreateShaderModule(data(ShaderPath + TEXT(".frag.spv"))),
		};
		const std::array PSSCIs = {
			VkPipelineShaderStageCreateInfo({.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, .pNext = nullptr, .flags = 0, .stage = VK_SHADER_STAGE_VERTEX_BIT, .module = SMs[0], .pName = "main", .pSpecializationInfo = nullptr }),
			VkPipelineShaderStageCreateInfo({.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, .pNext = nullptr, .flags = 0, .stage = VK_SHADER_STAGE_FRAGMENT_BIT, .module = SMs[1], .pName = "main", .pSpecializationInfo = nullptr }),
		}; 
		CreatePipeline_VsFs(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP, 0, VK_FALSE, PSSCIs);
		for (auto i : SMs) { vkDestroyShaderModule(Device, i, GetAllocationCallbacks()); }
	}
	virtual void CreateDescriptorSet() override {
		VK::CreateDescriptorPool(DescriptorPools.emplace_back(), 0, {
#pragma region SECOND_TEXTURE
			VkDescriptorPoolSize({.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, .descriptorCount = 2 }),
#pragma endregion
#pragma region UB
			VkDescriptorPoolSize({.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, .descriptorCount = static_cast<uint32_t>(size(SwapchainImages)) }),
#pragma endregion
		});
		const std::array DSLs = { DescriptorSetLayouts[0] };
		const VkDescriptorSetAllocateInfo DSAI = {
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
			.pNext = nullptr,
			.descriptorPool = DescriptorPools[0],
			.descriptorSetCount = static_cast<uint32_t>(size(DSLs)), .pSetLayouts = data(DSLs)
		};
#pragma region UB
		for (size_t i = 0; i < size(SwapchainImages); ++i) {
			VERIFY_SUCCEEDED(vkAllocateDescriptorSets(Device, &DSAI, &DescriptorSets.emplace_back()));
		}
#pragma endregion
	}
	virtual void UpdateDescriptorSet() override {
		VK::CreateDescriptorUpdateTemplate(DescriptorUpdateTemplates.emplace_back(), {
			VkDescriptorUpdateTemplateEntry({
				.dstBinding = 0, .dstArrayElement = 0,
				.descriptorCount = _countof(DescriptorUpdateInfo::DII), .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
				.offset = offsetof(DescriptorUpdateInfo, DII), .stride = sizeof(DescriptorUpdateInfo)
			}),
#pragma region SECOND_TEXTURE
			VkDescriptorUpdateTemplateEntry({
				.dstBinding = 1, .dstArrayElement = 0,
				.descriptorCount = _countof(DescriptorUpdateInfo::DII_1), .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
				.offset = offsetof(DescriptorUpdateInfo, DII_1), .stride = sizeof(DescriptorUpdateInfo)
			}),
#pragma endregion
#pragma region UB
			VkDescriptorUpdateTemplateEntry({
				.dstBinding = 2, .dstArrayElement = 0,
				.descriptorCount = _countof(DescriptorUpdateInfo::DBI), .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
				.offset = offsetof(DescriptorUpdateInfo, DBI), .stride = sizeof(DescriptorUpdateInfo)
			}),
#pragma endregion
		}, DescriptorSetLayouts[0]);
#pragma region UB
		for (size_t i = 0; i < size(SwapchainImages); ++i) {
			const DescriptorUpdateInfo DUI = {
				VkDescriptorImageInfo({.sampler = VK_NULL_HANDLE, .imageView = ImageViews[0], .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL }),
#pragma region SECOND_TEXTURE
				VkDescriptorImageInfo({.sampler = VK_NULL_HANDLE, .imageView = ImageViews[1], .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL }),
#pragma endregion
				VkDescriptorBufferInfo({.buffer = UniformBuffers[i].Buffer, .offset = 0, .range = VK_WHOLE_SIZE }),
			};
			vkUpdateDescriptorSetWithTemplate(Device, DescriptorSets[i], DescriptorUpdateTemplates[0], &DUI);
		}
#pragma endregion
	}

	virtual void PopulateCommandBuffer(const size_t i) override;

#ifdef USE_LEAP
	virtual void OnHand(const LEAP_HAND& Hand) override {
		Leap::OnHand(Hand);
		
		const auto Index = eLeapHandType_Right == Hand.type ? 0 : 1;
		for (auto i = 0; i < _countof(Hand.digits); ++i) {
			const auto& Digit = Hand.digits[i];
			for (auto j = 0; j < _countof(Digit.bones); ++j) {
				const auto& Bone = Digit.bones[j];
				const auto x = std::clamp(Bone.next_joint.x, -100.0f, 100.0f) / 100.0f;
				const auto y = std::clamp(Bone.next_joint.y, 0.0f, 300.0f) / 300.0f;
				const auto z = std::clamp(Bone.next_joint.z, -100.0f, 100.0f) / 100.0f;
				Tracking.Hands[Index][i][j] = glm::vec4(x, y, z, 1.0f);
			}
		}
	}
	virtual void UpdateLeapImage() override {
		if (!empty(Images)) {
			const auto Layers = static_cast<uint32_t>(size(ImageData));
			const auto LayerSize = size(ImageData[0]);
			const auto TotalSize = Layers * LayerSize;
			const auto Extent = VkExtent3D({ .width = ImageProperties[0].width, .height = ImageProperties[0].height, .depth = 1 });

			VkBuffer Buffer;
			VkDeviceMemory DeviceMemory;
			{
				CreateBuffer(&Buffer, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, TotalSize);
				AllocateDeviceMemory(&DeviceMemory, Buffer, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
				VERIFY_SUCCEEDED(vkBindBufferMemory(Device, Buffer, DeviceMemory, 0));

				CopyToHostVisibleDeviceMemory(DeviceMemory, 0, TotalSize, data(ImageData[0]));

				std::vector<VkBufferImageCopy> BICs;
				for (uint32_t i = 0; i < Layers; ++i) {
					BICs.emplace_back(VkBufferImageCopy({
						.bufferOffset = i * LayerSize, .bufferRowLength = 0, .bufferImageHeight = 0,
						.imageSubresource = VkImageSubresourceLayers({.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .mipLevel = 0, .baseArrayLayer = i, .layerCount = 1 }),
						.imageOffset = VkOffset3D({.x = 0, .y = 0, .z = 0 }),
						.imageExtent = Extent }));
				}
				const auto& CB = CommandBuffers[0];

				constexpr VkCommandBufferBeginInfo CBBI = {
					.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
					.pNext = nullptr,
					.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
					.pInheritanceInfo = nullptr
				};
				VERIFY_SUCCEEDED(vkBeginCommandBuffer(CB, &CBBI)); {
					PopulateCommandBuffer_CopyBufferToImage(CB, Buffer, Images[0].Image, VK_ACCESS_SHADER_READ_BIT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, BICs, 1, Layers);
				} VERIFY_SUCCEEDED(vkEndCommandBuffer(CB));

				SubmitAndWait(GraphicsQueue, CB);
			}
			vkFreeMemory(Device, DeviceMemory, GetAllocationCallbacks());
			vkDestroyBuffer(Device, Buffer, GetAllocationCallbacks());
		}
	}
	virtual void UpdateDistortionImage() override {
		if (!empty(Images)) {
			const auto Layers = static_cast<uint32_t>(size(ImageData));
			constexpr auto LayerSize = sizeof(DistortionMatrices[0]);
			const auto TotalSize = Layers * LayerSize;
			constexpr auto Extent = VkExtent3D({ .width = LEAP_DISTORTION_MATRIX_N, .height = LEAP_DISTORTION_MATRIX_N, .depth = 1 });

			VkBuffer Buffer;
			VkDeviceMemory DeviceMemory;
			{
				CreateBuffer(&Buffer, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, TotalSize);
				AllocateDeviceMemory(&DeviceMemory, Buffer, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
				VERIFY_SUCCEEDED(vkBindBufferMemory(Device, Buffer, DeviceMemory, 0));

				CopyToHostVisibleDeviceMemory(DeviceMemory, 0, TotalSize, data(DistortionMatrices));

				std::vector<VkBufferImageCopy> BICs;
				for (uint32_t i = 0; i < Layers; ++i) {
					BICs.emplace_back(VkBufferImageCopy({
						.bufferOffset = i * LayerSize, .bufferRowLength = 0, .bufferImageHeight = 0,
						.imageSubresource = VkImageSubresourceLayers({.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .mipLevel = 0, .baseArrayLayer = i, .layerCount = 1 }),
						.imageOffset = VkOffset3D({.x = 0, .y = 0, .z = 0 }),
						.imageExtent = Extent }));
				}
				const auto& CB = CommandBuffers[0];

				constexpr VkCommandBufferBeginInfo CBBI = {
					.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
					.pNext = nullptr,
					.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
					.pInheritanceInfo = nullptr
				};
				VERIFY_SUCCEEDED(vkBeginCommandBuffer(CB, &CBBI)); {
					PopulateCommandBuffer_CopyBufferToImage(CB, Buffer, Images[1].Image, VK_ACCESS_SHADER_READ_BIT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, BICs, 1, Layers);
				} VERIFY_SUCCEEDED(vkEndCommandBuffer(CB));
				SubmitAndWait(GraphicsQueue, CB);
			}
			vkFreeMemory(Device, DeviceMemory, GetAllocationCallbacks());
			vkDestroyBuffer(Device, Buffer, GetAllocationCallbacks());
		}
	}
#endif

private:
	struct DescriptorUpdateInfo
	{
		VkDescriptorImageInfo DII[1];
#pragma region SECOND_TEXTURE
		VkDescriptorImageInfo DII_1[1]; 
#pragma endregion
#pragma region UB
		VkDescriptorBufferInfo DBI[1];
#pragma endregion
	};

	struct HandTracking
	{
		//!< 16バイトアライン境界をまたいではいけないので vec3 ではなく、vec4 を使用する
#ifdef USE_LEAP
		std::array<std::array<std::array<glm::vec4, _countof(LEAP_DIGIT::bones)>, _countof(LEAP_HAND::digits)>, 2> Hands;
#else
		std::array<std::array<std::array<glm::vec4, 4>, 5>, 2> Hands;
#endif
	};
	using HandTracking = struct HandTracking;
	HandTracking Tracking;
};
#pragma endregion