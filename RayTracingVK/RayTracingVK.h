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
	virtual void CreateGeometry() override;
	virtual void CreatePipeline() override;
#endif
};
#pragma endregion
