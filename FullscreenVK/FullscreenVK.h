#pragma once

#include "resource.h"

#pragma region Code
#include "../VKExt.h"

#define USE_DRAW_INDIRECT

class FullscreenVK : public VKExt
{
private:
	using Super = VKExt;
public:
	FullscreenVK() : Super() {}
	virtual ~FullscreenVK() {}

protected:
#ifdef USE_DRAW_INDIRECT
	virtual void CreateIndirectBuffer() override { CreateIndirectBuffer_Draw(4); }
#endif
	virtual void CreateShaderModule() override { CreateShaderModle_VsFs(); }
	virtual void CreatePipeline() override { CreatePipeline_VsFs(); }
	virtual void PopulateCommandBuffer(const size_t i) override;
};
#pragma endregion