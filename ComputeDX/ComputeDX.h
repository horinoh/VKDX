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
	virtual void CreatePipelineState() override { Super::CreatePipelineState_Compute(); }

	virtual void CreateIndirectBuffer() override { CreateIndirectBuffer_Dispatch(32, 1, 1); }

	virtual void PopulateCommandList(ID3D12GraphicsCommandList* CommandList, ID3D12Resource* SwapChainResource, const D3D12_CPU_DESCRIPTOR_HANDLE& DescriptorHandle);

	virtual void Draw() override { Dispatch(); }

private:
};
#pragma endregion
