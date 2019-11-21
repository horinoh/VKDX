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
#ifdef USE_SECONDARY_COMMAND_BUFFER
	virtual void AllocateSecondaryCommandBuffer() override { AddSecondaryCommandBuffer(); }
#endif
	virtual void CreateVertexBuffer() override;
	virtual void CreateIndexBuffer() override;
	virtual void CreateIndirectBuffer() override { CreateIndirectBuffer_DrawIndexed(IndexCount, 1); }
	virtual void CreateDescriptorSetLayout() override { 
		DescriptorSetLayouts.resize(1); 
		VKExt::CreateDescriptorSetLayout(DescriptorSetLayouts[0], {});
	}
	virtual void CreatePipelineLayout() override {
		assert(!DescriptorSetLayouts.empty() && "");
		PipelineLayouts.resize(1);
#ifdef USE_PUSH_CONSTANTS
		const VkPushConstantRange PCR = { VK_SHADER_STAGE_FRAGMENT_BIT, 0, static_cast<uint32_t>(Color.size() * sizeof(Color[0])) };
		VKExt::CreatePipelineLayout(PipelineLayouts[0], {}, { PCR });
#else
		VKExt::CreatePipelineLayout(PipelineLayouts[0], {}, {});
#endif
	}
	virtual void CreateShaderModule() override { CreateShaderModle_VsFs(); }
	virtual void CreatePipeline() override { CreatePipeline_VsFs_Vertex<Vertex_PositionColor>(); }
	virtual void PopulateCommandBuffer(const size_t i) override;

	uint32_t IndexCount = 0;
#ifdef USE_PUSH_CONSTANTS
	const std::array<float, 4> Color = { 1.0f, 0.0f, 0.0f, 1.0f };
#endif
};
#pragma endregion