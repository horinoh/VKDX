#pragma once

#include "resource.h"

#pragma region Code
#include "../VKExt.h"

class TriangleVK : public VKExt
{
private:
	using Super = VKExt;
public:
	TriangleVK() : Super() {}
	virtual ~TriangleVK() {}

protected:
	virtual void CreateGeometry() override {
		const auto& CB = CommandBuffers[0];
		const auto& PDMP = SelectedPhysDevice.second.PDMP;

#if true
		const std::array Vertices = {
	#ifdef USE_VIEWPORT_Y_UP
			Vertex_PositionColor({.Position = { 0.0f, 0.5f, 0.0f }, .Color = { 1.0f, 0.0f, 0.0f, 1.0f } }), //!< CT
			Vertex_PositionColor({.Position = { -0.5f, -0.5f, 0.0f }, .Color = { 0.0f, 1.0f, 0.0f, 1.0f } }), //!< LB
			Vertex_PositionColor({.Position = { 0.5f, -0.5f, 0.0f }, .Color = { 0.0f, 0.0f, 1.0f, 1.0f } }), //!< RB
	#else
			Vertex_PositionColor({.Position = { 0.5f, -0.5f, 0.0f }, .Color = { 0.0f, 0.0f, 1.0f, 1.0f } }), //!< RB
			Vertex_PositionColor({.Position = { -0.5f, -0.5f, 0.0f }, .Color = { 0.0f, 1.0f, 0.0f, 1.0f } }), //!< LB
			Vertex_PositionColor({.Position = { 0.0f, 0.5f, 0.0f }, .Color = { 1.0f, 0.0f, 0.0f, 1.0f } }), //!< CT
	#endif
		};
#else
		//!< ピクセル指定
		constexpr float W = 1280.0f, H = 720.0f;
		constexpr std::array Vertices = {
			Vertex_PositionColor({.Position = { W * 0.5f, 100.0f, 0.0f }, .Color = { 1.0f, 0.0f, 0.0f, 1.0f } }), //!< CT
			Vertex_PositionColor({.Position = { W * 0.5f - 200.0f, H - 100.0f, 0.0f }, .Color = { 0.0f, 1.0f, 0.0f, 1.0f } }), //!< LB
			Vertex_PositionColor({.Position = { W * 0.5f + 200.0f, H - 100.0f, 0.0f }, .Color = { 0.0f, 0.0f, 1.0f, 1.0f } }), //!< RB
		};
#endif
		constexpr std::array Indices = { uint32_t(0), uint32_t(1), uint32_t(2) };

		VertexBuffers.emplace_back().Create(Device, PDMP, TotalSizeOf(Vertices));
#ifdef USE_DEBUG_UTILS
		VK::Scoped<StagingBuffer> Staging_Vertex(Device);
#else
		StagingBuffer Staging_Vertex;
#endif
		Staging_Vertex.Create(Device, PDMP, TotalSizeOf(Vertices), std::data(Vertices));

#ifdef USE_DEBUG_UTILS
		SetObjectName(Device, VertexBuffers.back().Buffer, "MyVertexBuffer");
#endif

		IndexBuffers.emplace_back().Create(Device, PDMP, TotalSizeOf(Indices));
#ifdef USE_DEBUG_UTILS
		VK::Scoped<StagingBuffer> Staging_Index(Device);
#else
		StagingBuffer Staging_Index;
#endif
		Staging_Index.Create(Device, PDMP, TotalSizeOf(Indices), std::data(Indices));

#ifdef USE_DEBUG_UTILS
		SetObjectName(Device, IndexBuffers.back().Buffer, "MyIndexBuffer");
#endif
		constexpr VkDrawIndexedIndirectCommand DIIC = { .indexCount = static_cast<uint32_t>(std::size(Indices)), .instanceCount = 1, .firstIndex = 0, .vertexOffset = 0, .firstInstance = 0 };
		IndirectBuffers.emplace_back().Create(Device, PDMP, DIIC);
#ifdef USE_DEBUG_UTILS
		VK::Scoped<StagingBuffer> Staging_Indirect(Device);
#else
		StagingBuffer Staging_Indirect;
#endif
		Staging_Indirect.Create(Device, PDMP, sizeof(DIIC), &DIIC);

#ifdef USE_DEBUG_UTILS
		SetObjectName(Device, IndirectBuffers.back().Buffer, "MyIndirectBuffer");
#endif

		constexpr VkCommandBufferBeginInfo CBBI = { .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO, .pNext = nullptr, .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, .pInheritanceInfo = nullptr };
		VERIFY_SUCCEEDED(vkBeginCommandBuffer(CB, &CBBI)); {
			VertexBuffers.back().PopulateCopyCommand(CB, TotalSizeOf(Vertices), Staging_Vertex.Buffer);
			IndexBuffers.back().PopulateCopyCommand(CB, TotalSizeOf(Indices), Staging_Index.Buffer);
			IndirectBuffers.back().PopulateCopyCommand(CB, sizeof(DIIC), Staging_Indirect.Buffer);
		} VERIFY_SUCCEEDED(vkEndCommandBuffer(CB));
		VK::SubmitAndWait(GraphicsQueue, CB);

		LOG_OK();
	}
#ifdef USE_PUSH_CONSTANTS
	virtual void CreatePipelineLayout() override {
		//!< 【プッシュコンスタント】 : デスクリプタセットよりも高速
		//!< パイプラインレイアウト全体で128byte (ハードによりこれ以上使える場合もある、GTX970Mの場合は256byteだった)
		//!< 各シェーダステージは1つのプッシュコンスタントレンジにしかアクセスできない
		//!< 各シェーダステージが「共通のレンジを持たない」ような「ワーストケース」では 128/5==25.6、1シェーダステージで25byte程度となる
		VK::CreatePipelineLayout(PipelineLayouts.emplace_back(), {}, { 
			VkPushConstantRange({.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT, .offset = 0, .size = static_cast<uint32_t>(size(Color) * sizeof(Color[0])) })
		});
	}
#endif
	virtual void CreateRenderPass() override { VKExt::CreateRenderPass_Clear(); }
	virtual void CreatePipeline() override {
		Pipelines.emplace_back();

		const std::array SMs = {
			VK::CreateShaderModule(GetFilePath(".vert.spv")),
#ifdef USE_PUSH_CONSTANTS
			VK::CreateShaderModule(GetFilePath("_pc.frag.spv")),
#else
			VK::CreateShaderModule(GetFilePath(".frag.spv")),
#endif
		};
		const std::array PSSCIs = {
			VkPipelineShaderStageCreateInfo({.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, .pNext = nullptr, .flags = 0, .stage = VK_SHADER_STAGE_VERTEX_BIT, .module = SMs[0], .pName = "main", .pSpecializationInfo = nullptr }),
			VkPipelineShaderStageCreateInfo({.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, .pNext = nullptr, .flags = 0, .stage = VK_SHADER_STAGE_FRAGMENT_BIT, .module = SMs[1], .pName = "main", .pSpecializationInfo = nullptr }),
		};
		//!< バインディング0にまとめて入れるとインターリーブ、セマンティックス毎にバインディングを分けると非インターリーブとなる
		const std::vector VIBDs = {
			VkVertexInputBindingDescription({.binding = 0, .stride = sizeof(Vertex_PositionColor), .inputRate = VK_VERTEX_INPUT_RATE_VERTEX }),
		};
		//!< 詰まっていても、DX の D3D12_APPEND_ALIGNED_ELEMENT のように offsetof() を回避する手段は無い? (Is there no D3D12_APPEND_ALIGNED_ELEMENT equivalent?)
		const std::vector VIADs = {
			//!< location = 0 は binding = 0 の offsetof(Vertex_PositionColor, Position)
			VkVertexInputAttributeDescription({.location = 0, .binding = 0, .format = VK_FORMAT_R32G32B32_SFLOAT, .offset = offsetof(Vertex_PositionColor, Position) }),
			//!< location = 1 は binding = 0 の offsetof(Vertex_PositionColor, Color)
			VkVertexInputAttributeDescription({.location = 1, .binding = 0, .format = VK_FORMAT_R32G32B32A32_SFLOAT, .offset = offsetof(Vertex_PositionColor, Color) }),
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
		VKExt::CreatePipeline_VsFs_Input(Pipelines[0], PipelineLayouts[0], RenderPasses[0], VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP, 0, PRSCI, VK_FALSE, VIBDs, VIADs, PSSCIs);

		for (auto& i : Threads) { i.join(); }
		Threads.clear();

		for (auto i : SMs) { vkDestroyShaderModule(Device, i, GetAllocationCallbacks()); }
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

#ifdef USE_PUSH_CONSTANTS
			vkCmdPushConstants(SCB, PipelineLayouts[0], VK_SHADER_STAGE_FRAGMENT_BIT, 0, static_cast<uint32_t>(size(Color) * sizeof(Color[0])), data(Color));
#endif
			vkCmdBindPipeline(SCB, VK_PIPELINE_BIND_POINT_GRAPHICS, Pipelines[0]);

			const std::array VBs = { VertexBuffers[0].Buffer };
			const std::array Offsets = { VkDeviceSize(0) };
			vkCmdBindVertexBuffers(SCB, 0, static_cast<uint32_t>(size(VBs)), data(VBs), data(Offsets));
			vkCmdBindIndexBuffer(SCB, IndexBuffers[0].Buffer, 0, VK_INDEX_TYPE_UINT32);

			vkCmdDrawIndexedIndirect(SCB, IndirectBuffers[0].Buffer, 0, 1, 0);
		} VERIFY_SUCCEEDED(vkEndCommandBuffer(SCB));
	}
	virtual void PopulateCommandBuffer(const size_t i) override {
		const auto RP = RenderPasses[0];
		const auto FB = Framebuffers[i];
		const auto CB = CommandBuffers[i];
		const auto SCB = SecondaryCommandBuffers[i];

		constexpr VkCommandBufferBeginInfo CBBI = {
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
			.pNext = nullptr,
			.flags = 0,
			.pInheritanceInfo = nullptr
		};
		VERIFY_SUCCEEDED(vkBeginCommandBuffer(CB, &CBBI)); {
#ifdef USE_DEBUG_UTILS
			ScopedLabel(CB, glm::vec4(0.0f, 1.0f, 0.0f, 1.0f), "Command Begin");
#endif
			constexpr std::array CVs = { VkClearValue({.color = Colors::SkyBlue }) };
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

#ifdef USE_PUSH_CONSTANTS
	const std::array<float, 4> Color = { 1.0f, 0.0f, 0.0f, 1.0f };
#endif
};
#pragma endregion