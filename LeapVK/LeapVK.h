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
	}
#endif

	virtual void CreateIndirectBuffer() override { CreateIndirectBuffer_Draw(4, 1); }
	virtual void CreateTexture() override {
#ifdef USE_LEAP
		//!< 配列テクスチャを作る場合、VK_IMAGE_TYPE_2D のままでレイヤー数を指定する
		//!< ビューには VK_IMAGE_VIEW_TYPE_2D_ARRAY を使用する	

		if(false){
			constexpr auto Layers = 2;// _countof(LEAP_IMAGE_EVENT::image); // TODO

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
			const auto Layers = static_cast<uint32_t>(size(DistortionMatrices));
			constexpr auto LayerSize = sizeof(DistortionMatrices[0]);
			const auto TotalSize = Layers * LayerSize;
			constexpr auto Extent = VkExtent3D({ .width = LEAP_DISTORTION_MATRIX_N, .height = LEAP_DISTORTION_MATRIX_N, .depth = 1 });

			Images.emplace_back(Image());
			CreateImage(&Images.back().Image, 0, VK_IMAGE_TYPE_2D, VK_FORMAT_R32G32_SFLOAT, Extent, 1, Layers, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);
			AllocateDeviceMemory(&Images.back().DeviceMemory, Images.back().Image, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
			VERIFY_SUCCEEDED(vkBindImageMemory(Device, Images.back().Image, Images.back().DeviceMemory, 0));

			{
				VkBuffer Buffer;
				VkDeviceMemory DeviceMemory;
				{
					CreateBuffer(&Buffer, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, TotalSize);
					AllocateDeviceMemory(&DeviceMemory, Buffer, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
					VERIFY_SUCCEEDED(vkBindBufferMemory(Device, Buffer, DeviceMemory, 0));

					CopyToHostVisibleDeviceMemory(DeviceMemory, 0, TotalSize, data(DistortionMatrices));

					std::vector<VkBufferImageCopy> BICs;
					for (uint32_t i = 0; i < Layers; ++i) {
						BICs.emplace_back(VkBufferImageCopy({
							.bufferOffset = i * LayerSize, .bufferRowLength = 0, .bufferImageHeight = 0,
							.imageSubresource = VkImageSubresourceLayers({.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .mipLevel = 0, .baseArrayLayer = i, .layerCount = 1 }),
							.imageOffset = VkOffset3D({.x = 0, .y = 0, .z = 0 }),
							.imageExtent = Extent }));
					}
					const auto& CB = CommandBuffers[0];
					PopulateCommandBuffer_CopyBufferToImage(CB, Buffer, Images.back().Image, VK_ACCESS_SHADER_READ_BIT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, BICs, 1, Layers);
				
					SubmitAndWait(GraphicsQueue, CB);
				}
				vkFreeMemory(Device, DeviceMemory, GetAllocationCallbacks());
				vkDestroyBuffer(Device, Buffer, GetAllocationCallbacks());
			}

			ImageViews.emplace_back(VkImageView());
			const VkImageViewCreateInfo IVCI = {
				.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
				.pNext = nullptr,
				.flags = 0,
				.image = Images.back().Image,
				.viewType = 1 < Layers ? VK_IMAGE_VIEW_TYPE_2D_ARRAY : VK_IMAGE_VIEW_TYPE_2D,
				.format = VK_FORMAT_R32G32_SFLOAT,
				.components = VkComponentMapping({.r = VK_COMPONENT_SWIZZLE_R, .g = VK_COMPONENT_SWIZZLE_G, .b = VK_COMPONENT_SWIZZLE_B, .a = VK_COMPONENT_SWIZZLE_A }),
				.subresourceRange = VkImageSubresourceRange({.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .baseMipLevel = 0, .levelCount = VK_REMAINING_MIP_LEVELS, .baseArrayLayer = 0, .layerCount = VK_REMAINING_ARRAY_LAYERS }),
			};
			VERIFY_SUCCEEDED(vkCreateImageView(Device, &IVCI, GetAllocationCallbacks(), &ImageViews.back()));
		}
#else
		CreateTextureArray1x1({0xff0000ff, 0xff00ff00});
#endif
	}
	virtual void CreateImmutableSampler() override {
		//!< https://developer.leapmotion.com/documentation/v4/images.html
		//!< フィルタを LINEAR、ラップモードを CLAMP
		Samplers.emplace_back(VkSampler());
		const VkSamplerCreateInfo SCI = {
			.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
			.pNext = nullptr,
			.flags = 0,
			.magFilter = VK_FILTER_LINEAR, .minFilter = VK_FILTER_LINEAR, .mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
			.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, .addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, .addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
			.mipLodBias = 0.0f,
			.anisotropyEnable = VK_FALSE, .maxAnisotropy = 1.0f,
			.compareEnable = VK_FALSE, .compareOp = VK_COMPARE_OP_NEVER,
			.minLod = 0.0f, .maxLod = 1.0f,
			.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE,
			.unnormalizedCoordinates = VK_FALSE
		};
		VERIFY_SUCCEEDED(vkCreateSampler(Device, &SCI, GetAllocationCallbacks(), &Samplers.back()));
	}

	virtual void CreateDescriptorSetLayout() override {
		DescriptorSetLayouts.emplace_back(VkDescriptorSetLayout());
		assert(!empty(Samplers) && "");
		const std::array ISs = { Samplers[0] };
		VKExt::CreateDescriptorSetLayout(DescriptorSetLayouts.back(), 0, {
			VkDescriptorSetLayoutBinding({.binding = 0, .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, .descriptorCount = static_cast<uint32_t>(size(ISs)), .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT, .pImmutableSamplers = data(ISs) })
			});
	}
	virtual void CreatePipelineLayout() override {
		assert(!empty(DescriptorSetLayouts) && "");
		PipelineLayouts.emplace_back(VkPipelineLayout());
		VKExt::CreatePipelineLayout(PipelineLayouts.back(), { DescriptorSetLayouts[0] }, {});
	}
	virtual void CreateRenderPass() { VK::CreateRenderPass(VK_ATTACHMENT_LOAD_OP_DONT_CARE, false); }

	virtual void CreateShaderModules() override { CreateShaderModle_VsFs(); }

	virtual void CreatePipelines() override { CreatePipeline_VsFs(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP, 0, VK_FALSE); }

	virtual void CreateDescriptorPool() override {
		DescriptorPools.emplace_back(VkDescriptorPool());
		VKExt::CreateDescriptorPool(DescriptorPools.back(), 0, {
			VkDescriptorPoolSize({.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, .descriptorCount = 1 })
			});
	}
	virtual void AllocateDescriptorSet() override {
		const std::array DSLs = { DescriptorSetLayouts[0] };
		const VkDescriptorSetAllocateInfo DSAI = {
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
			.pNext = nullptr,
			.descriptorPool = DescriptorPools[0],
			.descriptorSetCount = static_cast<uint32_t>(size(DSLs)), .pSetLayouts = data(DSLs)
		};
		DescriptorSets.emplace_back(VkDescriptorSet());
		VERIFY_SUCCEEDED(vkAllocateDescriptorSets(Device, &DSAI, &DescriptorSets.back()));
	}
	virtual void CreateDescriptorUpdateTemplate() override {
		DescriptorUpdateTemplates.emplace_back(VkDescriptorUpdateTemplate());
		VK::CreateDescriptorUpdateTemplate(DescriptorUpdateTemplates.back(), {
			VkDescriptorUpdateTemplateEntry({
				.dstBinding = 0, .dstArrayElement = 0,
				.descriptorCount = _countof(DescriptorUpdateInfo::DII), .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
				.offset = offsetof(DescriptorUpdateInfo, DII), .stride = sizeof(DescriptorUpdateInfo)
			}),
			}, DescriptorSetLayouts[0]);
	}
	virtual void UpdateDescriptorSet() override {
		const DescriptorUpdateInfo DUI = {
			VkDescriptorImageInfo({.sampler = VK_NULL_HANDLE, .imageView = ImageViews[0], .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL }),
		};
		vkUpdateDescriptorSetWithTemplate(Device, DescriptorSets[0], DescriptorUpdateTemplates[0], &DUI);
	}

	virtual void PopulateCommandBuffer(const size_t i) override;

private:
	struct DescriptorUpdateInfo
	{
		VkDescriptorImageInfo DII[1];
	};
};
#pragma endregion