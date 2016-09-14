#include "stdafx.h"

#include "VKExt.h"

void VKExt::CreateDescriptorSetLayout_1UniformBuffer(const VkShaderStageFlags ShaderStageFlags)
{
	const std::vector<VkDescriptorSetLayoutBinding> DescriptorSetLayoutBindings = {
		{ 
			0,
			VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 
			1,
			ShaderStageFlags,
			nullptr
		},
	};
	const VkDescriptorSetLayoutCreateInfo DescriptorSetLayoutCreateInfo = {
		VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
		nullptr,
		0,
		static_cast<uint32_t>(DescriptorSetLayoutBindings.size()), DescriptorSetLayoutBindings.data()
	};
	VkDescriptorSetLayout DescriptorSetLayout = VK_NULL_HANDLE;
	VERIFY_SUCCEEDED(vkCreateDescriptorSetLayout(Device, &DescriptorSetLayoutCreateInfo, nullptr, &DescriptorSetLayout));
	DescriptorSetLayouts.push_back(DescriptorSetLayout);

#ifdef _DEBUG
	std::cout << "CreateDescriptorSetLayout" << COUT_OK << std::endl << std::endl;
#endif
}
void VKExt::CreateDescritporPool_1UniformBuffer()
{
	const std::vector<VkDescriptorPoolSize> DescriptorPoolSizes = {
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1 },
	};
	if (!DescriptorPoolSizes.empty()) {
		const VkDescriptorPoolCreateInfo DescriptorPoolCreateInfo = {
			VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
			nullptr,
			0,
			1, //!< maxSets ... プールから確保される最大のデスクリプタ数
			static_cast<uint32_t>(DescriptorPoolSizes.size()), DescriptorPoolSizes.data()
		};
		VERIFY_SUCCEEDED(vkCreateDescriptorPool(Device, &DescriptorPoolCreateInfo, nullptr, &DescriptorPool));

#ifdef _DEBUG
		std::cout << "CreateDescriptorPool" << COUT_OK << std::endl << std::endl;
#endif
	}
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
	const auto ShaderPath = GetShaderPath();
	std::vector<VkShaderModule> ShaderModules = {
		CreateShaderModule((ShaderPath + L".vert.spv").data()),
		CreateShaderModule((ShaderPath + L".frag.spv").data())
	};
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

	//!< 別途 VkDynamicState にするので、ここでは nullptr を指定している、ただし個数は指定しておく必要があるので注意!
	const VkPipelineViewportStateCreateInfo PipelineViewportStateCreateInfo = {
		VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
		nullptr,
		0,
		1, nullptr,
		1, nullptr
	};

	const VkPipelineRasterizationStateCreateInfo PipelineRasterizationStateCreateInfo = {
		VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
		nullptr,
		0,
		VK_FALSE,
		VK_FALSE,
		VK_POLYGON_MODE_FILL,
		VK_CULL_MODE_BACK_BIT,
		VK_FRONT_FACE_COUNTER_CLOCKWISE,
		VK_FALSE, 0.0f, 0.0f, 0.0f,
		1.0f
	};

	const VkPipelineMultisampleStateCreateInfo PipelineMultisampleStateCreateInfo = {
		VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
		nullptr,
		0,
		VK_SAMPLE_COUNT_1_BIT,
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

	//!< アタッチメント毎に異なるブレンドをしたい場合はデバイスで有効になっていないとだめ (VkPhysicalDeviceFeatures.independentBlend)
	const std::vector<VkPipelineColorBlendAttachmentState> PipelineColorBlendAttachmentStates = {
		{
			VK_FALSE,
			VK_BLEND_FACTOR_ZERO, VK_BLEND_FACTOR_ZERO,
			VK_BLEND_OP_ADD,
			VK_BLEND_FACTOR_ZERO, VK_BLEND_FACTOR_ZERO,
			VK_BLEND_OP_ADD,
			VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
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

	//!< DirectX12 に合わせる為、Viewport と Scissor を VkDynamicState とする
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

	//!< DescriptorSetLayouts は CreateDescriptorSetLayout() 内で作成してある
	const std::vector<VkPushConstantRange> PushConstantRanges = {
	};
	const VkPipelineLayoutCreateInfo PipelineLayoutCreateInfo = {
		VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
		nullptr,
		0,
		static_cast<uint32_t>(DescriptorSetLayouts.size()), DescriptorSetLayouts.data(),
		static_cast<uint32_t>(PushConstantRanges.size()), PushConstantRanges.data()
	};
	VkPipelineLayout PipelineLayout = VK_NULL_HANDLE;
	VERIFY_SUCCEEDED(vkCreatePipelineLayout(Device, &PipelineLayoutCreateInfo, nullptr, &PipelineLayout));
	//!< PipelineLayout を作成したら、DescritptorSetLayout は破棄して良い
	for (auto i : DescriptorSetLayouts) {
		vkDestroyDescriptorSetLayout(Device, i, nullptr);
	}
	DescriptorSetLayouts.clear();

	/**
	basePipelineHandle と basePipelineIndex は同時に使用できない(排他)
	親には VK_PIPELINE_CREATE_ALLOW_DERIVATIVES_BIT フラグが必要
	・basePipelineHandle ... 既に存在する場合、親パイプラインを指定
	・basePipelineIndex ... GraphicsPipelineCreateInfos 配列で親パイプラインも同時に作成する場合、配列内での親パイプラインの添字。親の添字の方が若い値でないといけない。
	*/
	const std::vector<VkGraphicsPipelineCreateInfo> GraphicsPipelineCreateInfos = {
		{
			VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
			nullptr,
			0, //!< VK_PIPELINE_CREATE_ALLOW_DERIVATIVES_BIT
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
			VK_NULL_HANDLE, 0 //!< basePipelineHandle, basePipelineIndex
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

	//!< Pipeline を作成したら、ShaderModules は破棄して良い
	for (auto i : ShaderModules) {
		vkDestroyShaderModule(Device, i, nullptr);
	}
	ShaderModules.clear();

	//!< Pipeline を作成したら、PipelineLayout は破棄して良い
	if (VK_NULL_HANDLE != PipelineLayout) {
		vkDestroyPipelineLayout(Device, PipelineLayout, nullptr);
		PipelineLayout = VK_NULL_HANDLE;
	}

#ifdef _DEBUG
	std::cout << "CreateGraphicsPipeline" << COUT_OK << std::endl << std::endl;
#endif
}
void VKExt::CreateGraphicsPipeline_VsPsTesTcsGs()
{
	const auto ShaderPath = GetShaderPath();
	std::vector<VkShaderModule> ShaderModules = {
		CreateShaderModule((ShaderPath + L".vert.spv").data()),
		CreateShaderModule((ShaderPath + L".frag.spv").data()),
		CreateShaderModule((ShaderPath + L".tese.spv").data()),
		CreateShaderModule((ShaderPath + L".tesc.spv").data()),
		CreateShaderModule((ShaderPath + L".geom.spv").data())
	};
	
	//!< #TODO

	for (auto i : ShaderModules) {
		vkDestroyShaderModule(Device, i, nullptr);
	}
	ShaderModules.clear();

#ifdef _DEBUG
	std::cout << "CreateGraphicsPipeline" << COUT_OK << std::endl << std::endl;
#endif
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
		{ 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL }
	};
	const std::vector<VkSubpassDescription> SubpassDescriptions = {
		{
			0,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			0, nullptr,
			static_cast<uint32_t>(ColorAttachmentReferences.size()), ColorAttachmentReferences.data(), nullptr,
			nullptr,
			0, nullptr
		}
	};
	const std::vector<VkSubpassDependency> SubpassDependencies = {
		{
			VK_SUBPASS_EXTERNAL, //!< レンダーパス外
			0,
			VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
			VK_ACCESS_MEMORY_READ_BIT,
			VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
			VK_DEPENDENCY_BY_REGION_BIT,
		},
		{
			0,
			VK_SUBPASS_EXTERNAL, //!< レンダーパス外
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
			VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
			VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
			VK_ACCESS_MEMORY_READ_BIT,
			VK_DEPENDENCY_BY_REGION_BIT,
		}
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
		}
	};

	const std::vector<VkAttachmentReference> ColorAttachmentReferences = {
		{ 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL }
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
			VK_SUBPASS_EXTERNAL, //!< レンダーパス外
			0,
			VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
			VK_ACCESS_MEMORY_READ_BIT,
			VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
			VK_DEPENDENCY_BY_REGION_BIT,
		},
		{
			0,
			VK_SUBPASS_EXTERNAL, //!< レンダーパス外
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
			VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
			VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
			VK_ACCESS_MEMORY_READ_BIT,
			VK_DEPENDENCY_BY_REGION_BIT,
		}
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
