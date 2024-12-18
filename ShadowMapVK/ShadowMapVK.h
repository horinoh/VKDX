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
	virtual void OnUpdate(const uint32_t i) override {
		Tr.World = glm::rotate(glm::mat4(1.0f), glm::radians(Degree), glm::vec3(0.0f, 1.0f, 0.0f));
		Degree += 1.0f;

#pragma region FRAME_OBJECT
		CopyToHostVisibleDeviceMemory(Device, UniformBuffers[i].DeviceMemory, 0, sizeof(Tr), &Tr);
#pragma endregion
	}
	virtual void CreateCommandBuffer() override {
		Super::CreateCommandBuffer();

#pragma region PASS1
		//!< パス1 : セカンダリコマンドバッファ
		const auto SCCount = static_cast<uint32_t>(std::size(Swapchain.ImageAndViews));
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
		const auto& PDMP = SelectedPhysDevice.second.PDMP;
		const auto CB = CommandBuffers[0];

#pragma region PASS0
		//!< インダイレクトバッファ(シャドウキャスタ描画用 : トーラス)
		constexpr VkDrawIndexedIndirectCommand DIIC = { .indexCount = 1, .instanceCount = 1, .firstIndex = 0, .vertexOffset = 0, .firstInstance = 0 };
		IndirectBuffers.emplace_back().Create(Device, PDMP, DIIC).SubmitCopyCommand(Device, PDMP, CB, GraphicsQueue, sizeof(DIIC), &DIIC);
		VK::Scoped<StagingBuffer> Staging_Indirect0(Device);
		Staging_Indirect0.Create(Device, PDMP, sizeof(DIIC), &DIIC);
#pragma endregion

#pragma region PASS1
#ifdef USE_SHADOWMAP_VISUALIZE
		//!< インダイレクトバッファ(シャドウマップ描画用 : フルスクリーン)
		constexpr VkDrawIndirectCommand DIC = { .vertexCount = 4, .instanceCount = 1, .firstVertex = 0, .firstInstance = 0 };
		IndirectBuffers.emplace_back().Create(Device, PDMP, DIC).SubmitCopyCommand(Device, PDMP, CB, GraphicsQueue, sizeof(DIC), &DIC);
		VK::Scoped<StagingBuffer> Staging_Indirect1(Device);
		Staging_Indirect1.Create(Device, PDMP, sizeof(DIC), &DIC);
#else
		//!< インダイレクトバッファ(シャドウレシーバ描画用 : トーラス、平面)
		constexpr VkDrawIndexedIndirectCommand DIIC1 = { .indexCount = 1, .instanceCount = 2, .firstIndex = 0, .vertexOffset = 0, .firstInstance = 0 };
		IndirectBuffers.emplace_back().Create(Device, PDMP, DIIC).SubmitCopyCommand(Device, PDMP, CB, GraphicsQueue, sizeof(DIIC1), &DIIC1);
		VK::Scoped<StagingBuffer> Staging_Indirect1(Device);
		Staging_Indirect1.Create(Device, PDMP, sizeof(DIIC), &DIIC);
#endif	
#pragma endregion

		constexpr VkCommandBufferBeginInfo CBBI = { .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO, .pNext = nullptr, .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, .pInheritanceInfo = nullptr };
		VERIFY_SUCCEEDED(vkBeginCommandBuffer(CB, &CBBI)); {
			IndirectBuffers[0].PopulateCopyCommand(CB, sizeof(DIIC), Staging_Indirect0.Buffer);
			IndirectBuffers[1].PopulateCopyCommand(CB, sizeof(DIC), Staging_Indirect1.Buffer);
		} VERIFY_SUCCEEDED(vkEndCommandBuffer(CB));
		VK::SubmitAndWait(GraphicsQueue, CB);
	}
	virtual void CreateUniformBuffer() override {
		{
			const auto Direction = glm::vec3(0.0f, 0.0f, 1.0f);
			const auto Z = glm::normalize(-Direction);
			const auto X = glm::normalize(glm::cross(Z, glm::vec3(0.0f, 1.0f, 0.0f)));
			const auto Y = glm::cross(X, Z);
			//!< シャドウキャスタのAABB
			const auto Center = glm::vec3(0.0f, 0.0f, 0.0f);
			const auto Radius = glm::vec3(1.0f, 1.0f, 1.0f);
			const std::array Points = {
				Center + glm::vec3(Radius.x,  Radius.y,  Radius.z),
				Center + glm::vec3(Radius.x,  Radius.y, -Radius.z),
				Center + glm::vec3(Radius.x, -Radius.y,  Radius.z),
				Center + glm::vec3(Radius.x, -Radius.y, -Radius.z),
				Center + glm::vec3(-Radius.x,  Radius.y,  Radius.z),
				Center + glm::vec3(-Radius.x,  Radius.y, -Radius.z),
				Center + glm::vec3(-Radius.x, -Radius.y,  Radius.z),
				Center + glm::vec3(-Radius.x, -Radius.y, -Radius.z),
			};

			auto Mn = (std::numeric_limits<float>::max)();
			auto Mx = (std::numeric_limits<float>::min)();
			for (auto i : Points) {
				const auto t = glm::dot(i, Z);
				Mn = (std::min)(Mn, t);
				Mx = (std::max)(Mx, t);
			}
			const auto ZRadius = (Mx - Mn) * 0.5f;

			Mn = (std::numeric_limits<float>::max)();
			Mx = (std::numeric_limits<float>::min)();
			for (auto i : Points) {
				const auto t = glm::dot(i, X);
				Mn = (std::min)(Mn, t);
				Mx = (std::max)(Mx, t);
			}
			const auto XRadius = (Mx - Mn) * 0.5f;

			Mn = (std::numeric_limits<float>::max)();
			Mx = (std::numeric_limits<float>::min)();
			for (auto i : Points) {
				const auto t = glm::dot(i, Y);
				Mn = (std::min)(Mn, t);
				Mx = (std::max)(Mx, t);
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
			constexpr auto Fov = 0.16f * std::numbers::pi_v<float>;
			const auto Aspect = GetAspectRatioOfClientRect();
			constexpr auto ZFar = 4.0f;
			constexpr auto ZNear = 2.0f;
			constexpr auto CamPos = glm::vec3(0.0f, 0.0f, 3.0f);
			constexpr auto CamTag = glm::vec3(0.0f);
			constexpr auto CamUp = glm::vec3(0.0f, 1.0f, 0.0f);
			constexpr auto Projection = glm::perspective(Fov, Aspect, ZNear, ZFar);
			const auto View = glm::lookAt(CamPos, CamTag, CamUp);
			Tr.Projection = Projection;
			Tr.View = View;
		}
#endif
		const auto World = glm::mat4(1.0f);
		Tr.World = World;

#pragma region FRAME_OBJECT
		for ([[maybe_unused]] const auto& i : Swapchain.ImageAndViews) {
			UniformBuffers.emplace_back().Create(Device, SelectedPhysDevice.second.PDMP, sizeof(Tr));
		}
#pragma endregion
	}
	virtual void CreateTexture() override {
		//!< パス0
		CreateTexture_Depth(ShadowMapExtent.width, ShadowMapExtent.height);
	}
	virtual void CreateImmutableSampler() override {
		//!< パス1 : イミュータブルサンプラ
		//!< シェーダ内で sampler2DShadow を使用する場合は、比較方法(compareEnable=VK_TRUE, VK_COMPARE_OP_...)を指定すること
		CreateImmutableSampler_LR();
	}
	virtual void CreatePipelineLayout() override {
		//!< パス0 : パイプラインレイアウト
		{
			CreateDescriptorSetLayout(DescriptorSetLayouts.emplace_back(), 0, {
				VkDescriptorSetLayoutBinding({.binding = 0, .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, .descriptorCount = 1, .stageFlags = VK_SHADER_STAGE_GEOMETRY_BIT, .pImmutableSamplers = nullptr }),
			});
			VKExt::CreatePipelineLayout(PipelineLayouts.emplace_back(), { DescriptorSetLayouts.back() }, {});
		}

		//!< パス1 : パイプラインレイアウト
		{
			const std::array ISs = { Samplers[0] };
			CreateDescriptorSetLayout(DescriptorSetLayouts.emplace_back(), 0, {
				//!< レンダーターゲット : 深度(RenderTarget : Depth)
				VkDescriptorSetLayoutBinding({.binding = 0, .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, .descriptorCount = static_cast<uint32_t>(size(ISs)), .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT, .pImmutableSamplers = data(ISs) }),
#ifndef USE_SHADOWMAP_VISUALIZE
				VkDescriptorSetLayoutBinding({.binding = 1, .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, .descriptorCount = 1, .stageFlags = VK_SHADER_STAGE_GEOMETRY_BIT, .pImmutableSamplers = nullptr }),
#endif
			});
			VKExt::CreatePipelineLayout(PipelineLayouts.emplace_back(), { DescriptorSetLayouts.back() }, {});
		}
	}
	virtual void CreateRenderPass() override {
		//!< パス0 : レンダーパス
		{
			constexpr std::array<VkAttachmentReference, 0> ColorAttach = {};
			constexpr VkAttachmentReference DepthAttach = { .attachment = static_cast<uint32_t>(size(ColorAttach)), .layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL };
			VK::CreateRenderPass(RenderPasses.emplace_back(), {
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
						.colorAttachmentCount = static_cast<uint32_t>(size(ColorAttach)), .pColorAttachments = data(ColorAttach), .pResolveAttachments = nullptr,
						.pDepthStencilAttachment = &DepthAttach,
						.preserveAttachmentCount = 0, .pPreserveAttachments = nullptr
					}),
				}, {
					//!< サブパス依存
				});
		}
		//!< パス1 : レンダーパス
		{
			constexpr std::array ColorAttach = { VkAttachmentReference({ .attachment = 0, .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL }), };
			VK::CreateRenderPass(RenderPasses.emplace_back(), {
					VkAttachmentDescription({
						.flags= 0,
						.format = SurfaceFormat.format,
						.samples = VK_SAMPLE_COUNT_1_BIT,
#ifdef USE_SHADOWMAP_VISUALIZE
						.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE, .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
#else
						.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR, .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
#endif
						.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE, .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
						.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED, .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
					}),
#ifndef USE_SHADOWMAP_VISUALIZE
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
						.flags= 0,
						.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
						.inputAttachmentCount = 0, .pInputAttachments = nullptr,
						.colorAttachmentCount = static_cast<uint32_t>(size(ColorAttach)), .pColorAttachments = data(ColorAttach), .pResolveAttachments = nullptr,
						.pDepthStencilAttachment = nullptr,
						.preserveAttachmentCount = 0, .pPreserveAttachments = nullptr
					}),
				}, {});
		}
	}
	virtual void CreatePipeline() override {
		const std::array SMs = {
			//!< Pass0 : シェーダモジュール
			VK::CreateShaderModule(GetFilePath(".vert.spv")),
			VK::CreateShaderModule(GetFilePath(".tese.spv")),
			VK::CreateShaderModule(GetFilePath(".tesc.spv")),
			VK::CreateShaderModule(GetFilePath(".geom.spv")),
			//!< Pass1 : シェーダモジュール
	#ifdef USE_SHADOWMAP_VISUALIZE
			VK::CreateShaderModule(GetFilePath("_sm_1.vert.spv")),
			VK::CreateShaderModule(GetFilePath("_sm_1.frag.spv")),
	#else
			VK::CreateShaderModule(GetFilePath("_1.frag.spv")),
			VK::CreateShaderModule(GetFilePath("_1.geom.spv")),
	#endif
		};

		Pipelines.resize(2);
		//!< PRSCI_0 : デプスバイアス有り (With depth bias)
		const VkPipelineRasterizationStateCreateInfo PRSCI_0 = {
			.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
			.pNext = nullptr,
			.flags = 0,
			.depthClampEnable = VK_FALSE,
			.rasterizerDiscardEnable = VK_FALSE,
			.polygonMode = VK_POLYGON_MODE_FILL,
			.cullMode = VK_CULL_MODE_BACK_BIT,
			.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
			//!< シャドウキャスタの描画でデプスバイアスを有効にする
			//!< depthBiasEnable, depthBiasConstantFactor, depthBiasClamp, depthBiasSlopeFactor
			//!< r * depthBiasConstantFactor + m * depthBiasSlopeFactor
			//!< depthBiasClamp : 非0.0fを指定の場合クランプが有効になる(絶対値がdepthBiasClamp以下になるようにクランプされる)
			.depthBiasEnable = VK_TRUE, .depthBiasConstantFactor = 1.0f, .depthBiasClamp = 0.0f, .depthBiasSlopeFactor = 1.75f,
			.lineWidth = 1.0f
		};
		//!< PRSCI_1 : デプスバイアス無し (No depth bias)
		const VkPipelineRasterizationStateCreateInfo PRSCI_1 = {
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
#ifdef _DEBUG
		//!< depthClampEnable にはデバイスフィーチャー depthClamp が有効であること
		VkPhysicalDeviceFeatures PDF;
		vkGetPhysicalDeviceFeatures(SelectedPhysDevice.first, &PDF);
		assert((!PRSCI_0.depthClampEnable || PDF.depthClamp) && "");
#endif
		const VkStencilOpState SOS_Front = { VK_STENCIL_OP_KEEP, VK_STENCIL_OP_KEEP, VK_STENCIL_OP_KEEP, VK_COMPARE_OP_NEVER, 0, 0, 0 };
		const VkStencilOpState SOS_Back = { VK_STENCIL_OP_KEEP, VK_STENCIL_OP_KEEP, VK_STENCIL_OP_KEEP, VK_COMPARE_OP_ALWAYS, 0, 0, 0 };
		const VkPipelineDepthStencilStateCreateInfo PDSSCI_0 = {
			.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
			.pNext = nullptr,
			.flags = 0,
			.depthTestEnable = VK_TRUE, .depthWriteEnable = VK_TRUE, .depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL,
			.depthBoundsTestEnable = VK_FALSE,
			.stencilTestEnable = VK_FALSE, .front = SOS_Front, .back = SOS_Back,
			.minDepthBounds = 0.0f, .maxDepthBounds = 1.0f
		};
		const VkPipelineDepthStencilStateCreateInfo PDSSCI_1 = {
			.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
			.pNext = nullptr,
			.flags = 0,
#ifdef USE_SHADOWMAP_VISUALIZE
			.depthTestEnable = VK_FALSE, .depthWriteEnable = VK_FALSE, .depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL,
#else
			.depthTestEnable = VK_TRUE, .depthWriteEnable = VK_TRUE, .depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL,
#endif
			.depthBoundsTestEnable = VK_FALSE,
			.stencilTestEnable = VK_FALSE, .front = SOS_Front, .back = SOS_Back,
			.minDepthBounds = 0.0f, .maxDepthBounds = 1.0f
		};
		const std::array PSSCIs_0 = {
			VkPipelineShaderStageCreateInfo({.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, .pNext = nullptr, .flags = 0, .stage = VK_SHADER_STAGE_VERTEX_BIT, .module = SMs[0], .pName = "main", .pSpecializationInfo = nullptr }),
			VkPipelineShaderStageCreateInfo({.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, .pNext = nullptr, .flags = 0, .stage = VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT, .module = SMs[1], .pName = "main", .pSpecializationInfo = nullptr }),
			VkPipelineShaderStageCreateInfo({.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, .pNext = nullptr, .flags = 0, .stage = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT, .module = SMs[2], .pName = "main", .pSpecializationInfo = nullptr }),
			VkPipelineShaderStageCreateInfo({.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, .pNext = nullptr, .flags = 0, .stage = VK_SHADER_STAGE_GEOMETRY_BIT, .module = SMs[3], .pName = "main", .pSpecializationInfo = nullptr }),
		};
#ifdef USE_SHADOWMAP_VISUALIZE
		const std::array PSSCIs_1 = {
			VkPipelineShaderStageCreateInfo({.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, .pNext = nullptr, .flags = 0, .stage = VK_SHADER_STAGE_VERTEX_BIT, .module = SMs[4], .pName = "main", .pSpecializationInfo = nullptr }),
			VkPipelineShaderStageCreateInfo({.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, .pNext = nullptr, .flags = 0, .stage = VK_SHADER_STAGE_FRAGMENT_BIT, .module = SMs[5], .pName = "main", .pSpecializationInfo = nullptr }),
		};
#else
		const std::array PSSCIs_1 = {
			VkPipelineShaderStageCreateInfo({.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, .pNext = nullptr, .flags = 0, .stage = VK_SHADER_STAGE_VERTEX_BIT, .module = SMs[0], .pName = "main", .pSpecializationInfo = nullptr }),
			VkPipelineShaderStageCreateInfo({.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, .pNext = nullptr, .flags = 0, .stage = VK_SHADER_STAGE_FRAGMENT_BIT, .module = SMs[4], .pName = "main", .pSpecializationInfo = nullptr }), //!< 
			VkPipelineShaderStageCreateInfo({.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, .pNext = nullptr, .flags = 0, .stage = VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT, .module = SMs[1], .pName = "main", .pSpecializationInfo = nullptr }),
			VkPipelineShaderStageCreateInfo({.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, .pNext = nullptr, .flags = 0, .stage = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT, .module = SMs[2], .pName = "main", .pSpecializationInfo = nullptr }),
			VkPipelineShaderStageCreateInfo({.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, .pNext = nullptr, .flags = 0, .stage = VK_SHADER_STAGE_GEOMETRY_BIT, .module = SMs[5], .pName = "main", .pSpecializationInfo = nullptr }), //!< 
		};
#endif
		const std::vector<VkVertexInputBindingDescription> VIBDs = {};
		const std::vector<VkVertexInputAttributeDescription> VIADs = {};
		const std::vector<VkPipelineColorBlendAttachmentState> PCBASs_0 = {};
		const std::vector PCBASs_1 = {
			VkPipelineColorBlendAttachmentState({
				.blendEnable = VK_FALSE,
				.srcColorBlendFactor = VK_BLEND_FACTOR_ONE, .dstColorBlendFactor = VK_BLEND_FACTOR_ONE, .colorBlendOp = VK_BLEND_OP_ADD,
				.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE, .dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE, .alphaBlendOp = VK_BLEND_OP_ADD,
				.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
			}),
		};
#ifdef USE_PIPELINE_SERIALIZE
		PipelineCacheSerializer PCS(Device, GetFilePath(".pco"), 2);
		//!< パス0 : パイプライン
		Threads.emplace_back(std::thread::thread(VK::CreatePipelineVsFsTesTcsGs, std::ref(Pipelines[0]), Device, PipelineLayouts[0], RenderPasses[0], VK_PRIMITIVE_TOPOLOGY_PATCH_LIST, 1, PRSCI_0, PDSSCI_0, &PSSCIs_0[0], nullptr, &PSSCIs_0[1], &PSSCIs_0[2], &PSSCIs_0[3], VIBDs, VIADs, PCBASs_0, PCS.GetPipelineCache(0)));
		//!< パス1 : パイプライン
#ifdef USE_SHADOWMAP_VISUALIZE
		Threads.emplace_back(std::thread::thread(VK::CreatePipelineVsFsTesTcsGs, std::ref(Pipelines[1]), Device, PipelineLayouts[1], RenderPasses[1], VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP, 0, PRSCI_1, PDSSCI_1, &PSSCIs_1[0], &PSSCIs_1[1], nullptr, nullptr, nullptr, VIBDs, VIADs, PCBASs_1, PCS.GetPipelineCache(1)));
#else
		Threads.emplace_back(std::thread::thread(VK::CreatePipelineVsFsTesTcsGs, std::ref(Pipelines[1]), Device, PipelineLayouts[1], RenderPasses[1], VK_PRIMITIVE_TOPOLOGY_PATCH_LIST, 1, PRSCI_1, PDSSCI_1, &PSSCIs_1[0], &PSSCIs_1[1], &PSSCIs_1[2], &PSSCIs_1[3], &PSSCIs_1[4], VIBDs, VIADs, PCBASs_1, PCS.GetPipelineCache(0)));
#endif
#else
		Threads.emplace_back(std::thread::thread(VK::CreatePipelineVsFsTesTcsGs, std::ref(Pipelines[0]), Device, PipelineLayouts[0], RenderPasses[0], VK_PRIMITIVE_TOPOLOGY_PATCH_LIST, 1, PRSCI_0, PDSSCI_0, &PSSCIs_0[0], nullptr, &PSSCIs_0[1], &PSSCIs_0[2], &PSSCIs_0[3], VIBDs, VIADs, PCBASs_0, VK_NULL_HANDLE));
#ifdef USE_SHADOWMAP_VISUALIZE
		Threads.emplace_back(std::thread::thread(VK::CreatePipelineVsFsTesTcsGs, std::ref(Pipelines[1]), Device, PipelineLayouts[1], RenderPasses[1], VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP, 0, PRSCI_1, PDSSCI_1, &PSSCIs_1[0], &PSSCIs_1[1], nullptr, nullptr, nullptr, VIBDs, VIADs, PCBASs_1, VK_NULL_HANDLE));
#else
		Threads.emplace_back(std::thread::thread(VK::CreatePipelineVsFsTesTcsGs, std::ref(Pipelines[1]), Device, PipelineLayouts[1], RenderPasses[1], VK_PRIMITIVE_TOPOLOGY_PATCH_LIST, 1, PRSCI_1, PDSSCI_1, &PSSCIs_1[0], &PSSCIs_1[1], &PSSCIs_1[2], &PSSCIs_1[3], &PSSCIs_1[4], VIBDs, VIADs, PCBASs_1, VK_NULL_HANDLE));
#endif
#endif
		for (auto& i : Threads) { i.join(); }
	}
	virtual void CreateFramebuffer() override {
		//!< パス0 : フレームバッファ
		{
			VK::CreateFramebuffer(Framebuffers.emplace_back(), RenderPasses[0], ShadowMapExtent.width, ShadowMapExtent.height, 1, { DepthTextures[0].View });
		}

		//!< パス1 : フレームバッファ
		{
			for (const auto& i : Swapchain.ImageAndViews) {
				VK::CreateFramebuffer(Framebuffers.emplace_back(), RenderPasses[1], Swapchain.Extent, 1, { 
					i.second,
#ifndef USE_SHADOWMAP_VISUALIZE
					DepthTextures[0].View
#endif
				});
			}
		}
	}
	virtual void CreateDescriptor() override {
		//!< Pass0, Pass1 : デスクリプタプール
		VK::CreateDescriptorPool(DescriptorPools.emplace_back(), 0, {
#pragma region FRAME_OBJECT
			VkDescriptorPoolSize({.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, .descriptorCount = static_cast<uint32_t>(std::size(Swapchain.ImageAndViews)) * 2 }), //!< UB * N * 2
#pragma endregion
			VkDescriptorPoolSize({.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, .descriptorCount = 1 }),
		});

		//!< パス0 : デスクリプタセット
		{
			const std::array DSLs = { DescriptorSetLayouts[0] };
			const VkDescriptorSetAllocateInfo DSAI = {
				.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
				.pNext = nullptr,
				.descriptorPool = DescriptorPools[0],
				.descriptorSetCount = static_cast<uint32_t>(size(DSLs)), .pSetLayouts = data(DSLs)
			};
#pragma region FRAME_OBJECT
			for ([[maybe_unused]] const auto& i : Swapchain.ImageAndViews) {
				VERIFY_SUCCEEDED(vkAllocateDescriptorSets(Device, &DSAI, &DescriptorSets.emplace_back()));
			}
#pragma endregion
		}
		//!< パス1 : デスクリプタセット
		{
			const std::array DSLs = { DescriptorSetLayouts[1] };
			const VkDescriptorSetAllocateInfo DSAI = {
				.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
				.pNext = nullptr,
				.descriptorPool = DescriptorPools[0],
				.descriptorSetCount = static_cast<uint32_t>(size(DSLs)), .pSetLayouts = data(DSLs)
			};
#pragma region FRAME_OBJECT
			for ([[maybe_unused]] const auto& i : Swapchain.ImageAndViews) {
				VERIFY_SUCCEEDED(vkAllocateDescriptorSets(Device, &DSAI, &DescriptorSets.emplace_back()));
			}
#pragma endregion
		}

#pragma region FRAME_OBJECT
		const auto SCCount = std::size(Swapchain.ImageAndViews);
		//!< パス0 :
		VkDescriptorUpdateTemplate DUT0;
		VK::CreateDescriptorUpdateTemplate(DUT0, VK_PIPELINE_BIND_POINT_GRAPHICS, {
			VkDescriptorUpdateTemplateEntry({
				.dstBinding = 0, .dstArrayElement = 0,
				.descriptorCount = _countof(DescriptorUpdateInfo_0::DBI), .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
				.offset = offsetof(DescriptorUpdateInfo_0, DBI), .stride = sizeof(DescriptorUpdateInfo_0)
			}),
		}, DescriptorSetLayouts[0]);
		for (size_t i = 0; i < SCCount; ++i) {
			const DescriptorUpdateInfo_0 DUI = {
				VkDescriptorBufferInfo({ .buffer = UniformBuffers[i].Buffer, .offset = 0, .range = VK_WHOLE_SIZE }),
			};
			vkUpdateDescriptorSetWithTemplate(Device, DescriptorSets[i], DUT0, &DUI);
			vkDestroyDescriptorUpdateTemplate(Device, DUT0, GetAllocationCallbacks());
		}

		//!< パス1 :
		VkDescriptorUpdateTemplate DUT1;
		VK::CreateDescriptorUpdateTemplate(DUT1, VK_PIPELINE_BIND_POINT_GRAPHICS, {
			VkDescriptorUpdateTemplateEntry({
				.dstBinding = 0, .dstArrayElement = 0,
				.descriptorCount = _countof(DescriptorUpdateInfo_1::DII), .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
				.offset = offsetof(DescriptorUpdateInfo_1, DII), .stride = sizeof(DescriptorUpdateInfo_1)
			}),
#ifndef USE_SHADOWMAP_VISUALIZE
			VkDescriptorUpdateTemplateEntry({
				.dstBinding = 1, .dstArrayElement = 0,
				.descriptorCount = std::size(DescriptorUpdateInfo_1::DBI), .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
				.offset = offsetof(DescriptorUpdateInfo_1, DBI), .stride = sizeof(DescriptorUpdateInfo_1)
			}),
#endif
		}, DescriptorSetLayouts[1]);
		for (size_t i = 0; i < SCCount; ++i) {
			const DescriptorUpdateInfo_1 DUI = {
				VkDescriptorImageInfo({ .sampler = VK_NULL_HANDLE, .imageView = DepthTextures[0].View, .imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL }),
				VkDescriptorBufferInfo({ .buffer = UniformBuffers[i].Buffer, .offset = 0, .range = VK_WHOLE_SIZE }),
			};
			vkUpdateDescriptorSetWithTemplate(Device, DescriptorSets[i + SCCount], DUT1, &DUI);
			vkDestroyDescriptorUpdateTemplate(Device, DUT1, GetAllocationCallbacks());
		}
#pragma endregion
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