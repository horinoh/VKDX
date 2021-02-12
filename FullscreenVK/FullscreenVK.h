#pragma once

#include "resource.h"

#pragma region Code
#include "../VKExt.h"

class FullscreenVK : public VKExt
{
private:
	using Super = VKExt;
public:
	FullscreenVK() : Super() {}
	virtual ~FullscreenVK() {}

protected:
#ifdef USE_DRAW_INDIRECT
	virtual void CreateGeometry() override { 
		constexpr VkDrawIndirectCommand DIC = { .vertexCount = 4, .instanceCount = 1, .firstVertex = 0, .firstInstance = 0 };
		IndirectBuffers.emplace_back().Create(Device, GetCurrentPhysicalDeviceMemoryProperties(), DIC, CommandBuffers[0], GraphicsQueue);
	}
#endif

	virtual void CreateShaderModule() override {
#ifdef USE_DISTANCE_FUNCTION
		const auto ShaderPath = GetBasePath();
		ShaderModules.emplace_back(VK::CreateShaderModule(data(ShaderPath + TEXT(".vert.spv"))));
		ShaderModules.emplace_back(VK::CreateShaderModule(data(ShaderPath + TEXT("_df.frag.spv")))); 
#else
		CreateShaderModle_VsFs();
#endif
	}
	virtual void CreatePipeline() override { CreatePipeline_VsFs(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP, 0, VK_FALSE); }
	virtual void CreateRenderPass() { VK::CreateRenderPass(VK_ATTACHMENT_LOAD_OP_DONT_CARE, false); }
	
	virtual void PopulateCommandBuffer(const size_t i) override;
};
#pragma endregion