#pragma once

#include "resource.h"

#pragma region Code
#include "../FBX.h"
#include "../VKExt.h"

class FbxVK : public VKExt, public Fbx
{
private:
	using Super = VKExt;
public:
	FbxVK() : Super() {}
	virtual ~FbxVK() {}

#pragma region FBX
	glm::vec3 ToVec3(const FbxVector4& rhs) { return glm::vec3(static_cast<FLOAT>(rhs[0]), static_cast<FLOAT>(rhs[1]), static_cast<FLOAT>(rhs[2])); }
	std::vector<uint32_t> Indices;
	std::vector<glm::vec3> Vertices;
	std::vector<glm::vec3> Normals;
	virtual void Process(FbxMesh* Mesh) override {
		Fbx::Process(Mesh);

		auto Max = glm::vec3((std::numeric_limits<float>::min)());
		auto Min = glm::vec3((std::numeric_limits<float>::max)());
		std::cout << "PolygonCount = " << Mesh->GetPolygonCount() << std::endl;
		for (auto i = 0; i < Mesh->GetPolygonCount(); ++i) {
			for (auto j = 0; j < Mesh->GetPolygonSize(i); ++j) {
				Indices.emplace_back(i * Mesh->GetPolygonSize(i) + j);

				Vertices.emplace_back(ToVec3(Mesh->GetControlPoints()[Mesh->GetPolygonVertex(i, j)]));
				Max.x = std::max(Max.x, Vertices.back().x);
				Max.y = std::max(Max.y, Vertices.back().y);
				Max.z = std::max(Max.z, Vertices.back().z);
				Min.x = std::min(Min.x, Vertices.back().x);
				Min.y = std::min(Min.y, Vertices.back().y);
				Min.z = std::min(Min.z, Vertices.back().z);

				for (auto k = 0; k < Mesh->GetElementNormalCount(); ++k) {
					Normals.emplace_back(ToVec3(Mesh->GetElementNormal(k)->GetDirectArray().GetAt(i)));
				}
			}
		}
		const auto Bound = std::max(std::max(Max.x - Min.x, Max.y - Min.y), Max.z - Min.z) * 1.0f;
		std::transform(begin(Vertices), end(Vertices), begin(Vertices), [&](const glm::vec3& rhs) { return rhs / Bound - glm::vec3(0.0f, (Max.y - Min.y) * 0.5f, Min.z) / Bound; });

		//for (auto i : Vertices) { std::cout << i.x << ", " << i.y << ", " << i.z << std::endl; }
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
		if (FindDirectory("FBX", Path)) {
			Load(ToString(Path) + "//bunny.FBX");
			//Load(ToString(Path) + "//dragon.FBX");
		}
		//Load(GetEnv("FBX_SDK_PATH") + "\\samples\\ConvertScene\\box.fbx");
		//Load(GetEnv("FBX_SDK_PATH") + "\\samples\\ViewScene\\humanoid.fbx"); 

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
			VkVertexInputBindingDescription({.binding = 0, .stride = sizeof(Vertices[0]), .inputRate = VK_VERTEX_INPUT_RATE_VERTEX }),
			VkVertexInputBindingDescription({.binding = 1, .stride = sizeof(Normals[0]), .inputRate = VK_VERTEX_INPUT_RATE_VERTEX }),
		};
		const std::vector VIADs = {
			//!< location = 0 は binding = 0
			VkVertexInputAttributeDescription({.location = 0, .binding = 0, .format = VK_FORMAT_R32G32B32_SFLOAT, .offset = 0 }),
			//!< location = 1 は binding = 1
			VkVertexInputAttributeDescription({.location = 1, .binding = 1, .format = VK_FORMAT_R32G32B32_SFLOAT, .offset = 0 }),
		};

		constexpr VkPipelineRasterizationStateCreateInfo PRSCI = {
			.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
			.pNext = nullptr,
			.flags = 0,
			.depthClampEnable = VK_FALSE,
			.rasterizerDiscardEnable = VK_FALSE,
			.polygonMode = VK_POLYGON_MODE_LINE,
			//.polygonMode = VK_POLYGON_MODE_FILL,
			.cullMode = VK_CULL_MODE_BACK_BIT,
			.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
			.depthBiasEnable = VK_FALSE, .depthBiasConstantFactor = 0.0f, .depthBiasClamp = 0.0f, .depthBiasSlopeFactor = 0.0f,
			.lineWidth = 1.0f
		};
		VKExt::CreatePipeline_VsFs_Input(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, 0, PRSCI, VK_FALSE, VIBDs, VIADs, PSSCIs);

		for (auto i : SMs) { vkDestroyShaderModule(Device, i, GetAllocationCallbacks()); }
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
