#pragma once

#include "resource.h"

#pragma region Code
#include "../DRACO.h"
#include "../VKExt.h"

#ifdef USE_CONVEXHULL
#include "../Colli.h"
#endif

class DracoVK : public VKExtDepth, public Draco 
{
private:
	using Super = VKExtDepth;
public:
	DracoVK() : Super() {}
	virtual ~DracoVK() {}

	std::vector<uint32_t> Indices;
	std::vector<glm::vec3> Vertices;
	std::vector<glm::vec3> Normals;
#ifdef USE_CONVEXHULL
	std::vector<uint32_t> IndicesCH;
	std::vector<glm::vec3> VerticesCH;
#endif

#pragma region DRACO
	virtual void Process(const draco::Mesh* Mesh) override {
		Draco::Process(Mesh);
		if (nullptr != Mesh) {
#pragma region POSITION
			const auto POSITION = Mesh->GetNamedAttribute(draco::GeometryAttribute::POSITION);
			if (nullptr != POSITION) {
				for (auto i = 0; i < POSITION->size(); ++i) {
					POSITION->ConvertValue(static_cast<draco::AttributeValueIndex>(i), &Vertices.emplace_back()[0]);
				}
				AdjustScale(Vertices, 1.0f);
			}
#pragma endregion

#pragma region NORMAL
			const auto NORMAL = Mesh->GetNamedAttribute(draco::GeometryAttribute::NORMAL);
			if (nullptr != NORMAL) {
				for (auto i = 0; i < NORMAL->size(); ++i) {
					NORMAL->ConvertValue(static_cast<draco::AttributeValueIndex>(i), &Normals.emplace_back()[0]);
				}
			}
#pragma endregion

#pragma region INDEX
			for (uint32_t i = 0; i < Mesh->num_faces(); ++i) {
				const auto Face = Mesh->face(static_cast<draco::FaceIndex>(i));
				for (auto j = 0; j < 3; ++j) {
					Indices.emplace_back(POSITION->mapped_index(Face[j]).value());
				}
			}
#pragma endregion
		}
	}
#pragma endregion

	virtual void CreateGeometry() override {
#ifdef USE_CONVEXHULL
		Load(DRC_PATH / "bunny4.drc");
		//Load(DRC_PATH / "dragon4.drc");
#else
		Load(DRC_PATH / "bunny.drc");
		//Load(DRC_PATH / "dragon.drc");
		//Load(DRC_SAMPLE_PATH / "car.drc");
		//Load(DRC_SAMPLE_PATH / "bunny_gltf.drc");
#endif
		
#ifdef USE_CONVEXHULL
		std::vector<Vec3> VerticesVec3;
		VerticesVec3.reserve(size(Vertices));
		for (auto& i : Vertices) { VerticesVec3.emplace_back(Vec3({ i.x, i.y, i.z })); }

		std::vector<Vec3> HullVertices;
		std::vector<TriangleIndices> HullIndices;
		BuildConvexHull(VerticesVec3, HullVertices, HullIndices);
		{
			for (auto& i : HullVertices) {
				VerticesCH.emplace_back(glm::vec3(i.X(), i.Y(), i.Z()));
			}
			for (auto i : HullIndices) {
				IndicesCH.emplace_back(static_cast<uint32_t>(std::get<0>(i)));
				IndicesCH.emplace_back(static_cast<uint32_t>(std::get<1>(i)));
				IndicesCH.emplace_back(static_cast<uint32_t>(std::get<2>(i)));
			}
		}
#endif

		const auto& CB = CommandBuffers[0];
		const auto PDMP = GetCurrentPhysicalDeviceMemoryProperties();

		VertexBuffers.emplace_back().Create(Device, PDMP, TotalSizeOf(Vertices));
		VK::Scoped<StagingBuffer> Staging_Vertex(Device);
		Staging_Vertex.Create(Device, PDMP, TotalSizeOf(Vertices), data(Vertices));

		VertexBuffers.emplace_back().Create(Device, PDMP, TotalSizeOf(Normals));
		VK::Scoped<StagingBuffer> Staging_Normal(Device);
		Staging_Normal.Create(Device, PDMP, TotalSizeOf(Normals), data(Normals));

		IndexBuffers.emplace_back().Create(Device, PDMP, TotalSizeOf(Indices));
		VK::Scoped<StagingBuffer> Staging_Index(Device);
		Staging_Index.Create(Device, PDMP, TotalSizeOf(Indices), data(Indices));

		const VkDrawIndexedIndirectCommand DIIC = { .indexCount = static_cast<uint32_t>(size(Indices)), .instanceCount = 1, .firstIndex = 0, .vertexOffset = 0, .firstInstance = 0 };
		IndirectBuffers.emplace_back().Create(Device, PDMP, DIIC);
		VK::Scoped<StagingBuffer> Staging_Indirect(Device);
		Staging_Indirect.Create(Device, PDMP, sizeof(DIIC), &DIIC);

#ifdef USE_CONVEXHULL
		VertexBuffers.emplace_back().Create(Device, PDMP, TotalSizeOf(VerticesCH));
		VK::Scoped<StagingBuffer> Staging_VertexCH(Device);
		Staging_VertexCH.Create(Device, PDMP, TotalSizeOf(VerticesCH), data(VerticesCH));

		IndexBuffers.emplace_back().Create(Device, PDMP, TotalSizeOf(IndicesCH));
		VK::Scoped<StagingBuffer> Staging_IndexCH(Device);
		Staging_IndexCH.Create(Device, PDMP, TotalSizeOf(IndicesCH), data(IndicesCH));

		const VkDrawIndexedIndirectCommand DIIC_CH = { .indexCount = static_cast<uint32_t>(size(IndicesCH)), .instanceCount = 1, .firstIndex = 0, .vertexOffset = 0, .firstInstance = 0 };
		IndirectBuffers.emplace_back().Create(Device, PDMP, DIIC_CH);
		VK::Scoped<StagingBuffer> Staging_IndirectCH(Device);
		Staging_IndirectCH.Create(Device, PDMP, sizeof(DIIC_CH), &DIIC_CH);
#endif

		constexpr VkCommandBufferBeginInfo CBBI = { .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO, .pNext = nullptr, .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, .pInheritanceInfo = nullptr };
		VERIFY_SUCCEEDED(vkBeginCommandBuffer(CB, &CBBI)); {
			VertexBuffers[0].PopulateCopyCommand(CB, TotalSizeOf(Vertices), Staging_Vertex.Buffer);
			VertexBuffers[1].PopulateCopyCommand(CB, TotalSizeOf(Normals), Staging_Normal.Buffer);
			IndexBuffers[0].PopulateCopyCommand(CB, TotalSizeOf(Indices), Staging_Index.Buffer);
			IndirectBuffers[0].PopulateCopyCommand(CB, sizeof(DIIC), Staging_Indirect.Buffer);
#ifdef USE_CONVEXHULL
			VertexBuffers[2].PopulateCopyCommand(CB, TotalSizeOf(VerticesCH), Staging_VertexCH.Buffer);
			IndexBuffers[1].PopulateCopyCommand(CB, TotalSizeOf(IndicesCH), Staging_IndexCH.Buffer);
			IndirectBuffers[1].PopulateCopyCommand(CB, sizeof(DIIC_CH), Staging_IndirectCH.Buffer);
#endif
		} VERIFY_SUCCEEDED(vkEndCommandBuffer(CB));
		VK::SubmitAndWait(GraphicsQueue, CB);
	}
	virtual void CreatePipeline() override {
		Pipelines.emplace_back();
#ifdef USE_CONVEXHULL
		Pipelines.emplace_back();
#endif

		const std::array SMs = {
			VK::CreateShaderModule(GetFilePath(".vert.spv")),
			VK::CreateShaderModule(GetFilePath(".frag.spv")),
		};
		const std::array PSSCIs = {
			VkPipelineShaderStageCreateInfo({.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, .pNext = nullptr, .flags = 0, .stage = VK_SHADER_STAGE_VERTEX_BIT, .module = SMs[0], .pName = "main", .pSpecializationInfo = nullptr }),
			VkPipelineShaderStageCreateInfo({.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, .pNext = nullptr, .flags = 0, .stage = VK_SHADER_STAGE_FRAGMENT_BIT, .module = SMs[1], .pName = "main", .pSpecializationInfo = nullptr }),
		};
		const std::vector VIBDs = {
			VkVertexInputBindingDescription({.binding = 0, .stride = sizeof(Vertices[0]), .inputRate = VK_VERTEX_INPUT_RATE_VERTEX }),
			VkVertexInputBindingDescription({.binding = 1, .stride = sizeof(Normals[0]), .inputRate = VK_VERTEX_INPUT_RATE_VERTEX }),
		};
		const std::vector VIADs = {
			VkVertexInputAttributeDescription({.location = 0, .binding = 0, .format = VK_FORMAT_R32G32B32_SFLOAT, .offset = 0 }),
			VkVertexInputAttributeDescription({.location = 1, .binding = 1, .format = VK_FORMAT_R32G32B32_SFLOAT, .offset = 0 }),
		};
		constexpr VkPipelineRasterizationStateCreateInfo PRSCI = {
			.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
			.pNext = nullptr,
			.flags = 0,
			.depthClampEnable = VK_FALSE,
			.rasterizerDiscardEnable = VK_FALSE,
			.polygonMode = VK_POLYGON_MODE_LINE,
			.cullMode = VK_CULL_MODE_BACK_BIT,
			.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
			.depthBiasEnable = VK_FALSE, .depthBiasConstantFactor = 0.0f, .depthBiasClamp = 0.0f, .depthBiasSlopeFactor = 0.0f,
			.lineWidth = 1.0f
		};
		VKExt::CreatePipeline_VsFs_Input(Pipelines[0], PipelineLayouts[0], RenderPasses[0], VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, 0, PRSCI, VK_TRUE, VIBDs, VIADs, PSSCIs);

#ifdef USE_CONVEXHULL
		const std::array SMs_CH = {
			VK::CreateShaderModule(GetFilePath("_CH.vert.spv")),
			VK::CreateShaderModule(GetFilePath("_CH.frag.spv")),
		};
		const std::array PSSCIs_CH = {
			VkPipelineShaderStageCreateInfo({.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, .pNext = nullptr, .flags = 0, .stage = VK_SHADER_STAGE_VERTEX_BIT, .module = SMs_CH[0], .pName = "main", .pSpecializationInfo = nullptr }),
			VkPipelineShaderStageCreateInfo({.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, .pNext = nullptr, .flags = 0, .stage = VK_SHADER_STAGE_FRAGMENT_BIT, .module = SMs_CH[1], .pName = "main", .pSpecializationInfo = nullptr }),
		};
		const std::vector VIBDs_CH = {
			VkVertexInputBindingDescription({.binding = 0, .stride = sizeof(VerticesCH[0]), .inputRate = VK_VERTEX_INPUT_RATE_VERTEX }),
		};
		const std::vector VIADs_CH = {
			VkVertexInputAttributeDescription({.location = 0, .binding = 0, .format = VK_FORMAT_R32G32B32_SFLOAT, .offset = 0 }),
		};
		constexpr VkPipelineRasterizationStateCreateInfo PRSCI_CH = {
			.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
			.pNext = nullptr,
			.flags = 0,
			.depthClampEnable = VK_FALSE,
			.rasterizerDiscardEnable = VK_FALSE,
			.polygonMode = VK_POLYGON_MODE_LINE,
			.cullMode = VK_CULL_MODE_BACK_BIT,
			.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
			.depthBiasEnable = VK_FALSE, .depthBiasConstantFactor = 0.0f, .depthBiasClamp = 0.0f, .depthBiasSlopeFactor = 0.0f,
			.lineWidth = 1.0f
		};
		VKExt::CreatePipeline_VsFs_Input(Pipelines[1], PipelineLayouts[0], RenderPasses[0], VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, 0, PRSCI_CH, VK_TRUE, VIBDs_CH, VIADs_CH, PSSCIs_CH);
#endif

		for (auto& i : Threads) { i.join(); }
		Threads.clear();
		
		for (auto i : SMs) { vkDestroyShaderModule(Device, i, GetAllocationCallbacks()); }
#ifdef USE_CONVEXHULL
		for (auto i : SMs_CH) { vkDestroyShaderModule(Device, i, GetAllocationCallbacks()); }
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

			const std::array Offsets = { VkDeviceSize(0) };

			vkCmdBindPipeline(SCB, VK_PIPELINE_BIND_POINT_GRAPHICS, Pipelines[0]);

			const std::array VBs = { VertexBuffers[0].Buffer };
			const std::array NBs = { VertexBuffers[1].Buffer };
			vkCmdBindVertexBuffers(SCB, 0, static_cast<uint32_t>(size(VBs)), data(VBs), data(Offsets));
			vkCmdBindVertexBuffers(SCB, 1, static_cast<uint32_t>(size(NBs)), data(NBs), data(Offsets));
			vkCmdBindIndexBuffer(SCB, IndexBuffers[0].Buffer, 0, VK_INDEX_TYPE_UINT32);

			vkCmdDrawIndexedIndirect(SCB, IndirectBuffers[0].Buffer, 0, 1, 0);

#ifdef USE_CONVEXHULL
			vkCmdBindPipeline(SCB, VK_PIPELINE_BIND_POINT_GRAPHICS, Pipelines[1]);

			const std::array VBs_CH = { VertexBuffers[2].Buffer };
			vkCmdBindVertexBuffers(SCB, 0, static_cast<uint32_t>(size(VBs_CH)), data(VBs_CH), data(Offsets));
			vkCmdBindIndexBuffer(SCB, IndexBuffers[1].Buffer, 0, VK_INDEX_TYPE_UINT32);

			vkCmdDrawIndexedIndirect(SCB, IndirectBuffers[1].Buffer, 0, 1, 0);
#endif
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