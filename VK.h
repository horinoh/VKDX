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

#ifndef THROW_ON_FAILED
#define THROW_ON_FAILED(vr) if(VK_SUCCESS != (vr)) { throw std::runtime_error("VERIFY_SUCCEEDED failed : " + VK::GetVkResultString(vr)); }
#endif
#ifndef MESSAGEBOX_ON_FAILED
#define MESSAGEBOX_ON_FAILED(vr) if(VK_SUCCESS != (vr)) { Win::ShowMessageBoxW(nullptr, VK::GetVkResultStringW(vr)); }
#endif
#ifndef VERIFY_SUCCEEDED
#define VERIFY_SUCCEEDED(vr) THROW_ON_FAILED(vr)
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
	VK();
	virtual ~VK();

	virtual void OnCreate(HWND hWnd, HINSTANCE hInstance) override;
	virtual void OnSize(HWND hWnd, HINSTANCE hInstance) override;
	virtual void OnTimer(HWND hWnd, HINSTANCE hInstance) override;
	virtual void OnPaint(HWND hWnd, HINSTANCE hInstance) override;
	virtual void OnDestroy(HWND hWnd, HINSTANCE hInstance) override;

	static std::string GetVkResultString(const VkResult Result);
	static std::wstring GetVkResultStringW(const VkResult Result);
	static std::string GetFormatString(const VkFormat Format);

protected:
	static FORCEINLINE void* AlignedMalloc(void* pUserData, size_t size, size_t alignment, VkSystemAllocationScope allocationScope) { return _aligned_malloc(size, alignment); }
	static FORCEINLINE void* AlignedRealloc(void* pUserData, void* pOriginal, size_t size, size_t alignment, VkSystemAllocationScope allocationScope) { return _aligned_realloc(pOriginal, size, alignment); }
	static FORCEINLINE void AlignedFree(void* pUserData, void* pMemory) { _aligned_free(pMemory); }
	VkFormat GetSupportedDepthFormat(VkPhysicalDevice PhysicalDevice) const;
	uint32_t GetMemoryType(const VkPhysicalDeviceMemoryProperties& PhysicalDeviceMemoryProperties, uint32_t TypeBits, VkFlags Properties) const;
	void SetImageLayout(VkCommandBuffer CommandBuffer, VkImage Image, VkImageAspectFlags ImageAspectFlags, VkImageLayout OldImageLayout, VkImageLayout NewImageLayout, VkImageSubresourceRange ImageSubresourceRange) const;
	void SetImageLayout(VkCommandBuffer CommandBuffer, VkImage Image, VkImageAspectFlags ImageAspectFlags, VkImageLayout OldImageLayout, VkImageLayout NewImageLayout) const;
	virtual VkShaderModule CreateShaderModule(const std::wstring& Path) const;

	virtual void CreateInstance();
	virtual VkPhysicalDevice EnumeratePhysicalDevice();
	virtual void CreateDevice(VkPhysicalDevice PhysicalDevice);
	virtual void GetDeviceQueue(VkDevice Device, const uint32_t QueueFamilyIndex);

	virtual void CreateCommandPool(const uint32_t QueueFamilyIndex);
	virtual void CreateCommandBuffer(const VkCommandPool CommandPool);

	virtual void CreateFence();
	virtual void CreateSemaphore();

	virtual void CreateSurface(HWND hWnd, HINSTANCE hInstance);
	virtual void CreateSwapchain(VkPhysicalDevice PhysicalDevice, const uint32_t Width, const uint32_t Height);
	virtual void CreateSwapchainClientRect(VkPhysicalDevice PhysicalDevice) {
		CreateSwapchain(PhysicalDevice, static_cast<uint32_t>(GetClientRectWidth()), static_cast<uint32_t>(GetClientRectHeight()));
	}
	virtual void CreateSwapchainImageView(VkCommandBuffer CommandBuffer, const VkFormat ColorFormat);

	virtual void CreateDepthStencilImage(const VkFormat DepthFormat);
	virtual void CreateDepthStencilDeviceMemory(const VkPhysicalDeviceMemoryProperties& PhysicalDeviceMemoryProperties);
	virtual void CreateDepthStencilView(VkCommandBuffer CommandBuffer, const VkFormat DepthFormat);

	virtual void CreateShader();

	virtual void CreateDescriptorSetLayout();
	virtual void CreateDescritporPool();
	virtual void CreateDescriptorSet(VkDescriptorPool DescritorPool);
	virtual void CreatePipelineLayout();

	virtual void CreateVertexInput();
	
	virtual void CreateViewport(const float Width, const float Height, const float MinDepth = 0.0f, const float MaxDepth = 1.0f);
	virtual void CreateViewportTopFront(const float Width, const float Height) { CreateViewport(Width, Height, 0.0f, 0.0f); }

	float GetAspectRatio(const float Width, const float Height) const { return Width / Height; }
	float GetAspectRatioClientRect() const { return GetAspectRatio(static_cast<float>(GetClientRectWidth()), static_cast<float>(GetClientRectHeight())); }

	virtual void CreatePipeline() { CreateGraphicsPipeline(); }
	virtual void CreateGraphicsPipeline();
	virtual void CreateComputePipeline();

	virtual void CreateRenderPass(const VkFormat ColorFormat, const VkFormat DepthFormat);
	
	virtual void CreateFramebuffer();

	virtual void CreateBuffer(const VkCommandPool CommandPool, const VkPhysicalDeviceMemoryProperties& PhysicalDeviceMemoryProperties, const VkBufferUsageFlagBits Usage, VkBuffer* Buffer, VkDeviceMemory* DeviceMemory, const size_t Size, const void* Source);
	virtual void CreateVertexBuffer(const VkCommandPool CommandPool, const VkPhysicalDeviceMemoryProperties& PhysicalDeviceMemoryProperties);
	virtual void CreateIndexBuffer(const VkCommandPool CommandPool, const VkPhysicalDeviceMemoryProperties& PhysicalDeviceMemoryProperties);
	virtual void CreateUniformBuffer(const VkPhysicalDeviceMemoryProperties& PhysicalDeviceMemoryProperties);

	// ----------------------------------

	virtual void Clear(const VkCommandBuffer CommandBuffer) {}
	virtual void PopulateCommandBuffer(const VkCommandBuffer CommandBuffer);

	virtual void ImageBarrier(VkCommandBuffer CommandBuffer, VkImage Image, VkImageLayout Old, VkImageLayout New);

	virtual void Draw();
	virtual void ExecuteCommandBuffer(const VkCommandBuffer CommandBuffer);
	virtual void Present();
	virtual void WaitForFence();

protected:
	const std::vector<const char*> EnabledLayerNames = { 
#ifdef _DEBUG
		"VK_LAYER_LUNARG_standard_validation"
#endif
	};

	VkAllocationCallbacks AllocationCallbacks;
	VkInstance Instance = VK_NULL_HANDLE;
	VkDevice Device = VK_NULL_HANDLE;
	
	VkQueue Queue = VK_NULL_HANDLE;
	uint32_t QueueFamilyIndex = UINT_MAX;

	std::vector<VkCommandPool> CommandPools;
	std::vector<VkCommandBuffer> CommandBuffers;

	VkFence Fence = VK_NULL_HANDLE;

	std::vector<VkSemaphore> PresentSemaphores;
	std::vector<VkSemaphore> RenderSemaphores;

	VkSurfaceKHR Surface;
	VkExtent2D ImageExtent;
	VkSwapchainKHR Swapchain = VK_NULL_HANDLE;
	std::vector<VkImage> SwapchainImages;
	std::vector<VkImageView> SwapchainImageViews;
	uint32_t SwapchainImageIndex = 0;

	VkImage DepthStencilImage = VK_NULL_HANDLE;
	VkDeviceMemory DepthStencilDeviceMemory = VK_NULL_HANDLE;
	VkImageView DepthStencilImageView = VK_NULL_HANDLE;

	std::vector<VkShaderModule> ShaderModules;

	std::vector<VkDescriptorSetLayout> DescriptorSetLayouts;
	VkPipelineLayout PipelineLayout = VK_NULL_HANDLE;

	VkDescriptorPool DescriptorPool = VK_NULL_HANDLE;
	std::vector<VkDescriptorSet> DescriptorSets;

	std::vector<VkVertexInputBindingDescription> VertexInputBindingDescriptions;
	std::vector<VkVertexInputAttributeDescription> VertexInputAttributeDescriptions;
	VkPipelineVertexInputStateCreateInfo PipelineVertexInputStateCreateInfo;

	std::vector<VkViewport> Viewports;
	std::vector<VkRect2D> ScissorRects;

	VkPipelineCache PipelineCache = VK_NULL_HANDLE;
	VkPipeline Pipeline = VK_NULL_HANDLE;
	VkRenderPass RenderPass = VK_NULL_HANDLE;
	std::vector<VkFramebuffer> Framebuffers;

	VkBuffer VertexBuffer = VK_NULL_HANDLE;
	VkDeviceMemory VertexDeviceMemory = VK_NULL_HANDLE;

	VkBuffer IndexBuffer = VK_NULL_HANDLE;
	VkDeviceMemory IndexDeviceMemory = VK_NULL_HANDLE;
	uint32_t IndexCount = 0;

	//!< #TODO 現状1つのみ、配列にする
	VkBuffer UniformBuffer = VK_NULL_HANDLE;
	VkDeviceMemory UniformDeviceMemory = VK_NULL_HANDLE;
	VkDescriptorBufferInfo UniformDescriptorBufferInfo;
	std::vector<VkWriteDescriptorSet> WriteDescriptorSets;

	//-------------------------------

	//VkSemaphore PresentSemaphore = VK_NULL_HANDLE;
	//VkSemaphore RenderSemaphore = VK_NULL_HANDLE;

	//VkPipelineStageFlags SubmitPipelineStages = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;

	//VkSurfaceKHR Surface;
	/*uint32_t QueueFamilyIndex = UINT32_MAX;
	VkFormat ColorFormat;
	VkColorSpaceKHR ColorSpace;
*/
	//VkCommandBuffer SetupCommandBuffer = VK_NULL_HANDLE;

	//VkCommandBuffer PrePresentCommandBuffer = VK_NULL_HANDLE;
	//VkCommandBuffer PostPresentCommandBuffer = VK_NULL_HANDLE;

//	std::vector<VkPipelineShaderStageCreateInfo> ShaderStageCreateInfos;
};

class VKExt : public VK
{
private:
	using Super = VK;
public:
	virtual void CreateShader_VsPs();
	virtual void CreateShader_VsPsTesTcsGs();
	virtual void CreateShader_Cs();

	virtual void CreateVertexInput_Position();
	virtual void CreateVertexInput_PositionColor();

	virtual void CreateGraphicsPipeline_VsPs();
	virtual void CreateGraphicsPipeline_VsPsTesTcsGs();

	virtual void CreateRenderPass(const VkFormat ColorFormat, const VkFormat DepthFormat) { CreateRenderPass_Color(ColorFormat); }
	virtual void CreateRenderPass_Color(const VkFormat ColorFormat);
	virtual void CreateRenderPass_ColorDepth(const VkFormat ColorFormat, const VkFormat DepthFormat);
	
	virtual void CreateFramebuffer() override { CreateFramebuffer_Color(); }
	virtual void CreateFramebuffer_Color();
	virtual void CreateFramebuffer_ColorDepth();

	template<typename T>
	void CreateUniformBuffer(const VkPhysicalDeviceMemoryProperties& PhysicalDeviceMemoryProperties, const T& Type) {
		//!< #TODO
#ifdef _DEBUG
		std::cout << "CreateUniformBuffer" << COUT_OK << std::endl << std::endl;
#endif
	}

	virtual void Clear(const VkCommandBuffer CommandBuffer) override { Clear_Color(CommandBuffer); }
	virtual void Clear_Color(const VkCommandBuffer CommandBuffer);
	virtual void Clear_Depth(const VkCommandBuffer CommandBuffer);
};