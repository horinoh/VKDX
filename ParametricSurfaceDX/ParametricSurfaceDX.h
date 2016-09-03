#pragma once

#include "resource.h"

#pragma region Code
#include "../DXExt.h"

class ParametricSurfaceDX : public DXExt
{
private:
	using Super = DXExt;
public:
	ParametricSurfaceDX() : DXExt() {}
	virtual ~ParametricSurfaceDX() {}

protected:
	virtual void CreateInputLayout() override { CreateInputLayout_Position(); }
	virtual void CreateVertexBuffer(ID3D12CommandAllocator* CommandAllocator, ID3D12GraphicsCommandList* CommandList) override;
	virtual void CreateIndexBuffer(ID3D12CommandAllocator* CommandAllocator, ID3D12GraphicsCommandList* CommandList) override;	virtual void CreateGraphicsPipelineState() override { CreateGraphicsPipelineState_VsPsDsHsGs(); }

	virtual void PopulateCommandList(ID3D12GraphicsCommandList* GraphicsCommandList) override;

private:
};
#pragma endregion