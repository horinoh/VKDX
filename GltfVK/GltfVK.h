#pragma once

#include "resource.h"

#pragma region Code
#include "../VKExt.h"
#include "../Gltf.h"

class GltfVK : public VKExt, public Gltf
{
private:
	using Super = VKExt;
public:
	GltfVK() : Super() {}
	virtual ~GltfVK() {}

protected:
	virtual void LoadScene() override;
	virtual void Process(const fx::gltf::Primitive& Prim) override;
	virtual void Process(const fx::gltf::Accessor& Acc) override;

	std::string Semantics;

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
		VKExt::CreatePipelineLayout(PipelineLayouts[0], {}, {});
	}
	virtual void CreateShaderModule() override { CreateShaderModle_VsFs(); }
	virtual void CreatePipeline() override { CreatePipeline_VsFs_Vertex<Vertex_PositionColor>(); }
	virtual void PopulateCommandBuffer(const size_t i) override;

	uint32_t IndexCount = 0;
};
#pragma endregion