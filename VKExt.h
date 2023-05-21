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

	void CreateRenderPass_Clear();
	void CreateRenderPass_Depth();

	//!< 引数のシェーダの順序は D3D12_GRAPHICS_PIPELINE_STATE_DESC 内の VS, PS, DS, HS, GS に合わせて、VS, FS, TES, TCS, GS にしておくことにする
	void CreatePipeline_VsFs(const VkPrimitiveTopology PT, const uint32_t PatchControlPoints, const VkPipelineRasterizationStateCreateInfo& PRSCI, const VkBool32 DepthEnable, const std::array<VkPipelineShaderStageCreateInfo, 2>& PSSCIs) { CreatePipeline_VsFs_Input(PT, PatchControlPoints, PRSCI, DepthEnable, {}, {}, PSSCIs); }
	void CreatePipeline_VsFs_Input(const VkPrimitiveTopology PT, const uint32_t PatchControlPoints, const VkPipelineRasterizationStateCreateInfo& PRSCI, const VkBool32 DepthEnable, const std::vector<VkVertexInputBindingDescription>& VIBDs, const std::vector<VkVertexInputAttributeDescription>& VIADs, const std::array<VkPipelineShaderStageCreateInfo, 2>& PSSCIs);
	void CreatePipeline_VsFsTesTcsGs(const VkPrimitiveTopology PT, const uint32_t PatchControlPoints, const VkPipelineRasterizationStateCreateInfo& PRSCI, const VkBool32 DepthEnable, const std::array<VkPipelineShaderStageCreateInfo, 5>& PSSCIs) { CreatePipeline_VsFsTesTcsGs_Input(PT, PatchControlPoints, PRSCI, DepthEnable, {}, {}, PSSCIs); }
	void CreatePipeline_VsFsTesTcsGs_Input(const VkPrimitiveTopology PT, const uint32_t PatchControlPoints, const VkPipelineRasterizationStateCreateInfo& PRSCI, const VkBool32 DepthEnable, const std::vector<VkVertexInputBindingDescription>& VIBDs, const std::vector<VkVertexInputAttributeDescription>& VIADs, const std::array<VkPipelineShaderStageCreateInfo, 5>& PSSCIs);
	//void CreatePipelineState_VsGsFs();
	//void CreatePipelineState_VsFsTesTcs();

	void CreatePipeline_MsFs(const VkBool32 DepthEnable, const std::array<VkPipelineShaderStageCreateInfo, 2>& PSSCIs) { CreatePipeline_TsMsFs(DepthEnable, { VkPipelineShaderStageCreateInfo({.module = VK_NULL_HANDLE }), PSSCIs[0], PSSCIs[1] }); }
	void CreatePipeline_TsMsFs(const VkBool32 DepthEnable, const std::array<VkPipelineShaderStageCreateInfo, 3>& PSSCIs);
};

class VKExtDepth : public VKExt
{
private:
	using Super = VKExt;
protected:
	virtual void CreateTexture() override {
		DepthTextures.emplace_back().Create(Device, GetCurrentPhysicalDeviceMemoryProperties(), DepthFormat, VkExtent3D({ .width = SurfaceExtent2D.width, .height = SurfaceExtent2D.height, .depth = 1 }));
	}
	virtual void CreateRenderPass() override { 
		Super::CreateRenderPass_Depth(); 
	}
	//!< パイプラインを深度を有効にして作成すること
	//virtual void CreatePipeline() override {
	//	CreatePipeline_XXX(VK_TRUE, ...);
	//}
	virtual void CreateFramebuffer() override {
		for (auto i : SwapchainImageViews) {
			VK::CreateFramebuffer(Framebuffers.emplace_back(), RenderPasses[0], SurfaceExtent2D.width, SurfaceExtent2D.height, 1, { i, DepthTextures.back().View });
		}
	}
	//!< 深度クリアの設定をすること
	//virtual void PopulateCommandBuffer(const size_t i) override {
	//	constexpr std::array CVs = { VkClearValue({.color = Colors::SkyBlue }), VkClearValue({.depthStencil = {.depth = 1.0f, .stencil = 0 } }) };
	//}
};