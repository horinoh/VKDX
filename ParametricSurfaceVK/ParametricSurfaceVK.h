#pragma once

#include "resource.h"

#pragma region Code
#include "../VKExt.h"

//!< パラメトリックサーフェス(ParametricSurface) http://www.3d-meier.de/tut3/Seite0.html

class ParametricSurfaceVK : public VKExt
{
private:
	using Super = VKExt;
public:
	ParametricSurfaceVK() : Super() {}
	virtual ~ParametricSurfaceVK() {}

protected:
	virtual void OverridePhysicalDeviceFeatures(VkPhysicalDeviceFeatures& PDF) const { assert(PDF.tessellationShader && "tessellationShader not enabled"); Super::OverridePhysicalDeviceFeatures(PDF); }

#ifdef USE_NO_SECONDARY_COMMAND_BUFFER
	//!< デフォルト実装はセカンダリを作成する実装なので、オーバーライドして作成しないようにしている
	virtual void CreateCommandPool() override {
		const VkCommandPoolCreateInfo CPCI = {
			.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
			.pNext = nullptr,
			.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
			.queueFamilyIndex = GraphicsQueueFamilyIndex
		};
		CommandPools.emplace_back(VkCommandPool());
		VERIFY_SUCCEEDED(vkCreateCommandPool(Device, &CPCI, GetAllocationCallbacks(), &CommandPools.back()));
	}
	virtual void AllocateCommandBuffer() override {
		const auto SCCount = static_cast<uint32_t>(SwapchainImages.size());
		assert(!empty(CommandPools) && "");
		const auto PrevCount = CommandBuffers.size();
		CommandBuffers.resize(PrevCount + SCCount);
		const VkCommandBufferAllocateInfo CBAI = {
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
			.pNext = nullptr,
			.commandPool = CommandPools[0],
			.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
			.commandBufferCount = SCCount
		};
		VERIFY_SUCCEEDED(vkAllocateCommandBuffers(Device, &CBAI, &CommandBuffers[PrevCount]));
	}
#endif

	virtual void CreateIndirectBuffer() override { CreateIndirectBuffer_DrawIndexed(1, 1); } //!< 最低でもインデックス数1が必要 (At least index count must be 1)
	
	virtual void CreateShaderModules() override { CreateShaderModle_VsFsTesTcsGs(); }
	virtual void CreatePipelines() override { 
#ifdef USE_SPECIALIZATION_INFO
		//!< パイプライン作成時に上書きするシェーダ内定数値の設定
		struct TessLevel { float Outer; float Inner; };
		const auto TL = TessLevel({ .Outer = 30.0f, .Inner = 30.0f });
		const std::vector SMEs = {
			VkSpecializationMapEntry({ .constantID = 0, .offset = offsetof(TessLevel, Outer), .size = sizeof(TL.Outer) }),
			VkSpecializationMapEntry({ .constantID = 1, .offset = offsetof(TessLevel, Inner), .size = sizeof(TL.Outer) }),
		};
		//!< VkPipelineShaderStageCreateInfo.pSpecializationInfoへ指定する
		const VkSpecializationInfo SI = {
			.mapEntryCount = static_cast<uint32_t>(size(SMEs)), .pMapEntries = data(SMEs),
			.dataSize = sizeof(TL), .pData = &TL
		};

		Pipelines.emplace_back(VkPipeline());
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
			.depthTestEnable = VK_FALSE, .depthWriteEnable = VK_FALSE, .depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL,
			.depthBoundsTestEnable = VK_FALSE,
			.stencilTestEnable = VK_FALSE, 
			.front = { .failOp = VK_STENCIL_OP_KEEP, .passOp = VK_STENCIL_OP_KEEP, .depthFailOp = VK_STENCIL_OP_KEEP, .compareOp = VK_COMPARE_OP_NEVER, .compareMask = 0, .writeMask = 0, .reference = 0 }, 
			.back = { .failOp = VK_STENCIL_OP_KEEP, .passOp = VK_STENCIL_OP_KEEP, .depthFailOp = VK_STENCIL_OP_KEEP, .compareOp = VK_COMPARE_OP_ALWAYS, .compareMask = 0, .writeMask = 0, .reference = 0 },
			.minDepthBounds = 0.0f, .maxDepthBounds = 1.0f
		};
		std::vector<std::thread> Threads;
		auto& PL = Pipelines.back();
		const auto RP = RenderPasses[0];
		const auto PLL = PipelineLayouts[0];
		const std::array PSSCIs = { 
			VkPipelineShaderStageCreateInfo({ .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, .pNext = nullptr, .flags = 0, .stage = VK_SHADER_STAGE_VERTEX_BIT, .module = ShaderModules[0], .pName = "main", .pSpecializationInfo = nullptr }),
			VkPipelineShaderStageCreateInfo({ .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, .pNext = nullptr, .flags = 0, .stage = VK_SHADER_STAGE_FRAGMENT_BIT, .module =ShaderModules[1], .pName = "main", .pSpecializationInfo = nullptr }),
			VkPipelineShaderStageCreateInfo({ .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, .pNext = nullptr, .flags = 0, .stage = VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT, .module = ShaderModules[2], .pName = "main", .pSpecializationInfo = nullptr }),
			VkPipelineShaderStageCreateInfo({ .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, .pNext = nullptr, .flags = 0, .stage = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT, .module = ShaderModules[3], .pName = "main", .pSpecializationInfo  = &SI }),
			VkPipelineShaderStageCreateInfo({ .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, .pNext = nullptr, .flags = 0, .stage = VK_SHADER_STAGE_GEOMETRY_BIT, .module = ShaderModules[4], .pName = "main", .pSpecializationInfo = nullptr }),
		};
		const std::vector<VkVertexInputBindingDescription> VIBDs = {};
		const std::vector<VkVertexInputAttributeDescription> VIADs = {};
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
		Threads.push_back(std::thread::thread(VK::CreatePipeline, std::ref(PL), Device, PLL, RP, VK_PRIMITIVE_TOPOLOGY_PATCH_LIST, 1, PRSCI, PDSSCI, &PSSCIs[0], &PSSCIs[1], &PSSCIs[2], &PSSCIs[3], &PSSCIs[4], VIBDs, VIADs, PCBASs, PCS.GetPipelineCache(0)));
#else
		Threads.push_back(std::thread::thread(VK::CreatePipeline, std::ref(PL), Device, PLL, RP, VK_PRIMITIVE_TOPOLOGY_PATCH_LIST, 1, PRSCI, PDSSCI, &PSSCIs[0], &PSSCIs[1], &PSSCIs[2], &PSSCIs[3], &PSSCIs[4], VIBDs, VIADs, PCBASs));
#endif
		for (auto& i : Threads) { i.join(); }
#else
		CreatePipeline_VsFsTesTcsGs(VK_PRIMITIVE_TOPOLOGY_PATCH_LIST, 1 , VK_FALSE);
#endif
	}

	virtual void PopulateCommandBuffer(const size_t i) override;
};
#pragma endregion