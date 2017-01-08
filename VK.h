#pragma once

#define VK_USE_PLATFORM_WIN32_KHR

#include <vulkan/vulkan.h>

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
	class Allocator
	{
	public:
		static FORCEINLINE void* AlignedMalloc(void* pUserData, size_t size, size_t alignment, VkSystemAllocationScope allocationScope) {
			return static_cast<Allocator*>(pUserData)->Malloc(size, alignment, allocationScope);
		}
		static FORCEINLINE void* AlignedRealloc(void* pUserData, void* pOriginal, size_t size, size_t alignment, VkSystemAllocationScope allocationScope) {
			return static_cast<Allocator*>(pUserData)->Realloc(pOriginal, size, alignment, allocationScope);
		}
		static FORCEINLINE void AlignedFree(void* pUserData, void* pMemory) {
			static_cast<Allocator*>(pUserData)->Free(pMemory);
		}
		static void AlignedAllocNotify(void* pUserData, size_t size, VkInternalAllocationType allocationType, VkSystemAllocationScope allocationScope) {}
		static void AligendFreeNotify(void* pUserData, size_t size, VkInternalAllocationType allocationType, VkSystemAllocationScope allocationScope) {}

		void* Malloc(size_t size, size_t alignment, VkSystemAllocationScope allocationScope) { return _aligned_malloc(size, alignment); }
		void* Realloc(void* pOriginal, size_t size, size_t alignment, VkSystemAllocationScope allocationScope) { return _aligned_realloc(pOriginal, size, alignment); }
		void* Free(void* pMemory) { _aligned_free(pMemory); }

		FORCEINLINE operator VkAllocationCallbacks() const {
			return VkAllocationCallbacks({
				reinterpret_cast<void*>(const_cast<Allocator*>(this)),
				AlignedMalloc,
				AlignedRealloc,
				AlignedFree,
				AlignedAllocNotify,
				AligendFreeNotify
			});
		}
	};
	static FORCEINLINE void* AlignedMalloc(void* pUserData, size_t size, size_t alignment, VkSystemAllocationScope allocationScope) { 
		return _aligned_malloc(size, alignment); 
	}
	static FORCEINLINE void* AlignedRealloc(void* pUserData, void* pOriginal, size_t size, size_t alignment, VkSystemAllocationScope allocationScope) {
		return _aligned_realloc(pOriginal, size, alignment);
	}
	static FORCEINLINE void AlignedFree(void* pUserData, void* pMemory) {
		_aligned_free(pMemory); 
	}
	static void AlignedAllocNotify(void* pUserData, size_t size, VkInternalAllocationType allocationType, VkSystemAllocationScope allocationScope) {}
	static void AligendFreeNotify(void* pUserData, size_t size, VkInternalAllocationType allocationType, VkSystemAllocationScope allocationScope) {}

	//!< https://software.intel.com/en-us/articles/api-without-secrets-introduction-to-vulkan-part-4 : Rendering Resources Creation
	class FrameResource
	{
	public:
		FrameResource(const FrameResource& rhs) = delete;
		FrameResource& operator=(const FrameResource& rhs) = delete;

		void Create(const VkDevice Device, const VkCommandPool CommandPool) {
			const VkCommandBufferAllocateInfo CommandBufferAllocateInfo = {
				VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
				nullptr,
				CommandPool,
				VK_COMMAND_BUFFER_LEVEL_PRIMARY,
				1
			};
			VERIFY_SUCCEEDED(vkAllocateCommandBuffers(Device, &CommandBufferAllocateInfo, &CommandBuffer));

			const VkSemaphoreCreateInfo SemaphoreCreateInfo = {
				VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
				nullptr,
				0
			};
			VERIFY_SUCCEEDED(vkCreateSemaphore(Device, &SemaphoreCreateInfo, nullptr, &NextImageAcquiredSemaphore));
			VERIFY_SUCCEEDED(vkCreateSemaphore(Device, &SemaphoreCreateInfo, nullptr, &RenderFinishedSemaphore));

			const VkFenceCreateInfo FenceCreateInfo = {
				VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
				nullptr,
				VK_FENCE_CREATE_SIGNALED_BIT
			};
			VERIFY_SUCCEEDED(vkCreateFence(Device, &FenceCreateInfo, nullptr, &Fence));
		}
		void Destroy(const VkDevice Device, const VkCommandPool CommandPool) {
			if (VK_NULL_HANDLE != CommandBuffer) {
				vkFreeCommandBuffers(Device, CommandPool, 1, &CommandBuffer);
				CommandBuffer = VK_NULL_HANDLE;
			}
			if (VK_NULL_HANDLE != NextImageAcquiredSemaphore) {
				vkDestroySemaphore(Device, NextImageAcquiredSemaphore, nullptr);
				NextImageAcquiredSemaphore = VK_NULL_HANDLE;
			}
			if (VK_NULL_HANDLE != RenderFinishedSemaphore) {
				vkDestroySemaphore(Device, RenderFinishedSemaphore, nullptr);
				RenderFinishedSemaphore = VK_NULL_HANDLE;
			}
			if (VK_NULL_HANDLE != Fence) {
				vkDestroyFence(Device, Fence, nullptr);
				Fence = VK_NULL_HANDLE;
			}
		}

		VkCommandBuffer CommandBuffer = VK_NULL_HANDLE;
		VkSemaphore NextImageAcquiredSemaphore = VK_NULL_HANDLE;	//!< �v���[���g�����܂ŃE�G�C�g
		VkSemaphore RenderFinishedSemaphore = VK_NULL_HANDLE;		//!< �`�抮������܂ŃE�G�C�g
		VkFence Fence = VK_NULL_HANDLE;
	};

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
	virtual void CreateImage(VkImage* Image, const VkImageUsageFlags Usage, const VkImageType ImageType, const VkFormat Format, const VkExtent3D& Extent3D, const uint32_t MipLevels, const uint32_t ArrayLayers) const;
	virtual void CopyToHostVisibleMemory(const VkBuffer Buffer, const VkDeviceMemory DeviceMemory, const size_t Size = 0, const void* Source = nullptr, const VkDeviceSize Offset = 0);
	virtual void SubmitCopyBuffer(const VkCommandBuffer CommandBuffer, const VkBuffer SrcBuffer, const VkBuffer DstBuffer, const VkAccessFlags AccessFlag, const VkPipelineStageFlagBits PipelineStageFlag, const size_t Size);

	template<typename T> void CreateHostVisibleMemory(VkDeviceMemory* DeviceMemory, const T Object) { DEBUG_BREAK(); /* �e���v���[�g���ꉻ����Ă��Ȃ��A�������邱�� */ }
	template<typename T> void CreateDeviceLocalMemory(VkDeviceMemory* DeviceMemory, const T Object) { DEBUG_BREAK(); /* �e���v���[�g���ꉻ����Ă��Ȃ��A�������邱�� */ }
	template<typename T> void BindDeviceMemory(const T Object, const VkDeviceMemory DeviceMemory, const VkDeviceSize Offset = 0) { DEBUG_BREAK(); /* �e���v���[�g���ꉻ����Ă��Ȃ��A�������邱�� */ }
	//!< �������Ńe���v���[�g���ꉻ���Ă���
#include "VKDeviceMemory.h"

	/**
	@brief �]���p�̃o�b�t�@�ƃ��������쐬���āA�������փf�[�^���R�s�[����A�o�b�t�@�ƃ��������o�C���h����
	@note �傫�ȃo�b�t�@��p�ӂ��āA�����o�b�t�@��(�I�t�Z�b�g����)�o�C���h���Ă����悤�ȗp�r�ɂ͎g���Ȃ��̂Œ��� (���񕪂Ƃ��Ă͉\)
	*/
	void CreateStagingBufferAndCopyToMemory(VkBuffer* Buffer, VkDeviceMemory* DeviceMemory, const size_t Size, const void* Source = nullptr, const VkDeviceSize Offset = 0);
	/**
	@brief �f�o�C�X���[�J���ȃo�b�t�@�ƃ��������쐬�A�o�b�t�@�ƃ��������o�C���h����
	@note �傫�ȃo�b�t�@��p�ӂ��āA�����o�b�t�@��(�I�t�Z�b�g����)�o�C���h���Ă����悤�ȗp�r�ɂ͎g���Ȃ��̂Œ��� (���񕪂Ƃ��Ă͉\)
	*/
	void CreateDeviceLocalBuffer(VkBuffer* Buffer, VkDeviceMemory* DeviceMemory, const VkBufferUsageFlags Usage, const size_t Size, const VkDeviceSize Offset = 0);

	virtual void CreateImageView(VkImageView* ImageView, const VkImage Image, const VkImageViewType ImageViewType, const VkFormat Format, const VkComponentMapping& ComponentMapping, const VkImageSubresourceRange& ImageSubresourceRange);

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
	virtual void CreateDepthStencilView() {
		CreateImageView(&DepthStencilImageView, DepthStencilImage, VK_IMAGE_VIEW_TYPE_2D, DepthFormat, ComponentMapping_Identity, ImageSubresourceRange_DepthStencil);
	}
	virtual void CreateDepthStencil(const VkCommandBuffer CommandBuffer) { /*CreateDepthStencilImage();CreateDepthStencilDeviceMemory();CreateDepthStencilView();*/ }
	
	virtual void LoadImage(VkImage* Image, VkDeviceMemory *DeviceMemory, VkImageView* ImageView, const std::string& Path) {}
	virtual void LoadImage(VkImage* Image, VkDeviceMemory *DeviceMemory, VkImageView* ImageView, const std::wstring& Path) { LoadImage(Image, DeviceMemory, ImageView, std::string(Path.begin(), Path.end())); }
	virtual void CreateViewport(const float Width, const float Height, const float MinDepth = 0.0f, const float MaxDepth = 1.0f);
	virtual void CreateViewportTopFront(const float Width, const float Height) { CreateViewport(Width, Height, 0.0f, 0.0f); }

	virtual void CreateVertexBuffer(const VkCommandBuffer CommandBuffer) {}
	virtual void CreateIndexBuffer(const VkCommandBuffer CommandBuffer) {}
	virtual void CreateIndirectBuffer(const VkCommandBuffer CommandBuffer) {}
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
	virtual void DestroyFramebuffer();

	virtual VkShaderModule CreateShaderModule(const std::wstring& Path) const;
	virtual void CreateShader(std::vector<VkShaderModule>& ShaderModules, std::vector<VkPipelineShaderStageCreateInfo>& PipelineShaderStageCreateInfos) const {}
	virtual void CreateVertexInput(std::vector<VkVertexInputBindingDescription>& VertexInputBindingDescriptions, std::vector<VkVertexInputAttributeDescription>& VertexInputAttributeDescriptions, const uint32_t Binding = 0) const {}
	virtual void CreateInputAssembly(VkPipelineInputAssemblyStateCreateInfo& PipelineInputAssemblyStateCreateInfo) const {}
	virtual void CreatePipeline() {}
	virtual VkPipelineCache LoadPipelineCache(const std::wstring& Path) const;
	virtual void StorePipelineCache(const std::wstring& Path, const VkPipelineCache PipelineCache) const;
	virtual VkPipelineCache CreatePipelineCache();
	virtual void CreateGraphicsPipeline();
	virtual void CreateComputePipeline();

	virtual void ClearColor(const VkCommandBuffer CommandBuffer, const VkImage Image, const VkClearColorValue& Color);
	virtual void PopulateCommandBuffer(const VkCommandBuffer CommandBuffer);

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
	VkSemaphore NextImageAcquiredSemaphore = VK_NULL_HANDLE;	//!< �v���[���g�����܂ŃE�G�C�g
	VkSemaphore RenderFinishedSemaphore = VK_NULL_HANDLE;		//!< �`�抮������܂ŃE�G�C�g

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

	//!< #TODO ����1�̂݁A�z��ɂ���
	VkBuffer UniformBuffer = VK_NULL_HANDLE;
	VkDeviceMemory UniformDeviceMemory = VK_NULL_HANDLE;
	VkDescriptorBufferInfo UniformDescriptorBufferInfo;

	std::vector<VkViewport> Viewports;
	std::vector<VkRect2D> ScissorRects;

	//!< �f�X�N���v�^�Z�b�g�A�p�C�v���C�����C�A�E�g�쐬���ɕK�v�ɂȂ�̂Ń����o�Ƃ���
	std::vector<VkDescriptorSetLayout> DescriptorSetLayouts;
	VkDescriptorPool DescriptorPool = VK_NULL_HANDLE;
	std::vector<VkDescriptorSet> DescriptorSets;
	VkPipelineLayout PipelineLayout = VK_NULL_HANDLE;

	VkPipeline Pipeline = VK_NULL_HANDLE;
	VkRenderPass RenderPass = VK_NULL_HANDLE;
	std::vector<VkFramebuffer> Framebuffers;

	/**
	@note �o�[�`�����t���[���Ɏ���������� #TODO
	1 �R�}���h�o�b�t�@
	2 �v���[���g�����Z�}�t�H
	3 �`�抮���Z�}�t�H
	4 �t�F���X
	5 �t���[���o�b�t�@
	*/

	//!< �悭�g�����
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

	template<typename T> static void SetName(VkDevice Device, T Object, const char* Name) { DEBUG_BREAK(); /* �e���v���[�g���ꉻ����Ă��Ȃ��AVKDebugMarker.h �Ɏ������邱�� */ }
	template<typename T> static void SetName(VkDevice Device, T Object, const std::string& Name) { SetName(Device, Name.c_str()); }
	template<typename T> static void SetName(VkDevice Device, T Object, const std::wstring& Name) { SetName(Device, std::string(Name.begin(), Name.end())); }

	template<typename T> static void SetTag(VkDevice Device, T Object, const uint64_t TagName, const size_t TagSize, const void* Tag) { DEBUG_BREAK(); /* �e���v���[�g���ꉻ����Ă��Ȃ��AVKDebugMarker.h �Ɏ������邱�� */ }
	
	//!< �������Ńe���v���[�g���ꉻ���Ă���
#include "VKDebugMarker.h"

#define VK_DEVICE_PROC_ADDR(proc) static PFN_vk ## proc ## EXT vk ## proc;
#include "VKProcDeviceAddr.h"
#undef VK_DEVICE_PROC_ADDR
};
#endif