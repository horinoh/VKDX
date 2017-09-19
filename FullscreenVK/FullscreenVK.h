#pragma once

#include "resource.h"

#pragma region Code
#include "../VKExt.h"

#define USE_DRAW_INDIRECT

class FullscreenVK : public VKExt
{
private:
	using Super = VKExt;
public:
	FullscreenVK() : Super() {}
	virtual ~FullscreenVK() {}

protected:
#ifdef USE_DRAW_INDIRECT
	virtual void CreateIndirectBuffer() override { CreateIndirectBuffer_4Vertices(); }
#endif

	virtual void CreateShader(std::vector<VkShaderModule>& ShaderModules, std::vector<VkPipelineShaderStageCreateInfo>& PipelineShaderStageCreateInfos) const override {
		CreateShader_VsPs(ShaderModules, PipelineShaderStageCreateInfos);
	}
	virtual void CreatePipeline() override { CreatePipeline_Graphics(); }
	virtual void PopulateCommandBuffer(const VkCommandBuffer CommandBuffer, const VkFramebuffer Framebuffer, const VkImage Image, const VkClearColorValue& Color) override;
};
#pragma endregion