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
		CopyToHostVisibleDeviceMemory(UniformBuffers[GetCurrentBackBufferIndex()].DeviceMemory, 0, sizeof(Tr), &Tr);
#pragma endregion
	}

	virtual void CreateGeometry() override {
		const auto PDMP = GetCurrentPhysicalDeviceMemoryProperties();
		constexpr VkDrawIndexedIndirectCommand DIIC = { .indexCount = 1, .instanceCount = 1, .firstIndex = 0, .vertexOffset = 0, .firstInstance = 0 };
		IndirectBuffers.emplace_back().Create(Device, PDMP, DIIC).SubmitCopyCommand(Device, PDMP, CommandBuffers[0], GraphicsQueue, sizeof(DIIC), &DIIC);
	}
	virtual void CreateUniformBuffer() override {
		constexpr auto Fov = 0.16f * std::numbers::pi_v<float>;
		const auto Aspect = GetAspectRatioOfClientRect();
		constexpr auto ZFar = 100.0f;
		constexpr auto ZNear = ZFar * 0.0001f;
		constexpr auto CamPos = glm::vec3(0.0f, 0.0f, 3.0f);
		constexpr auto CamTag = glm::vec3(0.0f);
		constexpr auto CamUp = glm::vec3(0.0f, 1.0f, 0.0f);
		const auto Projection = glm::perspective(Fov, Aspect, ZNear, ZFar);
		const auto View = glm::lookAt(CamPos, CamTag, CamUp);
		constexpr auto World = glm::mat4(1.0f);
		Tr = Transform({ Projection, View, World, glm::vec4(0.0f, 0.0f, 0.0f, 1.0f) });
#pragma region FRAME_OBJECT
		for (size_t i = 0; i < size(SwapchainImages); ++i) {
			UniformBuffers.emplace_back().Create(Device, GetCurrentPhysicalDeviceMemoryProperties(), sizeof(Tr));
		}
#pragma endregion
	}
	virtual void CreateTexture() override {
		const auto PDMP = GetCurrentPhysicalDeviceMemoryProperties();
		std::wstring Path;
		if (FindDirectory("DDS", Path)) {
			const auto CB = CommandBuffers[0];
			//!< [0] キューブ(Cube) : PX, NX, PY, NY, PZ, NZ
			DDSTextures.emplace_back().Create(Device, PDMP, ToString(Path + TEXT("\\CubeMap\\DebugCube.dds"))).SubmitCopyCommand(Device, PDMP, CB, GraphicsQueue, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
			//!< [1] 法線(Normal)
			DDSTextures.emplace_back().Create(Device, PDMP, ToString(Path + TEXT("\\Metal012_2K-JPG\\Metal012_2K_Normal.dds"))).SubmitCopyCommand(Device, PDMP, CB, GraphicsQueue, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
		}
#if !defined(USE_SKY_DOME)
		//!< [2] 深度(DepthMap)
		DepthTextures.emplace_back().Create(Device, PDMP, DepthFormat, VkExtent3D({ .width = SurfaceExtent2D.width, .height = SurfaceExtent2D.height, .depth = 1 }));
#endif
	}
	virtual void CreateImmutableSampler() override {
		constexpr VkSamplerCreateInfo SCI = {
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
		VERIFY_SUCCEEDED(vkCreateSampler(Device, &SCI, GetAllocationCallbacks(), &Samplers.emplace_back()));
	}
	virtual void CreatePipelineLayout() override {
		const std::array ISs = { Samplers[0] };
		VKExt::CreateDescriptorSetLayout(DescriptorSetLayouts.emplace_back(), 0, {
			//!< UniformBuffer
			VkDescriptorSetLayoutBinding({.binding = 0, .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, .descriptorCount = 1, .stageFlags = VK_SHADER_STAGE_GEOMETRY_BIT, .pImmutableSamplers = nullptr }),
			//!< Sampler + Image0
			VkDescriptorSetLayoutBinding({.binding = 1, .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, .descriptorCount = static_cast<uint32_t>(size(ISs)), .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT, .pImmutableSamplers = data(ISs) }),
			//!< Sampler + Image1
			VkDescriptorSetLayoutBinding({.binding = 2, .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, .descriptorCount = static_cast<uint32_t>(size(ISs)), .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT, .pImmutableSamplers = data(ISs) }),
		});
		VKExt::CreatePipelineLayout(PipelineLayouts.emplace_back(), DescriptorSetLayouts, {});
	}
#if !defined(USE_SKY_DOME)
	virtual void CreateRenderPass() override { VKExt::CreateRenderPass_Depth(); }
#endif
	virtual void CreatePipeline() override {
		const auto ShaderPath = GetBasePath();
		const std::array SMs = {
#ifdef USE_SKY_DOME
			VK::CreateShaderModule(data(ShaderPath + TEXT(".vert.spv"))),
			VK::CreateShaderModule(data(ShaderPath + TEXT("_sd.frag.spv"))),
			VK::CreateShaderModule(data(ShaderPath + TEXT("_sd.tese.spv"))),
			VK::CreateShaderModule(data(ShaderPath + TEXT(".tesc.spv"))),
			VK::CreateShaderModule(data(ShaderPath + TEXT("_sd.geom.spv"))),
#else
			VK::CreateShaderModule(data(ShaderPath + TEXT(".vert.spv"))),
			VK::CreateShaderModule(data(ShaderPath + TEXT(".frag.spv"))),
			VK::CreateShaderModule(data(ShaderPath + TEXT(".tese.spv"))),
			VK::CreateShaderModule(data(ShaderPath + TEXT(".tesc.spv"))),
			VK::CreateShaderModule(data(ShaderPath + TEXT(".geom.spv"))),
#endif
		};
		const std::array PSSCIs = {
			VkPipelineShaderStageCreateInfo({.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, .pNext = nullptr, .flags = 0, .stage = VK_SHADER_STAGE_VERTEX_BIT, .module = SMs[0], .pName = "main", .pSpecializationInfo = nullptr }),
			VkPipelineShaderStageCreateInfo({.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, .pNext = nullptr, .flags = 0, .stage = VK_SHADER_STAGE_FRAGMENT_BIT, .module = SMs[1], .pName = "main", .pSpecializationInfo = nullptr }),
			VkPipelineShaderStageCreateInfo({.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, .pNext = nullptr, .flags = 0, .stage = VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT, .module = SMs[2], .pName = "main", .pSpecializationInfo = nullptr }),
			VkPipelineShaderStageCreateInfo({.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, .pNext = nullptr, .flags = 0, .stage = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT, .module = SMs[3], .pName = "main", .pSpecializationInfo = nullptr }),
			VkPipelineShaderStageCreateInfo({.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, .pNext = nullptr, .flags = 0, .stage = VK_SHADER_STAGE_GEOMETRY_BIT, .module = SMs[4], .pName = "main", .pSpecializationInfo = nullptr }),
		};
#ifdef USE_SKY_DOME
		CreatePipeline_VsFsTesTcsGs(VK_PRIMITIVE_TOPOLOGY_PATCH_LIST, 1, VK_FALSE, PSSCIs);
#else
		CreatePipeline_VsFsTesTcsGs(VK_PRIMITIVE_TOPOLOGY_PATCH_LIST, 1, VK_TRUE, PSSCIs);
#endif
		for (auto i : SMs) { vkDestroyShaderModule(Device, i, GetAllocationCallbacks()); }
	}
#if !defined(USE_SKY_DOME)
	virtual void CreateFramebuffer() override {
		for (auto i : SwapchainImageViews) {
			VK::CreateFramebuffer(Framebuffers.emplace_back(), RenderPasses[0], SurfaceExtent2D.width, SurfaceExtent2D.height, 1, { i, DepthTextures.back().View });
		}
	}
#endif
	virtual void CreateDescriptorSet() override {
		VK::CreateDescriptorPool(DescriptorPools.emplace_back(), 0, {
#pragma region FRAME_OBJECT
			VkDescriptorPoolSize({.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, .descriptorCount = static_cast<uint32_t>(size(SwapchainImages)) }), //!< UB * N
#pragma endregion
			VkDescriptorPoolSize({.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, .descriptorCount = 1 }), //!< Sampler + Image0
			VkDescriptorPoolSize({.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, .descriptorCount = 1 }), //!< Sampler + Image1
		});
		const std::array DSLs = { DescriptorSetLayouts[0] };
		const VkDescriptorSetAllocateInfo DSAI = {
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
			.pNext = nullptr,
			.descriptorPool = DescriptorPools[0],
			.descriptorSetCount = static_cast<uint32_t>(size(DSLs)), .pSetLayouts = data(DSLs)
		};
#pragma region FRAME_OBJECT
		for (size_t i = 0; i < size(SwapchainImages); ++i) {
			VERIFY_SUCCEEDED(vkAllocateDescriptorSets(Device, &DSAI, &DescriptorSets.emplace_back()));
		}
#pragma endregion
	}
	virtual void UpdateDescriptorSet() override {
		VK::CreateDescriptorUpdateTemplate(DescriptorUpdateTemplates.emplace_back(), {
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
#pragma region FRAME_OBJECT
		for (size_t i = 0; i < size(SwapchainImages); ++i) {
			const DescriptorUpdateInfo DUI = {
				VkDescriptorBufferInfo({ .buffer = UniformBuffers[i].Buffer, .offset = 0, .range = VK_WHOLE_SIZE }),
				VkDescriptorImageInfo({ .sampler = VK_NULL_HANDLE, .imageView = DDSTextures[0].View, .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL }), //!< Sampler + Image0
				VkDescriptorImageInfo({ .sampler = VK_NULL_HANDLE, .imageView = DDSTextures[1].View, .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL }), //!< Sampler + Image1
			};
			vkUpdateDescriptorSetWithTemplate(Device, DescriptorSets[i], DescriptorUpdateTemplates[0], &DUI);
		}
#pragma endregion
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
		VkDescriptorBufferInfo DBI[1]; //!< UniformBuffer
		VkDescriptorImageInfo DII_0[1]; //!< Image0
		VkDescriptorImageInfo DII_1[1]; //!< Image1
	};
};
#pragma endregion