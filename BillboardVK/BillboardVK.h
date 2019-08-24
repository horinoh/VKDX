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
	virtual void OverridePhysicalDeviceFeatures(VkPhysicalDeviceFeatures& PDF) const { assert(PDF.tessellationShader && "tessellationShader not enabled"); Super::OverridePhysicalDeviceFeatures(PDF); }

	virtual void CreateDepthStencil() override {
		//VK_FORMAT_D32_SFLOAT_S8_UINT,
		//VK_FORMAT_D32_SFLOAT,
		//VK_FORMAT_D24_UNORM_S8_UINT,
		//VK_FORMAT_D16_UNORM_S8_UINT,
		//VK_FORMAT_D16_UNORM

		//Super::CreateDepthStencil(SurfaceExtent2D.width, SurfaceExtent2D.height, VK_FORMAT_D24_UNORM_S8_UINT);
	}

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
		const auto CamPos = glm::vec3(0.0f, 0.0f, 6.0f);
		const auto CamTag = glm::vec3(0.0f);
		const auto CamUp = glm::vec3(0.0f, 1.0f, 0.0f);
		const auto Tr = Transform({
			/*GetVulkanClipSpace() * */ glm::perspective(Fov, Aspect, ZNear, ZFar),
			glm::lookAt(CamPos, CamTag, CamUp),
			glm::mat4(1.0f)
			});

		CreateBuffer(&UniformBuffer, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, sizeof(Tr));
#if 1
		AllocateBufferMemory(&UniformDeviceMemory, UniformBuffer, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
		VERIFY_SUCCEEDED(vkBindBufferMemory(Device, UniformBuffer, UniformDeviceMemory, 0));
		CopyToHostVisibleDeviceMemory(UniformDeviceMemory, sizeof(Tr), &Tr);
#else
		uint32_t HeapIndex;
		VkDeviceSize Offset;
		SuballocateBufferMemory(HeapIndex, Offset, UniformBuffer, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
		CopyToHostVisibleDeviceMemory(DeviceMemories[HeapIndex], sizeof(Tr), &Tr, Offset);
#endif
	}

	virtual void CreateDescriptorPool() override {
		DescriptorPools.resize(1);
		VKExt::CreateDescriptorPool(DescriptorPools[0], /*VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT*/0, {
				{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1 } 
			});
	}
	virtual void AllocateDescriptorSet() override {
		assert(!DescriptorPools.empty() && "");
		assert(!DescriptorSetLayouts.empty() && "");
		std::vector<VkDescriptorSet> DSs;
		VKExt::AllocateDescriptorSet(DSs, DescriptorPools[0], {
				DescriptorSetLayouts[0] 
			});
		std::copy(DSs.begin(), DSs.end(), std::back_inserter(DescriptorSets));
	}
	virtual void UpdateDescriptorSet() override {
		assert(!DescriptorSets.empty() && "");
		assert(VK_NULL_HANDLE != UniformBuffer && "");
		const std::array<VkDescriptorBufferInfo, 1> DBIs = {
			{ UniformBuffer, 0/*オフセット(要アライン)*/, VK_WHOLE_SIZE }
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