#include "stdafx.h"

#include "VKImage.h"

VkFormat VKImage::ToVkFormat(const gli::format GLIFormat)
{
#define GLI_FORMAT_TO_VK_FORMAT_ENTRY(glientry, vkentry) case gli::format::FORMAT_ ## glientry: return VK_FORMAT_ ## vkentry;
	switch (GLIFormat)
	{
	default: assert(false && "Not supported"); break;
#include "VKGLIFormat.h"
	}
#undef GLI_FORMAT_TO_VK_FORMAT_ENTRY
	return VK_FORMAT_UNDEFINED;
}
VkImageViewType VKImage::ToVkImageViewType(const gli::target GLITarget)
{
#define GLI_TARGET_TO_VK_IMAGE_VIEW_TYPE_ENTRY(entry) case gli::target::TARGET_ ## entry: return VK_IMAGE_VIEW_TYPE_ ## entry
	switch (GLITarget)
	{
	default: assert(false && "Not supported"); break;
		GLI_TARGET_TO_VK_IMAGE_VIEW_TYPE_ENTRY(1D);
		GLI_TARGET_TO_VK_IMAGE_VIEW_TYPE_ENTRY(1D_ARRAY);
		GLI_TARGET_TO_VK_IMAGE_VIEW_TYPE_ENTRY(2D);
		GLI_TARGET_TO_VK_IMAGE_VIEW_TYPE_ENTRY(2D_ARRAY);
		GLI_TARGET_TO_VK_IMAGE_VIEW_TYPE_ENTRY(3D);
		GLI_TARGET_TO_VK_IMAGE_VIEW_TYPE_ENTRY(CUBE);
		GLI_TARGET_TO_VK_IMAGE_VIEW_TYPE_ENTRY(CUBE_ARRAY);
	}
#undef GLI_TARGET_TO_VK_IMAGE_VIEW_TYPE_ENTRY

	return VK_IMAGE_VIEW_TYPE_MAX_ENUM;
}

VkImageType VKImage::ToVkImageType(const gli::target GLITarget)
{
	switch (GLITarget)
	{
	default: assert(false && "Not supported"); break;
	case gli::target::TARGET_1D: 
	case gli::target::TARGET_1D_ARRAY:
		return VK_IMAGE_TYPE_1D;
	case gli::target::TARGET_2D: 
	case gli::target::TARGET_2D_ARRAY: 
	case gli::target::TARGET_CUBE: 
	case gli::target::TARGET_CUBE_ARRAY: 
		return VK_IMAGE_TYPE_2D;
	case gli::target::TARGET_3D: 
		return VK_IMAGE_TYPE_3D;
	}
	return VK_IMAGE_TYPE_MAX_ENUM;
}
VkComponentSwizzle VKImage::ToVkComponentSwizzle(const gli::swizzle GLISwizzle)
{
	switch (GLISwizzle)
	{
	case gli::SWIZZLE_ZERO: //!< ?
	case gli::SWIZZLE_ONE: //!< ?
	default: assert(false && "Not supported"); break;
	case gli::SWIZZLE_RED: return VK_COMPONENT_SWIZZLE_R;
	case gli::SWIZZLE_GREEN: return VK_COMPONENT_SWIZZLE_G;
	case gli::SWIZZLE_BLUE: return VK_COMPONENT_SWIZZLE_B;
	case gli::SWIZZLE_ALPHA: return VK_COMPONENT_SWIZZLE_A;
	}
	return VK_COMPONENT_SWIZZLE_IDENTITY;
}

void VKImage::CreateImage(VkImage* Image, const VkImageUsageFlags Usage, const gli::texture& GLITexture) const
{
	const auto GLIExtent3D = GLITexture.extent(0);
	const VkExtent3D Extent3D = {
		static_cast<const uint32_t>(GLIExtent3D.x), static_cast<const uint32_t>(GLIExtent3D.y), static_cast<const uint32_t>(GLIExtent3D.z)
	};
	Super::CreateImage(Image, Usage, ToVkImageType(GLITexture.target()), ToVkFormat(GLITexture.format()), Extent3D, static_cast<const uint32_t>(GLITexture.levels()), static_cast<const uint32_t>(GLITexture.layers()));
}

void VKImage::SubmitCopyImage(const VkCommandBuffer CommandBuffer, const VkBuffer SrcBuffer, const VkImage DstImage, const gli::texture& GLITexture)
{
	const auto ArrayLayers = static_cast<const uint32_t>(GLITexture.layers());
	//const auto Faces = static_cast<const uint32_t>(GLITexture.faces());
	const auto MipLevels = static_cast<const uint32_t>(GLITexture.levels());

	VERIFY_SUCCEEDED(vkBeginCommandBuffer(CommandBuffer, &CommandBufferBeginInfo_OneTime)); {
		//!< 各レイヤー、ミップレベルの、バッファコピー領域を準備
		std::vector<VkBufferImageCopy> BufferImageCopies;
		BufferImageCopies.reserve(ArrayLayers);
		VkDeviceSize Offset = 0;
		const VkOffset3D Offset3D = {
			0, 0, 0
		};
		for (uint32_t i = 0; i < ArrayLayers; ++i) {
			//for (uint32_t j = 0; j < Faces; ++j) {}
			for (uint32_t k = 0; k < MipLevels; ++k) {
				const VkImageSubresourceLayers ImageSubresourceLayers = {
					VK_IMAGE_ASPECT_COLOR_BIT,
					k,
					i, 1
				};
				const auto GLIExtent3D = GLITexture.extent(k);
				const VkExtent3D Extent3D = {
					static_cast<const uint32_t>(GLIExtent3D.x), static_cast<const uint32_t>(GLIExtent3D.y), static_cast<const uint32_t>(GLIExtent3D.z)
				};
				const VkBufferImageCopy BufferImageCopy = {
					Offset,
					0,
					0,
					ImageSubresourceLayers,
					Offset3D,
					Extent3D
				};
				BufferImageCopies.push_back(BufferImageCopy);

				Offset += static_cast<const VkDeviceSize>(GLITexture.size(k));
			}
		}

		const VkImageSubresourceRange ImageSubresourceRange = {
			VK_IMAGE_ASPECT_COLOR_BIT,
			0, MipLevels,
			0, ArrayLayers
		};
		const VkImageMemoryBarrier ImageMemoryBarrier_UndefinedToTransferDst = {
			VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
			nullptr,
			0,
			VK_ACCESS_TRANSFER_WRITE_BIT,
			VK_IMAGE_LAYOUT_UNDEFINED,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			VK_QUEUE_FAMILY_IGNORED,
			VK_QUEUE_FAMILY_IGNORED,
			DstImage,
			ImageSubresourceRange
		};
		vkCmdPipelineBarrier(CommandBuffer,
			VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
			VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
			0,
			0, nullptr,
			0, nullptr,
			1, &ImageMemoryBarrier_UndefinedToTransferDst);
		{
			vkCmdCopyBufferToImage(CommandBuffer, SrcBuffer, DstImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, static_cast<uint32_t>(BufferImageCopies.size()), BufferImageCopies.data());
		}
		const VkImageMemoryBarrier ImageMemoryBarrier_TransferDstToShaderReadOnly = {
			VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
			nullptr,
			VK_ACCESS_TRANSFER_WRITE_BIT,
			VK_ACCESS_SHADER_READ_BIT,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			VK_QUEUE_FAMILY_IGNORED,
			VK_QUEUE_FAMILY_IGNORED,
			DstImage,
			ImageSubresourceRange
		};
		vkCmdPipelineBarrier(CommandBuffer,
			VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
			VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
			0,
			0, nullptr,
			0, nullptr,
			1, &ImageMemoryBarrier_TransferDstToShaderReadOnly);

	} VERIFY_SUCCEEDED(vkEndCommandBuffer(CommandBuffer));

	//!< サブミット
	const VkSubmitInfo SubmitInfo = {
		VK_STRUCTURE_TYPE_SUBMIT_INFO,
		nullptr,
		0, nullptr,
		nullptr,
		1, &CommandBuffer,
		0, nullptr
	};
	VERIFY_SUCCEEDED(vkQueueSubmit(GraphicsQueue, 1, &SubmitInfo, VK_NULL_HANDLE));
	VERIFY_SUCCEEDED(vkDeviceWaitIdle(Device)); //!< フェンスでも良い
}

void VKImage::LoadImage_DDS(VkImage* Image, VkDeviceMemory *DeviceMemory, VkImageView* ImageView, const std::string& Path)
{
	//const auto GLITexture(gli::load(Path.c_str())); //!< DDS or KTX or KMG
	const auto GLITexture(gli::load_dds(Path.c_str()));
	assert(!GLITexture.empty() && "Load image failed");

	//const auto ArrayLayers = static_cast<const uint32_t>(GLITexture.layers());

	//VkFormatProperties FormatProperties;
	//vkGetPhysicalDeviceFormatProperties(PhysicalDevice, Format, &FormatProperties);
	//assert(VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT & FormatProperties.optimalTilingFeatures && "");

	VkBuffer StagingBuffer = VK_NULL_HANDLE;
	VkDeviceMemory StagingDeviceMemory = VK_NULL_HANDLE;
	{
		//!< ステージング用のバッファとメモリを作成、データをメモリへコピー、バインド
		CreateStagingBufferAndCopyToMemory(&StagingBuffer, &StagingDeviceMemory, GLITexture.size(), GLITexture.data());

		//!< イメージを作成
		CreateImage(Image, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT, GLITexture);
		//!< デバイスローカル用のメモリを作成
		CreateDeviceLocalMemory(DeviceMemory, *Image);
		//!< バインド
		BindDeviceMemory(*Image, *DeviceMemory/*, 0*/);

		//!< ステージングからデバイスローカルへのコピーコマンドを発行
		const VkCommandBuffer CommandBuffer = CommandBuffers[0];
		SubmitCopyImage(CommandBuffer, StagingBuffer, *Image, GLITexture);
	}
	if (VK_NULL_HANDLE != StagingDeviceMemory) {
		vkFreeMemory(Device, StagingDeviceMemory, GetAllocationCallbacks());
	}
	if (VK_NULL_HANDLE != StagingBuffer) {
		vkDestroyBuffer(Device, StagingBuffer, GetAllocationCallbacks());
	}

	//!< ビューを作成
	CreateImageView(ImageView, *Image, ToVkImageViewType(GLITexture.target()), ToVkFormat(GLITexture.format()), ToVkComponentMapping(GLITexture.swizzles()), ImageSubresourceRange_Color);

	//!< サンプラを作成
	CreateSampler(static_cast<const float>(GLITexture.levels()));
}