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
	virtual void CreateShaderModule() override { CreateShaderModle_VsFs(); }
	virtual void CreatePipeline() override {
		const std::vector VIBDs = { 
			//!< ���_�� (Per Vertex)
			VkVertexInputBindingDescription({ .binding = 0, .stride = sizeof(Vertex_PositionColor), .inputRate = VK_VERTEX_INPUT_RATE_VERTEX }),
			//!< �C���X�^���X�� (Per Instance)
			VkVertexInputBindingDescription({ .binding = 1, .stride = sizeof(Instance_OffsetXY), .inputRate = VK_VERTEX_INPUT_RATE_INSTANCE }),
		};
		const std::vector VIADs = { 
			//!< ���_�� (Per Vertex)
			VkVertexInputAttributeDescription({ .location = 0, .binding = 0, .format = VK_FORMAT_R32G32B32_SFLOAT, .offset = offsetof(Vertex_PositionColor, Position) }),
			VkVertexInputAttributeDescription({ .location = 1, .binding = 0, .format = VK_FORMAT_R32G32B32A32_SFLOAT, .offset = offsetof(Vertex_PositionColor, Color) }),
			//!< �C���X�^���X�� (Per Instance)
			VkVertexInputAttributeDescription({ .location = 2, .binding = 1, .format = VK_FORMAT_R32G32_SFLOAT, .offset = offsetof(Instance_OffsetXY, Offset) }),
		};
		CreatePipeline_VsFs_Input(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP, 0, VK_FALSE, VIBDs, VIADs);
	}
	virtual void PopulateCommandBuffer(const size_t i) override;
};
#pragma endregion