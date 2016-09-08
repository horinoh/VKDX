#include "stdafx.h"

#include <fstream>

#include "VK.h"

#pragma comment(lib, "vulkan-1.lib")

void VK::OnCreate(HWND hWnd, HINSTANCE hInstance)
{
#ifdef _DEBUG
	PerformanceCounter PC("OnCreate : ");
#endif

	Super::OnCreate(hWnd, hInstance);

	//!< デバイス、キュー
	CreateInstance();
	CreateSurface(hWnd, hInstance);
	GetPhysicalDevice();
	GetQueueFamily();
	CreateDevice();
	
	//!< コマンドプール、バッファ
	CreateCommandPool(GraphicsQueueFamilyIndex);
	auto CommandPool = CommandPools[0];
	AllocateCommandBuffer(CommandPool);
	auto CommandBuffer = CommandBuffers[0];

	CreateFence();
	CreateSemaphore();

	//!< スワップチェイン
	CreateSwapchain(CommandBuffer);
	//!< デプスステンシル
	CreateDepthStencil(CommandBuffer);

	//!< デスクリプタセット
	CreateDescriptorSetLayout();
	CreateDescritporPool();
	CreateDescriptorSet(DescriptorPool);

	//!< バーテックスインプット
	CreateVertexInput();

	//!< パイプライン
	CreatePipelineLayout();
	CreateRenderPass();
	CreatePipeline();
	CreateFramebuffer();

	//!< バーテックスバッファ、インデックスバッファ
	CreateVertexBuffer(CommandBuffer);
	CreateIndexBuffer(CommandBuffer);

	//!< ユニフォームバッファ
	//CreateUniformBuffer();
}
void VK::OnSize(HWND hWnd, HINSTANCE hInstance)
{
#ifdef _DEBUG
	PerformanceCounter PC("OnSize : ");
#endif

	Super::OnSize(hWnd, hInstance);

	CreateViewport(static_cast<float>(SurfaceExtent2D.width), static_cast<float>(SurfaceExtent2D.height));
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

	if (VK_NULL_HANDLE != Device) {
		VERIFY_SUCCEEDED(vkDeviceWaitIdle(Device));
	}

	if (VK_NULL_HANDLE != UniformDeviceMemory) {
		vkFreeMemory(Device, UniformDeviceMemory, nullptr);
	}
	if (VK_NULL_HANDLE != UniformBuffer) {
		vkDestroyBuffer(Device, UniformBuffer, nullptr);
	}

	if (VK_NULL_HANDLE != IndexDeviceMemory) {
		vkFreeMemory(Device, IndexDeviceMemory, nullptr);
	}
	if (VK_NULL_HANDLE != IndexBuffer) {
		vkDestroyBuffer(Device, IndexBuffer, nullptr);
	}
	if (VK_NULL_HANDLE != VertexDeviceMemory) {
		vkFreeMemory(Device, VertexDeviceMemory, nullptr);
	}
	if (VK_NULL_HANDLE != VertexBuffer) {
		vkDestroyBuffer(Device, VertexBuffer, nullptr);
	}

	for (auto i : Framebuffers) {
		vkDestroyFramebuffer(Device, i, nullptr);
	}
	if (VK_NULL_HANDLE != RenderPass) {
		vkDestroyRenderPass(Device, RenderPass, nullptr);
	}
	if (VK_NULL_HANDLE != Pipeline) {
		vkDestroyPipeline(Device, Pipeline, nullptr);
	}
	if (VK_NULL_HANDLE != PipelineCache) {
		vkDestroyPipelineCache(Device, PipelineCache, nullptr);
	}
	if (VK_NULL_HANDLE != PipelineLayout) {
		vkDestroyPipelineLayout(Device, PipelineLayout, nullptr);
	}

	if(!DescriptorSets.empty()) {
		vkFreeDescriptorSets(Device, DescriptorPool, static_cast<uint32_t>(DescriptorSets.size()), DescriptorSets.data());
	}
	if (VK_NULL_HANDLE != DescriptorPool) {
		vkDestroyDescriptorPool(Device, DescriptorPool, nullptr);
	}
	for (auto i : DescriptorSetLayouts) {
		vkDestroyDescriptorSetLayout(Device, i, nullptr);
	}

	if (VK_NULL_HANDLE != DepthStencilImageView) {
		vkDestroyImageView(Device, DepthStencilImageView, nullptr);
	}
	if (VK_NULL_HANDLE != DepthStencilDeviceMemory) {
		vkFreeMemory(Device, DepthStencilDeviceMemory, nullptr);
	}
	if (VK_NULL_HANDLE != DepthStencilImage) {
		vkDestroyImage(Device, DepthStencilImage, nullptr);
	}

	for (auto i : SwapchainImageViews) {
		vkDestroyImageView(Device, i, nullptr);
	}
	//!< SwapchainImages は vkGetSwapchainImagesKHR() で取得したもの、破棄しない
	if (VK_NULL_HANDLE != Swapchain) {
		vkDestroySwapchainKHR(Device, Swapchain, nullptr);
	}
	if (VK_NULL_HANDLE != RenderSemaphore) {
		vkDestroySemaphore(Device, RenderSemaphore, nullptr);
	}
	if (VK_NULL_HANDLE != PresentSemaphore) {
		vkDestroySemaphore(Device, PresentSemaphore, nullptr);
	}
	if (VK_NULL_HANDLE != Fence) {
		vkDestroyFence(Device, Fence, nullptr);
	}

	//!< ここでは Pool と Buffer が 1:1 とする
	for (auto i = 0; i < CommandBuffers.size(); ++i) {
		vkFreeCommandBuffers(Device, CommandPools[i], 1, &CommandBuffers[i]);
		vkDestroyCommandPool(Device, CommandPools[i], nullptr);
	}

	//!< Queue は vkGetDeviceQueue() で取得したもの、破棄しない

	if (VK_NULL_HANDLE != Device) {
		vkDestroyDevice(Device, nullptr);
	}
	
	//!< PhysicalDevice は vkEnumeratePhysicalDevices() で取得したもの、破棄しない

	if (VK_NULL_HANDLE != Surface) {
		vkDestroySurfaceKHR(Instance, Surface, nullptr);
	}

#ifdef _DEBUG
	DebugReport::DestroyDebugReportCallback(Instance, DebugReportCallback);
#endif

	if (VK_NULL_HANDLE != Instance) {
		//vkDestroyInstance(Instance, &AllocationCallbacks);
		vkDestroyInstance(Instance, nullptr);
	}
}

std::string VK::GetVkResultString(const VkResult Result)
{
#define VK_RESULT_ENTRY(vr) case VK_##vr: return #vr;
	switch (Result)
	{
	default: DEBUG_BREAK(); return "";
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
	default: DEBUG_BREAK(); return "";
#include "VKFormat.h"
	}
#undef VK_FORMAT_ENTRY
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
uint32_t VK::GetMemoryType(const VkPhysicalDeviceMemoryProperties& PhysicalDeviceMemoryProperties, uint32_t TypeBits, VkFlags Properties)
{
	for (auto i = 0; i < 32; ++i) {
		if (TypeBits & 1) {
			if ((PhysicalDeviceMemoryProperties.memoryTypes[i].propertyFlags & Properties) == Properties) {
				return i;
			}
		}
		TypeBits >>= 1;
	}
	DEBUG_BREAK(); //!< MemoryType not found
	return 0;
}

//!< 遷移する前に OldImageLayout が完了していなければならないアクション
VkAccessFlags VK::GetSrcAccessMask(VkImageLayout OldImageLayout, VkImageLayout NewImageLayout)
{
	VkAccessFlags AccessMask = 0;

	switch (OldImageLayout)
	{
	case VK_IMAGE_LAYOUT_UNDEFINED:
		AccessMask = 0;
		break;
	case VK_IMAGE_LAYOUT_PREINITIALIZED:
		//!< ホストによる書き込みが完了していなければならない
		AccessMask = VK_ACCESS_HOST_WRITE_BIT; 
		break;
	case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
		//!< カラーバッファ書き込みが完了していなければならない
		AccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		break;
	case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
		//!< デプスステンシルバッファ書き込みが完了していなければならない
		AccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		break;
	case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
		//!< イメージ読み込みが完了していなければならない
		AccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		break;
	case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
		//!< イメージへの書き込みが完了していなければならない
		AccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		break;
	case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
		//!< シェーダによるイメージ読み込みが完了していなければならない
		AccessMask = VK_ACCESS_SHADER_READ_BIT;
		break;
	}
	switch (NewImageLayout)
	{
	case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
		AccessMask |= VK_ACCESS_TRANSFER_READ_BIT;
		break;
	case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
		AccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		break;
	case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
		if (0 == AccessMask){
			AccessMask = VK_ACCESS_HOST_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT;
		}
		break;
	}
	return AccessMask;
}
VkAccessFlags VK::GetDstAccessMask(VkImageLayout OldImageLayout, VkImageLayout NewImageLayout)
{
	VkAccessFlags AccessMask = 0;

	switch (NewImageLayout)
	{
	case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
		AccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		break;
	case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
		AccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		break;
	case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
		AccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		break;
	case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
		AccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		break;
	case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
		AccessMask = VK_ACCESS_SHADER_READ_BIT;
		break;
	}
	return AccessMask;
}
void VK::SetImageLayout(VkCommandBuffer CommandBuffer, VkImage Image, VkImageLayout OldImageLayout, VkImageLayout NewImageLayout, VkImageSubresourceRange ImageSubresourceRange) const
{
	const auto SrcAccessMask = GetSrcAccessMask(OldImageLayout, NewImageLayout);
	const auto DstAccessMask = GetDstAccessMask(OldImageLayout, NewImageLayout);

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
	const VkCommandBufferBeginInfo CommandBufferBeginInfo = {
		VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		nullptr,
		VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
		nullptr
	};
	vkCmdPipelineBarrier(CommandBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0, 0, nullptr, 0, nullptr, 1, &ImageMemoryBarrier);
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
			std::cout << "\t" << "\"" << i.layerName << "\"" << std::endl;
			//InstanceLayerNames.push_back({ i.layerName,{} });
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
			std::cout << "\t" << "\t" << "\"" << i.extensionName << "\"" << std::endl;
			//InstanceLayerNames.back().second.push_back({ i.extensionName });
#endif
		}
	}
}

void VK::CreateInstance()
{
	EnumerateInstanceLayer();

	const VkApplicationInfo ApplicationInfo = {
		VK_STRUCTURE_TYPE_APPLICATION_INFO,
		nullptr,
		"VKDX Application Name", 0,
		"VKDX Engine Name", 0,
		VK_API_VERSION_1_0
	};
	const std::vector<const char*> EnabledLayerNames = {
#ifdef _DEBUG
		//!< ↓標準的なバリデーションレイヤセットを最適な順序でロードする指定
		"VK_LAYER_LUNARG_standard_validation", 
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

	//VERIFY_SUCCEEDED(vkCreateInstance(&InstanceCreateInfo, &AllocationCallbacks, &Instance));
	VERIFY_SUCCEEDED(vkCreateInstance(&InstanceCreateInfo, nullptr, &Instance));

	CreateDebugReportCallback();

#ifdef DEBUG_STDOUT
	std::cout << "CreateInstace" << COUT_OK << std::endl << std::endl;
#endif
}
void VK::CreateDebugReportCallback()
{
#ifdef _DEBUG
	DebugReport::GetInstanceProcAddr(Instance);

	auto Callback = [](VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objectType, uint64_t object, size_t location, int32_t messageCode, const char* pLayerPrefix, const char* pMessage, void* pUserData) -> VkBool32
	{
		using namespace std;
		if (VK_DEBUG_REPORT_ERROR_BIT_EXT & flags) {
			cout << Red << pMessage << White << endl;
		}
		else if (VK_DEBUG_REPORT_WARNING_BIT_EXT & flags) {
			cout << Yellow << pMessage << White << endl;
		}
		else if (VK_DEBUG_REPORT_INFORMATION_BIT_EXT & flags) {
			//cout << Green << pMessage << White << endl;
		}
		else if (VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT & flags) {
			cout << Yellow << pMessage << White << endl;
		}
		else if (VK_DEBUG_REPORT_DEBUG_BIT_EXT & flags) {
			//cout << Green << pMessage << White << endl;
		}
		return false;
	};
	const auto Flags = VK_DEBUG_REPORT_INFORMATION_BIT_EXT 
		| VK_DEBUG_REPORT_WARNING_BIT_EXT 
		| VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT 
		| VK_DEBUG_REPORT_ERROR_BIT_EXT 
		| VK_DEBUG_REPORT_DEBUG_BIT_EXT 
		| VK_DEBUG_REPORT_FLAG_BITS_MAX_ENUM_EXT;
	DebugReport::CreateDebugReportCallback(Instance, Callback, Flags, &DebugReportCallback);
#endif
}
/**
@brief デバイス(GPU)とホスト(CPU)の同期
*/
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
	VERIFY_SUCCEEDED(vkCreateWin32SurfaceKHR(Instance, &SurfaceCreateInfo, nullptr, &Surface));
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
		std::cout << "HeapIndex = " << PhysicalDeviceMemoryProperties.memoryTypes[i].heapIndex << ", ";
		std::cout << "PropertyFlags = ";
#define MEMORY_PROPERTY_ENTRY(entry) if(VK_MEMORY_PROPERTY_##entry##_BIT & PhysicalDeviceMemoryProperties.memoryTypes[i].propertyFlags) { std::cout << #entry << " "; }
		MEMORY_PROPERTY_ENTRY(DEVICE_LOCAL);
		MEMORY_PROPERTY_ENTRY(HOST_VISIBLE);
		MEMORY_PROPERTY_ENTRY(HOST_COHERENT);
		MEMORY_PROPERTY_ENTRY(HOST_CACHED);
		MEMORY_PROPERTY_ENTRY(LAZILY_ALLOCATED);
#undef MEMORY_PROPERTY_ENTRY
		std::cout << std::endl;
	}
	std::cout << "\t" << "\t" << "\t" << "MemoryHeap" << std::endl;
	for (uint32_t i = 0; i < PhysicalDeviceMemoryProperties.memoryHeapCount; ++i) {
		std::cout << "\t" << "\t" << "\t" << "\t";
		std::cout << "Size = " << PhysicalDeviceMemoryProperties.memoryHeaps[i].size << ", ";
		std::cout << "Flags = ";
		std::cout << ((VK_MEMORY_HEAP_DEVICE_LOCAL_BIT & PhysicalDeviceMemoryProperties.memoryHeaps[i].flags) ? "DEVICE_LOCAL" : "") << std::endl;
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
	std::cout << Yellow << "\t" << "PhysicalDevices" << White << std::endl;
#define PHYSICAL_DEVICE_TYPE_ENTRY(entry) if(VK_PHYSICAL_DEVICE_TYPE_##entry == PhysicalDeviceProperties.deviceType) { std::cout << #entry; }
	for (const auto& i : PhysicalDevices) {
		//!< 物理デバイスのプロパティ
		VkPhysicalDeviceProperties PhysicalDeviceProperties;
		vkGetPhysicalDeviceProperties(i, &PhysicalDeviceProperties);
		std::cout << "\t" << "\t" << PhysicalDeviceProperties.deviceName << ", DeviceType = ";
		PHYSICAL_DEVICE_TYPE_ENTRY(OTHER);
		PHYSICAL_DEVICE_TYPE_ENTRY(INTEGRATED_GPU);
		PHYSICAL_DEVICE_TYPE_ENTRY(DISCRETE_GPU);
		PHYSICAL_DEVICE_TYPE_ENTRY(VIRTUAL_GPU);
		PHYSICAL_DEVICE_TYPE_ENTRY(CPU);
		std::cout << std::endl;
		PhysicalDeviceProperties.limits;
		
		//!< 物理デバイスのフィーチャー
		VkPhysicalDeviceFeatures PhysicalDeviceFeatures;
		vkGetPhysicalDeviceFeatures(i, &PhysicalDeviceFeatures);
	}
#undef PHYSICAL_DEVICE_TYPE_ENTRY
#endif

	//!< ここでは最初の物理デバイスを選択することにする #TODO
	PhysicalDevice = PhysicalDevices[0];

	//!< 選択した物理デバイスのメモリプロパティを取得
	vkGetPhysicalDeviceMemoryProperties(PhysicalDevice, &PhysicalDeviceMemoryProperties);
	EnumeratePhysicalDeviceMemoryProperties(PhysicalDeviceMemoryProperties);

#ifdef _DEBUG
	EnumerateDeviceLayer(PhysicalDevice);
#endif
}
void VK::EnumerateDeviceLayer(VkPhysicalDevice PhysicalDevice)
{
	uint32_t DeviceLayerPropertyCount = 0;
	VERIFY_SUCCEEDED(vkEnumerateDeviceLayerProperties(PhysicalDevice, &DeviceLayerPropertyCount, nullptr));
	if (DeviceLayerPropertyCount) {
		std::vector<VkLayerProperties> LayerProperties(DeviceLayerPropertyCount);
		VERIFY_SUCCEEDED(vkEnumerateDeviceLayerProperties(PhysicalDevice, &DeviceLayerPropertyCount, LayerProperties.data()));
		for (const auto& i : LayerProperties) {
#ifdef DEBUG_STDOUT
			std::cout << "\t" << "\"" << i.layerName << "\"" << std::endl;
			//DeviceLayerNames.push_back({ i.layerName, {} });
#endif
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
			if (!strcmp(VK_EXT_DEBUG_MARKER_EXTENSION_NAME, i.extensionName)) {
#ifdef DEBUG_STDOUT
				std::cout << Red;
#endif
			}
#ifdef DEBUG_STDOUT
			std::cout << "\t" << "\t" << "\"" << i.extensionName << "\"" << White << std::endl;
#endif
			//DeviceLayerNames.back().second.push_back({ i.extensionName });
		}
	}
}
void VK::GetQueueFamily()
{
	//!< キューのプロパティを列挙
	uint32_t QueueFamilyPropertyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(PhysicalDevice, &QueueFamilyPropertyCount, nullptr);
	assert(QueueFamilyPropertyCount && "QueueFamilyProperty not found");
	std::vector<VkQueueFamilyProperties> QueueProperties(QueueFamilyPropertyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(PhysicalDevice, &QueueFamilyPropertyCount, QueueProperties.data());

#ifdef DEBUG_STDOUT
	std::cout << Yellow << "\t" << "QueueProperties" << White << std::endl;
#define QUEUE_FLAG_ENTRY(entry) if(VK_QUEUE_##entry##_BIT & i.queueFlags) { std::cout << #entry << " | "; }
	for (const auto& i : QueueProperties) {
		std::cout << "\t" << "\t" << "QueueCount = " << i.queueCount << ", ";
		std::cout << "QueueFlags = ";
		QUEUE_FLAG_ENTRY(GRAPHICS);
		QUEUE_FLAG_ENTRY(COMPUTE);
		QUEUE_FLAG_ENTRY(TRANSFER);
		QUEUE_FLAG_ENTRY(SPARSE_BINDING);
		std::cout << std::endl;
	}
#undef QUEUE_FLAG_ENTRY
#endif

	for (uint32_t i = 0; i < QueueFamilyPropertyCount; ++i) {
		//!< グラフィック機能を持つもの
		if (VK_QUEUE_GRAPHICS_BIT & QueueProperties[i].queueFlags) {
			if (UINT32_MAX != GraphicsQueueFamilyIndex) {
				GraphicsQueueFamilyIndex = i;
			}
		}
		//else if (VK_QUEUE_TRANSFER_BIT & QueueProperties[i].queueFlags) {
		//	//!< #TODO
		//	TransferQueueFamilyIndex = i; //!< デバイスによっては転送専用キューを持つ、転送を多用する場合は専用キューを使用した方が良い
		//}
		//else if (VK_QUEUE_COMPUTE_BIT & QueueProperties[i].queueFlags) {
		//	//!< #TODO
		//	ComputeQueueFamilyIndex = i;
		//}

		//!< プレゼント機能を持つもの
		VkBool32 Supported = VK_FALSE;
		VERIFY_SUCCEEDED(vkGetPhysicalDeviceSurfaceSupportKHR(PhysicalDevice, i, Surface, &Supported));
		if (Supported) {
			if (UINT32_MAX != PresentQueueFamilyIndex) {
				PresentQueueFamilyIndex = i;
			}
		}
	}
	//!< グラフィックとプレゼントを同時にサポートするキューがあれば優先
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
	//!< グラフィックとプレゼントのキューインデックスが別の場合は追加で必要
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
#endif
	};
#ifdef _DEBUG
	std::vector<const char*> EnabledExtensions = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME,
	};
	//!< ↓デバッグマーカー拡張があるなら追加
	if (DebugMarker::HasDebugMarkerExtension(PhysicalDevice)) {
		EnabledExtensions.push_back(VK_EXT_DEBUG_MARKER_EXTENSION_NAME);
	}
#else
	const std::vector<const char*> EnabledExtensions = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME,
	};
#endif
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

	//!< キューの取得 (グラフィック、プレゼントキューは同じインデックスの場合もあるが別名として取得)
	vkGetDeviceQueue(Device, GraphicsQueueFamilyIndex, 0, &GraphicsQueue);
	vkGetDeviceQueue(Device, PresentQueueFamilyIndex, 0, &PresentQueue);

	CreateDebugMarker();

#ifdef DEBUG_STDOUT
	std::cout << "CreateDevice" << COUT_OK << std::endl << std::endl;
#endif
}
void VK::CreateDebugMarker()
{
#ifdef _DEBUG
	DebugMarker::GetDeviceProcAddr(Device);
#endif
}

void VK::CreateCommandPool(const uint32_t QueueFamilyIndex)
{
	const VkCommandPoolCreateInfo CommandPoolInfo = {
		VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
		nullptr,
		/**
		VK_COMMAND_POOL_CREATE_TRANSIENT_BIT ... 頻繁に更新される、ライフスパンが短い場合(メモリアロケーションのヒントとなる)
		VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT ... 指定した場合は vkResetCommandBuffer(), vkBeginCommandBuffer() によるリセットが可能、指定しない場合は vkResetCommandPool() のみ可能
		*/
		VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
		QueueFamilyIndex
	};

	VkCommandPool CommandPool;
	VERIFY_SUCCEEDED(vkCreateCommandPool(Device, &CommandPoolInfo, nullptr, &CommandPool));
	CommandPools.push_back(CommandPool);

#ifdef DEBUG_STDOUT
	std::cout << "CreateCommandPool" << COUT_OK << std::endl << std::endl;
#endif
}

void VK::AllocateCommandBuffer(const VkCommandPool CommandPool, const VkCommandBufferLevel CommandBufferLevel)
{
	VkCommandBuffer CommandBuffer;
	const VkCommandBufferAllocateInfo CommandBufferAllocateInfo = {
		VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		nullptr,
		CommandPool,
		CommandBufferLevel,
		1
	};
	VERIFY_SUCCEEDED(vkAllocateCommandBuffers(Device, &CommandBufferAllocateInfo, &CommandBuffer));
	CommandBuffers.push_back(CommandBuffer);

#ifdef DEBUG_STDOUT
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

	//!< プレゼント完了同期用
	VERIFY_SUCCEEDED(vkCreateSemaphore(Device, &SemaphoreCreateInfo, nullptr, &PresentSemaphore));

	//!< 描画完了同期用
	VERIFY_SUCCEEDED(vkCreateSemaphore(Device, &SemaphoreCreateInfo, nullptr, &RenderSemaphore));

#ifdef _DEBUG
	std::cout << "CreateSemaphore" << COUT_OK << std::endl << std::endl;
#endif
}

VkSurfaceFormatKHR VK::SelectSurfaceFormat()
{
	//!< サーフェスのフォーマットを取得
	uint32_t SurfaceFormatCount;
	VERIFY_SUCCEEDED(vkGetPhysicalDeviceSurfaceFormatsKHR(PhysicalDevice, Surface, &SurfaceFormatCount, nullptr));
	assert(SurfaceFormatCount);
	std::vector<VkSurfaceFormatKHR> SurfaceFormats(SurfaceFormatCount);
	VERIFY_SUCCEEDED(vkGetPhysicalDeviceSurfaceFormatsKHR(PhysicalDevice, Surface, &SurfaceFormatCount, SurfaceFormats.data()));
#ifdef DEBUG_STDOUT
	std::cout << "\t" << "\t" << Lightblue << "Format" << White << std::endl;
	auto SelectedFormat = VK_FORMAT_UNDEFINED;
	for (auto& i : SurfaceFormats) {
		if (VK_FORMAT_UNDEFINED != i.format) {
			if (VK_FORMAT_UNDEFINED == SelectedFormat) {
				SelectedFormat = i.format;
				std::cout << Yellow;
			}
		}
		std::cout << "\t" << "\t" << "\t" << GetFormatString(i.format) << std::endl;
		std::cout << White;
	}
	std::cout << std::endl;
#endif

	//!< ここでは、最初の非 VK_FORMAT_UNDEFINED を選択する
	for (auto& i : SurfaceFormats) {
		if (VK_FORMAT_UNDEFINED != i.format) {
			return i;
		}
	}
	return VkSurfaceFormatKHR({ VK_FORMAT_B8G8R8A8_UNORM , SurfaceFormats[0].colorSpace });
}
VkPresentModeKHR VK::SelectSurfacePresentMode()
{
	uint32_t PresentModeCount;
	VERIFY_SUCCEEDED(vkGetPhysicalDeviceSurfacePresentModesKHR(PhysicalDevice, Surface, &PresentModeCount, nullptr));
	assert(PresentModeCount);
	std::vector<VkPresentModeKHR> PresentModes(PresentModeCount);
	VERIFY_SUCCEEDED(vkGetPhysicalDeviceSurfacePresentModesKHR(PhysicalDevice, Surface, &PresentModeCount, PresentModes.data()));

	/**
	可能なら VK_PRESENT_MODE_MAILBOX_KHR を選択
	VK_PRESENT_MODE_IMMEDIATE_KHR
	VK_PRESENT_MODE_MAILBOX_KHR ... Vブランクで表示される、テアリングは起こらない、キューは1つで常に最新で上書きされる (ゲーム)
	VK_PRESENT_MODE_FIFO_KHR ... Vブランクで表示される、テアリングは起こらない (ムービー)
	VK_PRESENT_MODE_FIFO_RELAXED_KHR ... 1Vブランク以上経ったイメージは次のVブランクを待たずにリリースされ得る、テアリングが起こる
	*/
	const VkPresentModeKHR PresentMode = [&]() {
		for (auto i : PresentModes) {
			if (VK_PRESENT_MODE_MAILBOX_KHR == i) {
				return i;
			}
		}
		for (auto i : PresentModes) {
			if (VK_PRESENT_MODE_FIFO_KHR == i) {
				return i;
			}
		}
		return PresentModes[0];
	}();

#ifdef DEBUG_STDOUT
	std::cout << "\t" << Lightblue << "Present Mode" << White << std::endl;
#define VK_PRESENT_MODE_ENTRY(entry) case VK_PRESENT_MODE_##entry##_KHR: std::cout << "\t" << "\t" << #entry << std::endl; break
	for (auto i : PresentModes) {
		if (PresentMode == i) {
			std::cout << Yellow;
		}
		switch (i)
		{
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

	return PresentMode;
}
void VK::CreateSwapchain(const uint32_t Width, const uint32_t Height)
{
	//!< サーフェスの情報を取得
	VkSurfaceCapabilitiesKHR SurfaceCapabilities;
	VERIFY_SUCCEEDED(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(PhysicalDevice, Surface, &SurfaceCapabilities));

	//!< イメージ枚数 (ここでは1枚多く取る ... MAILBOX の場合3枚あった方が良い)
	const auto MinImageCount = (std::min)(SurfaceCapabilities.minImageCount + 1, SurfaceCapabilities.maxImageCount);

	//!< サーフェスのサイズが未定義の場合は明示的に指定する。(サーフェスのサイズが定義されている場合はそれに従わないとならない)
	if (-1 == SurfaceCapabilities.currentExtent.width) {
		SurfaceExtent2D = {
			(std::max)((std::min)(Width, SurfaceCapabilities.maxImageExtent.width), SurfaceCapabilities.minImageExtent.width), 
			(std::max)((std::min)(Height, SurfaceCapabilities.minImageExtent.height), SurfaceCapabilities.minImageExtent.height) 
		};
	}
	else {
		SurfaceExtent2D = SurfaceCapabilities.currentExtent;
	}
#ifdef DEBUG_STDOUT
	std::cout << "\t" << "\t" << Lightblue << "ImageExtent (" << (-1 == SurfaceCapabilities.currentExtent.width ? "Undefined" : "Defined") << ")" << White << std::endl;
	std::cout << "\t" << "\t" << "\t" << SurfaceExtent2D.width << " x " << SurfaceExtent2D.height << std::endl;
#endif

	//!< イメージ使用法 (可能ならVK_IMAGE_USAGE_TRANSFER_DST_BIT をセットする。イメージクリアできるように)
	const VkImageUsageFlags ImageUsageFlags = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | (VK_IMAGE_USAGE_TRANSFER_DST_BIT & SurfaceCapabilities.supportedUsageFlags);

	//!< サーフェスを回転、反転させるかどうか。(可能なら VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR)
	const auto PreTransform = (SurfaceCapabilities.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR) ? VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR : SurfaceCapabilities.currentTransform;

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
	VERIFY_SUCCEEDED(vkCreateSwapchainKHR(Device, &SwapchainCreateInfo, nullptr, &Swapchain));
#ifdef DEBUG_STDOUT
	std::cout << "\t" << "Swapchain" << std::endl;
#endif
	if (VK_NULL_HANDLE != OldSwapchain) {
		for (auto i : SwapchainImageViews) {
			vkDestroyImageView(Device, i, nullptr);
		}
		vkDestroySwapchainKHR(Device, OldSwapchain, nullptr);
	}

#ifdef DEBUG_STDOUT
	std::cout << "\t" << "SwapchainImageIndex = " << SwapchainImageIndex << std::endl;
#endif
}
void VK::CreateSwapchain(const VkCommandBuffer CommandBuffer)
{
	CreateSwapchainOfClientRect();
	CreateSwapchainImageView(CommandBuffer);

#ifdef DEBUG_STDOUT
	std::cout << "CreateSwapchain" << COUT_OK << std::endl << std::endl;
#endif
}

void VK::CreateSwapchainImageView(VkCommandBuffer CommandBuffer)
{
	//!< スワップチェインイメージの取得
	uint32_t SwapchainImageCount;
	VERIFY_SUCCEEDED(vkGetSwapchainImagesKHR(Device, Swapchain, &SwapchainImageCount, nullptr));
	assert(SwapchainImageCount && "Swapchain image count == 0");
	SwapchainImages.resize(SwapchainImageCount);
	VERIFY_SUCCEEDED(vkGetSwapchainImagesKHR(Device, Swapchain, &SwapchainImageCount, SwapchainImages.data()));
#ifdef _DEBUG
	std::cout << "\t" << "SwapchainImageCount = " << SwapchainImageCount << std::endl;
#endif

	const VkCommandBufferBeginInfo CommandBufferBeginInfo = {
		VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		nullptr,
		VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
		nullptr
	};
	VERIFY_SUCCEEDED(vkBeginCommandBuffer(CommandBuffer, &CommandBufferBeginInfo)); {
		for (auto& i : SwapchainImages) {
			const VkImageMemoryBarrier ImageMemoryBarrier_PresentToTransfer = {
				VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER, 
				nullptr,                                
				0, 
				VK_ACCESS_TRANSFER_WRITE_BIT,         
				VK_IMAGE_LAYOUT_UNDEFINED, //!<「現在のレイアウト」or「UNDEFINED」を指定すること、イメージコンテンツを保持したい場合は「UNDEFINED」はダメ         
				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,    
				PresentQueueFamilyIndex,
				PresentQueueFamilyIndex, 
				i,
				ImageSubresourceRange_Color
			};
			const VkImageMemoryBarrier ImageMemoryBarrier_TransferToPresent = {
				VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,    
				nullptr,
				VK_ACCESS_TRANSFER_WRITE_BIT,
				VK_ACCESS_MEMORY_READ_BIT,
				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
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
				1, &ImageMemoryBarrier_PresentToTransfer); {

				//!< 初期カラーで塗りつぶす
				const VkClearColorValue Green = { 0.0f, 1.0f, 0.0f, 1.0f };
				vkCmdClearColorImage(CommandBuffer, i, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &Green, 1, &ImageSubresourceRange_Color);
			
			} vkCmdPipelineBarrier(CommandBuffer, 
				VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 
				0,
				0, nullptr, 
				0, nullptr, 
				1, &ImageMemoryBarrier_TransferToPresent);
		}
	} VERIFY_SUCCEEDED(vkEndCommandBuffer(CommandBuffer));

	VERIFY_SUCCEEDED(vkDeviceWaitIdle(Device));
	const VkSubmitInfo SubmitInfo = {
		VK_STRUCTURE_TYPE_SUBMIT_INFO,
		nullptr,
		0, nullptr, nullptr,
		1,  &CommandBuffer,
		0, nullptr
	};
	VERIFY_SUCCEEDED(vkQueueSubmit(GraphicsQueue, 1, &SubmitInfo, Fence));
	VERIFY_SUCCEEDED(vkQueueWaitIdle(GraphicsQueue));

	WaitForFence();

	for(auto i : SwapchainImages) {
		const VkImageViewCreateInfo ImageViewCreateInfo = {
			VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
			nullptr,
			0,
			i,
			VK_IMAGE_VIEW_TYPE_2D,
			ColorFormat,
			ComponentMapping_SwizzleIdentity,
			ImageSubresourceRange_Color,
		};

		VkImageView ImageView;
		VERIFY_SUCCEEDED(vkCreateImageView(Device, &ImageViewCreateInfo, nullptr, &ImageView));
		SwapchainImageViews.push_back(ImageView);

#ifdef DEBUG_STDOUT
		std::cout << "\t" << "SwapchainImageView" << std::endl;
#endif
	}

	VERIFY_SUCCEEDED(vkAcquireNextImageKHR(Device, Swapchain, UINT64_MAX, PresentSemaphore, nullptr, &SwapchainImageIndex));
}

void VK::CreateDepthStencilImage()
{
	DepthFormat = GetSupportedDepthFormat();

	const VkExtent3D Extent3D = {
		SurfaceExtent2D.width, SurfaceExtent2D.height, 1
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

#ifdef DEBUG_STDOUT
	std::cout << "\t" << "DepthStencilImage" << std::endl;
#endif
}
void VK::CreateDepthStencilDeviceMemory()
{
	VkMemoryRequirements MemoryRequirements;
	vkGetImageMemoryRequirements(Device, DepthStencilImage, &MemoryRequirements);
	const VkMemoryAllocateInfo MemoryAllocateInfo = {
		VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
		nullptr,
		MemoryRequirements.size,
		GetMemoryType(MemoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) //!< DEVICE_LOCAL にすること
	};
	VERIFY_SUCCEEDED(vkAllocateMemory(Device, &MemoryAllocateInfo, nullptr, &DepthStencilDeviceMemory));
	VERIFY_SUCCEEDED(vkBindImageMemory(Device, DepthStencilImage, DepthStencilDeviceMemory, 0));

#ifdef DEBUG_STDOUT
	std::cout << "\t" << "DepthStencilDeviceMemory" << std::endl;
#endif
}
void VK::CreateDepthStencilView(VkCommandBuffer CommandBuffer)
{
	const VkImageViewCreateInfo ImageViewCreateInfo = {
		VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
		nullptr,
		0,
		DepthStencilImage,
		VK_IMAGE_VIEW_TYPE_2D,
		DepthFormat,
		ComponentMapping_SwizzleIdentity,
		ImageSubresourceRange_DepthStencil
	};
	VERIFY_SUCCEEDED(vkCreateImageView(Device, &ImageViewCreateInfo, nullptr, &DepthStencilImageView));

#ifdef DEBUG_STDOUT
	std::cout << "\t" << "DepthStencilImageView" << std::endl;
#endif
}
void VK::CreateDepthStencil(const VkCommandBuffer CommandBuffer)
{
	//CreateDepthStencilImage();
	//CreateDepthStencilDeviceMemory();
	//CreateDepthStencilView(CommandBuffer);

#ifdef DEBUG_STDOUT
	std::cout << "CreateDepthStencil" << COUT_OK << std::endl << std::endl;
#endif
}

/**
@brief シェーダとのバインディングのレイアウト
@note DescriptorSetLayout は「型」のようなもの
# TODO ここの実装は消す、Extへ持っていく
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

#ifdef DEBUG_STDOUT
	std::cout << "CreateDescriptorSetLayout" << COUT_OK << std::endl << std::endl;
#endif
}
void VK::CreateDescritporPool()
{
	const std::vector<VkDescriptorPoolSize> DescriptorPoolSizes = {
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

#ifdef DEBUG_STDOUT
		std::cout << "CreateDescriptorPool" << COUT_OK << std::endl << std::endl;
#endif
	}
}
/**
@brief シェーダとのバインディングのレイアウト
@note DescriptorSet は「DescriptorSetLayt 型」のインスタンスのようなもの
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

#ifdef DEBUG_STDOUT
		std::cout << "CreateDescriptorSet" << COUT_OK << std::endl << std::endl;
#endif
	}
}
void VK::CreateVertexInput()
{
#ifdef DEBUG_STDOUT
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

#ifdef DEBUG_STDOUT
	std::cout << "CreateViewport" << COUT_OK << std::endl << std::endl;
#endif
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

#ifdef DEBUG_STDOUT
	std::cout << "CreatePipelineLayout" << COUT_OK << std::endl << std::endl;
#endif
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

	return ShaderModule;
}

void VK::CreateGraphicsPipeline()
{
#ifdef DEBUG_STDOUT
	std::cout << "CreateGraphicsPipeline" << COUT_OK << std::endl << std::endl;
#endif
}

void VK::CreateComputePipeline()
{
	std::vector<VkShaderModule> ShaderModules;
	{
		ShaderModules.push_back(CreateShaderModule(SHADER_PATH L"CS.cpom.spv"));
		//#TODO
	}
	for (auto i : ShaderModules) {
		vkDestroyShaderModule(Device, i, nullptr);
	}

#ifdef DEBUG_STDOUT
	std::cout << "CreateComputePipeline" << COUT_OK << std::endl << std::endl;
#endif
}

void VK::CreateRenderPass()
{
#ifdef DEBUG_STDOUT
	std::cout << "CreateRenderPass" << COUT_OK << std::endl << std::endl;
#endif
}

void VK::CreateFramebuffer()
{
#ifdef DEBUG_STDOUT
	std::cout << "CreateFramebuffer" << COUT_OK << std::endl << std::endl;
#endif
}

void VK::CreateDeviceLocalBuffer(const VkCommandBuffer CommandBuffer, const VkBufferUsageFlags Usage, const VkAccessFlags AccessFlag, const VkPipelineStageFlagBits PipelineStageFlag, VkBuffer* Buffer, VkDeviceMemory* DeviceMemory, const size_t Size, const void* Source)
{
	VkBuffer StagingBuffer = VK_NULL_HANDLE;
	VkDeviceMemory StagingDeviceMemory = VK_NULL_HANDLE;
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
			GetMemoryType(StagingMemoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT /*| VK_MEMORY_PROPERTY_HOST_COHERENT_BIT*/) //!< HOST_VISIBLE にすること
		};
		VERIFY_SUCCEEDED(vkAllocateMemory(Device, &StagingMemoryAllocateInfo, nullptr, &StagingDeviceMemory));

		void *Data;
		VERIFY_SUCCEEDED(vkMapMemory(Device, StagingDeviceMemory, 0, StagingMemoryAllocateInfo.allocationSize, 0, &Data)); {
			memcpy(Data, Source, Size);

			//!< ↓VK_MEMORY_PROPERTY_HOST_COHERENT_BIT の場合は必要なし
			const std::vector<VkMappedMemoryRange> MappedMemoryRanges = {
				{
					VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE,
					nullptr,
					StagingDeviceMemory,
					0,
					VK_WHOLE_SIZE
				}
			};
			VERIFY_SUCCEEDED(vkFlushMappedMemoryRanges(Device, static_cast<uint32_t>(MappedMemoryRanges.size()), MappedMemoryRanges.data()));

		} vkUnmapMemory(Device, StagingDeviceMemory);
		VERIFY_SUCCEEDED(vkBindBufferMemory(Device, StagingBuffer, StagingDeviceMemory, 0));

		//!< デバイスローカル用のバッファとメモリを作成
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
			GetMemoryType(MemoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) //!< DEVICE_LOCAL にすること
		};
		VERIFY_SUCCEEDED(vkAllocateMemory(Device, &MemoryAllocateInfo, nullptr, DeviceMemory));
		VERIFY_SUCCEEDED(vkBindBufferMemory(Device, *Buffer, *DeviceMemory, 0));

		//!< ステージングバッファ から デバイスローカルバッファ　へコピーするコマンドを発行
		const VkCommandBufferBeginInfo CommandBufferBeginInfo = {
			VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
			nullptr,
			VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
			nullptr
		};
		VERIFY_SUCCEEDED(vkBeginCommandBuffer(CommandBuffer, &CommandBufferBeginInfo)); {
			const VkBufferCopy BufferCopy = {
				0,
				0,
				Size
			};
			vkCmdCopyBuffer(CommandBuffer, StagingBuffer, *Buffer, 1, &BufferCopy);

			//!< バッファを「転送先として」から「目的のバッファとして」へ変更する
			const VkBufferMemoryBarrier BufferMemoryBarrier = {
				VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
				nullptr,
				VK_ACCESS_MEMORY_WRITE_BIT,
				AccessFlag,
				VK_QUEUE_FAMILY_IGNORED,
				VK_QUEUE_FAMILY_IGNORED,
				*Buffer,
				0,
				VK_WHOLE_SIZE
			};
			vkCmdPipelineBarrier(CommandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, PipelineStageFlag, 0, 0, nullptr, 1, &BufferMemoryBarrier, 0, nullptr);

		} VERIFY_SUCCEEDED(vkEndCommandBuffer(CommandBuffer));

		//!< サブミット
		const VkSubmitInfo SubmitInfo = {
			VK_STRUCTURE_TYPE_SUBMIT_INFO,
			nullptr,
			0, nullptr,
			nullptr,
			1, &CommandBuffer,
			0, nullptr
		};
		VERIFY_SUCCEEDED(vkQueueSubmit(GraphicsQueue, 1, &SubmitInfo, VK_NULL_HANDLE));
		VERIFY_SUCCEEDED(vkDeviceWaitIdle(Device)); //!< フェンスでも良い
	}

	if(VK_NULL_HANDLE != StagingDeviceMemory) {
		vkFreeMemory(Device, StagingDeviceMemory, nullptr);
	}
	if (VK_NULL_HANDLE != StagingBuffer) {
		vkDestroyBuffer(Device, StagingBuffer, nullptr);
	}
}
void VK::CreateHostVisibleBuffer(const VkBufferUsageFlagBits Usage, VkBuffer* Buffer, VkDeviceMemory* DeviceMemory, const size_t Size, const void* Source)
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
	vkGetBufferMemoryRequirements(Device, *Buffer, &MemoryRequirements);
	const VkMemoryAllocateInfo MemoryAllocateInfo = {
		VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
		nullptr,
		MemoryRequirements.size,
		GetMemoryType(MemoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT/*| VK_MEMORY_PROPERTY_HOST_COHERENT_BIT*/) //!< HOST_VISIBLE にすること
	};
	VERIFY_SUCCEEDED(vkAllocateMemory(Device, &MemoryAllocateInfo, nullptr, DeviceMemory));
	void *Data;
	VERIFY_SUCCEEDED(vkMapMemory(Device, *DeviceMemory, 0, MemoryAllocateInfo.allocationSize, 0, &Data)); {
		memcpy(Data, Source, Size);
		
		//!< ↓VK_MEMORY_PROPERTY_HOST_COHERENT_BIT の場合は必要なし
		const std::vector<VkMappedMemoryRange> MappedMemoryRanges = {
			{
				VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE,
				nullptr,
				*DeviceMemory,
				0,
				VK_WHOLE_SIZE
			}
		};
		VERIFY_SUCCEEDED(vkFlushMappedMemoryRanges(Device, static_cast<uint32_t>(MappedMemoryRanges.size()), MappedMemoryRanges.data()));

	} vkUnmapMemory(Device, *DeviceMemory);
	VERIFY_SUCCEEDED(vkBindBufferMemory(Device, *Buffer, *DeviceMemory, 0));
}
void VK::CreateVertexBuffer(const VkCommandBuffer CommandBuffer)
{
#ifdef DEBUG_STDOUT
	std::cout << "CreateVertexBuffer" << COUT_OK << std::endl << std::endl;
#endif
}
void VK::CreateIndexBuffer(const VkCommandBuffer CommandBuffer)
{
#ifdef DEBUG_STDOUT
	std::cout << "CreateIndexBuffer" << COUT_OK << std::endl << std::endl;
#endif
}
void VK::CreateUniformBuffer()
{
	glm::vec4 Color(1.0f, 0.0f, 0.0f, 1.0f);
	const auto Size = sizeof(Color);

	CreateHostVisibleBuffer(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, &UniformBuffer, &UniformDeviceMemory, Size, &Color);

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

#ifdef DEBUG_STDOUT
	std::cout << "CreateUniformBuffer" << COUT_OK << std::endl << std::endl;
#endif
}

void VK::PopulateCommandBuffer(const VkCommandBuffer CommandBuffer)
{
	//!< VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT ... メモリをコマンドプールへ返す
	//VERIFY_SUCCEEDED(vkResetCommandBuffer(CommandBuffer, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT));

	//const VkCommandBufferInheritanceInfo CommandBufferInheritanceInfo = {
	//	VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO,
	//	nullptr,
	//	VK_NULL_HANDLE,
	//	0,
	//	VK_NULL_HANDLE,
	//	VK_FALSE,
	//	0,
	//	0
	//};
	const VkCommandBufferBeginInfo BeginInfo = {
		VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		nullptr,
		VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT,
		nullptr//&CommandBufferInheritanceInfo
	};
	VERIFY_SUCCEEDED(vkBeginCommandBuffer(CommandBuffer, &BeginInfo)); {
		vkCmdSetViewport(CommandBuffer, 0, static_cast<uint32_t>(Viewports.size()), Viewports.data());
		vkCmdSetScissor(CommandBuffer, 0, static_cast<uint32_t>(ScissorRects.size()), ScissorRects.data());

		auto Image = SwapchainImages[SwapchainImageIndex];
		SetImageLayout(CommandBuffer, Image, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, ImageSubresourceRange_Color); {
			const std::vector<VkClearValue> ClearValues = {
				{ Colors::SkyBlue }, { 1.0f, 0 }
			};
			const VkRenderPassBeginInfo RenderPassBeginInfo = {
				VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
				nullptr,
				RenderPass,
				Framebuffers[SwapchainImageIndex],
				ScissorRects[0],
				static_cast<uint32_t>(ClearValues.size()), ClearValues.data()
			};
			vkCmdBeginRenderPass(CommandBuffer, &RenderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE); {
				//Clear(CommandBuffer);
			} vkCmdEndRenderPass(CommandBuffer);
		} SetImageLayout(CommandBuffer, Image, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, ImageSubresourceRange_Color);
	} VERIFY_SUCCEEDED(vkEndCommandBuffer(CommandBuffer));
}
void VK::Draw()
{
	auto CommandPool = CommandPools[0];
	VERIFY_SUCCEEDED(vkResetCommandPool(Device, CommandPool, VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT));

	auto CommandBuffer = CommandBuffers[0];
	PopulateCommandBuffer(CommandBuffer);

	VERIFY_SUCCEEDED(vkDeviceWaitIdle(Device));
	const VkPipelineStageFlags PipelineStageFlags = VK_PIPELINE_STAGE_TRANSFER_BIT;
	const VkSubmitInfo SubmitInfo = {
		VK_STRUCTURE_TYPE_SUBMIT_INFO,
		nullptr,
		1, &PresentSemaphore, &PipelineStageFlags,
		1,  &CommandBuffer,
		1, &RenderSemaphore
	};
	VERIFY_SUCCEEDED(vkQueueSubmit(GraphicsQueue, 1, &SubmitInfo, Fence));
	VERIFY_SUCCEEDED(vkQueueWaitIdle(GraphicsQueue));

	WaitForFence();

	Present();
}
void VK::Present()
{
	const VkPresentInfoKHR PresentInfo = {
		VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
		nullptr,
		1, &RenderSemaphore,
		1, &Swapchain, &SwapchainImageIndex,
		nullptr
	};
	VERIFY_SUCCEEDED(vkQueuePresentKHR(GraphicsQueue, &PresentInfo));

	VERIFY_SUCCEEDED(vkAcquireNextImageKHR(Device, Swapchain, UINT64_MAX, PresentSemaphore, VK_NULL_HANDLE, &SwapchainImageIndex));

#ifdef DEBUG_STDOUT
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

#ifdef DEBUG_STDOUT
	//std::cout << "Fence" << std::endl;
#endif
}

#ifdef _DEBUG
#define VK_INSTANCE_PROC_ADDR(proc) PFN_vk ## proc ## EXT DebugReport::vk ## proc;
#include "VKProcInstanceAddr.h"
#undef VK_INSTANCE_PROC_ADDR

void DebugReport::GetInstanceProcAddr(VkInstance Instance)
{
#ifdef _DEBUG
#define VK_INSTANCE_PROC_ADDR(proc) vk ## proc = reinterpret_cast<PFN_vk ## proc ## EXT>(vkGetInstanceProcAddr(Instance, "vk" #proc "EXT"));
#include "VKProcInstanceAddr.h"
#undef VK_INSTANCE_PROC_ADDR
#endif
}
void DebugReport::DestroyDebugReportCallback(VkInstance Instance, VkDebugReportCallbackEXT DebugReportCallback)
{
	if (VK_NULL_HANDLE != DebugReportCallback) {
		vkDestroyDebugReportCallback(Instance, DebugReportCallback, nullptr);
	}
}

#define VK_DEVICE_PROC_ADDR(proc) PFN_vk ## proc ## EXT DebugMarker::vk ## proc = VK_NULL_HANDLE;
#include "VKProcDeviceAddr.h"
#undef VK_DEVICE_PROC_ADDR
/**
(初回のみ)RenderDoc を起動、Warning が出ていたらクリックして VulkanCapture を有効にしておくこと、Windows のレジストリが作成される
RenderDoc から実行した場合にしか VK_EXT_DEBUG_MARKER_EXTENSION_NAME は有効にならないので注意
*/
bool DebugMarker::HasDebugMarkerExtension(VkPhysicalDevice PhysicalDevice)
{
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
					if (!strcmp(VK_EXT_DEBUG_MARKER_EXTENSION_NAME, j.extensionName)) {
						return true;
					}
				}
			}
		}
	}
	return false;
}
void DebugMarker::GetDeviceProcAddr(VkDevice Device)
{
#define VK_DEVICE_PROC_ADDR(proc) vk ## proc = reinterpret_cast<PFN_vk ## proc ## EXT>(vkGetDeviceProcAddr(Device, "vk" #proc "EXT"));
#include "VKProcDeviceAddr.h"
#undef VK_DEVICE_PROC_ADDR
}
void DebugMarker::Insert(VkCommandBuffer CommandBuffer, const char* Name, const glm::vec4& Color)
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
void DebugMarker::Begin(VkCommandBuffer CommandBuffer, const char* Name, const glm::vec4& Color)
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
void DebugMarker::End(VkCommandBuffer CommandBuffer)
{
	if (VK_NULL_HANDLE != vkCmdDebugMarkerEnd) {
		vkCmdDebugMarkerEnd(CommandBuffer);
	}
}
#endif


