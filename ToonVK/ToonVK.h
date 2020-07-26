#pragma once

#include "resource.h"

#pragma region Code
#include "../VKExt.h"

//#define USE_FRAME_DESCRIPTOR_SETS

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

#pragma region FRAME_OBJECT
		CopyToHostVisibleDeviceMemory(UniformBuffers[SwapchainImageIndex].DeviceMemory, sizeof(Tr), &Tr, 0);
#pragma endregion
	}
	virtual void OverridePhysicalDeviceFeatures(VkPhysicalDeviceFeatures& PDF) const { assert(PDF.tessellationShader && "tessellationShader not enabled"); Super::OverridePhysicalDeviceFeatures(PDF); }

#ifdef USE_DEPTH
	virtual void CreateTexture() override {
		const VkExtent3D Extent = { SurfaceExtent2D.width, SurfaceExtent2D.height, 1 };
		const VkComponentMapping CompMap = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };

		Images.push_back(Image());
		VK::CreateImage(&Images.back().Image, 0, VK_IMAGE_TYPE_2D, DepthFormat, Extent, 1, 1, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);
		
		AllocateDeviceMemory(&Images.back().DeviceMemory, Images.back().Image, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		VERIFY_SUCCEEDED(vkBindImageMemory(Device, Images.back().Image, Images.back().DeviceMemory, 0));

		ImageViews.push_back(VkImageView());
		VK::CreateImageView(&ImageViews.back(), Images.back().Image, VK_IMAGE_VIEW_TYPE_2D, DepthFormat, CompMap, { VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT, 0, 1, 0, 1 });
	}
	virtual void CreateFramebuffer() override { 
		const auto RP = RenderPasses[0];
		const auto DIV = ImageViews[0];
		for (auto i : SwapchainImageViews) {
			Framebuffers.push_back(VkFramebuffer());
			VK::CreateFramebuffer(Framebuffers.back(), RP, SurfaceExtent2D.width, SurfaceExtent2D.height, 1, { i, DIV });
		}
	}
	virtual void CreateRenderPass() override { 
		RenderPasses.push_back(VkRenderPass());
		const auto ClearOnLoad = true;
		const std::array<VkAttachmentReference, 1> ColorAttach = { { 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL }, };
		const VkAttachmentReference DepthAttach = { 1, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL };
		VK::CreateRenderPass(RenderPasses.back(), {
			{
				0,
				ColorFormat,
				VK_SAMPLE_COUNT_1_BIT,
				ClearOnLoad ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_STORE,
				VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
				VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
			},
			{
				0,
				DepthFormat,
				VK_SAMPLE_COUNT_1_BIT,
				VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE,
				VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
				VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
			},
		}, {
			{
				0,
				VK_PIPELINE_BIND_POINT_GRAPHICS,
				0, nullptr,
				static_cast<uint32_t>(ColorAttach.size()), ColorAttach.data(), nullptr,
				&DepthAttach,
				0, nullptr
			},
		}, {});
	}
#endif

	virtual void CreateIndirectBuffer() override { CreateIndirectBuffer_DrawIndexed(1, 1); }
	virtual void CreateDescriptorSetLayout() override {
		DescriptorSetLayouts.push_back(VkDescriptorSetLayout());
		VKExt::CreateDescriptorSetLayout(DescriptorSetLayouts.back(), 0, {
			{ 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_GEOMETRY_BIT, nullptr },
		});
	}
	virtual void CreatePipelineLayout() override {
		assert(!DescriptorSetLayouts.empty() && "");
		PipelineLayouts.push_back(VkPipelineLayout());
		VKExt::CreatePipelineLayout(PipelineLayouts.back(), {
				DescriptorSetLayouts[0] 
			}, {});
	}

	virtual void CreateDescriptorPool() override {
		DescriptorPools.push_back(VkDescriptorPool());
		VKExt::CreateDescriptorPool(DescriptorPools.back(), 0, {
#pragma region FRAME_OBJECT
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, static_cast<uint32_t>(SwapchainImages.size()) }
#pragma endregion
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
#pragma region FRAME_OBJECT
		for (auto i = 0; i < SwapchainImages.size(); ++i) {
			DescriptorSets.push_back(VkDescriptorSet());
			VERIFY_SUCCEEDED(vkAllocateDescriptorSets(Device, &DSAI, &DescriptorSets.back()));
		}
#pragma endregion
	}
	virtual void CreateDescriptorUpdateTemplate() override {
		DescriptorUpdateTemplates.push_back(VkDescriptorUpdateTemplate());
		assert(!DescriptorSetLayouts.empty() && "");
		VK::CreateDescriptorUpdateTemplate(DescriptorUpdateTemplates.back(), {
			{
				0, 0,
				_countof(DescriptorUpdateInfo::DescriptorBufferInfos), VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
				offsetof(DescriptorUpdateInfo, DescriptorBufferInfos), sizeof(DescriptorUpdateInfo)
			},
		}, DescriptorSetLayouts[0]);
	}
	virtual void UpdateDescriptorSet() override {
#pragma region FRAME_OBJECT
		for (auto i = 0; i < SwapchainImages.size(); ++i) {
			const DescriptorUpdateInfo DUI = {
				{ UniformBuffers[i].Buffer, 0, VK_WHOLE_SIZE },
			};
			vkUpdateDescriptorSetWithTemplate(Device, DescriptorSets[i], DescriptorUpdateTemplates[0], &DUI);
		}
#pragma endregion
	}

	virtual void CreateUniformBuffer() override {
		const auto Fov = 0.16f * glm::pi<float>();
		const auto Aspect = GetAspectRatioOfClientRect();
		const auto ZFar = 100.0f;
		const auto ZNear = ZFar * 0.0001f;
		const auto CamPos = glm::vec3(0.0f, 0.0f, 3.0f);
		const auto CamTag = glm::vec3(0.0f);
		const auto CamUp = glm::vec3(0.0f, 1.0f, 0.0f);
		Tr = Transform({ glm::perspective(Fov, Aspect, ZNear, ZFar), glm::lookAt(CamPos, CamTag, CamUp), glm::mat4(1.0f) });
#pragma region FRAME_OBJECT
		for (auto i = 0; i < SwapchainImages.size(); ++i) {
			UniformBuffers.push_back(UniformBuffer());
			CreateBuffer(&UniformBuffers.back().Buffer, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, sizeof(Tr));
			AllocateDeviceMemory(&UniformBuffers.back().DeviceMemory, UniformBuffers.back().Buffer, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
			VERIFY_SUCCEEDED(vkBindBufferMemory(Device, UniformBuffers.back().Buffer, UniformBuffers.back().DeviceMemory, 0));
		}
#pragma endregion
	}

	virtual void CreateShaderModules() override {
#ifdef USE_SCREENSPACE_WIREFRAME
		const auto ShaderPath = GetBasePath();
		ShaderModules.push_back(VKExt::CreateShaderModules((ShaderPath + TEXT(".vert.spv")).data()));
		ShaderModules.push_back(VKExt::CreateShaderModules((ShaderPath + TEXT("_wf.frag.spv")).data()));
		ShaderModules.push_back(VKExt::CreateShaderModules((ShaderPath + TEXT(".tese.spv")).data()));
		ShaderModules.push_back(VKExt::CreateShaderModules((ShaderPath + TEXT(".tesc.spv")).data()));
		ShaderModules.push_back(VKExt::CreateShaderModules((ShaderPath + TEXT("_wf.geom.spv")).data()));
#else
		CreateShaderModle_VsFsTesTcsGs();
#endif
	}
	virtual void CreatePipelines() override { 
#ifdef USE_DEPTH
		CreatePipeline_VsFsTesTcsGs(VK_PRIMITIVE_TOPOLOGY_PATCH_LIST, 1, VK_TRUE);
#else
		CreatePipeline_VsFsTesTcsGs(VK_PRIMITIVE_TOPOLOGY_PATCH_LIST, 1, VK_FALSE);
#endif
	}
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
};
#pragma endregion