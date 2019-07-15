#pragma once

#include "resource.h"

#pragma region Code
#include "../VKExt.h"

class TriangleVK : public VKExt
{
private:
	using Super = VKExt;
public:
	TriangleVK() : Super() {}
	virtual ~TriangleVK() {}

protected:
	virtual void CreateVertexBuffer() override;
	virtual void CreateIndexBuffer() override;
	virtual void CreateIndirectBuffer() override { CreateIndirectBuffer_DrawIndexed(IndexCount); }
	virtual void CreatePipelineLayout() override;
	virtual void CreatePipeline() override { CreatePipeline_VsFs_Vertex<Vertex_PositionColor>(); }
	virtual void PopulateCommandBuffer(const size_t i) override;
};
#pragma endregion