#pragma once

#include "VK.h"

class VKExt : public VK
{
private:
	using Super = VK;
public:
	virtual void CreateShader_VsPs();
	virtual void CreateShader_VsPsTesTcsGs();
	virtual void CreateShader_Cs();

	virtual void CreateDescriptorSetLayout_1UniformBuffer(const VkShaderStageFlags ShaderStageFlags = VK_SHADER_STAGE_ALL_GRAPHICS);
	virtual void CreateDescritporPool_1UniformBuffer();

	virtual void CreateVertexInput_Position();
	virtual void CreateVertexInput_PositionColor();

	virtual void CreateGraphicsPipeline_VsPs();
	virtual void CreateGraphicsPipeline_VsPsTesTcsGs();

	virtual void CreateRenderPass() { CreateRenderPass_Color(); }
	virtual void CreateRenderPass_Color();
	virtual void CreateRenderPass_ColorDepth();

	virtual void CreateFramebuffer() override { CreateFramebuffer_Color(); }
	virtual void CreateFramebuffer_Color();
	virtual void CreateFramebuffer_ColorDepth();

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

	virtual void Clear(const VkCommandBuffer CommandBuffer) override { Clear_Color(CommandBuffer); }
	virtual void Clear_Color(const VkCommandBuffer CommandBuffer);
	virtual void Clear_Depth(const VkCommandBuffer CommandBuffer);
};