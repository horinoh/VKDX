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
#ifdef USE_SECONDARY_COMMAND_BUFFER
	virtual void AllocateSecondaryCommandBuffer() override { AddSecondaryCommandBuffer(); }
#endif
#ifdef USE_DRAW_INDIRECT
	virtual void CreateIndirectBuffer() override { CreateIndirectBuffer_Draw(4, 1); }
#endif
	virtual void CreateShaderModule() override { CreateShaderModle_VsFs(); }
	virtual void CreatePipeline() override { CreatePipeline_VsFs(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP); }
	virtual void PopulateCommandBuffer(const size_t i) override;
};
#pragma endregion