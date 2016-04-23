#include "stdafx.h"

#include <fstream>

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
	Super::OnCreate(hWnd, hInstance);

	CreateInstance();
	
	auto PhysicalDevice = CreateDevice(); 
	
	const auto ColorFomat = VK_FORMAT_B8G8R8A8_UNORM;
	CreateSwapchain(hWnd, hInstance, PhysicalDevice, ColorFormat);

	VkPhysicalDeviceMemoryProperties PhysicalDeviceMemoryProperties;
	vkGetPhysicalDeviceMemoryProperties(PhysicalDevice, &PhysicalDeviceMemoryProperties);	
	const auto DepthFormat = GetSupportedDepthFormat(PhysicalDevice);
	CreateDepthStencil(PhysicalDeviceMemoryProperties, DepthFormat);

	CreateRenderPass(ColorFormat, DepthFormat);

	CreateFramebuffers();

	CreateShader();
	
	CreateDescriptorSetLayout();
	CreatePipelineLayout();

	CreateVertexInput();
	CreateViewport();
	CreatePipeline();

	//------------------------------------------

	CreateSemaphore();

	CreateCommandPool();
	CreateSetupCommandBuffer();
	
	CreateDescriptorSet();

	CreateCommandBuffers();
	
	CreateUniformBuffer();
	
	FlushSetupCommandBuffer();
	CreateSetupCommandBuffer();
	CreateVertexBuffer();
	CreateIndexBuffer();

	CreateFence();

	PopulateCommandBuffer();
}
void VK::OnSize(HWND hWnd, HINSTANCE hInstance)
{
	Super::OnSize(hWnd, hInstance);
}
void VK::OnTimer(HWND hWnd, HINSTANCE hInstance)
{
	Super::OnTimer(hWnd, hInstance);
}
void VK::OnPaint(HWND hWnd, HINSTANCE hInstance)
{
	Super::OnPaint(hWnd, hInstance);

	//PopulateCommandBuffer();

	ExecuteCommandBuffer();

	Present();
	
	//WaitForFence(); todo : fence と barrier の使い分け
}
void VK::OnDestroy(HWND hWnd, HINSTANCE hInstance)
{
	Super::OnDestroy(hWnd, hInstance);

	vkDestroyFence(Device, Fence, nullptr);

	vkFreeMemory(Device, IndexDeviceMemory, nullptr);
	vkDestroyBuffer(Device, IndexBuffer, nullptr);

	vkFreeMemory(Device, VertexDeviceMemory, nullptr);
	vkDestroyBuffer(Device, VertexBuffer, nullptr);

	vkFreeMemory(Device, UniformDeviceMemory, nullptr);
	vkDestroyBuffer(Device, UniformBuffer, nullptr);

	//for (auto& i : ShaderStageCreateInfos) {
	//	vkDestroyShaderModule(Device, i.module, nullptr);
	//}

	vkFreeCommandBuffers(Device, CommandPool, 1, &PostPresentCommandBuffer);
	vkFreeCommandBuffers(Device, CommandPool, 1, &PrePresentCommandBuffer);
	vkFreeCommandBuffers(Device, CommandPool, static_cast<uint32_t>(CommandBuffers.size()), CommandBuffers.data());


	vkDestroyPipeline(Device, Pipeline, nullptr);
	vkDestroyPipelineCache(Device, PipelineCache, nullptr);

	vkDestroyPipelineLayout(Device, PipelineLayout, nullptr);
	
	vkFreeDescriptorSets(Device, DescriptorPool, static_cast<uint32_t>(DescriptorSets.size()), DescriptorSets.data());
	vkDestroyDescriptorPool(Device, DescriptorPool, nullptr);
	
	if (SetupCommandBuffer != VK_NULL_HANDLE) {
		vkFreeCommandBuffers(Device, CommandPool, 1, &SetupCommandBuffer);
	}

	vkDestroyCommandPool(Device, CommandPool, nullptr);

	vkDestroySemaphore(Device, RenderSemaphore, nullptr);
	vkDestroySemaphore(Device, PresentSemaphore, nullptr);

#pragma region DescriptorSetLayout
	for (auto i : DescriptorSetLayouts) {
		vkDestroyDescriptorSetLayout(Device, i, nullptr);
	}
#pragma endregion

#pragma region Shader
	for (auto i : ShaderModules) {
		vkDestroyShaderModule(Device, i, nullptr);
	}
#pragma endregion

#pragma region Framebuffer
	for (auto i : Framebuffers) {
		vkDestroyFramebuffer(Device, i, nullptr);
	}
#pragma endregion
	
#pragma region RenderPass
	vkDestroyRenderPass(Device, RenderPass, nullptr);
#pragma endregion

#pragma region DepthStencil
	vkDestroyImageView(Device, DepthStencilImageView, nullptr);
	vkFreeMemory(Device, DepthStencilDeviceMemory, nullptr);
	vkDestroyImage(Device, DepthStencilImage, nullptr);
#pragma endregion

#pragma region Swapchain
	vkDestroySemaphore(Device, Semaphore, nullptr);
	for (auto i : SwapchainImageViews) {
		vkDestroyImageView(Device, i, nullptr);
	}
	for (auto i : SwapchainImages) {
		vkDestroyImage(Device, i, nullptr);
	}
	vkDestroySwapchainKHR(Device, Swapchain, nullptr);
#pragma endregion

	vkDestroyDevice(Device, nullptr);
	
#pragma region Instance
	//vkDestroyInstance(Instance, &AllocationCallbacks);
	vkDestroyInstance(Instance, nullptr);
#pragma endregion
}

VkFormat VK::GetSupportedDepthFormat(VkPhysicalDevice PhysicalDevice) const
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
	assert(false && "DepthFormat not found");
	return VK_FORMAT_UNDEFINED;
}
uint32_t VK::GetMemoryType(const VkPhysicalDeviceMemoryProperties& PhysicalDeviceMemoryProperties, uint32_t TypeBits, VkFlags Properties) const
{
	for (auto i = 0; i < 32; ++i) {
		if (TypeBits & 1) {
			if ((PhysicalDeviceMemoryProperties.memoryTypes[i].propertyFlags & Properties) == Properties) {
				return i;
			}
		}
		TypeBits >>= 1;
	}
	assert(false && "MemoryType not found");
	return 0;
}
void VK::SetImageLayout(VkCommandBuffer CommandBuffer, VkImage Image, VkImageAspectFlags ImageAspectFlags, VkImageLayout OldImageLayout, VkImageLayout NewImageLayout, VkImageSubresourceRange ImageSubresourceRange) const
{
	VkAccessFlags SrcAccessMask = 0;
	VkAccessFlags DstAccessMask = 0;

	if (OldImageLayout == VK_IMAGE_LAYOUT_PREINITIALIZED) {
		SrcAccessMask = VK_ACCESS_HOST_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT;
	}
	if (OldImageLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) {
		SrcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	}
	if (OldImageLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
		SrcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
	}
	if (OldImageLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL) {
		SrcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
	}
	if (OldImageLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
		SrcAccessMask = VK_ACCESS_SHADER_READ_BIT;
	}

	if (NewImageLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
		DstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	}
	if (NewImageLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL) {
		SrcAccessMask |= VK_ACCESS_TRANSFER_READ_BIT;
		DstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
	}
	if (NewImageLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) {
		DstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		SrcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
	}
	if (NewImageLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
		DstAccessMask |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
	}
	if (NewImageLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
		SrcAccessMask = VK_ACCESS_HOST_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT;
		DstAccessMask = VK_ACCESS_SHADER_READ_BIT;
	}

	const VkImageMemoryBarrier ImageMemoryBarrier = {
		VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
		nullptr,
		SrcAccessMask,
		DstAccessMask,
		OldImageLayout,
		NewImageLayout,
		VK_QUEUE_FAMILY_IGNORED,
		VK_QUEUE_FAMILY_IGNORED,
		Image,
		ImageSubresourceRange
	};
	VkPipelineStageFlags SrcPipelineStageFlags = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
	VkPipelineStageFlags DstPipelineStageFlags = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
	vkCmdPipelineBarrier(CommandBuffer, SrcPipelineStageFlags, DstPipelineStageFlags, 0, 0, nullptr, 0, nullptr, 1, &ImageMemoryBarrier);
}
void VK::SetImageLayout(VkCommandBuffer CommandBuffer, VkImage Image, VkImageAspectFlags ImageAspectFlags, VkImageLayout OldImageLayout, VkImageLayout NewImageLayout) const
{
	const VkImageSubresourceRange ImageSubresourceRange = {
		ImageAspectFlags,
		0, 1,
		0, 1
	};
	SetImageLayout(CommandBuffer, Image, ImageAspectFlags, OldImageLayout, NewImageLayout, ImageSubresourceRange);
}
/**
@brief glslangValidator.exe を用いてコンパイルする。
(環境変数 Path が通っているのでフルパスを指定せずに使用可能)
Build Event - Post-Build Event に以下のように記述してある
for %%1 in (*.vert, *.tesc, *.tese, *.geom, *.frag, *.comp) do glslangValidator -V %%1 -o %%1.spv
*/
VkShaderModule VK::CreateShaderModule(const std::string& Path) const
{
	VkShaderModule ShaderModule = VK_NULL_HANDLE;

	std::ifstream In(Path.c_str(), std::ios::in | std::ios::binary);
	assert(false == In.fail());

	In.seekg(0, std::ios_base::end);
	const auto CodeSize = In.tellg();
	In.seekg(0, std::ios_base::beg);

	auto Code = new char[CodeSize];
	In.read(Code, CodeSize);
	In.close();

	const VkShaderModuleCreateInfo ModuleCreateInfo = {
		VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
		nullptr,
		0,
		CodeSize, reinterpret_cast<uint32_t*>(Code)
	};
	VERIFY_SUCCEEDED(vkCreateShaderModule(Device, &ModuleCreateInfo, nullptr, &ShaderModule));

	delete[] Code;

	assert(VK_NULL_HANDLE != ShaderModule);
	return ShaderModule;
}
void VK::CreateInstance()
{
	const VkApplicationInfo ApplicationInfo = {
		VK_STRUCTURE_TYPE_APPLICATION_INFO,
		nullptr,
		"ApplicationName", 0,
		"EngineName", 0,
		VK_API_VERSION
	};
	const std::vector<const char*> EnabledExtensions = {
		VK_KHR_SURFACE_EXTENSION_NAME
#ifdef _DEBUG
		, VK_EXT_DEBUG_REPORT_EXTENSION_NAME
#endif
	};
	const VkInstanceCreateInfo InstanceCreateInfo = {
		VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
		nullptr,
		0,
		&ApplicationInfo,
#ifdef _DEBUG
		static_cast<uint32_t>(EnabledLayerNames.size()), EnabledLayerNames.data(),
#else
		0, nullptr,
#endif
		static_cast<uint32_t>(EnabledExtensions.size()), EnabledExtensions.data()
	};

	//AllocationCallbacks = {
	//	nullptr,
	//	AlignedMalloc,
	//	AlignedRealloc,
	//	AlignedFree,
	//	nullptr,
	//	nullptr
	//};
	//VERIFY_SUCCEEDED(vkCreateInstance(&InstanceCreateInfo, &AllocationCallbacks, &Instance));
	VERIFY_SUCCEEDED(vkCreateInstance(&InstanceCreateInfo, nullptr, &Instance));
}
VkPhysicalDevice VK::CreateDevice()
{
	uint32_t PhysicalDeviceCount = 0;
	VERIFY_SUCCEEDED(vkEnumeratePhysicalDevices(Instance, &PhysicalDeviceCount, nullptr));
	assert(PhysicalDeviceCount);
	std::vector<VkPhysicalDevice> PhysicalDevices(PhysicalDeviceCount);
	VERIFY_SUCCEEDED(vkEnumeratePhysicalDevices(Instance, &PhysicalDeviceCount, PhysicalDevices.data()));

	//!< ここでは最初の物理デバイスを使用することにする
	auto PhysicalDevice = PhysicalDevices[0];

	CreateDevice(PhysicalDevice);
	return PhysicalDevice;
}
void VK::CreateDevice(VkPhysicalDevice PhysicalDevice)
{
	uint32_t QueueFamilyPropertyCount;
	vkGetPhysicalDeviceQueueFamilyProperties(PhysicalDevice, &QueueFamilyPropertyCount, nullptr);
	assert(QueueFamilyPropertyCount);
	std::vector<VkQueueFamilyProperties> QueueProperties(QueueFamilyPropertyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(PhysicalDevice, &QueueFamilyPropertyCount, QueueProperties.data());
	for (uint32_t i = 0; i < QueueFamilyPropertyCount; ++i) {
		//!< 描画機能のある最初のキューを探す
		if (VK_QUEUE_GRAPHICS_BIT & QueueProperties[i].queueFlags) {
			const std::vector<float> QueuePriorities = { 0.0f };
			const std::vector<VkDeviceQueueCreateInfo> QueueCreateInfos = {
				{
					VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
					nullptr,
					0,
					i,
					static_cast<uint32_t>(QueuePriorities.size()), QueuePriorities.data()
				}
			};
			const std::vector<const char*> EnabledExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
			const VkDeviceCreateInfo DeviceCreateInfo = {
				VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
				nullptr,
				0,
				static_cast<uint32_t>(QueueCreateInfos.size()), QueueCreateInfos.data(),
#ifdef _DEBUG
				static_cast<uint32_t>(EnabledLayerNames.size()), EnabledLayerNames.data(),
#else
				0, nullptr,
#endif
				static_cast<uint32_t>(EnabledExtensions.size()), EnabledExtensions.data(),
				nullptr
			};
			VERIFY_SUCCEEDED(vkCreateDevice(PhysicalDevice, &DeviceCreateInfo, nullptr, &Device));

			//!< キューを取得
			vkGetDeviceQueue(Device, i, 0, &Queue);

			//vkGetPhysicalDeviceProperties(PhysicalDevice, &PhysicalDeviceProperties);
			//vkGetPhysicalDeviceMemoryProperties(PhysicalDevice, &PhysicalDeviceMemoryProperties);

			//GetSupportedDepthFormat(PhysicalDevice);
			break;
		}
	}
}
void VK::CreateSwapchain(HWND hWnd, HINSTANCE hInstance, VkPhysicalDevice PhysicalDevice, const VkFormat ColorFormat)
{
	VkSurfaceKHR Surface = VK_NULL_HANDLE;
#if defined(_WIN32)
	const VkWin32SurfaceCreateInfoKHR SurfaceCreateInfo = {
		VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
		nullptr,
		0,
		hInstance,
		hWnd
	};
	VERIFY_SUCCEEDED(vkCreateWin32SurfaceKHR(Instance, &SurfaceCreateInfo, nullptr, &Surface));
#endif
	{
		VkSurfaceCapabilitiesKHR SurfaceCapabilities;
		VERIFY_SUCCEEDED(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(PhysicalDevice, Surface, &SurfaceCapabilities));
		const auto MinImageCount = (std::min)(SurfaceCapabilities.minImageCount + 1, SurfaceCapabilities.maxImageCount);

		uint32_t SurfaceFormatCount;
		VERIFY_SUCCEEDED(vkGetPhysicalDeviceSurfaceFormatsKHR(PhysicalDevice, Surface, &SurfaceFormatCount, nullptr));
		assert(SurfaceFormatCount);
		std::vector<VkSurfaceFormatKHR> SurfaceFormats(SurfaceFormatCount);
		VERIFY_SUCCEEDED(vkGetPhysicalDeviceSurfaceFormatsKHR(PhysicalDevice, Surface, &SurfaceFormatCount, SurfaceFormats.data()));
		const auto ImageFormat = (1 == SurfaceFormatCount && VK_FORMAT_UNDEFINED == SurfaceFormats[0].format) ? VK_FORMAT_B8G8R8A8_UNORM : SurfaceFormats[0].format;
		const auto ImageColorSpace = SurfaceFormats[0].colorSpace;
		//!< イメージの幅と高さを覚えておく
		ImageExtent = SurfaceCapabilities.currentExtent;
		if (-1 == SurfaceCapabilities.currentExtent.width) {
			ImageExtent = { static_cast<uint32_t>(GetClientRectWidth()), static_cast<uint32_t>(GetClientRectHeight()) };
		}
		const auto PreTransform = (SurfaceCapabilities.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR) ? VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR : SurfaceCapabilities.currentTransform;;

		uint32_t PresentModeCount;
		VERIFY_SUCCEEDED(vkGetPhysicalDeviceSurfacePresentModesKHR(PhysicalDevice, Surface, &PresentModeCount, nullptr));
		assert(PresentModeCount);
		std::vector<VkPresentModeKHR> PresentModes(PresentModeCount);
		VERIFY_SUCCEEDED(vkGetPhysicalDeviceSurfacePresentModesKHR(PhysicalDevice, Surface, &PresentModeCount, PresentModes.data()));
		VkPresentModeKHR PresentMode = VK_PRESENT_MODE_FIFO_KHR;
		for (auto i : PresentModes) {
			if (VK_PRESENT_MODE_MAILBOX_KHR == i) {
				PresentMode = i;
				break;
			}
			else if (VK_PRESENT_MODE_IMMEDIATE_KHR == i) {
				PresentMode = i;
			}
		}

#pragma region Swapchain
		auto OldSwapchain = Swapchain;
		const VkSwapchainCreateInfoKHR SwapchainCreateInfo = {
			VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
			nullptr,
			0,
			Surface,
			MinImageCount,
			ImageFormat,
			ImageColorSpace,
			ImageExtent,
			1,
			VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
			VK_SHARING_MODE_EXCLUSIVE,
			0, nullptr,
			PreTransform,
			VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
			PresentMode,
			true,
			OldSwapchain
		};
		VERIFY_SUCCEEDED(vkCreateSwapchainKHR(Device, &SwapchainCreateInfo, nullptr, &Swapchain));
#pragma endregion
		if (VK_NULL_HANDLE != OldSwapchain) {
			vkDestroySwapchainKHR(Device, OldSwapchain, nullptr);
		}
	}
	if (VK_NULL_HANDLE != Surface) {
		vkDestroySurfaceKHR(Instance, Surface, nullptr);
	}

#pragma region SwapchainImage
	uint32_t SwapchainImageCount;
	VERIFY_SUCCEEDED(vkGetSwapchainImagesKHR(Device, Swapchain, &SwapchainImageCount, nullptr));
	assert(SwapchainImageCount);
	SwapchainImages.resize(SwapchainImageCount);
	VERIFY_SUCCEEDED(vkGetSwapchainImagesKHR(Device, Swapchain, &SwapchainImageCount, SwapchainImages.data()));
#pragma endregion

#pragma region SwapchainImageView
	SwapchainImageViews.resize(SwapchainImageCount);
	for (uint32_t i = 0; i < SwapchainImageCount; ++i) {
		SetImageLayout(SetupCommandBuffer, SwapchainImages[i], VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
		const VkImageViewCreateInfo ImageViewCreateInfo = {
			VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
			nullptr,
			0,
			SwapchainImages[i],
			VK_IMAGE_VIEW_TYPE_2D,
			ColorFormat,
			{ VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A },
			{
				VK_IMAGE_ASPECT_COLOR_BIT,
				0, 1,
				0, 1
			}
		};
		VERIFY_SUCCEEDED(vkCreateImageView(Device, &ImageViewCreateInfo, nullptr, &SwapchainImageViews[i]));
	}
#pragma endregion

#pragma region SwapchainImageIndex
	const VkSemaphoreCreateInfo SemaphoreCreateInfo = {
		VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
		nullptr,
		0
	};
	VERIFY_SUCCEEDED(vkCreateSemaphore(Device, &SemaphoreCreateInfo, nullptr, &Semaphore));
	VERIFY_SUCCEEDED(vkAcquireNextImageKHR(Device, Swapchain, UINT64_MAX, Semaphore, nullptr, &SwapchainImageIndex));
#pragma endregion
}

void VK::CreateDepthStencil(const VkPhysicalDeviceMemoryProperties& PhysicalDeviceMemoryProperties, const VkFormat DepthFormat)
{
#pragma region DepthStencilImage
	const VkImageCreateInfo ImageCreateInfo = {
		VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
		nullptr,
		0,
		VK_IMAGE_TYPE_2D,
		DepthFormat,
		{ ImageExtent.width, ImageExtent.height, 1 },
		1,
		1,
		VK_SAMPLE_COUNT_1_BIT,
		VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
		VK_SHARING_MODE_EXCLUSIVE,
		0, nullptr,
		VK_IMAGE_LAYOUT_UNDEFINED
	};
	VERIFY_SUCCEEDED(vkCreateImage(Device, &ImageCreateInfo, nullptr, &DepthStencilImage));
#pragma endregion

#pragma region DepthStencilDeviceMemory
	VkMemoryRequirements MemoryRequirements;
	vkGetImageMemoryRequirements(Device, DepthStencilImage, &MemoryRequirements);
	const auto MemoryTypeIndex = GetMemoryType(PhysicalDeviceMemoryProperties, MemoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	const VkMemoryAllocateInfo MemoryAllocateInfo = {
		VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
		nullptr,
		MemoryRequirements.size,
		MemoryTypeIndex
	};
	VERIFY_SUCCEEDED(vkAllocateMemory(Device, &MemoryAllocateInfo, nullptr, &DepthStencilDeviceMemory));
	VERIFY_SUCCEEDED(vkBindImageMemory(Device, DepthStencilImage, DepthStencilDeviceMemory, 0));
#pragma endregion

#pragma region DepthStencilImageView
	SetImageLayout(SetupCommandBuffer, DepthStencilImage, VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
	const VkImageViewCreateInfo ImageViewCreateInfo = {
		VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
		nullptr,
		0,
		DepthStencilImage,
		VK_IMAGE_VIEW_TYPE_2D,
		DepthFormat,
		{ VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY },
		{
			VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT,
			0, 1,
			0, 1
		}
	};
	VERIFY_SUCCEEDED(vkCreateImageView(Device, &ImageViewCreateInfo, nullptr, &DepthStencilImageView));
#pragma endregion
}

void VK::CreateRenderPass(const VkFormat ColorFormat, const VkFormat DepthFormat)
{
#pragma region Attachment
	const std::vector<VkAttachmentDescription> AttachmentDescriptions = {
		{
			0,
			ColorFormat,
			VK_SAMPLE_COUNT_1_BIT,
			VK_ATTACHMENT_LOAD_OP_CLEAR,
			VK_ATTACHMENT_STORE_OP_STORE,
			VK_ATTACHMENT_LOAD_OP_DONT_CARE,
			VK_ATTACHMENT_STORE_OP_DONT_CARE,
			VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
		},
		{
			0,
			DepthFormat,
			VK_SAMPLE_COUNT_1_BIT,
			VK_ATTACHMENT_LOAD_OP_CLEAR,
			VK_ATTACHMENT_STORE_OP_STORE,
			VK_ATTACHMENT_LOAD_OP_DONT_CARE,
			VK_ATTACHMENT_STORE_OP_DONT_CARE,
			VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
			VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
		}
	};
#pragma endregion

#pragma region Subpass
	const std::vector<VkAttachmentReference> ColorAttachmentReferences = {
		{
			0,
			VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
		}
	};
	const VkAttachmentReference DepthAttachmentReference = {
		1,
		VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
	};
	const std::vector<VkSubpassDescription> SubpassDescriptions = {
		{
			0,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			0, nullptr,
			static_cast<uint32_t>(ColorAttachmentReferences.size()), ColorAttachmentReferences.data(),
			nullptr,
			&DepthAttachmentReference,
			0, nullptr
		}
	};
#pragma endregion

#pragma region RenderPass
	const VkRenderPassCreateInfo RenderPassCreateInfo = {
		VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
		nullptr,
		0,
		static_cast<uint32_t>(AttachmentDescriptions.size()), AttachmentDescriptions.data(),
		static_cast<uint32_t>(SubpassDescriptions.size()), SubpassDescriptions.data(),
		0, nullptr
	};
	VERIFY_SUCCEEDED(vkCreateRenderPass(Device, &RenderPassCreateInfo, nullptr, &RenderPass));
#pragma endregion
}

void VK::CreateFramebuffers()
{
	Framebuffers.resize(SwapchainImages.size());
	for (uint32_t i = 0; i < Framebuffers.size(); ++i) {
		const std::vector<VkImageView> Attachments = {
			SwapchainImageViews[i],
			DepthStencilImageView
		};
		const VkFramebufferCreateInfo FramebufferCreateInfo = {
			VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
			nullptr,
			0,
			RenderPass,
			static_cast<uint32_t>(Attachments.size()), Attachments.data(),
			ImageExtent.width, ImageExtent.height,
			1
		};
		VERIFY_SUCCEEDED(vkCreateFramebuffer(Device, &FramebufferCreateInfo, nullptr, &Framebuffers[i]));
	}
}

void VK::CreateShader()
{
	ShaderModules.push_back(CreateShaderModule("XXX.vert.spv"));
	ShaderModules.push_back(CreateShaderModule("XXX.frag.spv"));

	PipelineShaderStageCreateInfos = {
		{
			VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
			nullptr,
		0,
		VK_SHADER_STAGE_VERTEX_BIT, ShaderModules[0],
		"main",
		nullptr
		},
		{
			VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
			nullptr,
		0,
		VK_SHADER_STAGE_FRAGMENT_BIT, ShaderModules[1],
		"main",
		nullptr
		}
	};
}
/**
@brief シェーダとのバインディングのレイアウト
@note DescriptorSetLayt は型のようなもの、DescriptorSet はインスタンスのようなもの
*/
void VK::CreateDescriptorSetLayout()
{
	const std::vector<VkDescriptorSetLayoutBinding> DescriptorSetLayoutBindings = {
		{
			0,
			VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			1,
			VK_SHADER_STAGE_VERTEX_BIT,
			nullptr
		}
	};
	const VkDescriptorSetLayoutCreateInfo DescriptorSetLayoutCreateInfo = {
		VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
		nullptr,
		0,
		static_cast<uint32_t>(DescriptorSetLayoutBindings.size()), DescriptorSetLayoutBindings.data()
	};
	DescriptorSetLayouts.resize(1);
	VERIFY_SUCCEEDED(vkCreateDescriptorSetLayout(Device, &DescriptorSetLayoutCreateInfo, nullptr, DescriptorSetLayouts.data()));
}
void VK::CreatePipelineLayout()
{
	const VkPipelineLayoutCreateInfo PipelineLayoutCreateInfo = {
		VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
		nullptr,
		0,
		static_cast<uint32_t>(DescriptorSetLayouts.size()), DescriptorSetLayouts.data(),
		0, nullptr
	};
	VERIFY_SUCCEEDED(vkCreatePipelineLayout(Device, &PipelineLayoutCreateInfo, nullptr, &PipelineLayout));
}

void VK::CreateVertexInput()
{
	std::tuple<glm::vec3, glm::vec3> Vertex;

	VertexInputBindingDescriptions = {
		{ 0, sizeof(Vertex), VK_VERTEX_INPUT_RATE_VERTEX }
	};
	VertexInputAttributeDescriptions = {
		{ 0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0 },
		{ 1, 0, VK_FORMAT_R32G32B32_SFLOAT, sizeof(std::get<0>(Vertex)) }
	};
	PipelineVertexInputStateCreateInfo = {
		VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
		nullptr,
		0,
		static_cast<uint32_t>(VertexInputBindingDescriptions.size()), VertexInputBindingDescriptions.data(),
		static_cast<uint32_t>(VertexInputAttributeDescriptions.size()), VertexInputAttributeDescriptions.data()
	};
}

void VK::CreateViewport()
{
	Viewports = {
		{
			0, 0,
			static_cast<float>(ImageExtent.width), static_cast<float>(ImageExtent.height),
			0.0f, 1.0f
		}
	};
	ScissorRects = {
		{
			{ 0, 0 },
			{ ImageExtent.width, ImageExtent.height }
		}
	};
}

void VK::CreatePipeline()
{
	const VkPipelineInputAssemblyStateCreateInfo PipelineInputAssemblyStateCreateInfo = {
		VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
		nullptr,
		0,
		VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
		VK_FALSE
	};

	const VkPipelineViewportStateCreateInfo PipelineViewportStateCreateInfo = {
		VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
		nullptr,
		0,
		static_cast<uint32_t>(Viewports.size()), Viewports.data(),
		static_cast<uint32_t>(ScissorRects.size()), ScissorRects.data()
	};

	
	const VkPipelineCacheCreateInfo PipelineCacheCreateInfo = {
		VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO,
		nullptr,
		0,
		0, nullptr
	};
	VERIFY_SUCCEEDED(vkCreatePipelineCache(Device, &PipelineCacheCreateInfo, nullptr, &PipelineCache));
	const VkGraphicsPipelineCreateInfo GraphicsPipelineCreateInfo = {
		VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
		nullptr,
		0,
		static_cast<uint32_t>(PipelineShaderStageCreateInfos.size()), PipelineShaderStageCreateInfos.data(),
		&PipelineVertexInputStateCreateInfo,
		nullptr, //VkPipelineInputAssemblyStateCreateInfo* pInputAssemblyState
		nullptr, //VkPipelineTessellationStateCreateInfo* pTessellationState
		&PipelineViewportStateCreateInfo,
		nullptr, //VkPipelineRasterizationStateCreateInfo* pRasterizationState
		nullptr, //VkPipelineMultisampleStateCreateInfo* pMultisampleState
		nullptr, //VkPipelineDepthStencilStateCreateInfo* pDepthStencilState
		nullptr, //VkPipelineColorBlendStateCreateInfo* pColorBlendState
		nullptr, //VkPipelineDynamicStateCreateInfo* pDynamicState
		PipelineLayout,
		nullptr, //VkRenderPass renderPass
		0, //uint32_t subpass
		nullptr, //VkPipeline basePipelineHandle
		0 //int32_t basePipelineIndex
	};
	VERIFY_SUCCEEDED(vkCreateGraphicsPipelines(Device, PipelineCache, 1, &GraphicsPipelineCreateInfo, nullptr, &Pipeline));
}

void VK::CreateSemaphore()
{
	const VkSemaphoreCreateInfo SemaphoreCreateInfo = {
		VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
		nullptr,
		0
	};
	VERIFY_SUCCEEDED(vkCreateSemaphore(Device, &SemaphoreCreateInfo, nullptr, &PresentSemaphore));
	VERIFY_SUCCEEDED(vkCreateSemaphore(Device, &SemaphoreCreateInfo, nullptr, &RenderSemaphore));
}
void VK::CreateCommandPool()
{
	const VkCommandPoolCreateInfo CommandPoolInfo = {
		VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
		nullptr,
		VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
		QueueFamilyIndex
	};
	VERIFY_SUCCEEDED(vkCreateCommandPool(Device, &CommandPoolInfo, nullptr, &CommandPool));
}

void VK::CreateSetupCommandBuffer()
{
	if (SetupCommandBuffer != VK_NULL_HANDLE) {
		vkFreeCommandBuffers(Device, CommandPool, 1, &SetupCommandBuffer);
		SetupCommandBuffer = VK_NULL_HANDLE;
	}

	const VkCommandBufferAllocateInfo CommandBufferAllocateInfo = {
		VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		nullptr,
		CommandPool,
		VK_COMMAND_BUFFER_LEVEL_PRIMARY,
		1
	};
	VERIFY_SUCCEEDED(vkAllocateCommandBuffers(Device, &CommandBufferAllocateInfo, &SetupCommandBuffer));

	const VkCommandBufferBeginInfo CommandBufferBeginInfo = {
		VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		nullptr,
		0,
		nullptr
	};
	VERIFY_SUCCEEDED(vkBeginCommandBuffer(SetupCommandBuffer, &CommandBufferBeginInfo));
}

void VK::CreateDescriptorSet()
{
	//const std::vector<VkDescriptorSetLayoutBinding> DescriptorSetLayoutBindings = {
	//	{
	//		0,
	//		VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
	//	1,
	//	VK_SHADER_STAGE_VERTEX_BIT,
	//	nullptr
	//	}
	//};
	//const VkDescriptorSetLayoutCreateInfo DescriptorSetLayoutCreateInfo = {
	//	VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
	//	nullptr,
	//	0,
	//	static_cast<uint32_t>(DescriptorSetLayoutBindings.size()), DescriptorSetLayoutBindings.data()
	//};
	//DescriptorSetLayouts.reserve(1);
	//VERIFY_SUCCEEDED(vkCreateDescriptorSetLayout(Device, &DescriptorSetLayoutCreateInfo, nullptr, DescriptorSetLayouts.data()));

	const std::vector<VkDescriptorPoolSize> DescriptorPoolSizes = {
		{
			VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			1
		}
	};
	const VkDescriptorPoolCreateInfo DescriptorPoolCreateInfo = {
		VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
		nullptr,
		0,
		1,
		static_cast<uint32_t>(DescriptorPoolSizes.size()), DescriptorPoolSizes.data()
	};
	VERIFY_SUCCEEDED(vkCreateDescriptorPool(Device, &DescriptorPoolCreateInfo, nullptr, &DescriptorPool));

	const VkDescriptorSetAllocateInfo DescriptorSetAllocateInfo = {
		VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
		nullptr,
		DescriptorPool,
		static_cast<uint32_t>(DescriptorSetLayouts.size()), DescriptorSetLayouts.data()
	};
	DescriptorSets.reserve(1);
	VERIFY_SUCCEEDED(vkAllocateDescriptorSets(Device, &DescriptorSetAllocateInfo, DescriptorSets.data()));
}

void VK::CreateCommandBuffers()
{
	//CommandBuffers.resize(SwapchainBuffers.size()/*ImageCount*/);

	/*const*/ VkCommandBufferAllocateInfo CommandBufferAllocateInfo = {
		VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		nullptr,
		CommandPool,
		VK_COMMAND_BUFFER_LEVEL_PRIMARY,
		static_cast<uint32_t>(CommandBuffers.size())
	};
	VERIFY_SUCCEEDED(vkAllocateCommandBuffers(Device, &CommandBufferAllocateInfo, CommandBuffers.data()));

	CommandBufferAllocateInfo.commandBufferCount = 1;
	VERIFY_SUCCEEDED(vkAllocateCommandBuffers(Device, &CommandBufferAllocateInfo, &PrePresentCommandBuffer));
	VERIFY_SUCCEEDED(vkAllocateCommandBuffers(Device, &CommandBufferAllocateInfo, &PostPresentCommandBuffer));
}
void VK::FlushSetupCommandBuffer()
{
	if (SetupCommandBuffer != VK_NULL_HANDLE) {
		VERIFY_SUCCEEDED(vkEndCommandBuffer(SetupCommandBuffer));

		const VkSubmitInfo SubmitInfo = {
			VK_STRUCTURE_TYPE_SUBMIT_INFO,
			nullptr,
			0, nullptr,
			nullptr,
			1, &SetupCommandBuffer,
			0, nullptr
		};
		VERIFY_SUCCEEDED(vkQueueSubmit(Queue, 1, &SubmitInfo, VK_NULL_HANDLE));

		VERIFY_SUCCEEDED(vkQueueWaitIdle(Queue));

		vkFreeCommandBuffers(Device, CommandPool, 1, &SetupCommandBuffer);
		SetupCommandBuffer = VK_NULL_HANDLE;
	}
}

void VK::CreateVertexBuffer()
{
	const std::vector<glm::vec3> Vertices = { {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f} }; //todo

#pragma region Staging
	VkBuffer StagingBuffer;
	VkDeviceMemory StagingDeviceMemory;
	{
		VkBufferCreateInfo BufferCreateInfo = {};
		BufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		BufferCreateInfo.size = sizeof(Vertices);
		BufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		VERIFY_SUCCEEDED(vkCreateBuffer(Device, &BufferCreateInfo, nullptr, &StagingBuffer));

		VkMemoryAllocateInfo MemoryAllocateInfo = {};
		MemoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		VkMemoryRequirements MemoryRequirements;
		vkGetBufferMemoryRequirements(Device, StagingBuffer, &MemoryRequirements);
		MemoryAllocateInfo.allocationSize = MemoryRequirements.size;
		MemoryAllocateInfo.memoryTypeIndex = GetMemoryType(MemoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
		VERIFY_SUCCEEDED(vkAllocateMemory(Device, &MemoryAllocateInfo, nullptr, &StagingDeviceMemory));
		void *Data;
		VERIFY_SUCCEEDED(vkMapMemory(Device, StagingDeviceMemory, 0, MemoryAllocateInfo.allocationSize, 0, &Data)); {
			memcpy(Data, Vertices.data(), sizeof(Vertices));
		} vkUnmapMemory(Device, StagingDeviceMemory);
		VERIFY_SUCCEEDED(vkBindBufferMemory(Device, StagingBuffer, StagingDeviceMemory, 0));
#pragma endregion

#pragma region VRAM
		BufferCreateInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
		VERIFY_SUCCEEDED(vkCreateBuffer(Device, &BufferCreateInfo, nullptr, &VertexBuffer));
		vkGetBufferMemoryRequirements(Device, VertexBuffer, &MemoryRequirements);
		MemoryAllocateInfo.allocationSize = MemoryRequirements.size;
		MemoryAllocateInfo.memoryTypeIndex = GetMemoryType(MemoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		VERIFY_SUCCEEDED(vkAllocateMemory(Device, &MemoryAllocateInfo, nullptr, &VertexDeviceMemory));
		VERIFY_SUCCEEDED(vkBindBufferMemory(Device, VertexBuffer, VertexDeviceMemory, 0));
#pragma endregion

#pragma region ToVRAMCommand
		VkCommandBufferAllocateInfo CommandBufferAllocateInfo = {};
		CommandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		CommandBufferAllocateInfo.commandPool = CommandPool;
		CommandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		CommandBufferAllocateInfo.commandBufferCount = 1;
		VkCommandBuffer CopyCommandBuffer;
		VERIFY_SUCCEEDED(vkAllocateCommandBuffers(Device, &CommandBufferAllocateInfo, &CopyCommandBuffer)); {
			VkCommandBufferBeginInfo CommandBufferBeginInfo = {};
			CommandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			CommandBufferBeginInfo.pNext = nullptr;
			VERIFY_SUCCEEDED(vkBeginCommandBuffer(CopyCommandBuffer, &CommandBufferBeginInfo)); {
				VkBufferCopy BufferCopy = {};
				BufferCopy.size = Vertices.size();
				vkCmdCopyBuffer(CopyCommandBuffer, StagingBuffer, VertexBuffer, 1, &BufferCopy);
			} VERIFY_SUCCEEDED(vkEndCommandBuffer(CopyCommandBuffer));

			VkSubmitInfo SubmitInfo = {};
			SubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
			SubmitInfo.commandBufferCount = 1;
			SubmitInfo.pCommandBuffers = &CopyCommandBuffer;
			VERIFY_SUCCEEDED(vkQueueSubmit(Queue, 1, &SubmitInfo, VK_NULL_HANDLE));
			VERIFY_SUCCEEDED(vkQueueWaitIdle(Queue));
		} vkFreeCommandBuffers(Device, CommandPool, 1, &CopyCommandBuffer);
#pragma endregion
	} 
	vkDestroyBuffer(Device, StagingBuffer, nullptr);
	vkFreeMemory(Device, StagingDeviceMemory, nullptr);
}

void VK::CreateIndexBuffer()
{
	const std::vector<uint32_t> Indices = { 0, 1, 2 };

#pragma region Staging
	VkBuffer StagingBuffer;
	VkDeviceMemory StagingDeviceMemory; 
	{
		VkBufferCreateInfo BufferCreateInfo = {};
		BufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		BufferCreateInfo.size = sizeof(Indices);
		BufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		VERIFY_SUCCEEDED(vkCreateBuffer(Device, &BufferCreateInfo, nullptr, &StagingBuffer));

		VkMemoryAllocateInfo MemoryAllocateInfo = {};
		MemoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		VkMemoryRequirements MemoryRequirements;
		vkGetBufferMemoryRequirements(Device, StagingBuffer, &MemoryRequirements);
		MemoryAllocateInfo.allocationSize = MemoryRequirements.size;
		MemoryAllocateInfo.memoryTypeIndex = GetMemoryType(MemoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
		VERIFY_SUCCEEDED(vkAllocateMemory(Device, &MemoryAllocateInfo, nullptr, &StagingDeviceMemory));
		void *Data;
		VERIFY_SUCCEEDED(vkMapMemory(Device, StagingDeviceMemory, 0, sizeof(Indices), 0, &Data)); {
			memcpy(Data, Indices.data(), sizeof(Indices));
		} vkUnmapMemory(Device, StagingDeviceMemory);
		VERIFY_SUCCEEDED(vkBindBufferMemory(Device, StagingBuffer, StagingDeviceMemory, 0));
#pragma endregion

#pragma region VRAM
		BufferCreateInfo.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
		VERIFY_SUCCEEDED(vkCreateBuffer(Device, &BufferCreateInfo, nullptr, &IndexBuffer));
		vkGetBufferMemoryRequirements(Device, IndexBuffer, &MemoryRequirements);
		MemoryAllocateInfo.allocationSize = MemoryRequirements.size;
		MemoryAllocateInfo.memoryTypeIndex = GetMemoryType(MemoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		VERIFY_SUCCEEDED(vkAllocateMemory(Device, &MemoryAllocateInfo, nullptr, &IndexDeviceMemory));
		VERIFY_SUCCEEDED(vkBindBufferMemory(Device, IndexBuffer, IndexDeviceMemory, 0));
		
		//vkCmdDrawIndexed() が Indices.size() を引数に取るので覚えておく必要がある todo
#pragma endregion

#pragma region ToVRAMCommand
		VkCommandBufferAllocateInfo CommandBufferAllocateInfo = {};
		CommandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		CommandBufferAllocateInfo.commandPool = CommandPool;
		CommandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		CommandBufferAllocateInfo.commandBufferCount = 1;
		VkCommandBuffer CopyCommandBuffer;
		VERIFY_SUCCEEDED(vkAllocateCommandBuffers(Device, &CommandBufferAllocateInfo, &CopyCommandBuffer)); {
			VkCommandBufferBeginInfo CommandBufferBeginInfo = {};
			CommandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			CommandBufferBeginInfo.pNext = nullptr;
			VERIFY_SUCCEEDED(vkBeginCommandBuffer(CopyCommandBuffer, &CommandBufferBeginInfo)); {
				VkBufferCopy BufferCopy = {};
				BufferCopy.size = Indices.size();
				vkCmdCopyBuffer(CopyCommandBuffer, StagingBuffer, IndexBuffer, 1, &BufferCopy);
			} VERIFY_SUCCEEDED(vkEndCommandBuffer(CopyCommandBuffer));

			VkSubmitInfo SubmitInfo = {};
			SubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
			SubmitInfo.commandBufferCount = 1;
			SubmitInfo.pCommandBuffers = &CopyCommandBuffer;
			VERIFY_SUCCEEDED(vkQueueSubmit(Queue, 1, &SubmitInfo, VK_NULL_HANDLE));
			VERIFY_SUCCEEDED(vkQueueWaitIdle(Queue));
		} vkFreeCommandBuffers(Device, CommandPool, 1, &CopyCommandBuffer);
#pragma endregion
	}
	vkDestroyBuffer(Device, StagingBuffer, nullptr);
	vkFreeMemory(Device, StagingDeviceMemory, nullptr);
}

void VK::CreateUniformBuffer()
{
	const auto Size = sizeof(glm::mat4) * 3; //!< World, View, Projection

	VkBufferCreateInfo BufferCreateInfo = {};
	BufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	BufferCreateInfo.size = Size;
	BufferCreateInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
	VERIFY_SUCCEEDED(vkCreateBuffer(Device, &BufferCreateInfo, nullptr, &UniformBuffer));

	VkMemoryAllocateInfo MemoryAllocateInfo = {};
	MemoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	MemoryAllocateInfo.pNext = nullptr;
	MemoryAllocateInfo.allocationSize = 0;
	MemoryAllocateInfo.memoryTypeIndex = 0; 
	VkMemoryRequirements MemoryRequirements; {
		vkGetBufferMemoryRequirements(Device, UniformBuffer, &MemoryRequirements);
		MemoryAllocateInfo.allocationSize = MemoryRequirements.size;
	}

	MemoryAllocateInfo.memoryTypeIndex = GetMemoryType(MemoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
	VERIFY_SUCCEEDED(vkAllocateMemory(Device, &MemoryAllocateInfo, nullptr, &UniformDeviceMemory));
	VERIFY_SUCCEEDED(vkBindBufferMemory(Device, UniformBuffer, UniformDeviceMemory, 0));

	UniformDescriptorBufferInfo.buffer = UniformBuffer;
	UniformDescriptorBufferInfo.offset = 0;
	UniformDescriptorBufferInfo.range = Size;
}

void VK::CreateFence()
{
	const VkFenceCreateInfo FenceCreateInfo = {
		VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
		nullptr,
		0
	};
	VERIFY_SUCCEEDED(vkCreateFence(Device, &FenceCreateInfo, nullptr, &Fence));
}

void VK::PopulateCommandBuffer()
{
	//VERIFY_SUCCEEDED(vkBeginCommandBuffer(drawCmdBuffers[i], &cmdBufInfo)); {
	//vkCmdBeginRenderPass(drawCmdBuffers[i], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
	//vkCmdBindPipeline(drawCmdBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelines.solid);
	//vkCmdPipelineBarrier();
	//} VERIFY_SUCCEEDED(vkEndCommandBuffer(drawCmdBuffers[i]));

	//!< バッファインデックスの更新
	VERIFY_SUCCEEDED(vkAcquireNextImageKHR(Device, Swapchain, UINT64_MAX, PresentSemaphore, nullptr, &CurrentSwapchainBufferIndex));

	vkCmdSetViewport(CommandBuffers[CurrentSwapchainBufferIndex], 0, static_cast<uint32_t>(Viewports.size()), Viewports.data());
	vkCmdSetScissor(CommandBuffers[CurrentSwapchainBufferIndex], 0, static_cast<uint32_t>(ScissorRects.size()), ScissorRects.data());

	const VkDeviceSize Offsets[] = { 0 };
	vkCmdBindVertexBuffers(CommandBuffers[CurrentSwapchainBufferIndex], 0, 1, &VertexBuffer, Offsets);
	vkCmdBindIndexBuffer(CommandBuffers[CurrentSwapchainBufferIndex], IndexBuffer, 0, VK_INDEX_TYPE_UINT32);

	//VkImageMemoryBarrier postPresentBarrier = {};
	//postPresentBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	//postPresentBarrier.pNext = nullptr;
	//postPresentBarrier.srcAccessMask = 0;
	//postPresentBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	//postPresentBarrier.oldLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	//postPresentBarrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	//postPresentBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	//postPresentBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	//postPresentBarrier.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
	//postPresentBarrier.image = SwapchainBuffers[CurrentSwapchainBufferIndex].Image;
}

void VK::ExecuteCommandBuffer()
{
	VERIFY_SUCCEEDED(vkDeviceWaitIdle(Device));

	const VkSubmitInfo SubmitInfo = {
		VK_STRUCTURE_TYPE_SUBMIT_INFO,
		nullptr,
		0, nullptr,
		nullptr,
		1,  &CommandBuffers[SwapchainImageIndex],
		0, nullptr
	};
#if 1
	if (VK_NULL_HANDLE != Fence) {
		VERIFY_SUCCEEDED(vkQueueSubmit(Queue, 1, &SubmitInfo, Fence));
	}
	else {
		VERIFY_SUCCEEDED(vkQueueSubmit(Queue, 1, &SubmitInfo, VK_NULL_HANDLE));
	}
	VERIFY_SUCCEEDED(vkQueueWaitIdle(Queue));
#else
	//!< 複数のコマンドバッファをサブミットしたい場合はセマフォと配列を使う
	VkPipelineStageFlags PipelineStageFlags = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT; {
		SubmitInfo.pWaitDstStageMask = &PipelineStageFlags;
	}
	if (VK_NULL_HANDLE != PresentSemaphore) {
		SubmitInfo.waitSemaphoreCount = 1;
		SubmitInfo.pWaitSemaphores = &PresentSemaphore;
	}
	if (VK_NULL_HANDLE != RenderSemaphore) {
		SubmitInfo.signalSemaphoreCount = 1;
		SubmitInfo.pSignalSemaphores = &RenderSemaphore;
	}
	VERIFY_SUCCEEDED(vkQueueSubmit(Queue, 1, &SubmitInfo, VK_NULL_HANDLE));
#endif

	VERIFY_SUCCEEDED(vkDeviceWaitIdle(Device));
}

void VK::Present()
{
	const VkPresentInfoKHR PresentInfo = {
		VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
		nullptr,
		0, nullptr,
		1, &Swapchain,
		&CurrentSwapchainBufferIndex,
		nullptr
	};
#if 0
	if (VK_NULL_HANDLE != RenderSemaphore) {
		PresentInfo.waitSemaphoreCount = 1;
		PresentInfo.pWaitSemaphores = &RenderSemaphore;
	}
#endif
	VERIFY_SUCCEEDED(vkQueuePresentKHR(Queue, &PresentInfo));
}

void VK::WaitForFence()
{
	VkResult Result;
	const uint64_t TimeOut = 100000000;
	do {
		Result = vkWaitForFences(Device, 1, &Fence, VK_TRUE, TimeOut);
	} while (VK_TIMEOUT != Result);
	VERIFY_SUCCEEDED(Result);
}
