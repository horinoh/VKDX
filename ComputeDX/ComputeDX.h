#pragma once

#include "resource.h"

#pragma region Code
#include "../DX.h"

class ComputeDX : public DX
{
private:
	using Super = DX;
public:
	ComputeDX() : DX() {}
	virtual ~ComputeDX() {}

protected:
	virtual void Draw() override {}

	virtual void CreateShader() override { CreateShader_Cs(); }
	virtual void CreatePipelineState() override { Super::CreateComputePipelineState(); }

	//virtual void PopulateCommandList(ID3D12GraphicsCommandList* GraphicsCommandList) override;

private:
};
#pragma endregion
