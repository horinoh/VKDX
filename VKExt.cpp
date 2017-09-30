#include "stdafx.h"

#include "VKExt.h"

void VKExt::CreateSampler_LR(VkSampler* Sampler, const float MaxLOD) const
{
	[&](VkSampler* Sampler, const float MaxLOD) {
		const VkSamplerCreateInfo SamplerCreateInfo = {
			VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
			nullptr,
			0,
			VK_FILTER_LINEAR, VK_FILTER_LINEAR, VK_SAMPLER_MIPMAP_MODE_LINEAR,
			VK_SAMPLER_ADDRESS_MODE_REPEAT, VK_SAMPLER_ADDRESS_MODE_REPEAT, VK_SAMPLER_ADDRESS_MODE_REPEAT,
			0.0f,
			VK_FALSE, 1.0f,
			VK_FALSE, VK_COMPARE_OP_NEVER,
			0.0f, MaxLOD,
			VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE,
			VK_FALSE
		};
		VERIFY_SUCCEEDED(vkCreateSampler(Device, &SamplerCreateInfo, GetAllocationCallbacks(), Sampler));
	}(Sampler, MaxLOD);
}

void VKExt::CreateIndirectBuffer_Vertices(const uint32_t Count)
{
	const VkDrawIndirectCommand DrawIndirectCommand = {
		Count, 1, 0, 0
	};
	const auto Stride = sizeof(DrawIndirectCommand);
	const auto Size = static_cast<VkDeviceSize>(Stride * 1);

	[&](VkBuffer* Buffer, VkDeviceMemory* DeviceMemory, const VkDeviceSize Size, const void* Data, const VkCommandBuffer CB) {
		VkBuffer StagingBuffer = VK_NULL_HANDLE;
		VkDeviceMemory StagingDeviceMemory = VK_NULL_HANDLE;
		{
			//!< �z�X�g�r�W�u���̃o�b�t�@�ƃ��������쐬�A�f�[�^���R�s�[ Create host visible buffer and memory, and copy data
			CreateBuffer(&StagingBuffer, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, Size);
			CreateHostVisibleMemory(&StagingDeviceMemory, StagingBuffer);
			CopyToHostVisibleMemory(StagingDeviceMemory, Size, Data);
			BindDeviceMemory(StagingBuffer, StagingDeviceMemory);

			//!< �f�o�C�X���[�J���̃o�b�t�@�ƃ��������쐬 Create device local buffer and memory
			CreateBuffer(Buffer, VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, Size);
			CreateDeviceLocalMemory(DeviceMemory, *Buffer);
			BindDeviceMemory(*Buffer, *DeviceMemory);

			//!< �z�X�g�r�W�u������f�o�C�X���[�J���ւ̃R�s�[�R�}���h�𔭍s Submit copy command host visible to device local
			SubmitCopyBuffer(CB, StagingBuffer, *Buffer, VK_ACCESS_INDIRECT_COMMAND_READ_BIT, VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT, Size);
		}
		if (VK_NULL_HANDLE != StagingDeviceMemory) {
			vkFreeMemory(Device, StagingDeviceMemory, GetAllocationCallbacks());
		}
		if (VK_NULL_HANDLE != StagingBuffer) {
			vkDestroyBuffer(Device, StagingBuffer, GetAllocationCallbacks());
		}
	}(&IndirectBuffer, &IndirectDeviceMemory, Size, &DrawIndirectCommand, CommandBuffers[0]);
}
void VKExt::CreateIndirectBuffer_Indexed(const uint32_t Count)
{
	const VkDrawIndexedIndirectCommand DrawIndexedIndirectCommand = {
		Count, 1, 0, 0, 0
	};
	const auto Stride = sizeof(DrawIndexedIndirectCommand);
	const auto Size = static_cast<VkDeviceSize>(Stride * 1);

	[&](VkBuffer* Buffer, VkDeviceMemory* DeviceMemory, const VkDeviceSize Size, const void* Data, const VkCommandBuffer CB) {
		VkBuffer StagingBuffer = VK_NULL_HANDLE;
		VkDeviceMemory StagingDeviceMemory = VK_NULL_HANDLE;
		{
			//!< �z�X�g�r�W�u���̃o�b�t�@�ƃ��������쐬�A�f�[�^���R�s�[ Create host visible buffer and memory, and copy data
			CreateBuffer(&StagingBuffer, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, Size);
			CreateHostVisibleMemory(&StagingDeviceMemory, StagingBuffer);
			CopyToHostVisibleMemory(StagingDeviceMemory, Size, Data);
			BindDeviceMemory(StagingBuffer, StagingDeviceMemory);

			//!< �f�o�C�X���[�J���̃o�b�t�@�ƃ��������쐬 Create device local buffer and memory
			CreateBuffer(Buffer, VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, Size);
			CreateDeviceLocalMemory(DeviceMemory, *Buffer);
			BindDeviceMemory(*Buffer, *DeviceMemory);

			//!< �z�X�g�r�W�u������f�o�C�X���[�J���ւ̃R�s�[�R�}���h�𔭍s Submit copy command host visible to device local
			SubmitCopyBuffer(CB, StagingBuffer, *Buffer, VK_ACCESS_INDIRECT_COMMAND_READ_BIT, VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT, Size);
		}
		if (VK_NULL_HANDLE != StagingDeviceMemory) {
			vkFreeMemory(Device, StagingDeviceMemory, GetAllocationCallbacks());
		}
		if (VK_NULL_HANDLE != StagingBuffer) {
			vkDestroyBuffer(Device, StagingBuffer, GetAllocationCallbacks());
		}
	}(&IndirectBuffer, &IndirectDeviceMemory, Size, &DrawIndexedIndirectCommand, CommandBuffers[0]);
}

void VKExt::CreaateWriteDescriptorSets_1UB(VkWriteDescriptorSet& WriteDescriptorSet, const std::vector<VkDescriptorBufferInfo>& DescriptorBufferInfos) const
{
	WriteDescriptorSet = {
		VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
		nullptr,
		DescriptorSets[0], 0, 0,//!< �f�X�N���v�^�Z�b�g�A�o�C���f�B���O�|�C���g�A�z��̏ꍇ�̓Y��(�z��łȂ����0)
		static_cast<uint32_t>(DescriptorBufferInfos.size()),
		VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
		nullptr,
		DescriptorBufferInfos.data(),
		nullptr
	};
}
void VKExt::UpdateDescriptorSet_1UB()
{
	[&](const VkBuffer Buffer) {
		std::vector<VkWriteDescriptorSet> WriteDescriptorSets;
		WriteDescriptorSets.resize(1);
		const std::vector<VkDescriptorBufferInfo> DescriptorBufferInfos = {
			{
				Buffer,
				0, //!< �I�t�Z�b�g (�v�A���C�����g)
				VK_WHOLE_SIZE
			},
		};
		CreateWriteDescriptorSets(WriteDescriptorSets.back(), {}, DescriptorBufferInfos, {});

		std::vector<VkCopyDescriptorSet> CopyDescriptorSets;

		vkUpdateDescriptorSets(Device,
			static_cast<uint32_t>(WriteDescriptorSets.size()), WriteDescriptorSets.data(),
			static_cast<uint32_t>(CopyDescriptorSets.size()), CopyDescriptorSets.data());
	}(UniformBuffer);
}

void VKExt::CreaateWriteDescriptorSets_1CIS(VkWriteDescriptorSet& WriteDescriptorSet, const std::vector<VkDescriptorImageInfo>& DescriptorImageInfos) const
{
	WriteDescriptorSet = {
		VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, 
		nullptr, 
		DescriptorSets[0], 0, 0,//!< �f�X�N���v�^�Z�b�g�A�o�C���f�B���O�|�C���g�A�z��̏ꍇ�̓Y��(�z��łȂ����0)
		static_cast<uint32_t>(DescriptorImageInfos.size()),
		VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
		DescriptorImageInfos.data(),
		nullptr,
		nullptr
	};
}
void VKExt::UpdateDescriptorSet_1CIS()
{
	[&](const VkSampler Sampler, const VkImageView ImageView) {
		std::vector<VkWriteDescriptorSet> WriteDescriptorSets;
		WriteDescriptorSets.resize(1);

		const std::vector<VkDescriptorImageInfo> DescriptorImageInfos = {
			{
				Sampler,
				ImageView,
				VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
			},
		};
		CreateWriteDescriptorSets(WriteDescriptorSets.back(), DescriptorImageInfos, {}, {});

	std::vector<VkCopyDescriptorSet> CopyDescriptorSets;

	vkUpdateDescriptorSets(Device,
		static_cast<uint32_t>(WriteDescriptorSets.size()), WriteDescriptorSets.data(),
		static_cast<uint32_t>(CopyDescriptorSets.size()), CopyDescriptorSets.data());

	}(Samplers[0], ImageView);
}

void VKExt::CreateRenderPass_Color()
{
	[&](VkRenderPass* RenderPass, const VkFormat ColorFormat) {
		const std::vector<VkAttachmentDescription> AttachmentDescriptions = {
			{
				0,
				ColorFormat,
				VK_SAMPLE_COUNT_1_BIT,
				VK_ATTACHMENT_LOAD_OP_DONT_CARE,		//!< VK_ATTACHMENT_LOAD_OP_CLEAR �ɂ���ƃ����_�[�p�X�̊J�n���ɃN���A���s�� (VkRenderPassBeginInfo.pClearValues�̃Z�b�g���K�{�ɂȂ�)
				VK_ATTACHMENT_STORE_OP_STORE,			//!< �����_�[�p�X�I�����ɕۑ�����(�\������̂ɕK�v)
				VK_ATTACHMENT_LOAD_OP_DONT_CARE,		//!< (�����ł�)�X�e���V���͖��g�p
				VK_ATTACHMENT_STORE_OP_DONT_CARE,		//!< (�����ł�)�X�e���V���͖��g�p
				VK_IMAGE_LAYOUT_UNDEFINED,				//!< �����_�[�p�X�J�n���̃��C�A�E�g (�������o���A�Ȃ��Ƀ����_�[�p�X�ԂŃ��C�A�E�g���ύX�����)
				VK_IMAGE_LAYOUT_PRESENT_SRC_KHR			//!< �����_�[�p�X�I�����̃��C�A�E�g
			},
		};

		const std::vector<VkAttachmentReference> InputAttachmentReferences = {};
		const std::vector<VkAttachmentReference> ColorAttachmentReferences = {
			{ 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL },
		};
		//const std::vector<VkAttachmentReference> ResolveAttachmentReferences = {
		//	{ VK_ATTACHMENT_UNUSED },
		//}; 
		//assert(ColorAttachmentReferences.size() == ResolveAttachmentReferences.size() && "Size must be same");
		const VkAttachmentReference* DepthStencilAttachmentReference = nullptr;
		const std::vector<uint32_t> PreserveAttachments = {}; //!< ���̃T�u�o�X���ł͎g�p���Ȃ����A�T�u�p�X�S�̂ɂ����ăR���e���c��ێ����Ȃ��Ă͂Ȃ�Ȃ��C���f�b�N�X���w��

		const std::vector<VkSubpassDescription> SubpassDescriptions = {
			{
				0,
				VK_PIPELINE_BIND_POINT_GRAPHICS,
				static_cast<uint32_t>(InputAttachmentReferences.size()), InputAttachmentReferences.data(),
				static_cast<uint32_t>(ColorAttachmentReferences.size()), ColorAttachmentReferences.data(), nullptr/*ResolveAttachmentReferences.data()*/,
				DepthStencilAttachmentReference,
				static_cast<uint32_t>(PreserveAttachments.size()), PreserveAttachments.data()
			},
		};

		//!< �T�u�p�X�Ԃ̈ˑ��֌W (�O�̕`�挋�ʂ����̏����Ŏg�p����ꍇ�Ȃ�)
		const std::vector<VkSubpassDependency> SubpassDependencies = {
#if 0
			//!< �K�v�������A�����ď����Ȃ炱��Ȋ��� No need this code, but if dare to write like this
			{
				VK_SUBPASS_EXTERNAL,							//!< �T�u�p�X�O����
				0,												//!< �T�u�p�X0��
				VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,			//!< �p�C�v���C���̍ŏI�X�e�[�W����
				VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,	//!< �J���[�o�̓X�e�[�W��
				VK_ACCESS_MEMORY_READ_BIT,						//!< �ǂݍ��݂���
				VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,			//!< �J���[�������݂�
				VK_DEPENDENCY_BY_REGION_BIT,					//!< �����������̈�ɑ΂��鏑�����݂��������Ă���ǂݍ��� (�w�肵�Ȃ��ꍇ�͎��O�ŏ������݊������Ǘ�)
			},
			{
				0,												//!< �T�u�p�X0����
				VK_SUBPASS_EXTERNAL,							//!< �T�u�p�X�O��
				VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,	//!< �J���[�o�̓X�e�[�W����
				VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,			//!< �p�C�v���C���̍ŏI�X�e�[�W��
				VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,			//!< �J���[�������݂���
				VK_ACCESS_MEMORY_READ_BIT,						//!< �ǂݍ��݂�
				VK_DEPENDENCY_BY_REGION_BIT,
			},
#endif
		};

		const VkRenderPassCreateInfo RenderPassCreateInfo = {
			VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
			nullptr,
			0,
			static_cast<uint32_t>(AttachmentDescriptions.size()), AttachmentDescriptions.data(),
			static_cast<uint32_t>(SubpassDescriptions.size()), SubpassDescriptions.data(),
			static_cast<uint32_t>(SubpassDependencies.size()), SubpassDependencies.data()
		};
		VERIFY_SUCCEEDED(vkCreateRenderPass(Device, &RenderPassCreateInfo, GetAllocationCallbacks(), RenderPass));
	}(&RenderPass, ColorFormat);
}
void VKExt::CreateRenderPass_ColorDepth()
{
	[&](VkRenderPass* RenderPass, const VkFormat ColorFormat, const VkFormat DepthFormat) {
		const std::vector<VkAttachmentDescription> AttachmentDescriptions = {
			{
				0,
				ColorFormat,
				VK_SAMPLE_COUNT_1_BIT,
				VK_ATTACHMENT_LOAD_OP_DONT_CARE,
				VK_ATTACHMENT_STORE_OP_STORE,
				VK_ATTACHMENT_LOAD_OP_DONT_CARE,
				VK_ATTACHMENT_STORE_OP_DONT_CARE,
				VK_IMAGE_LAYOUT_UNDEFINED,
				VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
			},
			{
				0,
				DepthFormat,
				VK_SAMPLE_COUNT_1_BIT,
				VK_ATTACHMENT_LOAD_OP_CLEAR,
				VK_ATTACHMENT_STORE_OP_STORE,
				VK_ATTACHMENT_LOAD_OP_DONT_CARE,
				VK_ATTACHMENT_STORE_OP_DONT_CARE,
				VK_IMAGE_LAYOUT_UNDEFINED,
				VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
			},
		};

		const std::vector<VkAttachmentReference> InputAttachmentReferences = {};
		const std::vector<VkAttachmentReference> ColorAttachmentReferences = {
			{ 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL },
		};
		const VkAttachmentReference DepthStencilAttachmentReference = {
			1, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
		};
		const std::vector<uint32_t> PreserveAttachments = {};
		const std::vector<VkSubpassDescription> SubpassDescriptions = {
			{
				0,
				VK_PIPELINE_BIND_POINT_GRAPHICS,
				static_cast<uint32_t>(InputAttachmentReferences.size()), InputAttachmentReferences.data(),
				static_cast<uint32_t>(ColorAttachmentReferences.size()), ColorAttachmentReferences.data(), nullptr,
				&DepthStencilAttachmentReference,
				static_cast<uint32_t>(PreserveAttachments.size()), PreserveAttachments.data()
			}
		};

		const std::vector<VkSubpassDependency> SubpassDependencies = {};

		const VkRenderPassCreateInfo RenderPassCreateInfo = {
			VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
			nullptr,
			0,
			static_cast<uint32_t>(AttachmentDescriptions.size()), AttachmentDescriptions.data(),
			static_cast<uint32_t>(SubpassDescriptions.size()), SubpassDescriptions.data(),
			static_cast<uint32_t>(SubpassDependencies.size()), SubpassDependencies.data()
		};
		VERIFY_SUCCEEDED(vkCreateRenderPass(Device, &RenderPassCreateInfo, GetAllocationCallbacks(), RenderPass));
	}(&RenderPass, ColorFormat, DepthFormat);
}

//!< �t�@�[�X�g�p�X�� ColorDepth �ɏ������݁A�Z�J���h�p�X�� PostProcess ���s���ꍇ�̗� In first pass ColorDepth, second pass PostProcess
void VKExt::CreateRenderPass_CD_PP()
{
	[&](VkRenderPass* RenderPass, const VkFormat ColorFormat, const VkFormat DepthFormat) {
		const std::vector<VkAttachmentDescription> AttachmentDescriptions = {
			{
				0,
				ColorFormat,
				VK_SAMPLE_COUNT_1_BIT,
				VK_ATTACHMENT_LOAD_OP_DONT_CARE,
				VK_ATTACHMENT_STORE_OP_DONT_CARE,
				VK_ATTACHMENT_LOAD_OP_DONT_CARE,
				VK_ATTACHMENT_STORE_OP_DONT_CARE,
				VK_IMAGE_LAYOUT_UNDEFINED,
				VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
			},
			{
				0,
				DepthFormat,
				VK_SAMPLE_COUNT_1_BIT,
				VK_ATTACHMENT_LOAD_OP_CLEAR,
				VK_ATTACHMENT_STORE_OP_STORE,
				VK_ATTACHMENT_LOAD_OP_DONT_CARE,
				VK_ATTACHMENT_STORE_OP_DONT_CARE,
				VK_IMAGE_LAYOUT_UNDEFINED,
				VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
			},
			{
				0,
				ColorFormat,
				VK_SAMPLE_COUNT_1_BIT,
				VK_ATTACHMENT_LOAD_OP_DONT_CARE,
				VK_ATTACHMENT_STORE_OP_STORE,
				VK_ATTACHMENT_LOAD_OP_DONT_CARE,
				VK_ATTACHMENT_STORE_OP_DONT_CARE,
				VK_IMAGE_LAYOUT_UNDEFINED,
				VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
			},
		};

		const std::vector<VkAttachmentReference> InputAttachmentReferences_Pass0 = {};
		const std::vector<VkAttachmentReference> ColorAttachmentReferences_Pass0 = { { 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL }, };
		const VkAttachmentReference DepthStencilAttachmentReference_Pass0 = { 1, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL };

		const std::vector<VkAttachmentReference> InputAttachmentReferences_Pass1 = { { 0, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL }, };
		const std::vector<VkAttachmentReference> ColorAttachmentReferences_Pass1 = { { 2, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL }, };

		const std::vector<uint32_t> PreserveAttachments = {};
		const std::vector<VkSubpassDescription> SubpassDescriptions = {
			{
				0,
				VK_PIPELINE_BIND_POINT_GRAPHICS,
				static_cast<uint32_t>(InputAttachmentReferences_Pass0.size()), InputAttachmentReferences_Pass0.data(),
				static_cast<uint32_t>(ColorAttachmentReferences_Pass0.size()), ColorAttachmentReferences_Pass0.data(), nullptr,
				&DepthStencilAttachmentReference_Pass0,
				static_cast<uint32_t>(PreserveAttachments.size()), PreserveAttachments.data()
			},
			{
				0,
				VK_PIPELINE_BIND_POINT_GRAPHICS,
				static_cast<uint32_t>(InputAttachmentReferences_Pass1.size()), InputAttachmentReferences_Pass1.data(),
				static_cast<uint32_t>(ColorAttachmentReferences_Pass1.size()), ColorAttachmentReferences_Pass1.data(), nullptr,
				nullptr,
				static_cast<uint32_t>(PreserveAttachments.size()), PreserveAttachments.data()
			}
		};


		const std::vector<VkSubpassDependency> SubpassDependencies = {
			{
				0,
				1,
				VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
				VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
				VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
				VK_ACCESS_INPUT_ATTACHMENT_READ_BIT,
				VK_DEPENDENCY_BY_REGION_BIT,
			},
		};

		const VkRenderPassCreateInfo RenderPassCreateInfo = {
			VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
			nullptr,
			0,
			static_cast<uint32_t>(AttachmentDescriptions.size()), AttachmentDescriptions.data(),
			static_cast<uint32_t>(SubpassDescriptions.size()), SubpassDescriptions.data(),
			static_cast<uint32_t>(SubpassDependencies.size()), SubpassDependencies.data()
		};
		VERIFY_SUCCEEDED(vkCreateRenderPass(Device, &RenderPassCreateInfo, GetAllocationCallbacks(), RenderPass));
	}(&RenderPass, ColorFormat, DepthFormat);
}

void VKExt::CreateFramebuffer_Color()
{
	Framebuffers.resize(SwapchainImages.size());
	for (uint32_t i = 0; i < Framebuffers.size(); ++i) {
		[&](VkFramebuffer* Framebuffer, const VkImageView ColorView, const VkRenderPass RenderPass, const uint32_t Width, const uint32_t Height) {
			const std::vector<VkImageView> Attachments = {
				ColorView 
			};
			const VkFramebufferCreateInfo FramebufferCreateInfo = {
				VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
				nullptr,
				0,
				RenderPass, //!< �����ō쐬����t���[���o�b�t�@�� RenderPass �Ɓu�R���p�`�v�ȕʂ̃����_�[�p�X�ł��g�p�\
				static_cast<uint32_t>(Attachments.size()), Attachments.data(),
				Width, Height,
				1
			};
			VERIFY_SUCCEEDED(vkCreateFramebuffer(Device, &FramebufferCreateInfo, GetAllocationCallbacks(), Framebuffer));
		}(&Framebuffers[i], SwapchainImageViews[i], RenderPass, SurfaceExtent2D.width, SurfaceExtent2D.height);
	}
}
void VKExt::CreateFramebuffer_ColorDepth()
{
	Framebuffers.resize(SwapchainImages.size());
	for (uint32_t i = 0; i < Framebuffers.size(); ++i) {
		[&](VkFramebuffer* Framebuffer, const VkImageView ColorView, const VkImageView DepthStencilView, const VkRenderPass RenderPass, const uint32_t Width, const uint32_t Height) {
			const std::vector<VkImageView> Attachments = {
				ColorView,
				DepthStencilView
			};
			const VkFramebufferCreateInfo FramebufferCreateInfo = {
				VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
				nullptr,
				0,
				RenderPass,
				static_cast<uint32_t>(Attachments.size()), Attachments.data(),
				Width, Height,
				1
			};
			VERIFY_SUCCEEDED(vkCreateFramebuffer(Device, &FramebufferCreateInfo, GetAllocationCallbacks(), Framebuffer));
		}(&Framebuffers[i], SwapchainImageViews[i], DepthStencilImageView, RenderPass, SurfaceExtent2D.width, SurfaceExtent2D.height);
	}
}

void VKExt::CreateShader_VsPs(std::vector<VkShaderModule>& ShaderModules, std::vector<VkPipelineShaderStageCreateInfo>& PipelineShaderStageCreateInfos) const
{
	const auto ShaderPath = GetBasePath();
	ShaderModules = {
		CreateShaderModule((ShaderPath + L".vert.spv").data()),
		CreateShaderModule((ShaderPath + L".frag.spv").data())
	};

	//!< HLSL �R���p�C�����̃f�t�H���g�G���g���|�C���g���� "main" �Ȃ̂ł���ɍ��킹�邱�Ƃɂ���
	const char* EntrypointName = "main";
	PipelineShaderStageCreateInfos = {
		{
			VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
			nullptr,
			0,
			VK_SHADER_STAGE_VERTEX_BIT, ShaderModules[0],
			EntrypointName,
			nullptr
		},
		{
			VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
			nullptr,
			0,
			VK_SHADER_STAGE_FRAGMENT_BIT, ShaderModules[1],
			EntrypointName,
			nullptr//&SpecializationInfo
		}
	};
}
void VKExt::CreateShader_VsPsTesTcsGs(std::vector<VkShaderModule>& ShaderModules, std::vector<VkPipelineShaderStageCreateInfo>& PipelineShaderStageCreateInfos) const
{
	const auto ShaderPath = GetBasePath();
	ShaderModules = {
		CreateShaderModule((ShaderPath + L".vert.spv").data()),
		CreateShaderModule((ShaderPath + L".frag.spv").data()),
		CreateShaderModule((ShaderPath + L".tese.spv").data()),
		CreateShaderModule((ShaderPath + L".tesc.spv").data()),
		CreateShaderModule((ShaderPath + L".geom.spv").data()) 
	};
	const char* EntrypointName = "main";
	PipelineShaderStageCreateInfos = {
		{
			VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
			nullptr,
			0,
			VK_SHADER_STAGE_VERTEX_BIT, ShaderModules[0],
			EntrypointName,
			nullptr
		},
		{
			VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
			nullptr,
			0,
			VK_SHADER_STAGE_FRAGMENT_BIT, ShaderModules[1],
			EntrypointName,
			nullptr
		},
		{
			VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
			nullptr,
			0,
			VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT, ShaderModules[2],
			EntrypointName,
			nullptr
		},
		{
			VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
			nullptr,
			0,
			VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT, ShaderModules[3],
			EntrypointName,
			nullptr
		},
		{
			VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
			nullptr,
			0,
			VK_SHADER_STAGE_GEOMETRY_BIT, ShaderModules[4],
			EntrypointName,
			nullptr
		},
	};
}
