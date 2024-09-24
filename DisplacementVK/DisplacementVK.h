#pragma once

#include "resource.h"

#pragma region Code
#include "../VKImage.h"

class DisplacementVK : public VKImageDepth
{
private:
	using Super = VKImageDepth;
public:
	DisplacementVK() : Super() {}
	virtual ~DisplacementVK() {}

protected:
	virtual void OnUpdate(const uint32_t i) override {
		Tr.World = glm::rotate(glm::rotate(glm::mat4(1.0f), glm::radians(Degree), glm::vec3(0.0f, 1.0f, 0.0f)), glm::radians(270.0f), glm::vec3(1.0f, 0.0f, 0.0f));

		if (IsUpdate()) {
			Degree += 1.0f;
		}

#pragma region FRAME_OBJECT
		CopyToHostVisibleDeviceMemory(Device, UniformBuffers[i].DeviceMemory, 0, sizeof(Tr), &Tr);
#pragma endregion
	}
	virtual void CreateGeometry() override {
		const auto& PDMP = SelectedPhysDevice.second.PDMP;
		constexpr VkDrawIndirectCommand DIC = { .vertexCount = 1, .instanceCount = 1, .firstVertex = 0, .firstInstance = 0 };
		IndirectBuffers.emplace_back().Create(Device, PDMP, DIC).SubmitCopyCommand(Device, PDMP, CommandBuffers[0], GraphicsQueue, sizeof(DIC), &DIC);
	}
	virtual void CreateUniformBuffer() override {
		constexpr auto Fov = 0.16f * std::numbers::pi_v<float>;
		const auto Aspect = GetAspectRatioOfClientRect();
		constexpr auto ZFar = 100.0f;
		constexpr auto ZNear = ZFar * 0.0001f;
		const auto CamPos = glm::vec3(0.0f, 1.0f, 3.0f);
		const auto CamTag = glm::vec3(0.0f);
		const auto CamUp = glm::vec3(0.0f, 1.0f, 0.0f);
		Tr = Transform({ .Projection = glm::perspective(Fov, Aspect, ZNear, ZFar), .View = glm::lookAt(CamPos, CamTag, CamUp), .World = glm::mat4(1.0f) });
#pragma region FRAME_OBJECT
		for ([[maybe_unused]] const auto& i : Swapchain.ImageAndViews) {
			UniformBuffers.emplace_back().Create(Device, SelectedPhysDevice.second.PDMP, sizeof(Tr));
		}
#pragma endregion
	}
	virtual void CreateTexture() override {
		const auto& PDMP = SelectedPhysDevice.second.PDMP;
		const auto CB = CommandBuffers[0];

		//!< [0] ディスプレースメント(Displacement)
		GLITextures.emplace_back().Create(Device, PDMP, DDS_PATH / "Rocks007_2K-JPG" / "Rocks007_2K_Displacement.dds");
		//!< [1] カラー(Color)
		GLITextures.emplace_back().Create(Device, PDMP, DDS_PATH / "Rocks007_2K-JPG" / "Rocks007_2K_Color.dds");

		{
			VK::Scoped<StagingBuffer> Staging0(Device);
			GLITextures[0].CopyToStagingBuffer(Device, PDMP, Staging0);

			VK::Scoped<StagingBuffer> Staging1(Device);
			GLITextures[1].CopyToStagingBuffer(Device, PDMP, Staging1);

			constexpr VkCommandBufferBeginInfo CBBI = {
				.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
				.pNext = nullptr,
				.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
				.pInheritanceInfo = nullptr
			};
			VERIFY_SUCCEEDED(vkBeginCommandBuffer(CB, &CBBI)); {
				GLITextures[0].PopulateCopyCommand(CB, VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT, Staging0.Buffer);
				GLITextures[1].PopulateCopyCommand(CB, VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT, Staging1.Buffer);
			} VERIFY_SUCCEEDED(vkEndCommandBuffer(CB));
			VK::SubmitAndWait(GraphicsQueue, CB);
		}

		//!< [2] 深度(Depth)
		Super::CreateTexture();
	}
	virtual void CreateImmutableSampler() override {
		CreateImmutableSampler_LR();
	}
	virtual void CreatePipelineLayout() override {
		const std::array ISs = { Samplers[0] };
		CreateDescriptorSetLayout(DescriptorSetLayouts.emplace_back(), 0, {
			//!< UniformBuffer
			VkDescriptorSetLayoutBinding({.binding = 0, .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, .descriptorCount = 1, .stageFlags = VK_SHADER_STAGE_GEOMETRY_BIT, .pImmutableSamplers = nullptr }),
			//!< Sampler + Image0
			VkDescriptorSetLayoutBinding({.binding = 1, .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, .descriptorCount = static_cast<uint32_t>(size(ISs)), .stageFlags = VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT, .pImmutableSamplers = data(ISs) }), 
			//!< Sampler + Image1
			VkDescriptorSetLayoutBinding({.binding = 2, .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, .descriptorCount = static_cast<uint32_t>(size(ISs)), .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT, .pImmutableSamplers = data(ISs) }),
		});
		VK::CreatePipelineLayout(PipelineLayouts.emplace_back(), DescriptorSetLayouts, {});
	}
	virtual void CreatePipeline() override { 
		Pipelines.emplace_back();

		const std::array SMs = {
			VK::CreateShaderModule(GetFilePath(".vert.spv")),
			VK::CreateShaderModule(GetFilePath(".frag.spv")),
			VK::CreateShaderModule(GetFilePath(".tese.spv")),
			VK::CreateShaderModule(GetFilePath(".tesc.spv")),
			VK::CreateShaderModule(GetFilePath(".geom.spv")),
		};
		const std::array PSSCIs = {
			VkPipelineShaderStageCreateInfo({.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, .pNext = nullptr, .flags = 0, .stage = VK_SHADER_STAGE_VERTEX_BIT, .module = SMs[0], .pName = "main", .pSpecializationInfo = nullptr }),
			VkPipelineShaderStageCreateInfo({.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, .pNext = nullptr, .flags = 0, .stage = VK_SHADER_STAGE_FRAGMENT_BIT, .module = SMs[1], .pName = "main", .pSpecializationInfo = nullptr }),
			VkPipelineShaderStageCreateInfo({.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, .pNext = nullptr, .flags = 0, .stage = VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT, .module = SMs[2], .pName = "main", .pSpecializationInfo = nullptr }),
			VkPipelineShaderStageCreateInfo({.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, .pNext = nullptr, .flags = 0, .stage = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT, .module = SMs[3], .pName = "main", .pSpecializationInfo = nullptr }),
			VkPipelineShaderStageCreateInfo({.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, .pNext = nullptr, .flags = 0, .stage = VK_SHADER_STAGE_GEOMETRY_BIT, .module = SMs[4], .pName = "main", .pSpecializationInfo = nullptr }),
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
		CreatePipeline_VsFsTesTcsGs(Pipelines[0], PipelineLayouts[0], RenderPasses[0], VK_PRIMITIVE_TOPOLOGY_PATCH_LIST, 1, PRSCI, VK_TRUE, PSSCIs);

		for (auto& i : Threads) { i.join(); }
		Threads.clear();

		for (auto i : SMs) { vkDestroyShaderModule(Device, i, GetAllocationCallbacks()); }
	}
	virtual void CreateDescriptor() override {
		VK::CreateDescriptorPool(DescriptorPools.emplace_back(), 0, {
#pragma region FRAME_OBJECT
			//!< UB * N
			VkDescriptorPoolSize({.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, .descriptorCount = static_cast<uint32_t>(std::size(Swapchain.ImageAndViews)) }), 
#pragma endregion
			//!< ImageSampler0 + ImageSampler1
			VkDescriptorPoolSize({.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, .descriptorCount = 2 }), 
		});
		const std::array DSLs = { DescriptorSetLayouts[0] };
		const VkDescriptorSetAllocateInfo DSAI = {
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
			.pNext = nullptr,
			.descriptorPool = DescriptorPools[0],
			.descriptorSetCount = static_cast<uint32_t>(size(DSLs)), .pSetLayouts = data(DSLs)
		};
#pragma region FRAME_OBJECT
		for ([[maybe_unused]] const auto& i : Swapchain.ImageAndViews) {
			VERIFY_SUCCEEDED(vkAllocateDescriptorSets(Device, &DSAI, &DescriptorSets.emplace_back()));
		}
#pragma endregion

#ifdef USE_UPDATE_DESCRIPTOR_SET_WITH_TEMPLATE
		struct DescriptorUpdateInfo
		{
			VkDescriptorBufferInfo DBI;
			VkDescriptorImageInfo DII_0;
			VkDescriptorImageInfo DII_1;
		};
		VkDescriptorUpdateTemplate DUT;
		VK::CreateDescriptorUpdateTemplate(DUT, VK_PIPELINE_BIND_POINT_GRAPHICS, {
			VkDescriptorUpdateTemplateEntry({
				.dstBinding = 0, .dstArrayElement = 0,
				.descriptorCount = 1, .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, //!< UB
				.offset = offsetof(DescriptorUpdateInfo, DBI), .stride = sizeof(DescriptorUpdateInfo)
			}),
			VkDescriptorUpdateTemplateEntry({
				.dstBinding = 1, .dstArrayElement = 0,
				.descriptorCount = 1, .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, //!< SamplerImage0
				.offset = offsetof(DescriptorUpdateInfo, DII_0), .stride = sizeof(DescriptorUpdateInfo)
			}),
			VkDescriptorUpdateTemplateEntry({
				.dstBinding = 2, .dstArrayElement = 0,
				.descriptorCount = 1, .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, //!< SamplerImage1
				.offset = offsetof(DescriptorUpdateInfo, DII_1), .stride = sizeof(DescriptorUpdateInfo)
			}),
		}, DescriptorSetLayouts[0]);
#pragma region FRAME_OBJECT
		for (size_t i = 0; i < size(SwapchainImages); ++i) {
			const DescriptorUpdateInfo DUI = {
				VkDescriptorBufferInfo({.buffer = UniformBuffers[i].Buffer, .offset = 0, .range = VK_WHOLE_SIZE }), //!< UniformBuffer
				VkDescriptorImageInfo({.sampler = VK_NULL_HANDLE, .imageView = GLITextures[0].View, .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL }), //!< SamplerImage0
				VkDescriptorImageInfo({.sampler = VK_NULL_HANDLE, .imageView = GLITextures[1].View, .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL }), //!< SamplerImage1
			};
			vkUpdateDescriptorSetWithTemplate(Device, DescriptorSets[i], DUT, &DUI);
		}
		vkDestroyDescriptorUpdateTemplate(Device, DUT, GetAllocationCallbacks());
#pragma endregion
#else
#pragma region FRAME_OBJECT
		const auto DII0 = VkDescriptorImageInfo({ .sampler = VK_NULL_HANDLE, .imageView = GLITextures[0].View, .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL }); //!< SamplerImage0
		const auto DII1 = VkDescriptorImageInfo({ .sampler = VK_NULL_HANDLE, .imageView = GLITextures[1].View, .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL }); //!< SamplerImage1
		constexpr std::array<VkCopyDescriptorSet, 0> CDSs = {};
		for (size_t i = 0; i < std::size(Swapchain.ImageAndViews); ++i) {
			const auto DBI = VkDescriptorBufferInfo({ .buffer = UniformBuffers[i].Buffer, .offset = 0, .range = VK_WHOLE_SIZE }); //!< UniformBuffer
			const std::array WDSs = {
				VkWriteDescriptorSet({
					.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
					.pNext = nullptr,
					.dstSet = DescriptorSets[i],
					.dstBinding = 0, .dstArrayElement = 0, .descriptorCount = 1, .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, //!< UB
					.pImageInfo = nullptr,
					.pBufferInfo = &DBI,
					.pTexelBufferView = nullptr
				}),
				VkWriteDescriptorSet({
					.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
					.pNext = nullptr,
					.dstSet = DescriptorSets[i],
					.dstBinding = 1, .dstArrayElement = 0, .descriptorCount = 1, .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, //!< SamplerImage0
					.pImageInfo = &DII0,
					.pBufferInfo = nullptr,
					.pTexelBufferView = nullptr
				}),
				VkWriteDescriptorSet({
					.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
					.pNext = nullptr,
					.dstSet = DescriptorSets[i],
					.dstBinding = 2, .dstArrayElement = 0, .descriptorCount = 1, .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, //!< SamplerImage1
					.pImageInfo = &DII1,
					.pBufferInfo = nullptr,
					.pTexelBufferView = nullptr
				}),
			};
			vkUpdateDescriptorSets(Device, static_cast<uint32_t>(size(WDSs)), data(WDSs), static_cast<uint32_t>(size(CDSs)), data(CDSs));
		}
#pragma endregion
#endif
	}
	virtual void PopulateSecondaryCommandBuffer(const size_t i) override {
		const auto RP = RenderPasses[0];
		const auto SCB = SecondaryCommandBuffers[i];

		const VkCommandBufferInheritanceInfo CBII = {
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO,
			.pNext = nullptr,
			.renderPass = RP,
			.subpass = 0,
			.framebuffer = VK_NULL_HANDLE,
			.occlusionQueryEnable = VK_FALSE, .queryFlags = 0,
			.pipelineStatistics = 0,
		};
		const VkCommandBufferBeginInfo SCBBI = {
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
			.pNext = nullptr,
			.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT,
			.pInheritanceInfo = &CBII
		};
		VERIFY_SUCCEEDED(vkBeginCommandBuffer(SCB, &SCBBI)); {
			vkCmdSetViewport(SCB, 0, static_cast<uint32_t>(size(Viewports)), data(Viewports));
			vkCmdSetScissor(SCB, 0, static_cast<uint32_t>(size(ScissorRects)), data(ScissorRects));

#pragma region FRAME_OBJECT
			const std::array DSs = { DescriptorSets[i] };
#pragma endregion
			constexpr std::array<uint32_t, 0> DynamicOffsets = {};
			vkCmdBindDescriptorSets(SCB, VK_PIPELINE_BIND_POINT_GRAPHICS, PipelineLayouts[0], 0, static_cast<uint32_t>(size(DSs)), data(DSs), static_cast<uint32_t>(size(DynamicOffsets)), data(DynamicOffsets));

			vkCmdBindPipeline(SCB, VK_PIPELINE_BIND_POINT_GRAPHICS, Pipelines[0]);

			vkCmdDrawIndirect(SCB, IndirectBuffers[0].Buffer, 0, 1, 0);
		} VERIFY_SUCCEEDED(vkEndCommandBuffer(SCB));
	}
	virtual void PopulateCommandBuffer(const size_t i) override {
		const auto RP = RenderPasses[0];
		const auto FB = Framebuffers[i];
		const auto SCB = SecondaryCommandBuffers[i];
		const auto CB = CommandBuffers[i];

		constexpr VkCommandBufferBeginInfo CBBI = {
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
			.pNext = nullptr,
			.flags = 0,
			.pInheritanceInfo = nullptr
		};
		VERIFY_SUCCEEDED(vkBeginCommandBuffer(CB, &CBBI)); {
			constexpr std::array CVs = { VkClearValue({.color = Colors::SkyBlue }), VkClearValue({.depthStencil = {.depth = 1.0f, .stencil = 0 } }) };
			const VkRenderPassBeginInfo RPBI = {
				.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
				.pNext = nullptr,
				.renderPass = RP,
				.framebuffer = FB,
				.renderArea = VkRect2D({.offset = VkOffset2D({.x = 0, .y = 0 }), .extent = Swapchain.Extent }),
				.clearValueCount = static_cast<uint32_t>(size(CVs)), .pClearValues = data(CVs)
			};
			vkCmdBeginRenderPass(CB, &RPBI, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS); {
				const std::array SCBs = { SCB };
				vkCmdExecuteCommands(CB, static_cast<uint32_t>(size(SCBs)), data(SCBs));
			} vkCmdEndRenderPass(CB);
		} VERIFY_SUCCEEDED(vkEndCommandBuffer(CB));
	}

private:
	struct Transform
	{
		glm::mat4 Projection;
		glm::mat4 View;
		glm::mat4 World;
	};
	using Transform = struct Transform;
	float Degree = 0.0f;
	Transform Tr;

#ifdef USE_UPDATE_DESCRIPTOR_SET_WITH_TEMPLATE
	struct DescriptorUpdateInfo
	{
		VkDescriptorBufferInfo DBI[1];
		VkDescriptorImageInfo DII_0[1];
		VkDescriptorImageInfo DII_1[1];
	};
#endif
};
#pragma endregion