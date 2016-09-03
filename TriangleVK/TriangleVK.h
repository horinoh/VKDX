#pragma once

#include "resource.h"

#pragma region Code
#include "../VKExt.h"

class TriangleVK : public VKExt
{
private:
	using Super = VKExt;
public:
	TriangleVK() : VKExt() {}
	virtual ~TriangleVK() {}

protected:
	virtual void CreateVertexInput() override { CreateVertexInput_PositionColor(); }
	virtual void CreateVertexBuffer(const VkCommandBuffer CommandBuffer) override;
	virtual void CreateIndexBuffer(const VkCommandBuffer CommandBuffer) override;
	virtual void CreateGraphicsPipeline() override { CreateGraphicsPipeline_VsPs(); }

	virtual void PopulateCommandBuffer(const VkCommandBuffer CommandBuffer) override;

private:
	using Vertex = struct Vertex { glm::vec3 Positon; glm::vec4 Color; };
};
#pragma endregion