#pragma once

#include "resource.h"

#pragma region Code
#include "../VKExt.h"

class FullscreenVK : public VKExt
{
private:
	using Super = VKExt;
public:
	FullscreenVK() : Super() {}
	virtual ~FullscreenVK() {}

protected:
#ifdef USE_DRAW_INDIRECT
	virtual void CreateIndirectBuffer() override { CreateIndirectBuffer_Draw(4, 1); }
#endif
	virtual void CreateShaderModules() override {
#ifdef USE_DISTANCE_FUNCTION
		const auto ShaderPath = GetBasePath();
		ShaderModules.push_back(VKExt::CreateShaderModules((ShaderPath + TEXT(".vert.spv")).data()));
		ShaderModules.push_back(VKExt::CreateShaderModules((ShaderPath + TEXT("_df.frag.spv")).data())); 
#else
		CreateShaderModle_VsFs();
#endif
	}
	virtual void CreatePipelines() override { CreatePipeline_VsFs(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP, 0, VK_FALSE); }
	virtual void CreateRenderPass() {
		RenderPasses.resize(1);
		const std::array<VkAttachmentReference, 1> ColorAttach = { { 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL }, };
		VK::CreateRenderPass(RenderPasses[0], {
				//!< アタッチメント
				{
					0,
					ColorFormat,
					VK_SAMPLE_COUNT_1_BIT,
					VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_STORE, //!<「開始時に何もしない(クリアしない)」,「終了時に保存」
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
	virtual void PopulateCommandBuffer(const size_t i) override;
};
#pragma endregion