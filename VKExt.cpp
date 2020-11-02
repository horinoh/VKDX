//#include "stdafx.h"
//#include "framework.h"

#include "VKExt.h"

void VKExt::CreateIndirectBuffer_Draw(const uint32_t VertexCount, const uint32_t InstanceCount)
{
	IndirectBuffers.emplace_back(IndirectBuffer());
	const VkDrawIndirectCommand DIC = { .vertexCount = VertexCount, .instanceCount = InstanceCount, .firstVertex = 0, .firstInstance = 0 };
	CreateBuffer_Indirect(&IndirectBuffers.back().Buffer, &IndirectBuffers.back().DeviceMemory, GraphicsQueue, CommandBuffers[0], static_cast<VkDeviceSize>(sizeof(DIC)), &DIC);
}
void VKExt::CreateIndirectBuffer_DrawIndexed(const uint32_t IndexCount, const uint32_t InstanceCount)
{
	IndirectBuffers.emplace_back(IndirectBuffer());
	const VkDrawIndexedIndirectCommand DIIC = { .indexCount = IndexCount, .instanceCount = InstanceCount, .firstIndex = 0, .vertexOffset = 0, .firstInstance = 0 };
	CreateBuffer_Indirect(&IndirectBuffers.back().Buffer, &IndirectBuffers.back().DeviceMemory, GraphicsQueue, CommandBuffers[0], static_cast<VkDeviceSize>(sizeof(DIIC)), &DIIC);
}
void VKExt::CreateIndirectBuffer_Dispatch(const uint32_t X, const uint32_t Y, const uint32_t Z)
{
	IndirectBuffers.emplace_back(IndirectBuffer());
	const VkDispatchIndirectCommand DIC = { .x = X, .y = Y, .z = Z };
	CreateBuffer_Indirect(&IndirectBuffers.back().Buffer, &IndirectBuffers.back().DeviceMemory, GraphicsQueue, CommandBuffers[0], static_cast<VkDeviceSize>(sizeof(DIC)), &DIC);
}

void VKExt::CreateShaderModle_VsFs()
{
	const auto ShaderPath = GetBasePath();
	ShaderModules.emplace_back(VK::CreateShaderModule(data(ShaderPath + TEXT(".vert.spv"))));
	ShaderModules.emplace_back(VK::CreateShaderModule(data(ShaderPath + TEXT(".frag.spv"))));
}
void VKExt::CreateShaderModle_VsFsTesTcsGs()
{
	const auto ShaderPath = GetBasePath();
	ShaderModules.emplace_back(VK::CreateShaderModule(data(ShaderPath + TEXT(".vert.spv"))));
	ShaderModules.emplace_back(VK::CreateShaderModule(data(ShaderPath + TEXT(".frag.spv"))));
	ShaderModules.emplace_back(VK::CreateShaderModule(data(ShaderPath + TEXT(".tese.spv"))));
	ShaderModules.emplace_back(VK::CreateShaderModule(data(ShaderPath + TEXT(".tesc.spv"))));
	ShaderModules.emplace_back(VK::CreateShaderModule(data(ShaderPath + TEXT(".geom.spv"))));
}
void VKExt::CreateShaderModle_Cs()
{
	const auto ShaderPath = GetBasePath();
	ShaderModules.emplace_back(VK::CreateShaderModule(data(ShaderPath + TEXT(".comp.spv"))));
}

void VKExt::CreatePipeline_VsFs_Input(const VkPrimitiveTopology Topology, const uint32_t PatchControlPoints, const VkBool32 DepthEnable, const std::vector<VkVertexInputBindingDescription>& VIBDs, const std::vector<VkVertexInputAttributeDescription>& VIADs)
{
	const VkPipelineRasterizationStateCreateInfo PRSCI = {
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
		.front = VkStencilOpState({ .failOp = VK_STENCIL_OP_KEEP, .passOp = VK_STENCIL_OP_KEEP, .depthFailOp = VK_STENCIL_OP_KEEP, .compareOp = VK_COMPARE_OP_NEVER, .compareMask = 0, .writeMask = 0, .reference = 0 }),
		.back = VkStencilOpState({ .failOp = VK_STENCIL_OP_KEEP, .passOp = VK_STENCIL_OP_KEEP, .depthFailOp = VK_STENCIL_OP_KEEP, .compareOp = VK_COMPARE_OP_ALWAYS, .compareMask = 0, .writeMask = 0, .reference = 0 }),
		.minDepthBounds = 0.0f, .maxDepthBounds = 1.0f
	};

	Pipelines.emplace_back(VkPipeline());
	std::vector<std::thread> Threads;
	auto& PL = Pipelines[0];
	const auto RP = RenderPasses[0];
	const auto PLL = PipelineLayouts[0];
	const std::array PSSCIs = {
		VkPipelineShaderStageCreateInfo({ .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, .pNext = nullptr, .flags = 0, .stage = VK_SHADER_STAGE_VERTEX_BIT, .module = ShaderModules[0], .pName = "main", .pSpecializationInfo = nullptr }),
		VkPipelineShaderStageCreateInfo({ .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, .pNext = nullptr, .flags = 0, .stage = VK_SHADER_STAGE_FRAGMENT_BIT, .module = ShaderModules[1], .pName = "main", .pSpecializationInfo = nullptr }),
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

	//!< メンバ関数をスレッドで使用したい場合、以下のようにthisを引数に取る形式を使用
	//std::thread::thread(&VKExt::Func, this, Arg0, Arg1,...);
#ifdef USE_PIPELINE_SERIALIZE
	PipelineCacheSerializer PCS(Device, GetBasePath() + TEXT(".pco"), 1);
	Threads.emplace_back(std::thread::thread(VK::CreatePipeline, std::ref(PL), Device, PLL, RP, Topology, PatchControlPoints, PRSCI, PDSSCI, &PSSCIs[0], &PSSCIs[1], nullptr, nullptr, nullptr, VIBDs, VIADs, PCBASs, PCS.GetPipelineCache(0)));
#else
	Threads.emplace_back(std::thread::thread(VK::CreatePipeline, std::ref(PL), Device, PLL, RP, Topology, PatchControlPoints, PRSCI, PDSSCI, &PSSCIs[0], &PSSCIs[1], nullptr, nullptr, nullptr, VIBDs, VIADs, PCBASs));
#endif

	for (auto& i : Threads) { i.join(); }
}

void VKExt::CreatePipeline_VsFsTesTcsGs_Input(const VkPrimitiveTopology Topology, const uint32_t PatchControlPoints, const VkBool32 DepthEnable, const std::vector<VkVertexInputBindingDescription>& VIBDs, const std::vector<VkVertexInputAttributeDescription>& VIADs)
{
	const VkPipelineRasterizationStateCreateInfo PRSCI = {
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
		.front = VkStencilOpState({ .failOp = VK_STENCIL_OP_KEEP, .passOp = VK_STENCIL_OP_KEEP, .depthFailOp = VK_STENCIL_OP_KEEP, .compareOp = VK_COMPARE_OP_NEVER, .compareMask = 0, .writeMask = 0, .reference = 0 }),
		.back = VkStencilOpState({ .failOp = VK_STENCIL_OP_KEEP, .passOp = VK_STENCIL_OP_KEEP, .depthFailOp = VK_STENCIL_OP_KEEP, .compareOp = VK_COMPARE_OP_ALWAYS, .compareMask = 0, .writeMask = 0, .reference = 0 }),
		.minDepthBounds = 0.0f, .maxDepthBounds = 1.0f
	};

	Pipelines.emplace_back(VkPipeline());
	std::vector<std::thread> Threads;
	auto& PL = Pipelines[0];
	const auto RP = RenderPasses[0];
	const auto PLL = PipelineLayouts[0];
	const std::array PSSCIs = {
		VkPipelineShaderStageCreateInfo({ .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, .pNext = nullptr, .flags = 0, .stage = VK_SHADER_STAGE_VERTEX_BIT, .module = ShaderModules[0], .pName = "main", .pSpecializationInfo = nullptr }),
		VkPipelineShaderStageCreateInfo({ .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, .pNext = nullptr, .flags = 0, .stage = VK_SHADER_STAGE_FRAGMENT_BIT, .module = ShaderModules[1], .pName = "main", .pSpecializationInfo = nullptr }),
		VkPipelineShaderStageCreateInfo({ .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, .pNext = nullptr, .flags = 0, .stage = VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT, .module = ShaderModules[2], .pName = "main", .pSpecializationInfo = nullptr }),
		VkPipelineShaderStageCreateInfo({ .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, .pNext = nullptr, .flags = 0, .stage = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT, .module = ShaderModules[3], .pName = "main", .pSpecializationInfo = nullptr }),
		VkPipelineShaderStageCreateInfo({ .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, .pNext = nullptr, .flags = 0, .stage = VK_SHADER_STAGE_GEOMETRY_BIT, .module = ShaderModules[4], .pName = "main", .pSpecializationInfo = nullptr }),
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
	PipelineCacheSerializer PCS(Device, GetBasePath() + TEXT(".pco"), 1);
	Threads.emplace_back(std::thread::thread(VK::CreatePipeline, std::ref(PL), Device, PLL, RP, Topology, PatchControlPoints, PRSCI, PDSSCI, &PSSCIs[0], &PSSCIs[1], &PSSCIs[2], &PSSCIs[3], &PSSCIs[4], VIBDs, VIADs, PCBASs, PCS.GetPipelineCache(0)));
#else
	Threads.emplace_back(std::thread::thread(VK::CreatePipeline, std::ref(PL), Device, PLL, RP, Topology, PatchControlPoints, PRSCI, PDSSCI, &PSSCIs[0], &PSSCIs[1], &PSSCIs[2], &PSSCIs[3], &PSSCIs[4], VIBDs, VIADs, PCBASs));
#endif
	for (auto& i : Threads) { i.join(); }
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

//void VKExt::CreateFramebuffer(VkFramebuffer& FB, const VkRenderPass RP, const uint32_t Width, const uint32_t Height, const uint32_t Layers, const std::initializer_list<VkImageView> il_IVs)
//{
//	const std::vector<VkImageView> IVs(cbegin(il_IVs), cend(il_IVs));
//
//	const VkFramebufferCreateInfo FCI = {
//		VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
//		nullptr,
//		0,
//		RP, //!< ここで作成するフレームバッファは RenderPass と「コンパチ」な別のレンダーパスでも使用可能
//		static_cast<uint32_t>(size(IVs)), data(IVs),
//		Width, Height,
//		Layers
//	};
//	VERIFY_SUCCEEDED(vkCreateFramebuffer(Device, &FCI, GetAllocationCallbacks(), &FB));
//}

//void VKExt::CreateRenderTexture(VkImage* Img, VkImageView* IV)
//{
//	const auto Format = VK_FORMAT_R8G8B8A8_UNORM;
//
//	const VkExtent3D Extent = { 800, 600, 1 };
//	const auto Faces = 1;
//	const auto Layers = 1 * Faces;
//	const auto Levels = 1;
//	CreateImage(Img, 0, VK_IMAGE_TYPE_2D, Format, Extent, Levels, Layers, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
//	//!< デプスの場合
//	//CreateImage(Img, 0, VK_IMAGE_TYPE_2D, Format, Extent, Levels, Layers, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
//
//#if 0
//	AllocateImageMemory(DM, *Img, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
//	VERIFY_SUCCEEDED(vkBindImageMemory(Device, *Img, *DM, 0));
//#else
//	uint32_t HeapIndex;
//	VkDeviceSize Offset;
//	SuballocateImageMemory(HeapIndex, Offset, *Img, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
//#endif
//
//	const VkComponentMapping CompMap = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };
//	CreateImageView(IV, *Img, VK_IMAGE_VIEW_TYPE_2D, Format, CompMap, ImageSubresourceRange_ColorAll);
//}
