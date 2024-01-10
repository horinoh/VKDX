#pragma once

#include "resource.h"

#pragma region Code
#include "../VKExt.h"

#ifdef USE_DEPTH
class RenderTargetVK : public VKExtDepth
#else
class RenderTargetVK : public VKExt
#endif
{
private:
#ifdef USE_DEPTH
	using Super = VKExtDepth;
#else
	using Super = VKExt;
#endif
public:
	RenderTargetVK() : Super() {}
	virtual ~RenderTargetVK() {}

protected:
	virtual void AllocateCommandBuffer() override {
		Super::AllocateCommandBuffer();

#pragma region PASS1
		const auto SCP = SecondaryCommandPools[0];
		const auto Count = static_cast<uint32_t>(size(SwapchainImages));
		assert(!empty(SecondaryCommandPools) && "");
		const auto Prev = size(SecondaryCommandBuffers);
		SecondaryCommandBuffers.resize(Prev + Count);
		const VkCommandBufferAllocateInfo CBAI = {
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
			.pNext = nullptr,
			.commandPool = SCP,
			.level = VK_COMMAND_BUFFER_LEVEL_SECONDARY,
			.commandBufferCount = Count
		};
		VERIFY_SUCCEEDED(vkAllocateCommandBuffers(Device, &CBAI, &SecondaryCommandBuffers[Prev]));
#pragma endregion
	}
	virtual void CreateGeometry() override {
		const auto PDMP = GetCurrentPhysicalDeviceMemoryProperties();
		const auto CB = CommandBuffers[0];

#pragma region PASS0
		//!< メッシュ描画用
		constexpr VkDrawIndexedIndirectCommand DIIC = { .indexCount = 1, .instanceCount = 1, .firstIndex = 0, .vertexOffset = 0, .firstInstance = 0 };
		IndirectBuffers.emplace_back().Create(Device, PDMP, DIIC);
		VK::Scoped<StagingBuffer> Staging0(Device);
		Staging0.Create(Device, PDMP, sizeof(DIIC), &DIIC);
#pragma endregion

#pragma region PASS1
		//!< レンダーテクスチャ描画用
		constexpr VkDrawIndirectCommand DIC = { .vertexCount = 4, .instanceCount = 1, .firstVertex = 0, .firstInstance = 0 };
		IndirectBuffers.emplace_back().Create(Device, PDMP, DIC);
		VK::Scoped<StagingBuffer> Staging1(Device);
		Staging1.Create(Device, PDMP, sizeof(DIC), &DIC); 
#pragma endregion

		constexpr VkCommandBufferBeginInfo CBBI = { .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO, .pNext = nullptr, .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, .pInheritanceInfo = nullptr };
		VERIFY_SUCCEEDED(vkBeginCommandBuffer(CB, &CBBI)); {
			IndirectBuffers[0].PopulateCopyCommand(CB, sizeof(DIIC), Staging0.Buffer);
			IndirectBuffers[1].PopulateCopyCommand(CB, sizeof(DIC), Staging1.Buffer);
		} VERIFY_SUCCEEDED(vkEndCommandBuffer(CB));
		VK::SubmitAndWait(GraphicsQueue, CB);
	}
	virtual void CreateTexture() override {
		CreateTexture_Render();
#ifdef USE_DEPTH
		Super::CreateTexture();
#endif
	}
	virtual void CreateImmutableSampler() override {
#pragma region PASS1
		CreateImmutableSampler_LinearRepeat();
#pragma endregion
	}
	virtual void CreatePipelineLayout() override {
#pragma region PASS0
		VK::CreatePipelineLayout(PipelineLayouts.emplace_back(), {}, {});
#pragma endregion

#pragma region PASS1
#ifdef USE_SUBPASS
		const std::array<VkSampler, 0> ISs = {};
		CreateDescriptorSetLayout(DescriptorSetLayouts.emplace_back(), 0, {
			VkDescriptorSetLayoutBinding({.binding = 0, .descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, .descriptorCount = static_cast<uint32_t>(size(ISs)), .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT, .pImmutableSamplers = data(ISs) })
		});
#endif
		const std::array ISs = { Samplers[0] };
		CreateDescriptorSetLayout(DescriptorSetLayouts.emplace_back(), 0, {
			VkDescriptorSetLayoutBinding({.binding = 0, .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, .descriptorCount = static_cast<uint32_t>(size(ISs)), .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT, .pImmutableSamplers = data(ISs) })
		});
		VK::CreatePipelineLayout(PipelineLayouts.emplace_back(), DescriptorSetLayouts, {});
#pragma endregion
	}
	virtual void CreateRenderPass() override {
#ifdef USE_SUBPASS
		//!< #VK_TODO
		const std::array ColorAttach = { VkAttachmentReference({ 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL }), };
		const std::array InputAttach = { VkAttachmentReference({ 1, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL }), };
		const std::array ColorAttach1 = { VkAttachmentReference({ 2, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL }), };
		VK::CreateRenderPass(RenderPasses.emplace_back(), {
			{
				0,
				ColorFormat,
				VK_SAMPLE_COUNT_1_BIT,
				VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE,
				VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
				VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
			},
			}, {
				{
					0,
					VK_PIPELINE_BIND_POINT_GRAPHICS,
					0, nullptr,
					static_cast<uint32_t>(size(ColorAttach)), data(ColorAttach), nullptr,
					nullptr,
					0, nullptr
				},
				{
					0,
					VK_PIPELINE_BIND_POINT_GRAPHICS,
					static_cast<uint32_t>(size(InputAttach)), data(InputAttach),
					static_cast<uint32_t>(size(ColorAttach)), data(ColorAttach), nullptr,
					nullptr,
					0, nullptr
				},

			}, {});
#endif
	
#pragma region PASS0
		{
			constexpr std::array CAs = { VkAttachmentReference({.attachment = 0, .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL }), };
			constexpr std::array RAs = { VkAttachmentReference({.attachment = VK_ATTACHMENT_UNUSED, .layout = VK_IMAGE_LAYOUT_UNDEFINED }), };
			constexpr std::array<uint32_t, 0> PAs = {};
#ifdef USE_DEPTH
			constexpr auto DA = VkAttachmentReference({ .attachment = static_cast<uint32_t>(size(CAs)), .layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL });
#endif
			VK::CreateRenderPass(RenderPasses.emplace_back(), {
				VkAttachmentDescription({
					.flags = 0,
					.format = ColorFormat,
					.samples = VK_SAMPLE_COUNT_1_BIT,
					.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR, .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
					.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE, .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
					.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED, .finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL //!< PRESENT_SRC_KHR ではなく、COLOR_ATTACHMENT_OPTIMAL
				}),
#ifdef USE_DEPTH
				VkAttachmentDescription({
					.flags = 0,
					.format = DepthFormat,
					.samples = VK_SAMPLE_COUNT_1_BIT,
					.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR, .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
					.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE, .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
					.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED, .finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
				}),
#endif
			}, {
				VkSubpassDescription({
					.flags = 0,
					.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
					.inputAttachmentCount = 0, .pInputAttachments = nullptr,
					.colorAttachmentCount = static_cast<uint32_t>(size(CAs)), .pColorAttachments = data(CAs), .pResolveAttachments = data(RAs),
#ifdef USE_DEPTH
					.pDepthStencilAttachment = &DA,
#else
					.pDepthStencilAttachment = nullptr,
#endif
					.preserveAttachmentCount = static_cast<uint32_t>(size(PAs)), .pPreserveAttachments = data(PAs)
				}),
			}, {});
		}
#pragma endregion

#pragma region PASS1
		VK::CreateRenderPass();
#pragma endregion
	}
	virtual void CreatePipeline() override {
		Pipelines.emplace_back();
		Pipelines.emplace_back();

#ifdef USE_PIPELINE_SERIALIZE
		PipelineCacheSerializer PCS(Device, GetFilePath(".pco"), 2);
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

#pragma region PASS0
		const std::array SMs0 = {
			VK::CreateShaderModule(GetFilePath(".vert.spv")),
			VK::CreateShaderModule(GetFilePath(".frag.spv")),
			VK::CreateShaderModule(GetFilePath(".tese.spv")),
			VK::CreateShaderModule(GetFilePath(".tesc.spv")),
			VK::CreateShaderModule(GetFilePath(".geom.spv")),
		};
		const std::array PSSCIs0 = {
			VkPipelineShaderStageCreateInfo({.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, .pNext = nullptr, .flags = 0, .stage = VK_SHADER_STAGE_VERTEX_BIT, .module = SMs0[0], .pName = "main", .pSpecializationInfo = nullptr }),
			VkPipelineShaderStageCreateInfo({.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, .pNext = nullptr, .flags = 0, .stage = VK_SHADER_STAGE_FRAGMENT_BIT, .module = SMs0[1], .pName = "main", .pSpecializationInfo = nullptr }),
			VkPipelineShaderStageCreateInfo({.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, .pNext = nullptr, .flags = 0, .stage = VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT, .module = SMs0[2], .pName = "main", .pSpecializationInfo = nullptr }),
			VkPipelineShaderStageCreateInfo({.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, .pNext = nullptr, .flags = 0, .stage = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT, .module = SMs0[3], .pName = "main", .pSpecializationInfo = nullptr }),
			VkPipelineShaderStageCreateInfo({.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, .pNext = nullptr, .flags = 0, .stage = VK_SHADER_STAGE_GEOMETRY_BIT, .module = SMs0[4], .pName = "main", .pSpecializationInfo = nullptr }),
		};
		CreatePipeline_VsFsTesTcsGs(Pipelines[0], PipelineLayouts[0], RenderPasses[0], VK_PRIMITIVE_TOPOLOGY_PATCH_LIST, 1, PRSCI, VK_TRUE, PSSCIs0);
#pragma endregion
		
#pragma region PASS1
		const std::array SMs1 = {
			VK::CreateShaderModule(GetFilePath("_1.vert.spv")),
			VK::CreateShaderModule(GetFilePath("_1.frag.spv")),
		}; 
		const std::array PSSCIs1 = {
			VkPipelineShaderStageCreateInfo({.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, .pNext = nullptr, .flags = 0, .stage = VK_SHADER_STAGE_VERTEX_BIT, .module = SMs1[0], .pName = "main", .pSpecializationInfo = nullptr }),
			VkPipelineShaderStageCreateInfo({.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, .pNext = nullptr, .flags = 0, .stage = VK_SHADER_STAGE_FRAGMENT_BIT, .module = SMs1[1], .pName = "main", .pSpecializationInfo = nullptr }),
		};
		CreatePipeline_VsFs(Pipelines[1], PipelineLayouts[1], RenderPasses[1], VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP, 0, PRSCI, VK_FALSE, PSSCIs1);
#pragma endregion	

		for (auto& i : Threads) { i.join(); }
		Threads.clear();

		for (auto i : SMs0) { vkDestroyShaderModule(Device, i, GetAllocationCallbacks()); }
		for (auto i : SMs1) { vkDestroyShaderModule(Device, i, GetAllocationCallbacks()); }
	}
	virtual void CreateFramebuffer() override {
#ifdef USE_SUBPASS
		//!< #VK_TODO
		//!< フレームバッファは VkRenderPassBeginInfo に渡す必要があるので単一パス(サブパス)で行う場合は、フレームバッファは1つにする
#endif

#pragma region PASS0
		VK::CreateFramebuffer(Framebuffers.emplace_back(), RenderPasses[0], SurfaceExtent2D.width, SurfaceExtent2D.height, 1, {
			RenderTextures.back().View,
#ifdef USE_DEPTH
			DepthTextures.back().View,
#endif
		});
#pragma endregion

#pragma region PASS1
		for (auto i : SwapchainImageViews) {
			VK::CreateFramebuffer(Framebuffers.emplace_back(), RenderPasses[1], SurfaceExtent2D.width, SurfaceExtent2D.height, 1, { i });
		}
#pragma endregion
	}
	virtual void CreateDescriptor() override {
#ifdef USE_SUBPASS
		VK::CreateDescriptorPool(DescriptorPools.emplace_back(), 0, {
			VkDescriptorPoolSize({.type = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, .descriptorCount = 1 })
		});
#endif
#pragma region PASS0
		VK::CreateDescriptorPool(DescriptorPools.emplace_back(), 0, {
			VkDescriptorPoolSize({.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, .descriptorCount = 1 })
		});
#pragma endregion
#pragma region PASS1
		const std::array DSLs = { DescriptorSetLayouts[0] };
		const VkDescriptorSetAllocateInfo DSAI = {
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
			.pNext = nullptr,
			.descriptorPool = DescriptorPools[0],
			.descriptorSetCount = static_cast<uint32_t>(size(DSLs)), .pSetLayouts = data(DSLs)
		};
		VERIFY_SUCCEEDED(vkAllocateDescriptorSets(Device, &DSAI, &DescriptorSets.emplace_back()));
#pragma endregion

		struct DescriptorUpdateInfo
		{
			VkDescriptorImageInfo DescriptorImageInfos[1];
		};
		VkDescriptorUpdateTemplate DUT;
#ifdef USE_SUBPASS
		VK::CreateDescriptorUpdateTemplate(DUT, VK_PIPELINE_BIND_POINT_GRAPHICS, {
			VkDescriptorUpdateTemplateEntry({
				.dstBinding = 0, .dstArrayElement = 0,
				.descriptorCount = 1, .descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,
				.offset = 0, .stride = sizeof(VkDescriptorImageInfo)
			}),
		}, DescriptorSetLayouts[0]);
#endif
#pragma region PASS1
		VK::CreateDescriptorUpdateTemplate(DUT, VK_PIPELINE_BIND_POINT_GRAPHICS, {
			VkDescriptorUpdateTemplateEntry({
				.dstBinding = 0, .dstArrayElement = 0,
				.descriptorCount = 1, .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
				.offset = 0, .stride = sizeof(VkDescriptorImageInfo)
			}),
		}, DescriptorSetLayouts[0]);
		const auto DII = VkDescriptorImageInfo({.sampler = VK_NULL_HANDLE, .imageView = RenderTextures.back().View, .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL });
		vkUpdateDescriptorSetWithTemplate(Device, DescriptorSets[0], DUT, &DII);
#pragma endregion
		vkDestroyDescriptorUpdateTemplate(Device, DUT, GetAllocationCallbacks());
	}
	
	virtual void PopulateSecondaryCommandBuffer(const size_t i) override {
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
				vkCmdSetViewport(SCB0, 0, static_cast<uint32_t>(size(Viewports)), data(Viewports));
				vkCmdSetScissor(SCB0, 0, static_cast<uint32_t>(size(ScissorRects)), data(ScissorRects));

				vkCmdBindPipeline(SCB0, VK_PIPELINE_BIND_POINT_GRAPHICS, Pipelines[0]);

				vkCmdDrawIndirect(SCB0, IndirectBuffers[0].Buffer, 0, 1, 0);
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
				vkCmdSetViewport(SCB1, 0, static_cast<uint32_t>(size(Viewports)), data(Viewports));
				vkCmdSetScissor(SCB1, 0, static_cast<uint32_t>(size(ScissorRects)), data(ScissorRects));

				vkCmdBindPipeline(SCB1, VK_PIPELINE_BIND_POINT_GRAPHICS, Pipelines[1]);

				constexpr std::array<uint32_t, 0> DynamicOffsets = {};
				vkCmdBindDescriptorSets(SCB1, VK_PIPELINE_BIND_POINT_GRAPHICS, PipelineLayouts[1], 0, static_cast<uint32_t>(size(DescriptorSets)), data(DescriptorSets), static_cast<uint32_t>(size(DynamicOffsets)), data(DynamicOffsets));

				vkCmdDrawIndirect(SCB1, IndirectBuffers[1].Buffer, 0, 1, 0);
			} VERIFY_SUCCEEDED(vkEndCommandBuffer(SCB1));
		}
#pragma endregion
	}
	virtual void PopulateCommandBuffer(const size_t i) override {
		const auto RP0 = RenderPasses[0];
		const auto FB0 = Framebuffers[0];
		const auto SCB0 = SecondaryCommandBuffers[i];

		const auto RP1 = RenderPasses[1];
		const auto FB1 = Framebuffers[i + 1];
		const auto SCCount = size(SwapchainImages);
		const auto SCB1 = SecondaryCommandBuffers[i + SCCount];

#ifdef USE_SUBPASS
		//vkCmdNextSubpass() を使用する
#endif

		const auto CB = CommandBuffers[i];
		const VkCommandBufferBeginInfo CBBI = {
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
			.pNext = nullptr,
			.flags = 0,
			.pInheritanceInfo = nullptr
		};
		VERIFY_SUCCEEDED(vkBeginCommandBuffer(CB, &CBBI)); {
			const auto RenderArea = VkRect2D({ .offset = VkOffset2D({.x = 0, .y = 0 }), .extent = SurfaceExtent2D });

#pragma region PASS0
			//!< メッシュ描画用セカンダリコマンドバッファを発行
			{
#ifdef USE_DEPTH
				constexpr std::array CVs = { VkClearValue({.color = Colors::SkyBlue }), VkClearValue({.depthStencil = {.depth = 1.0f, .stencil = 0 } }) };
#else
				constexpr std::array CVs = { VkClearValue({.color = Colors::SkyBlue }) };
#endif
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

			//!< リソースバリア
			{
				const std::array IMBs = {
					//!< COLOR_ATTACHMENT_READ_BIT -> SHADER_READ_BIT
					VkImageMemoryBarrier({
						.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
						.pNext = nullptr,
						.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, .dstAccessMask = VK_ACCESS_SHADER_READ_BIT,
						.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, .newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
						.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED, .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
						.image = RenderTextures[0].Image,
						.subresourceRange = VkImageSubresourceRange({.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .baseMipLevel = 0, .levelCount = 1, .baseArrayLayer = 0, .layerCount = 1 })
					}),
				};
				//!< COLOR_ATTACHMENT_OUTPUT_BIT -> FRAGMENT_SHADER_BIT
				vkCmdPipelineBarrier(CB,
					VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
					VK_DEPENDENCY_BY_REGION_BIT,
					0, nullptr,
					0, nullptr,
					static_cast<uint32_t>(size(IMBs)), data(IMBs));
			}

#pragma region PASS1
			//!< レンダーテクスチャ描画用セカンダリコマンドバッファを発行
			{
				constexpr std::array<VkClearValue, 0> CVs = {};
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
};
#pragma endregion