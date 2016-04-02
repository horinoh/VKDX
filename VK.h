#pragma once

#define VK_USE_PLATFORM_WIN32_KHR

#include "vulkan/vulkan.h"

#include "Win.h"

#define VERIFY_SUCCEEDED(vr) VERIFY(VK_SUCCESS == (vr))

class VK : public Win
{
private:
	using Super = Win;
public:
	VK();
	virtual ~VK();

	virtual void OnCreate(HWND hWnd, HINSTANCE hInstance) override;
	virtual void OnSize(HWND hWnd, HINSTANCE hInstance) override;
	virtual void OnTimer(HWND hWnd, HINSTANCE hInstance) override;
	virtual void OnPaint(HWND hWnd, HINSTANCE hInstance) override;
	virtual void OnDestroy(HWND hWnd, HINSTANCE hInstance) override;

protected:
	virtual void CreateInstance();
	VkBool32 GetSupportedDepthFormat(VkFormat& Format);
	virtual void CreateDevice();
	virtual void CreateSurface(HWND hWnd, HINSTANCE hInstance);
	virtual void CreateCommandPool();
	virtual void CreateSetupCommandBuffer();
	void SetImageLayout(VkCommandBuffer CommandBuffer, VkImage Image, VkImageAspectFlags ImageAspectFlags, VkImageLayout OldImageLayout, VkImageLayout NewImageLayout, VkImageSubresourceRange ImageSubresourceRange);
	void SetImageLayout(VkCommandBuffer CommandBuffer, VkImage Image, VkImageAspectFlags ImageAspectFlags, VkImageLayout OldImageLayout, VkImageLayout NewImageLayout);
	virtual void CreateSwapchain();
	virtual void CreateDrawCommandBuffers();
	VkBool32 VK::GetMemoryType(uint32_t TypeBits, VkFlags Properties, uint32_t* TypeIndex);
	virtual void CreateDepthStencil();
	virtual void CreateRenderPass();
	virtual void CreatePipelineCache();
	virtual void CreateFramebuffers();
	virtual void FlushSetupCommandBuffer();
	virtual void CreateVertexBuffer();
	virtual void CreateIndexBuffer();

	virtual void PopulateCommandBuffer();

protected:
	VkInstance Instance;

	VkPhysicalDevice PhysicalDevice;
	VkDevice Device;
	VkPhysicalDeviceProperties PhysicalDeviceProperties;
	VkPhysicalDeviceMemoryProperties PhysicalDeviceMemoryProperties;
	VkQueue Queue;
	VkFormat DepthFormat; 
	VkPipelineStageFlags SubmitPipelineStages = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	VkSemaphore PresentSemaphore;
	VkSemaphore RenderSemaphore;

	VkSurfaceKHR Surface;
	uint32_t QueueNodeIndex = UINT32_MAX;
	VkFormat ColorFormat;
	VkColorSpaceKHR ColorSpace;

	VkCommandPool CommandPool;

	VkCommandBuffer SetupCommandBuffer = VK_NULL_HANDLE;

	VkSwapchainKHR Swapchain = VK_NULL_HANDLE;
	struct SwapchainBuffer {
		VkImage Image;
		VkImageView ImageView;
	};
	using VulkanSwapchainBuffer = struct SwapchainBuffer;
	std::vector<VulkanSwapchainBuffer> SwapchainBuffers;

	std::vector<VkCommandBuffer> DrawCommandBuffers;
	VkCommandBuffer PrePresentCommandBuffer = VK_NULL_HANDLE;
	VkCommandBuffer PostPresentCommandBuffer = VK_NULL_HANDLE;

	struct DepthStencil {
		VkImage Image;
		VkDeviceMemory DeviceMemory;
		VkImageView ImageView;
	} ;
	using VulkanDepthStencil = struct DepthStencil;
	VulkanDepthStencil DepthStencil;

	VkRenderPass RenderPass;

	VkPipelineCache PipelineCache;

	std::vector<VkFramebuffer> Framebuffers;
};

