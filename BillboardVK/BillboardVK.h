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
		Tr = Transform({ /*GetVulkanClipSpace() * */ glm::perspective(Fov, Aspect, ZNear, ZFar), glm::lookAt(CamPos, CamTag, CamUp), glm::mat4(1.0f) });

		CreateBuffer(&UniformBuffer, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, sizeof(Tr));

		SuballocateBufferMemory(HeapIndex, Offset, UniformBuffer, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
		CopyToHostVisibleDeviceMemory(DeviceMemories[HeapIndex], sizeof(Tr), &Tr, Offset);
	}

#ifdef USE_DESCRIPTOR_UPDATE_TEMPLATE
	virtual void CreateDescriptorUpdateTemplate() override {
		const std::array<VkDescriptorUpdateTemplateEntry, 1> DUTEs = {
			{
				0/*binding*/, 0/*arrayElement*/,
				_countof(DescriptorUpdateInfo::DescriptorBufferInfos), VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
				offsetof(DescriptorUpdateInfo, DescriptorBufferInfos), sizeof(DescriptorUpdateInfo)
			}
		};

		assert(!DescriptorSetLayouts.empty() && "");
		const VkDescriptorUpdateTemplateCreateInfo DUTCI = {
			VK_STRUCTURE_TYPE_DESCRIPTOR_UPDATE_TEMPLATE_CREATE_INFO,
			nullptr,
			0,
			static_cast<uint32_t>(DUTEs.size()), DUTEs.data(),	
#ifdef USE_PUSH_DESCRIPTOR
			VK_DESCRIPTOR_UPDATE_TEMPLATE_TYPE_PUSH_DESCRIPTORS_KHR,
#else
			VK_DESCRIPTOR_UPDATE_TEMPLATE_TYPE_DESCRIPTOR_SET,
#endif
			DescriptorSetLayouts[0],
			VK_PIPELINE_BIND_POINT_GRAPHICS, VK_NULL_HANDLE, 0
		};
		DescriptorUpdateTemplates.resize(1);
		VERIFY_SUCCEEDED(vkCreateDescriptorUpdateTemplate(Device, &DUTCI, GetAllocationCallbacks(), &DescriptorUpdateTemplates[0]));
	}
#endif

	virtual void CreateDescriptorPool() override {
		DescriptorPools.resize(1);
		VKExt::CreateDescriptorPool(DescriptorPools[0], /*VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT*/0, {
				//{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1 } 
				{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, static_cast<uint32_t>(SwapchainImages.size()) }
			});
	}
	virtual void AllocateDescriptorSet() override {
		assert(!DescriptorSetLayouts.empty() && "");
		const std::array<VkDescriptorSetLayout, 1> DSLs = { DescriptorSetLayouts[0] };
		assert(!DescriptorPools.empty() && "");
		const VkDescriptorSetAllocateInfo DSAI = {
			VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
			nullptr,
			DescriptorPools[0],
			static_cast<uint32_t>(DSLs.size()), DSLs.data()
		};
		//DescriptorSets.resize(1);
		DescriptorSets.resize(SwapchainImages.size());
		for (auto& i : DescriptorSets) {
			VERIFY_SUCCEEDED(vkAllocateDescriptorSets(Device, &DSAI, &i));
		}

		for (auto i : DescriptorSets) {
			UpdateDescriptorSet(i);
		}
	}
	virtual void UpdateDescriptorSet(const VkDescriptorSet DS) override {
		const DescriptorUpdateInfo DUI = {
			{ UniformBuffer, 0/*offset*/, VK_WHOLE_SIZE/*range*/ },
		};
#ifdef USE_DESCRIPTOR_UPDATE_TEMPLATE
		assert(!DescriptorUpdateTemplates.empty() && "");
		vkUpdateDescriptorSetWithTemplate(Device, DS, DescriptorUpdateTemplates[0], &DUI);
#else
		VKExt::UpdateDescriptorSet(
			{
				{
					VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
					nullptr,
					DS, 0/*binding*/, 0/*arrayElement*/,
					_countof(DescriptorUpdateInfo::DescriptorBufferInfos), VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, nullptr/*VkDescriptorImageInfo*/, DUI.DescriptorBufferInfos, nullptr/*VkBufferView*/
				}
			},
			{});
#endif
	}

	virtual void UpdateDescriptorSet() override {
		Tr.World = glm::rotate(glm::mat4(1.0f), glm::radians(Degree), glm::vec3(1.0f, 0.0f, 0.0f));
		Degree += 1.0f;
		CopyToHostVisibleDeviceMemory(DeviceMemories[HeapIndex], sizeof(Tr), &Tr, Offset);

		assert(!DescriptorSets.empty() && "");
		UpdateDescriptorSet(DescriptorSets[0]);
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

	float Degree = 0.0f;
	Transform Tr;
	uint32_t HeapIndex;
	VkDeviceSize Offset;

	struct DescriptorUpdateInfo 
	{
		VkDescriptorBufferInfo DescriptorBufferInfos[1];
	};
};
#pragma endregion