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
	virtual void CreateGeometry() override { 
		constexpr D3D12_DRAW_INDEXED_ARGUMENTS DIA = { .IndexCountPerInstance = 1, .InstanceCount = 1, .StartIndexLocation = 0, .BaseVertexLocation = 0, .StartInstanceLocation = 0 };
		IndirectBuffers.emplace_back().Create(COM_PTR_GET(Device), COM_PTR_GET(CommandAllocators[0]), COM_PTR_GET(GraphicsCommandLists[0]), COM_PTR_GET(CommandQueue), COM_PTR_GET(Fence), DIA);
	}
	virtual void CreateShaderBlob() override { CreateShaderBlob_VsPsDsHsGs(); }
	virtual void CreatePipelineState() override { CreatePipelineState_VsPsDsHsGs(D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH, FALSE); }

	virtual void PopulateCommandList(const size_t i) override;
};
#pragma endregion