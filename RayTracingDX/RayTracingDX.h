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
	virtual void CreateDevice(HWND hWnd) override {
		Super::CreateDevice(hWnd);
		D3D12_FEATURE_DATA_D3D12_OPTIONS5 FDO5;
		VERIFY_SUCCEEDED(Device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS5, reinterpret_cast<void*>(&FDO5), sizeof(FDO5)));
		assert(D3D12_RAYTRACING_TIER_NOT_SUPPORTED != FDO5.RaytracingTier && "Raytracing not supported");
	}

	virtual void CreateGeometry() override;
	virtual void CreateTexture() override;
	virtual void CreateRootSignature() override;
	virtual void CreateShaderBlob() override;
	virtual void CreatePipelineState() override;
#endif
};
#pragma endregion