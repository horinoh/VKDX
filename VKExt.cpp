#include "stdafx.h"

#include "VK.h"

void VKExt::CreateShader_VsPs()
{
	ShaderModules.push_back(CreateShaderModule(SHADER_PATH L"VS.vert.spv"));
	ShaderModules.push_back(CreateShaderModule(SHADER_PATH L"FS.frag.spv"));

#ifdef _DEBUG
	std::cout << "CreateShader" << COUT_OK << std::endl << std::endl;
#endif
}
void VKExt::CreateShader_VsPsTesTcsGs()
{
	ShaderModules.push_back(CreateShaderModule(SHADER_PATH L"VS.vert.spv"));
	ShaderModules.push_back(CreateShaderModule(SHADER_PATH L"FS.frag.spv"));
	ShaderModules.push_back(CreateShaderModule(SHADER_PATH L"TES.tese.spv"));
	ShaderModules.push_back(CreateShaderModule(SHADER_PATH L"TCS.tesc.spv"));
	ShaderModules.push_back(CreateShaderModule(SHADER_PATH L"GS.geom.spv"));

#ifdef _DEBUG
	std::cout << "CreateShader" << COUT_OK << std::endl << std::endl;
#endif
}
void VKExt::CreateShader_Cs()
{
	ShaderModules.push_back(CreateShaderModule(SHADER_PATH L"CS.cpom.spv"));

#ifdef _DEBUG
	std::cout << "CreateShader" << COUT_OK << std::endl << std::endl;
#endif
}

void VKExt::CreateVertexInput_Position()
{
	VertexInputBindingDescriptions = {
		{ 0, sizeof(glm::vec3), VK_VERTEX_INPUT_RATE_VERTEX }
	};
	VertexInputAttributeDescriptions = {
		{ 0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0 },
	};
	PipelineVertexInputStateCreateInfo = {
		VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
		nullptr,
		0,
		static_cast<uint32_t>(VertexInputBindingDescriptions.size()), VertexInputBindingDescriptions.data(),
		static_cast<uint32_t>(VertexInputAttributeDescriptions.size()), VertexInputAttributeDescriptions.data()
	};

#ifdef _DEBUG
	std::cout << "CreateVertexInput" << COUT_OK << std::endl << std::endl;
#endif
}
void VKExt::CreateVertexInput_PositionColor()
{
	VertexInputBindingDescriptions = {
		{ 0, sizeof(glm::vec3) + sizeof(glm::vec4), VK_VERTEX_INPUT_RATE_VERTEX }
	};
	VertexInputAttributeDescriptions = {
		{ 0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0 },
		{ 1, 0, VK_FORMAT_R32G32B32_SFLOAT, sizeof(glm::vec3) }
	};
	PipelineVertexInputStateCreateInfo = {
		VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
		nullptr,
		0,
		static_cast<uint32_t>(VertexInputBindingDescriptions.size()), VertexInputBindingDescriptions.data(),
		static_cast<uint32_t>(VertexInputAttributeDescriptions.size()), VertexInputAttributeDescriptions.data()
	};

#ifdef _DEBUG
	std::cout << "CreateVertexInput" << COUT_OK << std::endl << std::endl;
#endif
}

void VKExt::CreateGraphicsPipeline_VsPs()
{
	assert(1 < ShaderModules.size() && "ShaderModules is not enough");

	//!< HLSL コンパイル時のデフォルトエントリポイント名が "main" なのでそれに合わせることにする
	const char* EntrypointName = "main";
	const std::vector<VkPipelineShaderStageCreateInfo> PipelineShaderStageCreateInfos = {
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

	//!< PipelineVertexInputStateCreateInfo は CreateVertexInput() 内で作成してある

	const VkPipelineInputAssemblyStateCreateInfo PipelineInputAssemblyStateCreateInfo = {
		VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
		nullptr,
		0,
		VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
		VK_FALSE
	};

	/**
	@brief デフォルト指定
	ここでは VkDynamicState に VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR を指定するので
	vkCmdSetViewport(), vkCmdSetScissor() で後からコマンドバッファによる上書きが可能
	*/
	const VkPipelineViewportStateCreateInfo PipelineViewportStateCreateInfo = {
		VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
		nullptr,
		0,
		static_cast<uint32_t>(Viewports.size()), Viewports.data(),
		static_cast<uint32_t>(ScissorRects.size()), ScissorRects.data()
	};

	const VkPipelineRasterizationStateCreateInfo PipelineRasterizationStateCreateInfo = {
		VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
		nullptr,
		0,
		VK_FALSE,
		VK_FALSE,
		VK_POLYGON_MODE_FILL,
		VK_CULL_MODE_NONE,/*VK_CULL_MODE_BACK_BIT,*/
		VK_FRONT_FACE_COUNTER_CLOCKWISE,
		VK_FALSE, 0.0f, 0.0f, 0.0f,
		1.0f
	};

	const VkPipelineMultisampleStateCreateInfo PipelineMultisampleStateCreateInfo = {
		VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
		nullptr,
		0,
		VK_SAMPLE_COUNT_1_BIT/*VK_SAMPLE_COUNT_4_BIT*/,
		VK_FALSE, 0.0f,
		nullptr,
		VK_FALSE, VK_FALSE
	};

	const VkStencilOpState StencilOpState_Front = {
		VK_STENCIL_OP_KEEP,
		VK_STENCIL_OP_KEEP,
		VK_STENCIL_OP_KEEP,
		VK_COMPARE_OP_NEVER, 0, 0, 0
	};
	const VkStencilOpState StencilOpState_Back = {
		VK_STENCIL_OP_KEEP,
		VK_STENCIL_OP_KEEP,
		VK_STENCIL_OP_KEEP,
		VK_COMPARE_OP_ALWAYS, 0, 0, 0
	};
	const VkPipelineDepthStencilStateCreateInfo PipelineDepthStencilStateCreateInfo = {
		VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
		nullptr,
		0,
		VK_TRUE,
		VK_TRUE,
		VK_COMPARE_OP_LESS_OR_EQUAL,
		VK_FALSE,
		VK_FALSE,
		StencilOpState_Front,
		StencilOpState_Back,
		0.0f, 0.0f
	};

	const std::vector<VkPipelineColorBlendAttachmentState> PipelineColorBlendAttachmentStates = {
		{
			VK_FALSE,
			VK_BLEND_FACTOR_ZERO, VK_BLEND_FACTOR_ZERO,
			VK_BLEND_OP_ADD,
			VK_BLEND_FACTOR_ZERO, VK_BLEND_FACTOR_ZERO,
			VK_BLEND_OP_ADD,
			0xf,
		}
	};
	const VkPipelineColorBlendStateCreateInfo PipelineColorBlendStateCreateInfo = {
		VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
		nullptr,
		0,
		VK_FALSE, VK_LOGIC_OP_CLEAR,
		static_cast<uint32_t>(PipelineColorBlendAttachmentStates.size()), PipelineColorBlendAttachmentStates.data(),
		{ 0.0f, 0.0f, 0.0f, 0.0f } //!< float blendConstants[4];
	};

	const std::vector<VkDynamicState> DynamicStates = {
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR
	};
	const VkPipelineDynamicStateCreateInfo PipelineDynamicStateCreateInfo = {
		VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
		nullptr,
		0,
		static_cast<uint32_t>(DynamicStates.size()), DynamicStates.data()
	};

	const std::vector<VkGraphicsPipelineCreateInfo> GraphicsPipelineCreateInfos = {
		{
			VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
			nullptr,
			0,
			static_cast<uint32_t>(PipelineShaderStageCreateInfos.size()), PipelineShaderStageCreateInfos.data(),
			&PipelineVertexInputStateCreateInfo,
			&PipelineInputAssemblyStateCreateInfo,
			nullptr,
			&PipelineViewportStateCreateInfo,
			&PipelineRasterizationStateCreateInfo,
			&PipelineMultisampleStateCreateInfo,
			&PipelineDepthStencilStateCreateInfo,
			&PipelineColorBlendStateCreateInfo,
			&PipelineDynamicStateCreateInfo,
			PipelineLayout,
			RenderPass,
			0,
			VK_NULL_HANDLE, 0
		}
	};
	//!< パイプラインをコンパイルして、vkGetPipelineCacheData()でディスクへ保存可能
	const VkPipelineCacheCreateInfo PipelineCacheCreateInfo = {
		VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO,
		nullptr,
		0,
		0, nullptr
	};
	VERIFY_SUCCEEDED(vkCreatePipelineCache(Device, &PipelineCacheCreateInfo, nullptr, &PipelineCache));
	VERIFY_SUCCEEDED(vkCreateGraphicsPipelines(Device, PipelineCache, static_cast<uint32_t>(GraphicsPipelineCreateInfos.size()), GraphicsPipelineCreateInfos.data(), nullptr, &Pipeline));

#ifdef _DEBUG
	std::cout << "CreateGraphicsPipeline" << COUT_OK << std::endl << std::endl;
#endif
}
void VKExt::CreateGraphicsPipeline_VsPsTesTcsGs()
{
#ifdef _DEBUG
	std::cout << "CreateGraphicsPipeline" << COUT_OK << std::endl << std::endl;
#endif
}

void VKExt::CreateRenderPass_Color(const VkFormat ColorFormat)
{
	const std::vector<VkAttachmentDescription> AttachmentDescriptions = {
		{
			0,
			ColorFormat,
			VK_SAMPLE_COUNT_1_BIT,
			VK_ATTACHMENT_LOAD_OP_CLEAR,
			VK_ATTACHMENT_STORE_OP_STORE,
			VK_ATTACHMENT_LOAD_OP_DONT_CARE,
			VK_ATTACHMENT_STORE_OP_DONT_CARE,
			VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
		},
	};

	const std::vector<VkAttachmentReference> ColorAttachmentReferences = {
		{ 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL }
	};
	const std::vector<VkSubpassDescription> SubpassDescriptions = {
		{
			0,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			0, nullptr,
			static_cast<uint32_t>(ColorAttachmentReferences.size()), ColorAttachmentReferences.data(),
			nullptr,
			nullptr,
			0, nullptr
		}
	};

	const VkRenderPassCreateInfo RenderPassCreateInfo = {
		VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
		nullptr,
		0,
		static_cast<uint32_t>(AttachmentDescriptions.size()), AttachmentDescriptions.data(),
		static_cast<uint32_t>(SubpassDescriptions.size()), SubpassDescriptions.data(),
		0, nullptr
	};
	VERIFY_SUCCEEDED(vkCreateRenderPass(Device, &RenderPassCreateInfo, nullptr, &RenderPass));
}
void VKExt::CreateRenderPass_ColorDepth(const VkFormat ColorFormat, const VkFormat DepthFormat)
{
	const std::vector<VkAttachmentDescription> AttachmentDescriptions = {
		{
			0,
			ColorFormat,
			VK_SAMPLE_COUNT_1_BIT,
			VK_ATTACHMENT_LOAD_OP_CLEAR,
			VK_ATTACHMENT_STORE_OP_STORE,
			VK_ATTACHMENT_LOAD_OP_DONT_CARE,
			VK_ATTACHMENT_STORE_OP_DONT_CARE,
			VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
		},
		{
			0,
			DepthFormat,
			VK_SAMPLE_COUNT_1_BIT,
			VK_ATTACHMENT_LOAD_OP_CLEAR,
			VK_ATTACHMENT_STORE_OP_STORE,
			VK_ATTACHMENT_LOAD_OP_DONT_CARE,
			VK_ATTACHMENT_STORE_OP_DONT_CARE,
			VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
			VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
		}
	};

	const std::vector<VkAttachmentReference> ColorAttachmentReferences = {
		{ 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL }
	};
	const VkAttachmentReference DepthAttachmentReference = { 1, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL };
	const std::vector<VkSubpassDescription> SubpassDescriptions = {
		{
			0,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			0, nullptr,
			static_cast<uint32_t>(ColorAttachmentReferences.size()), ColorAttachmentReferences.data(),
			nullptr,
			&DepthAttachmentReference,
			0, nullptr
		}
	};

	const VkRenderPassCreateInfo RenderPassCreateInfo = {
		VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
		nullptr,
		0,
		static_cast<uint32_t>(AttachmentDescriptions.size()), AttachmentDescriptions.data(),
		static_cast<uint32_t>(SubpassDescriptions.size()), SubpassDescriptions.data(),
		0, nullptr
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
			ImageExtent.width, ImageExtent.height,
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
			ImageExtent.width, ImageExtent.height,
			1
		};
		VERIFY_SUCCEEDED(vkCreateFramebuffer(Device, &FramebufferCreateInfo, nullptr, &Framebuffers[i]));
	}
}

void VKExt::Clear_Color(const VkCommandBuffer CommandBuffer)
{
	const VkClearColorValue SkyBlue = { 0.529411793f, 0.807843208f, 0.921568692f, 1.0f };
	const std::vector<VkImageSubresourceRange> ImageSubresourceRanges_Color = {
		{
			VK_IMAGE_ASPECT_COLOR_BIT,
			0, 1,
			0, 1
		}
	};
	vkCmdClearColorImage(CommandBuffer,
		SwapchainImages[SwapchainImageIndex],
		VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
		&SkyBlue,
		static_cast<uint32_t>(ImageSubresourceRanges_Color.size()), ImageSubresourceRanges_Color.data());
}
void VKExt::Clear_Depth(const VkCommandBuffer CommandBuffer)
{
	if (VK_NULL_HANDLE != DepthStencilImage) {
		const VkClearDepthStencilValue ClearDepthStencil = { 1.0f, 0 };
		const std::vector<VkImageSubresourceRange> ImageSubresourceRanges_DepthStencil = {
			{
				VK_IMAGE_ASPECT_DEPTH_BIT,
				0, 1,
				0, 1
			}
		};
		vkCmdClearDepthStencilImage(CommandBuffer,
			DepthStencilImage,
			VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
			&ClearDepthStencil,
			static_cast<uint32_t>(ImageSubresourceRanges_DepthStencil.size()), ImageSubresourceRanges_DepthStencil.data());
	}
}
