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

#pragma region FRAME_OBJECT
		CopyToHostVisibleDeviceMemory(UniformBuffers[GetCurrentBackBufferIndex()].DeviceMemory, 0, sizeof(Tr), &Tr);
#pragma endregion
	}
	virtual void OverridePhysicalDeviceFeatures(VkPhysicalDeviceFeatures& PDF) const { assert(PDF.tessellationShader && "tessellationShader not enabled"); Super::OverridePhysicalDeviceFeatures(PDF); }
	virtual void CreateGeometry() override { 
		constexpr VkDrawIndexedIndirectCommand DIIC = { .indexCount = 1, .instanceCount = 1, .firstIndex = 0, .vertexOffset = 0, .firstInstance = 0 };
		IndirectBuffers.emplace_back().Create(Device, GetCurrentPhysicalDeviceMemoryProperties(), DIIC, CommandBuffers[0], GraphicsQueue);
	}
	virtual void CreateUniformBuffer() override {
		//constexpr auto Fov = 0.16f * glm::pi<float>();
		constexpr auto Fov = 0.16f * std::numbers::pi_v<float>;
		const auto Aspect = GetAspectRatioOfClientRect();
		constexpr auto ZFar = 100.0f;
		constexpr auto ZNear = ZFar * 0.0001f;
		constexpr auto CamPos = glm::vec3(0.0f, 0.0f, 3.0f);
		constexpr auto CamTag = glm::vec3(0.0f);
		constexpr auto CamUp = glm::vec3(0.0f, 1.0f, 0.0f);
		Tr = Transform({ .Projection = glm::perspective(Fov, Aspect, ZNear, ZFar), .View = glm::lookAt(CamPos, CamTag, CamUp), .World = glm::mat4(1.0f) });
#pragma region FRAME_OBJECT
		for (size_t i = 0; i < size(SwapchainImages); ++i) {
			UniformBuffers.emplace_back().Create(Device, GetCurrentPhysicalDeviceMemoryProperties(), sizeof(Tr));
		}
#pragma endregion
	}

#ifdef USE_DEPTH
	//!< 深度テクスチャ
	virtual void CreateTexture() override {		
		const VkExtent3D Extent = { .width = SurfaceExtent2D.width, .height = SurfaceExtent2D.height, .depth = 1 };
		VK::CreateImage(&Images.emplace_back().Image, 0, VK_IMAGE_TYPE_2D, DepthFormat, Extent, 1, 1, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);
		
		AllocateDeviceMemory(&Images.back().DeviceMemory, Images.back().Image, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		VERIFY_SUCCEEDED(vkBindImageMemory(Device, Images.back().Image, Images.back().DeviceMemory, 0));

		const VkComponentMapping CM = { .r = VK_COMPONENT_SWIZZLE_R, .g = VK_COMPONENT_SWIZZLE_G, .b = VK_COMPONENT_SWIZZLE_B, .a = VK_COMPONENT_SWIZZLE_A };
		const VkImageSubresourceRange ISR = { .aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT, .baseMipLevel = 0, .levelCount = 1, .baseArrayLayer = 0, .layerCount = 1 };
		VK::CreateImageView(&ImageViews.emplace_back(), Images.back().Image, VK_IMAGE_VIEW_TYPE_2D, DepthFormat, CM, ISR);
	}
#endif

	virtual void CreatePipelineLayout() override {
		CreateDescriptorSetLayout(DescriptorSetLayouts.emplace_back(), 0, {
			VkDescriptorSetLayoutBinding({.binding = 0, .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, .descriptorCount = 1, .stageFlags = VK_SHADER_STAGE_GEOMETRY_BIT, .pImmutableSamplers = nullptr }),
		});

		VK::CreatePipelineLayout(PipelineLayouts.emplace_back(), DescriptorSetLayouts, {});
	}
#ifdef USE_DEPTH
	virtual void CreateRenderPass() override { VK::CreateRenderPass(VK_ATTACHMENT_LOAD_OP_CLEAR, true); }
#endif

	virtual void CreatePipeline() override { 
#ifdef USE_SCREENSPACE_WIREFRAME
		const auto ShaderPath = GetBasePath();
		ShaderModules.emplace_back(VK::CreateShaderModule(data(ShaderPath + TEXT(".vert.spv"))));
		ShaderModules.emplace_back(VK::CreateShaderModule(data(ShaderPath + TEXT("_wf.frag.spv"))));
		ShaderModules.emplace_back(VK::CreateShaderModule(data(ShaderPath + TEXT(".tese.spv"))));
		ShaderModules.emplace_back(VK::CreateShaderModule(data(ShaderPath + TEXT(".tesc.spv"))));
		ShaderModules.emplace_back(VK::CreateShaderModule(data(ShaderPath + TEXT("_wf.geom.spv"))));
#else
		CreateShaderModle_VsFsTesTcsGs();
#endif

#ifdef USE_DEPTH
		CreatePipeline_VsFsTesTcsGs(VK_PRIMITIVE_TOPOLOGY_PATCH_LIST, 1, VK_TRUE);
#else
		CreatePipeline_VsFsTesTcsGs(VK_PRIMITIVE_TOPOLOGY_PATCH_LIST, 1, VK_FALSE);
#endif
	}

#ifdef USE_DEPTH
	virtual void CreateFramebuffer() override {
		const auto RP = RenderPasses[0];
		const auto DIV = ImageViews[0];
		for (auto i : SwapchainImageViews) {
			VK::CreateFramebuffer(Framebuffers.emplace_back(), RP, SurfaceExtent2D.width, SurfaceExtent2D.height, 1, { i, DIV });
		}
	}
#endif

	virtual void CreateDescriptorSet() override {
		VKExt::CreateDescriptorPool(DescriptorPools.emplace_back(), 0, {
#pragma region FRAME_OBJECT
			VkDescriptorPoolSize({.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, .descriptorCount = static_cast<uint32_t>(size(SwapchainImages)) }) //!< UB * N
#pragma endregion
		});

		const std::array DSLs = { DescriptorSetLayouts[0] };
		const VkDescriptorSetAllocateInfo DSAI = {
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
			.pNext = nullptr,
			.descriptorPool = DescriptorPools[0],
			.descriptorSetCount = static_cast<uint32_t>(size(DSLs)), .pSetLayouts = data(DSLs)
		};
#pragma region FRAME_OBJECT
		const auto SCCount = size(SwapchainImages);
		for (size_t i = 0; i < SCCount; ++i) {
			VERIFY_SUCCEEDED(vkAllocateDescriptorSets(Device, &DSAI, &DescriptorSets.emplace_back()));
		}
#pragma endregion
	}

	virtual void UpdateDescriptorSet() override {
		VK::CreateDescriptorUpdateTemplate(DescriptorUpdateTemplates.emplace_back(), {
			VkDescriptorUpdateTemplateEntry({
				.dstBinding = 0, .dstArrayElement = 0,
				.descriptorCount = _countof(DescriptorUpdateInfo::DescriptorBufferInfos), .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
				.offset = offsetof(DescriptorUpdateInfo, DescriptorBufferInfos), .stride = sizeof(DescriptorUpdateInfo)
			}),
		}, DescriptorSetLayouts[0]);

#pragma region FRAME_OBJECT
		for (size_t i = 0; i < size(SwapchainImages); ++i) {
			const DescriptorUpdateInfo DUI = {
				VkDescriptorBufferInfo({ .buffer = UniformBuffers[i].Buffer, .offset = 0, .range = VK_WHOLE_SIZE }),
			};
			vkUpdateDescriptorSetWithTemplate(Device, DescriptorSets[i], DescriptorUpdateTemplates[0], &DUI);
		}
#pragma endregion
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