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

#ifdef USE_SECONDARY_COMMAND_BUFFER
	virtual void AllocateSecondaryCommandBuffer() override { AddSecondaryCommandBuffer(); }
#endif
	virtual void CreateIndirectBuffer() override { CreateIndirectBuffer_DrawIndexed(1, 1); }
//	virtual void CreateIndirectBuffer() override { CreateIndirectBuffer_Draw(4, 1); }
	virtual void CreateDescriptorSetLayout() override {
		DescriptorSetLayouts.resize(1);
		assert(!Samplers.empty() && "");
		const std::array<VkSampler, 1> ISs = {
			Samplers[0]
		};
		VKExt::CreateDescriptorSetLayout(DescriptorSetLayouts[0], {
			{ 0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, static_cast<uint32_t>(ISs.size()), VK_SHADER_STAGE_FRAGMENT_BIT, ISs.data() }
		});
	}
	virtual void CreatePipelineLayout() override {
		assert(!DescriptorSetLayouts.empty() && "");
		PipelineLayouts.resize(1);
		VKExt::CreatePipelineLayout(PipelineLayouts[0], {
				DescriptorSetLayouts[0]
			}, {});
	}
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
		assert(!Samplers.empty() && "");
		assert(VK_NULL_HANDLE != ImageView && "");
		const DescriptorUpdateInfo DUI = {
			{ VK_NULL_HANDLE, ImageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL },
		};
		assert(!DescriptorSets.empty() && "");
		assert(!DescriptorUpdateTemplates.empty() && "");
		vkUpdateDescriptorSetWithTemplate(Device, DescriptorSets[0], DescriptorUpdateTemplates[0], &DUI);
	}
	virtual void CreateSampler() override {
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
	virtual void CreateShaderModule() override {
		const auto ShaderPath = GetBasePath();
		ShaderModules.push_back(VKExt::CreateShaderModule((ShaderPath + TEXT(".vert.spv")).data()));
		ShaderModules.push_back(VKExt::CreateShaderModule((ShaderPath + TEXT(".frag.spv")).data()));
		ShaderModules.push_back(VKExt::CreateShaderModule((ShaderPath + TEXT(".tese.spv")).data()));
		ShaderModules.push_back(VKExt::CreateShaderModule((ShaderPath + TEXT(".tesc.spv")).data()));
		ShaderModules.push_back(VKExt::CreateShaderModule((ShaderPath + TEXT(".geom.spv")).data()));

		ShaderModules.push_back(VKExt::CreateShaderModule((ShaderPath + TEXT("_1") + TEXT(".vert.spv")).data())); //!<
		ShaderModules.push_back(VKExt::CreateShaderModule((ShaderPath + TEXT("_1") + TEXT(".frag.spv")).data())); //!<
	}
	virtual void CreatePipeline() override { 
		Pipelines.resize(2); //!<
#ifdef USE_PIPELINE_SERIALIZE
		PipelineCacheSerializer PCS(Device, GetBasePath() + TEXT(".pco"), 2);
#endif
		std::vector<std::thread> Threads;
		{
			auto& PL = Pipelines[0];
			const auto RP = RenderPasses[0];
			const auto PLL = PipelineLayouts[0];
			Threads.push_back(std::thread::thread([&](VkPipeline& PL, const VkPipelineLayout PLL, const VkRenderPass RP,
				const VkShaderModule VS, const VkShaderModule FS, const VkShaderModule TES, const VkShaderModule TCS, const VkShaderModule GS)
				{
#ifdef USE_PIPELINE_SERIALIZE
					VK::CreatePipeline(PL, PLL, RP, VS, FS, TES, TCS, GS, {}, {}, VK_PRIMITIVE_TOPOLOGY_PATCH_LIST, 1, PCS.GetPipelineCache(0));				
#else
					VK::CreatePipeline(PL, PLL, RP, VS, FS, TES, TCS, GS, {}, {}, VK_PRIMITIVE_TOPOLOGY_PATCH_LIST, 1);
#endif
				},
				std::ref(PL), PLL, RP, ShaderModules[0], ShaderModules[1], ShaderModules[2], ShaderModules[3], ShaderModules[4]));
		}
		{
			auto& PL = Pipelines[1]; //!< 
			const auto RP = RenderPasses[0];
			const auto PLL = PipelineLayouts[0];
			Threads.push_back(std::thread::thread([&](VkPipeline& PL, const VkPipelineLayout PLL, const VkRenderPass RP,
				const VkShaderModule VS, const VkShaderModule FS, const VkShaderModule TES, const VkShaderModule TCS, const VkShaderModule GS)
				{
#ifdef USE_PIPELINE_SERIALIZE
					VK::CreatePipeline(PL, PLL, RP, VS, FS, TES, TCS, GS, {}, {}, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP, 0, PCS.GetPipelineCache(1)); //!< 
#else
					VK::CreatePipeline(PL, PLL, RP, VS, FS, TES, TCS, GS, {}, {}, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP);
#endif
				},
				std::ref(PL), PLL, RP, ShaderModules[5], ShaderModules[6], NullShaderModule, NullShaderModule, NullShaderModule)); //!<
		}
		for (auto& i : Threads) {
			i.join();
		}
	}
	virtual void PopulateCommandBuffer(const size_t i) override;

private:
		struct DescriptorUpdateInfo
		{
			VkDescriptorImageInfo DescriptorImageInfos[1];
		};
};
#pragma endregion