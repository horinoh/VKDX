#pragma once

#include "VKExt.h"

#pragma warning(push)
#pragma warning(disable : 4100)
//#pragma warning(disable : 4201)
#pragma warning(disable : 4244)
#pragma warning(disable : 4458)
#pragma warning(disable : 5054)
#include <gli/gli.hpp>
#pragma warning(pop)


class VKImage : public VKExt
{
private:
	using Super = VKExt;

	static [[nodiscard]] VkFormat ToVkFormat(const gli::format GLIFormat);
	static [[nodiscard]] VkImageViewType ToVkImageViewType(const gli::target GLITarget);
	static [[nodiscard]] VkImageType ToVkImageType(const gli::target GLITarget);
	static [[nodiscard]] VkComponentSwizzle ToVkComponentSwizzle(const gli::swizzle GLISwizzle);
	static [[nodiscard]] VkComponentMapping ToVkComponentMapping(const gli::texture::swizzles_type ST) { return { ToVkComponentSwizzle(ST.r), ToVkComponentSwizzle(ST.g), ToVkComponentSwizzle(ST.b), ToVkComponentSwizzle(ST.a) }; }

protected:
	virtual void CreateImage(VkImage* Image, const VkSampleCountFlagBits SampleCount, const VkImageUsageFlags Usage, const gli::texture& GLITexture) const;
	virtual void PopulateCommandBuffer_CopyBufferToImage(const VkCommandBuffer CB, const VkBuffer Src, const VkImage Dst, const VkAccessFlags AF, const VkImageLayout IL, const VkPipelineStageFlags PSF, const gli::texture& GLITexture);
	virtual void PopulateCommandBuffer_CopyImageToBuffer(const VkCommandBuffer CB, const VkImage Src, const VkBuffer Dst, const VkAccessFlags AF, const VkImageLayout IL,	const VkPipelineStageFlags PSF, const gli::texture& GLITexture);
	virtual void CreateImageView(VkImageView* ImageView, const VkImage Image, const gli::texture& GLITexture);

	virtual [[nodiscard]] gli::texture LoadImage(VkImage* Img, VkDeviceMemory* DM, const VkPipelineStageFlags PSF, std::string_view Path) {
		assert(std::filesystem::exists(Path) && "");
		assert(Path.ends_with(".dds") && "");
		return LoadImage_DDS(Img, DM, PSF, Path);
	}
	[[nodiscard]] gli::texture LoadImage_DDS(VkImage* Image, VkDeviceMemory* DM, const VkPipelineStageFlags PSF, std::string_view Path);
};

