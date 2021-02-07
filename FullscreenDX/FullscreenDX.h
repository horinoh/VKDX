#pragma once

#include "resource.h"

#pragma region Code
#include "../DXExt.h"

class FullscreenDX : public DXExt
{
private:
	using Super = DXExt;
public:
	FullscreenDX() : Super() {}
	virtual ~FullscreenDX() {}

protected:
#ifdef USE_DRAW_INDIRECT
	virtual void CreateGeometry() override { CreateIndirectBuffer_Draw(4, 1); }
#endif
	
	virtual void CreateShaderBlobs() override {
#ifdef USE_DISTANCE_FUNCTION
		const auto ShaderPath = GetBasePath();
		ShaderBlobs.emplace_back(COM_PTR<ID3DBlob>());
		VERIFY_SUCCEEDED(D3DReadFileToBlob(data(ShaderPath + TEXT(".vs.cso")), COM_PTR_PUT(ShaderBlobs.back())));
		ShaderBlobs.emplace_back(COM_PTR<ID3DBlob>());
		VERIFY_SUCCEEDED(D3DReadFileToBlob(data(ShaderPath + TEXT("_df.ps.cso")), COM_PTR_PUT(ShaderBlobs.back())));
#else
		CreateShaderBlob_VsPs(); 
#endif
	}
	virtual void CreatePipelineState() override { CreatePipelineState_VsPs(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE, FALSE); }

	virtual void PopulateCommandList(const size_t i) override;
};
#pragma endregion