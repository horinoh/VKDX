#pragma once

#include "resource.h"

#pragma region Code
#include "../VK.h"

class TriangleVK : public VK
{
private:
	using Super = VK;
public:
	TriangleVK() : VK() {}
	virtual ~TriangleVK() {}

protected:
	virtual void CreateShader() override;
	virtual void CreateVertexInput() override;
	virtual void CreateVertexBuffer(const VkCommandPool CommandPool, const VkPhysicalDeviceMemoryProperties& PhysicalDeviceMemoryProperties) override;
	virtual void CreateIndexBuffer(const VkCommandPool CommandPool, const VkPhysicalDeviceMemoryProperties& PhysicalDeviceMemoryProperties) override;
	virtual void CreatePipeline() override;

	virtual void PopulateCommandBuffer(const VkCommandBuffer CommandBuffer) override;

private:
	using Vertex = struct Vertex { glm::vec3 Positon; glm::vec4 Color; };
};
#pragma endregion