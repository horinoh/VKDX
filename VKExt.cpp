#include "VKExt.h"

void VKExt::CreateGeometry([[maybe_unused]]const std::vector<VKExt::GeometryCreateInfo>& GCIs)
{
#if 0
	struct GeometryCreateCommand {
		const VKExt::GeometryCreateInfo* GCI = nullptr;

		std::vector<StagingBuffer> VertexStagingBuffers = {};
		StagingBuffer IndexStagingBuffer;
		StagingBuffer IndirectStagingBuffer;

		VkDrawIndexedIndirectCommand DIIC;
		VkDrawIndirectCommand DIC;

		size_t VertexStart = 0, IndexStart = 0, IndirectStart = 0;
	};
	std::vector<GeometryCreateCommand> GCCs;

	for (const auto& i : GCIs) {
		auto& GCC = GCCs.emplace_back();
		GCC.GCI = &i;

		//!< バーテックスバッファ、ステージングの作成 (Create vertex buffer, staging)
		for (const auto& i : i.Vtxs) {
			auto& VSB = GCC.VertexStagingBuffers.emplace_back(BufferAndDeviceMemory({ VK_NULL_HANDLE, VK_NULL_HANDLE }));
			GCC.VertexStart = std::size(VertexBuffers);
			CreateDeviceLocalBuffer(VertexBuffers.emplace_back(), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, i.first);
			CreateHostVisibleBuffer(&VSB.first, &VSB.second, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, i.first, i.second);
		}

		//!< インデックスバッファ、ステージングの作成 (Create index buffer, staging)
		const auto HasIdx = i.Idx.first != 0 && i.Idx.second != nullptr && i.IdxCount != 0;
		if (HasIdx) {
			GCC.IndexStart = std::size(IndexBuffers);
			CreateDeviceLocalBuffer(IndexBuffers.emplace_back(), VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, i.Idx.first);
			CreateHostVisibleBuffer(&GCC.IndexStagingBuffer.first, &GCC.IndexStagingBuffer.second, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, i.Idx.first, i.Idx.second);
		}

		//!< インダイレクトバッファ、ステージングの作成 (Create indirect buffer, staging)
		GCC.IndirectStart = std::size(IndirectBuffers);
		if (HasIdx) {
			GCC.DIIC = VkDrawIndexedIndirectCommand({
				.indexCount = i.IdxCount,
				.instanceCount = i.InstCount,
				.firstIndex = 0,
				.vertexOffset = 0,
				.firstInstance = 0
				});
			CreateDeviceLocalBuffer(IndirectBuffers.emplace_back(), VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, sizeof(GCC.DIIC));
			CreateHostVisibleBuffer(&GCC.IndirectStagingBuffer.first, &GCC.IndirectStagingBuffer.second, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, sizeof(GCC.DIIC), &GCC.DIIC);
		}
		else {
			GCC.DIC = VkDrawIndirectCommand({
				.vertexCount = i.VtxCount,
				.instanceCount = i.InstCount,
				.firstVertex = 0,
				.firstInstance = 0
				});
			CreateDeviceLocalBuffer(IndirectBuffers.emplace_back(), VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, sizeof(GCC.DIC));
			CreateHostVisibleBuffer(&GCC.IndirectStagingBuffer.first, &GCC.IndirectStagingBuffer.second, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, sizeof(GCC.DIC), &GCC.DIC);
		}
	}

	const auto& CB = CommandBuffers[0];
	constexpr VkCommandBufferBeginInfo CBBI = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		.pNext = nullptr,
		.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
		.pInheritanceInfo = nullptr
	};
	VERIFY_SUCCEEDED(vkBeginCommandBuffer(CB, &CBBI)); {
		for (const auto& i : GCCs) {
			for (auto j = 0; j < std::size(i.GCI->Vtxs); ++j) {
				PopulateCopyCommand(CB, i.VertexStagingBuffers[j].first, VertexBuffers[i.VertexStart + j].first, i.GCI->Vtxs[j].first, VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT, VK_PIPELINE_STAGE_VERTEX_INPUT_BIT);
			}
			if (VK_NULL_HANDLE != i.IndexStagingBuffer.first) {
				PopulateCopyCommand(CB, i.IndexStagingBuffer.first, IndexBuffers[i.IndexStart].first, i.GCI->Idx.first, VK_ACCESS_INDEX_READ_BIT, VK_PIPELINE_STAGE_VERTEX_INPUT_BIT);
				PopulateCopyCommand(CB, i.IndirectStagingBuffer.first, IndirectBuffers[i.IndirectStart].first, sizeof(i.DIIC), VK_ACCESS_INDIRECT_COMMAND_READ_BIT, VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT);
			}
			else {
				PopulateCopyCommand(CB, i.IndirectStagingBuffer.first, IndirectBuffers[i.IndirectStart].first, sizeof(i.DIC), VK_ACCESS_INDIRECT_COMMAND_READ_BIT, VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT);
			}
		}
	} VERIFY_SUCCEEDED(vkEndCommandBuffer(CB));

	//!< コピーコマンド発行 (Submit copy command)
	SubmitAndWait(CB);

	for (const auto& i : GCCs) {
		for (auto& i : i.VertexStagingBuffers) {
			vkFreeMemory(Device, i.second, nullptr);
			vkDestroyBuffer(Device, i.first, nullptr);
		}
		if (VK_NULL_HANDLE != i.IndexStagingBuffer.first) {
			vkFreeMemory(Device, i.IndexStagingBuffer.second, nullptr);
			vkDestroyBuffer(Device, i.IndexStagingBuffer.first, nullptr);
		}
		vkFreeMemory(Device, i.IndirectStagingBuffer.second, nullptr);
		vkDestroyBuffer(Device, i.IndirectStagingBuffer.first, nullptr);
	}
#endif
}

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
			//!< サブパス依存 (敢えて書く場合)
			VkSubpassDependency({
				.srcSubpass = VK_SUBPASS_EXTERNAL, .dstSubpass = 0,																		//!< サブパス外からサブパス0へ
				.srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,	//!< パイプラインの最終ステージからカラー出力ステージへ
				.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT, .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,						//!< 読み込みからカラー書き込みへ
				.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT,																			//!< 同じメモリ領域に対する書き込みが完了してから読み込み (指定しない場合は自前で書き込み完了を管理)
			}),
			VkSubpassDependency({
				.srcSubpass = 0, .dstSubpass = VK_SUBPASS_EXTERNAL,																		//!< サブパス0からサブパス外へ
				.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, .dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,	//!< カラー出力ステージからパイプラインの最終ステージへ
				.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, .dstAccessMask = VK_ACCESS_MEMORY_READ_BIT,						//!< カラー書き込みから読み込みへ
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
			.failOp = VK_STENCIL_OP_KEEP,		//!< ステンシルテスト失敗時
			.passOp = VK_STENCIL_OP_KEEP,		//!< ステンシルテスト成功、デプステスト失敗時
			.depthFailOp = VK_STENCIL_OP_KEEP,	//!< ステンシルテスト成功、デプステスト成功時
			.compareOp = VK_COMPARE_OP_NEVER,	//!< 既存のステンシル値との比較方法
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

	//!< ブレンド (Blend)
	//!< 例) 
	//!< ブレンド	: Src * A + Dst * (1 - A)	= Src:VK_BLEND_FACTOR_SRC_ALPHA, Dst:VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA, Op:VK_BLEND_OP_ADD
	//!< 加算		: Src * 1 + Dst * 1			= Src:VK_BLEND_FACTOR_ONE, Dst:VK_BLEND_FACTOR_ONE, Op:VK_BLEND_OP_ADD
	//!< 乗算		: Src * 0 + Dst * Src		= Src:VK_BLEND_FACTOR_ZERO, Dst:VK_BLEND_FACTOR_SRC_COLOR, Op:VK_BLEND_OP_ADD
	const std::vector PCBASs = {
		VkPipelineColorBlendAttachmentState({
			.blendEnable = VK_FALSE,
			.srcColorBlendFactor = VK_BLEND_FACTOR_ONE, .dstColorBlendFactor = VK_BLEND_FACTOR_ONE, .colorBlendOp = VK_BLEND_OP_ADD, //!< ブレンド 
			.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE, .dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE, .alphaBlendOp = VK_BLEND_OP_ADD, //!< アルファ
			.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
		}),
	};

	//!< メンバ関数をスレッドで使用したい場合は、以下のようにthisを引数に取る形式を使用すればよい
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

void VKExt::CreatePipeline(VkPipeline& PL,
	const std::vector<VkPipelineShaderStageCreateInfo>& PSSCIs,
	const VkPipelineVertexInputStateCreateInfo& PVISCI,
	const VkPipelineInputAssemblyStateCreateInfo& PIASCI,
	const VkPipelineTessellationStateCreateInfo& PTSCI,
	const VkPipelineViewportStateCreateInfo& PVSCI,
	const VkPipelineRasterizationStateCreateInfo& PRSCI,
	const VkPipelineMultisampleStateCreateInfo& PMSCI,
	const VkPipelineDepthStencilStateCreateInfo& PDSSCI,
	const VkPipelineColorBlendStateCreateInfo& PCBSCI,
	const VkPipelineDynamicStateCreateInfo& PDSCI,
	const VkPipelineLayout PLL,
	const VkRenderPass RP)
{
	const std::array GPCIs = {
		VkGraphicsPipelineCreateInfo({
			.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
			.pNext = nullptr,
#ifdef _DEBUG
			.flags = VK_PIPELINE_CREATE_DISABLE_OPTIMIZATION_BIT,
#else
			.flags = 0,
#endif
			.stageCount = static_cast<uint32_t>(std::size(PSSCIs)), .pStages = std::data(PSSCIs),
			.pVertexInputState = &PVISCI,
			.pInputAssemblyState = &PIASCI,
			.pTessellationState = &PTSCI,
			.pViewportState = &PVSCI,
			.pRasterizationState = &PRSCI,
			.pMultisampleState = &PMSCI,
			.pDepthStencilState = &PDSSCI,
			.pColorBlendState = &PCBSCI,
			.pDynamicState = &PDSCI,
			.layout = PLL,
			.renderPass = RP, .subpass = 0,
			.basePipelineHandle = VK_NULL_HANDLE, .basePipelineIndex = -1
		})
	};
	VERIFY_SUCCEEDED(vkCreateGraphicsPipelines(Device, VK_NULL_HANDLE, static_cast<uint32_t>(std::size(GPCIs)), std::data(GPCIs), nullptr, &PL));
}
void VKExt::CreatePipeline(VkPipeline& PL,
	const VkShaderModule VS, const VkShaderModule FS, const VkShaderModule TCS, const VkShaderModule TES, const VkShaderModule GS,
	const std::vector<VkVertexInputBindingDescription>& VIBDs, const std::vector<VkVertexInputAttributeDescription>& VIADs,
	const VkPrimitiveTopology PT,
	const uint32_t PatchControlPoints,
	const VkPolygonMode PM, const VkCullModeFlags CMF, const VkFrontFace FF,
	const VkBool32 DepthEnable,
	const VkPipelineLayout PLL,
	const VkRenderPass RP)
{
	std::vector<VkPipelineShaderStageCreateInfo> PSSCIs;
	if (VK_NULL_HANDLE != VS) {
		PSSCIs.emplace_back(VkPipelineShaderStageCreateInfo({ .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, .pNext = nullptr, .flags = 0, .stage = VK_SHADER_STAGE_VERTEX_BIT, .module = VS, .pName = "main", .pSpecializationInfo = nullptr }));
	}
	if (VK_NULL_HANDLE != FS) {
		PSSCIs.emplace_back(VkPipelineShaderStageCreateInfo({ .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, .pNext = nullptr, .flags = 0, .stage = VK_SHADER_STAGE_FRAGMENT_BIT, .module = FS, .pName = "main", .pSpecializationInfo = nullptr }));
	}
	if (VK_NULL_HANDLE != TCS) {
		PSSCIs.emplace_back(VkPipelineShaderStageCreateInfo({ .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, .pNext = nullptr, .flags = 0, .stage = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT, .module = TCS, .pName = "main", .pSpecializationInfo = nullptr }));
	}
	if (VK_NULL_HANDLE != TES) {
		PSSCIs.emplace_back(VkPipelineShaderStageCreateInfo({ .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, .pNext = nullptr, .flags = 0, .stage = VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT, .module = TES, .pName = "main", .pSpecializationInfo = nullptr }));
	}
	if (VK_NULL_HANDLE != GS) {
		PSSCIs.emplace_back(VkPipelineShaderStageCreateInfo({ .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, .pNext = nullptr, .flags = 0, .stage = VK_SHADER_STAGE_GEOMETRY_BIT, .module = GS, .pName = "main", .pSpecializationInfo = nullptr }));
	}

	const VkPipelineVertexInputStateCreateInfo PVISCI = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
		.vertexBindingDescriptionCount = static_cast<uint32_t>(std::size(VIBDs)), .pVertexBindingDescriptions = std::data(VIBDs),
		.vertexAttributeDescriptionCount = static_cast<uint32_t>(std::size(VIADs)), .pVertexAttributeDescriptions = std::data(VIADs)
	};

	const VkPipelineInputAssemblyStateCreateInfo PIASCI = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
		.topology = PT,
		.primitiveRestartEnable = VK_FALSE
	};

	const VkPipelineTessellationStateCreateInfo PTSCI = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
		.patchControlPoints = PatchControlPoints
	};

	//!< ダイナミックステートにするのでここでは決め打ち
	constexpr VkPipelineViewportStateCreateInfo PVSCI = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
		.viewportCount = 1, .pViewports = nullptr,
		.scissorCount = 1, .pScissors = nullptr
	};

	const VkPipelineRasterizationStateCreateInfo PRSCI = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
		.depthClampEnable = VK_FALSE,
		.rasterizerDiscardEnable = VK_FALSE,
		.polygonMode = PM,
		.cullMode = CMF,
		.frontFace = FF,
		.depthBiasEnable = VK_FALSE, .depthBiasConstantFactor = 0.0f, .depthBiasClamp = 0.0f, .depthBiasSlopeFactor = 0.0f,
		.lineWidth = 1.0f
	};

	constexpr VkSampleMask SM = 0xffffffff;
	const VkPipelineMultisampleStateCreateInfo PMSCI = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
		.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
		.sampleShadingEnable = VK_FALSE, .minSampleShading = 0.0f,
		.pSampleMask = &SM,
		.alphaToCoverageEnable = VK_FALSE, .alphaToOneEnable = VK_FALSE
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

	constexpr std::array PCBASs = {
		VkPipelineColorBlendAttachmentState({
			.blendEnable = VK_FALSE,
			.srcColorBlendFactor = VK_BLEND_FACTOR_ONE, .dstColorBlendFactor = VK_BLEND_FACTOR_ONE, .colorBlendOp = VK_BLEND_OP_ADD,
			.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE, .dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE, .alphaBlendOp = VK_BLEND_OP_ADD,
			.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
		}),
	};
	const VkPipelineColorBlendStateCreateInfo PCBSCI = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
		.logicOpEnable = VK_FALSE, .logicOp = VK_LOGIC_OP_COPY,
		.attachmentCount = static_cast<uint32_t>(std::size(PCBASs)), .pAttachments = std::data(PCBASs),
		.blendConstants = { 1.0f, 1.0f, 1.0f, 1.0f }
	};

	constexpr std::array DSs = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR, };
	const VkPipelineDynamicStateCreateInfo PDSCI = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
		.dynamicStateCount = static_cast<uint32_t>(std::size(DSs)), .pDynamicStates = std::data(DSs)
	};

	CreatePipeline(PL, PSSCIs, PVISCI, PIASCI, PTSCI, PVSCI, PRSCI, PMSCI, PDSSCI, PCBSCI, PDSCI, PLL, RP);
}

#if 0
//!< ファーストパスで ColorDepth に書き込み、セカンドパスで PostProcess を行う場合の例 (In first pass ColorDepth, second pass PostProcess)
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
