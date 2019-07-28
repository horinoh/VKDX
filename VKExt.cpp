#include "stdafx.h"

#include "VKExt.h"

//void VKExt::CreateDescriptorSetLayoutBindings_1UB(std::vector<VkDescriptorSetLayoutBinding>& DescriptorSetLayoutBindings, const VkShaderStageFlags ShaderStageFlags /*= VK_SHADER_STAGE_ALL_GRAPHICS*/) const
//{
//	DescriptorSetLayoutBindings = {
//		{
//			0, //!< binding = 0 バインディング Binding
//			VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, //!< タイプ Type
//			1, //!< 個数 Count
//			ShaderStageFlags,
//			nullptr
//		},
//	};
//}
//void VKExt::CreateDescriptorSetLayoutBindings_1CIS(std::vector<VkDescriptorSetLayoutBinding>& DescriptorSetLayoutBindings, const VkShaderStageFlags ShaderStageFlags /*= VK_SHADER_STAGE_ALL_GRAPHICS*/) const
//{
//	DescriptorSetLayoutBindings = {
//		{
//			0, //!< binding = 0
//			VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER/*VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE*/,
//			1,
//			ShaderStageFlags,
//			nullptr
//		},
//	};
//}
//void VKExt::CreateDescriptorSetLayoutBindings_1UB_1CIS(std::vector<VkDescriptorSetLayoutBinding>& DescriptorSetLayoutBindings, const VkShaderStageFlags ShaderStageFlags_UB /*= VK_SHADER_STAGE_ALL_GRAPHICS*/, const VkShaderStageFlags ShaderStageFlags_CIS /*= VK_SHADER_STAGE_ALL_GRAPHICS*/) const
//{
//	DescriptorSetLayoutBindings = {
//		{
//			0, //!< binding = 0
//			VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
//			1,
//			ShaderStageFlags_UB,
//			nullptr
//		},
//		{
//			1, //!< binding = 1
//			VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER/*VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE*/,
//			1,
//			ShaderStageFlags_CIS,
//			nullptr
//		}
//	};
//}
//void VKExt::CreateDescriptorSetLayoutBindings_1SI(std::vector<VkDescriptorSetLayoutBinding>& DescriptorSetLayoutBindings, const VkShaderStageFlags ShaderStageFlags /*= VK_SHADER_STAGE_ALL_GRAPHICS*/) const
//{
//	DescriptorSetLayoutBindings = {
//		{
//			0, //!< binding = 0
//			VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
//			1,
//			ShaderStageFlags,
//			nullptr
//		},
//	};
//}

void VKExt::CreateDescriptorSetLayout_1UB(VkDescriptorSetLayout& DSL, const VkShaderStageFlags SSF)
{
	const  std::array<VkDescriptorSetLayoutBinding, 1> DSLBs = {
		{
			0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1,
			SSF,
			nullptr
		},
	};
	const VkDescriptorSetLayoutCreateInfo DSLCI = {
		VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
		nullptr,
		0,
		static_cast<uint32_t>(DSLBs.size()), DSLBs.data()
	};

	VERIFY_SUCCEEDED(vkCreateDescriptorSetLayout(Device, &DSLCI, GetAllocationCallbacks(), &DSL));
}

void VKExt::CreateDescriptorSetLayout_1CIS(VkDescriptorSetLayout& DSL, const VkShaderStageFlags SSF)
{
	//!< ImmutableSampler == STATIC_SAMPLER_DESC 相当？
#if 0
	const VkSamplerCreateInfo SCI = {
		VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
		nullptr,
		0,
		VK_FILTER_LINEAR, VK_FILTER_LINEAR, VK_SAMPLER_MIPMAP_MODE_LINEAR,
		VK_SAMPLER_ADDRESS_MODE_REPEAT, VK_SAMPLER_ADDRESS_MODE_REPEAT, VK_SAMPLER_ADDRESS_MODE_REPEAT,
		0.0f,
		VK_FALSE, 1.0f,
		VK_FALSE, VK_COMPARE_OP_NEVER,
		0.0f, 1.0f,
		VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE,
		VK_FALSE
	};
	VkSampler Sampler = VK_NULL_HANDLE;
	VERIFY_SUCCEEDED(vkCreateSampler(Device, &SCI, GetAllocationCallbacks(), &Sampler));
	const std::array<VkSampler, 1> ISs = { { Sampler } };
	const std::array<VkDescriptorSetLayoutBinding, 1> DSLBs = {
		{
			0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, static_cast<uint32_t>(ISs.size()),
			VK_SHADER_STAGE_FRAGMENT_BIT,
			ISs.data()
		},
	};
#else
	const std::array<VkDescriptorSetLayoutBinding, 1> DSLBs = {
		{
			0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1,
			SSF,
			nullptr
		},
	};
#endif
	const VkDescriptorSetLayoutCreateInfo DSLCI = {
		VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
		nullptr,
		0,
		static_cast<uint32_t>(DSLBs.size()), DSLBs.data()
	};

	VERIFY_SUCCEEDED(vkCreateDescriptorSetLayout(Device, &DSLCI, GetAllocationCallbacks(), &DSL));
}

void VKExt::CreateDescriptorSetLayout_1UB_1CIS(VkDescriptorSetLayout& DSL, const VkShaderStageFlags SSF_UB, const VkShaderStageFlags SSF_CIS)
{
	const  std::array<VkDescriptorSetLayoutBinding, 2> DSLBs = { {
		{
			0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1,
			SSF_UB,
			nullptr
		},
		{
			1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1,
			SSF_CIS,
			nullptr
		}
	} };
	const VkDescriptorSetLayoutCreateInfo DSLCI = {
		VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
		nullptr,
		0,
		static_cast<uint32_t>(DSLBs.size()), DSLBs.data()
	};

	VERIFY_SUCCEEDED(vkCreateDescriptorSetLayout(Device, &DSLCI, GetAllocationCallbacks(), &DSL));
}

void VKExt::CreatePipelineLayout_1DSL(const VkDescriptorSetLayout& DSL)
{
	const std::array<VkDescriptorSetLayout, 1> DSLs = {
		DSL,
	};

	const std::array<VkPushConstantRange, 0> PCRs = {};

	const VkPipelineLayoutCreateInfo PLCI = {
		VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
		nullptr,
		0,
		static_cast<uint32_t>(DSLs.size()), DSLs.data(),
		static_cast<uint32_t>(PCRs.size()), PCRs.data()
	};
	VERIFY_SUCCEEDED(vkCreatePipelineLayout(Device, &PLCI, GetAllocationCallbacks(), &PipelineLayout));

	LOG_OK();
}

void VKExt::CreateDescriptorSet_1DSL(VkDescriptorSet& DS, const VkDescriptorPool DP, const VkDescriptorSetLayout& DSL)
{
	DescriptorSets.resize(1);
	auto& DS = DescriptorSets[0];

	const std::array<VkDescriptorSetLayout, 1> DSLs = {
		DSL,
	};
	const VkDescriptorSetAllocateInfo DSAI = {
		VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
		nullptr,
		DP,
		static_cast<uint32_t>(DSLs.size()), DSLs.data()
	};
	VERIFY_SUCCEEDED(vkAllocateDescriptorSets(Device, &DSAI, &DS));

	LOG_OK();
}

void VKExt::CreateDescriptorPool_1UB()
{
	const std::array<VkDescriptorPoolSize, 1> DPSs = {
		{
			VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			1
		}
	};
	const uint32_t MaxSets = [&]() {
		uint32_t Count = 0;
		for (const auto& i : DPSs) {
			Count = std::max(Count, i.descriptorCount);
		}
		return Count;
	}();
	const VkDescriptorPoolCreateInfo DPCI = {
		VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
		nullptr,
		0, //!< デスクリプタセットを個々に解放したい場合には VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT を指定(プールごとの場合は不要)
		MaxSets,
		static_cast<uint32_t>(DPSs.size()), DPSs.data()
	};
	VERIFY_SUCCEEDED(vkCreateDescriptorPool(Device, &DPCI, GetAllocationCallbacks(), &DescriptorPool));
	assert(VK_NULL_HANDLE != DescriptorPool && "Failed to create descriptor pool");

	LOG_OK();
}
void VKExt::UpdateDescriptorSet_1UB()
{
	const std::array<VkDescriptorBufferInfo, 1> DBIs = {
			{
				UniformBuffer,
				0/*オフセット(要アライン)*/,
				VK_WHOLE_SIZE
			}
	};
	const std::array<VkWriteDescriptorSet, 1> WDSs = {
		{
			VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
			nullptr,
			DescriptorSets[0], 0, 0, //!< デスクリプタセット、バインディングポイント、配列の場合の添字(配列でなければ0)
			static_cast<uint32_t>(DBIs.size()),
			VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			nullptr,
			DBIs.data(),
			nullptr
		}
	};

	const std::array<VkCopyDescriptorSet, 0> CDSs = {};

	vkUpdateDescriptorSets(Device,
		static_cast<uint32_t>(WDSs.size()), WDSs.data(),
		static_cast<uint32_t>(CDSs.size()), CDSs.data());

	LOG_OK();
}

void VKExt::CreateDescriptorPool_1CIS()
{
	const std::array<VkDescriptorPoolSize, 1> DPSs = {
		{
			VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			1
		}
	};
	const uint32_t MaxSets = [&]() {
		uint32_t Count = 0;
		for (const auto& i : DPSs) {
			Count = std::max(Count, i.descriptorCount);
		}
		return Count;
	}();
	const VkDescriptorPoolCreateInfo DPCI = {
		VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
		nullptr,
		0,
		MaxSets,
		static_cast<uint32_t>(DPSs.size()), DPSs.data()
	};
	VERIFY_SUCCEEDED(vkCreateDescriptorPool(Device, &DPCI, GetAllocationCallbacks(), &DescriptorPool));
	assert(VK_NULL_HANDLE != DescriptorPool && "Failed to create descriptor pool");

	LOG_OK();
}

void VKExt::UpdateDescriptorSet_1CIS()
{
	const std::array<VkDescriptorImageInfo, 1> DIIs = {
		{
			Samplers[0],
			ImageView,
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
		}
	};
	const std::array<VkWriteDescriptorSet, 1> WDSs = {
		{
			VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
			nullptr,
			DescriptorSets[0], 0, 0,
			static_cast<uint32_t>(DIIs.size()),
			VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			DIIs.data(),
			nullptr,
			nullptr
		}
	};

	const std::array<VkCopyDescriptorSet, 0> CDSs = {};

	vkUpdateDescriptorSets(Device,
		static_cast<uint32_t>(WDSs.size()), WDSs.data(),
		static_cast<uint32_t>(CDSs.size()), CDSs.data());

	LOG_OK();
}

#if 0
void VKExt::CreateDescriptorPool_1SI()
{
	const std::array<VkDescriptorPoolSize, 1> DPSs = {
		{ 
			VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 
			1
		}
	};
	const uint32_t MaxSets = [&]() {
		uint32_t Count = 0;
		for (const auto& i : DPSs) {
			Count = std::max(Count, i.descriptorCount);
		}
		return Count;
	}();
	const VkDescriptorPoolCreateInfo DPCI = {
		VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
		nullptr,
		0,
		MaxSets,
		static_cast<uint32_t>(DPSs.size()), DPSs.data()
	};
	VERIFY_SUCCEEDED(vkCreateDescriptorPool(Device, &DPCI, GetAllocationCallbacks(), &DescriptorPool));
	assert(VK_NULL_HANDLE != DescriptorPool && "Failed to create descriptor pool");

	LOG_OK();
}
void VKExt::UpdateDescriptorSet_1SI()
{
	const std::array<VkDescriptorImageInfo, 1> DIIs = {
		{
			Samplers[0],
			ImageView,
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
		}
	};
	const std::array<VkWriteDescriptorSet, 1> WDSs = {
		{
			VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
			nullptr,
			DescriptorSets[0], 0, 0,
			static_cast<uint32_t>(DIIs.size()),
			VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
			DIIs.data(),
			nullptr,
			nullptr
		}
	};

	const std::array<VkCopyDescriptorSet, 0> CDSs = {};

	vkUpdateDescriptorSets(Device,
		static_cast<uint32_t>(WDSs.size()), WDSs.data(),
		static_cast<uint32_t>(CDSs.size()), CDSs.data());

	LOG_OK();
}
#endif
#if 1
void VKExt::CreateDescriptorPool_1SI()
{
	const std::array<VkDescriptorPoolSize, 1> DPSs = {
	{
		VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
		1
	}
	};
	const uint32_t MaxSets = [&]() {
		uint32_t Count = 0;
		for (const auto& i : DPSs) {
			Count = std::max(Count, i.descriptorCount);
		}
		return Count;
	}();
	const VkDescriptorPoolCreateInfo DPCI = {
		VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
		nullptr,
		0,
		MaxSets,
		static_cast<uint32_t>(DPSs.size()), DPSs.data()
	};
	VERIFY_SUCCEEDED(vkCreateDescriptorPool(Device, &DPCI, GetAllocationCallbacks(), &DescriptorPool));
	assert(VK_NULL_HANDLE != DescriptorPool && "Failed to create descriptor pool");

	LOG_OK();
}
void VKExt::UpdateDescriptorSet_1SI()
{
	const std::array<VkDescriptorImageInfo, 1> DIIs = {
	{
		VK_NULL_HANDLE,
		ImageView,
		VK_IMAGE_LAYOUT_GENERAL
	}
	};
	const std::array<VkWriteDescriptorSet, 1> WDSs = {
		{
			VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
			nullptr,
			DescriptorSets[0], 0, 0,
			static_cast<uint32_t>(DIIs.size()),
			VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
			DIIs.data(),
			nullptr,
			nullptr
		}
	};

	const std::array<VkCopyDescriptorSet, 0> CDSs = {};

	vkUpdateDescriptorSets(Device,
		static_cast<uint32_t>(WDSs.size()), WDSs.data(),
		static_cast<uint32_t>(CDSs.size()), CDSs.data());

	LOG_OK();
}
#endif

void VKExt::CreateDescriptorPool_1UB_1CIS()
{
	const std::array<VkDescriptorPoolSize, 2> DPSs = { {
		{
			VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			1
		},
		{
			VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			1
		}
	} };
	const uint32_t MaxSets = [&]() {
		uint32_t Count = 0;
		for (const auto& i : DPSs) {
			Count = std::max(Count, i.descriptorCount);
		}
		return Count;
	}();
	const VkDescriptorPoolCreateInfo DPCI = {
		VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
		nullptr,
		0,
		MaxSets,
		static_cast<uint32_t>(DPSs.size()), DPSs.data()
	};
	VERIFY_SUCCEEDED(vkCreateDescriptorPool(Device, &DPCI, GetAllocationCallbacks(), &DescriptorPool));
	assert(VK_NULL_HANDLE != DescriptorPool && "Failed to create descriptor pool");

	LOG_OK();
}
void VKExt::UpdateDescriptorSet_1UB_1CIS()
{
	const std::array<VkDescriptorBufferInfo, 1> DBIs = {
		{
			UniformBuffer,
			0,
			VK_WHOLE_SIZE
		}
	};
	const std::array<VkDescriptorImageInfo, 1> DIIs = {
		{
			Samplers[0],
			ImageView,
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
		}
	};
	const std::array<VkWriteDescriptorSet, 2> WDSs = { {
		{
			VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
			nullptr,
			DescriptorSets[0], 0, 0,
			static_cast<uint32_t>(DBIs.size()),
			VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			nullptr,
			DBIs.data(),
			nullptr
		},
		{
			VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
			nullptr,
			DescriptorSets[0], 1, 0,
			static_cast<uint32_t>(DIIs.size()),
			VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			DIIs.data(),
			nullptr,
			nullptr
		}
	} };

	const std::array<VkCopyDescriptorSet, 0> CDSs = {};

	vkUpdateDescriptorSets(Device,
		static_cast<uint32_t>(WDSs.size()), WDSs.data(),
		static_cast<uint32_t>(CDSs.size()), CDSs.data());

	LOG_OK();
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

	ShaderModules.resize(5);
	const auto ShaderPath = GetBasePath();
	ShaderModules[0] = CreateShaderModule((ShaderPath + TEXT(".vert.spv")).data());
	ShaderModules[1] = CreateShaderModule((ShaderPath + TEXT(".frag.spv")).data());
	
	const auto RP = RenderPasses[0];

	auto Thread = std::thread::thread([&](VkPipeline& P, const VkPipelineLayout PL,
		const VkShaderModule VS, const VkShaderModule FS, const VkShaderModule TES, const VkShaderModule TCS, const VkShaderModule GS,
		const VkRenderPass RP, VkPipelineCache PC)
		{
			CreatePipeline_Default(P, PL, VS, FS, TES, TCS, GS, RP, PC);
		},
		std::ref(Pipeline), PipelineLayout, ShaderModules[0], ShaderModules[1], NullShaderModule, NullShaderModule, NullShaderModule, RP, PCs[0]);

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

	ShaderModules.resize(5);
	const auto ShaderPath = GetBasePath();
	ShaderModules[0] = CreateShaderModule((ShaderPath + TEXT(".vert.spv")).data());
	ShaderModules[1] = CreateShaderModule((ShaderPath + TEXT(".frag.spv")).data());
	ShaderModules[2] = CreateShaderModule((ShaderPath + TEXT(".tese.spv")).data());
	ShaderModules[3] = CreateShaderModule((ShaderPath + TEXT(".tesc.spv")).data());
	ShaderModules[4] = CreateShaderModule((ShaderPath + TEXT(".geom.spv")).data());

	const auto RP = RenderPasses[0];

	auto Thread = std::thread::thread([&](VkPipeline& P, const VkPipelineLayout PL,
		const VkShaderModule VS, const VkShaderModule FS, const VkShaderModule TES, const VkShaderModule TCS, const VkShaderModule GS,
		const VkRenderPass RP, VkPipelineCache PC)
		{
			CreatePipeline_Tesselation(P, PL, VS, FS, TES, TCS, GS, RP, PC);
		},
		std::ref(Pipeline), PipelineLayout, ShaderModules[0], ShaderModules[1], ShaderModules[2], ShaderModules[3], ShaderModules[4], RP, PCs[0]);

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
void VKExt::CreateSampler_LR(VkSampler* Sampler, const float MaxLOD) const
{
	//!< シェーダ内での記述例 layout (set=0, binding=0) uniform sampler2D Sampler;
	//!< VK にはシェーダビジビリティとレジスタの設定が無く、正規化の設定がある
	[&](VkSampler* Sampler, const float MaxLOD) {
		const VkSamplerCreateInfo SamplerCreateInfo = {
			VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
			nullptr,
			0,
			VK_FILTER_LINEAR, VK_FILTER_LINEAR, VK_SAMPLER_MIPMAP_MODE_LINEAR, // min, mag, mip
			VK_SAMPLER_ADDRESS_MODE_REPEAT, VK_SAMPLER_ADDRESS_MODE_REPEAT, VK_SAMPLER_ADDRESS_MODE_REPEAT, // u, v, w
			0.0f, // lod bias
			VK_FALSE, 1.0f, // anisotropy
			VK_FALSE, VK_COMPARE_OP_NEVER, // compare
			0.0f, MaxLOD, // min, maxlod
			VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE, // border
			VK_FALSE // addressing VK_FALSE:正規化[0.0-1.0], VK_TRUE:画像のディメンション
		};
		VERIFY_SUCCEEDED(vkCreateSampler(Device, &SamplerCreateInfo, GetAllocationCallbacks(), Sampler));
	}(Sampler, MaxLOD);
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
			//!< VK_ATTACHMENT_LOAD_OP_CLEAR : レンダーパスの「開始時にクリア」を行う (VkRenderPassBeginInfo.pClearValuesのセットが必須になる)
			VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE,
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
			//!< VK_ATTACHMENT_LOAD_OP_CLEAR : レンダーパスの「開始時にクリア」を行う (VkRenderPassBeginInfo.pClearValuesのセットが必須になる)
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

void VKExt::CreateFramebuffer_Color()
{
	Framebuffers.resize(SwapchainImages.size());
	for (uint32_t i = 0; i < Framebuffers.size(); ++i) {
		[&](VkFramebuffer* Framebuffer, const VkImageView ColorView, const VkRenderPass RP, const uint32_t Width, const uint32_t Height) {
			const std::vector<VkImageView> Attachments = {
				ColorView 
			};
			const VkFramebufferCreateInfo FramebufferCreateInfo = {
				VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
				nullptr,
				0,
				RP, //!< ここで作成するフレームバッファは RenderPass と「コンパチ」な別のレンダーパスでも使用可能
				static_cast<uint32_t>(Attachments.size()), Attachments.data(),
				Width, Height,
				1
			};
			VERIFY_SUCCEEDED(vkCreateFramebuffer(Device, &FramebufferCreateInfo, GetAllocationCallbacks(), Framebuffer));
		}(&Framebuffers[i], SwapchainImageViews[i], RenderPasses[0], SurfaceExtent2D.width, SurfaceExtent2D.height);
	}
}
void VKExt::CreateFramebuffer_ColorDepth()
{
	Framebuffers.resize(SwapchainImages.size());
	for (uint32_t i = 0; i < Framebuffers.size(); ++i) {
		[&](VkFramebuffer* Framebuffer, const VkImageView ColorView, const VkImageView DepthStencilView, const VkRenderPass RP, const uint32_t Width, const uint32_t Height) {
			const std::vector<VkImageView> Attachments = {
				ColorView,
				DepthStencilView
			};
			const VkFramebufferCreateInfo FramebufferCreateInfo = {
				VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
				nullptr,
				0,
				RP,
				static_cast<uint32_t>(Attachments.size()), Attachments.data(),
				Width, Height,
				1
			};
			VERIFY_SUCCEEDED(vkCreateFramebuffer(Device, &FramebufferCreateInfo, GetAllocationCallbacks(), Framebuffer));
		}(&Framebuffers[i], SwapchainImageViews[i], DepthStencilImageView, RenderPasses[0], SurfaceExtent2D.width, SurfaceExtent2D.height);
	}
}

void VKExt::CreateShader_VsPs(std::vector<VkShaderModule>& ShaderModules, std::vector<VkPipelineShaderStageCreateInfo>& PipelineShaderStageCreateInfos) const
{
	const auto ShaderPath = GetBasePath();
	ShaderModules = {
		CreateShaderModule((ShaderPath + TEXT(".vert.spv")).data()),
		CreateShaderModule((ShaderPath + TEXT(".frag.spv")).data())
	};

	//!< HLSL コンパイル時のデフォルトエントリポイント名が "main" なのでそれに合わせることにする
	const char* EntrypointName = "main";

	//!< シェーダ内のコンスタント変数をパイプライン作成時に変更する場合に使用
	const std::array<VkSpecializationMapEntry, 0> SMEs = { /*{ uint32_t constantID, uint32_t offset, size_t size },*/};
	const VkSpecializationInfo SI = { static_cast<uint32_t>(SMEs.size()), SMEs.data() };

	PipelineShaderStageCreateInfos = {
		{
			VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
			nullptr,
			0,
			VK_SHADER_STAGE_VERTEX_BIT, ShaderModules[0],
			EntrypointName,
			nullptr //!< &SI
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
		CreateShaderModule((ShaderPath + TEXT(".vert.spv")).data()),
		CreateShaderModule((ShaderPath + TEXT(".frag.spv")).data()),
		CreateShaderModule((ShaderPath + TEXT(".tese.spv")).data()),
		CreateShaderModule((ShaderPath + TEXT(".tesc.spv")).data()),
		CreateShaderModule((ShaderPath + TEXT(".geom.spv")).data()) 
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

void VKExt::CreateShader_Cs(std::vector<VkShaderModule>& ShaderModules, std::vector<VkPipelineShaderStageCreateInfo>& PipelineShaderStageCreateInfos) const
{
	const auto ShaderPath = GetBasePath();
	ShaderModules = {
		CreateShaderModule((ShaderPath + TEXT(".comp.spv")).data()),
	};
	const char* EntrypointName = "main";
	PipelineShaderStageCreateInfos = {
		{
			VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
			nullptr,
			0,
			VK_SHADER_STAGE_COMPUTE_BIT, ShaderModules[0],
			EntrypointName,
			nullptr
		},
	};
}
