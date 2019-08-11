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
	virtual void CreateDescriptorSetLayout() override {
		DescriptorSetLayouts.resize(1);
		VKExt::CreateDescriptorSetLayout(DescriptorSetLayouts[0], {
				{ 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_GEOMETRY_BIT, nullptr } 
			});
	}
	virtual void CreatePipelineLayout() override {
		assert(!DescriptorSetLayouts.empty() && "");
		PipelineLayouts.resize(1);
		VKExt::CreatePipelineLayout(PipelineLayouts[0], {
				DescriptorSetLayouts[0] 
			}, {});
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

	virtual void CreateDescriptorPool() override { 
		DescriptorPools.resize(1);
		VKExt::CreateDescriptorPool(DescriptorPools[0], {
				{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1 } 
			});
	}
	virtual void CreateDescriptorSet() override {
		assert(!DescriptorPools.empty() && "");
		assert(!DescriptorSetLayouts.empty() && "");
		DescriptorSets.resize(1);
		VKExt::CreateDescriptorSet(DescriptorSets[0], DescriptorPools[0], {
				DescriptorSetLayouts[0] 
			});
	}
	virtual void UpdateDescriptorSet() override {
		assert(!DescriptorSets.empty() && "");
		assert(VK_NULL_HANDLE != UniformBuffer && "");
		const std::array<VkDescriptorBufferInfo, 1> DBIs = {
			{ UniformBuffer, 0, VK_WHOLE_SIZE }
		};
		VKExt::UpdateDescriptorSet(
			{
				{
					VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
					nullptr,
					DescriptorSets[0], 0, 0,
					static_cast<uint32_t>(DBIs.size()), VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, nullptr, DBIs.data(), nullptr
				}
			},
			{});
	}
	virtual void CreateShaderModule() override { CreateShaderModle_VsFsTesTcsGs(); }
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