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
	virtual void DrawFrame([[maybe_unused]] const uint32_t i) override {
#pragma region FRAME_OBJECT
		//CopyToHostVisibleDeviceMemory(UniformBuffers[i].DeviceMemory, 0, sizeof(Tr), &Tr);
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
		constexpr VkDrawIndexedIndirectCommand DIIC = { .indexCount = 1, .instanceCount = 1, .firstIndex = 0, .vertexOffset = 0, .firstInstance = 0 };
		IndirectBuffers.emplace_back().Create(Device, PDMP, DIIC);
		VK::Scoped<StagingBuffer> Staging_Indirect0(Device);
		Staging_Indirect0.Create(Device, PDMP, sizeof(DIIC), &DIIC);
#pragma endregion

#pragma region PASS1
		//!< レンダーテクスチャ描画用
		constexpr VkDrawIndirectCommand DIC = { .vertexCount = 4, .instanceCount = 1, .firstVertex = 0, .firstInstance = 0 };
		IndirectBuffers.emplace_back().Create(Device, PDMP, DIC);
		VK::Scoped<StagingBuffer> Staging_Indirect1(Device);
		Staging_Indirect1.Create(Device, PDMP, sizeof(DIC), &DIC);
#pragma endregion

		constexpr VkCommandBufferBeginInfo CBBI = { .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO, .pNext = nullptr, .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, .pInheritanceInfo = nullptr };
		VERIFY_SUCCEEDED(vkBeginCommandBuffer(CB, &CBBI)); {
			IndirectBuffers[0].PopulateCopyCommand(CB, sizeof(DIIC), Staging_Indirect0.Buffer);
			IndirectBuffers[1].PopulateCopyCommand(CB, sizeof(DIC), Staging_Indirect1.Buffer);
		} VERIFY_SUCCEEDED(vkEndCommandBuffer(CB));
		VK::SubmitAndWait(GraphicsQueue, CB);
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
#ifdef DRAW_QUILT
			VK::CreateShaderModule(data(ShaderPath + TEXT("_1_Quilt") + TEXT(".frag.spv"))),
#else
			VK::CreateShaderModule(data(ShaderPath + TEXT("_1") + TEXT(".frag.spv"))),
#endif
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
	virtual void CreateDescriptor() override {
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

		VkDescriptorUpdateTemplate DUT;
#pragma region PASS0
		VK::CreateDescriptorUpdateTemplate(DUT, VK_PIPELINE_BIND_POINT_GRAPHICS, {
			VkDescriptorUpdateTemplateEntry({
				.dstBinding = 0, .dstArrayElement = 0,
				.descriptorCount = 1, .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
				.offset = 0, .stride = sizeof(VkDescriptorBufferInfo)
			}),
		}, DescriptorSetLayouts[0]);
#pragma region FRAME_OBJECT
		for (size_t i = 0; i < size(SwapchainImages); ++i) {
			const auto DBI = VkDescriptorBufferInfo({ .buffer = UniformBuffers[i].Buffer, .offset = 0, .range = VK_WHOLE_SIZE });
			vkUpdateDescriptorSetWithTemplate(Device, DescriptorSets[i], DUT, &DBI);
		}
		vkDestroyDescriptorUpdateTemplate(Device, DUT, GetAllocationCallbacks());
#pragma endregion
#pragma endregion

#pragma region PASS1
		VK::CreateDescriptorUpdateTemplate(DUT, VK_PIPELINE_BIND_POINT_GRAPHICS, {
			VkDescriptorUpdateTemplateEntry({
				.dstBinding = 0, .dstArrayElement = 0,
				.descriptorCount = 1, .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
				.offset = 0, .stride = sizeof(VkDescriptorImageInfo)
			}),
		}, DescriptorSetLayouts[1]);
		const auto DII = VkDescriptorImageInfo({.sampler = VK_NULL_HANDLE, .imageView = RenderTextures[0].View, .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL });
		vkUpdateDescriptorSetWithTemplate(Device, DescriptorSets[size(SwapchainImages)], DUT, &DII);
		vkDestroyDescriptorUpdateTemplate(Device, DUT, GetAllocationCallbacks());
#pragma endregion

#pragma region FRAME_OBJECT
		for (size_t i = 0; i < size(SwapchainImages); ++i) {
			CopyToHostVisibleDeviceMemory(UniformBuffers[i].DeviceMemory, 0, sizeof(Transform), &Transform);
		}
#pragma endregion
	}

	virtual void PopulateCommandBuffer(const size_t i) override {
#pragma region PASS0
		//!< メッシュ描画用
		const auto RP0 = RenderPasses[0];
		const auto FB0 = Framebuffers[0];
		const auto SCB0 = SecondaryCommandBuffers[i];
		{
			const VkCommandBufferInheritanceInfo CBII = {
				.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO,
				.pNext = nullptr,
				.renderPass = RP0,
				.subpass = 0,
				.framebuffer = FB0,
				.occlusionQueryEnable = VK_FALSE, .queryFlags = 0,
				.pipelineStatistics = 0,
			};
			const VkCommandBufferBeginInfo CBBI = {
				.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
				.pNext = nullptr,
				.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT,
				.pInheritanceInfo = &CBII
			};
			VERIFY_SUCCEEDED(vkBeginCommandBuffer(SCB0, &CBBI)); {
				const auto PLL = PipelineLayouts[0];
#pragma region FRAME_OBJECT
				const std::array DSs = { DescriptorSets[i] };
#pragma endregion
				constexpr std::array<uint32_t, 0> DynamicOffsets = {};
				vkCmdBindDescriptorSets(SCB0, VK_PIPELINE_BIND_POINT_GRAPHICS, PLL, 0, static_cast<uint32_t>(size(DSs)), data(DSs), static_cast<uint32_t>(size(DynamicOffsets)), data(DynamicOffsets));

				vkCmdBindPipeline(SCB0, VK_PIPELINE_BIND_POINT_GRAPHICS, Pipelines[0]);

				const auto ViewTotal = GetQuiltSetting().GetViewTotal();
				const auto ViewportMax = GetViewportMax();
				const auto Repeat = ViewTotal / ViewportMax + (std::min)(ViewTotal % ViewportMax, 1);
				for (auto j = 0; j < Repeat; ++j) {
					const auto Start = j * ViewportMax;
					const auto Count = (std::min)(ViewTotal - j * ViewportMax, ViewportMax);
#pragma region PUSH_CONSTANT
					QuiltDraw.ViewIndexOffset = Start;
					vkCmdPushConstants(SCB0, PLL, VK_SHADER_STAGE_GEOMETRY_BIT, 0, static_cast<uint32_t>(sizeof(QuiltDraw)), &QuiltDraw);
#pragma endregion
					vkCmdSetViewport(SCB0, 0, Count, &QuiltViewports[Start]);
					vkCmdSetScissor(SCB0, 0, Count, &QuiltScissorRects[Start]);
					vkCmdDrawIndirect(SCB0, IndirectBuffers[0].Buffer, 0, 1, 0);
				}
			} VERIFY_SUCCEEDED(vkEndCommandBuffer(SCB0));
		}
#pragma endregion

#pragma region PASS1
		//!< レンダーテクスチャ描画用
		const auto RP1 = RenderPasses[1];
		const auto FB1 = Framebuffers[i + 1];
		const auto SCCount = size(SwapchainImages);
		const auto SCB1 = SecondaryCommandBuffers[i + SCCount]; //!< オフセットさせる(ここでは2つのセカンダリコマンドバッファがぞれぞれスワップチェインイメージ数だけある)
		{
			const VkCommandBufferInheritanceInfo CBII = {
				.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO,
				.pNext = nullptr,
				.renderPass = RP1,
				.subpass = 0,
				.framebuffer = FB1,
				.occlusionQueryEnable = VK_FALSE, .queryFlags = 0,
				.pipelineStatistics = 0,
			};
			const VkCommandBufferBeginInfo CBBI = {
				.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
				.pNext = nullptr,
				.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT,
				.pInheritanceInfo = &CBII
			};
			VERIFY_SUCCEEDED(vkBeginCommandBuffer(SCB1, &CBBI)); {
				const auto PLL = PipelineLayouts[1];

				vkCmdSetViewport(SCB1, 0, static_cast<uint32_t>(size(Viewports)), data(Viewports));
				vkCmdSetScissor(SCB1, 0, static_cast<uint32_t>(size(ScissorRects)), data(ScissorRects));

				vkCmdBindPipeline(SCB1, VK_PIPELINE_BIND_POINT_GRAPHICS, Pipelines[1]);

				const std::array DSs = { DescriptorSets[size(SwapchainImages)] };
				constexpr std::array<uint32_t, 0> DynamicOffsets = {};
				vkCmdBindDescriptorSets(SCB1, VK_PIPELINE_BIND_POINT_GRAPHICS, PLL, 0, static_cast<uint32_t>(size(DSs)), data(DSs), static_cast<uint32_t>(size(DynamicOffsets)), data(DynamicOffsets));

#pragma region PUSH_CONSTANT
				vkCmdPushConstants(SCB1, PLL, VK_SHADER_STAGE_FRAGMENT_BIT, 0, static_cast<uint32_t>(sizeof(HoloDraw)), &HoloDraw);
#pragma endregion

				vkCmdDrawIndirect(SCB1, IndirectBuffers[1].Buffer, 0, 1, 0);
			} VERIFY_SUCCEEDED(vkEndCommandBuffer(SCB1));
		}
#pragma endregion

		const auto CB = CommandBuffers[i];
		constexpr VkCommandBufferBeginInfo CBBI = {
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
			.pNext = nullptr,
			.flags = 0,
			.pInheritanceInfo = nullptr
		};
		VERIFY_SUCCEEDED(vkBeginCommandBuffer(CB, &CBBI)); {

#pragma region PASS0
			//!< メッシュ描画用
			{
				const auto RenderArea = VkRect2D({ .offset = VkOffset2D({.x = 0, .y = 0 }), .extent = QuiltExtent2D });

				const std::array CVs = { VkClearValue({.color = Colors::SkyBlue }), VkClearValue({.depthStencil = {.depth = 1.0f, .stencil = 0 } }) };
				const VkRenderPassBeginInfo RPBI = {
					.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
					.pNext = nullptr,
					.renderPass = RP0,
					.framebuffer = FB0,
					.renderArea = RenderArea,
					.clearValueCount = static_cast<uint32_t>(size(CVs)), .pClearValues = data(CVs)
				};
				vkCmdBeginRenderPass(CB, &RPBI, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS); {
					const std::array SCBs = { SCB0 };
					vkCmdExecuteCommands(CB, static_cast<uint32_t>(size(SCBs)), data(SCBs));
				} vkCmdEndRenderPass(CB);
			}
#pragma endregion

			//!< リソースバリア : VK_ACCESS_COLOR_ATTACHMENT_READ_BIT -> VK_ACCESS_SHADER_READ_BIT
			{
				const std::array IMBs = {
					VkImageMemoryBarrier({
						.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
						.pNext = nullptr,
						.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT, .dstAccessMask = VK_ACCESS_SHADER_READ_BIT,
						.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, .newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
						.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED, .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
						.image = RenderTextures[0].Image,
						.subresourceRange = VkImageSubresourceRange({.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .baseMipLevel = 0, .levelCount = 1, .baseArrayLayer = 0, .layerCount = 1 })
					}),
				};
				vkCmdPipelineBarrier(CB,
					VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
					VK_DEPENDENCY_BY_REGION_BIT,
					0, nullptr,
					0, nullptr,
					static_cast<uint32_t>(size(IMBs)), data(IMBs));
			}

#pragma region PASS1
			//!< レンダーテクスチャ描画用
			{
				const auto RenderArea = VkRect2D({ .offset = VkOffset2D({.x = 0, .y = 0 }), .extent = SurfaceExtent2D });

				const std::array<VkClearValue, 0> CVs = {};
				const VkRenderPassBeginInfo RPBI = {
					.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
					.pNext = nullptr,
					.renderPass = RP1,
					.framebuffer = FB1,
					.renderArea = RenderArea,
					.clearValueCount = static_cast<uint32_t>(size(CVs)), .pClearValues = data(CVs)
				};
				vkCmdBeginRenderPass(CB, &RPBI, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS); {
					const std::array SCBs = { SCB1 };
					vkCmdExecuteCommands(CB, static_cast<uint32_t>(size(SCBs)), data(SCBs));
				} vkCmdEndRenderPass(CB);
			}
#pragma endregion

		} VERIFY_SUCCEEDED(vkEndCommandBuffer(CB));
	}

	virtual void CreateViewport(const float Width, const float Height, const float MinDepth = 0.0f, const float MaxDepth = 1.0f) override {
#pragma region PASS0
		{
			//!< VKではデフォルトで「Yが下」を向くが、高さに負の値を指定すると「Yが上」を向き、DXと同様になる (In VK, by specifying negative height, Y become up. same as DX)
			//!< 通常基点は「左上」を指定するが、高さに負の値を指定する場合は「左下」を指定すること (When negative height, specify left bottom as base, otherwise left up)

			//!< キルトの構造
			//!< ------------> RightMost
			//!< ---------------------->
			//!< ---------------------->
			//!< ---------------------->
			//!< LeftMost ------------->
			const auto& QS = GetQuiltSetting();
			const auto W = QS.GetViewWidth(), H = QS.GetViewHeight();
			for (auto i = 0; i < QS.GetViewRow(); ++i) {
				for (auto j = 0; j < QS.GetViewColumn(); ++j) {
					const auto X = j * W, Y = QS.GetHeight() - (i + 1) * H, BottomY = QS.GetHeight() - i * H;
					QuiltViewports.emplace_back(VkViewport({ .x = static_cast<float>(X), .y = static_cast<float>(BottomY), .width = static_cast<float>(W), .height = -static_cast<float>(H), .minDepth = MinDepth, .maxDepth = MaxDepth }));
					QuiltScissorRects.emplace_back(VkRect2D({ VkOffset2D({.x = X, .y = Y }), VkExtent2D({.width = static_cast<uint32_t>(W), .height = static_cast<uint32_t>(H) }) }));

					//!< そのまま出力する場合
					//Logf("QuiltViewport[%d] = (%d, %d) %d x %d\n", i * QS.GetViewColumn() + j, static_cast<int>(QuiltViewports.back().x), static_cast<int>(QuiltViewports.back().y), static_cast<int>(QuiltViewports.back().width), static_cast<int>(QuiltViewports.back().height));
					//!< 左上起点、高さを正の値で出力する場合
					Logf("QuiltViewport[%d] = (%d, %d) %d x %d\n", i * QS.GetViewColumn() + j, static_cast<int>(QuiltViewports.back().x), static_cast<int>(QuiltViewports.back().y + QuiltViewports.back().height), static_cast<int>(QuiltViewports.back().width), static_cast<int>(std::abs(QuiltViewports.back().height)));
				}
			}
		}
#pragma endregion

#pragma region PASS1
		{
			Super::CreateViewport(Width, Height, MinDepth, MaxDepth);
		}
#pragma endregion
	}
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

	VkExtent2D QuiltExtent2D;
	std::vector<VkViewport> QuiltViewports;
	std::vector<VkRect2D> QuiltScissorRects;
};
#pragma endregion