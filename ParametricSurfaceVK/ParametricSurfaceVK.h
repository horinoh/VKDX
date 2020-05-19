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

#ifndef USE_SECONDARY_COMMAND_BUFFER
	//!< デフォルトはセカンダリを作成する実装なのでオーバーライドする
	virtual void CreateCommandPool() override {
		const VkCommandPoolCreateInfo CPCI = {
			VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
			nullptr,
			VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
			GraphicsQueueFamilyIndex
		};
		CommandPools.resize(1);
		VERIFY_SUCCEEDED(vkCreateCommandPool(Device, &CPCI, GetAllocationCallbacks(), &CommandPools[0]));
	}
	virtual void AllocateCommandBuffer() override {
		assert(!CommandPools.empty() && "");
		const auto PrevCount = CommandBuffers.size();
		CommandBuffers.resize(PrevCount + SwapchainImages.size());
		const VkCommandBufferAllocateInfo CBAI = {
			VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
			nullptr,
			CommandPools[0],
			VK_COMMAND_BUFFER_LEVEL_PRIMARY,
			static_cast<uint32_t>(SwapchainImages.size())
		};
		VERIFY_SUCCEEDED(vkAllocateCommandBuffers(Device, &CBAI, &CommandBuffers[PrevCount]));
	}
#endif
	virtual void CreateIndirectBuffer() override { CreateIndirectBuffer_DrawIndexed(1, 1); } //!< 最低でもインデックス数1が必要 (At least index count must be 1)
	virtual void CreateShaderModules() override { CreateShaderModle_VsFsTesTcsGs(); }
	virtual void CreatePipelines() override { 
#ifdef USE_SPECIALIZATION_INFO
		//!< パイプライン作成時に上書きするシェーダ内定数値の設定
		struct TessLevel {
			float Outer;
			float Inner;
		};
		const TessLevel TL = { 30.0f, 30.0f };
		const std::vector<VkSpecializationMapEntry> SMEs = {
			{ 0, offsetof(TessLevel, Outer), sizeof(TL.Outer) },
			{ 1, offsetof(TessLevel, Inner), sizeof(TL.Outer) },
		};
		//!< VkPipelineShaderStageCreateInfo.pSpecializationInfoへ指定する
		const VkSpecializationInfo SI = {
			static_cast<uint32_t>(SMEs.size()), SMEs.data(),
			sizeof(TL), &TL
		};

		Pipelines.resize(1);
		const VkPipelineDepthStencilStateCreateInfo PDSSCI = {
			VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
			nullptr,
			0,
			VK_FALSE, VK_FALSE, VK_COMPARE_OP_LESS_OR_EQUAL,
			VK_FALSE,
			VK_FALSE, { VK_STENCIL_OP_KEEP, VK_STENCIL_OP_KEEP, VK_STENCIL_OP_KEEP, VK_COMPARE_OP_NEVER, 0, 0, 0 }, { VK_STENCIL_OP_KEEP, VK_STENCIL_OP_KEEP, VK_STENCIL_OP_KEEP, VK_COMPARE_OP_ALWAYS, 0, 0, 0 },
			0.0f, 1.0f
		};
		std::vector<std::thread> Threads;
		auto& PL = Pipelines[0];
		const auto RP = RenderPasses[0];
		const auto PLL = PipelineLayouts[0];
		const std::array<VkPipelineShaderStageCreateInfo, 5> PSSCIs = {
			{ VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, nullptr, 0, VK_SHADER_STAGE_VERTEX_BIT, ShaderModules[0], "main", nullptr },
			{ VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, nullptr, 0, VK_SHADER_STAGE_FRAGMENT_BIT, ShaderModules[1], "main", nullptr },
			{ VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, nullptr, 0, VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT, ShaderModules[2], "main", nullptr },
			{ VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, nullptr, 0, VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT, ShaderModules[3], "main", &SI/*VkSpecializationInfoはここに指定する*/ }, 
			{ VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, nullptr, 0, VK_SHADER_STAGE_GEOMETRY_BIT, ShaderModules[4], "main", nullptr },
		};
		const std::vector<VkVertexInputBindingDescription> VIBDs = {};
		const std::vector<VkVertexInputAttributeDescription> VIADs = {};
		const std::vector< VkPipelineColorBlendAttachmentState> PCBASs = {
			{
				VK_FALSE, VK_BLEND_FACTOR_ONE,
				VK_BLEND_FACTOR_ONE, VK_BLEND_OP_ADD,
				VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ONE, VK_BLEND_OP_ADD,
				VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
			},
		};
#ifdef USE_PIPELINE_SERIALIZE
		PipelineCacheSerializer PCS(Device, GetBasePath() + TEXT(".pco"), 1);
		Threads.push_back(std::thread::thread(VK::CreatePipeline, std::ref(PL), Device, PLL, RP, VK_PRIMITIVE_TOPOLOGY_PATCH_LIST, 1, PDSSCI, &PSSCIs[0], &PSSCIs[1], &PSSCIs[2], &PSSCIs[3], &PSSCIs[4], VIBDs, VIADs, PCBASs, PCS.GetPipelineCache(0)));
#else
		Threads.push_back(std::thread::thread(VK::CreatePipeline, std::ref(PL), Device, PLL, RP, VK_PRIMITIVE_TOPOLOGY_PATCH_LIST, 1, PDSSCI, &PSSCIs[0], &PSSCIs[1], &PSSCIs[2], &PSSCIs[3], &PSSCIs[4], VIBDs, VIADs, PCBASs));
#endif
		for (auto& i : Threads) { i.join(); }
#else
		CreatePipeline_VsFsTesTcsGs(VK_PRIMITIVE_TOPOLOGY_PATCH_LIST, 1 , VK_FALSE);
#endif
	}
	virtual void PopulateCommandBuffer(const size_t i) override;
};
#pragma endregion