#pragma once

#include "resource.h"

#pragma region Code
#include "../VKExt.h"

class DeferredVK : public VKExtDepth
{
private:
	using Super = VKExtDepth;
public:
	DeferredVK() : Super() {}
	virtual ~DeferredVK() {}

protected:
	virtual void DrawFrame(const uint32_t i) override {
		Tr.World = glm::rotate(glm::mat4(1.0f), glm::radians(Degree), glm::vec3(1.0f, 0.0f, 0.0f));
		Degree += 1.0f;

#pragma region FRAME_OBJECT
		CopyToHostVisibleDeviceMemory(UniformBuffers[i].DeviceMemory, 0, sizeof(Tr), &Tr);
#pragma endregion
	}
	virtual void AllocateCommandBuffer() override {
		Super::AllocateCommandBuffer();
#pragma region FRAME_OBJECT
		const auto SCCount = static_cast<uint32_t>(size(SwapchainImages));
#pragma region PASS1
		const auto PrevCount = size(SecondaryCommandBuffers);
		SecondaryCommandBuffers.resize(PrevCount + SCCount);
		const VkCommandBufferAllocateInfo SCBAI = {
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
			.pNext = nullptr,
			.commandPool = SecondaryCommandPools[0],
			.level = VK_COMMAND_BUFFER_LEVEL_SECONDARY,
			.commandBufferCount = SCCount
		};
		VERIFY_SUCCEEDED(vkAllocateCommandBuffers(Device, &SCBAI, &SecondaryCommandBuffers[PrevCount]));
#pragma endregion
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
		constexpr auto Fov = 0.16f * std::numbers::pi_v<float>;
		const auto Aspect = GetAspectRatioOfClientRect();
		constexpr auto ZFar = 4.0f;
		constexpr auto ZNear = 2.0f;
		constexpr auto CamPos = glm::vec3(0.0f, 0.0f, 3.0f);
		constexpr auto CamTag = glm::vec3(0.0f);
		constexpr auto CamUp = glm::vec3(0.0f, 1.0f, 0.0f);
		const auto Projection = glm::perspective(Fov, Aspect, ZNear, ZFar);
		const auto View = glm::lookAt(CamPos, CamTag, CamUp);
		constexpr auto World = glm::mat4(1.0f);

		const auto ViewProjection = Projection * View;
		const auto InverseViewProjection = glm::inverse(ViewProjection);

		Tr = Transform({ Projection, View, World, InverseViewProjection });

#pragma region FRAME_OBJECT
		for (size_t i = 0; i < size(SwapchainImages); ++i) {
			UniformBuffers.emplace_back().Create(Device, GetCurrentPhysicalDeviceMemoryProperties(), sizeof(Tr));
		}
#pragma endregion
	}
	virtual void CreateTexture() override {
		const auto PDMP = GetCurrentPhysicalDeviceMemoryProperties();
		const auto Extent = VkExtent3D({ .width = SurfaceExtent2D.width, .height = SurfaceExtent2D.height, .depth = 1 });
		//!< カラー(Color)
		RenderTextures.emplace_back().Create(Device, PDMP, ColorFormat, Extent);
#pragma region MRT 
		//!< 法線(Normal)
		RenderTextures.emplace_back().Create(Device, PDMP, VK_FORMAT_A2B10G10R10_UNORM_PACK32, Extent);
		//!< 深度(Depth)
		RenderTextures.emplace_back().Create(Device, PDMP, VK_FORMAT_R32_SFLOAT, Extent);
		//!< 未定
		RenderTextures.emplace_back().Create(Device, PDMP, VK_FORMAT_B8G8R8A8_UNORM, Extent);
#pragma endregion
		//!< 深度バッファ(Depth Buffer)
		Super::CreateTexture();
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
			VK::CreatePipelineLayout(PipelineLayouts.emplace_back(), { DescriptorSetLayouts.back() }, {});
		}
#pragma endregion

#pragma region PASS1
		{
			const std::array ISs = { Samplers[0] };
			CreateDescriptorSetLayout(DescriptorSetLayouts.emplace_back(), 0, {
				//!< カラー(Color)
				VkDescriptorSetLayoutBinding({.binding = 0, .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, .descriptorCount = static_cast<uint32_t>(size(ISs)), .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT, .pImmutableSamplers = data(ISs) }),
#pragma region MRT 
				//!< 法線(Normal)
				VkDescriptorSetLayoutBinding({.binding = 1, .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, .descriptorCount = static_cast<uint32_t>(size(ISs)), .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT, .pImmutableSamplers = data(ISs) }),
				//!< 深度(Depth)
				VkDescriptorSetLayoutBinding({.binding = 2, .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, .descriptorCount = static_cast<uint32_t>(size(ISs)), .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT, .pImmutableSamplers = data(ISs) }),
				//!< 未定
				VkDescriptorSetLayoutBinding({.binding = 3, .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, .descriptorCount = static_cast<uint32_t>(size(ISs)), .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT, .pImmutableSamplers = data(ISs) }),
#pragma endregion
				VkDescriptorSetLayoutBinding({.binding = 4, .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, .descriptorCount = 1, .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT, .pImmutableSamplers = nullptr }),
				});
			VK::CreatePipelineLayout(PipelineLayouts.emplace_back(), { DescriptorSetLayouts.back() }, {});
		}
#pragma endregion
	}
	virtual void CreateRenderPass() override {
#pragma region PASS0
		{
			constexpr std::array CAs = {
				//!< カラー(Color)
				VkAttachmentReference({.attachment = 0, .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL }),
#pragma region MRT 
				//!< 法線(Normal)
				VkAttachmentReference({.attachment = 1, .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL }),
				//!< 深度(Depth)
				VkAttachmentReference({.attachment = 2, .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL }),
				//!< 未定
				VkAttachmentReference({.attachment = 3, .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL }),
#pragma endregion
			};
			constexpr VkAttachmentReference DA = { .attachment = static_cast<uint32_t>(size(CAs)), .layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL };
			VK::CreateRenderPass(RenderPasses.emplace_back(), {
				//!< カラー(Color)
				VkAttachmentDescription({
					.flags = 0,
					.format = ColorFormat,
					.samples = VK_SAMPLE_COUNT_1_BIT,
					.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR, .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
					.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE, .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
					.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED, .finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
				}),
#pragma region MRT 
				//!< 法線(Normal)
				VkAttachmentDescription({
					.flags = 0,
					.format = VK_FORMAT_A2B10G10R10_UNORM_PACK32,
					.samples = VK_SAMPLE_COUNT_1_BIT,
					.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR, .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
					.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE, .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
					.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED, .finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
				}),
				//!< 深度(Depth)
				VkAttachmentDescription({
					.flags = 0,
					.format = VK_FORMAT_R32_SFLOAT,
					.samples = VK_SAMPLE_COUNT_1_BIT,
					.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR, .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
					.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE, .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
					.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED, .finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
				}),
				//!< 未定
				VkAttachmentDescription({
					.flags = 0,
					.format = VK_FORMAT_B8G8R8A8_UNORM,
					.samples = VK_SAMPLE_COUNT_1_BIT,
					.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR, .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
					.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE, .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
					.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED, .finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
				}),
#pragma endregion
				//!< 深度バッファ(Depth Buffer)
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
				}, {});
		}
#pragma endregion

#pragma region PASS1
		VK::CreateRenderPass();
#pragma endregion
	}
	virtual void CreatePipeline() override {
		std::vector<std::thread> Threads;
		Pipelines.resize(2);
		const auto ShaderPath = GetBasePath();
#ifdef USE_PIPELINE_SERIALIZE
		PipelineCacheSerializer PCS(Device, GetBasePath() + TEXT(".pco"), 2);
#endif
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
		const std::vector<VkVertexInputBindingDescription> VIBDs = {};
		const std::vector<VkVertexInputAttributeDescription> VIADs = {};

#pragma region PASS0
		constexpr VkPipelineDepthStencilStateCreateInfo PDSSCI0 = {
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
		const std::vector PCBASs0 = {
			//!< カラー(Color)
			VkPipelineColorBlendAttachmentState({
				.blendEnable = VK_FALSE,
				.srcColorBlendFactor = VK_BLEND_FACTOR_ONE, .dstColorBlendFactor = VK_BLEND_FACTOR_ONE, .colorBlendOp = VK_BLEND_OP_ADD,
				.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE, .dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE, .alphaBlendOp = VK_BLEND_OP_ADD,
				.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
			}),
#pragma region MRT 
			//!< 法線(Normal)
			VkPipelineColorBlendAttachmentState({
				.blendEnable = VK_FALSE,
				.srcColorBlendFactor = VK_BLEND_FACTOR_ONE, .dstColorBlendFactor = VK_BLEND_FACTOR_ONE, .colorBlendOp = VK_BLEND_OP_ADD,
				.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE, .dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE, .alphaBlendOp = VK_BLEND_OP_ADD,
				.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
			}),
			//!< 深度(Depth)
			VkPipelineColorBlendAttachmentState({
				.blendEnable = VK_FALSE,
				.srcColorBlendFactor = VK_BLEND_FACTOR_ONE, .dstColorBlendFactor = VK_BLEND_FACTOR_ONE, .colorBlendOp = VK_BLEND_OP_ADD,
				.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE, .dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE, .alphaBlendOp = VK_BLEND_OP_ADD,
				.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
			}),
			//!< 未定
			VkPipelineColorBlendAttachmentState({
				.blendEnable = VK_FALSE,
				.srcColorBlendFactor = VK_BLEND_FACTOR_ONE, .dstColorBlendFactor = VK_BLEND_FACTOR_ONE, .colorBlendOp = VK_BLEND_OP_ADD,
				.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE, .dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE, .alphaBlendOp = VK_BLEND_OP_ADD,
				.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
			}),
#pragma endregion
		};
#ifdef USE_PIPELINE_SERIALIZE
		Threads.emplace_back(std::thread::thread(VK::CreatePipeline_, std::ref(Pipelines[0]), Device, PipelineLayouts[0], RenderPasses[0], VK_PRIMITIVE_TOPOLOGY_PATCH_LIST, 1, PRSCI, PDSSCI0, &PSSCIs0[0], &PSSCIs0[1], &PSSCIs0[2], &PSSCIs0[3], &PSSCIs0[4], VIBDs, VIADs, PCBASs0, PCS.GetPipelineCache(0)));
#else
		Threads.emplace_back(std::thread::thread(VK::CreatePipeline_, std::ref(Pipelines[0]), Device, PipelineLayouts[0], RenderPasses[0], VK_PRIMITIVE_TOPOLOGY_PATCH_LIST, 1, PRSCI, PDSSCI0, &PSSCIs0[0], &PSSCIs0[1], &PSSCIs0[2], &PSSCIs0[3], &PSSCIs0[4], VIBDs, VIADs, PCBASs0));
#endif
#pragma endregion

#pragma region PASS1
		constexpr VkPipelineDepthStencilStateCreateInfo PDSSCI1 = {
			.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
			.pNext = nullptr,
			.flags = 0,
			.depthTestEnable = VK_FALSE, .depthWriteEnable = VK_FALSE, .depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL,
			.depthBoundsTestEnable = VK_FALSE,
			.stencilTestEnable = VK_FALSE,
			.front = VkStencilOpState({.failOp = VK_STENCIL_OP_KEEP, .passOp = VK_STENCIL_OP_KEEP, .depthFailOp = VK_STENCIL_OP_KEEP, .compareOp = VK_COMPARE_OP_NEVER, .compareMask = 0, .writeMask = 0, .reference = 0 }),
			.back = VkStencilOpState({.failOp = VK_STENCIL_OP_KEEP, .passOp = VK_STENCIL_OP_KEEP, .depthFailOp = VK_STENCIL_OP_KEEP, .compareOp = VK_COMPARE_OP_ALWAYS, .compareMask = 0, .writeMask = 0, .reference = 0 }),
			.minDepthBounds = 0.0f, .maxDepthBounds = 1.0f
		};
		const std::array SMs1 = {
			VK::CreateShaderModule(data(ShaderPath + TEXT("_1") + TEXT(".vert.spv"))),
#ifdef USE_GBUFFER_VISUALIZE
			VK::CreateShaderModule(data(ShaderPath + TEXT("_gb_1") + TEXT(".frag.spv"))),
			VK::CreateShaderModule(data(ShaderPath + TEXT("_gb_1") + TEXT(".geom.spv"))),
#else
			VK::CreateShaderModule(data(ShaderPath + TEXT("_1") + TEXT(".frag.spv"))),
#endif
		};
		const std::array PSSCIs1 = {
#ifdef USE_GBUFFER_VISUALIZE
			VkPipelineShaderStageCreateInfo({.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, .pNext = nullptr, .flags = 0, .stage = VK_SHADER_STAGE_VERTEX_BIT, .module = SMs1[0], .pName = "main", .pSpecializationInfo = nullptr }),
			VkPipelineShaderStageCreateInfo({.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, .pNext = nullptr, .flags = 0, .stage = VK_SHADER_STAGE_FRAGMENT_BIT, .module = SMs1[1], .pName = "main", .pSpecializationInfo = nullptr }),
			VkPipelineShaderStageCreateInfo({.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, .pNext = nullptr, .flags = 0, .stage = VK_SHADER_STAGE_GEOMETRY_BIT, .module = SMs1[2], .pName = "main", .pSpecializationInfo = nullptr }),
#else
			VkPipelineShaderStageCreateInfo({.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, .pNext = nullptr, .flags = 0, .stage = VK_SHADER_STAGE_VERTEX_BIT, .module = SMs1[0], .pName = "main", .pSpecializationInfo = nullptr }),
			VkPipelineShaderStageCreateInfo({.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, .pNext = nullptr, .flags = 0, .stage = VK_SHADER_STAGE_FRAGMENT_BIT, .module = SMs1[1], .pName = "main", .pSpecializationInfo = nullptr }),
#endif
		};
		const std::vector PCBASs1 = {
			VkPipelineColorBlendAttachmentState({
				.blendEnable = VK_FALSE,
				.srcColorBlendFactor = VK_BLEND_FACTOR_ONE, .dstColorBlendFactor = VK_BLEND_FACTOR_ONE, .colorBlendOp = VK_BLEND_OP_ADD,
				.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE, .dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE, .alphaBlendOp = VK_BLEND_OP_ADD,
				.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
			}),
		};
#ifdef USE_PIPELINE_SERIALIZE
#ifdef USE_GBUFFER_VISUALIZE
		Threads.emplace_back(std::thread::thread(VK::CreatePipeline_, std::ref(Pipelines[1]), Device, PipelineLayouts[1], RenderPasses[1], VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP, 0, PRSCI, PDSSCI1, &PSSCIs1[0], &PSSCIs1[1], nullptr, nullptr, &PSSCIs1[2], VIBDs, VIADs, PCBASs1, PCS.GetPipelineCache(1)));
#else
		Threads.emplace_back(std::thread::thread(VK::CreatePipeline_, std::ref(Pipelines[1]), Device, PipelineLayouts[1], RenderPasses[1], VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP, 0, PRSCI, PDSSCI1, &PSSCIs1[0], &PSSCIs1[1], nullptr, nullptr, nullptr, VIBDs, VIADs, PCBASs1, PCS.GetPipelineCache(1)));
#endif
#else
#ifdef USE_GBUFFER_VISUALIZE
		Threads.emplace_back(std::thread::thread(VK::CreatePipeline_, std::ref(Pipelines[1]), Device, PipelineLayouts[1], RenderPasses[1], VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP, 0, PRSCI, PDSSCI1, &PSSCIs1[0], &PSSCIs1[1], nullptr, nullptr, &PSSCIs1[2], VIBDs, VIADs, PCBASs1));
#else
		Threads.emplace_back(std::thread::thread(VK::CreatePipeline_, std::ref(Pipelines[1]), Device, PipelineLayouts[1], RenderPasses[1], VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP, 0, PRSCI, PDSSCI1, &PSSCIs1[0], &PSSCIs1[1], nullptr, nullptr, nullptr, VIBDs, VIADs, PCBASs1));
#endif
#endif
#pragma endregion

		for (auto& i : Threads) { i.join(); }

		for (auto i : SMs0) { vkDestroyShaderModule(Device, i, GetAllocationCallbacks()); }
		for (auto i : SMs1) { vkDestroyShaderModule(Device, i, GetAllocationCallbacks()); }
	}
	virtual void CreateFramebuffer() override {
#pragma region PASS0
		{
			VK::CreateFramebuffer(Framebuffers.emplace_back(), RenderPasses[0], SurfaceExtent2D.width, SurfaceExtent2D.height, 1, {
				//!< カラー(Color)
				RenderTextures[0].View,
#pragma region MRT 
				//!< 法線(Normal)
				RenderTextures[1].View,
				//!< 深度(Depth)
				RenderTextures[2].View,
				//!< 未定
				RenderTextures[3].View,
#pragma endregion
				//!< 深度バッファ(Depth Buffer)
				DepthTextures.back().View,
				});
		}
#pragma endregion

#pragma region PASS1
		{
			for (auto i : SwapchainImageViews) {
				VK::CreateFramebuffer(Framebuffers.emplace_back(), RenderPasses[1], SurfaceExtent2D.width, SurfaceExtent2D.height, 1, { i });
			}
		}
#pragma endregion
	}
	virtual void CreateDescriptor() override {
		VK::CreateDescriptorPool(DescriptorPools.emplace_back(), 0, {
#pragma region FRAME_OBJECT
			VkDescriptorPoolSize({.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, .descriptorCount = static_cast<uint32_t>(size(SwapchainImages)) * 2 }), //!< UB * N * 2
#pragma endregion
#pragma region MRT 
			//!< レンダーターゲット : カラー(RenderTarget : Color), 法線(RenderTarget : Normal), 深度(RenderTarget : Depth), 未定
			VkDescriptorPoolSize({.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, .descriptorCount = 4 }), //!< CIS * 4
#pragma endregion
			});

#pragma region PASS0
		{
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

			VkDescriptorUpdateTemplate DUT;
			VK::CreateDescriptorUpdateTemplate(DUT, VK_PIPELINE_BIND_POINT_GRAPHICS, {
				VkDescriptorUpdateTemplateEntry({
					.dstBinding = 0, .dstArrayElement = 0,
					.descriptorCount = 1, .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
					.offset = 0, .stride = sizeof(VkDescriptorBufferInfo)
				}),
				}, DescriptorSetLayouts[0]);
			for (size_t i = 0; i < size(SwapchainImages); ++i) {
				const auto DBI = VkDescriptorBufferInfo({ .buffer = UniformBuffers[i].Buffer, .offset = 0, .range = VK_WHOLE_SIZE });
				vkUpdateDescriptorSetWithTemplate(Device, DescriptorSets[i], DUT, &DBI);
			}
			vkDestroyDescriptorUpdateTemplate(Device, DUT, GetAllocationCallbacks());
		}
#pragma endregion

#pragma region PASS1
		{
			const std::array DSLs = { DescriptorSetLayouts[1] };
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
#pragma endregion

#pragma region PASS1
		{
#pragma region FRAME_OBJECT
			struct DescriptorUpdateInfo
			{
				//!< レンダーターゲット : カラー(RenderTarget : Color)
				VkDescriptorImageInfo DII;
#pragma region MRT 
				//!< レンダーターゲット : 法線(RenderTarget : Normal)
				VkDescriptorImageInfo DII1;
				//!< レンダーターゲット : 深度(RenderTarget : Depth)
				VkDescriptorImageInfo DII2;
				//!< レンダーターゲット : 未定
				VkDescriptorImageInfo DII3;
#pragma endregion
				VkDescriptorBufferInfo DBI;
			};
			VkDescriptorUpdateTemplate DUT;
			VK::CreateDescriptorUpdateTemplate(DUT, VK_PIPELINE_BIND_POINT_GRAPHICS, {
				//!< レンダーターゲット : カラー(RenderTarget : Color)
				VkDescriptorUpdateTemplateEntry({
					.dstBinding = 0, .dstArrayElement = 0,
					.descriptorCount = 1, .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
					.offset = offsetof(DescriptorUpdateInfo, DII), .stride = sizeof(DescriptorUpdateInfo)
				}),
#pragma region MRT 
				//!< レンダーターゲット : 法線(RenderTarget : Normal)
				VkDescriptorUpdateTemplateEntry({
					.dstBinding = 1, .dstArrayElement = 0,
					.descriptorCount = 1, .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
					.offset = offsetof(DescriptorUpdateInfo, DII1), .stride = sizeof(DescriptorUpdateInfo)
				}),
				//!< レンダーターゲット : 深度(RenderTarget : Depth)
				VkDescriptorUpdateTemplateEntry({
					.dstBinding = 2, .dstArrayElement = 0,
					.descriptorCount = 1, .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
					.offset = offsetof(DescriptorUpdateInfo, DII2), .stride = sizeof(DescriptorUpdateInfo)
				}),
				//!< レンダーターゲット : 未定
				VkDescriptorUpdateTemplateEntry({
					.dstBinding = 3, .dstArrayElement = 0,
					.descriptorCount = 1, .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
					.offset = offsetof(DescriptorUpdateInfo, DII3), .stride = sizeof(DescriptorUpdateInfo)
				}),
#pragma endregion
				VkDescriptorUpdateTemplateEntry({
					.dstBinding = 4, .dstArrayElement = 0,
					.descriptorCount = 1, .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
					.offset = offsetof(DescriptorUpdateInfo, DBI), .stride = sizeof(DescriptorUpdateInfo)
				}),
				}, DescriptorSetLayouts[1]);
			for (size_t i = 0; i < size(SwapchainImages); ++i) {
				const DescriptorUpdateInfo DUI = {
					//!< レンダーターゲット : カラー(RenderTarget : Color)
					VkDescriptorImageInfo({.sampler = VK_NULL_HANDLE, .imageView = RenderTextures[0].View, .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL }),
#pragma region MRT 
					//!< レンダーターゲット : 法線(RenderTarget : Normal)
					VkDescriptorImageInfo({.sampler = VK_NULL_HANDLE, .imageView = RenderTextures[1].View, .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL }),
					//!< レンダーターゲット : 深度(RenderTarget : Depth)
					VkDescriptorImageInfo({.sampler = VK_NULL_HANDLE, .imageView = RenderTextures[2].View, .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL }),
					//!< レンダーターゲット : 未定
					VkDescriptorImageInfo({.sampler = VK_NULL_HANDLE, .imageView = RenderTextures[3].View, .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL }),
#pragma endregion
					VkDescriptorBufferInfo({.buffer = UniformBuffers[i].Buffer, .offset = 0, .range = VK_WHOLE_SIZE }),
				};
				vkUpdateDescriptorSetWithTemplate(Device, DescriptorSets[i + size(SwapchainImages)], DUT, &DUI);
			}
			vkDestroyDescriptorUpdateTemplate(Device, DUT, GetAllocationCallbacks());
#pragma endregion
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
				.occlusionQueryEnable = VK_FALSE,
				.queryFlags = 0,
				.pipelineStatistics = 0,
			};
			const VkCommandBufferBeginInfo CBBI = {
				.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
				.pNext = nullptr,
				.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT,
				.pInheritanceInfo = &CBII
			};
			VERIFY_SUCCEEDED(vkBeginCommandBuffer(SCB0, &CBBI)); {
				vkCmdSetViewport(SCB0, 0, static_cast<uint32_t>(size(Viewports)), data(Viewports));
				vkCmdSetScissor(SCB0, 0, static_cast<uint32_t>(size(ScissorRects)), data(ScissorRects));

				vkCmdBindPipeline(SCB0, VK_PIPELINE_BIND_POINT_GRAPHICS, Pipelines[0]);

#pragma region FRAME_OBJECT
				const std::array DSs = { DescriptorSets[i] };
#pragma endregion
				constexpr std::array<uint32_t, 0> DynamicOffsets = {};
				vkCmdBindDescriptorSets(SCB0, VK_PIPELINE_BIND_POINT_GRAPHICS, PipelineLayouts[0], 0, static_cast<uint32_t>(size(DSs)), data(DSs), static_cast<uint32_t>(size(DynamicOffsets)), data(DynamicOffsets));

				vkCmdDrawIndirect(SCB0, IndirectBuffers[0].Buffer, 0, 1, 0);
			} VERIFY_SUCCEEDED(vkEndCommandBuffer(SCB0));
		}
#pragma endregion

#pragma region PASS1
		//!< レンダーテクスチャ描画用
		const auto RP1 = RenderPasses[1];
		const auto FB1 = Framebuffers[i + 1];
#pragma region FRAME_OBJECT
		const auto SCCount = static_cast<uint32_t>(size(SwapchainImages));
#pragma endregion
		const auto SCB1 = SecondaryCommandBuffers[i + SCCount]; //!< オフセットさせる(ここでは2つのセカンダリコマンドバッファがぞれぞれスワップチェインイメージ数だけある)
		{
			const VkCommandBufferInheritanceInfo CBII = {
				.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO,
				.pNext = nullptr,
				.renderPass = RP1,
				.subpass = 0,
				.framebuffer = FB1,
				.occlusionQueryEnable = VK_FALSE,
				.queryFlags = 0,
				.pipelineStatistics = 0,
			};
			const VkCommandBufferBeginInfo CBBI = {
				.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
				.pNext = nullptr,
				.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT,
				.pInheritanceInfo = &CBII
			};
			VERIFY_SUCCEEDED(vkBeginCommandBuffer(SCB1, &CBBI)); {
				vkCmdSetViewport(SCB1, 0, static_cast<uint32_t>(size(Viewports)), data(Viewports));
				vkCmdSetScissor(SCB1, 0, static_cast<uint32_t>(size(ScissorRects)), data(ScissorRects));

				vkCmdBindPipeline(SCB1, VK_PIPELINE_BIND_POINT_GRAPHICS, Pipelines[1]);

#pragma region FRAME_OBJECT
				const std::array DSs = { DescriptorSets[i + SCCount] };
#pragma endregion
				constexpr std::array<uint32_t, 0> DynamicOffsets = {};
				vkCmdBindDescriptorSets(SCB1, VK_PIPELINE_BIND_POINT_GRAPHICS, PipelineLayouts[1], 0, static_cast<uint32_t>(size(DSs)), data(DSs), static_cast<uint32_t>(size(DynamicOffsets)), data(DynamicOffsets));

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
			const VkRect2D RenderArea = { .offset = VkOffset2D({.x = 0, .y = 0 }), .extent = SurfaceExtent2D };

#pragma region PASS0
			//!< メッシュ描画用
			{
				constexpr std::array CVs = {
					//!< カラー(Color)
					VkClearValue({.color = Colors::SkyBlue}),
	#pragma region MRT
					//!< 法線(Normal)
					VkClearValue({.color = VkClearColorValue({0.5f, 0.5f, 1.0f, 1.0f})}),
					//!< 深度(Depth)
					VkClearValue({.color = Colors::Red}),
					//!< 未定
					VkClearValue({.color = Colors::SkyBlue}),
	#pragma endregion
					VkClearValue({.depthStencil = VkClearDepthStencilValue({.depth = 1.0f, .stencil = 0 })}),
				};
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

			//!< リソースバリア : VK_ACCESS_COLOR_ATTACHMENT_READ_BIT -> VK_ACCESS_SHADER_READ_BIT -> SubpassDependencyで代用可能かも？
			{
				const std::array IMBs = {
					//!< カラー(Color)
					VkImageMemoryBarrier({
						.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
						.pNext = nullptr,
						.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT, .dstAccessMask = VK_ACCESS_SHADER_READ_BIT,
						.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, .newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
						.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED, .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
						.image = RenderTextures[0].Image,
						.subresourceRange = VkImageSubresourceRange({.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .baseMipLevel = 0, .levelCount = 1, .baseArrayLayer = 0, .layerCount = 1 })
					}),
	#pragma region MRT
					//!< 法線(Normal)
					VkImageMemoryBarrier({
						.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
						.pNext = nullptr,
						.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT, .dstAccessMask = VK_ACCESS_SHADER_READ_BIT,
						.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, .newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
						.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED, .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
						.image = RenderTextures[1].Image,
						.subresourceRange = VkImageSubresourceRange({.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .baseMipLevel = 0, .levelCount = 1, .baseArrayLayer = 0, .layerCount = 1 })
					}),
					//!< 深度(Depth)
					VkImageMemoryBarrier({
						.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
						.pNext = nullptr,
						.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT, .dstAccessMask = VK_ACCESS_SHADER_READ_BIT,
						.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, .newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
						.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED, .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
						.image = RenderTextures[2].Image,
						.subresourceRange = VkImageSubresourceRange({.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .baseMipLevel = 0, .levelCount = 1, .baseArrayLayer = 0, .layerCount = 1 })
					}),
					//!< 未定
					VkImageMemoryBarrier({
						.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
						.pNext = nullptr,
						.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT, .dstAccessMask = VK_ACCESS_SHADER_READ_BIT,
						.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, .newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
						.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED, .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
						.image = RenderTextures[3].Image,
						.subresourceRange = VkImageSubresourceRange({.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .baseMipLevel = 0, .levelCount = 1, .baseArrayLayer = 0, .layerCount = 1 })
					}),
	#pragma endregion
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

#ifdef USE_GBUFFER_VISUALIZE
	virtual void CreateViewport(const float Width, const float Height, const float MinDepth = 0.0f, const float MaxDepth = 1.0f) override {
		const auto W = Width * 0.5f, H = Height * 0.5f;
		Viewports = {
	#ifdef USE_VIEWPORT_Y_UP
			//!< 全画面用(Fullscreen)
			VkViewport({.x = 0.0f, .y = Height, .width = Width, .height = -Height, .minDepth = MinDepth, .maxDepth = MaxDepth }),
			//!< 分割画面用(DividedScreens)
			VkViewport({.x = 0.0f, .y = H, .width = W, .height = -H, .minDepth = MinDepth, .maxDepth = MaxDepth }),
			VkViewport({.x = W, .y = H, .width = W, .height = -H, .minDepth = MinDepth, .maxDepth = MaxDepth }),
			VkViewport({.x = 0.0f, .y = Height, .width = W, .height = -H, .minDepth = MinDepth, .maxDepth = MaxDepth }),
			VkViewport({.x = W, .y = Height, .width = W, .height = -H, .minDepth = MinDepth, .maxDepth = MaxDepth }),
	#else
			//!< 全画面用
			VkViewport({.x = 0.0f, .y = 0.0f, .width = Width, .height = Height, .minDepth = MinDepth, .maxDepth = MaxDepth }),
			//!< 分割画面用
			VkViewport({.x = 0.0f, .y = 0.0f, .width = W, .height = H, .minDepth = MinDepth, .maxDepth = MaxDepth }),
			VkViewport({.x = W, .y = 0.0f, .width = W, .height = H, .minDepth = MinDepth, .maxDepth = MaxDepth }),
			VkViewport({.x = 0.0f, .y = H, .width = W, .height = H, .minDepth = MinDepth, .maxDepth = MaxDepth }),
			VkViewport({.x = W, .y = H, .width = W, .height = H, .minDepth = MinDepth, .maxDepth = MaxDepth }),
	#endif
		};
		ScissorRects = {
			//!< 全画面用(Fullscreen)
			VkRect2D({.offset = VkOffset2D({.x = 0, .y = 0 }), .extent = VkExtent2D({.width = static_cast<uint32_t>(Width), .height = static_cast<uint32_t>(Height) }) }),
			//!< 分割画面用(DividedScreens)
			VkRect2D({.offset = VkOffset2D({.x = 0, .y = 0 }), .extent = VkExtent2D({.width = static_cast<uint32_t>(W), .height = static_cast<uint32_t>(H) }) }),
			VkRect2D({.offset = VkOffset2D({.x = static_cast<int32_t>(W), .y = 0 }), .extent = VkExtent2D({.width = static_cast<uint32_t>(W), .height = static_cast<uint32_t>(H) }) }),
			VkRect2D({.offset = VkOffset2D({.x = 0, .y = static_cast<int32_t>(H) }), .extent = VkExtent2D({.width = static_cast<uint32_t>(W), .height = static_cast<uint32_t>(H) }) }),
			VkRect2D({.offset = VkOffset2D({.x = static_cast<int32_t>(W), .y = static_cast<int32_t>(H) }), .extent = VkExtent2D({.width = static_cast<uint32_t>(W), .height = static_cast<uint32_t>(H) }) }),
		};
		LOG_OK();
	}
#endif

private:
	float Degree = 0.0f;
	struct Transform
	{
		glm::mat4 Projection;
		glm::mat4 View;
		glm::mat4 World;
		glm::mat4 InverseViewProjection;
	};
	using Transform = struct Transform;
	Transform Tr;
};
#pragma endregion