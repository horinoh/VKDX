#pragma once

#include "VK.h"

class VKExt : public VK
{
private:
	using Super = VK;
public:
	using Vertex_PositionColor = struct Vertex_PositionColor { glm::vec3 Position; glm::vec4 Color; };
	using Instance_OffsetXY = struct Instance_OffsetXY { glm::vec2 Offset; };

	virtual void CreateBuffer_Vertex(VkBuffer* Buf, const VkQueue Queue, const VkCommandBuffer CB, const VkDeviceSize Size, const void* Source) { CreateAndCopyToBuffer(Buf, Queue, CB, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT, VK_PIPELINE_STAGE_VERTEX_INPUT_BIT, Size, Source); }
	virtual void CreateBuffer_Index(VkBuffer* Buf, const VkQueue Queue, const VkCommandBuffer CB, const VkDeviceSize Size, const void* Source) { CreateAndCopyToBuffer(Buf, Queue, CB, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_ACCESS_INDEX_READ_BIT, VK_PIPELINE_STAGE_VERTEX_INPUT_BIT, Size, Source); }
	virtual void CreateBuffer_Indirect(VkBuffer* Buf, const VkQueue Queue, const VkCommandBuffer CB, const VkDeviceSize Size, const void* Source) { CreateAndCopyToBuffer(Buf, Queue, CB, VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, VK_ACCESS_INDIRECT_COMMAND_READ_BIT, VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT, Size, Source); }
	
#if 0
	//!< layout(set = 0, binding = 0) buffer MyBuffer { vec4 MyVec4; mat4 MyMat4; }
	void CreateBuffer_Storage(VkBuffer *Buffer, const VkDeviceSize Size) {
		CreateBuffer(Buffer, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, Size);
		//CreateBuffer(Buffer, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, Size); //!< VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC もあるよ
		
		uint32_t HeapIndex;
		VkDeviceSize Offset;
		SuballocateBufferMemory(HeapIndex, Offset, *Buffer, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		VERIFY_SUCCEEDED(vkBindBufferMemory(Device, *Buffer, DeviceMemories[HeapIndex], Offset));
	}
	//!< layout (set=0, binding=0) uniform samplerBuffer MySamplerBuffer;
	void CreateBuffer_UniformTexel(VkBuffer* Buffer, const VkDeviceSize Size) {
		CreateBuffer(Buffer, VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT, Size);

		uint32_t HeapIndex;
		VkDeviceSize Offset;
		SuballocateBufferMemory(HeapIndex, Offset, *Buffer, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		VERIFY_SUCCEEDED(vkBindBufferMemory(Device, *Buffer, DeviceMemories[HeapIndex], Offset));
	}
	//!< layout (set=0, binding=0, r32f) uniform imageBuffer MyImageBuffer;
	void CreateBuffer_StorageTexel(VkBuffer* Buffer, const VkDeviceSize Size) {
		CreateBuffer(Buffer, VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT, Size);

		uint32_t HeapIndex;
		VkDeviceSize Offset;
		SuballocateBufferMemory(HeapIndex, Offset, *Buffer, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		VERIFY_SUCCEEDED(vkBindBufferMemory(Device, *Buffer, DeviceMemories[HeapIndex], Offset));
	}
#endif

	/** 
	アプリ内ではサンプラとサンプルドイメージは別のオブジェクトとして扱うが、シェーダ内ではまとめた一つのオブジェクトとして扱うことができ、プラットフォームによっては効率が良い場合がある
	(コンバインドイメージサンプラ == サンプラ + サンプルドイメージ)
	デスクリプタタイプに VK_DESCRIPTOR_TYPE_SAMPLER や VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE を指定するか、VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER を指定するかの違い
	IMAGE			... VkImage
	TEXEL_BUFFER	... VkBuffer
	STORAGE			... 付いているものはシェーダから書き込み可能

	VK_DESCRIPTOR_TYPE_SAMPLER ... サンプラ (VkSampler)
		layout (set=0, binding=0) uniform sampler MySampler;
	VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE ... サンプルドイメージ (VkImage)
		layout (set=0, binding=0) uniform texture2D MyTexture2D;
	VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER ... コンバインドイメージサンプラ (VkSampler + VkImage)
		layout (set=0, binding=0) uniform sampler2D MySampler2D;
	VK_DESCRIPTOR_TYPE_STORAGE_IMAGE ... ストレージイメージ (VkImage)
		シェーダから書き込み可能、アトミックな操作が可能
		レイアウトは VK_IMAGE_LAYOUT_GENERAL にしておくこと
	layout (set=0, binding=0, r32f) uniform image2D MyImage2D;

	VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER ... ユニフォームテクセルバッファ (VkBuffer)
		1Dのイメージのように扱われる
		1Dイメージは最低限4096テクセルだが、ユニフォームテクセルバッファは最低限65536テクセル(イメージよりも大きなデータへアクセス可能)
		layout (set=0, binding=0) uniform samplerBuffer MySamplerBuffer;
	VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER ... ストレージテクセルバッファ (vkBuffer)
		シェーダから書き込み可能、アトミックな操作が可能
		layout (set=0, binding=0, r32f) uniform imageBuffer MyImageBuffer;

	VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC ... ユニフォームバッファ (VkBuffer)
		ダイナミックユニフォームバッファの場合は
		layout (set=0, binding=0) uniform MyUniform { vec4 MyVec4; mat4 MyMat4; }

	VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC ... ストレージバッファ (VkBuffer)
		シェーダから書き込み可能、アトミックな操作が可能

		ダイナミックストレージバッファの場合は
		layout (set=0, binding=0) buffer MyBuffer { vec4 MyVec4; mat4 MyMat4; }
	
	VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT ... (前レンダーパスで)レンダーターゲット(アタッチメント)として使われたものを(レンダーパス中で)入力として取る場合
		layout (input_attachment_index=0, set=0, binding=0) uniform subpassInput MySubpassInput;
	*/

	void CreateIndirectBuffer_Draw(const uint32_t IndexCount, const uint32_t InstanceCount);
	void CreateIndirectBuffer_DrawIndexed(const uint32_t IndexCount, const uint32_t InstanceCount);
	void CreateIndirectBuffer_Dispatch(const uint32_t X, const uint32_t Y, const uint32_t Z);
	
	void CreateShaderModle_VsFs();
	void CreateShaderModle_VsFsTesTcsGs();
	void CreateShaderModle_Cs();

	void CreatePipeline_VsFs(const VkPrimitiveTopology Topology);
	void CreatePipeline_VsFsTesTcsGs(const VkPrimitiveTopology Topology);
	void CreatePipeline_Cs(VkPipeline& /*PL*/) { assert(0 && "TODO"); }

	void CreateRenderPass_ColorDepth(VkRenderPass& RP, const VkFormat Color, const VkFormat Depth);
	void CreateRenderPass_ColorDepth_PostProcess(VkRenderPass& RP, const VkFormat Color, const VkFormat Depth);
	void CreateRenderPass_Color_PostProcess(VkRenderPass& RP, const VkFormat Color);

	void CreateFramebuffer_Color();
	void CreateFramebuffer_ColorDepth();
	virtual void CreateFramebuffer() override { CreateFramebuffer_Color(); }
};