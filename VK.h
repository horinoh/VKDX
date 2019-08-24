#pragma once

#include <fstream>

//!< VK_NO_PROTOYYPES が定義されてる場合は DLL を使用する。If VK_NO_PROTOYYPES is defined, using DLL. 
//!< Vk.props 内 C/C++ - Preprocessor - Preprocessor Definitions に定義してある Definition is in VK.props in C/C++ - Preprocessor - Preprocessor Definitions
//#define VK_NO_PROTOTYPES //!< VK.props に定義

#ifdef _WINDOWS
#define VK_USE_PLATFORM_WIN32_KHR
#else
#define VK_USE_PLATFORM_XLIB_KHR
//#define VK_USE_PLATFORM_XCB_KHR
//#define VK_USE_PLATFORM_WAYLAND_KHR
//#define VK_USE_PLATFORM_ANDROID_KHR
#endif
#include <vulkan/vulkan.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#ifndef BREAK_ON_FAILED
#define BREAK_ON_FAILED(vr) if(VK_SUCCESS != (vr)) { Log(VK::GetVkResultString(vr).c_str()); DEBUG_BREAK(); }
#endif
#ifndef THROW_ON_FAILED
#define THROW_ON_FAILED(vr) if(VK_SUCCESS != (vr)) { throw std::runtime_error("VERIFY_SUCCEEDED failed : " + VK::GetVkResultString(vr)); }
#endif
#ifndef MESSAGEBOX_ON_FAILED
#define MESSAGEBOX_ON_FAILED(vr) if(VK_SUCCESS != (vr)) { Win::ShowMessageBoxW(nullptr, VK::GetVkResultStringW(vr)); }
#endif

#ifdef _DEBUG
#define USE_RENDERDOC
#endif

#define USE_VIEWPORT_Y_UP

#include "Cmn.h"
#ifdef _WINDOWS
#include "Win.h"
#endif

#ifndef USE_DEBUG_MARKER
//!< エクステンションは見つかるのに、使用できないので封印... Extension is found, but not available...
//!< VK_EXT_debug_marker not available for devices associated with ICD nvoglv64.dll
//!< https://devtalk.nvidia.com/default/topic/1001794/vulkan-vk_ext_debug_marker-missing-after-new-5-2-build-update-/
//#define USE_DEBUG_MARKER
#endif

namespace Colors
{
	const VkClearColorValue Black = { 0.0f, 0.0f, 0.0f, 1.0f };
	const VkClearColorValue Blue = { 0.0f, 0.0f, 1.0f, 1.0f };
	const VkClearColorValue Brown = { 0.647058845f, 0.164705887f, 0.164705887f, 1.0f };
	const VkClearColorValue Gray = { 0.501960814f, 0.501960814f, 0.501960814f, 1.0f };
	const VkClearColorValue Green = { 0.0f, 0.501960814f, 0.0f, 1.0f };
	const VkClearColorValue Magenta = { 1.0f, 0.0f, 1.0f, 1.0f };
	const VkClearColorValue Orange = { 1.0f, 0.647058845f, 0.0f, 1.0f };
	const VkClearColorValue Pink = { 1.0f, 0.752941251f, 0.796078503f, 1.0f };
	const VkClearColorValue Purple = { 0.501960814f, 0.0f, 0.501960814f, 1.0f };
	const VkClearColorValue Red = { 1.0f, 0.0f, 0.0f, 1.0f };
	const VkClearColorValue SkyBlue = { 0.529411793f, 0.807843208f, 0.921568692f, 1.0f };
	const VkClearColorValue Transparent = { 0.0f, 0.0f, 0.0f, 0.0f };
	const VkClearColorValue White = { 1.0f, 1.0f, 1.0f, 1.0f };
	const VkClearColorValue Yellow = { 1.0f, 1.0f, 0.0f, 1.0f };
}

class VK : public Cmn
#ifdef _WINDOWS
	, public Win
#endif
{
private:
	using Super = Win;

public:
#ifdef _WINDOWS
	virtual void OnCreate(HWND hWnd, HINSTANCE hInstance, LPCWSTR Title) override;
	//virtual void OnSize(HWND hWnd, HINSTANCE hInstance) override {}
	virtual void OnExitSizeMove(HWND hWnd, HINSTANCE hInstance) override;
	//virtual void OnTimer(HWND hWnd, HINSTANCE hInstance) override { Super::OnTimer(hWnd, hInstance); }
	virtual void OnPaint(HWND hWnd, HINSTANCE hInstance) override { 
		Super::OnPaint(hWnd, hInstance);
		Draw(); 
	}
	virtual void OnDestroy(HWND hWnd, HINSTANCE hInstance) override;
#endif

	static char* GetVkResultChar(const VkResult Result);
	static std::string GetVkResultString(const VkResult Result) { return std::string(GetVkResultChar(Result)); }
	static std::wstring GetVkResultWstring(const VkResult Result) { return ToWString(GetVkResultString(Result)); }
	static char* GetFormatChar(const VkFormat Format);
	static std::string GetFormatString(const VkFormat Format) { return std::string(GetFormatChar(Format)); }
	static std::wstring GetFormatWstring(const VkFormat Format) { return ToWString(GetFormatString(Format)); }
	static char* GetColorSpaceChar(const VkColorSpaceKHR ColorSpace);
	static std::string GetColorSpaceString(const VkColorSpaceKHR ColorSpace) { return std::string(GetColorSpaceChar(ColorSpace)); }
	static std::wstring GetColorSpaceWstring(const VkColorSpaceKHR ColorSpace) { return ToWString(GetColorSpaceString(ColorSpace)); }
	static char* GetImageViewTypeChar(const VkImageViewType ImageViewType);
	static std::string GetImageViewTypeString(const VkImageViewType ImageViewType) { return std::string(GetImageViewTypeChar(ImageViewType)); }
	static std::wstring GetImageViewTypeWstring(const VkImageViewType ImageViewType) { return ToWString(GetImageViewTypeString(ImageViewType)); }
	static char* GetComponentSwizzleChar(const VkComponentSwizzle ComponentSwizzle);
	static std::string GetComponentSwizzleString(const VkComponentSwizzle ComponentSwizzle) { return std::string(GetComponentSwizzleChar(ComponentSwizzle)); }
	static std::wstring GetComponentSwizzleWstring(const VkComponentSwizzle ComponentSwizzle) { return ToWString(GetComponentSwizzleString(ComponentSwizzle)); }
	//static char* GetComponentMappingChar(const VkComponentMapping& ComponentMapping);
	static std::string GetComponentMappingString(const VkComponentMapping& ComponentMapping) {
		return GetComponentSwizzleString(ComponentMapping.r)
			+ ", " + GetComponentSwizzleString(ComponentMapping.g)
			+ ", " + GetComponentSwizzleString(ComponentMapping.b)
			+ ", " + GetComponentSwizzleString(ComponentMapping.a);
	}
	static std::wstring GetComponentMappingWstring(const VkComponentMapping& ComponentMapping) { return ToWString(GetComponentMappingString(ComponentMapping)); }

protected:
	static FORCEINLINE void* AlignedMalloc(void* pUserData, size_t size, size_t alignment, VkSystemAllocationScope allocationScope) { return _aligned_malloc(size, alignment); }
	static FORCEINLINE void* AlignedRealloc(void* pUserData, void* pOriginal, size_t size, size_t alignment, VkSystemAllocationScope allocationScope) { return _aligned_realloc(pOriginal, size, alignment); }
	static FORCEINLINE void AlignedFree(void* pUserData, void* pMemory) { _aligned_free(pMemory); }
	static void AlignedAllocNotify(void* pUserData, size_t size, VkInternalAllocationType allocationType, VkSystemAllocationScope allocationScope) {}
	static void AligendFreeNotify(void* pUserData, size_t size, VkInternalAllocationType allocationType, VkSystemAllocationScope allocationScope) {}

	static bool IsSupportedDepthFormat(VkPhysicalDevice PhysicalDevice, const VkFormat DepthFormat);
	static uint32_t GetMemoryTypeIndex(const VkPhysicalDeviceMemoryProperties& PDMP, const uint32_t TypeBits, const VkFlags Properties);

	virtual void CreateBuffer(VkBuffer* Buffer, const VkBufferUsageFlags Usage, const size_t Size) const;
	virtual void ValidateImageCreateInfo(const VkImageCreateInfo& ICI) const {
		if (ICI.flags & VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT) {
			assert(ICI.samples == VK_SAMPLE_COUNT_1_BIT && "Must be VK_SAMPLE_COUNT_1_BIT");
			assert(ICI.extent.width == ICI.extent.height && "Must be square");
			assert(ICI.arrayLayers >= 6 && "Invalid ArrayLayers");
		}
		else {
			assert(ICI.arrayLayers >= 1 && "Invalid ArrayLayers");
		}
	}
	virtual void CreateImage(VkImage* Image, const VkImageCreateFlags CreateFlags, const VkImageType ImageType, const VkFormat Format, const VkExtent3D& Extent3D, const uint32_t MipLevels, const uint32_t ArrayLayers, const VkSampleCountFlagBits SampleCount, const VkImageUsageFlags Usage) const;

	virtual void CopyToHostVisibleDeviceMemory(const VkDeviceMemory DeviceMemory, const size_t Size, const void* Source, const VkDeviceSize Offset = 0);
	virtual void CmdCopyBufferToBuffer(const VkCommandBuffer CB, const VkBuffer Src, const VkBuffer Dst, const VkAccessFlags AF, const VkPipelineStageFlagBits PSF, const size_t Size);

	void EnumerateMemoryRequirements(const VkMemoryRequirements& MR);
	void AllocateBufferMemory(VkDeviceMemory* DM, const VkBuffer Buffer, const VkMemoryPropertyFlags MPF);
	void SuballocateBufferMemory(uint32_t& HeapIndex, VkDeviceSize& Offset, const VkBuffer Buffer, const VkMemoryPropertyFlags MPF);
	void AllocateImageMemory(VkDeviceMemory* DM, const VkImage Image, const VkMemoryPropertyFlags MPF);
	void SuballocateImageMemory(uint32_t& HeapIndex, VkDeviceSize& Offset, const VkImage Image, const VkMemoryPropertyFlags MPF);

	virtual void CreateBufferView(VkBufferView* BufferView, const VkBuffer Buffer, const VkFormat Format, const VkDeviceSize Offset = 0, const VkDeviceSize Range = VK_WHOLE_SIZE);
	virtual void CreateImageView(VkImageView* ImageView, const VkImage Image, const VkImageViewType ImageViewType, const VkFormat Format, const VkComponentMapping& ComponentMapping, const VkImageSubresourceRange& ImageSubresourceRange);

	virtual void ValidateFormatProperties(VkPhysicalDevice PD, const VkFormat Format, const VkImageUsageFlags Usage) const;
	virtual void ValidateFormatProperties_SampledImage(VkPhysicalDevice PD, const VkFormat Format, const VkImageUsageFlags Usage, const VkFilter Mag, const VkFilter Min, const VkSamplerMipmapMode Mip) const;
	virtual void ValidateFormatProperties_StorageImage(VkPhysicalDevice PD, const VkFormat Format, const VkImageUsageFlags Usage, const bool Atomic) const;

#ifdef _DEBUG
	static void MarkerInsert(VkCommandBuffer CB, const glm::vec4& Color, const char* Name);
	static void MarkerInsert(VkCommandBuffer CB, const glm::vec4& Color, const std::string& Name) { MarkerInsert(CB, Color, Name.c_str()); }
	static void MarkerInsert(VkCommandBuffer CB, const glm::vec4& Color, const std::wstring& Name) { MarkerInsert(CB, Color, ToString(Name)); }
	static void MarkerBegin(VkCommandBuffer CB, const glm::vec4& Color, const char* Name);
	static void MarkerBegin(VkCommandBuffer CB, const glm::vec4& Color, const std::string& Name) { MarkerBegin(CB, Color, Name.c_str()); }
	static void MarkerBegin(VkCommandBuffer CB, const glm::vec4& Color, const std::wstring& Name) { MarkerBegin(CB, Color, ToString(Name)); }
	static void MarkerEnd(VkCommandBuffer CB);
	class ScopedMarker
	{
	public:
		ScopedMarker(VkCommandBuffer CB, const glm::vec4& Color, const std::string& Name) : CommandBuffer(CB) { MarkerBegin(CommandBuffer, Color, Name); }
		~ScopedMarker() { MarkerEnd(CommandBuffer); }
	private:
		VkCommandBuffer CommandBuffer;
	};

	static void MarkerSetName(VkDevice Device, const VkDebugReportObjectTypeEXT Type, const uint64_t Object, const char* Name);
	static void MarkerSetTag(VkDevice Device, const VkDebugReportObjectTypeEXT Type, const uint64_t Object, const uint64_t TagName, const size_t TagSize, const void* TagData);
	template<typename T> static void MarkerSetObjectName(VkDevice Device, T Object, const char* Name) { DEBUG_BREAK(); /* テンプレート特殊化されていない Not template specialized */ }
	template<typename T> static void MarkerSetObjectName(VkDevice Device, T Object, const std::string& Name) { MarkerSetObjectName(Device, Name.c_str()); }
	template<typename T> static void MarkerSetObjectName(VkDevice Device, T Object, const std::wstring& Name) { MarkerSetObjectName(Device, ToString(Name)); }
	template<typename T> static void MarkerSetObjectTag(VkDevice Device, T Object, const uint64_t TagName, const size_t TagSize, const void* TagData) { DEBUG_BREAK(); /* テンプレート特殊化されていない Not template specialized */ }
	//!< ↓ここでテンプレート特殊化している Template specialization here
#include "VKDebugMarker.inl"
#endif

	virtual void EnumerateInstanceLayerProperties();
	virtual void EnumerateInstanceExtensionProperties(const char* LayerName);
	//const VkCommandBufferInheritanceInfo CommandBufferInheritanceInfo_None = {
	//	VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO,
	//	nullptr,
	//	VK_NULL_HANDLE,
	//	0,
	//	VK_NULL_HANDLE,
	//	VK_FALSE,
	//	0,
	//	0
	//};

#ifdef VK_NO_PROTOYYPES
	void LoadVulkanLibrary();
#endif //!< VK_NO_PROTOYYPES

	virtual void CreateInstance();
#ifdef _DEBUG
	virtual void CreateDebugReportCallback();
#endif
	virtual void CreateSurface(
#ifdef VK_USE_PLATFORM_WIN32_KHR
		HWND hWnd, HINSTANCE hInstance
#elif defined(VK_USE_PLATFORM_XLIB_KHR)
		Display* Dsp, Window Wnd
#else
		xcb_connection_t* Cnt, xcb_window_t Wnd
#endif
	);

	virtual void EnumeratePhysicalDeviceProperties(const VkPhysicalDeviceProperties& PDP);
	virtual void EnumeratePhysicalDeviceFeatures(const VkPhysicalDeviceFeatures& PDF);
	virtual void EnumeratePhysicalDeviceMemoryProperties(const VkPhysicalDeviceMemoryProperties& PDMP);
	virtual void EnumeratePhysicalDevice(VkInstance Instance);
	virtual void EnumeratePhysicalDeviceLayerProperties(VkPhysicalDevice PD);
	virtual void EnumeratePhysicalDeviceExtensionProperties(VkPhysicalDevice PD, const char* LayerName);
	virtual void EnumerateQueueFamilyProperties(VkPhysicalDevice PD, VkSurfaceKHR Surface, std::vector<VkQueueFamilyProperties>& QFPs);
	virtual void OverridePhysicalDeviceFeatures(VkPhysicalDeviceFeatures& PDF) const;
	virtual void CreateQueueFamilyPriorities(VkPhysicalDevice PD, VkSurfaceKHR Surface, const std::vector<VkQueueFamilyProperties>& QFPs, std::vector<std::vector<float>>& QueueFamilyPriorites);
	virtual void CreateDevice(VkPhysicalDevice PD, VkSurfaceKHR Surface);

	virtual void AllocateDeviceMemory();

	virtual void CreateFence(VkDevice Device);
	virtual void CreateSemaphore(VkDevice Device);

	virtual void CreateCommandPool();
	virtual void AllocateCommandBuffer();
	virtual void CreateCommandPool(VkCommandPool& CP, VkDevice Device, const VkCommandPoolCreateFlags Flags, const uint32_t QueueFamilyIndex);
	virtual void AllocateCommandBuffer(std::vector<VkCommandBuffer>& CB, const VkCommandPool CP, const VkCommandBufferLevel Level, const uint32_t Count);

	virtual VkSurfaceFormatKHR SelectSurfaceFormat(VkPhysicalDevice PD, VkSurfaceKHR Surface);
	virtual VkExtent2D SelectSurfaceExtent(const VkSurfaceCapabilitiesKHR& Cap, const uint32_t Width, const uint32_t Height);
	virtual VkImageUsageFlags SelectImageUsage(const VkSurfaceCapabilitiesKHR& Cap);
	virtual VkSurfaceTransformFlagBitsKHR SelectSurfaceTransform(const VkSurfaceCapabilitiesKHR& Cap);
	virtual VkPresentModeKHR SelectSurfacePresentMode(VkPhysicalDevice PD, VkSurfaceKHR Surface);
	virtual void CreateSwapchain(VkPhysicalDevice PD, VkSurfaceKHR Surface, const uint32_t Width, const uint32_t Height);
	virtual void CreateSwapchain(VkPhysicalDevice PD, VkSurfaceKHR Surface, const RECT& Rect) { CreateSwapchain(PD, Surface, static_cast<uint32_t>(Rect.right - Rect.left), static_cast<uint32_t>(Rect.bottom - Rect.top)); }
	virtual void ResizeSwapchain(const uint32_t Width, const uint32_t Height);
	virtual void ResizeSwapchain(const RECT& Rect) { ResizeSwapchain(static_cast<uint32_t>(Rect.right - Rect.left), static_cast<uint32_t>(Rect.bottom - Rect.top)); }
	virtual void GetSwapchainImage(VkDevice Device, VkSwapchainKHR Swapchain);
	virtual void CreateSwapchainImageView();
	virtual void InitializeSwapchainImage(const VkCommandBuffer CB, const VkClearColorValue* CCV = nullptr);
	
	virtual void CreateDepthStencil() {}
	virtual void CreateDepthStencil(const uint32_t Width, const uint32_t Height, const VkFormat DepthFormat);
	virtual void InitializeDepthStencilImage(const VkCommandBuffer CB);
	
	virtual void LoadImage(VkImage* Image, VkDeviceMemory *DeviceMemory, VkImageView* ImageView, const std::string& Path) { assert(false && "Not implemanted"); }
	virtual void LoadImage(VkImage* Image, VkDeviceMemory *DeviceMemory, VkImageView* ImageView, const std::wstring& Path) { LoadImage(Image, DeviceMemory, ImageView, ToString(Path)); }
	virtual void CreateViewport(const float Width, const float Height, const float MinDepth = 0.0f, const float MaxDepth = 1.0f);
	virtual void CreateViewportTopFront(const float Width, const float Height) { CreateViewport(Width, Height, 0.0f, 0.0f); }

	virtual void SubmitStagingCopy(const VkQueue Queue, const VkCommandBuffer CB, const VkBuffer Buffer, const VkDeviceSize Size, const void* Source, const VkAccessFlagBits AF, const VkPipelineStageFlagBits PSF);
	virtual void CreateVertexBuffer() {}
	virtual void CreateIndexBuffer() {}
	virtual void CreateIndirectBuffer() {}
	virtual void CreateUniformBuffer() {}
	virtual void CreateStorageBuffer(VkBuffer* Buffer, VkDeviceMemory* DeviceMemory, const VkDeviceSize Size);
	virtual void CreateStorageBuffer() {}
	virtual void CreateUniformTexelBuffer(VkBuffer* Buffer, VkDeviceMemory* DeviceMemory, const VkDeviceSize Size, const VkFormat Format, VkBufferView* View);
	virtual void CreateUniformTexelBuffer() {}
	virtual void CreateStorageTexelBuffer(VkBuffer* Buffer, VkDeviceMemory* DeviceMemory, const VkDeviceSize Size, const VkFormat Format, VkBufferView* View);
	virtual void CreateStorageTexelBuffer() {}

	virtual void CreateDescriptorSetLayout(VkDescriptorSetLayout& DSL, const std::initializer_list<VkDescriptorSetLayoutBinding> il_DSLBs);
	virtual void CreateDescriptorSetLayout() {}

	virtual void CreatePipelineLayout(VkPipelineLayout& PL, const std::initializer_list<VkDescriptorSetLayout> il_DSLs, const std::initializer_list<VkPushConstantRange> il_PCRs);
	virtual void CreatePipelineLayout() {
		PipelineLayouts.resize(1);
		CreatePipelineLayout(PipelineLayouts[0], {}, {});
	}

	virtual void CreateDescriptorPool(VkDescriptorPool& DP, const VkDescriptorPoolCreateFlags Flags, const std::initializer_list<VkDescriptorPoolSize> il_DPSs);
	virtual void CreateDescriptorPool() {}

	virtual void AllocateDescriptorSet(std::vector<VkDescriptorSet>& DSs, const VkDescriptorPool DP, const std::initializer_list <VkDescriptorSetLayout> il_DSLs);
	virtual void AllocateDescriptorSet() {}

	virtual void UpdateDescriptorSet(const std::initializer_list <VkWriteDescriptorSet> il_WDSs, const std::initializer_list <VkCopyDescriptorSet> il_CDSs);
	virtual void UpdateDescriptorSet() {}

	virtual void CreateTexture() {}
	virtual void CreateSampler(VkSampler* Sampler, const float MaxLOD = (std::numeric_limits<float>::max)()) const {}

	virtual void CreateRenderPass() { RenderPasses.resize(1); CreateRenderPass_Default(RenderPasses[0], ColorFormat); }
	virtual void CreateRenderPass_Default(VkRenderPass& RP, const VkFormat Color);

	virtual void CreateFramebuffer(VkFramebuffer& FB, const VkRenderPass RP, const uint32_t Width, const uint32_t Height, const uint32_t Layers, const std::initializer_list<VkImageView> il_IVs);
	virtual void CreateFramebuffer() {}
	virtual void DestroyFramebuffer();

	virtual VkShaderModule CreateShaderModule(const std::wstring& Path) const;
	virtual void CreateShaderModle(const std::initializer_list<VkShaderModule> il_SMs) { std::copy(il_SMs.begin(), il_SMs.end(), std::back_inserter(ShaderModules)); }
	virtual void CreateShaderModule() {}

	static bool ValidatePipelineCache(const VkPhysicalDevice PD, const size_t Size, const void* Data);
	virtual void CreatePipeline();
	void CreatePipeline_Default(VkPipeline& Pipeline, const VkPipelineLayout PL, 
		const VkShaderModule VS, const VkShaderModule FS, const VkShaderModule TES, const VkShaderModule TCS, const VkShaderModule GS,
		const VkRenderPass RP, VkPipelineCache PC = VK_NULL_HANDLE);
	//virtual void CreatePipeline_Compute();

	virtual void ClearColor(const VkCommandBuffer CommandBuffer, const VkImage Image, const VkClearColorValue& Color);
	virtual void ClearDepthStencil(const VkCommandBuffer CommandBuffer, const VkImage Image, const VkClearDepthStencilValue& DepthStencil);
	virtual void ClearColorAttachment(const VkCommandBuffer CommandBuffer, const VkClearColorValue& Color);
	virtual void ClearDepthStencilAttachment(const VkCommandBuffer CommandBuffer, const VkClearDepthStencilValue& DepthStencil);
	virtual void PopulateCommandBuffer(const size_t i);

	virtual void Draw();
	virtual void Dispatch();
	virtual void Present();

	const VkAllocationCallbacks AllocationCallbacks = {
		nullptr,
		AlignedMalloc,
		AlignedRealloc,
		AlignedFree,
		AlignedAllocNotify,
		AligendFreeNotify
	};
	const VkAllocationCallbacks* GetAllocationCallbacks() const { return nullptr/*&AllocationCallbacks*/; }
	
	void SetCurrentPhysicalDevice(VkPhysicalDevice PD) { CurrentPhysicalDevice = PD; vkGetPhysicalDeviceMemoryProperties(CurrentPhysicalDevice, &CurrentPhysicalDeviceMemoryProperties); }
	virtual VkPhysicalDevice GetCurrentPhysicalDevice() const { return CurrentPhysicalDevice; }; 
	virtual VkPhysicalDeviceMemoryProperties GetCurrentPhysicalDeviceMemoryProperties() const { return CurrentPhysicalDeviceMemoryProperties; }

#ifdef VK_NO_PROTOYYPES
protected:
#ifdef _WINDOWS
	HMODULE VulkanLibrary = nullptr;
#else
	void* VulkanLibrary = nullptr;
#endif

public:
	//static PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr;

	//!< グローバルレベル関数 Global level functions
#define VK_GLOBAL_PROC_ADDR(proc) static PFN_vk ## proc vk ## proc;
#include "VKGlobalProcAddr.h"
#undef VK_GLOBAL_PROC_ADDR

	//!< インスタンスレベル関数 Instance level functions
#define VK_INSTANCE_PROC_ADDR(proc) static PFN_vk ## proc vk ## proc;
#include "VKInstanceProcAddr.h"
#undef VK_INSTANCE_PROC_ADDR

	//!< デバイスレベル関数 Device level functions
#define VK_DEVICE_PROC_ADDR(proc) static PFN_vk ## proc vk ## proc;
#include "VKDeviceProcAddr.h"
#undef VK_DEVICE_PROC_ADDR
#endif //!< VK_NO_PROTOYYPES

#ifdef _DEBUG
public:
	//!< インスタンスレベル関数(Debug) Instance level functions(Debug)
#define VK_INSTANCE_PROC_ADDR(proc) static PFN_vk ## proc ## EXT vk ## proc;
#include "VKDebugReport.h"
#undef VK_INSTANCE_PROC_ADDR

	//!< デバイスレベル関数(Debug) Device level functions(Debug)
#define VK_DEVICE_PROC_ADDR(proc) static PFN_vk ## proc ## EXT vk ## proc;
#include "VKDebugMarker.h"
#undef VK_DEVICE_PROC_ADDR
#endif //!< _DEBUG

protected:
	//using LAYER_PROPERTY = std::pair<VkLayerProperties, std::vector<VkExtensionProperties>>;
	//using LAYER_PROPERTIES = std::vector<LAYER_PROPERTY>;
	//using PHYSICAL_DEVICE_LAYER_PROPERTY = std::pair<VkPhysicalDevice, LAYER_PROPERTIES>;
	//using PHYSICAL_DEVICE_LAYER_PROPERTIES = std::vector<PHYSICAL_DEVICE_LAYER_PROPERTY>;
	//LAYER_PROPERTIES InstanceLayerProperties;
	//PHYSICAL_DEVICE_LAYER_PROPERTIES PhysicalDeviceLayerProperties;

	VkInstance Instance = VK_NULL_HANDLE;
#ifdef _DEBUG
	VkDebugReportCallbackEXT DebugReportCallback = VK_NULL_HANDLE;
#endif
	VkSurfaceKHR Surface = VK_NULL_HANDLE;
	std::vector<VkPhysicalDevice> PhysicalDevices;
	VkPhysicalDevice CurrentPhysicalDevice = VK_NULL_HANDLE;
	VkPhysicalDeviceMemoryProperties CurrentPhysicalDeviceMemoryProperties;
	VkDevice Device = VK_NULL_HANDLE;
	VkQueue GraphicsQueue = VK_NULL_HANDLE;
	VkQueue PresentQueue = VK_NULL_HANDLE;
	VkQueue ComputeQueue = VK_NULL_HANDLE;
	//VkQueue TransferQueue = VK_NULL_HANDLE;
	//VkQueue SparceBindingQueue = VK_NULL_HANDLE;
	uint32_t GraphicsQueueFamilyIndex = UINT32_MAX;
	uint32_t GraphicsQueueIndex = UINT32_MAX; //!< ファミリ内でのインデックス (必ずしも覚えておく必要は無いが一応覚えておく)
	uint32_t PresentQueueFamilyIndex = UINT32_MAX;
	uint32_t PresentQueueIndex = UINT32_MAX;
	uint32_t ComputeQueueFamilyIndex = UINT32_MAX;
	uint32_t ComputeQueueIndex = UINT32_MAX;
	//uint32_t TransferQueueFamilyIndex = UINT32_MAX;
	//uint32_t TransferQueueIndex = UINT32_MAX;;
	//uint32_t SparceBindingQueueFamilyIndex = UINT32_MAX;
	//uint32_t SparceBindingQueueIndex = UINT32_MAX;;

	/**
	フェンス		... デバイスとホストの同期(GPUとCPUの同期)
	セマフォ		... 複数キュー間の同期
	イベント		... コマンドバッファ間の同期(同一キューファミリ)
	バリア		... コマンドバッファ内の同期
	*/
	VkFence Fence = VK_NULL_HANDLE;
	VkFence ComputeFence = VK_NULL_HANDLE;
	VkSemaphore NextImageAcquiredSemaphore = VK_NULL_HANDLE;	//!< プレゼント完了までウエイト
	VkSemaphore RenderFinishedSemaphore = VK_NULL_HANDLE;		//!< 描画完了するまでウエイト

	std::vector<VkCommandPool> CommandPools;
	std::vector<VkCommandBuffer> CommandBuffers;

	VkExtent2D SurfaceExtent2D;
	VkFormat ColorFormat = VK_FORMAT_B8G8R8A8_UNORM;
	VkSwapchainKHR Swapchain = VK_NULL_HANDLE;
	std::vector<VkImage> SwapchainImages;
	uint32_t SwapchainImageIndex = 0;
	std::vector<VkImageView> SwapchainImageViews;

	std::vector<VkDeviceMemory> DeviceMemories;
	std::vector<VkDeviceSize> DeviceMemoryOffsets;

	//VkFormat DepthFormat = VK_FORMAT_D24_UNORM_S8_UINT;
	VkImage DepthStencilImage = VK_NULL_HANDLE;
	VkDeviceMemory DepthStencilDeviceMemory = VK_NULL_HANDLE;
	VkImageView DepthStencilImageView = VK_NULL_HANDLE;

	VkImage Image = VK_NULL_HANDLE;
	VkDeviceMemory ImageDeviceMemory = VK_NULL_HANDLE;
	VkImageView ImageView = VK_NULL_HANDLE;
	std::vector<VkSampler> Samplers;

	std::vector<VkBuffer> VertexBuffers;
	std::vector<VkBuffer> IndexBuffers;
	std::vector<VkBuffer> IndirectBuffers;

	//!< 現状1つのみ、配列にする #VK_TODO
	VkBuffer UniformBuffer = VK_NULL_HANDLE;
	VkDeviceMemory UniformDeviceMemory = VK_NULL_HANDLE;
	VkDescriptorBufferInfo UniformDescriptorBufferInfo;

	std::vector<VkViewport> Viewports;
	std::vector<VkRect2D> ScissorRects;

	std::vector<VkDescriptorSetLayout> DescriptorSetLayouts;
	std::vector<VkDescriptorPool> DescriptorPools;
	std::vector<VkDescriptorSet> DescriptorSets;
	std::vector< VkPipelineLayout> PipelineLayouts;

	VkPipeline Pipeline = VK_NULL_HANDLE;
	VkPipelineCache PipelineCache = VK_NULL_HANDLE;
	std::vector<VkRenderPass> RenderPasses;
	std::vector<VkFramebuffer> Framebuffers;

	std::vector<VkShaderModule> ShaderModules;

	//!< https://matthewwellings.com/blog/the-new-vulkan-coordinate-system/
	//!< VKのクリップスペースはYが反転、Zが半分 (Vulkan clip space has inverted Y and half Z)
	static glm::mat4 GetVulkanClipSpace() {
		return glm::mat4(1.0f, 0.0f, 0.0f, 0.0f,
				0.0f, -1.0f, 0.0f, 0.0f,
				0.0f, 0.0f, 0.5f, 0.0f,
				0.0f, 0.0f, 0.5f, 1.0f);
	}

	/**
	@note バーチャルフレームに持たせるもの #VK_TODO
	1 コマンドバッファ
	2 プレゼント完了セマフォ
	3 描画完了セマフォ
	4 フェンス
	5 フレームバッファ
	*/

	//!< よく使うやつ
	const VkComponentMapping ComponentMapping_Identity = { VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, };
	const VkComponentMapping ComponentMapping_RGBA = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A, };
	const VkComponentMapping ComponentMapping_BGRA = { VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_A, };
	const VkImageSubresourceRange ImageSubresourceRange_Color = {
		VK_IMAGE_ASPECT_COLOR_BIT,
		0, 1,
		0, 1
	};
	const VkImageSubresourceRange ImageSubresourceRange_DepthStencil = {
		VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT,
		0, 1,
		0, 1
	};
	//!< 全てのミップマップ(レイヤー)を使用したい場合は、正確な値を知らなくても VK_REMAINING_MIP_LEVELS(VK_REMAINING_ARRAY_LAYERS) を指定すれば良い
	//!< If want to use all miplevels(layer), we can use VK_REMAINING_MIP_LEVELS(VK_REMAINING_ARRAY_LAYERS) without knowing the exact number
	const VkImageSubresourceRange ImageSubresourceRange_ColorAll = {
		VK_IMAGE_ASPECT_COLOR_BIT,
		0, VK_REMAINING_MIP_LEVELS,
		0, VK_REMAINING_ARRAY_LAYERS
	};
	const VkImageSubresourceRange ImageSubresourceRange_DepthStencilAll = {
		VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT,
		0, VK_REMAINING_MIP_LEVELS,
		0, VK_REMAINING_ARRAY_LAYERS
	};
	const VkClearDepthStencilValue ClearDepthStencilValue = { 1.0f, 0 };

	const VkShaderModule NullShaderModule = static_cast<const VkShaderModule>(VK_NULL_HANDLE);
};