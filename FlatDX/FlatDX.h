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
	virtual void CreateIndirectBuffer() override { CreateIndirectBuffer_DrawIndexed(1, 1); }
	virtual void CreateShaderBlob() override { CreateShaderBlob_VsPsDsHsGs(); }
	virtual void CreatePipelineState() override { CreatePipelineState_VsPsDsHsGs_Tesselation(); }
	virtual void PopulateCommandList(const size_t i) override;
};
#pragma endregion