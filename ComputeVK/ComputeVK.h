#pragma once

#include "resource.h"

#pragma region Code
#include "../VKExt.h"

class ComputeVK : public VKExt
{
private:
	using Super = VKExt;
public:
	ComputeVK() : Super() {}
	virtual ~ComputeVK() {}

protected:

	//!< #VK_TODO 出力テクスチャ用のimage2Dを用意しないとならない

	virtual void CreatePipeline() override { Super::CreatePipeline_Compute(); }
	
	virtual void CreateIndirectBuffer() override { CreateIndirectBuffer_Dispatch(32, 1, 1); }
	virtual void CreateShader(std::vector<VkShaderModule>& ShaderModules, std::vector<VkPipelineShaderStageCreateInfo>& PipelineShaderStageCreateInfos) const override {
		CreateShader_Cs(ShaderModules, PipelineShaderStageCreateInfos);
	}
	virtual void PopulateCommandBuffer(const size_t i) override;

	virtual void Draw() override { Dispatch(); }

private:
};
#pragma endregion