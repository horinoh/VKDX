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
	virtual void CreateIndirectBuffer() override { CreateIndirectBuffer_DrawIndexed(1, 1); } //!< �Œ�ł��C���f�b�N�X��1���K�v (At least index count must be 1)
	virtual void CreateShaderBlob() override { CreateShaderBlob_VsPsDsHsGs(); }
	virtual void CreatePipelineState() override { PipelineStates.resize(1); CreatePipelineState_VsPsDsHsGs_Tesselation(PipelineStates[0]); }
	virtual void PopulateCommandList(const size_t i) override;
};
#pragma endregion