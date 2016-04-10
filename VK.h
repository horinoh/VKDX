#pragma once

#define VK_USE_PLATFORM_WIN32_KHR

#include <vulkan/vulkan.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
//#include <glm/gtc/matrix_transform.hpp>

//!< fopen ‚Å‚È‚­ fopen_s ‚ðŽg‚¦‚Æ“{‚ç‚ê‚é‚ªAgli ‚ÌƒR[ƒh‚Í‘‚«Š·‚¦‚½‚­‚È‚¢‚Ì‚Å warning ‚ð—}§‚·‚é
#pragma warning (push)
#pragma warning (disable : 4996)
#include <gli/gli.hpp>
#pragma warning (pop) 

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
	virtual void CreateSemaphore();
	virtual void CreateDevice();

	virtual void CreateSurface(HWND hWnd, HINSTANCE hInstance);
	
	virtual void CreateCommandPool();
	virtual void CreateSetupCommandBuffer();
	
	void SetImageLayout(VkCommandBuffer CommandBuffer, VkImage Image, VkImageAspectFlags ImageAspectFlags, VkImageLayout OldImageLayout, VkImageLayout NewImageLayout, VkImageSubresourceRange ImageSubresourceRange);
	void SetImageLayout(VkCommandBuffer CommandBuffer, VkImage Image, VkImageAspectFlags ImageAspectFlags, VkImageLayout OldImageLayout, VkImageLayout NewImageLayout);
	virtual void CreateSwapchain();

	virtual void CreatePipelineLayout();

	virtual void CreateCommandBuffers();

	VkBool32 VK::GetMemoryType(uint32_t TypeBits, VkFlags Properties, uint32_t* TypeIndex);
	virtual void CreateDepthStencil();
	virtual void CreateRenderPass();

	virtual void CreateShader(const std::string& Path, const VkShaderStageFlagBits Stage);
	virtual void CreateShader();

	virtual void CreatePipelineCache();
	virtual void CreateFramebuffers();
	virtual void FlushSetupCommandBuffer();

	virtual void CreateVertexInput();
	virtual void CreateVertexBuffer();
	virtual void CreateIndexBuffer();

	virtual void CreatePipeline();

	virtual void CreateFence();

	virtual void PopulateCommandBuffer();
	virtual void ExecuteCommandBuffer();
	virtual void Present();
	virtual void WaitForFence();

protected:
	VkInstance Instance;
	VkPhysicalDevice PhysicalDevice;
	VkDevice Device;
	VkQueue Queue;
	VkPhysicalDeviceProperties PhysicalDeviceProperties;
	VkPhysicalDeviceMemoryProperties PhysicalDeviceMemoryProperties;
	VkFormat DepthFormat; 

	VkSemaphore PresentSemaphore = VK_NULL_HANDLE;
	VkSemaphore RenderSemaphore = VK_NULL_HANDLE;

	//VkPipelineStageFlags SubmitPipelineStages = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;

	VkSurfaceKHR Surface;
	uint32_t QueueNodeIndex = UINT32_MAX;
	VkFormat ColorFormat;
	VkColorSpaceKHR ColorSpace;

	VkCommandPool CommandPool;

	VkCommandBuffer SetupCommandBuffer = VK_NULL_HANDLE;

	VkSwapchainKHR Swapchain = VK_NULL_HANDLE;
	struct VulkanSwapchainBuffer {
		VkImage Image;
		VkImageView ImageView;
	};
	using VulkanSwapchainBuffer = struct VulkanSwapchainBuffer;
	std::vector<VulkanSwapchainBuffer> SwapchainBuffers;
	uint32_t CurrentSwapchainBufferIndex = 0; //!< SwapchainBuffers[], DrawCommandBuffers[] “™‚Ì“YŽš‚Æ‚µ‚ÄŽg‚í‚ê‚é

	//VkDescriptorSet DescriptorSet;
	VkDescriptorSetLayout DescriptorSetLayout;
	VkPipelineLayout PipelineLayout;

	std::vector<VkCommandBuffer> CommandBuffers;
	VkCommandBuffer PrePresentCommandBuffer = VK_NULL_HANDLE;
	VkCommandBuffer PostPresentCommandBuffer = VK_NULL_HANDLE;

	struct VulkanDepthStencil {
		VkImage Image;
		VkDeviceMemory DeviceMemory;
		VkImageView ImageView;
	} ;
	using VulkanDepthStencil = struct VulkanDepthStencil;
	VulkanDepthStencil DepthStencil;

	VkRenderPass RenderPass;

	std::vector<VkPipelineShaderStageCreateInfo> ShaderStageCreateInfos;

	VkPipelineCache PipelineCache;

	VkPipelineVertexInputStateCreateInfo PipelineVertexInputStateCreateInfo;
	std::vector<VkVertexInputBindingDescription> VertexInputBindingDescriptions;
	std::vector<VkVertexInputAttributeDescription> VertexInputAttributeDescriptions;

	VkPipeline Pipeline;

	VkBuffer VertexBuffer = VK_NULL_HANDLE;
	VkDeviceMemory VertexDeviceMemory = VK_NULL_HANDLE;

	VkBuffer IndexBuffer = VK_NULL_HANDLE;
	VkDeviceMemory IndexDeviceMemory = VK_NULL_HANDLE;

	std::vector<VkFramebuffer> Framebuffers;

	VkFence Fence;
};

