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
	virtual void CreateIndirectBuffer() override { CreateIndirectBuffer_Draw(4, 1); }
#endif
	virtual void CreateShaderModules() override {
#ifdef USE_DISTANCE_FUNCTION
		const auto ShaderPath = GetBasePath();
		ShaderModules.push_back(VKExt::CreateShaderModules((ShaderPath + TEXT(".vert.spv")).data()));
		ShaderModules.push_back(VKExt::CreateShaderModules((ShaderPath + TEXT("_df.frag.spv")).data())); 
#else
		CreateShaderModle_VsFs();
#endif
	}
	virtual void CreatePipelines() override { CreatePipeline_VsFs(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP, 0, VK_FALSE); }
	virtual void CreateRenderPass() { RenderPasses.resize(1); CreateRenderPass_Default(RenderPasses[0], ColorFormat, false); }
	virtual void PopulateCommandBuffer(const size_t i) override;
};
#pragma endregion