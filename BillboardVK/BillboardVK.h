#pragma once

#include "resource.h"

#pragma region Code
#include "../VKExt.h"

class BillboardVK : public VKExt
{
private:
	using Super = VKExt;
public:
	BillboardVK() : Super() {}
	virtual ~BillboardVK() {}

protected:
	virtual void OverridePhysicalDeviceFeatures(VkPhysicalDeviceFeatures& PhysicalDeviceFeatures) const { assert(PhysicalDeviceFeatures.tessellationShader && "tessellationShader not enabled"); }

	virtual void CreateDepthStencil() override {
		//VK_FORMAT_D32_SFLOAT_S8_UINT,
		//VK_FORMAT_D32_SFLOAT,
		//VK_FORMAT_D24_UNORM_S8_UINT,
		//VK_FORMAT_D16_UNORM_S8_UINT,
		//VK_FORMAT_D16_UNORM
		//Super::CreateDepthStencil(SurfaceExtent2D.width, SurfaceExtent2D.height, VK_FORMAT_D24_UNORM_S8_UINT);
	}

	virtual void CreateIndirectBuffer() override { CreateIndirectBuffer_DrawIndexed(1); }

	virtual void CreateShader(std::vector<VkShaderModule>& ShaderModules, std::vector<VkPipelineShaderStageCreateInfo>& PipelineShaderStageCreateInfos) const override {
		CreateShader_VsPsTesTcsGs(ShaderModules, PipelineShaderStageCreateInfos);
	}

	virtual void CreateInputAssembly(VkPipelineInputAssemblyStateCreateInfo& PipelineInputAssemblyStateCreateInfo) const override {
		CreateInputAssembly_Topology(PipelineInputAssemblyStateCreateInfo, VK_PRIMITIVE_TOPOLOGY_PATCH_LIST);
	}
	virtual void CreateTessellationState(VkPipelineTessellationStateCreateInfo& PipelineTessellationStateCreateInfo) const override {
		const uint32_t PatchControlPoint = 1;
		PipelineTessellationStateCreateInfo = { VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO, nullptr, 0, PatchControlPoint };
	}

	virtual void CreateDescriptorSetLayoutBindings(std::vector<VkDescriptorSetLayoutBinding>& DescriptorSetLayoutBindings) const override {
		CreateDescriptorSetLayoutBindings_1UB(DescriptorSetLayoutBindings, VK_SHADER_STAGE_GEOMETRY_BIT);
	}
	virtual void CreateDescriptorPoolSizes(std::vector<VkDescriptorPoolSize>& DescriptorPoolSizes) const override {
		CreateDescriptorPoolSizes_1UB(DescriptorPoolSizes);
	}
	virtual void CreateWriteDescriptorSets(std::vector<VkWriteDescriptorSet>& WriteDescriptorSets, const std::vector<VkDescriptorBufferInfo>& DescriptorBufferInfos, const std::vector<VkDescriptorImageInfo>& DescriptorImageInfos, const std::vector<VkBufferView>& BufferViews) const override {
		CreateWriteDescriptorSets_1UB(WriteDescriptorSets, DescriptorBufferInfos);
	}
	virtual void UpdateDescriptorSet() override {
		UpdateDescriptorSet_1UB();
	}

	virtual void CreateUniformBuffer() override {
		const auto Fov = 0.16f * glm::pi<float>();
		const auto Aspect = GetAspectRatioOfClientRect();
		const auto ZFar = 100.0f;
		const auto ZNear = ZFar * 0.0001f;
		const auto CamPos = glm::vec3(0.0f, 0.0f, 6.0f);
		const auto CamTag = glm::vec3(0.0f);
		const auto CamUp = glm::vec3(0.0f, 1.0f, 0.0f);

		Super::CreateUniformBuffer<Transform>({
			/*GetVulkanClipSpace() */ glm::perspective(Fov, Aspect, ZNear, ZFar),
			glm::lookAt(CamPos, CamTag, CamUp),
			glm::mat4(1.0f)
		});
	}

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