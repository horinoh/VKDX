#pragma once

#include "resource.h"

#pragma region Code
#include "../DX.h"

class ParametricSurfaceDX : public DX
{
private:
	using Super = DX;
public:
	ParametricSurfaceDX() : DX() {}
	virtual ~ParametricSurfaceDX() {}

protected:
	virtual void CreateShader() override { CreateShader_VsPsDsHsGs(); }
	//virtual void CreateInputLayout() override;
	virtual void CreateGraphicsPipelineState() override { CreateGraphicsPipelineState_VsPsDsHsGs(); }

	virtual void PopulateCommandList(ID3D12GraphicsCommandList* GraphicsCommandList) override;
private:
};
#pragma endregion