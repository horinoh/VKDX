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

#ifdef USE_MANUAL_CLEAR
	//!< �蓮�ŃN���A����ꍇ�� VK_IMAGE_USAGE_TRANSFER_DST_BIT �t���O���ǉ��ŕK�v
	virtual void CreateSwapchain() override { VK::CreateSwapchain(GetCurrentPhysicalDevice(), Surface, GetClientRectWidth(), GetClientRectHeight(), VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT); }
	//!< �����_�[�p�X�ł̃N���A�͂��Ȃ�
	virtual void CreateRenderPass() { VK::CreateRenderPass(VK_ATTACHMENT_LOAD_OP_DONT_CARE, false); }
#endif
};
#pragma endregion