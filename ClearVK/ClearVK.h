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

			const auto Image = SwapchainBackBuffers[i].Image;
			//!< バリア (描き込み先へ)
			ImageMemoryBarrier(CB,
				Image,
				VK_ACCESS_MEMORY_READ_BIT, VK_ACCESS_TRANSFER_WRITE_BIT,
				VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);

			//!< カラーでクリア、レンダーパス内では使用できない (Cannot be used in renderpass)
			constexpr std::array ISRs = { 
				VkImageSubresourceRange({
					.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
					.baseMipLevel = 0, .levelCount = VK_REMAINING_MIP_LEVELS,
					.baseArrayLayer = 0, .layerCount = VK_REMAINING_ARRAY_LAYERS
				}),
			};
			vkCmdClearColorImage(CB, Image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &Colors::Blue, static_cast<uint32_t>(size(ISRs)), data(ISRs));

			//!< バリア (プレゼント元へ)
			ImageMemoryBarrier(CB,
				Image,
				VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_MEMORY_READ_BIT,
				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
				VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);
		} VERIFY_SUCCEEDED(vkEndCommandBuffer(CB));
#else
		PopulateCommandBuffer_Clear(i, Colors::SkyBlue);
#endif
	}
};
#pragma endregion