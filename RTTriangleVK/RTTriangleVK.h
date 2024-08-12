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
		if (!HasRayTracingSupport(SelectedPhysDevice.first)) { return; }

		const auto& PDMP = SelectedPhysDevice.second.PDMP;
		const auto& CB = CommandBuffers[0];

#pragma region BLAS_GEOMETRY
		//!< �o�[�e�b�N�X�o�b�t�@ (VertexBuffer) 
		const std::array Vertices = { glm::vec3({ 0.0f, 0.5f, 0.0f }), glm::vec3({ -0.5f, -0.5f, 0.0f }), glm::vec3({ 0.5f, -0.5f, 0.0f }), };
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
		ASBGI_Blas.scratchData = VkDeviceOrHostAddressKHR({ .deviceAddress = GetDeviceAddress(Device, Scratch.Buffer) });
#endif
		ASBGI_Tlas.scratchData = VkDeviceOrHostAddressKHR({ .deviceAddress = GetDeviceAddress(Device, Scratch.Buffer) });
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
		if (!HasRayTracingSupport(SelectedPhysDevice.first)) { return; }
#pragma region PIPELINE
		const auto PLL = PipelineLayouts.back();

		const std::array SMs = {
			VK::CreateShaderModule(GetFilePath(".rgen.spv")),
			VK::CreateShaderModule(GetFilePath(".rmiss.spv")),
			VK::CreateShaderModule(GetFilePath(".rchit.spv")),
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
				.maxPipelineRayRecursionDepth = 1,
				.pLibraryInfo = nullptr,
				.pLibraryInterface = nullptr,
				.pDynamicState = &PDSCI,
				.layout = PLL,
				.basePipelineHandle = VK_NULL_HANDLE, .basePipelineIndex = -1
			}),
		};
#ifdef _DEBUG
		//!< �����̊��ł� PDRTPP.maxRayRecursionDepth == 31 ������ (DX �� [0, 31])
		for (auto i : RTPCIs) { assert(i.maxPipelineRayRecursionDepth <= PDRTPP.maxRayRecursionDepth); }
#endif
		VERIFY_SUCCEEDED(vkCreateRayTracingPipelinesKHR(Device, VK_NULL_HANDLE, VK_NULL_HANDLE, static_cast<uint32_t>(size(RTPCIs)), data(RTPCIs), GetAllocationCallbacks(), &Pipelines.emplace_back()));
#pragma endregion

		for (auto i : SMs) { vkDestroyShaderModule(Device, i, GetAllocationCallbacks()); }
	}
	virtual void CreateShaderBindingTable() override {
		const auto& PDMP = SelectedPhysDevice.second.PDMP;
		auto& SBT = ShaderBindingTables.emplace_back(); {
			//!< �e�O���[�v�̃n���h���̌� (Gen��1�Œ�)
			constexpr auto MissCount = 1;
			constexpr auto HitCount = 1;
			//!< �V�F�[�_���R�[�h�T�C�Y
			constexpr auto GenRecordSize = 0;
			constexpr auto MissRecordSize = 0;
			constexpr auto HitRecordSize = 0;
			//!< �X�g���C�h (�e�V�F�[�_���R�[�h�̐擪�A���C�����g)
			const auto GenStride = Cmn::RoundUp(PDRTPP.shaderGroupHandleSize + GenRecordSize, PDRTPP.shaderGroupHandleAlignment);
			const auto MissStride = Cmn::RoundUp(PDRTPP.shaderGroupHandleSize + MissRecordSize, PDRTPP.shaderGroupHandleAlignment);
			const auto HitStride = Cmn::RoundUp(PDRTPP.shaderGroupHandleSize + HitRecordSize, PDRTPP.shaderGroupHandleAlignment);
			//!< �T�C�Y (�e�O���[�v�̐擪�A���C�����g)
			const auto GenSize = Cmn::RoundUp(GenStride, PDRTPP.shaderGroupBaseAlignment);
			const auto MissSize = Cmn::RoundUp(MissCount * MissStride, PDRTPP.shaderGroupBaseAlignment);
			const auto HitSize = Cmn::RoundUp(HitCount * HitStride, PDRTPP.shaderGroupBaseAlignment);
			//!< ���[�W����
			//!< Gen �ł̓X�g���C�h�ɃT�C�Y�Ɠ����l���w�肵�Ȃ��Ă͂Ȃ�Ȃ��̂Œ���
			SBT.StridedDeviceAddressRegions[0] = VkStridedDeviceAddressRegionKHR({ .stride = GenSize, .size = GenSize });
			SBT.StridedDeviceAddressRegions[1] = VkStridedDeviceAddressRegionKHR({ .stride = MissStride, .size = MissSize });
			SBT.StridedDeviceAddressRegions[2] = VkStridedDeviceAddressRegionKHR({ .stride = HitStride, .size = HitSize });
		
			//!< �o�b�t�@�̍쐬�A�f�o�C�X�A�h���X�̌���
			SBT.Create(Device, PDMP);

			//!< �n���h���f�[�^(�R�s�[��)���擾
#if true
			constexpr auto TotalHandleCount = 1 + MissCount + HitCount;
			std::vector<std::byte> HandleData(PDRTPP.shaderGroupHandleSize * TotalHandleCount);
			VERIFY_SUCCEEDED(vkGetRayTracingShaderGroupHandlesKHR(Device, Pipelines.back(), 0, TotalHandleCount/*�n���h�����Ȃ̂��O���[�v���Ȃ̂��H*/, size(HandleData), data(HandleData)));
#else
			std::vector<std::byte> HandleData(AlignedHandleSize * size(SBT.StridedDeviceAddressRegions));
			VERIFY_SUCCEEDED(vkGetRayTracingShaderGroupHandlesKHR(Device, Pipelines.back(), 0, static_cast<uint32_t>(size(SBT.StridedDeviceAddressRegions)), size(HandleData), data(HandleData)));
#endif

			//!< (�}�b�v����)�n���h���f�[�^���o�b�t�@�ɏ�������
			auto MapData = SBT.Map(Device); {
				auto HData = data(HandleData);
				auto Data = reinterpret_cast<std::byte*>(MapData);

				//!< �O���[�v (Gen)
				{
					const auto& Region = SBT.StridedDeviceAddressRegions[0]; {
						//!< �O���[�v�̃n���h��
						std::memcpy(Data, HData, PDRTPP.shaderGroupHandleSize);
						HData += PDRTPP.shaderGroupHandleSize;
						Data += Region.size;
					}
				}

				//!< �O���[�v (Miss)
				{
					const auto Count = MissCount;
					const auto& Region = SBT.StridedDeviceAddressRegions[1]; {
						auto p = Data;
						//!< �O���[�v�̃n���h��
						for (auto i = 0; i < Count; ++i, HData += PDRTPP.shaderGroupHandleSize, p += Region.stride) {
							std::memcpy(p, HData, PDRTPP.shaderGroupHandleSize);
						}
						Data += Region.size;
					}
				}

				//!< �O���[�v (Hit)
				{
					const auto Count = HitCount;
					const auto& Region = SBT.StridedDeviceAddressRegions[2]; {
						auto p = Data;
						//!< �O���[�v�̃n���h��
						for (auto i = 0; i < Count; ++i, HData += PDRTPP.shaderGroupHandleSize, p += Region.stride) {
							std::memcpy(p, HData, PDRTPP.shaderGroupHandleSize);
						}
						Data += Region.size;
					}
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
			//!< ���ꂼ��̃X�^�b�N�T�C�Y (�K�؂Ȓl�͕s���A�Ƃ肠�����K���Ɍ��߂Ă���)
			constexpr auto RayGenStack = 10, ClosestHitStack = 10, MissStack = 10, IntersectionStack = 10, AnyHitStack = 10, CallableStack = 10;
			constexpr auto RecursionDepth = 2;

			//!< �p�C�v���C���X�^�b�N�T�C�Y�̌v�Z���@�͈ȉ��̂悤�ɂȂ�
			constexpr auto PipelineStackSize = RayGenStack
				+ (std::min)(1, RecursionDepth) * (std::max)((std::max)(ClosestHitStack, MissStack), IntersectionStack + AnyHitStack)
				+ (std::max)(0, RecursionDepth - 1) * (std::max)(ClosestHitStack, MissStack)
				+ 2 * CallableStack;
			//!< VK_DYNAMIC_STATE_RAY_TRACING_PIPELINE_STACK_SIZE_KHR �g�p���ɂ̓Z�b�g����K�v������
			vkCmdSetRayTracingPipelineStackSizeKHR(CB, PipelineStackSize);

			PopulateBeginRenderTargetCommand(CB, RT); {
				vkCmdBindPipeline(CB, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, Pipelines[0]);

				const std::array DSs = { DescriptorSets[0] };
				constexpr std::array<uint32_t, 0> DynamicOffsets = {};
				vkCmdBindDescriptorSets(CB, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, PipelineLayouts[0], 0, static_cast<uint32_t>(size(DSs)), data(DSs), static_cast<uint32_t>(size(DynamicOffsets)), data(DynamicOffsets));

				const auto& SBT = ShaderBindingTables.back();
#ifdef USE_INDIRECT
				vkCmdTraceRaysIndirectKHR(CB, &SBT.StridedDeviceAddressRegions[0], &SBT.StridedDeviceAddressRegions[1], &SBT.StridedDeviceAddressRegions[2], &SBT.StridedDeviceAddressRegions[3], GetDeviceAddress(Device, IndirectBuffers[0].Buffer));
#else
				vkCmdTraceRaysKHR(CB, &SBT.StridedDeviceAddressRegions[0], &SBT.StridedDeviceAddressRegions[1], &SBT.StridedDeviceAddressRegions[2], &SBT.StridedDeviceAddressRegions[3], GetClientRectWidth(), GetClientRectHeight(), 1);
#endif
			} PopulateEndRenderTargetCommand(CB, RT, Swapchain.ImageAndViews[i].first, static_cast<uint32_t>(GetClientRectWidth()), static_cast<uint32_t>(GetClientRectHeight()));

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
