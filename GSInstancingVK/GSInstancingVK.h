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
		constexpr VkDrawIndexedIndirectCommand DIIC = { .indexCount = 1, .instanceCount = 1, .firstIndex = 0, .vertexOffset = 0, .firstInstance = 0 };
		IndirectBuffers.emplace_back().Create(Device, GetCurrentPhysicalDeviceMemoryProperties(), DIIC, CommandBuffers[0], GraphicsQueue);
	}
	virtual void CreateShaderModule() override { CreateShaderModle_VsFsTesTcsGs(); }
	virtual void CreatePipeline() override { CreatePipeline_VsFsTesTcsGs(VK_PRIMITIVE_TOPOLOGY_PATCH_LIST, 1, VK_FALSE); }

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