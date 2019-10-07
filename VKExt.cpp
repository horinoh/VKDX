//#include "stdafx.h"
//#include "framework.h"

#include "VKExt.h"

void VKExt::CreateShaderModle_VsFs()
{
	const auto ShaderPath = GetBasePath();
	CreateShaderModle({
		VKExt::CreateShaderModule((ShaderPath + TEXT(".vert.spv")).data()),
		VKExt::CreateShaderModule((ShaderPath + TEXT(".frag.spv")).data()),
		});
}
void VKExt::CreateShaderModle_VsFsTesTcsGs()
{
	const auto ShaderPath = GetBasePath();
	CreateShaderModle({
		VKExt::CreateShaderModule((ShaderPath + TEXT(".vert.spv")).data()),
		VKExt::CreateShaderModule((ShaderPath + TEXT(".frag.spv")).data()),
		VKExt::CreateShaderModule((ShaderPath + TEXT(".tese.spv")).data()),
		VKExt::CreateShaderModule((ShaderPath + TEXT(".tesc.spv")).data()),
		VKExt::CreateShaderModule((ShaderPath + TEXT(".geom.spv")).data()),
		});
}
void VKExt::CreateShaderModle_Cs()
{
	const auto ShaderPath = GetBasePath();
	CreateShaderModle({
		VKExt::CreateShaderModule((ShaderPath + TEXT(".comp.spv")).data()),
		});
}

void VKExt::CreatePipeline_Tesselation(VkPipeline& Pipeline, const VkPipelineLayout PL, 
	const VkShaderModule VS, const VkShaderModule FS, const VkShaderModule TES, const VkShaderModule TCS, const VkShaderModule GS, 
	const VkRenderPass RP, VkPipelineCache PC)
{
	PERFORMANCE_COUNTER();

	VkPhysicalDeviceFeatures PDF;
	vkGetPhysicalDeviceFeatures(GetCurrentPhysicalDevice(), &PDF);

	std::vector<VkPipelineShaderStageCreateInfo> PSSCI;
	if (VK_NULL_HANDLE != VS) {
		PSSCI.push_back({
			VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
			nullptr,
			0,
			VK_SHADER_STAGE_VERTEX_BIT, VS,
			"main",
			nullptr
			});
	}
	if (VK_NULL_HANDLE != FS) {
		PSSCI.push_back({
			VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
			nullptr,
			0,
			VK_SHADER_STAGE_FRAGMENT_BIT, FS,
			"main",
			nullptr
			});
	}
	if (VK_NULL_HANDLE != TES) {
		PSSCI.push_back({
			VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
			nullptr,
			0,
			VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT, TES,
			"main",
			nullptr
			});
	}
	if (VK_NULL_HANDLE != TCS) {
		PSSCI.push_back({
			VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
			nullptr,
			0,
			VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT, TCS,
			"main",
			nullptr
			});
	}
	if (VK_NULL_HANDLE != GS) {
		PSSCI.push_back({
			VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
			nullptr,
			0,
			VK_SHADER_STAGE_GEOMETRY_BIT, GS,
			"main",
			nullptr
			});
	}
	assert(!PSSCI.empty() && "");

	const std::array<VkVertexInputBindingDescription, 0> VIBDs = {};
	const std::array<VkVertexInputAttributeDescription, 0> VIADs = {};
	const VkPipelineVertexInputStateCreateInfo PVISCI = {
		VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
		nullptr,
		0,
		static_cast<uint32_t>(VIBDs.size()), VIBDs.data(),
		static_cast<uint32_t>(VIADs.size()), VIADs.data()
	};

	const VkPipelineInputAssemblyStateCreateInfo PIASCI = {
		VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
		nullptr,
		0,
		VK_PRIMITIVE_TOPOLOGY_PATCH_LIST, //!< トポロジに PATCH_KIST を指定
		VK_FALSE
	};
	assert((
		(PIASCI.topology != VK_PRIMITIVE_TOPOLOGY_LINE_LIST_WITH_ADJACENCY || PIASCI.topology != VK_PRIMITIVE_TOPOLOGY_LINE_STRIP_WITH_ADJACENCY || PIASCI.topology != VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST_WITH_ADJACENCY || PIASCI.topology != VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP_WITH_ADJACENCY)
		|| PDF.geometryShader) && "");
	assert((PIASCI.topology != VK_PRIMITIVE_TOPOLOGY_PATCH_LIST || PDF.tessellationShader) && "");
	assert((
		(PIASCI.topology != VK_PRIMITIVE_TOPOLOGY_POINT_LIST || PIASCI.topology != VK_PRIMITIVE_TOPOLOGY_LINE_LIST || PIASCI.topology != VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST || PIASCI.topology != VK_PRIMITIVE_TOPOLOGY_LINE_LIST_WITH_ADJACENCY || PIASCI.topology != VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST_WITH_ADJACENCY || PIASCI.topology != VK_PRIMITIVE_TOPOLOGY_PATCH_LIST)
		|| PIASCI.primitiveRestartEnable == VK_FALSE) && "");

	const VkPipelineTessellationStateCreateInfo PTSCI = {
		VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO,
		nullptr,
		0,
		1 //!< パッチコントロールポイントを指定
	};

	const VkPipelineViewportStateCreateInfo PVSCI = {
		VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
		nullptr,
		0,
		1, nullptr,
		1, nullptr
	};
	assert((PVSCI.viewportCount <= 1 || PDF.multiViewport) && "");

	const VkPipelineRasterizationStateCreateInfo PRSCI = {
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
	assert((!PRSCI.depthClampEnable || PDF.depthClamp) && "");
	assert((PRSCI.polygonMode != VK_POLYGON_MODE_FILL || PDF.fillModeNonSolid) && "");
	assert((PRSCI.lineWidth <= 1.0f || PDF.wideLines) && "");

	const VkSampleMask SM = 0xffffffff;
	const VkPipelineMultisampleStateCreateInfo PMSCI = {
		VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
		nullptr,
		0,
		VK_SAMPLE_COUNT_1_BIT,
		VK_FALSE, 0.0f,
		&SM,
		VK_FALSE, VK_FALSE
	};
	assert((PMSCI.sampleShadingEnable == VK_FALSE || PDF.sampleRateShading) && "");
	assert((PMSCI.minSampleShading >= 0.0f && PMSCI.minSampleShading <= 1.0f) && "");
	assert((PMSCI.alphaToOneEnable == VK_FALSE || PDF.alphaToOne) && "");

	const VkPipelineDepthStencilStateCreateInfo PDSSCI = {
		VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
		nullptr,
		0,
		VK_TRUE,
		VK_TRUE,
		VK_COMPARE_OP_LESS_OR_EQUAL,
		VK_FALSE,
		VK_FALSE,
		{ VK_STENCIL_OP_KEEP, VK_STENCIL_OP_KEEP, VK_STENCIL_OP_KEEP, VK_COMPARE_OP_NEVER, 0, 0, 0 },
		{ VK_STENCIL_OP_KEEP, VK_STENCIL_OP_KEEP, VK_STENCIL_OP_KEEP, VK_COMPARE_OP_ALWAYS, 0, 0, 0 },
		0.0f, 1.0f
	};

	const std::array<VkPipelineColorBlendAttachmentState, 1> PCBASs = {
		{
			VK_FALSE,
			VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ONE, VK_BLEND_OP_ADD,
			VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ONE, VK_BLEND_OP_ADD,
			VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
		}
	};
	if (!PDF.independentBlend) {
		for (auto i : PCBASs) {
			assert(memcmp(&i, &PCBASs[0], sizeof(PCBASs[0])) == 0 && "");
		}
	}
	const VkPipelineColorBlendStateCreateInfo PCBSCI = {
		VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
		nullptr,
		0,
		VK_FALSE, VK_LOGIC_OP_COPY,
		static_cast<uint32_t>(PCBASs.size()), PCBASs.data(),
		{ 1.0f, 1.0f, 1.0f, 1.0f }
	};

	const std::array<VkDynamicState, 2> DSs = {
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR
	};
	const VkPipelineDynamicStateCreateInfo PDSCI = {
		VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
		nullptr,
		0,
		static_cast<uint32_t>(DSs.size()), DSs.data()
	};

	const std::array<VkGraphicsPipelineCreateInfo, 1> GPCIs = {
		{
			VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
			nullptr,
#ifdef _DEBUG
			VK_PIPELINE_CREATE_DISABLE_OPTIMIZATION_BIT,
#else
			0,
#endif
			static_cast<uint32_t>(PSSCI.size()), PSSCI.data(),
			&PVISCI,
			&PIASCI,
			&PTSCI,
			&PVSCI,
			&PRSCI,
			&PMSCI,
			&PDSSCI,
			&PCBSCI,
			&PDSCI,
			PL,
			RP, 0,
			VK_NULL_HANDLE, -1
		}
	};
	VERIFY_SUCCEEDED(vkCreateGraphicsPipelines(Device,
		PC,
		static_cast<uint32_t>(GPCIs.size()), GPCIs.data(),
		GetAllocationCallbacks(),
		&Pipeline));

	LOG_OK();
}

void VKExt::CreatePipeline_VsFs()
{
	std::array<VkPipelineCache, 1> PCs = { VK_NULL_HANDLE };
	const auto PCOPath = GetBasePath() + TEXT(".pco");
	DeleteFile(PCOPath.data());

	{
		std::ifstream In(PCOPath.c_str(), std::ios::in | std::ios::binary);
		if (!In.fail()) {
			In.seekg(0, std::ios_base::end);
			const size_t Size = In.tellg();
			In.seekg(0, std::ios_base::beg);
			assert(Size && "");
			if (Size) {
				auto Data = new char[Size];
				In.read(Data, Size);
				ValidatePipelineCache(GetCurrentPhysicalDevice(), Size, Data);
				const VkPipelineCacheCreateInfo PCCI = {
					VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO,
					nullptr,
					0,
					Size, Data
				};
				for (auto& i : PCs) {
					VERIFY_SUCCEEDED(vkCreatePipelineCache(Device, &PCCI, GetAllocationCallbacks(), &i));
				}
				delete[] Data;
			}
			In.close();
		}
		else {
			const VkPipelineCacheCreateInfo PCCI = {
				VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO,
				nullptr,
				0,
				0, nullptr
			};
			for (auto& i : PCs) {
				VERIFY_SUCCEEDED(vkCreatePipelineCache(Device, &PCCI, GetAllocationCallbacks(), &i));
			}
		}
	}

	assert(ShaderModules.size() > 1 && "");
	
	const auto RP = RenderPasses[0];
	const auto PL = PipelineLayouts[0];

	auto Thread = std::thread::thread([&](VkPipeline& P, const VkPipelineLayout PL,
		const VkShaderModule VS, const VkShaderModule FS, const VkShaderModule TES, const VkShaderModule TCS, const VkShaderModule GS,
		const VkRenderPass RP, VkPipelineCache PC)
		{
			CreatePipeline_Default(P, PL, VS, FS, TES, TCS, GS, RP, PC);
		},
		std::ref(Pipeline), PL, ShaderModules[0], ShaderModules[1], NullShaderModule, NullShaderModule, NullShaderModule, RP, PCs[0]);

	Thread.join();

	if (PCs.size() > 1) {
		VERIFY_SUCCEEDED(vkMergePipelineCaches(Device, PCs.back(), static_cast<uint32_t>(PCs.size() - 1), PCs.data()));
	}
	{
		size_t Size;
		VERIFY_SUCCEEDED(vkGetPipelineCacheData(Device, PCs.back(), &Size, nullptr));
		if (Size) {
			auto Data = new char[Size];
			VERIFY_SUCCEEDED(vkGetPipelineCacheData(Device, PCs.back(), &Size, Data));
			std::ofstream Out(PCOPath.c_str(), std::ios::out | std::ios::binary);
			if (!Out.fail()) {
				Out.write(Data, Size);
				Out.close();
			}
			delete[] Data;
		}
	}
	for (auto i : PCs) {
		vkDestroyPipelineCache(Device, i, GetAllocationCallbacks());
	}
}
void VKExt::CreatePipeline_VsFsTesTcsGs_Tesselation()
{
	std::array<VkPipelineCache, 1> PCs = { VK_NULL_HANDLE };
	const auto PCOPath = GetBasePath() + TEXT(".pco");
	DeleteFile(PCOPath.data());

	{
		std::ifstream In(PCOPath.c_str(), std::ios::in | std::ios::binary);
		if (!In.fail()) {
			In.seekg(0, std::ios_base::end);
			const size_t Size = In.tellg();
			In.seekg(0, std::ios_base::beg);
			assert(Size && "");
			if (Size) {
				auto Data = new char[Size];
				In.read(Data, Size);
				ValidatePipelineCache(GetCurrentPhysicalDevice(), Size, Data);
				const VkPipelineCacheCreateInfo PCCI = {
					VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO,
					nullptr,
					0,
					Size, Data
				};
				for (auto& i : PCs) {
					VERIFY_SUCCEEDED(vkCreatePipelineCache(Device, &PCCI, GetAllocationCallbacks(), &i));
				}
				delete[] Data;
			}
			In.close();
		}
		else {
			const VkPipelineCacheCreateInfo PCCI = {
				VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO,
				nullptr,
				0,
				0, nullptr
			};
			for (auto& i : PCs) {
				VERIFY_SUCCEEDED(vkCreatePipelineCache(Device, &PCCI, GetAllocationCallbacks(), &i));
			}
		}
	}

	assert(ShaderModules.size() > 4 && "");

	const auto RP = RenderPasses[0];
	const auto PL = PipelineLayouts[0];

	auto Thread = std::thread::thread([&](VkPipeline& P, const VkPipelineLayout PL,
		const VkShaderModule VS, const VkShaderModule FS, const VkShaderModule TES, const VkShaderModule TCS, const VkShaderModule GS,
		const VkRenderPass RP, VkPipelineCache PC)
		{
			CreatePipeline_Tesselation(P, PL, VS, FS, TES, TCS, GS, RP, PC);
		},
		std::ref(Pipeline), PL, ShaderModules[0], ShaderModules[1], ShaderModules[2], ShaderModules[3], ShaderModules[4], RP, PCs[0]);

	Thread.join();

	if (PCs.size() > 1) {
		VERIFY_SUCCEEDED(vkMergePipelineCaches(Device, PCs.back(), static_cast<uint32_t>(PCs.size() - 1), PCs.data()));
	}
	{
		size_t Size;
		VERIFY_SUCCEEDED(vkGetPipelineCacheData(Device, PCs.back(), &Size, nullptr));
		if (Size) {
			auto Data = new char[Size];
			VERIFY_SUCCEEDED(vkGetPipelineCacheData(Device, PCs.back(), &Size, Data));
			std::ofstream Out(PCOPath.c_str(), std::ios::out | std::ios::binary);
			if (!Out.fail()) {
				Out.write(Data, Size);
				Out.close();
			}
			delete[] Data;
		}
	}
	for (auto i : PCs) {
		vkDestroyPipelineCache(Device, i, GetAllocationCallbacks());
	}

}

void VKExt::CreateRenderPass_ColorDepth(VkRenderPass& RP, const VkFormat Color, const VkFormat Depth)
{
	const std::array<VkAttachmentDescription, 2> ADs = { {
		{
			0,
			Color,
			VK_SAMPLE_COUNT_1_BIT,
			VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_STORE,
			VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
			VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
		},
		{
			0,
			Depth,
			VK_SAMPLE_COUNT_1_BIT,
			VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE, //!< VK_ATTACHMENT_LOAD_OP_CLEAR : VkRenderPassBeginInfo.pClearValues 使用必須
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

//!< ファーストパスで ColorDepth に書き込み、セカンドパスで PostProcess を行う場合の例 In first pass ColorDepth, second pass PostProcess
void VKExt::CreateRenderPass_ColorDepth_PostProcess(VkRenderPass& RP, const VkFormat Color, const VkFormat Depth)
{
	const std::array<VkAttachmentDescription, 3> ADs = { {
		{
			0,
			Color,
			VK_SAMPLE_COUNT_1_BIT,
			VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
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
			VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_STORE,
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