#include "VKImage.h"

VkFormat VKImage::ToVkFormat(const gli::format GLIFormat)
{
	//!< 圧縮テクスチャ
	//!< DXT1	... BC1		bpp4	RGB,RGBA	A2諧調
	//!< DXT2,3	...	BC2		bpp8	RGBA		A16諧調
	//!< DXT4,5	... BC3		bpp8	RGBA
	//!< ATI1N	... BC4		bpp4	R			ハイトマップ等
	//!< ATI2N	... BC5		bpp8	RG			ノーマルマップ等
	//!<			BC6H	bpp8	RGB			HDR
	//!<			BC7		bpp8	RGB,RGBA
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
		using enum gli::target;
	default: assert(false && "Not supported"); break;
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
	return VK_IMAGE_TYPE_MAX_ENUM;
}

bool VKImage::IsCube(const gli::target GLITarget)
{
	switch (GLITarget)
	{
		using enum gli::target;
	default: break;
	case TARGET_CUBE:
	case TARGET_CUBE_ARRAY:
		return true;
	}
	return false;
}

VkComponentSwizzle VKImage::ToVkComponentSwizzle(const gli::swizzle GLISwizzle)
{
	switch (GLISwizzle)
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

VkComponentMapping VKImage::ToVkComponentMapping(const gli::texture::swizzles_type GLISwizzlesType)
{
	return { 
		ToVkComponentSwizzle(GLISwizzlesType.r), 
		ToVkComponentSwizzle(GLISwizzlesType.g), 
		ToVkComponentSwizzle(GLISwizzlesType.b), 
		ToVkComponentSwizzle(GLISwizzlesType.a)
	};
}

void VKImage::CreateImage(VkImage* Img, const VkSampleCountFlagBits SampleCount, const VkImageUsageFlags Usage, const gli::texture& GLITexture) const
{
	const auto CreateFlag = IsCube(GLITexture.target()) ? VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT : 0;
	const auto Type = ToVkImageType(GLITexture.target());
	const auto Format = ToVkFormat(GLITexture.format());

	const auto GLIExtent3D = GLITexture.extent(0);
	const VkExtent3D Extent3D = {
		static_cast<const uint32_t>(GLIExtent3D.x), static_cast<const uint32_t>(GLIExtent3D.y), static_cast<const uint32_t>(GLIExtent3D.z)
	};

	const auto Faces = static_cast<const uint32_t>(GLITexture.faces());
	const auto Layers = static_cast<const uint32_t>(GLITexture.layers()) * Faces;
	const auto Levels = static_cast<const uint32_t>(GLITexture.levels());

	Super::CreateImage(Img, CreateFlag, Type, Format, Extent3D, Levels, Layers, SampleCount, Usage);
}

//!< @param コマンドバッファ
//!< @param コピー元バッファ
//!< @param コピー先イメージ
//!< @param (コピー後の)イメージのアクセスフラグ ex) VK_ACCESS_SHADER_READ_BIT,...等
//!< @param (コピー後の)イメージのレイアウト ex) VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,...等
//!< @param (コピー後に)イメージが使われるパイプラインステージ ex) VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,...等
void VKImage::CopyBufferToImage(const VkCommandBuffer CB, const VkBuffer Src, const VkImage Dst, const VkAccessFlags AF, const VkImageLayout IL, const VkPipelineStageFlags PSF, const gli::texture& GLITexture)
{
	//!< キューブマップの場合は、複数レイヤのイメージとして作成する。When cubemap, create as layered image.
	//!< イメージビューを介して、レイヤをフェイスとして扱うようハードウエアへ伝える。Tell the hardware that it should interpret its layers as faces
	//!< キューブマップの場合フェイスの順序は +X-X+Y-Y+Z-Z When cubemap, faces order is +X-X+Y-Y+Z-Z

	//GLITexture.base_face();
	const auto Faces = static_cast<const uint32_t>(GLITexture.faces());
	//GLITexture.base_layer();
	const auto Layers = static_cast<const uint32_t>(GLITexture.layers()) * Faces;
	//GLITexture.base_level();
	const auto Levels = static_cast<const uint32_t>(GLITexture.levels());

	const VkCommandBufferBeginInfo CBBI = {
		VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		nullptr,
		VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
		nullptr
	};
	VERIFY_SUCCEEDED(vkBeginCommandBuffer(CB, &CBBI)); {
		//!< バッファの内容をイメージにコピーするための情報
		const VkOffset3D Offset3D = { 0, 0, 0 };
		std::vector<VkBufferImageCopy> BICs;
		BICs.reserve(Layers);
		VkDeviceSize Offset = 0;

		for (uint32_t i = 0; i < Layers; ++i) {
			for (uint32_t j = 0; j < Levels; ++j) {
				const VkImageSubresourceLayers ISL = {
					VK_IMAGE_ASPECT_COLOR_BIT,
					j,
					i, 1
				};
				const VkExtent3D Extent3D = { 
					static_cast<const uint32_t>(GLITexture.extent(j).x), static_cast<const uint32_t>(GLITexture.extent(j).y), static_cast<const uint32_t>(GLITexture.extent(j).z)
				};
				BICs.push_back({ Offset, 0, 0, ISL, Offset3D, Extent3D });
				Offset += static_cast<const VkDeviceSize>(GLITexture.size(j));
			}
		}

		assert(!empty(BICs) && "BufferImageCopy is empty");

		const VkImageSubresourceRange ISR = {
			VK_IMAGE_ASPECT_COLOR_BIT,
			0, Levels,
			0, Layers
		};
		const std::vector<VkImageMemoryBarrier> IMBs_Pre = {
			{
				VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
				nullptr,
				0, VK_ACCESS_TRANSFER_WRITE_BIT,
				VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, //!< イメージ作成直後は UNDEFINED なので TRANSFER_DST へ
				VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED,
				Dst,
				ISR
			}
		};
		vkCmdPipelineBarrier(CB,
			VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
			0,
			0, nullptr,
			0, nullptr,
			static_cast<uint32_t>(IMBs_Pre.size()), IMBs_Pre.data());
		{
			assert(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL == IMBs_Pre[0].newLayout);
			vkCmdCopyBufferToImage(CB, Src, Dst, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, static_cast<uint32_t>(BICs.size()), BICs.data());
		}
		const std::vector<VkImageMemoryBarrier> IMBs_Post = {
			{
				VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
				nullptr,
				VK_ACCESS_TRANSFER_WRITE_BIT, AF,
				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, IL,
				VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED,
				Dst,
				ISR
			}
		};
		assert(IMBs_Pre[0].dstAccessMask == IMBs_Post[0].srcAccessMask);
		assert(IMBs_Pre[0].newLayout == IMBs_Post[0].oldLayout);
		vkCmdPipelineBarrier(CB,
			VK_PIPELINE_STAGE_TRANSFER_BIT, PSF,
			0,
			0, nullptr,
			0, nullptr,
			static_cast<uint32_t>(IMBs_Post.size()), IMBs_Post.data());

	} VERIFY_SUCCEEDED(vkEndCommandBuffer(CB));
}

//!< @param コマンドバッファ
//!< @param コピー元イメージ
//!< @param コピー先バッファ
//!< @param (コピー後の)イメージのアクセスフラグ ex) VK_ACCESS_SHADER_READ_BIT,...等
//!< @param (コピー後の)イメージのレイアウト ex) VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,...等
//!< @param (コピー後に)イメージが使われるパイプラインステージ ex) VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,...等
void VKImage::CopyImageToBuffer(const VkCommandBuffer CB, const VkImage Src, const VkBuffer Dst, const VkAccessFlags AF, const VkImageLayout IL, const VkPipelineStageFlags PSF, const gli::texture& GLITexture)
{
	const auto Faces = static_cast<const uint32_t>(GLITexture.faces());
	const auto Layers = static_cast<const uint32_t>(GLITexture.layers()) * Faces;
	const auto Levels = static_cast<const uint32_t>(GLITexture.levels());

	const VkCommandBufferBeginInfo CBBI = {
		VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		nullptr,
		VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
		nullptr
	};
	VERIFY_SUCCEEDED(vkBeginCommandBuffer(CB, &CBBI)); {
		const VkOffset3D Offset3D = { 0, 0, 0 };
		std::vector<VkBufferImageCopy> BICs;
		BICs.reserve(Layers);
		VkDeviceSize Offset = 0;

		for (uint32_t i = 0; i < Layers; ++i) {
			for (uint32_t j = 0; j < Levels; ++j) {
				const VkImageSubresourceLayers ISL = {
					VK_IMAGE_ASPECT_COLOR_BIT,
					j,
					i, 1
				};
				const VkExtent3D Extent3D = {
					static_cast<const uint32_t>(GLITexture.extent(j).x), static_cast<const uint32_t>(GLITexture.extent(j).y), static_cast<const uint32_t>(GLITexture.extent(j).z)
				};
				BICs.push_back({ Offset, 0, 0, ISL, Offset3D, Extent3D });
				Offset += static_cast<const VkDeviceSize>(GLITexture.size(j));
			}
		}
		assert(!empty(BICs) && "BufferImageCopy is empty");

		const VkImageSubresourceRange ISR = {
			VK_IMAGE_ASPECT_COLOR_BIT,
			0, Levels,
			0, Layers
		};
		const VkImageMemoryBarrier IMB_Pre = {
			VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
			nullptr,
			0, VK_ACCESS_TRANSFER_READ_BIT,
			VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, //!< イメージ作成直後は UNDEFINED なので TRANSFER_SRC へ
			VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED,
			Src,
			ISR
		};
		vkCmdPipelineBarrier(CB,
			VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
			0,
			0, nullptr,
			0, nullptr,
			1, &IMB_Pre);
		{
			assert(VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL == IMB_Pre.newLayout);
			vkCmdCopyImageToBuffer(CB, Src, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, Dst, static_cast<uint32_t>(BICs.size()), BICs.data());
		}
		const VkImageMemoryBarrier IMB_Post = {
			VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
			nullptr,
			VK_ACCESS_TRANSFER_READ_BIT, AF,
			VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, IL,
			VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED,
			Src,
			ISR
		};
		assert(IMB_Pre.dstAccessMask == IMB_Post.srcAccessMask);
		assert(IMB_Pre.newLayout == IMB_Post.oldLayout);
		vkCmdPipelineBarrier(CB,
			VK_PIPELINE_STAGE_TRANSFER_BIT, PSF,
			0,
			0, nullptr,
			0, nullptr,
			1, &IMB_Post);

	} VERIFY_SUCCEEDED(vkEndCommandBuffer(CB));
}

void VKImage::CreateImageView(VkImageView* IV, const VkImage Img, const gli::texture& GLITexture)
{
	const auto Type = ToVkImageViewType(GLITexture.target());
	const auto Format = ToVkFormat(GLITexture.format());
	const auto CompMap = ToVkComponentMapping(GLITexture.swizzles());

	Super::CreateImageView(IV, Img, Type, Format, CompMap, ImageSubresourceRange_ColorAll);
}

gli::texture VKImage::LoadImage_DDS(VkImage* Img, VkDeviceMemory* DM, const VkPipelineStageFlags PSF, const std::string& Path)
{
	//!< DDS or KTX or KMG を読み込める (DDS or KTX or KMG can be read)
	const auto GLITexture(gli::load(Path.c_str()));
	assert(!GLITexture.empty() && "Load image failed");

#ifdef DEBUG_STDOUT
	VkFormatProperties FormatProperties;
	vkGetPhysicalDeviceFormatProperties(GetCurrentPhysicalDevice(), ToVkFormat(GLITexture.format()), &FormatProperties);

#define FORMAT_ENTRIES() FORMAT_PROPERTY_ENTRY(SAMPLED_IMAGE_BIT);\
	FORMAT_PROPERTY_ENTRY(STORAGE_IMAGE_BIT);\
	FORMAT_PROPERTY_ENTRY(STORAGE_IMAGE_ATOMIC_BIT);\
	FORMAT_PROPERTY_ENTRY(UNIFORM_TEXEL_BUFFER_BIT);\
	FORMAT_PROPERTY_ENTRY(STORAGE_TEXEL_BUFFER_BIT);\
	FORMAT_PROPERTY_ENTRY(STORAGE_TEXEL_BUFFER_ATOMIC_BIT);\
	FORMAT_PROPERTY_ENTRY(VERTEX_BUFFER_BIT);\
	FORMAT_PROPERTY_ENTRY(COLOR_ATTACHMENT_BIT);\
	FORMAT_PROPERTY_ENTRY(COLOR_ATTACHMENT_BLEND_BIT);\
	FORMAT_PROPERTY_ENTRY(DEPTH_STENCIL_ATTACHMENT_BIT);\
	FORMAT_PROPERTY_ENTRY(BLIT_SRC_BIT);\
	FORMAT_PROPERTY_ENTRY(BLIT_DST_BIT);\
	FORMAT_PROPERTY_ENTRY(SAMPLED_IMAGE_FILTER_LINEAR_BIT);\
	FORMAT_PROPERTY_ENTRY(SAMPLED_IMAGE_FILTER_CUBIC_BIT_IMG);\
	FORMAT_PROPERTY_ENTRY(TRANSFER_SRC_BIT_KHR);\
	FORMAT_PROPERTY_ENTRY(TRANSFER_DST_BIT_KHR);\
	std::cout << std::endl; std::cout << std::endl;

	std::cout << "\t" << "\t" << "linearTilingFeatures = ";
#define FORMAT_PROPERTY_ENTRY(entry) if(VK_FORMAT_FEATURE_##entry & FormatProperties.linearTilingFeatures) { std::cout << #entry << " | "; }
	FORMAT_ENTRIES()
#undef FORMAT_PROPERTY_ENTRY

		std::cout << "\t" << "\t" << "optimalTilingFeatures = ";
#define FORMAT_PROPERTY_ENTRY(entry) if(VK_FORMAT_FEATURE_##entry & FormatProperties.optimalTilingFeatures) { std::cout << #entry << " | "; }
	FORMAT_ENTRIES()
#undef FORMAT_PROPERTY_ENTRY

		std::cout << "\t" << "\t" << "bufferFeatures = ";
#define FORMAT_PROPERTY_ENTRY(entry) if(VK_FORMAT_FEATURE_##entry & FormatProperties.bufferFeatures) { std::cout << #entry << " | "; }
	FORMAT_ENTRIES()
#undef FORMAT_PROPERTY_ENTRY

		std::cout << std::endl;
#endif //!< DEBUG_STDOUT

	auto CB = CommandBuffers[0];//CommandPools[0].second[0];
	const auto Size = static_cast<VkDeviceSize>(GLITexture.size());

	VkBuffer StagingBuffer = VK_NULL_HANDLE;
	VkDeviceMemory StagingDeviceMemory = VK_NULL_HANDLE;
	{
		//!< ホストビジブルのバッファとメモリを作成、データをコピー( Create host visible buffer and memory, and copy data)
		CreateBuffer(&StagingBuffer, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, Size);
		AllocateDeviceMemory(&StagingDeviceMemory, StagingBuffer, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
		CopyToHostVisibleDeviceMemory(StagingDeviceMemory, Size, GLITexture.data(), 0);
		VERIFY_SUCCEEDED(vkBindBufferMemory(Device, StagingBuffer, StagingDeviceMemory, 0));

		//!< デバイスローカルのイメージとメモリを作成 (Create device local image and memory)
		//!< VK_IMAGE_USAGE_SAMPLED_BIT : サンプルドイメージ ... シェーダ内でサンプラとともに使われる為に指定する
		//!< - 全てのテクスチャフォーマットとリニアフィルタをサポートするわけではない (ValidateFormatProoerties()でチェックしている)
		//!< - プラットフォームによってはコンバインドイメージサンプラ(サンプラとサンプルドイメージを１つにまとめたもの)を使った方が効率が良い場合がある
		CreateImage(Img, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT, GLITexture);

#ifdef USE_SUBALLOC
		uint32_t HeapIndex;
		VkDeviceSize Offset;
		SuballocateImageMemory(HeapIndex, Offset, *Img, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
#else
		AllocateDeviceMemory(DM, *Img, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		VERIFY_SUCCEEDED(vkBindImageMemory(Device, *Img, *DM, 0));
#endif

		//!< ホストビジブルからデバイスローカルへのコピーコマンドを発行 (Submit copy command from host visible to device local)
		CopyBufferToImage(CB, StagingBuffer, *Img, VK_ACCESS_SHADER_READ_BIT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, PSF, GLITexture);
		const std::vector<VkSubmitInfo> SIs = {
			{
				VK_STRUCTURE_TYPE_SUBMIT_INFO,
				nullptr,
				0, nullptr,
				nullptr,
				1, &CB,
				0, nullptr
			}
		};
		VERIFY_SUCCEEDED(vkQueueSubmit(GraphicsQueue, static_cast<uint32_t>(SIs.size()), SIs.data(), VK_NULL_HANDLE));
		VERIFY_SUCCEEDED(vkQueueWaitIdle(GraphicsQueue));
	}
	if (VK_NULL_HANDLE != StagingDeviceMemory) {
		vkFreeMemory(Device, StagingDeviceMemory, GetAllocationCallbacks());
	}
	if (VK_NULL_HANDLE != StagingBuffer) {
		vkDestroyBuffer(Device, StagingBuffer, GetAllocationCallbacks());
	}

	ValidateFormatProperties_SampledImage(GetCurrentPhysicalDevice(), ToVkFormat(GLITexture.format()), VK_SAMPLE_COUNT_1_BIT, VK_FILTER_LINEAR, VK_FILTER_LINEAR, VK_SAMPLER_MIPMAP_MODE_LINEAR);

	return GLITexture;
}

#if 0
//!< Image を Buffer へコピーする例
void VKImage::XXX(VkBuffer* Buf, VkDeviceMemory* DM, const gli::texture& GLITexture, const VkCommandBuffer CB)
{
	const auto Size = static_cast<VkDeviceSize>(GLITexture.size());
		CreateBuffer(Buf, VK_BUFFER_USAGE_TRANSFER_DST_BIT, Size);
		CreateDeviceMemory(DM, *Buf, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
		BindDeviceMemory(*Buf, *DM);

		VkImage Image;
		VkDeviceMemory IDM;
		CreateImage(&Image, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT, GLITexture);
		CreateDeviceMemory(&IDM, Image, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		BindDeviceMemory(Image, IDM);

		CopyImageToBuffer(CB, Image, *Buf, VK_ACCESS_SHADER_READ_BIT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, GLITexture);
		const std::vector<VkSubmitInfo> SIs = {
			{
				VK_STRUCTURE_TYPE_SUBMIT_INFO,
				nullptr,
				0, nullptr,
				nullptr,
				1, &CB,
				0, nullptr
			}
		};
		VERIFY_SUCCEEDED(vkQueueSubmit(GraphicsQueue, static_cast<uint32_t>(SIs.size()), SIs.data(), VK_NULL_HANDLE));
		VERIFY_SUCCEEDED(vkQueueWaitIdle(GraphicsQueue));

		vkFreeMemory(Device, IDM, GetAllocationCallbacks());
		vkDestroyImage(Device, Image, GetAllocationCallbacks());

		vkFreeMemory(Device, *DM, GetAllocationCallbacks());
		vkDestroyBuffer(Device, *Buf, GetAllocationCallbacks());
}
#endif

#if 0
//!< Storage Image を作成する例
VkDeviceMemory* DeviceMemory;
VkImage* Image;
VkImageView* ImageView;
Super::CreateImage(Image, 0, VK_IMAGE_TYPE_2D, VK_FORMAT_R8G8B8A8_UNORM, { 1280, 720 }, 1, 1, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_USAGE_STORAGE_BIT);
CreateDeviceMemory(DeviceMemory, *Image, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
BindDeviceMemory(*Image, *DeviceMemory);
Super::CreateImageView(ImageView, *Image, VK_IMAGE_VIEW_TYPE_2D, VK_FORMAT_R8G8B8A8_UNORM, 
	{ VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A }, 
	ImageSubresourceRange_ColorAll);
//!< イメージレイアウトを VK_IMAGE_LAYOUT_GENERAL へ変更すること、このレイアウトでのみ操作がサポートされている
#endif