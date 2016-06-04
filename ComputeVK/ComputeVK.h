#pragma once

#include "resource.h"

#pragma region Code
#include "../VK.h"

class ComputeVK : public VK
{
private:
	using Super = VK;
public:
	ComputeVK() : VK() {}
	virtual ~ComputeVK() {}

protected:
	virtual void CreateShader() override { CreateShader_Cs(); }
	virtual void CreatePipeline() override { Super::CreateComputePipeline(); }

	//virtual void PopulateCommandBuffer(const VkCommandBuffer CommandBuffer) override;

	virtual void Draw() override {}

private:

};
#pragma endregion