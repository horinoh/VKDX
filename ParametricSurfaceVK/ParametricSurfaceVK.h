#pragma once

#include "resource.h"

#pragma region Code
#include "../VKExt.h"

class ParametricSurfaceVK : public VKExt
{
private:
	using Super = VKExt;
public:
	ParametricSurfaceVK() : VKExt() {}
	virtual ~ParametricSurfaceVK() {}

protected:
	virtual void CreateVertexInput() override { CreateVertexInput_Position(); }
	virtual void CreateShader(std::vector<VkShaderModule>& ShaderModules, std::vector<VkPipelineShaderStageCreateInfo>& PipelineShaderStageCreateInfos) const override {
		CreateShader_VsPsTesTcsGs(ShaderModules, PipelineShaderStageCreateInfos);
	}
};
#pragma endregion