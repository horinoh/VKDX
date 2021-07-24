#pragma once

#include "resource.h"

#pragma region Code
#include "../VKExt.h"
#include "../Holo.h"

class HoloVK : public VKExt, public Holo
{
private:
	using Super = VKExt;
public:
	HoloVK() : Super(), Holo() 
	{
		const auto& QS = GetQuiltSetting();
		QuiltExtent2D = { .width = static_cast<uint32_t>(QS.GetWidth()), .height = static_cast<uint32_t>(QS.GetHeight()) };
	}
	virtual ~HoloVK() {}

protected:
	virtual void OnCreate(HWND hWnd, HINSTANCE hInstance, LPCWSTR Title) {
		SetWindow(hWnd);
		Super::OnCreate(hWnd, hInstance, Title);
	}
	virtual void OnTimer(HWND hWnd, HINSTANCE hInstance) override {
		Super::OnTimer(hWnd, hInstance); 
#pragma region FRAME_OBJECT
		//CopyToHostVisibleDeviceMemory(UniformBuffers[GetCurrentBackBufferIndex()].DeviceMemory, 0, sizeof(Tr), &Tr);
#pragma endregion
	}

	virtual void AllocateCommandBuffer() override {
		Super::AllocateCommandBuffer();
#pragma region PASS	1
		const auto SCCount = static_cast<uint32_t>(size(SwapchainImages));
		assert(!empty(SecondaryCommandPools) && "");
		const auto PrevCount = size(SecondaryCommandBuffers);
		SecondaryCommandBuffers.resize(PrevCount + SCCount);
		const VkCommandBufferAllocateInfo CBAI = {
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
			.pNext = nullptr,
			.commandPool = SecondaryCommandPools[0],
			.level = VK_COMMAND_BUFFER_LEVEL_SECONDARY,
			.commandBufferCount = SCCount
		};
		VERIFY_SUCCEEDED(vkAllocateCommandBuffers(Device, &CBAI, &SecondaryCommandBuffers[PrevCount]));
#pragma endregion
	}
	virtual void CreateGeometry() override {
		const auto PDMP = GetCurrentPhysicalDeviceMemoryProperties();
		const auto CB = CommandBuffers[0];
#pragma region PASS0
		//!< メッシュ描画用
		{
			constexpr VkDrawIndexedIndirectCommand DIIC = { .indexCount = 1, .instanceCount = 1, .firstIndex = 0, .vertexOffset = 0, .firstInstance = 0 };
			IndirectBuffers.emplace_back().Create(Device, PDMP, sizeof(DIIC)).SubmitCopyCommand(Device, PDMP, CB, GraphicsQueue, sizeof(DIIC), &DIIC);
		}
#pragma endregion

#pragma region PASS1
		//!< レンダーテクスチャ描画用
		{
			constexpr VkDrawIndirectCommand DIC = { .vertexCount = 4, .instanceCount = 1, .firstVertex = 0, .firstInstance = 0 };
			IndirectBuffers.emplace_back().Create(Device, PDMP, sizeof(DIC)).SubmitCopyCommand(Device, PDMP, CB, GraphicsQueue, sizeof(DIC), &DIC); 
		}
#pragma endregion
	}
	virtual void CreateUniformBuffer() override {
		constexpr auto Fov = glm::radians(14.0f);
		const auto Aspect = HoloDraw.DisplayAspect;
		constexpr auto ZFar = 100.0f;
		constexpr auto ZNear = 0.1f;

		constexpr auto CamPos = glm::vec3(0.0f, 0.0f, 7.0f);
		constexpr auto CamTag = glm::vec3(0.0f);
		constexpr auto CamUp = glm::vec3(0.0f, 1.0f, 0.0f);

		const auto Projection = glm::perspective(Fov, Aspect, ZNear, ZFar);
		const auto View = glm::lookAt(CamPos, CamTag, CamUp);
		const auto World = glm::mat4(1.0f);

		Transform.World = World;
		Transform.View = View;
		Transform.Projection = Projection;

#pragma region FRAME_OBJECT
		for (size_t i = 0; i < size(SwapchainImages); ++i) {
			UniformBuffers.emplace_back().Create(Device, GetCurrentPhysicalDeviceMemoryProperties(), sizeof(Transform));
		}
#pragma endregion
	}
	virtual void CreateTexture() override {
		const auto PDMP = GetCurrentPhysicalDeviceMemoryProperties();
		const auto Extent = VkExtent3D({ .width = QuiltExtent2D.width, .height = QuiltExtent2D.height, .depth = 1 });
		RenderTextures.emplace_back().Create(Device, PDMP, ColorFormat, Extent);
		DepthTextures.emplace_back().Create(Device, PDMP, DepthFormat, Extent);
	}
	virtual void CreateImmutableSampler() override {
#pragma region PASS1
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
#pragma endregion
	}

	virtual void CreatePipelineLayout() override {
#pragma region PASS0
		{
			CreateDescriptorSetLayout(DescriptorSetLayouts.emplace_back(), 0, {
				VkDescriptorSetLayoutBinding({.binding = 0, .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, .descriptorCount = 1, .stageFlags = VK_SHADER_STAGE_GEOMETRY_BIT, .pImmutableSamplers = nullptr }),
			});
			VK::CreatePipelineLayout(PipelineLayouts.emplace_back(), { DescriptorSetLayouts.back() }, {
	#pragma region PUSH_CONSTANT
				VkPushConstantRange({.stageFlags = VK_SHADER_STAGE_GEOMETRY_BIT, .offset = 0, .size = static_cast<uint32_t>(sizeof(QuiltDraw)) })
	#pragma endregion
			});
		}
#pragma endregion

#pragma region PASS1
		{
			const std::array ISs = { Samplers[0] };
			CreateDescriptorSetLayout(DescriptorSetLayouts.emplace_back(), 0, {
				VkDescriptorSetLayoutBinding({.binding = 0, .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, .descriptorCount = static_cast<uint32_t>(size(ISs)), .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT, .pImmutableSamplers = data(ISs) })
			});
			VK::CreatePipelineLayout(PipelineLayouts.emplace_back(), { DescriptorSetLayouts.back() }, {
	#pragma region PUSH_CONSTANT
				VkPushConstantRange({.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT, .offset = 0, .size = static_cast<uint32_t>(sizeof(HoloDraw)) })
	#pragma endregion
			});
		}
#pragma endregion
	}
	virtual void CreateRenderPass() override {
#pragma region PASS0
		{
			const std::array CAs = { VkAttachmentReference({.attachment = 0, .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL }), };
			const auto DA = VkAttachmentReference({ .attachment = static_cast<uint32_t>(size(CAs)), .layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL });
			VK::CreateRenderPass(RenderPasses.emplace_back(), {
				VkAttachmentDescription({
					.flags = 0,
					.format = ColorFormat,
					.samples = VK_SAMPLE_COUNT_1_BIT,
					.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR, .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
					.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE, .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
					.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED, .finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
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
					.colorAttachmentCount = static_cast<uint32_t>(size(CAs)), .pColorAttachments = data(CAs), .pResolveAttachments = nullptr,
					.pDepthStencilAttachment = &DA,
					.preserveAttachmentCount = 0, .pPreserveAttachments = nullptr
				}),
			}, { });
		}
#pragma endregion

#pragma region PASS1
		{
			const std::array CAs = { VkAttachmentReference({.attachment = 0, .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL }), };
			VK::CreateRenderPass(RenderPasses.emplace_back(), {
				VkAttachmentDescription({
					.flags = 0,
					.format = ColorFormat,
					.samples = VK_SAMPLE_COUNT_1_BIT,
					.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE, .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
					.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE, .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
					.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED, .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
				}),
			}, {
				VkSubpassDescription({
					.flags = 0,
					.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
					.inputAttachmentCount = 0, .pInputAttachments = nullptr,
					.colorAttachmentCount = static_cast<uint32_t>(size(CAs)), .pColorAttachments = data(CAs), .pResolveAttachments = nullptr,
					.pDepthStencilAttachment = nullptr,
					.preserveAttachmentCount = 0, .pPreserveAttachments = nullptr
				}),
			}, {});
		}
#pragma endregion
	}
	virtual void CreatePipeline() override {
		std::vector<std::thread> Threads;
		Pipelines.resize(2);
		const auto ShaderPath = GetBasePath();
#ifdef USE_PIPELINE_SERIALIZE
		PipelineCacheSerializer PCS(Device, GetBasePath() + TEXT(".pco"), 2);
#endif
		const VkPipelineRasterizationStateCreateInfo PRSCI = {
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
		const VkPipelineDepthStencilStateCreateInfo PDSSCI = {
			.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
			.pNext = nullptr,
			.flags = 0,
			.depthTestEnable = VK_TRUE, .depthWriteEnable = VK_TRUE, .depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL,
			.depthBoundsTestEnable = VK_FALSE,
			.stencilTestEnable = VK_FALSE,
			.front = VkStencilOpState({.failOp = VK_STENCIL_OP_KEEP, .passOp = VK_STENCIL_OP_KEEP, .depthFailOp = VK_STENCIL_OP_KEEP, .compareOp = VK_COMPARE_OP_NEVER, .compareMask = 0, .writeMask = 0, .reference = 0 }),
			.back = VkStencilOpState({.failOp = VK_STENCIL_OP_KEEP, .passOp = VK_STENCIL_OP_KEEP, .depthFailOp = VK_STENCIL_OP_KEEP, .compareOp = VK_COMPARE_OP_ALWAYS, .compareMask = 0, .writeMask = 0, .reference = 0 }),
			.minDepthBounds = 0.0f, .maxDepthBounds = 1.0f
		};
		const std::vector<VkVertexInputBindingDescription> VIBDs = {};
		const std::vector<VkVertexInputAttributeDescription> VIADs = {};
		const std::vector PCBASs = {
			VkPipelineColorBlendAttachmentState({
				.blendEnable = VK_FALSE,
				.srcColorBlendFactor = VK_BLEND_FACTOR_ONE, .dstColorBlendFactor = VK_BLEND_FACTOR_ONE, .colorBlendOp = VK_BLEND_OP_ADD,
				.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE, .dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE, .alphaBlendOp = VK_BLEND_OP_ADD,
				.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
			}),
		};

#pragma region PASS0
		const std::array SMs0 = {
			VK::CreateShaderModule(data(ShaderPath + TEXT(".vert.spv"))),
			VK::CreateShaderModule(data(ShaderPath + TEXT(".frag.spv"))),
			VK::CreateShaderModule(data(ShaderPath + TEXT(".tese.spv"))),
			VK::CreateShaderModule(data(ShaderPath + TEXT(".tesc.spv"))),
			VK::CreateShaderModule(data(ShaderPath + TEXT(".geom.spv"))),
		};
		const std::array PSSCIs0 = {
			VkPipelineShaderStageCreateInfo({.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, .pNext = nullptr, .flags = 0, .stage = VK_SHADER_STAGE_VERTEX_BIT, .module = SMs0[0], .pName = "main", .pSpecializationInfo = nullptr }),
			VkPipelineShaderStageCreateInfo({.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, .pNext = nullptr, .flags = 0, .stage = VK_SHADER_STAGE_FRAGMENT_BIT, .module = SMs0[1], .pName = "main", .pSpecializationInfo = nullptr }),
			VkPipelineShaderStageCreateInfo({.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, .pNext = nullptr, .flags = 0, .stage = VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT, .module = SMs0[2], .pName = "main", .pSpecializationInfo = nullptr }),
			VkPipelineShaderStageCreateInfo({.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, .pNext = nullptr, .flags = 0, .stage = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT, .module = SMs0[3], .pName = "main", .pSpecializationInfo = nullptr }),
			VkPipelineShaderStageCreateInfo({.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, .pNext = nullptr, .flags = 0, .stage = VK_SHADER_STAGE_GEOMETRY_BIT, .module = SMs0[4], .pName = "main", .pSpecializationInfo = nullptr }),
		};
#ifdef USE_PIPELINE_SERIALIZE
		Threads.emplace_back(std::thread::thread(VK::CreatePipeline_, std::ref(Pipelines[0]), Device, PipelineLayouts[0], RenderPasses[0], VK_PRIMITIVE_TOPOLOGY_PATCH_LIST, 1, PRSCI, PDSSCI, &PSSCIs0[0], &PSSCIs0[1], &PSSCIs0[2], &PSSCIs0[3], &PSSCIs0[4], VIBDs, VIADs, PCBASs, PCS.GetPipelineCache(0)));
#else
		Threads.emplace_back(std::thread::thread(VK::CreatePipeline_, std::ref(Pipelines[0]), Device, PipelineLayouts[0], RenderPasses[0], VK_PRIMITIVE_TOPOLOGY_PATCH_LIST, 1, PRSCI, PDSSCI, &PSSCIs0[0], &PSSCIs0[1], &PSSCIs0[2], &PSSCIs0[3], &PSSCIs0[4], VIBDs, VIADs, PCBASs));
#endif
#pragma endregion

#pragma region PASS1
		const std::array SMs1 = {
			VK::CreateShaderModule(data(ShaderPath + TEXT("_1") + TEXT(".vert.spv"))),
			VK::CreateShaderModule(data(ShaderPath + TEXT("_1") + TEXT(".frag.spv"))),
			//VK::CreateShaderModule(data(ShaderPath + TEXT("_1_Quilt") + TEXT(".frag.spv"))),
		};
		const std::array PSSCIs1 = {
			VkPipelineShaderStageCreateInfo({.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, .pNext = nullptr, .flags = 0, .stage = VK_SHADER_STAGE_VERTEX_BIT, .module = SMs1[0], .pName = "main", .pSpecializationInfo = nullptr }),
			VkPipelineShaderStageCreateInfo({.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, .pNext = nullptr, .flags = 0, .stage = VK_SHADER_STAGE_FRAGMENT_BIT, .module = SMs1[1], .pName = "main", .pSpecializationInfo = nullptr }),
		};
#ifdef USE_PIPELINE_SERIALIZE
		Threads.emplace_back(std::thread::thread(VK::CreatePipeline_, std::ref(Pipelines[1]), Device, PipelineLayouts[1], RenderPasses[1], VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP, 0, PRSCI, PDSSCI, &PSSCIs1[0], &PSSCIs1[1], nullptr, nullptr, nullptr, VIBDs, VIADs, PCBASs, PCS.GetPipelineCache(1)));
#else
		Threads.emplace_back(std::thread::thread(VK::CreatePipeline_, std::ref(Pipelines[1]), Device, PipelineLayouts[1], RenderPasses[1], VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP, 0, PRSCI, PDSSCI, &PSSCIs1[0], &PSSCIs1[1], nullptr, nullptr, nullptr, VIBDs, VIADs, PCBASs));
#endif
#pragma endregion

		for (auto& i : Threads) { i.join(); }

		for (auto i : SMs0) { vkDestroyShaderModule(Device, i, GetAllocationCallbacks()); }
		for (auto i : SMs1) { vkDestroyShaderModule(Device, i, GetAllocationCallbacks()); }
	}
	virtual void CreateFramebuffer() override {
#pragma region PASS0
		VK::CreateFramebuffer(Framebuffers.emplace_back(), RenderPasses[0], QuiltExtent2D.width, QuiltExtent2D.height, 1, {
			RenderTextures[0].View,
			DepthTextures.back().View
		});
#pragma endregion

#pragma region PASS1
		for (auto i : SwapchainImageViews) {
			VK::CreateFramebuffer(Framebuffers.emplace_back(), RenderPasses[1], SurfaceExtent2D.width, SurfaceExtent2D.height, 1, { i });
		}
#pragma endregion
	}
	virtual void CreateDescriptorSet() override {
#pragma region PASS0
		{
			VK::CreateDescriptorPool(DescriptorPools.emplace_back(), 0, {
#pragma region FRAME_OBJECT
				VkDescriptorPoolSize({.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, .descriptorCount = static_cast<uint32_t>(size(SwapchainImages)) }) //!< UB * N
#pragma endregion
			});

			const std::array DSLs = { DescriptorSetLayouts[0] };
			const VkDescriptorSetAllocateInfo DSAI = {
				.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
				.pNext = nullptr,
				.descriptorPool = DescriptorPools.back(),
				.descriptorSetCount = static_cast<uint32_t>(size(DSLs)), .pSetLayouts = data(DSLs)
			};
#pragma region FRAME_OBJECT
			for (size_t i = 0; i < size(SwapchainImages); ++i) {
				VERIFY_SUCCEEDED(vkAllocateDescriptorSets(Device, &DSAI, &DescriptorSets.emplace_back()));
			}
#pragma endregion
		}
#pragma endregion

#pragma region PASS1
		{
			VKExt::CreateDescriptorPool(DescriptorPools.emplace_back(), 0, {
				VkDescriptorPoolSize({.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, .descriptorCount = 1 })
			});

			const std::array DSLs = { DescriptorSetLayouts[1] };
			const VkDescriptorSetAllocateInfo DSAI = {
				.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
				.pNext = nullptr,
				.descriptorPool = DescriptorPools.back(),
				.descriptorSetCount = static_cast<uint32_t>(size(DSLs)), .pSetLayouts = data(DSLs)
			};
			VERIFY_SUCCEEDED(vkAllocateDescriptorSets(Device, &DSAI, &DescriptorSets.emplace_back()));
		}
#pragma endregion
	}
	virtual void UpdateDescriptorSet() override {
#pragma region PASS0
		VK::CreateDescriptorUpdateTemplate(DescriptorUpdateTemplates.emplace_back(), {
			VkDescriptorUpdateTemplateEntry({
				.dstBinding = 0, .dstArrayElement = 0,
				.descriptorCount = _countof(DescriptorUpdateInfo_0::DescriptorBufferInfos), .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
				.offset = offsetof(DescriptorUpdateInfo_0, DescriptorBufferInfos), .stride = sizeof(DescriptorUpdateInfo_0)
			}),
		}, DescriptorSetLayouts[0]);
#pragma region FRAME_OBJECT
		for (size_t i = 0; i < size(SwapchainImages); ++i) {
			const DescriptorUpdateInfo_0 DUI = {
				VkDescriptorBufferInfo({.buffer = UniformBuffers[i].Buffer, .offset = 0, .range = VK_WHOLE_SIZE }),
			};
			vkUpdateDescriptorSetWithTemplate(Device, DescriptorSets[i], DescriptorUpdateTemplates.back(), &DUI);
		}
#pragma endregion
#pragma endregion

#pragma region PASS1
		VK::CreateDescriptorUpdateTemplate(DescriptorUpdateTemplates.emplace_back(), {
			VkDescriptorUpdateTemplateEntry({
				.dstBinding = 0, .dstArrayElement = 0,
				.descriptorCount = _countof(DescriptorUpdateInfo_1::DescriptorImageInfos), .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
				.offset = offsetof(DescriptorUpdateInfo_1, DescriptorImageInfos), .stride = sizeof(DescriptorUpdateInfo_1)
			}),
		}, DescriptorSetLayouts[1]);
		const DescriptorUpdateInfo_1 DUI = {
			VkDescriptorImageInfo({.sampler = VK_NULL_HANDLE, .imageView = RenderTextures[0].View, .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL }),
		};
		vkUpdateDescriptorSetWithTemplate(Device, DescriptorSets[size(SwapchainImages)], DescriptorUpdateTemplates.back(), &DUI);
#pragma endregion

#pragma region FRAME_OBJECT
		for (size_t i = 0; i < size(SwapchainImages); ++i) {
			CopyToHostVisibleDeviceMemory(UniformBuffers[i].DeviceMemory, 0, sizeof(Transform), &Transform);
		}
#pragma endregion
	}

	virtual void PopulateCommandBuffer(const size_t i) override;

	virtual void CreateViewport(const float Width, const float Height, const float MinDepth = 0.0f, const float MaxDepth = 1.0f) override;
	virtual int GetViewportMax() const override {
		VkPhysicalDeviceProperties PDP;
		vkGetPhysicalDeviceProperties(GetCurrentPhysicalDevice(), &PDP);
		return PDP.limits.maxViewports;
	}
private:
	struct TRANSFORM
	{
		glm::mat4 Projection;
		glm::mat4 View;
		glm::mat4 World;	
	};
	using TRANSFORM = struct TRANSFORM;
	TRANSFORM Transform;

	struct DescriptorUpdateInfo_0
	{
		VkDescriptorBufferInfo DescriptorBufferInfos[1];
	}; 
	struct DescriptorUpdateInfo_1
	{
		VkDescriptorImageInfo DescriptorImageInfos[1];
	};

	VkExtent2D QuiltExtent2D;
	std::vector<VkViewport> QuiltViewports;
	std::vector<VkRect2D> QuiltScissorRects;
};
#pragma endregion