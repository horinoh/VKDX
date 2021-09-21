#pragma once

#include "resource.h"

#pragma region Code
#include "../FBX.h"
#include "../VKExt.h"
#include "../DXMesh.h"

class MeshletBuildVK : public VKExt, public Fbx
{
private:
	using Super = VKExt;
public:
	MeshletBuildVK() : Super() {}
	virtual ~MeshletBuildVK() {}

#pragma region FBX
	std::vector<uint32_t> Indices;
	//std::vector<glm::vec3> Vertices;
	std::vector<DirectX::XMFLOAT3> Vertices;
	std::vector<glm::vec3> Normals;
	glm::vec3 ToVec3(const FbxVector4& rhs) { return glm::vec3(static_cast<FLOAT>(rhs[0]), static_cast<FLOAT>(rhs[1]), static_cast<FLOAT>(rhs[2])); }
	DirectX::XMFLOAT3 ToFloat3(const FbxVector4& rhs) { return DirectX::XMFLOAT3(static_cast<FLOAT>(rhs[0]), static_cast<FLOAT>(rhs[1]), static_cast<FLOAT>(rhs[2])); }
	virtual void Process(FbxMesh* Mesh) override {
		Fbx::Process(Mesh);

		auto Max = glm::vec3((std::numeric_limits<float>::min)());
		auto Min = glm::vec3((std::numeric_limits<float>::max)());
		std::cout << "PolygonCount = " << Mesh->GetPolygonCount() << std::endl;
		for (auto i = 0; i < Mesh->GetPolygonCount(); ++i) {
			for (auto j = 0; j < Mesh->GetPolygonSize(i); ++j) {
				Indices.emplace_back(i * Mesh->GetPolygonSize(i) + j);

				//Vertices.emplace_back(ToVec3(Mesh->GetControlPoints()[Mesh->GetPolygonVertex(i, j)]));
				Vertices.emplace_back(ToFloat3(Mesh->GetControlPoints()[Mesh->GetPolygonVertex(i, j)]));
				Max.x = std::max(Max.x, Vertices.back().x);
				Max.y = std::max(Max.y, Vertices.back().y);
				Max.z = std::max(Max.z, Vertices.back().z);
				Min.x = std::min(Min.x, Vertices.back().x);
				Min.y = std::min(Min.y, Vertices.back().y);
				Min.z = std::min(Min.z, Vertices.back().z);
			}
		}
		const auto Bound = std::max(std::max(Max.x - Min.x, Max.y - Min.y), Max.z - Min.z) * 1.0f;
		//std::transform(begin(Vertices), end(Vertices), begin(Vertices), [&](const glm::vec3& rhs) { return rhs / Bound - glm::vec3(0.0f, (Max.y - Min.y) * 0.5f, Min.z) / Bound; });
		std::transform(begin(Vertices), end(Vertices), begin(Vertices), [&](const DirectX::XMFLOAT3& rhs) { return DirectX::XMFLOAT3(rhs.x / Bound, (rhs.y - (Max.y - Min.y) * 0.5f) / Bound, (rhs.z - Min.z) / Bound); });

		FbxArray<FbxVector4> Nrms;
		Mesh->GetPolygonVertexNormals(Nrms);
		for (auto i = 0; i < Nrms.Size(); ++i) {
			Normals.emplace_back(ToVec3(Nrms[i]));
		}

		FbxStringList UVSetNames;
		Mesh->GetUVSetNames(UVSetNames);
		for (auto i = 0; i < UVSetNames.GetCount(); ++i) {
			FbxArray<FbxVector2> UVs;
			Mesh->GetPolygonVertexUVs(UVSetNames.GetStringAt(i), UVs);
		}
	}
#pragma endregion

	virtual void CreateInstance([[maybe_unused]] const std::vector<const char*>& AdditionalLayers, const std::vector<const char*>& AdditionalExtensions) override {
		//!< VK_LAYER_RENDERDOC_Capture ‚ðŽg—p‚µ‚È‚¢
		VK::CreateInstance(AdditionalLayers, AdditionalExtensions);
	}
	virtual void CreateDevice(HWND hWnd, HINSTANCE hInstance, [[maybe_unused]] void* pNext, [[maybe_unused]] const std::vector<const char*>& AddExtensions) override {
		if (HasMeshShaderSupport(GetCurrentPhysicalDevice())) {
			VkPhysicalDeviceMeshShaderFeaturesNV PDMSF = { .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_FEATURES_NV, .pNext = nullptr, .taskShader = VK_TRUE, .meshShader = VK_TRUE, };
			Super::CreateDevice(hWnd, hInstance, &PDMSF, {
				VK_NV_MESH_SHADER_EXTENSION_NAME
				});
		}
		else {
			Super::CreateDevice(hWnd, hInstance, pNext, AddExtensions);
		}
	}
	void CreateGeometry() override {
		std::wstring Path;
		if (FindDirectory("FBX", Path)) {
			Load(ToString(Path) + "//bunny4.FBX");
		}
		std::vector<DirectX::Meshlet> Meshlets;
		std::vector<uint8_t> VertexIndices;
		std::vector<DirectX::MeshletTriangle> Triangles;
		DirectX::ComputeMeshlets(data(Indices), size(Indices) / 3, data(Vertices), size(Vertices), nullptr, Meshlets, VertexIndices, Triangles);

		if (HasMeshShaderSupport(GetCurrentPhysicalDevice())) {
			const auto& CB = CommandBuffers[0];
			const auto PDMP = GetCurrentPhysicalDeviceMemoryProperties();
			constexpr VkDrawMeshTasksIndirectCommandNV DMTIC = { .taskCount = 1, .firstTask = 0 };
			IndirectBuffers.emplace_back().Create(Device, PDMP, DMTIC).SubmitCopyCommand(Device, PDMP, CB, GraphicsQueue, sizeof(DMTIC), &DMTIC);
		}
	}
	virtual void CreateRenderPass() { VKExt::CreateRenderPass_Clear(); }
	virtual void CreatePipeline() override {
		if (HasMeshShaderSupport(GetCurrentPhysicalDevice())) {
			const auto ShaderPath = GetBasePath();
			const std::array SMs = {
				VK::CreateShaderModule(data(ShaderPath + TEXT(".task.spv"))),
				VK::CreateShaderModule(data(ShaderPath + TEXT(".mesh.spv"))),
				VK::CreateShaderModule(data(ShaderPath + TEXT(".frag.spv"))),
			};
			const std::array PSSCIs = {
				VkPipelineShaderStageCreateInfo({.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, .pNext = nullptr, .flags = 0, .stage = VK_SHADER_STAGE_TASK_BIT_NV, .module = SMs[0], .pName = "main", .pSpecializationInfo = nullptr }),
				VkPipelineShaderStageCreateInfo({.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, .pNext = nullptr, .flags = 0, .stage = VK_SHADER_STAGE_MESH_BIT_NV, .module = SMs[1], .pName = "main", .pSpecializationInfo = nullptr }),
				VkPipelineShaderStageCreateInfo({.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, .pNext = nullptr, .flags = 0, .stage = VK_SHADER_STAGE_FRAGMENT_BIT, .module = SMs[2], .pName = "main", .pSpecializationInfo = nullptr }),
			};
			CreatePipeline_TsMsFs(VK_FALSE, PSSCIs);
			for (auto i : SMs) { vkDestroyShaderModule(Device, i, GetAllocationCallbacks()); }
		}
	}
	virtual void PopulateCommandBuffer(const size_t i) override {
		const auto CB = CommandBuffers[i];
		constexpr VkCommandBufferBeginInfo CBBI = {
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
			.pNext = nullptr,
			.flags = 0,
			.pInheritanceInfo = nullptr
		};
		VERIFY_SUCCEEDED(vkBeginCommandBuffer(CB, &CBBI)); {
			vkCmdSetViewport(CB, 0, static_cast<uint32_t>(size(Viewports)), data(Viewports));
			vkCmdSetScissor(CB, 0, static_cast<uint32_t>(size(ScissorRects)), data(ScissorRects));

			vkCmdBindPipeline(CB, VK_PIPELINE_BIND_POINT_GRAPHICS, Pipelines[0]);

			constexpr std::array CVs = { VkClearValue({.color = Colors::SkyBlue }) };
			const VkRenderPassBeginInfo RPBI = {
				.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
				.pNext = nullptr,
				.renderPass = RenderPasses[0],
				.framebuffer = Framebuffers[i],
				.renderArea = VkRect2D({.offset = VkOffset2D({.x = 0, .y = 0 }), .extent = SurfaceExtent2D }),
				.clearValueCount = static_cast<uint32_t>(size(CVs)), .pClearValues = data(CVs)
			};
			vkCmdBeginRenderPass(CB, &RPBI, VK_SUBPASS_CONTENTS_INLINE); {
				if (HasMeshShaderSupport(GetCurrentPhysicalDevice())) {
					vkCmdDrawMeshTasksIndirectNV(CB, IndirectBuffers[0].Buffer, 0, 1, 0);
				}
			} vkCmdEndRenderPass(CB);
		} VERIFY_SUCCEEDED(vkEndCommandBuffer(CB));
	}
};
#pragma endregion