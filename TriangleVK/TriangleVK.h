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
	virtual void CreateVertexBuffer() override;
	virtual void CreateIndexBuffer() override;
	virtual void CreateIndirectBuffer() override;

	virtual void CreateDescriptorSetLayout() override { 
		DescriptorSetLayouts.emplace_back(VkDescriptorSetLayout());
		VKExt::CreateDescriptorSetLayout(DescriptorSetLayouts.back(), 0, {});
	}
	virtual void CreatePipelineLayout() override {
		assert(!DescriptorSetLayouts.empty() && "");
		PipelineLayouts.emplace_back(VkPipelineLayout());
#ifdef USE_PUSH_CONSTANTS
		const VkPushConstantRange PCR = { VK_SHADER_STAGE_FRAGMENT_BIT, 0, static_cast<uint32_t>(Color.size() * sizeof(Color[0])) };
		VKExt::CreatePipelineLayout(PipelineLayouts.back(), {}, { PCR });
#else
		VKExt::CreatePipelineLayout(PipelineLayouts.back(), {}, {});
#endif
	}
	virtual void CreateShaderModules() override { 
		const auto ShaderPath = GetBasePath();
		ShaderModules.emplace_back(VK::CreateShaderModule((ShaderPath + TEXT(".vert.spv")).data()));
#ifdef USE_PUSH_CONSTANTS
		ShaderModules.emplace_back(VK::CreateShaderModule((ShaderPath + TEXT("_pc.frag.spv")).data()));
#else
		ShaderModules.emplace_back(VK::CreateShaderModule((ShaderPath + TEXT(".frag.spv")).data()));
#endif
	}
	virtual void CreatePipelines() override {
		//!< �o�C���f�B���O0�ɂ܂Ƃ߂ē����C���^�[���[�u�A�Z�}���e�B�b�N�X���Ƀo�C���f�B���O�𕪂���Ɣ�C���^�[���[�u�ƂȂ�
		const uint32_t Binding = 0;
		const std::vector<VkVertexInputBindingDescription> VIBDs = { {
			{ Binding, sizeof(Vertex_PositionColor), VK_VERTEX_INPUT_RATE_VERTEX },
		} };
		//!< �l�܂��Ă��Ă��ADX �� D3D12_APPEND_ALIGNED_ELEMENT �̂悤�� offsetof() ����������i�͖���? (Is there no D3D12_APPEND_ALIGNED_ELEMENT equivalent?)
		const std::vector<VkVertexInputAttributeDescription> VIADs = { {
			{ 0, Binding, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex_PositionColor, Position) },
			{ 1, Binding, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(Vertex_PositionColor, Color) },
		} };
		VKExt::CreatePipeline_VsFs_Input(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP, 0, VK_FALSE, VIBDs, VIADs);
	}

	virtual void PopulateCommandBuffer(const size_t i) override;

	uint32_t IndexCount = 0;
#ifdef USE_PUSH_CONSTANTS
	const std::array<float, 4> Color = { 1.0f, 0.0f, 0.0f, 1.0f };
#endif
};
#pragma endregion