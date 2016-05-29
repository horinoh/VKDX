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
	//virtual void CreateShader() override;
	//virtual void CreatePipeline() override;

	//virtual void PopulateCommandBuffer(const VkCommandBuffer CommandBuffer) override;

private:
};
#pragma endregion