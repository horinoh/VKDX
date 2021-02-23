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
	virtual void CreateDevice(HWND hWnd) override { Super::CreateDevice(hWnd); assert(HasRaytracingSupport(COM_PTR_GET(Device)) && "Raytracing is not supported"); }

	virtual void CreateGeometry() override;
	virtual void CreateTexture() override;
	virtual void CreateRootSignature() override;
	virtual void CreatePipelineState() override;
#pragma endregion
};
#pragma endregion