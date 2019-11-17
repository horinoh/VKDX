#pragma once

#include "VK.h"

class VKExt : public VK
{
private:
	using Super = VK;
public:
	//using Vertex_Position = struct Vertex_Position { glm::vec3 Position; };
	using Vertex_PositionColor = struct Vertex_PositionColor { glm::vec3 Position; glm::vec4 Color; };
	using Vertex_PositionNormalTexcoord = struct Vertex_PositionNormalTexcoord { glm::vec3 Position; glm::vec3 Normal; glm::vec2 Texcoord; };
	using Instance_OffsetXY = struct Instance_OffsetXY { glm::vec2 Offset; };

	virtual void CreateBuffer_Vertex(const VkQueue /*Queue*/, const VkCommandBuffer /*CB*/, VkBuffer* Buffer, const VkDeviceSize Size, const void* Source) {
		//!< �f�o�C�X���[�J���o�b�t�@(DLB)���쐬 (Create device local buffer(DLB))
		CreateBuffer(Buffer, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, Size);

		//!< �f�o�C�X���[�J��������(DLM)���T�u�A���P�[�g (Suballocate device local memory(DLM))
		uint32_t HeapIndex;
		VkDeviceSize Offset;
		SuballocateBufferMemory(HeapIndex, Offset, *Buffer, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		//!< �X�e�[�W���O��p���Ă�DLB�ւ̃R�s�[�R�}���h�𔭍s(�z�X�g�r�W�u�����쐬���ăf�[�^���R�s�[���A�o�b�t�@�Ԃ̃R�s�[�ɂ��f�o�C�X���[�J���֔��f)
		SubmitStagingCopy(GraphicsQueue, CommandBuffers[0], *Buffer, Size, Source, VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT, VK_PIPELINE_STAGE_VERTEX_INPUT_BIT);
	}
	virtual void CreateBuffer_Index(const VkQueue /*Queue*/, const VkCommandBuffer /*CB*/, VkBuffer* Buffer, const VkDeviceSize Size, const void* Source) {
		CreateBuffer(Buffer, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, Size);

		uint32_t HeapIndex;
		VkDeviceSize Offset;
		SuballocateBufferMemory(HeapIndex, Offset, *Buffer, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		SubmitStagingCopy(GraphicsQueue, CommandBuffers[0], *Buffer, Size, Source, VK_ACCESS_INDEX_READ_BIT, VK_PIPELINE_STAGE_VERTEX_INPUT_BIT);
	}
	virtual void CreateBuffer_Indirect(const VkQueue /*Queue*/, const VkCommandBuffer /*CB*/, VkBuffer* Buffer, const VkDeviceSize Size, const void* Source) {
		CreateBuffer(Buffer, VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, Size);

		uint32_t HeapIndex;
		VkDeviceSize Offset;
		SuballocateBufferMemory(HeapIndex, Offset, *Buffer, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		SubmitStagingCopy(GraphicsQueue, CommandBuffers[0], *Buffer, Size, Source, VK_ACCESS_INDIRECT_COMMAND_READ_BIT, VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT);
	}
	void CreateIndirectBuffer_Draw(const uint32_t IndexCount, const uint32_t InstanceCount) {
		IndirectBuffers.push_back(VkBuffer());
		const VkDrawIndirectCommand DIC = { IndexCount, InstanceCount, 0, 0 };
		CreateBuffer_Indirect(GraphicsQueue, CommandBuffers[0], &IndirectBuffers.back(), static_cast<VkDeviceSize>(sizeof(DIC)), &DIC);
	}
	void CreateIndirectBuffer_DrawIndexed(const uint32_t IndexCount, const uint32_t InstanceCount) {
		IndirectBuffers.push_back(VkBuffer());
		const VkDrawIndexedIndirectCommand DIIC = { IndexCount, InstanceCount, 0, 0, 0 };
		CreateBuffer_Indirect(GraphicsQueue, CommandBuffers[0], &IndirectBuffers.back(), static_cast<VkDeviceSize>(sizeof(DIIC)), &DIIC);
	}
	void CreateIndirectBuffer_Dispatch(const uint32_t X, const uint32_t Y, const uint32_t Z) {
		IndirectBuffers.push_back(VkBuffer());
		const VkDispatchIndirectCommand DIC = { X, Y, Z };
		CreateBuffer_Indirect(GraphicsQueue, CommandBuffers[0], &IndirectBuffers.back(), static_cast<VkDeviceSize>(sizeof(DIC)), &DIC);
	}

	//!< layout(set = 0, binding = 0) buffer MyBuffer { vec4 MyVec4; mat4 MyMat4; }
	void CreateBuffer_Storage(VkBuffer *Buffer, const VkDeviceSize Size) {
		CreateBuffer(Buffer, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, Size);
		//CreateBuffer(Buffer, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, Size); //!< VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC �������
		
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
	
	void CreateShaderModle_VsFs();
	void CreateShaderModle_VsFsTesTcsGs();
	void CreateShaderModle_Cs();

	template<typename T> void CreatePipeline_Vertex(VkPipeline& PL, const VkPipelineLayout PLL, const VkRenderPass RP,
		const VkShaderModule VS, const VkShaderModule FS, const VkShaderModule TES, const VkShaderModule TCS, const VkShaderModule GS,
		VkPipelineCache PC = VK_NULL_HANDLE);
	template<typename T, typename U> void CreatePipeline_Vertex_Instance(VkPipeline& PL, const VkPipelineLayout PLL, const VkRenderPass RP,
		const VkShaderModule VS, const VkShaderModule FS, const VkShaderModule TES, const VkShaderModule TCS, const VkShaderModule GS,
		VkPipelineCache PC = VK_NULL_HANDLE);

	void CreatePipeline_VsFs();
	void CreatePipeline_VsFsTesTcsGs_Tesselation();
	void CreatePipeline_Cs(VkPipeline& /*PL*/) { assert(0 && "TODO"); }
	//!< �������Ńe���v���[�g���ꉻ���Ă��� (Template specialization here)
#include "VKPipeline.inl"

	void CreateRenderPass_ColorDepth(VkRenderPass& RP, const VkFormat Color, const VkFormat Depth);
	void CreateRenderPass_ColorDepth_PostProcess(VkRenderPass& RP, const VkFormat Color, const VkFormat Depth);
	void CreateRenderPass_Color_PostProcess(VkRenderPass& RP, const VkFormat Color);

	void CreateFramebuffer_Color();
	void CreateFramebuffer_ColorDepth();
	virtual void CreateFramebuffer() override { CreateFramebuffer_Color(); }

	//virtual void CreateInputAssembly(VkPipelineInputAssemblyStateCreateInfo& PipelineInputAssemblyStateCreateInfo) const override { 
	//	CreateInputAssembly_Topology(PipelineInputAssemblyStateCreateInfo, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP);
	//}
};