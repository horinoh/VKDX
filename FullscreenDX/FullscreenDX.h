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
	virtual void CreateIndirectBuffer() override { CreateIndirectBuffer_Draw(4, 1); }
#endif
	virtual void CreateShaderBlobs() override { CreateShaderBlob_VsPs(); }
	virtual void CreatePipelineStates() override { CreatePipelineState_VsPs(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE); }
	virtual void PopulateCommandList(const size_t i) override;
};
#pragma endregion