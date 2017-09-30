#pragma once

#include "resource.h"

#pragma region Code
#include "../VKExt.h"

#define USE_DRAW_INDIRECT

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
#ifdef USE_DRAW_INDIRECT
	virtual void CreateIndirectBuffer() override { CreateIndirectBuffer_Indexed(IndexCount); }
#endif

	virtual void CreateShader(std::vector<VkShaderModule>& ShaderModules, std::vector<VkPipelineShaderStageCreateInfo>& PipelineShaderStageCreateInfos) const override {
		CreateShader_VsPs(ShaderModules, PipelineShaderStageCreateInfos);
	}
	virtual void CreateVertexInput(std::vector<VkVertexInputBindingDescription>& VertexInputBindingDescriptions, std::vector<VkVertexInputAttributeDescription>& VertexInputAttributeDescriptions, const uint32_t Binding = 0) const override {
		CreateVertexInputT<Vertex_PositionColor>(VertexInputBindingDescriptions, VertexInputAttributeDescriptions, Binding);
	}

	virtual void PopulateCommandBuffer(const VkCommandBuffer CommandBuffer, const VkFramebuffer Framebuffer, const VkImage Image, const VkClearColorValue& Color) override;

private:
	using Vertex = struct Vertex { glm::vec3 Positon; glm::vec4 Color; };
};
#pragma endregion