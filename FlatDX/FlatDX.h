#pragma once

#include "resource.h"

#pragma region Code
#include "../DXExt.h"

class FlatDX : public DXExt
{
private:
	using Super = DXExt;
public:
	FlatDX() : Super() {}
	virtual ~FlatDX() {}

protected:
	virtual void CreateBottomLevel() override { CreateIndirectBuffer_DrawIndexed(1, 1); }
	virtual void CreateShaderBlobs() override { CreateShaderBlob_VsPsDsHsGs(); }
	virtual void CreatePipelineStates() override { CreatePipelineState_VsPsDsHsGs(D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH, FALSE); }
	virtual void PopulateCommandList(const size_t i) override;
};
#pragma endregion