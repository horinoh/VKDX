#include "stdafx.h"

#include <fstream>

#include "VK.h"

#pragma comment(lib, "vulkan-1.lib")

#ifdef VK_NO_PROTOYYPES
#define VK_GLOBAL_PROC_ADDR(proc) PFN_vk ## proc VK::vk ## proc = VK_NULL_HANDLE;
#include "VKGlobalProcAddr.h"
#undef VK_GLOBAL_PROC_ADDR
#define VK_INSTANCE_PROC_ADDR(proc) PFN_vk ## proc VK::vk ## proc = VK_NULL_HANDLE;
#include "VKInstanceProcAddr.h"
#undef VK_INSTANCE_PROC_ADDR
#define VK_DEVICE_PROC_ADDR(proc) PFN_vk ## proc VK::vk ## proc = VK_NULL_HANDLE;
#include "VKDeviceProcAddr.h"
#undef VK_DEVICE_PROC_ADDR
#endif //!< VK_NO_PROTOYYPES

#ifdef _DEBUG
#define VK_INSTANCE_PROC_ADDR(proc) PFN_vk ## proc ## EXT VK::vk ## proc = VK_NULL_HANDLE;
#include "VKDebugReport.h"
#undef VK_INSTANCE_PROC_ADDR
#define VK_DEVICE_PROC_ADDR(proc) PFN_vk ## proc ## EXT VK::vk ## proc = VK_NULL_HANDLE;
#include "VKDebugMarker.h"
#undef VK_DEVICE_PROC_ADDR
#endif //!< _DEBUG

void VK::OnCreate(HWND hWnd, HINSTANCE hInstance, LPCWSTR Title)
{
#ifdef _DEBUG
	PerformanceCounter PC("OnCreate : ");
#endif

	Super::OnCreate(hWnd, hInstance, Title);

#ifdef DEBUG_STDOUT
	std::cout << "VK_HEADER_VERSION = " << VK_HEADER_VERSION << std::endl;
#endif
#ifdef VK_NO_PROTOYYPES
	LoadVulkanDLL();
#endif //!< VK_NO_PROTOYYPES

	CreateInstance();
	CreateSurface(hWnd, hInstance);
	GetPhysicalDevice();
	GetQueueFamily();
	CreateDevice();
	
	CreateFence();
	CreateSemaphore();

	CreateSwapchain();

	CreateDepthStencil();

	CreateVertexBuffer();
	CreateIndexBuffer();
	CreateIndirectBuffer();

	CreateTexture();

	CreateDescriptorSetLayout();
	{
		//!< #VK_TODO
		//CreateUniformBuffer();
	}
	CreateDescriptorSet();
	UpdateDescriptorSet();

	CreatePushConstantRanges();

	CreateRenderPass();
	CreateFramebuffer();

	CreatePipeline();
}

/**
@note 殆どのものを壊して作り直さないとダメ #VK_TODO
almost every thing must be recreated

LEARNING VULKAN : p367
need to destroy and recreate the framebuffer,
command pool, graphics pipeline, Render Pass, depth buffer image, image view, vertex
buffer, and so on.
*/
void VK::OnSize(HWND hWnd, HINSTANCE hInstance)
{
#ifdef _DEBUG
	PerformanceCounter PC("OnSize : ");
#endif

	Super::OnSize(hWnd, hInstance);

	//ResizeSwapChainToClientRect();

	CreateViewport(static_cast<float>(SurfaceExtent2D.width), static_cast<float>(SurfaceExtent2D.height));

	//DestroyFramebuffer();
	//CreateFramebuffer();

	for (auto i = 0; i < CommandBuffers.size(); ++i) {
		PopulateCommandBuffer(i);
	}
}
void VK::OnDestroy(HWND hWnd, HINSTANCE hInstance)
{
	Super::OnDestroy(hWnd, hInstance);

	if (VK_NULL_HANDLE != Device) {
		VERIFY_SUCCEEDED(vkDeviceWaitIdle(Device));
	}

	DestroyFramebuffer();

	if (VK_NULL_HANDLE != RenderPass) {
		vkDestroyRenderPass(Device, RenderPass, GetAllocationCallbacks());
		RenderPass = VK_NULL_HANDLE;
	}
	if (VK_NULL_HANDLE != Pipeline) {
		vkDestroyPipeline(Device, Pipeline, GetAllocationCallbacks());
		Pipeline = VK_NULL_HANDLE;
	}

	if (VK_NULL_HANDLE != PipelineLayout) {
		vkDestroyPipelineLayout(Device, PipelineLayout, GetAllocationCallbacks());
	}

	//!< VkDescriptorPoolCreateInfo に VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT を指定した場合「のみ」デスクリプタセットを個別に開放できる
#if 0
	if (!DescriptorSets.empty()) {
		vkFreeDescriptorSets(Device, DescriptorPool, static_cast<uint32_t>(DescriptorSets.size()), DescriptorSets.data());
	}
#endif
	DescriptorSets.clear();
	if (VK_NULL_HANDLE != DescriptorPool) {
		vkDestroyDescriptorPool(Device, DescriptorPool, GetAllocationCallbacks());
		DescriptorPool = VK_NULL_HANDLE;
	}
	for (auto i : DescriptorSetLayouts) {
		vkDestroyDescriptorSetLayout(Device, i, GetAllocationCallbacks());
	}
	DescriptorSetLayouts.clear();

	if (VK_NULL_HANDLE != UniformDeviceMemory) {
		vkFreeMemory(Device, UniformDeviceMemory, GetAllocationCallbacks());
		UniformDeviceMemory = VK_NULL_HANDLE;
	}
	if (VK_NULL_HANDLE != UniformBuffer) {
		vkDestroyBuffer(Device, UniformBuffer, GetAllocationCallbacks());
		UniformBuffer = VK_NULL_HANDLE;
	}

	if (VK_NULL_HANDLE != IndirectDeviceMemory) {
		vkFreeMemory(Device, IndirectDeviceMemory, GetAllocationCallbacks());
		IndirectDeviceMemory = VK_NULL_HANDLE;
	}
	if (VK_NULL_HANDLE != IndirectBuffer) {
		vkDestroyBuffer(Device, IndirectBuffer, GetAllocationCallbacks());
		IndirectBuffer = VK_NULL_HANDLE;
	}
	if (VK_NULL_HANDLE != IndexDeviceMemory) {
		vkFreeMemory(Device, IndexDeviceMemory, GetAllocationCallbacks());
		IndexDeviceMemory = VK_NULL_HANDLE;
	}
	if (VK_NULL_HANDLE != IndexBuffer) {
		vkDestroyBuffer(Device, IndexBuffer, GetAllocationCallbacks());
		IndexBuffer = VK_NULL_HANDLE;
	}
	if (VK_NULL_HANDLE != VertexDeviceMemory) {
		vkFreeMemory(Device, VertexDeviceMemory, GetAllocationCallbacks());
		VertexDeviceMemory = VK_NULL_HANDLE;
	}
	if (VK_NULL_HANDLE != VertexBuffer) {
		vkDestroyBuffer(Device, VertexBuffer, GetAllocationCallbacks());
		VertexBuffer = VK_NULL_HANDLE;
	}

	for (auto i : Samplers) {
		vkDestroySampler(Device, i, GetAllocationCallbacks());
	}

	if (VK_NULL_HANDLE != ImageView) {
		vkDestroyImageView(Device, ImageView, GetAllocationCallbacks());
		ImageView = VK_NULL_HANDLE;
	}
	if (VK_NULL_HANDLE != ImageDeviceMemory) {
		vkFreeMemory(Device, ImageDeviceMemory, GetAllocationCallbacks());
		ImageDeviceMemory = VK_NULL_HANDLE;
	}
	if (VK_NULL_HANDLE != Image) {
		vkDestroyImage(Device, Image, GetAllocationCallbacks());
		Image = VK_NULL_HANDLE;
	}

	if (VK_NULL_HANDLE != DepthStencilImageView) {
		vkDestroyImageView(Device, DepthStencilImageView, GetAllocationCallbacks());
		DepthStencilImageView = VK_NULL_HANDLE;
	}
	if (VK_NULL_HANDLE != DepthStencilDeviceMemory) {
		vkFreeMemory(Device, DepthStencilDeviceMemory, GetAllocationCallbacks());
		DepthStencilDeviceMemory = VK_NULL_HANDLE;
	}
	if (VK_NULL_HANDLE != DepthStencilImage) {
		vkDestroyImage(Device, DepthStencilImage, GetAllocationCallbacks());
		DepthStencilImage = VK_NULL_HANDLE;
	}

	for (auto i : SwapchainImageViews) {
		vkDestroyImageView(Device, i, GetAllocationCallbacks());
	}
	SwapchainImageViews.clear();
	//!< SwapchainImages は取得したもの、破棄しない
	if (VK_NULL_HANDLE != Swapchain) {
		vkDestroySwapchainKHR(Device, Swapchain, GetAllocationCallbacks());
		Swapchain = VK_NULL_HANDLE;
	}

	if (!CommandPools.empty()) {
		const auto CP = CommandPools[0]; //!< 現状は0番のコマンドプール決め打ち #VK_TODO 

		for (auto i : SecondaryCommandBuffers) {
			if (!i.empty()) {
				vkFreeCommandBuffers(Device, CP, static_cast<uint32_t>(i.size()), i.data());
				i.clear();
			}
		}
		SecondaryCommandBuffers.clear();

		if (!CommandBuffers.empty()) {
			vkFreeCommandBuffers(Device, CP, static_cast<uint32_t>(CommandBuffers.size()), CommandBuffers.data());
			CommandBuffers.clear();
		}
		std::for_each(CommandPools.begin(), CommandPools.end(), [&](const VkCommandPool rhs) { vkDestroyCommandPool(Device, rhs, GetAllocationCallbacks()); });
		CommandPools.clear();
	}
	
	if (VK_NULL_HANDLE != RenderFinishedSemaphore) {
		vkDestroySemaphore(Device, RenderFinishedSemaphore, GetAllocationCallbacks());
		RenderFinishedSemaphore = VK_NULL_HANDLE;
	}
	if (VK_NULL_HANDLE != NextImageAcquiredSemaphore) {
		vkDestroySemaphore(Device, NextImageAcquiredSemaphore, GetAllocationCallbacks());
		NextImageAcquiredSemaphore = VK_NULL_HANDLE;
	}
	if (VK_NULL_HANDLE != Fence) {
		vkDestroyFence(Device, Fence, GetAllocationCallbacks());
		Fence = VK_NULL_HANDLE;
	}

	//!< Queue は vkGetDeviceQueue() で取得したもの、破棄しない

	if (VK_NULL_HANDLE != Device) {
		vkDestroyDevice(Device, GetAllocationCallbacks());
		Device = VK_NULL_HANDLE;
	}
	
	//!< PhysicalDevice は vkEnumeratePhysicalDevices() で取得したもの、破棄しない

	if (VK_NULL_HANDLE != Surface) {
		vkDestroySurfaceKHR(Instance, Surface, GetAllocationCallbacks());
		Surface = VK_NULL_HANDLE;
	}

#ifdef _DEBUG
	if (VK_NULL_HANDLE != DebugReportCallback) {
		vkDestroyDebugReportCallback(Instance, DebugReportCallback, nullptr);
		DebugReportCallback = VK_NULL_HANDLE;
	}
#endif

	if (VK_NULL_HANDLE != Instance) {
		vkDestroyInstance(Instance, GetAllocationCallbacks());
		Instance = VK_NULL_HANDLE;
	}

#ifdef VK_NO_PROTOYYPES
	if (!FreeLibrary(VulkanDLL)) {
		assert(false && "FreeLibrary failed");
	}
#endif //!< VK_NO_PROTOYYPES
}

std::string VK::GetVkResultString(const VkResult Result)
{
#define VK_RESULT_ENTRY(vr) case VK_##vr: return #vr;
	switch (Result)
	{
	default: DEBUG_BREAK(); return "Not found";
#include "VkResult.h"
	}
#undef VK_RESULT_CASE
}
std::wstring VK::GetVkResultStringW(const VkResult Result)
{
	const auto ResultString = GetVkResultString(Result);
	return std::wstring(ResultString.begin(), ResultString.end());
}
std::string VK::GetFormatString(const VkFormat Format)
{
#define VK_FORMAT_ENTRY(vf) case VK_FORMAT_##vf: return #vf;
	switch (Format)
	{
	default: DEBUG_BREAK(); return "Not found";
#include "VKFormat.h"
	}
#undef VK_FORMAT_ENTRY
}
std::string VK::GetColorSpaceString(const VkColorSpaceKHR ColorSpace)
{
#define VK_COLOR_SPACE_ENTRY(vcs) case VK_COLOR_SPACE_##vcs: return #vcs;
	switch (ColorSpace)
	{
	default: DEBUG_BREAK(); return "Not found";
#include "VKColorSpace.h"
	}
#undef VK_COLOR_SPACE_ENTRY
}

std::string VK::GetImageViewTypeString(const VkImageViewType ImageViewType)
{
#define VK_IMAGE_VIEW_TYPE_ENTRY(vivt) case VK_IMAGE_VIEW_TYPE_##vivt: return #vivt;
	switch (ImageViewType)
	{
	default: DEBUG_BREAK(); return "Not found";
#include "VKImageViewType.h"
	}
#undef VK_IMAGE_VIEW_TYPE_ENTRY
}

std::string VK::GetComponentSwizzleString(const VkComponentSwizzle ComponentSwizzle)
{
#define VK_COMPONENT_SWIZZLE_ENTRY(vcs) case VK_COMPONENT_SWIZZLE_##vcs: return #vcs;
	switch (ComponentSwizzle)
	{
	default: DEBUG_BREAK(); return "Not found";
#include "VKComponentSwizzle.h"
	}
#undef VK_COMPONENT_SWIZZLE_ENTRY
}

bool VK::HasExtension(const VkPhysicalDevice PhysicalDevice, const char* ExtensionName)
{
	//!< エクステンションは見つかるのに、使用できないので封印... Extension is found, but not available...
#if 1
	//!< VK_EXT_debug_marker not available for devices associated with ICD nvoglv64.dll
	//!< https://devtalk.nvidia.com/default/topic/1001794/vulkan-vk_ext_debug_marker-missing-after-new-5-2-build-update-/
	return false;
#else
	uint32_t DeviceLayerPropertyCount = 0;
	VERIFY_SUCCEEDED(vkEnumerateDeviceLayerProperties(PhysicalDevice, &DeviceLayerPropertyCount, nullptr));
	if (DeviceLayerPropertyCount) {
		std::vector<VkLayerProperties> LayerProperties(DeviceLayerPropertyCount);
		VERIFY_SUCCEEDED(vkEnumerateDeviceLayerProperties(PhysicalDevice, &DeviceLayerPropertyCount, LayerProperties.data()));
		for (const auto& i : LayerProperties) {
			uint32_t DeviceExtensionPropertyCount = 0;
			VERIFY_SUCCEEDED(vkEnumerateDeviceExtensionProperties(PhysicalDevice, nullptr, &DeviceExtensionPropertyCount, nullptr));
			if (DeviceExtensionPropertyCount) {
				std::vector<VkExtensionProperties> ExtensionProperties(DeviceExtensionPropertyCount);
				VERIFY_SUCCEEDED(vkEnumerateDeviceExtensionProperties(PhysicalDevice, i.layerName, &DeviceExtensionPropertyCount, ExtensionProperties.data()));
				for (const auto& j : ExtensionProperties) {
					if (!strcmp(ExtensionName, j.extensionName)) {
						return true;
					}
				}
			}
		}
	}
	return false;
#endif
}

VkFormat VK::GetSupportedDepthFormat(VkPhysicalDevice PhysicalDevice)
{
	const std::vector<VkFormat> DepthFormats = {
		VK_FORMAT_D32_SFLOAT_S8_UINT,
		VK_FORMAT_D32_SFLOAT,
		VK_FORMAT_D24_UNORM_S8_UINT,
		VK_FORMAT_D16_UNORM_S8_UINT,
		VK_FORMAT_D16_UNORM
	};
	for (auto& i : DepthFormats) {
		VkFormatProperties FormatProperties;
		vkGetPhysicalDeviceFormatProperties(PhysicalDevice, i, &FormatProperties);
		if (FormatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) {
			return i;
		}
	}
	DEBUG_BREAK(); //!< DepthFormat not found
	return VK_FORMAT_UNDEFINED;
}
uint32_t VK::GetMemoryType(const VkPhysicalDeviceMemoryProperties& PhysicalDeviceMemoryProperties, const uint32_t MemoryTypeBits, const VkFlags Properties)
{
	for (auto i = 0; i < 32; ++i) {
		if (MemoryTypeBits & (1 << i)) {
			if ((PhysicalDeviceMemoryProperties.memoryTypes[i].propertyFlags & Properties) == Properties) {
				return i;
			}
		}
	}
	DEBUG_BREAK();
	return 0;
}

//!< 遷移する前に OldImageLayout が完了していなければならないアクション
//VkAccessFlags VK::GetSrcAccessMask(VkImageLayout OldImageLayout, VkImageLayout NewImageLayout)
//{
//	VkAccessFlags AccessMask = 0;
//
//	switch (OldImageLayout)
//	{
//	case VK_IMAGE_LAYOUT_UNDEFINED:
//		AccessMask = 0;
//		break;
//	case VK_IMAGE_LAYOUT_PREINITIALIZED:
//		//!< ホストによる書き込みが完了していなければならない
//		AccessMask = VK_ACCESS_HOST_WRITE_BIT; 
//		break;
//	case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
//		//!< カラーバッファ書き込みが完了していなければならない
//		AccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
//		break;
//	case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
//		//!< デプスステンシルバッファ書き込みが完了していなければならない
//		AccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
//		break;
//	case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
//		//!< イメージ読み込みが完了していなければならない
//		AccessMask = VK_ACCESS_TRANSFER_READ_BIT;
//		break;
//	case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
//		//!< イメージへの書き込みが完了していなければならない
//		AccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
//		break;
//	case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
//		//!< シェーダによるイメージ読み込みが完了していなければならない
//		AccessMask = VK_ACCESS_SHADER_READ_BIT;
//		break;
//	}
//	switch (NewImageLayout)
//	{
//	case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
//		AccessMask |= VK_ACCESS_TRANSFER_READ_BIT;
//		break;
//	case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
//		AccessMask = VK_ACCESS_TRANSFER_READ_BIT;
//		break;
//	case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
//		if (0 == AccessMask){
//			AccessMask = VK_ACCESS_HOST_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT;
//		}
//		break;
//	}
//	return AccessMask;
//}
//VkAccessFlags VK::GetDstAccessMask(VkImageLayout OldImageLayout, VkImageLayout NewImageLayout)
//{
//	VkAccessFlags AccessMask = 0;
//
//	switch (NewImageLayout)
//	{
//	case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
//		AccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
//		break;
//	case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
//		AccessMask = VK_ACCESS_TRANSFER_READ_BIT;
//		break;
//	case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
//		AccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
//		break;
//	case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
//		AccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
//		break;
//	case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
//		AccessMask = VK_ACCESS_SHADER_READ_BIT;
//		break;
//	}
//	return AccessMask;
//}
//void VK::SetImageLayout(VkCommandBuffer CommandBuffer, VkImage Image, VkImageLayout OldImageLayout, VkImageLayout NewImageLayout, VkImageSubresourceRange ImageSubresourceRange) const
//{
//	const auto SrcAccessMask = GetSrcAccessMask(OldImageLayout, NewImageLayout);
//	const auto DstAccessMask = GetDstAccessMask(OldImageLayout, NewImageLayout);
//
//	const VkImageMemoryBarrier ImageMemoryBarrier = {
//		VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
//		nullptr,
//		SrcAccessMask,
//		DstAccessMask,
//		OldImageLayout,
//		NewImageLayout,
//		VK_QUEUE_FAMILY_IGNORED,
//		VK_QUEUE_FAMILY_IGNORED,
//		Image,
//		ImageSubresourceRange
//	};
//	vkCmdPipelineBarrier(CommandBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0, 0, nullptr, 0, nullptr, 1, &ImageMemoryBarrier);
//}

void VK::CreateBuffer(VkBuffer* Buffer, const VkBufferUsageFlags Usage, const size_t Size) const
{
	const VkBufferCreateInfo BufferCreateInfo = {
		VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
		nullptr,
		0,
		Size,
		Usage,
		VK_SHARING_MODE_EXCLUSIVE,
		0, nullptr
	};
	VERIFY_SUCCEEDED(vkCreateBuffer(Device, &BufferCreateInfo, GetAllocationCallbacks(), Buffer));
}

void VK::CreateImage(VkImage* Image, const VkImageUsageFlags Usage, const VkSampleCountFlagBits SampleCount, const VkImageType ImageType, const VkFormat Format, const VkExtent3D& Extent3D, const uint32_t MipLevels, const uint32_t ArrayLayers) const
{
	ValidateFormatProperties(Usage, Format);

	const VkImageCreateInfo ImageCreateInfo = {
		VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
		nullptr,
		0,
		ImageType,
		Format,
		Extent3D,
		MipLevels,
		ArrayLayers,
		SampleCount, //!< キューブマップの場合は VK_SAMPLE_COUNT_1_BIT しか使えない
		VK_IMAGE_TILING_OPTIMAL,
		Usage,
		VK_SHARING_MODE_EXCLUSIVE,
		0, nullptr,
		VK_IMAGE_LAYOUT_UNDEFINED
	};
	VERIFY_SUCCEEDED(vkCreateImage(Device, &ImageCreateInfo, GetAllocationCallbacks(), Image));
}

void VK::CopyToHostVisibleMemory(const VkDeviceMemory DeviceMemory, const size_t Size, const void* Source, const VkDeviceSize Offset)
{
	if (Size && nullptr != Source) {
		void *Data;
		VERIFY_SUCCEEDED(vkMapMemory(Device, DeviceMemory, Offset, Size, static_cast<VkMemoryMapFlags>(0), &Data)); {
			memcpy(Data, Source, Size);

			const std::vector<VkMappedMemoryRange> MappedMemoryRanges = {
				{
					VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE,
					nullptr,
					DeviceMemory,
					0,
					VK_WHOLE_SIZE
				}
			};
			//!< ↓ホストから可視にするために無効化する (無くても動くけど必要？)
			//VERIFY_SUCCEEDED(vkInvalidateMappedMemoryRanges(Device, static_cast<uint32_t>(MappedMemoryRanges.size()), MappedMemoryRanges.data()));

			//!< フラッシュしておかないと、サブミットされた他のコマンドからはすぐには見えない (VK_MEMORY_PROPERTY_HOST_COHERENT_BIT の場合は必要なし)
			VERIFY_SUCCEEDED(vkFlushMappedMemoryRanges(Device, static_cast<uint32_t>(MappedMemoryRanges.size()), MappedMemoryRanges.data()));
		} vkUnmapMemory(Device, DeviceMemory);
	}
}
void VK::SubmitCopyBuffer(const VkCommandBuffer CommandBuffer, const VkBuffer SrcBuffer, const VkBuffer DstBuffer, const VkAccessFlags AccessFlag, const VkPipelineStageFlagBits PipelineStageFlag, const size_t Size)
{
	VERIFY_SUCCEEDED(vkBeginCommandBuffer(CommandBuffer, &CommandBufferBeginInfo_OneTime)); {
		const VkBufferCopy BufferCopy = {
			0,
			0,
			Size
		};
		vkCmdCopyBuffer(CommandBuffer, SrcBuffer, DstBuffer, 1, &BufferCopy);

		const VkBufferMemoryBarrier BufferMemoryBarrier = {
			VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
			nullptr,
			VK_ACCESS_MEMORY_WRITE_BIT,
			AccessFlag,
			VK_QUEUE_FAMILY_IGNORED,
			VK_QUEUE_FAMILY_IGNORED,
			DstBuffer,
			0,
			VK_WHOLE_SIZE
		};
		//!< バッファを「転送先(VK_PIPELINE_STAGE_TRANSFER_BIT)」から「目的のバッファ(PipelineStageFlag(バーテックスバッファ等))」へ変更する
		vkCmdPipelineBarrier(CommandBuffer, 
			VK_PIPELINE_STAGE_TRANSFER_BIT, PipelineStageFlag,
			0, 
			0, nullptr,
			1, &BufferMemoryBarrier, 
			0, nullptr);

	} VERIFY_SUCCEEDED(vkEndCommandBuffer(CommandBuffer));

	//!< サブミット
	const std::vector<VkSubmitInfo> SubmitInfos = {
		{
			VK_STRUCTURE_TYPE_SUBMIT_INFO,
			nullptr,
			0, nullptr,
			nullptr,
			1, &CommandBuffer,
			0, nullptr
		}
	};
	VERIFY_SUCCEEDED(vkQueueSubmit(GraphicsQueue, static_cast<uint32_t>(SubmitInfos.size()), SubmitInfos.data(), VK_NULL_HANDLE));
	VERIFY_SUCCEEDED(vkQueueWaitIdle(GraphicsQueue));
}

//!< Cubemapを作る場合、まずレイヤ化されたイメージを作成し、(イメージビューを用いて)レイヤをフェイスとして扱うようハードウエアに伝える
void VK::CreateImageView(VkImageView* ImageView, const VkImage Image, const VkImageViewType ImageViewType, const VkFormat Format, const VkComponentMapping& ComponentMapping, const VkImageSubresourceRange& ImageSubresourceRange)
{
	const VkImageViewCreateInfo ImageViewCreateInfo = {
		VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
		nullptr,
		0,
		Image,
		ImageViewType,
		Format,
		ComponentMapping,
		ImageSubresourceRange
	};
	VERIFY_SUCCEEDED(vkCreateImageView(Device, &ImageViewCreateInfo, GetAllocationCallbacks(), ImageView));

#ifdef DEBUG_STDOUT
	std::cout << "\t" << "\t" << "ImageViewType = " << GetImageViewTypeString(ImageViewType) << std::endl;
	std::cout << "\t" << "\t" << "Format = " << GetFormatString(Format) << std::endl;
	std::cout << "\t" << "\t" << "ComponentMapping = (" << GetComponentMappingString(ComponentMapping) << ")" << std::endl;
#endif

#ifdef DEBUG_STDOUT
	std::cout << "\t" << "CreateImageView" << COUT_OK << std::endl << std::endl;
#endif
}

void VK::CreateBufferView(VkBufferView* BufferView, const VkBuffer Buffer, const VkFormat Format, const VkDeviceSize Offset, const VkDeviceSize Range)
{
	const VkBufferViewCreateInfo BufferViewCreateInfo = {
		VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO,
		nullptr,
		0,
		Buffer,
		Format,
		Offset,
		Range
	};
	VERIFY_SUCCEEDED(vkCreateBufferView(Device, &BufferViewCreateInfo, GetAllocationCallbacks(), BufferView));
}

void VK::ValidateFormatProperties(const VkImageUsageFlags Usage, const VkFormat Format) const
{
#ifdef _DEBUG
	VkFormatProperties FormatProperties;
	vkGetPhysicalDeviceFormatProperties(PhysicalDevice, Format, &FormatProperties);

	//!< サンプルドイメージでは全てのフォーマットがサポートされてるわけではない Not all formats are supported for sampled images
	if (Usage & VK_IMAGE_USAGE_SAMPLED_BIT) {
		if (!(FormatProperties.optimalTilingFeatures &  VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT)) {
			std::cout << Yellow << "VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT not supported" << White << std::endl;
			DEBUG_BREAK();
		}
		//!< #VK_TODO リニア使用時のみチェックする
		const auto bUseLiner = true;//!< VK_FILTER_LINEAR == VkSamplerCreateInfo.magFilter || VK_FILTER_LINEAR == VkSamplerCreateInfo.minFilter || VK_SAMPLER_MIPMAP_MODE_LINEAR == VkSamplerCreateInfo.mipmapMode;
		if (bUseLiner) {
			if (!(FormatProperties.linearTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT)) {
				std::cout << Red << "VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT not supported" << White << std::endl;
				DEBUG_BREAK();
			}
		}
	}

	if (Usage & VK_IMAGE_USAGE_STORAGE_BIT) {
		if (!(FormatProperties.optimalTilingFeatures &  VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT)) {
			std::cout << Red << "VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT not supported" << White << std::endl;
			DEBUG_BREAK();
		}
		//!< #VK_TODO アトミック使用時のみチェックする
		const auto bUseAtomic = false; 
		if (bUseAtomic) {
			if (!(FormatProperties.linearTilingFeatures & VK_FORMAT_FEATURE_STORAGE_IMAGE_ATOMIC_BIT)) {
				std::cout << Red << "VK_FORMAT_FEATURE_STORAGE_IMAGE_ATOMIC_BIT not supported" << White << std::endl;
				DEBUG_BREAK();
			}
		}
	}

	if (Usage & VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT) {
		if (true) { //!< #VK_TODO
			//!< カラーの場合 In case color
			if (!(FormatProperties.optimalTilingFeatures &  VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT)) {
				std::cout << Red << "VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT not supported" << White << std::endl;
				DEBUG_BREAK();
			}
		}
		else {
			//!< デプスステンシルの場合 In case depth stencil
			if (!(FormatProperties.optimalTilingFeatures &  VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)) {
				std::cout << Red << "VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT not supported" << White << std::endl;
				DEBUG_BREAK();
			}
		}
	}

	if (Usage & VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT) {
		if (!(FormatProperties.bufferFeatures &  VK_FORMAT_FEATURE_UNIFORM_TEXEL_BUFFER_BIT)) {
			std::cout << Red << "VK_FORMAT_FEATURE_UNIFORM_TEXEL_BUFFER_BIT not supported" << White << std::endl;
			DEBUG_BREAK();
		}
	}

	if (Usage & VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT) {
		if (!(FormatProperties.bufferFeatures &  VK_FORMAT_FEATURE_STORAGE_TEXEL_BUFFER_BIT)) {
			std::cout << Red << "VK_FORMAT_FEATURE_STORAGE_TEXEL_BUFFER_BIT not supported" << White << std::endl;
			DEBUG_BREAK();
		}
		//!< #VK_TODO アトミック使用時のみチェックする
		const auto bUseAtomic = false;
		if (bUseAtomic) {
			if (!(FormatProperties.linearTilingFeatures & VK_FORMAT_FEATURE_STORAGE_TEXEL_BUFFER_ATOMIC_BIT)) {
				std::cout << Red << "VK_FORMAT_FEATURE_STORAGE_TEXEL_BUFFER_ATOMIC_BIT not supported" << White << std::endl;
				DEBUG_BREAK();
			}
		}
	}
#endif
}

void VK::EnumerateInstanceLayer()
{
	uint32_t InstanceLayerPropertyCount = 0;
	VERIFY_SUCCEEDED(vkEnumerateInstanceLayerProperties(&InstanceLayerPropertyCount, nullptr));
	if (InstanceLayerPropertyCount) {
		std::vector<VkLayerProperties> LayerProperties(InstanceLayerPropertyCount);
		VERIFY_SUCCEEDED(vkEnumerateInstanceLayerProperties(&InstanceLayerPropertyCount, LayerProperties.data()));
		for (const auto& i : LayerProperties) {
#ifdef DEBUG_STDOUT
			if (strlen(i.layerName)) {
				std::cout << "\t" << "\"" << i.layerName << "\"" << std::endl;
			}
#endif
			EnumerateInstanceExtenstion(i.layerName);
		}
	}
}
void VK::EnumerateInstanceExtenstion(const char* layerName)
{
	uint32_t InstanceExtensionPropertyCount = 0;
	VERIFY_SUCCEEDED(vkEnumerateInstanceExtensionProperties(nullptr, &InstanceExtensionPropertyCount, nullptr));
	if (InstanceExtensionPropertyCount) {
		std::vector<VkExtensionProperties> ExtensionProperties(InstanceExtensionPropertyCount);
		VERIFY_SUCCEEDED(vkEnumerateInstanceExtensionProperties(layerName, &InstanceExtensionPropertyCount, ExtensionProperties.data()));
		for (const auto& i : ExtensionProperties) {
#ifdef DEBUG_STDOUT
			if (strlen(i.extensionName)) {
				std::cout << "\t" << "\t" << "\"" << i.extensionName << "\"" << std::endl;
			}
#endif
		}
	}
}

#ifdef VK_NO_PROTOYYPES
void VK::LoadVulkanDLL()
{
	VulkanDLL = LoadLibraryA("vulkan-1.dll");
	//VulkanDLL = dlopen("libvulkan.so.1", RTLD_NOW);
	assert(nullptr != VulkanDLL && "LoadLibrary failed");
#define VK_GLOBAL_PROC_ADDR(proc) vk ## proc = reinterpret_cast<PFN_vk ## proc>(vkGetInstanceProcAddr(nullptr, "vk" #proc)); assert(nullptr != vk ## proc && #proc);
#include "VKGlobalProcAddr.h"
#undef VK_GLOBAL_PROC_ADDR
}
#endif //!< VK_NO_PROTOYYPES

void VK::CreateInstance()
{
	EnumerateInstanceLayer();

	const auto ApplicationName = GetTitle();
	const VkApplicationInfo ApplicationInfo = {
		VK_STRUCTURE_TYPE_APPLICATION_INFO,
		nullptr,
		ApplicationName.data(), 0,
		"VKDX Engine Name", 0,
		VK_API_VERSION_1_0
	};
	const std::vector<const char*> EnabledLayerNames = {
#ifdef _DEBUG
		//!< ↓標準的なバリデーションレイヤセットを最適な順序でロードする指定
		//!< (プログラムからやらない場合は環境変数 VK_INSTANCE_LAYERS へセットしておいてもよい)
		"VK_LAYER_LUNARG_standard_validation", 
		"VK_LAYER_LUNARG_object_tracker",
		//!< API 呼び出しとパラメータをコンソール出力する (出力がうるさいのでここでは指定しない)
		//"VK_LAYER_LUNARG_api_dump",
#endif
	};
	const std::vector<const char*> EnabledExtensions = {
		VK_KHR_SURFACE_EXTENSION_NAME,
#ifdef VK_USE_PLATFORM_WIN32_KHR
		VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
#endif
#ifdef _DEBUG
		//!< ↓デバッグレポート用
		VK_EXT_DEBUG_REPORT_EXTENSION_NAME
#endif
	};
	const VkInstanceCreateInfo InstanceCreateInfo = {
		VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
		nullptr,
		0,
		&ApplicationInfo,
		static_cast<uint32_t>(EnabledLayerNames.size()), EnabledLayerNames.data(),
		static_cast<uint32_t>(EnabledExtensions.size()), EnabledExtensions.data()
	};

	VERIFY_SUCCEEDED(vkCreateInstance(&InstanceCreateInfo, GetAllocationCallbacks(), &Instance));

#ifdef VK_NO_PROTOYYPES
#define VK_INSTANCE_PROC_ADDR(proc) vk ## proc = reinterpret_cast<PFN_vk ## proc>(vkGetInstanceProcAddr(Instance, "vk" #proc)); assert(nullptr != vk ## proc && #proc);
#include "VKInstanceProcAddr.h"
#undef VK_INSTANCE_PROC_ADDR
#endif //!< VK_NO_PROTOYYPES

#ifdef _DEBUG
	CreateDebugReportCallback();
#endif

#ifdef DEBUG_STDOUT
	std::cout << "CreateInstace" << COUT_OK << std::endl << std::endl;
#endif
}

#ifdef _DEBUG
void VK::CreateDebugReportCallback()
{
#define VK_INSTANCE_PROC_ADDR(proc) vk ## proc = reinterpret_cast<PFN_vk ## proc ## EXT>(vkGetInstanceProcAddr(Instance, "vk" #proc "EXT")); assert(nullptr != vk ## proc && #proc);
#include "VKDebugReport.h"
#undef VK_INSTANCE_PROC_ADDR

	auto Callback = [](VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objectType, uint64_t object, size_t location, int32_t messageCode, const char* pLayerPrefix, const char* pMessage, void* pUserData) -> VkBool32
	{
		using namespace std;
		if (VK_DEBUG_REPORT_ERROR_BIT_EXT & flags) {
			DEBUG_BREAK();
			cout << Red << "[ DebugReport ] : " << pMessage << White << endl;
			return VK_TRUE;
		}
		else if (VK_DEBUG_REPORT_WARNING_BIT_EXT & flags) {
			DEBUG_BREAK();
			cout << Yellow << "[ DebugReport ] : " << pMessage << White << endl;
			return VK_TRUE;
		}
		else if (VK_DEBUG_REPORT_INFORMATION_BIT_EXT & flags) {
			//cout << Green << "[ DebugReport ] : " << pMessage << White << endl;
		}
		else if (VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT & flags) {
			//DEBUG_BREAK();
			cout << Yellow << "[ DebugReport ] : " << pMessage << White << endl;
			return VK_TRUE;
		}
		else if (VK_DEBUG_REPORT_DEBUG_BIT_EXT & flags) {
			//cout << Green << "[ DebugReport ] : " << pMessage << White << endl;
		}
		return VK_FALSE;
	};
	const auto Flags = VK_DEBUG_REPORT_INFORMATION_BIT_EXT
		| VK_DEBUG_REPORT_WARNING_BIT_EXT
		| VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT
		| VK_DEBUG_REPORT_ERROR_BIT_EXT
		| VK_DEBUG_REPORT_DEBUG_BIT_EXT;
	CreateDebugReportCallback(Instance, Callback, Flags, &DebugReportCallback);
}
#endif //!< _DEBUG

#ifdef _DEBUG
void VK::MarkerInsert(VkCommandBuffer CommandBuffer, const char* Name, const glm::vec4& Color)
{
	if (VK_NULL_HANDLE != vkCmdDebugMarkerInsert) {
		VkDebugMarkerMarkerInfoEXT DebugMarkerMarkerInfo = {
			VK_STRUCTURE_TYPE_DEBUG_MARKER_MARKER_INFO_EXT,
			nullptr,
			Name,
		};
		memcpy(DebugMarkerMarkerInfo.color, &Color, sizeof(DebugMarkerMarkerInfo.color));
		vkCmdDebugMarkerInsert(CommandBuffer, &DebugMarkerMarkerInfo);
	}
}
void VK::MarkerBegin(VkCommandBuffer CommandBuffer, const char* Name, const glm::vec4& Color)
{
	if (VK_NULL_HANDLE != vkCmdDebugMarkerBegin) {
		VkDebugMarkerMarkerInfoEXT DebugMarkerMarkerInfo = {
			VK_STRUCTURE_TYPE_DEBUG_MARKER_MARKER_INFO_EXT,
			nullptr,
			Name,
		};
		memcpy(DebugMarkerMarkerInfo.color, &Color, sizeof(DebugMarkerMarkerInfo.color));
		vkCmdDebugMarkerBegin(CommandBuffer, &DebugMarkerMarkerInfo);
	}
}
void VK::MarkerEnd(VkCommandBuffer CommandBuffer)
{
	if (VK_NULL_HANDLE != vkCmdDebugMarkerEnd) {
		vkCmdDebugMarkerEnd(CommandBuffer);
	}
}
#endif //!< _DEBUG

void VK::CreateSurface(HWND hWnd, HINSTANCE hInstance)
{
#ifdef VK_USE_PLATFORM_WIN32_KHR
	const VkWin32SurfaceCreateInfoKHR SurfaceCreateInfo = {
		VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
		nullptr,
		0,
		hInstance,
		hWnd
	};
	VERIFY_SUCCEEDED(vkCreateWin32SurfaceKHR(Instance, &SurfaceCreateInfo, GetAllocationCallbacks(), &Surface));
#else
	assert(false && "Not supported");
#endif

#ifdef DEBUG_STDOUT
	std::cout << "\t" << "Surface" << std::endl;
#endif
}
void VK::EnumeratePhysicalDeviceMemoryProperties(const VkPhysicalDeviceMemoryProperties& PhysicalDeviceMemoryProperties)
{
#ifdef DEBUG_STDOUT
	std::cout << "\t" << "\t" << "\t" << "MemoryType" << std::endl;
	for (uint32_t i = 0; i < PhysicalDeviceMemoryProperties.memoryTypeCount; ++i) {
		std::cout << "\t" << "\t" << "\t" << "\t";
		std::cout << "[" << i << "] ";
		std::cout << "HeapIndex = " << PhysicalDeviceMemoryProperties.memoryTypes[i].heapIndex;
		if (PhysicalDeviceMemoryProperties.memoryTypes[i].propertyFlags) {
			std::cout << ", PropertyFlags = ";
#define MEMORY_PROPERTY_ENTRY(entry) if(VK_MEMORY_PROPERTY_##entry##_BIT & PhysicalDeviceMemoryProperties.memoryTypes[i].propertyFlags) { std::cout << #entry << " | "; }
			MEMORY_PROPERTY_ENTRY(DEVICE_LOCAL);
			MEMORY_PROPERTY_ENTRY(HOST_VISIBLE);
			MEMORY_PROPERTY_ENTRY(HOST_COHERENT);
			MEMORY_PROPERTY_ENTRY(HOST_CACHED);
			MEMORY_PROPERTY_ENTRY(LAZILY_ALLOCATED);
#undef MEMORY_PROPERTY_ENTRY
		}
		std::cout << std::endl;
	}
	std::cout << "\t" << "\t" << "\t" << "MemoryHeap" << std::endl;
	for (uint32_t i = 0; i < PhysicalDeviceMemoryProperties.memoryHeapCount; ++i) {
		std::cout << "\t" << "\t" << "\t" << "\t";
		std::cout << "[" << i << "] ";
		std::cout << "Size = " << PhysicalDeviceMemoryProperties.memoryHeaps[i].size;
		if (PhysicalDeviceMemoryProperties.memoryHeaps[i].flags) {
			std::cout << ", Flags = ";
			if (VK_MEMORY_HEAP_DEVICE_LOCAL_BIT & PhysicalDeviceMemoryProperties.memoryHeaps[i].flags) { std::cout << "DEVICE_LOCAL" << " | "; }
			if (VK_MEMORY_HEAP_MULTI_INSTANCE_BIT_KHX & PhysicalDeviceMemoryProperties.memoryHeaps[i].flags) { std::cout << "MULTI_INSTANCE" << " | "; }
		}
		std::cout << std::endl;
	}
#endif
}
void VK::GetPhysicalDevice()
{
	//!< 物理デバイス(GPU)の列挙
	uint32_t PhysicalDeviceCount = 0;
	VERIFY_SUCCEEDED(vkEnumeratePhysicalDevices(Instance, &PhysicalDeviceCount, nullptr));
	if (!PhysicalDeviceCount) { DEBUG_BREAK(); } //!< PhysicalDevice not found
	std::vector<VkPhysicalDevice> PhysicalDevices(PhysicalDeviceCount);
	VERIFY_SUCCEEDED(vkEnumeratePhysicalDevices(Instance, &PhysicalDeviceCount, PhysicalDevices.data()));

#ifdef DEBUG_STDOUT
	std::cout << "\t" << "PhysicalDevices" << std::endl;
#define PHYSICAL_DEVICE_TYPE_ENTRY(entry) if(VK_PHYSICAL_DEVICE_TYPE_##entry == PhysicalDeviceProperties.deviceType) { std::cout << #entry; }
#define PROPERTY_LIMITS_ENTRY(entry) std::cout << "\t" << "\t" << "\t" << "\t" << #entry << " = " << PhysicalDeviceProperties.limits.##entry << std::endl
#define DEVICE_FEATURE_ENTRY(entry) if (PhysicalDeviceFeatures.##entry) { std::cout << "\t" << "\t" << "\t" << "\t" << #entry << std::endl; }
	for (const auto& i : PhysicalDevices) {
		VkPhysicalDeviceProperties PhysicalDeviceProperties;
		vkGetPhysicalDeviceProperties(i, &PhysicalDeviceProperties);
		std::cout << "\t" << "\t" << PhysicalDeviceProperties.deviceName << ", DeviceType = ";
		PHYSICAL_DEVICE_TYPE_ENTRY(OTHER);
		PHYSICAL_DEVICE_TYPE_ENTRY(INTEGRATED_GPU);
		PHYSICAL_DEVICE_TYPE_ENTRY(DISCRETE_GPU);
		PHYSICAL_DEVICE_TYPE_ENTRY(VIRTUAL_GPU);
		PHYSICAL_DEVICE_TYPE_ENTRY(CPU);
		std::cout << std::endl;
		{
			std::cout << "\t" << "\t" << "\t" << "PhysicalDeviceProperties.PhysicalDeviceLimits" << std::endl;
			PROPERTY_LIMITS_ENTRY(maxUniformBufferRange);
			//PROPERTY_LIMITS_ENTRY(maxStorageBufferRange);
			PROPERTY_LIMITS_ENTRY(maxPushConstantsSize);
			PROPERTY_LIMITS_ENTRY(maxFragmentOutputAttachments);
			PROPERTY_LIMITS_ENTRY(maxColorAttachments);			
		}

		VkPhysicalDeviceFeatures PhysicalDeviceFeatures;
		vkGetPhysicalDeviceFeatures(i, &PhysicalDeviceFeatures);
		std::cout << "\t" << "\t" << "\t" << "PhysicalDeviceFeatures" << std::endl;
		DEVICE_FEATURE_ENTRY(textureCompressionBC);
		DEVICE_FEATURE_ENTRY(textureCompressionETC2);
		DEVICE_FEATURE_ENTRY(textureCompressionASTC_LDR);
		DEVICE_FEATURE_ENTRY(fillModeNonSolid);
		DEVICE_FEATURE_ENTRY(tessellationShader);

		VkPhysicalDeviceMemoryProperties PDMP;
		vkGetPhysicalDeviceMemoryProperties(i, &PDMP);
		EnumeratePhysicalDeviceMemoryProperties(PDMP);
		EnumerateDeviceLayer(i);

		std::cout << std::endl;
	}
#undef PHYSICAL_DEVICE_TYPE_ENTRY
#undef PROPERTY_LIMITS_ENTRY
#undef DEVICE_FEATURE_ENTRY
#endif

	//!< ここでは最初の物理デバイスを選択することにする #VK_TODO
	PhysicalDevice = PhysicalDevices[0];
	//!< 選択した物理デバイスのメモリプロパティを取得 (よく使うので覚えておく)
	vkGetPhysicalDeviceMemoryProperties(PhysicalDevice, &PhysicalDeviceMemoryProperties);
}
#ifdef DEBUG_STDOUT
void VK::EnumerateDeviceLayer(VkPhysicalDevice PhysicalDevice)
{
	uint32_t DeviceLayerPropertyCount = 0;
	VERIFY_SUCCEEDED(vkEnumerateDeviceLayerProperties(PhysicalDevice, &DeviceLayerPropertyCount, nullptr));
	if (DeviceLayerPropertyCount) {
		std::vector<VkLayerProperties> LayerProperties(DeviceLayerPropertyCount);
		VERIFY_SUCCEEDED(vkEnumerateDeviceLayerProperties(PhysicalDevice, &DeviceLayerPropertyCount, LayerProperties.data()));
		for (const auto& i : LayerProperties) {
			if (strlen(i.layerName)) {
				std::cout << "\t" << "\t" << "\t" << "\"" << i.layerName << "\"" << std::endl;
			}
			EnumerateDeviceExtenstion(PhysicalDevice, i.layerName);
		}
	}
}
void VK::EnumerateDeviceExtenstion(VkPhysicalDevice PhysicalDevice, const char* layerName)
{
	uint32_t DeviceExtensionPropertyCount = 0;
	VERIFY_SUCCEEDED(vkEnumerateDeviceExtensionProperties(PhysicalDevice, nullptr, &DeviceExtensionPropertyCount, nullptr));
	if (DeviceExtensionPropertyCount) {
		std::vector<VkExtensionProperties> ExtensionProperties(DeviceExtensionPropertyCount);
		VERIFY_SUCCEEDED(vkEnumerateDeviceExtensionProperties(PhysicalDevice, layerName, &DeviceExtensionPropertyCount, ExtensionProperties.data()));
		for (const auto& i : ExtensionProperties) {
			if (!strcmp(VK_EXT_DEBUG_MARKER_EXTENSION_NAME, i.extensionName)) { std::cout << Yellow; }
			if (strlen(i.extensionName)) {
				std::cout << "\t" << "\t" << "\t" << "\t" << "\"" << i.extensionName << "\"" << std::endl;
			}
			std::cout << White;
		}
	}
}
#endif //!< DEBUG_STDOUT

void VK::GetQueueFamily()
{
	//!< キューのプロパティを列挙
	uint32_t QueueFamilyPropertyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(PhysicalDevice, &QueueFamilyPropertyCount, nullptr);
	assert(QueueFamilyPropertyCount && "QueueFamilyProperty not found");
	std::vector<VkQueueFamilyProperties> QueueProperties(QueueFamilyPropertyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(PhysicalDevice, &QueueFamilyPropertyCount, QueueProperties.data());

#ifdef DEBUG_STDOUT
	std::cout << "\t" << "QueueProperties" << std::endl;
#define QUEUE_FLAG_ENTRY(entry) if(VK_QUEUE_##entry##_BIT & QueueProperties[i].queueFlags) { std::cout << #entry << " | "; }
	for (uint32_t i = 0; i < QueueProperties.size(); ++i) {
		std::cout << "\t" << "\t" << "[" << i << "] " << "QueueCount = " << QueueProperties[i].queueCount << ", ";
		std::cout << "QueueFlags = ";
		QUEUE_FLAG_ENTRY(GRAPHICS);
		QUEUE_FLAG_ENTRY(COMPUTE);
		QUEUE_FLAG_ENTRY(TRANSFER);
		QUEUE_FLAG_ENTRY(SPARSE_BINDING);
		std::cout << std::endl;

		VkBool32 Supported = VK_FALSE;
		VERIFY_SUCCEEDED(vkGetPhysicalDeviceSurfaceSupportKHR(PhysicalDevice, i, Surface, &Supported));
		if (Supported) {
			std::cout << "\t" << "\t" << "\t" << "Surface(Present) Supported" << std::endl;
		}
	}
#undef QUEUE_FLAG_ENTRY
#endif

	for (uint32_t i = 0; i < QueueProperties.size(); ++i) {
		//!< グラフィック機能を持つキュー Queue index which has graphics
		if (VK_QUEUE_GRAPHICS_BIT & QueueProperties[i].queueFlags) {
			if (UINT32_MAX != GraphicsQueueFamilyIndex) {
				GraphicsQueueFamilyIndex = i;
			}
		}
		//else if (VK_QUEUE_TRANSFER_BIT & QueueProperties[i].queueFlags) {
		//	//!< #VK_TODO
		//	TransferQueueFamilyIndex = i; //!< デバイスによっては転送専用キューを持つ、転送を多用する場合は専用キューを使用した方が良い
		//}
		//else if (VK_QUEUE_COMPUTE_BIT & QueueProperties[i].queueFlags) {
		//	//!< #VK_TODO
		//	ComputeQueueFamilyIndex = i;
		//}

		//!< プレゼンテーション機能を持つキュー Queue index which has presentation
		VkBool32 Supported = VK_FALSE;
		VERIFY_SUCCEEDED(vkGetPhysicalDeviceSurfaceSupportKHR(PhysicalDevice, i, Surface, &Supported));
		if (Supported) {
			if (UINT32_MAX != PresentQueueFamilyIndex) {
				PresentQueueFamilyIndex = i;
			}
		}
	}
	//!< グラフィックとプレゼンテーションを同時にサポートするキューがあれば優先 Prioritize queue which support both of graphics and presentation
	for (uint32_t i = 0; i < QueueFamilyPropertyCount; ++i) {
		if (VK_QUEUE_GRAPHICS_BIT & QueueProperties[i].queueFlags) {
			VkBool32 Supported = VK_FALSE;
			VERIFY_SUCCEEDED(vkGetPhysicalDeviceSurfaceSupportKHR(PhysicalDevice, i, Surface, &Supported));
			if (Supported) {
				GraphicsQueueFamilyIndex = i;
				PresentQueueFamilyIndex = i;
				break;
			}
		}
	}
	assert(UINT32_MAX != GraphicsQueueFamilyIndex && "GraphicsQueue not found");
	assert(UINT32_MAX != PresentQueueFamilyIndex && "PresentQueue not found");

#ifdef DEBUG_STDOUT
	std::cout << std::endl;
	std::cout << "\t" << "\t" << "GraphicsQueueFamilyIndex = " << GraphicsQueueFamilyIndex << std::endl;
	std::cout << "\t" << "\t" << "PresentQueueFamilyIndex = " << PresentQueueFamilyIndex << std::endl;
#endif
}
void VK::OverridePhysicalDeviceFeatures(VkPhysicalDeviceFeatures& PhysicalDeviceFeatures) const
{
#ifdef DEBUG_STDOUT
	std::cout << "\t" << "DeviceFeatures" << std::endl;
#define VK_DEVICEFEATURE_ENTRY(entry) if(PhysicalDeviceFeatures.entry) { std::cout << "\t" << "\t" << #entry << std::endl; }
#include "VKDeviceFeature.h"
#undef VK_DEVICEFEATURE_ENTRY
#endif //!< DEBUG_STDOUT
}
void VK::CreateDevice()
{
	const std::vector<float> QueuePriorities = { 0.0f };
	std::vector<VkDeviceQueueCreateInfo> QueueCreateInfos = {
		{
			VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
			nullptr,
			0,
			GraphicsQueueFamilyIndex,
			static_cast<uint32_t>(QueuePriorities.size()), QueuePriorities.data()
		},
	};
	//!< グラフィックとプレゼントのキューインデックスが別の場合は追加で必要 If graphics and presentation queue index is different, we must add one more
	if (GraphicsQueueFamilyIndex != PresentQueueFamilyIndex) {
		QueueCreateInfos.push_back(
			{
				VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
				nullptr,
				0,
				PresentQueueFamilyIndex,
				static_cast<uint32_t>(QueuePriorities.size()), QueuePriorities.data()
			}
		);
	}

	const std::vector<const char*> EnabledLayerNames = {
#ifdef _DEBUG
		//!< ↓標準的なバリデーションレイヤセットを最適な順序でロードする指定
		"VK_LAYER_LUNARG_standard_validation", 
		"VK_LAYER_LUNARG_object_tracker",
#endif
	};

	std::vector<const char*> EnabledExtensions = {
		//!< スワップチェインはプラットフォームに特有の機能なのでデバイス作製時に VK_KHR_SWAPCHAIN_EXTENSION_NAME エクステンションを有効にして作成しておく
		VK_KHR_SWAPCHAIN_EXTENSION_NAME,
	};
#ifdef _DEBUG
	//!< ↓デバッグマーカー拡張があるなら追加。If we have debug marker extension, add
	//!< RenderDoc から起動した時にのみ拡張は有効みたい。This extention will be valid, only if invoked from RenderDoc
	const auto HasDebugMarkerExt = HasExtension(PhysicalDevice, VK_EXT_DEBUG_MARKER_EXTENSION_NAME);
	if (HasDebugMarkerExt) {
		EnabledExtensions.push_back(VK_EXT_DEBUG_MARKER_EXTENSION_NAME);
	}
#endif

	VkPhysicalDeviceFeatures PhysicalDeviceFeatures;
	//!< デバイスフィーチャーを「有効にしないと」と使用できない機能が多々あるので注意
	//!< ここでは可能なだけ有効にしている (パフォーマンス的には良くない)
	vkGetPhysicalDeviceFeatures(PhysicalDevice, &PhysicalDeviceFeatures);
	OverridePhysicalDeviceFeatures(PhysicalDeviceFeatures);
	const VkDeviceCreateInfo DeviceCreateInfo = {
		VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
		nullptr,
		0,
		static_cast<uint32_t>(QueueCreateInfos.size()), QueueCreateInfos.data(),
		static_cast<uint32_t>(EnabledLayerNames.size()), EnabledLayerNames.data(),
		static_cast<uint32_t>(EnabledExtensions.size()), EnabledExtensions.data(),
		&PhysicalDeviceFeatures
	};
	VERIFY_SUCCEEDED(vkCreateDevice(PhysicalDevice, &DeviceCreateInfo, GetAllocationCallbacks(), &Device));

#ifdef VK_NO_PROTOYYPES
#define VK_DEVICE_PROC_ADDR(proc) vk ## proc = reinterpret_cast<PFN_vk ## proc>(vkGetDeviceProcAddr(Device, "vk" #proc)); assert(nullptr != vk ## proc && #proc); assert(nullptr != vk ## proc && #proc);
#include "VKDeviceProcAddr.h"
#undef VK_DEVICE_PROC_ADDR
#endif //!< VK_NO_PROTOYYPES

	//!< キューの取得 (グラフィック、プレゼントキューは同じインデックスの場合もあるが別の変数に取得しておく) Graphics and presentation index may be same, but save to individual variables
	vkGetDeviceQueue(Device, GraphicsQueueFamilyIndex, 0, &GraphicsQueue);
	vkGetDeviceQueue(Device, PresentQueueFamilyIndex, 0, &PresentQueue);
 
#ifdef _DEBUG
	if (HasDebugMarkerExt) {
#define VK_DEVICE_PROC_ADDR(proc) vk ## proc = reinterpret_cast<PFN_vk ## proc ## EXT>(vkGetDeviceProcAddr(Device, "vk" #proc "EXT")); assert(nullptr != vk ## proc && #proc);
#include "VKDebugMarker.h"
#undef VK_DEVICE_PROC_ADDR
	}
#endif

#ifdef DEBUG_STDOUT
	std::cout << "CreateDevice" << COUT_OK << std::endl << std::endl;
#endif
}

void VK::CreateFence()
{
	const VkFenceCreateInfo FenceCreateInfo = {
		VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
		nullptr,
		VK_FENCE_CREATE_SIGNALED_BIT //!< 初回と２回目以降を同じに扱う為にシグナル済み状態で作成 Create signaled state to do same operation on first time and second time
	};
	VERIFY_SUCCEEDED(vkCreateFence(Device, &FenceCreateInfo, GetAllocationCallbacks(), &Fence));

#ifdef DEBUG_STDOUT
	std::cout << "CreateFence" << COUT_OK << std::endl << std::endl;
#endif
}

/**
@brief 複数キュー間の同期、単一キューへの疎粒度のサブミットの同期
*/
void VK::CreateSemaphore()
{
	const VkSemaphoreCreateInfo SemaphoreCreateInfo = {
		VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
		nullptr,
		0
	};

	//!< プレゼント完了同期用 To wait presentation finish
	VERIFY_SUCCEEDED(vkCreateSemaphore(Device, &SemaphoreCreateInfo, GetAllocationCallbacks(), &NextImageAcquiredSemaphore));

	//!< 描画完了同期用 To wait render finish
	VERIFY_SUCCEEDED(vkCreateSemaphore(Device, &SemaphoreCreateInfo, GetAllocationCallbacks(), &RenderFinishedSemaphore));

#ifdef _DEBUG
	std::cout << "CreateSemaphore" << COUT_OK << std::endl << std::endl;
#endif
}

void VK::CreateCommandPool(const uint32_t QueueFamilyIndex)
{
	/**
	@brief VkCommandPoolCreateFlags
	* VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT	... コマンドバッファ毎にリセットが可能、指定無しだとプール単位でまとめてリセットしかできない
	* VK_COMMAND_POOL_CREATE_TRANSIENT_BIT			... 頻繁に更新される、ライフスパンが短い場合 (メモリアロケーションのヒントとなる)
	*/
	const VkCommandPoolCreateInfo CommandPoolInfo = {
		VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
		nullptr,
		VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT/*| VK_COMMAND_POOL_CREATE_TRANSIENT_BIT*/,
		QueueFamilyIndex
	};

	VkCommandPool CommandPool;
	VERIFY_SUCCEEDED(vkCreateCommandPool(Device, &CommandPoolInfo, GetAllocationCallbacks(), &CommandPool));
	CommandPools.push_back(CommandPool);

#ifdef DEBUG_STDOUT
	std::cout << "CreateCommandPool" << COUT_OK << std::endl << std::endl;
#endif
}

/**
@brief 以下のように確保される Allocate like this
CommandBuffers[Count]
SecondaryCommandBuffers[Count][SecondaryCount]
*/
void VK::AllocateCommandBuffer(const VkCommandPool CommandPool, const size_t Count, const size_t SecondaryCount)
{
	if (Count) {
		const VkCommandBufferAllocateInfo CommandBufferAllocateInfo = {
			VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
			nullptr,
			CommandPool,
			VK_COMMAND_BUFFER_LEVEL_PRIMARY,
			static_cast<uint32_t>(Count)
		}; 

		CommandBuffers.resize(Count);
		VERIFY_SUCCEEDED(vkAllocateCommandBuffers(Device, &CommandBufferAllocateInfo, CommandBuffers.data()));

		SecondaryCommandBuffers.resize(Count);
		if (SecondaryCount) {
			const VkCommandBufferAllocateInfo SecondaryCommandBufferAllocateInfo = {
				VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
				nullptr,
				CommandPool,
				VK_COMMAND_BUFFER_LEVEL_SECONDARY,
				static_cast<uint32_t>(SecondaryCount)
			};

			for (auto i : SecondaryCommandBuffers) {
				i.resize(SecondaryCount);
				VERIFY_SUCCEEDED(vkAllocateCommandBuffers(Device, &SecondaryCommandBufferAllocateInfo, i.data()));
			}
		}
	}

#ifdef DEBUG_STDOUT
	std::cout << "CreateCommandBuffer" << COUT_OK << std::endl << std::endl;
#endif
}

void VK::CreateSwapchain()
{
	CreateSwapchainOfClientRect();
	
	//!< スワップチェインイメージの枚数が決まったので、ここでコマンドバッファを確保する Count of swapchain image is fixed, create commandbuffer here
	{
		CreateCommandPool(GraphicsQueueFamilyIndex);
		AllocateCommandBuffer(CommandPools[0], SwapchainImages.size()); //!< 現状は0番のコマンドプール決め打ち #VK_TODO
	}

	//!< ビューを作成 CreateView
	CreateSwapchainImageView();
	
	//!< イメージの初期化 Initialize images
	//InitializeSwapchainImage(CommandBuffers[0]);
	InitializeSwapchainImage(CommandBuffers[0], &Colors::Red);
}
VkSurfaceFormatKHR VK::SelectSurfaceFormat()
{
	uint32_t SurfaceFormatCount;
	VERIFY_SUCCEEDED(vkGetPhysicalDeviceSurfaceFormatsKHR(PhysicalDevice, Surface, &SurfaceFormatCount, nullptr));
	assert(SurfaceFormatCount && "Surface format count is zero");
	std::vector<VkSurfaceFormatKHR> SurfaceFormats(SurfaceFormatCount);
	VERIFY_SUCCEEDED(vkGetPhysicalDeviceSurfaceFormatsKHR(PhysicalDevice, Surface, &SurfaceFormatCount, SurfaceFormats.data()));

	//!< ここでは、最初の非 UNDEFINED を選択する。Select first format but UNDEFINED here
	for (uint32_t i = 0; i < SurfaceFormats.size(); ++i) {
		if (VK_FORMAT_UNDEFINED != SurfaceFormats[i].format) {
#ifdef DEBUG_STDOUT
			[&]() {
				std::cout << "\t" << "\t" << Lightblue << "Format" << White << std::endl;
				for (uint32_t j = 0; j < SurfaceFormats.size();++j) {
					if (i == j) { std::cout << Yellow; }
					std::cout << "\t" << "\t" << "\tFormat = " << GetFormatString(SurfaceFormats[j].format) << ", ColorSpace = " << GetColorSpaceString(SurfaceFormats[j].colorSpace) << std::endl;
					std::cout << White;
				}
				std::cout << std::endl;
			}();
#endif
			return SurfaceFormats[i];
		}
	}
	
	//!< 要素が 1 つのみで UNDEFINED の場合、好きなものを選択できる。Has only 1 element and is UNDEFINED, we can choose
	if (1 == SurfaceFormats.size() && VK_FORMAT_UNDEFINED == SurfaceFormats[0].format) {
#ifdef DEBUG_STDOUT
		std::cout << Yellow;
		std::cout << "\t" << "\t" << "\tFormat = " << GetFormatString(VK_FORMAT_B8G8R8A8_UNORM) << ", ColorSpace = " << GetColorSpaceString(VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) << std::endl;
		std::cout << White;
#endif
		return VkSurfaceFormatKHR({ VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR });
	}

	assert(false && "Valid surface format not found");
	return SurfaceFormats[0];
}
VkPresentModeKHR VK::SelectSurfacePresentMode()
{
	uint32_t PresentModeCount;
	VERIFY_SUCCEEDED(vkGetPhysicalDeviceSurfacePresentModesKHR(PhysicalDevice, Surface, &PresentModeCount, nullptr));
	assert(PresentModeCount);
	std::vector<VkPresentModeKHR> PresentModes(PresentModeCount);
	VERIFY_SUCCEEDED(vkGetPhysicalDeviceSurfacePresentModesKHR(PhysicalDevice, Surface, &PresentModeCount, PresentModes.data()));

	//!< 可能なら VK_PRESENT_MODE_MAILBOX_KHR を選択、次いで VK_PRESENT_MODE_FIFO_KHR を選択
	/**
	@brief VkPresentModeKHR
	* VK_PRESENT_MODE_IMMEDIATE_KHR		... テアリングが起こる。 Tearing happen
	* VK_PRESENT_MODE_MAILBOX_KHR		... キューは 1 つで常に最新で上書きされる。Queue is 1, and always update to new image
	* VK_PRESENT_MODE_FIFO_KHR			... VulkanAPI が必ずサポートする VulkanAPI always support this
	* VK_PRESENT_MODE_FIFO_RELAXED_KHR	... 1Vブランク以上経ったイメージは次のVブランクを待たずにプレゼンテーションされ得る (余裕が無い場合にはテアリングが起こる)
	*/
	const VkPresentModeKHR SelectedPresentMode = [&]() {
		for (auto i : PresentModes) {
			if (VK_PRESENT_MODE_MAILBOX_KHR == i) {
				//!< 可能なら MAILBOX。Want to use MAILBOX
				return i;
			}
		}
		for (auto i : PresentModes) {
			if (VK_PRESENT_MODE_FIFO_KHR == i) {
				//!< FIFO は VulkanAPI が必ずサポートする。 VulkanAPI always support FIFO
				return i;
			}
		}
		return PresentModes[0];
	}();

#ifdef DEBUG_STDOUT
	std::cout << "\t" << Lightblue << "Present Mode" << White << std::endl;
#define VK_PRESENT_MODE_ENTRY(entry) case VK_PRESENT_MODE_##entry##_KHR: std::cout << "\t" << "\t" << #entry << std::endl; break
	for (auto i : PresentModes) {
		if (SelectedPresentMode == i) { std::cout << Yellow; }
		switch (i) {
		default: assert(0 && "Unknown VkPresentMode"); break;
		VK_PRESENT_MODE_ENTRY(IMMEDIATE);	
		VK_PRESENT_MODE_ENTRY(MAILBOX);		
		VK_PRESENT_MODE_ENTRY(FIFO);		
		VK_PRESENT_MODE_ENTRY(FIFO_RELAXED);
		}
		std::cout << White;
#undef VK_PRESENT_MODE_ENTRY
	}
#endif

	return SelectedPresentMode;
}
void VK::CreateSwapchain(const uint32_t Width, const uint32_t Height)
{
	VkSurfaceCapabilitiesKHR SurfaceCapabilities;
	VERIFY_SUCCEEDED(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(PhysicalDevice, Surface, &SurfaceCapabilities));

	//!< minImageCount + 1 枚取る (MAILBOX では 3 枚欲しいので) Use minImageCount + 1 images (We want 3 image to use MAILBOX)
	//!< 自分の環境では minImageCount は 2。In my environment minImageCount is 2
	const auto MinImageCount = (std::min)(SurfaceCapabilities.minImageCount + 1, SurfaceCapabilities.maxImageCount);
#ifdef DEBUG_STDOUT
	std::cout << "\t" << "\t" << Lightblue << "ImageCount = " << White << MinImageCount << std::endl;
#endif

	if (0xffffffff == SurfaceCapabilities.currentExtent.width) {
		//!< 0xffffffff の場合はイメージサイズがウインドウサイズを決定する。If 0xffffffff, size of image determines the size of the window
		SurfaceExtent2D = {
			(std::max)((std::min)(Width, SurfaceCapabilities.maxImageExtent.width), SurfaceCapabilities.minImageExtent.width), 
			(std::max)((std::min)(Height, SurfaceCapabilities.minImageExtent.height), SurfaceCapabilities.minImageExtent.height) 
		};
	}
	else {
		//!< そうでない場合は currentExtent。Otherwise, use currentExtent
		SurfaceExtent2D = SurfaceCapabilities.currentExtent;
	}
#ifdef DEBUG_STDOUT
	std::cout << "\t" << "\t" << Lightblue << "ImageExtent (" << (0xffffffff == SurfaceCapabilities.currentExtent.width ? "Undefined" : "Defined") << ")" << White << std::endl;
	std::cout << "\t" << "\t" << "\t" << SurfaceExtent2D.width << " x " << SurfaceExtent2D.height << std::endl;
#endif

	const auto DesiredUsage = VK_IMAGE_USAGE_TRANSFER_DST_BIT; //!< イメージクリア用に TRANSFER_DST(サポートされていれば) For clear image TRANSFER_DST(If supported)
	const VkImageUsageFlags ImageUsageFlags = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | (DesiredUsage & SurfaceCapabilities.supportedUsageFlags);

	//!< サーフェスを回転、反転させるかどうか。Rotate, mirror surface or not
	const auto DesiredTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR; //!< ここでは IDENTITY を希望。Want IDENTITY here
	const auto PreTransform = (SurfaceCapabilities.supportedTransforms & DesiredTransform) ? DesiredTransform : SurfaceCapabilities.currentTransform;

	//!< サーフェスのフォーマットを選択
	const auto SurfaceFormat = SelectSurfaceFormat();

	//!< サーフェスのプレゼントモードを選択
	const auto PresentMode = SelectSurfacePresentMode();

	//!< セッティングを変更してスワップチェインを再作成できるようにしておく。既存を開放するために OldSwapchain に覚える
	auto OldSwapchain = Swapchain;
	const VkSwapchainCreateInfoKHR SwapchainCreateInfo = {
		VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
		nullptr,
		0,
		Surface,
		MinImageCount,
		(ColorFormat = SurfaceFormat.format),
		SurfaceFormat.colorSpace,
		SurfaceExtent2D,
		1,
		ImageUsageFlags,
		VK_SHARING_MODE_EXCLUSIVE,
		0, nullptr,
		PreTransform,
		VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
		PresentMode,
		true,
		OldSwapchain
	};
	VERIFY_SUCCEEDED(vkCreateSwapchainKHR(Device, &SwapchainCreateInfo, GetAllocationCallbacks(), &Swapchain));
#ifdef DEBUG_STDOUT
	std::cout << "CreateSwapchain" << COUT_OK << std::endl << std::endl;
#endif

	if (VK_NULL_HANDLE != OldSwapchain) {
		for (auto i : SwapchainImageViews) {
			vkDestroyImageView(Device, i, GetAllocationCallbacks());
		}
		SwapchainImageViews.clear();

		vkDestroySwapchainKHR(Device, OldSwapchain, GetAllocationCallbacks());
	}

	//!< スワップチェインイメージの取得
	uint32_t SwapchainImageCount;
	VERIFY_SUCCEEDED(vkGetSwapchainImagesKHR(Device, Swapchain, &SwapchainImageCount, nullptr));
	assert(SwapchainImageCount && "Swapchain image count == 0");
	SwapchainImages.resize(SwapchainImageCount);
	VERIFY_SUCCEEDED(vkGetSwapchainImagesKHR(Device, Swapchain, &SwapchainImageCount, SwapchainImages.data()));
#ifdef DEBUG_STDOUT
	std::cout << "GetSwapchainImages" << COUT_OK << std::endl << std::endl;
#endif
}

/**
@note Vulaknでは、1つのコマンドバッファで複数のスワップチェインイメージをまとめて処理できるっぽい
*/
void VK::InitializeSwapchainImage(const VkCommandBuffer CommandBuffer, const VkClearColorValue* ClearColorValue)
{
	VERIFY_SUCCEEDED(vkBeginCommandBuffer(CommandBuffer, &CommandBufferBeginInfo_OneTime)); {
		for (auto& i : SwapchainImages) {
			if (nullptr == ClearColorValue) {
				const VkImageMemoryBarrier ImageMemoryBarrier_UndefinedToPresent = {
					VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
					nullptr,
					0,
					VK_ACCESS_MEMORY_READ_BIT,
					VK_IMAGE_LAYOUT_UNDEFINED, //!<「現在のレイアウト」または「UNDEFINED」を指定すること、イメージコンテンツを保持したい場合は「UNDEFINED」はダメ         
					VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, //!< プレゼンテーション可能な VK_IMAGE_LAYOUT_PRESENT_SRC_KHR へ
					PresentQueueFamilyIndex,
					PresentQueueFamilyIndex,
					i,
					ImageSubresourceRange_Color
				};
				vkCmdPipelineBarrier(CommandBuffer,
					VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT/*VK_PIPELINE_STAGE_TRANSFER_BIT*/,
					0,
					0, nullptr,
					0, nullptr,
					1, &ImageMemoryBarrier_UndefinedToPresent);
			}
			else {
				const VkImageMemoryBarrier ImageMemoryBarrier_UndefinedToTransfer = {
					VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
					nullptr,
					0,
					VK_ACCESS_TRANSFER_WRITE_BIT,
					VK_IMAGE_LAYOUT_UNDEFINED, //!<「現在のレイアウト」または「UNDEFINED」を指定すること        
					VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, //!< デスティネーションへ
					PresentQueueFamilyIndex,
					PresentQueueFamilyIndex,
					i,
					ImageSubresourceRange_Color
				};
				vkCmdPipelineBarrier(CommandBuffer,
					VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
					0,
					0, nullptr,
					0, nullptr,
					1, &ImageMemoryBarrier_UndefinedToTransfer);
				{
					vkCmdClearColorImage(CommandBuffer, i, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, ClearColorValue, 1, &ImageSubresourceRange_Color);
				}
				const VkImageMemoryBarrier ImageMemoryBarrier_TransferToPresent = {
					VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
					nullptr,
					VK_ACCESS_TRANSFER_WRITE_BIT,
					VK_ACCESS_MEMORY_READ_BIT,
					VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, //!< デスティネーションから
					VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, //!< プレゼンテーション可能な VK_IMAGE_LAYOUT_PRESENT_SRC_KHR へ
					PresentQueueFamilyIndex,
					PresentQueueFamilyIndex,
					i,
					ImageSubresourceRange_Color
				};
				vkCmdPipelineBarrier(CommandBuffer,
					VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
					0,
					0, nullptr,
					0, nullptr,
					1, &ImageMemoryBarrier_TransferToPresent);
			}
		}
	} VERIFY_SUCCEEDED(vkEndCommandBuffer(CommandBuffer));

	const VkSubmitInfo SubmitInfo = {
		VK_STRUCTURE_TYPE_SUBMIT_INFO,
		nullptr,
		0, nullptr, nullptr,
		1,  &CommandBuffer,
		0, nullptr
	};
	VERIFY_SUCCEEDED(vkQueueSubmit(GraphicsQueue, 1, &SubmitInfo, VK_NULL_HANDLE));
	VERIFY_SUCCEEDED(vkQueueWaitIdle(GraphicsQueue));

#ifdef DEBUG_STDOUT
	std::cout << "InitializeSwapchainImage" << COUT_OK << std::endl << std::endl;
#endif
}
void VK::ResizeSwapchain(const uint32_t Width, const uint32_t Height)
{
	//!< #TODO
	if (VK_NULL_HANDLE != Device) {
		VERIFY_SUCCEEDED(vkDeviceWaitIdle(Device));
	}

	//if (!CommandPools.empty() && !CommandBuffers.empty()) {
	//	const auto CP = CommandPools[0]; //!< 現状は0番のコマンドプール決め打ち #VK_TODO 
	//	vkFreeCommandBuffers(Device, CP, static_cast<uint32_t>(CommandBuffers.size()), CommandBuffers.data());
	//}
	//CommandBuffers.clear();
	//std::for_each(CommandPools.begin(), CommandPools.end(), [&](const VkCommandPool rhs) { vkDestroyCommandPool(Device, rhs, GetAllocationCallbacks()); });
	//CommandPools.clear();

	CreateSwapchain();
}

void VK::CreateSwapchainImageView()
{
	for(auto i : SwapchainImages) {
		VkImageView ImageView;
		CreateImageView(&ImageView, i, VK_IMAGE_VIEW_TYPE_2D, ColorFormat, ComponentMapping_Identity, ImageSubresourceRange_Color);
		
		SwapchainImageViews.push_back(ImageView);
	}

#ifdef DEBUG_STDOUT
	std::cout << "\t" << "SwapchainImageIndex = " << SwapchainImageIndex << std::endl;
#endif

#ifdef DEBUG_STDOUT
	std::cout << "CreateSwapchainImageView" << COUT_OK << std::endl << std::endl;
#endif
}

void VK::CreateDepthStencilImage()
{
	DepthFormat = GetSupportedDepthFormat();

	const VkExtent3D Extent3D = {
		SurfaceExtent2D.width, SurfaceExtent2D.height, 1
	};
	const VkImageUsageFlags Usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
	CreateImage(&DepthStencilImage, Usage, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_TYPE_2D, DepthFormat, Extent3D, 1, 1);

#ifdef DEBUG_STDOUT
	std::cout << "\t" << "DepthStencilImage" << std::endl;
#endif
}
void VK::CreateDepthStencilDeviceMemory()
{
	CreateDeviceLocalMemory(&DepthStencilDeviceMemory, DepthStencilImage);

	BindDeviceMemory(DepthStencilImage, DepthStencilDeviceMemory/*, 0*/);

#ifdef DEBUG_STDOUT
	std::cout << "\t" << "DepthStencilDeviceMemory" << std::endl;
#endif
}

void VK::CreateViewport(const float Width, const float Height, const float MinDepth, const float MaxDepth)
{
	Viewports = {
		{
			0, 0,
			Width, Height,
			MinDepth, MaxDepth
		}
	};
	ScissorRects = {
		{
			{ 0, 0 },
			{ static_cast<uint32_t>(Width), static_cast<uint32_t>(Height) }
		}
	};

#ifdef DEBUG_STDOUT
	std::cout << "CreateViewport" << COUT_OK << std::endl << std::endl;
#endif
}

#if 0
void VK::CreateStorageBuffer()
{
	const auto Size = 256;

	VkBuffer Buffer = VK_NULL_HANDLE;
	VkDeviceMemory DeviceMemory = VK_NULL_HANDLE;

	[&](VkBuffer* Buffer, VkDeviceMemory* DeviceMemory, const VkDeviceSize Size) {
		const auto Usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;

		CreateBuffer(Buffer, Usage, Size);
		CreateDeviceLocalMemory(DeviceMemory, *Buffer);
		BindDeviceMemory(*Buffer, *DeviceMemory);

		//!< View は必要ない No need view

	}(&Buffer, &DeviceMemory, Size);

	if (VK_NULL_HANDLE != DeviceMemory) {
		vkFreeMemory(Device, DeviceMemory, GetAllocationCallbacks());
	}
	if (VK_NULL_HANDLE != Buffer) {
		vkDestroyBuffer(Device, Buffer, GetAllocationCallbacks());
	}
}
void VK::CreateUniformTexelBuffer()
{
	const auto Size = 256;

	VkBuffer Buffer = VK_NULL_HANDLE;
	VkDeviceMemory DeviceMemory = VK_NULL_HANDLE;
	VkBufferView View = VK_NULL_HANDLE;

	[&](VkBuffer* Buffer, VkDeviceMemory* DeviceMemory, VkBufferView* View, const VkDeviceSize Size) {
		const auto Usage = VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT;

		CreateBuffer(Buffer, Usage, Size);
		CreateDeviceLocalMemory(DeviceMemory, *Buffer);

		BindDeviceMemory(*Buffer, *DeviceMemory);

		//!< UniformTexelBuffer の場合は、フォーマットを指定する必要があるため、ビューを作成する UniformTexelBuffer need format, so create view
		const auto Format = VK_FORMAT_R8G8B8A8_UNORM;
		ValidateFormatProperties(Usage, Format);
		CreateBufferView(View, *Buffer, Format);
	}(&Buffer, &DeviceMemory, &View, Size);

	if (VK_NULL_HANDLE != DeviceMemory) {
		vkFreeMemory(Device, DeviceMemory, GetAllocationCallbacks());
	}
	if (VK_NULL_HANDLE != Buffer) {
		vkDestroyBuffer(Device, Buffer, GetAllocationCallbacks());
	}
	if (VK_NULL_HANDLE != View) {
		vkDestroyBufferView(Device, View, GetAllocationCallbacks());
	}
}
void VK::CreateStorageTexelBuffer()
{
	const auto Size = 256;

	VkBuffer Buffer = VK_NULL_HANDLE;
	VkDeviceMemory DeviceMemory = VK_NULL_HANDLE;
	VkBufferView View = VK_NULL_HANDLE;
	
	[&](VkBuffer* Buffer, VkDeviceMemory* DeviceMemory, VkBufferView* View, const VkDeviceSize Size) {
		const auto Usage = VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT;

		CreateBuffer(Buffer, Usage, Size);
		CreateDeviceLocalMemory(DeviceMemory, *Buffer);
		BindDeviceMemory(*Buffer, *DeviceMemory);

		//!< UniformStorageBuffer の場合は、フォーマットを指定する必要があるため、ビューを作成する UniformStorageBuffer need format, so create view
		const auto Format = VK_FORMAT_R8G8B8A8_UNORM;
		ValidateFormatProperties(Usage, Format);
		CreateBufferView(View, *Buffer, Format);
	}(&Buffer, &DeviceMemory, &View, Size);
	
	if (VK_NULL_HANDLE != DeviceMemory) {
		vkFreeMemory(Device, DeviceMemory, GetAllocationCallbacks());
	}
	if (VK_NULL_HANDLE != Buffer) {
		vkDestroyBuffer(Device, Buffer, GetAllocationCallbacks());
	}
	if (VK_NULL_HANDLE != View) {
		vkDestroyBufferView(Device, View, GetAllocationCallbacks());
	}
}
#endif

/**
@brief シェーダとのバインディング (DX::CreateRootSignature()相当)
@note デスクリプタを使用しない場合でも、デスクリプタセットレイアウト自体は作成しなくてはならない
*/
void VK::CreateDescriptorSetLayout()
{
	std::vector<VkDescriptorSetLayoutBinding> DescriptorSetLayoutBindings = {
		/**
		uint32_t              binding;
		VkDescriptorType      descriptorType; ... VK_DESCRIPTOR_TYPE_[UNIFORM_BUFFER, SAMPLER, COMBINED_IMAGE_SAMPLER, SAMPLED_IMAGE, ...]
		uint32_t              descriptorCount;
		VkShaderStageFlags    stageFlags; ... VK_SHADER_STAGE_[VERTEX_BIT, TESSELLATION_CONTROL_BIT, TESSELLATION_EVALUATION_BIT, GEOMETRY_BIT, FRAGMENT_BIT, COMPUTE_BIT, ALL_GRAPHICS, ALL]
		const VkSampler*      pImmutableSamplers;
		*/
	};
	CreateDescriptorSetLayoutBindings(DescriptorSetLayoutBindings);

	const VkDescriptorSetLayoutCreateInfo DescriptorSetLayoutCreateInfo = {
		VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
		nullptr,
		0,
		static_cast<uint32_t>(DescriptorSetLayoutBindings.size()), DescriptorSetLayoutBindings.data()
	};
	VkDescriptorSetLayout DescriptorSetLayout = VK_NULL_HANDLE;
	VERIFY_SUCCEEDED(vkCreateDescriptorSetLayout(Device, &DescriptorSetLayoutCreateInfo, GetAllocationCallbacks(), &DescriptorSetLayout));
	DescriptorSetLayouts.push_back(DescriptorSetLayout);

#ifdef DEBUG_STDOUT
	std::cout << "CreateDescriptorSetLayout" << COUT_OK << std::endl << std::endl;
#endif
}
/**
@brief シェーダとのバインディングのレイアウト
@note DescriptorSet は「DescriptorSetLayt 型」のインスタンスのようなもの
@note 更新は vkUpdateDescriptorSets() で行う
*/
void VK::CreateDescriptorSet()
{
	std::vector<VkDescriptorPoolSize> DescriptorPoolSizes = {
		/**
		VkDescriptorType    type; ... VK_DESCRIPTOR_TYPE_[SAMPLER, SAMPLED_IMAGE, UNIFORM_BUFFER, ...]
		uint32_t            descriptorCount;
		*/
	};
	CreateDescriptorPoolSizes(DescriptorPoolSizes);

	if (!DescriptorPoolSizes.empty()) {
		//!< プールを作成 Create pool
		const uint32_t MaxSets = [&]() {
			uint32_t MaxDescriptorCount = 0;
			for (const auto& i : DescriptorPoolSizes) {
				MaxDescriptorCount = std::max(MaxDescriptorCount, i.descriptorCount);
			}
			return MaxDescriptorCount;
		}();
		const VkDescriptorPoolCreateInfo DescriptorPoolCreateInfo = {
			VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
			nullptr,
			0/*VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT*/, //!< デスクリプタセットを個々に解放したい場合に指定(プールごとの場合は不要)
			MaxSets,
			static_cast<uint32_t>(DescriptorPoolSizes.size()), DescriptorPoolSizes.data()
		};
		VERIFY_SUCCEEDED(vkCreateDescriptorPool(Device, &DescriptorPoolCreateInfo, GetAllocationCallbacks(), &DescriptorPool));
		assert(VK_NULL_HANDLE != DescriptorPool && "Failed to create descriptor pool");

		//!< プールからデスクリプタセットを作成 Create descriptor set from pool
		const VkDescriptorSetAllocateInfo DescriptorSetAllocateInfo = {
			VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
			nullptr,
			DescriptorPool,
			static_cast<uint32_t>(DescriptorSetLayouts.size()), DescriptorSetLayouts.data()
		};
		DescriptorSets.resize(DescriptorSetLayouts.size());
		VERIFY_SUCCEEDED(vkAllocateDescriptorSets(Device, &DescriptorSetAllocateInfo, DescriptorSets.data()));

#ifdef DEBUG_STDOUT
		std::cout << "CreateDescriptorSet" << COUT_OK << std::endl << std::endl;
#endif
	}
}

void VK::UpdateDescriptorSet()
{
	std::vector<VkWriteDescriptorSet> WriteDescriptorSets;
	//WriteDescriptorSets.resize(1);
	//std::vector<VkDescriptorImageInfo> DescriptorImageInfos;
	//std::vector<VkDescriptorBufferInfo> DescriptorBufferInfos;
	//std::vector<VkBufferView> BufferViews;
	//CreateWriteDescriptorSets(WriteDescriptorSets.back(), DescriptorImageInfos, DescriptorBufferInfos, BufferViews);

	std::vector<VkCopyDescriptorSet> CopyDescriptorSets;
	//CopyDescriptorSets.resize(1);
	//CreateCopyDescriptorSets(CopyDescriptorSets.back());

	vkUpdateDescriptorSets(Device,
		static_cast<uint32_t>(WriteDescriptorSets.size()), WriteDescriptorSets.data(),
		static_cast<uint32_t>(CopyDescriptorSets.size()), CopyDescriptorSets.data());
}

/**
@brief デスクリプタセットよりも高速
パイプラインレイアウト全体で128 Byte (ハードが許せばこれ以上使える場合もある ex)GTX970M ... 256byte)

各シェーダステージは1つのプッシュコンスタントにしかアクセスできない
各々のシェーダステージが共通のレンジを持たないような「ワーストケース」では 128 / 5(シェーダステージ)で 1シェーダステージで 25-6 Byte程度になる
*/
void VK::CreatePushConstantRanges()
{
#if 1
	//!< ここではバーテックス、フラグメントシェーダに 64byte 確保 (使用しない分には問題ない) In this case, assign 64byte for vertex and fragment shader
	PushConstantRanges = {
		{ VK_SHADER_STAGE_VERTEX_BIT, 0, 64 },
		{ VK_SHADER_STAGE_FRAGMENT_BIT, 64, 64 },
	};
#else
	PushConstantRanges = {};
#endif
}

void VK::DestroyFramebuffer()
{
	for (auto i : Framebuffers) {
		vkDestroyFramebuffer(Device, i, GetAllocationCallbacks());
	}
	Framebuffers.clear();
}

/**
@brief シェーダコンパイル、リンクはパイプラインオブジェクト作成時に行われる Shader compilation and linkage is performed during the pipeline object creation
*/
VkShaderModule VK::CreateShaderModule(const std::wstring& Path) const
{
	VkShaderModule ShaderModule = VK_NULL_HANDLE;

	std::ifstream In(Path.c_str(), std::ios::in | std::ios::binary);
	if (!In.fail()) {
		In.seekg(0, std::ios_base::end);
		const auto Size = In.tellg();
		In.seekg(0, std::ios_base::beg);

		if (Size) {
			auto Data = new char[Size];
			In.read(Data, Size);

			const VkShaderModuleCreateInfo ModuleCreateInfo = {
				VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
				nullptr,
				0,
				Size, reinterpret_cast<uint32_t*>(Data)
			};
			VERIFY_SUCCEEDED(vkCreateShaderModule(Device, &ModuleCreateInfo, GetAllocationCallbacks(), &ShaderModule));

			delete[] Data;
		}
		In.close();
	}
	return ShaderModule;
}

/**
@brief DXに合わせるため、ここでは VkDynamicState を使用している For compatibility with DX, we use VkDynamicState
ここでは個数のみを指定して nullptr を指定している、後で vkCmdSetViewport(), vkCmdSetScissor() で指定すること We specifying only count, and set nulloptr, later use vkCmdSetViewport(), vkCmdSetScissor()
2つ以上のビューポートを使用するにはデバイスフィーチャー multiViewport が有効であること If we use 2 or more viewport device feature multiViewport must be enabled
ビューポートのインデックスはジオメトリシェーダで指定する Viewport index is specified in geometry shader
*/
void VK::CreateViewportState_Dynamic(VkPipelineViewportStateCreateInfo& PipelineViewportStateCreateInfo, const uint32_t Count) const
{
	PipelineViewportStateCreateInfo = {
		VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
		nullptr,
		0,
		Count, nullptr,
		Count, nullptr
	};
}

VkPipelineCache VK::LoadPipelineCache(const std::wstring& Path) const
{
	VkPipelineCache PipelineCache = VK_NULL_HANDLE;
	size_t Size = 0;

	std::ifstream In(Path.c_str(), std::ios::in | std::ios::binary);
	if (!In.fail()) {
		In.seekg(0, std::ios_base::end);
		Size = In.tellg();
		In.seekg(0, std::ios_base::beg);
	}

	char* Data = nullptr;
	if (Size) {
		Data = new char[Size];
		In.read(Data, Size);
	}

	const VkPipelineCacheCreateInfo PipelineCacheCreateInfo = {
		VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO,
		nullptr,
		0,
		Size, Data
	};
	VERIFY_SUCCEEDED(vkCreatePipelineCache(Device, &PipelineCacheCreateInfo, GetAllocationCallbacks(), &PipelineCache));

	if (nullptr != Data) {
		delete[] Data;
	}

	In.close();

	return PipelineCache;
}

void VK::LoadPipelineCaches(const std::wstring& Path, std::vector<VkPipelineCache>& PipelineCaches) const
{
	size_t Size = 0;

	std::ifstream In(Path.c_str(), std::ios::in | std::ios::binary);
	if (!In.fail()) {
		In.seekg(0, std::ios_base::end);
		Size = In.tellg();
		In.seekg(0, std::ios_base::beg);
	}

	char* Data = nullptr;
	if (Size) {
		Data = new char[Size];
		In.read(Data, Size);
	}

	const VkPipelineCacheCreateInfo PipelineCacheCreateInfo = {
		VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO,
		nullptr,
		0,
		Size, Data
	};
	for (auto& i : PipelineCaches) {
		VERIFY_SUCCEEDED(vkCreatePipelineCache(Device, &PipelineCacheCreateInfo, GetAllocationCallbacks(), &i));
	}

	if (nullptr != Data) {
		delete[] Data;
	}

	In.close();
}

void VK::StorePipelineCache(const std::wstring& Path, const VkPipelineCache PipelineCache) const
{
	if (VK_NULL_HANDLE != PipelineCache) {
		size_t Size;
		VERIFY_SUCCEEDED(vkGetPipelineCacheData(Device, PipelineCache, &Size, nullptr));
		if (Size) {
			auto Data = new char[Size];
			VERIFY_SUCCEEDED(vkGetPipelineCacheData(Device, PipelineCache, &Size, Data));

			std::ofstream Out(Path.c_str(), std::ios::out | std::ios::binary);
			if (!Out.fail()) {
				Out.write(Data, Size);
				Out.close();
			}

			delete[] Data;
		}
	}
}

VkPipelineCache VK::CreatePipelineCache() const
{
	VkPipelineCache PipelineCache = VK_NULL_HANDLE;
	const VkPipelineCacheCreateInfo PipelineCacheCreateInfo = {
		VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO,
		nullptr,
		0,
		0, nullptr
	};
	VERIFY_SUCCEEDED(vkCreatePipelineCache(Device, &PipelineCacheCreateInfo, GetAllocationCallbacks(), &PipelineCache));
	return PipelineCache;
}
void VK::CreatePipelineCaches(std::vector<VkPipelineCache>& PipelineCaches) const
{
	const VkPipelineCacheCreateInfo PipelineCacheCreateInfo = {
		VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO,
		nullptr,
		0,
		0, nullptr
	};
	for (auto& i : PipelineCaches) {
		VERIFY_SUCCEEDED(vkCreatePipelineCache(Device, &PipelineCacheCreateInfo, GetAllocationCallbacks(), &i));
	}
}

void VK::CreatePipeline()
{
	//!< スレッドでパイプラインを作成する例(パイプラインキャッシュを活用)
#if 0
	const auto ThreadCount = 10;
	const auto PipelineCountPerThread = 1;

	//!< パイプラインキャッシュをファイルから読み込む (スレッド数分だけ(同じものを)用意する)
	std::vector<VkPipelineCache> PipelineCaches(ThreadCount);
	LoadPipelineCaches(GetBasePath() + L".pco", PipelineCaches);

	//!< パイプラインキャッシュを使用して、各スレッドでパイプラインを作成する
	std::vector<std::vector<VkPipeline>> Pipelines(ThreadCount);
	for (auto i : Pipelines) { i.resize(PipelineCountPerThread); }

	std::vector<std::thread> Threads(ThreadCount);
	for (auto i = 0; i < Threads.size(); ++i) {
		Threads[i] = std::thread::thread([&](std::vector<VkPipeline>& Pipelines, VkPipelineCache PipelineCache) {
			//const std::vector<VkGraphicsPipelineCreateInfo> GraphicsPipelineCreateInfos(Pipelines.size());
			//VERIFY_SUCCEEDED(vkCreateGraphicsPipelines(Device,
			//	PipelineCache,
			//	static_cast<uint32_t>(GraphicsPipelineCreateInfos.size()), GraphicsPipelineCreateInfos.data(), 
			//	GetAllocationCallbacks(),
			//	Pipelines.data()));

			//const std::vector<VkComputePipelineCreateInfo> ComputePipelineCreateInfos(Pipelines.size());
			//VERIFY_SUCCEEDED(vkCreateComputePipelines(Device,
			//	PipelineCache,
			//	static_cast<uint32_t>(ComputePipelineCreateInfos.size()), ComputePipelineCreateInfos.data(),
			//	GetAllocationCallbacks(),
			//	Pipelines.data()));

			std::cout << "Creating pipelines in thread\n"; 
		}, Pipelines[i], PipelineCaches[i]);
	}
	for (auto& i : Threads) {
		i.join();
	}

	//!< 各スレッドが作成したパイプラインキャッシュをマージする (ここでは最後の要素へマージしている)
	VERIFY_SUCCEEDED(vkMergePipelineCaches(Device, PipelineCaches.back(), static_cast<uint32_t>(PipelineCaches.size() - 1), PipelineCaches.data()));	
	//!< マージ後のパイプラインキャッシュをファイルへ保存
	StorePipelineCache(GetBasePath() + L".pco", PipelineCaches.back());
	//!< 破棄して良い
	for (auto& i : PipelineCaches) {
		vkDestroyPipelineCache(Device, i, GetAllocationCallbacks());
	}
#endif

	CreatePipeline_Graphics();
}

void VK::CreatePipeline_Graphics()
{
#ifdef _DEBUG
	PerformanceCounter PC("CreatePipeline_Graphics : ");
#endif

	//!< シェーダ
	std::vector<VkShaderModule> ShaderModules;
	std::vector<VkPipelineShaderStageCreateInfo> PipelineShaderStageCreateInfos;
	CreateShader(ShaderModules, PipelineShaderStageCreateInfos);

	//!< バーテックスインプット
	std::vector<VkVertexInputBindingDescription> VertexInputBindingDescriptions;
	std::vector<VkVertexInputAttributeDescription> VertexInputAttributeDescriptions;
	CreateVertexInput(VertexInputBindingDescriptions, VertexInputAttributeDescriptions, 0);
	const VkPipelineVertexInputStateCreateInfo PipelineVertexInputStateCreateInfo = {
		VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
		nullptr,
		0,
		static_cast<uint32_t>(VertexInputBindingDescriptions.size()), VertexInputBindingDescriptions.data(),
		static_cast<uint32_t>(VertexInputAttributeDescriptions.size()), VertexInputAttributeDescriptions.data()
	};

	//!< インプットアセンブリ (トポロジ)
	VkPipelineInputAssemblyStateCreateInfo PipelineInputAssemblyStateCreateInfo;
	CreateInputAssembly(PipelineInputAssemblyStateCreateInfo);

	//!< テセレーションステート (テセレーションを使用する場合には、不定値のままにしないこと)
	VkPipelineTessellationStateCreateInfo PipelineTessellationStateCreateInfo;
	CreateTessellationState(PipelineTessellationStateCreateInfo);

	//!< ビューポート(シザー)
	VkPipelineViewportStateCreateInfo PipelineViewportStateCreateInfo;
	CreateViewportState(PipelineViewportStateCreateInfo);

	const VkPipelineRasterizationStateCreateInfo PipelineRasterizationStateCreateInfo = {
		VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
		nullptr,
		0,
		VK_FALSE, //!< VK_TRUE にするにはデバイスフィーチャー depthClampEnable が有効であること
		VK_FALSE,
		VK_POLYGON_MODE_FILL, //!< LINEやPOINT を有効にするにはデバイスフィーチャー fillModeNonSolid が有効であること
		VK_CULL_MODE_BACK_BIT,
		VK_FRONT_FACE_COUNTER_CLOCKWISE,
		VK_FALSE, 0.0f, 0.0f, 0.0f,
		1.0f //!< 1.0f より大きな値を指定するにはデバイスフィーチャー widelines が有効であること
	};

	//const VkSampleMask SampleMask = 0xffffffff;
	const VkPipelineMultisampleStateCreateInfo PipelineMultisampleStateCreateInfo = {
		VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
		nullptr,
		0,
		VK_SAMPLE_COUNT_1_BIT,
		VK_FALSE, 0.0f, //! VK_TRUE にするにはデバイスフィーチャー minSampleShading が有効であること
		nullptr/*&SampleMask*/, //!< 0xffffffff の場合 nullptr でよい
		VK_FALSE, VK_FALSE //!< alphaToOneEnable を VK_TRUE にするにはデバイスフィーチャー alphaToOne が有効であること
	};

	const VkPipelineDepthStencilStateCreateInfo PipelineDepthStencilStateCreateInfo = {
		VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
		nullptr,
		0,
		VK_TRUE,
		VK_TRUE,
		VK_COMPARE_OP_LESS_OR_EQUAL,
		VK_FALSE,
		VK_FALSE,
		{ VK_STENCIL_OP_KEEP, VK_STENCIL_OP_KEEP, VK_STENCIL_OP_KEEP, VK_COMPARE_OP_NEVER, 0, 0, 0 },
		{ VK_STENCIL_OP_KEEP, VK_STENCIL_OP_KEEP, VK_STENCIL_OP_KEEP, VK_COMPARE_OP_ALWAYS, 0, 0, 0 },
		0.0f, 1.0f
	};

	//!< デバイスフィーチャー independentBlend が有効で無い場合は、配列の各要素は「完全に同じ値」であること If device feature independentBlend is not enabled, each array element must be exactly same
	//!< VK_BLEND_FACTOR_SRC1 系をを使用するには、デバイスフィーチャー dualSrcBlend が有効であること
	///!< SRCコンポーネント * SRCファクタ OP DSTコンポーネント * DSTファクタ
	const std::vector<VkPipelineColorBlendAttachmentState> PipelineColorBlendAttachmentStates = {
		{
			VK_FALSE,
			VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ONE, VK_BLEND_OP_ADD,
			VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ONE, VK_BLEND_OP_ADD,
			VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
		}
	};
	const VkPipelineColorBlendStateCreateInfo PipelineColorBlendStateCreateInfo = {
		VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
		nullptr,
		0,
		VK_FALSE, VK_LOGIC_OP_COPY, //!< ブレンド時に論理オペレーションを行う (ブレンドは無効になる) (整数型アタッチメントに対してのみ)
		static_cast<uint32_t>(PipelineColorBlendAttachmentStates.size()), PipelineColorBlendAttachmentStates.data(),
		{ 1.0f, 1.0f, 1.0f, 1.0f }
	};

	//!< ダイナミックステート
	std::vector<VkDynamicState> DynamicStates;
	CreateDynamicState(DynamicStates);
	const VkPipelineDynamicStateCreateInfo PipelineDynamicStateCreateInfo = {
		VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
		nullptr,
		0,
		static_cast<uint32_t>(DynamicStates.size()), DynamicStates.data()
	};

	//!< パイプラインレイアウト
	const VkPipelineLayoutCreateInfo PipelineLayoutCreateInfo = {
		VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
		nullptr,
		0,
		static_cast<uint32_t>(DescriptorSetLayouts.size()), DescriptorSetLayouts.data(),
		static_cast<uint32_t>(PushConstantRanges.size()), PushConstantRanges.data()
	};
	VERIFY_SUCCEEDED(vkCreatePipelineLayout(Device, &PipelineLayoutCreateInfo, GetAllocationCallbacks(), &PipelineLayout));
	//!< 本来 PipelineLayout を作成したら、DescritptorSetLayout は破棄しても良い
	//!< ただし、同じレイアウトの DescriptorSet を再作成するような場合に必要になるのでここでは残しておくことにする

	/**
	@brief 継承
	basePipelineHandle, basePipelineIndex は同時に使用できない、それぞれ使用しない場合は VK_NULL_HANDLE, -1 を指定
	親には VK_PIPELINE_CREATE_ALLOW_DERIVATIVES_BIT フラグが必要、子には VK_PIPELINE_CREATE_DERIVATIVE_BIT フラグが必要
	・basePipelineHandle ... 既に親とするパイプラインが存在する場合に指定
	・basePipelineIndex ... 同配列内で親パイプラインも同時に作成する場合、配列内での親パイプラインの添字、親の添字の方が若い値でないといけない
	*/
	const std::vector<VkGraphicsPipelineCreateInfo> GraphicsPipelineCreateInfos = {
		{
			VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
			nullptr,
#ifdef _DEBUG
			VK_PIPELINE_CREATE_DISABLE_OPTIMIZATION_BIT,
#else
			0,
#endif
			static_cast<uint32_t>(PipelineShaderStageCreateInfos.size()), PipelineShaderStageCreateInfos.data(),
			&PipelineVertexInputStateCreateInfo,
			&PipelineInputAssemblyStateCreateInfo,
			&PipelineTessellationStateCreateInfo,
			&PipelineViewportStateCreateInfo,
			&PipelineRasterizationStateCreateInfo,
			&PipelineMultisampleStateCreateInfo,
			&PipelineDepthStencilStateCreateInfo,
			&PipelineColorBlendStateCreateInfo,
			&PipelineDynamicStateCreateInfo,
			PipelineLayout,
			RenderPass, 0, //!< 指定したレンダーパス限定ではなく、互換性のある他のレンダーパスでも使用可能
			VK_NULL_HANDLE, -1 //!< basePipelineHandle, basePipelineIndex
		}
	};

	//!< パイプラインキャッシュ (ファイルから読み込む、読み込めない場合は新規作成)
	const auto PipelineCache = VK_NULL_HANDLE;//LoadPipelineCache(GetBasePath() + L".pco");

	//!< (グラフィック)パイプライン
	//!< キャッシュがデータを含む場合はドライバはパイプライン作成時に使用する、またドライバはパイプライン作成結果をキャッシュする
	VERIFY_SUCCEEDED(vkCreateGraphicsPipelines(Device, 
		PipelineCache, 
		static_cast<uint32_t>(GraphicsPipelineCreateInfos.size()), GraphicsPipelineCreateInfos.data(), 
		GetAllocationCallbacks(), 
		&Pipeline));

	//!< パイプライン を作成したら、シェーダモジュール は破棄して良い
	for (auto i : ShaderModules) {
		vkDestroyShaderModule(Device, i, GetAllocationCallbacks());
	}
	ShaderModules.clear();

#if 0
	//!< パイプラインマージの例
	const std::vector<VkPipelineCache> SrcCaches = { PipelineCache };
	VkPipelineCache DstCache;
	VERIFY_SUCCEEDED(vkMergePipelineCaches(Device, DstCache, static_cast<uint32_t>(SrcCaches.size()), SrcCaches.data()));
#endif

#if 0
	//!< パイプラインキャッシュをファイルへ保存
	if (VK_NULL_HANDLE != PipelineCache) {
		const auto BasePath = GetBasePath();
		StorePipelineCache(BasePath + L".pco", PipelineCache);

		vkDestroyPipelineCache(Device, PipelineCache, GetAllocationCallbacks());
	}
#endif

#ifdef DEBUG_STDOUT
	std::cout << "CreatePipeline_Graphics" << COUT_OK << std::endl << std::endl;
#endif
}
void VK::CreatePipeline_Compute()
{
#ifdef _DEBUG
	PerformanceCounter PC("CreatePipeline_Compute : ");
#endif

	//!< シェーダモジュール
	const auto ShaderPath = GetBasePath();
	const auto ShaderModule = CreateShaderModule((ShaderPath + L".comp.spv").data());
	const char* EntrypointName = "main";
	const VkPipelineShaderStageCreateInfo PipelineShaderStageCreateInfo = {
			VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
			nullptr,
			0,
			VK_SHADER_STAGE_COMPUTE_BIT, ShaderModule,
			EntrypointName,
			nullptr //!< &SpecializationInfo
	};
	
	//!< パイプラインレイアウト
	const VkPipelineLayoutCreateInfo PipelineLayoutCreateInfo = {
		VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
		nullptr,
		0,
		static_cast<uint32_t>(DescriptorSetLayouts.size()), DescriptorSetLayouts.data(),
		static_cast<uint32_t>(PushConstantRanges.size()), PushConstantRanges.data()
	};
	VERIFY_SUCCEEDED(vkCreatePipelineLayout(Device, &PipelineLayoutCreateInfo, GetAllocationCallbacks(), &PipelineLayout));

	const std::vector<VkComputePipelineCreateInfo> ComputePipelineCreateInfos = {
		{
			VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
			nullptr,
#ifdef _DEBUG
			VK_PIPELINE_CREATE_DISABLE_OPTIMIZATION_BIT,
#else
			0,
#endif
			PipelineShaderStageCreateInfo,
			PipelineLayout,
			VK_NULL_HANDLE, -1 //!< basePipelineHandle, basePipelineIndex
		},
	};
	VERIFY_SUCCEEDED(vkCreateComputePipelines(Device,
		/*PipelineCache*/VK_NULL_HANDLE,
		static_cast<uint32_t>(ComputePipelineCreateInfos.size()), ComputePipelineCreateInfos.data(),
		GetAllocationCallbacks(),
		&Pipeline));

	vkDestroyShaderModule(Device, ShaderModule, GetAllocationCallbacks());

#ifdef DEBUG_STDOUT
	std::cout << "CreatePipeline_Compute" << COUT_OK << std::endl << std::endl;
#endif
}

/**
@brief クリア Clear
@note 「レンダーパス外」でクリア Out of renderpass ... vkCmdClearColorImage()
@note 「レンダーパス開始時」にクリア Begining of renderpass ... VkAttachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR, VkRenderPassBeginInfo.pClearValues
@note 「各々のサブパス」でクリア Each subpass ... vkCmdClearAttachments()
*/
//!< 「レンダーパス外」にてクリアを行う
void VK::ClearColor(const VkCommandBuffer CommandBuffer, const VkImage Image, const VkClearColorValue& Color)
{
	const VkImageMemoryBarrier ImageMemoryBarrier_PresentToTransfer = {
		VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
		nullptr,
		VK_ACCESS_MEMORY_READ_BIT,
		VK_ACCESS_TRANSFER_WRITE_BIT,
		VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		PresentQueueFamilyIndex,
		PresentQueueFamilyIndex,
		Image,
		ImageSubresourceRange_Color
	};
	vkCmdPipelineBarrier(CommandBuffer,
		VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
		0,
		0, nullptr,
		0, nullptr,
		1, &ImageMemoryBarrier_PresentToTransfer);
	{
		//!< vkCmdClearColorImage() はレンダーパス内では使用できない
		vkCmdClearColorImage(CommandBuffer, Image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &Color, 1, &ImageSubresourceRange_Color);
	}
	const VkImageMemoryBarrier ImageMemoryBarrier_TransferToPresent = {
		VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
		nullptr,
		VK_ACCESS_TRANSFER_WRITE_BIT,
		VK_ACCESS_MEMORY_READ_BIT,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
		PresentQueueFamilyIndex,
		PresentQueueFamilyIndex,
		Image,
		ImageSubresourceRange_Color
	};
	vkCmdPipelineBarrier(CommandBuffer,
		VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
		0,
		0, nullptr,
		0, nullptr,
		1, &ImageMemoryBarrier_TransferToPresent);
}
void VK::ClearDepthStencil(const VkCommandBuffer CommandBuffer, const VkImage Image, const VkClearDepthStencilValue& DepthStencil)
{
	const VkImageMemoryBarrier ImageMemoryBarrier_DepthToTransfer = {
		VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
		nullptr,
		VK_ACCESS_MEMORY_READ_BIT,
		VK_ACCESS_TRANSFER_WRITE_BIT,
		VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		PresentQueueFamilyIndex,
		PresentQueueFamilyIndex,
		Image,
		ImageSubresourceRange_DepthStencil
	};
	vkCmdPipelineBarrier(CommandBuffer,
		VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
		0,
		0, nullptr,
		0, nullptr,
		1, &ImageMemoryBarrier_DepthToTransfer);
	{
		//!< vkCmdClearDepthStencilImage() はレンダーパス内では使用できない
		vkCmdClearDepthStencilImage(CommandBuffer, Image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &ClearDepthStencilValue, 1, &ImageSubresourceRange_DepthStencil);
	}
	const VkImageMemoryBarrier ImageMemoryBarrier_TransferToDepth = {
		VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
		nullptr,
		VK_ACCESS_TRANSFER_WRITE_BIT,
		VK_ACCESS_MEMORY_READ_BIT,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
		PresentQueueFamilyIndex,
		PresentQueueFamilyIndex,
		Image,
		ImageSubresourceRange_DepthStencil
	};
	vkCmdPipelineBarrier(CommandBuffer,
		VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
		0,
		0, nullptr,
		0, nullptr,
		1, &ImageMemoryBarrier_TransferToDepth);
}
//!<「サブパス」にてクリアするときに使う
//!< Drawコール前に使用すると、「それなら VkAttachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR を使え」と Warnning が出るので注意
void VK::ClearColorAttachment(const VkCommandBuffer CommandBuffer, const VkClearColorValue& Color)
{
	const VkClearValue ClearValue = { Color };
	const std::vector<VkClearAttachment> ClearAttachments = {
		{
			VK_IMAGE_ASPECT_COLOR_BIT,
			0, //!< カラーアタッチメントのインデックス #VK_TODO 現状決め打ち
			ClearValue
		},
	};
	const std::vector<VkClearRect> ClearRects = {
		{
			ScissorRects[0],
			0, 1 //!< 開始レイヤとレイヤ数 #VK_TODO 現状決め打ち
		},
	};
	vkCmdClearAttachments(CommandBuffer,
		static_cast<uint32_t>(ClearAttachments.size()), ClearAttachments.data(),
		static_cast<uint32_t>(ClearRects.size()), ClearRects.data());
}
void VK::ClearDepthStencilAttachment(const VkCommandBuffer CommandBuffer, const VkClearDepthStencilValue& DepthStencil)
{
	VkClearValue ClearValue;
	ClearValue.depthStencil = DepthStencil;
	const std::vector<VkClearAttachment> ClearAttachments = {
		{
			VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT,
			0, //!< ここでは無視される
			ClearValue
		},
	};
	const std::vector<VkClearRect> ClearRects = {
		{
			ScissorRects[0],
			0, 1 //!< 開始レイヤとレイヤ数 #VK_TODO 現状決め打ち
		},
	};
	vkCmdClearAttachments(CommandBuffer,
		static_cast<uint32_t>(ClearAttachments.size()), ClearAttachments.data(),
		static_cast<uint32_t>(ClearRects.size()), ClearRects.data());
}

void VK::PopulateCommandBuffer(const size_t i)
{
	const auto CB = CommandBuffers[i];
	const auto SCB = SecondaryCommandBuffers[i];
	const auto FB = Framebuffers[i];
	const auto Image = SwapchainImages[i];

	/**
	@brief コマンドバッファのリセット Reset of command buffer
	* vkBeginCommandBuffer() で自動的にリセットされるので、明示的に vkResetCommandBuffer() をコールしなくても良い
	* ただし vkResetCommandBuffer() で明示的にリセットする場合には、メモリを開放するかどうかを指定できる
	*/
	//VERIFY_SUCCEEDED(vkResetCommandBuffer(CB, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT));
	VERIFY_SUCCEEDED(vkBeginCommandBuffer(CB, &CommandBufferBeginInfo)); {
		//!< ビューポート、シザー
		vkCmdSetViewport(CB, 0, static_cast<uint32_t>(Viewports.size()), Viewports.data());
		vkCmdSetScissor(CB, 0, static_cast<uint32_t>(ScissorRects.size()), ScissorRects.data());

#if 1
		//!< クリア
		ClearColor(CB, Image, Colors::SkyBlue);
#endif

#ifdef _DEBUG
		//!< レンダーエリアの最低粒度を確保
		VkExtent2D Granularity;
		vkGetRenderAreaGranularity(Device, RenderPass, &Granularity);
		//!<「自分の環境では」 Granularity = { 1, 1 } だったのでほぼなんでも大丈夫みたい、環境によっては注意が必要
		assert(ScissorRects[0].extent.width >= Granularity.width && ScissorRects[0].extent.height >= Granularity.height && "ScissorRect is too small");
#endif
		//!< (ここでは)レンダーパス開始時にカラーはクリアせず、デプスはクリアしている In this case, not clear color, but clear depth on begining of renderpas
		std::vector<VkClearValue> ClearValues(2);
		//ClearValues[0].color = Colors::SkyBlue; //!< If VkAttachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR, need this
		ClearValues[1].depthStencil = ClearDepthStencilValue;
		const VkRenderPassBeginInfo RenderPassBeginInfo = {
			VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
			nullptr,
			RenderPass,
			FB,
			ScissorRects[0], //!< フレームバッファのサイズ以下を指定できる
			static_cast<uint32_t>(ClearValues.size()), ClearValues.data()
		};

		vkCmdBeginRenderPass(CB, &RenderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE); {
			//vkCmdBindPipeline();
			//vkCmdBindDescriptorSets();
			//vkCmdBindVertexBuffers();
			//vkCmdBindIndexBuffer();
			//vkCmdDrawIndirect();

			//!< vkCmdNextSubpass(CommandBuffer, VK_SUBPASS_CONTENTS_INLINE);

			//if (!SCB.empty()) {
			//	vkCmdExecuteCommands(CB, static_cast<uint32_t>(SCB.size()), SCB.data());
			//}
		} vkCmdEndRenderPass(CB);
	} VERIFY_SUCCEEDED(vkEndCommandBuffer(CB));
}

void VK::Draw()
{
	WaitForFence();

	//!< 次のイメージが取得できたらセマフォが通知される
	VERIFY_SUCCEEDED(vkAcquireNextImageKHR(Device, Swapchain, UINT64_MAX, NextImageAcquiredSemaphore, VK_NULL_HANDLE, &SwapchainImageIndex));

	//!< デスクリプタセットを更新したら、コマンドバッファを記録し直さないとダメ？
	//UpdateDescriptorSet();

	/**
	@brief 各々のセマフォはパイプラインステージに関連付けられる
	コマンドは指定のパイプラインステージに到達するまで実行され、そこでセマフォがシグナルされるまで待つ
	*/
	const VkPipelineStageFlags PipelineStageFlags = VK_PIPELINE_STAGE_TRANSFER_BIT;
	const std::vector<VkSubmitInfo> SubmitInfos = {
		{
			VK_STRUCTURE_TYPE_SUBMIT_INFO,
			nullptr,
			1, &NextImageAcquiredSemaphore, &PipelineStageFlags,	//!< 次イメージが取得できる(プレゼント完了)までウエイト
			1, &CommandBuffers[SwapchainImageIndex],
			1, &RenderFinishedSemaphore								//!< 描画完了を通知する
		},
	};
	VERIFY_SUCCEEDED(vkQueueSubmit(GraphicsQueue, static_cast<uint32_t>(SubmitInfos.size()), SubmitInfos.data(), Fence));

	Present();
}
void VK::Dispatch()
{
	//!< #VK_TODO
}
void VK::Present()
{
	const VkPresentInfoKHR PresentInfo = {
		VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
		nullptr,
		1, &RenderFinishedSemaphore, //!< 描画が完了するまで待つ
		1, &Swapchain, &SwapchainImageIndex,
		nullptr
	};
	VERIFY_SUCCEEDED(vkQueuePresentKHR(GraphicsQueue, &PresentInfo));

#ifdef DEBUG_STDOUT
	//std::cout << "\t" << "SwapchainImageIndex = " << SwapchainImageIndex << std::endl;
#endif
}
void VK::WaitForFence()
{
	VkResult Result;
	const uint64_t TimeOut = 100000000;
	do {
		//!< リソース使用可能のシグナルまで待つ
		Result = vkWaitForFences(Device, 1, &Fence, VK_TRUE, TimeOut);
		if (VK_TIMEOUT == Result) {
#ifdef _DEBUG
			std::cout << "TIMEOUT" << std::endl;
#endif
		}
	} while (VK_SUCCESS != Result);
	VERIFY_SUCCEEDED(Result);

	//!< シグナルをリセット
	vkResetFences(Device, 1, &Fence);

#ifdef DEBUG_STDOUT
	//std::cout << "Fence" << std::endl;
#endif
}
