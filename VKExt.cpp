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

void VKExt::CreatePipelineLayout_1UB_GS()
{
	const  std::array<VkDescriptorSetLayoutBinding, 1> DSLBs = {
		{
			0,
			VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			1,
			VK_SHADER_STAGE_GEOMETRY_BIT,
			nullptr
		},
	};

	const VkDescriptorSetLayoutCreateInfo DSLCI = {
		VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
		nullptr,
		0,
		static_cast<uint32_t>(DSLBs.size()), DSLBs.data()
	};

	VkDescriptorSetLayout DSL = VK_NULL_HANDLE;
	VERIFY_SUCCEEDED(vkCreateDescriptorSetLayout(Device, &DSLCI, GetAllocationCallbacks(), &DSL));
	DescriptorSetLayouts.push_back(DSL);

	const std::array<VkPushConstantRange, 0> PCRs = {};

	const VkPipelineLayoutCreateInfo PLCI = {
		VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
		nullptr,
		0,
		static_cast<uint32_t>(DescriptorSetLayouts.size()), DescriptorSetLayouts.data(),
		static_cast<uint32_t>(PCRs.size()), PCRs.data()
	};
	VERIFY_SUCCEEDED(vkCreatePipelineLayout(Device, &PLCI, GetAllocationCallbacks(), &PipelineLayout));

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

void VKExt::CreatePipelineLayout_1CIS_FS()
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
			0,
			VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			static_cast<uint32_t>(ISs.size()),
			VK_SHADER_STAGE_FRAGMENT_BIT,
			ISs.data()
		},
	};
#else
	const std::array<VkDescriptorSetLayoutBinding, 1> DSLBs = {
		{
			0,
			VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			1,
			VK_SHADER_STAGE_FRAGMENT_BIT,
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

	VkDescriptorSetLayout DSL = VK_NULL_HANDLE;
	VERIFY_SUCCEEDED(vkCreateDescriptorSetLayout(Device, &DSLCI, GetAllocationCallbacks(), &DSL));
	DescriptorSetLayouts.push_back(DSL);

	const std::array<VkPushConstantRange, 0> PCRs = {};

	const VkPipelineLayoutCreateInfo PipelineLayoutCreateInfo = {
		VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
		nullptr,
		0,
		static_cast<uint32_t>(DescriptorSetLayouts.size()), DescriptorSetLayouts.data(),
		static_cast<uint32_t>(PCRs.size()), PCRs.data()
	};
	VERIFY_SUCCEEDED(vkCreatePipelineLayout(Device, &PipelineLayoutCreateInfo, GetAllocationCallbacks(), &PipelineLayout));

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

void VKExt::CreatePipelineLayout_1UB_GS_1CIS_FS()
{
	const std::array<VkDescriptorSetLayoutBinding, 2> DSLBs = { {
		{
			0,
			VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			1,
			VK_SHADER_STAGE_GEOMETRY_BIT,
			nullptr
		},
		{
			1,
			VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			1,
			VK_SHADER_STAGE_FRAGMENT_BIT,
			nullptr
		}
	} };

	const VkDescriptorSetLayoutCreateInfo DSLCI = {
		VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
		nullptr,
		0,
		static_cast<uint32_t>(DSLBs.size()), DSLBs.data()
	};

	VkDescriptorSetLayout DSL = VK_NULL_HANDLE;
	VERIFY_SUCCEEDED(vkCreateDescriptorSetLayout(Device, &DSLCI, GetAllocationCallbacks(), &DSL));
	DescriptorSetLayouts.push_back(DSL);

	const std::array<VkPushConstantRange, 0> PCRs = {};

	const VkPipelineLayoutCreateInfo PLCI = {
		VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
		nullptr,
		0,
		static_cast<uint32_t>(DescriptorSetLayouts.size()), DescriptorSetLayouts.data(),
		static_cast<uint32_t>(PCRs.size()), PCRs.data()
	};
	VERIFY_SUCCEEDED(vkCreatePipelineLayout(Device, &PLCI, GetAllocationCallbacks(), &PipelineLayout));

	LOG_OK();
}
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

void VKExt::CreateRenderPass_1C(VkRenderPass& RenderPass, const VkFormat Format)
{
	//!< アタッチメントディスクリプション
	const std::array<VkAttachmentDescription, 1> AttachDescs = {
		{
			0,
			Format,
			VK_SAMPLE_COUNT_1_BIT,
			VK_ATTACHMENT_LOAD_OP_DONT_CARE,		//!< VK_ATTACHMENT_LOAD_OP_CLEAR にするとレンダーパスの開始時にクリアを行う (VkRenderPassBeginInfo.pClearValuesのセットが必須になる)
			VK_ATTACHMENT_STORE_OP_STORE,			//!< レンダーパス終了時に保存する(表示するのに必要)
			VK_ATTACHMENT_LOAD_OP_DONT_CARE,		//!< (ここでは)ステンシルは未使用
			VK_ATTACHMENT_STORE_OP_DONT_CARE,		//!< (ここでは)ステンシルは未使用
			VK_IMAGE_LAYOUT_UNDEFINED,				//!< レンダーパス開始時のレイアウト (メモリバリアなしにレンダーパス間でレイアウトが変更される)
			VK_IMAGE_LAYOUT_PRESENT_SRC_KHR			//!< レンダーパス終了時のレイアウト
		}
	};

	//!< アタッチメントリファレンス
	const std::array<VkAttachmentReference, 0> InputAttachRefs = {};
	const std::array<VkAttachmentReference, 1> ColorAttachRefs = {
		{ 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL }
	};
	const std::array<VkAttachmentReference, 1> ResolveAttachRefs = {
		{ VK_ATTACHMENT_UNUSED },
	};
	assert(ColorAttachRefs.size() == ResolveAttachRefs.size() && "Size must be same");
	const VkAttachmentReference* DepthAR = nullptr;
	const std::array<uint32_t, 0> PreserveAttach = {}; //!< このサブバス内では使用しないが、サブパス全体においてコンテンツを保持しなくてはならないインデックスを指定
	const std::array<VkSubpassDescription, 1> SubpassDescs = {
		{
			0,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			static_cast<uint32_t>(InputAttachRefs.size()), InputAttachRefs.data(),
			static_cast<uint32_t>(ColorAttachRefs.size()), ColorAttachRefs.data(), ResolveAttachRefs.data(),
			DepthAR,
			static_cast<uint32_t>(PreserveAttach.size()), PreserveAttach.data()
		}
	};

	//!< サブパス
#if 0
	const std::array<VkSubpassDependency, 0> SubpassDepends = {};
#else
	const std::array<VkSubpassDependency, 2> SubpassDepends = { {
		//!< 必要無いが、あえて書くならこんな感じ No need this code, but if dare to write like this
		{
			VK_SUBPASS_EXTERNAL,							//!< サブパス外から
			0,												//!< サブパス0へ
			VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,			//!< パイプラインの最終ステージから
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,	//!< カラー出力ステージへ
			VK_ACCESS_MEMORY_READ_BIT,						//!< 読み込みから
			VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,			//!< カラー書き込みへ
			VK_DEPENDENCY_BY_REGION_BIT,					//!< 同じメモリ領域に対する書き込みが完了してから読み込み (指定しない場合は自前で書き込み完了を管理)
		},
		{
			0,												//!< サブパス0から
			VK_SUBPASS_EXTERNAL,							//!< サブパス外へ
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,	//!< カラー出力ステージから
			VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,			//!< パイプラインの最終ステージへ
			VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,			//!< カラー書き込みから
			VK_ACCESS_MEMORY_READ_BIT,						//!< 読み込みへ
			VK_DEPENDENCY_BY_REGION_BIT,
		}
	} };
#endif

	const VkRenderPassCreateInfo RPCI = {
		VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
		nullptr,
		0,
		static_cast<uint32_t>(AttachDescs.size()), AttachDescs.data(),
		static_cast<uint32_t>(SubpassDescs.size()), SubpassDescs.data(),
		static_cast<uint32_t>(SubpassDepends.size()), SubpassDepends.data()
	};
	VERIFY_SUCCEEDED(vkCreateRenderPass(Device, &RPCI, GetAllocationCallbacks(), &RenderPass));
}

void VKExt::CreateRenderPass_ColorDepth()
{
	const auto CF = ColorFormat;
	const auto DF = VK_FORMAT_D24_UNORM_S8_UINT;

	[&](VkRenderPass* RenderPass, const VkFormat ColorFormat, const VkFormat DepthFormat) {
		const std::vector<VkAttachmentDescription> AttachmentDescriptions = {
			{
				0,
				ColorFormat,
				VK_SAMPLE_COUNT_1_BIT,
				VK_ATTACHMENT_LOAD_OP_DONT_CARE,
				VK_ATTACHMENT_STORE_OP_STORE,
				VK_ATTACHMENT_LOAD_OP_DONT_CARE,
				VK_ATTACHMENT_STORE_OP_DONT_CARE,
				VK_IMAGE_LAYOUT_UNDEFINED,
				VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
			},
			{
				0,
				DepthFormat,
				VK_SAMPLE_COUNT_1_BIT,
				VK_ATTACHMENT_LOAD_OP_CLEAR,
				VK_ATTACHMENT_STORE_OP_STORE,
				VK_ATTACHMENT_LOAD_OP_DONT_CARE,
				VK_ATTACHMENT_STORE_OP_DONT_CARE,
				VK_IMAGE_LAYOUT_UNDEFINED,
				VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
			},
		};

		const std::vector<VkAttachmentReference> InputAttachmentReferences = {};
		const std::vector<VkAttachmentReference> ColorAttachmentReferences = {
			{ 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL },
		};
		const VkAttachmentReference DepthStencilAttachmentReference = {
			1, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
		};
		const std::vector<uint32_t> PreserveAttachments = {};
		const std::vector<VkSubpassDescription> SubpassDescriptions = {
			{
				0,
				VK_PIPELINE_BIND_POINT_GRAPHICS,
				static_cast<uint32_t>(InputAttachmentReferences.size()), InputAttachmentReferences.data(),
				static_cast<uint32_t>(ColorAttachmentReferences.size()), ColorAttachmentReferences.data(), nullptr,
				&DepthStencilAttachmentReference,
				static_cast<uint32_t>(PreserveAttachments.size()), PreserveAttachments.data()
			}
		};

		const std::vector<VkSubpassDependency> SubpassDependencies = {};

		const VkRenderPassCreateInfo RenderPassCreateInfo = {
			VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
			nullptr,
			0,
			static_cast<uint32_t>(AttachmentDescriptions.size()), AttachmentDescriptions.data(),
			static_cast<uint32_t>(SubpassDescriptions.size()), SubpassDescriptions.data(),
			static_cast<uint32_t>(SubpassDependencies.size()), SubpassDependencies.data()
		};
		VERIFY_SUCCEEDED(vkCreateRenderPass(Device, &RenderPassCreateInfo, GetAllocationCallbacks(), RenderPass));
	}(&RenderPass, CF, DF);
}

//!< ファーストパスで ColorDepth に書き込み、セカンドパスで PostProcess を行う場合の例 In first pass ColorDepth, second pass PostProcess
void VKExt::CreateRenderPass_CD_PP()
{
	const auto CF = ColorFormat;
	const auto DF = VK_FORMAT_D24_UNORM_S8_UINT;

	[&](VkRenderPass* RenderPass, const VkFormat ColorFormat, const VkFormat DepthFormat) {
		const std::vector<VkAttachmentDescription> AttachmentDescriptions = {
			{
				0,
				ColorFormat,
				VK_SAMPLE_COUNT_1_BIT,
				VK_ATTACHMENT_LOAD_OP_DONT_CARE,
				VK_ATTACHMENT_STORE_OP_DONT_CARE,
				VK_ATTACHMENT_LOAD_OP_DONT_CARE,
				VK_ATTACHMENT_STORE_OP_DONT_CARE,
				VK_IMAGE_LAYOUT_UNDEFINED,
				VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
			},
			{
				0,
				DepthFormat,
				VK_SAMPLE_COUNT_1_BIT,
				VK_ATTACHMENT_LOAD_OP_CLEAR,
				VK_ATTACHMENT_STORE_OP_STORE,
				VK_ATTACHMENT_LOAD_OP_DONT_CARE,
				VK_ATTACHMENT_STORE_OP_DONT_CARE,
				VK_IMAGE_LAYOUT_UNDEFINED,
				VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
			},
			{
				0,
				ColorFormat,
				VK_SAMPLE_COUNT_1_BIT,
				VK_ATTACHMENT_LOAD_OP_DONT_CARE,
				VK_ATTACHMENT_STORE_OP_STORE,
				VK_ATTACHMENT_LOAD_OP_DONT_CARE,
				VK_ATTACHMENT_STORE_OP_DONT_CARE,
				VK_IMAGE_LAYOUT_UNDEFINED,
				VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
			},
		};

		const std::vector<VkAttachmentReference> InputAttachmentReferences_Pass0 = {};
		const std::vector<VkAttachmentReference> ColorAttachmentReferences_Pass0 = { { 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL }, };
		const VkAttachmentReference DepthStencilAttachmentReference_Pass0 = { 1, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL };

		const std::vector<VkAttachmentReference> InputAttachmentReferences_Pass1 = { { 0, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL }, };
		const std::vector<VkAttachmentReference> ColorAttachmentReferences_Pass1 = { { 2, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL }, };

		const std::vector<uint32_t> PreserveAttachments = {};
		const std::vector<VkSubpassDescription> SubpassDescriptions = {
			{
				0,
				VK_PIPELINE_BIND_POINT_GRAPHICS,
				static_cast<uint32_t>(InputAttachmentReferences_Pass0.size()), InputAttachmentReferences_Pass0.data(),
				static_cast<uint32_t>(ColorAttachmentReferences_Pass0.size()), ColorAttachmentReferences_Pass0.data(), nullptr,
				&DepthStencilAttachmentReference_Pass0,
				static_cast<uint32_t>(PreserveAttachments.size()), PreserveAttachments.data()
			},
			{
				0,
				VK_PIPELINE_BIND_POINT_GRAPHICS,
				static_cast<uint32_t>(InputAttachmentReferences_Pass1.size()), InputAttachmentReferences_Pass1.data(),
				static_cast<uint32_t>(ColorAttachmentReferences_Pass1.size()), ColorAttachmentReferences_Pass1.data(), nullptr,
				nullptr,
				static_cast<uint32_t>(PreserveAttachments.size()), PreserveAttachments.data()
			}
		};


		const std::vector<VkSubpassDependency> SubpassDependencies = {
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

		const VkRenderPassCreateInfo RenderPassCreateInfo = {
			VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
			nullptr,
			0,
			static_cast<uint32_t>(AttachmentDescriptions.size()), AttachmentDescriptions.data(),
			static_cast<uint32_t>(SubpassDescriptions.size()), SubpassDescriptions.data(),
			static_cast<uint32_t>(SubpassDependencies.size()), SubpassDependencies.data()
		};
		VERIFY_SUCCEEDED(vkCreateRenderPass(Device, &RenderPassCreateInfo, GetAllocationCallbacks(), RenderPass));
	}(&RenderPass, CF, DF);
}

void VKExt::CreateFramebuffer_Color()
{
	Framebuffers.resize(SwapchainImages.size());
	for (uint32_t i = 0; i < Framebuffers.size(); ++i) {
		[&](VkFramebuffer* Framebuffer, const VkImageView ColorView, const VkRenderPass RenderPass, const uint32_t Width, const uint32_t Height) {
			const std::vector<VkImageView> Attachments = {
				ColorView 
			};
			const VkFramebufferCreateInfo FramebufferCreateInfo = {
				VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
				nullptr,
				0,
				RenderPass, //!< ここで作成するフレームバッファは RenderPass と「コンパチ」な別のレンダーパスでも使用可能
				static_cast<uint32_t>(Attachments.size()), Attachments.data(),
				Width, Height,
				1
			};
			VERIFY_SUCCEEDED(vkCreateFramebuffer(Device, &FramebufferCreateInfo, GetAllocationCallbacks(), Framebuffer));
		}(&Framebuffers[i], SwapchainImageViews[i], RenderPass, SurfaceExtent2D.width, SurfaceExtent2D.height);
	}
}
void VKExt::CreateFramebuffer_ColorDepth()
{
	Framebuffers.resize(SwapchainImages.size());
	for (uint32_t i = 0; i < Framebuffers.size(); ++i) {
		[&](VkFramebuffer* Framebuffer, const VkImageView ColorView, const VkImageView DepthStencilView, const VkRenderPass RenderPass, const uint32_t Width, const uint32_t Height) {
			const std::vector<VkImageView> Attachments = {
				ColorView,
				DepthStencilView
			};
			const VkFramebufferCreateInfo FramebufferCreateInfo = {
				VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
				nullptr,
				0,
				RenderPass,
				static_cast<uint32_t>(Attachments.size()), Attachments.data(),
				Width, Height,
				1
			};
			VERIFY_SUCCEEDED(vkCreateFramebuffer(Device, &FramebufferCreateInfo, GetAllocationCallbacks(), Framebuffer));
		}(&Framebuffers[i], SwapchainImageViews[i], DepthStencilImageView, RenderPass, SurfaceExtent2D.width, SurfaceExtent2D.height);
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
