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
	virtual void OverridePhysicalDeviceFeatures(VkPhysicalDeviceFeatures& PDF) const { assert(PDF.tessellationShader && "tessellationShader not enabled"); Super::OverridePhysicalDeviceFeatures(PDF); }

	virtual void CreateViewport(const float Width, const float Height, const float MinDepth = 0.0f, const float MaxDepth = 1.0f) override {
		const auto W = Width * 0.5f, H = Height * 0.5f;
		Viewports = {
	#ifdef USE_VIEWPORT_Y_UP
			{ 0.0f, H, W, -H, MinDepth, MaxDepth },
			{ W, H, W, -H, MinDepth, MaxDepth },
			{ 0.0f, Height, W, -H, MinDepth, MaxDepth },
			{ W, Height, W, -H, MinDepth, MaxDepth },
	#else
			{ 0.0f, 0.0f, W, H,MinDepth, MaxDepth },
			{ W, 0.0f, W, H, MinDepth, MaxDepth },
			{ 0.0f, H, W, H, MinDepth, MaxDepth },
			{ W, H, W, H, MinDepth, MaxDepth },
	#endif
		};
		//!< offset, extent‚ÅŽw’è (left, top, right, bottom‚ÅŽw’è‚ÌDX‚Æ‚ÍˆÙ‚È‚é‚Ì‚Å’ˆÓ)
		ScissorRects = {
			{ { 0, 0 }, { static_cast<uint32_t>(W), static_cast<uint32_t>(H) } },
			{ { static_cast<int32_t>(W), 0 }, { static_cast<uint32_t>(W), static_cast<uint32_t>(H) } },
			{ { 0, static_cast<int32_t>(H) }, { static_cast<uint32_t>(W), static_cast<uint32_t>(H) } },
			{ { static_cast<int32_t>(W), static_cast<int32_t>(H) }, { static_cast<uint32_t>(W), static_cast<uint32_t>(H) } },
		};
		LOG_OK();
	}
	virtual void CreateIndirectBuffer() override { CreateIndirectBuffer_DrawIndexed(1, 1); }
	virtual void CreateShaderModules() override { CreateShaderModle_VsFsTesTcsGs(); }
	virtual void CreatePipelines() override { CreatePipeline_VsFsTesTcsGs(VK_PRIMITIVE_TOPOLOGY_PATCH_LIST, 1, VK_FALSE); }
	virtual void PopulateCommandBuffer(const size_t i) override;
};
#pragma endregion