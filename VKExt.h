#pragma once

#include "VK.h"

class VKExt : public VK
{
private:
	using Super = VK;
public:
	using Vertex_Position = struct Vertex_Position { glm::vec3 Position; };
	using Vertex_PositionColor = struct Vertex_PositionColor { glm::vec3 Position; glm::vec4 Color; };
	using Vertex_PositionNormal = struct Vertex_PositionNormal { glm::vec3 Position; glm::vec3 Normal; };
	using Instance_OffsetXY = struct Instance_OffsetXY { glm::vec2 Offset; };

protected:
#ifdef USE_RENDERDOC
	virtual void CreateInstance([[maybe_unused]] const std::vector<const char*>& AdditionalLayers, const std::vector<const char*>& AdditionalExtensions) override {
		//!< #TIPS "VK_LAYER_RENDERDOC_Capture" を使用すると、メッシュシェーダーやレイトレーシングと同時に使用した場合、vkCreateDevice() でコケるようになるので注意 (If we use "VK_LAYER_RENDERDOC_Capture" with mesh shader or raytracing, vkCreateDevice() failed)
		std::vector  ALs(AdditionalLayers);
		ALs.emplace_back("VK_LAYER_RENDERDOC_Capture");
		Super::CreateInstance(ALs, AdditionalExtensions);
	}
	virtual void CreateDevice(HWND hWnd, HINSTANCE hInstance, void* pNext, [[maybe_unused]] const std::vector<const char*>& AdditionalExtensions) override {
		std::vector AEs(AdditionalExtensions);
		//AEs.emplace_back(VK_EXT_DEBUG_MARKER_EXTENSION_NAME);
		Super::CreateDevice(hWnd, hInstance, pNext, AEs);
	}
#endif

	using SizeAndDataPtr = std::pair<const size_t, const void*>;
	struct GeometryCreateInfo {
		std::vector<SizeAndDataPtr> Vtxs = {};
		SizeAndDataPtr Idx = { 0, nullptr };
		uint32_t IdxCount = 0, VtxCount = 0, InstCount = 0;
	};
	void CreateGeometry(const std::vector<GeometryCreateInfo>& GCIs);

	void CreateTexture_Depth(const uint32_t Width, const uint32_t Height) {
		DepthTextures.emplace_back().Create(Device, SelectedPhysDevice.second.PDMP, DepthFormat, VkExtent3D({ .width = Width, .height = Height, .depth = 1 }));
	}
	void CreateTexture_Depth(const VkExtent2D& Ext) {
		CreateTexture_Depth(Ext.width, Ext.height);
	}
	void CreateTexture_Depth() {
		CreateTexture_Depth(Swapchain.Extent);
	}
	void CreateTexture_Render(const uint32_t Width, const uint32_t Height) {
		RenderTextures.emplace_back().Create(Device, SelectedPhysDevice.second.PDMP, SurfaceFormat.format, VkExtent3D({ .width = Width, .height = Height, .depth = 1 }));
	}
	void CreateTexture_Render(const VkExtent2D& Ext) {
		CreateTexture_Render(Ext.width, Ext.height);
	}
	void CreateTexture_Render() {
		CreateTexture_Render(Swapchain.Extent.width, Swapchain.Extent.height);
	}

	void CreateRenderPass_Default(const VkAttachmentLoadOp LoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE, const VkImageLayout FinalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
	void CreateRenderPass_Clear() { CreateRenderPass_Default(VK_ATTACHMENT_LOAD_OP_CLEAR); }
	void CreateRenderPass_Depth(const VkImageLayout FinalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

	//!< レンダーターゲット (COLOR_ATTACHMENT) へ出力
	void CreateRenderPass_RT() { CreateRenderPass_Default(VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL); }
	void CreateRenderPass_Clear_RT() { CreateRenderPass_Default(VK_ATTACHMENT_LOAD_OP_CLEAR, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL); }
	void CreateRenderPass_Depth_RT() { CreateRenderPass_Depth(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL); }

	virtual void CreateRenderPass() override { CreateRenderPass_Default(); }

	//!< 引数のシェーダの順序は D3D12_GRAPHICS_PIPELINE_STATE_DESC 内の VS, PS, DS, HS, GS に合わせて、VS, FS, TES, TCS, GS にしておくことにする
	void CreatePipeline_VsFs_Input(VkPipeline& PL, const VkPipelineLayout PLL, const VkRenderPass RP, const VkPrimitiveTopology PT, const uint32_t PatchControlPoints, const VkPipelineRasterizationStateCreateInfo& PRSCI, const VkBool32 DepthEnable, const std::vector<VkVertexInputBindingDescription>& VIBDs, const std::vector<VkVertexInputAttributeDescription>& VIADs, const std::array<VkPipelineShaderStageCreateInfo, 2>& PSSCIs);
	void CreatePipeline_VsFs(VkPipeline& PL, const VkPipelineLayout PLL, const VkRenderPass RP, const VkPrimitiveTopology PT, const uint32_t PatchControlPoints, const VkPipelineRasterizationStateCreateInfo& PRSCI, const VkBool32 DepthEnable, const std::array<VkPipelineShaderStageCreateInfo, 2>& PSSCIs) { CreatePipeline_VsFs_Input(PL, PLL, RP, PT, PatchControlPoints, PRSCI, DepthEnable, {}, {}, PSSCIs); }
	void CreatePipeline_VsFsTesTcsGs_Input(VkPipeline& PL, const VkPipelineLayout PLL, const VkRenderPass RP, const VkPrimitiveTopology PT, const uint32_t PatchControlPoints, const VkPipelineRasterizationStateCreateInfo& PRSCI, const VkBool32 DepthEnable, const std::vector<VkVertexInputBindingDescription>& VIBDs, const std::vector<VkVertexInputAttributeDescription>& VIADs, const std::array<VkPipelineShaderStageCreateInfo, 5>& PSSCIs);
	void CreatePipeline_VsFsTesTcsGs(VkPipeline& PL, const VkPipelineLayout PLL, const VkRenderPass RP, const VkPrimitiveTopology PT, const uint32_t PatchControlPoints, const VkPipelineRasterizationStateCreateInfo& PRSCI, const VkBool32 DepthEnable, const std::array<VkPipelineShaderStageCreateInfo, 5>& PSSCIs) { CreatePipeline_VsFsTesTcsGs_Input(PL, PLL, RP,  PT, PatchControlPoints, PRSCI, DepthEnable, {}, {}, PSSCIs); }
	void CreatePipelineState_VsFsGs_Input(VkPipeline& PL, const VkPipelineLayout PLL, const VkRenderPass RP, const VkPrimitiveTopology PT, const uint32_t PatchControlPoints, const VkPipelineRasterizationStateCreateInfo& PRSCI, const VkBool32 DepthEnable, const std::vector<VkVertexInputBindingDescription>& VIBDs, const std::vector<VkVertexInputAttributeDescription>& VIADs, const std::array<VkPipelineShaderStageCreateInfo, 3>& PSSCIs);
	void CreatePipelineState_VsFsGs(VkPipeline& PL, const VkPipelineLayout PLL, const VkRenderPass RP, const VkPrimitiveTopology PT, const uint32_t PatchControlPoints, const VkPipelineRasterizationStateCreateInfo& PRSCI, const VkBool32 DepthEnable, const std::array<VkPipelineShaderStageCreateInfo, 3>& PSSCIs) { CreatePipelineState_VsFsGs_Input(PL, PLL, RP, PT, PatchControlPoints, PRSCI, DepthEnable, {}, {}, PSSCIs); }
	//void CreatePipelineState_VsFsTesTcs();

	void CreatePipeline_TsMsFs(VkPipeline& PL, const VkPipelineLayout PLL, const VkRenderPass RP, const VkBool32 DepthEnable, const std::array<VkPipelineShaderStageCreateInfo, 3>& PSSCIs);
	void CreatePipeline_MsFs(VkPipeline& PL, const VkPipelineLayout PLL, const VkRenderPass RP, const VkBool32 DepthEnable, const std::array<VkPipelineShaderStageCreateInfo, 2>& PSSCIs) { CreatePipeline_TsMsFs(PL, PLL, RP, DepthEnable, { VkPipelineShaderStageCreateInfo({.module = VK_NULL_HANDLE }), PSSCIs[0], PSSCIs[1] }); }

	void CreatePipeline(VkPipeline& PL,
		const std::vector<VkPipelineShaderStageCreateInfo>& PSSCIs,
		const VkPipelineVertexInputStateCreateInfo& PVISCI,
		const VkPipelineInputAssemblyStateCreateInfo& PIASCI,
		const VkPipelineTessellationStateCreateInfo& PTSCI,
		const VkPipelineViewportStateCreateInfo& PVSCI,
		const VkPipelineRasterizationStateCreateInfo& PRSCI,
		const VkPipelineMultisampleStateCreateInfo& PMSCI,
		const VkPipelineDepthStencilStateCreateInfo& PDSSCI,
		const VkPipelineColorBlendStateCreateInfo& PCBSCI,
		const VkPipelineDynamicStateCreateInfo& PDSCI,
		const VkPipelineLayout PLL,
		const VkRenderPass RP);
	void CreatePipeline(VkPipeline& PL,
		const VkShaderModule VS, const VkShaderModule FS, const VkShaderModule TCS, const VkShaderModule TES, const VkShaderModule GS,
		const std::vector<VkVertexInputBindingDescription>& VIBDs, const std::vector<VkVertexInputAttributeDescription>& VIADs,
		const VkPrimitiveTopology PT,
		const uint32_t PatchControlPoints,
		const VkPolygonMode PM, const VkCullModeFlags CMF, const VkFrontFace FF,
		const VkBool32 DepthEnable,
		const VkPipelineLayout PLL,
		const VkRenderPass RP);
	void CreatePipeline(VkPipeline& PL,
		const VkShaderModule VS, const VkShaderModule FS,
		const std::vector<VkVertexInputBindingDescription>& VIBDs, const std::vector<VkVertexInputAttributeDescription>& VIADs,
		const VkPipelineLayout PLL,
		const VkRenderPass RP) {
		CreatePipeline(PL,
			VS, FS, VK_NULL_HANDLE, VK_NULL_HANDLE, VK_NULL_HANDLE,
			VIBDs, VIADs,
			VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP,
			0,
			VK_POLYGON_MODE_FILL, VK_CULL_MODE_BACK_BIT, VK_FRONT_FACE_COUNTER_CLOCKWISE,
			VK_FALSE,
			PLL,
			RP);
	}
	void CreatePipeline(VkPipeline& PL,
		const VkShaderModule VS, const VkShaderModule FS,
		const VkPipelineLayout PLL,
		const VkRenderPass RP) {
		CreatePipeline(PL,
			VS, FS,
			{}, {},
			PLL,
			RP);
	}
	void CreatePipeline(VkPipeline& PL,
		const VkShaderModule VS, const VkShaderModule FS, const VkShaderModule TCS, const VkShaderModule TES, const VkShaderModule GS,
		const VkPipelineLayout PLL,
		const VkRenderPass RP) {
		CreatePipeline(PL,
			VS, FS, TCS, TES, GS,
			{}, {},
			VK_PRIMITIVE_TOPOLOGY_PATCH_LIST,
			1,
			VK_POLYGON_MODE_FILL, VK_CULL_MODE_BACK_BIT, VK_FRONT_FACE_COUNTER_CLOCKWISE,
			VK_TRUE,
			PLL,
			RP);
	}

	void CreateFrameBuffer_Default() {
		for (const auto& i : Swapchain.ImageAndViews) {
			VK::CreateFramebuffer(Framebuffers.emplace_back(), RenderPasses[0], Swapchain.Extent, 1, { i.second });
		}
	}
	void CreateFrameBuffer_Depth() {
		for (const auto& i : Swapchain.ImageAndViews) {
			VK::CreateFramebuffer(Framebuffers.emplace_back(), RenderPasses[0], Swapchain.Extent, 1, { i.second, DepthTextures[0].View});
		}
	}

	void CreateImmutableSampler_Default(const VkFilter Filter = VK_FILTER_LINEAR, const VkSamplerMipmapMode MipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR, const VkSamplerAddressMode AddressMode = VK_SAMPLER_ADDRESS_MODE_REPEAT) {
		const VkSamplerCreateInfo SCI = {
			.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
			.pNext = nullptr,
			.flags = 0,
			.magFilter = Filter, .minFilter = Filter, .mipmapMode = MipmapMode,
			.addressModeU = AddressMode, .addressModeV = AddressMode, .addressModeW = AddressMode,
			.mipLodBias = 0.0f,
			.anisotropyEnable = VK_FALSE, .maxAnisotropy = 1.0f,
			.compareEnable = VK_FALSE, .compareOp = VK_COMPARE_OP_NEVER,
			.minLod = 0.0f, .maxLod = 1.0f,
			.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE,
			.unnormalizedCoordinates = VK_FALSE // addressing VK_FALSE:正規化[0.0-1.0], VK_TRUE:画像のディメンション
		};
		VERIFY_SUCCEEDED(vkCreateSampler(Device, &SCI, GetAllocationCallbacks(), &Samplers.emplace_back()));
	}
	//!< LinearRepeat
	void CreateImmutableSampler_LR() { 
		CreateImmutableSampler_Default(); 
	}
	//!< NearestRepeat
	void CreateImmutableSampler_NR() { 
		CreateImmutableSampler_Default(VK_FILTER_NEAREST, VK_SAMPLER_MIPMAP_MODE_NEAREST);
	}
	//!< LinearClamp
	void CreateImmutableSampler_LC() { 
		CreateImmutableSampler_Default(VK_FILTER_LINEAR, VK_SAMPLER_MIPMAP_MODE_LINEAR, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE); 
	}
	//!< NearestClamp
	void CreateImmutableSampler_NC() {
		CreateImmutableSampler_Default(VK_FILTER_NEAREST, VK_SAMPLER_MIPMAP_MODE_NEAREST, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE); 
	}

	void PopulateCommandBuffer_Clear(const size_t i, const VkClearColorValue& Color) {
		const auto CB = CommandBuffers[i];
		constexpr VkCommandBufferBeginInfo CBBI = {
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
			.pNext = nullptr,
			.flags = 0,
			.pInheritanceInfo = nullptr
		};
		VERIFY_SUCCEEDED(vkBeginCommandBuffer(CB, &CBBI)); {
			vkCmdSetViewport(CB, 0, static_cast<uint32_t>(std::size(Viewports)), std::data(Viewports));
			vkCmdSetScissor(CB, 0, static_cast<uint32_t>(std::size(ScissorRects)), std::data(ScissorRects));
			const std::array CVs = { VkClearValue({.color = Color }) };
			const VkRenderPassBeginInfo RPBI = {
				.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
				.pNext = nullptr,
				.renderPass = RenderPasses[0],
				.framebuffer = Framebuffers[i],
				.renderArea = VkRect2D({.offset = VkOffset2D({.x = 0, .y = 0 }), .extent = Swapchain.Extent }),
				.clearValueCount = static_cast<uint32_t>(std::size(CVs)), .pClearValues = std::data(CVs)
			};
			vkCmdBeginRenderPass(CB, &RPBI, VK_SUBPASS_CONTENTS_INLINE); {
			} vkCmdEndRenderPass(CB);
		} VERIFY_SUCCEEDED(vkEndCommandBuffer(CB));
	}
};

class VKExtDepth : public VKExt
{
private:
	using Super = VKExt;
protected:
	virtual void CreateTexture() override {
		CreateTexture_Depth();
	}
	virtual void CreateRenderPass() override { 
		Super::CreateRenderPass_Depth(); 
	}

	//!< [使用時] パイプラインを深度を有効にして作成すること ([On use] Enable depth and create pipeline)
	//virtual void CreatePipeline() override {
	//	CreatePipeline_XXX(..., VK_TRUE, ...);
	//}

	virtual void CreateFramebuffer() override {
		CreateFrameBuffer_Depth();
	}

	//!< [使用時] 深度クリアの設定をすること ([On use] Set depth clear settings)
	//virtual void PopulateCommandBuffer(const size_t i) override {
	//	constexpr std::array CVs = { VkClearValue({.color = Colors::SkyBlue }), VkClearValue({.depthStencil = {.depth = 1.0f, .stencil = 0 } }) };
	//}
};