#pragma once

#include "VKExt.h"

class VKRT : public VKExt
{
private:
	using Super = VKExt;
protected:
	virtual void OnDestroy(HWND hWnd, HINSTANCE hInstance) override {	
		for (auto i : ShaderBindingTables) { i.Destroy(Device); } ShaderBindingTables.clear();
		for (auto i : BLASs) { i.Destroy(Device); } BLASs.clear();
		for (auto i : TLASs) { i.Destroy(Device); } TLASs.clear();

		Super::OnDestroy(hWnd, hInstance);
	}

	virtual void CreateInstance([[maybe_unused]] const std::vector<const char*>& AdditionalLayers, const std::vector<const char*>& AdditionalExtensions) override {
		//Super::CreateInstance(AdditionalLayers, AdditionalExtensions); //!< VK_LAYER_RENDERDOC_Capture を使用する
		VK::CreateInstance(AdditionalLayers, AdditionalExtensions); //!< VK_LAYER_RENDERDOC_Capture を使用しない
	}
	virtual void CreateDevice(HWND hWnd, HINSTANCE hInstance, [[maybe_unused]] void* pNext, [[maybe_unused]] const std::vector<const char*>& AddExtensions) override {
		if (HasRayTracingSupport(GetCurrentPhysicalDevice())) {
#ifdef _DEBUG
#if true
			assert(VK_HEADER_VERSION_COMPLETE >= VK_MAKE_VERSION(1, 2, 162) && "RayTracing require 1.2.162 or later");
#else
			uint32_t APIVersion; VERIFY_SUCCEEDED(vkEnumerateInstanceVersion(&APIVersion)); assert(APIVersion >= VK_MAKE_VERSION(1, 2, 162) && "RayTracing require 1.2.162 or later");
#endif
#endif
			//!< VkPhysicalDeviceRayTracingPipelinePropertiesKHR はよく使うので覚えておく
			VkPhysicalDeviceProperties2 PDP2 = { .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2, .pNext = &PDRTPP, };
			vkGetPhysicalDeviceProperties2(GetCurrentPhysicalDevice(), &PDP2);

			VkPhysicalDeviceBufferDeviceAddressFeatures PDBDAF = { .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES, .pNext = nullptr, .bufferDeviceAddress = VK_TRUE };
			VkPhysicalDeviceRayTracingPipelineFeaturesKHR PDRTPF = { .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR, .pNext = &PDBDAF, .rayTracingPipeline = VK_TRUE, .rayTracingPipelineTraceRaysIndirect = VK_TRUE };
			VkPhysicalDeviceAccelerationStructureFeaturesKHR PDASF = { .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR, .pNext = &PDRTPF, .accelerationStructure = VK_TRUE };
			VkPhysicalDeviceDescriptorIndexingFeatures PDDIF = { .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES, .pNext = &PDASF, .shaderUniformBufferArrayNonUniformIndexing = VK_TRUE, .shaderSampledImageArrayNonUniformIndexing = VK_TRUE, .runtimeDescriptorArray = VK_TRUE };
			Super::CreateDevice(hWnd, hInstance, &PDDIF/*PDASF*/, {
				VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME,
				VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME,
				VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME,
				VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME,

				VK_KHR_SHADER_FLOAT_CONTROLS_EXTENSION_NAME,
				
				//!< ↓VK1.2以降ではコア機能に昇格している
				VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME,
				VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME,
				VK_KHR_SPIRV_1_4_EXTENSION_NAME,
				});
		}
		else {
			Super::CreateDevice(hWnd, hInstance, pNext, AddExtensions);
		}
	}
	virtual void CreateSwapchain() override {
		//!< 最後にコピーするため VK_IMAGE_USAGE_TRANSFER_DST_BIT が必要
		VK::CreateSwapchain(GetCurrentPhysicalDevice(), Surface, GetClientRectWidth(), GetClientRectHeight(), VK_IMAGE_USAGE_TRANSFER_DST_BIT);
	}
	virtual void CreateTexture() override {
		if (!HasRayTracingSupport(GetCurrentPhysicalDevice())) { return; }
		//!< スワップチェインと同じカラーフォーマットにしておく、(レイアウトは変更したり戻したりするので、戻せるレイアウトにしておく(ここでは TRANSFER_SRC_OPTIMAL))
		StorageTextures.emplace_back().Create(Device, GetCurrentPhysicalDeviceMemoryProperties(), ColorFormat, VkExtent3D({ .width = static_cast<uint32_t>(GetClientRectWidth()), .height = static_cast<uint32_t>(GetClientRectHeight()), .depth = 1 }))
			.SubmitSetLayoutCommand(CommandBuffers[0], GraphicsQueue, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
	}
	virtual void CreatePipelineLayout() override {
		if (!HasRayTracingSupport(GetCurrentPhysicalDevice())) { return; }
		CreateDescriptorSetLayout(DescriptorSetLayouts.emplace_back(), 0, {
			//!< TLAS
			VkDescriptorSetLayoutBinding({.binding = 0, .descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, .descriptorCount = 1, .stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR, .pImmutableSamplers = nullptr }),
			//!< 出力 (StorageImage)
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
				.pNext = &WDSAS, //!< pNext に VkWriteDescriptorSetAccelerationStructureKHR を指定すること
				.dstSet = DescriptorSets[0],
				.dstBinding = 0, .dstArrayElement = 0,
				.descriptorCount = 1, .descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR,
				.pImageInfo = nullptr, .pBufferInfo = nullptr, .pTexelBufferView = nullptr
			}),
			//!< 出力 (StorageImage)
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

	[[nodiscard]] static bool HasRayTracingSupport(const VkPhysicalDevice PD) {
		VkPhysicalDeviceBufferDeviceAddressFeatures PDBDAF = { .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES, .pNext = nullptr };
		VkPhysicalDeviceRayTracingPipelineFeaturesKHR PDRTPF = { .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR, .pNext = &PDBDAF };
		VkPhysicalDeviceAccelerationStructureFeaturesKHR PDASF = { .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR, .pNext = &PDRTPF };
		VkPhysicalDeviceFeatures2 PDF2 = { .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2, .pNext = &PDASF };
		vkGetPhysicalDeviceFeatures2(PD, &PDF2);
		return PDBDAF.bufferDeviceAddress && PDRTPF.rayTracingPipeline && PDASF.accelerationStructure;
	}

#pragma region RAYTRACING
	//class DeviceLocalShaderDeviceStorageBuffer : public DeviceLocalBuffer
	//{
	//private:
	//	using Super = DeviceLocalBuffer;
	//public:
	//	DeviceLocalShaderDeviceStorageBuffer& Create(const VkDevice Device, const VkPhysicalDeviceMemoryProperties PDMP, const size_t Size) {
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
		ASBuffer& Create(const VkDevice Dev, const VkPhysicalDeviceMemoryProperties PDMP, const size_t Size) {
			Super::Create(Dev, PDMP, Size, Usage);
			return *this;
		}
	public:
		void PopulateCopyCommand(const VkCommandBuffer CB, const size_t Size, const VkBuffer Staging) {
			Super::PopulateCopyCommand(CB, Size, Staging, VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR, VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR);
		}
		void SubmitCopyCommand(const VkDevice Dev, const VkPhysicalDeviceMemoryProperties PDMP, const VkCommandBuffer CB, const VkQueue Queue, const size_t Size, const void* Source) {
			Super::SubmitCopyCommand(Dev, PDMP, CB, Queue, Size, Source, VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR, VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR);
		}
	};
	class VertexASBuffer : public ASBuffer
	{
	private:
		using Super = ASBuffer;
	public:
		VertexASBuffer& Create(const VkDevice Dev, const VkPhysicalDeviceMemoryProperties PDMP, const size_t Size) {
			DeviceLocalBuffer::Create(Dev, PDMP, Size, Usage | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
			return *this;
		}
	};
	class IndexASBuffer : public ASBuffer
	{
	private:
		using Super = ASBuffer;
	public:
		IndexASBuffer& Create(const VkDevice Dev, const VkPhysicalDeviceMemoryProperties PDMP, const size_t Size) {
			DeviceLocalBuffer::Create(Dev, PDMP, Size, Usage | VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
			return *this;
		}
	};
	class HostVisibleASBuffer : public HostVisibleBuffer
	{
	private:
		using Super = HostVisibleBuffer;
	public:
		HostVisibleASBuffer& Create(const VkDevice Dev, const VkPhysicalDeviceMemoryProperties PDMP, const size_t Size, const void* Source = nullptr) {
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
		AccelerationStructureBuffer& Create(const VkDevice Dev, const VkPhysicalDeviceMemoryProperties PDMP, const VkAccelerationStructureTypeKHR Type, const size_t Size) {
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

			//!< QueryPool が指定された場合は COMPACTED_SIZE プロパティを QueryPool へ書き込む
			if (VK_NULL_HANDLE != QP) {
				vkCmdResetQueryPool(CB, QP, 0, 1);
				const std::array ASs = { AccelerationStructure };
				vkCmdWriteAccelerationStructuresPropertiesKHR(CB, static_cast<uint32_t>(size(ASs)), data(ASs), VK_QUERY_TYPE_ACCELERATION_STRUCTURE_COMPACTED_SIZE_KHR, QP, 0);
			}
		}
		void SubmitBuildCommand(const VkDevice Dev, const VkPhysicalDeviceMemoryProperties PDMP, const VkDeviceSize Size, VkAccelerationStructureBuildGeometryInfoKHR ASBGI, const std::vector<VkAccelerationStructureBuildRangeInfoKHR>& ASBRIs, VkQueue Queue, const VkCommandBuffer CB, const VkQueryPool QP = VK_NULL_HANDLE) {
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
			constexpr std::array<VkMemoryBarrier, 0> MBs = {};
			const std::array BMBs = {
				VkBufferMemoryBarrier({
					.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
					.pNext = nullptr,
					.srcAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR | VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_KHR, .dstAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR,
					.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED, .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
					.buffer = Buffer, .offset = 0, .size = VK_WHOLE_SIZE
				}),
			};
			constexpr std::array<VkImageMemoryBarrier, 0> IMBs = {};
			vkCmdPipelineBarrier(CB,
				VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR, VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR,
				0,
				static_cast<uint32_t>(size(MBs)), data(MBs),
				static_cast<uint32_t>(size(BMBs)), data(BMBs),
				static_cast<uint32_t>(size(IMBs)), data(IMBs));
		}
		static void PopulateMemoryBarrierCommand(const VkCommandBuffer CB) {
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
				static_cast<uint32_t>(size(MBs)), data(MBs),
				static_cast<uint32_t>(size(BMBs)), data(BMBs),
				static_cast<uint32_t>(size(IMBs)), data(IMBs));
		}
	};
	class BLAS : public AccelerationStructureBuffer
	{
	private:
		using Super = AccelerationStructureBuffer;
	public:
		BLAS& Create(const VkDevice Dev, const VkPhysicalDeviceMemoryProperties PDMP, const size_t Size) {
			Super::Create(Dev, PDMP, VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR, Size);
			return *this;
		}
		void PopulateMemoryBarrierCommand(const VkCommandBuffer CB) {
			constexpr std::array<VkMemoryBarrier, 0> MBs = {};
			constexpr std::array<VkImageMemoryBarrier, 0> IMBs = {};
			const std::array BMBs = {
				VkBufferMemoryBarrier({
					.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
					.pNext = nullptr,
					.srcAccessMask = VK_ACCESS_MEMORY_WRITE_BIT, .dstAccessMask = VK_ACCESS_MEMORY_READ_BIT,
					.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED, .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
					.buffer = Buffer, .offset = 0, .size = VK_WHOLE_SIZE
				}),
			};
			vkCmdPipelineBarrier(CB, VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR, VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR, 0,
				static_cast<uint32_t>(size(MBs)), data(MBs),
				static_cast<uint32_t>(size(BMBs)), data(BMBs),
				static_cast<uint32_t>(size(IMBs)), data(IMBs));
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
		TLAS& Create(const VkDevice Dev, const VkPhysicalDeviceMemoryProperties PDMP, const size_t Size) {
			Super::Create(Dev, PDMP, VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR, Size);
			return *this;
		}
	};
	class ScratchBuffer : public DeviceLocalBuffer
	{
	private:
		using Super = DeviceLocalBuffer;
	public:
		void Create(const VkDevice Dev, const VkPhysicalDeviceMemoryProperties PDMP, const size_t Size) {
			Super::Create(Dev, PDMP, Size, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT);
		}
	};
	class ShaderBindingTable : public BufferMemory
	{
	private:
		using Super = BufferMemory;
	public:
		//!< 順序は決まっていないが、Gen, Miss, Hit, Call 用の 4 つ分
		std::array<VkStridedDeviceAddressRegionKHR, 4> StridedDeviceAddressRegions = {
			VkStridedDeviceAddressRegionKHR({.deviceAddress = 0, .stride = 0, .size = 0 }),
			VkStridedDeviceAddressRegionKHR({.deviceAddress = 0, .stride = 0, .size = 0 }), 
			VkStridedDeviceAddressRegionKHR({.deviceAddress = 0, .stride = 0, .size = 0 }), 
			VkStridedDeviceAddressRegionKHR({.deviceAddress = 0, .stride = 0, .size = 0 }),
		};
		ShaderBindingTable& Create(const VkDevice Dev, const VkPhysicalDeviceMemoryProperties PDMP) {
			//!< グループの総サイズ
			const auto Size = std::accumulate(begin(StridedDeviceAddressRegions), end(StridedDeviceAddressRegions), 0, [](int Acc, const auto& lhs) { return Acc + static_cast<int>(lhs.size); });
			VK::CreateBufferMemory(&Buffer, &DeviceMemory, Dev, PDMP, Size, VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

			Win::Log("StridedDeviceAddressRegions\n");
			//!< デバイスアドレスの決定
			auto da = GetDeviceAddress(Dev, Buffer);
			for (auto& i : StridedDeviceAddressRegions) {
				i.deviceAddress = da;
				da += i.size;
				if (i.size) {
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

