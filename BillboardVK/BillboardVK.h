#pragma once

#include "resource.h"

#pragma region Code
#include "../VKExt.h"

class BillboardVK : public VKExtDepth
{
private:
	using Super = VKExtDepth;
public:
	BillboardVK() : Super() {}
	virtual ~BillboardVK() {}

protected:
#ifdef USE_PUSH_DESCRIPTOR
	virtual void CreateDevice(HWND hWnd, HINSTANCE hInstance, void* pNext, [[maybe_unused]] const std::vector<const char*>& AdditionalExtensions) override { 
		Super::CreateDevice(hWnd, hInstance, pNext, { VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME }); 
	}
#endif
	virtual void DrawFrame(const uint32_t i) override {
		Tr.World = glm::rotate(glm::mat4(1.0f), glm::radians(Degree), glm::vec3(1.0f, 0.0f, 0.0f));
		Degree += 1.0f;

#pragma region FRAME_OBJECT
		//CopyToHostVisibleDeviceMemory(Device, UniformBuffers[i].DeviceMemory, 0, sizeof(Tr), &Tr, offsetof(Transform, World), sizeof(Transform::World));
		CopyToHostVisibleDeviceMemory(Device, UniformBuffers[i].DeviceMemory, 0, sizeof(Tr), &Tr);
#pragma endregion
	}
	virtual void CreateGeometry() override {
		const auto PDMP = GetCurrentPhysicalDeviceMemoryProperties();
		constexpr VkDrawIndexedIndirectCommand DIIC = { .indexCount = 1, .instanceCount = 1, .firstIndex = 0, .vertexOffset = 0, .firstInstance = 0 };
		IndirectBuffers.emplace_back().Create(Device, PDMP, DIIC).SubmitCopyCommand(Device, PDMP, CommandBuffers[0], GraphicsQueue, sizeof(DIIC), &DIIC);
	}
	virtual void CreateUniformBuffer() override {
		//const auto Fov = 0.16f * glm::pi<float>();
		constexpr auto Fov = 0.16f * std::numbers::pi_v<float>;
		const auto Aspect = GetAspectRatioOfClientRect();
		constexpr auto ZFar = 100.0f;
		constexpr auto ZNear = ZFar * 0.0001f;
		const auto CamPos = glm::vec3(0.0f, 0.0f, 6.0f);
		const auto CamTag = glm::vec3(0.0f);
		const auto CamUp = glm::vec3(0.0f, 1.0f, 0.0f);
		Tr = Transform({ glm::perspective(Fov, Aspect, ZNear, ZFar), glm::lookAt(CamPos, CamTag, CamUp), glm::mat4(1.0f) });
#pragma region FRAME_OBJECT
		for (size_t i = 0; i < size(SwapchainImages); ++i) {
			UniformBuffers.emplace_back().Create(Device, GetCurrentPhysicalDeviceMemoryProperties(), sizeof(Tr));
		}
#pragma endregion
	}
	virtual void CreatePipelineLayout() override {
		CreateDescriptorSetLayout(DescriptorSetLayouts.emplace_back(),
#ifdef USE_PUSH_DESCRIPTOR
			VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR,
#else
			0,
#endif
			{
				VkDescriptorSetLayoutBinding({.binding = 0, .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, .descriptorCount = 1, .stageFlags = VK_SHADER_STAGE_GEOMETRY_BIT, .pImmutableSamplers = nullptr })
			});
		VKExt::CreatePipelineLayout(PipelineLayouts.emplace_back(), DescriptorSetLayouts, {});
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
#ifdef USE_PUSH_DESCRIPTOR
		const std::array DUTEs = {
			VkDescriptorUpdateTemplateEntry({
				.dstBinding = 0, .dstArrayElement = 0,
				.descriptorCount = 1, .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
				.offset = 0, .stride = sizeof(VkDescriptorBufferInfo)
			})
		};
		const VkDescriptorUpdateTemplateCreateInfo DUTCI = {
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_UPDATE_TEMPLATE_CREATE_INFO,
			.pNext = nullptr,
			.flags = 0,
			.descriptorUpdateEntryCount = static_cast<uint32_t>(size(DUTEs)), .pDescriptorUpdateEntries = data(DUTEs),
			.templateType = VK_DESCRIPTOR_UPDATE_TEMPLATE_TYPE_PUSH_DESCRIPTORS_KHR, //!< VK_DESCRIPTOR_UPDATE_TEMPLATE_TYPE_PUSH_DESCRIPTORS_KHR を指定
			.descriptorSetLayout = DescriptorSetLayouts[0],
			.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
			.pipelineLayout = PipelineLayouts[0], .set = 0 //!< (VK_DESCRIPTOR_UPDATE_TEMPLATE_TYPE_PUSH_DESCRIPTORS_KHR使用時は)パイプラインレイアウトを指定
		};
		VERIFY_SUCCEEDED(vkCreateDescriptorUpdateTemplate(Device, &DUTCI, GetAllocationCallbacks(), &DescriptorUpdateTemplates.emplace_back()));
#else
		VK::CreateDescriptorPool(DescriptorPools.emplace_back(), /*VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT*/0, {
#pragma region FRAME_OBJECT
			VkDescriptorPoolSize({
				//!< UB * N
				.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, .descriptorCount = static_cast<uint32_t>(size(SwapchainImages))
				}), 
#pragma endregion
			});
		const std::array DSLs = { DescriptorSetLayouts[0] };
		const VkDescriptorSetAllocateInfo DSAI = {
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
			.pNext = nullptr,
			.descriptorPool = DescriptorPools[0],
			.descriptorSetCount = static_cast<uint32_t>(size(DSLs)), .pSetLayouts = data(DSLs)
		};
#pragma region FRAME_OBJECT
		for (size_t i = 0; i < size(SwapchainImages); ++i) {
			VERIFY_SUCCEEDED(vkAllocateDescriptorSets(Device, &DSAI, &DescriptorSets.emplace_back()));
		}
#pragma endregion

		VkDescriptorUpdateTemplate DUT;
		VK::CreateDescriptorUpdateTemplate(DUT, VK_PIPELINE_BIND_POINT_GRAPHICS, {
			VkDescriptorUpdateTemplateEntry({
				.dstBinding = 0, .dstArrayElement = 0,
				.descriptorCount = 1, .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
				.offset = 0, .stride = sizeof(VkDescriptorBufferInfo)
			}),
		}, DescriptorSetLayouts[0]);
#pragma region FRAME_OBJECT
		for (size_t i = 0; i < size(SwapchainImages); ++i) {
			const auto DBI = VkDescriptorBufferInfo({ .buffer = UniformBuffers[i].Buffer, .offset = 0, .range = VK_WHOLE_SIZE });
			vkUpdateDescriptorSetWithTemplate(Device, DescriptorSets[i], DUT, &DBI);
		}
		vkDestroyDescriptorUpdateTemplate(Device, DUT, GetAllocationCallbacks());
#pragma endregion
#endif //!< USE_PUSH_DESCRIPTOR
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
			const auto PLL = PipelineLayouts[0];

			vkCmdSetViewport(SCB, 0, static_cast<uint32_t>(size(Viewports)), data(Viewports));
			vkCmdSetScissor(SCB, 0, static_cast<uint32_t>(size(ScissorRects)), data(ScissorRects));

#ifdef USE_PUSH_DESCRIPTOR
#pragma region FRAME_OBJECT
			const auto DBI = VkDescriptorBufferInfo({.buffer = UniformBuffers[i].Buffer, .offset = 0, .range = VK_WHOLE_SIZE });
#pragma endregion
			vkCmdPushDescriptorSetWithTemplateKHR(SCB, DescriptorUpdateTemplates.back(), PLL, 0, &DBI);
#else
#pragma region FRAME_OBJECT
			const std::array DSs = { DescriptorSets[i] };
#pragma endregion
			constexpr std::array<uint32_t, 0> DynamicOffsets = {};
			vkCmdBindDescriptorSets(SCB, VK_PIPELINE_BIND_POINT_GRAPHICS, PLL, 0, static_cast<uint32_t>(size(DSs)), data(DSs), static_cast<uint32_t>(size(DynamicOffsets)), data(DynamicOffsets));
#endif
			vkCmdBindPipeline(SCB, VK_PIPELINE_BIND_POINT_GRAPHICS, Pipelines[0]);

			vkCmdDrawIndirect(SCB, IndirectBuffers[0].Buffer, 0, 1, 0);
		} VERIFY_SUCCEEDED(vkEndCommandBuffer(SCB));

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
				.renderArea = VkRect2D({.offset = VkOffset2D({.x = 0, .y = 0 }), .extent = SurfaceExtent2D }),
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

	//struct DescriptorUpdateInfo 
	//{
	//	VkDescriptorBufferInfo DBI[1];
	//};
};
#pragma endregion