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

	void CreateTexture_Depth(const uint32_t Width, const uint32_t Height) {
		DepthTextures.emplace_back().Create(Device, GetCurrentPhysicalDeviceMemoryProperties(), DepthFormat, VkExtent3D({ .width = Width, .height = Height, .depth = 1 }));
	}
	void CreateTexture_Depth() {
		CreateTexture_Depth(SurfaceExtent2D.width, SurfaceExtent2D.height);
	}
	void CreateTexture_Render(const uint32_t Width, const uint32_t Height) {
		RenderTextures.emplace_back().Create(Device, GetCurrentPhysicalDeviceMemoryProperties(), ColorFormat, VkExtent3D({ .width = Width, .height = Height, .depth = 1 }));
	}
	void CreateTexture_Render() {
		CreateTexture_Render(SurfaceExtent2D.width, SurfaceExtent2D.height);
	}

	void CreateRenderPass_None();
	void CreateRenderPass_Clear();
	void CreateRenderPass_Depth();

	//!< 引数のシェーダの順序は D3D12_GRAPHICS_PIPELINE_STATE_DESC 内の VS, PS, DS, HS, GS に合わせて、VS, FS, TES, TCS, GS にしておくことにする
	void CreatePipeline_VsFs_Input(const VkPrimitiveTopology PT, const uint32_t PatchControlPoints, const VkPipelineRasterizationStateCreateInfo& PRSCI, const VkBool32 DepthEnable, const std::vector<VkVertexInputBindingDescription>& VIBDs, const std::vector<VkVertexInputAttributeDescription>& VIADs, const std::array<VkPipelineShaderStageCreateInfo, 2>& PSSCIs);
	void CreatePipeline_VsFs(const VkPrimitiveTopology PT, const uint32_t PatchControlPoints, const VkPipelineRasterizationStateCreateInfo& PRSCI, const VkBool32 DepthEnable, const std::array<VkPipelineShaderStageCreateInfo, 2>& PSSCIs) { CreatePipeline_VsFs_Input(PT, PatchControlPoints, PRSCI, DepthEnable, {}, {}, PSSCIs); }
	void CreatePipeline_VsFsTesTcsGs_Input(const VkPrimitiveTopology PT, const uint32_t PatchControlPoints, const VkPipelineRasterizationStateCreateInfo& PRSCI, const VkBool32 DepthEnable, const std::vector<VkVertexInputBindingDescription>& VIBDs, const std::vector<VkVertexInputAttributeDescription>& VIADs, const std::array<VkPipelineShaderStageCreateInfo, 5>& PSSCIs);
	void CreatePipeline_VsFsTesTcsGs(const VkPrimitiveTopology PT, const uint32_t PatchControlPoints, const VkPipelineRasterizationStateCreateInfo& PRSCI, const VkBool32 DepthEnable, const std::array<VkPipelineShaderStageCreateInfo, 5>& PSSCIs) { CreatePipeline_VsFsTesTcsGs_Input(PT, PatchControlPoints, PRSCI, DepthEnable, {}, {}, PSSCIs); }
	void CreatePipelineState_VsFsGs_Input(const VkPrimitiveTopology PT, const uint32_t PatchControlPoints, const VkPipelineRasterizationStateCreateInfo& PRSCI, const VkBool32 DepthEnable, const std::vector<VkVertexInputBindingDescription>& VIBDs, const std::vector<VkVertexInputAttributeDescription>& VIADs, const std::array<VkPipelineShaderStageCreateInfo, 3>& PSSCIs);
	void CreatePipelineState_VsFsGs(const VkPrimitiveTopology PT, const uint32_t PatchControlPoints, const VkPipelineRasterizationStateCreateInfo& PRSCI, const VkBool32 DepthEnable, const std::array<VkPipelineShaderStageCreateInfo, 3>& PSSCIs) { CreatePipelineState_VsFsGs_Input(PT, PatchControlPoints, PRSCI, DepthEnable, {}, {}, PSSCIs); }
	//void CreatePipelineState_VsFsTesTcs();

	void CreatePipeline_MsFs(const VkBool32 DepthEnable, const std::array<VkPipelineShaderStageCreateInfo, 2>& PSSCIs) { CreatePipeline_TsMsFs(DepthEnable, { VkPipelineShaderStageCreateInfo({.module = VK_NULL_HANDLE }), PSSCIs[0], PSSCIs[1] }); }
	void CreatePipeline_TsMsFs(const VkBool32 DepthEnable, const std::array<VkPipelineShaderStageCreateInfo, 3>& PSSCIs);

	void CreateFrameBuffer_Default() {
		for (auto i : SwapchainImageViews) {
			VK::CreateFramebuffer(Framebuffers.emplace_back(), RenderPasses[0], SurfaceExtent2D.width, SurfaceExtent2D.height, 1, { i });
		}
	}
	void CreateFrameBuffer_Depth() {
		for (auto i : SwapchainImageViews) {
			VK::CreateFramebuffer(Framebuffers.emplace_back(), RenderPasses[0], SurfaceExtent2D.width, SurfaceExtent2D.height, 1, { i, DepthTextures[0].View});
		}
	}

	void CreateImmutableSampler_LinearRepeat() {
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
			.unnormalizedCoordinates = VK_FALSE // addressing VK_FALSE:正規化[0.0-1.0], VK_TRUE:画像のディメンション
		};
		VERIFY_SUCCEEDED(vkCreateSampler(Device, &SCI, GetAllocationCallbacks(), &Samplers.emplace_back()));
	}
	void CreateImmutableSampler_NearestRepeat() {
		const VkSamplerCreateInfo SCI = {
			.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
			.pNext = nullptr,
			.flags = 0,
			.magFilter = VK_FILTER_NEAREST, .minFilter = VK_FILTER_NEAREST, .mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST,
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
	void CreateImmutableSampler_LinearClamp() {
		const VkSamplerCreateInfo SCI = {
			.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
			.pNext = nullptr,
			.flags = 0,
			.magFilter = VK_FILTER_LINEAR, .minFilter = VK_FILTER_LINEAR, .mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
			.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, .addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, .addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
			.mipLodBias = 0.0f,
			.anisotropyEnable = VK_FALSE, .maxAnisotropy = 1.0f,
			.compareEnable = VK_FALSE, .compareOp = VK_COMPARE_OP_NEVER,
			.minLod = 0.0f, .maxLod = 1.0f,
			.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE,
			.unnormalizedCoordinates = VK_FALSE
		};
		VERIFY_SUCCEEDED(vkCreateSampler(Device, &SCI, GetAllocationCallbacks(), &Samplers.emplace_back()));
	}
	void CreateImmutableSampler_NearestClamp() {
		const VkSamplerCreateInfo SCI = {
			.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
			.pNext = nullptr,
			.flags = 0,
			.magFilter = VK_FILTER_NEAREST, .minFilter = VK_FILTER_NEAREST, .mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST,
			.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, .addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, .addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
			.mipLodBias = 0.0f,
			.anisotropyEnable = VK_FALSE, .maxAnisotropy = 1.0f,
			.compareEnable = VK_FALSE, .compareOp = VK_COMPARE_OP_NEVER,
			.minLod = 0.0f, .maxLod = 1.0f,
			.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE,
			.unnormalizedCoordinates = VK_FALSE
		};
		VERIFY_SUCCEEDED(vkCreateSampler(Device, &SCI, GetAllocationCallbacks(), &Samplers.emplace_back()));
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
			vkCmdSetViewport(CB, 0, static_cast<uint32_t>(size(Viewports)), data(Viewports));
			vkCmdSetScissor(CB, 0, static_cast<uint32_t>(size(ScissorRects)), data(ScissorRects));
			const std::array CVs = { VkClearValue({.color = Color }) };
			const VkRenderPassBeginInfo RPBI = {
				.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
				.pNext = nullptr,
				.renderPass = RenderPasses[0],
				.framebuffer = Framebuffers[i],
				.renderArea = VkRect2D({.offset = VkOffset2D({.x = 0, .y = 0 }), .extent = SurfaceExtent2D }),
				.clearValueCount = static_cast<uint32_t>(size(CVs)), .pClearValues = data(CVs)
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
	//!< パイプラインを深度を有効にして作成すること
	//virtual void CreatePipeline() override {
	//	CreatePipeline_XXX(..., VK_TRUE, ...);
	//}
	virtual void CreateFramebuffer() override {
		CreateFrameBuffer_Depth();
	}
	//!< 深度クリアの設定をすること
	//virtual void PopulateCommandBuffer(const size_t i) override {
	//	constexpr std::array CVs = { VkClearValue({.color = Colors::SkyBlue }), VkClearValue({.depthStencil = {.depth = 1.0f, .stencil = 0 } }) };
	//}
};