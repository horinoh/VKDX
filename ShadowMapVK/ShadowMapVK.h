#pragma once

#include "resource.h"

#pragma region Code
#include "../VKExt.h"

class ShadowMapVK : public VKExt
{
private:
	using Super = VKExt;
public:
	ShadowMapVK() : Super() {}
	virtual ~ShadowMapVK() {}

protected:
	virtual void OnTimer(HWND hWnd, HINSTANCE hInstance) override {
		Super::OnTimer(hWnd, hInstance);

		//Tr.World = glm::rotate(glm::mat4(1.0f), glm::radians(Degree), glm::vec3(1.0f, 0.0f, 0.0f));
		//Degree += 1.0f;

		CopyToHostVisibleDeviceMemory(DeviceMemories[HeapIndex], sizeof(Tr), &Tr, Offset);
	}
	virtual void OverridePhysicalDeviceFeatures(VkPhysicalDeviceFeatures& PDF) const { assert(PDF.tessellationShader && "tessellationShader not enabled"); Super::OverridePhysicalDeviceFeatures(PDF); }

	virtual void AllocateCommandBuffer() override {
		Super::AllocateCommandBuffer();

		//!< パス1 : セカンダリコマンドバッファ
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
		//!< パス0 : フレームバッファ
		{
			assert(1 + 1 == ImageViews.size() && "");
			Framebuffers.push_back(VkFramebuffer());
			VK::CreateFramebuffer(Framebuffers.back(), RenderPasses[0], SurfaceExtent2D.width, SurfaceExtent2D.height, 1, {
				//!< レンダーターゲット : 深度(RenderTarget : Depth)
				ImageViews[0],
				//!< 深度バッファ(Depth Buffer)
				ImageViews[1],
			});
		}

		//!< パス1 : フレームバッファ
		{
			for (auto i : SwapchainImageViews) {
				Framebuffers.push_back(VkFramebuffer());
				VK::CreateFramebuffer(Framebuffers.back(), RenderPasses[1], SurfaceExtent2D.width, SurfaceExtent2D.height, 1, { i });
			}
		}
	}
	virtual void CreateRenderPass() override {
		//!< パス0 : レンダーパス
		{
			RenderPasses.push_back(VkRenderPass());
			const std::array<VkAttachmentReference, 1> ColorAttach = { {
				//!< レンダーターゲット : 深度(RenderTarget : Depth)
				{ 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL },
			} };
			const VkAttachmentReference DepthAttach = { static_cast<uint32_t>(ColorAttach.size()), VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL };
			VK::CreateRenderPass(RenderPasses[0], {
				//!< アタッチメント(Attachment)
				//!< レンダーターゲット : 深度(RenderTarget : Depth)
				{
					0,
					VK_FORMAT_R32_SFLOAT,
					VK_SAMPLE_COUNT_1_BIT,
					VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE,
					VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
					VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
				},
				//!< 深度バッファ(Depth Buffer)
				{
					0,
					DepthFormat,
					VK_SAMPLE_COUNT_1_BIT,
					VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE,
					VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
					VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
				},
				}, {
					//!< サブパス(SubPass)
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
		//!< パス1 : レンダーパス
		{
			RenderPasses.push_back(VkRenderPass());
			const std::array<VkAttachmentReference, 1> ColorAttach = { { { 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL }, } };
			VK::CreateRenderPass(RenderPasses[1], {
				//!< アタッチメント
				{
					0,
					ColorFormat,
					VK_SAMPLE_COUNT_1_BIT,
					VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_STORE,
					VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
					VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
				},
				}, {
					//!< サブパス
					{
						0,
						VK_PIPELINE_BIND_POINT_GRAPHICS,
						0, nullptr,
						static_cast<uint32_t>(ColorAttach.size()), ColorAttach.data(), nullptr,
						nullptr,
						0, nullptr
					},
				}, {
					//!< サブパス依存
				});
		}
	}
	virtual void CreateIndirectBuffer() override {
		//!< パス0 : インダイレクトバッファ(メッシュ描画用)
		CreateIndirectBuffer_DrawIndexed(1, 2);
		//!< パス1 : インダイレクトバッファ(レンダーテクスチャ描画用)
		CreateIndirectBuffer_Draw(4, 1);
	}
	virtual void CreateDescriptorSetLayout() override {
		assert(!Samplers.empty() && "");
		const std::array<VkSampler, 1> ISs = { Samplers[0] };

		//!< パス0 : デスクリプタセットレイアウト
		{
			DescriptorSetLayouts.push_back(VkDescriptorSetLayout());
			VKExt::CreateDescriptorSetLayout(DescriptorSetLayouts.back(), 0, {
				{ 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_GEOMETRY_BIT, nullptr },
			});
		}

		//!< パス1 : デスクリプタセットレイアウト
		{
			DescriptorSetLayouts.push_back(VkDescriptorSetLayout());
			VKExt::CreateDescriptorSetLayout(DescriptorSetLayouts.back(), 0, {
				//!< レンダーターゲット : 深度(RenderTarget : Depth)
				{ 0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, static_cast<uint32_t>(ISs.size()), VK_SHADER_STAGE_FRAGMENT_BIT, ISs.data() },
				{ 1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr },
			});
		}
	}
	virtual void CreatePipelineLayout() override {
		assert(2 == DescriptorSetLayouts.size() && "");

		//!< パス0 : パイプラインレイアウト
		{
			PipelineLayouts.push_back(VkPipelineLayout());
			VKExt::CreatePipelineLayout(PipelineLayouts.back(), { DescriptorSetLayouts[0] }, {});
		}

		//!< パス1 : パイプラインレイアウト
		{
			PipelineLayouts.push_back(VkPipelineLayout());
			VKExt::CreatePipelineLayout(PipelineLayouts.back(), { DescriptorSetLayouts[1] }, {});
		}
	}

	virtual void CreateDescriptorPool() override {
		//!< パス0,1 : デスクリプタプール
		DescriptorPools.push_back(VkDescriptorPool());
		VKExt::CreateDescriptorPool(DescriptorPools.back(), 0, {
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1 + 1 },
			//!< レンダーターゲット : 深度(RenderTarget : Depth)
			{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1 },
		});
	}
	virtual void AllocateDescriptorSet() override {
		assert(2 == DescriptorSetLayouts.size() && "");
		assert(!DescriptorPools.empty() && "");

		//!< パス0 : デスクリプタセット
		{
			DescriptorSets.push_back(VkDescriptorSet());
			const std::array<VkDescriptorSetLayout, 1> DSLs = { DescriptorSetLayouts[0] };
			const VkDescriptorSetAllocateInfo DSAI = {
				VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
				nullptr,
				DescriptorPools[0],
				static_cast<uint32_t>(DSLs.size()), DSLs.data()
			};
			VERIFY_SUCCEEDED(vkAllocateDescriptorSets(Device, &DSAI, &DescriptorSets.back()));
		}

		//!< パス1 : デスクリプタセット
		{
			DescriptorSets.push_back(VkDescriptorSet());
			const std::array<VkDescriptorSetLayout, 1> DSLs = { DescriptorSetLayouts[1] };
			const VkDescriptorSetAllocateInfo DSAI = {
				VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
				nullptr,
				DescriptorPools[0],
				static_cast<uint32_t>(DSLs.size()), DSLs.data()
			};
			VERIFY_SUCCEEDED(vkAllocateDescriptorSets(Device, &DSAI, &DescriptorSets.back()));
		}
	}
	virtual void CreateDescriptorUpdateTemplate() override {
		assert(2 == DescriptorSetLayouts.size() && "");

		//!< パス0 : デスクリプタアップデートテンプレート
		{
			DescriptorUpdateTemplates.push_back(VkDescriptorUpdateTemplate());
			VK::CreateDescriptorUpdateTemplate(DescriptorUpdateTemplates.back(), {
				{
					0, 0,
					_countof(DescriptorUpdateInfo_0::DBI), VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
					offsetof(DescriptorUpdateInfo_0, DBI), sizeof(DescriptorUpdateInfo_0)
				},
				}, DescriptorSetLayouts[0]);
		}

		//!< パス1 : デスクリプタアップデートテンプレート
		{
			DescriptorUpdateTemplates.push_back(VkDescriptorUpdateTemplate());
			VK::CreateDescriptorUpdateTemplate(DescriptorUpdateTemplates.back(), {
				//!< レンダーターゲット : 深度(RenderTarget : Depth)
				{
					0, 0,
					_countof(DescriptorUpdateInfo_1::DII), VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
					offsetof(DescriptorUpdateInfo_1, DII), sizeof(DescriptorUpdateInfo_1)
				},
				{
					1, 0,
					_countof(DescriptorUpdateInfo_1::DBI), VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
					offsetof(DescriptorUpdateInfo_1, DBI), sizeof(DescriptorUpdateInfo_1)
				},
				}, DescriptorSetLayouts[1]);
		}
	}
	virtual void UpdateDescriptorSet() override {
		assert(2 == DescriptorSets.size() && "");
		assert(2 == DescriptorUpdateTemplates.size() && "");

		//!< パス0 :
		{
			assert(!UniformBuffers.empty() && "");
			const DescriptorUpdateInfo_0 DUI = {
				{ UniformBuffers[0], Offset, VK_WHOLE_SIZE },
			};
			vkUpdateDescriptorSetWithTemplate(Device, DescriptorSets[0], DescriptorUpdateTemplates[0], &DUI);
		}
		//!< パス1 :
		{
			assert(1 <= ImageViews.size() && "");
			const DescriptorUpdateInfo_1 DUI = {
				//!< レンダーターゲット : 深度(RenderTarget : Depth)
				{ VK_NULL_HANDLE, ImageViews[0], VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL },
				{ UniformBuffers[0], Offset, VK_WHOLE_SIZE },
			};
			vkUpdateDescriptorSetWithTemplate(Device, DescriptorSets[1], DescriptorUpdateTemplates[1], &DUI);
		}
	}

	virtual void CreateTexture() override {
		const VkExtent3D Extent = { SurfaceExtent2D.width, SurfaceExtent2D.height, 1 };
		const VkComponentMapping CompMap = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };
		//!< レンダーターゲット : 深度(RenderTarget : Depth)
		{
			const auto Format = VK_FORMAT_R32_SFLOAT;
			Images.push_back(VkImage());
			CreateImage(&Images.back(), 0, VK_IMAGE_TYPE_2D, Format, Extent, 1, 1, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);

			uint32_t Idx;
			VkDeviceSize Ofs;
			SuballocateImageMemory(Idx, Ofs, Images.back(), VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

			ImageViews.push_back(VkImageView());
			CreateImageView(&ImageViews.back(), Images.back(), VK_IMAGE_VIEW_TYPE_2D, Format, CompMap, { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 });
		}
		//!< 深度バッファ(Depth Buffer)
		{
			Images.push_back(VkImage());
			CreateImage(&Images.back(), 0, VK_IMAGE_TYPE_2D, DepthFormat, Extent, 1, 1, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);

			uint32_t Idx;
			VkDeviceSize Ofs;
			SuballocateImageMemory(Idx, Ofs, Images.back(), VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

			ImageViews.push_back(VkImageView());
			CreateImageView(&ImageViews.back(), Images.back(), VK_IMAGE_VIEW_TYPE_2D, DepthFormat, CompMap, { VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT, 0, 1, 0, 1 });
		}
	}
	virtual void CreateImmutableSampler() override {
		//!< パス1 : イミュータブルサンプラ
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
	virtual void CreateUniformBuffer() override {
		{
			const auto Fov = 0.16f * glm::pi<float>();
			const auto Aspect = GetAspectRatioOfClientRect();
			const auto ZFar = 4.0f;
			const auto ZNear = 2.0f;
			const auto CamPos = glm::vec3(0.0f, 0.0f, 3.0f);
			const auto CamTag = glm::vec3(0.0f);
			const auto CamUp = glm::vec3(0.0f, 1.0f, 0.0f);
			const auto Projection = glm::perspective(Fov, Aspect, ZNear, ZFar);
			const auto View = glm::lookAt(CamPos, CamTag, CamUp);
			const auto World = glm::mat4(1.0f);

			const auto ViewProjection = Projection * View;
			const auto InverseViewProjection = glm::inverse(ViewProjection);

			Tr = Transform({ Projection, View, World, InverseViewProjection });

			UniformBuffers.push_back(VkBuffer());
			CreateBuffer(&UniformBuffers.back(), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, sizeof(Tr));
			SuballocateBufferMemory(HeapIndex, Offset, UniformBuffers.back(), VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
		}
	}
	virtual void CreateShaderModules() override {
		const auto ShaderPath = GetBasePath();
		//!< パス0 : シェーダモジュール
		ShaderModules.push_back(VKExt::CreateShaderModules((ShaderPath + TEXT(".vert.spv")).data()));
		ShaderModules.push_back(VKExt::CreateShaderModules((ShaderPath + TEXT(".frag.spv")).data()));
		ShaderModules.push_back(VKExt::CreateShaderModules((ShaderPath + TEXT(".tese.spv")).data()));
		ShaderModules.push_back(VKExt::CreateShaderModules((ShaderPath + TEXT(".tesc.spv")).data()));
		ShaderModules.push_back(VKExt::CreateShaderModules((ShaderPath + TEXT(".geom.spv")).data()));
		//!< パス1 : シェーダモジュール
		ShaderModules.push_back(VKExt::CreateShaderModules((ShaderPath + TEXT("_1") + TEXT(".vert.spv")).data()));
#ifdef USE_SHADOWMAP_VISUALIZE
		ShaderModules.push_back(VKExt::CreateShaderModules((ShaderPath + TEXT("_sm_1") + TEXT(".frag.spv")).data()));
#else
		ShaderModules.push_back(VKExt::CreateShaderModules((ShaderPath + TEXT("_1") + TEXT(".frag.spv")).data()));
#endif
	}
	virtual void CreatePipelines() override {
		Pipelines.resize(2);
		std::vector<std::thread> Threads;
		const VkPipelineDepthStencilStateCreateInfo PDSSCI_0 = {
			VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
			nullptr,
			0,
			VK_TRUE, VK_TRUE, VK_COMPARE_OP_LESS_OR_EQUAL,
			VK_FALSE,
			VK_FALSE, { VK_STENCIL_OP_KEEP, VK_STENCIL_OP_KEEP, VK_STENCIL_OP_KEEP, VK_COMPARE_OP_NEVER, 0, 0, 0 }, { VK_STENCIL_OP_KEEP, VK_STENCIL_OP_KEEP, VK_STENCIL_OP_KEEP, VK_COMPARE_OP_ALWAYS, 0, 0, 0 },
			0.0f, 1.0f
		};
		const VkPipelineDepthStencilStateCreateInfo PDSSCI_1 = {
			VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
			nullptr,
			0,
			VK_FALSE, VK_FALSE, VK_COMPARE_OP_LESS_OR_EQUAL,
			VK_FALSE,
			VK_FALSE, { VK_STENCIL_OP_KEEP, VK_STENCIL_OP_KEEP, VK_STENCIL_OP_KEEP, VK_COMPARE_OP_NEVER, 0, 0, 0 }, { VK_STENCIL_OP_KEEP, VK_STENCIL_OP_KEEP, VK_STENCIL_OP_KEEP, VK_COMPARE_OP_ALWAYS, 0, 0, 0 },
			0.0f, 1.0f
		};
		const std::array<VkPipelineShaderStageCreateInfo, 5> PSSCIs_0 = {
			VkPipelineShaderStageCreateInfo({ VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, nullptr, 0, VK_SHADER_STAGE_VERTEX_BIT, ShaderModules[0], "main", nullptr }),
			VkPipelineShaderStageCreateInfo({ VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, nullptr, 0, VK_SHADER_STAGE_FRAGMENT_BIT, ShaderModules[1], "main", nullptr }),
			VkPipelineShaderStageCreateInfo({ VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, nullptr, 0, VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT, ShaderModules[2], "main", nullptr }),
			VkPipelineShaderStageCreateInfo({ VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, nullptr, 0, VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT, ShaderModules[3], "main", nullptr }),
			VkPipelineShaderStageCreateInfo({ VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, nullptr, 0, VK_SHADER_STAGE_GEOMETRY_BIT, ShaderModules[4], "main", nullptr }),
		};
#ifdef USE_SHADOWMAP_VISUALIZE
		const std::array<VkPipelineShaderStageCreateInfo, 3> PSSCIs_1 = {
			VkPipelineShaderStageCreateInfo({ VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, nullptr, 0, VK_SHADER_STAGE_VERTEX_BIT, ShaderModules[5], "main", nullptr }),
			VkPipelineShaderStageCreateInfo({ VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, nullptr, 0, VK_SHADER_STAGE_FRAGMENT_BIT, ShaderModules[6], "main", nullptr }),
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
			//!< レンダーターゲット : 深度(RenderTarget : Depth)
			{
				VK_FALSE, VK_BLEND_FACTOR_ONE,
				VK_BLEND_FACTOR_ONE, VK_BLEND_OP_ADD,
				VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ONE, VK_BLEND_OP_ADD,
				VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
			},
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
		//!< パス0 : パイプライン
		Threads.push_back(std::thread::thread(VK::CreatePipeline, std::ref(Pipelines[0]), Device, PipelineLayouts[0], RenderPasses[0], VK_PRIMITIVE_TOPOLOGY_PATCH_LIST, 1, PDSSCI_0, &PSSCIs_0[0], &PSSCIs_0[1], &PSSCIs_0[2], &PSSCIs_0[3], &PSSCIs_0[4], VIBDs, VIADs, PCBASs_0, PCS.GetPipelineCache(0)));
		//!< パス1 : パイプライン
#ifdef USE_SHADOWMAP_VISUALIZE
		Threads.push_back(std::thread::thread(VK::CreatePipeline, std::ref(Pipelines[1]), Device, PipelineLayouts[1], RenderPasses[1], VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP, 0, PDSSCI_1, &PSSCIs_1[0], &PSSCIs_1[1], nullptr, nullptr, nullptr, VIBDs, VIADs, PCBASs_1, PCS.GetPipelineCache(1)));
#else
		Threads.push_back(std::thread::thread(VK::CreatePipeline, std::ref(Pipelines[1]), Device, PipelineLayouts[1], RenderPasses[1], VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP, 0, PDSSCI_1, &PSSCIs_1[0], &PSSCIs_1[1], nullptr, nullptr, nullptr, VIBDs, VIADs, PCBASs_1, PCS.GetPipelineCache(1)));
#endif
#else
		Threads.push_back(std::thread::thread(VK::CreatePipeline, std::ref(Pipelines[0]), Device, PipelineLayouts[0], RenderPasses[0], VK_PRIMITIVE_TOPOLOGY_PATCH_LIST, 1, PDSSCI_0, &PSSCIs_0[0], &PSSCIs_0[1], &PSSCIs_0[2], &PSSCIs_0[3], &PSSCIs_0[4], VIBDs, VIADs, PCBASs_0));
#ifdef USE_SHADOWMAP_VISUALIZE
		Threads.push_back(std::thread::thread(VK::CreatePipeline, std::ref(Pipelines[1]), Device, PipelineLayouts[1], RenderPasses[1], VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP, 0, PDSSCI_1, &PSSCIs_1[0], &PSSCIs_1[1], nullptr, nullptr, nullptr, VIBDs, VIADs, PCBASs_1));
#else
		Threads.push_back(std::thread::thread(VK::CreatePipeline, std::ref(Pipelines[1]), Device, PipelineLayouts[1], RenderPasses[1], VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP, 0, PDSSCI_1, &PSSCIs_1[0], &PSSCIs_1[1], nullptr, nullptr, nullptr, VIBDs, VIADs, PCBASs_1));
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
		//!< レンダーターゲット : 深度(RenderTarget : Depth)
		VkDescriptorImageInfo DII[1];
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
	uint32_t HeapIndex;
	VkDeviceSize Offset;
};
#pragma endregion