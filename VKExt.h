#pragma once

#include "VK.h"

class VKExt : public VK
{
private:
	using Super = VK;
public:
	using Vertex_Position = struct Vertex_Position { glm::vec3 Position; };
	using Vertex_PositionColor = struct Vertex_PositionColor { glm::vec3 Position; glm::vec4 Color; };

	void CreateIndirectBuffer_4Vertices();
	void CreateIndirectBuffer_Indexed();

	//!< �P�̃��j�t�H�[���o�b�t�@
	void CreateDescriptorSetLayoutBindings_1UB(std::vector<VkDescriptorSetLayoutBinding>& DescriptorSetLayoutBindings, const VkShaderStageFlags ShaderStageFlags = VK_SHADER_STAGE_ALL_GRAPHICS) const {
		DescriptorSetLayoutBindings.push_back({ 
			0, //!< �o�C���f�B���O
			VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, //!< �^�C�v
			1, //!< ��
			ShaderStageFlags, 
			nullptr
		});
	}
	void CreateDescriptorPoolSizes_1UB(std::vector<VkDescriptorPoolSize>& DescriptorPoolSizes) const {
		DescriptorPoolSizes.push_back({ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1 });
	}

	/** 
	�v���b�g�t�H�[���ɂ���Ă̓R���o�C���h�C���[�W�T���v����p���������������ǂ��ꍇ������ On some platforms, it may be more optimal to use combined image sampler
	
	VK_DESCRIPTOR_TYPE_SAMPLER ... �T���v��
		layout (set=0, binding=0) uniform sampler MySampler;
	VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE ... �T���v���h�C���[�W
		layout (set=0, binding=0) uniform texture2D MyTexture2D;
	VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER ... �R���o�C���h�C���[�W�T���v��
		layout (set=0, binding=0) uniform sampler2D MySampler2D;
	VK_DESCRIPTOR_TYPE_STORAGE_IMAGE ... �X�g���[�W�C���[�W (�V�F�[�_���珑�����݉\)
		layout (set=0, binding=0, r32f) uniform image2D MyImage2D;

	VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER ... ���j�t�H�[���e�N�Z���o�b�t�@
		layout (set=0, binding=0) uniform samplerBuffer MySamplerBuffer;
	VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER ... �X�g���[�W�e�N�Z���o�b�t�@(�V�F�[�_���珑�����݉\)
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
	*/

	//!< �P�̃R���o�C���h�C���[�W�T���v�� (�C���[�W�ƃT���v�����܂Ƃ߂�����)
	void CreateDescriptorSetLayoutBindings_1CIS(std::vector<VkDescriptorSetLayoutBinding>& DescriptorSetLayoutBindings, const VkShaderStageFlags ShaderStageFlags = VK_SHADER_STAGE_ALL_GRAPHICS) const {
		DescriptorSetLayoutBindings.push_back({
			0, //!< �o�C���f�B���O
			VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, //!< �^�C�v
			1, //!< ��
			ShaderStageFlags,
			nullptr
		});
	}
	void CreateDescriptorPoolSizes_1CIS(std::vector<VkDescriptorPoolSize>& DescriptorPoolSizes) const {
		DescriptorPoolSizes.push_back({ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1 });
	}
	void CreaateWriteDescriptorSets_1CIS(std::vector<VkWriteDescriptorSet>& WriteDescriptorSets, VkDescriptorImageInfo* DescriptorImageInfo, VkDescriptorBufferInfo* DescriptorBufferInfo, VkBufferView* BufferView) const;

	void CreateSampler_LinearRepeat(const float MaxLOD = (std::numeric_limits<float>::max)());

	template<typename T>
	void CreateVertexInputT(std::vector<VkVertexInputBindingDescription>& VertexInputBindingDescriptions, std::vector<VkVertexInputAttributeDescription>& VertexInputAttributeDescriptions, const uint32_t Binding) const {}
	template<>
	void CreateVertexInputT<Vertex_Position>(std::vector<VkVertexInputBindingDescription>& VertexInputBindingDescriptions, std::vector<VkVertexInputAttributeDescription>& VertexInputAttributeDescriptions, const uint32_t Binding) const {
		VertexInputBindingDescriptions = {
			{ Binding, sizeof(Vertex_Position), VK_VERTEX_INPUT_RATE_VERTEX }
		};
		VertexInputAttributeDescriptions = {
			{ 0, Binding, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex_Position, Position) },
		};
	}
	template<>
	void CreateVertexInputT<Vertex_PositionColor>(std::vector<VkVertexInputBindingDescription>& VertexInputBindingDescriptions, std::vector<VkVertexInputAttributeDescription>& VertexInputAttributeDescriptions, const uint32_t Binding) const {
		VertexInputBindingDescriptions = {
			{ Binding, sizeof(Vertex_PositionColor), VK_VERTEX_INPUT_RATE_VERTEX }
		};
		VertexInputAttributeDescriptions = {
			{ 0, Binding, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex_PositionColor, Position) },
			{ 1, Binding, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex_PositionColor, Color) }
		};
	}

//	template<typename T>
//	void CreateUniformBuffer(const VkPhysicalDeviceMemoryProperties& PhysicalDeviceMemoryProperties, const T& Type) {
//		//!< #TODO
//		const auto Size = sizeof(T);
//		CreateHostVisibleBuffer(PhysicalDeviceMemoryProperties, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, &UniformBuffer, &UniformDeviceMemory, Size, &Type);
//		//CreateUniformBufferDescriptorBufferInfo(static_cast<UINT>(Size));
//#ifdef _DEBUG
//		std::cout << "CreateUniformBuffer" << COUT_OK << std::endl << std::endl;
//#endif
//	}
	virtual void CreateRenderPass() { CreateRenderPass_Color(); }
	void CreateRenderPass_Color();
	void CreateRenderPass_ColorDepth();

	virtual void CreateFramebuffer() override { CreateFramebuffer_Color(); }
	void CreateFramebuffer_Color();
	void CreateFramebuffer_ColorDepth();

	void CreateShader_VsPs(std::vector<VkShaderModule>& ShaderModules, std::vector<VkPipelineShaderStageCreateInfo>& PipelineShaderStageCreateInfos) const;
	void CreateShader_VsPsTesTcsGs(std::vector<VkShaderModule>& ShaderModules, std::vector<VkPipelineShaderStageCreateInfo>& PipelineShaderStageCreateInfos) const;

protected:
#if 1
	/**
	//!< �V�F�[�_���ɂ͈ȉ��̂悤�ȋL�q������ (������̂̓X�J���l�̂�)
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
	//!< VkPipelineShaderStageCreateInfo.pSpecializationInfo �֑΂��Ďw�肷��
	const VkSpecializationInfo SpecializationInfo = {
		static_cast<uint32_t>(SpecializationMapEntries.size()), SpecializationMapEntries.data(),
		sizeof(SpecData), &SpecData
	};
#endif
};