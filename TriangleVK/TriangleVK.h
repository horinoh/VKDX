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
	virtual void CreateShaderModules() override { CreateShaderModle_VsFs(); }
	virtual void CreatePipelines() override {
		Pipelines.push_back(VkPipeline());
		std::vector<std::thread> Threads;
		const std::vector<VkVertexInputBindingDescription> VIBDs = { {
			{ 0, sizeof(Vertex_PositionColor), VK_VERTEX_INPUT_RATE_VERTEX },
		} };
		//!< ‹l‚Ü‚Á‚Ä‚¢‚Ä‚àADX ‚Ì D3D12_APPEND_ALIGNED_ELEMENT ‚Ì‚æ‚¤‚É offsetof() ‚ð‰ñ”ð‚·‚éŽè’i‚Í–³‚¢? (Is there no D3D12_APPEND_ALIGNED_ELEMENT equivalent?)
		const std::vector<VkVertexInputAttributeDescription> VIADs = { {
			{ 0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex_PositionColor, Position) },
			{ 1, 0, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(Vertex_PositionColor, Color) },
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
#ifdef USE_PUSH_CONSTANTS
	const std::array<float, 4> Color = { 1.0f, 0.0f, 0.0f, 1.0f };
#endif
};
#pragma endregion