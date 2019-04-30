#pragma once

#include "VKExt.h"

//!< single_channel_bitmap_data_snorm() ‚Å uint_t ‚ª–³‚¢‚Æ“{‚ç‚ê‚é‚Ì‚Å’è‹`‚µ‚Ä‚¨‚­
using uint_t = uint8_t;
#include <gli/gli.hpp>
//#include <gli/convert.hpp>

class VKImage : public VKExt
{
private:
	using Super = VKExt;

	static VkFormat ToVkFormat(const gli::format GLIFormat);
	static VkImageViewType ToVkImageViewType(const gli::target GLITarget);
	static VkImageType ToVkImageType(const gli::target GLITarget);
	static bool IsCube(const gli::target GLITarget);
	static VkComponentSwizzle ToVkComponentSwizzle(const gli::swizzle GLISwizzle);
	static VkComponentMapping ToVkComponentMapping(const gli::texture::swizzles_type GLISwizzlesType);

protected:
	virtual void CreateImage(VkImage* Image, const VkSampleCountFlagBits SampleCount, const VkImageUsageFlags Usage, const gli::texture& GLITexture) const;
	virtual void SubmitCopyImage(const VkCommandBuffer CommandBuffer, const VkBuffer SrcBuffer, const VkImage DstImage, const gli::texture& GLITexture);
	virtual void CreateImageView(VkImageView* ImageView, const VkImage Image, const gli::texture& GLITexture);

	virtual void LoadImage(VkImage* Image, VkDeviceMemory *DeviceMemory, VkImageView* ImageView, const std::string& Path) override;
	void LoadImage_DDS(VkImage* Image, VkDeviceMemory *DeviceMemory, VkImageView* ImageView, const std::string& Path);
};

