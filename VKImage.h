#pragma once

#include "VKRT.h"

#pragma warning(push)
#pragma warning(disable : 4100)
//#pragma warning(disable : 4201)
#pragma warning(disable : 4244)
#pragma warning(disable : 4458)
#pragma warning(disable : 5054)
#pragma warning(disable : 4189)
#include <gli/gli.hpp>
#pragma warning(pop)

class VKImage : public VKExt
{
private:
	using Super = VKExt;
public:
	[[nodiscard]] static VkFormat ToVkFormat(const gli::format GLIFormat);
	[[nodiscard]] static VkImageViewType ToVkImageViewType(const gli::target GLITarget);
	[[nodiscard]] static VkImageType ToVkImageType(const gli::target GLITarget);
	[[nodiscard]] static VkComponentSwizzle ToVkComponentSwizzle(const gli::swizzle GLISwizzle);
	[[nodiscard]] static VkComponentMapping ToVkComponentMapping(const gli::texture::swizzles_type GLISwizzleType) { 
		return VkComponentMapping({ .r = ToVkComponentSwizzle(GLISwizzleType.r), .g = ToVkComponentSwizzle(GLISwizzleType.g), .b = ToVkComponentSwizzle(GLISwizzleType.b), .a = ToVkComponentSwizzle(GLISwizzleType.a) });
	}
	static void GetPhysicalDeviceImageFormatProperties(VkImageFormatProperties& IFP, const VkPhysicalDevice& Dev, const gli::texture& GLI) {
		VERIFY_SUCCEEDED(vkGetPhysicalDeviceImageFormatProperties(Dev,
			VKImage::ToVkFormat(GLI.format()),
			VKImage::ToVkImageType(GLI.target()),
			VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
			gli::is_target_cube(GLI.target()) ? VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT : 0,
			&IFP));
		OutputDebugStringA(data(std::format("maxExtent = {} x {} x {}\n", IFP.maxExtent.width, IFP.maxExtent.height, IFP.maxExtent.depth)));
		OutputDebugStringA(data(std::format("maxMipLevels = {}\n", IFP.maxMipLevels)));
		OutputDebugStringA(data(std::format("maxArrayLayers = {}\n", IFP.maxArrayLayers)));
		OutputDebugStringA(data(std::format("sampleCounts = {}\n", IFP.sampleCounts)));
		OutputDebugStringA(data(std::format("maxResourceSize = {}\n", IFP.maxResourceSize)));
	}
	//static void PopulateCommandBuffer_CopyImageToBuffer(const VkCommandBuffer CB, const VkImage Src, const VkBuffer Dst, const VkAccessFlags AF, const VkImageLayout IL, const VkPipelineStageFlags PSF, const gli::texture& GliTexture) {
	//	const auto Layers = static_cast<const uint32_t>(GliTexture.layers()) * static_cast<const uint32_t>(GliTexture.faces());
	//	const auto Levels = static_cast<const uint32_t>(GliTexture.levels());
	//	std::vector<VkBufferImageCopy> BICs; BICs.reserve(Layers);
	//	VkDeviceSize Offset = 0;
	//	for (uint32_t i = 0; i < Layers; ++i) {
	//		for (uint32_t j = 0; j < Levels; ++j) {
	//			BICs.emplace_back(VkBufferImageCopy({
	//				.bufferOffset = Offset, .bufferRowLength = 0, .bufferImageHeight = 0,
	//				.imageSubresource = VkImageSubresourceLayers({.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .mipLevel = j, .baseArrayLayer = i, .layerCount = 1 }),
	//				.imageOffset = VkOffset3D({.x = 0, .y = 0, .z = 0 }),
	//				.imageExtent = VkExtent3D({.width = static_cast<const uint32_t>(GliTexture.extent(j).x), .height = static_cast<const uint32_t>(GliTexture.extent(j).y), .depth = static_cast<const uint32_t>(GliTexture.extent(j).z) }) }));
	//			Offset += static_cast<const VkDeviceSize>(GliTexture.size(j));
	//		}
	//	}
	//	VK::PopulateCommandBuffer_CopyImageToBuffer(CB, Src, Dst, AF, IL, PSF, BICs, Levels, Layers);
	//}

	class GLITexture : public Texture
	{
	private:
		using Super = Texture;
		gli::texture GliTexture;

	public:
		GLITexture& Create(const VkDevice Dev, const VkPhysicalDeviceMemoryProperties PDMP, const std::filesystem::path& Path) {
			assert(std::filesystem::exists(Path) && "");
			//!< 対応フォーマット DDS, KTX
			if (IsDDS(Path) || IsKTX(Path)) {
				GliTexture = gli::load(data(Path.string()));
				assert(!GliTexture.empty() && "Load image failed");
				const auto Format = ToVkFormat(GliTexture.format());
				VK::CreateImageMemory(&Image, &DeviceMemory, Dev, PDMP,
					gli::is_target_cube(GliTexture.target()) ? VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT : 0,
					ToVkImageType(GliTexture.target()),
					Format,
					VkExtent3D({ .width = static_cast<const uint32_t>(GliTexture.extent(0).x), .height = static_cast<const uint32_t>(GliTexture.extent(0).y), .depth = static_cast<const uint32_t>(GliTexture.extent(0).z) }),
					static_cast<const uint32_t>(GliTexture.levels()),
					static_cast<const uint32_t>(GliTexture.layers()) * static_cast<const uint32_t>(GliTexture.faces()),
					VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);

				const VkImageViewCreateInfo IVCI = {
					.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
					.pNext = nullptr,
					.flags = 0,
					.image = Image,
					.viewType = ToVkImageViewType(GliTexture.target()),
					.format = Format,
					.components = ToVkComponentMapping(GliTexture.swizzles()),
					.subresourceRange = VkImageSubresourceRange({.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .baseMipLevel = 0, .levelCount = VK_REMAINING_MIP_LEVELS, .baseArrayLayer = 0, .layerCount = VK_REMAINING_ARRAY_LAYERS })
				};
				VERIFY_SUCCEEDED(vkCreateImageView(Dev, &IVCI, GetAllocationCallbacks(), &View));
			}
			return *this;
		}
		void CopyToStagingBuffer(const VkDevice Dev, const VkPhysicalDeviceMemoryProperties PDMP, StagingBuffer& Staging) {
			Staging.Create(Dev, PDMP, static_cast<VkDeviceSize>(GliTexture.size()), GliTexture.data());
		}
		void PopulateCopyCommand(const VkCommandBuffer CB, const VkPipelineStageFlags PSF, const VkBuffer Staging) {
			//!< キューブマップの場合は、複数レイヤのイメージとして作成する。(When cubemap, create as layered image)
			//!< イメージビューを介して、レイヤをフェイスとして扱うようハードウエアへ伝える (Tell the hardware that it should interpret its layers as faces)
			//!< キューブマップの場合フェイスの順序は +X-X+Y-Y+Z-Z (When cubemap, faces order is +X-X+Y-Y+Z-Z)
			const auto Layers = static_cast<const uint32_t>(GliTexture.layers()) * static_cast<const uint32_t>(GliTexture.faces());
			const auto Levels = static_cast<const uint32_t>(GliTexture.levels());
			std::vector<VkBufferImageCopy2> BICs; BICs.reserve(Layers * Levels);
			VkDeviceSize Offset = 0;
			for (uint32_t i = 0; i < Layers; ++i) {
				for (uint32_t j = 0; j < Levels; ++j) {
					BICs.emplace_back(VkBufferImageCopy2({
						.sType = VK_STRUCTURE_TYPE_BUFFER_IMAGE_COPY_2,
						.pNext = nullptr,
						.bufferOffset = Offset, .bufferRowLength = 0, .bufferImageHeight = 0,
						.imageSubresource = VkImageSubresourceLayers({.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .mipLevel = j, .baseArrayLayer = i, .layerCount = 1 }),
						.imageOffset = VkOffset3D({.x = 0, .y = 0, .z = 0 }),
						.imageExtent = VkExtent3D({.width = static_cast<const uint32_t>(GliTexture.extent(j).x), .height = static_cast<const uint32_t>(GliTexture.extent(j).y), .depth = static_cast<const uint32_t>(GliTexture.extent(j).z) }) }));
					Offset += static_cast<const VkDeviceSize>(GliTexture.size(j));
				}
			}
			VK::PopulateCopyBufferToImageCommand(CB, Staging, Image, VK_ACCESS_SHADER_READ_BIT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, PSF, BICs, Levels, Layers);
		}
		void SubmitCopyCommand(const VkDevice Dev, const VkPhysicalDeviceMemoryProperties PDMP, const VkCommandBuffer CB, const VkQueue Queue, const VkPipelineStageFlags PSF) {
			VK::Scoped<StagingBuffer> Staging(Dev);
			CopyToStagingBuffer(Dev, PDMP, Staging);

			constexpr VkCommandBufferBeginInfo CBBI = {
				.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
				.pNext = nullptr, 
				.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
				.pInheritanceInfo = nullptr
			};
			VERIFY_SUCCEEDED(vkBeginCommandBuffer(CB, &CBBI)); {
				PopulateCopyCommand(CB, PSF, Staging.Buffer);
			} VERIFY_SUCCEEDED(vkEndCommandBuffer(CB));
			VK::SubmitAndWait(Queue, CB);
		}
		void GetPhysicalDeviceImageFormatProperties(VkImageFormatProperties& IFP, const VkPhysicalDevice& PD) {
			VKImage::GetPhysicalDeviceImageFormatProperties(IFP, PD, GliTexture);
		}
	};
	
	void SaveToFile(gli::texture GLITex, std::string_view FileName) {
		gli::save(GLITex, data(FileName));
	}

protected:
	virtual void OnDestroy(HWND hWnd, HINSTANCE hInstance) override {
		for (auto& i : GLITextures) { i.Destroy(Device); } GLITextures.clear();
		Super::OnDestroy(hWnd, hInstance);
	}
	std::vector<GLITexture> GLITextures;
};

class VKImageDepth : public VKExtDepth
{
private:
	using Super = VKExtDepth;

protected:
	virtual void OnDestroy(HWND hWnd, HINSTANCE hInstance) override {
		for (auto& i : GLITextures) { i.Destroy(Device); } GLITextures.clear();
		Super::OnDestroy(hWnd, hInstance);
	}
	std::vector<VKImage::GLITexture> GLITextures;
};

class VKImageRT : public VKRT
{
private:
	using Super = VKRT;

protected:
	virtual void OnDestroy(HWND hWnd, HINSTANCE hInstance) override {
		for (auto& i : GLITextures) { i.Destroy(Device); } GLITextures.clear();
		Super::OnDestroy(hWnd, hInstance);
	}
	std::vector<VKImage::GLITexture> GLITextures;
};