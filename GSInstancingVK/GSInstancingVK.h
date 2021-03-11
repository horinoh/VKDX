#pragma once

#include "resource.h"

#pragma region Code
#include "../VKExt.h"

class GSInstancingVK : public VKExt
{
private:
	using Super = VKExt;
public:
	GSInstancingVK() : Super() {}
	virtual ~GSInstancingVK() {}

protected:
	virtual void CreateGeometry() override { 
		const auto PDMP = GetCurrentPhysicalDeviceMemoryProperties();
		constexpr VkDrawIndexedIndirectCommand DIIC = { .indexCount = 1, .instanceCount = 1, .firstIndex = 0, .vertexOffset = 0, .firstInstance = 0 };
		IndirectBuffers.emplace_back().Create(Device, PDMP, DIIC);
		IndirectBuffers.back().SubmitCopyCommand(Device, PDMP, CommandBuffers[0], GraphicsQueue, sizeof(DIIC), &DIIC);
	}
	virtual void CreateRenderPass() override { VKExt::CreateRenderPass_Clear(); }
	virtual void CreatePipeline() override { 
		const auto ShaderPath = GetBasePath();
		const std::array SMs = {
			VK::CreateShaderModule(data(ShaderPath + TEXT(".vert.spv"))),
			VK::CreateShaderModule(data(ShaderPath + TEXT(".frag.spv"))),
			VK::CreateShaderModule(data(ShaderPath + TEXT(".tese.spv"))),
			VK::CreateShaderModule(data(ShaderPath + TEXT(".tesc.spv"))),
			VK::CreateShaderModule(data(ShaderPath + TEXT(".geom.spv"))),
		};
		const std::array PSSCIs = {
			VkPipelineShaderStageCreateInfo({.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, .pNext = nullptr, .flags = 0, .stage = VK_SHADER_STAGE_VERTEX_BIT, .module = SMs[0], .pName = "main", .pSpecializationInfo = nullptr }),
			VkPipelineShaderStageCreateInfo({.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, .pNext = nullptr, .flags = 0, .stage = VK_SHADER_STAGE_FRAGMENT_BIT, .module = SMs[1], .pName = "main", .pSpecializationInfo = nullptr }),
			VkPipelineShaderStageCreateInfo({.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, .pNext = nullptr, .flags = 0, .stage = VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT, .module = SMs[2], .pName = "main", .pSpecializationInfo = nullptr }),
			VkPipelineShaderStageCreateInfo({.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, .pNext = nullptr, .flags = 0, .stage = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT, .module = SMs[3], .pName = "main", .pSpecializationInfo = nullptr }),
			VkPipelineShaderStageCreateInfo({.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, .pNext = nullptr, .flags = 0, .stage = VK_SHADER_STAGE_GEOMETRY_BIT, .module = SMs[4], .pName = "main", .pSpecializationInfo = nullptr }),
		}; 
		CreatePipeline_VsFsTesTcsGs(VK_PRIMITIVE_TOPOLOGY_PATCH_LIST, 1, VK_FALSE, PSSCIs);
		for (auto i : SMs) { vkDestroyShaderModule(Device, i, GetAllocationCallbacks()); }
	}

	virtual void PopulateCommandBuffer(const size_t i) override;

	virtual void CreateViewport(const float Width, const float Height, const float MinDepth = 0.0f, const float MaxDepth = 1.0f) override {
		const auto W = Width * 0.5f, H = Height * 0.5f;
		Viewports = {
	#ifdef USE_VIEWPORT_Y_UP
			VkViewport({.x = 0.0f, .y = H, .width = W, .height = -H, .minDepth = MinDepth, .maxDepth = MaxDepth }),
			VkViewport({.x = W, .y = H, .width = W, .height = -H, .minDepth = MinDepth, .maxDepth = MaxDepth }),
			VkViewport({.x = 0.0f, .y = Height, .width = W, .height = -H, .minDepth = MinDepth, .maxDepth = MaxDepth }),
			VkViewport({.x = W, .y = Height, .width = W, .height = -H, .minDepth = MinDepth, .maxDepth = MaxDepth }),
	#else
			VkViewport({.x = 0.0f, .y = 0.0f, .width = W, .height = H, .minDeoth = MinDepth, .maxDepth = MaxDepth }),
			VkViewport({.x = W, .y = 0.0f, .width = W, .height = H, .minDepth = MinDepth, .maxDepth = MaxDepth }),
			VkViewport({.x = 0.0f, .y = H, .width = W, .height = H, .minDepth = MinDepth, .maxDepth = MaxDepth }),
			VkViewport({.x = W, .y = H, .width = W, .height = H, .minDepth = MinDepth, .maxDepth = MaxDepth }),
	#endif
		};
		//!< offset, extent‚ÅŽw’è (left, top, right, bottom‚ÅŽw’è‚ÌDX‚Æ‚ÍˆÙ‚È‚é‚Ì‚Å’ˆÓ)
		ScissorRects = {
			VkRect2D({ VkOffset2D({.x = 0, .y = 0 }), VkExtent2D({.width = static_cast<uint32_t>(W), .height = static_cast<uint32_t>(H) }) }),
			VkRect2D({ VkOffset2D({.x = static_cast<int32_t>(W), .y = 0 }), VkExtent2D({.width = static_cast<uint32_t>(W), .height = static_cast<uint32_t>(H) }) }),
			VkRect2D({ VkOffset2D({.x = 0, .y = static_cast<int32_t>(H) }), VkExtent2D({.width = static_cast<uint32_t>(W), .height = static_cast<uint32_t>(H) }) }),
			VkRect2D({ VkOffset2D({.x = static_cast<int32_t>(W), .y = static_cast<int32_t>(H) }), VkExtent2D({.width = static_cast<uint32_t>(W), .height = static_cast<uint32_t>(H) }) }),
		};
		LOG_OK();
	}
};
#pragma endregion