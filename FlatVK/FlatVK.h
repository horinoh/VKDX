#pragma once

#include "resource.h"

#pragma region Code
#include "../VKExt.h"

class FlatVK : public VKExt
{
private:
	using Super = VKExt;
public:
	FlatVK() : Super() {}
	virtual ~FlatVK() {}

protected:
	virtual void OverridePhysicalDeviceFeatures(VkPhysicalDeviceFeatures& PhysicalDeviceFeatures) const { assert(PhysicalDeviceFeatures.tessellationShader && "tessellationShader not enabled"); }

	virtual void CreateIndirectBuffer() override { CreateIndirectBuffer_DrawIndexed(1); }

	virtual void CreateShader(std::vector<VkShaderModule>& ShaderModules, std::vector<VkPipelineShaderStageCreateInfo>& PipelineShaderStageCreateInfos) const override {
		CreateShader_VsPsTesTcsGs(ShaderModules, PipelineShaderStageCreateInfos);
	}

	virtual void CreateInputAssembly(VkPipelineInputAssemblyStateCreateInfo& PipelineInputAssemblyStateCreateInfo) const override {
		CreateInputAssembly_Topology(PipelineInputAssemblyStateCreateInfo, VK_PRIMITIVE_TOPOLOGY_PATCH_LIST);
	}
	virtual void CreateTessellationState(VkPipelineTessellationStateCreateInfo& PipelineTessellationStateCreateInfo) const override {
		const uint32_t PatchControlPoint = 1;
		PipelineTessellationStateCreateInfo = { VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO, nullptr, 0, PatchControlPoint };
	}

	virtual void PopulateCommandBuffer(const size_t i) override;
};
#pragma endregion