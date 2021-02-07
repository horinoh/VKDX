#pragma once

#include "resource.h"

#pragma region Code
#include "../DXExt.h"

class RayTracingDX : public DXExt
{
private:
	using Super = DXExt;
public:
	RayTracingDX() : Super() {}
	virtual ~RayTracingDX() {}

#ifdef USE_RAYTRACING
	virtual void CreateGeometry() override;
	virtual void CreatePipelineState() override;
#endif
};
#pragma endregion