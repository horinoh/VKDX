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

	DeviceLocalUniformTexelBuffer VertexBuffer;
	DeviceLocalUniformTexelBuffer VertexIndexBuffer;
	DeviceLocalStorageBuffer MeshletBuffer;
	DeviceLocalStorageBuffer TriangleBuffer;

#pragma region FBX
	std::vector<uint32_t> Indices;
	std::vector<glm::vec3> Vertices;
	std::vector<DirectX::XMFLOAT3> VerticesDX;
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

				Vertices.emplace_back(ToVec3(Mesh->GetControlPoints()[Mesh->GetPolygonVertex(i, j)]));
				VerticesDX.emplace_back(ToFloat3(Mesh->GetControlPoints()[Mesh->GetPolygonVertex(i, j)]));
				Max.x = std::max(Max.x, VerticesDX.back().x);
				Max.y = std::max(Max.y, VerticesDX.back().y);
				Max.z = std::max(Max.z, VerticesDX.back().z);
				Min.x = std::min(Min.x, VerticesDX.back().x);
				Min.y = std::min(Min.y, VerticesDX.back().y);
				Min.z = std::min(Min.z, VerticesDX.back().z);
			}
		}
		const auto Bound = std::max(std::max(Max.x - Min.x, Max.y - Min.y), Max.z - Min.z) * 1.0f;
		std::transform(begin(Vertices), end(Vertices), begin(Vertices), [&](const glm::vec3& rhs) { return rhs / Bound - glm::vec3(0.0f, (Max.y - Min.y) * 0.5f, Min.z) / Bound; });
		std::transform(begin(VerticesDX), end(VerticesDX), begin(VerticesDX), [&](const DirectX::XMFLOAT3& rhs) { return DirectX::XMFLOAT3(rhs.x / Bound, (rhs.y - (Max.y - Min.y) * 0.5f) / Bound, (rhs.z - Min.z) / Bound); });

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

	virtual void OnDestroy(HWND hWnd, HINSTANCE hInstance) override {
		TriangleBuffer.Destroy(Device);
		MeshletBuffer.Destroy(Device);
		VertexIndexBuffer.Destroy(Device);
		VertexBuffer.Destroy(Device);
		Super::OnDestroy(hWnd, hInstance);
	}

	virtual void CreateInstance([[maybe_unused]] const std::vector<const char*>& AdditionalLayers, const std::vector<const char*>& AdditionalExtensions) override {
		//!< VK_LAYER_RENDERDOC_Capture ���g�p���Ȃ�
		VK::CreateInstance(AdditionalLayers, AdditionalExtensions);
	}
	virtual void CreateDevice(HWND hWnd, HINSTANCE hInstance, [[maybe_unused]] void* pNext, [[maybe_unused]] const std::vector<const char*>& AddExtensions) override {
		if (HasMeshShaderSupport(GetCurrentPhysicalDevice())) {
			VkPhysicalDeviceMeshShaderFeaturesNV PDMSF = { .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_FEATURES_NV, .pNext = nullptr, .taskShader = VK_TRUE, .meshShader = VK_TRUE, };
			Super::CreateDevice(hWnd, hInstance, &PDMSF, { VK_NV_MESH_SHADER_EXTENSION_NAME });
		}
		else {
			Super::CreateDevice(hWnd, hInstance, pNext, AddExtensions);
		}
	}
	void CreateGeometry() override {
		if (HasMeshShaderSupport(GetCurrentPhysicalDevice())) {
			const auto& CB = CommandBuffers[0];
			const auto PDMP = GetCurrentPhysicalDeviceMemoryProperties();
			constexpr VkDrawMeshTasksIndirectCommandNV DMTIC = { .taskCount = 1, .firstTask = 0 };
			IndirectBuffers.emplace_back().Create(Device, PDMP, DMTIC).SubmitCopyCommand(Device, PDMP, CB, GraphicsQueue, sizeof(DMTIC), &DMTIC);

			std::wstring Path;
			if (FindDirectory("FBX", Path)) {
				Load(ToString(Path) + "//bunny4.FBX");
			}

			std::vector<DirectX::Meshlet> Meshlets;
			std::vector<uint8_t> VertexIndices;
			std::vector<DirectX::MeshletTriangle> Triangles; //!< uint32_t �� 30bit ���g�p���� i0, i1, i2 ���ꂼ�� 10bit
			if (FAILED(DirectX::ComputeMeshlets(data(Indices), size(Indices) / 3, data(VerticesDX), size(VerticesDX), nullptr, Meshlets, VertexIndices, Triangles,
				DirectX::MESHLET_DEFAULT_MAX_VERTS, DirectX::MESHLET_DEFAULT_MAX_PRIMS))) { assert(false); }

			//assert(size(Meshlets) <= 32 && "32 �𒴂���ꍇ�͕�����ɕ����ĕ`�悷��K�v�����邪�A�����ł͒����Ă͂����Ȃ����ƂƂ���");
			Logf("Meshlets Count = %d\n", size(Meshlets));
			for (size_t i = 0; i < std::min<size_t>(size(Meshlets), 8); ++i) {
				Logf("\t[%d] VertCount = %d, PrimCount = %d\n", i, Meshlets[i].VertCount, Meshlets[i].PrimCount);
			}
			Log("\t...\n");

			//!< �e�N�Z���o�b�t�@�n�̓t�H�[�}�b�g���T�|�[�g����邩�`�F�b�N���� (UNIFORM_TEXEL_BUFFER �� R32G32B32A32_SFLOAT ���T�|�[�g���邩)
#ifdef _DEBUG
			VkFormatProperties FP;
			const auto PD = GetCurrentPhysicalDevice();
			vkGetPhysicalDeviceFormatProperties(PD, VK_FORMAT_R32G32B32A32_SFLOAT, &FP); 
			assert((FP.bufferFeatures & VK_FORMAT_FEATURE_UNIFORM_TEXEL_BUFFER_BIT) && "Format not supported");
			vkGetPhysicalDeviceFormatProperties(PD, VK_FORMAT_R32_UINT, &FP);
			assert((FP.bufferFeatures & VK_FORMAT_FEATURE_UNIFORM_TEXEL_BUFFER_BIT) && "Format not supported");
#endif
			VertexBuffer.Create(Device, PDMP, TotalSizeOf(Vertices), VK_FORMAT_R32G32B32A32_SFLOAT)
				.SubmitCopyCommand(Device, PDMP, CB, GraphicsQueue, TotalSizeOf(Vertices), data(Vertices), VK_ACCESS_NONE_KHR, VK_PIPELINE_STAGE_MESH_SHADER_BIT_NV);
			VertexIndexBuffer.Create(Device, PDMP, TotalSizeOf(VertexIndices), VK_FORMAT_R32_UINT)
				.SubmitCopyCommand(Device, PDMP, CB, GraphicsQueue, TotalSizeOf(VertexIndices), data(VertexIndices), VK_ACCESS_NONE_KHR, VK_PIPELINE_STAGE_MESH_SHADER_BIT_NV);
			MeshletBuffer.Create(Device, PDMP, TotalSizeOf(Meshlets))
				.SubmitCopyCommand(Device, PDMP, CB, GraphicsQueue, TotalSizeOf(Meshlets), data(Meshlets), VK_ACCESS_NONE_KHR, VK_PIPELINE_STAGE_MESH_SHADER_BIT_NV); 
			TriangleBuffer.Create(Device, PDMP, TotalSizeOf(Triangles))
				.SubmitCopyCommand(Device, PDMP, CB, GraphicsQueue, TotalSizeOf(Triangles), data(Triangles), VK_ACCESS_NONE_KHR, VK_PIPELINE_STAGE_MESH_SHADER_BIT_NV);
		}
	}
	virtual void CreateRenderPass() { VKExt::CreateRenderPass_Clear(); }
	virtual void CreatePipelineLayout() override {
		CreateDescriptorSetLayout(DescriptorSetLayouts.emplace_back(), 0, {
			VkDescriptorSetLayoutBinding({.binding = 0, .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, .descriptorCount = 1, .stageFlags = VK_SHADER_STAGE_MESH_BIT_NV, .pImmutableSamplers = nullptr }),
			VkDescriptorSetLayoutBinding({.binding = 1, .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER , .descriptorCount = 1, .stageFlags = VK_SHADER_STAGE_MESH_BIT_NV, .pImmutableSamplers = nullptr }),
			VkDescriptorSetLayoutBinding({.binding = 2, .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER , .descriptorCount = 1, .stageFlags = VK_SHADER_STAGE_MESH_BIT_NV, .pImmutableSamplers = nullptr }),
			VkDescriptorSetLayoutBinding({.binding = 3, .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER , .descriptorCount = 1, .stageFlags = VK_SHADER_STAGE_MESH_BIT_NV, .pImmutableSamplers = nullptr }),
		});
		VK::CreatePipelineLayout(PipelineLayouts.emplace_back(), DescriptorSetLayouts, {});
	}
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
	virtual void CreateDescriptor() override {
		VKExt::CreateDescriptorPool(DescriptorPools.emplace_back(), 0, {
			VkDescriptorPoolSize({.type = VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, .descriptorCount = 2 }),
			VkDescriptorPoolSize({.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, .descriptorCount = 2 }),
		});
		const std::array DSLs = { DescriptorSetLayouts[0] };
		const VkDescriptorSetAllocateInfo DSAI = {
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
			.pNext = nullptr,
			.descriptorPool = DescriptorPools[0],
			.descriptorSetCount = static_cast<uint32_t>(size(DSLs)), .pSetLayouts = data(DSLs)
		};
		VERIFY_SUCCEEDED(vkAllocateDescriptorSets(Device, &DSAI, &DescriptorSets.emplace_back()));

		struct DescriptorUpdateInfo
		{
			VkBufferView BV_0;
			VkBufferView BV_1;
			VkDescriptorBufferInfo DBI_0;
			VkDescriptorBufferInfo DBI_1;
		};
		VkDescriptorUpdateTemplate DUT;
		VK::CreateDescriptorUpdateTemplate(DUT, {
			VkDescriptorUpdateTemplateEntry({
				.dstBinding = 0, .dstArrayElement = 0,
				.descriptorCount = 1, .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER,
				.offset = offsetof(DescriptorUpdateInfo, BV_0), .stride = sizeof(DescriptorUpdateInfo)
			}),
			VkDescriptorUpdateTemplateEntry({
				.dstBinding = 1, .dstArrayElement = 0,
				.descriptorCount = 1, .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER,
				.offset = offsetof(DescriptorUpdateInfo, BV_1), .stride = sizeof(DescriptorUpdateInfo)
			}),
			VkDescriptorUpdateTemplateEntry({
				.dstBinding = 2, .dstArrayElement = 0,
				.descriptorCount = 1, .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
				.offset = offsetof(DescriptorUpdateInfo, DBI_0), .stride = sizeof(DescriptorUpdateInfo)
			}),
			VkDescriptorUpdateTemplateEntry({
				.dstBinding = 3, .dstArrayElement = 0,
				.descriptorCount = 1, .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
				.offset = offsetof(DescriptorUpdateInfo, DBI_1), .stride = sizeof(DescriptorUpdateInfo)
			}),
		}, DescriptorSetLayouts[0]);
		const DescriptorUpdateInfo DUI = {
			VertexBuffer.View,
			VertexIndexBuffer.View,
			VkDescriptorBufferInfo({.buffer = MeshletBuffer.Buffer, .offset = 0, .range = VK_WHOLE_SIZE }),
			VkDescriptorBufferInfo({.buffer = TriangleBuffer.Buffer, .offset = 0, .range = VK_WHOLE_SIZE }),
		};
		vkUpdateDescriptorSetWithTemplate(Device, DescriptorSets[0], DUT, &DUI);
		vkDestroyDescriptorUpdateTemplate(Device, DUT, GetAllocationCallbacks());
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

			const std::array DSs = { DescriptorSets[0] };
			constexpr std::array<uint32_t, 0> DynamicOffsets = {};
			vkCmdBindDescriptorSets(CB, VK_PIPELINE_BIND_POINT_GRAPHICS, PipelineLayouts[0], 0, static_cast<uint32_t>(size(DSs)), data(DSs), static_cast<uint32_t>(size(DynamicOffsets)), data(DynamicOffsets));

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