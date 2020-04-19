#pragma once

#include "resource.h"

#pragma region Code
#include "../VKExt.h"

class RenderTargetVK : public VKExt
{
private:
	using Super = VKExt;
public:
	RenderTargetVK() : Super() {}
	virtual ~RenderTargetVK() {}

protected:
	virtual void OverridePhysicalDeviceFeatures(VkPhysicalDeviceFeatures& PDF) const { assert(PDF.tessellationShader && "tessellationShader not enabled"); Super::OverridePhysicalDeviceFeatures(PDF); }

	virtual void CreateIndirectBuffer() override { CreateIndirectBuffer_DrawIndexed(1, 1); } //!< メッシュ描画用
//	virtual void CreateIndirectBuffer() override { CreateIndirectBuffer_Draw(4, 1); } //!< フルスクリーン描画用 #VK_TODO
	virtual void CreateDescriptorSetLayout() override {
		DescriptorSetLayouts.resize(1);
		assert(!Samplers.empty() && "");
		const std::array<VkSampler, 1> ISs = {
			Samplers[0]
		};
		VKExt::CreateDescriptorSetLayout(DescriptorSetLayouts[0], 0, {
			{ 0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, static_cast<uint32_t>(ISs.size()), VK_SHADER_STAGE_FRAGMENT_BIT, ISs.data() }
		});
	}
	virtual void CreatePipelineLayout() override {
		assert(!DescriptorSetLayouts.empty() && "");
		PipelineLayouts.resize(2);
		VKExt::CreatePipelineLayout(PipelineLayouts[0], {}, {});
		VKExt::CreatePipelineLayout(PipelineLayouts[1], { DescriptorSetLayouts[0] }, {});
	}

#pragma region DESCRIPTOR
	virtual void CreateDescriptorUpdateTemplate() override {
		const std::array<VkDescriptorUpdateTemplateEntry, 1> DUTEs = {
			{
				0, 0,
				_countof(DescriptorUpdateInfo::DescriptorImageInfos), VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
				offsetof(DescriptorUpdateInfo, DescriptorImageInfos), sizeof(DescriptorUpdateInfo)
			}
		};
		assert(!DescriptorSetLayouts.empty() && "");
		const VkDescriptorUpdateTemplateCreateInfo DUTCI = {
			VK_STRUCTURE_TYPE_DESCRIPTOR_UPDATE_TEMPLATE_CREATE_INFO,
			nullptr,
			0,
			static_cast<uint32_t>(DUTEs.size()), DUTEs.data(),
			VK_DESCRIPTOR_UPDATE_TEMPLATE_TYPE_DESCRIPTOR_SET,
			DescriptorSetLayouts[0],
			VK_PIPELINE_BIND_POINT_GRAPHICS, VK_NULL_HANDLE, 0
		};
		DescriptorUpdateTemplates.resize(1);
		VERIFY_SUCCEEDED(vkCreateDescriptorUpdateTemplate(Device, &DUTCI, GetAllocationCallbacks(), &DescriptorUpdateTemplates[0]));
	}
	virtual void UpdateDescriptorSet() override {
		Super::UpdateDescriptorSet();

		assert(!Samplers.empty() && "");
		assert(VK_NULL_HANDLE != ImageView && "");
		const DescriptorUpdateInfo DUI = {
			{ VK_NULL_HANDLE, ImageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL },
		};
		assert(!DescriptorSets.empty() && "");
		assert(!DescriptorUpdateTemplates.empty() && "");
		vkUpdateDescriptorSetWithTemplate(Device, DescriptorSets[0], DescriptorUpdateTemplates[0], &DUI);
	}
#pragma endregion //!< DESCRIPTOR
	virtual void CreateTexture() override { 
		CreateRenderTexture(&Image, &ImageDeviceMemory, &ImageView); 
	}
	virtual void CreateImmutableSampler() override {
		Samplers.resize(1);
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
		VERIFY_SUCCEEDED(vkCreateSampler(Device, &SCI, GetAllocationCallbacks(), &Samplers[0]));
	}
	virtual void CreateShaderModules() override {
		const auto ShaderPath = GetBasePath();
		ShaderModules.push_back(VKExt::CreateShaderModules((ShaderPath + TEXT(".vert.spv")).data()));
		ShaderModules.push_back(VKExt::CreateShaderModules((ShaderPath + TEXT(".frag.spv")).data()));
		ShaderModules.push_back(VKExt::CreateShaderModules((ShaderPath + TEXT(".tese.spv")).data()));
		ShaderModules.push_back(VKExt::CreateShaderModules((ShaderPath + TEXT(".tesc.spv")).data()));
		ShaderModules.push_back(VKExt::CreateShaderModules((ShaderPath + TEXT(".geom.spv")).data()));

		ShaderModules.push_back(VKExt::CreateShaderModules((ShaderPath + TEXT("_1") + TEXT(".vert.spv")).data())); //!<
		ShaderModules.push_back(VKExt::CreateShaderModules((ShaderPath + TEXT("_1") + TEXT(".frag.spv")).data())); //!<
	}
	virtual void CreatePipelines() override { 
		Pipelines.resize(2); //!<
		std::vector<std::thread> Threads;
		const VkPipelineDepthStencilStateCreateInfo PDSSCI = {
			VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
			nullptr,
			0,
			VK_FALSE,
			VK_FALSE,
			VK_COMPARE_OP_LESS_OR_EQUAL,
			VK_FALSE,
			VK_FALSE,
			{ VK_STENCIL_OP_KEEP, VK_STENCIL_OP_KEEP, VK_STENCIL_OP_KEEP, VK_COMPARE_OP_NEVER, 0, 0, 0 },
			{ VK_STENCIL_OP_KEEP, VK_STENCIL_OP_KEEP, VK_STENCIL_OP_KEEP, VK_COMPARE_OP_ALWAYS, 0, 0, 0 },
			0.0f, 1.0f
		};
		const std::vector<VkPipelineShaderStageCreateInfo> PSSCIs = {
			{ VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, nullptr, 0, VK_SHADER_STAGE_VERTEX_BIT, ShaderModules[0], "main", nullptr },
			{ VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, nullptr, 0, VK_SHADER_STAGE_FRAGMENT_BIT, ShaderModules[1], "main", nullptr },
			{ VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, nullptr, 0, VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT, ShaderModules[2], "main", nullptr },
			{ VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, nullptr, 0, VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT, ShaderModules[3], "main", nullptr },
			{ VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, nullptr, 0, VK_SHADER_STAGE_GEOMETRY_BIT, ShaderModules[4], "main", nullptr },

			{ VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, nullptr, 0, VK_SHADER_STAGE_VERTEX_BIT, ShaderModules[5], "main", nullptr },
			{ VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, nullptr, 0, VK_SHADER_STAGE_FRAGMENT_BIT, ShaderModules[6], "main", nullptr },
		};
		const std::vector<VkVertexInputBindingDescription> VIBDs = {};
		const std::vector<VkVertexInputAttributeDescription> VIADs = {};
#ifdef USE_PIPELINE_SERIALIZE
		PipelineCacheSerializer PCS(Device, GetBasePath() + TEXT(".pco"), 2);
		Threads.push_back(std::thread::thread(VK::CreatePipeline, std::ref(Pipelines[0]), Device, PipelineLayouts[0], RenderPasses[0], VK_PRIMITIVE_TOPOLOGY_PATCH_LIST, 1, PDSSCI ,&PSSCIs[0], &PSSCIs[1], &PSSCIs[2], &PSSCIs[3], &PSSCIs[4], VIBDs, VIADs, PCS.GetPipelineCache(0)));
		Threads.push_back(std::thread::thread(VK::CreatePipeline, std::ref(Pipelines[1]), Device, PipelineLayouts[1], RenderPasses[0], VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP, 0, PDSSCI, &PSSCIs[5], &PSSCIs[6], nullptr, nullptr, nullptr, VIBDs, VIADs, PCS.GetPipelineCache(1)));
#else
		Threads.push_back(std::thread::thread(VK::CreatePipeline, std::ref(Pipelines[0]), Device, PipelineLayouts[0], RenderPasses[0], VK_PRIMITIVE_TOPOLOGY_PATCH_LIST, 1, PDSSCI, &PSSCIs[0], &PSSCIs[1], &PSSCIs[2], &PSSCIs[3], &PSSCIs[4], VIBDs, VIADs, nullptr));
		Threads.push_back(std::thread::thread(VK::CreatePipeline, std::ref(Pipelines[1]), Device, PipelineLayouts[1], RenderPasses[0], VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP, 0, PDSSCI, &PSSCIs[5], &PSSCIs[6], nullptr, nullptr, nullptr, VIBDs, VIADs, nullptr));
#endif
		for (auto& i : Threads) { i.join(); }
	}
	virtual void PopulateCommandBuffer(const size_t i) override;

private:
		struct DescriptorUpdateInfo
		{
			VkDescriptorImageInfo DescriptorImageInfos[1];
		};
};
#pragma endregion