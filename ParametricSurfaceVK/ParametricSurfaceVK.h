#pragma once

#include "resource.h"

#pragma region Code
#include "../VKExt.h"

class ParametricSurfaceVK : public VKExt
{
private:
	using Super = VKExt;
public:
	ParametricSurfaceVK() : Super() {}
	virtual ~ParametricSurfaceVK() {}

protected:
	virtual void OverridePhysicalDeviceFeatures(VkPhysicalDeviceFeatures& PhysicalDeviceFeatures) const { assert(PhysicalDeviceFeatures.tessellationShader && "tessellationShader not enabled"); }

	virtual void CreateIndirectBuffer() override { CreateIndirectBuffer_DrawIndexed(1); } //!< 最低でもインデックス数1が必要 At least index count must be 1
	virtual void CreatePipeline() override { CreatePipeline_VsFsTesTcsGs_Tesselation(); }
	virtual void PopulateCommandBuffer(const size_t i) override;
};
#pragma endregion