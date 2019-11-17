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
#ifdef USE_SECONDARY_COMMAND_BUFFER
	virtual void AllocateSecondaryCommandBuffer() override { AddSecondaryCommandBuffer(); }
#endif
	virtual void CreateVertexBuffer() override;
	virtual void CreateIndexBuffer() override;
	virtual void CreateIndirectBuffer() override { CreateIndirectBuffer_DrawIndexed(IndexCount, InstanceCount); }
	virtual void CreateDescriptorSetLayout() override {
		DescriptorSetLayouts.resize(1);
		VKExt::CreateDescriptorSetLayout(DescriptorSetLayouts[0], {});
	}
	virtual void CreatePipelineLayout() override {
		assert(!DescriptorSetLayouts.empty() && "");
		PipelineLayouts.resize(1);
		VKExt::CreatePipelineLayout(PipelineLayouts[0], {}, {});
	}
	virtual void CreateShaderModule() override { CreateShaderModle_VsFs(); }
	virtual void CreatePipeline() override { CreatePipeline_VsFs_Vertex_Instance<Vertex_PositionColor, Instance_OffsetXY>(); }
	virtual void PopulateCommandBuffer(const size_t i) override;

	uint32_t IndexCount = 0;
	uint32_t InstanceCount = 0;
};
#pragma endregion