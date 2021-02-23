#pragma once

#include "resource.h"

#pragma region Code
#include "../VKExt.h"

class InstancingVK : public VKExt
{
private:
	using Super = VKExt;
public:
	InstancingVK() : Super() {}
	virtual ~InstancingVK() {}

protected:
	virtual void CreateGeometry() override;
	virtual void CreatePipelineLayout() override {
		VKExt::CreatePipelineLayout(PipelineLayouts.emplace_back(), {}, {});
	}
	virtual void CreatePipeline() override {
		const auto ShaderPath = GetBasePath();
		const std::array SMs = {
			VK::CreateShaderModule(data(ShaderPath + TEXT(".vert.spv"))),
			VK::CreateShaderModule(data(ShaderPath + TEXT(".frag.spv")))
		};
		const std::vector VIBDs = { 
			//!< 頂点毎 (Per Vertex)
			VkVertexInputBindingDescription({ .binding = 0, .stride = sizeof(Vertex_PositionColor), .inputRate = VK_VERTEX_INPUT_RATE_VERTEX }),
			//!< インスタンス毎 (Per Instance)
			VkVertexInputBindingDescription({ .binding = 1, .stride = sizeof(Instance_OffsetXY), .inputRate = VK_VERTEX_INPUT_RATE_INSTANCE }),
		};
		const std::vector VIADs = { 
			//!< 頂点毎 (Per Vertex)
			VkVertexInputAttributeDescription({ .location = 0, .binding = 0, .format = VK_FORMAT_R32G32B32_SFLOAT, .offset = offsetof(Vertex_PositionColor, Position) }),
			VkVertexInputAttributeDescription({ .location = 1, .binding = 0, .format = VK_FORMAT_R32G32B32A32_SFLOAT, .offset = offsetof(Vertex_PositionColor, Color) }),
			//!< インスタンス毎 (Per Instance)
			VkVertexInputAttributeDescription({ .location = 2, .binding = 1, .format = VK_FORMAT_R32G32_SFLOAT, .offset = offsetof(Instance_OffsetXY, Offset) }),
		};
		CreatePipeline_VsFs_Input(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP, 0, VK_FALSE, VIBDs, VIADs, SMs);
		for (auto i : SMs) { vkDestroyShaderModule(Device, i, GetAllocationCallbacks()); }
	}

	virtual void PopulateCommandBuffer(const size_t i) override;
};
#pragma endregion