#include "VKImage.h"

VkFormat VKImage::ToVkFormat(const gli::format GLIFormat)
{
	//gli::is_compressed(GLIFormat);
	//gli::is_s3tc_compressed(GLIFormat);
	//const auto FI = gli::detail::get_format_info(GLIFormat);

	//!< 圧縮テクスチャ
	//!< DXT1	... BC1		bpp4	RGB,RGBA	A2諧調
	//!< DXT2,3	...	BC2		bpp8	RGBA		A16諧調
	//!< DXT4,5	... BC3		bpp8	RGBA
	//!< ATI1N	... BC4		bpp4	R			ハイトマップ等
	//!< ATI2N	... BC5		bpp8	RG			ノーマルマップ等
	//!<			BC6H	bpp8	RGB			HDR
	//!<			BC7		bpp8	RGB,RGBA
#define GLI_FORMAT_TO_VK_FORMAT_ENTRY(glientry, vkentry) case FORMAT_ ## glientry: return VK_FORMAT_ ## vkentry;
	switch (GLIFormat)
	{
		using enum gli::format;
#include "VKGLIFormat.h"
	}
#undef GLI_FORMAT_TO_VK_FORMAT_ENTRY
	Error("GLIFormat is not supported");
	assert(false && "Not supported");
	return VK_FORMAT_UNDEFINED;
}
VkImageViewType VKImage::ToVkImageViewType(const gli::target GLITarget)
{
#define GLI_TARGET_TO_VK_IMAGE_VIEW_TYPE_ENTRY(entry) case TARGET_ ## entry: return VK_IMAGE_VIEW_TYPE_ ## entry
	switch (GLITarget)
	{
		using enum gli::target;
		GLI_TARGET_TO_VK_IMAGE_VIEW_TYPE_ENTRY(1D);
		GLI_TARGET_TO_VK_IMAGE_VIEW_TYPE_ENTRY(1D_ARRAY);
		GLI_TARGET_TO_VK_IMAGE_VIEW_TYPE_ENTRY(2D);
		GLI_TARGET_TO_VK_IMAGE_VIEW_TYPE_ENTRY(2D_ARRAY);
		GLI_TARGET_TO_VK_IMAGE_VIEW_TYPE_ENTRY(3D);
		GLI_TARGET_TO_VK_IMAGE_VIEW_TYPE_ENTRY(CUBE);
		GLI_TARGET_TO_VK_IMAGE_VIEW_TYPE_ENTRY(CUBE_ARRAY);
	}
#undef GLI_TARGET_TO_VK_IMAGE_VIEW_TYPE_ENTRY
	assert(false && "Not supported");
	return VK_IMAGE_VIEW_TYPE_MAX_ENUM;
}
VkImageType VKImage::ToVkImageType(const gli::target GLITarget)
{
	assert(gli::TARGET_INVALID != static_cast<int>(GLITarget) && "TARGET_INVALID");
	switch (GLITarget)
	{
		using enum gli::target;
	case TARGET_1D: 
	case TARGET_1D_ARRAY:
		return VK_IMAGE_TYPE_1D;
	case TARGET_2D: 
	case TARGET_2D_ARRAY: 
	case TARGET_CUBE:
	case TARGET_CUBE_ARRAY:
		return VK_IMAGE_TYPE_2D;
	case TARGET_3D:
		return VK_IMAGE_TYPE_3D;
	}
	assert(false && "Not supported");
	return VK_IMAGE_TYPE_MAX_ENUM;
}
VkComponentSwizzle VKImage::ToVkComponentSwizzle(const gli::swizzle Swizzle)
{
	switch (Swizzle)
	{
	case gli::SWIZZLE_ZERO: return VK_COMPONENT_SWIZZLE_ZERO;
	case gli::SWIZZLE_ONE: return VK_COMPONENT_SWIZZLE_ONE;
	case gli::SWIZZLE_RED: return VK_COMPONENT_SWIZZLE_R;
	case gli::SWIZZLE_GREEN: return VK_COMPONENT_SWIZZLE_G;
	case gli::SWIZZLE_BLUE: return VK_COMPONENT_SWIZZLE_B;
	case gli::SWIZZLE_ALPHA: return VK_COMPONENT_SWIZZLE_A;
	}
	return VK_COMPONENT_SWIZZLE_IDENTITY;
}
