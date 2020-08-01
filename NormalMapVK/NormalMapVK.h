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
	virtual void OnTimer(HWND hWnd, HINSTANCE hInstance) override {
		Super::OnTimer(hWnd, hInstance);

		//Tr.World = glm::rotate(glm::mat4(1.0f), glm::radians(Degree), glm::vec3(1.0f, 0.0f, 0.0f));

		const auto VW = Tr.View * Tr.World;
		Tr.LocalCameraPosition = glm::inverse(VW) * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);

		const auto LightPos = glm::rotate(glm::mat4(1.0f), glm::radians(Degree), glm::vec3(0.0f, 1.0f, 0.0f)) * glm::vec4(10.0f, 0.0f, 0.0f, 0.0f);
		//const auto LightPos = glm::rotate(glm::mat4(1.0f), glm::radians(Degree), glm::vec3(1.0f, 0.0f, 0.0f)) * glm::vec4(0.0f, 10.0f, 0.0f, 0.0f);
		Tr.LocalLightDirection = glm::normalize(glm::inverse(Tr.World) * LightPos);

		Degree += 1.0f;

#pragma region FRAME_OBJECT
		CopyToHostVisibleDeviceMemory(UniformBuffers[SwapchainImageIndex].DeviceMemory, sizeof(Tr), &Tr, 0);
#pragma endregion
	}
	virtual void OverridePhysicalDeviceFeatures(VkPhysicalDeviceFeatures& PDF) const { assert(PDF.tessellationShader && "tessellationShader not enabled"); Super::OverridePhysicalDeviceFeatures(PDF); }

	virtual void CreateFramebuffer() override { 
		assert(!RenderPasses.empty() && "");
		const auto RP = RenderPasses[0];
		const auto DIV = ImageViews[2];
		for (auto i : SwapchainImageViews) {
			Framebuffers.push_back(VkFramebuffer());
			VK::CreateFramebuffer(Framebuffers.back(), RP, SurfaceExtent2D.width, SurfaceExtent2D.height, 1, { i, DIV });
		}
	}
	virtual void CreateRenderPass() override { 
		RenderPasses.push_back(VkRenderPass());
		const std::array<VkAttachmentReference, 1> ColorAttach = { { 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL }, };
		const VkAttachmentReference DepthAttach = { 1, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL };
		VK::CreateRenderPass(RenderPasses.back(), {
				//!< アタッチメント
				{
					0,
					ColorFormat,
					VK_SAMPLE_COUNT_1_BIT,
					VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE,
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
				//!< サブパス
				{
					0,
					VK_PIPELINE_BIND_POINT_GRAPHICS,
					0, nullptr,
					static_cast<uint32_t>(ColorAttach.size()), ColorAttach.data(), nullptr,
					&DepthAttach,
					0, nullptr
				},
			}, {
				//!< サブパス依存
			});
	}
	virtual void CreateIndirectBuffer() override { CreateIndirectBuffer_DrawIndexed(1, 1); }

	virtual void CreateImmutableSampler() override {
		Samplers.push_back(VkSampler());
		const VkSamplerCreateInfo SCI = {
			VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
			nullptr,
			0,
			VK_FILTER_LINEAR, VK_FILTER_LINEAR, VK_SAMPLER_MIPMAP_MODE_LINEAR, // min, mag, mip
			VK_SAMPLER_ADDRESS_MODE_REPEAT, VK_SAMPLER_ADDRESS_MODE_REPEAT, VK_SAMPLER_ADDRESS_MODE_REPEAT, // u, v, w
			0.0f,
			VK_FALSE, 1.0f,
			VK_FALSE, VK_COMPARE_OP_NEVER,
			0.0f, 1.0f,
			VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE,
			VK_FALSE
		};
		VERIFY_SUCCEEDED(vkCreateSampler(Device, &SCI, GetAllocationCallbacks(), &Samplers.back()));
	}

	virtual void CreateDescriptorSetLayout() override {
		DescriptorSetLayouts.push_back(VkDescriptorSetLayout());
		assert(!Samplers.empty() && "");
		const std::array<VkSampler, 1> ISs = { Samplers[0] };
		VKExt::CreateDescriptorSetLayout(DescriptorSetLayouts.back(), 0, {
			{ 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_GEOMETRY_BIT, nullptr }, //!< UniformBuffer
#ifdef USE_COMBINED_IMAGE_SAMPLER
			{ 1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, static_cast<uint32_t>(ISs.size()), VK_SHADER_STAGE_FRAGMENT_BIT, ISs.data() }, //!< Sampler + Image0
			{ 2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, static_cast<uint32_t>(ISs.size()), VK_SHADER_STAGE_FRAGMENT_BIT, ISs.data() }, //!< Sampler + Image1
#else
			{ 1, VK_DESCRIPTOR_TYPE_SAMPLER, static_cast<uint32_t>(ISs.size()), VK_SHADER_STAGE_FRAGMENT_BIT, ISs.data() }, //!< Sampler
			{ 2, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr }, //!< Image0
			{ 3, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr }, //!< Image1
#endif
		});
	}
	virtual void CreatePipelineLayout() override {
		assert(!DescriptorSetLayouts.empty() && "");
		PipelineLayouts.push_back(VkPipelineLayout());
		VKExt::CreatePipelineLayout(PipelineLayouts[0], {
				DescriptorSetLayouts.back() 
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
		const auto Projection = glm::perspective(Fov, Aspect, ZNear, ZFar);
		const auto View = glm::lookAt(CamPos, CamTag, CamUp);
		const auto World = glm::mat4(1.0f);
		Tr = Transform({ Projection, View, World, glm::vec4(0.0f, 0.0f, 0.0f, 1.0f), glm::vec4(10.0f, 0.0f, 0.0f, 0.0f) });
#pragma region FRAME_OBJECT
		for (auto i = 0; i < SwapchainImages.size(); ++i) {
			UniformBuffers.push_back(UniformBuffer());
			CreateBuffer(&UniformBuffers.back().Buffer, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, sizeof(Tr));
			AllocateDeviceMemory(&UniformBuffers.back().DeviceMemory, UniformBuffers.back().Buffer, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
			VERIFY_SUCCEEDED(vkBindBufferMemory(Device, UniformBuffers.back().Buffer, UniformBuffers.back().DeviceMemory, 0));
		}
#pragma endregion
	}
	virtual void CreateTexture() override {
		std::wstring Path;
		if (FindDirectory("DDS", Path)) {
#ifdef USE_PARALLAX_MAP
			//!< [0] 法線(Normal)
			Images.push_back(Image());
			auto GLITexture = LoadImage(&Images.back().Image, &Images.back().DeviceMemory, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, ToString(Path + TEXT("\\Leather009_2K-JPG\\Leather009_2K_Normal.dds")));
			ImageViews.push_back(VkImageView());
			CreateImageView(&ImageViews.back(), Images.back().Image, GLITexture);

			//!< [1] ディスプレースメント(Displacement)
			Images.push_back(Image());
			GLITexture = LoadImage(&Images.back().Image, &Images.back().DeviceMemory, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, ToString(Path + TEXT("\\Leather009_2K-JPG\\Leather009_2K_Displacement.dds")));
			ImageViews.push_back(VkImageView());
			CreateImageView(&ImageViews.back(), Images.back().Image, GLITexture);
#else
			//!< [0] 法線(Normal)
			Images.push_back(Image());
			auto GLITexture = LoadImage(&Images.back().Image, &Images.back().DeviceMemory, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, ToString(Path + TEXT("\\Rocks007_2K-JPG\\Rocks007_2K_Normal.dds")));
			ImageViews.push_back(VkImageView());
			CreateImageView(&ImageViews.back(), Images.back().Image, GLITexture);

			//!< [1] カラー(Color)
			Images.push_back(Image());
			GLITexture = LoadImage(&Images.back().Image, &Images.back().DeviceMemory, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, ToString(Path + TEXT("\\Rocks007_2K-JPG\\Rocks007_2K_Color.dds")));
			ImageViews.push_back(VkImageView());
			CreateImageView(&ImageViews.back(), Images.back().Image, GLITexture);
#endif
		}
		//!< [2] 深度(Depth)
		{
			Images.push_back(Image());

			const VkExtent3D Extent = { SurfaceExtent2D.width, SurfaceExtent2D.height, 1 };
			const VkComponentMapping CompMap = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };
			VK::CreateImage(&Images.back().Image, 0, VK_IMAGE_TYPE_2D, DepthFormat, Extent, 1, 1, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);

			AllocateDeviceMemory(&Images.back().DeviceMemory, Images.back().Image, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
			VERIFY_SUCCEEDED(vkBindImageMemory(Device, Images.back().Image, Images.back().DeviceMemory, 0));

			ImageViews.push_back(VkImageView());
			VK::CreateImageView(&ImageViews.back(), Images.back().Image, VK_IMAGE_VIEW_TYPE_2D, DepthFormat, CompMap, { VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT, 0, 1, 0, 1 });
		}
	}

	virtual void CreateDescriptorPool() override {
		DescriptorPools.push_back(VkDescriptorPool());
		VKExt::CreateDescriptorPool(DescriptorPools.back(), 0, {
#pragma region FRAME_OBJECT
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, static_cast<uint32_t>(SwapchainImages.size()) }, //!< UniformBuffer
#pragma endregion
#ifdef USE_COMBINED_IMAGE_SAMPLER
			{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1 }, //!< Sampler + Image0
			{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1 }, //!< Sampler + Image1
#else
			{ VK_DESCRIPTOR_TYPE_SAMPLER, 1 }, //!< Sampler
			{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1 }, //!< Image0
			{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1 }, //!< Image1
#endif
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
				_countof(DescriptorUpdateInfo::DBI), VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, //!< UniformBuffer
				offsetof(DescriptorUpdateInfo, DBI), sizeof(DescriptorUpdateInfo)
			},
#ifdef USE_COMBINED_IMAGE_SAMPLER
			{
				1, 0,
				_countof(DescriptorUpdateInfo::DII_0), VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, //!< Sampler + Image0
				offsetof(DescriptorUpdateInfo, DII_0), sizeof(DescriptorUpdateInfo)
			},
			{
				2, 0,
				_countof(DescriptorUpdateInfo::DII_1), VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, //!< Sampler + Image1
				offsetof(DescriptorUpdateInfo, DII_1), sizeof(DescriptorUpdateInfo)
			},
#else
			{
				2, 0,
				_countof(DescriptorUpdateInfo::DII_0), VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, //!< Image0
				offsetof(DescriptorUpdateInfo, DII_0), sizeof(DescriptorUpdateInfo)
			},
			{
				3, 0,
				_countof(DescriptorUpdateInfo::DII_1), VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, //!< Image1
				offsetof(DescriptorUpdateInfo, DII_1), sizeof(DescriptorUpdateInfo)
			},
#endif
		}, DescriptorSetLayouts[0]);
	}
	virtual void UpdateDescriptorSet() override {
#pragma region FRAME_OBJECT
		for (auto i = 0; i < SwapchainImages.size(); ++i) {
			const DescriptorUpdateInfo DUI = {
				{ UniformBuffers[i].Buffer, 0, VK_WHOLE_SIZE }, //!< UniformBuffer
#ifdef USE_COMBINED_IMAGE_SAMPLER
				{ VK_NULL_HANDLE, ImageViews[0], VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL }, //!< Sampler + Image0
				{ VK_NULL_HANDLE, ImageViews[1], VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL }, //!< Sampler + Image1
#else
				{ Samplers[0], VK_NULL_HANDLE, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL }, //!< Sampler
				{ VK_NULL_HANDLE, ImageViews[0], VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL }, //!< Image0
				{ VK_NULL_HANDLE, ImageViews[1], VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL }, //!< Image1
#endif
			};
			vkUpdateDescriptorSetWithTemplate(Device, DescriptorSets[i], DescriptorUpdateTemplates[0], &DUI);
		}
#pragma endregion
	}
	
	virtual void CreateShaderModules() override {
		const auto ShaderPath = GetBasePath();
		ShaderModules.push_back(VKExt::CreateShaderModules((ShaderPath + TEXT(".vert.spv")).data()));
#ifdef USE_COMBINED_IMAGE_SAMPLER
#ifdef USE_PARALLAX_MAP
		ShaderModules.push_back(VKExt::CreateShaderModules((ShaderPath + TEXT("_cis_pm.frag.spv")).data()));
#else
		ShaderModules.push_back(VKExt::CreateShaderModules((ShaderPath + TEXT("_cis.frag.spv")).data()));
#endif
#else
#ifdef USE_PARALLAX_MAP
		ShaderModules.push_back(VKExt::CreateShaderModules((ShaderPath + TEXT("_pm.frag.spv")).data()));
#else
		ShaderModules.push_back(VKExt::CreateShaderModules((ShaderPath + TEXT(".frag.spv")).data()));
#endif
#endif
		ShaderModules.push_back(VKExt::CreateShaderModules((ShaderPath + TEXT(".tese.spv")).data()));
		ShaderModules.push_back(VKExt::CreateShaderModules((ShaderPath + TEXT(".tesc.spv")).data()));
		ShaderModules.push_back(VKExt::CreateShaderModules((ShaderPath + TEXT(".geom.spv")).data()));
	}
	virtual void CreatePipelines() override { CreatePipeline_VsFsTesTcsGs(VK_PRIMITIVE_TOPOLOGY_PATCH_LIST, 1, VK_TRUE); }
	virtual void PopulateCommandBuffer(const size_t i) override;

private:
	struct Transform
	{
		glm::mat4 Projection;
		glm::mat4 View;
		glm::mat4 World;
		glm::vec4 LocalCameraPosition;
		glm::vec4 LocalLightDirection;
	};
	using Transform = struct Transform;
	float Degree = 0.0f;
	Transform Tr;
	uint32_t HeapIndex;
	VkDeviceSize Offset;

	struct DescriptorUpdateInfo
	{
		VkDescriptorBufferInfo DBI[1];
#ifndef USE_COMBINED_IMAGE_SAMPLER
		VkDescriptorImageInfo DII_S[1]; //!< Sampler
#endif
		VkDescriptorImageInfo DII_0[1]; //!< Image0
		VkDescriptorImageInfo DII_1[1]; //!< Image1
	};
};
#pragma endregion