#pragma once

#include "resource.h"

#pragma region Code
#include "../VKExt.h"

class FullscreenVK : public VKExt
{
private:
	using Super = VKExt;
public:
	FullscreenVK() : Super() {}
	virtual ~FullscreenVK() {}

protected:
	virtual void CreateShader(std::vector<VkShaderModule>& ShaderModules, std::vector<VkPipelineShaderStageCreateInfo>& PipelineShaderStageCreateInfos) const override {
		CreateShader_VsPs(ShaderModules, PipelineShaderStageCreateInfos);
	}
	virtual void CreatePipeline() override { CreateGraphicsPipeline(); }
	virtual void PopulateCommandBuffer(const VkCommandBuffer CommandBuffer) override;
};
#pragma endregion