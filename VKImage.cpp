#include "stdafx.h"

#include "VKImage.h"

VkFormat VKImage::ToVkFormat(const gli::format GLIFormat)
{
	switch (GLIFormat)
	{
	default: assert(false && "Not supported"); break;
	case gli::format::FORMAT_BGRA8_UNORM_PACK8: return VK_FORMAT_R8G8B8A8_UNORM;
		//!< #TODO ...
	}
	return VK_FORMAT_UNDEFINED;
}
VkImageViewType VKImage::ToVkImageViewType(const gli::target GLITarget)
{
	switch (GLITarget)
	{
	default: assert(false && "Not supported"); break;
	case gli::target::TARGET_1D: return VK_IMAGE_VIEW_TYPE_1D;
	case gli::target::TARGET_1D_ARRAY: return VK_IMAGE_VIEW_TYPE_1D_ARRAY;
	case gli::target::TARGET_2D: return VK_IMAGE_VIEW_TYPE_2D;
	case gli::target::TARGET_2D_ARRAY: return VK_IMAGE_VIEW_TYPE_2D_ARRAY;
	case gli::target::TARGET_3D: return VK_IMAGE_VIEW_TYPE_3D;
	case gli::target::TARGET_CUBE: return VK_IMAGE_VIEW_TYPE_CUBE;
	case gli::target::TARGET_CUBE_ARRAY: return VK_IMAGE_VIEW_TYPE_CUBE_ARRAY;
	}
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

void VKImage::LoadImage_DDS(VkImage* Image, VkDeviceMemory *DeviceMemory, VkImageView* ImageView, const std::string& Path)
{
	//const auto GLITexture(gli::load(Path.c_str())); //!< DDS or KTX or KMG
	const auto GLITexture(gli::load_dds(Path.c_str()));
	assert(!GLITexture.empty() && "Load image failed");

	const auto Format = ToVkFormat(GLITexture.format());
	const auto Levels = GLITexture.levels();
	const auto Layers = GLITexture.layers();

	//VkFormatProperties FormatProperties;
	//vkGetPhysicalDeviceFormatProperties(PhysicalDevice, Format, &FormatProperties);
	//assert(VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT & FormatProperties.optimalTilingFeatures && "");

	VkBuffer StagingBuffer = VK_NULL_HANDLE;
	VkDeviceMemory StagingDeviceMemory = VK_NULL_HANDLE;
	{
		//!< ステージング用のバッファとメモリを作成
		CreateStagingBufferAndCopyToMemory(&StagingBuffer, &StagingDeviceMemory, GLITexture.size(), GLITexture.data());

		//!< デバイスローカル用のイメージを作成
		const auto GLIExtent3D = GLITexture.extent(0);
		const VkExtent3D Extent3D = {
			static_cast<const uint32_t>(GLIExtent3D.x), static_cast<const uint32_t>(GLIExtent3D.y), static_cast<const uint32_t>(GLIExtent3D.z)
		};
		const VkImageCreateInfo ImageCreateInfo = {
			VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
			nullptr,
			0,
			ToVkImageType(GLITexture.target()),
			Format,
			Extent3D,
			static_cast<const uint32_t>(Levels),
			static_cast<const uint32_t>(Layers),
			VK_SAMPLE_COUNT_1_BIT,
			VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
			VK_SHARING_MODE_EXCLUSIVE,
			0, nullptr,
			VK_IMAGE_LAYOUT_UNDEFINED
		};
		VERIFY_SUCCEEDED(vkCreateImage(Device, &ImageCreateInfo, nullptr, Image));
		//!< デバイスローカル用のメモリを作成
		CreateDeviceLocalMemory(DeviceMemory, *Image);
		BindDeviceMemory(*Image, *DeviceMemory, 0);

		//!< 転送
		{
			const VkCommandBuffer CommandBuffer = CommandBuffers[0];

			VERIFY_SUCCEEDED(vkBeginCommandBuffer(CommandBuffer, &CommandBufferBeginInfo_OneTime)); {
				std::vector<VkBufferImageCopy> BufferImageCopies;
				BufferImageCopies.reserve(Layers);
				VkDeviceSize Offset = 0;
				for (uint32_t i = 0; i < Levels; ++i) {
					const VkImageSubresourceLayers ImageSubresourceLayers = {
						VK_IMAGE_ASPECT_COLOR_BIT,
						i,
						0,
						static_cast<uint32_t>(Layers)
					};
					const VkOffset3D Offset3D = {
						0, 0, 0
					};
					const auto GLIExtent3D = GLITexture.extent(i);
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

					Offset += static_cast<const VkDeviceSize>(GLITexture.size(i));
				}

				const VkImageSubresourceRange ImageSubresourceRange = {
					VK_IMAGE_ASPECT_COLOR_BIT,
					0,
					static_cast<const uint32_t>(Levels),
					0,
					static_cast<const uint32_t>(Layers)
				};
				{
					const VkImageMemoryBarrier ImageMemoryBarrier = {
						VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
						nullptr,
						0,
						VK_ACCESS_TRANSFER_WRITE_BIT,
						VK_IMAGE_LAYOUT_UNDEFINED,
						VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
						VK_QUEUE_FAMILY_IGNORED,
						VK_QUEUE_FAMILY_IGNORED,
						*Image,
						ImageSubresourceRange
					};
					vkCmdPipelineBarrier(CommandBuffer,
						VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 
						VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
						0,
						0, nullptr,
						0, nullptr,
						1, &ImageMemoryBarrier);
				}
				vkCmdCopyBufferToImage(CommandBuffer, StagingBuffer, *Image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, static_cast<uint32_t>(BufferImageCopies.size()), BufferImageCopies.data());
				{
					const VkImageMemoryBarrier ImageMemoryBarrier = {
						VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
						nullptr,
						VK_ACCESS_TRANSFER_WRITE_BIT,
						VK_ACCESS_SHADER_READ_BIT,
						VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
						VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
						VK_QUEUE_FAMILY_IGNORED,
						VK_QUEUE_FAMILY_IGNORED,
						*Image,
						ImageSubresourceRange
					};
					vkCmdPipelineBarrier(CommandBuffer,
						VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
						VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
						0,
						0, nullptr,
						0, nullptr,
						1, &ImageMemoryBarrier);
				}

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
	}
	if (VK_NULL_HANDLE != StagingDeviceMemory) {
		vkFreeMemory(Device, StagingDeviceMemory, nullptr);
	}
	if (VK_NULL_HANDLE != StagingBuffer) {
		vkDestroyBuffer(Device, StagingBuffer, nullptr);
	}

	//!< ビューを作成
	const auto Swizzles = GLITexture.swizzles();
	const VkComponentMapping ComponentMapping = {
		ToVkComponentSwizzle(Swizzles.r),
		ToVkComponentSwizzle(Swizzles.g),
		ToVkComponentSwizzle(Swizzles.b),
		ToVkComponentSwizzle(Swizzles.a),
	};
	CreateImageView(ImageView, *Image, ToVkImageViewType(GLITexture.target()), Format, ComponentMapping, ImageSubresourceRange_Color);

	//!< サンプラを作成
	CreateSampler(static_cast<const float>(Levels));
}