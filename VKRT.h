#pragma once

#include "VKExt.h"

class VKRT : public VKExt
{
private:
	using Super = VKExt;
protected:
	class AABB
	{
	public:
		glm::vec3 Min;
		glm::vec3 Max;
	};

	virtual void OnDestroy(HWND hWnd, HINSTANCE hInstance) override {	
		for (auto i : ShaderBindingTables) { i.Destroy(Device); } ShaderBindingTables.clear();
		for (auto i : BLASs) { i.Destroy(Device); } BLASs.clear();
		for (auto i : TLASs) { i.Destroy(Device); } TLASs.clear();

		Super::OnDestroy(hWnd, hInstance);
	}

	virtual void CreateInstance([[maybe_unused]] const std::vector<const char*>& AdditionalLayers, const std::vector<const char*>& AdditionalExtensions) override {
		//Super::CreateInstance(AdditionalLayers, AdditionalExtensions); //!< VK_LAYER_RENDERDOC_Capture ���g�p����
		VK::CreateInstance(AdditionalLayers, AdditionalExtensions); //!< VK_LAYER_RENDERDOC_Capture ���g�p���Ȃ�
	}
	virtual void CreateDevice(HWND hWnd, HINSTANCE hInstance) override {
		if (HasRayTracingSupport(SelectedPhysDevice.first)) {
#ifdef _DEBUG
#if true
			assert(VK_HEADER_VERSION_COMPLETE >= VK_MAKE_VERSION(1, 2, 162) && "RayTracing require 1.2.162 or later");
#else
			uint32_t APIVersion; VERIFY_SUCCEEDED(vkEnumerateInstanceVersion(&APIVersion)); assert(APIVersion >= VK_MAKE_VERSION(1, 2, 162) && "RayTracing require 1.2.162 or later");
#endif
#endif
			//!< VkPhysicalDeviceRayTracingPipelinePropertiesKHR PDRTPP �͂悭�g���̂Ń����o�Ƃ��Ċo���Ă���
			VkPhysicalDeviceProperties2 PDP2 = {
				.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2,
				.pNext = &PDRTPP
			};
			vkGetPhysicalDeviceProperties2(SelectedPhysDevice.first, &PDP2);

			RTFeature RTF;
			Super::CreateDevice(hWnd, hInstance, RTF.GetPtr(), RTF.ExtNames);
		}
		else {
			Super::CreateDevice(hWnd, hInstance);
		}
	}
	virtual bool CreateSwapchain() override {
		//!< �Ō�ɃR�s�[���邽�� VK_IMAGE_USAGE_TRANSFER_DST_BIT ���K�v
		if (VK::CreateSwapchain(SelectedPhysDevice.first, Surface, GetClientRectWidth(), GetClientRectHeight(), VK_IMAGE_USAGE_TRANSFER_DST_BIT)) {
			VK::GetSwapchainImages();
			return false;
		}
		return false;
	}
	virtual void CreateTexture() override {
		if (!HasRayTracingSupport(SelectedPhysDevice.first)) { return; }
		//!< �X���b�v�`�F�C���Ɠ����J���[�t�H�[�}�b�g�ɂ��Ă����A(���C�A�E�g�͕ύX������߂����肷��̂ŁA�߂��郌�C�A�E�g�ɂ��Ă���(�����ł� TRANSFER_SRC_OPTIMAL))
		StorageTextures.emplace_back().Create(Device, SelectedPhysDevice.second.PDMP, SurfaceFormat.format, VkExtent3D({ .width = static_cast<uint32_t>(GetClientRectWidth()), .height = static_cast<uint32_t>(GetClientRectHeight()), .depth = 1 }))
			.SubmitSetLayoutCommand(CommandBuffers[0], GraphicsQueue, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
	}
	virtual void CreatePipelineLayout() override {
		if (!HasRayTracingSupport(SelectedPhysDevice.first)) { return; }
		CreateDescriptorSetLayout(DescriptorSetLayouts.emplace_back(), 0, {
			//!< TLAS
			VkDescriptorSetLayoutBinding({.binding = 0, .descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, .descriptorCount = 1, .stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR | VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR, .pImmutableSamplers = nullptr }),
			//!< �o�� (StorageImage)
			VkDescriptorSetLayoutBinding({.binding = 1, .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, .descriptorCount = 1, .stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR, .pImmutableSamplers = nullptr }),
			});
		VK::CreatePipelineLayout(PipelineLayouts.emplace_back(), DescriptorSetLayouts, {});
	}
	virtual void CreateDescriptor() override {
		VKExt::CreateDescriptorPool(DescriptorPools.emplace_back(), VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT, {
			VkDescriptorPoolSize({.type = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, .descriptorCount = 1 }),
			VkDescriptorPoolSize({.type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, .descriptorCount = 1 })
			});
		const std::array DSLs = { DescriptorSetLayouts[0] };
		const VkDescriptorSetAllocateInfo DSAI = {
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
			.pNext = nullptr,
			.descriptorPool = DescriptorPools[0],
			.descriptorSetCount = static_cast<uint32_t>(size(DSLs)), .pSetLayouts = data(DSLs)
		};
		VERIFY_SUCCEEDED(vkAllocateDescriptorSets(Device, &DSAI, &DescriptorSets.emplace_back()));

		const std::array ASs = { TLASs[0].AccelerationStructure };
		const auto WDSAS = VkWriteDescriptorSetAccelerationStructureKHR({ .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_KHR, .pNext = nullptr, .accelerationStructureCount = static_cast<uint32_t>(size(ASs)), .pAccelerationStructures = data(ASs) });
		const auto DII = VkDescriptorImageInfo({ .sampler = VK_NULL_HANDLE, .imageView = StorageTextures[0].View, .imageLayout = VK_IMAGE_LAYOUT_GENERAL });
		const std::array WDSs = {
			//!< TLAS
			VkWriteDescriptorSet({
				.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
				.pNext = &WDSAS, //!< pNext �� VkWriteDescriptorSetAccelerationStructureKHR ���w�肷�邱��
				.dstSet = DescriptorSets[0],
				.dstBinding = 0, .dstArrayElement = 0,
				.descriptorCount = 1, .descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR,
				.pImageInfo = nullptr, .pBufferInfo = nullptr, .pTexelBufferView = nullptr
			}),
			//!< �o�� (StorageImage)
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
		vkUpdateDescriptorSets(Device, static_cast<uint32_t>(std::size(WDSs)), std::data(WDSs), static_cast<uint32_t>(std::size(CDSs)), std::data(CDSs));
	}

	[[nodiscard]] static bool HasRayTracingSupport(const VkPhysicalDevice PD) {
		RTFeature RTF;
		VkPhysicalDeviceFeatures2 PDF2 = {
			.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2, 
			.pNext = RTF.GetPtr()
		};
		vkGetPhysicalDeviceFeatures2(PD, &PDF2);
		return RTF.PDRTPF.rayTracingPipeline && RTF.PDASF.accelerationStructure;
	}

#pragma region RAYTRACING
	//class DeviceLocalShaderDeviceStorageBuffer : public DeviceLocalBuffer
	//{
	//private:
	//	using Super = DeviceLocalBuffer;
	//public:
	//	DeviceLocalShaderDeviceStorageBuffer& Create(const VkDevice Device, const VkPhysicalDeviceMemoryProperties& PDMP, const size_t Size) {
	//		Super::Create(Device, PDMP, Size, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
	//		return *this;
	//	}
	//};
	class ASBuffer : public DeviceLocalBuffer
	{
	private:
		using Super = DeviceLocalBuffer;
	protected:
		const VkBufferUsageFlags Usage = VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
	public:
		ASBuffer& Create(const VkDevice Dev, const VkPhysicalDeviceMemoryProperties& PDMP, const size_t Size) {
			Super::Create(Dev, PDMP, Size, Usage);
			return *this;
		}
	public:
		void PopulateCopyCommand(const VkCommandBuffer CB, const size_t Size, const VkBuffer Staging) {
			Super::PopulateCopyCommand(CB, Size, Staging, VK_ACCESS_2_ACCELERATION_STRUCTURE_READ_BIT_KHR, VK_PIPELINE_STAGE_2_RAY_TRACING_SHADER_BIT_KHR);
		}
		void SubmitCopyCommand(const VkDevice Dev, const VkPhysicalDeviceMemoryProperties& PDMP, const VkCommandBuffer CB, const VkQueue Queue, const size_t Size, const void* Source) {
			Super::SubmitCopyCommand(Dev, PDMP, CB, Queue, Size, Source, VK_ACCESS_2_ACCELERATION_STRUCTURE_READ_BIT_KHR, VK_PIPELINE_STAGE_2_RAY_TRACING_SHADER_BIT_KHR);
		}
	};
#if true
	class VertexASBuffer : public ASBuffer
	{
	private:
		using Super = ASBuffer;
	public:
		VertexASBuffer& Create(const VkDevice Dev, const VkPhysicalDeviceMemoryProperties& PDMP, const size_t Size) {
			DeviceLocalBuffer::Create(Dev, PDMP, Size, Usage | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
			return *this;
		}
	};
	class IndexASBuffer : public ASBuffer
	{
	private:
		using Super = ASBuffer;
	public:
		IndexASBuffer& Create(const VkDevice Dev, const VkPhysicalDeviceMemoryProperties& PDMP, const size_t Size) {
			DeviceLocalBuffer::Create(Dev, PDMP, Size, Usage | VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
			return *this;
		}
	};
#else
	using VertexASBuffer = ASBuffer;
	using IndexASBuffer = ASBuffer;
#endif
	class HostVisibleASBuffer : public HostVisibleBuffer
	{
	private:
		using Super = HostVisibleBuffer;
	public:
		HostVisibleASBuffer& Create(const VkDevice Dev, const VkPhysicalDeviceMemoryProperties& PDMP, const size_t Size, const void* Source = nullptr) {
			Super::Create(Dev, PDMP, Size, VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, Source);
			return *this;
		}
	};
	class AccelerationStructureBuffer : public DeviceLocalBuffer
	{
	private:
		using Super = DeviceLocalBuffer;
	public:
		VkAccelerationStructureKHR AccelerationStructure;
		AccelerationStructureBuffer& Create(const VkDevice Dev, const VkPhysicalDeviceMemoryProperties& PDMP, const VkAccelerationStructureTypeKHR Type, const size_t Size) {
			Super::Create(Dev, PDMP, Size, VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT);
			const VkAccelerationStructureCreateInfoKHR ASCI = {
				.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR,
				.pNext = nullptr,
				.createFlags = 0,
				.buffer = Buffer,
				.offset = 0,
				.size = Size,
				.type = Type,
				.deviceAddress = 0
			};
			vkCreateAccelerationStructureKHR(Dev, &ASCI, GetAllocationCallbacks(), &AccelerationStructure);
			return *this;
		}
		void Destroy(const VkDevice Dev) {
			if (VK_NULL_HANDLE != AccelerationStructure) { vkDestroyAccelerationStructureKHR(Dev, AccelerationStructure, GetAllocationCallbacks()); }
			Super::Destroy(Dev);
		}
		void PopulateBuildCommand(const VkAccelerationStructureBuildGeometryInfoKHR& ASBGI, const std::vector<VkAccelerationStructureBuildRangeInfoKHR>& ASBRIs, const VkCommandBuffer CB, const VkQueryPool QP = VK_NULL_HANDLE) {
			assert(ASBGI.geometryCount == size(ASBRIs));
			const std::array ASBGIs = { ASBGI };
			const std::array ASBRIss = { data(ASBRIs) };
			vkCmdBuildAccelerationStructuresKHR(CB, static_cast<uint32_t>(size(ASBGIs)), data(ASBGIs), data(ASBRIss));

			//!< QueryPool ���w�肳�ꂽ�ꍇ�� COMPACTED_SIZE �v���p�e�B�� QueryPool �֏�������
			if (VK_NULL_HANDLE != QP) {
				vkCmdResetQueryPool(CB, QP, 0, 1);
				const std::array ASs = { AccelerationStructure };
				vkCmdWriteAccelerationStructuresPropertiesKHR(CB, static_cast<uint32_t>(size(ASs)), data(ASs), VK_QUERY_TYPE_ACCELERATION_STRUCTURE_COMPACTED_SIZE_KHR, QP, 0);
			}
		}
		void SubmitBuildCommand(const VkDevice Dev, const VkPhysicalDeviceMemoryProperties& PDMP, const VkDeviceSize Size, VkAccelerationStructureBuildGeometryInfoKHR ASBGI, const std::vector<VkAccelerationStructureBuildRangeInfoKHR>& ASBRIs, VkQueue Queue, const VkCommandBuffer CB, const VkQueryPool QP = VK_NULL_HANDLE) {
			Scoped<ScratchBuffer> Scratch(Dev);
			Scratch.Create(Dev, PDMP, Size);
			ASBGI.dstAccelerationStructure = AccelerationStructure;
			ASBGI.scratchData = VkDeviceOrHostAddressKHR({ .deviceAddress = GetDeviceAddress(Dev, Scratch.Buffer) });
			constexpr VkCommandBufferBeginInfo CBBI = { .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO, .pNext = nullptr, .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, .pInheritanceInfo = nullptr };
			VERIFY_SUCCEEDED(vkBeginCommandBuffer(CB, &CBBI)); {
				PopulateBuildCommand(ASBGI, ASBRIs, CB, QP);
			} VERIFY_SUCCEEDED(vkEndCommandBuffer(CB));
			SubmitAndWait(Queue, CB);
		}
		void PopulateBufferMemoryBarrierCommand(const VkCommandBuffer CB) {
			BufferMemoryBarrier(CB,
				VK_PIPELINE_STAGE_2_ACCELERATION_STRUCTURE_BUILD_BIT_KHR, VK_PIPELINE_STAGE_2_ACCELERATION_STRUCTURE_BUILD_BIT_KHR,
				Buffer,
				VK_ACCESS_2_ACCELERATION_STRUCTURE_READ_BIT_KHR | VK_ACCESS_2_ACCELERATION_STRUCTURE_WRITE_BIT_KHR, VK_ACCESS_2_ACCELERATION_STRUCTURE_READ_BIT_KHR);
		}
		static void PopulateMemoryBarrierCommand(const VkCommandBuffer CB) {
#ifdef USE_SYNCHRONIZATION2
			constexpr std::array MBs = {
				VkMemoryBarrier2({
					.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER_2,
					.pNext = nullptr,
					.srcStageMask = VK_PIPELINE_STAGE_2_ACCELERATION_STRUCTURE_BUILD_BIT_KHR, .srcAccessMask = VK_ACCESS_2_ACCELERATION_STRUCTURE_WRITE_BIT_KHR,
					.dstStageMask = VK_PIPELINE_STAGE_2_ACCELERATION_STRUCTURE_BUILD_BIT_KHR, .dstAccessMask = VK_ACCESS_2_ACCELERATION_STRUCTURE_READ_BIT_KHR
				}), 
			};
			constexpr std::array<VkBufferMemoryBarrier2, 0> BMBs = {};
			constexpr std::array<VkImageMemoryBarrier2, 0> IMBs = {};
			const VkDependencyInfo DI = {
				.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
				.pNext = nullptr,
				.dependencyFlags = 0,
				.memoryBarrierCount = static_cast<uint32_t>(std::size(MBs)), .pMemoryBarriers = std::data(MBs),
				.bufferMemoryBarrierCount = static_cast<uint32_t>(std::size(BMBs)),.pBufferMemoryBarriers = std::data(BMBs),
				.imageMemoryBarrierCount = static_cast<uint32_t>(std::size(IMBs)), .pImageMemoryBarriers = std::data(IMBs),
			};
			vkCmdPipelineBarrier2(CB, &DI);
#else
			constexpr std::array MBs = {
				VkMemoryBarrier({
					.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER,
					.pNext = nullptr,
					.srcAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_KHR,
					.dstAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR
				}),
			};
			constexpr std::array<VkBufferMemoryBarrier, 0> BMBs = {};
			constexpr std::array<VkImageMemoryBarrier, 0> IMBs = {};
			vkCmdPipelineBarrier(CB,
				VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR, VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR,
				0,
				static_cast<uint32_t>(std::size(MBs)), std::data(MBs),
				static_cast<uint32_t>(std::size(BMBs)), std::data(BMBs),
				static_cast<uint32_t>(std::size(IMBs)), std::data(IMBs));
#endif
		}
	};
	class BLAS : public AccelerationStructureBuffer
	{
	private:
		using Super = AccelerationStructureBuffer;
	public:
		BLAS& Create(const VkDevice Dev, const VkPhysicalDeviceMemoryProperties& PDMP, const size_t Size) {
			Super::Create(Dev, PDMP, VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR, Size);
			return *this;
		}
		void PopulateMemoryBarrierCommand(const VkCommandBuffer CB) {
			BufferMemoryBarrier(CB,
				VK_PIPELINE_STAGE_2_ACCELERATION_STRUCTURE_BUILD_BIT_KHR, VK_PIPELINE_STAGE_2_ACCELERATION_STRUCTURE_BUILD_BIT_KHR,
				Buffer,
				VK_ACCESS_2_MEMORY_WRITE_BIT, VK_ACCESS_2_MEMORY_READ_BIT);
		}
		void PopulateCopyCommand(const VkAccelerationStructureKHR& Src, const VkAccelerationStructureKHR& Dst, const VkCommandBuffer CB) {
			const VkCopyAccelerationStructureInfoKHR CASI = {
				.sType = VK_STRUCTURE_TYPE_COPY_ACCELERATION_STRUCTURE_INFO_KHR,
				.pNext = nullptr,
				.src = Src, .dst = Dst,
				.mode = VK_COPY_ACCELERATION_STRUCTURE_MODE_COMPACT_KHR
			};
			vkCmdCopyAccelerationStructureKHR(CB, &CASI);
		}
		void SubmitCopyCommand(const VkAccelerationStructureKHR& Src, const VkAccelerationStructureKHR& Dst, const VkQueue Queue, const VkCommandBuffer CB) {
			constexpr VkCommandBufferBeginInfo CBBI = { .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO, .pNext = nullptr, .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, .pInheritanceInfo = nullptr };
			VERIFY_SUCCEEDED(vkBeginCommandBuffer(CB, &CBBI)); {
				PopulateCopyCommand(Src, Dst, CB);
			} VERIFY_SUCCEEDED(vkEndCommandBuffer(CB));
			SubmitAndWait(Queue, CB);
		}
	};
	class TLAS : public AccelerationStructureBuffer
	{
	private:
		using Super = AccelerationStructureBuffer;
	public:
		TLAS& Create(const VkDevice Dev, const VkPhysicalDeviceMemoryProperties& PDMP, const size_t Size) {
			Super::Create(Dev, PDMP, VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR, Size);
			return *this;
		}
	};
	class ScratchBuffer : public DeviceLocalBuffer
	{
	private:
		using Super = DeviceLocalBuffer;
	public:
		void Create(const VkDevice Dev, const VkPhysicalDeviceMemoryProperties& PDMP, const size_t Size) {
			Super::Create(Dev, PDMP, Size, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT);
		}
	};
	class ShaderBindingTable : public BufferMemory
	{
	private:
		using Super = BufferMemory;
	public:
		//!< �����͔C�ӂ����AGen, Miss, Hit, Call �p�� 4 �� 
		//!< (������ vkCmdTraceRaysKHR(), vkCmdTraceRaysIndirectKHR() �ɂ��̂܂܂̏����œn�������ꍇ�� Gen, Miss, Hit, Call �̏��ɂ��邱��)
		//!< �g�p���Ȃ��V�F�[�_�̉ӏ��̓[���N���A (.deviceAddress = 0, .stride = 0, .size = 0) ���Ă�������
		std::array<VkStridedDeviceAddressRegionKHR, 4> StridedDeviceAddressRegions = {
			VkStridedDeviceAddressRegionKHR({.deviceAddress = 0, .stride = 0, .size = 0 }),
			VkStridedDeviceAddressRegionKHR({.deviceAddress = 0, .stride = 0, .size = 0 }), 
			VkStridedDeviceAddressRegionKHR({.deviceAddress = 0, .stride = 0, .size = 0 }), 
			VkStridedDeviceAddressRegionKHR({.deviceAddress = 0, .stride = 0, .size = 0 }),
		};
		ShaderBindingTable& Create(const VkDevice Dev, const VkPhysicalDeviceMemoryProperties& PDMP) {
			//!< �O���[�v�̑��T�C�Y
			const auto Size = std::accumulate(begin(StridedDeviceAddressRegions), end(StridedDeviceAddressRegions), 0, [](int Acc, const auto& lhs) { return Acc + static_cast<int>(lhs.size); });
			VK::CreateBufferMemory(&Buffer, &DeviceMemory, Dev, PDMP, Size, VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

			Win::Log("StridedDeviceAddressRegions\n");
			//!< �f�o�C�X�A�h���X�̌���
			auto da = GetDeviceAddress(Dev, Buffer);
			for (auto& i : StridedDeviceAddressRegions) {
				if (i.size) {
					i.deviceAddress = da;
					da += i.size;
					Win::Logf("\tdeviceAddress = %d, size = %d, stride = %d\n", i.deviceAddress, i.size, i.stride);
				}
			}

			return *this;
		}
		void* Map(const VkDevice Dev) {
			void* Data;
			VERIFY_SUCCEEDED(vkMapMemory(Dev, DeviceMemory, 0, VK_WHOLE_SIZE, static_cast<VkMemoryMapFlags>(0), &Data));
			return Data;
		}
		void Unmap(const VkDevice Dev) {
			vkUnmapMemory(Dev, DeviceMemory);
		}
	};
#pragma endregion

	std::vector<BLAS> BLASs;
	std::vector<TLAS> TLASs;
	std::vector<ShaderBindingTable> ShaderBindingTables;

	VkPhysicalDeviceRayTracingPipelinePropertiesKHR PDRTPP = { .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_PROPERTIES_KHR, .pNext = nullptr };
};

