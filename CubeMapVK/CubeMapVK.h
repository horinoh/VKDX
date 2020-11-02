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

#pragma region FRAME_OBJECT
		CopyToHostVisibleDeviceMemory(UniformBuffers[SwapchainImageIndex].DeviceMemory, sizeof(Tr), &Tr, 0);
#pragma endregion
	}
	virtual void OverridePhysicalDeviceFeatures(VkPhysicalDeviceFeatures& PDF) const { assert(PDF.tessellationShader && "tessellationShader not enabled"); Super::OverridePhysicalDeviceFeatures(PDF); }

#if !defined(USE_SKY_DOME)
	virtual void CreateFramebuffer() override {
		assert(!empty(RenderPasses) && "");
		const auto RP = RenderPasses[0];
		const auto DIV = ImageViews[2];
		for (auto i : SwapchainImageViews) {
			Framebuffers.emplace_back(VkFramebuffer());
			VK::CreateFramebuffer(Framebuffers.back(), RP, SurfaceExtent2D.width, SurfaceExtent2D.height, 1, { i, DIV });
		}
	}
	virtual void CreateRenderPass() override { 
		RenderPasses.emplace_back(VkRenderPass());
		const std::array ColorAttach = { VkAttachmentReference({ .attachment = 0, .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL }), };
		const VkAttachmentReference DepthAttach = { .attachment = 1, .layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL };
		VK::CreateRenderPass(RenderPasses.back(), {
				VkAttachmentDescription({
					.flags = 0,
					.format = ColorFormat,
					.samples = VK_SAMPLE_COUNT_1_BIT,
					.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR, .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
					.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE, .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
					.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED, .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
				}),
				VkAttachmentDescription({
					.flags = 0,
					.format = DepthFormat,
					.samples = VK_SAMPLE_COUNT_1_BIT,
					.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR, .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
					.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE, .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
					.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED, .finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
				}),
			}, {
				VkSubpassDescription({
					.flags = 0,
					.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
					.inputAttachmentCount = 0, .pInputAttachments = nullptr,
					.colorAttachmentCount = static_cast<uint32_t>(size(ColorAttach)), .pColorAttachments = data(ColorAttach), .pResolveAttachments = nullptr,
					.pDepthStencilAttachment = &DepthAttach,
					.preserveAttachmentCount = 0, .pPreserveAttachments = nullptr
				}),
			}, {
				//!< サブパス依存
			});
	}
#endif
	virtual void CreateIndirectBuffer() override { CreateIndirectBuffer_DrawIndexed(1, 1); }

	virtual void CreateImmutableSampler() override {
		Samplers.emplace_back(VkSampler());
		const VkSamplerCreateInfo SCI = {
			.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
			.pNext = nullptr,
			.flags = 0,
			.magFilter = VK_FILTER_LINEAR, .minFilter = VK_FILTER_LINEAR, .mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
			.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT, .addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT, .addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT,
			.mipLodBias = 0.0f,
			.anisotropyEnable = VK_FALSE, .maxAnisotropy = 1.0f,
			.compareEnable = VK_FALSE, .compareOp = VK_COMPARE_OP_NEVER,
			.minLod = 0.0f, .maxLod = 1.0f,
			.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE,
			.unnormalizedCoordinates = VK_FALSE
		};
		VERIFY_SUCCEEDED(vkCreateSampler(Device, &SCI, GetAllocationCallbacks(), &Samplers.back()));
	}

	virtual void CreateDescriptorSetLayout() override {
		DescriptorSetLayouts.emplace_back(VkDescriptorSetLayout());
		assert(!empty(Samplers) && "");
		const std::array ISs = { Samplers[0] };
		VKExt::CreateDescriptorSetLayout(DescriptorSetLayouts.back(), 0, {
			VkDescriptorSetLayoutBinding({ .binding = 0, .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, .descriptorCount = 1, .stageFlags = VK_SHADER_STAGE_GEOMETRY_BIT, .pImmutableSamplers = nullptr }), //!< UniformBuffer
			VkDescriptorSetLayoutBinding({ .binding = 1, .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, .descriptorCount = static_cast<uint32_t>(size(ISs)), .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT, .pImmutableSamplers = data(ISs) }), //!< Sampler + Image0
			VkDescriptorSetLayoutBinding({ .binding = 2, .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, .descriptorCount = static_cast<uint32_t>(size(ISs)), .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT, .pImmutableSamplers = data(ISs) }), //!< Sampler + Image1
		});
	}
	virtual void CreatePipelineLayout() override {
		assert(!empty(DescriptorSetLayouts) && "");
		PipelineLayouts.emplace_back(VkPipelineLayout());
		VKExt::CreatePipelineLayout(PipelineLayouts.back(), {
				DescriptorSetLayouts[0]
			}, {});
	}

	virtual void CreateUniformBuffer() override {
		//const auto Fov = 0.16f * glm::pi<float>();
		const auto Fov = 0.16f * std::numbers::pi_v<float>;
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

#pragma region FRAME_OBJECT
		const auto SCCount = size(SwapchainImages);
		for (size_t i = 0; i < SCCount; ++i) {
			UniformBuffers.emplace_back(UniformBuffer());
			CreateBuffer(&UniformBuffers.back().Buffer, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, sizeof(Tr));
			AllocateDeviceMemory(&UniformBuffers.back().DeviceMemory, UniformBuffers.back().Buffer, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
			VERIFY_SUCCEEDED(vkBindBufferMemory(Device, UniformBuffers.back().Buffer, UniformBuffers.back().DeviceMemory, 0));
		}
#pragma endregion
	}
	virtual void CreateTexture() override {
		std::wstring Path;
		if (FindDirectory("DDS", Path)) {
			//!< [0] キューブ(Cube) : PX, NX, PY, NY, PZ, NZ
			Images.emplace_back(Image());
			auto GLITexture = LoadImage(&Images.back().Image, &Images.back().DeviceMemory, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, ToString(Path + TEXT("\\CubeMap\\DebugCube.dds")));
			ImageViews.emplace_back(VkImageView());
			CreateImageView(&ImageViews.back(), Images.back().Image, GLITexture);

			//!< [1] 法線(Normal)
			Images.emplace_back(Image());
			GLITexture = LoadImage(&Images.back().Image, &Images.back().DeviceMemory, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, ToString(Path + TEXT("\\Metal012_2K-JPG\\Metal012_2K_Normal.dds")));
			ImageViews.emplace_back(VkImageView());
			CreateImageView(&ImageViews.back(), Images.back().Image, GLITexture);
		}
#if !defined(USE_SKY_DOME)
		//!< [2] 深度(DepthMap)
		{
			Images.emplace_back(Image());

			const VkExtent3D Extent = { .width = SurfaceExtent2D.width, .height = SurfaceExtent2D.height, .depth = 1 };
			const VkComponentMapping CompMap = { .r = VK_COMPONENT_SWIZZLE_R, .g = VK_COMPONENT_SWIZZLE_G, .b = VK_COMPONENT_SWIZZLE_B, .a = VK_COMPONENT_SWIZZLE_A };

			VK::CreateImage(&Images.back().Image, 0, VK_IMAGE_TYPE_2D, DepthFormat, Extent, 1, 1, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);

			AllocateDeviceMemory(&Images.back().DeviceMemory, Images.back().Image, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
			VERIFY_SUCCEEDED(vkBindImageMemory(Device, Images.back().Image, Images.back().DeviceMemory, 0));

			ImageViews.emplace_back(VkImageView());
			VK::CreateImageView(&ImageViews.back(), Images.back().Image, VK_IMAGE_VIEW_TYPE_2D, DepthFormat, CompMap, VkImageSubresourceRange({ .aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT, .baseMipLevel = 0, .levelCount = 1, .baseArrayLayer = 0, .layerCount = 1 }));
		}
#endif
	}

	virtual void CreateDescriptorPool() override {
#pragma region FRAME_OBJECT
		const auto SCCount = static_cast<uint32_t>(size(SwapchainImages));
#pragma endregion

		DescriptorPools.emplace_back(VkDescriptorPool());
		VKExt::CreateDescriptorPool(DescriptorPools.back(), 0, {
#pragma region FRAME_OBJECT
			VkDescriptorPoolSize({ .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, .descriptorCount = SCCount }), //!< UB * N
#pragma endregion
			VkDescriptorPoolSize({ .type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, .descriptorCount = 1 }), //!< Sampler + Image0
			VkDescriptorPoolSize({ .type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, .descriptorCount = 1 }), //!< Sampler + Image1
		});
	}
	virtual void AllocateDescriptorSet() override {
		assert(!empty(DescriptorSetLayouts) && "");
		const std::array DSLs = { DescriptorSetLayouts[0] };
		assert(!empty(DescriptorPools) && "");
		const VkDescriptorSetAllocateInfo DSAI = {
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
			.pNext = nullptr,
			.descriptorPool = DescriptorPools[0],
			.descriptorSetCount = static_cast<uint32_t>(size(DSLs)), .pSetLayouts = data(DSLs)
		};
#pragma region FRAME_OBJECT
		const auto SCCount = size(SwapchainImages);
		for (size_t i = 0; i < SCCount; ++i) {
			DescriptorSets.emplace_back(VkDescriptorSet());
			VERIFY_SUCCEEDED(vkAllocateDescriptorSets(Device, &DSAI, &DescriptorSets.back()));
		}
#pragma endregion
	}
	virtual void CreateDescriptorUpdateTemplate() override {
		DescriptorUpdateTemplates.emplace_back(VkDescriptorUpdateTemplate());
		assert(!empty(DescriptorSetLayouts) && "");
		VK::CreateDescriptorUpdateTemplate(DescriptorUpdateTemplates.back(), {
			VkDescriptorUpdateTemplateEntry({
				.dstBinding = 0, .dstArrayElement = 0,
				.descriptorCount = _countof(DescriptorUpdateInfo::DBI), .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, //!< UniformBuffer
				.offset = offsetof(DescriptorUpdateInfo, DBI), .stride = sizeof(DescriptorUpdateInfo)
			}),
			VkDescriptorUpdateTemplateEntry({
				.dstBinding = 1, .dstArrayElement = 0,
				.descriptorCount = _countof(DescriptorUpdateInfo::DII_0), .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, //!< Sampler + Image0
				.offset = offsetof(DescriptorUpdateInfo, DII_0), .stride = sizeof(DescriptorUpdateInfo)
			}),
			VkDescriptorUpdateTemplateEntry({
				.dstBinding = 2, .dstArrayElement = 0,
				.descriptorCount = _countof(DescriptorUpdateInfo::DII_1), .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, //!< Sampler + Image1
				.offset = offsetof(DescriptorUpdateInfo, DII_1), .stride = sizeof(DescriptorUpdateInfo)
			}),
		}, DescriptorSetLayouts[0]);
	}
	virtual void UpdateDescriptorSet() override {
#pragma region FRAME_OBJECT
		const auto SCCount = size(SwapchainImages);
		for (size_t i = 0; i < SCCount; ++i) {
			const DescriptorUpdateInfo DUI = {
				VkDescriptorBufferInfo({ .buffer = UniformBuffers[i].Buffer, .offset = 0, .range = VK_WHOLE_SIZE }),
				VkDescriptorImageInfo({ .sampler = VK_NULL_HANDLE, .imageView = ImageViews[0], .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL }), //!< Sampler + Image0
				VkDescriptorImageInfo({ .sampler = VK_NULL_HANDLE, .imageView = ImageViews[1], .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL }), //!< Sampler + Image1
			};
			vkUpdateDescriptorSetWithTemplate(Device, DescriptorSets[i], DescriptorUpdateTemplates[0], &DUI);
		}
#pragma endregion
	}

	virtual void CreateShaderModules() override {
#ifdef USE_SKY_DOME
		const auto ShaderPath = GetBasePath();
		ShaderModules.emplace_back(VK::CreateShaderModule(data(ShaderPath + TEXT(".vert.spv"))));
		ShaderModules.emplace_back(VK::CreateShaderModule(data(ShaderPath + TEXT("_sd.frag.spv"))));
		ShaderModules.emplace_back(VK::CreateShaderModule(data(ShaderPath + TEXT("_sd.tese.spv"))));
		ShaderModules.emplace_back(VK::CreateShaderModule(data(ShaderPath + TEXT(".tesc.spv")))));
		ShaderModules.emplace_back(VK::CreateShaderModule(data(ShaderPath + TEXT("_sd.geom.spv"))))); 
#else
		CreateShaderModle_VsFsTesTcsGs(); 
#endif
	}
	virtual void CreatePipelines() override { 
#if !defined(USE_SKY_DOME)
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