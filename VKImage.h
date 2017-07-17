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

	static VkFormat ToVkFormat(const gli::format GLIFormat);
	static VkImageViewType ToVkImageViewType(const gli::target GLITarget);
	static VkImageType ToVkImageType(const gli::target GLITarget);
	static VkComponentSwizzle ToVkComponentSwizzle(const gli::swizzle GLISwizzle);
	static void CreateVkComponentMapping(VkComponentMapping& ComponentMapping, const gli::texture::swizzles_type GLISwizzlesType) { 
		ComponentMapping = { ToVkComponentSwizzle(GLISwizzlesType.r), ToVkComponentSwizzle(GLISwizzlesType.g), ToVkComponentSwizzle(GLISwizzlesType.b), ToVkComponentSwizzle(GLISwizzlesType.a) };
	}
	static VkComponentMapping ToVkComponentMapping(const gli::texture::swizzles_type GLISwizzlesType) {
		return { ToVkComponentSwizzle(GLISwizzlesType.r), ToVkComponentSwizzle(GLISwizzlesType.g), ToVkComponentSwizzle(GLISwizzlesType.b), ToVkComponentSwizzle(GLISwizzlesType.a) };
	}

protected:
	virtual void CreateImage(VkImage* Image, const VkImageUsageFlags Usage, const VkSampleCountFlagBits SampleCount, const gli::texture& GLITexture) const;
	virtual void SubmitCopyImage(const VkCommandBuffer CommandBuffer, const VkBuffer SrcBuffer, const VkImage DstImage, const gli::texture& GLITexture);
	virtual void CreateImageView(VkImageView* ImageView, const VkImage Image, const gli::texture& GLITexture);

	virtual void LoadImage(VkImage* Image, VkDeviceMemory *DeviceMemory, VkImageView* ImageView, const std::string& Path) override { 
		LoadImage_DDS(Image, DeviceMemory, ImageView, Path);

#ifdef DEBUG_STDOUT
		std::cout << "\t" << "\t" << "ImageFile = " << Path.c_str() << std::endl;
#endif

#ifdef DEBUG_STDOUT
		std::cout << "\t" << "LoadImage" << COUT_OK << std::endl << std::endl;
#endif
	}

	void LoadImage_DDS(VkImage* Image, VkDeviceMemory *DeviceMemory, VkImageView* ImageView, const std::string& Path);
};

