#pragma once

#define VK_USE_PLATFORM_WIN32_KHR

#include <vulkan/vulkan.h>
#ifdef VK_ONLINE_COMPILE
//!< GLSL をオンラインコンパイルする場合
#include <../glslang/SPIRV/GlslangToSpv.h>
#endif

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
//#include <glm/gtc/matrix_transform.hpp>

#include "Win.h"

#ifndef BREAK_ON_FAILED
#define BREAK_ON_FAILED(vr) if(VK_SUCCESS != (vr)) { DEBUG_BREAK(); }
#endif
#ifndef THROW_ON_FAILED
#define THROW_ON_FAILED(vr) if(VK_SUCCESS != (vr)) { throw std::runtime_error("VERIFY_SUCCEEDED failed : " + VK::GetVkResultString(vr)); }
#endif
#ifndef MESSAGEBOX_ON_FAILED
#define MESSAGEBOX_ON_FAILED(vr) if(VK_SUCCESS != (vr)) { Win::ShowMessageBoxW(nullptr, VK::GetVkResultStringW(vr)); }
#endif
#ifndef VERIFY_SUCCEEDED
#define VERIFY_SUCCEEDED(vr) BREAK_ON_FAILED(vr)
//#define VERIFY_SUCCEEDED(vr) THROW_ON_FAILED(vr)
//#define VERIFY_SUCCEEDED(vr) MESSAGEBOX_ON_FAILED(vr)
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

class VK : public Win
{
private:
	using Super = Win;

public:
	virtual void OnCreate(HWND hWnd, HINSTANCE hInstance, LPCWSTR Title) override;
	virtual void OnSize(HWND hWnd, HINSTANCE hInstance) override;
	virtual void OnTimer(HWND hWnd, HINSTANCE hInstance) override;
	virtual void OnPaint(HWND hWnd, HINSTANCE hInstance) override;
	virtual void OnDestroy(HWND hWnd, HINSTANCE hInstance) override;

	static std::string GetVkResultString(const VkResult Result);
	static std::wstring GetVkResultStringW(const VkResult Result);
	static std::string GetFormatString(const VkFormat Format);

protected:
	static FORCEINLINE void* AlignedMalloc(void* pUserData, size_t size, size_t alignment, VkSystemAllocationScope allocationScope) { 
		return _aligned_malloc(size, alignment); 
	}
	static FORCEINLINE void* AlignedRealloc(void* pUserData, void* pOriginal, size_t size, size_t alignment, VkSystemAllocationScope allocationScope) {
		return _aligned_realloc(pOriginal, size, alignment);
	}
	static FORCEINLINE void AlignedFree(void* pUserData, void* pMemory) {
		_aligned_free(pMemory); 
	}
	static VkFormat GetSupportedDepthFormat(VkPhysicalDevice PhysicalDevice);
	static uint32_t GetMemoryType(const VkPhysicalDeviceMemoryProperties& PhysicalDeviceMemoryProperties, const uint32_t MemoryTypeBits, const VkFlags Properties);
	virtual FORCEINLINE VkFormat GetSupportedDepthFormat() const { return GetSupportedDepthFormat(PhysicalDevice); }
	virtual FORCEINLINE uint32_t GetMemoryType(const uint32_t MemoryTypeBits, const VkFlags Properties) const { return GetMemoryType(PhysicalDeviceMemoryProperties, MemoryTypeBits, Properties); }
	//static VkAccessFlags GetSrcAccessMask(VkImageLayout OldImageLayout, VkImageLayout NewImageLayout);
	//static VkAccessFlags GetDstAccessMask(VkImageLayout OldImageLayout, VkImageLayout NewImageLayout);
	//void SetImageLayout(VkCommandBuffer CommandBuffer, VkImage Image, VkImageLayout OldImageLayout, VkImageLayout NewImageLayout, VkImageSubresourceRange ImageSubresourceRange) const;

	virtual void CreateBuffer(VkBuffer* Buffer, const VkBufferUsageFlags Usage, const size_t Size);
	virtual void SubmitCopyBuffer(const VkCommandBuffer CommandBuffer, const VkBuffer SrcBuffer, const VkBuffer DstBuffer, const VkAccessFlags AccessFlag, const VkPipelineStageFlagBits PipelineStageFlag, const size_t Size);

	template<typename T> void CreateHostVisibleMemory(VkDeviceMemory* DeviceMemory, const T Object, const size_t Size = 0, const void* Source = nullptr) { DEBUG_BREAK(); /* テンプレート特殊化されていない、実装すること */ }
	template<> void CreateHostVisibleMemory(VkDeviceMemory* DeviceMemory, const VkBuffer Object, const size_t Size, const void* Source) {
		VkMemoryRequirements MemoryRequirements;
		vkGetBufferMemoryRequirements(Device, Object, &MemoryRequirements);
		const VkMemoryAllocateInfo MemoryAllocateInfo = {
			VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
			nullptr,
			MemoryRequirements.size,
			GetMemoryType(MemoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT/*| VK_MEMORY_PROPERTY_HOST_COHERENT_BIT*/) //!< HOST_VISIBLE にすること
		};
		VERIFY_SUCCEEDED(vkAllocateMemory(Device, &MemoryAllocateInfo, nullptr, DeviceMemory));
		VERIFY_SUCCEEDED(vkBindBufferMemory(Device, Object, *DeviceMemory, 0));
		if (Size && nullptr != Source) {
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
		}
	}

	template<typename T> void CreateDeviceLocalMemory(VkDeviceMemory* DeviceMemory, const T Object) { DEBUG_BREAK(); /* テンプレート特殊化されていない、実装すること */ }
	template<> void CreateDeviceLocalMemory(VkDeviceMemory* DeviceMemory, const VkBuffer Object) {
		VkMemoryRequirements MemoryRequirements;
		vkGetBufferMemoryRequirements(Device, Object, &MemoryRequirements);
		const VkMemoryAllocateInfo MemoryAllocateInfo = {
			VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
			nullptr,
			MemoryRequirements.size,
			GetMemoryType(MemoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
		};
		VERIFY_SUCCEEDED(vkAllocateMemory(Device, &MemoryAllocateInfo, nullptr, DeviceMemory));
		VERIFY_SUCCEEDED(vkBindBufferMemory(Device, Object, *DeviceMemory, 0));
	}
	template<> void CreateDeviceLocalMemory(VkDeviceMemory* DeviceMemory, const VkImage Object) {
		VkMemoryRequirements MemoryRequirements;
		vkGetImageMemoryRequirements(Device, Object, &MemoryRequirements);
		const VkMemoryAllocateInfo MemoryAllocateInfo = {
			VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
			nullptr,
			MemoryRequirements.size,
			GetMemoryType(MemoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
		};
		VERIFY_SUCCEEDED(vkAllocateMemory(Device, &MemoryAllocateInfo, nullptr, DeviceMemory));
		VERIFY_SUCCEEDED(vkBindImageMemory(Device, Object, *DeviceMemory, 0));
	}

	void CreateStagingBufferAndMemory(VkBuffer* Buffer, VkDeviceMemory* DeviceMemory, const size_t Size, const void* Source = nullptr) {
		//!< 転送元のバッファを作成
		CreateBuffer(Buffer, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, Size);
		//!< ホストから可視のメモリを作成
		CreateHostVisibleMemory(DeviceMemory, *Buffer, Size, Source);
	}
	void CreateDeviceLocalBufferAndMemory(VkBuffer* Buffer, VkDeviceMemory* DeviceMemory, const VkBufferUsageFlags Usage, const size_t Size) {
		//!< 転送先のバッファを作成
		CreateBuffer(Buffer, Usage | VK_BUFFER_USAGE_TRANSFER_DST_BIT, Size);
		//!< デバイスローカルのメモリを作成
		CreateDeviceLocalMemory(DeviceMemory, *Buffer);
	}

	virtual void EnumerateInstanceLayer();
	virtual void EnumerateInstanceExtenstion(const char* layerName);
	virtual void CreateInstance();
	virtual void CreateDebugReportCallback();
	virtual void CreateSurface(HWND hWnd, HINSTANCE hInstance);

	virtual void EnumeratePhysicalDeviceMemoryProperties(const VkPhysicalDeviceMemoryProperties& PhysicalDeviceMemoryProperties);
	virtual void GetPhysicalDevice();
	virtual void EnumerateDeviceLayer(VkPhysicalDevice PhysicalDevice);
	virtual void EnumerateDeviceExtenstion(VkPhysicalDevice PhysicalDevice, const char* layerName);
	virtual void GetQueueFamily();
	virtual void CreateDevice();
	virtual void CreateDebugMarker();

	virtual void CreateCommandPool(const uint32_t QueueFamilyIndex);
	virtual void AllocateCommandBuffer(const VkCommandPool CommandPool, const VkCommandBufferLevel CommandBufferLevel = VK_COMMAND_BUFFER_LEVEL_PRIMARY);

	virtual void CreateFence();
	virtual void CreateSemaphore();

	virtual VkSurfaceFormatKHR SelectSurfaceFormat();
	virtual VkPresentModeKHR SelectSurfacePresentMode();
	virtual void CreateSwapchain(const uint32_t Width, const uint32_t Height);
	virtual void CreateSwapchainOfClientRect() {
		CreateSwapchain(static_cast<uint32_t>(GetClientRectWidth()), static_cast<uint32_t>(GetClientRectHeight()));
	}
	virtual void GetSwapchainImage(const VkCommandBuffer CommandBuffer);
	virtual void GetSwapchainImage(const VkCommandBuffer CommandBuffer, const VkClearColorValue& ClearColorValue);
	virtual void CreateSwapchainImageView();
	virtual void CreateSwapchain(const VkCommandBuffer CommandBuffer);
	virtual void ResizeSwapchain(const uint32_t Width, const uint32_t Height);
	virtual void ResizeSwapChainToClientRect() {
		ResizeSwapchain(static_cast<const uint32_t>(GetClientRectWidth()), static_cast<const uint32_t>(GetClientRectHeight()));
	}

	virtual void CreateDepthStencilImage();
	virtual void CreateDepthStencilDeviceMemory();
	virtual void CreateDepthStencilView();
	virtual void CreateDepthStencil(const VkCommandBuffer CommandBuffer) { /*CreateDepthStencilImage();CreateDepthStencilDeviceMemory();CreateDepthStencilView(CommandBuffer);*/ }
	
	virtual void LoadImage(VkImage* Image, VkDeviceMemory *DeviceMemory, VkImageView* ImageView, const std::string& Path) {}
	virtual void LoadImage(VkImage* Image, VkDeviceMemory *DeviceMemory, VkImageView* ImageView, const std::wstring& Path) { LoadImage(Image, DeviceMemory, ImageView, std::string(Path.begin(), Path.end())); }
	virtual void CreateTextureView(VkImageView* ImageView, const VkImage Image, const VkImageViewType ImageViewType, const VkFormat Format, const VkComponentMapping& ComponentMapping);

	virtual void CreateViewport(const float Width, const float Height, const float MinDepth = 0.0f, const float MaxDepth = 1.0f);
	virtual void CreateViewportTopFront(const float Width, const float Height) { CreateViewport(Width, Height, 0.0f, 0.0f); }

	virtual void CreateVertexBuffer(const VkCommandBuffer CommandBuffer);
	virtual void CreateIndexBuffer(const VkCommandBuffer CommandBuffer);
	virtual void CreateUniformBuffer();

	virtual void CreateDescriptorSetLayoutBindings(std::vector<VkDescriptorSetLayoutBinding>& DescriptorSetLayoutBindings) const {}
	virtual void CreateDescriptorSetLayout();

	virtual void CreateDescriptorPoolSizes(std::vector<VkDescriptorPoolSize>& DescriptorPoolSizes) const {}
	virtual void CreateDescriptorSet();

	virtual void CreateWriteDescriptorSets(std::vector<VkWriteDescriptorSet>& WriteDescriptorSets, VkDescriptorImageInfo* DescriptorImageInfo, VkDescriptorBufferInfo* DescriptorBufferInfo, VkBufferView* BufferView) const {}
	virtual void CreateCopyDescriptorSets(std::vector<VkCopyDescriptorSet>& CopyDescriptorSets) const {}
	virtual void UpdateDescriptorSet();

	virtual void CreateTexture() {}
	virtual void CreateSampler(const float MaxLOD = (std::numeric_limits<float>::max)()) {}

	virtual void CreateRenderPass() {}
	virtual void CreateFramebuffer() {}

	virtual VkShaderModule CreateShaderModule(const std::wstring& Path) const;
	virtual void CreateShader(std::vector<VkShaderModule>& ShaderModules, std::vector<VkPipelineShaderStageCreateInfo>& PipelineShaderStageCreateInfos) const {}
	virtual void CreateVertexInput(std::vector<VkVertexInputBindingDescription>& VertexInputBindingDescriptions, std::vector<VkVertexInputAttributeDescription>& VertexInputAttributeDescriptions, const uint32_t Binding = 0) const {}
	virtual void CreatePipeline() {}
	virtual VkPipelineCache LoadPipelineCache(const std::wstring& Path) const;
	virtual void StorePipelineCache(const std::wstring& Path, const VkPipelineCache PipelineCache) const;
	virtual void CreateGraphicsPipeline();
	virtual void CreateComputePipeline();

	virtual void PopulateCommandBuffer(const VkCommandBuffer CommandBuffer);

	virtual void Draw();
	virtual void Present();
	virtual void WaitForFence();

	const VkAllocationCallbacks AllocationCallbacks = {
		nullptr,
		AlignedMalloc,
		AlignedRealloc,
		AlignedFree,
		nullptr,
		nullptr
	};
protected:
	VkInstance Instance = VK_NULL_HANDLE;
#ifdef _DEBUG
	VkDebugReportCallbackEXT DebugReportCallback = VK_NULL_HANDLE;
#endif
	VkSurfaceKHR Surface = VK_NULL_HANDLE;
	VkPhysicalDevice PhysicalDevice = VK_NULL_HANDLE;
	VkPhysicalDeviceMemoryProperties PhysicalDeviceMemoryProperties;
	VkDevice Device = VK_NULL_HANDLE;
	VkQueue GraphicsQueue = VK_NULL_HANDLE;
	VkQueue PresentQueue = VK_NULL_HANDLE;
	uint32_t GraphicsQueueFamilyIndex = UINT32_MAX;
	uint32_t PresentQueueFamilyIndex = UINT32_MAX;
	//uint32_t TransferQueueFamilyIndex = UINT_MAX;
	//uint32_t ComputeQueueFamilyIndex = UINT_MAX;

	std::vector<VkCommandPool> CommandPools;
	std::vector<VkCommandBuffer> CommandBuffers;

	VkFence Fence = VK_NULL_HANDLE;
	VkSemaphore NextImageAcquiredSemaphore = VK_NULL_HANDLE;
	VkSemaphore RenderFinishedSemaphore = VK_NULL_HANDLE;

	VkExtent2D SurfaceExtent2D;
	VkFormat ColorFormat = VK_FORMAT_B8G8R8A8_UNORM;
	VkSwapchainKHR Swapchain = VK_NULL_HANDLE;
	std::vector<VkImage> SwapchainImages;
	std::vector<VkImageView> SwapchainImageViews;
	uint32_t SwapchainImageIndex = 0;

	VkFormat DepthFormat = VK_FORMAT_D24_UNORM_S8_UINT;
	VkImage DepthStencilImage = VK_NULL_HANDLE;
	VkDeviceMemory DepthStencilDeviceMemory = VK_NULL_HANDLE;
	VkImageView DepthStencilImageView = VK_NULL_HANDLE;

	VkImage Image = VK_NULL_HANDLE;
	VkDeviceMemory ImageDeviceMemory = VK_NULL_HANDLE;
	VkImageView ImageView = VK_NULL_HANDLE;
	VkSampler Sampler = VK_NULL_HANDLE;

	VkBuffer VertexBuffer = VK_NULL_HANDLE;
	VkDeviceMemory VertexDeviceMemory = VK_NULL_HANDLE;

	VkBuffer IndexBuffer = VK_NULL_HANDLE;
	VkDeviceMemory IndexDeviceMemory = VK_NULL_HANDLE;
	uint32_t IndexCount = 0;

	//!< #TODO 現状1つのみ、配列にする
	VkBuffer UniformBuffer = VK_NULL_HANDLE;
	VkDeviceMemory UniformDeviceMemory = VK_NULL_HANDLE;
	VkDescriptorBufferInfo UniformDescriptorBufferInfo;

	std::vector<VkViewport> Viewports;
	std::vector<VkRect2D> ScissorRects;

	//!< デスクリプタセット、パイプラインレイアウト作成時に必要になるのでメンバとする
	std::vector<VkDescriptorSetLayout> DescriptorSetLayouts;
	VkDescriptorPool DescriptorPool = VK_NULL_HANDLE;
	std::vector<VkDescriptorSet> DescriptorSets;
	VkPipelineLayout PipelineLayout = VK_NULL_HANDLE;

	VkPipeline Pipeline = VK_NULL_HANDLE;
	VkRenderPass RenderPass = VK_NULL_HANDLE;
	std::vector<VkFramebuffer> Framebuffers;

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
	const VkClearDepthStencilValue ClearDepthStencilValue = { 1.0f, 0 };
	const VkCommandBufferBeginInfo CommandBufferBeginInfo_OneTime = {
		VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		nullptr,
		VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
		nullptr
	};
};

#ifdef _DEBUG
class DebugReport
{
public:
	//using DebugReport = std::function<VkBool32(VkDebugReportFlagsEXT, VkDebugReportObjectTypeEXT, uint64_t, size_t, int32_t, const char*, const char*, void*)>;

	static void GetInstanceProcAddr(VkInstance Instance);

	template<typename T>
	static void CreateDebugReportCallback(VkInstance Instance, T Callback, const VkDebugReportFlagsEXT Flags, VkDebugReportCallbackEXT* DebugReportCallback) {
		if (VK_NULL_HANDLE != vkCreateDebugReportCallback) {
			const VkDebugReportCallbackCreateInfoEXT DebugReportCallbackCreateInfo = {
				VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT,
				nullptr,
				Flags,
				Callback,
				nullptr
			};
			vkCreateDebugReportCallback(Instance, &DebugReportCallbackCreateInfo, nullptr, DebugReportCallback);
		}
	}
	static void DestroyDebugReportCallback(VkInstance Instance, VkDebugReportCallbackEXT DebugReportCallback);

#define VK_INSTANCE_PROC_ADDR(proc) static PFN_vk ## proc ## EXT vk ## proc;
#include "VKProcInstanceAddr.h"
#undef VK_INSTANCE_PROC_ADDR
};

class DebugMarker
{
public:
	static bool HasDebugMarkerExtension(VkPhysicalDevice PhysicalDevice);
	static void GetDeviceProcAddr(VkDevice Device);

	static void Insert(VkCommandBuffer CommandBuffer, const char* Name, const glm::vec4& Color);
	static void Insert(VkCommandBuffer CommandBuffer, const std::string& Name, const glm::vec4& Color) { Insert(CommandBuffer, Name.c_str(), Color); }
	static void Insert(VkCommandBuffer CommandBuffer, const std::wstring& Name, const glm::vec4& Color) { Insert(CommandBuffer, std::string(Name.begin(), Name.end()), Color); }
	
	static void Begin(VkCommandBuffer CommandBuffer, const char* Name, const glm::vec4& Color);
	static void Begin(VkCommandBuffer CommandBuffer, const std::string& Name, const glm::vec4& Color) { Begin(CommandBuffer, Name.c_str(), Color); }
	static void Begin(VkCommandBuffer CommandBuffer, const std::wstring& Name, const glm::vec4& Color) { Begin(CommandBuffer, std::string(Name.begin(), Name.end()), Color); }
	static void End(VkCommandBuffer CommandBuffer);

	template<typename T> static void SetName(VkDevice Device, T Object, const char* Name) { DEBUG_BREAK(); /* テンプレート特殊化されていない、VKDebugMarker.h に実装すること */ }
	template<typename T> static void SetName(VkDevice Device, T Object, const std::string& Name) { SetName(Device, Name.c_str()); }
	template<typename T> static void SetName(VkDevice Device, T Object, const std::wstring& Name) { SetName(Device, std::string(Name.begin(), Name.end())); }

	template<typename T> static void SetTag(VkDevice Device, T Object, const uint64_t TagName, const size_t TagSize, const void* Tag) { DEBUG_BREAK(); /* テンプレート特殊化されていない、VKDebugMarker.h に実装すること */ }
	
	//!< ↓ここでテンプレート特殊化している
#include "VKDebugMarker.h"

#define VK_DEVICE_PROC_ADDR(proc) static PFN_vk ## proc ## EXT vk ## proc;
#include "VKProcDeviceAddr.h"
#undef VK_DEVICE_PROC_ADDR
};
#endif