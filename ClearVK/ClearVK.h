#pragma once

#include "resource.h"

#pragma region Code
#include "../VKExt.h"

class ClearVK : public VKExt
{
private:
	using Super = VKExt;
public:
	ClearVK() : Super() {}
	virtual ~ClearVK() {}

#ifdef USE_MANUAL_CLEAR
	virtual void CreateSwapchain() override {
		VK::CreateSwapchain(GetCurrentPhysicalDevice(), Surface, GetClientRectWidth(), GetClientRectHeight(), VK_IMAGE_USAGE_TRANSFER_DST_BIT); 
		GetSwapchainImages();
	}
#else
	virtual void CreateRenderPass() { VKExt::CreateRenderPass_Clear(); }
#endif
	virtual void PopulateCommandBuffer(const size_t i) override {
#ifdef USE_MANUAL_CLEAR
		const auto CB = CommandBuffers[i];
		constexpr VkCommandBufferBeginInfo CBBI = {
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
			.pNext = nullptr,
			.flags = 0,
			.pInheritanceInfo = nullptr
		};
		VERIFY_SUCCEEDED(vkBeginCommandBuffer(CB, &CBBI)); {
			vkCmdSetViewport(CB, 0, static_cast<uint32_t>(size(Viewports)), data(Viewports));
			vkCmdSetScissor(CB, 0, static_cast<uint32_t>(size(ScissorRects)), data(ScissorRects));

			const auto Image = SwapchainImages[i];
			constexpr VkImageSubresourceRange ISR = {
				.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
				.baseMipLevel = 0, .levelCount = VK_REMAINING_MIP_LEVELS,
				.baseArrayLayer = 0, .layerCount = VK_REMAINING_ARRAY_LAYERS
			};

			//!< バリア (描き込み先へ)
			constexpr std::array<VkMemoryBarrier, 0> MBs = {};
			constexpr std::array<VkBufferMemoryBarrier, 0> BMBs = {};
			{
				const std::array IMBs = {
					VkImageMemoryBarrier({
						.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
						.pNext = nullptr,
						.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT, .dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
						.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED, .newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
						.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED, .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
						.image = Image,
						.subresourceRange = ISR
					}),
				};
				vkCmdPipelineBarrier(CB, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 
					static_cast<uint32_t>(size(MBs)), data(MBs),
					static_cast<uint32_t>(size(BMBs)), data(BMBs),
					static_cast<uint32_t>(size(IMBs)), data(IMBs));
			}

			//!< カラーでクリア、レンダーパス内では使用できない (Cannot be used in renderpass)
			constexpr std::array ISRs = { ISR };
			vkCmdClearColorImage(CB, Image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &Colors::Blue, static_cast<uint32_t>(size(ISRs)), data(ISRs));

			//!< バリア (プレゼント元へ)
			{
				const std::array IMBs = {
					VkImageMemoryBarrier({
						.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
						.pNext = nullptr,
						.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT, .dstAccessMask = VK_ACCESS_MEMORY_READ_BIT,
						.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, .newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
						.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED, .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
						.image = Image,
						.subresourceRange = ISR
					}),
				};
				vkCmdPipelineBarrier(CB, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
					static_cast<uint32_t>(size(MBs)), data(MBs),
					static_cast<uint32_t>(size(BMBs)), data(BMBs),
					static_cast<uint32_t>(size(IMBs)), data(IMBs));
			}
		} VERIFY_SUCCEEDED(vkEndCommandBuffer(CB));
#else
		PopulateCommandBuffer_Clear(i, Colors::SkyBlue);
#endif
	}
};
#pragma endregion