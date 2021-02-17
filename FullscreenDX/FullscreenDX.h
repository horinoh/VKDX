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
	virtual void CreateGeometry() override { 
		constexpr D3D12_DRAW_ARGUMENTS DA = { .VertexCountPerInstance = 4, .InstanceCount = 1, .StartVertexLocation = 0, .StartInstanceLocation = 0 };
		IndirectBuffers.emplace_back().Create(COM_PTR_GET(Device), COM_PTR_GET(CommandAllocators[0]), COM_PTR_GET(GraphicsCommandLists[0]), COM_PTR_GET(CommandQueue), COM_PTR_GET(Fence), DA);
	}
#endif
	virtual void CreateShaderBlob() override {
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