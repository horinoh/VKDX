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
	virtual void CreateVertexBuffer(const VkPhysicalDeviceMemoryProperties& PhysicalDeviceMemoryProperties) override;
	virtual void CreateIndexBuffer(const VkPhysicalDeviceMemoryProperties& PhysicalDeviceMemoryProperties) override;

private:
	using Vertex = std::tuple<glm::vec3, glm::vec4>;
};
#pragma endregion