#pragma once

#include "VK.h"

class VKExt : public VK
{
private:
	using Super = VK;
public:
	using Vertex_Position = struct Vertex_Position { glm::vec3 Position; };
	using Vertex_PositionColor = struct Vertex_PositionColor { glm::vec3 Position; glm::vec4 Color; };

	void CreateIndirectBuffer_Vertices(const uint32_t Count);
	void CreateIndirectBuffer_Indexed(const uint32_t Count);
	void CreateIndirectBuffer_Dispatch(const uint32_t X, const uint32_t Y, const uint32_t Z);

	//!< １つのユニフォームバッファ One uniform buffer
	void CreateDescriptorSetLayoutBindings_1UB(std::vector<VkDescriptorSetLayoutBinding>& DescriptorSetLayoutBindings, const VkShaderStageFlags ShaderStageFlags = VK_SHADER_STAGE_ALL_GRAPHICS) const {
		DescriptorSetLayoutBindings.push_back({ 
			0, //!< バインディング Binding
			VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, //!< タイプ Type
			1, //!< 個数 Count
			ShaderStageFlags, 
			nullptr
		});
	}
	void CreateDescriptorPoolSizes_1UB(std::vector<VkDescriptorPoolSize>& DescriptorPoolSizes) const {
		DescriptorPoolSizes.push_back({ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1 });
	}
	void CreaateWriteDescriptorSets_1UB(VkWriteDescriptorSet& WriteDescriptorSet, const std::vector<VkDescriptorBufferInfo>& DescriptorBufferInfos) const;
	void UpdateDescriptorSet_1UB();

	/** 
	プラットフォームによってはコンバインドイメージサンプラを用いた方が効率が良い場合がある On some platforms, it may be more optimal to use combined image sampler
	
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
	//!< １つのコンバインドイメージサンプラ (イメージとサンプラをまとめたもの)
	void CreateDescriptorSetLayoutBindings_1CIS(std::vector<VkDescriptorSetLayoutBinding>& DescriptorSetLayoutBindings, const VkShaderStageFlags ShaderStageFlags = VK_SHADER_STAGE_ALL_GRAPHICS) const {
		DescriptorSetLayoutBindings.push_back({
			0, //!< バインディング
			VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, //!< タイプ
			1, //!< 個数
			ShaderStageFlags,
			nullptr
		});
	}
	void CreateDescriptorPoolSizes_1CIS(std::vector<VkDescriptorPoolSize>& DescriptorPoolSizes) const {
		DescriptorPoolSizes.push_back({ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1 });
	}
	void CreaateWriteDescriptorSets_1CIS(VkWriteDescriptorSet& WriteDescriptorSet, const std::vector<VkDescriptorImageInfo>& DescriptorImageInfos) const;
	void UpdateDescriptorSet_1CIS();

	//!< LinearRepeat
	void CreateSampler_LR(VkSampler* Sampler, const float MaxLOD = (std::numeric_limits<float>::max)()) const;

	template<typename T>
	void CreateVertexInputT(std::vector<VkVertexInputBindingDescription>& VertexInputBindingDescriptions, std::vector<VkVertexInputAttributeDescription>& VertexInputAttributeDescriptions, const uint32_t Binding) const {}
	template<>
	void CreateVertexInputT<Vertex_Position>(std::vector<VkVertexInputBindingDescription>& VertexInputBindingDescriptions, std::vector<VkVertexInputAttributeDescription>& VertexInputAttributeDescriptions, const uint32_t Binding) const {
		VertexInputBindingDescriptions = {
			{ Binding, sizeof(Vertex_Position), VK_VERTEX_INPUT_RATE_VERTEX } //!< バーテックス毎(インスタンス毎にする場合にはVK_VERTEX_INPUT_RATE_INSTANCEを使用する)
		};
		VertexInputAttributeDescriptions = {
			{ 0, Binding, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex_Position, Position) }, //!< layout (location = 0) in vec3 InPosition
		};
	}
	template<>
	void CreateVertexInputT<Vertex_PositionColor>(std::vector<VkVertexInputBindingDescription>& VertexInputBindingDescriptions, std::vector<VkVertexInputAttributeDescription>& VertexInputAttributeDescriptions, const uint32_t Binding) const {
		VertexInputBindingDescriptions = {
			{ Binding, sizeof(Vertex_PositionColor), VK_VERTEX_INPUT_RATE_VERTEX }
		};
		VertexInputAttributeDescriptions = {
			{ 0, Binding, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex_PositionColor, Position) }, //!< layout(location = 0) in vec3 InPosition;
			{ 1, Binding, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(Vertex_PositionColor, Color) }, //!< layout(location = 1) in vec4 InColor
		};
	}

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
	void CreateUniformBuffer(const T& Type) {
		const auto Size = sizeof(T);

		[&](VkBuffer* Buffer, VkDeviceMemory* DeviceMemory, const VkDeviceSize Size, const void* Data) {
			const auto Usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;

			CreateBuffer(Buffer, Usage, Size);
			CreateHostVisibleMemory(DeviceMemory, *Buffer);
			CopyToHostVisibleMemory(*DeviceMemory, Size, Data);

			BindDeviceMemory(*Buffer, *DeviceMemory);

			//!< View は必要ない No need view

		}(&UniformBuffer, &UniformDeviceMemory, Size, &Type);

#ifdef _DEBUG
		std::cout << "CreateUniformBuffer" << COUT_OK << std::endl << std::endl;
#endif
	}

	virtual void CreateInputAssembly_TriangleStrip(VkPipelineInputAssemblyStateCreateInfo& PipelineInputAssemblyStateCreateInfo) const { 
		CreateInputAssembly_Topology(PipelineInputAssemblyStateCreateInfo, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP);
	}
	virtual void CreateInputAssembly_PatchList(VkPipelineInputAssemblyStateCreateInfo& PipelineInputAssemblyStateCreateInfo) const { 
		CreateInputAssembly_Topology(PipelineInputAssemblyStateCreateInfo, VK_PRIMITIVE_TOPOLOGY_PATCH_LIST);
	}
	virtual void CreateInputAssembly(VkPipelineInputAssemblyStateCreateInfo& PipelineInputAssemblyStateCreateInfo) const override { 
		CreateInputAssembly_TriangleStrip(PipelineInputAssemblyStateCreateInfo); 
	}

	void CreateTessellationState_PatchControlPoint(VkPipelineTessellationStateCreateInfo& PipelineTessellationStateCreateInfo, const uint32_t PatchControlPoint) const { 
		PipelineTessellationStateCreateInfo = { VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO, nullptr, 0, PatchControlPoint };
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