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
	static FORCEINLINE void* AlignedMalloc(void* pUserData, size_t size, size_t alignment, VkSystemAllocationScope allocationScope) { return _aligned_malloc(size, alignment); }
	static FORCEINLINE void* AlignedRealloc(void* pUserData, void* pOriginal, size_t size, size_t alignment, VkSystemAllocationScope allocationScope) { return _aligned_realloc(pOriginal, size, alignment); }
	static FORCEINLINE void AlignedFree(void* pUserData, void* pMemory) { _aligned_free(pMemory); }
	VkBool32 GetSupportedDepthFormat(VkPhysicalDevice PhysicalDevice, VkFormat& Format);
	VkBool32 GetMemoryType(uint32_t TypeBits, VkFlags Properties, uint32_t* TypeIndex) const;

#pragma region DeviceQueue
	virtual void CreateInstance();
	virtual VkPhysicalDevice CreateDevice();
	virtual void CreateDevice(VkPhysicalDevice PhysicalDevice);
	virtual void CreateSurface(HWND hWnd, HINSTANCE hInstance, VkPhysicalDevice PhysicalDevice);
#pragma endregion
	
	virtual void CreateSemaphore();

	virtual void CreateCommandPool();
	virtual void CreateSetupCommandBuffer();
	
#pragma region SwapChain
	void SetImageLayout(VkCommandBuffer CommandBuffer, VkImage Image, VkImageAspectFlags ImageAspectFlags, VkImageLayout OldImageLayout, VkImageLayout NewImageLayout, VkImageSubresourceRange ImageSubresourceRange);
	void SetImageLayout(VkCommandBuffer CommandBuffer, VkImage Image, VkImageAspectFlags ImageAspectFlags, VkImageLayout OldImageLayout, VkImageLayout NewImageLayout);
	virtual void CreateSwapchain(VkPhysicalDevice PhysicalDevice);
#pragma endregion

#pragma region RootSignature
	virtual void CreateDescriptorSet();
	virtual void CreatePipelineLayout();
#pragma endregion

	virtual void CreateCommandBuffers();

	virtual void CreateDepthStencil();
	virtual void CreateRenderPass();

#pragma region Shader
	virtual void CreateShader(const std::string& Path, const VkShaderStageFlagBits Stage);
	virtual void CreateShader();
#pragma endregion

#pragma region Viewport
	virtual void CreateViewport();
#pragma endregion

	virtual void CreateFramebuffers();
	virtual void FlushSetupCommandBuffer();

#pragma region InputLayout
	virtual void CreateVertexInput();
#pragma endregion

#pragma region VertexBuffer
	virtual void CreateVertexBuffer();
	virtual void CreateIndexBuffer();
#pragma endregion

#pragma region ConstantBuffer
	virtual void CreateUniformBuffer();
#pragma endregion

	virtual void CreatePipelineCache();
	virtual void CreatePipeline();

	virtual void CreateFence();

	virtual void PopulateCommandBuffer();
	virtual void ExecuteCommandBuffer();
	virtual void Present();
	virtual void WaitForFence();

protected:
#pragma  region DeviceQueue
	VkAllocationCallbacks AllocationCallbacks;
	VkInstance Instance;
	VkDevice Device;
	VkQueue Queue;
	VkPhysicalDeviceMemoryProperties PhysicalDeviceMemoryProperties;
	VkFormat DepthFormat; 
#pragma endregion

	VkSemaphore PresentSemaphore = VK_NULL_HANDLE;
	VkSemaphore RenderSemaphore = VK_NULL_HANDLE;

	//VkPipelineStageFlags SubmitPipelineStages = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;

	VkSurfaceKHR Surface;
	uint32_t QueueFamilyIndex = UINT32_MAX;
	VkFormat ColorFormat;
	VkColorSpaceKHR ColorSpace;

	VkCommandPool CommandPool;

	VkCommandBuffer SetupCommandBuffer = VK_NULL_HANDLE;

#pragma region SwapChain
	VkSwapchainKHR Swapchain = VK_NULL_HANDLE;
	struct VulkanSwapchainBuffer {
		VkImage Image;
		VkImageView ImageView;
	};
	using VulkanSwapchainBuffer = struct VulkanSwapchainBuffer;
	std::vector<VulkanSwapchainBuffer> SwapchainBuffers;
	uint32_t CurrentSwapchainBufferIndex = 0; //!< SwapchainBuffers[], DrawCommandBuffers[] “™‚Ì“YŽš‚Æ‚µ‚ÄŽg‚í‚ê‚é
#pragma endregion

#pragma region RootSignature
	std::vector<VkDescriptorSetLayout> DescriptorSetLayouts;
	VkDescriptorPool DescriptorPool;
	std::vector<VkDescriptorSet> DescriptorSets;

	VkPipelineLayout PipelineLayout;
#pragma endregion

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

#pragma region Shader
	std::vector<VkPipelineShaderStageCreateInfo> ShaderStageCreateInfos;
#pragma endregion

	VkPipeline Pipeline;
	VkPipelineCache PipelineCache;

#pragma region Viewport
	std::vector<VkViewport> Viewports;
	std::vector<VkRect2D> ScissorRects;
#pragma endregion

#pragma region InputLayout
	VkPipelineVertexInputStateCreateInfo PipelineVertexInputStateCreateInfo;
	std::vector<VkVertexInputBindingDescription> VertexInputBindingDescriptions;
	std::vector<VkVertexInputAttributeDescription> VertexInputAttributeDescriptions;
#pragma endregion

#pragma region VertexBuffer
	VkBuffer VertexBuffer = VK_NULL_HANDLE;
	VkDeviceMemory VertexDeviceMemory = VK_NULL_HANDLE;
	VkBuffer IndexBuffer = VK_NULL_HANDLE;
	VkDeviceMemory IndexDeviceMemory = VK_NULL_HANDLE;
	uint32_t IndexCount = 3;
#pragma endregion

#pragma region ConstantBuffer
	VkBuffer UniformBuffer = VK_NULL_HANDLE;
	VkDeviceMemory UniformDeviceMemory = VK_NULL_HANDLE;
	VkDescriptorBufferInfo UniformDescriptorBufferInfo;
#pragma endregion

	std::vector<VkFramebuffer> Framebuffers;

	VkFence Fence;
};

