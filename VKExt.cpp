#include "VKExt.h"

void VKExt::CreateRenderPass_Default(const VkAttachmentLoadOp LoadOp, const VkImageLayout FinalLayout)
{
	constexpr std::array<VkAttachmentReference, 0> IAs = {};
	constexpr std::array CAs = { VkAttachmentReference({.attachment = 0, .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL }), };
	constexpr std::array RAs = { VkAttachmentReference({.attachment = VK_ATTACHMENT_UNUSED, .layout = VK_IMAGE_LAYOUT_UNDEFINED }), };
	assert(std::size(CAs) == std::size(RAs) && "");
	constexpr std::array<uint32_t, 0> PAs = {};
	Super::CreateRenderPass(RenderPasses.emplace_back(),
		{
			VkAttachmentDescription({
				.flags = 0,
				.format = SurfaceFormat.format,
				.samples = VK_SAMPLE_COUNT_1_BIT,
				.loadOp = LoadOp, .storeOp = VK_ATTACHMENT_STORE_OP_STORE,	
				.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE, .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
				.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED, .finalLayout = FinalLayout
			}),
		},
		{
			VkSubpassDescription({
				.flags = 0,
				.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
				.inputAttachmentCount = static_cast<uint32_t>(std::size(IAs)), .pInputAttachments = std::data(IAs),
				.colorAttachmentCount = static_cast<uint32_t>(std::size(CAs)), .pColorAttachments = std::data(CAs), .pResolveAttachments = std::data(RAs),
				.pDepthStencilAttachment = nullptr,
				.preserveAttachmentCount = static_cast<uint32_t>(std::size(PAs)), .pPreserveAttachments = std::data(PAs)
			}),
		}, 
		{
#if 0
			//!< �T�u�p�X�ˑ� (�����ď����ꍇ)
			VkSubpassDependency({
				.srcSubpass = VK_SUBPASS_EXTERNAL, .dstSubpass = 0,																		//!< �T�u�p�X�O����T�u�p�X0��
				.srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,	//!< �p�C�v���C���̍ŏI�X�e�[�W����J���[�o�̓X�e�[�W��
				.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT, .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,						//!< �ǂݍ��݂���J���[�������݂�
				.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT,																			//!< �����������̈�ɑ΂��鏑�����݂��������Ă���ǂݍ��� (�w�肵�Ȃ��ꍇ�͎��O�ŏ������݊������Ǘ�)
			}),
			VkSubpassDependency({
				.srcSubpass = 0, .dstSubpass = VK_SUBPASS_EXTERNAL,																		//!< �T�u�p�X0����T�u�p�X�O��
				.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, .dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,	//!< �J���[�o�̓X�e�[�W����p�C�v���C���̍ŏI�X�e�[�W��
				.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, .dstAccessMask = VK_ACCESS_MEMORY_READ_BIT,						//!< �J���[�������݂���ǂݍ��݂�
				.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT,
			}),
#endif
		});
}

void VKExt::CreateRenderPass_Depth(const VkImageLayout FinalLayout)
{
	constexpr std::array<VkAttachmentReference, 0> IAs = {};
	constexpr std::array CAs = { VkAttachmentReference({.attachment = 0, .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL }), };
	constexpr std::array RAs = { VkAttachmentReference({.attachment = VK_ATTACHMENT_UNUSED, .layout = VK_IMAGE_LAYOUT_UNDEFINED }), };
	assert(std::size(CAs) == std::size(RAs) && "");
	constexpr auto DA = VkAttachmentReference({ .attachment = 1, .layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL });
	constexpr std::array<uint32_t, 0> PAs = {};

	Super::CreateRenderPass(RenderPasses.emplace_back(), {
		VkAttachmentDescription({
			.flags = 0,
			.format = SurfaceFormat.format,
			.samples = VK_SAMPLE_COUNT_1_BIT,
			.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR, .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
			.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE, .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
			.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED, .finalLayout = FinalLayout
		}),
		VkAttachmentDescription({
			.flags = 0,
			.format = DepthFormat,
			.samples = VK_SAMPLE_COUNT_1_BIT,
			.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR, .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
			.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE, .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
			.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED, .finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
		}),
	}, {
		VkSubpassDescription({
			.flags = 0,
			.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
			.inputAttachmentCount = static_cast<uint32_t>(std::size(IAs)), .pInputAttachments = std::data(IAs),
			.colorAttachmentCount = static_cast<uint32_t>(std::size(CAs)), .pColorAttachments = std::data(CAs), .pResolveAttachments = std::data(RAs),
			.pDepthStencilAttachment = &DA,
			.preserveAttachmentCount = static_cast<uint32_t>(std::size(PAs)), .pPreserveAttachments = std::data(PAs)
		}),
	},
	{});
}

void VKExt::CreatePipeline_VsFs_Input(VkPipeline& PL, const VkPipelineLayout PLL, const VkRenderPass RP, const VkPrimitiveTopology PT, const uint32_t PatchControlPoints, const VkPipelineRasterizationStateCreateInfo& PRSCI, const VkBool32 DepthEnable, const std::vector<VkVertexInputBindingDescription>& VIBDs, const std::vector<VkVertexInputAttributeDescription>& VIADs, const std::array<VkPipelineShaderStageCreateInfo, 2>& PSSCIs)
{
	const VkPipelineDepthStencilStateCreateInfo PDSSCI = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
		.depthTestEnable = DepthEnable, .depthWriteEnable = DepthEnable, .depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL,
		.depthBoundsTestEnable = VK_FALSE,
		.stencilTestEnable = VK_FALSE, 
		.front = VkStencilOpState({ 
			.failOp = VK_STENCIL_OP_KEEP,		//!< �X�e���V���e�X�g���s��
			.passOp = VK_STENCIL_OP_KEEP,		//!< �X�e���V���e�X�g�����A�f�v�X�e�X�g���s��
			.depthFailOp = VK_STENCIL_OP_KEEP,	//!< �X�e���V���e�X�g�����A�f�v�X�e�X�g������
			.compareOp = VK_COMPARE_OP_NEVER,	//!< �����̃X�e���V���l�Ƃ̔�r���@
			.compareMask = 0, .writeMask = 0, .reference = 0
			}),
		.back = VkStencilOpState({ 
			.failOp = VK_STENCIL_OP_KEEP,
			.passOp = VK_STENCIL_OP_KEEP, 
			.depthFailOp = VK_STENCIL_OP_KEEP, 
			.compareOp = VK_COMPARE_OP_ALWAYS, 
			.compareMask = 0, .writeMask = 0, .reference = 0
			}),
		.minDepthBounds = 0.0f, .maxDepthBounds = 1.0f
	};

	//!< �u�����h (Blend)
	//!< ��) 
	//!< �u�����h	: Src * A + Dst * (1 - A)	= Src:VK_BLEND_FACTOR_SRC_ALPHA, Dst:VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA, Op:VK_BLEND_OP_ADD
	//!< ���Z		: Src * 1 + Dst * 1			= Src:VK_BLEND_FACTOR_ONE, Dst:VK_BLEND_FACTOR_ONE, Op:VK_BLEND_OP_ADD
	//!< ��Z		: Src * 0 + Dst * Src		= Src:VK_BLEND_FACTOR_ZERO, Dst:VK_BLEND_FACTOR_SRC_COLOR, Op:VK_BLEND_OP_ADD
	const std::vector PCBASs = {
		VkPipelineColorBlendAttachmentState({
			.blendEnable = VK_FALSE,
			.srcColorBlendFactor = VK_BLEND_FACTOR_ONE, .dstColorBlendFactor = VK_BLEND_FACTOR_ONE, .colorBlendOp = VK_BLEND_OP_ADD, //!< �u�����h 
			.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE, .dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE, .alphaBlendOp = VK_BLEND_OP_ADD, //!< �A���t�@
			.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
		}),
	};

	//!< �����o�֐����X���b�h�Ŏg�p�������ꍇ�́A�ȉ��̂悤��this�������Ɏ��`�����g�p����΂悢
	//std::thread::thread(&VKExt::Func, this, Arg0, Arg1,...);
#ifdef USE_PIPELINE_SERIALIZE
	PipelineCacheSerializer PCS(Device, std::filesystem::path(".") / (GetTitleString() + ".pco"), 1);
	Threads.emplace_back(std::thread::thread(Super::CreatePipelineVsFsTesTcsGs, std::ref(PL), Device, PLL, RP, PT, PatchControlPoints, PRSCI, PDSSCI, &PSSCIs[0], &PSSCIs[1], nullptr, nullptr, nullptr, VIBDs, VIADs, PCBASs, PCS.GetPipelineCache(0)));
#else
	Threads.emplace_back(std::thread::thread(Super::CreatePipelineVsFsTesTcsGs, std::ref(PL), Device, PLL, RP, PT, PatchControlPoints, PRSCI, PDSSCI, &PSSCIs[0], &PSSCIs[1], nullptr, nullptr, nullptr, VIBDs, VIADs, PCBASs, VK_NULL_HANDLE));
#endif
}

void VKExt::CreatePipeline_VsFsTesTcsGs_Input(VkPipeline& PL, const VkPipelineLayout PLL, const VkRenderPass RP, const VkPrimitiveTopology PT, const uint32_t PatchControlPoints, const VkPipelineRasterizationStateCreateInfo& PRSCI, const VkBool32 DepthEnable, const std::vector<VkVertexInputBindingDescription>& VIBDs, const std::vector<VkVertexInputAttributeDescription>& VIADs, const std::array<VkPipelineShaderStageCreateInfo, 5>& PSSCIs)
{
	const VkPipelineDepthStencilStateCreateInfo PDSSCI = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
		.depthTestEnable = DepthEnable, .depthWriteEnable = DepthEnable, .depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL,
		.depthBoundsTestEnable = VK_FALSE,
		.stencilTestEnable = VK_FALSE, 
		.front = VkStencilOpState({ 
			.failOp = VK_STENCIL_OP_KEEP, 
			.passOp = VK_STENCIL_OP_KEEP, 
			.depthFailOp = VK_STENCIL_OP_KEEP, 
			.compareOp = VK_COMPARE_OP_NEVER,
			.compareMask = 0, .writeMask = 0, .reference = 0
			}),
		.back = VkStencilOpState({ 
			.failOp = VK_STENCIL_OP_KEEP, 
			.passOp = VK_STENCIL_OP_KEEP, 
			.depthFailOp = VK_STENCIL_OP_KEEP, 
			.compareOp = VK_COMPARE_OP_ALWAYS,
			.compareMask = 0, .writeMask = 0, .reference = 0 
			}),
		.minDepthBounds = 0.0f, .maxDepthBounds = 1.0f
	};
	const std::vector PCBASs = {
		VkPipelineColorBlendAttachmentState({
			.blendEnable = VK_FALSE,
			.srcColorBlendFactor = VK_BLEND_FACTOR_ONE, .dstColorBlendFactor = VK_BLEND_FACTOR_ONE, .colorBlendOp = VK_BLEND_OP_ADD,
			.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE, .dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE, .alphaBlendOp = VK_BLEND_OP_ADD,
			.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
		}),
	};

#ifdef USE_PIPELINE_SERIALIZE
	PipelineCacheSerializer PCS(Device, std::filesystem::path(".") / (GetTitleString() + ".pco"), 1);
	Threads.emplace_back(std::thread::thread(Super::CreatePipelineVsFsTesTcsGs, std::ref(PL), Device, PLL, RP, PT, PatchControlPoints, PRSCI, PDSSCI, &PSSCIs[0], &PSSCIs[1], &PSSCIs[2], &PSSCIs[3], &PSSCIs[4], VIBDs, VIADs, PCBASs, PCS.GetPipelineCache(0)));
#else
	Threads.emplace_back(std::thread::thread(Super::CreatePipelineVsFsTesTcsGs, std::ref(PL), Device, PLL, RP, PT, PatchControlPoints, PRSCI, PDSSCI, &PSSCIs[0], &PSSCIs[1], &PSSCIs[2], &PSSCIs[3], &PSSCIs[4], VIBDs, VIADs, PCBASs, VK_NULL_HANDLE));
#endif
}

void VKExt::CreatePipelineState_VsFsGs_Input(VkPipeline& PL, const VkPipelineLayout PLL, const VkRenderPass RP, const VkPrimitiveTopology PT, const uint32_t PatchControlPoints, const VkPipelineRasterizationStateCreateInfo& PRSCI, const VkBool32 DepthEnable, const std::vector<VkVertexInputBindingDescription>& VIBDs, const std::vector<VkVertexInputAttributeDescription>& VIADs, const std::array<VkPipelineShaderStageCreateInfo, 3>& PSSCIs)
{
	const VkPipelineDepthStencilStateCreateInfo PDSSCI = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
		.depthTestEnable = DepthEnable, .depthWriteEnable = DepthEnable, .depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL,
		.depthBoundsTestEnable = VK_FALSE,
		.stencilTestEnable = VK_FALSE,
		.front = VkStencilOpState({
			.failOp = VK_STENCIL_OP_KEEP,
			.passOp = VK_STENCIL_OP_KEEP,
			.depthFailOp = VK_STENCIL_OP_KEEP,
			.compareOp = VK_COMPARE_OP_NEVER,
			.compareMask = 0, .writeMask = 0, .reference = 0
			}),
		.back = VkStencilOpState({
			.failOp = VK_STENCIL_OP_KEEP,
			.passOp = VK_STENCIL_OP_KEEP,
			.depthFailOp = VK_STENCIL_OP_KEEP,
			.compareOp = VK_COMPARE_OP_ALWAYS,
			.compareMask = 0, .writeMask = 0, .reference = 0
			}),
		.minDepthBounds = 0.0f, .maxDepthBounds = 1.0f
	};
	const std::vector PCBASs = {
		VkPipelineColorBlendAttachmentState({
			.blendEnable = VK_FALSE,
			.srcColorBlendFactor = VK_BLEND_FACTOR_ONE, .dstColorBlendFactor = VK_BLEND_FACTOR_ONE, .colorBlendOp = VK_BLEND_OP_ADD,
			.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE, .dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE, .alphaBlendOp = VK_BLEND_OP_ADD,
			.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
		}),
	};

#ifdef USE_PIPELINE_SERIALIZE
	PipelineCacheSerializer PCS(Device, std::filesystem::path(".") / (GetTitleString() + ".pco"), 1);
	Threads.emplace_back(std::thread::thread(Super::CreatePipelineVsFsTesTcsGs, std::ref(PL), Device, PLL, RP, PT, PatchControlPoints, PRSCI, PDSSCI, &PSSCIs[0], &PSSCIs[1], nullptr, nullptr, &PSSCIs[2], VIBDs, VIADs, PCBASs, PCS.GetPipelineCache(0)));
#else
	Threads.emplace_back(std::thread::thread(Super::CreatePipelineVsFsTesTcsGs, std::ref(PL), Device, PLL, RP, PT, PatchControlPoints, PRSCI, PDSSCI, &PSSCIs[0], &PSSCIs[1], nullptr, nullptr, &PSSCIs[2], VIBDs, VIADs, PCBASs, VK_NULL_HANDLE));
#endif
}

void VKExt::CreatePipeline_TsMsFs(VkPipeline& PL, const VkPipelineLayout PLL, const VkRenderPass RP, const VkBool32 DepthEnable, const std::array<VkPipelineShaderStageCreateInfo, 3>& PSSCIs)
{
	constexpr VkPipelineRasterizationStateCreateInfo PRSCI = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
		.depthClampEnable = VK_FALSE,
		.rasterizerDiscardEnable = VK_FALSE,
		.polygonMode = VK_POLYGON_MODE_FILL,
		.cullMode = VK_CULL_MODE_BACK_BIT,
		.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
		.depthBiasEnable = VK_FALSE, .depthBiasConstantFactor = 0.0f, .depthBiasClamp = 0.0f, .depthBiasSlopeFactor = 0.0f,
		.lineWidth = 1.0f
	};
	const VkPipelineDepthStencilStateCreateInfo PDSSCI = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
		.depthTestEnable = DepthEnable, .depthWriteEnable = DepthEnable, .depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL,
		.depthBoundsTestEnable = VK_FALSE,
		.stencilTestEnable = VK_FALSE,
		.front = VkStencilOpState({
			.failOp = VK_STENCIL_OP_KEEP,
			.passOp = VK_STENCIL_OP_KEEP,
			.depthFailOp = VK_STENCIL_OP_KEEP,
			.compareOp = VK_COMPARE_OP_NEVER,
			.compareMask = 0, .writeMask = 0, .reference = 0
			}),
		.back = VkStencilOpState({
			.failOp = VK_STENCIL_OP_KEEP,
			.passOp = VK_STENCIL_OP_KEEP,
			.depthFailOp = VK_STENCIL_OP_KEEP,
			.compareOp = VK_COMPARE_OP_ALWAYS,
			.compareMask = 0, .writeMask = 0, .reference = 0
			}),
		.minDepthBounds = 0.0f, .maxDepthBounds = 1.0f
	};
	const std::vector PCBASs = {
		VkPipelineColorBlendAttachmentState({
			.blendEnable = VK_FALSE,
			.srcColorBlendFactor = VK_BLEND_FACTOR_ONE, .dstColorBlendFactor = VK_BLEND_FACTOR_ONE, .colorBlendOp = VK_BLEND_OP_ADD,
			.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE, .dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE, .alphaBlendOp = VK_BLEND_OP_ADD,
			.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
		}),
	};
	constexpr std::array DSs = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR, };
	const VkPipelineDynamicStateCreateInfo PDSCI = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
		.dynamicStateCount = static_cast<uint32_t>(size(DSs)), .pDynamicStates = data(DSs)
	};

#ifdef USE_PIPELINE_SERIALIZE
	PipelineCacheSerializer PCS(Device, std::filesystem::path(".") / (GetTitleString() + ".pco"), 1);
	Threads.emplace_back(std::thread::thread(Super::CreatePipelineTsMsFs, std::ref(PL), Device, PLL, RP, PRSCI, PDSSCI, VK_NULL_HANDLE == PSSCIs[0].module ? nullptr : &PSSCIs[0], &PSSCIs[1], &PSSCIs[2], PCBASs, PCS.GetPipelineCache(0)));
#else
	Threads.emplace_back(std::thread::thread(Super::CreatePipelineTsMsFs, std::ref(PL), Device, PLL, RP, PRSCI, PDSSCI, VK_NULL_HANDLE == PSSCIs[0].module ? nullptr : &PSSCIs[0], &PSSCIs[1], &PSSCIs[2], PCBASs, VK_NULL_HANDLE));
#endif
}

#if 0
//!< �t�@�[�X�g�p�X�� ColorDepth �ɏ������݁A�Z�J���h�p�X�� PostProcess ���s���ꍇ�̗� (In first pass ColorDepth, second pass PostProcess)
void VKExt::CreateRenderPass_ColorDepth_PostProcess(VkRenderPass& RP, const VkFormat Color, const VkFormat Depth, bool ClearOnLoad)
{
	const std::array<VkAttachmentDescription, 3> ADs = { {
		{
			0,
			Color,
			VK_SAMPLE_COUNT_1_BIT,
			ClearOnLoad ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
			VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
			VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
		},
		{
			0,
			Depth,
			VK_SAMPLE_COUNT_1_BIT,
			VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE,
			VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
			VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
		},
		{
			0,
			Color,
			VK_SAMPLE_COUNT_1_BIT,
			ClearOnLoad ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_STORE,
			VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
			VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
		},
	} };

	const std::array<VkAttachmentReference, 0> InputARs_Pass0 = {};
	const std::array<VkAttachmentReference, 1> ColorARs_Pass0 = {
		{ 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL }, 
	};
	const VkAttachmentReference DepthARs_Pass0 = {
		1, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL 
	};

	const std::array<VkAttachmentReference, 1> InputARs_Pass1 = { 
		{ 0, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL },
	};
	const std::array<VkAttachmentReference, 1> ColorARs_Pass1 = {
		{ 2, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL }, 
	};

	const std::array<uint32_t, 0> PreserveAttaches = {};
	const std::array<VkSubpassDescription, 2> SubpassDescs = { {
		{
			0,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			static_cast<uint32_t>(size(InputARs_Pass0)), data(InputARs_Pass0),
			static_cast<uint32_t>(size(ColorARs_Pass0)), data(ColorARs_Pass0), nullptr,
			&DepthARs_Pass0,
			static_cast<uint32_t>(size(PreserveAttaches)), data(PreserveAttaches)
		},
		{
			0,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			static_cast<uint32_t>(size(InputARs_Pass1)), data(InputARs_Pass1),
			static_cast<uint32_t>(size(ColorARs_Pass1)), data(ColorARs_Pass1), nullptr,
			nullptr,
			static_cast<uint32_t>(size(PreserveAttaches)), data(PreserveAttaches)
		}
	} };

	const std::array<VkSubpassDependency, 1> SubpassDepends = {
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

	const VkRenderPassCreateInfo RPCI = {
		VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
		nullptr,
		0,
		static_cast<uint32_t>(size(ADs)), data(ADs),
		static_cast<uint32_t>(size(SubpassDescs)), data(SubpassDescs),
		static_cast<uint32_t>(size(SubpassDepends)), data(SubpassDepends)
	};
	VERIFY_SUCCEEDED(vkCreateRenderPass(Device, &RPCI, GetAllocationCallbacks(), &RP));
}

void VKExt::CreateRenderPass_Color_PostProcess(VkRenderPass& RP, const VkFormat Color, bool ClearOnLoad)
{
	const std::array<VkAttachmentDescription, 2> ADs = { {
		{
			0,
			Color,
			VK_SAMPLE_COUNT_1_BIT,
			ClearOnLoad ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_STORE,
			VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
			VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
		},
		{
			0,
			Color,
			VK_SAMPLE_COUNT_1_BIT,
			VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_STORE,
			VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
			VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
		},
	} };

	const std::array<VkAttachmentReference, 0> Input_Pass0 = {};
	const std::array<VkAttachmentReference, 1> Color_Pass0 = { { 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL }, };

	const std::array<VkAttachmentReference, 1> Input_Pass1 = { { 0, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL }, };
	const std::array<VkAttachmentReference, 1> Color_Pass1 = { { 1, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL }, };

	const std::array<uint32_t, 0> Preserve = {};
	const std::array<VkSubpassDescription, 2> SubpassDescs = { {
		{
			0,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			static_cast<uint32_t>(size(Input_Pass0)), data(Input_Pass0),
			static_cast<uint32_t>(size(Color_Pass0)), data(Color_Pass0), nullptr,
			nullptr,
			static_cast<uint32_t>(size(Preserve)), data(Preserve)
		},
		{
			0,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			static_cast<uint32_t>(size(Input_Pass1)), data(Input_Pass1),
			static_cast<uint32_t>(size(Color_Pass1)), data(Color_Pass1), nullptr,
			nullptr,
			static_cast<uint32_t>(size(Preserve)), data(Preserve)
		}
	} };

	const std::array<VkSubpassDependency, 3> SubpassDepends = { {
			{
				VK_SUBPASS_EXTERNAL, 0,
				VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
				VK_ACCESS_MEMORY_READ_BIT, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
				VK_DEPENDENCY_BY_REGION_BIT,
			},
			{
				0, 1,
				VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
				VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_ACCESS_INPUT_ATTACHMENT_READ_BIT,
				VK_DEPENDENCY_BY_REGION_BIT,
			},
			{
				1, VK_SUBPASS_EXTERNAL,
				VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
				VK_ACCESS_INPUT_ATTACHMENT_READ_BIT, VK_ACCESS_MEMORY_READ_BIT,
				VK_DEPENDENCY_BY_REGION_BIT,
			},
	} };

	const VkRenderPassCreateInfo RPCI = {
		VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
		nullptr,
		0,
		static_cast<uint32_t>(size(ADs)), data(ADs),
		static_cast<uint32_t>(size(SubpassDescs)), data(SubpassDescs),
		static_cast<uint32_t>(size(SubpassDepends)), data(SubpassDepends)
	};
	VERIFY_SUCCEEDED(vkCreateRenderPass(Device, &RPCI, GetAllocationCallbacks(), &RP));
}
#endif
