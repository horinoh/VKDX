#pragma once

#include "resource.h"

#pragma region Code
#include "../VK.h"

class ComputeVK : public VKExt
{
private:
	using Super = VKExt;
public:
	ComputeVK() : VKExt() {}
	virtual ~ComputeVK() {}

protected:
	virtual void CreateShader() override { CreateShader_Cs(); }
	virtual void CreatePipeline() override { Super::CreateComputePipeline(); }

	//virtual void PopulateCommandBuffer(const VkCommandBuffer CommandBuffer) override;

	virtual void Draw() override {}

private:

};
#pragma endregion