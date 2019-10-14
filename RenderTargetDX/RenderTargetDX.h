#pragma once

#include "resource.h"

#pragma region Code
#include "../DXExt.h"

class RenderTargetDX : public DXExt
{
private:
	using Super = DXExt;
public:
	RenderTargetDX() : Super() {}
	virtual ~RenderTargetDX() {}

protected:
	virtual void CreateIndirectBuffer() override { CreateIndirectBuffer_DrawIndexed(1, 1); }
	virtual void CreateShaderBlob() override { CreateShaderBlob_VsPsDsHsGs(); }
	virtual void CreatePipelineState() override {
		PipelineStates.resize(2);
		CreatePipelineState_VsPsDsHsGs_Tesselation(PipelineStates[0]);
		//CreatePipelineState_VsPs(PipelineStates[1]);
	}
	virtual void PopulateCommandList(const size_t i) override;
};
#pragma endregion