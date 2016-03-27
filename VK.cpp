#include "stdafx.h"

#include "VK.h"

#pragma comment(lib, "vulkan-1.lib")

VK::VK()
{
}
VK::~VK()
{
}

void VK::OnCreate(HWND hWnd, HINSTANCE hInstance)
{
	CreateInstance();
	CreateDevice();
	CreateSurface(hWnd, hInstance);
	CreateCommandPool();
	CreateSetupCommandBuffer();
	CreateSwapchain();
	CreateDrawCommandBuffers();
	CreateDepthStencil();
	CreatePipelineCache();
	CreateFramebuffers();

	FlushSetupCommandBuffer();

	CreateSetupCommandBuffer();
}
void VK::OnSize(HWND hWnd, HINSTANCE hInstance)
{
}
void VK::OnTimer(HWND hWnd, HINSTANCE hInstance)
{
}
void VK::OnPaint(HWND hWnd, HINSTANCE hInstance)
{
}
void VK::OnDestroy(HWND hWnd, HINSTANCE hInstance)
{
	for (auto i : Framebuffers) {
		vkDestroyFramebuffer(Device, i, nullptr);
	}

	vkDestroyPipelineCache(Device, PipelineCache, nullptr);

	vkDestroyRenderPass(Device, RenderPass, nullptr);

	vkDestroyImageView(Device, DepthStencil.ImageView, nullptr);
	vkDestroyImage(Device, DepthStencil.Image, nullptr);
	vkFreeMemory(Device, DepthStencil.DeviceMemory, nullptr);

	vkFreeCommandBuffers(Device, CommandPool, 1, &PostPresentCommandBuffer);
	vkFreeCommandBuffers(Device, CommandPool, 1, &PrePresentCommandBuffer);
	vkFreeCommandBuffers(Device, CommandPool, static_cast<uint32_t>(DrawCommandBuffers.size()), DrawCommandBuffers.data());

	for (auto& i : SwapchainBuffers) {
		vkDestroyImageView(Device, i.ImageView, nullptr);
	}
	vkDestroySwapchainKHR(Device, Swapchain, nullptr);

	if (SetupCommandBuffer != VK_NULL_HANDLE) {
		vkFreeCommandBuffers(Device, CommandPool, 1, &SetupCommandBuffer);
	}

	vkDestroyCommandPool(Device, CommandPool, nullptr);

	vkDestroySurfaceKHR(Instance, Surface, nullptr);

	vkDestroySemaphore(Device, RenderSemaphore, nullptr);
	vkDestroySemaphore(Device, PresentSemaphore, nullptr);
	vkDestroyDevice(Device, nullptr);
	
	vkDestroyInstance(Instance, nullptr);
}

void VK::CreateInstance()
{
#pragma region CreateInstance
	VkInstanceCreateInfo InstanceCreateInfo = {};
	InstanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	InstanceCreateInfo.pNext = nullptr;

	VkApplicationInfo ApplicationInfo = {};	{
		ApplicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		ApplicationInfo.pApplicationName = "VK";
		ApplicationInfo.pEngineName = "None";
		ApplicationInfo.apiVersion = VK_MAKE_VERSION(1, 0, 5);

		InstanceCreateInfo.pApplicationInfo = &ApplicationInfo;
	}

	const std::vector<const char*> EnabledExtensions = {
		VK_KHR_SURFACE_EXTENSION_NAME,
		VK_EXT_DEBUG_REPORT_EXTENSION_NAME,
#if defined(_WIN32)
		VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
#endif
	}; {
		InstanceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(EnabledExtensions.size());
		InstanceCreateInfo.ppEnabledExtensionNames = EnabledExtensions.data();
	}

	VERIFY_SUCCEEDED(vkCreateInstance(&InstanceCreateInfo, nullptr, &Instance));
#pragma endregion
}

VkBool32 VK::GetSupportedDepthFormat(VkFormat& Format)
{
	const std::vector<VkFormat> DepthFormats = {
		VK_FORMAT_D32_SFLOAT_S8_UINT,
		VK_FORMAT_D32_SFLOAT,
		VK_FORMAT_D24_UNORM_S8_UINT,
		VK_FORMAT_D16_UNORM_S8_UINT,
		VK_FORMAT_D16_UNORM
	};
	for (auto& f : DepthFormats) {
		VkFormatProperties FormatProperties;
		vkGetPhysicalDeviceFormatProperties(PhysicalDevice, f, &FormatProperties);
		if (FormatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) {
			Format = f;
			return true;
		}
	}
	return false;
}
void VK::CreateDevice()
{
	uint32_t PhysicalDeviceCount = 0;
	VERIFY_SUCCEEDED(vkEnumeratePhysicalDevices(Instance, &PhysicalDeviceCount, nullptr));
	if (PhysicalDeviceCount) {
		std::vector<VkPhysicalDevice> PhysicalDevices(PhysicalDeviceCount);
		VERIFY_SUCCEEDED(vkEnumeratePhysicalDevices(Instance, &PhysicalDeviceCount, PhysicalDevices.data()));

		//!< ここでは最初の物理デバイスを使用することにする
		PhysicalDevice = PhysicalDevices[0];

		uint32_t QueueCount;
		vkGetPhysicalDeviceQueueFamilyProperties(PhysicalDevice, &QueueCount, nullptr);
		if (QueueCount) {
			std::vector<VkQueueFamilyProperties> QueueProperties(QueueCount);
			vkGetPhysicalDeviceQueueFamilyProperties(PhysicalDevice, &QueueCount, QueueProperties.data());
			for (uint32_t i = 0; i < QueueCount; ++i) {
				//!< 描画機能のある最初のキューを探す
				if (QueueProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
					//!< デバイスの作成
#pragma region CreateDevice
					VkDeviceCreateInfo DeviceCreateInfo = {};
					DeviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
					DeviceCreateInfo.pNext = nullptr;
					DeviceCreateInfo.pEnabledFeatures = nullptr;

					VkDeviceQueueCreateInfo QueueCreateInfo = {}; 
					const std::vector<float> QueuePriorities = { 0.0f }; {
						QueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
						QueueCreateInfo.queueFamilyIndex = i;
					
						{
							QueueCreateInfo.queueCount = static_cast<uint32_t>(QueuePriorities.size());
							QueueCreateInfo.pQueuePriorities = QueuePriorities.data();
						}
						
						DeviceCreateInfo.queueCreateInfoCount = 1;
						DeviceCreateInfo.pQueueCreateInfos = &QueueCreateInfo;
					}

					const std::vector<const char*> EnabledExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME }; {
						DeviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(EnabledExtensions.size());
						DeviceCreateInfo.ppEnabledExtensionNames = EnabledExtensions.data();
					}

					VERIFY_SUCCEEDED(vkCreateDevice(PhysicalDevice, &DeviceCreateInfo, nullptr, &Device));

					vkGetPhysicalDeviceProperties(PhysicalDevice, &PhysicalDeviceProperties);
					vkGetPhysicalDeviceMemoryProperties(PhysicalDevice, &PhysicalDeviceMemoryProperties);
					vkGetDeviceQueue(Device, i, 0, &Queue); 
#pragma endregion

					//!< デプスフォーマットの取得
					VERIFY(GetSupportedDepthFormat(DepthFormat));

					//!< セマフォの作成
#pragma region CreateSemaphore
					VkSemaphoreCreateInfo SemaphoreCreateInfo = {};
					SemaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
					SemaphoreCreateInfo.pNext = nullptr;
					SemaphoreCreateInfo.flags = 0;

					VERIFY_SUCCEEDED(vkCreateSemaphore(Device, &SemaphoreCreateInfo, nullptr, &PresentSemaphore));
					VERIFY_SUCCEEDED(vkCreateSemaphore(Device, &SemaphoreCreateInfo, nullptr, &RenderSemaphore));
#pragma endregion
					break;
				}
			}
		}
	}
}

void VK::CreateSurface(HWND hWnd, HINSTANCE hInstance)
{
#if defined(_WIN32)
#pragma region CreateSurface
	VkWin32SurfaceCreateInfoKHR SurfaceCreateInfo = {};
	SurfaceCreateInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	SurfaceCreateInfo.hwnd = hWnd;
	SurfaceCreateInfo.hinstance = hInstance;
	VERIFY_SUCCEEDED(vkCreateWin32SurfaceKHR(Instance, &SurfaceCreateInfo, nullptr, &Surface));
#pragma endregion
#endif

	uint32_t QueueCount;
	vkGetPhysicalDeviceQueueFamilyProperties(PhysicalDevice, &QueueCount, nullptr);
	if (QueueCount) {
		//!< プレゼントをサポートするキューを列挙
		std::vector<VkBool32> SupportsPresent(QueueCount);
		for (uint32_t i = 0; i < QueueCount; ++i) {
			vkGetPhysicalDeviceSurfaceSupportKHR(PhysicalDevice, i, Surface, &SupportsPresent[i]);
		}
		std::vector<VkQueueFamilyProperties> QueueProperties(QueueCount);
		vkGetPhysicalDeviceQueueFamilyProperties(PhysicalDevice, &QueueCount, QueueProperties.data());

		//!< グラフィックとプレゼントをサポートするキューを探す
#pragma region CreateQueueNodeIndex
		uint32_t GraphicsQueueNodeIndex = UINT32_MAX;
		uint32_t PresentQueueNodeIndex = UINT32_MAX;
		for (uint32_t i = 0; i < QueueCount; i++) {
			if (QueueProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
				if (GraphicsQueueNodeIndex == UINT32_MAX) {
					GraphicsQueueNodeIndex = i;
				}
				if (SupportsPresent[i] == VK_TRUE) {
					GraphicsQueueNodeIndex = i;
					PresentQueueNodeIndex = i;
					break;
				}
			}
		}
		if (PresentQueueNodeIndex == UINT32_MAX) {
			//!< 両方を満たすものがないのでプレゼント単品で満たすものを探す
			for (uint32_t i = 0; i < QueueCount; ++i) {
				if (SupportsPresent[i] == VK_TRUE) {
					PresentQueueNodeIndex = i;
					break;
				}
			}
		}
		VERIFY(GraphicsQueueNodeIndex != UINT32_MAX && PresentQueueNodeIndex != UINT32_MAX);
		VERIFY(GraphicsQueueNodeIndex == PresentQueueNodeIndex);

		QueueNodeIndex = GraphicsQueueNodeIndex;
#pragma endregion

#pragma region CreateColorFormatColorSpace
		uint32_t FormatCount;
		VERIFY_SUCCEEDED(vkGetPhysicalDeviceSurfaceFormatsKHR(PhysicalDevice, Surface, &FormatCount, nullptr));
		if (FormatCount) {
			std::vector<VkSurfaceFormatKHR> SurfaceFormats(FormatCount);
			VERIFY_SUCCEEDED(vkGetPhysicalDeviceSurfaceFormatsKHR(PhysicalDevice, Surface, &FormatCount, SurfaceFormats.data()));
		
			ColorFormat = SurfaceFormats[0].format == VK_FORMAT_UNDEFINED ? VK_FORMAT_B8G8R8A8_UNORM : SurfaceFormats[0].format;
			ColorSpace = SurfaceFormats[0].colorSpace;
		}
#pragma endregion
	}
}

void VK::CreateCommandPool()
{
	VkCommandPoolCreateInfo CommandPoolInfo = {};
	CommandPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	CommandPoolInfo.queueFamilyIndex = QueueNodeIndex;
	CommandPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	VERIFY_SUCCEEDED(vkCreateCommandPool(Device, &CommandPoolInfo, nullptr, &CommandPool));
}

void VK::CreateSetupCommandBuffer()
{
	if (SetupCommandBuffer != VK_NULL_HANDLE) {
		vkFreeCommandBuffers(Device, CommandPool, 1, &SetupCommandBuffer);
		SetupCommandBuffer = VK_NULL_HANDLE;
	}

	VkCommandBufferAllocateInfo CommandBufferAllocateInfo = {};
	CommandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	CommandBufferAllocateInfo.commandPool = CommandPool;
	CommandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	CommandBufferAllocateInfo.commandBufferCount = 1;
	VERIFY_SUCCEEDED(vkAllocateCommandBuffers(Device, &CommandBufferAllocateInfo, &SetupCommandBuffer));

	VkCommandBufferBeginInfo CommandBufferBeginInfo = {};
	CommandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	VERIFY_SUCCEEDED(vkBeginCommandBuffer(SetupCommandBuffer, &CommandBufferBeginInfo));
}

void VK::SetImageLayout(VkCommandBuffer CommandBuffer, VkImage Image, VkImageAspectFlags ImageAspectFlags, VkImageLayout OldImageLayout, VkImageLayout NewImageLayout, VkImageSubresourceRange ImageSubresourceRange)
{
	VkImageMemoryBarrier ImageMemoryBarrier = {};
	ImageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	ImageMemoryBarrier.pNext = NULL;
	ImageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	ImageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	ImageMemoryBarrier.oldLayout = OldImageLayout;
	ImageMemoryBarrier.newLayout = NewImageLayout;
	ImageMemoryBarrier.image = Image;
	ImageMemoryBarrier.subresourceRange = ImageSubresourceRange;

	if (OldImageLayout == VK_IMAGE_LAYOUT_PREINITIALIZED) {
		ImageMemoryBarrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT;
	}
	if (OldImageLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) {
		ImageMemoryBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	}
	if (OldImageLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
		ImageMemoryBarrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
	}
	if (OldImageLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL) {
		ImageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
	}
	if (OldImageLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
		ImageMemoryBarrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
	}

	if (NewImageLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
		ImageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	}
	if (NewImageLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL) {
		ImageMemoryBarrier.srcAccessMask = ImageMemoryBarrier.srcAccessMask | VK_ACCESS_TRANSFER_READ_BIT;
		ImageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
	}
	if (NewImageLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)	{
		ImageMemoryBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		ImageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
	}
	if (NewImageLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
		ImageMemoryBarrier.dstAccessMask = ImageMemoryBarrier.dstAccessMask | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
	}
	if (NewImageLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
		ImageMemoryBarrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT;
		ImageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
	}

	VkPipelineStageFlags SrcPipelineStageFlags = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
	VkPipelineStageFlags DstPipelineStageFlags = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
	vkCmdPipelineBarrier(CommandBuffer, SrcPipelineStageFlags, DstPipelineStageFlags, 0, 0, nullptr, 0, nullptr, 1, &ImageMemoryBarrier);
}
void VK::SetImageLayout(VkCommandBuffer CommandBuffer, VkImage Image, VkImageAspectFlags ImageAspectFlags, VkImageLayout OldImageLayout, VkImageLayout NewImageLayout)
{
	VkImageSubresourceRange ImageSubresourceRange = {};
	ImageSubresourceRange.aspectMask = ImageAspectFlags;
	ImageSubresourceRange.baseMipLevel = 0;
	ImageSubresourceRange.levelCount = 1;
	ImageSubresourceRange.layerCount = 1;
	SetImageLayout(CommandBuffer, Image, ImageAspectFlags, OldImageLayout, NewImageLayout, ImageSubresourceRange);
}

void VK::CreateSwapchain()
{
	VkSwapchainKHR OldSwapchain = Swapchain;

#pragma region CreateSwapchain
	VkSwapchainCreateInfoKHR SwapchainCreateInfo = {};
	SwapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	SwapchainCreateInfo.pNext = nullptr;
	SwapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	SwapchainCreateInfo.imageArrayLayers = 1;
	SwapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	SwapchainCreateInfo.queueFamilyIndexCount = 0;
	SwapchainCreateInfo.pQueueFamilyIndices = nullptr;
	SwapchainCreateInfo.clipped = true;
	SwapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	SwapchainCreateInfo.surface = Surface;
	SwapchainCreateInfo.imageFormat = ColorFormat;
	SwapchainCreateInfo.imageColorSpace = ColorSpace;
	SwapchainCreateInfo.oldSwapchain = OldSwapchain;
	{
		VkSurfaceCapabilitiesKHR SurfaceCapabilities;
		VERIFY_SUCCEEDED(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(PhysicalDevice, Surface, &SurfaceCapabilities));

		SwapchainCreateInfo.minImageCount = (std::min)(SurfaceCapabilities.minImageCount + 1, SurfaceCapabilities.maxImageCount);
		SwapchainCreateInfo.preTransform = (SurfaceCapabilities.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR) ? VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR : SurfaceCapabilities.currentTransform;;

		VkExtent2D Extent2D = {};
		if (SurfaceCapabilities.currentExtent.width == -1) {
			Extent2D.width = 1280;
			Extent2D.height = 720;
		}
		else {
			Extent2D = SurfaceCapabilities.currentExtent;
		}
		SwapchainCreateInfo.imageExtent = { Extent2D.width, Extent2D.height };
	}
	{
		uint32_t PresentModeCount;
		VERIFY_SUCCEEDED(vkGetPhysicalDeviceSurfacePresentModesKHR(PhysicalDevice, Surface, &PresentModeCount, nullptr));
		if (PresentModeCount) {
			std::vector<VkPresentModeKHR> PresentModes(PresentModeCount);
			VERIFY_SUCCEEDED(vkGetPhysicalDeviceSurfacePresentModesKHR(PhysicalDevice, Surface, &PresentModeCount, PresentModes.data()));
			VkPresentModeKHR PresentMode = VK_PRESENT_MODE_FIFO_KHR;
			for (size_t i = 0; i < PresentModeCount; i++) {
				if (PresentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR) {
					PresentMode = VK_PRESENT_MODE_MAILBOX_KHR;
					break;
				}
				if ((PresentMode != VK_PRESENT_MODE_MAILBOX_KHR) && (PresentModes[i] == VK_PRESENT_MODE_IMMEDIATE_KHR)) {
					PresentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
				}
			}

			SwapchainCreateInfo.presentMode = PresentMode;
		}
	}
	VERIFY_SUCCEEDED(vkCreateSwapchainKHR(Device, &SwapchainCreateInfo, nullptr, &Swapchain));
#pragma endregion

	if (OldSwapchain != VK_NULL_HANDLE) {
		vkDestroySwapchainKHR(Device, OldSwapchain, nullptr);
	}

#pragma region CreateImageView
	uint32_t ImageCount;
	VERIFY_SUCCEEDED(vkGetSwapchainImagesKHR(Device, Swapchain, &ImageCount, nullptr));
	if (ImageCount) {
		std::vector<VkImage> Images(ImageCount);
		VERIFY_SUCCEEDED(vkGetSwapchainImagesKHR(Device, Swapchain, &ImageCount, Images.data()));

		VkImageViewCreateInfo ImageViewCreateInfo = {};
		ImageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		ImageViewCreateInfo.pNext = nullptr;
		ImageViewCreateInfo.components = {
			VK_COMPONENT_SWIZZLE_R,
			VK_COMPONENT_SWIZZLE_G,
			VK_COMPONENT_SWIZZLE_B,
			VK_COMPONENT_SWIZZLE_A
		};
		ImageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		ImageViewCreateInfo.subresourceRange.baseMipLevel = 0;
		ImageViewCreateInfo.subresourceRange.levelCount = 1;
		ImageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
		ImageViewCreateInfo.subresourceRange.layerCount = 1;
		ImageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		ImageViewCreateInfo.flags = 0;
		ImageViewCreateInfo.format = ColorFormat;
		
		SwapchainBuffers.resize(ImageCount);
		for (uint32_t i = 0; i < ImageCount; i++) {
			SwapchainBuffers[i].Image = Images[i];
			SetImageLayout(SetupCommandBuffer,
				SwapchainBuffers[i].Image,
				VK_IMAGE_ASPECT_COLOR_BIT,
				VK_IMAGE_LAYOUT_UNDEFINED,
				VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
			ImageViewCreateInfo.image = SwapchainBuffers[i].Image;
			VERIFY_SUCCEEDED(vkCreateImageView(Device, &ImageViewCreateInfo, nullptr, &SwapchainBuffers[i].ImageView));
		}
	}
#pragma endregion
}

void VK::CreateDrawCommandBuffers()
{
	DrawCommandBuffers.resize(SwapchainBuffers.size()/*ImageCount*/);

	VkCommandBufferAllocateInfo CommandBufferAllocateInfo = {};
	CommandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	CommandBufferAllocateInfo.commandPool = CommandPool;
	CommandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	CommandBufferAllocateInfo.commandBufferCount = static_cast<uint32_t>(DrawCommandBuffers.size());
	VERIFY_SUCCEEDED(vkAllocateCommandBuffers(Device, &CommandBufferAllocateInfo, DrawCommandBuffers.data()));

	CommandBufferAllocateInfo.commandBufferCount = 1;
	VERIFY_SUCCEEDED(vkAllocateCommandBuffers(Device, &CommandBufferAllocateInfo, &PrePresentCommandBuffer));
	VERIFY_SUCCEEDED(vkAllocateCommandBuffers(Device, &CommandBufferAllocateInfo, &PostPresentCommandBuffer));
}

VkBool32 VK::GetMemoryType(uint32_t TypeBits, VkFlags Properties, uint32_t* TypeIndex)
{
	for (uint32_t i = 0; i < 32; i++) {
		if ((TypeBits & 1) == 1) {
			if ((PhysicalDeviceMemoryProperties.memoryTypes[i].propertyFlags & Properties) == Properties) {
				*TypeIndex = i;
				return true;
			}
		}
		TypeBits >>= 1;
	}
	return false;
}
void VK::CreateDepthStencil()
{
#pragma region CreateImageView
	VkImageViewCreateInfo ImageViewCreateInfo = {};
	ImageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	ImageViewCreateInfo.pNext = nullptr;
	ImageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	ImageViewCreateInfo.flags = 0;
	ImageViewCreateInfo.subresourceRange = {};
	ImageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
	ImageViewCreateInfo.subresourceRange.baseMipLevel = 0;
	ImageViewCreateInfo.subresourceRange.levelCount = 1;
	ImageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
	ImageViewCreateInfo.subresourceRange.layerCount = 1;
	ImageViewCreateInfo.format = DepthFormat;
#pragma region AllocateMemory
	{
		VkMemoryAllocateInfo MemroyAllocateInfo = {};
		MemroyAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		MemroyAllocateInfo.pNext = nullptr;
		MemroyAllocateInfo.allocationSize = 0;
		MemroyAllocateInfo.memoryTypeIndex = 0;
		{
			VkMemoryRequirements MemoryRequirements;
#pragma region CreateImage
			{
				VkImageCreateInfo ImageCreateInfo = {};
				ImageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
				ImageCreateInfo.pNext = nullptr;
				ImageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
				ImageCreateInfo.mipLevels = 1;
				ImageCreateInfo.arrayLayers = 1;
				ImageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
				ImageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
				ImageCreateInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
				ImageCreateInfo.flags = 0;
				ImageCreateInfo.format = DepthFormat;
				//ImageCreateInfo.extent = { width, height, 1 }; todo
				ImageCreateInfo.extent = { 1280, 720, 1 };

				VERIFY_SUCCEEDED(vkCreateImage(Device, &ImageCreateInfo, nullptr, &DepthStencil.Image));
			}
#pragma endregion
			vkGetImageMemoryRequirements(Device, DepthStencil.Image, &MemoryRequirements);

			MemroyAllocateInfo.allocationSize = MemoryRequirements.size;
			GetMemoryType(MemoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &MemroyAllocateInfo.memoryTypeIndex);
		}

		VERIFY_SUCCEEDED(vkAllocateMemory(Device, &MemroyAllocateInfo, nullptr, &DepthStencil.DeviceMemory));
	}
#pragma endregion

	VERIFY_SUCCEEDED(vkBindImageMemory(Device, DepthStencil.Image, DepthStencil.DeviceMemory, 0));
	SetImageLayout(SetupCommandBuffer,
		DepthStencil.Image,
		VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT,
		VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
	ImageViewCreateInfo.image = DepthStencil.Image;

	VERIFY_SUCCEEDED(vkCreateImageView(Device, &ImageViewCreateInfo, nullptr, &DepthStencil.ImageView));
#pragma endregion
}

void VK::CreateRenderPass()
{
	VkRenderPassCreateInfo RenderPassCreateInfo = {};
	RenderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	RenderPassCreateInfo.pNext = nullptr;
	RenderPassCreateInfo.subpassCount = 1;
	RenderPassCreateInfo.dependencyCount = 0;
	RenderPassCreateInfo.pDependencies = nullptr;
#pragma region AttachmentDescriptions
	std::vector<VkAttachmentDescription> AttachmentDescriptions(2);	{
		AttachmentDescriptions[0].samples = VK_SAMPLE_COUNT_1_BIT;
		AttachmentDescriptions[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		AttachmentDescriptions[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		AttachmentDescriptions[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		AttachmentDescriptions[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		AttachmentDescriptions[0].initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		AttachmentDescriptions[0].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		AttachmentDescriptions[0].format = ColorFormat;

		AttachmentDescriptions[1].samples = VK_SAMPLE_COUNT_1_BIT;
		AttachmentDescriptions[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		AttachmentDescriptions[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		AttachmentDescriptions[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		AttachmentDescriptions[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		AttachmentDescriptions[1].initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		AttachmentDescriptions[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		AttachmentDescriptions[1].format = DepthFormat;
		RenderPassCreateInfo.attachmentCount = static_cast<uint32_t>(AttachmentDescriptions.size());
		RenderPassCreateInfo.pAttachments = AttachmentDescriptions.data();
	}
#pragma endregion
#pragma region Subpass
	VkSubpassDescription SubpassDescription = {};
	VkAttachmentReference ColorAttachmentReference = {}; 
	VkAttachmentReference DepthAttachmentReference = {}; {
		SubpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		SubpassDescription.flags = 0;
		SubpassDescription.inputAttachmentCount = 0;
		SubpassDescription.pInputAttachments = nullptr;
		SubpassDescription.pResolveAttachments = nullptr;
		SubpassDescription.preserveAttachmentCount = 0;
		SubpassDescription.pPreserveAttachments = nullptr;
		{
			ColorAttachmentReference.attachment = 0;
			ColorAttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			
			SubpassDescription.colorAttachmentCount = 1;
			SubpassDescription.pColorAttachments = &ColorAttachmentReference;
		}
		{
			DepthAttachmentReference.attachment = 1;
			DepthAttachmentReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

			SubpassDescription.pDepthStencilAttachment = &DepthAttachmentReference;
		}
		RenderPassCreateInfo.pSubpasses = &SubpassDescription;
	}
#pragma endregion
	VERIFY_SUCCEEDED(vkCreateRenderPass(Device, &RenderPassCreateInfo, nullptr, &RenderPass));
}

void VK::CreatePipelineCache()
{
	VkPipelineCacheCreateInfo PipelineCacheCreateInfo = {};
	PipelineCacheCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
	VERIFY_SUCCEEDED(vkCreatePipelineCache(Device, &PipelineCacheCreateInfo, nullptr, &PipelineCache));
}

void VK::CreateFramebuffers()
{
	VkFramebufferCreateInfo FrameBufferCreateInfo = {};
	FrameBufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	FrameBufferCreateInfo.pNext = nullptr;
	FrameBufferCreateInfo.width = 1280;//width; todo
	FrameBufferCreateInfo.height = 720;//height; todo
	FrameBufferCreateInfo.layers = 1;
	FrameBufferCreateInfo.renderPass = RenderPass;
	std::vector<VkImageView> ImageViews(2); {
		ImageViews[1] = DepthStencil.ImageView;

		FrameBufferCreateInfo.attachmentCount = static_cast<uint32_t>(ImageViews.size());
		FrameBufferCreateInfo.pAttachments = ImageViews.data();
	}
	Framebuffers.resize(SwapchainBuffers.size()/*ImageCount*/);
	for (uint32_t i = 0; i < Framebuffers.size(); ++i) {
		ImageViews[0] = SwapchainBuffers[i].ImageView;
		VERIFY_SUCCEEDED(vkCreateFramebuffer(Device, &FrameBufferCreateInfo, nullptr, &Framebuffers[i]));
	}
}

void VK::FlushSetupCommandBuffer()
{
	if (SetupCommandBuffer != VK_NULL_HANDLE) {
		VERIFY_SUCCEEDED(vkEndCommandBuffer(SetupCommandBuffer));

		VkSubmitInfo SubmitInfo = {};
		SubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		SubmitInfo.commandBufferCount = 1;
		SubmitInfo.pCommandBuffers = &SetupCommandBuffer;

		VERIFY_SUCCEEDED(vkQueueSubmit(Queue, 1, &SubmitInfo, VK_NULL_HANDLE));

		VERIFY_SUCCEEDED(vkQueueWaitIdle(Queue));

		vkFreeCommandBuffers(Device, CommandPool, 1, &SetupCommandBuffer);
		SetupCommandBuffer = VK_NULL_HANDLE;
	}
}
