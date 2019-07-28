#pragma once

#include "resource.h"

#pragma region Code
#include "../VKExt.h"

class ToonVK : public VKExt
{
private:
	using Super = VKExt;
public:
	ToonVK() : Super() {}
	virtual ~ToonVK() {}

protected:
	virtual void OverridePhysicalDeviceFeatures(VkPhysicalDeviceFeatures& PDF) const { assert(PDF.tessellationShader && "tessellationShader not enabled"); Super::OverridePhysicalDeviceFeatures(PDF); }

	virtual void CreateIndirectBuffer() override { CreateIndirectBuffer_DrawIndexed(1); }
	virtual void CreatePipelineLayout() override { 
		DescriptorSetLayouts.resize(1);
		auto& DSL = DescriptorSetLayouts[0];
		CreateDescriptorSetLayout_1UB(DSL, VK_SHADER_STAGE_GEOMETRY_BIT);
		CreatePipelineLayout_1DSL(DSL);
	}

	virtual void CreateUniformBuffer() override {
		const auto Fov = 0.16f * glm::pi<float>();
		const auto Aspect = GetAspectRatioOfClientRect();
		const auto ZFar = 100.0f;
		const auto ZNear = ZFar * 0.0001f;
		const auto CamPos = glm::vec3(0.0f, 0.0f, 3.0f);
		const auto CamTag = glm::vec3(0.0f);
		const auto CamUp = glm::vec3(0.0f, 1.0f, 0.0f);
		
		Super::CreateUniformBufferT<Transform>({
			GetVulkanClipSpace() * glm::perspective(Fov, Aspect, ZNear, ZFar),
			glm::lookAt(CamPos, CamTag, CamUp),
			glm::mat4(1.0f)
		});
	}

	virtual void CreateDescriptorPool() override { CreateDescriptorPool_1UB(); }
	virtual void UpdateDescriptorSet() override { UpdateDescriptorSet_1UB(); }

	virtual void CreatePipeline() override { CreatePipeline_VsFsTesTcsGs_Tesselation(); }
	virtual void PopulateCommandBuffer(const size_t i) override;

private:
	struct Transform
	{
		glm::mat4 Projection;
		glm::mat4 View;
		glm::mat4 World;
	};
	using Transform = struct Transform;
};
#pragma endregion