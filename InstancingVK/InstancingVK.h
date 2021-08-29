#pragma once

#include "resource.h"

#pragma region Code
#include "../VKExt.h"

class InstancingVK : public VKExt
{
private:
	using Super = VKExt;
public:
	InstancingVK() : Super() {}
	virtual ~InstancingVK() {}

protected:
	virtual void CreateGeometry() override {
		const auto PDMP = GetCurrentPhysicalDeviceMemoryProperties();
		const auto& CB = CommandBuffers[0];

		constexpr std::array Vertices = {
			Vertex_PositionColor({.Position = { 0.0f, 0.5f, 0.0f }, .Color = { 1.0f, 0.0f, 0.0f, 1.0f } }),
			Vertex_PositionColor({.Position = { -0.5f, -0.5f, 0.0f }, .Color = { 0.0f, 1.0f, 0.0f, 1.0f } }),
			Vertex_PositionColor({.Position = { 0.5f, -0.5f, 0.0f }, .Color = { 0.0f, 0.0f, 1.0f, 1.0f } }),
		};
		constexpr std::array Instances = {
			Instance_OffsetXY({ { -0.5f, -0.5f } }),
			Instance_OffsetXY({ { -0.25f, -0.25f } }),
			Instance_OffsetXY({ { 0.0f, 0.0f } }),
			Instance_OffsetXY({ { 0.25f, 0.25f } }),
			Instance_OffsetXY({ { 0.5f, 0.5f } }),
		};
		constexpr std::array<uint32_t, 3> Indices = { 0, 1, 2 };
		constexpr VkDrawIndexedIndirectCommand DIIC = { .indexCount = static_cast<uint32_t>(size(Indices)), .instanceCount = static_cast<uint32_t>(size(Instances)), .firstIndex = 0, .vertexOffset = 0, .firstInstance = 0 };

		VertexBuffers.emplace_back().Create(Device, PDMP, sizeof(Vertices));
		VK::Scoped<StagingBuffer> Staging_Vertex0(Device);
		Staging_Vertex0.Create(Device, PDMP, sizeof(Vertices), data(Vertices));

		VertexBuffers.emplace_back().Create(Device, PDMP, sizeof(Instances));
		VK::Scoped<StagingBuffer> Staging_Vertex1(Device);
		Staging_Vertex1.Create(Device, PDMP, sizeof(Instances), data(Instances));

		IndexBuffers.emplace_back().Create(Device, PDMP, sizeof(Indices));
		VK::Scoped<StagingBuffer> Staging_Index(Device);
		Staging_Index.Create(Device, PDMP, sizeof(Indices), data(Indices));

		IndirectBuffers.emplace_back().Create(Device, PDMP, DIIC);
		VK::Scoped<StagingBuffer> Staging_Indirect(Device);
		Staging_Indirect.Create(Device, PDMP, sizeof(DIIC), &DIIC);

		constexpr VkCommandBufferBeginInfo CBBI = { .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO, .pNext = nullptr, .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, .pInheritanceInfo = nullptr };
		VERIFY_SUCCEEDED(vkBeginCommandBuffer(CB, &CBBI)); {
			VertexBuffers[0].PopulateCopyCommand(CB, sizeof(Vertices), Staging_Vertex0.Buffer);
			VertexBuffers[1].PopulateCopyCommand(CB, sizeof(Instances), Staging_Vertex1.Buffer);
			IndexBuffers.back().PopulateCopyCommand(CB, sizeof(Indices), Staging_Index.Buffer);
			IndirectBuffers.back().PopulateCopyCommand(CB, sizeof(DIIC), Staging_Indirect.Buffer);
		} VERIFY_SUCCEEDED(vkEndCommandBuffer(CB));
		VK::SubmitAndWait(GraphicsQueue, CB);

		LOG_OK();
	}
	virtual void CreatePipelineLayout() override {
		VKExt::CreatePipelineLayout(PipelineLayouts.emplace_back(), {}, {});
	}
	virtual void CreateRenderPass() override { VKExt::CreateRenderPass_Clear(); }
	virtual void CreatePipeline() override {
		const auto ShaderPath = GetBasePath();
		const std::array SMs = {
			VK::CreateShaderModule(data(ShaderPath + TEXT(".vert.spv"))),
			VK::CreateShaderModule(data(ShaderPath + TEXT(".frag.spv")))
		};
		const std::array PSSCIs = {
			VkPipelineShaderStageCreateInfo({.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, .pNext = nullptr, .flags = 0, .stage = VK_SHADER_STAGE_VERTEX_BIT, .module = SMs[0], .pName = "main", .pSpecializationInfo = nullptr }),
			VkPipelineShaderStageCreateInfo({.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, .pNext = nullptr, .flags = 0, .stage = VK_SHADER_STAGE_FRAGMENT_BIT, .module = SMs[1], .pName = "main", .pSpecializationInfo = nullptr }),
		};
		const std::vector VIBDs = { 
			//!< 頂点毎 (Per Vertex)
			VkVertexInputBindingDescription({.binding = 0, .stride = sizeof(Vertex_PositionColor), .inputRate = VK_VERTEX_INPUT_RATE_VERTEX }),
			//!< インスタンス毎 (Per Instance)
			VkVertexInputBindingDescription({.binding = 1, .stride = sizeof(Instance_OffsetXY), .inputRate = VK_VERTEX_INPUT_RATE_INSTANCE }),
		};
		const std::vector VIADs = { 
			//!< 頂点毎 (Per Vertex)
			VkVertexInputAttributeDescription({.location = 0, .binding = 0, .format = VK_FORMAT_R32G32B32_SFLOAT, .offset = offsetof(Vertex_PositionColor, Position) }),
			VkVertexInputAttributeDescription({.location = 1, .binding = 0, .format = VK_FORMAT_R32G32B32A32_SFLOAT, .offset = offsetof(Vertex_PositionColor, Color) }),
			//!< インスタンス毎 (Per Instance)
			VkVertexInputAttributeDescription({.location = 2, .binding = 1, .format = VK_FORMAT_R32G32_SFLOAT, .offset = offsetof(Instance_OffsetXY, Offset) }),
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
		CreatePipeline_VsFs_Input(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP, 0, PRSCI, VK_FALSE, VIBDs, VIADs, PSSCIs);

		for (auto i : SMs) { vkDestroyShaderModule(Device, i, GetAllocationCallbacks()); }
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

			vkCmdBindPipeline(SCB, VK_PIPELINE_BIND_POINT_GRAPHICS, Pipelines[0]);

			const std::array VBs = { VertexBuffers[0].Buffer, VertexBuffers[1].Buffer };
			constexpr std::array<VkDeviceSize, 2> Offsets = { 0, 0 };
			vkCmdBindVertexBuffers(SCB, 0, static_cast<uint32_t>(size(VBs)), data(VBs), data(Offsets));

			vkCmdBindIndexBuffer(SCB, IndexBuffers[0].Buffer, 0, VK_INDEX_TYPE_UINT32);

			vkCmdDrawIndexedIndirect(SCB, IndirectBuffers[0].Buffer, 0, 1, 0);
		} VERIFY_SUCCEEDED(vkEndCommandBuffer(SCB));

		const auto CB = CommandBuffers[i];
		constexpr VkCommandBufferBeginInfo CBBI = {
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
			.pNext = nullptr,
			.flags = 0,
			.pInheritanceInfo = nullptr
		};
		VERIFY_SUCCEEDED(vkBeginCommandBuffer(CB, &CBBI)); {
			constexpr std::array CVs = { VkClearValue({.color = Colors::SkyBlue }) };
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
};
#pragma endregion