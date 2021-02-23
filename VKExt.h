#pragma once

#include "VK.h"

class VKExt : public VK
{
private:
	using Super = VK;
public:
	using Vertex_PositionColor = struct Vertex_PositionColor { glm::vec3 Position; glm::vec4 Color; };
	using Instance_OffsetXY = struct Instance_OffsetXY { glm::vec2 Offset; };

	//!< �����̃V�F�[�_�̏����� D3D12_GRAPHICS_PIPELINE_STATE_DESC ���� VS, PS, DS, HS, GS �ɍ��킹�āAVS, FS, TES, TCS, GS �ɂ��Ă������Ƃɂ���
	void CreatePipeline_VsFs(const VkPrimitiveTopology PT, const uint32_t PatchControlPoints, const VkBool32 DepthEnable, const std::array<VkShaderModule, 2>& SMs) { CreatePipeline_VsFs_Input(PT, PatchControlPoints, DepthEnable, {}, {}, SMs); }
	void CreatePipeline_VsFs_Input(const VkPrimitiveTopology PT, const uint32_t PatchControlPoints, const VkBool32 DepthEnable, const std::vector<VkVertexInputBindingDescription>& VIBDs, const std::vector<VkVertexInputAttributeDescription>& VIADs, const std::array<VkShaderModule, 2>& SMs);
	void CreatePipeline_VsFsTesTcsGs(const VkPrimitiveTopology PT, const uint32_t PatchControlPoints, const VkBool32 DepthEnable, const std::array<VkShaderModule, 5>& SMs) { CreatePipeline_VsFsTesTcsGs_Input(PT, PatchControlPoints, DepthEnable, {}, {}, SMs); }
	void CreatePipeline_VsFsTesTcsGs_Input(const VkPrimitiveTopology PT, const uint32_t PatchControlPoints, const VkBool32 DepthEnable, const std::vector<VkVertexInputBindingDescription>& VIBDs, const std::vector<VkVertexInputAttributeDescription>& VIADs, const std::array<VkShaderModule, 5>& SMs);
	void CreatePipeline_Cs() { /*Pipelines.emplace_back(VkPipeline());*/assert(0 && "TODO"); }
};