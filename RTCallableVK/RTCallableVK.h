#pragma once

#include "resource.h"

#pragma region Code
#include "../VKExt.h"

class RTCallableVK : public VKExt
{
private:
	using Super = VKExt;
public:
	RTCallableVK() : Super() {}
	virtual ~RTCallableVK() {}

	//!< #TIPS VK�C���X�^���X�쐬���� "VK_LAYER_RENDERDOC_Capture" ���g�p����ƁA���b�V���V�F�[�_�[�⃌�C�g���[�V���O�Ɠ����Ɏg�p�����ꍇ�AvkCreateDevice() �ŃR�P��悤�ɂȂ�̂Œ��� (If we use "VK_LAYER_RENDERDOC_Capture" with mesh shader or raytracing, vkCreateDevice() failed)

#pragma region RAYTRACING
	virtual void CreateDevice(HWND hWnd, HINSTANCE hInstance, [[maybe_unused]] void* pNext, [[maybe_unused]] const std::vector<const char*>& AddExtensions) override {
		if (HasMeshShaderSupport(GetCurrentPhysicalDevice())) {
#ifdef _DEBUG
			uint32_t APIVersion; VERIFY_SUCCEEDED(vkEnumerateInstanceVersion(&APIVersion)); assert(APIVersion >= VK_MAKE_VERSION(1, 2, 162) && "RayTracing require 1.2.162 or later");
#endif
			VkPhysicalDeviceBufferDeviceAddressFeatures PDBDAF = { .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES, .pNext = nullptr, .bufferDeviceAddress = VK_TRUE };
			VkPhysicalDeviceRayTracingPipelineFeaturesKHR PDRTPF = { .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR, .pNext = &PDBDAF, .rayTracingPipeline = VK_TRUE, .rayTracingPipelineTraceRaysIndirect = VK_TRUE };
			VkPhysicalDeviceAccelerationStructureFeaturesKHR PDASF = { .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR, .pNext = &PDRTPF, .accelerationStructure = VK_TRUE };
			Super::CreateDevice(hWnd, hInstance, &PDASF, {
				VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME,
				VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME,
				//!< VK_KHR_acceleration_structure
				VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME,
				VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME,
				VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME,
				//!< VK_KHR_ray_tracing_pipeline
				VK_KHR_SPIRV_1_4_EXTENSION_NAME,
				//!< VK_KHR_spirv_1_4
				VK_KHR_SHADER_FLOAT_CONTROLS_EXTENSION_NAME,
				});
		}
		else {
			Super::CreateDevice(hWnd, hInstance);
		}
	}
	virtual void CreateSwapchain() override { VK::CreateSwapchain(GetCurrentPhysicalDevice(), Surface, GetClientRectWidth(), GetClientRectHeight(), VK_IMAGE_USAGE_TRANSFER_DST_BIT); }
	virtual void CreateGeometry() override {
		if (!HasRayTracingSupport(GetCurrentPhysicalDevice())) { return; }

		const auto PDMP = GetCurrentPhysicalDeviceMemoryProperties();
		const auto& CB = CommandBuffers[0];

#pragma region BLAS_GEOMETRY
		constexpr std::array Vertices = { glm::vec3({ 0.0f, 0.5f, 0.0f }), glm::vec3({ -0.5f, -0.5f, 0.0f }), glm::vec3({ 0.5f, -0.5f, 0.0f }), };
		Scoped<BufferMemory> VertBuf(Device);
		VertBuf.Create(Device, PDMP, VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, sizeof(Vertices), VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, data(Vertices));

		constexpr std::array Indices = { uint32_t(0), uint32_t(1), uint32_t(2) };
		Scoped<BufferMemory> IndBuf(Device);
		IndBuf.Create(Device, PDMP, VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, sizeof(Indices), VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, data(Indices));

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
			.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR, //!< �{�g�����x�����w��
			.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR,
			.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR,
			.srcAccelerationStructure = VK_NULL_HANDLE, .dstAccelerationStructure = VK_NULL_HANDLE,
			.geometryCount = static_cast<uint32_t>(size(ASGs_Blas)),.pGeometries = data(ASGs_Blas), .ppGeometries = nullptr, //!< [GLSL] gl_GeometryIndexEXT ([HLSL] GeometryIndex() ����)
			.scratchData = VkDeviceOrHostAddressKHR({.deviceAddress = 0})
		};
		constexpr auto ASBRI_Blas = VkAccelerationStructureBuildRangeInfoKHR({ .primitiveCount = 1, .primitiveOffset = 0, .firstVertex = 0, .transformOffset = 0 });

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
		//!< instanceCustomIndex					: 0==�s���͗l, 1==�c��, 2==����
		//!< instanceShaderBindingTableRecordOffset	: 0==��, 1==��
		const std::array ASIs = {
			#pragma region INSTANCES
			VkAccelerationStructureInstanceKHR({
				.transform = VkTransformMatrixKHR({1.0f, 0.0f, 0.0f, -0.5f,
													0.0f, 1.0f, 0.0f, 0.0f,
													0.0f, 0.0f, 1.0f, 0.0f}),
				.instanceCustomIndex = 0, //!< [GLSL] gl_InstanceCustomIndexEXT ([HLSL] InstanceID()����)
				.mask = 0xff,
				.instanceShaderBindingTableRecordOffset = 0, //!< �q�b�g�V�F�[�_�C���f�b�N�X
				.flags = VK_GEOMETRY_INSTANCE_TRIANGLE_FRONT_COUNTERCLOCKWISE_BIT_KHR,
				.accelerationStructureReference = GetDeviceAddress(Device, BLASs.back().Buffer)
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
		InstBuf.Create(Device, PDMP, VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, sizeof(ASIs), VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, data(ASIs));

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
			.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR, //!< �g�b�v���x�����w��
			.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR,
			.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR,
			.srcAccelerationStructure = VK_NULL_HANDLE, .dstAccelerationStructure = VK_NULL_HANDLE,
			.geometryCount = 1,.pGeometries = &ASG_Tlas, .ppGeometries = nullptr, //!< [GLSL] gl_GeometryIndexEXT ([HLSL] GeometryIndex() ����)
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

		const VkTraceRaysIndirectCommandKHR TRIC = { .width = static_cast<uint32_t>(GetClientRectWidth()), .height = static_cast<uint32_t>(GetClientRectHeight()), .depth = 1 };
		IndirectBuffers.emplace_back().Create(Device, PDMP, TRIC);
		VK::Scoped<StagingBuffer> Staging_Indirect(Device);
		Staging_Indirect.Create(Device, PDMP, sizeof(TRIC), &TRIC);

		constexpr VkCommandBufferBeginInfo CBBI = { .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO, .pNext = nullptr, .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, .pInheritanceInfo = nullptr };
		VERIFY_SUCCEEDED(vkBeginCommandBuffer(CB, &CBBI)); {
			BLASs.back().PopulateBuildCommand(ASBGI_Blas, ASBRI_Blas, CB);
			BLASs.back().PopulateBarrierCommand(CB);
			TLASs.back().PopulateBuildCommand(ASBGI_Tlas, ASBRI_Tlas, CB);
			IndirectBuffers.back().PopulateCopyCommand(CB, sizeof(TRIC), Staging_Indirect.Buffer);
		} VERIFY_SUCCEEDED(vkEndCommandBuffer(CB));
		SubmitAndWait(GraphicsQueue, CB);
	}
	virtual void CreateTexture() override {
		if (!HasRayTracingSupport(GetCurrentPhysicalDevice())) { return; }
		const auto PDMP = GetCurrentPhysicalDeviceMemoryProperties();

		StorageTextures.emplace_back().Create(Device, PDMP, ColorFormat, VkExtent3D({ .width = static_cast<uint32_t>(GetClientRectWidth()), .height = static_cast<uint32_t>(GetClientRectHeight()), .depth = 1 }))
			.SubmitSetLayoutCommand(CommandBuffers[0], GraphicsQueue, VK_IMAGE_LAYOUT_GENERAL);
	}
	virtual void CreatePipelineLayout() override {
		if (!HasRayTracingSupport(GetCurrentPhysicalDevice())) { return; }
		CreateDescriptorSetLayout(DescriptorSetLayouts.emplace_back(), 0, {
			//!< TLAS
			VkDescriptorSetLayoutBinding({.binding = 0, .descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, .descriptorCount = 1, .stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR, .pImmutableSamplers = nullptr }),
			//!< Storage Image
			VkDescriptorSetLayoutBinding({.binding = 1, .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, .descriptorCount = 1, .stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR, .pImmutableSamplers = nullptr }),
		});
		VK::CreatePipelineLayout(PipelineLayouts.emplace_back(), DescriptorSetLayouts, {});
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

		CreateShaderBindingTable();

		for (auto i : SMs) { vkDestroyShaderModule(Device, i, GetAllocationCallbacks()); }
	}
	virtual void CreateShaderBindingTable() override {
		VkPhysicalDeviceRayTracingPipelinePropertiesKHR PDRTPP = { .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_PROPERTIES_KHR, .pNext = nullptr };
		VkPhysicalDeviceProperties2 PDP2 = { .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2, .pNext = &PDRTPP, };
		vkGetPhysicalDeviceProperties2(GetCurrentPhysicalDevice(), &PDP2);

		const auto RgenRecordSize = Cmn::RoundUp(PDRTPP.shaderGroupHandleSize + 0, PDRTPP.shaderGroupHandleAlignment);
		const auto RgenTableSize = 1 * RgenRecordSize;

		const auto MissRecordSize = Cmn::RoundUp(PDRTPP.shaderGroupHandleSize + 0, PDRTPP.shaderGroupHandleAlignment);
		const auto MissTableSize = 1 * MissRecordSize;

#pragma region HIT
		constexpr auto HitCount = 2;
		const auto RchitRecordSize = Cmn::RoundUp(PDRTPP.shaderGroupHandleSize + 0, PDRTPP.shaderGroupHandleAlignment);
		const auto RchitTableSize = HitCount * RchitRecordSize;
#pragma endregion

#pragma region CALLABLE
		constexpr auto CallableCount = 3;
		const auto RcallRecordSize = Cmn::RoundUp(PDRTPP.shaderGroupHandleSize + 0, PDRTPP.shaderGroupHandleAlignment);
		const auto RcallTableSize = CallableCount * RcallRecordSize;
#pragma endregion

		std::vector<std::byte> HandleData(RgenTableSize + MissTableSize + RchitTableSize + RcallTableSize);
		VERIFY_SUCCEEDED(vkGetRayTracingShaderGroupHandlesKHR(Device, Pipelines.back(), 0, 2 + HitCount + CallableCount, size(HandleData), data(HandleData)));

		const auto PDMP = GetCurrentPhysicalDeviceMemoryProperties();
		ShaderBindingTables.emplace_back().Create(Device, PDMP, RgenTableSize); {
			auto Data = ShaderBindingTables.back().Map(Device); {
				std::memcpy(Data, data(HandleData), PDRTPP.shaderGroupHandleSize);
			} ShaderBindingTables.back().Unmap(Device);
		}
		ShaderBindingTables.emplace_back().Create(Device, PDMP, MissTableSize); {
			auto Data = ShaderBindingTables.back().Map(Device); {
				std::memcpy(Data, data(HandleData) + RgenTableSize, PDRTPP.shaderGroupHandleSize);
			} ShaderBindingTables.back().Unmap(Device);
		}
#pragma region HIT
		ShaderBindingTables.emplace_back().Create(Device, PDMP, RchitTableSize); {
			auto Data = ShaderBindingTables.back().Map(Device); {
				std::memcpy(Data, data(HandleData) + RgenTableSize + MissTableSize, PDRTPP.shaderGroupHandleSize * HitCount);
			} ShaderBindingTables.back().Unmap(Device);
		}
#pragma endregion
#pragma region CALLABLE
		ShaderBindingTables.emplace_back().Create(Device, PDMP, RcallTableSize); {
			auto Data = ShaderBindingTables.back().Map(Device); {
				std::memcpy(Data, data(HandleData) + RgenTableSize + MissTableSize + RchitTableSize, PDRTPP.shaderGroupHandleSize * CallableCount);
			} ShaderBindingTables.back().Unmap(Device);
		}
#pragma endregion
	}

	virtual void CreateDescriptorSet() override {
		VKExt::CreateDescriptorPool(DescriptorPools.emplace_back(), VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT, {
			VkDescriptorPoolSize({.type = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, .descriptorCount = 1 }), //!< TLAS
			VkDescriptorPoolSize({.type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, .descriptorCount = 1 }) //!< StorageImage
		});
		const std::array DSLs = { DescriptorSetLayouts[0] };
		const VkDescriptorSetAllocateInfo DSAI = {
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
			.pNext = nullptr,
			.descriptorPool = DescriptorPools[0],
			.descriptorSetCount = static_cast<uint32_t>(size(DSLs)), .pSetLayouts = data(DSLs)
		};
		VERIFY_SUCCEEDED(vkAllocateDescriptorSets(Device, &DSAI, &DescriptorSets.emplace_back()));
	}
	virtual void UpdateDescriptorSet() override {
		const std::array ASs = { TLASs[0].AccelerationStructure };
		const auto WDSAS = VkWriteDescriptorSetAccelerationStructureKHR({ .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_KHR, .pNext = nullptr, .accelerationStructureCount = static_cast<uint32_t>(size(ASs)), .pAccelerationStructures = data(ASs) });
		const auto DII = VkDescriptorImageInfo({ .sampler = VK_NULL_HANDLE, .imageView = StorageTextures[0].View, .imageLayout = VK_IMAGE_LAYOUT_GENERAL });
		const std::array WDSs = {
			VkWriteDescriptorSet({
				.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
				.pNext = &WDSAS, //!< pNext �� VkWriteDescriptorSetAccelerationStructureKHR ���w�肷��
				.dstSet = DescriptorSets[0],
				.dstBinding = 0, .dstArrayElement = 0,
				.descriptorCount = 1, .descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR,
				.pImageInfo = nullptr, .pBufferInfo = nullptr, .pTexelBufferView = nullptr
			}),
			VkWriteDescriptorSet({
				.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
				.pNext = nullptr,
				.dstSet = DescriptorSets[0],
				.dstBinding = 1, .dstArrayElement = 0,
				.descriptorCount = 1, .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
				.pImageInfo = &DII, .pBufferInfo = nullptr, .pTexelBufferView = nullptr
			})
		};
		constexpr std::array<VkCopyDescriptorSet, 0> CDSs = {};
		vkUpdateDescriptorSets(Device, static_cast<uint32_t>(size(WDSs)), data(WDSs), static_cast<uint32_t>(size(CDSs)), data(CDSs));
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
			vkCmdBindPipeline(CB, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, Pipelines[0]);

			const std::array DSs = { DescriptorSets[0] };
			constexpr std::array<uint32_t, 0> DynamicOffsets = {};
			vkCmdBindDescriptorSets(CB, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, PipelineLayouts[0], 0, static_cast<uint32_t>(size(DSs)), data(DSs), static_cast<uint32_t>(size(DynamicOffsets)), data(DynamicOffsets));

			VkPhysicalDeviceRayTracingPipelinePropertiesKHR PDRTPP = { .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_PROPERTIES_KHR, .pNext = nullptr };
			VkPhysicalDeviceProperties2 PDP2 = { .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2, .pNext = &PDRTPP, };
			vkGetPhysicalDeviceProperties2(GetCurrentPhysicalDevice(), &PDP2);

			const auto AlignedSize = RoundUp(PDRTPP.shaderGroupHandleSize, PDRTPP.shaderGroupHandleAlignment);
			const auto RayGen = VkStridedDeviceAddressRegionKHR({
				.deviceAddress = GetDeviceAddress(Device, ShaderBindingTables[0].Buffer),
				.stride = AlignedSize,
				.size = AlignedSize * 1
				});
			const auto Miss = VkStridedDeviceAddressRegionKHR({
				.deviceAddress = GetDeviceAddress(Device, ShaderBindingTables[1].Buffer),
				.stride = AlignedSize,
				.size = AlignedSize * 1
				});
#pragma region HIT
			constexpr auto HitCount = 2;
			const auto Hit = VkStridedDeviceAddressRegionKHR({
				.deviceAddress = GetDeviceAddress(Device, ShaderBindingTables[2].Buffer),
				.stride = AlignedSize,
				.size = AlignedSize * HitCount
				});
#pragma endregion
#pragma region CALLABLE
			constexpr auto CallableCount = 3;
			const auto Callable = VkStridedDeviceAddressRegionKHR({
				.deviceAddress = GetDeviceAddress(Device, ShaderBindingTables[3].Buffer),
				.stride = AlignedSize,
				.size = AlignedSize * CallableCount
				});
#pragma endregion

			vkCmdTraceRaysIndirectKHR(CB, &RayGen, &Miss, &Hit, &Callable, GetDeviceAddress(Device, IndirectBuffers[0].Buffer));

			constexpr auto ISR = VkImageSubresourceRange({ .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .baseMipLevel = 0, .levelCount = 1, .baseArrayLayer = 0, .layerCount = 1 });
			{
				const std::array IMBs = {
					VkImageMemoryBarrier({
						.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
						.pNext = nullptr,
						.srcAccessMask = 0, .dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
						.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED, .newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
						.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED, .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
						.image = SwapchainImages[i],
						.subresourceRange = ISR
					}),
					VkImageMemoryBarrier({
						.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
						.pNext = nullptr,
						.srcAccessMask = 0, .dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT,
						.oldLayout = VK_IMAGE_LAYOUT_GENERAL, .newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
						.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED, .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
						.image = StorageTextures[0].Image,
						.subresourceRange = ISR
					})
				};
				vkCmdPipelineBarrier(CB, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, nullptr, 0, nullptr, static_cast<uint32_t>(size(IMBs)), data(IMBs));
			}

			const std::array ICs = {
				VkImageCopy({
					.srcSubresource = VkImageSubresourceLayers({.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .mipLevel = 0, .baseArrayLayer = 0, .layerCount = 1 }),
					.srcOffset = VkOffset3D({.x = 0, .y = 0, .z = 0}),
					.dstSubresource = VkImageSubresourceLayers({.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .mipLevel = 0, .baseArrayLayer = 0, .layerCount = 1 }),
					.dstOffset = VkOffset3D({.x = 0, .y = 0, .z = 0}),
					.extent = VkExtent3D({.width = static_cast<uint32_t>(GetClientRectWidth()), .height = static_cast<uint32_t>(GetClientRectHeight()), .depth = 1 }),
				}),
			};
			vkCmdCopyImage(CB, StorageTextures[0].Image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, SwapchainImages[i], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, static_cast<uint32_t>(size(ICs)), data(ICs));

			{
				const std::array IMBs = {
					VkImageMemoryBarrier({
						.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
						.pNext = nullptr,
						.srcAccessMask = 0, .dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
						.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, .newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
						.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED, .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
						.image = SwapchainImages[i],
						.subresourceRange = ISR
					}),
					VkImageMemoryBarrier({
						.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
						.pNext = nullptr,
						.srcAccessMask = 0, .dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT,
						.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, .newLayout = VK_IMAGE_LAYOUT_GENERAL,
						.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED, .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
						.image = StorageTextures[0].Image,
						.subresourceRange = ISR
					})
				};
				vkCmdPipelineBarrier(CB, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, nullptr, 0, nullptr, static_cast<uint32_t>(size(IMBs)), data(IMBs));
			}

		} VERIFY_SUCCEEDED(vkEndCommandBuffer(CB));
	}
#pragma endregion
};
#pragma endregion
