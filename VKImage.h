#pragma once

#include "VKExt.h"

#pragma warning(push)
#pragma warning(disable : 4100)
#pragma warning(disable : 4201)
#pragma warning(disable : 4458)
#pragma warning(disable : 5054) //!< std:c++latest
#include <gli/gli.hpp>
#pragma warning(pop)


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
	virtual void CopyBufferToImage(const VkCommandBuffer CB, const VkBuffer Src, const VkImage Dst, const VkAccessFlags AF, const VkImageLayout IL, const VkPipelineStageFlags PSF, const gli::texture& GLITexture);
	virtual void CopyImageToBuffer(const VkCommandBuffer CB, const VkImage Src, const VkBuffer Dst, const VkAccessFlags AF, const VkImageLayout IL,	const VkPipelineStageFlags PSF, const gli::texture& GLITexture);
	virtual void CreateImageView(VkImageView* ImageView, const VkImage Image, const gli::texture& GLITexture);

	virtual gli::texture LoadImage(VkImage* Img, VkDeviceMemory* DM, const VkPipelineStageFlags PSF, const std::string& Path) { return LoadImage_DDS(Img, DM, PSF, Path); }
	gli::texture LoadImage_DDS(VkImage* Image, VkDeviceMemory* DM, const VkPipelineStageFlags PSF, const std::string& Path);
};

