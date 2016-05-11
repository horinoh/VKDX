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
	
	CreateCommandBuffer();
	const auto ColorFormat = VK_FORMAT_B8G8R8A8_UNORM;
	CreateSwapchain(hWnd, hInstance, PhysicalDevice, ColorFormat);

	VkPhysicalDeviceMemoryProperties PhysicalDeviceMemoryProperties;
	vkGetPhysicalDeviceMemoryProperties(PhysicalDevice, &PhysicalDeviceMemoryProperties);	
	const auto DepthFormat = GetSupportedDepthFormat(PhysicalDevice);
	CreateDepthStencil(PhysicalDeviceMemoryProperties, DepthFormat);

	CreateShader();
	CreateDescriptorSetLayout();
	CreatePipelineLayout();
	CreateDescriptorSet();

	CreateVertexInput();
	CreateViewport();
	CreatePipeline();

	CreateVertexBuffer(PhysicalDeviceMemoryProperties);
	CreateIndexBuffer(PhysicalDeviceMemoryProperties);
	CreateUniformBuffer(PhysicalDeviceMemoryProperties);

	CreateFramebuffers();
	CreateRenderPass(ColorFormat, DepthFormat);
	
	CreateFence();

	//CreateSemaphore();

	//CreateSetupCommandBuffer();
	//FlushSetupCommandBuffer();
	//CreateSetupCommandBuffer();
	
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
	
	//WaitForFence();
}
void VK::OnDestroy(HWND hWnd, HINSTANCE hInstance)
{
	Super::OnDestroy(hWnd, hInstance);

	//WaitForFence();
	vkDestroyFence(Device, Fence, nullptr);

#pragma region RenderPass
	if (VK_NULL_HANDLE != RenderPass) {
		vkDestroyRenderPass(Device, RenderPass, nullptr);
	}
	for (auto i : Framebuffers) {
		vkDestroyFramebuffer(Device, i, nullptr);
	}
#pragma endregion

#pragma region UniformBuffer
	if (VK_NULL_HANDLE != UniformDeviceMemory) {
		vkFreeMemory(Device, UniformDeviceMemory, nullptr);
	}
	if (VK_NULL_HANDLE != UniformBuffer) {
		vkDestroyBuffer(Device, UniformBuffer, nullptr);
	}
#pragma endregion
#pragma region IndexBuffer
	if (VK_NULL_HANDLE != IndexDeviceMemory) {
		vkFreeMemory(Device, IndexDeviceMemory, nullptr);
	}
	if (VK_NULL_HANDLE != IndexBuffer) {
		vkDestroyBuffer(Device, IndexBuffer, nullptr);
	}
#pragma endregion
#pragma region VertexBuffer
	if (VK_NULL_HANDLE != VertexDeviceMemory) {
		vkFreeMemory(Device, VertexDeviceMemory, nullptr);
	}
	if (VK_NULL_HANDLE != VertexBuffer) {
		vkDestroyBuffer(Device, VertexBuffer, nullptr);
	}
#pragma endregion

#pragma region Pipeline
	if (VK_NULL_HANDLE != Pipeline) {
		vkDestroyPipeline(Device, Pipeline, nullptr);
	}
	if (VK_NULL_HANDLE != PipelineCache) {
		vkDestroyPipelineCache(Device, PipelineCache, nullptr);
	}
#pragma endregion

#pragma region DescriptorSet
	vkFreeDescriptorSets(Device, DescriptorPool, static_cast<uint32_t>(DescriptorSets.size()), DescriptorSets.data());
	if (VK_NULL_HANDLE != DescriptorPool) {
		vkDestroyDescriptorPool(Device, DescriptorPool, nullptr);
	}
	if (VK_NULL_HANDLE != PipelineLayout) {
		vkDestroyPipelineLayout(Device, PipelineLayout, nullptr);
	}
	for (auto i : DescriptorSetLayouts) {
		vkDestroyDescriptorSetLayout(Device, i, nullptr);
	}
#pragma endregion
#pragma region Shader
	for (auto i : ShaderModules) {
		vkDestroyShaderModule(Device, i, nullptr);
	}
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
	if (VK_NULL_HANDLE != Swapchain) {
		vkDestroySwapchainKHR(Device, Swapchain, nullptr);
	}
#pragma endregion

#pragma region CommandBuffer
	vkFreeCommandBuffers(Device, CommandPool, static_cast<uint32_t>(CommandBuffers.size()), CommandBuffers.data());
	if (VK_NULL_HANDLE != CommandPool) {
		vkDestroyCommandPool(Device, CommandPool, nullptr);
	}
#pragma endregion

#pragma region Device
	if (VK_NULL_HANDLE != Device) {
		vkDestroyDevice(Device, nullptr);
	}
	if (VK_NULL_HANDLE != Instance) {
		//vkDestroyInstance(Instance, &AllocationCallbacks);
		vkDestroyInstance(Instance, nullptr);
	}
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
VkShaderModule VK::CreateShaderModule(const std::wstring& Path) const
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
		VK_API_VERSION_1_0
	};
	const std::vector<const char*> EnabledExtensions = {
		VK_KHR_SURFACE_EXTENSION_NAME,
		VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
#ifdef _DEBUG
		VK_EXT_DEBUG_REPORT_EXTENSION_NAME
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

#ifdef _DEBUG
	std::cout << "CreateInstace" << COUT_OK << std::endl << std::endl;
#endif
}
VkPhysicalDevice VK::CreateDevice()
{
	uint32_t PhysicalDeviceCount = 0;
	VERIFY_SUCCEEDED(vkEnumeratePhysicalDevices(Instance, &PhysicalDeviceCount, nullptr));
	assert(PhysicalDeviceCount);
	std::vector<VkPhysicalDevice> PhysicalDevices(PhysicalDeviceCount);
	VERIFY_SUCCEEDED(vkEnumeratePhysicalDevices(Instance, &PhysicalDeviceCount, PhysicalDevices.data()));

#ifdef _DEBUG
	std::cout << Yellow << "\t" << "PhysicalDevices" << White << std::endl;
	for (const auto& i : PhysicalDevices) {
		VkPhysicalDeviceProperties PhysicalDeviceProperties;
		vkGetPhysicalDeviceProperties(i, &PhysicalDeviceProperties);
		std::cout << "\t" << "\t" << PhysicalDeviceProperties.deviceName << std::endl;

		//VkPhysicalDeviceFeatures PhysicalDeviceFeatures;
		//vkGetPhysicalDeviceFeatures(i, &PhysicalDeviceFeatures);
	}
#endif

	//!< ここでは最初の物理デバイスを使用することにする
	auto PhysicalDevice = PhysicalDevices[0];

	const auto QueueFamilyIndex = CreateDevice(PhysicalDevice);
	CreateCommandPool(QueueFamilyIndex);

	return PhysicalDevice;
}
uint32_t VK::CreateDevice(VkPhysicalDevice PhysicalDevice)
{
	uint32_t QueueFamilyPropertyCount;
	vkGetPhysicalDeviceQueueFamilyProperties(PhysicalDevice, &QueueFamilyPropertyCount, nullptr);
	assert(QueueFamilyPropertyCount);
	std::vector<VkQueueFamilyProperties> QueueProperties(QueueFamilyPropertyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(PhysicalDevice, &QueueFamilyPropertyCount, QueueProperties.data());

#ifdef _DEBUG
	std::cout << Yellow << "\t" << "QueueProperties" << White << std::endl;
	for (const auto& i : QueueProperties) {
		if (VK_QUEUE_GRAPHICS_BIT & i.queueFlags) { std::cout << Red; }
		std::cout << "\t" << "\t" << "QueueFlags = " << i.queueFlags << std::endl;
		std::cout << White;
	}
#endif

	for (uint32_t i = 0; i < QueueFamilyPropertyCount; ++i) {
		if (VK_QUEUE_GRAPHICS_BIT & QueueProperties[i].queueFlags) {
#ifdef _DEBUG
			std::cout << "\t" << "QueueFamilyIndex = " << i << std::endl;
#endif
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

			vkGetDeviceQueue(Device, i, 0, &Queue);

#ifdef _DEBUG
			std::cout << "CreateDevice" << COUT_OK << std::endl << std::endl;
#endif
			return i;
		}
	}
	assert(false && "QueueFamilyIndex not found");
	return 0;
}
void VK::CreateCommandPool(const uint32_t QueueFamilyIndex)
{
	const VkCommandPoolCreateInfo CommandPoolInfo = {
		VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
		nullptr,
		VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
		QueueFamilyIndex
	};
	VERIFY_SUCCEEDED(vkCreateCommandPool(Device, &CommandPoolInfo, nullptr, &CommandPool));

#ifdef _DEBUG
	std::cout << "CreateCommandPool" << COUT_OK << std::endl << std::endl;
#endif
}

void VK::CreateCommandBuffer()
{
	CommandBuffers.resize(1);
	const VkCommandBufferAllocateInfo CommandBufferAllocateInfo = {
		VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		nullptr,
		CommandPool,
		VK_COMMAND_BUFFER_LEVEL_PRIMARY,
		static_cast<uint32_t>(CommandBuffers.size())
	};
	VERIFY_SUCCEEDED(vkAllocateCommandBuffers(Device, &CommandBufferAllocateInfo, CommandBuffers.data()));

#ifdef _DEBUG
	std::cout << "CreateCommandBuffer" << COUT_OK << std::endl << std::endl;
#endif
}

void VK::CreateSwapchain(HWND hWnd, HINSTANCE hInstance, VkPhysicalDevice PhysicalDevice, const VkFormat ColorFormat)
{
	VkSurfaceKHR Surface = VK_NULL_HANDLE;
#ifdef _WIN32
	const VkWin32SurfaceCreateInfoKHR SurfaceCreateInfo = {
		VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
		nullptr,
		0,
		hInstance,
		hWnd
	};
	VERIFY_SUCCEEDED(vkCreateWin32SurfaceKHR(Instance, &SurfaceCreateInfo, nullptr, &Surface));
#ifdef _DEBUG
	std::cout << "\t" << "Surface" << std::endl;
#endif
#endif

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
#ifdef _DEBUG
		std::cout << "\t" << "Swapchain" << std::endl;
#endif
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
#ifdef _DEBUG
	for (const auto& i : SwapchainImages) {
		std::cout << "\t" << "SwapchainImage" << std::endl;
	}
#endif
#pragma endregion

#pragma region SwapchainImageView
	SwapchainImageViews.resize(SwapchainImageCount);
	for (uint32_t i = 0; i < SwapchainImageCount; ++i) {
		SetImageLayout(/*SetupCommandBuffer*/CommandBuffers[0], SwapchainImages[i], VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
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
#ifdef _DEBUG
		std::cout << "\t" << "SwapchainImageView" << std::endl;
#endif
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

#ifdef _DEBUG
	std::cout << "CreateSwapchain" << COUT_OK << std::endl << std::endl;
#endif
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
#ifdef _DEBUG
	std::cout << "\t" << "DepthStencilImage" << std::endl;
#endif
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
	SetImageLayout(/*SetupCommandBuffer*/CommandBuffers[0], DepthStencilImage, VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
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
#ifdef _DEBUG
	std::cout << "\t" << "DepthStencilImageView" << std::endl;
#endif
#pragma endregion

#ifdef _DEBUG
	std::cout << "CreateDepthStencil" << COUT_OK << std::endl << std::endl;
#endif
}

void VK::CreateShader()
{
	ShaderModules.push_back(CreateShaderModule(SHADER_PATH L"VS.vert.spv"));
	ShaderModules.push_back(CreateShaderModule(SHADER_PATH L"FS.frag.spv"));

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

#ifdef _DEBUG
	std::cout << "CreateShader" << COUT_OK << std::endl << std::endl;
#endif
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

#ifdef _DEBUG
	std::cout << "CreateDescriptorSetLayout" << COUT_OK << std::endl << std::endl;
#endif
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

#ifdef _DEBUG
	std::cout << "CreatePipelineLayout" << COUT_OK << std::endl << std::endl;
#endif
}
void VK::CreateDescriptorSet()
{
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

#ifdef _DEBUG
	std::cout << "CreateDescriptorSet" << COUT_OK << std::endl << std::endl;
#endif
}

void VK::CreateVertexInput()
{
	Vertex Vertices;
	VertexInputBindingDescriptions = {
		{ 0, sizeof(Vertex), VK_VERTEX_INPUT_RATE_VERTEX }
	};
	VertexInputAttributeDescriptions = {
		{ 0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0 },
		{ 1, 0, VK_FORMAT_R32G32B32_SFLOAT, sizeof(std::get<0>(Vertex())) }
	};
	PipelineVertexInputStateCreateInfo = {
		VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
		nullptr,
		0,
		static_cast<uint32_t>(VertexInputBindingDescriptions.size()), VertexInputBindingDescriptions.data(),
		static_cast<uint32_t>(VertexInputAttributeDescriptions.size()), VertexInputAttributeDescriptions.data()
	};

#ifdef _DEBUG
	std::cout << "CreateVertexInput" << COUT_OK << std::endl << std::endl;
#endif
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

#ifdef _DEBUG
	std::cout << "CreateViewport" << COUT_OK << std::endl << std::endl;
#endif
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

	const VkPipelineRasterizationStateCreateInfo PipelineRasterizationStateCreateInfo = {
		VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
		nullptr,
		0,
		VK_FALSE,
		VK_FALSE,
		VK_POLYGON_MODE_FILL,
		VK_CULL_MODE_NONE,
		VK_FRONT_FACE_COUNTER_CLOCKWISE,
		VK_FALSE, 0.0f, 0.0f, 0.0f,
		0.0f
	};

	const VkPipelineMultisampleStateCreateInfo PipelineMultisampleStateCreateInfo = {
		VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
		nullptr,
		0,
		VK_SAMPLE_COUNT_1_BIT,
		VK_FALSE, 0.0f,
		nullptr,
		VK_FALSE, VK_FALSE
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
		{
			VK_STENCIL_OP_KEEP,
			VK_STENCIL_OP_KEEP,
			VK_STENCIL_OP_KEEP,
			VK_COMPARE_OP_NEVER, 0, 0, 0
		},
		{
			VK_STENCIL_OP_KEEP,
			VK_STENCIL_OP_KEEP,
			VK_STENCIL_OP_KEEP,
			VK_COMPARE_OP_ALWAYS, 0, 0, 0
		},
		0.0f, 0.0f
	};

	const std::vector<VkPipelineColorBlendAttachmentState> VkPipelineColorBlendAttachmentStates = {
		{
			VK_FALSE,
			VK_BLEND_FACTOR_ZERO, VK_BLEND_FACTOR_ZERO,
			VK_BLEND_OP_ADD,
			VK_BLEND_FACTOR_ZERO, VK_BLEND_FACTOR_ZERO,
			VK_BLEND_OP_ADD,
			0,
		}
	};
	const VkPipelineColorBlendStateCreateInfo PipelineColorBlendStateCreateInfo = {
		VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
		nullptr,
		0,
		VK_FALSE, VK_LOGIC_OP_CLEAR,
		static_cast<uint32_t>(VkPipelineColorBlendAttachmentStates.size()), VkPipelineColorBlendAttachmentStates.data(),
		{ 0.0f, 0.0f, 0.0f, 0.0f }
	};

	const std::vector<VkDynamicState> DynamicStates = {
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR
	};
	const VkPipelineDynamicStateCreateInfo PipelineDynamicStateCreateInfo = {
		VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
		nullptr,
		0,
		static_cast<uint32_t>(DynamicStates.size()), DynamicStates.data()
	};

	//!< パイプラインをコンパイルして、vkGetPipelineCacheData()でディスクへ保存可能
	const VkPipelineCacheCreateInfo PipelineCacheCreateInfo = {
		VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO,
		nullptr,
		0,
		0, nullptr
	};
	VERIFY_SUCCEEDED(vkCreatePipelineCache(Device, &PipelineCacheCreateInfo, nullptr, &PipelineCache));

	const std::vector<VkGraphicsPipelineCreateInfo> GraphicsPipelineCreateInfos = {
		{
			VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
			nullptr,
			0,
			static_cast<uint32_t>(PipelineShaderStageCreateInfos.size()), PipelineShaderStageCreateInfos.data(),
			&PipelineVertexInputStateCreateInfo,
			&PipelineInputAssemblyStateCreateInfo,
			nullptr,
			&PipelineViewportStateCreateInfo,
			&PipelineRasterizationStateCreateInfo,
			&PipelineMultisampleStateCreateInfo,
			&PipelineDepthStencilStateCreateInfo,
			&PipelineColorBlendStateCreateInfo,
			&PipelineDynamicStateCreateInfo,
			PipelineLayout,
			RenderPass,
			0,
			VK_NULL_HANDLE, 0
		}
	};
	VERIFY_SUCCEEDED(vkCreateGraphicsPipelines(Device, PipelineCache, static_cast<uint32_t>(GraphicsPipelineCreateInfos.size()), GraphicsPipelineCreateInfos.data(), nullptr, &Pipeline));

#ifdef _DEBUG
	std::cout << "CreatePipeline" << COUT_OK << std::endl << std::endl;
#endif
}

void VK::CreateVertexBuffer(const VkPhysicalDeviceMemoryProperties& PhysicalDeviceMemoryProperties)
{
	const std::vector<Vertex> Vertices = {
		{ Vertex({ 0.0f,   0.25f, 0.0f }, { 0.0f, 0.0f, 0.0f, 1.0f }) },
		{ Vertex({ 0.25f, -0.25f, 0.0f }, { 0.0f, 0.0f, 0.0f, 1.0f }) },
		{ Vertex({ -0.25f, -0.25f, 0.0f }, { 0.0f, 0.0f, 0.0f, 1.0f }) },
	};

	const auto Size = sizeof(Vertices);
	const auto Stride = sizeof(Vertices[0]);

	VkBuffer StagingBuffer;
	VkDeviceMemory StagingDeviceMemory;

#pragma region Staging
	{
		const VkBufferCreateInfo BufferCreateInfo = {
			VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
			nullptr,
			0,
			Size,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_SHARING_MODE_EXCLUSIVE,
			0, nullptr
		};
		VERIFY_SUCCEEDED(vkCreateBuffer(Device, &BufferCreateInfo, nullptr, &StagingBuffer));

		VkMemoryRequirements MemoryRequirements;
		vkGetBufferMemoryRequirements(Device, StagingBuffer, &MemoryRequirements);
		const VkMemoryAllocateInfo MemoryAllocateInfo = {
			VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
			nullptr,
			MemoryRequirements.size,
			GetMemoryType(PhysicalDeviceMemoryProperties, MemoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
		};
		VERIFY_SUCCEEDED(vkAllocateMemory(Device, &MemoryAllocateInfo, nullptr, &StagingDeviceMemory));

		void *Data;
		VERIFY_SUCCEEDED(vkMapMemory(Device, StagingDeviceMemory, 0, /*MemoryAllocateInfo.allocationSize*/Size, 0, &Data)); {
			memcpy(Data, Vertices.data(), Size);
		} vkUnmapMemory(Device, StagingDeviceMemory);
		VERIFY_SUCCEEDED(vkBindBufferMemory(Device, StagingBuffer, StagingDeviceMemory, 0));
	}
#pragma endregion

#pragma region VRAM
	{
		const VkBufferCreateInfo BufferCreateInfo = {
			VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
			nullptr,
			0,
			Size,
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VK_SHARING_MODE_EXCLUSIVE,
			0, nullptr
		};
		VERIFY_SUCCEEDED(vkCreateBuffer(Device, &BufferCreateInfo, nullptr, &VertexBuffer));

		VkMemoryRequirements MemoryRequirements;
		vkGetBufferMemoryRequirements(Device, VertexBuffer, &MemoryRequirements);

		const VkMemoryAllocateInfo MemoryAllocateInfo = {
			VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
			nullptr,
			MemoryRequirements.size,
			GetMemoryType(PhysicalDeviceMemoryProperties, MemoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
		};
		VERIFY_SUCCEEDED(vkAllocateMemory(Device, &MemoryAllocateInfo, nullptr, &VertexDeviceMemory));
		VERIFY_SUCCEEDED(vkBindBufferMemory(Device, VertexBuffer, VertexDeviceMemory, 0));
	}
#pragma endregion

#pragma region ToVRAMCommand
	{
		const VkCommandBufferAllocateInfo CommandBufferAllocateInfo = {
			VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
			nullptr,
			CommandPool,
			VK_COMMAND_BUFFER_LEVEL_PRIMARY,
			1
		};
		VkCommandBuffer CopyCommandBuffer;
		VERIFY_SUCCEEDED(vkAllocateCommandBuffers(Device, &CommandBufferAllocateInfo, &CopyCommandBuffer)); {
			const VkCommandBufferBeginInfo CommandBufferBeginInfo = {
				VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
				nullptr,
				0,
				nullptr
			};
			VERIFY_SUCCEEDED(vkBeginCommandBuffer(CopyCommandBuffer, &CommandBufferBeginInfo)); {
				const VkBufferCopy BufferCopy = {
					0,
					0,
					Size
				};
				vkCmdCopyBuffer(CopyCommandBuffer, StagingBuffer, VertexBuffer, 1, &BufferCopy);
			} VERIFY_SUCCEEDED(vkEndCommandBuffer(CopyCommandBuffer));

			const VkSubmitInfo SubmitInfo = {
				VK_STRUCTURE_TYPE_SUBMIT_INFO,
				nullptr,
				0, nullptr,
				nullptr,
				1, &CopyCommandBuffer,
				0, nullptr
			};
			VERIFY_SUCCEEDED(vkQueueSubmit(Queue, 1, &SubmitInfo, VK_NULL_HANDLE));
			VERIFY_SUCCEEDED(vkQueueWaitIdle(Queue));
		} vkFreeCommandBuffers(Device, CommandPool, 1, &CopyCommandBuffer);
	} 
#pragma endregion

	vkDestroyBuffer(Device, StagingBuffer, nullptr);
	vkFreeMemory(Device, StagingDeviceMemory, nullptr);

#ifdef _DEBUG
	std::cout << "CreateVertexBuffer" << COUT_OK << std::endl << std::endl;
#endif
}

void VK::CreateIndexBuffer(const VkPhysicalDeviceMemoryProperties& PhysicalDeviceMemoryProperties)
{
	const std::vector<uint32_t> Indices = { 0, 1, 2 };

	const auto Size = sizeof(Indices);
	//!< vkCmdDrawIndexed() が引数に取るので覚えておく必要がある
	IndexCount = static_cast<uint32_t>(Indices.size());

	VkBuffer StagingBuffer;
	VkDeviceMemory StagingDeviceMemory;
#pragma region Staging
	{
		const VkBufferCreateInfo BufferCreateInfo = {
			VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
			nullptr,
			0,
			Size,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_SHARING_MODE_EXCLUSIVE,
			0, nullptr
		};
		VERIFY_SUCCEEDED(vkCreateBuffer(Device, &BufferCreateInfo, nullptr, &StagingBuffer));

		VkMemoryRequirements MemoryRequirements;
		vkGetBufferMemoryRequirements(Device, StagingBuffer, &MemoryRequirements);
		const VkMemoryAllocateInfo MemoryAllocateInfo = {
			VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
			nullptr,
			MemoryRequirements.size,
			GetMemoryType(PhysicalDeviceMemoryProperties, MemoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
		};
		VERIFY_SUCCEEDED(vkAllocateMemory(Device, &MemoryAllocateInfo, nullptr, &StagingDeviceMemory));

		void *Data;
		VERIFY_SUCCEEDED(vkMapMemory(Device, StagingDeviceMemory, 0, /*MemoryAllocateInfo.allocationSize*/Size, 0, &Data)); {
			memcpy(Data, Indices.data(), Size);
		} vkUnmapMemory(Device, StagingDeviceMemory);
		VERIFY_SUCCEEDED(vkBindBufferMemory(Device, StagingBuffer, StagingDeviceMemory, 0));
	}
#pragma endregion

#pragma region VRAM
	{
		const VkBufferCreateInfo BufferCreateInfo = {
			VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
			nullptr,
			0,
			Size,
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VK_SHARING_MODE_EXCLUSIVE,
			0, nullptr
		};
		VERIFY_SUCCEEDED(vkCreateBuffer(Device, &BufferCreateInfo, nullptr, &IndexBuffer));

		VkMemoryRequirements MemoryRequirements;
		vkGetBufferMemoryRequirements(Device, IndexBuffer, &MemoryRequirements);

		const VkMemoryAllocateInfo MemoryAllocateInfo = {
			VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
			nullptr,
			MemoryRequirements.size,
			GetMemoryType(PhysicalDeviceMemoryProperties, MemoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
		};
		VERIFY_SUCCEEDED(vkAllocateMemory(Device, &MemoryAllocateInfo, nullptr, &IndexDeviceMemory));
		VERIFY_SUCCEEDED(vkBindBufferMemory(Device, IndexBuffer, IndexDeviceMemory, 0));
	}
#pragma endregion

#pragma region ToVRAMCommand
	{
		const VkCommandBufferAllocateInfo CommandBufferAllocateInfo = {
			VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
			nullptr,
			CommandPool,
			VK_COMMAND_BUFFER_LEVEL_PRIMARY,
			1
		};
		VkCommandBuffer CopyCommandBuffer;
		VERIFY_SUCCEEDED(vkAllocateCommandBuffers(Device, &CommandBufferAllocateInfo, &CopyCommandBuffer)); {
			const VkCommandBufferBeginInfo CommandBufferBeginInfo = {
				VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
				nullptr,
				0,
				nullptr
			};
			VERIFY_SUCCEEDED(vkBeginCommandBuffer(CopyCommandBuffer, &CommandBufferBeginInfo)); {
				const VkBufferCopy BufferCopy = {
					0,
					0,
					Size
				};
				vkCmdCopyBuffer(CopyCommandBuffer, StagingBuffer, IndexBuffer, 1, &BufferCopy);
			} VERIFY_SUCCEEDED(vkEndCommandBuffer(CopyCommandBuffer));

			const VkSubmitInfo SubmitInfo = {
				VK_STRUCTURE_TYPE_SUBMIT_INFO,
				nullptr,
				0, nullptr,
				nullptr,
				1, &CopyCommandBuffer,
				0, nullptr
			};
			VERIFY_SUCCEEDED(vkQueueSubmit(Queue, 1, &SubmitInfo, VK_NULL_HANDLE));
			VERIFY_SUCCEEDED(vkQueueWaitIdle(Queue));
		} vkFreeCommandBuffers(Device, CommandPool, 1, &CopyCommandBuffer);
	}
#pragma endregion

	vkDestroyBuffer(Device, StagingBuffer, nullptr);
	vkFreeMemory(Device, StagingDeviceMemory, nullptr);

#ifdef _DEBUG
	std::cout << "CreateIndexBuffer" << COUT_OK << std::endl << std::endl;
#endif
}

void VK::CreateUniformBuffer(const VkPhysicalDeviceMemoryProperties& PhysicalDeviceMemoryProperties)
{
	const auto Size = sizeof(glm::mat4) * 3; //!< World, View, Projection

	const VkBufferCreateInfo BufferCreateInfo = {
		VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
		nullptr,
		0,
		Size,
		VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
		VK_SHARING_MODE_EXCLUSIVE,
		0, nullptr
	};
	VERIFY_SUCCEEDED(vkCreateBuffer(Device, &BufferCreateInfo, nullptr, &UniformBuffer));

	VkMemoryRequirements MemoryRequirements;
	vkGetBufferMemoryRequirements(Device, UniformBuffer, &MemoryRequirements); 
	const VkMemoryAllocateInfo MemoryAllocateInfo = {
		VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
		nullptr,
		MemoryRequirements.size,
		GetMemoryType(PhysicalDeviceMemoryProperties, MemoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
	};
	VERIFY_SUCCEEDED(vkAllocateMemory(Device, &MemoryAllocateInfo, nullptr, &UniformDeviceMemory));
	VERIFY_SUCCEEDED(vkBindBufferMemory(Device, UniformBuffer, UniformDeviceMemory, 0));

	UniformDescriptorBufferInfo = {
		UniformBuffer,
		0,
		Size
	};

	WriteDescriptorSets = {
		{
			// todo
			VK_STRUCTURE_TYPE_APPLICATION_INFO,
			nullptr,
			//VkStructureType                  sType;
			//const void*                      pNext;
			//VkDescriptorSet                  dstSet;
			//uint32_t                         dstBinding;
			//uint32_t                         dstArrayElement;
			//uint32_t                         descriptorCount;
			//VkDescriptorType                 descriptorType;
			//const VkDescriptorImageInfo*     pImageInfo;
			//const VkDescriptorBufferInfo*    pBufferInfo;
			//const VkBufferView*              pTexelBufferView;
		}
	};

	//todo
	//vkUpdateDescriptorSets(Device, static_cast<uint32_t>(WriteDescriptorSets.size()), WriteDescriptorSets.data(), 0, nullptr);

#ifdef _DEBUG
	std::cout << "CreateUniformBuffer" << COUT_OK << std::endl << std::endl;
#endif
}

//void VK::CreateSemaphore()
//{
//	const VkSemaphoreCreateInfo SemaphoreCreateInfo = {
//		VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
//		nullptr,
//		0
//	};
//	VERIFY_SUCCEEDED(vkCreateSemaphore(Device, &SemaphoreCreateInfo, nullptr, &PresentSemaphore));
//	VERIFY_SUCCEEDED(vkCreateSemaphore(Device, &SemaphoreCreateInfo, nullptr, &RenderSemaphore));
//}
//void VK::CreateSetupCommandBuffer()
//{
//	if (SetupCommandBuffer != VK_NULL_HANDLE) {
//		vkFreeCommandBuffers(Device, CommandPool, 1, &SetupCommandBuffer);
//		SetupCommandBuffer = VK_NULL_HANDLE;
//	}
//
//	const VkCommandBufferAllocateInfo CommandBufferAllocateInfo = {
//		VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
//		nullptr,
//		CommandPool,
//		VK_COMMAND_BUFFER_LEVEL_PRIMARY,
//		1
//	};
//	VERIFY_SUCCEEDED(vkAllocateCommandBuffers(Device, &CommandBufferAllocateInfo, &SetupCommandBuffer));
//
//	const VkCommandBufferBeginInfo CommandBufferBeginInfo = {
//		VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
//		nullptr,
//		0,
//		nullptr
//	};
//	VERIFY_SUCCEEDED(vkBeginCommandBuffer(SetupCommandBuffer, &CommandBufferBeginInfo));
//}

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

//void VK::FlushSetupCommandBuffer()
//{
//	if (SetupCommandBuffer != VK_NULL_HANDLE) {
//		VERIFY_SUCCEEDED(vkEndCommandBuffer(SetupCommandBuffer));
//
//		const VkSubmitInfo SubmitInfo = {
//			VK_STRUCTURE_TYPE_SUBMIT_INFO,
//			nullptr,
//			0, nullptr,
//			nullptr,
//			1, &SetupCommandBuffer,
//			0, nullptr
//		};
//		VERIFY_SUCCEEDED(vkQueueSubmit(Queue, 1, &SubmitInfo, VK_NULL_HANDLE));
//
//		VERIFY_SUCCEEDED(vkQueueWaitIdle(Queue));
//
//		vkFreeCommandBuffers(Device, CommandPool, 1, &SetupCommandBuffer);
//		SetupCommandBuffer = VK_NULL_HANDLE;
//	}
//}

void VK::CreateFence()
{
	const VkFenceCreateInfo FenceCreateInfo = {
		VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
		nullptr,
		0
	};
	VERIFY_SUCCEEDED(vkCreateFence(Device, &FenceCreateInfo, nullptr, &Fence));
#ifdef _DEBUG
	std::cout << "CreateFence" << COUT_OK << std::endl << std::endl;
#endif
}

void VK::PopulateCommandBuffer()
{
	//VERIFY_SUCCEEDED(vkBeginCommandBuffer(drawCmdBuffers[i], &cmdBufInfo)); {
	//vkCmdBeginRenderPass(drawCmdBuffers[i], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
	//vkCmdBindPipeline(drawCmdBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelines.solid);
	//vkCmdPipelineBarrier();
	//} VERIFY_SUCCEEDED(vkEndCommandBuffer(drawCmdBuffers[i]));

	//!< バッファインデックスの更新
	//VERIFY_SUCCEEDED(vkAcquireNextImageKHR(Device, Swapchain, UINT64_MAX, PresentSemaphore, nullptr, &SwapchainImageIndex));

	vkCmdSetViewport(CommandBuffers[SwapchainImageIndex], 0, static_cast<uint32_t>(Viewports.size()), Viewports.data());
	vkCmdSetScissor(CommandBuffers[SwapchainImageIndex], 0, static_cast<uint32_t>(ScissorRects.size()), ScissorRects.data());

	{
		const VkClearColorValue ClearColor = {
			0.5f, 0.5f, 1.0f, 1.0f
		};
		const std::vector<VkImageSubresourceRange> Ranges = {
			{
				VK_IMAGE_ASPECT_COLOR_BIT,
				0, 1,
				0, 1
			}
		};
		vkCmdClearColorImage(CommandBuffers[SwapchainImageIndex],
			SwapchainImages[SwapchainImageIndex],
			VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			&ClearColor,
			static_cast<uint32_t>(Ranges.size()), Ranges.data());
	}
	{
		const VkClearDepthStencilValue ClearDepthStencil = {
			1.0f, 0
		};
		const std::vector<VkImageSubresourceRange> Ranges = {
			{
				VK_IMAGE_ASPECT_DEPTH_BIT,
				0, 1,
				0, 1
			}
		};
		vkCmdClearDepthStencilImage(CommandBuffers[SwapchainImageIndex],
			DepthStencilImage,
			VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
			&ClearDepthStencil,
			static_cast<uint32_t>(Ranges.size()), Ranges.data());
	}

	//const VkDeviceSize Offsets[] = { 0 };
	//vkCmdBindVertexBuffers(CommandBuffers[SwapchainImageIndex], 0, 1, &VertexBuffer, Offsets);
	//vkCmdBindIndexBuffer(CommandBuffers[SwapchainImageIndex], IndexBuffer, 0, VK_INDEX_TYPE_UINT32);

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
	VERIFY_SUCCEEDED(vkQueueSubmit(Queue, 1, &SubmitInfo, Fence));
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
		&SwapchainImageIndex,
		nullptr
	};
#if 0
	if (VK_NULL_HANDLE != RenderSemaphore) {
		PresentInfo.waitSemaphoreCount = 1;
		PresentInfo.pWaitSemaphores = &RenderSemaphore;
	}
#endif
	//todo
	//VERIFY_SUCCEEDED(vkQueuePresentKHR(Queue, &PresentInfo));
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
