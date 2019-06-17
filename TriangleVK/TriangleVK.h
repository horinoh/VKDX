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

	virtual void CreateShader(std::vector<VkShaderModule>& ShaderModules, std::vector<VkPipelineShaderStageCreateInfo>& PipelineShaderStageCreateInfos) const override {
		CreateShader_VsPs(ShaderModules, PipelineShaderStageCreateInfos);
	}
	virtual void CreateVertexInput(std::vector<VkVertexInputBindingDescription>& VertexInputBindingDescriptions, std::vector<VkVertexInputAttributeDescription>& VertexInputAttributeDescriptions) const override {
		CreateVertexInputBinding<Vertex_PositionColor>(VertexInputBindingDescriptions, VertexInputAttributeDescriptions, 0);
	}
	virtual void CreatePipelineLayout() override;
	virtual void PopulateCommandBuffer(const size_t i) override;
};
#pragma endregion