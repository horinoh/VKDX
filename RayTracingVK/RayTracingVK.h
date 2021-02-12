#pragma once

#include "resource.h"

#pragma region Code
#include "../VKExt.h"

class RayTracingVK : public VKExt
{
private:
	using Super = VKExt;
public:
	RayTracingVK() : Super() {}
	virtual ~RayTracingVK() {}

#ifdef USE_RAYTRACING
	virtual void CreateDevice(VkPhysicalDevice PD, VkSurfaceKHR Surface) override {
		Super::CreateDevice(PD, Surface);
#ifdef VK_NO_PROTOYYPES
#define VK_DEVICE_PROC_ADDR(proc) vk ## proc = reinterpret_cast<PFN_vk ## proc>(vkGetDeviceProcAddr(Device, "vk" #proc)); assert(nullptr != vk ## proc && #proc && #proc);
		VK_DEVICE_PROC_ADDR(GetAccelerationStructureBuildSizesKHR)
		VK_DEVICE_PROC_ADDR(CreateAccelerationStructureKHR)
		VK_DEVICE_PROC_ADDR(CmdBuildAccelerationStructuresKHR)
		VK_DEVICE_PROC_ADDR(GetRayTracingShaderGroupHandlesKHR)
		VK_DEVICE_PROC_ADDR(CreateRayTracingPipelinesKHR)
		VK_DEVICE_PROC_ADDR(DestroyAccelerationStructureKHR)
#undef VK_DEVICE_PROC_ADDR
#endif
	}
	virtual void CreateGeometry() override;
	virtual void CreateTexture() override;
	virtual void CreatePipelineLayout() override;
	virtual void CreateShaderModule() override;
	virtual void CreatePipeline() override;
#endif
};
#pragma endregion
