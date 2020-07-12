#pragma once

#include "resource.h"


#pragma region Code
#include "../VKImage.h"

class CubeMapVK : public VKImage
{
private:
	using Super = VKImage;
public:
	CubeMapVK() : Super() {}
	virtual ~CubeMapVK() {}

protected:
	virtual void OnTimer(HWND hWnd, HINSTANCE hInstance) override {
		Super::OnTimer(hWnd, hInstance);

		const auto CamPos = glm::vec3(glm::rotate(glm::mat4(1.0f), glm::radians(Degree), glm::vec3(0.0f, 1.0f, 0.0f)) * glm::vec4(0.0f, 0.0f, 3.0f, 1.0f));
#ifdef USE_SKY_DOME
		Tr.View = glm::lookAt(CamPos, glm::vec3(0.0f, 2.0f * glm::sin(glm::radians(Degree)), 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		Tr.World = glm::translate(glm::mat4(1.0f), CamPos); //!< カメラ位置に球を配置
#else
		Tr.View = glm::lookAt(CamPos, glm::vec3(0.0f, 0.0f * glm::sin(glm::radians(Degree)), 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
#endif

		const auto VW = Tr.View * Tr.World;
		Tr.LocalCameraPosition = glm::inverse(VW) * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);

		Degree += 1.0f;

		CopyToHostVisibleDeviceMemory(UniformBuffers[0].DeviceMemory, sizeof(Tr), &Tr, 0);
	}
	virtual void OverridePhysicalDeviceFeatures(VkPhysicalDeviceFeatures& PDF) const { assert(PDF.tessellationShader && "tessellationShader not enabled"); Super::OverridePhysicalDeviceFeatures(PDF); }

#if !defined(USE_SKY_DOME) || defined(USE_DEPTH)
	virtual void CreateFramebuffer() override {
		assert(!RenderPasses.empty() && "");
		assert(3 == ImageViews.size() && "");
		const auto RP = RenderPasses[0];
		const auto DIV = ImageViews[2];
		for (auto i : SwapchainImageViews) {
			Framebuffers.push_back(VkFramebuffer());
			VK::CreateFramebuffer(Framebuffers.back(), RP, SurfaceExtent2D.width, SurfaceExtent2D.height, 1, { i, DIV });
		}
	}
	virtual void CreateRenderPass() override { 
		RenderPasses.resize(1);
		const std::array<VkAttachmentReference, 1> ColorAttach = { { 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL }, };
		const VkAttachmentReference DepthAttach = { 1, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL };
		VK::CreateRenderPass(RenderPasses[0], {
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
#endif
	virtual void CreateIndirectBuffer() override { CreateIndirectBuffer_DrawIndexed(1, 1); }

	virtual void CreateImmutableSampler() override {
		Samplers.resize(1);
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
		VERIFY_SUCCEEDED(vkCreateSampler(Device, &SCI, GetAllocationCallbacks(), &Samplers[0]));
	}

	virtual void CreateDescriptorSetLayout() override {
		DescriptorSetLayouts.resize(1);
		assert(!Samplers.empty() && "");
		const std::array<VkSampler, 1> ISs = { Samplers[0] };
		VKExt::CreateDescriptorSetLayout(DescriptorSetLayouts[0], 0, {
			{ 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_GEOMETRY_BIT, nullptr }, //!< UniformBuffer
			{ 1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, static_cast<uint32_t>(ISs.size()), VK_SHADER_STAGE_FRAGMENT_BIT, ISs.data() }, //!< Sampler + Image0
			{ 2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, static_cast<uint32_t>(ISs.size()), VK_SHADER_STAGE_FRAGMENT_BIT, ISs.data() }, //!< Sampler + Image1
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
		const auto Projection = glm::perspective(Fov, Aspect, ZNear, ZFar);
		const auto View = glm::lookAt(CamPos, CamTag, CamUp);
		const auto World = glm::mat4(1.0f);
		Tr = Transform({ Projection, View, World, glm::vec4(0.0f, 0.0f, 0.0f, 1.0f) });

		UniformBuffers.push_back(UniformBuffer());
		CreateBuffer(&UniformBuffers.back().Buffer, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, sizeof(Tr));
		AllocateDeviceMemory(&UniformBuffers.back().DeviceMemory, UniformBuffers.back().Buffer, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
		VERIFY_SUCCEEDED(vkBindBufferMemory(Device, UniformBuffers.back().Buffer, UniformBuffers.back().DeviceMemory, 0));
	}
	virtual void CreateTexture() override {
		std::wstring Path;
		if (FindDirectory("DDS", Path)) {
			//!< キューブマップ : PX, NX, PY, NY, PZ, NZ
			Images.push_back(VkImage()); ImageViews.push_back(VkImageView());
			LoadImage(&Images.back(), &ImageViews.back(), ToString(Path + TEXT("\\CubeMap\\DebugCube.dds")));
			//LoadImage(&Images.back(), &ImageViews.back(), ToString(Path + TEXT("\\CubeMap\\DesertCube.dds")));
			//LoadImage(&Images.back(), &ImageViews.back(), ToString(Path + TEXT("\\CubeMap\\GrassCube.dds")));
			//LoadImage(&Images.back(), &ImageViews.back(), ToString(Path + TEXT("\\CubeMap\\SunsetCube.dds")));

			//!< 法線マップ
			Images.push_back(VkImage()); ImageViews.push_back(VkImageView());
			//LoadImage(&Images.back(), &ImageViews.back(), ToString(Path + TEXT("\\Leather009_2K-JPG\\Leather009_2K_Normal.dds")));
			LoadImage(&Images.back(), &ImageViews.back(), ToString(Path + TEXT("\\Metal012_2K-JPG\\Metal012_2K_Normal.dds")));
		}
#if !defined(USE_SKY_DOME) || defined(USE_DEPTH)
		const VkExtent3D Extent = { SurfaceExtent2D.width, SurfaceExtent2D.height, 1 };
		const VkComponentMapping CompMap = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };
		{
			Images.push_back(VkImage());
			VK::CreateImage(&Images.back(), 0, VK_IMAGE_TYPE_2D, DepthFormat, Extent, 1, 1, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);

			uint32_t Idx;
			VkDeviceSize Ofs;
			SuballocateImageMemory(Idx, Ofs, Images.back(), VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

			ImageViews.push_back(VkImageView());
			VK::CreateImageView(&ImageViews.back(), Images.back(), VK_IMAGE_VIEW_TYPE_2D, DepthFormat, CompMap, { VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT, 0, 1, 0, 1 });
		}
#endif
	}

	virtual void CreateDescriptorPool() override {
		DescriptorPools.resize(1);
		VKExt::CreateDescriptorPool(DescriptorPools[0], 0, {
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1 }, //!< UniformBuffer
			{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1 }, //!< Sampler + Image0
			{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1 }, //!< Sampler + Image1
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
		DescriptorUpdateTemplates.resize(1);
		assert(!DescriptorSetLayouts.empty() && "");
		VK::CreateDescriptorUpdateTemplate(DescriptorUpdateTemplates[0], {
			{
				0, 0,
				_countof(DescriptorUpdateInfo::DBI), VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, //!< UniformBuffer
				offsetof(DescriptorUpdateInfo, DBI), sizeof(DescriptorUpdateInfo)
			},
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
		}, DescriptorSetLayouts[0]);
	}
	virtual void UpdateDescriptorSet() override {
		assert(2 <= ImageViews.size() && "");
		const DescriptorUpdateInfo DUI = {
			{ UniformBuffers[0].Buffer, 0, VK_WHOLE_SIZE }, //!< UniformBuffer
			{ VK_NULL_HANDLE, ImageViews[0], VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL }, //!< Sampler + Image0
			{ VK_NULL_HANDLE, ImageViews[1], VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL }, //!< Sampler + Image1
		};
		assert(!DescriptorSets.empty() && "");
		assert(!DescriptorUpdateTemplates.empty() && "");
		vkUpdateDescriptorSetWithTemplate(Device, DescriptorSets[0], DescriptorUpdateTemplates[0], &DUI);
	}

	virtual void CreateShaderModules() override {
#ifdef USE_SKY_DOME
		const auto ShaderPath = GetBasePath();
		ShaderModules.push_back(VKExt::CreateShaderModules((ShaderPath + TEXT(".vert.spv")).data()));
		ShaderModules.push_back(VKExt::CreateShaderModules((ShaderPath + TEXT("_sd.frag.spv")).data()));
		ShaderModules.push_back(VKExt::CreateShaderModules((ShaderPath + TEXT("_sd.tese.spv")).data()));
		ShaderModules.push_back(VKExt::CreateShaderModules((ShaderPath + TEXT(".tesc.spv")).data()));
		ShaderModules.push_back(VKExt::CreateShaderModules((ShaderPath + TEXT("_sd.geom.spv")).data())); 
#else
		CreateShaderModle_VsFsTesTcsGs(); 
#endif
	}
	virtual void CreatePipelines() override { 
#if !defined(USE_SKY_DOME) || defined(USE_DEPTH)
		CreatePipeline_VsFsTesTcsGs(VK_PRIMITIVE_TOPOLOGY_PATCH_LIST, 1, VK_TRUE);
#else
		CreatePipeline_VsFsTesTcsGs(VK_PRIMITIVE_TOPOLOGY_PATCH_LIST, 1, VK_FALSE);
#endif
	}
	virtual void PopulateCommandBuffer(const size_t i) override;

private:
	struct Transform
	{
		glm::mat4 Projection;
		glm::mat4 View;
		glm::mat4 World;
		glm::vec4 LocalCameraPosition;
	};
	using Transform = struct Transform;
	float Degree = 0.0f;
	Transform Tr;

	struct DescriptorUpdateInfo
	{
		VkDescriptorBufferInfo DBI[1];
		VkDescriptorImageInfo DII_0[1]; //!< Image0
		VkDescriptorImageInfo DII_1[1]; //!< Image1
	};
};
#pragma endregion