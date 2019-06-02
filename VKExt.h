#pragma once

#include "VK.h"

class VKExt : public VK
{
private:
	using Super = VK;
public:
	using Vertex_Position = struct Vertex_Position { glm::vec3 Position; };
	using Vertex_PositionColor = struct Vertex_PositionColor { glm::vec3 Position; glm::vec4 Color; };

	void CreateIndirectBuffer_Draw(const uint32_t Count);
	void CreateIndirectBuffer_DrawIndexed(const uint32_t Count);
	void CreateIndirectBuffer_Dispatch(const uint32_t X, const uint32_t Y, const uint32_t Z);

	//!<１つのユニフォームバッファ One uniform buffer
	void CreateDescriptorSetLayoutBindings_1UB(std::vector<VkDescriptorSetLayoutBinding>& DescriptorSetLayoutBindings, const VkShaderStageFlags ShaderStageFlags = VK_SHADER_STAGE_ALL_GRAPHICS) const;
	void CreateDescriptorPoolSizes_1UB(std::vector<VkDescriptorPoolSize>& DescriptorPoolSizes) const;
	void CreateWriteDescriptorSets_1UB(std::vector<VkWriteDescriptorSet>& WriteDescriptorSets, const std::vector<VkDescriptorBufferInfo>& DescriptorBufferInfos) const;
	void UpdateDescriptorSet_1UB();

	/** 
	アプリ内ではサンプラとサンプルドイメージは別のオブジェクトとして扱うが、シェーダ内ではまとめた一つのオブジェクトとして扱うことができ、プラットフォームによっては効率が良い場合がある
	(コンバインドイメージサンプラ == サンプラ + サンプルドイメージ)
	デスクリプタタイプに VK_DESCRIPTOR_TYPE_SAMPLER や VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE を指定するか、VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER を指定するかの違い

	VK_DESCRIPTOR_TYPE_SAMPLER ... サンプラ
		layout (set=0, binding=0) uniform sampler MySampler;
	VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE ... サンプルドイメージ
		layout (set=0, binding=0) uniform texture2D MyTexture2D;

	VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER ... コンバインドイメージサンプラ
		layout (set=0, binding=0) uniform sampler2D MySampler2D;

	VK_DESCRIPTOR_TYPE_STORAGE_IMAGE ... ストレージイメージ (シェーダから書き込み可能)
	layout (set=0, binding=0, r32f) uniform image2D MyImage2D;

	VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER ... ユニフォームテクセルバッファ
		layout (set=0, binding=0) uniform samplerBuffer MySamplerBuffer;
	VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER ... ストレージテクセルバッファ(シェーダから書き込み可能)
		layout (set=0, binding=0, r32f) uniform imageBuffer MyImageBuffer;

	VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER ... 
	VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC ... 
		layout (set=0, binding=0) uniform MyUniform
		{
			vec4 MyVec4;
			mat4 MyMat4;
		}

	VK_DESCRIPTOR_TYPE_STORAGE_BUFFER ... 
	VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC ...
		layout (set=0, binding=0) buffer MyBuffer
		{
			vec4 MyVec4;
			mat4 MyMat4;
		}
	
	VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT ... (前レンダーパスで)レンダーターゲット(アタッチメント)として使われたものを(レンダーパス中で)入力として取る場合
		layout (input_attachment_index=0, set=0, binding=0) uniform subpassInput MySubpassInput;

	*/
	//!< １つのコンバインドイメージサンプラ(イメージ + サンプラ) One combined image sampler
	void CreateDescriptorSetLayoutBindings_1CIS(std::vector<VkDescriptorSetLayoutBinding>& DescriptorSetLayoutBindings, const VkShaderStageFlags ShaderStageFlags = VK_SHADER_STAGE_ALL_GRAPHICS) const;
	void CreateDescriptorPoolSizes_1CIS(std::vector<VkDescriptorPoolSize>& DescriptorPoolSizes) const;
	void CreateWriteDescriptorSets_1CIS(std::vector<VkWriteDescriptorSet>& WriteDescriptorSets, const std::vector<VkDescriptorImageInfo>& DescriptorImageInfos) const;
	void UpdateDescriptorSet_1CIS();

	//!< 1つのユニフォームバッファと1つのコンバインドイメージサンプラ One uniform buffer and one combined image sampler 
	void CreateDescriptorSetLayoutBindings_1UB_1CIS(std::vector<VkDescriptorSetLayoutBinding>& DescriptorSetLayoutBindings, const VkShaderStageFlags ShaderStageFlags_UB = VK_SHADER_STAGE_ALL_GRAPHICS, const VkShaderStageFlags ShaderStageFlags_CIS = VK_SHADER_STAGE_ALL_GRAPHICS) const;
	void CreateDescriptorPoolSizes_1UB_1CIS(std::vector<VkDescriptorPoolSize>& DescriptorPoolSizes) const;
	void CreateWriteDescriptorSets_1UB_1CIS(std::vector<VkWriteDescriptorSet>& WriteDescriptorSets, const std::vector<VkDescriptorBufferInfo>& DescriptorBufferInfos, const std::vector<VkDescriptorImageInfo>& DescriptorImageInfos) const;
	void UpdateDescriptorSet_1UB_1CIS();

	//!< １つのストレージイメージ One storage image
	void CreateDescriptorSetLayoutBindings_1SI(std::vector<VkDescriptorSetLayoutBinding>& DescriptorSetLayoutBindings, const VkShaderStageFlags ShaderStageFlags = VK_SHADER_STAGE_ALL_GRAPHICS) const;
	void CreateDescriptorPoolSizes_1SI(std::vector<VkDescriptorPoolSize>& DescriptorPoolSizes) const;
	void CreateWriteDescriptorSets_1SI(std::vector<VkWriteDescriptorSet>& WriteDescriptorSets, const std::vector<VkDescriptorImageInfo>& DescriptorImageInfos) const;
	void UpdateDescriptorSet_1SI();

	//!< LinearRepeat
	void CreateSampler_LR(VkSampler* Sampler, const float MaxLOD = (std::numeric_limits<float>::max)()) const;

	template<typename T> void CreateVertexInputBinding(std::vector<VkVertexInputBindingDescription>& VertexInputBindingDescriptions, std::vector<VkVertexInputAttributeDescription>& VertexInputAttributeDescriptions, const uint32_t Binding) const {}
	//!< ↓ここでテンプレート特殊化している Template specialization here
#include "VKVertexInput.inl"

	virtual void CreateRenderPass() { CreateRenderPass_Color(); }
	void CreateRenderPass_Color();
	void CreateRenderPass_ColorDepth();
	void CreateRenderPass_CD_PP();

	virtual void CreateFramebuffer() override { CreateFramebuffer_Color(); }
	void CreateFramebuffer_Color();
	void CreateFramebuffer_ColorDepth();

	void CreateShader_VsPs(std::vector<VkShaderModule>& ShaderModules, std::vector<VkPipelineShaderStageCreateInfo>& PipelineShaderStageCreateInfos) const;
	void CreateShader_VsPsTesTcsGs(std::vector<VkShaderModule>& ShaderModules, std::vector<VkPipelineShaderStageCreateInfo>& PipelineShaderStageCreateInfos) const;
	void CreateShader_Cs(std::vector<VkShaderModule>& ShaderModules, std::vector<VkPipelineShaderStageCreateInfo>& PipelineShaderStageCreateInfos) const;

	template<typename T>
	void CreateUniformBufferT(const T& Type) { CreateUniformBuffer(&UniformBuffer, &UniformDeviceMemory, sizeof(Type), &Type); }

	virtual void CreateInputAssembly(VkPipelineInputAssemblyStateCreateInfo& PipelineInputAssemblyStateCreateInfo) const override { 
		CreateInputAssembly_Topology(PipelineInputAssemblyStateCreateInfo, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP);
	}

protected:
#if 1
	/**
	@brief パイプライン作成時にシェーダ内の定数値を上書き指定できる
	
	//!< シェーダ側には以下のような記述をする (扱えるのはスカラ値のみ)
	layout (constant_id = 0) const int IntValue = 0;
	layout (constant_id = 1) const float FloatValue = 0.0f;
	layout (constant_id = 2) const bool BoolValue = false;
	*/
	struct SpecializationData {
		int IntValue;
		float FloatValue;
		bool BoolValue;
	};
	SpecializationData SpecData = { 1, 1.0f, true };
	const std::vector<VkSpecializationMapEntry> SpecializationMapEntries = {
		{ 0, offsetof(SpecializationData, IntValue), sizeof(SpecData.IntValue) },
		{ 1, offsetof(SpecializationData, FloatValue), sizeof(SpecData.FloatValue) },
		{ 2, offsetof(SpecializationData, BoolValue), sizeof(SpecData.BoolValue) },
	};
	//!< VkPipelineShaderStageCreateInfo.pSpecializationInfo へ対して指定する
	const VkSpecializationInfo SpecializationInfo = {
		static_cast<uint32_t>(SpecializationMapEntries.size()), SpecializationMapEntries.data(),
		sizeof(SpecData), &SpecData
	};
#endif
};