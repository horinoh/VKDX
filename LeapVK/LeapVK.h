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
	virtual void OnUpdate(const uint32_t Index) override {
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
		CopyToHostVisibleDeviceMemory(Device, UniformBuffers[Index].DeviceMemory, 0, sizeof(Tracking), &Tracking);
#pragma endregion
	}
	virtual void CreateGeometry() override {
		const auto& PDMP = SelectedPhysDevice.second.PDMP;
		constexpr VkDrawIndirectCommand DIC = { .vertexCount = 4, .instanceCount = 1, .firstVertex = 0, .firstInstance = 0 };
		IndirectBuffers.emplace_back().Create(Device, PDMP, DIC).SubmitCopyCommand(Device, PDMP, CommandBuffers[0], GraphicsQueue, sizeof(DIC), &DIC);
	}
#pragma region UB
	virtual void CreateUniformBuffer() override {
		for([[maybe_unused]] const auto& i : Swapchain.ImageAndViews) {
			UniformBuffers.emplace_back().Create(Device, SelectedPhysDevice.second.PDMP, sizeof(Tracking));
		}
	}
#pragma endregion
	virtual void CreateTexture() override {
#ifdef USE_LEAP		
		//!< Leap イメージ
		{
			Textures.emplace_back().Create(Device, GetCurrentPhysicalDeviceMemoryProperties(), VK_FORMAT_R8_UNORM, VkExtent3D({ .width = ImageProperties[0].width, .height = ImageProperties[0].height, .depth = 1 }), 1, static_cast<uint32_t>(size(ImageData)));
			UpdateLeapImage();
		}
		//!< ディストーションマップ
		{
			Textures.emplace_back().Create(Device, GetCurrentPhysicalDeviceMemoryProperties(), VK_FORMAT_R32G32_SFLOAT, VkExtent3D({ .width = LEAP_DISTORTION_MATRIX_N, .height = LEAP_DISTORTION_MATRIX_N, .depth = 1 }), 1, static_cast<uint32_t>(size(DistortionMatrices)));
			UpdateDistortionImage();
		}
#else	
		CreateTextureArray1x1({ 0xff0000ff, 0xff00ff00 }, VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT);
		CreateTextureArray1x1({ 0xffff0000, 0xff00ffff }, VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT);
#endif
	}
	virtual void CreateImmutableSampler() override {
		//!< https://developer.leapmotion.com/documentation/v4/images.html
		//!< フィルタを LINEAR、ラップモードを CLAMP
		CreateImmutableSampler_LC();
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
		Pipelines.emplace_back();

		const std::array SMs = {
			VK::CreateShaderModule(GetFilePath(".vert.spv")),
			VK::CreateShaderModule(GetFilePath(".frag.spv")),
		};
		const std::array PSSCIs = {
			VkPipelineShaderStageCreateInfo({.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, .pNext = nullptr, .flags = 0, .stage = VK_SHADER_STAGE_VERTEX_BIT, .module = SMs[0], .pName = "main", .pSpecializationInfo = nullptr }),
			VkPipelineShaderStageCreateInfo({.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, .pNext = nullptr, .flags = 0, .stage = VK_SHADER_STAGE_FRAGMENT_BIT, .module = SMs[1], .pName = "main", .pSpecializationInfo = nullptr }),
		}; 
		
		constexpr VkPipelineRasterizationStateCreateInfo PRSCI = {
			.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
			.pNext = nullptr,
			.flags = 0,
			.depthClampEnable = VK_FALSE,
			.rasterizerDiscardEnable = VK_FALSE,
			.polygonMode = VK_POLYGON_MODE_FILL,
			.cullMode = VK_CULL_MODE_BACK_BIT,
			.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
			.depthBiasEnable = VK_FALSE, .depthBiasConstantFactor = 0.0f, .depthBiasClamp = 0.0f, .depthBiasSlopeFactor = 0.0f,
			.lineWidth = 1.0f
		}; 
		CreatePipeline_VsFs(Pipelines[0], PipelineLayouts[0], RenderPasses[0], VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP, 0, PRSCI, VK_FALSE, PSSCIs);

		for (auto& i : Threads) { i.join(); }
		Threads.clear();

		for (auto i : SMs) { vkDestroyShaderModule(Device, i, GetAllocationCallbacks()); }
	}
	virtual void CreateDescriptor() override {
		VK::CreateDescriptorPool(DescriptorPools.emplace_back(), 0, {
#pragma region SECOND_TEXTURE
			VkDescriptorPoolSize({.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, .descriptorCount = 2 }),
#pragma endregion
#pragma region UB
			VkDescriptorPoolSize({.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, .descriptorCount = static_cast<uint32_t>(std::size(Swapchain.ImageAndViews))}),
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
		for ([[maybe_unused]] const auto& i : Swapchain.ImageAndViews) {
			VERIFY_SUCCEEDED(vkAllocateDescriptorSets(Device, &DSAI, &DescriptorSets.emplace_back()));
		}
#pragma endregion

		struct DescriptorUpdateInfo
		{
			VkDescriptorImageInfo DII;
#pragma region SECOND_TEXTURE
			VkDescriptorImageInfo DII_1;
#pragma endregion
#pragma region UB
			VkDescriptorBufferInfo DBI;
#pragma endregion
		};
		VkDescriptorUpdateTemplate DUT;
		VK::CreateDescriptorUpdateTemplate(DUT, VK_PIPELINE_BIND_POINT_GRAPHICS, {
			VkDescriptorUpdateTemplateEntry({
				.dstBinding = 0, .dstArrayElement = 0,
				.descriptorCount = 1, .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
				.offset = offsetof(DescriptorUpdateInfo, DII), .stride = sizeof(DescriptorUpdateInfo)
			}),
#pragma region SECOND_TEXTURE
			VkDescriptorUpdateTemplateEntry({
				.dstBinding = 1, .dstArrayElement = 0,
				.descriptorCount = 1, .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
				.offset = offsetof(DescriptorUpdateInfo, DII_1), .stride = sizeof(DescriptorUpdateInfo)
			}),
#pragma endregion
#pragma region UB
			VkDescriptorUpdateTemplateEntry({
				.dstBinding = 2, .dstArrayElement = 0,
				.descriptorCount = 1, .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
				.offset = offsetof(DescriptorUpdateInfo, DBI), .stride = sizeof(DescriptorUpdateInfo)
			}),
#pragma endregion
		}, DescriptorSetLayouts[0]);
#pragma region UB
		for (size_t i = 0; i < std::size(Swapchain.ImageAndViews); ++i) {
			const DescriptorUpdateInfo DUI = {
				VkDescriptorImageInfo({.sampler = VK_NULL_HANDLE, .imageView = Textures[0].View, .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL }),
#pragma region SECOND_TEXTURE
				VkDescriptorImageInfo({.sampler = VK_NULL_HANDLE, .imageView = Textures[1].View, .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL }),
#pragma endregion
				VkDescriptorBufferInfo({.buffer = UniformBuffers[i].Buffer, .offset = 0, .range = VK_WHOLE_SIZE }),
			};
			vkUpdateDescriptorSetWithTemplate(Device, DescriptorSets[i], DUT, &DUI);
		}
		vkDestroyDescriptorUpdateTemplate(Device, DUT, GetAllocationCallbacks());
#pragma endregion
	}

	virtual void PopulateCommandBuffer(const size_t i) override;

#ifdef USE_LEAP
	virtual void OnHand(const LEAP_HAND& Hand) override {
		Leap::OnHand(Hand);
		
		const auto Index = eLeapHandType_Right == Hand.type ? 0 : 1;
		for (auto i = 0; i < std::size(Hand.digits); ++i) {
			const auto& Digit = Hand.digits[i];
			for (auto j = 0; j < std::size(Digit.bones); ++j) {
				const auto& Bone = Digit.bones[j];
				const auto x = std::clamp(Bone.next_joint.x, -100.0f, 100.0f) / 100.0f;
				const auto y = std::clamp(Bone.next_joint.y, 0.0f, 300.0f) / 300.0f;
				const auto z = std::clamp(Bone.next_joint.z, -100.0f, 100.0f) / 100.0f;
				Tracking.Hands[Index][i][j] = glm::vec4(x, y, z, 1.0f);
			}
		}
	}
	virtual void UpdateLeapImage() override {
		if (!empty(Textures)) {
			const auto CB = CommandBuffers[0];
			{
				const auto Layers = static_cast<uint32_t>(std::size(ImageData));
				const auto LayerSize = std::size(ImageData[0]);
				const auto TotalSize = Layers * LayerSize;
				const auto Extent = VkExtent3D({ .width = ImageProperties[0].width, .height = ImageProperties[0].height, .depth = 1 });

				VK::Scoped<BufferMemory> StagingBuffer(Device);
				StagingBuffer.Create(Device, GetCurrentPhysicalDeviceMemoryProperties(), TotalSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, data(ImageData[0]));
				std::vector<VkBufferImageCopy2> BICs;
				for (uint32_t i = 0; i < Layers; ++i) {
					BICs.emplace_back(VkBufferImageCopy2({
						.sType = VK_STRUCTURE_TYPE_BUFFER_IMAGE_COPY_2,
						.pNext = nullptr,
						.bufferOffset = i * LayerSize, .bufferRowLength = 0, .bufferImageHeight = 0,
						.imageSubresource = VkImageSubresourceLayers({.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .mipLevel = 0, .baseArrayLayer = i, .layerCount = 1 }),
						.imageOffset = VkOffset3D({.x = 0, .y = 0, .z = 0 }),.imageExtent = Extent
					}));
				}
				constexpr VkCommandBufferBeginInfo CBBI = { .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO, .pNext = nullptr, .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, .pInheritanceInfo = nullptr };
				VERIFY_SUCCEEDED(vkBeginCommandBuffer(CB, &CBBI)); {
					PopulateCopyBufferToImageCommand(CB, StagingBuffer.Buffer, Textures[0].Image, VK_ACCESS_SHADER_READ_BIT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, BICs, 1, Layers);
				} VERIFY_SUCCEEDED(vkEndCommandBuffer(CB));
				VK::SubmitAndWait(GraphicsQueue, CB);
			}
		}
	}
	virtual void UpdateDistortionImage() override {
		if (size(Textures) > 1) {
			const auto CB = CommandBuffers[0];
			{
				const auto Layers = static_cast<uint32_t>(std::size(ImageData));
				constexpr auto LayerSize = sizeof(DistortionMatrices[0]);
				const auto TotalSize = Layers * LayerSize;
				constexpr auto Extent = VkExtent3D({ .width = LEAP_DISTORTION_MATRIX_N, .height = LEAP_DISTORTION_MATRIX_N, .depth = 1 });

				VK::Scoped<BufferMemory> StagingBuffer(Device);
				StagingBuffer.Create(Device, GetCurrentPhysicalDeviceMemoryProperties(), TotalSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, data(DistortionMatrices));
				std::vector<VkBufferImageCopy2> BICs;
				for (uint32_t i = 0; i < Layers; ++i) {
					BICs.emplace_back(VkBufferImageCopy2({
						.sType = VK_STRUCTURE_TYPE_BUFFER_IMAGE_COPY_2,
						.pNext = nullptr,
						.bufferOffset = i * LayerSize, .bufferRowLength = 0, .bufferImageHeight = 0,
						.imageSubresource = VkImageSubresourceLayers({.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .mipLevel = 0, .baseArrayLayer = i, .layerCount = 1 }),
						.imageOffset = VkOffset3D({.x = 0, .y = 0, .z = 0 }),.imageExtent = Extent 
					}));
				}
				constexpr VkCommandBufferBeginInfo CBBI = { .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO, .pNext = nullptr, .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, .pInheritanceInfo = nullptr };
				VERIFY_SUCCEEDED(vkBeginCommandBuffer(CB, &CBBI)); {
					PopulateCopyBufferToImageCommand(CB, StagingBuffer.Buffer, Textures[1].Image, VK_ACCESS_SHADER_READ_BIT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, BICs, 1, Layers);
				} VERIFY_SUCCEEDED(vkEndCommandBuffer(CB));
				VK::SubmitAndWait(GraphicsQueue, CB);
			}
		}
	}
#endif

private:
	struct HandTracking
	{
		//!< 16バイトアライン境界をまたいではいけないので vec3 ではなく、vec4 を使用する
#ifdef USE_LEAP
		std::array<std::array<std::array<glm::vec4, std::size(LEAP_DIGIT::bones)>, std::size(LEAP_HAND::digits)>, 2> Hands;
#else
		std::array<std::array<std::array<glm::vec4, 4>, 5>, 2> Hands;
#endif
	};
	using HandTracking = struct HandTracking;
	HandTracking Tracking;
};
#pragma endregion