#pragma once

#include "resource.h"

#pragma region Code
#include "../VKExt.h"

class FlatVK : public VKExt
{
private:
	using Super = VKExt;
public:
	FlatVK() : Super() {}
	virtual ~FlatVK() {}

protected:
	virtual void OverridePhysicalDeviceFeatures(VkPhysicalDeviceFeatures& PDF) const { assert(PDF.tessellationShader && "tessellationShader not enabled"); Super::OverridePhysicalDeviceFeatures(PDF); }
	virtual void CreateIndirectBuffer() override { CreateIndirectBuffer_DrawIndexed(1); }
	virtual void CreatePipeline() override { CreatePipeline_VsFsTesTcsGs_Tesselation(); }
	virtual void PopulateCommandBuffer(const size_t i) override;
};
#pragma endregion