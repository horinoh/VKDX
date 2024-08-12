#pragma once

#include "resource.h"

#pragma region Code
#include "../FBX.h"
#include "../VKMS.h"
#include "../DXMesh.h"

class MeshletBuildVK : public VKMSDepth, public Fbx
{
private:
	using Super = VKMSDepth;
public:
	MeshletBuildVK() : Super() {}
	virtual ~MeshletBuildVK() {}

	DeviceLocalUniformTexelBuffer VertexBuffer;
	DeviceLocalUniformTexelBuffer VertexIndexBuffer;
	DeviceLocalUniformTexelBuffer MeshletBuffer;
	DeviceLocalUniformTexelBuffer TriangleBuffer;

#pragma region FBX
	std::vector<uint32_t> Indices;
	std::vector<glm::vec3> Vertices;
	std::vector<DirectX::XMFLOAT3> VerticesDX;
	std::vector<glm::vec3> Normals;
	glm::vec3 ToVec3(const FbxVector4& rhs) { return glm::vec3(static_cast<FLOAT>(rhs[0]), static_cast<FLOAT>(rhs[1]), static_cast<FLOAT>(rhs[2])); }
	DirectX::XMFLOAT3 ToFloat3(const FbxVector4& rhs) { return DirectX::XMFLOAT3(static_cast<FLOAT>(rhs[0]), static_cast<FLOAT>(rhs[1]), static_cast<FLOAT>(rhs[2])); }
	virtual void Process(FbxMesh* Mesh) override {
		Fbx::Process(Mesh);

		auto Max = (std::numeric_limits<glm::vec3>::min)();
		auto Min = (std::numeric_limits<glm::vec3>::max)();
		std::cout << "PolygonCount = " << Mesh->GetPolygonCount() << std::endl;
		for (auto i = 0; i < Mesh->GetPolygonCount(); ++i) {
			for (auto j = 0; j < Mesh->GetPolygonSize(i); ++j) {
				Indices.emplace_back(i * Mesh->GetPolygonSize(i) + j);

				Vertices.emplace_back(ToVec3(Mesh->GetControlPoints()[Mesh->GetPolygonVertex(i, j)]));
				VerticesDX.emplace_back(ToFloat3(Mesh->GetControlPoints()[Mesh->GetPolygonVertex(i, j)]));
				Max.x = (std::max)(Max.x, VerticesDX.back().x);
				Max.y = (std::max)(Max.y, VerticesDX.back().y);
				Max.z = (std::max)(Max.z, VerticesDX.back().z);
				Min.x = (std::min)(Min.x, VerticesDX.back().x);
				Min.y = (std::min)(Min.y, VerticesDX.back().y);
				Min.z = (std::min)(Min.z, VerticesDX.back().z);
			}
		}
		AdjustScale(Vertices, 1.0f);
		//DX::AdjustScale(VerticesDX, Min, Max);
		const auto Bound = (std::max)((std::max)(Max.x - Min.x, Max.y - Min.y), Max.z - Min.z) * 1.0f;
		std::ranges::transform(VerticesDX, std::begin(VerticesDX), [&](const DirectX::XMFLOAT3& rhs) { return DirectX::XMFLOAT3(rhs.x / Bound, (rhs.y - (Max.y - Min.y) * 0.5f) / Bound, (rhs.z - Min.z) / Bound); });

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

	void CreateGeometry() override {
		if (HasMeshShaderSupport(SelectedPhysDevice.first)) {
			const auto& CB = CommandBuffers[0];
			const auto& PDMP = SelectedPhysDevice.second.PDMP;

			//Load(FBX_PATH / "dragon.FBX");
			//Load(FBX_PATH / "bunny4.FBX");
			Load(FBX_PATH / "bunny.FBX");

			std::vector<DirectX::Meshlet> Meshlets;
			std::vector<uint8_t> VertexIndices8;
			std::vector<DirectX::MeshletTriangle> Triangles; //!< uint32_t の 30bit を使用して i0, i1, i2 それぞれ 10bit
			if (FAILED(DirectX::ComputeMeshlets(data(Indices), size(Indices) / 3, data(VerticesDX), size(VerticesDX), nullptr, Meshlets, VertexIndices8, Triangles,
				DirectX::MESHLET_DEFAULT_MAX_VERTS, DirectX::MESHLET_DEFAULT_MAX_PRIMS))) { assert(false); }

			{
				Logf("Vertex Count = %d\n", size(Vertices));
				Logf("Index Count = %d\n", size(Indices));

				//!< VertexIndices8 は UINT32 や UINT16 の配列に置き換えて参照する、ここでは UINT32
				//const auto VertexIndices16 = reinterpret_cast<const uint16_t*>(data(VertexIndices8));
				const auto VertexIndices32 = reinterpret_cast<const uint32_t*>(data(VertexIndices8));
				assert(size(Vertices) == TotalSizeOf(VertexIndices8) / sizeof(Indices[0]) && "");

				Log("---- Meshlet build ----\n");
				Logf("Meshlet Count = %d\n", size(Meshlets));
				Logf("VertexIndex Count = %d\n", size(VertexIndices8));
				Logf("Triangle Count = %d\n", size(Triangles));
				for (size_t i = 0; i < (std::min<size_t>)(size(Meshlets), 8); ++i) {
					const auto& ML = Meshlets[i];
					Logf("\tMeshlet [%d] PrimCount = %d, PrimOffset = %d, VertCount = %d, VertOffset = %d\n", i, ML.PrimCount, ML.PrimOffset, ML.VertCount, ML.VertOffset);
					for (uint32_t j = 0; j < (std::min<uint32_t>)(ML.PrimCount, 8); ++j) {
						const auto& Tri = Triangles[ML.PrimOffset + j];
						Logf("\t\t%d, %d, %d => %d, %d, %d\n", Tri.i0, Tri.i1, Tri.i2, VertexIndices32[ML.VertOffset + Tri.i0], VertexIndices32[ML.VertOffset + Tri.i1], VertexIndices32[ML.VertOffset + Tri.i2]);
					}
					Log("\t\t...\n");
				}
				Log("\t...\n");
			}

			//!< テクセルバッファ系はフォーマットがサポートされるかチェックする (UNIFORM_TEXEL_BUFFER が R32G32B32A32_SFLOAT をサポートするか)
#ifdef _DEBUG
			VkFormatProperties FP;
			const auto PD = SelectedPhysDevice.first;
			vkGetPhysicalDeviceFormatProperties(PD, VK_FORMAT_R32G32B32A32_SFLOAT, &FP); 
			assert((FP.bufferFeatures & VK_FORMAT_FEATURE_UNIFORM_TEXEL_BUFFER_BIT) && "Format not supported");
			vkGetPhysicalDeviceFormatProperties(PD, VK_FORMAT_R32_UINT, &FP);
			assert((FP.bufferFeatures & VK_FORMAT_FEATURE_UNIFORM_TEXEL_BUFFER_BIT) && "Format not supported");
#endif
			VertexBuffer.Create(Device, PDMP, TotalSizeOf(Vertices), VK_FORMAT_R32G32B32A32_SFLOAT, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT)
				.SubmitCopyCommand(Device, PDMP, CB, GraphicsQueue, TotalSizeOf(Vertices), data(Vertices), VK_ACCESS_NONE_KHR, VK_PIPELINE_STAGE_MESH_SHADER_BIT_EXT);
			VertexIndexBuffer.Create(Device, PDMP, TotalSizeOf(VertexIndices8), VK_FORMAT_R32_UINT, VK_BUFFER_USAGE_INDEX_BUFFER_BIT)
				.SubmitCopyCommand(Device, PDMP, CB, GraphicsQueue, TotalSizeOf(VertexIndices8), data(VertexIndices8), VK_ACCESS_NONE_KHR, VK_PIPELINE_STAGE_MESH_SHADER_BIT_EXT);

			VkPhysicalDeviceProperties PDP = {}; vkGetPhysicalDeviceProperties(SelectedPhysDevice.first, &PDP);
			assert(TotalSizeOf(Meshlets) == Cmn::RoundUp(TotalSizeOf(Meshlets), PDP.limits.minStorageBufferOffsetAlignment));
			MeshletBuffer.Create(Device, PDMP, TotalSizeOf(Meshlets), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT)
				.SubmitCopyCommand(Device, PDMP, CB, GraphicsQueue, TotalSizeOf(Meshlets), data(Meshlets), VK_ACCESS_NONE_KHR, VK_PIPELINE_STAGE_MESH_SHADER_BIT_EXT); 
			assert(TotalSizeOf(Triangles) == Cmn::RoundUp(TotalSizeOf(Triangles), PDP.limits.minStorageBufferOffsetAlignment));
			TriangleBuffer.Create(Device, PDMP, TotalSizeOf(Triangles), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT)
				.SubmitCopyCommand(Device, PDMP, CB, GraphicsQueue, TotalSizeOf(Triangles), data(Triangles), VK_ACCESS_NONE_KHR, VK_PIPELINE_STAGE_MESH_SHADER_BIT_EXT);

			const VkDrawMeshTasksIndirectCommandEXT DMTIC = { .groupCountX = static_cast<uint32_t>(IterationCount(size(Meshlets), 32)), .groupCountY = 1, .groupCountZ = 1 };
			Logf("Meshlet Chunk Count = %d\n", DMTIC.groupCountX);
			IndirectBuffers.emplace_back().Create(Device, PDMP, DMTIC).SubmitCopyCommand(Device, PDMP, CB, GraphicsQueue, sizeof(DMTIC), &DMTIC);
		}
	}
	virtual void CreatePipelineLayout() override {
		CreateDescriptorSetLayout(DescriptorSetLayouts.emplace_back(), 0, {
			VkDescriptorSetLayoutBinding({.binding = 0, .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, .descriptorCount = 1, .stageFlags = VK_SHADER_STAGE_MESH_BIT_EXT, .pImmutableSamplers = nullptr }),
			VkDescriptorSetLayoutBinding({.binding = 1, .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER , .descriptorCount = 1, .stageFlags = VK_SHADER_STAGE_MESH_BIT_EXT, .pImmutableSamplers = nullptr }),
			VkDescriptorSetLayoutBinding({.binding = 2, .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER , .descriptorCount = 1, .stageFlags = VK_SHADER_STAGE_MESH_BIT_EXT, .pImmutableSamplers = nullptr }),
			VkDescriptorSetLayoutBinding({.binding = 3, .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER , .descriptorCount = 1, .stageFlags = VK_SHADER_STAGE_MESH_BIT_EXT, .pImmutableSamplers = nullptr }),
		});
		VK::CreatePipelineLayout(PipelineLayouts.emplace_back(), DescriptorSetLayouts, {});
	}
	virtual void CreatePipeline() override {
		Pipelines.emplace_back();

		if (HasMeshShaderSupport(SelectedPhysDevice.first)) {
			const std::array SMs = {
				VK::CreateShaderModule(GetFilePath(".task.spv")),
				VK::CreateShaderModule(GetFilePath(".mesh.spv")),
				VK::CreateShaderModule(GetFilePath(".frag.spv")),
			};
			const std::array PSSCIs = {
				VkPipelineShaderStageCreateInfo({.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, .pNext = nullptr, .flags = 0, .stage = VK_SHADER_STAGE_TASK_BIT_EXT, .module = SMs[0], .pName = "main", .pSpecializationInfo = nullptr }),
				VkPipelineShaderStageCreateInfo({.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, .pNext = nullptr, .flags = 0, .stage = VK_SHADER_STAGE_MESH_BIT_EXT, .module = SMs[1], .pName = "main", .pSpecializationInfo = nullptr }),
				VkPipelineShaderStageCreateInfo({.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, .pNext = nullptr, .flags = 0, .stage = VK_SHADER_STAGE_FRAGMENT_BIT, .module = SMs[2], .pName = "main", .pSpecializationInfo = nullptr }),
			};
			CreatePipeline_TsMsFs(Pipelines[0], PipelineLayouts[0], RenderPasses[0], VK_TRUE, PSSCIs);
			
			for (auto& i : Threads) { i.join(); }
			Threads.clear();

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
		VK::CreateDescriptorUpdateTemplate(DUT, VK_PIPELINE_BIND_POINT_GRAPHICS, {
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

			constexpr std::array CVs = { VkClearValue({.color = Colors::SkyBlue }), VkClearValue({.depthStencil = {.depth = 1.0f, .stencil = 0 } }) };
			const VkRenderPassBeginInfo RPBI = {
				.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
				.pNext = nullptr,
				.renderPass = RenderPasses[0],
				.framebuffer = Framebuffers[i],
				.renderArea = VkRect2D({.offset = VkOffset2D({.x = 0, .y = 0 }), .extent = Swapchain.Extent }),
				.clearValueCount = static_cast<uint32_t>(size(CVs)), .pClearValues = data(CVs)
			};
			vkCmdBeginRenderPass(CB, &RPBI, VK_SUBPASS_CONTENTS_INLINE); {
				if (HasMeshShaderSupport(SelectedPhysDevice.first)) {
					vkCmdDrawMeshTasksIndirectEXT(CB, IndirectBuffers[0].Buffer, 0, 1, 0);
				}
			} vkCmdEndRenderPass(CB);
		} VERIFY_SUCCEEDED(vkEndCommandBuffer(CB));
	}
};
#pragma endregion