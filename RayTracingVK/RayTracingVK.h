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

#pragma region RAYTRACING
	virtual void CreateDevice(VkPhysicalDevice PD, VkSurfaceKHR S) override {
		Super::CreateDevice(PD, S);
	}
	virtual void CreateGeometry() override;
	virtual void CreateTexture() override;
	virtual void CreatePipelineLayout() override;
	virtual void CreatePipeline() override;
#pragma endregion
};
#pragma endregion
