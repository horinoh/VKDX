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
		CreateIndirectBuffer(&IndirectBuffer, &IndirectDeviceMemory, static_cast<VkDeviceSize>(sizeof(DIC)), &DIC, CommandPools[0].second[0]);
	}
	void CreateIndirectBuffer_DrawIndexed(const uint32_t Count) {
		const VkDrawIndexedIndirectCommand DIIC = { Count, 1, 0, 0, 0 };
		CreateIndirectBuffer(&IndirectBuffer, &IndirectDeviceMemory, static_cast<VkDeviceSize>(sizeof(DIIC)), &DIIC, CommandPools[0].second[0]);
	}
	void CreateIndirectBuffer_Dispatch(const uint32_t X, const uint32_t Y, const uint32_t Z) {
		const VkDispatchIndirectCommand DIC = { X, Y, Z };
		CreateIndirectBuffer(&IndirectBuffer, &IndirectDeviceMemory, static_cast<VkDeviceSize>(sizeof(DIC)), &DIC, CommandPools[0].second[0]);
	}

	void CreateDescriptorSetLayout(VkDescriptorSetLayout& DSL, const std::initializer_list<VkDescriptorSetLayoutBinding> il_DSLBs);

	void CreatePipelineLayout(VkPipelineLayout& PL, const std::initializer_list<VkDescriptorSetLayout> il_DSLs, const std::initializer_list<VkPushConstantRange> il_PCRs);

	void CreateDescriptorPool(VkDescriptorPool& DP, const std::initializer_list<VkDescriptorPoolSize> il_DPSs);

	void CreateDescriptorSet(VkDescriptorSet& DS, const VkDescriptorPool DP, const std::initializer_list <VkDescriptorSetLayout> il_DSL);

	void UpdateDescriptorSet_1UB(const VkDescriptorSet DS, const VkBuffer Buffer);
	void UpdateDescriptorSet_1CIS(const VkDescriptorSet DS, const VkSampler Sampler, const VkImageView IV);
	void UpdateDescriptorSet_1SI(const VkDescriptorSet DS, const VkImageView IV);
	void UpdateDescriptorSet_1UB_1CIS(const VkDescriptorSet DS, const VkBuffer Buffer, const VkSampler Sampler);

	/** 
	�A�v�����ł̓T���v���ƃT���v���h�C���[�W�͕ʂ̃I�u�W�F�N�g�Ƃ��Ĉ������A�V�F�[�_���ł͂܂Ƃ߂���̃I�u�W�F�N�g�Ƃ��Ĉ������Ƃ��ł��A�v���b�g�t�H�[���ɂ���Ă͌������ǂ��ꍇ������
	(�R���o�C���h�C���[�W�T���v�� == �T���v�� + �T���v���h�C���[�W)
	�f�X�N���v�^�^�C�v�� VK_DESCRIPTOR_TYPE_SAMPLER �� VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE ���w�肷�邩�AVK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER ���w�肷�邩�̈Ⴂ
	IMAGE			... VkImage
	TEXEL_BUFFER	... VkBuffer
	STORAGE			... �t���Ă�����̂̓V�F�[�_���珑�����݉\

	VK_DESCRIPTOR_TYPE_SAMPLER ... �T���v�� (VkSampler)
		layout (set=0, binding=0) uniform sampler MySampler;
	VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE ... �T���v���h�C���[�W (VkImage)
		layout (set=0, binding=0) uniform texture2D MyTexture2D;
	VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER ... �R���o�C���h�C���[�W�T���v�� (VkSampler + VkImage)
		layout (set=0, binding=0) uniform sampler2D MySampler2D;
	VK_DESCRIPTOR_TYPE_STORAGE_IMAGE ... �X�g���[�W�C���[�W (VkImage)
		�V�F�[�_���珑�����݉\�A�A�g�~�b�N�ȑ��삪�\
		���C�A�E�g�� VK_IMAGE_LAYOUT_GENERAL �ɂ��Ă�������
	layout (set=0, binding=0, r32f) uniform image2D MyImage2D;

	VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER ... ���j�t�H�[���e�N�Z���o�b�t�@ (VkBuffer)
		1D�̃C���[�W�̂悤�Ɉ�����
		1D�C���[�W�͍Œ��4096�e�N�Z�������A���j�t�H�[���e�N�Z���o�b�t�@�͍Œ��65536�e�N�Z��(�C���[�W�����傫�ȃf�[�^�փA�N�Z�X�\)
		layout (set=0, binding=0) uniform samplerBuffer MySamplerBuffer;
	VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER ... �X�g���[�W�e�N�Z���o�b�t�@ (vkBuffer)
		�V�F�[�_���珑�����݉\�A�A�g�~�b�N�ȑ��삪�\
		layout (set=0, binding=0, r32f) uniform imageBuffer MyImageBuffer;

	VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC ... ���j�t�H�[���o�b�t�@ (VkBuffer)
		�_�C�i�~�b�N���j�t�H�[���o�b�t�@�̏ꍇ��
		layout (set=0, binding=0) uniform MyUniform { vec4 MyVec4; mat4 MyMat4; }

	VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC ... �X�g���[�W�o�b�t�@ (VkBuffer)
		�V�F�[�_���珑�����݉\�A�A�g�~�b�N�ȑ��삪�\
		�_�C�i�~�b�N�X�g���[�W�o�b�t�@�̏ꍇ��
		layout (set=0, binding=0) buffer MyBuffer { vec4 MyVec4; mat4 MyMat4; }
	
	VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT ... (�O�����_�[�p�X��)�����_�[�^�[�Q�b�g(�A�^�b�`�����g)�Ƃ��Ďg��ꂽ���̂�(�����_�[�p�X����)���͂Ƃ��Ď��ꍇ
		layout (input_attachment_index=0, set=0, binding=0) uniform subpassInput MySubpassInput;
	*/

	template<typename T> void CreatePipeline_Vertex(VkPipeline& Pipeline, const VkPipelineLayout PL,
		const VkShaderModule VS, const VkShaderModule FS, const VkShaderModule TES, const VkShaderModule TCS, const VkShaderModule GS,
		const VkRenderPass RP, VkPipelineCache PC = VK_NULL_HANDLE);
	void CreatePipeline_Tesselation(VkPipeline& Pipeline, const VkPipelineLayout PL,
		const VkShaderModule VS, const VkShaderModule FS, const VkShaderModule TES, const VkShaderModule TCS, const VkShaderModule GS,
		const VkRenderPass RP, VkPipelineCache PC = VK_NULL_HANDLE);
	
	void CreatePipeline_VsFs();
	void CreatePipeline_VsFsTesTcsGs_Tesselation();
	//!< �������Ńe���v���[�g���ꉻ���Ă��� (Template specialization here)
#include "VKPipeline.inl"

	//!< LinearRepeat
	void CreateSampler_LR(VkSampler* Sampler, const float MaxLOD = (std::numeric_limits<float>::max)()) const;

	//virtual void CreateRenderPass() { CreateRenderPass_1C(RenderPass, ColorFormat); }
	//void CreateRenderPass_1C(VkRenderPass& RenderPass, const VkFormat Format);
	void CreateRenderPass_ColorDepth(VkRenderPass& RP, const VkFormat Color, const VkFormat Depth);
	void CreateRenderPass_ColorDepth_PostProcess(VkRenderPass& RP, const VkFormat Color, const VkFormat Depth);

	virtual void CreateFramebuffer() override { CreateFramebuffer_Color(); }
	void CreateFramebuffer_Color();
	void CreateFramebuffer_ColorDepth();

	void CreateShader_VsPs(std::vector<VkShaderModule>& ShaderModules, std::vector<VkPipelineShaderStageCreateInfo>& PipelineShaderStageCreateInfos) const;
	void CreateShader_VsPsTesTcsGs(std::vector<VkShaderModule>& ShaderModules, std::vector<VkPipelineShaderStageCreateInfo>& PipelineShaderStageCreateInfos) const;
	void CreateShader_Cs(std::vector<VkShaderModule>& ShaderModules, std::vector<VkPipelineShaderStageCreateInfo>& PipelineShaderStageCreateInfos) const;

	template<typename T>
	void CreateUniformBufferT(const T& Type) { CreateUniformBuffer(&UniformBuffer, &UniformDeviceMemory, sizeof(Type), &Type); }

	//virtual void CreateInputAssembly(VkPipelineInputAssemblyStateCreateInfo& PipelineInputAssemblyStateCreateInfo) const override { 
	//	CreateInputAssembly_Topology(PipelineInputAssemblyStateCreateInfo, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP);
	//}

protected:
#if 1
	/**
	@brief �p�C�v���C���쐬���ɃV�F�[�_���̒萔�l���㏑���w��ł���
	
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