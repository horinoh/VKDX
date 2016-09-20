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
	const auto ShaderPath = GetShaderPath();
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
	const auto ShaderPath = GetShaderPath();
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
