#pragma once

#include "resource.h"

#pragma region Code
#include "../VKRT.h"

class RTShadowVK : public VKRT
{
private:
	using Super = VKRT;
public:
	RTShadowVK() : Super() {}
	virtual ~RTShadowVK() {}

	virtual void CreateGeometry() override {
		if (!HasRayTracingSupport(SelectedPhysDevice.first)) { return; }

		const auto& PDMP = SelectedPhysDevice.second.PDMP;
		const auto& CB = CommandBuffers[0];

#pragma region BLAS_GEOMETRY
		const std::array Vertices = { glm::vec3({ 0.0f, 0.5f, 0.0f }), glm::vec3({ -0.5f, -0.5f, 0.0f }), glm::vec3({ 0.5f, -0.5f, 0.0f }), };
		Scoped<HostVisibleASBuffer> VB(Device);
		VB.Create(Device, PDMP, TotalSizeOf(Vertices), data(Vertices));

		constexpr std::array Indices = { uint32_t(0), uint32_t(1), uint32_t(2) };
		Scoped<HostVisibleASBuffer> IB(Device);
		IB.Create(Device, PDMP, TotalSizeOf(Indices), data(Indices));

		const std::array ASGs_Blas = {
			VkAccelerationStructureGeometryKHR({
				.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR,
				.pNext = nullptr,
				.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_KHR,
				.geometry = VkAccelerationStructureGeometryDataKHR({
					.triangles = VkAccelerationStructureGeometryTrianglesDataKHR({
						.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR,
						.pNext = nullptr,
						.vertexFormat = VK_FORMAT_R32G32B32_SFLOAT,
						.vertexData = VkDeviceOrHostAddressConstKHR({.deviceAddress = GetDeviceAddress(Device, VB.Buffer)}), .vertexStride = sizeof(Vertices[0]), .maxVertex = static_cast<uint32_t>(size(Vertices)),
						.indexType = VK_INDEX_TYPE_UINT32,
						.indexData = VkDeviceOrHostAddressConstKHR({.deviceAddress = GetDeviceAddress(Device, IB.Buffer)}),
						.transformData = VkDeviceOrHostAddressConstKHR({.deviceAddress = 0}),
					}),
				}),
				.flags = VK_GEOMETRY_OPAQUE_BIT_KHR
			}),
		};
#pragma endregion

#pragma region BLAS
		VkAccelerationStructureBuildGeometryInfoKHR ASBGI_Blas = {
			.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR,
			.pNext = nullptr,
			.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR, //!< ボトムレベルを指定
			.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR,
			.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR,
			.srcAccelerationStructure = VK_NULL_HANDLE, .dstAccelerationStructure = VK_NULL_HANDLE,
			.geometryCount = static_cast<uint32_t>(size(ASGs_Blas)), .pGeometries = data(ASGs_Blas), .ppGeometries = nullptr, 
			.scratchData = VkDeviceOrHostAddressKHR({.deviceAddress = 0})
		};
		const std::vector ASBRIs_Blas = {
			VkAccelerationStructureBuildRangeInfoKHR({.primitiveCount = static_cast<uint32_t>(size(Indices) / 3), .primitiveOffset = 0, .firstVertex = 0, .transformOffset = 0 }),
		};
		assert(ASBGI_Blas.geometryCount == size(ASBRIs_Blas));

		VkAccelerationStructureBuildSizesInfoKHR ASBSI_Blas = { .sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR, };
		{
			const std::array MaxPrimitiveCounts = {
				std::ranges::max_element(ASBRIs_Blas, [](const auto& lhs, const auto& rhs) { return lhs.primitiveCount < rhs.primitiveCount; })->primitiveCount
			};
			assert(ASBGI_Blas.geometryCount == size(MaxPrimitiveCounts));
			vkGetAccelerationStructureBuildSizesKHR(Device, VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR, &ASBGI_Blas, data(MaxPrimitiveCounts), &ASBSI_Blas);
		}

		BLASs.emplace_back().Create(Device, PDMP, ASBSI_Blas.accelerationStructureSize);
		ASBGI_Blas.dstAccelerationStructure = BLASs.back().AccelerationStructure;
#pragma endregion

#pragma region TLAS_GEOMETRY
		const auto Tri = glm::mat4x3(glm::scale(glm::rotate(glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.75f, 0.0f)), glm::radians(45.0f), glm::vec3(1.0f, 0.0f, 0.0f)), glm::vec3(0.25f, 0.25f, 1.0f)));
		const auto Floor = glm::mat4x3(glm::scale(glm::rotate(glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -0.75f, 0.0f)), glm::radians(80.0f), glm::vec3(1.0f, 0.0f, 0.0f)), glm::vec3(2.0f, 2.0f, 1.0f)));
		const std::array ASIs = {
			VkAccelerationStructureInstanceKHR({
				.transform = VkTransformMatrixKHR({ Tri[0].x, Tri[1].x, Tri[2].x, Tri[3].x,
													Tri[0].y, Tri[1].y, Tri[2].y, Tri[3].y,
													Tri[0].z, Tri[1].z, Tri[2].z, Tri[3].z, }),
				.instanceCustomIndex = 0,
				.mask = 0xff,
				.instanceShaderBindingTableRecordOffset = 0,
				.flags = VK_GEOMETRY_INSTANCE_TRIANGLE_FRONT_COUNTERCLOCKWISE_BIT_KHR,
				.accelerationStructureReference = GetDeviceAddress(Device, BLASs.back().AccelerationStructure) 
			}),
			VkAccelerationStructureInstanceKHR({
				.transform = VkTransformMatrixKHR({ Floor[0].x, Floor[1].x, Floor[2].x, Floor[3].x,
													Floor[0].y, Floor[1].y, Floor[2].y, Floor[3].y,
													Floor[0].z, Floor[1].z, Floor[2].z, Floor[3].z, }),
				.instanceCustomIndex = 0,
				.mask = 0xff,
				.instanceShaderBindingTableRecordOffset = 0,
				.flags = VK_GEOMETRY_INSTANCE_TRIANGLE_FRONT_COUNTERCLOCKWISE_BIT_KHR,
				.accelerationStructureReference = GetDeviceAddress(Device, BLASs.back().AccelerationStructure)
			}),
		};
		Scoped<HostVisibleASBuffer> InstBuf(Device);
		InstBuf.Create(Device, PDMP, sizeof(ASIs), data(ASIs));

		const std::array ASGs_Tlas = {
			VkAccelerationStructureGeometryKHR({
				.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR,
				.pNext = nullptr,
				.geometryType = VK_GEOMETRY_TYPE_INSTANCES_KHR,
				.geometry = VkAccelerationStructureGeometryDataKHR({
					.instances = VkAccelerationStructureGeometryInstancesDataKHR({
						.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR,
						.pNext = nullptr,
						.arrayOfPointers = VK_FALSE,
						.data = VkDeviceOrHostAddressConstKHR({.deviceAddress = GetDeviceAddress(Device, InstBuf.Buffer)})
					})
				}),
				.flags = VK_GEOMETRY_OPAQUE_BIT_KHR
			}),
		};

#pragma region TLAS
		VkAccelerationStructureBuildGeometryInfoKHR ASBGI_Tlas = {
			.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR,
			.pNext = nullptr,
			.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR, //!< トップレベルを指定
			.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR,
			.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR,
			.srcAccelerationStructure = VK_NULL_HANDLE, .dstAccelerationStructure = VK_NULL_HANDLE,
			.geometryCount = static_cast<uint32_t>(size(ASGs_Tlas)), .pGeometries = data(ASGs_Tlas), .ppGeometries = nullptr, 
			.scratchData = VkDeviceOrHostAddressKHR({.deviceAddress = 0})
		};
		const std::vector ASBRIs_Tlas = {
			VkAccelerationStructureBuildRangeInfoKHR({.primitiveCount = static_cast<uint32_t>(size(ASIs)), .primitiveOffset = 0, .firstVertex = 0, .transformOffset = 0 }),
		};
		assert(ASBGI_Tlas.geometryCount == size(ASBRIs_Tlas));

		VkAccelerationStructureBuildSizesInfoKHR ASBSI_Tlas = { .sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR, };
		{
			const std::array MaxPrimitiveCounts = {
				std::ranges::max_element(ASBRIs_Tlas, [](const auto& lhs, const auto& rhs) { return lhs.primitiveCount < rhs.primitiveCount; })->primitiveCount
			};
			assert(ASBGI_Tlas.geometryCount == size(MaxPrimitiveCounts));
			vkGetAccelerationStructureBuildSizesKHR(Device, VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR, &ASBGI_Tlas, data(MaxPrimitiveCounts), &ASBSI_Tlas);
		}

		TLASs.emplace_back().Create(Device, PDMP, ASBSI_Tlas.accelerationStructureSize);
		ASBGI_Tlas.dstAccelerationStructure = TLASs.back().AccelerationStructure;
#pragma endregion

#pragma region SCRATCH
		Scoped<ScratchBuffer> Scratch(Device);
		Scratch.Create(Device, PDMP, std::max(ASBSI_Blas.buildScratchSize, ASBSI_Tlas.buildScratchSize));
		ASBGI_Blas.scratchData = VkDeviceOrHostAddressKHR({ .deviceAddress = GetDeviceAddress(Device, Scratch.Buffer) });
		ASBGI_Tlas.scratchData = VkDeviceOrHostAddressKHR({ .deviceAddress = GetDeviceAddress(Device, Scratch.Buffer) });
#pragma endregion

		constexpr VkCommandBufferBeginInfo CBBI = { .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO, .pNext = nullptr, .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, .pInheritanceInfo = nullptr };
		VERIFY_SUCCEEDED(vkBeginCommandBuffer(CB, &CBBI)); {
			BLASs.back().PopulateBuildCommand(ASBGI_Blas, ASBRIs_Blas, CB);
			AccelerationStructureBuffer::PopulateMemoryBarrierCommand(CB);

			TLASs.back().PopulateBuildCommand(ASBGI_Tlas, ASBRIs_Tlas, CB);
		} VERIFY_SUCCEEDED(vkEndCommandBuffer(CB));
		SubmitAndWait(GraphicsQueue, CB);
	}
	virtual void CreatePipeline() override {
		if (!HasRayTracingSupport(SelectedPhysDevice.first)) { return; }
#pragma region PIPELINE
		const auto PLL = PipelineLayouts.back();

		const std::array SMs = {
			VK::CreateShaderModule(GetFilePath(".rgen.spv")),
			VK::CreateShaderModule(GetFilePath(".rmiss.spv")),
			VK::CreateShaderModule(GetFilePath("_1.rmiss.spv")),
			VK::CreateShaderModule(GetFilePath(".rchit.spv")),
		};
		const std::array PSSCIs = {
			VkPipelineShaderStageCreateInfo({.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, .pNext = nullptr, .flags = 0, .stage = VK_SHADER_STAGE_RAYGEN_BIT_KHR, .module = SMs[0], .pName = "main", .pSpecializationInfo = nullptr }),
			VkPipelineShaderStageCreateInfo({.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, .pNext = nullptr, .flags = 0, .stage = VK_SHADER_STAGE_MISS_BIT_KHR, .module = SMs[1], .pName = "main", .pSpecializationInfo = nullptr }),
			VkPipelineShaderStageCreateInfo({.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, .pNext = nullptr, .flags = 0, .stage = VK_SHADER_STAGE_MISS_BIT_KHR, .module = SMs[2], .pName = "main", .pSpecializationInfo = nullptr }),
			VkPipelineShaderStageCreateInfo({.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, .pNext = nullptr, .flags = 0, .stage = VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR, .module = SMs[3], .pName = "main", .pSpecializationInfo = nullptr }),
		};
		const std::array RTSGCIs = {
			VkRayTracingShaderGroupCreateInfoKHR({.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR, .pNext = nullptr, .type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR, .generalShader = 0, .closestHitShader = VK_SHADER_UNUSED_KHR, .anyHitShader = VK_SHADER_UNUSED_KHR, .intersectionShader = VK_SHADER_UNUSED_KHR, .pShaderGroupCaptureReplayHandle = nullptr }),
			VkRayTracingShaderGroupCreateInfoKHR({.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR, .pNext = nullptr, .type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR, .generalShader = 1, .closestHitShader = VK_SHADER_UNUSED_KHR, .anyHitShader = VK_SHADER_UNUSED_KHR, .intersectionShader = VK_SHADER_UNUSED_KHR, .pShaderGroupCaptureReplayHandle = nullptr }),
			VkRayTracingShaderGroupCreateInfoKHR({.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR, .pNext = nullptr, .type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR, .generalShader = 2, .closestHitShader = VK_SHADER_UNUSED_KHR, .anyHitShader = VK_SHADER_UNUSED_KHR, .intersectionShader = VK_SHADER_UNUSED_KHR, .pShaderGroupCaptureReplayHandle = nullptr }),
			VkRayTracingShaderGroupCreateInfoKHR({.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR, .pNext = nullptr, .type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR, .generalShader = VK_SHADER_UNUSED_KHR, .closestHitShader = 3, .anyHitShader = VK_SHADER_UNUSED_KHR, .intersectionShader = VK_SHADER_UNUSED_KHR, .pShaderGroupCaptureReplayHandle = nullptr }),
		};

		constexpr std::array DSs = { VK_DYNAMIC_STATE_RAY_TRACING_PIPELINE_STACK_SIZE_KHR };
		const VkPipelineDynamicStateCreateInfo PDSCI = {
			.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
			.pNext = nullptr,
			.flags = 0,
			.dynamicStateCount = static_cast<uint32_t>(size(DSs)), .pDynamicStates = data(DSs)
		};

		const std::array RTPCIs = {
			VkRayTracingPipelineCreateInfoKHR({
				.sType = VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_KHR,
				.pNext = nullptr,
				.flags = 0,
				.stageCount = static_cast<uint32_t>(size(PSSCIs)), .pStages = data(PSSCIs),
				.groupCount = static_cast<uint32_t>(size(RTSGCIs)), .pGroups = data(RTSGCIs),
				.maxPipelineRayRecursionDepth = 2, //!< 2 にする
				.pLibraryInfo = nullptr,
				.pLibraryInterface = nullptr,
				.pDynamicState = &PDSCI,
				.layout = PLL,
				.basePipelineHandle = VK_NULL_HANDLE, .basePipelineIndex = -1
			}),
		};
#ifdef _DEBUG
		for (auto i : RTPCIs) { assert(i.maxPipelineRayRecursionDepth <= PDRTPP.maxRayRecursionDepth); }
#endif
		VERIFY_SUCCEEDED(vkCreateRayTracingPipelinesKHR(Device, VK_NULL_HANDLE, VK_NULL_HANDLE, static_cast<uint32_t>(size(RTPCIs)), data(RTPCIs), GetAllocationCallbacks(), &Pipelines.emplace_back()));
#pragma endregion

		for (auto i : SMs) { vkDestroyShaderModule(Device, i, GetAllocationCallbacks()); }
	}
	virtual void CreateShaderBindingTable() override {
		const auto& PDMP = SelectedPhysDevice.second.PDMP;
		auto& SBT = ShaderBindingTables.emplace_back(); {
			constexpr auto MissCount = 2;
			constexpr auto HitCount = 1;

			constexpr auto GenRecordSize = 0;
			constexpr auto MissRecordSize = 0;
			constexpr auto HitRecordSize = 0;

			const auto GenStride = Cmn::RoundUp(PDRTPP.shaderGroupHandleSize + GenRecordSize, PDRTPP.shaderGroupHandleAlignment);
			const auto MissStride = Cmn::RoundUp(PDRTPP.shaderGroupHandleSize + MissRecordSize, PDRTPP.shaderGroupHandleAlignment);
			const auto HitStride = Cmn::RoundUp(PDRTPP.shaderGroupHandleSize + HitRecordSize, PDRTPP.shaderGroupHandleAlignment);

			const auto GenSize = Cmn::RoundUp(GenStride, PDRTPP.shaderGroupBaseAlignment);
			const auto MissSize = Cmn::RoundUp(MissCount * MissStride, PDRTPP.shaderGroupBaseAlignment);
			const auto HitSize = Cmn::RoundUp(HitCount * HitStride, PDRTPP.shaderGroupBaseAlignment);

			SBT.StridedDeviceAddressRegions[0] = VkStridedDeviceAddressRegionKHR({ .stride = GenSize, .size = GenSize });
			SBT.StridedDeviceAddressRegions[1] = VkStridedDeviceAddressRegionKHR({ .stride = MissStride, .size = MissSize });
			SBT.StridedDeviceAddressRegions[2] = VkStridedDeviceAddressRegionKHR({ .stride = HitStride, .size = HitSize });

			SBT.Create(Device, PDMP);

			constexpr auto TotalHandleCount = 1 + MissCount + HitCount;
			std::vector<std::byte> HandleData(PDRTPP.shaderGroupHandleSize * TotalHandleCount);
			VERIFY_SUCCEEDED(vkGetRayTracingShaderGroupHandlesKHR(Device, Pipelines.back(), 0, TotalHandleCount/*ハンドル数なのかグループ数なのか？*/, size(HandleData), data(HandleData)));

			auto MapData = SBT.Map(Device); {
				auto HData = data(HandleData);
				auto Data = reinterpret_cast<std::byte*>(MapData);

				{
					const auto& Region = SBT.StridedDeviceAddressRegions[0]; {
						std::memcpy(Data, HData, PDRTPP.shaderGroupHandleSize);
						HData += PDRTPP.shaderGroupHandleSize;
						Data += Region.size;
					}
				}

				{
					const auto Count = MissCount;
					const auto& Region = SBT.StridedDeviceAddressRegions[1]; {
						auto p = Data;
						for (auto i = 0; i < Count; ++i, HData += PDRTPP.shaderGroupHandleSize, p += Region.stride) {
							std::memcpy(p, HData, PDRTPP.shaderGroupHandleSize);
						}
						Data += Region.size;
					}
				}

				{
					const auto Count = HitCount;
					const auto& Region = SBT.StridedDeviceAddressRegions[2]; {
						auto p = Data;
						for (auto i = 0; i < Count; ++i, HData += PDRTPP.shaderGroupHandleSize, p += Region.stride) {
							std::memcpy(p, HData, PDRTPP.shaderGroupHandleSize);
						}
						Data += Region.size;
					}
				}
			} SBT.Unmap(Device);
		}

		const VkTraceRaysIndirectCommandKHR TRIC = { .width = static_cast<uint32_t>(GetClientRectWidth()), .height = static_cast<uint32_t>(GetClientRectHeight()), .depth = 1 };
		IndirectBuffers.emplace_back().Create(Device, PDMP, TRIC).SubmitCopyCommand(Device, PDMP, CommandBuffers[0], GraphicsQueue, sizeof(TRIC), &TRIC);
	}
	virtual void PopulateCommandBuffer(const size_t i) override {
		if (!HasRayTracingSupport(SelectedPhysDevice.first)) { return; }

		const auto CB = CommandBuffers[i];
		const auto RT = StorageTextures[0].Image;
		constexpr VkCommandBufferBeginInfo CBBI = {
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
			.pNext = nullptr,
			.flags = 0,
			.pInheritanceInfo = nullptr
		};
		VERIFY_SUCCEEDED(vkBeginCommandBuffer(CB, &CBBI)); {
			vkCmdSetRayTracingPipelineStackSizeKHR(CB, 100);

			PopulateBeginRenderTargetCommand(CB, RT); {
				vkCmdBindPipeline(CB, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, Pipelines[0]);

				const std::array DSs = { DescriptorSets[0] };
				constexpr std::array<uint32_t, 0> DynamicOffsets = {};
				vkCmdBindDescriptorSets(CB, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, PipelineLayouts[0], 0, static_cast<uint32_t>(size(DSs)), data(DSs), static_cast<uint32_t>(size(DynamicOffsets)), data(DynamicOffsets));

				const auto& SBT = ShaderBindingTables.back();
				vkCmdTraceRaysIndirectKHR(CB, &SBT.StridedDeviceAddressRegions[0], &SBT.StridedDeviceAddressRegions[1], &SBT.StridedDeviceAddressRegions[2], &SBT.StridedDeviceAddressRegions[3], GetDeviceAddress(Device, IndirectBuffers[0].Buffer));
			} PopulateEndRenderTargetCommand(CB, RT, Swapchain.ImageAndViews[i].first, static_cast<uint32_t>(GetClientRectWidth()), static_cast<uint32_t>(GetClientRectHeight()));

		} VERIFY_SUCCEEDED(vkEndCommandBuffer(CB));
	}
};
#pragma endregion