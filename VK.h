#pragma once

#define VK_USE_PLATFORM_WIN32_KHR

#include <vulkan/vulkan.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
//#include <glm/gtc/matrix_transform.hpp>

//!< fopen でなく fopen_s を使えと怒られるが、gli のコードは書き換えたくないので warning を抑制する
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
	VkFormat GetSupportedDepthFormat(VkPhysicalDevice PhysicalDevice) const;
	uint32_t GetMemoryType(const VkPhysicalDeviceMemoryProperties& PhysicalDeviceMemoryProperties, uint32_t TypeBits, VkFlags Properties) const;
	void SetImageLayout(VkCommandBuffer CommandBuffer, VkImage Image, VkImageAspectFlags ImageAspectFlags, VkImageLayout OldImageLayout, VkImageLayout NewImageLayout, VkImageSubresourceRange ImageSubresourceRange) const;
	void SetImageLayout(VkCommandBuffer CommandBuffer, VkImage Image, VkImageAspectFlags ImageAspectFlags, VkImageLayout OldImageLayout, VkImageLayout NewImageLayout) const;
	virtual VkShaderModule CreateShaderModule(const std::string& Path) const;

	virtual void CreateInstance();
	virtual VkPhysicalDevice CreateDevice();
	virtual uint32_t CreateDevice(VkPhysicalDevice PhysicalDevice);
	virtual void CreateCommandPool(const uint32_t QueueFamilyIndex);

	virtual void CreateSwapchain(HWND hWnd, HINSTANCE hInstance, VkPhysicalDevice PhysicalDevice, const VkFormat ColorFormat);
	
	virtual void CreateDepthStencil(const VkPhysicalDeviceMemoryProperties& PhysicalDeviceMemoryProperties, const VkFormat DepthFormat);
	
	virtual void CreateRenderPass(const VkFormat ColorFormat, const VkFormat DepthFormat);

	virtual void CreateFramebuffers();

	virtual void CreateShader();

	virtual void CreateDescriptorSetLayout();
	virtual void CreatePipelineLayout();

	virtual void CreateVertexInput();
	
	virtual void CreateViewport();

	virtual void CreatePipeline();

	virtual void CreateDescriptorSet();

	virtual void CreateVertexBuffer(const VkPhysicalDeviceMemoryProperties& PhysicalDeviceMemoryProperties);
	virtual void CreateIndexBuffer(const VkPhysicalDeviceMemoryProperties& PhysicalDeviceMemoryProperties);
	virtual void CreateUniformBuffer(const VkPhysicalDeviceMemoryProperties& PhysicalDeviceMemoryProperties);

	virtual void CreateCommandBuffers();

	// ----------------------------------

	//virtual void CreateSemaphore();

	//virtual void CreateSetupCommandBuffer();
	//virtual void FlushSetupCommandBuffer();

	virtual void CreateFence();

	virtual void PopulateCommandBuffer();
	virtual void ExecuteCommandBuffer();
	virtual void Present();
	virtual void WaitForFence();

protected:
#ifdef _DEBUG
	const std::vector<const char*> EnabledLayerNames = { "VK_LAYER_LUNARG_standard_validation" };
#endif

#pragma  region Instnce
	VkAllocationCallbacks AllocationCallbacks;
	VkInstance Instance;
#pragma endregion

#pragma region Device
	VkDevice Device;
	VkQueue Queue;
#pragma endregion
	
	VkCommandPool CommandPool = VK_NULL_HANDLE;

#pragma region SwapChain
	VkExtent2D ImageExtent;
	VkSwapchainKHR Swapchain = VK_NULL_HANDLE;
	std::vector<VkImage> SwapchainImages;
	std::vector<VkImageView> SwapchainImageViews;
	VkSemaphore Semaphore;
	uint32_t SwapchainImageIndex = 0;
#pragma endregion

#pragma region DepthStencil
	VkImage DepthStencilImage;
	VkDeviceMemory DepthStencilDeviceMemory;
	VkImageView DepthStencilImageView;
#pragma endregion

	VkRenderPass RenderPass;

#pragma region Framebuffer
	std::vector<VkFramebuffer> Framebuffers;
#pragma endregion

#pragma region Shader
	std::vector<VkShaderModule> ShaderModules;
	std::vector<VkPipelineShaderStageCreateInfo> PipelineShaderStageCreateInfos;
#pragma endregion

#pragma region DescriptorSetLayout
	std::vector<VkDescriptorSetLayout> DescriptorSetLayouts;
#pragma endregion

	VkPipelineLayout PipelineLayout;

#pragma region VertexInput
private:
	using Vertex = std::tuple<glm::vec3, glm::vec4>;
protected:
	std::vector<VkVertexInputBindingDescription> VertexInputBindingDescriptions;
	std::vector<VkVertexInputAttributeDescription> VertexInputAttributeDescriptions;
	VkPipelineVertexInputStateCreateInfo PipelineVertexInputStateCreateInfo;
#pragma endregion

#pragma region Viewport
	std::vector<VkViewport> Viewports;
	std::vector<VkRect2D> ScissorRects;
#pragma endregion

#pragma region GraphicsPipeline
	VkPipelineCache PipelineCache;
	VkPipeline Pipeline;
#pragma endregion

#pragma region VertexBuffer
	VkBuffer VertexBuffer = VK_NULL_HANDLE;
	VkDeviceMemory VertexDeviceMemory = VK_NULL_HANDLE;
#pragma endregion

#pragma region IndexBuffer
	VkBuffer IndexBuffer = VK_NULL_HANDLE;
	VkDeviceMemory IndexDeviceMemory = VK_NULL_HANDLE;
	uint32_t IndexCount = 0;
#pragma endregion

#pragma region UniformBuffer
	VkBuffer UniformBuffer = VK_NULL_HANDLE;
	VkDeviceMemory UniformDeviceMemory = VK_NULL_HANDLE;
	VkDescriptorBufferInfo UniformDescriptorBufferInfo;
	std::vector<VkWriteDescriptorSet> WriteDescriptorSets;
#pragma endregion

#pragma region CommandBuffer
	std::vector<VkCommandBuffer> CommandBuffers;
#pragma endregion

	//-------------------------------

	//VkSemaphore PresentSemaphore = VK_NULL_HANDLE;
	//VkSemaphore RenderSemaphore = VK_NULL_HANDLE;

	//VkPipelineStageFlags SubmitPipelineStages = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;

	//VkSurfaceKHR Surface;
	/*uint32_t QueueFamilyIndex = UINT32_MAX;
	VkFormat ColorFormat;
	VkColorSpaceKHR ColorSpace;
*/
	VkCommandBuffer SetupCommandBuffer = VK_NULL_HANDLE;

#pragma region RootSignature
	VkDescriptorPool DescriptorPool;
	std::vector<VkDescriptorSet> DescriptorSets;
#pragma endregion

	//VkCommandBuffer PrePresentCommandBuffer = VK_NULL_HANDLE;
	//VkCommandBuffer PostPresentCommandBuffer = VK_NULL_HANDLE;

//	std::vector<VkPipelineShaderStageCreateInfo> ShaderStageCreateInfos;

	VkFence Fence;
};

