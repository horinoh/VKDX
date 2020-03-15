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
	virtual void CreateShaderModules() override { CreateShaderModle_VsFs(); }
	virtual void CreatePipelines() override {
		Pipelines.push_back(VkPipeline());
		std::vector<std::thread> Threads;
		const std::vector<VkVertexInputBindingDescription> VIBDs = { {
			{ 0, sizeof(Vertex_PositionColor), VK_VERTEX_INPUT_RATE_VERTEX },
			{ 1, sizeof(Instance_OffsetXY), VK_VERTEX_INPUT_RATE_INSTANCE },
		} };
		const std::vector<VkVertexInputAttributeDescription> VIADs = { {
			//!< Per Vertex
			{ 0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex_PositionColor, Position) },
			{ 1, 0, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(Vertex_PositionColor, Color) },
			//!< Per Instance
			{ 2, 1, VK_FORMAT_R32G32_SFLOAT, offsetof(Instance_OffsetXY, Offset) },
		} };
#ifdef USE_PIPELINE_SERIALIZE
		PipelineCacheSerializer PCS(Device, (GetBasePath() + TEXT(".pco")).c_str(), 1);
		Threads.push_back(std::thread::thread(VK::CreatePipeline, std::ref(Pipelines[0]), Device, PipelineLayouts[0], RenderPasses[0], VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP, 0, ShaderModules[0], ShaderModules[1], NullShaderModule, NullShaderModule, NullShaderModule, VIBDs, VIADs, PCS.GetPipelineCache(0)));
#else
		Threads.push_back(std::thread::thread(VK::CreatePipeline, std::ref(Pipelines[0]), Device, PipelineLayouts[0], RenderPasses[0], VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP, 0, ShaderModules[0], ShaderModules[1], NullShaderModule, NullShaderModule, NullShaderModule, VIBDs, VIADs, nullptr));
#endif
		for (auto& i : Threads) { i.join(); }
	}
	virtual void PopulateCommandBuffer(const size_t i) override;

	uint32_t IndexCount = 0;
	uint32_t InstanceCount = 0;
};
#pragma endregion