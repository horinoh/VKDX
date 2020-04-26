//#include "stdafx.h"
//#include "framework.h"

#include "VKExt.h"

void VKExt::CreateIndirectBuffer_Draw(const uint32_t IndexCount, const uint32_t InstanceCount)
{
	IndirectBuffers.push_back(VkBuffer());
	const VkDrawIndirectCommand DIC = { IndexCount, InstanceCount, 0, 0 };
	CreateBuffer_Indirect(&IndirectBuffers.back(), GraphicsQueue, CommandBuffers[0], static_cast<VkDeviceSize>(sizeof(DIC)), &DIC);
}
void VKExt::CreateIndirectBuffer_DrawIndexed(const uint32_t IndexCount, const uint32_t InstanceCount)
{
	IndirectBuffers.push_back(VkBuffer());
	const VkDrawIndexedIndirectCommand DIIC = { IndexCount, InstanceCount, 0, 0, 0 };
	CreateBuffer_Indirect(&IndirectBuffers.back(), GraphicsQueue, CommandBuffers[0], static_cast<VkDeviceSize>(sizeof(DIIC)), &DIIC);
}
void VKExt::CreateIndirectBuffer_Dispatch(const uint32_t X, const uint32_t Y, const uint32_t Z)
{
	IndirectBuffers.push_back(VkBuffer());
	const VkDispatchIndirectCommand DIC = { X, Y, Z };
	CreateBuffer_Indirect(&IndirectBuffers.back(), GraphicsQueue, CommandBuffers[0], static_cast<VkDeviceSize>(sizeof(DIC)), &DIC);
}

void VKExt::CreateShaderModle_VsFs()
{
	const auto ShaderPath = GetBasePath();
	ShaderModules.push_back(VKExt::CreateShaderModules((ShaderPath + TEXT(".vert.spv")).data()));
	ShaderModules.push_back(VKExt::CreateShaderModules((ShaderPath + TEXT(".frag.spv")).data()));
}
void VKExt::CreateShaderModle_VsFsTesTcsGs()
{
	const auto ShaderPath = GetBasePath();
	ShaderModules.push_back(VKExt::CreateShaderModules((ShaderPath + TEXT(".vert.spv")).data()));
	ShaderModules.push_back(VKExt::CreateShaderModules((ShaderPath + TEXT(".frag.spv")).data()));
	ShaderModules.push_back(VKExt::CreateShaderModules((ShaderPath + TEXT(".tese.spv")).data()));
	ShaderModules.push_back(VKExt::CreateShaderModules((ShaderPath + TEXT(".tesc.spv")).data()));
	ShaderModules.push_back(VKExt::CreateShaderModules((ShaderPath + TEXT(".geom.spv")).data()));
}
void VKExt::CreateShaderModle_Cs()
{
	const auto ShaderPath = GetBasePath();
	ShaderModules.push_back(VKExt::CreateShaderModules((ShaderPath + TEXT(".comp.spv")).data()));
}

void VKExt::CreatePipeline_VsFs_Input(const VkPrimitiveTopology Topology, const uint32_t PatchControlPoints, const VkBool32 DepthEnable, const std::vector<VkVertexInputBindingDescription>& VIBDs, const std::vector<VkVertexInputAttributeDescription>& VIADs)
{
	const VkPipelineDepthStencilStateCreateInfo PDSSCI = {
		VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
		nullptr,
		0,
		DepthEnable, //!< Test
		DepthEnable, //!< Write
		VK_COMPARE_OP_LESS_OR_EQUAL,
		VK_FALSE,
		VK_FALSE,
		{ VK_STENCIL_OP_KEEP, VK_STENCIL_OP_KEEP, VK_STENCIL_OP_KEEP, VK_COMPARE_OP_NEVER, 0, 0, 0 },
		{ VK_STENCIL_OP_KEEP, VK_STENCIL_OP_KEEP, VK_STENCIL_OP_KEEP, VK_COMPARE_OP_ALWAYS, 0, 0, 0 },
		0.0f, 1.0f
	};

	Pipelines.resize(1);
	std::vector<std::thread> Threads;
	auto& PL = Pipelines[0];
	const auto RP = RenderPasses[0];
	const auto PLL = PipelineLayouts[0];
	const std::vector<VkPipelineShaderStageCreateInfo> PSSCIs = {
		{ VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, nullptr, 0, VK_SHADER_STAGE_VERTEX_BIT, ShaderModules[0], "main", nullptr },
		{ VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, nullptr, 0, VK_SHADER_STAGE_FRAGMENT_BIT, ShaderModules[1], "main", nullptr },
	};

	//!< メンバ関数をスレッドで使用したい場合、以下のようにthisを引数に取る形式を使用
	//std::thread::thread(&VKExt::Func, this, Arg0, Arg1,...);

#ifdef USE_PIPELINE_SERIALIZE
	PipelineCacheSerializer PCS(Device, GetBasePath() + TEXT(".pco"), 1);
	Threads.push_back(std::thread::thread(VK::CreatePipeline, std::ref(PL), Device, PLL, RP, Topology, PatchControlPoints, PDSSCI, &PSSCIs[0], &PSSCIs[1], nullptr, nullptr, nullptr, VIBDs, VIADs, PCS.GetPipelineCache(0)));
#else
	Threads.push_back(std::thread::thread(VK::CreatePipeline, std::ref(PL), Device, PLL, RP, Topology, PatchControlPoints, PDSSCI, &PSSCIs[0], &PSSCIs[1], nullptr, nullptr, nullptr, VIBDs, VIADs, nullptr));
#endif

	for (auto& i : Threads) { i.join(); }
}

void VKExt::CreatePipeline_VsFsTesTcsGs_Input(const VkPrimitiveTopology Topology, const uint32_t PatchControlPoints, const VkBool32 DepthEnable, const std::vector<VkVertexInputBindingDescription>& VIBDs, const std::vector<VkVertexInputAttributeDescription>& VIADs)
{
	const VkPipelineDepthStencilStateCreateInfo PDSSCI = {
		VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
		nullptr,
		0,
		DepthEnable,
		DepthEnable,
		VK_COMPARE_OP_LESS_OR_EQUAL,
		VK_FALSE,
		VK_FALSE,
		{ VK_STENCIL_OP_KEEP, VK_STENCIL_OP_KEEP, VK_STENCIL_OP_KEEP, VK_COMPARE_OP_NEVER, 0, 0, 0 },
		{ VK_STENCIL_OP_KEEP, VK_STENCIL_OP_KEEP, VK_STENCIL_OP_KEEP, VK_COMPARE_OP_ALWAYS, 0, 0, 0 },
		0.0f, 1.0f
	};

	Pipelines.resize(1);
	std::vector<std::thread> Threads;
	auto& PL = Pipelines[0];
	const auto RP = RenderPasses[0];
	const auto PLL = PipelineLayouts[0];
	const std::vector<VkPipelineShaderStageCreateInfo> PSSCIs = {
		{ VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, nullptr, 0, VK_SHADER_STAGE_VERTEX_BIT, ShaderModules[0], "main", nullptr },
		{ VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, nullptr, 0, VK_SHADER_STAGE_FRAGMENT_BIT, ShaderModules[1], "main", nullptr },
		{ VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, nullptr, 0, VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT, ShaderModules[2], "main", nullptr },
		{ VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, nullptr, 0, VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT, ShaderModules[3], "main", nullptr },
		{ VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, nullptr, 0, VK_SHADER_STAGE_GEOMETRY_BIT, ShaderModules[4], "main", nullptr },
	};
#ifdef USE_PIPELINE_SERIALIZE
	PipelineCacheSerializer PCS(Device, GetBasePath() + TEXT(".pco"), 1);
	Threads.push_back(std::thread::thread(VK::CreatePipeline, std::ref(PL), Device, PLL, RP, Topology, PatchControlPoints, PDSSCI, &PSSCIs[0], &PSSCIs[1], &PSSCIs[2], &PSSCIs[3], &PSSCIs[4], VIBDs, VIADs, PCS.GetPipelineCache(0)));
#else
	Threads.push_back(std::thread::thread(VK::CreatePipeline, std::ref(PL), Device, PLL, RP, Topology, PatchControlPoints, PDSSCI, &PSSCIs[0], &PSSCIs[1], &PSSCIs[2], &PSSCIs[3], &PSSCIs[4], VIBDs, VIADs, nullptr));
#endif
	for (auto& i : Threads) { i.join(); }
}

void VKExt::CreateRenderPass_ColorDepth(VkRenderPass& RP, const VkFormat Color, const VkFormat Depth, bool ClearOnLoad)
{
	const std::array<VkAttachmentDescription, 2> ADs = { {
		{
			0,
			Color,
			VK_SAMPLE_COUNT_1_BIT,
			ClearOnLoad ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_STORE,
			VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
			VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
		},
		{
			0,
			Depth,
			VK_SAMPLE_COUNT_1_BIT,
			VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE, //!< VK_ATTACHMENT_LOAD_OP_CLEAR を指定したら VkRenderPassBeginInfo.pClearValues 使用必須
			VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
			VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
		},
	} };

	const std::array<VkAttachmentReference, 0> InputARs = {};
	const std::array<VkAttachmentReference, 1> ColorARs = {
		{ 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL },
	};
	const VkAttachmentReference DepthAR = {
		1, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
	};
	const std::array<uint32_t, 0> PreserveAttaches = {};
	const std::array<VkSubpassDescription, 1> SubpassDescs = {
		{
			0,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			static_cast<uint32_t>(InputARs.size()), InputARs.data(),
			static_cast<uint32_t>(ColorARs.size()), ColorARs.data(), nullptr,
			&DepthAR,
			static_cast<uint32_t>(PreserveAttaches.size()), PreserveAttaches.data()
		}
	};

	const std::array<VkSubpassDependency, 0> SubpassDepends = {};

	const VkRenderPassCreateInfo RPCI = {
		VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
		nullptr,
		0,
		static_cast<uint32_t>(ADs.size()), ADs.data(),
		static_cast<uint32_t>(SubpassDescs.size()), SubpassDescs.data(),
		static_cast<uint32_t>(SubpassDepends.size()), SubpassDepends.data()
	};
	VERIFY_SUCCEEDED(vkCreateRenderPass(Device, &RPCI, GetAllocationCallbacks(), &RP));
}

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
			static_cast<uint32_t>(InputARs_Pass0.size()), InputARs_Pass0.data(),
			static_cast<uint32_t>(ColorARs_Pass0.size()), ColorARs_Pass0.data(), nullptr,
			&DepthARs_Pass0,
			static_cast<uint32_t>(PreserveAttaches.size()), PreserveAttaches.data()
		},
		{
			0,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			static_cast<uint32_t>(InputARs_Pass1.size()), InputARs_Pass1.data(),
			static_cast<uint32_t>(ColorARs_Pass1.size()), ColorARs_Pass1.data(), nullptr,
			nullptr,
			static_cast<uint32_t>(PreserveAttaches.size()), PreserveAttaches.data()
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
		static_cast<uint32_t>(ADs.size()), ADs.data(),
		static_cast<uint32_t>(SubpassDescs.size()), SubpassDescs.data(),
		static_cast<uint32_t>(SubpassDepends.size()), SubpassDepends.data()
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
			static_cast<uint32_t>(Input_Pass0.size()), Input_Pass0.data(),
			static_cast<uint32_t>(Color_Pass0.size()), Color_Pass0.data(), nullptr,
			nullptr,
			static_cast<uint32_t>(Preserve.size()), Preserve.data()
		},
		{
			0,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			static_cast<uint32_t>(Input_Pass1.size()), Input_Pass1.data(),
			static_cast<uint32_t>(Color_Pass1.size()), Color_Pass1.data(), nullptr,
			nullptr,
			static_cast<uint32_t>(Preserve.size()), Preserve.data()
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
		static_cast<uint32_t>(ADs.size()), ADs.data(),
		static_cast<uint32_t>(SubpassDescs.size()), SubpassDescs.data(),
		static_cast<uint32_t>(SubpassDepends.size()), SubpassDepends.data()
	};
	VERIFY_SUCCEEDED(vkCreateRenderPass(Device, &RPCI, GetAllocationCallbacks(), &RP));
}

//void VKExt::CreateFramebuffer(VkFramebuffer& FB, const VkRenderPass RP, const uint32_t Width, const uint32_t Height, const uint32_t Layers, const std::initializer_list<VkImageView> il_IVs)
//{
//	const std::vector<VkImageView> IVs(il_IVs.begin(), il_IVs.end());
//
//	const VkFramebufferCreateInfo FCI = {
//		VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
//		nullptr,
//		0,
//		RP, //!< ここで作成するフレームバッファは RenderPass と「コンパチ」な別のレンダーパスでも使用可能
//		static_cast<uint32_t>(IVs.size()), IVs.data(),
//		Width, Height,
//		Layers
//	};
//	VERIFY_SUCCEEDED(vkCreateFramebuffer(Device, &FCI, GetAllocationCallbacks(), &FB));
//}

void VKExt::CreateFramebuffer_Color()
{
	Framebuffers.resize(SwapchainImages.size());
	for (uint32_t i = 0; i < Framebuffers.size(); ++i) {
		VK::CreateFramebuffer(Framebuffers[i], RenderPasses[0], SurfaceExtent2D.width, SurfaceExtent2D.height, 1, {
			SwapchainImageViews[i]
			});
	}
}
void VKExt::CreateFramebuffer_ColorDepth()
{
	Framebuffers.resize(SwapchainImages.size());
	for (uint32_t i = 0; i < Framebuffers.size(); ++i) {
		VK::CreateFramebuffer(Framebuffers[i], RenderPasses[0], SurfaceExtent2D.width, SurfaceExtent2D.height, 1, {
			SwapchainImageViews[i],
			DepthStencilImageView
			});
	}
}

void VKExt::CreateRenderTexture(VkImage* Img, VkImageView* IV)
{
	const auto Format = VK_FORMAT_R8G8B8A8_UNORM;

	const VkExtent3D Extent = { 800, 600, 1 };
	const auto Faces = 1;
	const auto Layers = 1 * Faces;
	const auto Levels = 1;
	CreateImage(Img, 0, VK_IMAGE_TYPE_2D, Format, Extent, Levels, Layers, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);

#if 0
	AllocateImageMemory(DM, *Img, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	VERIFY_SUCCEEDED(vkBindImageMemory(Device, *Img, *DM, 0));
#else
	uint32_t HeapIndex;
	VkDeviceSize Offset;
	SuballocateImageMemory(HeapIndex, Offset, *Img, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
#endif

	const VkComponentMapping CompMap = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };
	CreateImageView(IV, *Img, VK_IMAGE_VIEW_TYPE_2D, Format, CompMap, ImageSubresourceRange_ColorAll);
}
