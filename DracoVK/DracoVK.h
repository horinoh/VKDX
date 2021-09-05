#pragma once

#include "resource.h"

#pragma region Code
#include "../DRACO.h"
#include "../VKExt.h"

class DracoVK : public VKExt, public Draco 
{
private:
	using Super = VKExt;
public:
	DracoVK() : Super() {}
	virtual ~DracoVK() {}

	std::vector<uint32_t> Indices;
	std::vector<glm::vec3> Vertices;
	std::vector<glm::vec3> Normals;
#pragma region DRACO
	virtual void Process(const draco::Mesh* Mesh) override {
		Draco::Process(Mesh);
		if (nullptr != Mesh) {
#pragma region POSITION
			const auto POSITION = Mesh->GetNamedAttribute(draco::GeometryAttribute::POSITION);
			if (nullptr != POSITION) {
				auto Max = glm::vec3((std::numeric_limits<float>::min)());
				auto Min = glm::vec3((std::numeric_limits<float>::max)());
				for (auto i = 0; i < POSITION->size(); ++i) {
					POSITION->ConvertValue(static_cast<draco::AttributeValueIndex>(i), &Vertices.emplace_back()[0]);

					Max.x = std::max(Max.x, Vertices.back().x);
					Max.y = std::max(Max.y, Vertices.back().y);
					Max.z = std::max(Max.z, Vertices.back().z);
					Min.x = std::min(Min.x, Vertices.back().x);
					Min.y = std::min(Min.y, Vertices.back().y);
					Min.z = std::min(Min.z, Vertices.back().z);
				}
				const auto Bound = std::max(std::max(Max.x - Min.x, Max.y - Min.y), Max.z - Min.z) * 1.0f;
				std::transform(begin(Vertices), end(Vertices), begin(Vertices), [&](const glm::vec3& rhs) { return rhs / Bound - glm::vec3(0.0f, (Max.y - Min.y) * 0.5f, Min.z) / Bound; });
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

#ifdef USE_RENDERDOC
	virtual void CreateInstance([[maybe_unused]] const std::vector<const char*>& AdditionalLayers, const std::vector<const char*>& AdditionalExtensions) override {
		//!< #TIPS　レイトレーシングやメッシュシェーダーと同時に "VK_LAYER_RENDERDOC_Capture" を使用した場合に vkCreateDevice() でコケていたので、用途によっては注意が必要
		Super::CreateInstance({ "VK_LAYER_RENDERDOC_Capture" }, AdditionalExtensions);
	}
	virtual void CreateDevice(HWND hWnd, HINSTANCE hInstance, void* pNext, [[maybe_unused]] const std::vector<const char*>& AdditionalExtensions) override {
		Super::CreateDevice(hWnd, hInstance, pNext, { VK_EXT_DEBUG_MARKER_EXTENSION_NAME });
	}
#endif
	virtual void CreateGeometry() override {
		std::wstring Path;
		if (FindDirectory("DRC", Path)) {
			Load(ToString(Path) + "//bunny.drc");
			//Load(ToString(Path) + "//dragon.drc");
			//Load(ToString(Path) + "//dragon4.drc");
		}
		//Load(std::string("..//draco//testdata//") + "car.drc");

		const auto& CB = CommandBuffers[0];
		const auto PDMP = GetCurrentPhysicalDeviceMemoryProperties();

		VertexBuffers.emplace_back().Create(Device, PDMP, Sizeof(Vertices));
		VK::Scoped<StagingBuffer> Staging_Vertex(Device);
		Staging_Vertex.Create(Device, PDMP, Sizeof(Vertices), data(Vertices));

		VertexBuffers.emplace_back().Create(Device, PDMP, Sizeof(Normals));
		VK::Scoped<StagingBuffer> Staging_Normal(Device);
		Staging_Normal.Create(Device, PDMP, Sizeof(Normals), data(Normals));

		IndexBuffers.emplace_back().Create(Device, PDMP, Sizeof(Indices));
		VK::Scoped<StagingBuffer> Staging_Index(Device);
		Staging_Index.Create(Device, PDMP, Sizeof(Indices), data(Indices));

		const VkDrawIndexedIndirectCommand DIIC = { .indexCount = static_cast<uint32_t>(size(Indices)), .instanceCount = 1, .firstIndex = 0, .vertexOffset = 0, .firstInstance = 0 };
		IndirectBuffers.emplace_back().Create(Device, PDMP, DIIC);
		VK::Scoped<StagingBuffer> Staging_Indirect(Device);
		Staging_Indirect.Create(Device, PDMP, sizeof(DIIC), &DIIC);

		constexpr VkCommandBufferBeginInfo CBBI = { .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO, .pNext = nullptr, .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, .pInheritanceInfo = nullptr };
		VERIFY_SUCCEEDED(vkBeginCommandBuffer(CB, &CBBI)); {
			VertexBuffers[0].PopulateCopyCommand(CB, Sizeof(Vertices), Staging_Vertex.Buffer);
			VertexBuffers[1].PopulateCopyCommand(CB, Sizeof(Normals), Staging_Normal.Buffer);
			IndexBuffers.back().PopulateCopyCommand(CB, Sizeof(Indices), Staging_Index.Buffer);
			IndirectBuffers.back().PopulateCopyCommand(CB, sizeof(DIIC), Staging_Indirect.Buffer);
		} VERIFY_SUCCEEDED(vkEndCommandBuffer(CB));
		VK::SubmitAndWait(GraphicsQueue, CB);
	}
	virtual void CreateTexture() override {
		DepthTextures.emplace_back().Create(Device, GetCurrentPhysicalDeviceMemoryProperties(), DepthFormat, VkExtent3D({ .width = SurfaceExtent2D.width, .height = SurfaceExtent2D.height, .depth = 1 }));
	}
	virtual void CreateRenderPass() override { VKExt::CreateRenderPass_Depth(); }
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
		VKExt::CreatePipeline_VsFs_Input(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, 0, PRSCI, VK_TRUE, VIBDs, VIADs, PSSCIs);

		for (auto i : SMs) { vkDestroyShaderModule(Device, i, GetAllocationCallbacks()); }
	}
	virtual void CreateFramebuffer() override {
		for (auto i : SwapchainImageViews) {
			VK::CreateFramebuffer(Framebuffers.emplace_back(), RenderPasses[0], SurfaceExtent2D.width, SurfaceExtent2D.height, 1, { i, DepthTextures.back().View });
		}
	}
	virtual void PopulateCommandBuffer(const size_t i) override {
		const auto RP = RenderPasses[0];
		const auto FB = Framebuffers[i];

#pragma region SECONDARY_COMMAND_BUFFER
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

			const std::array VBs = { VertexBuffers[0].Buffer };
			const std::array NBs = { VertexBuffers[1].Buffer };
			const std::array Offsets = { VkDeviceSize(0) };
			vkCmdBindVertexBuffers(SCB, 0, static_cast<uint32_t>(size(VBs)), data(VBs), data(Offsets));
			vkCmdBindVertexBuffers(SCB, 1, static_cast<uint32_t>(size(NBs)), data(NBs), data(Offsets));
			vkCmdBindIndexBuffer(SCB, IndexBuffers[0].Buffer, 0, VK_INDEX_TYPE_UINT32);

			vkCmdDrawIndexedIndirect(SCB, IndirectBuffers[0].Buffer, 0, 1, 0);
		} VERIFY_SUCCEEDED(vkEndCommandBuffer(SCB));
#pragma endregion

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