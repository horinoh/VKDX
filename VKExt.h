#pragma once

#include "VK.h"

class VKExt : public VK
{
private:
	using Super = VK;
public:
	//using Vertex_Position = struct Vertex_Position { glm::vec3 Position; };
	using Vertex_PositionColor = struct Vertex_PositionColor { glm::vec3 Position; glm::vec4 Color; };

	void CreateIndirectBuffer_Draw(const uint32_t Count) {
		const VkDrawIndirectCommand DIC = { Count, 1, 0, 0 };
		CreateIndirectBuffer(&IndirectBuffer, &IndirectDeviceMemory, static_cast<VkDeviceSize>(sizeof(DIC)), &DIC, CommandBuffers[0]);
	}
	void CreateIndirectBuffer_DrawIndexed(const uint32_t Count) {
		const VkDrawIndexedIndirectCommand DIIC = { Count, 1, 0, 0, 0 };
		CreateIndirectBuffer(&IndirectBuffer, &IndirectDeviceMemory, static_cast<VkDeviceSize>(sizeof(DIIC)), &DIIC, CommandBuffers[0]);
	}
	void CreateIndirectBuffer_Dispatch(const uint32_t X, const uint32_t Y, const uint32_t Z) {
		const VkDispatchIndirectCommand DIC = { X, Y, Z };
		CreateIndirectBuffer(&IndirectBuffer, &IndirectDeviceMemory, static_cast<VkDeviceSize>(sizeof(DIC)), &DIC, CommandBuffers[0]);
	}

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
	
	void CreateShaderModle_VsFs();
	void CreateShaderModle_VsFsTesTcsGs();
	void CreateShaderModle_Cs();

	template<typename T> void CreatePipeline_Vertex(VkPipeline& Pipeline, const VkPipelineLayout PL,
		const VkShaderModule VS, const VkShaderModule FS, const VkShaderModule TES, const VkShaderModule TCS, const VkShaderModule GS,
		const VkRenderPass RP, VkPipelineCache PC = VK_NULL_HANDLE);
	void CreatePipeline_Tesselation(VkPipeline& Pipeline, const VkPipelineLayout PL,
		const VkShaderModule VS, const VkShaderModule FS, const VkShaderModule TES, const VkShaderModule TCS, const VkShaderModule GS,
		const VkRenderPass RP, VkPipelineCache PC = VK_NULL_HANDLE);
	void CreatePipeline_VsFs();
	void CreatePipeline_VsFsTesTcsGs_Tesselation();
	void CreatePipeline_Cs() { assert(0 && "TODO"); }
	//!< ↓ここでテンプレート特殊化している (Template specialization here)
#include "VKPipeline.inl"

	//!< LinearRepeat
	void CreateSampler_LR(VkSampler* Sampler, const float MaxLOD = (std::numeric_limits<float>::max)()) const;

	void CreateRenderPass_ColorDepth(VkRenderPass& RP, const VkFormat Color, const VkFormat Depth);
	void CreateRenderPass_ColorDepth_PostProcess(VkRenderPass& RP, const VkFormat Color, const VkFormat Depth);

	//virtual void CreateFramebuffer(VkFramebuffer& FB, const VkRenderPass RP, const uint32_t Width, const uint32_t Height, const uint32_t Layers, const std::initializer_list<VkImageView> il_IVs);
	void CreateFramebuffer_Color();
	void CreateFramebuffer_ColorDepth();
	virtual void CreateFramebuffer() override { CreateFramebuffer_Color(); }

	template<typename T>
	void CreateUniformBuffer(const T& Type) { VK::CreateUniformBuffer(&UniformBuffer, &UniformDeviceMemory, sizeof(Type), &Type); }

	//virtual void CreateInputAssembly(VkPipelineInputAssemblyStateCreateInfo& PipelineInputAssemblyStateCreateInfo) const override { 
	//	CreateInputAssembly_Topology(PipelineInputAssemblyStateCreateInfo, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP);
	//}
};