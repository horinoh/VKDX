#pragma once

#include "resource.h"

#pragma region Code
#include "../VKImage.h"

class NormalMapVK : public VKImage
{
private:
	using Super = VKImage;
public:
	NormalMapVK() : Super() {}
	virtual ~NormalMapVK() {}

protected:
	virtual void OverridePhysicalDeviceFeatures(VkPhysicalDeviceFeatures& PDF) const { assert(PDF.tessellationShader && "tessellationShader not enabled"); Super::OverridePhysicalDeviceFeatures(PDF); }

	virtual void CreateIndirectBuffer() override { CreateIndirectBuffer_DrawIndexed(1); }

	virtual void CreateDescriptorSetLayout() override {
		DescriptorSetLayouts.resize(1);
#ifdef USE_IMMUTABLE_SAMPLER
		assert(!Samplers.empty() && "");
		const std::array<VkSampler, 1> ISs = { 
			Samplers[0] 
		};
		VKExt::CreateDescriptorSetLayout(DescriptorSetLayouts[0], {
				{ 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_GEOMETRY_BIT, nullptr },
				{ 1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, static_cast<uint32_t>(ISs.size()), VK_SHADER_STAGE_FRAGMENT_BIT, ISs.data() }
			});
#else
		VKExt::CreateDescriptorSetLayout(DescriptorSetLayouts[0],
			{
				{ 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_GEOMETRY_BIT, nullptr },
				{ 1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr },
			});
#endif
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
		const auto Tr = Transform({
					GetVulkanClipSpace() * glm::perspective(Fov, Aspect, ZNear, ZFar),
					glm::lookAt(CamPos, CamTag, CamUp),
					glm::mat4(1.0f)
			});

		CreateBuffer(&UniformBuffer, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, sizeof(Tr));

		uint32_t HeapIndex;
		VkDeviceSize Offset;
		SuballocateBufferMemory(HeapIndex, Offset, UniformBuffer, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
		CopyToHostVisibleDeviceMemory(DeviceMemories[HeapIndex], sizeof(Tr), &Tr, Offset);
	}

	virtual void CreateDescriptorPool() override {
		DescriptorPools.resize(1);
		VKExt::CreateDescriptorPool(DescriptorPools[0], /*VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT*/0, {
				{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1 }, 
				{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,1 } 
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
		//assert(!Samplers.empty() && "");
		const std::array<VkDescriptorBufferInfo, 1> DBIs = {
			{ UniformBuffer, 0, VK_WHOLE_SIZE }
		};
		const std::array<VkDescriptorImageInfo, 1> DIIs = {
#ifdef USE_IMMUTABLE_SAMPLER
			{ VK_NULL_HANDLE, ImageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL }
#else
			{ Samplers[0], ImageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL }
#endif
		};
		VKExt::UpdateDescriptorSet(
			{
				{
					VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
					nullptr,
					DescriptorSets[0], 0, 0,
					static_cast<uint32_t>(DBIs.size()), VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, nullptr, DBIs.data(), nullptr
				},
				{
					VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
					nullptr,
					DescriptorSets[0], 1, 0,
					static_cast<uint32_t>(DIIs.size()), VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, DIIs.data(), nullptr, nullptr
				}
			},
			{});
	}

	virtual void CreateTexture() override {
		LoadImage(&Image, &ImageDeviceMemory, &ImageView, "NormalMap.dds");
	}
	virtual void CreateSampler() override {
		Samplers.resize(1);
#ifdef USE_IMMUTABLE_SAMPLER
		const VkSamplerCreateInfo SCI = {
			VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
			nullptr,
			0,
			VK_FILTER_LINEAR, VK_FILTER_LINEAR, VK_SAMPLER_MIPMAP_MODE_LINEAR,
			VK_SAMPLER_ADDRESS_MODE_REPEAT, VK_SAMPLER_ADDRESS_MODE_REPEAT, VK_SAMPLER_ADDRESS_MODE_REPEAT,
			0.0f,
			VK_FALSE, 1.0f,
			VK_FALSE, VK_COMPARE_OP_NEVER,
			0.0f, 1.0f,
			VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE,
			VK_FALSE
		};
#else
		const VkSamplerCreateInfo SCI = {
			VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
			nullptr,
			0,
			VK_FILTER_NEAREST, VK_FILTER_NEAREST, VK_SAMPLER_MIPMAP_MODE_NEAREST,
			VK_SAMPLER_ADDRESS_MODE_REPEAT, VK_SAMPLER_ADDRESS_MODE_REPEAT, VK_SAMPLER_ADDRESS_MODE_REPEAT,
			0.0f,
			VK_FALSE, 1.0f,
			VK_FALSE, VK_COMPARE_OP_NEVER,
			0.0f, 1.0f,
			VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE,
			VK_FALSE
		};
#endif
		VERIFY_SUCCEEDED(vkCreateSampler(Device, &SCI, GetAllocationCallbacks(), &Samplers[0]));
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