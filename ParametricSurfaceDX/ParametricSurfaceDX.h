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
	virtual void CreateIndirectBuffer() override { CreateIndirectBuffer_DrawIndexed(1); } //!< �Œ�ł��C���f�b�N�X��1���K�v (At least index count must be 1)
	virtual void CreateShaderBlob() override { CreateShaderBlob_VsPsDsHsGs(); }
	virtual void CreatePipelineState() override { CreatePipelineState_VsPsDsHsGs_Tesselation(); }
	virtual void PopulateCommandList(const size_t i) override;
};
#pragma endregion