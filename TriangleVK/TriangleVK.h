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
	virtual void CreateGeometry() override;
	virtual void CreatePipelineLayout() override {
		//CreateDescriptorSetLayout(DescriptorSetLayouts.emplace_back(), 0, {});

		//!< プッシュコンスタント : デスクリプタセットよりも高速
		//!< パイプラインレイアウト全体で128byte (ハードによりこれ以上使える場合もある、GTX970Mの場合は256byteだった)
		//!< 各シェーダステージは1つのプッシュコンスタントレンジにしかアクセスできない
		//!< 各シェーダステージが「共通のレンジを持たない」ような「ワーストケース」では 128/5==25.6、1シェーダステージで25byte程度となる
		VK::CreatePipelineLayout(PipelineLayouts.emplace_back(), {}, { 
#ifdef USE_PUSH_CONSTANTS
			VkPushConstantRange({.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT, .offset = 0, .size = static_cast<uint32_t>(size(Color) * sizeof(Color[0])) })
#endif
			});
	}
	virtual void CreateShaderModule() override { 
		const auto ShaderPath = GetBasePath();
		ShaderModules.emplace_back(VK::CreateShaderModule(data(ShaderPath + TEXT(".vert.spv"))));
#ifdef USE_PUSH_CONSTANTS
		ShaderModules.emplace_back(VK::CreateShaderModule(data(ShaderPath + TEXT("_pc.frag.spv"))));
#else
		ShaderModules.emplace_back(VK::CreateShaderModule(data(ShaderPath + TEXT(".frag.spv"))));
#endif
	}
	virtual void CreatePipeline() override {
		//!< バインディング0にまとめて入れるインターリーブ、セマンティックス毎にバインディングを分けると非インターリーブとなる
		const std::vector VIBDs = { 
			VkVertexInputBindingDescription({ .binding = 0, .stride = sizeof(Vertex_PositionColor), .inputRate = VK_VERTEX_INPUT_RATE_VERTEX }),
		};
		//!< 詰まっていても、DX の D3D12_APPEND_ALIGNED_ELEMENT のように offsetof() を回避する手段は無い? (Is there no D3D12_APPEND_ALIGNED_ELEMENT equivalent?)
		const std::vector VIADs = { 
			VkVertexInputAttributeDescription({ .location = 0, .binding = 0, .format = VK_FORMAT_R32G32B32_SFLOAT, .offset = offsetof(Vertex_PositionColor, Position) }),
			VkVertexInputAttributeDescription({ .location = 1, .binding = 0, .format = VK_FORMAT_R32G32B32A32_SFLOAT, .offset = offsetof(Vertex_PositionColor, Color) }),
		};
		VKExt::CreatePipeline_VsFs_Input(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP, 0, VK_FALSE, VIBDs, VIADs);
	}

	virtual void PopulateCommandBuffer(const size_t i) override;

#ifdef USE_PUSH_CONSTANTS
	const std::array<float, 4> Color = { 1.0f, 0.0f, 0.0f, 1.0f };
#endif
};
#pragma endregion