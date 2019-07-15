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

	virtual void CreatePipelineLayout() override { CreatePipelineLayout_1UB_GS(); }

	virtual void CreateUniformBuffer() override {
		const auto Fov = 0.16f * glm::pi<float>();
		const auto Aspect = GetAspectRatioOfClientRect();
		const auto ZFar = 100.0f;
		const auto ZNear = ZFar * 0.0001f;
		const auto CamPos = glm::vec3(0.0f, 0.0f, 6.0f);
		const auto CamTag = glm::vec3(0.0f);
		const auto CamUp = glm::vec3(0.0f, 1.0f, 0.0f);

		Super::CreateUniformBufferT<Transform>({
			/*GetVulkanClipSpace() */ glm::perspective(Fov, Aspect, ZNear, ZFar),
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