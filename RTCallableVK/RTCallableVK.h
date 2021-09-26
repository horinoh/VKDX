#pragma once

#include "resource.h"

#pragma region Code
#include "../VKRT.h"

class RTCallableVK : public VKRT
{
private:
	using Super = VKRT;
public:
	RTCallableVK() : Super() {}
	virtual ~RTCallableVK() {}

	virtual void CreateGeometry() override {
		if (!HasRayTracingSupport(GetCurrentPhysicalDevice())) { return; }

		const auto PDMP = GetCurrentPhysicalDeviceMemoryProperties();
		const auto& CB = CommandBuffers[0];

#pragma region BLAS_GEOMETRY
		constexpr std::array Vertices = { glm::vec3({ 0.0f, 0.5f, 0.0f }), glm::vec3({ -0.5f, -0.5f, 0.0f }), glm::vec3({ 0.5f, -0.5f, 0.0f }), };
		Scoped<BufferMemory> VertBuf(Device);
		VertBuf.Create(Device, PDMP, sizeof(Vertices), VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, data(Vertices));

		constexpr std::array Indices = { uint32_t(0), uint32_t(1), uint32_t(2) };
		Scoped<BufferMemory> IndBuf(Device);
		IndBuf.Create(Device, PDMP, sizeof(Indices), VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, data(Indices));

		const std::vector ASGs_Blas = {
			VkAccelerationStructureGeometryKHR({
				.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR,
				.pNext = nullptr,
				.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_KHR,
				.geometry = VkAccelerationStructureGeometryDataKHR({
					.triangles = VkAccelerationStructureGeometryTrianglesDataKHR({
						.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR,
						.pNext = nullptr,
						.vertexFormat = VK_FORMAT_R32G32B32_SFLOAT,
						.vertexData = VkDeviceOrHostAddressConstKHR({.deviceAddress = GetDeviceAddress(Device, VertBuf.Buffer)}), .vertexStride = sizeof(Vertices[0]), .maxVertex = static_cast<uint32_t>(size(Vertices)),
						.indexType = VK_INDEX_TYPE_UINT32,
						.indexData = VkDeviceOrHostAddressConstKHR({.deviceAddress = GetDeviceAddress(Device, IndBuf.Buffer)}),
						.transformData = VkDeviceOrHostAddressConstKHR({.deviceAddress = 0}),
					}),
				}),
				.flags = VK_GEOMETRY_OPAQUE_BIT_KHR
			}),
		};
#pragma endregion

#pragma region BLAS_AND_SCRATCH
		VkAccelerationStructureBuildGeometryInfoKHR ASBGI_Blas = {
			.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR,
			.pNext = nullptr,
			.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR,
			.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR,
			.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR,
			.srcAccelerationStructure = VK_NULL_HANDLE, .dstAccelerationStructure = VK_NULL_HANDLE,
			.geometryCount = static_cast<uint32_t>(size(ASGs_Blas)),.pGeometries = data(ASGs_Blas), .ppGeometries = nullptr, //!< [GLSL] gl_GeometryIndexEXT ([HLSL] GeometryIndex() 相当)
			.scratchData = VkDeviceOrHostAddressKHR({.deviceAddress = 0})
		};
		constexpr auto ASBRI_Blas = VkAccelerationStructureBuildRangeInfoKHR({ .primitiveCount = static_cast<uint32_t>(size(Indices) / 3), .primitiveOffset = 0, .firstVertex = 0, .transformOffset = 0 });

		Scoped<ScratchBuffer> Scratch_Blas(Device);
		{
			VkAccelerationStructureBuildSizesInfoKHR ASBSI = { .sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR, };
			const std::array MaxPrimitiveCounts = { ASBRI_Blas.primitiveCount };
			vkGetAccelerationStructureBuildSizesKHR(Device, VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR, &ASBGI_Blas, data(MaxPrimitiveCounts), &ASBSI);

			BLASs.emplace_back().Create(Device, PDMP, ASBSI.accelerationStructureSize);
			Scratch_Blas.Create(Device, PDMP, ASBSI.buildScratchSize);

			ASBGI_Blas.dstAccelerationStructure = BLASs.back().AccelerationStructure;
			ASBGI_Blas.scratchData = VkDeviceOrHostAddressKHR({ .deviceAddress = VK::GetDeviceAddress(Device, Scratch_Blas.Buffer) });
		}
#pragma endregion

#pragma region TLAS_GEOMETRY
		//!< instanceCustomIndex					: 0==市松模様, 1==縦線, 2==横線 (ここでは CallableShader の出し分けに使用)
		//!< instanceShaderBindingTableRecordOffset	: 0==赤, 1==緑 (HitShader の出し分けに使用)
		const std::array ASIs = {
			#pragma region INSTANCES
			VkAccelerationStructureInstanceKHR({
				.transform = VkTransformMatrixKHR({1.0f, 0.0f, 0.0f, -0.5f,
													0.0f, 1.0f, 0.0f, 0.0f,
													0.0f, 0.0f, 1.0f, 0.0f}),
				.instanceCustomIndex = 0, //!< [GLSL] gl_InstanceCustomIndexEXT ([HLSL] InstanceID()相当) (ここでは CallableShader の出し分けに使用)
				.mask = 0xff,
				.instanceShaderBindingTableRecordOffset = 0, //!< ヒットシェーダインデックス (HitShader の出し分けに使用)
				.flags = VK_GEOMETRY_INSTANCE_TRIANGLE_FRONT_COUNTERCLOCKWISE_BIT_KHR,
				.accelerationStructureReference = GetDeviceAddress(Device, BLASs.back().AccelerationStructure)
			}),
			VkAccelerationStructureInstanceKHR({
				.transform = VkTransformMatrixKHR({1.0f, 0.0f, 0.0f, 0.5f,
													0.0f, 1.0f, 0.0f, 0.0f,
													0.0f, 0.0f, 1.0f, 0.0f}),
				.instanceCustomIndex = 1,
				.mask = 0xff,
				.instanceShaderBindingTableRecordOffset = 1,
				.flags = VK_GEOMETRY_INSTANCE_TRIANGLE_FRONT_COUNTERCLOCKWISE_BIT_KHR,
				.accelerationStructureReference = GetDeviceAddress(Device, BLASs.back().Buffer)
			}),
			#pragma endregion
		};
		Scoped<BufferMemory> InstBuf(Device);
		InstBuf.Create(Device, PDMP, sizeof(ASIs), VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, data(ASIs));

		const auto ASG_Tlas = VkAccelerationStructureGeometryKHR({
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
			});

#pragma region TLAS_AND_SCRATCH
		VkAccelerationStructureBuildGeometryInfoKHR ASBGI_Tlas = {
			.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR,
			.pNext = nullptr,
			.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR,
			.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR,
			.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR,
			.srcAccelerationStructure = VK_NULL_HANDLE, .dstAccelerationStructure = VK_NULL_HANDLE,
			.geometryCount = 1,.pGeometries = &ASG_Tlas, .ppGeometries = nullptr, //!< [GLSL] gl_GeometryIndexEXT ([HLSL] GeometryIndex() 相当)
			.scratchData = VkDeviceOrHostAddressKHR({.deviceAddress = 0})
		};
		constexpr auto ASBRI_Tlas = VkAccelerationStructureBuildRangeInfoKHR({ .primitiveCount = static_cast<uint32_t>(size(ASIs)), .primitiveOffset = 0, .firstVertex = 0, .transformOffset = 0 });

		Scoped<ScratchBuffer> Scratch_Tlas(Device);
		{
			VkAccelerationStructureBuildSizesInfoKHR ASBSI = { .sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR, };
			const std::array MaxPrimitiveCounts = { ASBRI_Tlas.primitiveCount };
			vkGetAccelerationStructureBuildSizesKHR(Device, VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR, &ASBGI_Tlas, data(MaxPrimitiveCounts), &ASBSI);

			TLASs.emplace_back().Create(Device, PDMP, ASBSI.accelerationStructureSize);
			Scratch_Tlas.Create(Device, PDMP, ASBSI.buildScratchSize);

			ASBGI_Tlas.dstAccelerationStructure = TLASs.back().AccelerationStructure;
			ASBGI_Tlas.scratchData = VkDeviceOrHostAddressKHR({ .deviceAddress = VK::GetDeviceAddress(Device, Scratch_Tlas.Buffer) });
		}
#pragma endregion

		constexpr VkCommandBufferBeginInfo CBBI = { .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO, .pNext = nullptr, .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, .pInheritanceInfo = nullptr };
		VERIFY_SUCCEEDED(vkBeginCommandBuffer(CB, &CBBI)); {
			BLASs.back().PopulateBuildCommand(ASBGI_Blas, ASBRI_Blas, CB);
			BLASs.back().PopulateBarrierCommand(CB);
			TLASs.back().PopulateBuildCommand(ASBGI_Tlas, ASBRI_Tlas, CB);
		} VERIFY_SUCCEEDED(vkEndCommandBuffer(CB));
		SubmitAndWait(GraphicsQueue, CB);
	}
	virtual void CreatePipeline() override {
		if (!HasRayTracingSupport(GetCurrentPhysicalDevice())) { return; }
#pragma region PIPELINE
		const auto PLL = PipelineLayouts.back();

		const auto ShaderPath = GetBasePath();
		const std::array SMs = {
			VK::CreateShaderModule(data(ShaderPath + TEXT(".rgen.spv"))),
			VK::CreateShaderModule(data(ShaderPath + TEXT(".rmiss.spv"))),
#pragma region HIT
			VK::CreateShaderModule(data(ShaderPath + TEXT(".rchit.spv"))),
			VK::CreateShaderModule(data(ShaderPath + TEXT("_1.rchit.spv"))),
#pragma endregion
#pragma region CALLABLE
			VK::CreateShaderModule(data(ShaderPath + TEXT(".rcall.spv"))),
			VK::CreateShaderModule(data(ShaderPath + TEXT("_1.rcall.spv"))),
			VK::CreateShaderModule(data(ShaderPath + TEXT("_2.rcall.spv"))),
#pragma endregion
		};
		const std::array PSSCIs = {
			VkPipelineShaderStageCreateInfo({.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, .pNext = nullptr, .flags = 0, .stage = VK_SHADER_STAGE_RAYGEN_BIT_KHR, .module = SMs[0], .pName = "main", .pSpecializationInfo = nullptr }),
			VkPipelineShaderStageCreateInfo({.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, .pNext = nullptr, .flags = 0, .stage = VK_SHADER_STAGE_MISS_BIT_KHR, .module = SMs[1], .pName = "main", .pSpecializationInfo = nullptr }),
#pragma region HIT
			VkPipelineShaderStageCreateInfo({.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, .pNext = nullptr, .flags = 0, .stage = VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR, .module = SMs[2], .pName = "main", .pSpecializationInfo = nullptr }),
			VkPipelineShaderStageCreateInfo({.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, .pNext = nullptr, .flags = 0, .stage = VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR, .module = SMs[3], .pName = "main", .pSpecializationInfo = nullptr }),
#pragma endregion
#pragma region CALLABLE
			VkPipelineShaderStageCreateInfo({.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, .pNext = nullptr, .flags = 0, .stage = VK_SHADER_STAGE_CALLABLE_BIT_KHR, .module = SMs[4], .pName = "main", .pSpecializationInfo = nullptr }),
			VkPipelineShaderStageCreateInfo({.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, .pNext = nullptr, .flags = 0, .stage = VK_SHADER_STAGE_CALLABLE_BIT_KHR, .module = SMs[5], .pName = "main", .pSpecializationInfo = nullptr }),
			VkPipelineShaderStageCreateInfo({.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, .pNext = nullptr, .flags = 0, .stage = VK_SHADER_STAGE_CALLABLE_BIT_KHR, .module = SMs[6], .pName = "main", .pSpecializationInfo = nullptr }),
#pragma endregion
		};
		const std::array RTSGCIs = {
			VkRayTracingShaderGroupCreateInfoKHR({.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR, .pNext = nullptr, .type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR, .generalShader = 0, .closestHitShader = VK_SHADER_UNUSED_KHR, .anyHitShader = VK_SHADER_UNUSED_KHR, .intersectionShader = VK_SHADER_UNUSED_KHR, .pShaderGroupCaptureReplayHandle = nullptr }),
			VkRayTracingShaderGroupCreateInfoKHR({.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR, .pNext = nullptr, .type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR, .generalShader = 1, .closestHitShader = VK_SHADER_UNUSED_KHR, .anyHitShader = VK_SHADER_UNUSED_KHR, .intersectionShader = VK_SHADER_UNUSED_KHR, .pShaderGroupCaptureReplayHandle = nullptr }),
#pragma region HIT
			VkRayTracingShaderGroupCreateInfoKHR({.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR, .pNext = nullptr, .type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR, .generalShader = VK_SHADER_UNUSED_KHR, .closestHitShader = 2, .anyHitShader = VK_SHADER_UNUSED_KHR, .intersectionShader = VK_SHADER_UNUSED_KHR, .pShaderGroupCaptureReplayHandle = nullptr }),
			VkRayTracingShaderGroupCreateInfoKHR({.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR, .pNext = nullptr, .type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR, .generalShader = VK_SHADER_UNUSED_KHR, .closestHitShader = 3, .anyHitShader = VK_SHADER_UNUSED_KHR, .intersectionShader = VK_SHADER_UNUSED_KHR, .pShaderGroupCaptureReplayHandle = nullptr }),
#pragma endregion
#pragma region CALLABLE
			VkRayTracingShaderGroupCreateInfoKHR({.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR, .pNext = nullptr, .type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR, .generalShader = 4, .closestHitShader = VK_SHADER_UNUSED_KHR, .anyHitShader = VK_SHADER_UNUSED_KHR, .intersectionShader = VK_SHADER_UNUSED_KHR, .pShaderGroupCaptureReplayHandle = nullptr }),
			VkRayTracingShaderGroupCreateInfoKHR({.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR, .pNext = nullptr, .type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR, .generalShader = 5, .closestHitShader = VK_SHADER_UNUSED_KHR, .anyHitShader = VK_SHADER_UNUSED_KHR, .intersectionShader = VK_SHADER_UNUSED_KHR, .pShaderGroupCaptureReplayHandle = nullptr }),
			VkRayTracingShaderGroupCreateInfoKHR({.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR, .pNext = nullptr, .type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR, .generalShader = 6, .closestHitShader = VK_SHADER_UNUSED_KHR, .anyHitShader = VK_SHADER_UNUSED_KHR, .intersectionShader = VK_SHADER_UNUSED_KHR, .pShaderGroupCaptureReplayHandle = nullptr }),
#pragma endregion
		};

		constexpr std::array DSs = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
		constexpr VkPipelineDynamicStateCreateInfo PDSCI = {
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
				.maxPipelineRayRecursionDepth = 1,
				.pLibraryInfo = nullptr,
				.pLibraryInterface = nullptr,
				.pDynamicState = &PDSCI,
				.layout = PLL,
				.basePipelineHandle = VK_NULL_HANDLE, .basePipelineIndex = -1
			}),
		};
		VERIFY_SUCCEEDED(vkCreateRayTracingPipelinesKHR(Device, VK_NULL_HANDLE, VK_NULL_HANDLE, static_cast<uint32_t>(size(RTPCIs)), data(RTPCIs), GetAllocationCallbacks(), &Pipelines.emplace_back()));
#pragma endregion

		for (auto i : SMs) { vkDestroyShaderModule(Device, i, GetAllocationCallbacks()); }
	}
	virtual void CreateShaderBindingTable() override {
		VkPhysicalDeviceRayTracingPipelinePropertiesKHR PDRTPP = { .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_PROPERTIES_KHR, .pNext = nullptr };
		VkPhysicalDeviceProperties2 PDP2 = { .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2, .pNext = &PDRTPP, };
		vkGetPhysicalDeviceProperties2(GetCurrentPhysicalDevice(), &PDP2);

		const auto RgenStride = Cmn::RoundUp(PDRTPP.shaderGroupHandleSize + 0, PDRTPP.shaderGroupHandleAlignment);
		const auto RgenSize = 1 * RgenStride;

		const auto MissStride = Cmn::RoundUp(PDRTPP.shaderGroupHandleSize + 0, PDRTPP.shaderGroupHandleAlignment);
		const auto MissSize = 1 * MissStride;

#pragma region HIT
		const auto RchitStride = Cmn::RoundUp(PDRTPP.shaderGroupHandleSize + 0, PDRTPP.shaderGroupHandleAlignment);
		const auto RchitSize = 2 * RchitStride;
#pragma endregion

#pragma region CALLABLE
		const auto RcallStride = Cmn::RoundUp(PDRTPP.shaderGroupHandleSize + 0, PDRTPP.shaderGroupHandleAlignment);
		const auto RcallSize = 3 * RcallStride;
#pragma endregion

		std::vector<std::byte> HandleData(RgenSize + MissSize + RchitSize + RcallSize);
		VERIFY_SUCCEEDED(vkGetRayTracingShaderGroupHandlesKHR(Device, Pipelines.back(), 0, 1 + 1 + 2 + 3, size(HandleData), data(HandleData)));

		const auto PDMP = GetCurrentPhysicalDeviceMemoryProperties();
		ShaderBindingTables.emplace_back().Create(Device, PDMP, RgenSize, RgenStride); {
			auto Data = ShaderBindingTables.back().Map(Device); {
				std::memcpy(Data, data(HandleData), PDRTPP.shaderGroupHandleSize);
			} ShaderBindingTables.back().Unmap(Device);
		}
		ShaderBindingTables.emplace_back().Create(Device, PDMP, MissSize, MissStride); {
			auto Data = ShaderBindingTables.back().Map(Device); {
				std::memcpy(Data, data(HandleData) + RgenSize, PDRTPP.shaderGroupHandleSize);
			} ShaderBindingTables.back().Unmap(Device);
		}
#pragma region HIT
		ShaderBindingTables.emplace_back().Create(Device, PDMP, RchitSize, RchitStride); {
			auto Data = ShaderBindingTables.back().Map(Device); {
				std::memcpy(Data, data(HandleData) + RgenSize + MissSize, PDRTPP.shaderGroupHandleSize * 2);
			} ShaderBindingTables.back().Unmap(Device);
		}
#pragma endregion
#pragma region CALLABLE
		ShaderBindingTables.emplace_back().Create(Device, PDMP, RcallSize, RcallStride); {
			auto Data = ShaderBindingTables.back().Map(Device); {
				std::memcpy(Data, data(HandleData) + RgenSize + MissSize + RchitSize, PDRTPP.shaderGroupHandleSize * 3);
			} ShaderBindingTables.back().Unmap(Device);
		}
#pragma endregion

		const VkTraceRaysIndirectCommandKHR TRIC = { .width = static_cast<uint32_t>(GetClientRectWidth()), .height = static_cast<uint32_t>(GetClientRectHeight()), .depth = 1 };
		IndirectBuffers.emplace_back().Create(Device, PDMP, TRIC).SubmitCopyCommand(Device, PDMP, CommandBuffers[0], GraphicsQueue, sizeof(TRIC), &TRIC);
	}
	virtual void PopulateCommandBuffer(const size_t i) override {
		if (!HasRayTracingSupport(GetCurrentPhysicalDevice())) { return; }

		const auto CB = CommandBuffers[i];
		constexpr VkCommandBufferBeginInfo CBBI = {
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
			.pNext = nullptr,
			.flags = 0,
			.pInheritanceInfo = nullptr
		};
		VERIFY_SUCCEEDED(vkBeginCommandBuffer(CB, &CBBI)); {
			PopulateBeginRenderTargetCommand(i); {
				vkCmdBindPipeline(CB, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, Pipelines[0]);

				const std::array DSs = { DescriptorSets[0] };
				constexpr std::array<uint32_t, 0> DynamicOffsets = {};
				vkCmdBindDescriptorSets(CB, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, PipelineLayouts[0], 0, static_cast<uint32_t>(size(DSs)), data(DSs), static_cast<uint32_t>(size(DynamicOffsets)), data(DynamicOffsets));

				vkCmdTraceRaysIndirectKHR(CB, &ShaderBindingTables[0].Region, &ShaderBindingTables[1].Region, &ShaderBindingTables[2].Region, &ShaderBindingTables[3].Region, GetDeviceAddress(Device, IndirectBuffers[0].Buffer));

			} PopulateEndRenderTargetCommand(i);
		} VERIFY_SUCCEEDED(vkEndCommandBuffer(CB));
	}
};
#pragma endregion

