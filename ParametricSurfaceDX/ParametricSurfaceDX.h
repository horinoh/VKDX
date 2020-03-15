#pragma once

#include "resource.h"

#pragma region Code
#include "../DXExt.h"

class ParametricSurfaceDX : public DXExt
{
private:
	using Super = DXExt;
public:
	ParametricSurfaceDX() : Super() {}
	virtual ~ParametricSurfaceDX() {}

protected:
#ifdef USE_BUNDLE
	virtual void CreateBundleCommandList() override { AddBundleCommandList(); }
#endif
	virtual void CreateIndirectBuffer() override { CreateIndirectBuffer_DrawIndexed(1, 1); } //!< 最低でもインデックス数1が必要 (At least index count must be 1)
	virtual void CreateShaderBlobs() override { CreateShaderBlob_VsPsDsHsGs(); }
	virtual void CreatePipelineStates() override { CreatePipelineState_VsPsDsHsGs(D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH); }
	virtual void PopulateCommandList(const size_t i) override;
};
#pragma endregion