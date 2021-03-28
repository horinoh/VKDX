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
	static [[nodiscard]] VkComponentMapping ToVkComponentMapping(const gli::texture::swizzles_type GLISwizzleType) { 
		return VkComponentMapping({ .r = ToVkComponentSwizzle(GLISwizzleType.r), .g = ToVkComponentSwizzle(GLISwizzleType.g), .b = ToVkComponentSwizzle(GLISwizzleType.b), .a = ToVkComponentSwizzle(GLISwizzleType.a) });
	}

protected:
	class DDSTexture : public Texture
	{
	private:
		using Super = Texture;
		gli::texture GLITexture;

	public:
		DDSTexture& Create(const VkDevice Dev, const VkPhysicalDeviceMemoryProperties PDMP, std::string_view Path) {
			assert(std::filesystem::exists(Path) && "");
			assert(Path.ends_with(".dds") && "");
			GLITexture = gli::load(data(Path));
			assert(!GLITexture.empty() && "Load image failed");

			const auto Format = ToVkFormat(GLITexture.format());
			VK::CreateImageMemory(&Image, &DeviceMemory, Dev, PDMP, 
				gli::is_target_cube(GLITexture.target()) ? VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT : 0, 
				ToVkImageType(GLITexture.target()), 
				Format, 
				VkExtent3D({ .width = static_cast<const uint32_t>(GLITexture.extent(0).x), .height = static_cast<const uint32_t>(GLITexture.extent(0).y), .depth = static_cast<const uint32_t>(GLITexture.extent(0).z) }),
				static_cast<const uint32_t>(GLITexture.levels()), 
				static_cast<const uint32_t>(GLITexture.layers()) * static_cast<const uint32_t>(GLITexture.faces()), 
				VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);

			const VkImageViewCreateInfo IVCI = {
				.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
				.pNext = nullptr,
				.flags = 0,
				.image = Image,
				.viewType = ToVkImageViewType(GLITexture.target()),
				.format = Format,
				.components = ToVkComponentMapping(GLITexture.swizzles()),
				.subresourceRange = VkImageSubresourceRange({.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .baseMipLevel = 0, .levelCount = VK_REMAINING_MIP_LEVELS, .baseArrayLayer = 0, .layerCount = VK_REMAINING_ARRAY_LAYERS })
			};
			VERIFY_SUCCEEDED(vkCreateImageView(Dev, &IVCI, GetAllocationCallbacks(), &View));
			return *this;
		}
		void CreateStagingBuffer(const VkDevice Dev, VkPhysicalDeviceMemoryProperties PDMP, BufferMemory& BM) {
#ifdef USE_EXPERIMENTAL
			BM.Create(Dev, PDMP, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, static_cast<VkDeviceSize>(Util::size(GLITexture)), VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, Util::data(GLITexture));
#else
			BM.Create(Dev, PDMP, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, static_cast<VkDeviceSize>(GLITexture.size()), VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, GLITexture.data());
#endif
		}
		void PopulateCopyCommand(const VkCommandBuffer CB, const VkPipelineStageFlags PSF, const VkBuffer Staging) {
			PopulateCommandBuffer_CopyBufferToImage(CB, Staging, Image, VK_ACCESS_SHADER_READ_BIT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, PSF, GLITexture);
		}
		void SubmitCopyCommand(const VkDevice Dev, const VkPhysicalDeviceMemoryProperties PDMP, const VkCommandBuffer CB, const VkQueue Queue, const VkPipelineStageFlags PSF) {
			VK::Scoped<BufferMemory> StagingBuffer(Dev);
			CreateStagingBuffer(Dev, PDMP, StagingBuffer);
			constexpr VkCommandBufferBeginInfo CBBI = { .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO, .pNext = nullptr, .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, .pInheritanceInfo = nullptr };
			VERIFY_SUCCEEDED(vkBeginCommandBuffer(CB, &CBBI)); {
				PopulateCopyCommand(CB, PSF, StagingBuffer.Buffer);
			} VERIFY_SUCCEEDED(vkEndCommandBuffer(CB));
			VK::SubmitAndWait(Queue, CB);
		}
	};

	virtual void OnDestroy(HWND hWnd, HINSTANCE hInstance) override {
		for (auto& i : DDSTextures) { i.Destroy(Device); } DDSTextures.clear();
		Super::OnDestroy(hWnd, hInstance);
	}

	static void PopulateCommandBuffer_CopyBufferToImage(const VkCommandBuffer CB, const VkBuffer Src, const VkImage Dst, const VkAccessFlags AF, const VkImageLayout IL, const VkPipelineStageFlags PSF, const gli::texture& GLITexture);
	static void PopulateCommandBuffer_CopyImageToBuffer(const VkCommandBuffer CB, const VkImage Src, const VkBuffer Dst, const VkAccessFlags AF, const VkImageLayout IL, const VkPipelineStageFlags PSF, const gli::texture& GLITexture);

	std::vector<DDSTexture> DDSTextures;
};