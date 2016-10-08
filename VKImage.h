#pragma once

//!< fopen でなく fopen_s を使えと怒られるが、gli のコードは書き換えたくないので warning を抑制する
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

	static VkFormat ToVKFormat(const gli::format GLIFormat);
	static VkImageViewType ToVKImageViewType(const gli::target GLITarget);
	static VkImageType ToVKImageType(const gli::target GLITarget);

protected:
	virtual void LoadImage(VkImage* Image, VkDeviceMemory *DeviceMemory, VkImageView* ImageView, const std::string& Path) override { LoadImage_DDS(Image, DeviceMemory, ImageView, Path); }

	void LoadImage_DDS(VkImage* Image, VkDeviceMemory *DeviceMemory, VkImageView* ImageView, const std::string& Path);
};

