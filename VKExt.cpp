#include "stdafx.h"

#include "VKExt.h"

void VKExt::CreateSampler_LinearRepeat(const float MaxLOD)
{
	const VkSamplerCreateInfo SamplerCreateInfo = {
		VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
		nullptr,
		0,
		VK_FILTER_LINEAR, VK_FILTER_LINEAR, VK_SAMPLER_MIPMAP_MODE_LINEAR,
		VK_SAMPLER_ADDRESS_MODE_REPEAT, VK_SAMPLER_ADDRESS_MODE_REPEAT, VK_SAMPLER_ADDRESS_MODE_REPEAT,
		0.0f,
		VK_FALSE, 0.0f,
		VK_FALSE, VK_COMPARE_OP_NEVER,
		0.0f, MaxLOD,
		VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE,
		VK_FALSE
	};
	VERIFY_SUCCEEDED(vkCreateSampler(Device, &SamplerCreateInfo, nullptr, &Sampler));
}

void VKExt::CreateIndirectBuffer_Indirect4Vertices(const VkCommandBuffer CommandBuffer)
{
	const VkDrawIndirectCommand DrawIndirectCommand = {
		4, 1, 0, 0
	};
	const auto Stride = sizeof(DrawIndirectCommand);
	const auto Size = static_cast<VkDeviceSize>(Stride * 1);

	VkBuffer StagingBuffer = VK_NULL_HANDLE;
	VkDeviceMemory StagingDeviceMemory = VK_NULL_HANDLE;
	{
		//!< ステージング用のバッファとメモリを作成、データをメモリへコピー、バインド
		CreateStagingBufferAndCopyToMemory(&StagingBuffer, &StagingDeviceMemory, Size, &DrawIndirectCommand);

		//!< デバイスローカル用のバッファとメモリを作成、バインド
		CreateDeviceLocalBuffer(&IndirectBuffer, &IndirectDeviceMemory, VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, Size);

		//!< ステージングからデバイスローカルへのコピーコマンドを発行
		SubmitCopyBuffer(CommandBuffer, StagingBuffer, IndirectBuffer, VK_ACCESS_INDIRECT_COMMAND_READ_BIT, VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT, Size);
	}
	if (VK_NULL_HANDLE != StagingDeviceMemory) {
		vkFreeMemory(Device, StagingDeviceMemory, nullptr);
	}
	if (VK_NULL_HANDLE != StagingBuffer) {
		vkDestroyBuffer(Device, StagingBuffer, nullptr);
	}
}
void VKExt::CreateIndirectBuffer_IndexedIndirect(const VkCommandBuffer CommandBuffer)
{
	const VkDrawIndexedIndirectCommand DrawIndexedIndirectCommand = {
		IndexCount, 1, 0, 0, 0
	};
	const auto Stride = sizeof(DrawIndexedIndirectCommand);
	const auto Size = static_cast<VkDeviceSize>(Stride * 1);

	VkBuffer StagingBuffer = VK_NULL_HANDLE;
	VkDeviceMemory StagingDeviceMemory = VK_NULL_HANDLE;
	{
		//!< ステージング用のバッファとメモリを作成、データをメモリへコピー、バインド
		CreateStagingBufferAndCopyToMemory(&StagingBuffer, &StagingDeviceMemory, Size, &DrawIndexedIndirectCommand);

		//!< デバイスローカル用のバッファとメモリを作成、バインド
		CreateDeviceLocalBuffer(&IndirectBuffer, &IndirectDeviceMemory, VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, Size);

		//!< ステージングからデバイスローカルへのコピーコマンドを発行
		SubmitCopyBuffer(CommandBuffer, StagingBuffer, IndirectBuffer, VK_ACCESS_INDIRECT_COMMAND_READ_BIT, VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT, Size);
	}
	if (VK_NULL_HANDLE != StagingDeviceMemory) {
		vkFreeMemory(Device, StagingDeviceMemory, nullptr);
	}
	if (VK_NULL_HANDLE != StagingBuffer) {
		vkDestroyBuffer(Device, StagingBuffer, nullptr);
	}
}

void VKExt::CreaateWriteDescriptorSets_1CIS(std::vector<VkWriteDescriptorSet>& WriteDescriptorSets, VkDescriptorImageInfo* DescriptorImageInfo, VkDescriptorBufferInfo* DescriptorBufferInfo, VkBufferView* BufferView) const
{
	*DescriptorImageInfo = {
		Sampler,
		ImageView,
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
	};
	WriteDescriptorSets.push_back({
		VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, 
		nullptr, 
		DescriptorSets[0], 0, //!< デスクリプタセットとバインディングポイント
		0,
		1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
		DescriptorImageInfo,
		nullptr,
		nullptr
	});
}

void VKExt::CreateRenderPass_Color()
{
	const std::vector<VkAttachmentDescription> AttachmentDescriptions = {
		{
			0,
			ColorFormat,
			VK_SAMPLE_COUNT_1_BIT,
			VK_ATTACHMENT_LOAD_OP_DONT_CARE,		//!< VK_ATTACHMENT_LOAD_OP_CLEAR にするとレンダーパスの開始時にクリアを行う (VkRenderPassBeginInfo.pClearValuesのセットが必須になる)
			VK_ATTACHMENT_STORE_OP_STORE,			//!< レンダーパス終了時に保存する(表示するのに必要)
			VK_ATTACHMENT_LOAD_OP_DONT_CARE,		//!< (ステンシルは)カラーアタッチメントの場合は関係なし
			VK_ATTACHMENT_STORE_OP_DONT_CARE,
			VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,		//!< レンダーパス開始時のレイアウト (メモリバリアなしにサブパス間でレイアウトが変更される)
			VK_IMAGE_LAYOUT_PRESENT_SRC_KHR			//!< レンダーパス終了時のレイアウト
		},
	};

	const std::vector<VkAttachmentReference> ColorAttachmentReferences = {
		{ 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL },
	};
	const std::vector<VkSubpassDescription> SubpassDescriptions = {
		{
			0,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			0, nullptr,
			static_cast<uint32_t>(ColorAttachmentReferences.size()), ColorAttachmentReferences.data(), nullptr,
			nullptr,
			0, nullptr
		},
	};

	//!< サブパス間の依存関係
	const std::vector<VkSubpassDependency> SubpassDependencies = {
		{
			VK_SUBPASS_EXTERNAL,							//!< サブパス外から
			0,												//!< サブパス0へ
			VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,			//!< パイプラインの最終ステージから
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,	//!< カラー出力ステージへ
			VK_ACCESS_MEMORY_READ_BIT,						//!< リードから
			VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,			//!< ライトへ
			VK_DEPENDENCY_BY_REGION_BIT,
		},
		{
			0,
			VK_SUBPASS_EXTERNAL,
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
			VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
			VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
			VK_ACCESS_MEMORY_READ_BIT,
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
	VERIFY_SUCCEEDED(vkCreateRenderPass(Device, &RenderPassCreateInfo, nullptr, &RenderPass));
}
void VKExt::CreateRenderPass_ColorDepth()
{
	const std::vector<VkAttachmentDescription> AttachmentDescriptions = {
		{
			0,
			ColorFormat,
			VK_SAMPLE_COUNT_1_BIT,
			VK_ATTACHMENT_LOAD_OP_DONT_CARE,
			VK_ATTACHMENT_STORE_OP_STORE,
			VK_ATTACHMENT_LOAD_OP_DONT_CARE,
			VK_ATTACHMENT_STORE_OP_DONT_CARE,
			VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
			VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
		},
		{
			0,
			DepthFormat,
			VK_SAMPLE_COUNT_1_BIT,
			VK_ATTACHMENT_LOAD_OP_DONT_CARE,
			VK_ATTACHMENT_STORE_OP_STORE,
			VK_ATTACHMENT_LOAD_OP_DONT_CARE,
			VK_ATTACHMENT_STORE_OP_DONT_CARE,
			VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
			VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
		},
	};

	const std::vector<VkAttachmentReference> ColorAttachmentReferences = {
		{ 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL },
	};
	const VkAttachmentReference DepthAttachmentReference = { 
		1, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL 
	};
	const std::vector<VkSubpassDescription> SubpassDescriptions = {
		{
			0,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			0, nullptr,
			static_cast<uint32_t>(ColorAttachmentReferences.size()), ColorAttachmentReferences.data(), nullptr,
			&DepthAttachmentReference,
			0, nullptr
		}
	};

	const std::vector<VkSubpassDependency> SubpassDependencies = {
		{
			VK_SUBPASS_EXTERNAL,
			0,
			VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
			VK_ACCESS_MEMORY_READ_BIT,
			VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
			VK_DEPENDENCY_BY_REGION_BIT,
		},
		{
			0,
			VK_SUBPASS_EXTERNAL,
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
			VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
			VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
			VK_ACCESS_MEMORY_READ_BIT,
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
	VERIFY_SUCCEEDED(vkCreateRenderPass(Device, &RenderPassCreateInfo, nullptr, &RenderPass));
}

void VKExt::CreateFramebuffer_Color()
{
	Framebuffers.resize(SwapchainImages.size());
	for (uint32_t i = 0; i < Framebuffers.size(); ++i) {
		const std::vector<VkImageView> Attachments = {
			SwapchainImageViews[i],
		};
		const VkFramebufferCreateInfo FramebufferCreateInfo = {
			VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
			nullptr,
			0,
			RenderPass,
			static_cast<uint32_t>(Attachments.size()), Attachments.data(),
			SurfaceExtent2D.width, SurfaceExtent2D.height,
			1
		};
		VERIFY_SUCCEEDED(vkCreateFramebuffer(Device, &FramebufferCreateInfo, nullptr, &Framebuffers[i]));
	}
}
void VKExt::CreateFramebuffer_ColorDepth()
{
	Framebuffers.resize(SwapchainImages.size());
	for (uint32_t i = 0; i < Framebuffers.size(); ++i) {
		const std::vector<VkImageView> Attachments = {
			SwapchainImageViews[i],
			DepthStencilImageView
		};
		const VkFramebufferCreateInfo FramebufferCreateInfo = {
			VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
			nullptr,
			0,
			RenderPass,
			static_cast<uint32_t>(Attachments.size()), Attachments.data(),
			SurfaceExtent2D.width, SurfaceExtent2D.height,
			1
		};
		VERIFY_SUCCEEDED(vkCreateFramebuffer(Device, &FramebufferCreateInfo, nullptr, &Framebuffers[i]));
	}
}

void VKExt::CreateShader_VsPs(std::vector<VkShaderModule>& ShaderModules, std::vector<VkPipelineShaderStageCreateInfo>& PipelineShaderStageCreateInfos) const
{
	const auto ShaderPath = GetBasePath();
	ShaderModules = {
		CreateShaderModule((ShaderPath + L".vert.spv").data()),
		CreateShaderModule((ShaderPath + L".frag.spv").data())
	};
	//!< HLSL コンパイル時のデフォルトエントリポイント名が "main" なのでそれに合わせることにする
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
