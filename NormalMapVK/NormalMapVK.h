#pragma once

#include "resource.h"

#pragma region Code
#include "../VKImage.h"

class NormalMapVK : public VKImageDepth
{
private:
	using Super = VKImageDepth;
public:
	NormalMapVK() : Super() {}
	virtual ~NormalMapVK() {}

protected:
	virtual void OnUpdate(const uint32_t i) override {
		const auto CameraPos = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
		Tr.LocalCameraPosition = glm::inverse(Tr.View * Tr.World) * CameraPos;

		const auto LightPos = glm::rotate(glm::mat4(1.0f), glm::radians(Degree), glm::vec3(0.0f, 1.0f, 0.0f)) * glm::vec4(10.0f, 0.0f, 0.0f, 0.0f);
		Tr.LocalLightDirection = glm::normalize(glm::inverse(Tr.World) * LightPos);

		if (IsUpdate()) {
			Degree += 1.0f;
		}

#pragma region FRAME_OBJECT
		CopyToHostVisibleDeviceMemory(Device, UniformBuffers[i].DeviceMemory, 0, sizeof(Tr), &Tr);
#pragma endregion
	}
	virtual void CreateGeometry() override {
		const auto& PDMP = SelectedPhysDevice.second.PDMP;
		constexpr VkDrawIndexedIndirectCommand DIIC = {.indexCount = 1, .instanceCount = 1, .firstIndex = 0, .vertexOffset = 0, .firstInstance = 0 };
		IndirectBuffers.emplace_back().Create(Device, PDMP, DIIC).SubmitCopyCommand(Device, PDMP, CommandBuffers[0], GraphicsQueue, sizeof(DIIC), &DIIC);
	}
	virtual void CreateUniformBuffer() override {
		constexpr auto Fov = 0.16f * std::numbers::pi_v<float>;
		const auto Aspect = GetAspectRatioOfClientRect();
		constexpr auto ZFar = 100.0f;
		constexpr auto ZNear = ZFar * 0.0001f;
		const auto CamPos = glm::vec3(0.0f, 0.0f, 3.0f);
		const auto CamTag = glm::vec3(0.0f);
		const auto CamUp = glm::vec3(0.0f, 1.0f, 0.0f);
		const auto Projection = glm::perspective(Fov, Aspect, ZNear, ZFar);
		const auto View = glm::lookAt(CamPos, CamTag, CamUp);
		const auto World = glm::mat4(1.0f);

		Tr = Transform({ Projection, View, World, glm::vec4(0.0f, 0.0f, 0.0f, 1.0f), glm::vec4(10.0f, 0.0f, 0.0f, 0.0f) });
#pragma region FRAME_OBJECT
		for ([[maybe_unused]] const auto& i : Swapchain.ImageAndViews) {
			UniformBuffers.emplace_back().Create(Device, SelectedPhysDevice.second.PDMP, sizeof(Tr));
		}
#pragma endregion
	}
	virtual void CreateTexture() override {
		const auto& PDMP = SelectedPhysDevice.second.PDMP;

		const auto CB = CommandBuffers[0];
#ifdef USE_PARALLAX_MAP
		//!< [0] 法線(Normal)
		GLITextures.emplace_back().Create(Device, PDMP, DDS_PATH / "Leather009_2K-JPG" / "Leather009_2K_Normal.dds");
		//!< [1] ディスプレースメント(Displacement)
		GLITextures.emplace_back().Create(Device, PDMP, DDS_PATH / "Leather009_2K-JPG" / "Leather009_2K_Displacement.dds");
#else
		//!< [0] 法線(Normal)
		GLITextures.emplace_back().Create(Device, PDMP, DDS_PATH / "Rocks007_2K-JPG" / "Rocks007_2K_Normal.dds");
		//!< [1] カラー(Color)
		GLITextures.emplace_back().Create(Device, PDMP, DDS_PATH / "Rocks007_2K-JPG" / "Rocks007_2K_Color.dds");
#endif
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
			VkDescriptorSetLayoutBinding({.binding = 0, .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, .descriptorCount = 1, .stageFlags = VK_SHADER_STAGE_GEOMETRY_BIT, .pImmutableSamplers = nullptr }), //!< UniformBuffer
#ifdef USE_SEPARATE_SAMPLER
			VkDescriptorSetLayoutBinding({.binding = 1, .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER, .descriptorCount = static_cast<uint32_t>(size(ISs)), .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT, .pImmutableSamplers = data(ISs) }), //!< Sampler
			VkDescriptorSetLayoutBinding({.binding = 2, .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, .descriptorCount = 1, .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT, .pImmutableSamplers = nullptr }), //!< Image0
			VkDescriptorSetLayoutBinding({.binding = 3, .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, .descriptorCount = 1, .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT, .pImmutableSamplers = nullptr }), //!< Image1
#else
			VkDescriptorSetLayoutBinding({.binding = 1, .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, .descriptorCount = static_cast<uint32_t>(size(ISs)), .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT, .pImmutableSamplers = data(ISs) }), //!< Sampler + Image0
			VkDescriptorSetLayoutBinding({.binding = 2, .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, .descriptorCount = static_cast<uint32_t>(size(ISs)), .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT, .pImmutableSamplers = data(ISs) }), //!< Sampler + Image1
#endif
		});
		VK::CreatePipelineLayout(PipelineLayouts.emplace_back(), DescriptorSetLayouts, {});
	}
	virtual void CreatePipeline() override {
		Pipelines.emplace_back();

		const std::array SMs = {
			VK::CreateShaderModule(GetFilePath(".vert.spv")),
#ifdef USE_SEPARATE_SAMPLER
#ifdef USE_PARALLAX_MAP
			VK::CreateShaderModule(GetFilePath("_pm.frag.spv")),
#else
			VK::CreateShaderModule(GetFilePath(".frag.spv")),
#endif
#else
#ifdef USE_PARALLAX_MAP
			VK::CreateShaderModule(GetFilePath("_cis_pm.frag.spv")),
#else
			VK::CreateShaderModule(GetFilePath("_cis.frag.spv")),
#endif
#endif
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
		VKExt::CreateDescriptorPool(DescriptorPools.emplace_back(), 0, {
#pragma region FRAME_OBJECT
			VkDescriptorPoolSize({.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, .descriptorCount = static_cast<uint32_t>(std::size(Swapchain.ImageAndViews)) }), //!< UB * N
#pragma endregion
#ifdef USE_SEPARATE_SAMPLER
			VkDescriptorPoolSize({.type = VK_DESCRIPTOR_TYPE_SAMPLER, .descriptorCount = 1 }), //!< Sampler
			VkDescriptorPoolSize({.type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, .descriptorCount = 2 }), //!< Image[0, 1]
#else
			VkDescriptorPoolSize({.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, .descriptorCount = 2 }), //!< Sampler + Image[0, 1]
#endif
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

		struct DescriptorUpdateInfo
		{
			VkDescriptorBufferInfo DBI;
#ifdef USE_SEPARATE_SAMPLER
			VkDescriptorImageInfo DII_S; //!< Sampler
			VkDescriptorImageInfo DII_0; //!< Image0
			VkDescriptorImageInfo DII_1; //!< Image1
#else
			VkDescriptorImageInfo DII_0; //!< Sampelr + Image0
			VkDescriptorImageInfo DII_1; //!< Sampler + Image1
#endif
		};
		VkDescriptorUpdateTemplate DUT;
		VK::CreateDescriptorUpdateTemplate(DUT, VK_PIPELINE_BIND_POINT_GRAPHICS, {
			VkDescriptorUpdateTemplateEntry({
				.dstBinding = 0, .dstArrayElement = 0,
				.descriptorCount = 1, .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, //!< UniformBuffer
				.offset = offsetof(DescriptorUpdateInfo, DBI), .stride = sizeof(DescriptorUpdateInfo)
			}),
#ifdef USE_SEPARATE_SAMPLER
			VkDescriptorUpdateTemplateEntry({
				.dstBinding = 2, .dstArrayElement = 0,
				.descriptorCount = 1, .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, //!< Image0
				.offset = offsetof(DescriptorUpdateInfo, DII_0), .stride = sizeof(DescriptorUpdateInfo)
			}),
			VkDescriptorUpdateTemplateEntry({
				.dstBinding = 3, .dstArrayElement = 0,
				.descriptorCount = 1, .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, //!< Image1
				.offset = offsetof(DescriptorUpdateInfo, DII_1), .stride = sizeof(DescriptorUpdateInfo)
			}),
#else
			VkDescriptorUpdateTemplateEntry({
				.dstBinding = 1, .dstArrayElement = 0,
				.descriptorCount = 1, .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, //!< Sampler + Image0
				.offset = offsetof(DescriptorUpdateInfo, DII_0), .stride = sizeof(DescriptorUpdateInfo)
			}),
			VkDescriptorUpdateTemplateEntry({
				.dstBinding = 2, .dstArrayElement = 0,
				.descriptorCount = 1, .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, //!< Sampler + Image1
				.offset = offsetof(DescriptorUpdateInfo, DII_1), .stride = sizeof(DescriptorUpdateInfo)
			}),
#endif
		}, DescriptorSetLayouts[0]);

#pragma region FRAME_OBJECT
		for (size_t i = 0; i < std::size(Swapchain.ImageAndViews); ++i) {
			const DescriptorUpdateInfo DUI = {
				VkDescriptorBufferInfo({.buffer = UniformBuffers[i].Buffer, .offset = 0, .range = VK_WHOLE_SIZE }), //!< UniformBuffer
#ifdef USE_SEPARATE_SAMPLER
				VkDescriptorImageInfo({.sampler = Samplers[0], .imageView = VK_NULL_HANDLE, .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL }), //!< Sampler
				VkDescriptorImageInfo({.sampler = VK_NULL_HANDLE, .imageView = GLITextures[0].View, .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL }), //!< Image0
				VkDescriptorImageInfo({.sampler = VK_NULL_HANDLE, .imageView = GLITextures[1].View, .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL }), //!< Image1
#else
				VkDescriptorImageInfo({.sampler = VK_NULL_HANDLE, .imageView = GLITextures[0].View, .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL }), //!< Sampler + Image0
				VkDescriptorImageInfo({.sampler = VK_NULL_HANDLE, .imageView = GLITextures[1].View, .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL }), //!< Sampler + Image1
#endif
			};
			vkUpdateDescriptorSetWithTemplate(Device, DescriptorSets[i], DUT, &DUI);
		}
#pragma endregion
		vkDestroyDescriptorUpdateTemplate(Device, DUT, GetAllocationCallbacks());
	}
	virtual void PopulateCommandBuffer(const size_t i) override {
		const auto RP = RenderPasses[0];
		const auto FB = Framebuffers[i];

		const auto SCB = SecondaryCommandBuffers[i];
		const VkCommandBufferInheritanceInfo CBII = {
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO,
			.pNext = nullptr,
			.renderPass = RP,
			.subpass = 0,
			.framebuffer = FB,
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

		const auto CB = CommandBuffers[i];
		const VkCommandBufferBeginInfo CBBI = {
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
			.pNext = nullptr,
			.flags = 0,
			.pInheritanceInfo = nullptr
		};
		VERIFY_SUCCEEDED(vkBeginCommandBuffer(CB, &CBBI)); {
			const std::array CVs = { VkClearValue({.color = Colors::SkyBlue }), VkClearValue({.depthStencil = {.depth = 1.0f, .stencil = 0 } }) };
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
		glm::vec4 LocalCameraPosition;
		glm::vec4 LocalLightDirection;
	};
	using Transform = struct Transform;
	float Degree = 0.0f;
	Transform Tr;
	uint32_t HeapIndex;
	VkDeviceSize Offset;
};
#pragma endregion