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
#ifdef USE_PUSH_DESCRIPTOR
	virtual void CreateDevice(HWND hWnd, HINSTANCE hInstance, void* pNext, [[maybe_unused]] const std::vector<const char*>& AdditionalExtensions) override { 
		Super::CreateDevice(hWnd, hInstance, pNext, { VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME }); 
}
#endif
	virtual void OnTimer(HWND hWnd, HINSTANCE hInstance) override {
		Super::OnTimer(hWnd, hInstance);

		Tr.World = glm::rotate(glm::mat4(1.0f), glm::radians(Degree), glm::vec3(1.0f, 0.0f, 0.0f));
		Degree += 1.0f;

#pragma region FRAME_OBJECT
		//CopyToHostVisibleDeviceMemory(UniformBuffers[GetCurrentBackBufferIndex()].DeviceMemory, 0, sizeof(Tr), &Tr, offsetof(Transform, World), sizeof(Transform::World));
		CopyToHostVisibleDeviceMemory(UniformBuffers[GetCurrentBackBufferIndex()].DeviceMemory, 0, sizeof(Tr), &Tr);
#pragma endregion
	}

	virtual void CreateGeometry() override {
		const auto PDMP = GetCurrentPhysicalDeviceMemoryProperties();
		constexpr VkDrawIndexedIndirectCommand DIIC = { .indexCount = 1, .instanceCount = 1, .firstIndex = 0, .vertexOffset = 0, .firstInstance = 0 };
		IndirectBuffers.emplace_back().Create(Device, PDMP, DIIC).SubmitCopyCommand(Device, PDMP, CommandBuffers[0], GraphicsQueue, sizeof(DIIC), &DIIC);
	}
	virtual void CreateUniformBuffer() override {
		//const auto Fov = 0.16f * glm::pi<float>();
		constexpr auto Fov = 0.16f * std::numbers::pi_v<float>;
		const auto Aspect = GetAspectRatioOfClientRect();
		constexpr auto ZFar = 100.0f;
		constexpr auto ZNear = ZFar * 0.0001f;
		constexpr auto CamPos = glm::vec3(0.0f, 0.0f, 6.0f);
		constexpr auto CamTag = glm::vec3(0.0f);
		constexpr auto CamUp = glm::vec3(0.0f, 1.0f, 0.0f);
		Tr = Transform({ glm::perspective(Fov, Aspect, ZNear, ZFar), glm::lookAt(CamPos, CamTag, CamUp), glm::mat4(1.0f) });
#pragma region FRAME_OBJECT
		for (size_t i = 0; i < size(SwapchainImages); ++i) {
			UniformBuffers.emplace_back().Create(Device, GetCurrentPhysicalDeviceMemoryProperties(), sizeof(Tr));
		}
#pragma endregion
	}
	virtual void CreateTexture() override {
		DepthTextures.emplace_back().Create(Device, GetCurrentPhysicalDeviceMemoryProperties(), DepthFormat, VkExtent3D({ .width = SurfaceExtent2D.width, .height = SurfaceExtent2D.height, .depth = 1 }));
	}
	virtual void CreatePipelineLayout() override {
		CreateDescriptorSetLayout(DescriptorSetLayouts.emplace_back(),
#ifdef USE_PUSH_DESCRIPTOR
			VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR,
#else
			0,
#endif
			{
				VkDescriptorSetLayoutBinding({.binding = 0, .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, .descriptorCount = 1, .stageFlags = VK_SHADER_STAGE_GEOMETRY_BIT, .pImmutableSamplers = nullptr })
			});
		VKExt::CreatePipelineLayout(PipelineLayouts.emplace_back(), DescriptorSetLayouts, {});
	}
	virtual void CreateRenderPass() override { VKExt::CreateRenderPass_Depth(); }
	virtual void CreatePipeline() override {
		const auto ShaderPath = GetBasePath();
		const std::array SMs = {
			VK::CreateShaderModule(data(ShaderPath + TEXT(".vert.spv"))),
			VK::CreateShaderModule(data(ShaderPath + TEXT(".frag.spv"))),
			VK::CreateShaderModule(data(ShaderPath + TEXT(".tese.spv"))),
			VK::CreateShaderModule(data(ShaderPath + TEXT(".tesc.spv"))),
			VK::CreateShaderModule(data(ShaderPath + TEXT(".geom.spv"))),
		};
		const std::array PSSCIs = {
			VkPipelineShaderStageCreateInfo({.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, .pNext = nullptr, .flags = 0, .stage = VK_SHADER_STAGE_VERTEX_BIT, .module = SMs[0], .pName = "main", .pSpecializationInfo = nullptr }),
			VkPipelineShaderStageCreateInfo({.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, .pNext = nullptr, .flags = 0, .stage = VK_SHADER_STAGE_FRAGMENT_BIT, .module = SMs[1], .pName = "main", .pSpecializationInfo = nullptr }),
			VkPipelineShaderStageCreateInfo({.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, .pNext = nullptr, .flags = 0, .stage = VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT, .module = SMs[2], .pName = "main", .pSpecializationInfo = nullptr }),
			VkPipelineShaderStageCreateInfo({.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, .pNext = nullptr, .flags = 0, .stage = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT, .module = SMs[3], .pName = "main", .pSpecializationInfo = nullptr }),
			VkPipelineShaderStageCreateInfo({.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, .pNext = nullptr, .flags = 0, .stage = VK_SHADER_STAGE_GEOMETRY_BIT, .module = SMs[4], .pName = "main", .pSpecializationInfo = nullptr }),
		};

		constexpr VkPipelineRasterizationStateCreateInfo PRSCI = {
			.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
			.pNext = nullptr,
			.flags = 0,
			.depthClampEnable = VK_FALSE,
			.rasterizerDiscardEnable = VK_FALSE,
			.polygonMode = VK_POLYGON_MODE_FILL,
			.cullMode = VK_CULL_MODE_BACK_BIT,
			.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
			.depthBiasEnable = VK_FALSE, .depthBiasConstantFactor = 0.0f, .depthBiasClamp = 0.0f, .depthBiasSlopeFactor = 0.0f,
			.lineWidth = 1.0f
		};
		CreatePipeline_VsFsTesTcsGs(VK_PRIMITIVE_TOPOLOGY_PATCH_LIST, 1, PRSCI, VK_TRUE, PSSCIs);
		
		for (auto i : SMs) { vkDestroyShaderModule(Device, i, GetAllocationCallbacks()); }
	}
	virtual void CreateFramebuffer() override { 
		for (auto i : SwapchainImageViews) {
			VK::CreateFramebuffer(Framebuffers.emplace_back(), RenderPasses[0], SurfaceExtent2D.width, SurfaceExtent2D.height, 1, { i, DepthTextures.back().View });
		}
	}
#ifndef USE_PUSH_DESCRIPTOR
	virtual void CreateDescriptorSet() override {
		VK::CreateDescriptorPool(DescriptorPools.emplace_back(), /*VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT*/0, {
#pragma region FRAME_OBJECT
			VkDescriptorPoolSize({.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, .descriptorCount = static_cast<uint32_t>(size(SwapchainImages)) }), //!< UB * N
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
		for (size_t i = 0; i < size(SwapchainImages); ++i) {
			VERIFY_SUCCEEDED(vkAllocateDescriptorSets(Device, &DSAI, &DescriptorSets.emplace_back()));
		}
#pragma endregion
	}
#endif
	virtual void UpdateDescriptorSet() override {
#ifdef USE_PUSH_DESCRIPTOR
		const std::array DUTEs = {
			VkDescriptorUpdateTemplateEntry({
				.dstBinding = 0, .dstArrayElement = 0,
				.descriptorCount = _countof(DescriptorUpdateInfo::DBI), .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
				.offset = offsetof(DescriptorUpdateInfo, DBI), .stride = sizeof(DescriptorUpdateInfo)
			})
		};
		const VkDescriptorUpdateTemplateCreateInfo DUTCI = {
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_UPDATE_TEMPLATE_CREATE_INFO,
			.pNext = nullptr,
			.flags = 0,
			.descriptorUpdateEntryCount = static_cast<uint32_t>(size(DUTEs)), .pDescriptorUpdateEntries = data(DUTEs),
			.templateType = VK_DESCRIPTOR_UPDATE_TEMPLATE_TYPE_PUSH_DESCRIPTORS_KHR, //!< VK_DESCRIPTOR_UPDATE_TEMPLATE_TYPE_PUSH_DESCRIPTORS_KHR を指定
			.descriptorSetLayout = DescriptorSetLayouts[0],
			.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
			.pipelineLayout = PipelineLayouts[0], .set = 0 //!< (VK_DESCRIPTOR_UPDATE_TEMPLATE_TYPE_PUSH_DESCRIPTORS_KHR使用時は)パイプラインレイアウトを指定
		};
		VERIFY_SUCCEEDED(vkCreateDescriptorUpdateTemplate(Device, &DUTCI, GetAllocationCallbacks(), &DescriptorUpdateTemplates.emplace_back()));
#else
		VK::CreateDescriptorUpdateTemplate(DescriptorUpdateTemplates.emplace_back(), {
			VkDescriptorUpdateTemplateEntry({
				.dstBinding = 0, .dstArrayElement = 0,
				.descriptorCount = _countof(DescriptorUpdateInfo::DBI), .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
				.offset = offsetof(DescriptorUpdateInfo, DBI), .stride = sizeof(DescriptorUpdateInfo)
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
#endif
	}

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

	struct DescriptorUpdateInfo 
	{
		VkDescriptorBufferInfo DBI[1];
	};
};
#pragma endregion