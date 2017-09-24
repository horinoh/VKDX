#pragma once

#include "resource.h"

#pragma region Code
#include "../VKExt.h"

class ParametricSurfaceVK : public VKExt
{
private:
	using Super = VKExt;
public:
	ParametricSurfaceVK() : Super() {}
	virtual ~ParametricSurfaceVK() {}

protected:
	virtual void OverridePhysicalDeviceFeatures(VkPhysicalDeviceFeatures& PhysicalDeviceFeatures) const { assert(PhysicalDeviceFeatures.tessellationShader && "tessellationShader not enabled"); }

	virtual void CreateVertexBuffer() override {}
	virtual void CreateIndexBuffer() override {}
	virtual void CreateShader(std::vector<VkShaderModule>& ShaderModules, std::vector<VkPipelineShaderStageCreateInfo>& PipelineShaderStageCreateInfos) const override {
		CreateShader_VsPsTesTcsGs(ShaderModules, PipelineShaderStageCreateInfos);
	}
	virtual void CreateInputAssembly(VkPipelineInputAssemblyStateCreateInfo& PipelineInputAssemblyStateCreateInfo) const override { CreateInputAssembly_PL(PipelineInputAssemblyStateCreateInfo); }
	//virtual void CreateVertexInput(std::vector<VkVertexInputBindingDescription>& VertexInputBindingDescriptions, std::vector<VkVertexInputAttributeDescription>& VertexInputAttributeDescriptions, const uint32_t Binding = 0) const {
	//	CreateVertexInputT<Vertex_Position>(VertexInputBindingDescriptions, VertexInputAttributeDescriptions, Binding);
	//}
	virtual void CreateTessellationState(VkPipelineTessellationStateCreateInfo& PipelineTessellationStateCreateInfo) const override {
		PipelineTessellationStateCreateInfo = {
			VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO,
			nullptr,
			0,
			4
		};
	}
};
#pragma endregion