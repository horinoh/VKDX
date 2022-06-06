#pragma once

#include "resource.h"

#pragma region Code
#include "../VKRT.h"

class RTTriangleVK : public VKRT
{
private:
	using Super = VKRT;
public:
	RTTriangleVK() : Super() {}
	virtual ~RTTriangleVK() {}

	virtual void CreateGeometry() override {
		if (!HasRayTracingSupport(GetCurrentPhysicalDevice())) { return; }

		const auto PDMP = GetCurrentPhysicalDeviceMemoryProperties();
		const auto& CB = CommandBuffers[0];

#pragma region BLAS_GEOMETRY
		//!< �o�[�e�b�N�X�o�b�t�@ (VertexBuffer) 
		constexpr std::array Vertices = { glm::vec3({ 0.0f, 0.5f, 0.0f }), glm::vec3({ -0.5f, -0.5f, 0.0f }), glm::vec3({ 0.5f, -0.5f, 0.0f }), };
		Scoped<HostVisibleASBuffer> VB(Device);
		VB.Create(Device, PDMP, TotalSizeOf(Vertices), data(Vertices));

		//!< �C���f�b�N�X�o�b�t�@ (IndexBuffer) 
		constexpr std::array Indices = { uint32_t(0), uint32_t(1), uint32_t(2) };
		Scoped<HostVisibleASBuffer> IB(Device);
		IB.Create(Device, PDMP, TotalSizeOf(Indices), data(Indices));

		//!< �W�I���g�� (Geometry)
		const std::array ASGs_Blas = {
			VkAccelerationStructureGeometryKHR({
				.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR,
				.pNext = nullptr,
				.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_KHR,
				.geometry = VkAccelerationStructureGeometryDataKHR({
					//!< �g���C�A���O�� (Triangle)
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
			.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR, //!< �{�g�����x�����w��
#ifdef USE_BLAS_COMPACTION
			.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR | VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_COMPACTION_BIT_KHR,
#else
			.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR,
#endif
			.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR,
			.srcAccelerationStructure = VK_NULL_HANDLE, .dstAccelerationStructure = VK_NULL_HANDLE,
			.geometryCount = static_cast<uint32_t>(size(ASGs_Blas)), .pGeometries = data(ASGs_Blas), .ppGeometries = nullptr, //!< �W�I���g�����w��
			.scratchData = VkDeviceOrHostAddressKHR({.deviceAddress = 0})
		};
		const std::vector ASBRIs_Blas = {
			VkAccelerationStructureBuildRangeInfoKHR({.primitiveCount = static_cast<uint32_t>(size(Indices) / 3), .primitiveOffset = 0, .firstVertex = 0, .transformOffset = 0 }),
		};
		assert(ASBGI_Blas.geometryCount == size(ASBRIs_Blas));

		//!< �T�C�Y�擾 (Get sizes)
		VkAccelerationStructureBuildSizesInfoKHR ASBSI_Blas = { .sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR, };
		{
			const std::array MaxPrimitiveCounts = {
				std::ranges::max_element(ASBRIs_Blas, [](const auto& lhs, const auto& rhs) { return lhs.primitiveCount < rhs.primitiveCount; })->primitiveCount
			};
			assert(ASBGI_Blas.geometryCount == size(MaxPrimitiveCounts));
			vkGetAccelerationStructureBuildSizesKHR(Device, VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR, &ASBGI_Blas, data(MaxPrimitiveCounts), &ASBSI_Blas);
		}

		//!< AS�쐬 (Create AS)
		BLASs.emplace_back().Create(Device, PDMP, ASBSI_Blas.accelerationStructureSize);
		ASBGI_Blas.dstAccelerationStructure = BLASs.back().AccelerationStructure;
		
#ifdef USE_BLAS_COMPACTION
			{
				//!< �N�G���v�[�����쐬
				VkQueryPool QueryPool = VK_NULL_HANDLE;
				const VkQueryPoolCreateInfo QPCI = {
					.sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO,
					.pNext = nullptr,
					.flags = 0,
					.queryType = VK_QUERY_TYPE_ACCELERATION_STRUCTURE_COMPACTED_SIZE_KHR,
					.queryCount = 1,
					.pipelineStatistics = 0
				};
				VERIFY_SUCCEEDED(vkCreateQueryPool(Device, &QPCI, GetAllocationCallbacks(), &QueryPool));

				//!< (�R���p�N�V�����T�C�Y���擾�ł���悤��)�N�G���v�[���������w�肵�� (�ꎞ)BLAS ���쐬
				BLAS Tmp;
				Tmp.Create(Device, PDMP, ASBSI_Blas.accelerationStructureSize).SubmitBuildCommand(Device, PDMP, ASBSI_Blas.buildScratchSize, ASBGI_Blas, ASBRIs_Blas, GraphicsQueue, CB, QueryPool);

				//!< �N�G���v�[������R���p�N�V�����T�C�Y���擾
				std::array CompactSizes = { VkDeviceSize(0) };
				VERIFY_SUCCEEDED(vkGetQueryPoolResults(Device, QueryPool, 0, static_cast<uint32_t>(size(CompactSizes)), sizeof(CompactSizes), data(CompactSizes), sizeof(CompactSizes[0]), VK_QUERY_RESULT_WAIT_BIT));
				std::cout << "BLAS Compaction = " << ASBSI_Blas.accelerationStructureSize << " -> " << CompactSizes[0] << std::endl;

				//!< �R���p�N�V�����T�C�Y�� (���K)BLAS ���쐬���� (�R�s�[����̂Ńr���h�͂��Ȃ���)
				BLASs.emplace_back().Create(Device, PDMP, CompactSizes[0])
					//!< �ꎞBLAS -> ���KBLAS �R�s�[�R�}���h�𔭍s����
					.SubmitCopyCommand(Tmp.AccelerationStructure, BLASs.back().AccelerationStructure, GraphicsQueue, CB);
				
				//!< ��n��
				Tmp.Destroy(Device);
				vkDestroyQueryPool(Device, QueryPool, GetAllocationCallbacks());
			}
#endif
#pragma endregion

#pragma region TLAS_GEOMETRY
		//!< �C���X�^���X�o�b�t�@ (InstanceBuffer) ... ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR | SHADER_DEVICE_ADDRESS_BIT�AHOST_VISIBLE_BIT | HOST_COHERENT_BIT �ō쐬
		const std::array ASIs = {
			VkAccelerationStructureInstanceKHR({
				.transform = VkTransformMatrixKHR({1.0f, 0.0f, 0.0f, 0.0f,
													0.0f, 1.0f, 0.0f, 0.0f,
													0.0f, 0.0f, 1.0f, 0.0f}),
				.instanceCustomIndex = 0,
				.mask = 0xff,
				.instanceShaderBindingTableRecordOffset = 0,
				.flags = VK_GEOMETRY_INSTANCE_TRIANGLE_FRONT_COUNTERCLOCKWISE_BIT_KHR,
				.accelerationStructureReference = GetDeviceAddress(Device, BLASs.back().AccelerationStructure) /* GetDeviceAddress(Device, BLASs.back().Buffer)�ł��ǂ� */
			}),
		};
		Scoped<HostVisibleASBuffer> InstBuf(Device);
		InstBuf.Create(Device, PDMP, sizeof(ASIs), data(ASIs));

		//!< �W�I���g�� (Geometry)
		const std::array ASGs_Tlas = {
			VkAccelerationStructureGeometryKHR({
				.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR,
				.pNext = nullptr,
				//!< �C���X�^���X (Instance)
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
			.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR, //!< �g�b�v���x�����w��
			.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR,
			.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR,
			.srcAccelerationStructure = VK_NULL_HANDLE, .dstAccelerationStructure = VK_NULL_HANDLE,
			.geometryCount = static_cast<uint32_t>(size(ASGs_Tlas)), .pGeometries = data(ASGs_Tlas), .ppGeometries = nullptr, //!< [GLSL] gl_GeometryIndexEXT ([HLSL] GeometryIndex() ����)
			.scratchData = VkDeviceOrHostAddressKHR({.deviceAddress = 0})
		};
		const std::vector ASBRIs_Tlas = {
			VkAccelerationStructureBuildRangeInfoKHR({.primitiveCount = static_cast<uint32_t>(size(ASIs)), .primitiveOffset = 0, .firstVertex = 0, .transformOffset = 0 }),
		};
		assert(ASBGI_Tlas.geometryCount == size(ASBRIs_Tlas));

		//!< �T�C�Y�擾 (Get sizes)
		VkAccelerationStructureBuildSizesInfoKHR ASBSI_Tlas = { .sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR, };
		{
			const std::array MaxPrimitiveCounts = {
				std::ranges::max_element(ASBRIs_Tlas, [](const auto& lhs, const auto& rhs) { return lhs.primitiveCount < rhs.primitiveCount; })->primitiveCount
			};
			assert(ASBGI_Tlas.geometryCount == size(MaxPrimitiveCounts));
			vkGetAccelerationStructureBuildSizesKHR(Device, VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR, &ASBGI_Tlas, data(MaxPrimitiveCounts), &ASBSI_Tlas);
		}

		//!< AS�쐬 (Create AS)
		TLASs.emplace_back().Create(Device, PDMP, ASBSI_Tlas.accelerationStructureSize);
		ASBGI_Tlas.dstAccelerationStructure = TLASs.back().AccelerationStructure;
#pragma endregion

#pragma region SCRATCH
		Scoped<ScratchBuffer> Scratch(Device);
#ifdef USE_BLAS_COMPACTION
		Scratch.Create(Device, PDMP, ASBSI_Tlas.buildScratchSize);
#else
		Scratch.Create(Device, PDMP, std::max(ASBSI_Blas.buildScratchSize, ASBSI_Tlas.buildScratchSize));
		ASBGI_Blas.scratchData = VkDeviceOrHostAddressKHR({ .deviceAddress = VK::GetDeviceAddress(Device, Scratch.Buffer) });
#endif
		ASBGI_Tlas.scratchData = VkDeviceOrHostAddressKHR({ .deviceAddress = VK::GetDeviceAddress(Device, Scratch.Buffer) });
#pragma endregion

		constexpr VkCommandBufferBeginInfo CBBI = { .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO, .pNext = nullptr, .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, .pInheritanceInfo = nullptr };
		VERIFY_SUCCEEDED(vkBeginCommandBuffer(CB, &CBBI)); {
#ifndef USE_BLAS_COMPACTION
			BLASs.back().PopulateBuildCommand(ASBGI_Blas, ASBRIs_Blas, CB);
#endif
			//!< TLAS �̃r���h���ɂ� BLAS �̃r���h���������Ă���K�v������̂Ńo���A
			AccelerationStructureBuffer::PopulateMemoryBarrierCommand(CB);

			TLASs.back().PopulateBuildCommand(ASBGI_Tlas, ASBRIs_Tlas, CB);
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
			VK::CreateShaderModule(data(ShaderPath + TEXT(".rchit.spv"))),
		};
		//!< �V�F�[�_�X�e�[�W
		const std::array PSSCIs = {
			VkPipelineShaderStageCreateInfo({.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, .pNext = nullptr, .flags = 0, .stage = VK_SHADER_STAGE_RAYGEN_BIT_KHR, .module = SMs[0], .pName = "main", .pSpecializationInfo = nullptr }),
			VkPipelineShaderStageCreateInfo({.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, .pNext = nullptr, .flags = 0, .stage = VK_SHADER_STAGE_MISS_BIT_KHR, .module = SMs[1], .pName = "main", .pSpecializationInfo = nullptr }),
			VkPipelineShaderStageCreateInfo({.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, .pNext = nullptr, .flags = 0, .stage = VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR, .module = SMs[2], .pName = "main", .pSpecializationInfo = nullptr }),
		};
		//!< �V�F�[�_�O���[�v
		//!< �q�b�g�n�� 1 �O���[�v�� 3��(closestHit, anyHit, intersection)��蓾��A����ȊO(general �^�C�v)�� 1 �� 1 �O���[�v�ƂȂ�
		//!< .generalShader, .closestHitShader, ... �ɃV�F�[�_�X�e�[�W�̃C���f�b�N�X���w�肷��
		const std::array RTSGCIs = {
			VkRayTracingShaderGroupCreateInfoKHR({.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR, .pNext = nullptr, .type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR, .generalShader = 0, .closestHitShader = VK_SHADER_UNUSED_KHR, .anyHitShader = VK_SHADER_UNUSED_KHR, .intersectionShader = VK_SHADER_UNUSED_KHR, .pShaderGroupCaptureReplayHandle = nullptr }),
			VkRayTracingShaderGroupCreateInfoKHR({.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR, .pNext = nullptr, .type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR, .generalShader = 1, .closestHitShader = VK_SHADER_UNUSED_KHR, .anyHitShader = VK_SHADER_UNUSED_KHR, .intersectionShader = VK_SHADER_UNUSED_KHR, .pShaderGroupCaptureReplayHandle = nullptr }),
			VkRayTracingShaderGroupCreateInfoKHR({.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR, .pNext = nullptr, .type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR, .generalShader = VK_SHADER_UNUSED_KHR, .closestHitShader = 2, .anyHitShader = VK_SHADER_UNUSED_KHR, .intersectionShader = VK_SHADER_UNUSED_KHR, .pShaderGroupCaptureReplayHandle = nullptr }),
		};

		constexpr std::array DSs = { VK_DYNAMIC_STATE_RAY_TRACING_PIPELINE_STACK_SIZE_KHR };
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
		const auto& PDMP = GetCurrentPhysicalDeviceMemoryProperties();
		auto& SBT = ShaderBindingTables.emplace_back(); {
			//!< �T�C�Y�A�A���C�����g���擾
			VkPhysicalDeviceRayTracingPipelinePropertiesKHR PDRTPP = { .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_PROPERTIES_KHR, .pNext = nullptr };
			VkPhysicalDeviceProperties2 PDP2 = { .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2, .pNext = &PDRTPP, };
			vkGetPhysicalDeviceProperties2(GetCurrentPhysicalDevice(), &PDP2);

			//!< �e�O���[�v�̃n���h���̌�
			const auto GenHandleCount = 1;
			const auto MissHandleCount = 1;
			const auto HitHandleCount = 1;
			const auto TotalHandleCount = GenHandleCount + MissHandleCount + HitHandleCount;

			//!< �e�O���[�v�̃X�g���C�h�ƃT�C�Y������
			const auto AlignedHandleSize = Cmn::RoundUp(PDRTPP.shaderGroupHandleSize, PDRTPP.shaderGroupHandleAlignment);
			const auto GenStrideSize = Cmn::RoundUp(GenHandleCount * AlignedHandleSize, PDRTPP.shaderGroupBaseAlignment);
			//!< RayGen �ł� stride �� size �������T�C�Y�łȂ��Ƃ����Ȃ��̂Œ���
			SBT.StridedDeviceAddressRegions.emplace_back(VkStridedDeviceAddressRegionKHR({ .stride = GenStrideSize, .size = GenStrideSize}));
			SBT.StridedDeviceAddressRegions.emplace_back(VkStridedDeviceAddressRegionKHR({ .stride = AlignedHandleSize, .size = Cmn::RoundUp(MissHandleCount * AlignedHandleSize, PDRTPP.shaderGroupBaseAlignment) }));
			SBT.StridedDeviceAddressRegions.emplace_back(VkStridedDeviceAddressRegionKHR({ .stride = AlignedHandleSize, .size = Cmn::RoundUp(HitHandleCount * AlignedHandleSize, PDRTPP.shaderGroupBaseAlignment) }));
		
			//!< �o�b�t�@�̍쐬�A�f�o�C�X�A�h���X�̌���
			SBT.Create(Device, PDMP);

			//!< �n���h���f�[�^(�R�s�[��)���擾
#if true
			std::vector<std::byte> HandleData(PDRTPP.shaderGroupHandleSize * TotalHandleCount);
			VERIFY_SUCCEEDED(vkGetRayTracingShaderGroupHandlesKHR(Device, Pipelines.back(), 0, TotalHandleCount/*�n���h�����Ȃ̂��O���[�v���Ȃ̂��H*/, size(HandleData), data(HandleData)));
#else
			std::vector<std::byte> HandleData(AlignedHandleSize * size(SBT.StridedDeviceAddressRegions));
			VERIFY_SUCCEEDED(vkGetRayTracingShaderGroupHandlesKHR(Device, Pipelines.back(), 0, static_cast<uint32_t>(size(SBT.StridedDeviceAddressRegions)), size(HandleData), data(HandleData)));
#endif

			//!< (�}�b�v����)�n���h���f�[�^���o�b�t�@�ɏ�������
			auto MapData = SBT.Map(Device); {
				auto BData = reinterpret_cast<std::byte*>(MapData);
				auto HData = data(HandleData);

				//!< �O���[�v (Gen)
				const auto& GenRegion = SBT.StridedDeviceAddressRegions[0]; {
					auto p = BData;
					//!< �O���[�v�̃n���h��
					for (auto i = 0; i < GenHandleCount; ++i, HData += PDRTPP.shaderGroupHandleSize, p += GenRegion.stride) {
						std::memcpy(p, HData, PDRTPP.shaderGroupHandleSize);
					}
					BData += GenRegion.size;
				}

				//!< �O���[�v (Miss)
				const auto& MissRegion = SBT.StridedDeviceAddressRegions[1]; {
					auto p = BData;
					//!< �O���[�v�̃n���h��
					for (auto i = 0; i < MissHandleCount; ++i, HData += PDRTPP.shaderGroupHandleSize, p += MissRegion.stride) {
						std::memcpy(p, HData, PDRTPP.shaderGroupHandleSize);
					}
					BData += MissRegion.size;
				}

				//!< �O���[�v (Hit)
				const auto& HitRegion = SBT.StridedDeviceAddressRegions[2]; {
					auto p = BData;
					//!< �O���[�v�̃n���h��
					for (auto i = 0; i < HitHandleCount; ++i, HData += PDRTPP.shaderGroupHandleSize, p += HitRegion.stride) {
						std::memcpy(p, HData, PDRTPP.shaderGroupHandleSize);
					}
					BData += HitRegion.size;
				}

			} SBT.Unmap(Device);
		}

#ifdef USE_INDIRECT
		//!< �C���_�C���N�g�o�b�t�@ (IndirectBuffer) 
		//!< VK �ł� ShaderTable ���܂߂Ȃ��̂ł����ƑO�ł��ł��邪�ADX �ɍ��킹�Ă����ōs�����Ƃɂ���
		const VkTraceRaysIndirectCommandKHR TRIC = { .width = static_cast<uint32_t>(GetClientRectWidth()), .height = static_cast<uint32_t>(GetClientRectHeight()), .depth = 1 };
		IndirectBuffers.emplace_back().Create(Device, PDMP, TRIC).SubmitCopyCommand(Device, PDMP, CommandBuffers[0], GraphicsQueue, sizeof(TRIC), &TRIC);
#endif
	}
	virtual void PopulateCommandBuffer(const size_t i) override {
		if (!HasRayTracingSupport(GetCurrentPhysicalDevice())) { return; }

		const auto CB = CommandBuffers[i];
		const auto RT = StorageTextures[0].Image;
		constexpr VkCommandBufferBeginInfo CBBI = {
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
			.pNext = nullptr,
			.flags = 0,
			.pInheritanceInfo = nullptr
		};
		VERIFY_SUCCEEDED(vkBeginCommandBuffer(CB, &CBBI)); {
			PopulateBeginRenderTargetCommand(CB, RT); {
				vkCmdBindPipeline(CB, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, Pipelines[0]);

				const std::array DSs = { DescriptorSets[0] };
				constexpr std::array<uint32_t, 0> DynamicOffsets = {};
				vkCmdBindDescriptorSets(CB, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, PipelineLayouts[0], 0, static_cast<uint32_t>(size(DSs)), data(DSs), static_cast<uint32_t>(size(DynamicOffsets)), data(DynamicOffsets));

				const auto& SBT = ShaderBindingTables.back();
				const auto CallableRegion = VkStridedDeviceAddressRegionKHR({ .deviceAddress = 0, .stride = 0, .size = 0 });
#ifdef USE_INDIRECT
				vkCmdTraceRaysIndirectKHR(CB, &SBT.StridedDeviceAddressRegions[0], &SBT.StridedDeviceAddressRegions[1], &SBT.StridedDeviceAddressRegions[2], &CallableRegion, GetDeviceAddress(Device, IndirectBuffers[0].Buffer));
#else
				vkCmdTraceRaysKHR(CB, &SBT.StridedDeviceAddressRegions[0], &SBT.StridedDeviceAddressRegions[1], &SBT.StridedDeviceAddressRegions[2], &CallableRegion, GetClientRectWidth(), GetClientRectHeight(), 1);
#endif
			} PopulateEndRenderTargetCommand(CB, RT, SwapchainImages[i], static_cast<uint32_t>(GetClientRectWidth()), static_cast<uint32_t>(GetClientRectHeight()));

		} VERIFY_SUCCEEDED(vkEndCommandBuffer(CB));
	}

	//!< #VK_TODO vkUpdateDescriptorSetWithTemplate() �� AccelerationStructure �Ή�
#if 0
private:
	struct DescriptorUpdateInfo
	{
		//VkDescriptorAccelerationStructureInfo DescriptorASInfos[1];
		VkDescriptorImageInfo DescriptorImageInfos[1];
	};
#endif
};
#pragma endregion
