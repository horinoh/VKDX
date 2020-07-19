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

		Tr.World = glm::rotate(glm::mat4(1.0f), glm::radians(Degree), glm::vec3(0.0f, 1.0f, 0.0f));
		Degree += 1.0f;

		CopyToHostVisibleDeviceMemory(UniformBuffers[0].DeviceMemory, sizeof(Tr), &Tr, 0);
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
	virtual void CreateRenderPass() override {
		//!< パス0 : レンダーパス
		{
			RenderPasses.push_back(VkRenderPass());
			const std::array<VkAttachmentReference, 0> ColorAttach = { { } };
			const VkAttachmentReference DepthAttach = { static_cast<uint32_t>(ColorAttach.size()), VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL };
			VK::CreateRenderPass(RenderPasses.back(), {
					//!< アタッチメント(Attachment)
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
			VK::CreateRenderPass(RenderPasses.back(), {
					//!< アタッチメント
					{
						0,
						ColorFormat,
						VK_SAMPLE_COUNT_1_BIT,
#ifdef USE_SHADOWMAP_VISUALIZE
						VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_STORE,
#else
						VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE,
#endif
						VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
						VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
					},
#ifndef USE_SHADOWMAP_VISUALIZE
					{
						0,
						DepthFormat,
						VK_SAMPLE_COUNT_1_BIT,
						VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE,
						VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
						VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
					},
#endif
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
	virtual void CreateFramebuffer() override {
		//!< パス0 : フレームバッファ
		{
			Framebuffers.push_back(VkFramebuffer());
			VK::CreateFramebuffer(Framebuffers.back(), RenderPasses[0], ShadowMapExtent.width, ShadowMapExtent.height, 1, {
				ImageViews[0],
			});
		}

		//!< パス1 : フレームバッファ
		{
			for (auto i : SwapchainImageViews) {
				Framebuffers.push_back(VkFramebuffer());
				VK::CreateFramebuffer(Framebuffers.back(), RenderPasses[1], SurfaceExtent2D.width, SurfaceExtent2D.height, 1, { 
					i,
#ifndef USE_SHADOWMAP_VISUALIZE
					ImageViews[1],
#endif
				});
			}
		}
	}
	virtual void CreateIndirectBuffer() override {
		//!< パス0 : インダイレクトバッファ(シャドウキャスタ描画用)
		CreateIndirectBuffer_DrawIndexed(1, 1);
#ifdef USE_SHADOWMAP_VISUALIZE
		//!< パス1 : インダイレクトバッファ(シャドウマップ描画用)
		CreateIndirectBuffer_Draw(4, 1);
#else
		//!< パス1 : インダイレクトバッファ(シャドウレシーバ描画用)
		CreateIndirectBuffer_DrawIndexed(1, 2);
#endif
	}
	virtual void CreateDescriptorSetLayout() override {		
		//!< パス0 : デスクリプタセットレイアウト
		{
			DescriptorSetLayouts.push_back(VkDescriptorSetLayout());
			VKExt::CreateDescriptorSetLayout(DescriptorSetLayouts.back(), 0, {
				{ 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_GEOMETRY_BIT, nullptr },
			});
		}

		//!< パス1 : デスクリプタセットレイアウト
		{
			assert(!Samplers.empty() && "");
			const std::array<VkSampler, 1> ISs = { Samplers[0] };
			DescriptorSetLayouts.push_back(VkDescriptorSetLayout());
			VKExt::CreateDescriptorSetLayout(DescriptorSetLayouts.back(), 0, {
				//!< レンダーターゲット : 深度(RenderTarget : Depth)
				{ 0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, static_cast<uint32_t>(ISs.size()), VK_SHADER_STAGE_FRAGMENT_BIT, ISs.data() },
#ifndef USE_SHADOWMAP_VISUALIZE
				{ 1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_GEOMETRY_BIT, nullptr },
#endif
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
		//!< パス0, 1 : デスクリプタプール
		DescriptorPools.push_back(VkDescriptorPool());
		VKExt::CreateDescriptorPool(DescriptorPools.back(), 0, {
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1 + 1 },
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
				{
					0, 0,
					_countof(DescriptorUpdateInfo_1::DII), VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
					offsetof(DescriptorUpdateInfo_1, DII), sizeof(DescriptorUpdateInfo_1)
				},
#ifndef USE_SHADOWMAP_VISUALIZE
				{
					1, 0,
					_countof(DescriptorUpdateInfo_1::DBI), VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
					offsetof(DescriptorUpdateInfo_1, DBI), sizeof(DescriptorUpdateInfo_1)
				},
#endif
				}, DescriptorSetLayouts[1]);
		}
	}
	virtual void UpdateDescriptorSet() override {
		assert(2 == DescriptorSets.size() && "");
		assert(2 == DescriptorUpdateTemplates.size() && "");

		//!< パス0 :
		{
			const DescriptorUpdateInfo_0 DUI = {
				{ UniformBuffers[0].Buffer, 0, VK_WHOLE_SIZE },
			};
			vkUpdateDescriptorSetWithTemplate(Device, DescriptorSets[0], DescriptorUpdateTemplates[0], &DUI);
		}
		//!< パス1 :
		{
			const DescriptorUpdateInfo_1 DUI = {
				{ VK_NULL_HANDLE, ImageViews[0], VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL },
				{ UniformBuffers[0].Buffer, 0, VK_WHOLE_SIZE },
			};
			vkUpdateDescriptorSetWithTemplate(Device, DescriptorSets[1], DescriptorUpdateTemplates[1], &DUI);
		}
	}

	virtual void CreateTexture() override {
		const VkComponentMapping CompMap = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };
		{
			const VkExtent3D Extent = { ShadowMapExtent.width, ShadowMapExtent.height, 1 };

			Images.push_back(Image());
			VK::CreateImage(&Images.back().Image, 0, VK_IMAGE_TYPE_2D, DepthFormat, Extent, 1, 1, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);

			AllocateDeviceMemory(&Images.back().DeviceMemory, Images.back().Image, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
			VERIFY_SUCCEEDED(vkBindImageMemory(Device, Images.back().Image, Images.back().DeviceMemory, 0));

			ImageViews.push_back(VkImageView());
			VK::CreateImageView(&ImageViews.back(), Images.back().Image, VK_IMAGE_VIEW_TYPE_2D, DepthFormat, CompMap, { VK_IMAGE_ASPECT_DEPTH_BIT, 0, 1, 0, 1 });
		}
#ifndef USE_SHADOWMAP_VISUALIZE
		{
			const VkExtent3D Extent = { SurfaceExtent2D.width, SurfaceExtent2D.height, 1 };

			Images.push_back(Image());
			VK::CreateImage(&Images.back().Image, 0, VK_IMAGE_TYPE_2D, DepthFormat, Extent, 1, 1, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);

			AllocateDeviceMemory(&Images.back().DeviceMemory, Images.back().Image, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
			VERIFY_SUCCEEDED(vkBindImageMemory(Device, Images.back().Image, Images.back().DeviceMemory, 0));

			ImageViews.push_back(VkImageView());
			VK::CreateImageView(&ImageViews.back(), Images.back().Image, VK_IMAGE_VIEW_TYPE_2D, DepthFormat, CompMap, { VK_IMAGE_ASPECT_DEPTH_BIT, 0, 1, 0, 1 });
		}
#endif
	}
	virtual void CreateImmutableSampler() override {
		//!< パス1 : イミュータブルサンプラ
		//!< シェーダ内で sampler2DShadow を使用する場合は、比較方法(compareEnable=VK_TRUE, VK_COMPARE_OP_...)を指定すること
		Samplers.resize(1);
		const VkSamplerCreateInfo SCI = {
			VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
			nullptr,
			0,
			VK_FILTER_LINEAR, VK_FILTER_LINEAR, VK_SAMPLER_MIPMAP_MODE_LINEAR,
			VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
			0.0f,
			VK_FALSE, 1.0f,
			VK_TRUE, VK_COMPARE_OP_LESS_OR_EQUAL,
			0.0f, 1.0f,
			VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE,
			VK_FALSE
		};
		VERIFY_SUCCEEDED(vkCreateSampler(Device, &SCI, GetAllocationCallbacks(), &Samplers[0]));
	}
	virtual void CreateUniformBuffer() override {
		{
			{
				const auto Direction = glm::vec3(0.0f, 0.0f, 1.0f);
				const auto Z = glm::normalize(-Direction);
				const auto X = glm::normalize(glm::cross(Z, glm::vec3(0.0f, 1.0f, 0.0f)));
				const auto Y = glm::cross(X, Z);
				//!< シャドウキャスタのAABB
				const auto Center = glm::vec3(0.0f, 0.0f, 0.0f);
				const auto Radius = glm::vec3(1.0f, 1.0f, 1.0f);
				const std::array<glm::vec3, 8> Points = {
					Center + glm::vec3( Radius.x,  Radius.y,  Radius.z),
					Center + glm::vec3( Radius.x,  Radius.y, -Radius.z),
					Center + glm::vec3( Radius.x, -Radius.y,  Radius.z),
					Center + glm::vec3( Radius.x, -Radius.y, -Radius.z),
					Center + glm::vec3(-Radius.x,  Radius.y,  Radius.z),
					Center + glm::vec3(-Radius.x,  Radius.y, -Radius.z),
					Center + glm::vec3(-Radius.x, -Radius.y,  Radius.z),
					Center + glm::vec3(-Radius.x, -Radius.y, -Radius.z),
				};

				auto Mn = (std::numeric_limits<float>::max)();
				auto Mx = (std::numeric_limits<float>::min)();
				for (auto i : Points) {
					const auto t = glm::dot(i, Z);
					Mn = std::min(Mn, t);
					Mx = std::max(Mx, t);
				}
				const auto ZRadius = (Mx - Mn) * 0.5f;

				Mn = (std::numeric_limits<float>::max)();
				Mx = (std::numeric_limits<float>::min)();
				for (auto i : Points) {
					const auto t = glm::dot(i, X);
					Mn = std::min(Mn, t);
					Mx = std::max(Mx, t);
				}
				const auto XRadius = (Mx - Mn) * 0.5f;

				Mn = (std::numeric_limits<float>::max)();
				Mx = (std::numeric_limits<float>::min)();
				for (auto i : Points) {
					const auto t = glm::dot(i, Y);
					Mn = std::min(Mn, t);
					Mx = std::max(Mx, t);
				}
				const auto YRadius = (Mx - Mn) * 0.5f;

				const auto Projection = glm::ortho(-XRadius, XRadius, -YRadius, YRadius, 1.0f, 1.0f + ZRadius * 2.0f);
				const auto View = glm::lookAt(Center - Z * (1.0f + ZRadius), Center, Y);
				Tr.LightProjection = Projection;
				Tr.LightView = View;
#ifdef USE_SHADOWMAP_VISUALIZE
				Tr.Projection = Projection;
				Tr.View = View;
#endif
			}
#ifndef USE_SHADOWMAP_VISUALIZE
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
				Tr.Projection = Projection;
				Tr.View = View;
			}
#endif
			const auto World = glm::mat4(1.0f);
			Tr.World = World;

			UniformBuffers.push_back(UniformBuffer());
			CreateBuffer(&UniformBuffers.back().Buffer, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, sizeof(Tr));
			AllocateDeviceMemory(&UniformBuffers.back().DeviceMemory, UniformBuffers.back().Buffer, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
			VERIFY_SUCCEEDED(vkBindBufferMemory(Device, UniformBuffers.back().Buffer, UniformBuffers.back().DeviceMemory, 0));
		}
	}
	virtual void CreateShaderModules() override {
		const auto ShaderPath = GetBasePath();
		//!< パス0 : シェーダモジュール
		ShaderModules.push_back(VKExt::CreateShaderModules((ShaderPath + TEXT(".vert.spv")).data()));
		ShaderModules.push_back(VKExt::CreateShaderModules((ShaderPath + TEXT(".tese.spv")).data()));
		ShaderModules.push_back(VKExt::CreateShaderModules((ShaderPath + TEXT(".tesc.spv")).data()));
		ShaderModules.push_back(VKExt::CreateShaderModules((ShaderPath + TEXT(".geom.spv")).data()));
		//!< パス1 : シェーダモジュール
#ifdef USE_SHADOWMAP_VISUALIZE
		ShaderModules.push_back(VKExt::CreateShaderModules((ShaderPath + TEXT("_sm_1") + TEXT(".vert.spv")).data()));
		ShaderModules.push_back(VKExt::CreateShaderModules((ShaderPath + TEXT("_sm_1") + TEXT(".frag.spv")).data()));
#else
		ShaderModules.push_back(VKExt::CreateShaderModules((ShaderPath + TEXT("_1") + TEXT(".frag.spv")).data()));
		ShaderModules.push_back(VKExt::CreateShaderModules((ShaderPath + TEXT("_1") + TEXT(".geom.spv")).data()));
#endif
	}
	virtual void CreatePipelines() override {
		Pipelines.resize(2);
		std::vector<std::thread> Threads;
		//!< PRSCI_0 : デプスバイアス有り (With depth bias)
		const VkPipelineRasterizationStateCreateInfo PRSCI_0 = {
			VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
			nullptr,
			0,
			VK_FALSE,
			VK_FALSE,
			VK_POLYGON_MODE_FILL,
			VK_CULL_MODE_BACK_BIT,
			VK_FRONT_FACE_COUNTER_CLOCKWISE,
			//!< シャドウキャスタの描画でデプスバイアスを有効にする
			//!< depthBiasEnable, depthBiasConstantFactor, depthBiasClamp, depthBiasSlopeFactor
			//!< r * depthBiasConstantFactor + m * depthBiasSlopeFactor
			//!< depthBiasClamp : 非0.0fを指定の場合クランプが有効になる(絶対値がdepthBiasClamp以下になるようにクランプされる)
			VK_TRUE, 1.0f, 0.0f, 1.75f,
			1.0f
		};
		//!< PRSCI_1 : デプスバイアス無し (No depth bias)
		const VkPipelineRasterizationStateCreateInfo PRSCI_1 = {
			VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
			nullptr,
			0,
			VK_FALSE,
			VK_FALSE,
			VK_POLYGON_MODE_FILL,
			VK_CULL_MODE_BACK_BIT,
			VK_FRONT_FACE_COUNTER_CLOCKWISE,
			VK_FALSE, 0.0f, 0.0f, 0.0f,
			1.0f
		};
#ifdef _DEBUG
		//!< depthClampEnable にはデバイスフィーチャー depthClamp が有効であること
		VkPhysicalDeviceFeatures PDF;
		vkGetPhysicalDeviceFeatures(GetCurrentPhysicalDevice(), &PDF);
		assert((!PRSCI_0.depthClampEnable || PDF.depthClamp) && "");
#endif
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
#ifdef USE_SHADOWMAP_VISUALIZE
			VK_FALSE, VK_FALSE, VK_COMPARE_OP_LESS_OR_EQUAL,
#else
			VK_TRUE, VK_TRUE, VK_COMPARE_OP_LESS_OR_EQUAL,
#endif
			VK_FALSE,
			VK_FALSE, { VK_STENCIL_OP_KEEP, VK_STENCIL_OP_KEEP, VK_STENCIL_OP_KEEP, VK_COMPARE_OP_NEVER, 0, 0, 0 }, { VK_STENCIL_OP_KEEP, VK_STENCIL_OP_KEEP, VK_STENCIL_OP_KEEP, VK_COMPARE_OP_ALWAYS, 0, 0, 0 },
			0.0f, 1.0f
		};
		const std::array<VkPipelineShaderStageCreateInfo, 4> PSSCIs_0 = {
			VkPipelineShaderStageCreateInfo({ VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, nullptr, 0, VK_SHADER_STAGE_VERTEX_BIT, ShaderModules[0], "main", nullptr }),
			VkPipelineShaderStageCreateInfo({ VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, nullptr, 0, VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT, ShaderModules[1], "main", nullptr }),
			VkPipelineShaderStageCreateInfo({ VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, nullptr, 0, VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT, ShaderModules[2], "main", nullptr }),
			VkPipelineShaderStageCreateInfo({ VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, nullptr, 0, VK_SHADER_STAGE_GEOMETRY_BIT, ShaderModules[3], "main", nullptr }),
		};
#ifdef USE_SHADOWMAP_VISUALIZE
		const std::array<VkPipelineShaderStageCreateInfo, 2> PSSCIs_1 = {
			VkPipelineShaderStageCreateInfo({ VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, nullptr, 0, VK_SHADER_STAGE_VERTEX_BIT, ShaderModules[4], "main", nullptr }),
			VkPipelineShaderStageCreateInfo({ VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, nullptr, 0, VK_SHADER_STAGE_FRAGMENT_BIT, ShaderModules[5], "main", nullptr }),
		};
#else
		const std::array<VkPipelineShaderStageCreateInfo, 5> PSSCIs_1 = {
			VkPipelineShaderStageCreateInfo({ VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, nullptr, 0, VK_SHADER_STAGE_VERTEX_BIT, ShaderModules[0], "main", nullptr }),
			VkPipelineShaderStageCreateInfo({ VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, nullptr, 0, VK_SHADER_STAGE_FRAGMENT_BIT, ShaderModules[4], "main", nullptr }), //!< 
			VkPipelineShaderStageCreateInfo({ VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, nullptr, 0, VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT, ShaderModules[1], "main", nullptr }),
			VkPipelineShaderStageCreateInfo({ VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, nullptr, 0, VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT, ShaderModules[2], "main", nullptr }),
			VkPipelineShaderStageCreateInfo({ VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, nullptr, 0, VK_SHADER_STAGE_GEOMETRY_BIT, ShaderModules[5], "main", nullptr }), //!< 
		};
#endif
		const std::vector<VkVertexInputBindingDescription> VIBDs = {};
		const std::vector<VkVertexInputAttributeDescription> VIADs = {};
		const std::vector<VkPipelineColorBlendAttachmentState> PCBASs_0 = {};
		const std::vector<VkPipelineColorBlendAttachmentState> PCBASs_1 = {
			{
				VK_FALSE, 
				VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ONE, VK_BLEND_OP_ADD,
				VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ONE, VK_BLEND_OP_ADD,
				VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
			},
		};
#ifdef USE_PIPELINE_SERIALIZE
		PipelineCacheSerializer PCS(Device, GetBasePath() + TEXT(".pco"), 2);
		//!< パス0 : パイプライン
		Threads.push_back(std::thread::thread(VK::CreatePipeline, std::ref(Pipelines[0]), Device, PipelineLayouts[0], RenderPasses[0], VK_PRIMITIVE_TOPOLOGY_PATCH_LIST, 1, PRSCI_0, PDSSCI_0, &PSSCIs_0[0], nullptr, &PSSCIs_0[1], &PSSCIs_0[2], &PSSCIs_0[3], VIBDs, VIADs, PCBASs_0, PCS.GetPipelineCache(0)));
		//!< パス1 : パイプライン
#ifdef USE_SHADOWMAP_VISUALIZE
		Threads.push_back(std::thread::thread(VK::CreatePipeline, std::ref(Pipelines[1]), Device, PipelineLayouts[1], RenderPasses[1], VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP, 0, PRSCI_1, PDSSCI_1, &PSSCIs_1[0], &PSSCIs_1[1], nullptr, nullptr, nullptr, VIBDs, VIADs, PCBASs_1, PCS.GetPipelineCache(1)));
#else
		Threads.push_back(std::thread::thread(VK::CreatePipeline, std::ref(Pipelines[1]), Device, PipelineLayouts[1], RenderPasses[1], VK_PRIMITIVE_TOPOLOGY_PATCH_LIST, 1, PRSCI_1, PDSSCI_1, &PSSCIs_1[0], &PSSCIs_1[1], &PSSCIs_1[2], &PSSCIs_1[3], &PSSCIs_1[4], VIBDs, VIADs, PCBASs_1, PCS.GetPipelineCache(0)));
#endif
#else
		Threads.push_back(std::thread::thread(VK::CreatePipeline, std::ref(Pipelines[0]), Device, PipelineLayouts[0], RenderPasses[0], VK_PRIMITIVE_TOPOLOGY_PATCH_LIST, 1, PRSCI_0, PDSSCI_0, &PSSCIs_0[0], nullptr, &PSSCIs_0[1], &PSSCIs_0[2], &PSSCIs_0[3], VIBDs, VIADs, PCBASs_0));
#ifdef USE_SHADOWMAP_VISUALIZE
		Threads.push_back(std::thread::thread(VK::CreatePipeline, std::ref(Pipelines[1]), Device, PipelineLayouts[1], RenderPasses[1], VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP, 0, PRSCI_1, PDSSCI_1, &PSSCIs_1[0], &PSSCIs_1[1], nullptr, nullptr, nullptr, VIBDs, VIADs, PCBASs_1));
#else
		Threads.push_back(std::thread::thread(VK::CreatePipeline, std::ref(Pipelines[1]), Device, PipelineLayouts[1], RenderPasses[1], VK_PRIMITIVE_TOPOLOGY_PATCH_LIST, 1, PRSCI_1, PDSSCI_1, &PSSCIs_1[0], &PSSCIs_1[1], &PSSCIs_1[2], &PSSCIs_1[3], &PSSCIs_1[4], VIBDs, VIADs, PCBASs_1));
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
		glm::mat4 LightProjection;
		glm::mat4 LightView;
	};
	using Transform = struct Transform;
	Transform Tr;

	VkExtent2D ShadowMapExtent = { 2048, 2048 };
};
#pragma endregion