#pragma once

//!< VK_NO_PROTOYYPES が定義されてる場合は DLL を使用する。If VK_NO_PROTOYYPES is defined, using DLL. 
//!< C/C++ - Preprocessor - Preprocessor Definitions に定義。 Definition is in C/C++ - Preprocessor - Preprocessor Definitions

#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
//#include <glm/gtc/matrix_transform.hpp>

#ifndef BREAK_ON_FAILED
#define BREAK_ON_FAILED(vr) if(VK_SUCCESS != (vr)) { DEBUG_BREAK(); }
#endif
#ifndef THROW_ON_FAILED
#define THROW_ON_FAILED(vr) if(VK_SUCCESS != (vr)) { throw std::runtime_error("VERIFY_SUCCEEDED failed : " + VK::GetVkResultString(vr)); }
#endif
#ifndef MESSAGEBOX_ON_FAILED
#define MESSAGEBOX_ON_FAILED(vr) if(VK_SUCCESS != (vr)) { Win::ShowMessageBoxW(nullptr, VK::GetVkResultStringW(vr)); }
#endif
#include "Win.h"

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
	//virtual void OnTimer(HWND hWnd, HINSTANCE hInstance) override { Super::OnTimer(hWnd, hInstance); }
	virtual void OnPaint(HWND hWnd, HINSTANCE hInstance) override { Super::OnPaint(hWnd, hInstance); Draw(); }
	virtual void OnDestroy(HWND hWnd, HINSTANCE hInstance) override;

	static std::string GetVkResultString(const VkResult Result);
	static std::wstring GetVkResultStringW(const VkResult Result);
	static std::string GetFormatString(const VkFormat Format);
	static std::string GetColorSpaceString(const VkColorSpaceKHR ColorSpace);
	static std::string GetImageViewTypeString(const VkImageViewType ImageViewType);
	static std::string GetComponentSwizzleString(const VkComponentSwizzle ComponentSwizzle);
	static std::string GetComponentMappingString(const VkComponentMapping& ComponentMapping) {
		return GetComponentSwizzleString(ComponentMapping.r) 
			+ ", " + GetComponentSwizzleString(ComponentMapping.g) 
			+ ", " + GetComponentSwizzleString(ComponentMapping.b)
			+ ", " + GetComponentSwizzleString(ComponentMapping.a);
	}

protected:
	static FORCEINLINE void* AlignedMalloc(void* pUserData, size_t size, size_t alignment, VkSystemAllocationScope allocationScope) { return _aligned_malloc(size, alignment); }
	static FORCEINLINE void* AlignedRealloc(void* pUserData, void* pOriginal, size_t size, size_t alignment, VkSystemAllocationScope allocationScope) { return _aligned_realloc(pOriginal, size, alignment); }
	static FORCEINLINE void AlignedFree(void* pUserData, void* pMemory) { _aligned_free(pMemory); }
	static void AlignedAllocNotify(void* pUserData, size_t size, VkInternalAllocationType allocationType, VkSystemAllocationScope allocationScope) {}
	static void AligendFreeNotify(void* pUserData, size_t size, VkInternalAllocationType allocationType, VkSystemAllocationScope allocationScope) {}

	static bool HasExtension(const VkPhysicalDevice PhysicalDevice, const char* ExtensionName);
	static VkFormat GetSupportedDepthFormat(VkPhysicalDevice PhysicalDevice);
	static uint32_t GetMemoryType(const VkPhysicalDeviceMemoryProperties& PhysicalDeviceMemoryProperties, const uint32_t MemoryTypeBits, const VkFlags Properties);
	virtual FORCEINLINE VkFormat GetSupportedDepthFormat() const { return GetSupportedDepthFormat(PhysicalDevice); }
	virtual FORCEINLINE uint32_t GetMemoryType(const uint32_t MemoryTypeBits, const VkFlags Properties) const { return GetMemoryType(PhysicalDeviceMemoryProperties, MemoryTypeBits, Properties); }
	//static VkAccessFlags GetSrcAccessMask(VkImageLayout OldImageLayout, VkImageLayout NewImageLayout);
	//static VkAccessFlags GetDstAccessMask(VkImageLayout OldImageLayout, VkImageLayout NewImageLayout);
	//void SetImageLayout(VkCommandBuffer CommandBuffer, VkImage Image, VkImageLayout OldImageLayout, VkImageLayout NewImageLayout, VkImageSubresourceRange ImageSubresourceRange) const;
	//virtual void MemoryBarrier() {}
	//virtual void BufferMemoryBarrier(const VkCommandBuffer CommandBuffer, const VkBuffer Buffer) {}
	//virtual void ImageMemoryBarrier(const VkCommandBuffer CommandBuffer, const VkImage Image) {}

	virtual void CreateBuffer(VkBuffer* Buffer, const VkBufferUsageFlags Usage, const size_t Size) const;
	virtual void CreateImage(VkImage* Image, const VkImageUsageFlags Usage, const VkSampleCountFlagBits SampleCount, const VkImageType ImageType, const VkFormat Format, const VkExtent3D& Extent3D, const uint32_t MipLevels, const uint32_t ArrayLayers) const;
	
	virtual void CopyToHostVisibleMemory(const VkDeviceMemory DeviceMemory, const size_t Size, const void* Source, const VkDeviceSize Offset = 0);
	
	virtual void SubmitCopyBuffer(const VkCommandBuffer CommandBuffer, const VkBuffer SrcBuffer, const VkBuffer DstBuffer, const VkAccessFlags AccessFlag, const VkPipelineStageFlagBits PipelineStageFlag, const size_t Size);
	
	template<typename T> void CreateHostVisibleMemory(VkDeviceMemory* DeviceMemory, const T Object) { DEBUG_BREAK(); /* テンプレート特殊化されていない、実装すること */ }
	template<typename T> void CreateDeviceLocalMemory(VkDeviceMemory* DeviceMemory, const T Object) { DEBUG_BREAK(); /* テンプレート特殊化されていない、実装すること */ }
	
	template<typename T> void BindDeviceMemory(const T Object, const VkDeviceMemory DeviceMemory, const VkDeviceSize Offset = 0) { DEBUG_BREAK(); /* テンプレート特殊化されていない、実装すること */ }
	//!< ↓ ここでテンプレート特殊化している Template specialization here
#include "VKDeviceMemory.inl"

	virtual void CreateImageView(VkImageView* ImageView, const VkImage Image, const VkImageViewType ImageViewType, const VkFormat Format, const VkComponentMapping& ComponentMapping, const VkImageSubresourceRange& ImageSubresourceRange);
	virtual void CreateBufferView(VkBufferView* BufferView, const VkBuffer Buffer, const VkFormat Format, const VkDeviceSize Offset = 0, const VkDeviceSize Range = VK_WHOLE_SIZE);
	
	virtual void ValidateFormatProperties(const VkImageUsageFlags Usage, const VkFormat Format) const;

#ifdef _DEBUG
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
#endif
#ifdef _DEBUG
	static void MarkerInsert(VkCommandBuffer CommandBuffer, const char* Name, const glm::vec4& Color);
	static void MarkerInsert(VkCommandBuffer CommandBuffer, const std::string& Name, const glm::vec4& Color) { MarkerInsert(CommandBuffer, Name.c_str(), Color); }
	static void MarkerInsert(VkCommandBuffer CommandBuffer, const std::wstring& Name, const glm::vec4& Color) { MarkerInsert(CommandBuffer, std::string(Name.begin(), Name.end()), Color); }
	static void MarkerBegin(VkCommandBuffer CommandBuffer, const char* Name, const glm::vec4& Color);
	static void MarkerBegin(VkCommandBuffer CommandBuffer, const std::string& Name, const glm::vec4& Color) { MarkerBegin(CommandBuffer, Name.c_str(), Color); }
	static void MarkerBegin(VkCommandBuffer CommandBuffer, const std::wstring& Name, const glm::vec4& Color) { MarkerBegin(CommandBuffer, std::string(Name.begin(), Name.end()), Color); }
	static void MarkerEnd(VkCommandBuffer CommandBuffer);
	template<typename T> static void MarkerSetObjectName(VkDevice Device, T Object, const char* Name) { DEBUG_BREAK(); /* テンプレート特殊化されていない Not template specialized */ }
	template<typename T> static void MarkerSetObjectName(VkDevice Device, T Object, const std::string& Name) { MarkerSetObjectName(Device, Name.c_str()); }
	template<typename T> static void MarkerSetObjectName(VkDevice Device, T Object, const std::wstring& Name) { MarkerSetObjectName(Device, std::string(Name.begin(), Name.end())); }
	template<typename T> static void MarkerSetObjectTag(VkDevice Device, T Object, const uint64_t TagName, const size_t TagSize, const void* Tag) { DEBUG_BREAK(); /* テンプレート特殊化されていない Not template specialized */ }
	//!< ↓ここでテンプレート特殊化している Template specialization here
#include "VKDebugMarker.inl"
#endif

	virtual void EnumerateInstanceLayer();
	virtual void EnumerateInstanceExtenstion(const char* layerName);
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
	void LoadVulkanDLL();
#endif //!< VK_NO_PROTOYYPES
	
	virtual void CreateInstance();
#ifdef _DEBUG
	virtual void CreateDebugReportCallback();
#endif
	virtual void CreateSurface(HWND hWnd, HINSTANCE hInstance);

	virtual void EnumeratePhysicalDeviceMemoryProperties(const VkPhysicalDeviceMemoryProperties& PhysicalDeviceMemoryProperties);
	virtual void GetPhysicalDevice();
	virtual void EnumerateDeviceLayer(VkPhysicalDevice PhysicalDevice);
	virtual void EnumerateDeviceExtenstion(VkPhysicalDevice PhysicalDevice, const char* layerName);
	virtual void GetQueueFamily();
	virtual void CreateDevice();

	virtual void CreateFence();
	virtual void CreateSemaphore();

	virtual void CreateCommandPool(const uint32_t QueueFamilyIndex);
	virtual void AllocateCommandBuffer(const VkCommandPool CommandPool, const size_t Count, const VkCommandBufferLevel CommandBufferLevel = VK_COMMAND_BUFFER_LEVEL_PRIMARY);

	virtual void CreateSwapchain();
	virtual VkSurfaceFormatKHR SelectSurfaceFormat();
	virtual VkPresentModeKHR SelectSurfacePresentMode();
	virtual void CreateSwapchain(const uint32_t Width, const uint32_t Height);
	virtual void CreateSwapchainOfClientRect() {
		CreateSwapchain(static_cast<uint32_t>(GetClientRectWidth()), static_cast<uint32_t>(GetClientRectHeight()));
	}
	virtual void CreateSwapchainImageView();
	virtual void InitializeSwapchainImage(const VkCommandBuffer CommandBuffer, const VkClearColorValue* ClearColorValue = nullptr);
	virtual void ResizeSwapchain(const uint32_t Width, const uint32_t Height);
	virtual void ResizeSwapChainToClientRect() {
		ResizeSwapchain(static_cast<const uint32_t>(GetClientRectWidth()), static_cast<const uint32_t>(GetClientRectHeight()));
	}

	virtual void CreateDepthStencil() { /*CreateDepthStencilImage();CreateDepthStencilDeviceMemory();CreateDepthStencilView();*/ }
	virtual void CreateDepthStencilImage();
	virtual void CreateDepthStencilDeviceMemory();
	virtual void CreateDepthStencilView() {
		CreateImageView(&DepthStencilImageView, DepthStencilImage, VK_IMAGE_VIEW_TYPE_2D, DepthFormat, ComponentMapping_Identity, ImageSubresourceRange_DepthStencil);
	}
	
	virtual void LoadImage(VkImage* Image, VkDeviceMemory *DeviceMemory, VkImageView* ImageView, const std::string& Path) {}
	virtual void LoadImage(VkImage* Image, VkDeviceMemory *DeviceMemory, VkImageView* ImageView, const std::wstring& Path) { LoadImage(Image, DeviceMemory, ImageView, std::string(Path.begin(), Path.end())); }
	virtual void CreateViewport(const float Width, const float Height, const float MinDepth = 0.0f, const float MaxDepth = 1.0f);
	virtual void CreateViewportTopFront(const float Width, const float Height) { CreateViewport(Width, Height, 0.0f, 0.0f); }

	virtual void CreateVertexBuffer() {}
	virtual void CreateIndexBuffer() {}
	virtual void CreateIndirectBuffer() {}
	virtual void CreateUniformBuffer();
	virtual void CreateStorageBuffer();
	virtual void CreateUniformTexelBuffer();
	virtual void CreateStorageTexelBuffer();

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
	virtual void DestroyFramebuffer();

	virtual VkShaderModule CreateShaderModule(const std::wstring& Path) const;
	virtual void CreateShader(std::vector<VkShaderModule>& ShaderModules, std::vector<VkPipelineShaderStageCreateInfo>& PipelineShaderStageCreateInfos) const {}
	virtual void CreateVertexInput(std::vector<VkVertexInputBindingDescription>& VertexInputBindingDescriptions, std::vector<VkVertexInputAttributeDescription>& VertexInputAttributeDescriptions, const uint32_t Binding = 0) const {}
	virtual void CreateInputAssembly(VkPipelineInputAssemblyStateCreateInfo& PipelineInputAssemblyStateCreateInfo) const;
	virtual void CreatePipeline() {}
	virtual VkPipelineCache LoadPipelineCache(const std::wstring& Path) const;
	virtual void StorePipelineCache(const std::wstring& Path, const VkPipelineCache PipelineCache) const;
	virtual VkPipelineCache CreatePipelineCache();
	virtual void CreateGraphicsPipeline();
	virtual void CreateComputePipeline();

	virtual void ClearColor(const VkCommandBuffer CommandBuffer, const VkImage Image, const VkClearColorValue& Color);
	virtual void PopulateCommandBuffer(const VkCommandBuffer CommandBuffer, const VkFramebuffer Framebuffer);
	virtual void PopulateCommandBuffer(const VkCommandBuffer CommandBuffer, const VkFramebuffer Framebuffer, const VkImage Image, const VkClearColorValue& Color);

	virtual void Draw();
	virtual void Present();
	virtual void WaitForFence();

	const VkAllocationCallbacks AllocationCallbacks = {
		nullptr,
		AlignedMalloc,
		AlignedRealloc,
		AlignedFree,
		AlignedAllocNotify,
		AligendFreeNotify
	};
	const VkAllocationCallbacks* GetAllocationCallbacks() const { return nullptr/*&AllocationCallbacks*/; }

#ifdef VK_NO_PROTOYYPES
protected:
	HMODULE VulkanDLL = nullptr;
public:
#define VK_GLOBAL_PROC_ADDR(proc) static PFN_vk ## proc vk ## proc;
#include "VKGlobalProcAddr.h"
#undef VK_GLOBAL_PROC_ADDR
#define VK_INSTANCE_PROC_ADDR(proc) static PFN_vk ## proc vk ## proc;
#include "VKInstanceProcAddr.h"
#undef VK_INSTANCE_PROC_ADDR
#define VK_DEVICE_PROC_ADDR(proc) static PFN_vk ## proc vk ## proc;
#include "VKDeviceProcAddr.h"
#undef VK_DEVICE_PROC_ADDR
#endif //!< VK_NO_PROTOYYPES

#ifdef _DEBUG
public:
#define VK_INSTANCE_PROC_ADDR(proc) static PFN_vk ## proc ## EXT vk ## proc;
#include "VKDebugReport.h"
#undef VK_INSTANCE_PROC_ADDR
#define VK_DEVICE_PROC_ADDR(proc) static PFN_vk ## proc ## EXT vk ## proc;
#include "VKDebugMarker.h"
#undef VK_DEVICE_PROC_ADDR
protected:
#endif //!< _DEBUG

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

	VkFence Fence = VK_NULL_HANDLE;
	VkSemaphore NextImageAcquiredSemaphore = VK_NULL_HANDLE;	//!< プレゼント完了までウエイト
	VkSemaphore RenderFinishedSemaphore = VK_NULL_HANDLE;		//!< 描画完了するまでウエイト

	std::vector<VkCommandPool> CommandPools;
	std::vector<VkCommandBuffer> CommandBuffers;

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

	VkBuffer IndirectBuffer = VK_NULL_HANDLE;
	VkDeviceMemory IndirectDeviceMemory = VK_NULL_HANDLE;

	//!< 現状1つのみ、配列にする #VK_TODO
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

	/**
	@note バーチャルフレームに持たせるもの #TODO
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

	/**
	@brief VkCommandBufferUsageFlags 
	* VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT		... 一度だけ使用する場合、毎回リセットする場合に指定、何度もサブミットするものには指定しない
	* VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT		... 前回のサブミットが完了してなくても、再度サブミットする場合に指定、パフォーマンス的に避けるべき
	* VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT	... セカンダリコマンドバッファでかつ、完全にレンダーパス内の場合に指定する

	@brief セカンダリコマンドバッファ
	* 基本的に、セカンダリ(コマンドバッファ)はプライマリ(コマンドバッファ)のステートを継承しない
	* セカンダリ記録後のプライマリのステートも未定義、プライマリに戻って再記録する場合はステートを再設定しなくてはならない
	* 例外) プライマリがレンダーパス内でそこからセカンダリを呼び出す場合には、プライマリのレンダーパス、サプバスステートは継承される
	* 全てのコマンドがプライマリ、セカンダリの両方で記録できるわけではない

	* セカンダリは直接サブミットできない、プライマリから呼び出される
		* vkCmdExecuteCommands(プライマリ, セカンダリ個数, セカンダリ配列);
	* セカンダリの場合は VK_SUBPASS_CONTENTS_INLINE の代わりに VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS を指定する
		* vkCmdBeginRenderPass(..., VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);
		* vkCmdNextSubpass(..., VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);
	*/
	const VkCommandBufferBeginInfo CommandBufferBeginInfo_OneTime = {
		VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		nullptr,
		VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
		nullptr
	};
	const VkCommandBufferBeginInfo CommandBufferBeginInfo = {
		VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		nullptr, 
		0,
		nullptr/*&CommandBufferInheritanceInfo_None*/ //!< セカンダリコマンドバッファの場合に使用
	};
};