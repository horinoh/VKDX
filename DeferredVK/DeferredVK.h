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
	virtual void OverridePhysicalDeviceFeatures(VkPhysicalDeviceFeatures& PDF) const { assert(PDF.tessellationShader && "tessellationShader not enabled"); Super::OverridePhysicalDeviceFeatures(PDF); }
	
#ifdef USE_GBUFFER_VISUALIZE
	virtual void CreateViewport(const float Width, const float Height, const float MinDepth = 0.0f, const float MaxDepth = 1.0f) override {
		const auto W = Width * 0.5f, H = Height * 0.5f;
		Viewports = {
	#ifdef USE_VIEWPORT_Y_UP
			//!< �S��ʗp(Fullscreen)
			{ 0.0f, Height, Width, -Height },
			//!< ������ʗp(DividedScreens)
			{ 0.0f, H, W, -H, MinDepth, MaxDepth },
			{ W, H, W, -H, MinDepth, MaxDepth },
			{ 0.0f, Height, W, -H, MinDepth, MaxDepth },
			{ W, Height, W, -H, MinDepth, MaxDepth },
	#else
			//!< �S��ʗp
			{ 0.0f, 0.0f, Width, Height },
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

		//!< �p�X1 : �Z�J���_���R�}���h�o�b�t�@
		assert(!SecondaryCommandPools.empty() && "");
		const auto PrevCount = SecondaryCommandBuffers.size();
		SecondaryCommandBuffers.resize(PrevCount + SwapchainImages.size());
		const VkCommandBufferAllocateInfo SCBAI = {
			VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
			nullptr,
			SecondaryCommandPools[0],
			VK_COMMAND_BUFFER_LEVEL_SECONDARY,
			static_cast<uint32_t>(SwapchainImages.size())
		};
		VERIFY_SUCCEEDED(vkAllocateCommandBuffers(Device, &SCBAI, &SecondaryCommandBuffers[PrevCount]));
	}
	virtual void CreateFramebuffer() override {
		assert(4 + 1 == ImageViews.size() && "");
		//!< �p�X0 : �t���[���o�b�t�@
		Framebuffers.push_back(VkFramebuffer());
		VK::CreateFramebuffer(Framebuffers.back(), RenderPasses[0], SurfaceExtent2D.width, SurfaceExtent2D.height, 1, {
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

		//!< �p�X1 : �t���[���o�b�t�@
		for (auto i : SwapchainImageViews) {
			Framebuffers.push_back(VkFramebuffer());
			VK::CreateFramebuffer(Framebuffers.back(), RenderPasses[1], SurfaceExtent2D.width, SurfaceExtent2D.height, 1, { i });
		}
	}
	virtual void CreateRenderPass() override {
		RenderPasses.resize(2);
		//!< �p�X0 : �����_�[�p�X
		{
			const std::array<VkAttachmentReference, 4> ColorAttach = { {
				//!< �����_�[�^�[�Q�b�g : �J���[(RenderTarget : Color)
				{ 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL }, 
#pragma region MRT 
				//!< �����_�[�^�[�Q�b�g : �@��(RenderTarget : Normal)
				{ 1, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL }, 
				//!< �����_�[�^�[�Q�b�g : �[�x(RenderTarget : Depth)
				{ 2, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL }, 
				//!< �����_�[�^�[�Q�b�g : ����
				{ 3, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL },
#pragma endregion
			} };
			const VkAttachmentReference DepthAttach = { static_cast<uint32_t>(ColorAttach.size()), VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL };
			VK::CreateRenderPass(RenderPasses[0], {
					//!< �A�^�b�`�����g(Attachment)
					//!< �����_�[�^�[�Q�b�g : �J���[(RenderTarget : Color)
					{
						0,
						ColorFormat,
						VK_SAMPLE_COUNT_1_BIT,
						VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE,
						VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
						VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
					},
#pragma region MRT 
					//!< �����_�[�^�[�Q�b�g : �@��(RenderTarget : Normal)
					{
						0,
						VK_FORMAT_A2B10G10R10_UNORM_PACK32, 
						VK_SAMPLE_COUNT_1_BIT,
						VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE,
						VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
						VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
					},
					//!< �����_�[�^�[�Q�b�g : �[�x(RenderTarget : Depth)
					{
						0,
						VK_FORMAT_R32_SFLOAT,
						VK_SAMPLE_COUNT_1_BIT,
						VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE,
						VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
						VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
					},
					//!< �����_�[�^�[�Q�b�g : ����
					{
						0,
						VK_FORMAT_B8G8R8A8_UNORM,
						VK_SAMPLE_COUNT_1_BIT,
						VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE,
						VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
						VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
					},
#pragma endregion
					//!< �[�x�o�b�t�@(Depth Buffer)
					{
						0,
						DepthFormat,
						VK_SAMPLE_COUNT_1_BIT,
						VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE,
						VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
						VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
					},
				}, {
					//!< �T�u�p�X(SubPass)
					{
						0,
						VK_PIPELINE_BIND_POINT_GRAPHICS,
						0, nullptr,
						static_cast<uint32_t>(ColorAttach.size()), ColorAttach.data(), nullptr,
						& DepthAttach,
						0, nullptr
					},
				}, {
					//!< �T�u�p�X�ˑ�
				});
		}
		//!< �p�X1 : �����_�[�p�X
		{
			const std::array<VkAttachmentReference, 1> ColorAttach = { { { 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL }, } };
			VK::CreateRenderPass(RenderPasses[1], {
				//!< �A�^�b�`�����g
				{
					0,
					ColorFormat,
					VK_SAMPLE_COUNT_1_BIT,
					VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_STORE,
					VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
					VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
				},
				}, {
					//!< �T�u�p�X
					{
						0,
						VK_PIPELINE_BIND_POINT_GRAPHICS,
						0, nullptr,
						static_cast<uint32_t>(ColorAttach.size()), ColorAttach.data(), nullptr,
						nullptr,
						0, nullptr
					},
				}, {
					//!< �T�u�p�X�ˑ�
				});
		}
	}
	virtual void CreateIndirectBuffer() override {
		//!< �p�X0 : �C���_�C���N�g�o�b�t�@(���b�V���`��p)
		CreateIndirectBuffer_DrawIndexed(1, 1);
		//!< �p�X1 : �C���_�C���N�g�o�b�t�@(�����_�[�e�N�X�`���`��p)
		CreateIndirectBuffer_Draw(4, 1);
	}
	virtual void CreateDescriptorSetLayout() override {
		//!< �p�X1 : �f�X�N���v�^�Z�b�g���C�A�E�g
		DescriptorSetLayouts.resize(1);
		assert(!Samplers.empty() && "");
		const std::array<VkSampler, 1> ISs = { Samplers[0] };
		VKExt::CreateDescriptorSetLayout(DescriptorSetLayouts[0], 0, {
			//!< �����_�[�^�[�Q�b�g : �J���[(RenderTarget : Color)
			{ 0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, static_cast<uint32_t>(ISs.size()), VK_SHADER_STAGE_FRAGMENT_BIT, ISs.data() },
#pragma region MRT 
			//!< �����_�[�^�[�Q�b�g : �@��(RenderTarget : Normal)
			{ 1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, static_cast<uint32_t>(ISs.size()), VK_SHADER_STAGE_FRAGMENT_BIT, ISs.data() },
			//!< �����_�[�^�[�Q�b�g : �[�x(RenderTarget : Depth)
			{ 2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, static_cast<uint32_t>(ISs.size()), VK_SHADER_STAGE_FRAGMENT_BIT, ISs.data() },
			//!< �����_�[�^�[�Q�b�g : ����
			{ 3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, static_cast<uint32_t>(ISs.size()), VK_SHADER_STAGE_FRAGMENT_BIT, ISs.data() },
#pragma endregion
		});
	}
	virtual void CreatePipelineLayout() override {
		assert(!DescriptorSetLayouts.empty() && "");
		PipelineLayouts.resize(2);
		//!< �p�X0 : �p�C�v���C�����C�A�E�g
		VKExt::CreatePipelineLayout(PipelineLayouts[0], {}, {});
		//!< �p�X1 : �p�C�v���C�����C�A�E�g
		VKExt::CreatePipelineLayout(PipelineLayouts[1], { DescriptorSetLayouts[0] }, {});
	}

	virtual void CreateDescriptorPool() override {
		//!< �p�X1 : �f�X�N���v�^�v�[��
		DescriptorPools.resize(1);
		VKExt::CreateDescriptorPool(DescriptorPools[0], 0, {
#pragma region MRT 
			//!< �����_�[�^�[�Q�b�g : �J���[(RenderTarget : Color), �@��(RenderTarget : Normal), �[�x(RenderTarget : Depth), ����
			{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 4 }
#pragma endregion
		});
	}
	virtual void AllocateDescriptorSet() override {
		//!< �p�X1 : �f�X�N���v�^�Z�b�g
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
		//!< �p�X1 : �f�X�N���v�^�A�b�v�f�[�g�e���v���[�g
		DescriptorUpdateTemplates.resize(1);
		assert(!DescriptorSetLayouts.empty() && "");
		VK::CreateDescriptorUpdateTemplate(DescriptorUpdateTemplates[0], {
			//!< �����_�[�^�[�Q�b�g : �J���[(RenderTarget : Color)
			{
				0, 0,
				_countof(DescriptorUpdateInfo::DescriptorImageInfos), VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
				offsetof(DescriptorUpdateInfo, DescriptorImageInfos), sizeof(DescriptorUpdateInfo)
			},
#pragma region MRT 
			//!< �����_�[�^�[�Q�b�g : �@��(RenderTarget : Normal)
			{
				1, 0,
				_countof(DescriptorUpdateInfo::DescriptorImageInfos1), VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
				offsetof(DescriptorUpdateInfo, DescriptorImageInfos1), sizeof(DescriptorUpdateInfo)
			},
			//!< �����_�[�^�[�Q�b�g : �[�x(RenderTarget : Depth)
			{
				2, 0,
				_countof(DescriptorUpdateInfo::DescriptorImageInfos2), VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
				offsetof(DescriptorUpdateInfo, DescriptorImageInfos2), sizeof(DescriptorUpdateInfo)
			},
			//!< �����_�[�^�[�Q�b�g : ����
			{
				3, 0,
				_countof(DescriptorUpdateInfo::DescriptorImageInfos3), VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
				offsetof(DescriptorUpdateInfo, DescriptorImageInfos3), sizeof(DescriptorUpdateInfo)
			},
#pragma endregion
			}, DescriptorSetLayouts[0]);
	}
	virtual void UpdateDescriptorSet() override {
		assert(4 + 1 == ImageViews.size() && "");
		const DescriptorUpdateInfo DUI = {
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
		};
		assert(!DescriptorSets.empty() && "");
		assert(!DescriptorUpdateTemplates.empty() && "");
		vkUpdateDescriptorSetWithTemplate(Device, DescriptorSets[0], DescriptorUpdateTemplates[0], &DUI);
	}

	virtual void CreateTexture() override {
		const VkExtent3D Extent = { SurfaceExtent2D.width, SurfaceExtent2D.height, 1 };
		const VkComponentMapping CompMap = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };
		//!< �����_�[�^�[�Q�b�g : �J���[(RenderTarget : Color)
		{
			Images.push_back(VkImage());
			CreateImage(&Images.back(), 0, VK_IMAGE_TYPE_2D, ColorFormat, Extent, 1, 1, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);

			uint32_t HeapIndex;
			VkDeviceSize Offset;
			SuballocateImageMemory(HeapIndex, Offset, Images.back(), VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

			ImageViews.push_back(VkImageView());
			CreateImageView(&ImageViews.back(), Images.back(), VK_IMAGE_VIEW_TYPE_2D, ColorFormat, CompMap, { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 });
		}
#pragma region MRT 
		//!< �����_�[�^�[�Q�b�g : �@��(RenderTarget : Normal)
		{
			const auto Format = VK_FORMAT_A2B10G10R10_UNORM_PACK32;
			Images.push_back(VkImage());
			CreateImage(&Images.back(), 0, VK_IMAGE_TYPE_2D, Format, Extent, 1, 1, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);

			uint32_t HeapIndex;
			VkDeviceSize Offset;
			SuballocateImageMemory(HeapIndex, Offset, Images.back(), VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

			ImageViews.push_back(VkImageView());
			CreateImageView(&ImageViews.back(), Images.back(), VK_IMAGE_VIEW_TYPE_2D, Format, CompMap, { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 });
		}
		//!< �����_�[�^�[�Q�b�g : �[�x(RenderTarget : Depth)
		{
			const auto Format = VK_FORMAT_R32_SFLOAT;
			Images.push_back(VkImage());
			CreateImage(&Images.back(), 0, VK_IMAGE_TYPE_2D, Format, Extent, 1, 1, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);

			uint32_t HeapIndex;
			VkDeviceSize Offset;
			SuballocateImageMemory(HeapIndex, Offset, Images.back(), VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

			ImageViews.push_back(VkImageView());
			CreateImageView(&ImageViews.back(), Images.back(), VK_IMAGE_VIEW_TYPE_2D, Format, CompMap, { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 });
		}
		//!< �����_�[�^�[�Q�b�g : ����
		{
			const auto Format = VK_FORMAT_B8G8R8A8_UNORM;
			Images.push_back(VkImage());
			CreateImage(&Images.back(), 0, VK_IMAGE_TYPE_2D, Format, Extent, 1, 1, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);

			uint32_t HeapIndex;
			VkDeviceSize Offset;
			SuballocateImageMemory(HeapIndex, Offset, Images.back(), VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

			ImageViews.push_back(VkImageView());
			CreateImageView(&ImageViews.back(), Images.back(), VK_IMAGE_VIEW_TYPE_2D, Format, CompMap, { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 });
		}
#pragma endregion
		//!< �[�x�o�b�t�@(Depth Buffer)
		{
			Images.push_back(VkImage());
			CreateImage(&Images.back(), 0, VK_IMAGE_TYPE_2D, DepthFormat, Extent, 1, 1, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);

			uint32_t HeapIndex;
			VkDeviceSize Offset;
			SuballocateImageMemory(HeapIndex, Offset, Images.back(), VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

			ImageViews.push_back(VkImageView());
			CreateImageView(&ImageViews.back(), Images.back(), VK_IMAGE_VIEW_TYPE_2D, DepthFormat, CompMap, { VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT, 0, 1, 0, 1 });
		}
	}
	virtual void CreateImmutableSampler() override {
		//!< �p�X1 : �C�~���[�^�u���T���v��
		Samplers.resize(1);
		const VkSamplerCreateInfo SCI = {
			VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
			nullptr,
			0,
			VK_FILTER_LINEAR, VK_FILTER_LINEAR, VK_SAMPLER_MIPMAP_MODE_LINEAR,
			VK_SAMPLER_ADDRESS_MODE_REPEAT, VK_SAMPLER_ADDRESS_MODE_REPEAT, VK_SAMPLER_ADDRESS_MODE_REPEAT,
			0.0f,
			VK_FALSE, 1.0f,
			VK_FALSE, VK_COMPARE_OP_NEVER,
			0.0f, 1.0f,
			VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE,
			VK_FALSE
		};
		VERIFY_SUCCEEDED(vkCreateSampler(Device, &SCI, GetAllocationCallbacks(), &Samplers[0]));
	}
	virtual void CreateShaderModules() override {
		const auto ShaderPath = GetBasePath();
		//!< �p�X0 : �V�F�[�_���W���[��
		ShaderModules.push_back(VKExt::CreateShaderModules((ShaderPath + TEXT(".vert.spv")).data()));
		ShaderModules.push_back(VKExt::CreateShaderModules((ShaderPath + TEXT(".frag.spv")).data()));
		ShaderModules.push_back(VKExt::CreateShaderModules((ShaderPath + TEXT(".tese.spv")).data()));
		ShaderModules.push_back(VKExt::CreateShaderModules((ShaderPath + TEXT(".tesc.spv")).data()));
		ShaderModules.push_back(VKExt::CreateShaderModules((ShaderPath + TEXT(".geom.spv")).data()));
		//!< �p�X1 : �V�F�[�_���W���[��
		ShaderModules.push_back(VKExt::CreateShaderModules((ShaderPath + TEXT("_1") + TEXT(".vert.spv")).data()));
#ifdef USE_GBUFFER_VISUALIZE
		ShaderModules.push_back(VKExt::CreateShaderModules((ShaderPath + TEXT("_gb_1") + TEXT(".frag.spv")).data()));
		ShaderModules.push_back(VKExt::CreateShaderModules((ShaderPath + TEXT("_gb_1") + TEXT(".geom.spv")).data()));
#else
		ShaderModules.push_back(VKExt::CreateShaderModules((ShaderPath + TEXT("_1") + TEXT(".frag.spv")).data()));
#endif
	}
	virtual void CreatePipelines() override {
		Pipelines.resize(2);
		std::vector<std::thread> Threads;
		const VkPipelineDepthStencilStateCreateInfo PDSSCI = {
			VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
			nullptr,
			0,
			VK_FALSE,
			VK_FALSE,
			VK_COMPARE_OP_LESS_OR_EQUAL,
			VK_FALSE,
			VK_FALSE,
			{ VK_STENCIL_OP_KEEP, VK_STENCIL_OP_KEEP, VK_STENCIL_OP_KEEP, VK_COMPARE_OP_NEVER, 0, 0, 0 },
			{ VK_STENCIL_OP_KEEP, VK_STENCIL_OP_KEEP, VK_STENCIL_OP_KEEP, VK_COMPARE_OP_ALWAYS, 0, 0, 0 },
			0.0f, 1.0f
		};
		const std::array<VkPipelineShaderStageCreateInfo, 5> PSSCIs_0 = {
			VkPipelineShaderStageCreateInfo({ VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, nullptr, 0, VK_SHADER_STAGE_VERTEX_BIT, ShaderModules[0], "main", nullptr }),
			VkPipelineShaderStageCreateInfo({ VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, nullptr, 0, VK_SHADER_STAGE_FRAGMENT_BIT, ShaderModules[1], "main", nullptr }),
			VkPipelineShaderStageCreateInfo({ VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, nullptr, 0, VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT, ShaderModules[2], "main", nullptr }),
			VkPipelineShaderStageCreateInfo({ VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, nullptr, 0, VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT, ShaderModules[3], "main", nullptr }),
			VkPipelineShaderStageCreateInfo({ VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, nullptr, 0, VK_SHADER_STAGE_GEOMETRY_BIT, ShaderModules[4], "main", nullptr }),
		};
#ifdef USE_GBUFFER_VISUALIZE
		const std::array<VkPipelineShaderStageCreateInfo, 3> PSSCIs_1 = {
			VkPipelineShaderStageCreateInfo({ VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, nullptr, 0, VK_SHADER_STAGE_VERTEX_BIT, ShaderModules[5], "main", nullptr }),
			VkPipelineShaderStageCreateInfo({ VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, nullptr, 0, VK_SHADER_STAGE_FRAGMENT_BIT, ShaderModules[6], "main", nullptr }),
			VkPipelineShaderStageCreateInfo({ VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, nullptr, 0, VK_SHADER_STAGE_GEOMETRY_BIT, ShaderModules[7], "main", nullptr }),
		};
#else
		const std::array<VkPipelineShaderStageCreateInfo, 2> PSSCIs_1 = {
			VkPipelineShaderStageCreateInfo({ VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, nullptr, 0, VK_SHADER_STAGE_VERTEX_BIT, ShaderModules[5], "main", nullptr }),
			VkPipelineShaderStageCreateInfo({ VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, nullptr, 0, VK_SHADER_STAGE_FRAGMENT_BIT, ShaderModules[6], "main", nullptr }),
		};
#endif
		const std::vector<VkVertexInputBindingDescription> VIBDs = {};
		const std::vector<VkVertexInputAttributeDescription> VIADs = {};
		const std::vector< VkPipelineColorBlendAttachmentState> PCBASs_0 = {
			//!< �����_�[�^�[�Q�b�g : �J���[(RenderTarget : Color)
			{ 
				VK_FALSE, VK_BLEND_FACTOR_ONE, 
				VK_BLEND_FACTOR_ONE, VK_BLEND_OP_ADD, 
				VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ONE, VK_BLEND_OP_ADD,
				VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
			},
#pragma region MRT 
			//!< �����_�[�^�[�Q�b�g : �@��(RenderTarget : Normal)
			{
				VK_FALSE, VK_BLEND_FACTOR_ONE,
				VK_BLEND_FACTOR_ONE, VK_BLEND_OP_ADD,
				VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ONE, VK_BLEND_OP_ADD,
				VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
			},
			//!< �����_�[�^�[�Q�b�g : �[�x(RenderTarget : Depth)
			{
				VK_FALSE, VK_BLEND_FACTOR_ONE,
				VK_BLEND_FACTOR_ONE, VK_BLEND_OP_ADD,
				VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ONE, VK_BLEND_OP_ADD,
				VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
			},
			//!< �����_�[�^�[�Q�b�g : ����
			{
				VK_FALSE, VK_BLEND_FACTOR_ONE,
				VK_BLEND_FACTOR_ONE, VK_BLEND_OP_ADD,
				VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ONE, VK_BLEND_OP_ADD,
				VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
			},
#pragma endregion
		};
		const std::vector< VkPipelineColorBlendAttachmentState> PCBASs_1 = {
			{
				VK_FALSE, VK_BLEND_FACTOR_ONE,
				VK_BLEND_FACTOR_ONE, VK_BLEND_OP_ADD,
				VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ONE, VK_BLEND_OP_ADD,
				VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
			},
		};
#ifdef USE_PIPELINE_SERIALIZE
		PipelineCacheSerializer PCS(Device, GetBasePath() + TEXT(".pco"), 2);
		//!< �p�X0 : �p�C�v���C��
		Threads.push_back(std::thread::thread(VK::CreatePipeline, std::ref(Pipelines[0]), Device, PipelineLayouts[0], RenderPasses[0], VK_PRIMITIVE_TOPOLOGY_PATCH_LIST, 1, PDSSCI, &PSSCIs_0[0], &PSSCIs_0[1], &PSSCIs_0[2], &PSSCIs_0[3], &PSSCIs_0[4], VIBDs, VIADs, PCBASs_0, PCS.GetPipelineCache(0)));
		//!< �p�X1 : �p�C�v���C��
#ifdef USE_GBUFFER_VISUALIZE
		Threads.push_back(std::thread::thread(VK::CreatePipeline, std::ref(Pipelines[1]), Device, PipelineLayouts[1], RenderPasses[1], VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP, 0, PDSSCI, &PSSCIs_1[0], &PSSCIs_1[1], nullptr, nullptr, &PSSCIs_1[2], VIBDs, VIADs, PCBASs_1, PCS.GetPipelineCache(1)));
#else
		Threads.push_back(std::thread::thread(VK::CreatePipeline, std::ref(Pipelines[1]), Device, PipelineLayouts[1], RenderPasses[1], VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP, 0, PDSSCI, &PSSCIs_1[0], &PSSCIs_1[1], nullptr, nullptr, nullptr, VIBDs, VIADs, PCBASs_1, PCS.GetPipelineCache(1)));
#endif
#else
		Threads.push_back(std::thread::thread(VK::CreatePipeline, std::ref(Pipelines[0]), Device, PipelineLayouts[0], RenderPasses[0], VK_PRIMITIVE_TOPOLOGY_PATCH_LIST, 1, PDSSCI, &PSSCIs_0[0], &PSSCIs_0[1], &PSSCIs_0[2], &PSSCIs_0[3], &PSSCIs_0[4], VIBDs, VIADs, PCBASs_0));
#ifdef USE_GBUFFER_VISUALIZE
		Threads.push_back(std::thread::thread(VK::CreatePipeline, std::ref(Pipelines[1]), Device, PipelineLayouts[1], RenderPasses[1], VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP, 0, PDSSCI, &PSSCIs_1[0], &PSSCIs_1[1], nullptr, nullptr, &PSSCIs_1[2], VIBDs, VIADs, PCBASs_1));
#else
		Threads.push_back(std::thread::thread(VK::CreatePipeline, std::ref(Pipelines[1]), Device, PipelineLayouts[1], RenderPasses[1], VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP, 0, PDSSCI, &PSSCIs_1[0], &PSSCIs_1[1], nullptr, nullptr, nullptr, VIBDs, VIADs, PCBASs_1));
#endif
#endif
		for (auto& i : Threads) { i.join(); }
	}
	virtual void PopulateCommandBuffer(const size_t i) override;

private:
	struct DescriptorUpdateInfo
	{
		//!< �����_�[�^�[�Q�b�g : �J���[(RenderTarget : Color)
		VkDescriptorImageInfo DescriptorImageInfos[1];
#pragma region MRT 
		//!< �����_�[�^�[�Q�b�g : �@��(RenderTarget : Normal)
		VkDescriptorImageInfo DescriptorImageInfos1[1];
		//!< �����_�[�^�[�Q�b�g : �[�x(RenderTarget : Depth)
		VkDescriptorImageInfo DescriptorImageInfos2[1];
		//!< �����_�[�^�[�Q�b�g : ����
		VkDescriptorImageInfo DescriptorImageInfos3[1];
#pragma endregion
	};
};
#pragma endregion