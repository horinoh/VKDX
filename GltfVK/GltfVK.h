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
	virtual void Process(const fx::gltf::Accessor& Acc) override;

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
	virtual void CreatePipeline() override { CreatePipeline_VsFs_Vertex<Vertex_PositionNormalTexcoord>(); }
	virtual void PopulateCommandBuffer(const size_t i) override;
};
#pragma endregion