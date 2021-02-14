#pragma once

#include "resource.h"

#pragma region Code
#include "../VKExt.h"

class DeferredVK : public VKExt
{
private:
	using Super = VKExt;
public:
	DeferredVK() : Super() {}
	virtual ~DeferredVK() {}

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
	
#ifdef USE_GBUFFER_VISUALIZE
	virtual void CreateViewport(const float Width, const float Height, const float MinDepth = 0.0f, const float MaxDepth = 1.0f) override {
		const auto W = Width * 0.5f, H = Height * 0.5f;
		Viewports = {
	#ifdef USE_VIEWPORT_Y_UP
			//!< �S��ʗp(Fullscreen)
			{ 0.0f, Height, Width, -Height, MinDepth, MaxDepth },
			//!< ������ʗp(DividedScreens)
			{ 0.0f, H, W, -H, MinDepth, MaxDepth },
			{ W, H, W, -H, MinDepth, MaxDepth },
			{ 0.0f, Height, W, -H, MinDepth, MaxDepth },
			{ W, Height, W, -H, MinDepth, MaxDepth },
	#else
			//!< �S��ʗp
			{ 0.0f, 0.0f, Width, Height, MinDepth, MaxDepth },
			//!< ������ʗp
			{ 0.0f, 0.0f, W, H,MinDepth, MaxDepth },
			{ W, 0.0f, W, H, MinDepth, MaxDepth },
			{ 0.0f, H, W, H, MinDepth, MaxDepth },
			{ W, H, W, H, MinDepth, MaxDepth },
	#endif
		};
		ScissorRects = {
			//!< �S��ʗp(Fullscreen)
			{ { 0, 0 }, { static_cast<uint32_t>(Width), static_cast<uint32_t>(Height) } },
			//!< ������ʗp(DividedScreens)
			{ { 0, 0 }, { static_cast<uint32_t>(W), static_cast<uint32_t>(H) } },
			{ { static_cast<int32_t>(W), 0 }, { static_cast<uint32_t>(W), static_cast<uint32_t>(H) } },
			{ { 0, static_cast<int32_t>(H) }, { static_cast<uint32_t>(W), static_cast<uint32_t>(H) } },
			{ { static_cast<int32_t>(W), static_cast<int32_t>(H) }, { static_cast<uint32_t>(W), static_cast<uint32_t>(H) } },
		};
		LOG_OK();
	}
#endif

	virtual void AllocateCommandBuffer() override {
		Super::AllocateCommandBuffer();

#pragma region FRAME_OBJECT
		const auto SCCount = static_cast<uint32_t>(size(SwapchainImages));
		//!< Pass1 : �Z�J���_���R�}���h�o�b�t�@
		assert(!empty(SecondaryCommandPools) && "");
		const auto PrevCount = size(SecondaryCommandBuffers);
		SecondaryCommandBuffers.resize(PrevCount + SCCount);
		const VkCommandBufferAllocateInfo SCBAI = {
			VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
			nullptr,
			SecondaryCommandPools[0],
			VK_COMMAND_BUFFER_LEVEL_SECONDARY,
			SCCount
		};
		VERIFY_SUCCEEDED(vkAllocateCommandBuffers(Device, &SCBAI, &SecondaryCommandBuffers[PrevCount]));
#pragma endregion
	}
	virtual void CreateFramebuffer() override {
		//!< Pass0 : �t���[���o�b�t�@
		{
			assert(4 + 1 == size(ImageViews) && "");
			VK::CreateFramebuffer(Framebuffers.emplace_back(), RenderPasses[0], SurfaceExtent2D.width, SurfaceExtent2D.height, 1, {
				//!< �����_�[�^�[�Q�b�g : �J���[(RenderTarget : Color)
				ImageViews[0],
	#pragma region MRT 
				//!< �����_�[�^�[�Q�b�g : �@��(RenderTarget : Normal)
				ImageViews[1],
				//!< �����_�[�^�[�Q�b�g : �[�x(RenderTarget : Depth)
				ImageViews[2],
				//!< �����_�[�^�[�Q�b�g : ����
				ImageViews[3],
	#pragma endregion
				//!< �[�x�o�b�t�@(Depth Buffer)
				ImageViews[4],
			});
		}

		//!< Pass1 : �t���[���o�b�t�@
		{
			for (auto i : SwapchainImageViews) {
				VK::CreateFramebuffer(Framebuffers.emplace_back(), RenderPasses[1], SurfaceExtent2D.width, SurfaceExtent2D.height, 1, { i });
			}
		}
	}
	virtual void CreateRenderPass() override {
		//!< Pass0 : �����_�[�p�X
		{
			constexpr std::array ColorAttach = {
				//!< �����_�[�^�[�Q�b�g : �J���[(RenderTarget : Color)
				VkAttachmentReference({ .attachment = 0, .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL }),
#pragma region MRT 
				//!< �����_�[�^�[�Q�b�g : �@��(RenderTarget : Normal)
				VkAttachmentReference({ .attachment = 1, .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL }),
				//!< �����_�[�^�[�Q�b�g : �[�x(RenderTarget : Depth)
				VkAttachmentReference({ .attachment = 2, .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL }),
				//!< �����_�[�^�[�Q�b�g : ����
				VkAttachmentReference({ .attachment = 3, .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL }),
#pragma endregion
			};
			const VkAttachmentReference DepthAttach = { static_cast<uint32_t>(size(ColorAttach)), VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL };
			VK::CreateRenderPass(RenderPasses.emplace_back(), {
				//!< �A�^�b�`�����g(Attachment)
				//!< �����_�[�^�[�Q�b�g : �J���[(RenderTarget : Color)
				VkAttachmentDescription({
					.flags = 0,
					.format = ColorFormat,
					.samples = VK_SAMPLE_COUNT_1_BIT,
					.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR, .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
					.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE, .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
					.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED, .finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
				}),
#pragma region MRT 
				//!< �����_�[�^�[�Q�b�g : �@��(RenderTarget : Normal)
				VkAttachmentDescription({
					.flags = 0,
					.format = VK_FORMAT_A2B10G10R10_UNORM_PACK32,
					.samples = VK_SAMPLE_COUNT_1_BIT,
					.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR, .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
					.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE, .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
					.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED, .finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
				}),
				//!< �����_�[�^�[�Q�b�g : �[�x(RenderTarget : Depth)
				VkAttachmentDescription({
					.flags = 0,
					.format = VK_FORMAT_R32_SFLOAT,
					.samples = VK_SAMPLE_COUNT_1_BIT,
					.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR, .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
					.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE, .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
					.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED, .finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
				}),
				//!< �����_�[�^�[�Q�b�g : ����
				VkAttachmentDescription({
					.flags = 0,
					.format = VK_FORMAT_B8G8R8A8_UNORM,
					.samples = VK_SAMPLE_COUNT_1_BIT,
					.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR, .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
					.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE, .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
					.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED, .finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
				}),
#pragma endregion
				//!< �[�x�o�b�t�@(Depth Buffer)
				VkAttachmentDescription({
					.flags = 0,
					.format = DepthFormat,
					.samples = VK_SAMPLE_COUNT_1_BIT,
					.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR, .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
					.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE, .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
					.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED, .finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
				}),
			}, {
				//!< �T�u�p�X(SubPass)
				VkSubpassDescription({
					.flags = 0,
					.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
					.inputAttachmentCount = 0, .pInputAttachments = nullptr,
					.colorAttachmentCount = static_cast<uint32_t>(size(ColorAttach)), .pColorAttachments = data(ColorAttach), .pResolveAttachments = nullptr,
					.pDepthStencilAttachment = &DepthAttach,
					.preserveAttachmentCount = 0, .pPreserveAttachments = nullptr
				}),
			}, {
				//!< �T�u�p�X�ˑ�
			});
		}
		//!< Pass1 : �����_�[�p�X
		{
			constexpr std::array ColorAttach = { VkAttachmentReference({ .attachment = 0, .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL }), };
			VK::CreateRenderPass(RenderPasses.emplace_back(), {
				//!< �A�^�b�`�����g
				VkAttachmentDescription({
					.flags = 0,
					.format = ColorFormat,
					.samples = VK_SAMPLE_COUNT_1_BIT,
					.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE, .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
					.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE, .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
					.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED, .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
				}),
			}, {
				//!< �T�u�p�X
				VkSubpassDescription({
					.flags = 0,
					.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
					.inputAttachmentCount = 0, .pInputAttachments = nullptr,
					.colorAttachmentCount = static_cast<uint32_t>(size(ColorAttach)), .pColorAttachments = data(ColorAttach), .pResolveAttachments = nullptr,
					.pDepthStencilAttachment = nullptr,
					.preserveAttachmentCount = 0, .pPreserveAttachments = nullptr
				}),
			}, {
				//!< �T�u�p�X�ˑ�
			});
		}
	}
	virtual void CreateGeometry() override {
		const auto PDMP = GetCurrentPhysicalDeviceMemoryProperties();
		const auto CB = CommandBuffers[0];
		//!< Pass0 : �C���_�C���N�g�o�b�t�@(���b�V���`��p)
		{
			constexpr VkDrawIndexedIndirectCommand DIIC = { .indexCount = 1, .instanceCount = 1, .firstIndex = 0, .vertexOffset = 0, .firstInstance = 0 };
			IndirectBuffers.emplace_back().Create(Device, PDMP, DIIC, CB, GraphicsQueue);
		}
		//!< Pass1 : �C���_�C���N�g�o�b�t�@(�����_�[�e�N�X�`���`��p)
		{
			constexpr VkDrawIndirectCommand DIC = { .vertexCount = 4, .instanceCount = 1, .firstVertex = 0, .firstInstance = 0 };
			IndirectBuffers.emplace_back().Create(Device, PDMP, DIC, CB, GraphicsQueue);
		}
	}
	//virtual void CreateDescriptorSetLayout() override {
	//	assert(!empty(Samplers) && "");

	//	//!< Pass0 : �f�X�N���v�^�Z�b�g���C�A�E�g
	//	{
	//		DescriptorSetLayouts.push_back(VkDescriptorSetLayout());
	//		VKExt::CreateDescriptorSetLayout(DescriptorSetLayouts.back(), 0, {
	//			{ 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_GEOMETRY_BIT, nullptr },
	//		});
	//	}

	//	//!< Pass1 : �f�X�N���v�^�Z�b�g���C�A�E�g
	//	{
	//		const std::array<VkSampler, 1> ISs = { Samplers[0] };

	//		DescriptorSetLayouts.push_back(VkDescriptorSetLayout());
	//		VKExt::CreateDescriptorSetLayout(DescriptorSetLayouts.back(), 0, {
	//			//!< �����_�[�^�[�Q�b�g : �J���[(RenderTarget : Color)
	//			{ 0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, static_cast<uint32_t>(size(ISs)), VK_SHADER_STAGE_FRAGMENT_BIT, data(ISs) },
	//#pragma region MRT 
	//			//!< �����_�[�^�[�Q�b�g : �@��(RenderTarget : Normal)
	//			{ 1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, static_cast<uint32_t>(size(ISs)), VK_SHADER_STAGE_FRAGMENT_BIT, data(ISs) },
	//			//!< �����_�[�^�[�Q�b�g : �[�x(RenderTarget : Depth)
	//			{ 2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, static_cast<uint32_t>(size(ISs)), VK_SHADER_STAGE_FRAGMENT_BIT, data(ISs) },
	//			//!< �����_�[�^�[�Q�b�g : ����
	//			{ 3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, static_cast<uint32_t>(size(ISs)), VK_SHADER_STAGE_FRAGMENT_BIT, data(ISs) },
	//#pragma endregion
	//			{ 4, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr },
	//		});
	//	}
	//}
	virtual void CreatePipelineLayout() override {
		//!< Pass0 : �p�C�v���C�����C�A�E�g
		{
			CreateDescriptorSetLayout(DescriptorSetLayouts.emplace_back(), 0, {
				VkDescriptorSetLayoutBinding({.binding = 0, .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, .descriptorCount = 1, .stageFlags = VK_SHADER_STAGE_GEOMETRY_BIT, .pImmutableSamplers = nullptr }),
			});
			VK::CreatePipelineLayout(PipelineLayouts.emplace_back(), { DescriptorSetLayouts.back() }, {});
		}

		//!< Pass1 : �p�C�v���C�����C�A�E�g
		{
			//VkDescriptorSetLayoutBinding({ .binding = 0, .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, .descriptorCount = 1, .stageFlags = VK_SHADER_STAGE_GEOMETRY_BIT, .pImmutableSamplers = nullptr }), //!< UniformBuffer

			const std::array ISs = { Samplers[0] };
			CreateDescriptorSetLayout(DescriptorSetLayouts.emplace_back(), 0, {
				//!< �����_�[�^�[�Q�b�g : �J���[(RenderTarget : Color)
				VkDescriptorSetLayoutBinding({.binding = 0, .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, .descriptorCount = static_cast<uint32_t>(size(ISs)), .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT, .pImmutableSamplers = data(ISs) }),
	#pragma region MRT 
				//!< �����_�[�^�[�Q�b�g : �@��(RenderTarget : Normal)
				VkDescriptorSetLayoutBinding({.binding = 1, .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, .descriptorCount = static_cast<uint32_t>(size(ISs)), .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT, .pImmutableSamplers = data(ISs) }),
				//!< �����_�[�^�[�Q�b�g : �[�x(RenderTarget : Depth)
				VkDescriptorSetLayoutBinding({.binding =  2, .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, .descriptorCount = static_cast<uint32_t>(size(ISs)), .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT, .pImmutableSamplers = data(ISs) }),
				//!< �����_�[�^�[�Q�b�g : ����
				VkDescriptorSetLayoutBinding({.binding = 3, .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, .descriptorCount = static_cast<uint32_t>(size(ISs)), .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT, .pImmutableSamplers = data(ISs) }),
	#pragma endregion
				VkDescriptorSetLayoutBinding({.binding = 4, .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, .descriptorCount = 1, .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT, .pImmutableSamplers = nullptr }),
			});

			VK::CreatePipelineLayout(PipelineLayouts.emplace_back(), { DescriptorSetLayouts.back() }, {});
		}
	}

	virtual void CreateDescriptorSet() override {
		//!< Pass0,1 : �f�X�N���v�^�v�[��
		VK::CreateDescriptorPool(DescriptorPools.emplace_back(), 0, {
#pragma region FRAME_OBJECT
			VkDescriptorPoolSize({ .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, .descriptorCount = static_cast<uint32_t>(size(SwapchainImages)) * 2 }), //!< UB * N * 2
#pragma endregion
#pragma region MRT 
			//!< �����_�[�^�[�Q�b�g : �J���[(RenderTarget : Color), �@��(RenderTarget : Normal), �[�x(RenderTarget : Depth), ����
			VkDescriptorPoolSize({ .type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, .descriptorCount = 4 }), //!< CIS * 4
#pragma endregion
		});

		//!< Pass0 : �f�X�N���v�^�Z�b�g
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
		}
		//!< Pass1 : �f�X�N���v�^�Z�b�g
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
	}
//	virtual void CreateDescriptorPool() override {
//#pragma region FRAME_OBJECT
//		const auto SCCount = static_cast<uint32_t>(size(SwapchainImages));
//#pragma endregion
//
//		//!< Pass0,1 : �f�X�N���v�^�v�[��
//		DescriptorPools.push_back(VkDescriptorPool());
//		VKExt::CreateDescriptorPool(DescriptorPools.back(), 0, {
//#pragma region FRAME_OBJECT
//			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, SCCount * 2 }, //!< UB * N * 2
//#pragma endregion
//#pragma region MRT 
//			//!< �����_�[�^�[�Q�b�g : �J���[(RenderTarget : Color), �@��(RenderTarget : Normal), �[�x(RenderTarget : Depth), ����
//			{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 4 }, //!< CIS * 4
//#pragma endregion
//		});
//	}
//	virtual void AllocateDescriptorSet() override {
//		assert(2 == size(DescriptorSetLayouts) && "");
//		assert(!empty(DescriptorPools) && "");
//
//		const auto SCCount = size(SwapchainImages);
//
//		//!< �p�X0 : �f�X�N���v�^�Z�b�g
//		{
//			const std::array<VkDescriptorSetLayout, 1> DSLs = { DescriptorSetLayouts[0] };
//			const VkDescriptorSetAllocateInfo DSAI = {
//				VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
//				nullptr,
//				DescriptorPools[0],
//				static_cast<uint32_t>(size(DSLs)), data(DSLs)
//			};
//#pragma region FRAME_OBJECT
//			for (size_t i = 0; i < SCCount; ++i) {
//				DescriptorSets.push_back(VkDescriptorSet());
//				VERIFY_SUCCEEDED(vkAllocateDescriptorSets(Device, &DSAI, &DescriptorSets.back()));
//			}
//#pragma endregion
//		}
//
//		//!< �p�X1 : �f�X�N���v�^�Z�b�g
//		{
//			const std::array<VkDescriptorSetLayout, 1> DSLs = { DescriptorSetLayouts[1] };
//			const VkDescriptorSetAllocateInfo DSAI = {
//				VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
//				nullptr,
//				DescriptorPools[0],
//				static_cast<uint32_t>(size(DSLs)), data(DSLs)
//			};
//#pragma region FRAME_OBJECT
//			for (size_t i = 0; i < SCCount; ++i) {
//				DescriptorSets.push_back(VkDescriptorSet());
//				VERIFY_SUCCEEDED(vkAllocateDescriptorSets(Device, &DSAI, &DescriptorSets.back()));
//			}
//#pragma endregion
//		}
//	}
	virtual void UpdateDescriptorSet() override {
#pragma region FRAME_OBJECT
		const auto SCCount = size(SwapchainImages);
		//!< Pass0 :
		VK::CreateDescriptorUpdateTemplate(DescriptorUpdateTemplates.emplace_back(), {
			VkDescriptorUpdateTemplateEntry({
				0, 0,
				_countof(DescriptorUpdateInfo_0::DBI), VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
				offsetof(DescriptorUpdateInfo_0, DBI), sizeof(DescriptorUpdateInfo_0)
			}),
		}, DescriptorSetLayouts[0]);
		for (size_t i = 0; i < SCCount; ++i) {
			const DescriptorUpdateInfo_0 DUI = {
				{ UniformBuffers[i].Buffer, 0, VK_WHOLE_SIZE },
			};
			vkUpdateDescriptorSetWithTemplate(Device, DescriptorSets[i], DescriptorUpdateTemplates.back(), &DUI);
		}

		//!< Pass1 :
		VK::CreateDescriptorUpdateTemplate(DescriptorUpdateTemplates.emplace_back(), {
			//!< �����_�[�^�[�Q�b�g : �J���[(RenderTarget : Color)
			VkDescriptorUpdateTemplateEntry({
				.dstBinding = 0, .dstArrayElement = 0,
				.descriptorCount = _countof(DescriptorUpdateInfo_1::DII), .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
				.offset = offsetof(DescriptorUpdateInfo_1, DII), .stride = sizeof(DescriptorUpdateInfo_1)
			}),
#pragma region MRT 
			//!< �����_�[�^�[�Q�b�g : �@��(RenderTarget : Normal)
			VkDescriptorUpdateTemplateEntry({
				.dstBinding = 1, .dstArrayElement = 0,
				.descriptorCount = _countof(DescriptorUpdateInfo_1::DII1), .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
				.offset = offsetof(DescriptorUpdateInfo_1, DII1), .stride = sizeof(DescriptorUpdateInfo_1)
			}),
			//!< �����_�[�^�[�Q�b�g : �[�x(RenderTarget : Depth)
			VkDescriptorUpdateTemplateEntry({
				.dstBinding = 2, .dstArrayElement = 0,
				.descriptorCount = _countof(DescriptorUpdateInfo_1::DII2), .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
				.offset = offsetof(DescriptorUpdateInfo_1, DII2), .stride = sizeof(DescriptorUpdateInfo_1)
			}),
			//!< �����_�[�^�[�Q�b�g : ����
			VkDescriptorUpdateTemplateEntry({
				.dstBinding = 3, .dstArrayElement = 0,
				.descriptorCount = _countof(DescriptorUpdateInfo_1::DII3), .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
				.offset = offsetof(DescriptorUpdateInfo_1, DII3), .stride = sizeof(DescriptorUpdateInfo_1)
			}),
#pragma endregion
			VkDescriptorUpdateTemplateEntry({
				.dstBinding = 4, .dstArrayElement = 0,
				.descriptorCount = _countof(DescriptorUpdateInfo_1::DBI), .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
				.offset = offsetof(DescriptorUpdateInfo_1, DBI), .stride = sizeof(DescriptorUpdateInfo_1)
			}),
		}, DescriptorSetLayouts[1]);
		for (size_t i = 0; i < SCCount; ++i) {
			const DescriptorUpdateInfo_1 DUI = {
				//!< �����_�[�^�[�Q�b�g : �J���[(RenderTarget : Color)
				{ VK_NULL_HANDLE, ImageViews[0], VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL },
	#pragma region MRT 
				//!< �����_�[�^�[�Q�b�g : �@��(RenderTarget : Normal)
				{ VK_NULL_HANDLE, ImageViews[1], VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL },
				//!< �����_�[�^�[�Q�b�g : �[�x(RenderTarget : Depth)
				{ VK_NULL_HANDLE, ImageViews[2], VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL },
				//!< �����_�[�^�[�Q�b�g : ����
				{ VK_NULL_HANDLE, ImageViews[3], VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL },
	#pragma endregion
				{ UniformBuffers[i].Buffer, 0, VK_WHOLE_SIZE },
			};
			vkUpdateDescriptorSetWithTemplate(Device, DescriptorSets[i + SCCount], DescriptorUpdateTemplates.back(), &DUI);
		}
#pragma endregion
	}

	virtual void CreateTexture() override {
		const VkExtent3D Extent = { .width = SurfaceExtent2D.width, .height = SurfaceExtent2D.height, .depth = 1 };
		const VkComponentMapping CM = { .r = VK_COMPONENT_SWIZZLE_R, .g = VK_COMPONENT_SWIZZLE_G, .b = VK_COMPONENT_SWIZZLE_B, .a = VK_COMPONENT_SWIZZLE_A };
		const VkImageSubresourceRange ISR = { .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .baseMipLevel = 0, .levelCount = 1, .baseArrayLayer = 0, .layerCount = 1 };
		//!< �����_�[�^�[�Q�b�g : �J���[(RenderTarget : Color)
		{
			Images.emplace_back();
			CreateImage(&Images.back().Image, 0, VK_IMAGE_TYPE_2D, ColorFormat, Extent, 1, 1, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);

			AllocateDeviceMemory(&Images.back().DeviceMemory, Images.back().Image, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
			VERIFY_SUCCEEDED(vkBindImageMemory(Device, Images.back().Image, Images.back().DeviceMemory, 0));

			CreateImageView(&ImageViews.emplace_back(), Images.back().Image, VK_IMAGE_VIEW_TYPE_2D, ColorFormat, CM, ISR);
		}
#pragma region MRT 
		//!< �����_�[�^�[�Q�b�g : �@��(RenderTarget : Normal)
		{
			const auto Format = VK_FORMAT_A2B10G10R10_UNORM_PACK32;
			Images.emplace_back();
			CreateImage(&Images.back().Image, 0, VK_IMAGE_TYPE_2D, Format, Extent, 1, 1, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);

			AllocateDeviceMemory(&Images.back().DeviceMemory, Images.back().Image, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
			VERIFY_SUCCEEDED(vkBindImageMemory(Device, Images.back().Image, Images.back().DeviceMemory, 0));

			CreateImageView(&ImageViews.emplace_back(), Images.back().Image, VK_IMAGE_VIEW_TYPE_2D, Format, CM, ISR);
		}
		//!< �����_�[�^�[�Q�b�g : �[�x(RenderTarget : Depth)
		{
			const auto Format = VK_FORMAT_R32_SFLOAT;
			Images.emplace_back();
			CreateImage(&Images.back().Image, 0, VK_IMAGE_TYPE_2D, Format, Extent, 1, 1, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);

			AllocateDeviceMemory(&Images.back().DeviceMemory, Images.back().Image, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
			VERIFY_SUCCEEDED(vkBindImageMemory(Device, Images.back().Image, Images.back().DeviceMemory, 0));

			CreateImageView(&ImageViews.emplace_back(), Images.back().Image, VK_IMAGE_VIEW_TYPE_2D, Format, CM, ISR);
		}
		//!< �����_�[�^�[�Q�b�g : ����
		{
			const auto Format = VK_FORMAT_B8G8R8A8_UNORM;
			Images.emplace_back();
			CreateImage(&Images.back().Image, 0, VK_IMAGE_TYPE_2D, Format, Extent, 1, 1, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);

			AllocateDeviceMemory(&Images.back().DeviceMemory, Images.back().Image, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
			VERIFY_SUCCEEDED(vkBindImageMemory(Device, Images.back().Image, Images.back().DeviceMemory, 0));

			CreateImageView(&ImageViews.emplace_back(), Images.back().Image, VK_IMAGE_VIEW_TYPE_2D, Format, CM, ISR);
		}
#pragma endregion
		//!< �[�x�o�b�t�@(Depth Buffer)
		{
			Images.emplace_back();
			CreateImage(&Images.back().Image, 0, VK_IMAGE_TYPE_2D, DepthFormat, Extent, 1, 1, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT/* | VK_IMAGE_USAGE_SAMPLED_BIT*/);

			AllocateDeviceMemory(&Images.back().DeviceMemory, Images.back().Image, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
			VERIFY_SUCCEEDED(vkBindImageMemory(Device, Images.back().Image, Images.back().DeviceMemory, 0));

			CreateImageView(&ImageViews.emplace_back(), Images.back().Image, VK_IMAGE_VIEW_TYPE_2D, DepthFormat, CM, VkImageSubresourceRange({ .aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT, .baseMipLevel = 0, .levelCount = 1, .baseArrayLayer = 0, .layerCount = 1 }));
		}
	}
	virtual void CreateImmutableSampler() override {
		//!< Pass1 : �C�~���[�^�u���T���v��
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
		VERIFY_SUCCEEDED(vkCreateSampler(Device, &SCI, GetAllocationCallbacks(), &Samplers.emplace_back()));
	}
	virtual void CreateUniformBuffer() override {
		{
			//const auto Fov = 0.16f * glm::pi<float>();
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
	}
	virtual void CreateShaderModule() override {
		const auto ShaderPath = GetBasePath();
		//!< Pass0 : �V�F�[�_���W���[��
		ShaderModules.emplace_back(VK::CreateShaderModule(data(ShaderPath + TEXT(".vert.spv"))));
		ShaderModules.emplace_back(VK::CreateShaderModule(data(ShaderPath + TEXT(".frag.spv"))));
		ShaderModules.emplace_back(VK::CreateShaderModule(data(ShaderPath + TEXT(".tese.spv"))));
		ShaderModules.emplace_back(VK::CreateShaderModule(data(ShaderPath + TEXT(".tesc.spv"))));
		ShaderModules.emplace_back(VK::CreateShaderModule(data(ShaderPath + TEXT(".geom.spv"))));
		//!< Pass1 : �V�F�[�_���W���[��
		ShaderModules.emplace_back(VK::CreateShaderModule(data(ShaderPath + TEXT("_1") + TEXT(".vert.spv"))));
#ifdef USE_GBUFFER_VISUALIZE
		ShaderModules.emplace_back(VK::CreateShaderModule(data(ShaderPath + TEXT("_gb_1") + TEXT(".frag.spv"))));
		ShaderModules.emplace_back(VK::CreateShaderModule(data(ShaderPath + TEXT("_gb_1") + TEXT(".geom.spv"))));
#else
		ShaderModules.emplace_back(VK::CreateShaderModule(data(ShaderPath + TEXT("_1") + TEXT(".frag.spv"))));
#endif
	}
	virtual void CreatePipeline() override {
		Pipelines.resize(2);
		std::vector<std::thread> Threads;
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
		const VkPipelineDepthStencilStateCreateInfo PDSSCI_0 = {
			.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
			.pNext = nullptr,
			.flags = 0,
			.depthTestEnable = VK_TRUE, .depthWriteEnable = VK_TRUE, .depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL,
			.depthBoundsTestEnable = VK_FALSE,
			.stencilTestEnable = VK_FALSE, 
			.front = VkStencilOpState({ .failOp = VK_STENCIL_OP_KEEP, .passOp = VK_STENCIL_OP_KEEP, .depthFailOp = VK_STENCIL_OP_KEEP, .compareOp = VK_COMPARE_OP_NEVER, .compareMask = 0, .writeMask = 0, .reference = 0 }),
			.back = VkStencilOpState({ .failOp = VK_STENCIL_OP_KEEP, .passOp = VK_STENCIL_OP_KEEP, .depthFailOp = VK_STENCIL_OP_KEEP, .compareOp = VK_COMPARE_OP_ALWAYS, .compareMask = 0, .writeMask = 0, .reference = 0 }),
			.minDepthBounds = 0.0f, .maxDepthBounds = 1.0f
		};
		const VkPipelineDepthStencilStateCreateInfo PDSSCI_1 = {
			.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
			.pNext = nullptr,
			.flags = 0,
			.depthTestEnable = VK_FALSE, .depthWriteEnable = VK_FALSE, .depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL,
			.depthBoundsTestEnable = VK_FALSE,
			.stencilTestEnable = VK_FALSE, 
			.front = VkStencilOpState({ .failOp = VK_STENCIL_OP_KEEP, .passOp = VK_STENCIL_OP_KEEP, .depthFailOp = VK_STENCIL_OP_KEEP, .compareOp = VK_COMPARE_OP_NEVER, .compareMask = 0, .writeMask = 0, .reference = 0 }),
			.back = VkStencilOpState({ .failOp = VK_STENCIL_OP_KEEP, .passOp = VK_STENCIL_OP_KEEP, .depthFailOp = VK_STENCIL_OP_KEEP, .compareOp = VK_COMPARE_OP_ALWAYS, .compareMask = 0, .writeMask = 0, .reference = 0 }),
			.minDepthBounds = 0.0f, .maxDepthBounds = 1.0f
		};
		const std::array PSSCIs_0 = {
			VkPipelineShaderStageCreateInfo({ .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, .pNext = nullptr, .flags = 0, .stage = VK_SHADER_STAGE_VERTEX_BIT, .module = ShaderModules[0], .pName = "main", .pSpecializationInfo = nullptr }),
			VkPipelineShaderStageCreateInfo({ .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, .pNext = nullptr, .flags = 0, .stage = VK_SHADER_STAGE_FRAGMENT_BIT, .module = ShaderModules[1], .pName = "main", .pSpecializationInfo = nullptr }),
			VkPipelineShaderStageCreateInfo({ .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, .pNext = nullptr, .flags = 0, .stage = VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT, .module = ShaderModules[2], .pName = "main", .pSpecializationInfo = nullptr }),
			VkPipelineShaderStageCreateInfo({ .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, .pNext = nullptr, .flags = 0, .stage = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT, .module = ShaderModules[3], .pName = "main", .pSpecializationInfo = nullptr }),
			VkPipelineShaderStageCreateInfo({ .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, .pNext = nullptr, .flags = 0, .stage = VK_SHADER_STAGE_GEOMETRY_BIT, .module = ShaderModules[4], .pName = "main", .pSpecializationInfo = nullptr }),
		};
#ifdef USE_GBUFFER_VISUALIZE
		const std::array PSSCIs_1 = {
			VkPipelineShaderStageCreateInfo({ .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, .pNext = nullptr, .flags = 0, .stage = VK_SHADER_STAGE_VERTEX_BIT, .module = ShaderModules[5], .pName = "main", .pSpecializationInfo = nullptr }),
			VkPipelineShaderStageCreateInfo({ .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, .pNext = nullptr, .flags = 0, .stage = VK_SHADER_STAGE_FRAGMENT_BIT, .module = ShaderModules[6], .pName = "main", .pSpecializationInfo = nullptr }),
			VkPipelineShaderStageCreateInfo({ .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, .pNext = nullptr, .flags = 0, .stage = VK_SHADER_STAGE_GEOMETRY_BIT, .module = ShaderModules[7], .pName = "main", .pSpecializationInfo = nullptr }),
		};
#else
		const std::array PSSCIs_1 = {
			VkPipelineShaderStageCreateInfo({ .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, .pNext = nullptr, .flags = 0, .stage = VK_SHADER_STAGE_VERTEX_BIT, .module = ShaderModules[5], .pName = "main", .pSpecializationInfo = nullptr }),
			VkPipelineShaderStageCreateInfo({ .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, .pNext = nullptr, .flags = 0, .stage = VK_SHADER_STAGE_FRAGMENT_BIT, .module = ShaderModules[6], .pName = "main", .pSpecializationInfo = nullptr }),
		};
#endif
		const std::vector<VkVertexInputBindingDescription> VIBDs = {};
		const std::vector<VkVertexInputAttributeDescription> VIADs = {};
		const std::vector PCBASs_0 = {
			//!< �����_�[�^�[�Q�b�g : �J���[(RenderTarget : Color)
			VkPipelineColorBlendAttachmentState({
				.blendEnable = VK_FALSE,
				.srcColorBlendFactor = VK_BLEND_FACTOR_ONE, .dstColorBlendFactor = VK_BLEND_FACTOR_ONE, .colorBlendOp = VK_BLEND_OP_ADD,
				.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE, .dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE, .alphaBlendOp = VK_BLEND_OP_ADD,
				.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
			}),
#pragma region MRT 
			//!< �����_�[�^�[�Q�b�g : �@��(RenderTarget : Normal)
			VkPipelineColorBlendAttachmentState({
				.blendEnable = VK_FALSE,
				.srcColorBlendFactor = VK_BLEND_FACTOR_ONE, .dstColorBlendFactor = VK_BLEND_FACTOR_ONE, .colorBlendOp = VK_BLEND_OP_ADD,
				.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE, .dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE, .alphaBlendOp = VK_BLEND_OP_ADD,
				.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
			}),
			//!< �����_�[�^�[�Q�b�g : �[�x(RenderTarget : Depth)
			VkPipelineColorBlendAttachmentState({
				.blendEnable = VK_FALSE,
				.srcColorBlendFactor = VK_BLEND_FACTOR_ONE, .dstColorBlendFactor = VK_BLEND_FACTOR_ONE, .colorBlendOp = VK_BLEND_OP_ADD,
				.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE, .dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE, .alphaBlendOp = VK_BLEND_OP_ADD,
				.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
			}),
			//!< �����_�[�^�[�Q�b�g : ����
			VkPipelineColorBlendAttachmentState({
				.blendEnable = VK_FALSE,
				.srcColorBlendFactor = VK_BLEND_FACTOR_ONE, .dstColorBlendFactor = VK_BLEND_FACTOR_ONE, .colorBlendOp = VK_BLEND_OP_ADD,
				.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE, .dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE, .alphaBlendOp = VK_BLEND_OP_ADD,
				.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
			}),
#pragma endregion
		};
		const std::vector PCBASs_1 = {
			VkPipelineColorBlendAttachmentState({
				.blendEnable = VK_FALSE,
				.srcColorBlendFactor = VK_BLEND_FACTOR_ONE, .dstColorBlendFactor = VK_BLEND_FACTOR_ONE, .colorBlendOp = VK_BLEND_OP_ADD,
				.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE, .dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE, .alphaBlendOp = VK_BLEND_OP_ADD,
				. colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
			}),
		};
#ifdef USE_PIPELINE_SERIALIZE
		PipelineCacheSerializer PCS(Device, GetBasePath() + TEXT(".pco"), 2);
		//!< Pass0 : �p�C�v���C��
		Threads.emplace_back(std::thread::thread(VK::CreatePipeline_, std::ref(Pipelines[0]), Device, PipelineLayouts[0], RenderPasses[0], VK_PRIMITIVE_TOPOLOGY_PATCH_LIST, 1, PRSCI, PDSSCI_0, &PSSCIs_0[0], &PSSCIs_0[1], &PSSCIs_0[2], &PSSCIs_0[3], &PSSCIs_0[4], VIBDs, VIADs, PCBASs_0, PCS.GetPipelineCache(0)));
		//!< Pass1 : �p�C�v���C��
#ifdef USE_GBUFFER_VISUALIZE
		Threads.emplace_back(std::thread::thread(VK::CreatePipeline_, std::ref(Pipelines[1]), Device, PipelineLayouts[1], RenderPasses[1], VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP, 0, PRSCI, PDSSCI_1, &PSSCIs_1[0], &PSSCIs_1[1], nullptr, nullptr, &PSSCIs_1[2], VIBDs, VIADs, PCBASs_1, PCS.GetPipelineCache(1)));
#else
		Threads.emplace_back(std::thread::thread(VK::CreatePipeline_, std::ref(Pipelines[1]), Device, PipelineLayouts[1], RenderPasses[1], VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP, 0,  PRSCI, PDSSCI_1, &PSSCIs_1[0], &PSSCIs_1[1], nullptr, nullptr, nullptr, VIBDs, VIADs, PCBASs_1, PCS.GetPipelineCache(1)));
#endif
#else
		Threads.emplace_back(std::thread::thread(VK::CreatePipeline_, std::ref(Pipelines[0]), Device, PipelineLayouts[0], RenderPasses[0], VK_PRIMITIVE_TOPOLOGY_PATCH_LIST, 1, PRSCI, PDSSCI_0, &PSSCIs_0[0], &PSSCIs_0[1], &PSSCIs_0[2], &PSSCIs_0[3], &PSSCIs_0[4], VIBDs, VIADs, PCBASs_0));
#ifdef USE_GBUFFER_VISUALIZE
		Threads.emplace_back(std::thread::thread(VK::CreatePipeline_, std::ref(Pipelines[1]), Device, PipelineLayouts[1], RenderPasses[1], VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP, 0, PRSCI, PDSSCI_1, &PSSCIs_1[0], &PSSCIs_1[1], nullptr, nullptr, &PSSCIs_1[2], VIBDs, VIADs, PCBASs_1));
#else
		Threads.emplace_back(std::thread::thread(VK::CreatePipeline_, std::ref(Pipelines[1]), Device, PipelineLayouts[1], RenderPasses[1], VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP, 0, PRSCI, PDSSCI_1, &PSSCIs_1[0], &PSSCIs_1[1], nullptr, nullptr, nullptr, VIBDs, VIADs, PCBASs_1));
#endif
#endif
		for (auto& i : Threads) { i.join(); }
	}
	virtual void PopulateCommandBuffer(const size_t i) override;

private:
	struct DescriptorUpdateInfo_0
	{
		VkDescriptorBufferInfo DBI[1];
	};
	struct DescriptorUpdateInfo_1
	{
		//!< �����_�[�^�[�Q�b�g : �J���[(RenderTarget : Color)
		VkDescriptorImageInfo DII[1];
#pragma region MRT 
		//!< �����_�[�^�[�Q�b�g : �@��(RenderTarget : Normal)
		VkDescriptorImageInfo DII1[1];
		//!< �����_�[�^�[�Q�b�g : �[�x(RenderTarget : Depth)
		VkDescriptorImageInfo DII2[1];
		//!< �����_�[�^�[�Q�b�g : ����
		VkDescriptorImageInfo DII3[1];
#pragma endregion
		VkDescriptorBufferInfo DBI[1];
	};
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