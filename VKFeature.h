#pragma once

#include <vector>
#include <vulkan/vulkan.h>

class CoreFeature
{
public:
	virtual void* GetPtr() { return &PDV13F; }

	VkPhysicalDeviceVulkan11Features PDV11F = {
		.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES,
		.pNext = nullptr,
		.storageBuffer16BitAccess = VK_FALSE,
		.uniformAndStorageBuffer16BitAccess = VK_FALSE,
		.storagePushConstant16 = VK_FALSE,
		.storageInputOutput16 = VK_FALSE,
		.multiview = VK_FALSE,
		.multiviewGeometryShader = VK_FALSE,
		.multiviewTessellationShader = VK_FALSE,
		.variablePointersStorageBuffer = VK_FALSE,
		.variablePointers = VK_FALSE,
		.protectedMemory = VK_FALSE,
		.samplerYcbcrConversion = VK_FALSE,
		.shaderDrawParameters = VK_FALSE
	};
	VkPhysicalDeviceVulkan12Features PDV12F = {
		.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES,
		.pNext = &PDV11F,
		.samplerMirrorClampToEdge = VK_FALSE,
		.drawIndirectCount = VK_FALSE,
		.storageBuffer8BitAccess = VK_FALSE,
		.uniformAndStorageBuffer8BitAccess = VK_FALSE,
		.storagePushConstant8 = VK_FALSE,
		.shaderBufferInt64Atomics = VK_FALSE,
		.shaderSharedInt64Atomics = VK_FALSE,
		.shaderFloat16 = VK_FALSE,
		.shaderInt8 = VK_FALSE,
		.descriptorIndexing = VK_FALSE,
		.shaderInputAttachmentArrayDynamicIndexing = VK_FALSE,
		.shaderUniformTexelBufferArrayDynamicIndexing = VK_FALSE,
		.shaderStorageTexelBufferArrayDynamicIndexing = VK_FALSE,
		.shaderUniformBufferArrayNonUniformIndexing = VK_FALSE,
		.shaderSampledImageArrayNonUniformIndexing = VK_FALSE,
		.shaderStorageBufferArrayNonUniformIndexing = VK_FALSE,
		.shaderStorageImageArrayNonUniformIndexing = VK_FALSE,
		.shaderInputAttachmentArrayNonUniformIndexing = VK_FALSE,
		.shaderUniformTexelBufferArrayNonUniformIndexing = VK_FALSE,
		.shaderStorageTexelBufferArrayNonUniformIndexing = VK_FALSE,
		.descriptorBindingUniformBufferUpdateAfterBind = VK_FALSE,
		.descriptorBindingSampledImageUpdateAfterBind = VK_FALSE,
		.descriptorBindingStorageImageUpdateAfterBind = VK_FALSE,
		.descriptorBindingStorageBufferUpdateAfterBind = VK_FALSE,
		.descriptorBindingUniformTexelBufferUpdateAfterBind = VK_FALSE,
		.descriptorBindingStorageTexelBufferUpdateAfterBind = VK_FALSE,
		.descriptorBindingUpdateUnusedWhilePending = VK_FALSE,
		.descriptorBindingPartiallyBound = VK_FALSE,
		.descriptorBindingVariableDescriptorCount = VK_FALSE,
		.runtimeDescriptorArray = VK_FALSE,
		.samplerFilterMinmax = VK_FALSE,
		.scalarBlockLayout = VK_FALSE,
		.imagelessFramebuffer = VK_FALSE,
		.uniformBufferStandardLayout = VK_FALSE,
		.shaderSubgroupExtendedTypes = VK_FALSE,
		.separateDepthStencilLayouts = VK_FALSE,
		.hostQueryReset = VK_FALSE,
		.timelineSemaphore = VK_TRUE,
		.bufferDeviceAddress = VK_TRUE,
		.bufferDeviceAddressCaptureReplay = VK_FALSE,
		.bufferDeviceAddressMultiDevice = VK_FALSE,
		.vulkanMemoryModel = VK_FALSE,
		.vulkanMemoryModelDeviceScope = VK_FALSE,
		.vulkanMemoryModelAvailabilityVisibilityChains = VK_FALSE,
		.shaderOutputViewportIndex = VK_FALSE,
		.shaderOutputLayer = VK_FALSE,
		.subgroupBroadcastDynamicId = VK_FALSE,
	};
	VkPhysicalDeviceVulkan13Features PDV13F = {
		.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
		.pNext = &PDV12F,
		.robustImageAccess = VK_FALSE,
		.inlineUniformBlock = VK_FALSE,
		.descriptorBindingInlineUniformBlockUpdateAfterBind = VK_FALSE,
		.pipelineCreationCacheControl = VK_FALSE,
		.privateData = VK_FALSE,
		.shaderDemoteToHelperInvocation = VK_FALSE,
		.shaderTerminateInvocation = VK_FALSE,
		.subgroupSizeControl = VK_FALSE,
		.computeFullSubgroups = VK_FALSE,
		.synchronization2 = VK_TRUE,
		.textureCompressionASTC_HDR = VK_FALSE,
		.shaderZeroInitializeWorkgroupMemory = VK_FALSE,
		.dynamicRendering = VK_TRUE,
		.shaderIntegerDotProduct = VK_FALSE,
		.maintenance4 = VK_TRUE,
	};
	std::vector<const char*> ExtNames = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};
};

class VKFeature : public CoreFeature 
{
private:
	using Super = CoreFeature;
public:
	VKFeature() {
		//!< Šg’£–¼‚ð’Ç‰Á‚·‚é•K—v‚ª‚ ‚é
		ExtNames.emplace_back(VK_EXT_NESTED_COMMAND_BUFFER_EXTENSION_NAME);
	}
	
	virtual void* GetPtr() override { return &PDNCBF; }

	VkPhysicalDeviceNestedCommandBufferFeaturesEXT PDNCBF = {
		.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_NESTED_COMMAND_BUFFER_FEATURES_EXT,
		.pNext = Super::GetPtr(),
		.nestedCommandBuffer = VK_TRUE,
		.nestedCommandBufferRendering = VK_TRUE,
		.nestedCommandBufferSimultaneousUse = VK_TRUE
	};
};

class RTFeature : public VKFeature
{
private:
	using Super = VKFeature;
public:
	RTFeature() {
		ExtNames.emplace_back(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME);
		ExtNames.emplace_back(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
		ExtNames.emplace_back(VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME);
	}

	virtual void* GetPtr() override { return &PDASF; }

	VkPhysicalDeviceRayTracingPipelineFeaturesKHR PDRTPF = {
		.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR,
		.pNext = Super::GetPtr(),
		.rayTracingPipeline = VK_TRUE,
		.rayTracingPipelineTraceRaysIndirect = VK_TRUE
	};
	VkPhysicalDeviceAccelerationStructureFeaturesKHR PDASF = {
		.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR,
		.pNext = &PDRTPF,
		.accelerationStructure = VK_TRUE
	};
};

class MSFeature : public VKFeature
{
private:
	using Super = VKFeature;
public:
	MSFeature() {
#ifdef USE_NV_MESH_SHADER
		ExtNames.emplace_back(VK_NV_MESH_SHADER_EXTENSION_NAME);
#else
		ExtNames.emplace_back(VK_EXT_MESH_SHADER_EXTENSION_NAME);
#endif
	}

	virtual void* GetPtr() override { return &PDMSF; }

#ifdef USE_NV_MESH_SHADER
	VkPhysicalDeviceMeshShaderFeaturesNV PDMSF = {
		.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_FEATURES_NV,
		.pNext = Super::GetPtr(),
		.taskShader = VK_TRUE,
		.meshShader = VK_TRUE
	};
#else
	VkPhysicalDeviceMeshShaderFeaturesEXT PDMSF = {
		.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_FEATURES_EXT,
		.pNext = Super::GetPtr(),
		.taskShader = VK_TRUE,
		.meshShader = VK_TRUE,
		.multiviewMeshShader = VK_FALSE,
		.primitiveFragmentShadingRateMeshShader = VK_FALSE,
		.meshShaderQueries = VK_FALSE
	};
#endif
};