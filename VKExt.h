#pragma once

#include "VK.h"

class VKExt : public VK
{
private:
	using Super = VK;
public:
	using Vertex_Position = struct Vertex_Position { glm::vec3 Position; };
	using Vertex_PositionColor = struct Vertex_PositionColor { glm::vec3 Position; glm::vec4 Color; };

	virtual void CreateDescriptorSetLayout_1UniformBuffer(const VkShaderStageFlags ShaderStageFlags = VK_SHADER_STAGE_ALL_GRAPHICS);
	virtual void CreateDescritporPool_1UniformBuffer();

	template<typename T>
	void CreateVertexInput(std::vector<VkVertexInputBindingDescription>& VertexInputBindingDescriptions, std::vector<VkVertexInputAttributeDescription>& VertexInputAttributeDescriptions, const uint32_t Binding = 0) {}
	template<>
	void CreateVertexInput<Vertex_Position>(std::vector<VkVertexInputBindingDescription>& VertexInputBindingDescriptions, std::vector<VkVertexInputAttributeDescription>& VertexInputAttributeDescriptions, const uint32_t Binding) {
		VertexInputBindingDescriptions = {
			{ Binding, sizeof(Vertex_Position), VK_VERTEX_INPUT_RATE_VERTEX }
		};
		VertexInputAttributeDescriptions = {
			{ 0, Binding, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex_Position, Position) },
		};
	}
	template<>
	void CreateVertexInput<Vertex_PositionColor>(std::vector<VkVertexInputBindingDescription>& VertexInputBindingDescriptions, std::vector<VkVertexInputAttributeDescription>& VertexInputAttributeDescriptions, const uint32_t Binding) {
		VertexInputBindingDescriptions = {
			{ Binding, sizeof(Vertex_PositionColor), VK_VERTEX_INPUT_RATE_VERTEX }
		};
		VertexInputAttributeDescriptions = {
			{ 0, Binding, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex_PositionColor, Position) },
			{ 1, Binding, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex_PositionColor, Color) }
		};
	}
	virtual void CreateVertexInput_Position();
	virtual void CreateVertexInput_PositionColor();

	template<typename T>
	void CreateUniformBuffer(const VkPhysicalDeviceMemoryProperties& PhysicalDeviceMemoryProperties, const T& Type) {
		//!< #TODO
		const auto Size = sizeof(T);
		CreateHostVisibleBuffer(PhysicalDeviceMemoryProperties, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, &UniformBuffer, &UniformDeviceMemory, Size, &Type);
		//CreateUniformBufferDescriptorBufferInfo(static_cast<UINT>(Size));
#ifdef _DEBUG
		std::cout << "CreateUniformBuffer" << COUT_OK << std::endl << std::endl;
#endif
	}

	virtual void CreateGraphicsPipeline_VsPs();
	virtual void CreateGraphicsPipeline_VsPsTesTcsGs();

	virtual void CreateRenderPass() { CreateRenderPass_Color(); }
	virtual void CreateRenderPass_Color();
	virtual void CreateRenderPass_ColorDepth();

	virtual void CreateFramebuffer() override { CreateFramebuffer_Color(); }
	virtual void CreateFramebuffer_Color();
	virtual void CreateFramebuffer_ColorDepth();

};