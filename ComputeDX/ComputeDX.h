#pragma once

#include "resource.h"

#pragma region Code
#include "../DXExt.h"

class ComputeDX : public DXExt
{
private:
	using Super = DXExt;
public:
	ComputeDX() : Super() {}
	virtual ~ComputeDX() {}

protected:
	virtual void Draw() override {}

	virtual void CreatePipelineState() override { Super::CreatePipelineState_Compute(); }

	//virtual void PopulateCommandList(ID3D12GraphicsCommandList* GraphicsCommandList) override;

private:
};
#pragma endregion
