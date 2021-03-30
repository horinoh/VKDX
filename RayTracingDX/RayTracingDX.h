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

#pragma region RAYTRACING
	virtual void CreateGeometry() override;
	virtual void CreateTexture() override;
	virtual void CreateRootSignature() override;
	virtual void CreatePipelineState() override;
	virtual void PopulateCommandList(const size_t i) override;
#pragma endregion
};
#pragma endregion