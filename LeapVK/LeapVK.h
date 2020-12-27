#pragma once

#include "resource.h"

#pragma region Code
#include "../VKExt.h"
#include "../Leap.h"

class LeapVK : public VKExt, public Leap
{
private:
	using Super = VKExt;
public:
	LeapVK() : Super() {}
	virtual ~LeapVK() {}

protected:
protected:
	virtual void OnTimer(HWND hWnd, HINSTANCE hInstance) override {
		Super::OnTimer(hWnd, hInstance);
#ifdef USE_LEAP
		InterpolatedTrackingEvent();
#endif
	}
#ifdef USE_LEAP
	virtual void OnTrackingEvent(const LEAP_TRACKING_EVENT* TE) override {
		Leap::OnTrackingEvent(TE);
	}
	virtual void OnImageEvent(const LEAP_IMAGE_EVENT* IE) override {
		Leap::OnImageEvent(IE);
		
		if (!empty(Images)) {
			for (uint32_t i = 0; i < _countof(IE->image); ++i) {
				//const auto& Image = IE->image[i];
				//const auto& Prop = Image.properties;
				//CopyToHostVisibleDeviceMemory(Images[0].DeviceMemory, 0, Prop.width * Prop.height * Prop.bpp, reinterpret_cast<std::byte*>(Image.data) + Image.offset);
			}
		}
	}
#endif
	virtual void CreateTexture() override {
		//!< 配列テクスチャを作る場合、VK_IMAGE_TYPE_2D のままでレイヤー数を指定する
		//!< ビューには VK_IMAGE_VIEW_TYPE_2D_ARRAY を使用する	
#ifdef USE_LEAP
		constexpr auto Layers = _countof(LEAP_IMAGE_EVENT::image);
#else
		constexpr auto Layers = 2;
#endif
		{
			Images.emplace_back(Image());
			CreateImage(&Images.back().Image, 0, VK_IMAGE_TYPE_2D, VK_FORMAT_R8_UNORM, VkExtent3D({ .width = 640, .height = 240, .depth = 1 }), 1, Layers, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);
			AllocateDeviceMemory(&Images.back().DeviceMemory, Images.back().Image, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT); //!< (更新するのでホストビジブルにしたかったが)イメージの場合、ホストビジブルなデバイスメモリは作れなかった
			VERIFY_SUCCEEDED(vkBindImageMemory(Device, Images.back().Image, Images.back().DeviceMemory, 0));

			//CopyToHostVisibleDeviceMemory(Images.back().DeviceMemory, 0, 640 * 240 * 1 * 2, nullptr);

			ImageViews.emplace_back(VkImageView());
			CreateImageView(&ImageViews.back(), Images.back().Image, VK_IMAGE_VIEW_TYPE_2D_ARRAY, VK_FORMAT_R8_UNORM,
				VkComponentMapping({ .r = VK_COMPONENT_SWIZZLE_R, .g = VK_COMPONENT_SWIZZLE_G, .b = VK_COMPONENT_SWIZZLE_B, .a = VK_COMPONENT_SWIZZLE_A }),
				VkImageSubresourceRange({ .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .baseMipLevel = 0, .levelCount = VK_REMAINING_MIP_LEVELS, .baseArrayLayer = 0, .layerCount = VK_REMAINING_ARRAY_LAYERS }));
		}

		//!< ディストーションマップ
		{
			Images.emplace_back(Image());
#ifdef USE_LEAP
			CreateImage(&Images.back().Image, 0, VK_IMAGE_TYPE_2D, VK_FORMAT_R32G32_SFLOAT, VkExtent3D({ .width = LEAP_DISTORTION_MATRIX_N, .height = LEAP_DISTORTION_MATRIX_N, .depth = 1 }), 1, Layers, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);
#else
			CreateImage(&Images.back().Image, 0, VK_IMAGE_TYPE_2D, VK_FORMAT_R32G32_SFLOAT, VkExtent3D({ .width = 64, .height = 64, .depth = 1 }), 1, Layers, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);
#endif
			AllocateDeviceMemory(&Images.back().DeviceMemory, Images.back().Image, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
			VERIFY_SUCCEEDED(vkBindImageMemory(Device, Images.back().Image, Images.back().DeviceMemory, 0));

			//!< TODO 転送
			VkBuffer Buffer;
			VkDeviceMemory DeviceMemory;
			{
				//LEAP_IMAGE_EVENT ImageEvent;//TODO

#ifdef USE_LEAP
				constexpr auto Size = sizeof(LEAP_DISTORTION_MATRIX);
#else
				constexpr auto Size = sizeof(64*64*sizeof(float)*2);
#endif
				CreateBuffer(&Buffer, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, Layers * Size);
				AllocateDeviceMemory(&DeviceMemory, Buffer, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
				VERIFY_SUCCEEDED(vkBindBufferMemory(Device, Buffer, DeviceMemory, 0));

				for (uint32_t i = 0; i < Layers; ++i) {
					//CopyToHostVisibleDeviceMemory(DeviceMemory, i * Size, Size, ImageEvent.image[i].distortion_matrix);
				}

				constexpr uint32_t Levels = 1;
				std::vector<VkBufferImageCopy> BICs;
				for (uint32_t i = 0; i < Layers; ++i) {
					for (uint32_t j = 0; j < Levels; ++j) {
#ifdef USE_LEAP
						BICs.emplace_back(VkBufferImageCopy({ .bufferOffset = i * Size, .bufferRowLength = 0, .bufferImageHeight = 0, .imageSubresource = VkImageSubresourceLayers({.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .mipLevel = j, .baseArrayLayer = i, .layerCount = 1 }), .imageOffset = VkOffset3D({.x = 0, .y = 0, .z = 0 }), .imageExtent = VkExtent3D({.width = LEAP_DISTORTION_MATRIX_N, .height = LEAP_DISTORTION_MATRIX_N, .depth = 1 }) }));
#else
						BICs.emplace_back(VkBufferImageCopy({ .bufferOffset = i * Size, .bufferRowLength = 0, .bufferImageHeight = 0, .imageSubresource = VkImageSubresourceLayers({.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .mipLevel = j, .baseArrayLayer = i, .layerCount = 1 }), .imageOffset = VkOffset3D({.x = 0, .y = 0, .z = 0 }), .imageExtent = VkExtent3D({.width = 64, .height = 64, .depth = 1 }) }));
#endif
					}
				}
				//auto CB = CommandBuffers[0];
				//CmdCopyBufferToImage(CB, Buffer, Images.back().Image, VK_ACCESS_SHADER_READ_BIT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, BICs, Layers, Levels);
			}
			vkFreeMemory(Device, DeviceMemory, GetAllocationCallbacks());
			vkDestroyBuffer(Device, Buffer, GetAllocationCallbacks());

			ImageViews.emplace_back(VkImageView());
			CreateImageView(&ImageViews.back(), Images.back().Image, VK_IMAGE_VIEW_TYPE_2D_ARRAY, VK_FORMAT_R32G32_SFLOAT,
				VkComponentMapping({ .r = VK_COMPONENT_SWIZZLE_R, .g = VK_COMPONENT_SWIZZLE_G, .b = VK_COMPONENT_SWIZZLE_B, .a = VK_COMPONENT_SWIZZLE_A }),
				VkImageSubresourceRange({ .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .baseMipLevel = 0, .levelCount = VK_REMAINING_MIP_LEVELS, .baseArrayLayer = 0, .layerCount = VK_REMAINING_ARRAY_LAYERS }));
		}
	}
};
#pragma endregion