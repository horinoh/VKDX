#pragma once

#include "resource.h"

#pragma region Code
#include "../VKExt.h"

class ClearVK : public VKExt
{
private:
	using Super = VKExt;
public:
	ClearVK() : Super() {}
	virtual ~ClearVK() {}
//#ifdef USE_RENDER_PASS_CLEAR
//	virtual void CreateRenderPass() { RenderPasses.resize(1); CreateRenderPass_Default(RenderPasses[0], ColorFormat, true); }
//#else
//	virtual void CreateRenderPass() { RenderPasses.resize(1); CreateRenderPass_Default(RenderPasses[0], ColorFormat, false); }
//#endif
	virtual void CreateRenderPass() {
		RenderPasses.resize(1);
		const std::array<VkAttachmentReference, 1> ColorAttach = { { 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL }, };
		VK::CreateRenderPass(RenderPasses[0], {
				//!< �A�^�b�`�����g
				{
					0,
					ColorFormat,
					VK_SAMPLE_COUNT_1_BIT,
#ifdef USE_RENDER_PASS_CLEAR
					VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE, //!<�u�J�n���ɃN���A�v,�u�I�����ɕۑ��v
#else
					VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_STORE, //!<�u�J�n���ɉ������Ȃ�(�N���A���Ȃ�)�v,�u�I�����ɕۑ��v
#endif
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
};
#pragma endregion