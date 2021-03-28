#include "VKImage.h"

VkFormat VKImage::ToVkFormat(const gli::format GLIFormat)
{
	//gli::is_compressed(GLIFormat);
	//gli::is_s3tc_compressed(GLIFormat);
	//const auto FI = gli::detail::get_format_info(GLIFormat);

	//!< ���k�e�N�X�`��
	//!< DXT1	... BC1		bpp4	RGB,RGBA	A2�~��
	//!< DXT2,3	...	BC2		bpp8	RGBA		A16�~��
	//!< DXT4,5	... BC3		bpp8	RGBA
	//!< ATI1N	... BC4		bpp4	R			�n�C�g�}�b�v��
	//!< ATI2N	... BC5		bpp8	RG			�m�[�}���}�b�v��
	//!<			BC6H	bpp8	RGB			HDR
	//!<			BC7		bpp8	RGB,RGBA
#define GLI_FORMAT_TO_VK_FORMAT_ENTRY(glientry, vkentry) case FORMAT_ ## glientry: return VK_FORMAT_ ## vkentry;
	switch (GLIFormat)
	{
		using enum gli::format;
#include "VKGLIFormat.h"
	}
#undef GLI_FORMAT_TO_VK_FORMAT_ENTRY
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

//!< @param �R�}���h�o�b�t�@
//!< @param �R�s�[���o�b�t�@
//!< @param �R�s�[��C���[�W
//!< @param (�R�s�[���)�C���[�W�̃A�N�Z�X�t���O ex) VK_ACCESS_SHADER_READ_BIT,...��
//!< @param (�R�s�[���)�C���[�W�̃��C�A�E�g ex) VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,...��
//!< @param (�R�s�[���)�C���[�W���g����p�C�v���C���X�e�[�W ex) VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,...��
void VKImage::PopulateCommandBuffer_CopyBufferToImage(const VkCommandBuffer CB, const VkBuffer Src, const VkImage Dst, const VkAccessFlags AF, const VkImageLayout IL, const VkPipelineStageFlags PSF, const gli::texture& GLITexture)
{
	//!< �L���[�u�}�b�v�̏ꍇ�́A�������C���̃C���[�W�Ƃ��č쐬����B(When cubemap, create as layered image)
	//!< �C���[�W�r���[����āA���C�����t�F�C�X�Ƃ��Ĉ����悤�n�[�h�E�G�A�֓`���� (Tell the hardware that it should interpret its layers as faces)
	//!< �L���[�u�}�b�v�̏ꍇ�t�F�C�X�̏����� +X-X+Y-Y+Z-Z (When cubemap, faces order is +X-X+Y-Y+Z-Z)

	const auto Layers = static_cast<const uint32_t>(GLITexture.layers()) * static_cast<const uint32_t>(GLITexture.faces());
	const auto Levels = static_cast<const uint32_t>(GLITexture.levels());
	std::vector<VkBufferImageCopy> BICs; BICs.reserve(Layers * Levels);
	VkDeviceSize Offset = 0;
	for (uint32_t i = 0; i < Layers; ++i) {
		for (uint32_t j = 0; j < Levels; ++j) {
			BICs.emplace_back(VkBufferImageCopy({ 
				.bufferOffset = Offset, .bufferRowLength = 0, .bufferImageHeight = 0, 
				.imageSubresource = VkImageSubresourceLayers({.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .mipLevel = j, .baseArrayLayer = i, .layerCount = 1 }), 
				.imageOffset = VkOffset3D({.x = 0, .y = 0, .z = 0 }), 
				.imageExtent = VkExtent3D({.width = static_cast<const uint32_t>(GLITexture.extent(j).x), .height = static_cast<const uint32_t>(GLITexture.extent(j).y), .depth = static_cast<const uint32_t>(GLITexture.extent(j).z) }) }));
			Offset += static_cast<const VkDeviceSize>(GLITexture.size(j));
		}
	}	
	VK::PopulateCommandBuffer_CopyBufferToImage(CB, Src, Dst, AF, IL, PSF, BICs, Levels, Layers);
}

//!< @param �R�}���h�o�b�t�@
//!< @param �R�s�[���C���[�W
//!< @param �R�s�[��o�b�t�@
//!< @param (�R�s�[���)�C���[�W�̃A�N�Z�X�t���O ex) VK_ACCESS_SHADER_READ_BIT,...��
//!< @param (�R�s�[���)�C���[�W�̃��C�A�E�g ex) VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,...��
//!< @param (�R�s�[���)�C���[�W���g����p�C�v���C���X�e�[�W ex) VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,...��
void VKImage::PopulateCommandBuffer_CopyImageToBuffer(const VkCommandBuffer CB, const VkImage Src, const VkBuffer Dst, const VkAccessFlags AF, const VkImageLayout IL, const VkPipelineStageFlags PSF, const gli::texture& GLITexture)
{
	const auto Layers = static_cast<const uint32_t>(GLITexture.layers()) * static_cast<const uint32_t>(GLITexture.faces());
	const auto Levels = static_cast<const uint32_t>(GLITexture.levels());
	std::vector<VkBufferImageCopy> BICs; BICs.reserve(Layers);
	VkDeviceSize Offset = 0;
	for (uint32_t i = 0; i < Layers; ++i) {
		for (uint32_t j = 0; j < Levels; ++j) {
			BICs.emplace_back(VkBufferImageCopy({ 
				.bufferOffset = Offset, .bufferRowLength = 0, .bufferImageHeight = 0, 
				.imageSubresource = VkImageSubresourceLayers({.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .mipLevel = j, .baseArrayLayer = i, .layerCount = 1 }),
				.imageOffset = VkOffset3D({.x = 0, .y = 0, .z = 0 }), 
				.imageExtent = VkExtent3D({.width = static_cast<const uint32_t>(GLITexture.extent(j).x), .height = static_cast<const uint32_t>(GLITexture.extent(j).y), .depth = static_cast<const uint32_t>(GLITexture.extent(j).z) }) }));
			Offset += static_cast<const VkDeviceSize>(GLITexture.size(j));
		}
	}
	VK::PopulateCommandBuffer_CopyImageToBuffer(CB, Src, Dst, AF, IL, PSF, BICs, Levels, Layers);
}

#if 0
//!< Storage Image ���쐬�����
VkDeviceMemory* DeviceMemory;
VkImage* Image;
VkImageView* ImageView;
Super::CreateImage(Image, 0, VK_IMAGE_TYPE_2D, VK_FORMAT_R8G8B8A8_UNORM, { 1280, 720 }, 1, 1, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_USAGE_STORAGE_BIT);
CreateDeviceMemory(DeviceMemory, *Image, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
BindDeviceMemory(*Image, *DeviceMemory);
Super::CreateImageView(ImageView, *Image, VK_IMAGE_VIEW_TYPE_2D, VK_FORMAT_R8G8B8A8_UNORM, 
	{ VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A }, 
	ImageSubresourceRange_ColorAll);
//!< �C���[�W���C�A�E�g�� VK_IMAGE_LAYOUT_GENERAL �֕ύX���邱�ƁA���̃��C�A�E�g�ł̂ݑ��삪�T�|�[�g����Ă���
#endif