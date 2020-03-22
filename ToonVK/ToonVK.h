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
	virtual void OnTimer(HWND hWnd, HINSTANCE hInstance) override {
		Super::OnTimer(hWnd, hInstance);

		Tr.World = glm::rotate(glm::mat4(1.0f), glm::radians(Degree), glm::vec3(1.0f, 0.0f, 0.0f));
		Degree += 1.0f;

		CopyToHostVisibleDeviceMemory(DeviceMemories[HeapIndex], sizeof(Tr), &Tr, Offset);
	}
	virtual void OverridePhysicalDeviceFeatures(VkPhysicalDeviceFeatures& PDF) const { assert(PDF.tessellationShader && "tessellationShader not enabled"); Super::OverridePhysicalDeviceFeatures(PDF); }

	virtual void AllocateSecondaryCommandBuffer() override { AddSecondaryCommandBuffer(); }
#ifdef USE_DEPTH_STENCIL
	virtual void CreateDepthStencil() override { VK::CreateDepthStencil(VK_FORMAT_D24_UNORM_S8_UINT, GetClientRectWidth(), GetClientRectHeight()); }
	virtual void CreateFramebuffer() override { CreateFramebuffer_ColorDepth(); }
	virtual void CreateRenderPass() override { RenderPasses.resize(1); CreateRenderPass_ColorDepth(RenderPasses[0], ColorFormat, VK_FORMAT_D24_UNORM_S8_UINT); }
#endif

	virtual void CreateIndirectBuffer() override { CreateIndirectBuffer_DrawIndexed(1, 1); }
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

#pragma region DESCRIPTOR
	virtual void CreateDescriptorPool() override {
		DescriptorPools.resize(1);
		VKExt::CreateDescriptorPool(DescriptorPools[0], 0, {
				{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1 }
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
		DescriptorSets.resize(1);
		VERIFY_SUCCEEDED(vkAllocateDescriptorSets(Device, &DSAI, &DescriptorSets[0]));
	}
	virtual void CreateDescriptorUpdateTemplate() override {
		const std::array<VkDescriptorUpdateTemplateEntry, 1> DUTEs = {
			{
				0, 0,
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
			VK_DESCRIPTOR_UPDATE_TEMPLATE_TYPE_DESCRIPTOR_SET,
			DescriptorSetLayouts[0],
			VK_PIPELINE_BIND_POINT_GRAPHICS, VK_NULL_HANDLE, 0
		};
		DescriptorUpdateTemplates.resize(1);
		VERIFY_SUCCEEDED(vkCreateDescriptorUpdateTemplate(Device, &DUTCI, GetAllocationCallbacks(), &DescriptorUpdateTemplates[0]));
	}
	virtual void UpdateDescriptorSet() override {
		Super::UpdateDescriptorSet();

		const DescriptorUpdateInfo DUI = {
			{ UniformBuffers[0], Offset, VK_WHOLE_SIZE },
		};
		assert(!DescriptorSets.empty() && "");
		assert(!DescriptorUpdateTemplates.empty() && "");
		vkUpdateDescriptorSetWithTemplate(Device, DescriptorSets[0], DescriptorUpdateTemplates[0], &DUI);
	}
#pragma endregion //!< DESCRIPTOR

	virtual void CreateUniformBuffer() override {
		const auto Fov = 0.16f * glm::pi<float>();
		const auto Aspect = GetAspectRatioOfClientRect();
		const auto ZFar = 100.0f;
		const auto ZNear = ZFar * 0.0001f;
		const auto CamPos = glm::vec3(0.0f, 0.0f, 3.0f);
		const auto CamTag = glm::vec3(0.0f);
		const auto CamUp = glm::vec3(0.0f, 1.0f, 0.0f);
		Tr = Transform({ glm::perspective(Fov, Aspect, ZNear, ZFar), glm::lookAt(CamPos, CamTag, CamUp), glm::mat4(1.0f) });

		UniformBuffers.resize(1);
		CreateBuffer(&UniformBuffers[0], VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, sizeof(Tr));
		SuballocateBufferMemory(HeapIndex, Offset, UniformBuffers[0], VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
	}

	virtual void CreateShaderModules() override { CreateShaderModle_VsFsTesTcsGs(); }
	virtual void CreatePipelines() override { CreatePipeline_VsFsTesTcsGs(VK_PRIMITIVE_TOPOLOGY_PATCH_LIST); }
	virtual void PopulateCommandBuffer(const size_t i) override;

private:
	float Degree = 0.0f;

	struct Transform
	{
		glm::mat4 Projection;
		glm::mat4 View;
		glm::mat4 World;
	};
	using Transform = struct Transform;
	Transform Tr;
	struct DescriptorUpdateInfo
	{
		VkDescriptorBufferInfo DescriptorBufferInfos[1];
	};

	uint32_t HeapIndex;
	VkDeviceSize Offset;
};
#pragma endregion