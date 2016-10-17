#pragma once

//!< fopen �łȂ� fopen_s ���g���Ɠ{���邪�Agli �̃R�[�h�͏������������Ȃ��̂� warning ��}������
#pragma warning (push)
#pragma warning (disable : 4996)
#include <gli/gli.hpp>
//#include <gli/convert.hpp>
#pragma warning (pop)

#include "VKExt.h"

class VKImage : public VKExt
{
private:
	using Super = VKExt;

	static VkFormat ToVkFormat(const gli::format GLIFormat);
	static VkImageViewType ToVkImageViewType(const gli::target GLITarget);
	static VkImageType ToVkImageType(const gli::target GLITarget);
	static VkComponentSwizzle ToVkComponentSwizzle(const gli::swizzle GLISwizzle);

protected:
	virtual void LoadImage(VkImage* Image, VkDeviceMemory *DeviceMemory, VkImageView* ImageView, const std::string& Path) override { LoadImage_DDS(Image, DeviceMemory, ImageView, Path); }

	void LoadImage_DDS(VkImage* Image, VkDeviceMemory *DeviceMemory, VkImageView* ImageView, const std::string& Path);
};

