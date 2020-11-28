#pragma once

#include "VK.h"

class VKExt : public VK
{
private:
	using Super = VK;
public:
	using Vertex_PositionColor = struct Vertex_PositionColor { glm::vec3 Position; glm::vec4 Color; };
	using Instance_OffsetXY = struct Instance_OffsetXY { glm::vec2 Offset; };

	virtual void CreateBuffer_Vertex(VkBuffer* Buf, VkDeviceMemory* DM, const VkQueue Queue, const VkCommandBuffer CB, const VkDeviceSize Size, const void* Source) { CreateAndCopyToBuffer(Buf, DM, Queue, CB, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT, VK_PIPELINE_STAGE_VERTEX_INPUT_BIT, Size, Source); }
	virtual void CreateBuffer_Index(VkBuffer* Buf, VkDeviceMemory* DM, const VkQueue Queue, const VkCommandBuffer CB, const VkDeviceSize Size, const void* Source) { CreateAndCopyToBuffer(Buf, DM, Queue, CB, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_ACCESS_INDEX_READ_BIT, VK_PIPELINE_STAGE_VERTEX_INPUT_BIT, Size, Source); }
	virtual void CreateBuffer_Indirect(VkBuffer* Buf, VkDeviceMemory* DM, const VkQueue Queue, const VkCommandBuffer CB, const VkDeviceSize Size, const void* Source) { CreateAndCopyToBuffer(Buf, DM, Queue, CB, VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, VK_ACCESS_INDIRECT_COMMAND_READ_BIT, VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT, Size, Source); }

	void CreateIndirectBuffer_Draw(const uint32_t VertexCount, const uint32_t InstanceCount);
	void CreateIndirectBuffer_DrawIndexed(const uint32_t IndexCount, const uint32_t InstanceCount);
	void CreateIndirectBuffer_Dispatch(const uint32_t X, const uint32_t Y, const uint32_t Z);
	
	void CreateShaderModle_VsFs();
	void CreateShaderModle_VsFsTesTcsGs();
	void CreateShaderModle_Cs();

	//!< 引数のシェーダの順序は D3D12_GRAPHICS_PIPELINE_STATE_DESC 内の VS, PS, DS, HS, GS に合わせて、VS, FS, TES, TCS, GS にしておくことにする
	void CreatePipeline_VsFs(const VkPrimitiveTopology Topology, const uint32_t PatchControlPoints, const VkBool32 DepthEnable) { CreatePipeline_VsFs_Input(Topology, PatchControlPoints, DepthEnable, {}, {}); }
	void CreatePipeline_VsFs_Input(const VkPrimitiveTopology Topology, const uint32_t PatchControlPoints, const VkBool32 DepthEnable, const std::vector<VkVertexInputBindingDescription>& VIBDs, const std::vector<VkVertexInputAttributeDescription>& VIADs);
	void CreatePipeline_VsFsTesTcsGs(const VkPrimitiveTopology Topology, const uint32_t PatchControlPoints, const VkBool32 DepthEnable) { CreatePipeline_VsFsTesTcsGs_Input(Topology, PatchControlPoints, DepthEnable, {}, {}); }
	void CreatePipeline_VsFsTesTcsGs_Input(const VkPrimitiveTopology Topology, const uint32_t PatchControlPoints, const VkBool32 DepthEnable, const std::vector<VkVertexInputBindingDescription>& VIBDs, const std::vector<VkVertexInputAttributeDescription>& VIADs);
	void CreatePipeline_Cs() { /*Pipelines.emplace_back(VkPipeline());*/assert(0 && "TODO"); }
};