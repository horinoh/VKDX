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
#ifdef _DEBUG
	PerformanceCounter PC("OnCreate : ");
#endif

	Super::OnCreate(hWnd, hInstance);

	CreateInstance();
	PhysicalDevice = EnumeratePhysicalDevice();
	//!< GetQueueFamily() 内で CreateDevice() される
	GetQueueFamily(PhysicalDevice);
	GetDeviceQueue(Device, GraphicsQueueFamilyIndex);
	
	CreateCommandPool(GraphicsQueueFamilyIndex);
	auto CommandPool = CommandPools[0];
	CreateCommandBuffer(CommandPool);
	
	CreateFence();
	CreateSemaphore();

	const auto ColorFormat = VK_FORMAT_B8G8R8A8_UNORM;
	CreateSurface(hWnd, hInstance);
	CreateSwapchainClientRect(PhysicalDevice);
	auto CommandBuffer = CommandBuffers[0];
	CreateSwapchainImageView(CommandBuffer, ColorFormat);

	const auto DepthFormat = GetSupportedDepthFormat(PhysicalDevice);
	{
		CreateDepthStencilImage(DepthFormat);
		CreateDepthStencilDeviceMemory(PhysicalDeviceMemoryProperties);
		CreateDepthStencilView(CommandBuffer, DepthFormat);
	}

	CreateShader();

	{
		CreateDescriptorSetLayout();

		CreateDescritporPool();
		CreateDescriptorSet(DescriptorPool);

		CreatePipelineLayout();
	}

	CreateVertexInput();

	CreateViewport(static_cast<float>(ImageExtent.width), static_cast<float>(ImageExtent.height));
	CreateRenderPass(ColorFormat, DepthFormat);
	CreatePipeline();
	CreateFramebuffer();

	CreateVertexBuffer(CommandPool, PhysicalDeviceMemoryProperties);
	CreateIndexBuffer(CommandPool, PhysicalDeviceMemoryProperties);
	//CreateUniformBuffer(PhysicalDeviceMemoryProperties);
}
void VK::OnSize(HWND hWnd, HINSTANCE hInstance)
{
#ifdef _DEBUG
	PerformanceCounter PC("OnSize : ");
#endif

	Super::OnSize(hWnd, hInstance);
	
	CreateViewport(static_cast<float>(ImageExtent.width), static_cast<float>(ImageExtent.height));
}
void VK::OnTimer(HWND hWnd, HINSTANCE hInstance)
{
	Super::OnTimer(hWnd, hInstance);
}
void VK::OnPaint(HWND hWnd, HINSTANCE hInstance)
{
	Super::OnPaint(hWnd, hInstance);

	Draw();
}
void VK::OnDestroy(HWND hWnd, HINSTANCE hInstance)
{
	Super::OnDestroy(hWnd, hInstance);

	//!< GPUの完了を待たなくてはならない
	WaitForFence();
	
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

	for (auto i : Framebuffers) {
		vkDestroyFramebuffer(Device, i, nullptr);
	}
	if (VK_NULL_HANDLE != RenderPass) {
		vkDestroyRenderPass(Device, RenderPass, nullptr);
	}
#pragma region Pipeline
	if (VK_NULL_HANDLE != Pipeline) {
		vkDestroyPipeline(Device, Pipeline, nullptr);
	}
	if (VK_NULL_HANDLE != PipelineCache) {
		vkDestroyPipelineCache(Device, PipelineCache, nullptr);
	}
#pragma endregion

#pragma region Layout
	if (VK_NULL_HANDLE != PipelineLayout) {
		vkDestroyPipelineLayout(Device, PipelineLayout, nullptr);
	}
	if(!DescriptorSets.empty()) {
		vkFreeDescriptorSets(Device, DescriptorPool, static_cast<uint32_t>(DescriptorSets.size()), DescriptorSets.data());
	}
#pragma endregion
	
	for (auto i : DescriptorSetLayouts) {
		vkDestroyDescriptorSetLayout(Device, i, nullptr);
	}
	if (VK_NULL_HANDLE != DescriptorPool) {
		vkDestroyDescriptorPool(Device, DescriptorPool, nullptr);
	}

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
	for (auto i : SwapchainImageViews) {
		vkDestroyImageView(Device, i, nullptr);
	}
	for (auto i : SwapchainImages) {
		vkDestroyImage(Device, i, nullptr);
	}
	if (VK_NULL_HANDLE != Swapchain) {
		vkDestroySwapchainKHR(Device, Swapchain, nullptr);
	}
	if (VK_NULL_HANDLE != Surface) {
		vkDestroySurfaceKHR(Instance, Surface, nullptr);
	}
#pragma endregion

	for (auto i : RenderSemaphores) {
		vkDestroySemaphore(Device, i, nullptr);
	}
	for (auto i : PresentSemaphores) {
		vkDestroySemaphore(Device, i, nullptr);
	}

	if (VK_NULL_HANDLE != Fence) {
		vkDestroyFence(Device, Fence, nullptr);
	}

#pragma region CommandBuffer
	vkFreeCommandBuffers(Device, CommandPools[0], 1, &CommandBuffers[0]);
	//vkFreeCommandBuffers(Device, CommandPools[1], 1, &CommandBuffers[1]);
	for (auto i : CommandPools) {
		vkDestroyCommandPool(Device, i, nullptr);
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

std::string VK::GetVkResultString(const VkResult Result)
{
	//!< 16進の文字列表記
	//std::stringstream ss;
	//ss << "0x" << std::hex << Result << std::dec;
	//ss.str();

#define VK_RESULT_ENTRY(vr) case VK_##vr: return #vr;
	switch (Result)
	{
	default: assert(0 && "Unknown VkResult"); return "";
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
	default: assert(0 && "Unknown VkFormat"); return "";
#include "VKFormat.h"
	}
#undef VK_FORMAT_ENTRY
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
	const VkCommandBufferBeginInfo CommandBufferBeginInfo = {
		VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		nullptr,
		0,
		nullptr
	};
	VERIFY_SUCCEEDED(vkBeginCommandBuffer(CommandBuffer, &CommandBufferBeginInfo)); {
		vkCmdPipelineBarrier(CommandBuffer, SrcPipelineStageFlags, DstPipelineStageFlags, 0, 0, nullptr, 0, nullptr, 1, &ImageMemoryBarrier);
	} VERIFY_SUCCEEDED(vkEndCommandBuffer(CommandBuffer));
	const VkSubmitInfo SubmitInfo = {
		VK_STRUCTURE_TYPE_SUBMIT_INFO,
		nullptr,
		0, nullptr,
		nullptr,
		1, &CommandBuffer,
		0, nullptr
	};
	VERIFY_SUCCEEDED(vkQueueSubmit(Queue, 1, &SubmitInfo, VK_NULL_HANDLE));
	VERIFY_SUCCEEDED(vkQueueWaitIdle(Queue));
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

void VK::EnumerateInstanceLayer()
{
	uint32_t InstanceLayerPropertyCount = 0;
	VERIFY_SUCCEEDED(vkEnumerateInstanceLayerProperties(&InstanceLayerPropertyCount, nullptr));
	if (InstanceLayerPropertyCount) {
		std::vector<VkLayerProperties> LayerProperties(InstanceLayerPropertyCount);
		VERIFY_SUCCEEDED(vkEnumerateInstanceLayerProperties(&InstanceLayerPropertyCount, LayerProperties.data()));
		for (const auto& i : LayerProperties) {
			i.layerName;
		}
	}
}
void VK::EnumerateInstanceExtenstion()
{
	uint32_t InstanceExtensionPropertyCount = 0;
	VERIFY_SUCCEEDED(vkEnumerateInstanceExtensionProperties(nullptr, &InstanceExtensionPropertyCount, nullptr));
	if (InstanceExtensionPropertyCount) {
		std::vector<VkExtensionProperties> ExtensionProperties(InstanceExtensionPropertyCount);
		VERIFY_SUCCEEDED(vkEnumerateInstanceExtensionProperties(nullptr, &InstanceExtensionPropertyCount, ExtensionProperties.data()));
		for (const auto& i : ExtensionProperties) {
			i.extensionName;
		}
	}
}

//VkBool32 VKAPI_PTR DebugCallback(VkDebugReportFlagsEXT flags,
//	VkDebugReportObjectTypeEXT objectType,
//	uint64_t object,
//	size_t location,
//	int32_t messageCode,
//	const char* pLayerPrefix,
//	const char* pMessage,
//	void* pUserData)
//{
//	return VK_FALSE;
//}

void VK::CreateInstance()
{
	EnumerateInstanceLayer();
	EnumerateInstanceExtenstion();

	const VkApplicationInfo ApplicationInfo = {
		VK_STRUCTURE_TYPE_APPLICATION_INFO,
		nullptr,
		"VKDX App", 0,
		"VKDX Engine", 0,
		VK_API_VERSION_1_0
	};
	const std::vector<const char*> EnabledExtensions = {
		VK_KHR_SURFACE_EXTENSION_NAME,
#ifdef _WIN32
		VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
#endif
#ifdef _DEBUG
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

	//const VkDebugReportCallbackCreateInfoEXT DebugReportCreateInfo = {
	//	VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT,
	//	nullptr,
	//	VK_DEBUG_REPORT_INFORMATION_BIT_EXT
	//	| VK_DEBUG_REPORT_WARNING_BIT_EXT 
	//	| VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT 
	//	| VK_DEBUG_REPORT_ERROR_BIT_EXT 
	//	| VK_DEBUG_REPORT_DEBUG_BIT_EXT 
	//	| VK_DEBUG_REPORT_FLAG_BITS_MAX_ENUM_EXT,
	//	DebugCallback,
	//	nullptr
	//};
	//static VkDebugReportCallbackEXT DebugReportCallback;
	//VERIFY_SUCCEEDED(vkCreateDebugReportCallbackEXT(Instance, &DebugReportCreateInfo, nullptr, &DebugReportCallback));

#ifdef _DEBUG
	std::cout << "CreateInstace" << COUT_OK << std::endl << std::endl;
#endif
}
VkPhysicalDevice VK::EnumeratePhysicalDevice()
{
	//!< 物理デバイス(GPU)の列挙
	uint32_t PhysicalDeviceCount = 0;
	VERIFY_SUCCEEDED(vkEnumeratePhysicalDevices(Instance, &PhysicalDeviceCount, nullptr));
	assert(PhysicalDeviceCount && "PhysicalDevice not found");
	std::vector<VkPhysicalDevice> PhysicalDevices(PhysicalDeviceCount);
	VERIFY_SUCCEEDED(vkEnumeratePhysicalDevices(Instance, &PhysicalDeviceCount, PhysicalDevices.data()));
#ifdef _DEBUG
	std::cout << Yellow << "\t" << "PhysicalDevices" << White << std::endl;
#define PHYSICAL_DEVICE_TYPE_ENTRY(entry) if(VK_PHYSICAL_DEVICE_TYPE_##entry == PhysicalDeviceProperties.deviceType) { std::cout << #entry << std::endl; }
	for (const auto& i : PhysicalDevices) {
		VkPhysicalDeviceProperties PhysicalDeviceProperties;
		vkGetPhysicalDeviceProperties(i, &PhysicalDeviceProperties);
		std::cout << "\t" << "\t" << PhysicalDeviceProperties.deviceName << " = ";
		PHYSICAL_DEVICE_TYPE_ENTRY(OTHER);
		PHYSICAL_DEVICE_TYPE_ENTRY(INTEGRATED_GPU);
		PHYSICAL_DEVICE_TYPE_ENTRY(DISCRETE_GPU);
		PHYSICAL_DEVICE_TYPE_ENTRY(VIRTUAL_GPU);
		PHYSICAL_DEVICE_TYPE_ENTRY(CPU);
		std::cout << std::endl;
		PhysicalDeviceProperties.limits;

		VkPhysicalDeviceFeatures PhysicalDeviceFeatures;
		vkGetPhysicalDeviceFeatures(i, &PhysicalDeviceFeatures);
	}
#undef PHYSICAL_DEVICE_TYPE
#endif

	//!< ここでは最初の物理デバイスを使用することにする
	return PhysicalDevices[0];
}
void VK::EnumerateDeviceLayer(VkPhysicalDevice PhysicalDevice)
{
	uint32_t DeviceLayerPropertyCount = 0;
	VERIFY_SUCCEEDED(vkEnumerateDeviceLayerProperties(PhysicalDevice, &DeviceLayerPropertyCount, nullptr));
	if (DeviceLayerPropertyCount) {
		std::vector<VkLayerProperties> LayerProperties(DeviceLayerPropertyCount);
		VERIFY_SUCCEEDED(vkEnumerateDeviceLayerProperties(PhysicalDevice, &DeviceLayerPropertyCount, LayerProperties.data()));
		for (const auto& i : LayerProperties) {
			i.layerName;
		}
	}
}
void VK::EnumerateDeviceExtenstion(VkPhysicalDevice PhysicalDevice)
{
	uint32_t DeviceExtensionPropertyCount = 0;
	VERIFY_SUCCEEDED(vkEnumerateDeviceExtensionProperties(PhysicalDevice, nullptr, &DeviceExtensionPropertyCount, nullptr));
	if (DeviceExtensionPropertyCount) {
		std::vector<VkExtensionProperties> ExtensionProperties(DeviceExtensionPropertyCount);
		VERIFY_SUCCEEDED(vkEnumerateDeviceExtensionProperties(PhysicalDevice, nullptr, &DeviceExtensionPropertyCount, ExtensionProperties.data()));
		for (const auto& i : ExtensionProperties) {
			i.extensionName;
		}
	}
}
void VK::GetQueueFamily(VkPhysicalDevice PhysicalDevice)
{
	EnumerateDeviceLayer(PhysicalDevice);
	EnumerateDeviceExtenstion(PhysicalDevice);

	//!< メモリプロパティを取得
	vkGetPhysicalDeviceMemoryProperties(PhysicalDevice, &PhysicalDeviceMemoryProperties);

	//!< キューのプロパティを列挙
	uint32_t QueueFamilyPropertyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(PhysicalDevice, &QueueFamilyPropertyCount, nullptr);
	assert(QueueFamilyPropertyCount && "QueueFamilyProperty not found");
	std::vector<VkQueueFamilyProperties> QueueProperties(QueueFamilyPropertyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(PhysicalDevice, &QueueFamilyPropertyCount, QueueProperties.data());
#ifdef _DEBUG
	std::cout << Yellow << "\t" << "QueueProperties" << White << std::endl;
#define QUEUE_FLAG_ENTRY(entry) if(VK_QUEUE_##entry##_BIT & i.queueFlags) { std::cout << #entry << " | "; }
	//!< デバイスによっては転送専用キューを持つ、転送を多用する場合は専用キューを使用した方が良い
	for (const auto& i : QueueProperties) {
		std::cout << "\t" << "\t" << "QueueFlags = ";
		QUEUE_FLAG_ENTRY(GRAPHICS);
		QUEUE_FLAG_ENTRY(COMPUTE);
		QUEUE_FLAG_ENTRY(TRANSFER);
		QUEUE_FLAG_ENTRY(SPARSE_BINDING);
		std::cout << std::endl;
	}
#undef QUEUE_PROPS
#endif

	//!< GRAPHICS 機能を持つものを探す
	for (uint32_t i = 0; i < QueueFamilyPropertyCount; ++i) {
		if (VK_QUEUE_GRAPHICS_BIT & QueueProperties[i].queueFlags) {
			GraphicsQueueFamilyIndex = i;
#ifdef _DEBUG
			std::cout << "\t" << "GraphicsQueueFamilyIndex = " << GraphicsQueueFamilyIndex << std::endl;
#endif
			CreateDevice(PhysicalDevice, GraphicsQueueFamilyIndex);
		}
		else if (VK_QUEUE_TRANSFER_BIT & QueueProperties[i].queueFlags) {
			//!< #TODO 
			//!< デバイスによっては転送専用キューを持つ、転送を多用する場合は専用キューを使用した方が良い
			//TransferQueueFamilyIndex = i;
		}
		else if (VK_QUEUE_COMPUTE_BIT & QueueProperties[i].queueFlags) {
			//!< #TODO
			//ComputeQueueFamilyIndex = i;
		}
	}
	assert(UINT_MAX != GraphicsQueueFamilyIndex && "GraphicsQueue not found");
}
void VK::CreateDevice(VkPhysicalDevice PhysicalDevice, const uint32_t QueueFamilyIndex)
{
	const std::vector<float> QueuePriorities = { 0.0f };
	const std::vector<VkDeviceQueueCreateInfo> QueueCreateInfos = {
		{
			VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
			nullptr,
			0,
			QueueFamilyIndex,
			static_cast<uint32_t>(QueuePriorities.size()), QueuePriorities.data()
		}
	};
	const std::vector<const char*> EnabledExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
	const VkDeviceCreateInfo DeviceCreateInfo = {
		VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
		nullptr,
		0,
		static_cast<uint32_t>(QueueCreateInfos.size()), QueueCreateInfos.data(),
		static_cast<uint32_t>(EnabledLayerNames.size()), EnabledLayerNames.data(),
		static_cast<uint32_t>(EnabledExtensions.size()), EnabledExtensions.data(),
		nullptr
	};
	VERIFY_SUCCEEDED(vkCreateDevice(PhysicalDevice, &DeviceCreateInfo, nullptr, &Device));

#ifdef _DEBUG
	std::cout << "CreateDevice" << COUT_OK << std::endl << std::endl;
#endif
}
/**
@brief マルチスレッドで「異なる」キューへサブミットできる
@note DirectX12 はマルチスレッドで「同じ」キューへもサブミットできるので注意
*/
void VK::GetDeviceQueue(VkDevice Device, const uint32_t QueueFamilyIndex)
{
	vkGetDeviceQueue(Device, QueueFamilyIndex, 0/*QueueFamily内でのインデックス*/, &Queue);

#ifdef _DEBUG
	std::cout << "GetDeviceQueue" << COUT_OK << std::endl << std::endl;
#endif
}

void VK::CreateCommandPool(const uint32_t QueueFamilyIndex)
{
	const VkCommandPoolCreateInfo CommandPoolInfo = {
		VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
		nullptr,
		VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
		QueueFamilyIndex
	};

	VkCommandPool CommandPool;
	VERIFY_SUCCEEDED(vkCreateCommandPool(Device, &CommandPoolInfo, nullptr, &CommandPool));
	CommandPools.push_back(CommandPool);

#ifdef _DEBUG
	std::cout << "CreateCommandPool" << COUT_OK << std::endl << std::endl;
#endif
}

void VK::CreateCommandBuffer(const VkCommandPool CommandPool)
{
	VkCommandBuffer CommandBuffer;
	const VkCommandBufferAllocateInfo CommandBufferAllocateInfo = {
		VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		nullptr,
		CommandPool,
		VK_COMMAND_BUFFER_LEVEL_PRIMARY,
		1
	};
	VERIFY_SUCCEEDED(vkAllocateCommandBuffers(Device, &CommandBufferAllocateInfo, &CommandBuffer));

	CommandBuffers.push_back(CommandBuffer);

#ifdef _DEBUG
	std::cout << "CreateCommandBuffer" << COUT_OK << std::endl << std::endl;
#endif
}

/**
@brief CPU と GPU の同期
*/
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

	//!< プレゼント完了同期用
	VkSemaphore PresentSemaphore;
	VERIFY_SUCCEEDED(vkCreateSemaphore(Device, &SemaphoreCreateInfo, nullptr, &PresentSemaphore));
	PresentSemaphores.push_back(PresentSemaphore);

	//!< 描画完了同期用
	VkSemaphore RenderSemaphore;
	VERIFY_SUCCEEDED(vkCreateSemaphore(Device, &SemaphoreCreateInfo, nullptr, &RenderSemaphore));
	RenderSemaphores.push_back(RenderSemaphore);

#ifdef _DEBUG
	std::cout << "CreateSemaphore" << COUT_OK << std::endl << std::endl;
#endif
}

/**
@brief デバイス(GPU)とホスト(CPU)の同期
*/
void VK::CreateSurface(HWND hWnd, HINSTANCE hInstance)
{
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
}

void VK::CreateSwapchain(VkPhysicalDevice PhysicalDevice, const uint32_t Width, const uint32_t Height)
{
	VkBool32 Supported = VK_FALSE;
	VERIFY_SUCCEEDED(vkGetPhysicalDeviceSurfaceSupportKHR(PhysicalDevice, GraphicsQueueFamilyIndex, Surface, &Supported));
	assert(VK_TRUE == Supported && "vkGetPhysicalDeviceSurfaceSupportKHR failed");

	//!< サーフェスの情報を取得
	VkSurfaceCapabilitiesKHR SurfaceCapabilities;
	VERIFY_SUCCEEDED(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(PhysicalDevice, Surface, &SurfaceCapabilities));

	const auto MinImageCount = (std::min)(SurfaceCapabilities.minImageCount + 1, SurfaceCapabilities.maxImageCount);

	//!< イメージのサイズを覚えておく
	ImageExtent = SurfaceCapabilities.currentExtent;
	//!< サーフェスのサイズが未定義の場合は明示的に指定。サーフェスのサイズが定義されている場合はそれに従わないとならない
	if (-1 == SurfaceCapabilities.currentExtent.width) {
		ImageExtent = { Width, Height };
	}
#ifdef _DEBUG
	std::cout << "\t" << "\t" << Lightblue << "ImageExtent" << White << std::endl;
	std::cout << "\t" << "\t" << "\t" << ImageExtent.width << " x " << ImageExtent.height << std::endl;
#endif

	//!< サーフェスを回転、反転させるかどうか。回転させない VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR を指定
	const auto PreTransform = (SurfaceCapabilities.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR) ? VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR : SurfaceCapabilities.currentTransform;

	//!< サーフェスのフォーマットを取得
	uint32_t SurfaceFormatCount;
	VERIFY_SUCCEEDED(vkGetPhysicalDeviceSurfaceFormatsKHR(PhysicalDevice, Surface, &SurfaceFormatCount, nullptr));
	assert(SurfaceFormatCount);
	std::vector<VkSurfaceFormatKHR> SurfaceFormats(SurfaceFormatCount);
	VERIFY_SUCCEEDED(vkGetPhysicalDeviceSurfaceFormatsKHR(PhysicalDevice, Surface, &SurfaceFormatCount, SurfaceFormats.data()));

	//!< 最初のサーフェスのフォーマットにする (VK_FORMAT_UNDEFINED の場合は VK_FORMAT_B8G8R8A8_UNORM)
	const auto ImageFormat = (1 == SurfaceFormatCount && VK_FORMAT_UNDEFINED == SurfaceFormats[0].format) ? VK_FORMAT_B8G8R8A8_UNORM : SurfaceFormats[0].format;
	const auto ImageColorSpace = SurfaceFormats[0].colorSpace;
#ifdef _DEBUG
	std::cout << "\t" << "\t" << Lightblue << "Format" << White << std::endl;
	for (auto& i : SurfaceFormats) {
		if (ImageFormat == i.format) {
			std::cout << Yellow;
		}
		std::cout << "\t" << "\t" << "\t" << GetFormatString(i.format) << std::endl;
		std::cout << White;
	}
	std::cout << std::endl;
#endif

	//!< プレゼントモードの取得
	uint32_t PresentModeCount;
	VERIFY_SUCCEEDED(vkGetPhysicalDeviceSurfacePresentModesKHR(PhysicalDevice, Surface, &PresentModeCount, nullptr));
	assert(PresentModeCount);
	std::vector<VkPresentModeKHR> PresentModes(PresentModeCount);
	VERIFY_SUCCEEDED(vkGetPhysicalDeviceSurfacePresentModesKHR(PhysicalDevice, Surface, &PresentModeCount, PresentModes.data()));
	VkPresentModeKHR PresentMode = VK_PRESENT_MODE_FIFO_KHR;
	//!< (選択できるなら)レイテンシが低く、テアリングの無い MAILBOX を選択
	for (auto i : PresentModes) {
		if (VK_PRESENT_MODE_MAILBOX_KHR == i) {
			PresentMode = i;
			break;
		}
		else if (VK_PRESENT_MODE_IMMEDIATE_KHR == i) {
			PresentMode = i;
		}
	}
#ifdef _DEBUG
	std::cout << "\t" << Lightblue << "Present Mode" << White << std::endl;
#define VK_PRESENT_MODE_ENTRY(entry) case VK_PRESENT_MODE_##entry##_KHR: std::cout << "\t" << "\t" << #entry << std::endl; break;
	for (auto i : PresentModes) {
		if (PresentMode == i) {
			std::cout << Yellow;
		}
		switch (i)
		{
		default: assert(0 && "Unknown VkPresentMode"); break;
		VK_PRESENT_MODE_ENTRY(IMMEDIATE) //!< VSync を待たない、テアリング
		VK_PRESENT_MODE_ENTRY(MAILBOX)   //!< V-Syncを待つ
		VK_PRESENT_MODE_ENTRY(FIFO)		 //!< V-Syncを待つ、レイテンシが低い
		VK_PRESENT_MODE_ENTRY(FIFO_RELAXED)
		}
		std::cout << White;
#undef VK_PRESENT_MODE_ENTRY
	}
#endif

	//!< セッティングを変更してスワップチェインを再作成できるように、既存のを開放する (開放するために一旦 OldSwapchainに覚えておく)
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
	if (VK_NULL_HANDLE != OldSwapchain) {
		for (auto i : SwapchainImageViews) {
			vkDestroyImageView(Device, i, nullptr);
		}
		vkDestroySwapchainKHR(Device, OldSwapchain, nullptr);
	}

	assert(!PresentSemaphores.empty() && "PresentSemaphore is empty");
	VERIFY_SUCCEEDED(vkAcquireNextImageKHR(Device, Swapchain, UINT64_MAX, PresentSemaphores[0], nullptr/*Fence*/, &SwapchainImageIndex));
#ifdef _DEBUG
	std::cout << "\t" << "SwapchainImageIndex = " << SwapchainImageIndex << std::endl;
#endif

#ifdef _DEBUG
	std::cout << "CreateSwapchain" << COUT_OK << std::endl << std::endl;
#endif
}
void VK::CreateSwapchainImageView(VkCommandBuffer CommandBuffer, const VkFormat ColorFormat)
{
	//!< スワップチェインイメージの列挙
	uint32_t SwapchainImageCount;
	VERIFY_SUCCEEDED(vkGetSwapchainImagesKHR(Device, Swapchain, &SwapchainImageCount, nullptr));
	assert(SwapchainImageCount && "Swapchain image count == 0");
	SwapchainImages.resize(SwapchainImageCount);
	VERIFY_SUCCEEDED(vkGetSwapchainImagesKHR(Device, Swapchain, &SwapchainImageCount, SwapchainImages.data()));
#ifdef _DEBUG
	for (const auto& i : SwapchainImages) {
		std::cout << "\t" << "SwapchainImage" << std::endl;
	}
#endif

	const VkCommandBufferBeginInfo CommandBufferBeginInfo = {
		VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		nullptr,
		0,
		nullptr
	};
	VERIFY_SUCCEEDED(vkBeginCommandBuffer(CommandBuffer, &CommandBufferBeginInfo)); {
		for (auto& i : SwapchainImages) {
			ImageBarrier(CommandBuffer, i, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
		}
	} VERIFY_SUCCEEDED(vkEndCommandBuffer(CommandBuffer));
	const VkSubmitInfo SubmitInfo = {
		VK_STRUCTURE_TYPE_SUBMIT_INFO,
		nullptr,
		0, nullptr,
		nullptr,
		1, &CommandBuffer,
		0, nullptr
	};
	VERIFY_SUCCEEDED(vkQueueSubmit(Queue, 1, &SubmitInfo, VK_NULL_HANDLE));
	VERIFY_SUCCEEDED(vkQueueWaitIdle(Queue));

	for(auto i : SwapchainImages) {
		SetImageLayout(CommandBuffer, i, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
		const VkComponentMapping ComponentMapping = {
			VK_COMPONENT_SWIZZLE_R,
			VK_COMPONENT_SWIZZLE_G,
			VK_COMPONENT_SWIZZLE_B,
			VK_COMPONENT_SWIZZLE_A
		};
		const VkImageSubresourceRange ImageSubresourceRange = {
			VK_IMAGE_ASPECT_COLOR_BIT,
			0, 1,
			0, 1
		};
		const VkImageViewCreateInfo ImageViewCreateInfo = {
			VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
			nullptr,
			0,
			i,
			VK_IMAGE_VIEW_TYPE_2D,
			ColorFormat,
			ComponentMapping,
			ImageSubresourceRange,
		};

		VkImageView ImageView;
		VERIFY_SUCCEEDED(vkCreateImageView(Device, &ImageViewCreateInfo, nullptr, &ImageView));
		SwapchainImageViews.push_back(ImageView);

#ifdef _DEBUG
		std::cout << "\t" << "SwapchainImageView" << std::endl;
#endif
	}
}

void VK::CreateDepthStencilImage(const VkFormat DepthFormat)
{
	const VkExtent3D Extent3D = {
		ImageExtent.width, ImageExtent.height, 1
	};
	const VkImageCreateInfo ImageCreateInfo = {
		VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
		nullptr,
		0,
		VK_IMAGE_TYPE_2D,
		DepthFormat,
		Extent3D,
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
}
void VK::CreateDepthStencilDeviceMemory(const VkPhysicalDeviceMemoryProperties& PhysicalDeviceMemoryProperties)
{
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

#ifdef _DEBUG
	std::cout << "\t" << "DepthStencilDeviceMemory" << std::endl;
#endif
}
void VK::CreateDepthStencilView(VkCommandBuffer CommandBuffer, const VkFormat DepthFormat)
{
	SetImageLayout(	CommandBuffer, DepthStencilImage, VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
	const VkComponentMapping ComponentMapping = {
		VK_COMPONENT_SWIZZLE_IDENTITY, 
		VK_COMPONENT_SWIZZLE_IDENTITY,
		VK_COMPONENT_SWIZZLE_IDENTITY,
		VK_COMPONENT_SWIZZLE_IDENTITY
	};
	const VkImageSubresourceRange ImageSubresourceRange = {
		VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT,
		0, 1,
		0, 1
	};
	const VkImageViewCreateInfo ImageViewCreateInfo = {
		VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
		nullptr,
		0,
		DepthStencilImage,
		VK_IMAGE_VIEW_TYPE_2D,
		DepthFormat,
		ComponentMapping,
		ImageSubresourceRange
	};
	VERIFY_SUCCEEDED(vkCreateImageView(Device, &ImageViewCreateInfo, nullptr, &DepthStencilImageView));

#ifdef _DEBUG
	std::cout << "\t" << "DepthStencilImageView" << std::endl;
#endif
}

void VK::CreateShader()
{
#ifdef _DEBUG
	std::cout << "CreateShader" << COUT_OK << std::endl << std::endl;
#endif
}

/**
@brief シェーダとのバインディングのレイアウト
@note DescriptorSetLayout は「型」のようなもの
*/
void VK::CreateDescriptorSetLayout()
{
	const std::vector<VkDescriptorSetLayoutBinding> DescriptorSetLayoutBindings = {
#if 0
		//!< 配列サイズ 1 の UBO、VS から可視
		{ 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr },
		//!< 配列サイズ 1 の SAMPLER、全てから可視
		{ 1, VK_DESCRIPTOR_TYPE_SAMPLER, 1, VK_SHADER_STAGE_ALL_GRAPHICS, nullptr },
		//!< 配列サイズ 10 の IMAGE、FS から可視
		{ 2, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 10, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr },
#endif
	};
	const VkDescriptorSetLayoutCreateInfo DescriptorSetLayoutCreateInfo = {
		VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
		nullptr,
		0,
		static_cast<uint32_t>(DescriptorSetLayoutBindings.size()), DescriptorSetLayoutBindings.data()
	};
	VkDescriptorSetLayout DescriptorSetLayout;
	VERIFY_SUCCEEDED(vkCreateDescriptorSetLayout(Device, &DescriptorSetLayoutCreateInfo, nullptr, &DescriptorSetLayout));
	DescriptorSetLayouts.push_back(DescriptorSetLayout);

#ifdef _DEBUG
	std::cout << "CreateDescriptorSetLayout" << COUT_OK << std::endl << std::endl;
#endif
}
void VK::CreateDescritporPool()
{
	const std::vector<VkDescriptorPoolSize> DescriptorPoolSizes = {
#if 0
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1 },
#endif
	};
	if (!DescriptorPoolSizes.empty()) {
		const VkDescriptorPoolCreateInfo DescriptorPoolCreateInfo = {
			VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
			nullptr,
			0,
			1, //!< maxSets ... プールから確保される最大のデスクリプタ数
			static_cast<uint32_t>(DescriptorPoolSizes.size()), DescriptorPoolSizes.data()
		};
		VERIFY_SUCCEEDED(vkCreateDescriptorPool(Device, &DescriptorPoolCreateInfo, nullptr, &DescriptorPool));

#ifdef _DEBUG
		std::cout << "CreateDescriptorPool" << COUT_OK << std::endl << std::endl;
#endif
	}
}
/**
@brief シェーダとのバインディングのレイアウト
@note DescriptorSet は　「DescriptorSetLayt 型」のインスタンスのようなもの
@note 更新は vkUpdateDescriptorSets() で行う
*/
void VK::CreateDescriptorSet(VkDescriptorPool DescritorPool)
{
	if (VK_NULL_HANDLE != DescritorPool) {
		const VkDescriptorSetAllocateInfo DescriptorSetAllocateInfo = {
			VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
			nullptr,
			DescriptorPool,
			static_cast<uint32_t>(DescriptorSetLayouts.size()), DescriptorSetLayouts.data()
		};
		VkDescriptorSet DescriptorSet;
		VERIFY_SUCCEEDED(vkAllocateDescriptorSets(Device, &DescriptorSetAllocateInfo, &DescriptorSet));
		DescriptorSets.push_back(DescriptorSet);

#ifdef _DEBUG
		std::cout << "CreateDescriptorSet" << COUT_OK << std::endl << std::endl;
#endif
	}
}
/**
@brief シェーダとのバインディングのレイアウト
@note PipelineLayout は引数に「DescriptorSetLayt 型のインスタンス」を取る「関数」のようなもの
*/
void VK::CreatePipelineLayout()
{
	//!< Push constants represent a high speed path to modify constant data in pipelines that is expected to outperform memory-backed resource updates.
	const std::vector<VkPushConstantRange> PushConstantRanges = {};
	const VkPipelineLayoutCreateInfo PipelineLayoutCreateInfo = {
		VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
		nullptr,
		0,
		static_cast<uint32_t>(DescriptorSetLayouts.size()), DescriptorSetLayouts.data(),
		static_cast<uint32_t>(PushConstantRanges.size()), PushConstantRanges.data()
	};
	VERIFY_SUCCEEDED(vkCreatePipelineLayout(Device, &PipelineLayoutCreateInfo, nullptr, &PipelineLayout));

#ifdef _DEBUG
	std::cout << "CreatePipelineLayout" << COUT_OK << std::endl << std::endl;
#endif
}

void VK::CreateVertexInput()
{
#ifdef _DEBUG
	std::cout << "CreateVertexInput" << COUT_OK << std::endl << std::endl;
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

#ifdef _DEBUG
	std::cout << "CreateViewport" << COUT_OK << std::endl << std::endl;
#endif
}

void VK::CreateGraphicsPipeline()
{
#ifdef _DEBUG
	std::cout << "CreateGraphicsPipeline" << COUT_OK << std::endl << std::endl;
#endif
}

void VK::CreateComputePipeline()
{
#ifdef _DEBUG
	std::cout << "CreateComputePipeline" << COUT_OK << std::endl << std::endl;
#endif
}

void VK::CreateRenderPass(const VkFormat ColorFormat, const VkFormat DepthFormat)
{
#ifdef _DEBUG
	std::cout << "CreateRenderPass" << COUT_OK << std::endl << std::endl;
#endif
}

void VK::CreateFramebuffer()
{
#ifdef _DEBUG
	std::cout << "CreateFramebuffer" << COUT_OK << std::endl << std::endl;
#endif
}

void VK::CreateDeviceLocalBuffer(const VkCommandPool CommandPool, const VkPhysicalDeviceMemoryProperties& PhysicalDeviceMemoryProperties, const VkBufferUsageFlagBits Usage, VkBuffer* Buffer, VkDeviceMemory* DeviceMemory, const size_t Size, const void* Source)
{
	VkBuffer StagingBuffer;
	VkDeviceMemory StagingDeviceMemory;
	{
		//!< ステージング用のバッファとメモリを作成
		const VkBufferCreateInfo StagingBufferCreateInfo = {
			VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
			nullptr,
			0,
			Size,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT, //!< TRANSFER_SRC にすること
			VK_SHARING_MODE_EXCLUSIVE,
			0, nullptr
		};
		VERIFY_SUCCEEDED(vkCreateBuffer(Device, &StagingBufferCreateInfo, nullptr, &StagingBuffer));
		VkMemoryRequirements StagingMemoryRequirements;
		vkGetBufferMemoryRequirements(Device, StagingBuffer, &StagingMemoryRequirements);
		const VkMemoryAllocateInfo StagingMemoryAllocateInfo = {
			VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
			nullptr,
			StagingMemoryRequirements.size,
			GetMemoryType(PhysicalDeviceMemoryProperties, StagingMemoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) //!< HOST_VISIBLE にすること
		};
		VERIFY_SUCCEEDED(vkAllocateMemory(Device, &StagingMemoryAllocateInfo, nullptr, &StagingDeviceMemory));
		void *Data;
		//!< #TODO サンプルだと vkMapMemory() の size に MemoryAllocateInfo.allocationSize を渡しているケースと、直サイズを渡しているケースがある、どっちが正しい？
		//VERIFY_SUCCEEDED(vkMapMemory(Device, StagingDeviceMemory, 0, StagingMemoryAllocateInfo.allocationSize, 0, &Data)); {
		VERIFY_SUCCEEDED(vkMapMemory(Device, StagingDeviceMemory, 0, Size, 0, &Data)); {
				memcpy(Data, Source, Size);
		} vkUnmapMemory(Device, StagingDeviceMemory);
		VERIFY_SUCCEEDED(vkBindBufferMemory(Device, StagingBuffer, StagingDeviceMemory, 0));

		//!< 目的のバッファとメモリを作成
		const VkBufferCreateInfo BufferCreateInfo = {
			VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
			nullptr,
			0,
			Size,
			static_cast<VkBufferUsageFlags>(Usage) | VK_BUFFER_USAGE_TRANSFER_DST_BIT, //!< TRANSFER_DST にすること
			VK_SHARING_MODE_EXCLUSIVE,
			0, nullptr
		};
		VERIFY_SUCCEEDED(vkCreateBuffer(Device, &BufferCreateInfo, nullptr, Buffer));
		VkMemoryRequirements MemoryRequirements;
		vkGetBufferMemoryRequirements(Device, *Buffer, &MemoryRequirements);
		const VkMemoryAllocateInfo MemoryAllocateInfo = {
			VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
			nullptr,
			MemoryRequirements.size,
			GetMemoryType(PhysicalDeviceMemoryProperties, MemoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) //!< DEVICE_LOCAL にすること
		};
		VERIFY_SUCCEEDED(vkAllocateMemory(Device, &MemoryAllocateInfo, nullptr, DeviceMemory));
		VERIFY_SUCCEEDED(vkBindBufferMemory(Device, *Buffer, *DeviceMemory, 0));

		//!< コピーコマンドを発行
		const VkCommandBufferAllocateInfo CommandBufferAllocateInfo = {
			VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
			nullptr,
			CommandPool,
			VK_COMMAND_BUFFER_LEVEL_PRIMARY,
			1
		};
		VkCommandBuffer CommandBuffer;
		VERIFY_SUCCEEDED(vkAllocateCommandBuffers(Device, &CommandBufferAllocateInfo, &CommandBuffer)); {
			const VkCommandBufferBeginInfo CommandBufferBeginInfo = {
				VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
				nullptr,
				0,
				nullptr
			};
			VERIFY_SUCCEEDED(vkBeginCommandBuffer(CommandBuffer, &CommandBufferBeginInfo)); {
				const VkBufferCopy BufferCopy = {
					0,
					0,
					Size
				};
				vkCmdCopyBuffer(CommandBuffer, StagingBuffer, *Buffer, 1, &BufferCopy);
			} VERIFY_SUCCEEDED(vkEndCommandBuffer(CommandBuffer));

			const VkSubmitInfo SubmitInfo = {
				VK_STRUCTURE_TYPE_SUBMIT_INFO,
				nullptr,
				0, nullptr,
				nullptr,
				1, &CommandBuffer,
				0, nullptr
			};
			//!< 一般的なキューを再利用するよりも転送専用のキューを作った方が良いらしい
			VERIFY_SUCCEEDED(vkQueueSubmit(Queue, 1, &SubmitInfo, VK_NULL_HANDLE));
			VERIFY_SUCCEEDED(vkQueueWaitIdle(Queue));
		} vkFreeCommandBuffers(Device, CommandPool, 1, &CommandBuffer);
	}
	vkDestroyBuffer(Device, StagingBuffer, nullptr);
	vkFreeMemory(Device, StagingDeviceMemory, nullptr);
}
void VK::CreateHostVisibleBuffer(const VkPhysicalDeviceMemoryProperties& PhysicalDeviceMemoryProperties, const VkBufferUsageFlagBits Usage, VkBuffer* Buffer, VkDeviceMemory* DeviceMemory, const size_t Size, const void* Source)
{
	const VkBufferCreateInfo BufferCreateInfo = {
		VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
		nullptr,
		0,
		Size,
		static_cast<VkBufferUsageFlags>(Usage),
		VK_SHARING_MODE_EXCLUSIVE,
		0, nullptr
	};
	VERIFY_SUCCEEDED(vkCreateBuffer(Device, &BufferCreateInfo, nullptr, Buffer));

	VkMemoryRequirements MemoryRequirements;
	vkGetBufferMemoryRequirements(Device, UniformBuffer, &MemoryRequirements);
	const VkMemoryAllocateInfo MemoryAllocateInfo = {
		VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
		nullptr,
		MemoryRequirements.size,
		GetMemoryType(PhysicalDeviceMemoryProperties, MemoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) //!< HOST_VISIBLE にすること
	};
	VERIFY_SUCCEEDED(vkAllocateMemory(Device, &MemoryAllocateInfo, nullptr, DeviceMemory));
	void *Data;
	//!< #TODO サンプルだと vkMapMemory() の size に MemoryAllocateInfo.allocationSize を渡しているケースと、直サイズを渡しているケースがある、どっちが正しい？
	//VERIFY_SUCCEEDED(vkMapMemory(Device, *DeviceMemory, 0, MemoryAllocateInfo.allocationSize, 0, &Data)); {
	VERIFY_SUCCEEDED(vkMapMemory(Device, *DeviceMemory, 0, Size, 0, &Data)); {
		memcpy(Data, Source, Size);
	} vkUnmapMemory(Device, *DeviceMemory);
	VERIFY_SUCCEEDED(vkBindBufferMemory(Device, *Buffer, *DeviceMemory, 0));
}
void VK::CreateVertexBuffer(const VkCommandPool CommandPool, const VkPhysicalDeviceMemoryProperties& PhysicalDeviceMemoryProperties)
{
#ifdef _DEBUG
	std::cout << "CreateVertexBuffer" << COUT_OK << std::endl << std::endl;
#endif
}
void VK::CreateIndexBuffer(const VkCommandPool CommandPool, const VkPhysicalDeviceMemoryProperties& PhysicalDeviceMemoryProperties)
{
#ifdef _DEBUG
	std::cout << "CreateIndexBuffer" << COUT_OK << std::endl << std::endl;
#endif
}
void VK::CreateUniformBuffer(const VkPhysicalDeviceMemoryProperties& PhysicalDeviceMemoryProperties)
{
	glm::vec4 Color(1.0f, 0.0f, 0.0f, 1.0f);
	const auto Size = sizeof(Color);

	CreateHostVisibleBuffer(PhysicalDeviceMemoryProperties, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, &UniformBuffer, &UniformDeviceMemory, Size, &Color);

	UniformDescriptorBufferInfo = {
		UniformBuffer,
		0,
		Size
	};

	VkDescriptorBufferInfo* DescriptorBufferInfo = nullptr;
	WriteDescriptorSets = {
		{
			VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
			nullptr,
			DescriptorSets[0], 0, //!< デスクリプタセットとバインディングポイント
			0,
			1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			nullptr,
			DescriptorBufferInfo,
			nullptr
		},
	};
	std::vector<VkCopyDescriptorSet> CopyDescriptorSets = {};
	vkUpdateDescriptorSets(Device, 
		static_cast<uint32_t>(WriteDescriptorSets.size()), WriteDescriptorSets.data(), 
		static_cast<uint32_t>(CopyDescriptorSets.size()), CopyDescriptorSets.data());

#ifdef _DEBUG
	std::cout << "CreateUniformBuffer" << COUT_OK << std::endl << std::endl;
#endif
}

void VK::PopulateCommandBuffer(const VkCommandBuffer CommandBuffer)
{
	Clear(CommandBuffer);
}

void VK::ImageBarrier(VkCommandBuffer CommandBuffer, VkImage Image, VkImageLayout Old, VkImageLayout New)
{
	const VkImageSubresourceRange ImageSubresourceRange = {
		VK_IMAGE_ASPECT_COLOR_BIT,
		0, 1,
		0, 1,
	};
	const VkImageMemoryBarrier ImageMemoryBarrier = {
		VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
		nullptr,
		0,
		VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
		Old,
		New,
		VK_QUEUE_FAMILY_IGNORED,
		VK_QUEUE_FAMILY_IGNORED,
		Image,
		ImageSubresourceRange
	};
	vkCmdPipelineBarrier(CommandBuffer,
		VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
		VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
		0,
		0, nullptr,
		0, nullptr,
		1, &ImageMemoryBarrier);
}

void VK::Draw()
{
	if (CommandPools.empty() || CommandBuffers.empty()) { return; }

	auto CommandPool = CommandPools[0];
	auto CommandBuffer = CommandBuffers[0];

	VERIFY_SUCCEEDED(vkResetCommandPool(Device, CommandPool, VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT));

	const VkCommandBufferBeginInfo BeginInfo = {
		VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		nullptr,
		0,
		nullptr
	};
	VERIFY_SUCCEEDED(vkBeginCommandBuffer(CommandBuffer, &BeginInfo));
	{
		vkCmdSetViewport(CommandBuffer, 0, static_cast<uint32_t>(Viewports.size()), Viewports.data());
		vkCmdSetScissor(CommandBuffer, 0, static_cast<uint32_t>(ScissorRects.size()), ScissorRects.data());

		auto Image = SwapchainImages[SwapchainImageIndex];
		ImageBarrier(CommandBuffer, Image, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
		{
			PopulateCommandBuffer(CommandBuffer);
		}
		ImageBarrier(CommandBuffer, Image, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
	}
	VERIFY_SUCCEEDED(vkEndCommandBuffer(CommandBuffer));

	ExecuteCommandBuffer(CommandBuffer);

	Present();

	WaitForFence();
}
void VK::ExecuteCommandBuffer(const VkCommandBuffer CommandBuffer)
{
	VERIFY_SUCCEEDED(vkDeviceWaitIdle(Device));

	const VkPipelineStageFlags PipelineStageFlags = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	const VkSubmitInfo SubmitInfo = {
		VK_STRUCTURE_TYPE_SUBMIT_INFO,
		nullptr,
		static_cast<uint32_t>(PresentSemaphores.size()), PresentSemaphores.data(),
		&PipelineStageFlags,
		1,  &CommandBuffer,
		static_cast<uint32_t>(RenderSemaphores.size()), RenderSemaphores.data()
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
	//VERIFY_SUCCEEDED(vkAcquireNextImageKHR(Device, Swapchain, UINT64_MAX, PresentSemaphores[0], nullptr, &SwapchainImageIndex));

	const VkPresentInfoKHR PresentInfo = {
		VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
		nullptr,
		static_cast<uint32_t>(RenderSemaphores.size()), RenderSemaphores.data(),
		1, &Swapchain,
		&SwapchainImageIndex,
		nullptr
	};
	VERIFY_SUCCEEDED(vkQueuePresentKHR(Queue, &PresentInfo));

	assert(!PresentSemaphores.empty() && "PresentSmaphore is empty");

	VERIFY_SUCCEEDED(vkAcquireNextImageKHR(Device, Swapchain, UINT64_MAX, PresentSemaphores[0], nullptr, &SwapchainImageIndex));

#ifdef _DEBUG
	//std::cout << "SwapchainImageIndex = " << SwapchainImageIndex << std::endl;
	//std::cout << SwapchainImageIndex;
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

#ifdef _DEBUG
	//std::cout << "Fence" << std::endl;
#endif
}
