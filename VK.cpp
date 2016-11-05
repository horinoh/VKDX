#include "stdafx.h"

#include <fstream>

#include "VK.h"

#pragma comment(lib, "vulkan-1.lib")

void VK::OnCreate(HWND hWnd, HINSTANCE hInstance, LPCWSTR Title)
{
#ifdef _DEBUG
	PerformanceCounter PC("OnCreate : ");
#endif

	Super::OnCreate(hWnd, hInstance, Title);

	//!< �f�o�C�X�A�L���[
	CreateInstance();
	CreateSurface(hWnd, hInstance);
	GetPhysicalDevice();
	GetQueueFamily();
	CreateDevice();
	
	//!< �R�}���h�v�[���A�o�b�t�@
	CreateCommandPool(GraphicsQueueFamilyIndex);
	auto CommandPool = CommandPools[0];
	AllocateCommandBuffer(CommandPool);
	auto CommandBuffer = CommandBuffers[0];

	CreateFence();
	CreateSemaphore();

	//!< �X���b�v�`�F�C��
	CreateSwapchain(CommandBuffer);
	//!< �f�v�X�X�e���V��
	CreateDepthStencil(CommandBuffer);

	//!< �o�[�e�b�N�X�o�b�t�@�A�C���f�b�N�X�o�b�t�@
	CreateVertexBuffer(CommandBuffer);
	CreateIndexBuffer(CommandBuffer);

	CreateTexture();

	//!< �f�X�N���v�^�Z�b�g
	CreateDescriptorSetLayout();
	CreateDescriptorSet();
	UpdateDescriptorSet();

	//!< ���j�t�H�[���o�b�t�@
	//CreateUniformBuffer();

	//!< �����_�[�p�X�A�t���[���o�b�t�@�A�p�C�v���C��
	CreateRenderPass();
	CreateFramebuffer();
	CreatePipeline();
}
void VK::OnSize(HWND hWnd, HINSTANCE hInstance)
{
#ifdef _DEBUG
	PerformanceCounter PC("OnSize : ");
#endif

	Super::OnSize(hWnd, hInstance);

	ResizeSwapChainToClientRect();

	CreateViewport(static_cast<float>(SurfaceExtent2D.width), static_cast<float>(SurfaceExtent2D.height));

	DestroyFramebuffer();
	CreateFramebuffer();
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

	DestroyFramebuffer();
	
	if (VK_NULL_HANDLE != RenderPass) {
		vkDestroyRenderPass(Device, RenderPass, nullptr);
		RenderPass = VK_NULL_HANDLE;
	}
	if (VK_NULL_HANDLE != Pipeline) {
		vkDestroyPipeline(Device, Pipeline, nullptr);
		Pipeline = VK_NULL_HANDLE;
	}
	//if (VK_NULL_HANDLE != PipelineCache) {
	//	vkDestroyPipelineCache(Device, PipelineCache, nullptr);
	//	PipelineCache = VK_NULL_HANDLE;
	//}

	if (VK_NULL_HANDLE != PipelineLayout) {
		vkDestroyPipelineLayout(Device, PipelineLayout, nullptr);
	}

	if (!DescriptorSets.empty()) {
		vkFreeDescriptorSets(Device, DescriptorPool, static_cast<uint32_t>(DescriptorSets.size()), DescriptorSets.data());
	}
	DescriptorSets.clear();
	if (VK_NULL_HANDLE != DescriptorPool) {
		vkDestroyDescriptorPool(Device, DescriptorPool, nullptr);
		DescriptorPool = VK_NULL_HANDLE;
	}
	for (auto i : DescriptorSetLayouts) {
		vkDestroyDescriptorSetLayout(Device, i, nullptr);
	}
	DescriptorSetLayouts.clear();

	if (VK_NULL_HANDLE != UniformDeviceMemory) {
		vkFreeMemory(Device, UniformDeviceMemory, nullptr);
		UniformDeviceMemory = VK_NULL_HANDLE;
	}
	if (VK_NULL_HANDLE != UniformBuffer) {
		vkDestroyBuffer(Device, UniformBuffer, nullptr);
		UniformBuffer = VK_NULL_HANDLE;
	}

	if (VK_NULL_HANDLE != IndexDeviceMemory) {
		vkFreeMemory(Device, IndexDeviceMemory, nullptr);
		IndexDeviceMemory = VK_NULL_HANDLE;
	}
	if (VK_NULL_HANDLE != IndexBuffer) {
		vkDestroyBuffer(Device, IndexBuffer, nullptr);
		IndexBuffer = VK_NULL_HANDLE;
	}
	if (VK_NULL_HANDLE != VertexDeviceMemory) {
		vkFreeMemory(Device, VertexDeviceMemory, nullptr);
		VertexDeviceMemory = VK_NULL_HANDLE;
	}
	if (VK_NULL_HANDLE != VertexBuffer) {
		vkDestroyBuffer(Device, VertexBuffer, nullptr);
		VertexBuffer = VK_NULL_HANDLE;
	}

	if (VK_NULL_HANDLE != Sampler) {
		vkDestroySampler(Device, Sampler, nullptr);
	}
	if (VK_NULL_HANDLE != ImageView) {
		vkDestroyImageView(Device, ImageView, nullptr);
		ImageView = VK_NULL_HANDLE;
	}
	if (VK_NULL_HANDLE != ImageDeviceMemory) {
		vkFreeMemory(Device, ImageDeviceMemory, nullptr);
		ImageDeviceMemory = VK_NULL_HANDLE;
	}
	if (VK_NULL_HANDLE != Image) {
		vkDestroyImage(Device, Image, nullptr);
		Image = VK_NULL_HANDLE;
	}

	if (VK_NULL_HANDLE != DepthStencilImageView) {
		vkDestroyImageView(Device, DepthStencilImageView, nullptr);
		DepthStencilImageView = VK_NULL_HANDLE;
	}
	if (VK_NULL_HANDLE != DepthStencilDeviceMemory) {
		vkFreeMemory(Device, DepthStencilDeviceMemory, nullptr);
		DepthStencilDeviceMemory = VK_NULL_HANDLE;
	}
	if (VK_NULL_HANDLE != DepthStencilImage) {
		vkDestroyImage(Device, DepthStencilImage, nullptr);
		DepthStencilImage = VK_NULL_HANDLE;
	}

	for (auto i : SwapchainImageViews) {
		vkDestroyImageView(Device, i, nullptr);
	}
	SwapchainImageViews.clear();
	//!< SwapchainImages �� vkGetSwapchainImagesKHR() �Ŏ擾�������́A�j�����Ȃ�
	if (VK_NULL_HANDLE != Swapchain) {
		vkDestroySwapchainKHR(Device, Swapchain, nullptr);
		Swapchain = VK_NULL_HANDLE;
	}
	if (VK_NULL_HANDLE != RenderFinishedSemaphore) {
		vkDestroySemaphore(Device, RenderFinishedSemaphore, nullptr);
		RenderFinishedSemaphore = VK_NULL_HANDLE;
	}
	if (VK_NULL_HANDLE != NextImageAcquiredSemaphore) {
		vkDestroySemaphore(Device, NextImageAcquiredSemaphore, nullptr);
		NextImageAcquiredSemaphore = VK_NULL_HANDLE;
	}
	if (VK_NULL_HANDLE != Fence) {
		vkDestroyFence(Device, Fence, nullptr);
		Fence = VK_NULL_HANDLE;
	}

	//!< �����ł� Pool �� Buffer �� 1:1 �Ƃ���
	for (auto i = 0; i < CommandBuffers.size(); ++i) {
		vkFreeCommandBuffers(Device, CommandPools[i], 1, &CommandBuffers[i]);
		vkDestroyCommandPool(Device, CommandPools[i], nullptr);
	}
	CommandPools.clear();
	CommandBuffers.clear();

	//!< Queue �� vkGetDeviceQueue() �Ŏ擾�������́A�j�����Ȃ�

	if (VK_NULL_HANDLE != Device) {
		vkDestroyDevice(Device, nullptr);
		Device = VK_NULL_HANDLE;
	}
	
	//!< PhysicalDevice �� vkEnumeratePhysicalDevices() �Ŏ擾�������́A�j�����Ȃ�

	if (VK_NULL_HANDLE != Surface) {
		vkDestroySurfaceKHR(Instance, Surface, nullptr);
		Surface = VK_NULL_HANDLE;
	}

#ifdef _DEBUG
	if (VK_NULL_HANDLE != DebugReportCallback) {
		DebugReport::DestroyDebugReportCallback(Instance, DebugReportCallback);
		DebugReportCallback = VK_NULL_HANDLE;
	}
#endif

	if (VK_NULL_HANDLE != Instance) {
		//vkDestroyInstance(Instance, &AllocationCallbacks);
		vkDestroyInstance(Instance, nullptr);
		Instance = VK_NULL_HANDLE;
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
uint32_t VK::GetMemoryType(const VkPhysicalDeviceMemoryProperties& PhysicalDeviceMemoryProperties, const uint32_t MemoryTypeBits, const VkFlags Properties)
{
	for (auto i = 0; i < 32; ++i) {
		if (MemoryTypeBits & (1 << i)) {
			if ((PhysicalDeviceMemoryProperties.memoryTypes[i].propertyFlags & Properties) == Properties) {
				return i;
			}
		}
	}
	DEBUG_BREAK();
	return 0;
}

//!< �J�ڂ���O�� OldImageLayout ���������Ă��Ȃ���΂Ȃ�Ȃ��A�N�V����
//VkAccessFlags VK::GetSrcAccessMask(VkImageLayout OldImageLayout, VkImageLayout NewImageLayout)
//{
//	VkAccessFlags AccessMask = 0;
//
//	switch (OldImageLayout)
//	{
//	case VK_IMAGE_LAYOUT_UNDEFINED:
//		AccessMask = 0;
//		break;
//	case VK_IMAGE_LAYOUT_PREINITIALIZED:
//		//!< �z�X�g�ɂ�鏑�����݂��������Ă��Ȃ���΂Ȃ�Ȃ�
//		AccessMask = VK_ACCESS_HOST_WRITE_BIT; 
//		break;
//	case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
//		//!< �J���[�o�b�t�@�������݂��������Ă��Ȃ���΂Ȃ�Ȃ�
//		AccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
//		break;
//	case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
//		//!< �f�v�X�X�e���V���o�b�t�@�������݂��������Ă��Ȃ���΂Ȃ�Ȃ�
//		AccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
//		break;
//	case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
//		//!< �C���[�W�ǂݍ��݂��������Ă��Ȃ���΂Ȃ�Ȃ�
//		AccessMask = VK_ACCESS_TRANSFER_READ_BIT;
//		break;
//	case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
//		//!< �C���[�W�ւ̏������݂��������Ă��Ȃ���΂Ȃ�Ȃ�
//		AccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
//		break;
//	case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
//		//!< �V�F�[�_�ɂ��C���[�W�ǂݍ��݂��������Ă��Ȃ���΂Ȃ�Ȃ�
//		AccessMask = VK_ACCESS_SHADER_READ_BIT;
//		break;
//	}
//	switch (NewImageLayout)
//	{
//	case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
//		AccessMask |= VK_ACCESS_TRANSFER_READ_BIT;
//		break;
//	case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
//		AccessMask = VK_ACCESS_TRANSFER_READ_BIT;
//		break;
//	case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
//		if (0 == AccessMask){
//			AccessMask = VK_ACCESS_HOST_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT;
//		}
//		break;
//	}
//	return AccessMask;
//}
//VkAccessFlags VK::GetDstAccessMask(VkImageLayout OldImageLayout, VkImageLayout NewImageLayout)
//{
//	VkAccessFlags AccessMask = 0;
//
//	switch (NewImageLayout)
//	{
//	case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
//		AccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
//		break;
//	case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
//		AccessMask = VK_ACCESS_TRANSFER_READ_BIT;
//		break;
//	case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
//		AccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
//		break;
//	case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
//		AccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
//		break;
//	case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
//		AccessMask = VK_ACCESS_SHADER_READ_BIT;
//		break;
//	}
//	return AccessMask;
//}
//void VK::SetImageLayout(VkCommandBuffer CommandBuffer, VkImage Image, VkImageLayout OldImageLayout, VkImageLayout NewImageLayout, VkImageSubresourceRange ImageSubresourceRange) const
//{
//	const auto SrcAccessMask = GetSrcAccessMask(OldImageLayout, NewImageLayout);
//	const auto DstAccessMask = GetDstAccessMask(OldImageLayout, NewImageLayout);
//
//	const VkImageMemoryBarrier ImageMemoryBarrier = {
//		VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
//		nullptr,
//		SrcAccessMask,
//		DstAccessMask,
//		OldImageLayout,
//		NewImageLayout,
//		VK_QUEUE_FAMILY_IGNORED,
//		VK_QUEUE_FAMILY_IGNORED,
//		Image,
//		ImageSubresourceRange
//	};
//	vkCmdPipelineBarrier(CommandBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0, 0, nullptr, 0, nullptr, 1, &ImageMemoryBarrier);
//}

void VK::CreateBuffer(VkBuffer* Buffer, const VkBufferUsageFlags Usage, const size_t Size) const
{
	const VkBufferCreateInfo BufferCreateInfo = {
		VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
		nullptr,
		0,
		Size,
		Usage,
		VK_SHARING_MODE_EXCLUSIVE,
		0, nullptr
	};
	VERIFY_SUCCEEDED(vkCreateBuffer(Device, &BufferCreateInfo, nullptr, Buffer));
}

void VK::CreateImage(VkImage* Image, const VkImageUsageFlags Usage, const VkImageType ImageType, const VkFormat Format, const VkExtent3D& Extent3D, const uint32_t MipLevels, const uint32_t ArrayLayers) const
{
	const VkImageCreateInfo ImageCreateInfo = {
		VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
		nullptr,
		0,
		ImageType,
		Format,
		Extent3D,
		MipLevels,
		ArrayLayers,
		VK_SAMPLE_COUNT_1_BIT,
		VK_IMAGE_TILING_OPTIMAL,
		Usage,
		VK_SHARING_MODE_EXCLUSIVE,
		0, nullptr,
		VK_IMAGE_LAYOUT_UNDEFINED
	};
	VERIFY_SUCCEEDED(vkCreateImage(Device, &ImageCreateInfo, nullptr, Image));
}

void VK::CopyToHostVisibleMemory(const VkBuffer Buffer, const VkDeviceMemory DeviceMemory, const size_t Size, const void* Source, const VkDeviceSize Offset)
{
	if (Size && nullptr != Source) {
		void *Data;
		VERIFY_SUCCEEDED(vkMapMemory(Device, DeviceMemory, Offset, Size, static_cast<VkMemoryMapFlags>(0), &Data)); {
			memcpy(Data, Source, Size);
			//!< ��VK_MEMORY_PROPERTY_HOST_COHERENT_BIT �̏ꍇ�͕K�v�Ȃ�
			const std::vector<VkMappedMemoryRange> MappedMemoryRanges = {
				{
					VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE,
					nullptr,
					DeviceMemory,
					0,
					VK_WHOLE_SIZE
				}
			};
			VERIFY_SUCCEEDED(vkFlushMappedMemoryRanges(Device, static_cast<uint32_t>(MappedMemoryRanges.size()), MappedMemoryRanges.data()));
		} vkUnmapMemory(Device, DeviceMemory);
	}
}
void VK::SubmitCopyBuffer(const VkCommandBuffer CommandBuffer, const VkBuffer SrcBuffer, const VkBuffer DstBuffer, const VkAccessFlags AccessFlag, const VkPipelineStageFlagBits PipelineStageFlag, const size_t Size)
{
	VERIFY_SUCCEEDED(vkBeginCommandBuffer(CommandBuffer, &CommandBufferBeginInfo_OneTime)); {
		const VkBufferCopy BufferCopy = {
			0,
			0,
			Size
		};
		vkCmdCopyBuffer(CommandBuffer, SrcBuffer, DstBuffer, 1, &BufferCopy);

		const VkBufferMemoryBarrier BufferMemoryBarrier = {
			VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
			nullptr,
			VK_ACCESS_MEMORY_WRITE_BIT,
			AccessFlag,
			VK_QUEUE_FAMILY_IGNORED,
			VK_QUEUE_FAMILY_IGNORED,
			DstBuffer,
			0,
			VK_WHOLE_SIZE
		};
		//!< �o�b�t�@���uVK_PIPELINE_STAGE_TRANSFER_BIT�v����uPipelineStageFlag�v�֕ύX����
		vkCmdPipelineBarrier(CommandBuffer, 
			VK_PIPELINE_STAGE_TRANSFER_BIT, PipelineStageFlag,
			0, 
			0, nullptr,
			1, &BufferMemoryBarrier, 
			0, nullptr);

	} VERIFY_SUCCEEDED(vkEndCommandBuffer(CommandBuffer));

	//!< �T�u�~�b�g
	const VkSubmitInfo SubmitInfo = {
		VK_STRUCTURE_TYPE_SUBMIT_INFO,
		nullptr,
		0, nullptr,
		nullptr,
		1, &CommandBuffer,
		0, nullptr
	};
	VERIFY_SUCCEEDED(vkQueueSubmit(GraphicsQueue, 1, &SubmitInfo, VK_NULL_HANDLE));
	VERIFY_SUCCEEDED(vkDeviceWaitIdle(Device)); //!< �t�F���X�ł��ǂ�
}

void VK::CreateStagingBufferAndCopyToMemory(VkBuffer* Buffer, VkDeviceMemory* DeviceMemory, const size_t Size, const void* Source /*= nullptr*/, const VkDeviceSize Offset /*= 0*/)
{
	//!< �]�����̃o�b�t�@���쐬
	CreateBuffer(Buffer, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, Size);
	//!< ������(�z�X�g�r�W�u��)���쐬
	CreateHostVisibleMemory(DeviceMemory, *Buffer);

	//!< ������(�z�X�g�r�W�u��)�փf�[�^���R�s�[
	CopyToHostVisibleMemory(*Buffer, *DeviceMemory, Size, Source, Offset);
	//!< �o�b�t�@�ƃ��������o�C���h
	BindDeviceMemory(*Buffer, *DeviceMemory, Offset);
}
void VK::CreateDeviceLocalBuffer(VkBuffer* Buffer, VkDeviceMemory* DeviceMemory, const VkBufferUsageFlags Usage, const size_t Size, const VkDeviceSize Offset /*= 0*/)
{
	//!< �o�b�t�@���쐬
	CreateBuffer(Buffer, Usage, Size);
	//!< ������(�f�o�C�X���[�J��)���쐬
	CreateDeviceLocalMemory(DeviceMemory, *Buffer);

	//!< �o�b�t�@�ƃ��������o�C���h
	BindDeviceMemory(*Buffer, *DeviceMemory, Offset);
}

void VK::CreateImageView(VkImageView* ImageView, const VkImage Image, const VkImageViewType ImageViewType, const VkFormat Format, const VkComponentMapping& ComponentMapping, const VkImageSubresourceRange& ImageSubresourceRange)
{
	const VkImageViewCreateInfo ImageViewCreateInfo = {
		VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
		nullptr,
		0,
		Image,
		ImageViewType,
		Format,
		ComponentMapping,
		ImageSubresourceRange
	};
	VERIFY_SUCCEEDED(vkCreateImageView(Device, &ImageViewCreateInfo, nullptr, ImageView));

#ifdef DEBUG_STDOUT
	std::cout << "\t" << "ImageView" << std::endl;
#endif
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
#endif
		}
	}
}

void VK::CreateInstance()
{
	EnumerateInstanceLayer();

	const auto ApplicationName = GetTitle();
	const VkApplicationInfo ApplicationInfo = {
		VK_STRUCTURE_TYPE_APPLICATION_INFO,
		nullptr,
		ApplicationName.data(), 0,
		"VKDX Engine Name", 0,
		VK_API_VERSION_1_0
	};
	const std::vector<const char*> EnabledLayerNames = {
#ifdef _DEBUG
		//!< ���W���I�ȃo���f�[�V�������C���Z�b�g���œK�ȏ����Ń��[�h����w��
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
		//!< ���f�o�b�O���|�[�g�p
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
@brief �f�o�C�X(GPU)�ƃz�X�g(CPU)�̓���
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
	//!< �����f�o�C�X(GPU)�̗�
	uint32_t PhysicalDeviceCount = 0;
	VERIFY_SUCCEEDED(vkEnumeratePhysicalDevices(Instance, &PhysicalDeviceCount, nullptr));
	if (!PhysicalDeviceCount) { DEBUG_BREAK(); } //!< PhysicalDevice not found
	std::vector<VkPhysicalDevice> PhysicalDevices(PhysicalDeviceCount);
	VERIFY_SUCCEEDED(vkEnumeratePhysicalDevices(Instance, &PhysicalDeviceCount, PhysicalDevices.data()));
#ifdef DEBUG_STDOUT
	std::cout << Yellow << "\t" << "PhysicalDevices" << White << std::endl;
#define PHYSICAL_DEVICE_TYPE_ENTRY(entry) if(VK_PHYSICAL_DEVICE_TYPE_##entry == PhysicalDeviceProperties.deviceType) { std::cout << #entry; }
	for (const auto& i : PhysicalDevices) {
		//!< �����f�o�C�X�̃v���p�e�B
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
		
		//!< �����f�o�C�X�̃t�B�[�`���[
		VkPhysicalDeviceFeatures PhysicalDeviceFeatures;
		vkGetPhysicalDeviceFeatures(i, &PhysicalDeviceFeatures);
	}
#undef PHYSICAL_DEVICE_TYPE_ENTRY
#endif

	//!< �����ł͍ŏ��̕����f�o�C�X��I�����邱�Ƃɂ��� #TODO
	PhysicalDevice = PhysicalDevices[0];

	//!< �I�����������f�o�C�X�̃������v���p�e�B���擾
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
		}
	}
}
void VK::GetQueueFamily()
{
	//!< �L���[�̃v���p�e�B���
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
		//!< �O���t�B�b�N�@�\��������
		if (VK_QUEUE_GRAPHICS_BIT & QueueProperties[i].queueFlags) {
			if (UINT32_MAX != GraphicsQueueFamilyIndex) {
				GraphicsQueueFamilyIndex = i;
			}
		}
		//else if (VK_QUEUE_TRANSFER_BIT & QueueProperties[i].queueFlags) {
		//	//!< #TODO
		//	TransferQueueFamilyIndex = i; //!< �f�o�C�X�ɂ���Ă͓]����p�L���[�����A�]���𑽗p����ꍇ�͐�p�L���[���g�p���������ǂ�
		//}
		//else if (VK_QUEUE_COMPUTE_BIT & QueueProperties[i].queueFlags) {
		//	//!< #TODO
		//	ComputeQueueFamilyIndex = i;
		//}

		//!< �v���[���g�@�\��������
		VkBool32 Supported = VK_FALSE;
		VERIFY_SUCCEEDED(vkGetPhysicalDeviceSurfaceSupportKHR(PhysicalDevice, i, Surface, &Supported));
		if (Supported) {
			if (UINT32_MAX != PresentQueueFamilyIndex) {
				PresentQueueFamilyIndex = i;
			}
		}
	}
	//!< �O���t�B�b�N�ƃv���[���g�𓯎��ɃT�|�[�g����L���[������ΗD��
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
	//!< �O���t�B�b�N�ƃv���[���g�̃L���[�C���f�b�N�X���ʂ̏ꍇ�͒ǉ��ŕK�v
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
		//!< ���W���I�ȃo���f�[�V�������C���Z�b�g���œK�ȏ����Ń��[�h����w��
		"VK_LAYER_LUNARG_standard_validation", 
#endif
	};
#ifdef _DEBUG
	std::vector<const char*> EnabledExtensions = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME,
	};
	//!< ���f�o�b�O�}�[�J�[�g��������Ȃ�ǉ�
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

	//!< �L���[�̎擾 (�O���t�B�b�N�A�v���[���g�L���[�͓����C���f�b�N�X�̏ꍇ�����邪�ʖ��Ƃ��Ď擾)
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
		VK_COMMAND_POOL_CREATE_TRANSIENT_BIT ... �p�ɂɍX�V�����A���C�t�X�p�����Z���ꍇ(�������A���P�[�V�����̃q���g�ƂȂ�)
		VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT ... �w�肵���ꍇ�� vkResetCommandBuffer(), vkBeginCommandBuffer() �ɂ�郊�Z�b�g���\�A�w�肵�Ȃ��ꍇ�� vkResetCommandPool() �̂݉\
		*/
		VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT | VK_COMMAND_POOL_CREATE_TRANSIENT_BIT,
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
@brief CPU �� GPU �̓���
*/
void VK::CreateFence()
{
	const VkFenceCreateInfo FenceCreateInfo = {
		VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
		nullptr,
		0// #TODO VK_FENCE_CREATE_SIGNALED_BIT //!< ����ƂQ��ڈȍ~�𓯂��Ɉ����ׂɁA�V�O�i���ςݏ�Ԃō쐬
	};
	VERIFY_SUCCEEDED(vkCreateFence(Device, &FenceCreateInfo, nullptr, &Fence));

#ifdef DEBUG_STDOUT
	std::cout << "CreateFence" << COUT_OK << std::endl << std::endl;
#endif
}

/**
@brief �����L���[�Ԃ̓����A�P��L���[�ւ̑a���x�̃T�u�~�b�g�̓���
*/
void VK::CreateSemaphore()
{
	const VkSemaphoreCreateInfo SemaphoreCreateInfo = {
		VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
		nullptr,
		0
	};

	//!< �v���[���g���������p
	VERIFY_SUCCEEDED(vkCreateSemaphore(Device, &SemaphoreCreateInfo, nullptr, &NextImageAcquiredSemaphore));

	//!< �`�抮�������p
	VERIFY_SUCCEEDED(vkCreateSemaphore(Device, &SemaphoreCreateInfo, nullptr, &RenderFinishedSemaphore));

#ifdef _DEBUG
	std::cout << "CreateSemaphore" << COUT_OK << std::endl << std::endl;
#endif
}

VkSurfaceFormatKHR VK::SelectSurfaceFormat()
{
	//!< �T�[�t�F�X�̃t�H�[�}�b�g���擾
	uint32_t SurfaceFormatCount;
	VERIFY_SUCCEEDED(vkGetPhysicalDeviceSurfaceFormatsKHR(PhysicalDevice, Surface, &SurfaceFormatCount, nullptr));
	assert(SurfaceFormatCount);
	std::vector<VkSurfaceFormatKHR> SurfaceFormats(SurfaceFormatCount);
	VERIFY_SUCCEEDED(vkGetPhysicalDeviceSurfaceFormatsKHR(PhysicalDevice, Surface, &SurfaceFormatCount, SurfaceFormats.data()));

	//!< �����ł́A�ŏ��̔� VK_FORMAT_UNDEFINED ��I������
	for (uint32_t i = 0; i < SurfaceFormats.size(); ++i) {
		if (VK_FORMAT_UNDEFINED != SurfaceFormats[i].format) {
#ifdef DEBUG_STDOUT
			[&]() {
				std::cout << "\t" << "\t" << Lightblue << "Format" << White << std::endl;
				for (uint32_t j = 0; j < SurfaceFormats.size();++j) {
					if (i == j) { std::cout << Yellow; }
					std::cout << "\t" << "\t" << "\t" << GetFormatString(SurfaceFormats[j].format) << std::endl;
					std::cout << White;
				}
				std::cout << std::endl;
			}();
#endif
			return SurfaceFormats[i];
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

	//!< �\�Ȃ� VK_PRESENT_MODE_MAILBOX_KHR ��I���A������ VK_PRESENT_MODE_FIFO_KHR ��I��
	/**
	VK_PRESENT_MODE_IMMEDIATE_KHR
	VK_PRESENT_MODE_MAILBOX_KHR ... V�u�����N�ŕ\�������A�e�A�����O�͋N����Ȃ��A�L���[��1�ŏ�ɍŐV�ŏ㏑������� (�Q�[����)
	VK_PRESENT_MODE_FIFO_KHR ... V�u�����N�ŕ\�������A�e�A�����O�͋N����Ȃ� (���[�r�[��)
	VK_PRESENT_MODE_FIFO_RELAXED_KHR ... 1V�u�����N�ȏ�o�����C���[�W�͎���V�u�����N��҂����Ƀ����[�X���꓾��A�e�A�����O���N����
	*/
	const VkPresentModeKHR SelectedPresentMode = [&]() {
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
		if (SelectedPresentMode == i) { std::cout << Yellow; }
		switch (i) {
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

	return SelectedPresentMode;
}
void VK::CreateSwapchain(const uint32_t Width, const uint32_t Height)
{
	//!< �T�[�t�F�X�̏����擾
	VkSurfaceCapabilitiesKHR SurfaceCapabilities;
	VERIFY_SUCCEEDED(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(PhysicalDevice, Surface, &SurfaceCapabilities));

	//!< �C���[�W���� (�����ł�1��������� ... MAILBOX �̏ꍇ3�������������ǂ��̂�)
	const auto MinImageCount = (std::min)(SurfaceCapabilities.minImageCount + 1, SurfaceCapabilities.maxImageCount);
#ifdef DEBUG_STDOUT
	std::cout << "\t" << "\t" << Lightblue << "ImageCount = " << White << MinImageCount << std::endl;
#endif

	//!< �T�[�t�F�X�T�C�Y
	//!< ����`�̏ꍇ�͖����I�Ɏw�肷��B(�T�C�Y����`����Ă���ꍇ�͂���ɏ]��Ȃ��ƂȂ�Ȃ�)
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

	//!< �C���[�W�g�p�@ (�C���[�W�N���A�ł���悤�ɉ\�Ȃ�VK_IMAGE_USAGE_TRANSFER_DST_BIT ���Z�b�g)
	const VkImageUsageFlags ImageUsageFlags = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | (VK_IMAGE_USAGE_TRANSFER_DST_BIT & SurfaceCapabilities.supportedUsageFlags);

	//!< �T�[�t�F�X����]�A���]�����邩�ǂ����B(�\�Ȃ� VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR ��I��)
	const auto PreTransform = (SurfaceCapabilities.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR) ? VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR : SurfaceCapabilities.currentTransform;

	//!< �T�[�t�F�X�̃t�H�[�}�b�g��I��
	const auto SurfaceFormat = SelectSurfaceFormat();

	//!< �T�[�t�F�X�̃v���[���g���[�h��I��
	const auto PresentMode = SelectSurfacePresentMode();

	//!< �Z�b�e�B���O��ύX���ăX���b�v�`�F�C�����č쐬�ł���悤�ɂ��Ă����B�������J�����邽�߂� OldSwapchain �Ɋo����
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
		SwapchainImageViews.clear();

		vkDestroySwapchainKHR(Device, OldSwapchain, nullptr);
	}
}
void VK::CreateSwapchain(const VkCommandBuffer CommandBuffer)
{
	CreateSwapchainOfClientRect();
	GetSwapchainImage(CommandBuffer);
	//GetSwapchainImage(CommandBuffer, VkClearColorValue({1.0f, 0.0f, 0.0f, 1.0f}));
	CreateSwapchainImageView();

#ifdef DEBUG_STDOUT
	std::cout << "CreateSwapchain" << COUT_OK << std::endl << std::endl;
#endif
}
void VK::ResizeSwapchain(const uint32_t Width, const uint32_t Height)
{
	//!< #TODO
	if (VK_NULL_HANDLE != Device) {
		VERIFY_SUCCEEDED(vkDeviceWaitIdle(Device));
	}

	for (auto i = 0; i < CommandBuffers.size(); ++i) {
		vkFreeCommandBuffers(Device, CommandPools[i], 1, &CommandBuffers[i]);
		vkDestroyCommandPool(Device, CommandPools[i], nullptr);
	}
	CommandPools.clear();
	CommandBuffers.clear();

	CreateCommandPool(GraphicsQueueFamilyIndex);
	auto CommandPool = CommandPools[0];
	AllocateCommandBuffer(CommandPool);
	auto CommandBuffer = CommandBuffers[0];

	//CreateSwapchain(CommandBuffer);
}

void VK::GetSwapchainImage(const VkCommandBuffer CommandBuffer)
{
	//!< �X���b�v�`�F�C���C���[�W�̎擾
	uint32_t SwapchainImageCount;
	VERIFY_SUCCEEDED(vkGetSwapchainImagesKHR(Device, Swapchain, &SwapchainImageCount, nullptr));
	assert(SwapchainImageCount && "Swapchain image count == 0");
	SwapchainImages.resize(SwapchainImageCount);
	VERIFY_SUCCEEDED(vkGetSwapchainImagesKHR(Device, Swapchain, &SwapchainImageCount, SwapchainImages.data()));
#ifdef _DEBUG
	std::cout << "\t" << "SwapchainImageCount = " << SwapchainImageCount << std::endl;
#endif

	//!< VK_IMAGE_LAYOUT_PRESENT_SRC_KHR �փ��C�A�E�g�ύX���s��
	VERIFY_SUCCEEDED(vkBeginCommandBuffer(CommandBuffer, &CommandBufferBeginInfo_OneTime)); {
		for (auto& i : SwapchainImages) {
			const VkImageMemoryBarrier ImageMemoryBarrier_UndefinedToPresent = {
				VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
				nullptr,
				0,
				VK_ACCESS_MEMORY_READ_BIT,
				VK_IMAGE_LAYOUT_UNDEFINED, //!<�u���݂̃��C�A�E�g�v�܂��́uUNDEFINED�v���w�肷�邱�ƁA�C���[�W�R���e���c��ێ��������ꍇ�́uUNDEFINED�v�̓_��         
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
				1, &ImageMemoryBarrier_UndefinedToPresent);
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
	VERIFY_SUCCEEDED(vkQueueSubmit(GraphicsQueue, 1, &SubmitInfo, VK_NULL_HANDLE));
	VERIFY_SUCCEEDED(vkQueueWaitIdle(GraphicsQueue)); //!< �t�F���X�ł��ǂ�
}
void VK::GetSwapchainImage(const VkCommandBuffer CommandBuffer, const VkClearColorValue& ClearColorValue)
{
	//!< �X���b�v�`�F�C���C���[�W�̎擾
	uint32_t SwapchainImageCount;
	VERIFY_SUCCEEDED(vkGetSwapchainImagesKHR(Device, Swapchain, &SwapchainImageCount, nullptr));
	assert(SwapchainImageCount && "Swapchain image count == 0");
	SwapchainImages.resize(SwapchainImageCount);
	VERIFY_SUCCEEDED(vkGetSwapchainImagesKHR(Device, Swapchain, &SwapchainImageCount, SwapchainImages.data()));
#ifdef _DEBUG
	std::cout << "\t" << "SwapchainImageCount = " << SwapchainImageCount << std::endl;
#endif

	//!< ���C�A�E�g�ύX�Ɠh��Ԃ����s��
	VERIFY_SUCCEEDED(vkBeginCommandBuffer(CommandBuffer, &CommandBufferBeginInfo_OneTime)); {
		for (auto& i : SwapchainImages) {
			const VkImageMemoryBarrier ImageMemoryBarrier_UndefinedToTransfer = {
				VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
				nullptr,
				0,
				VK_ACCESS_TRANSFER_WRITE_BIT,
				VK_IMAGE_LAYOUT_UNDEFINED, //!<�u���݂̃��C�A�E�g�v�܂��́uUNDEFINED�v���w�肷�邱��        
				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
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
				1, &ImageMemoryBarrier_UndefinedToTransfer); 
			{
				vkCmdClearColorImage(CommandBuffer, i, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &ClearColorValue, 1, &ImageSubresourceRange_Color);
			} 
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
	VERIFY_SUCCEEDED(vkQueueSubmit(GraphicsQueue, 1, &SubmitInfo, VK_NULL_HANDLE));
	VERIFY_SUCCEEDED(vkQueueWaitIdle(GraphicsQueue)); //!< �t�F���X�ł���
}

void VK::CreateSwapchainImageView()
{
	for(auto i : SwapchainImages) {
		VkImageView ImageView;
		CreateImageView(&ImageView, i, VK_IMAGE_VIEW_TYPE_2D, ColorFormat, ComponentMapping_Identity, ImageSubresourceRange_Color);
		
		SwapchainImageViews.push_back(ImageView);
	}

#ifdef DEBUG_STDOUT
	std::cout << "\t" << "SwapchainImageIndex = " << SwapchainImageIndex << std::endl;
#endif

#ifdef DEBUG_STDOUT
	std::cout << "\t" << "SwapchainImageView" << std::endl;
#endif
}

void VK::CreateDepthStencilImage()
{
	DepthFormat = GetSupportedDepthFormat();

	const VkExtent3D Extent3D = {
		SurfaceExtent2D.width, SurfaceExtent2D.height, 1
	};
	CreateImage(&DepthStencilImage, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT, VK_IMAGE_TYPE_2D, DepthFormat, Extent3D, 1, 1);

#ifdef DEBUG_STDOUT
	std::cout << "\t" << "DepthStencilImage" << std::endl;
#endif
}
void VK::CreateDepthStencilDeviceMemory()
{
	CreateDeviceLocalMemory(&DepthStencilDeviceMemory, DepthStencilImage);

	BindDeviceMemory(DepthStencilImage, DepthStencilDeviceMemory/*, 0*/);

#ifdef DEBUG_STDOUT
	std::cout << "\t" << "DepthStencilDeviceMemory" << std::endl;
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

void VK::CreateUniformBuffer()
{
	glm::vec4 Color(1.0f, 0.0f, 0.0f, 1.0f);
	const auto Size = sizeof(Color);

	//!< ���j�t�H�[���o�b�t�@�͍X�V����̂Ńz�X�g�r�W�u���Ƃ��č쐬����
	CreateBuffer(&UniformBuffer, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, Size);
	CreateHostVisibleMemory(&UniformDeviceMemory, UniformBuffer);
	CopyToHostVisibleMemory(UniformBuffer, UniformDeviceMemory, Size, &Color/*, 0*/);
	BindDeviceMemory(UniformBuffer, UniformDeviceMemory/*, 0*/);

	const VkBufferViewCreateInfo BufferViewCreateInfo = {
		VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO,
		nullptr,
		0,
		UniformBuffer,
		VK_FORMAT_R8G8B8A8_UNORM,
		Size,
		VK_WHOLE_SIZE
	};
	VkBufferView BufferView = VK_NULL_HANDLE;
	VERIFY_SUCCEEDED(vkCreateBufferView(Device, &BufferViewCreateInfo, nullptr, &BufferView));

	const VkDescriptorBufferInfo UniformDescriptorBufferInfo = {
		UniformBuffer,
		0,
		Size
	};

	VkDescriptorBufferInfo* DescriptorBufferInfo = nullptr;
	const std::vector<VkWriteDescriptorSet> WriteDescriptorSets = {
		{
			VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
			nullptr,
			DescriptorSets[0], 0, //!< �f�X�N���v�^�Z�b�g�ƃo�C���f�B���O�|�C���g
			0,
			1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			nullptr,
			DescriptorBufferInfo,
			nullptr
		},
	};
	const std::vector<VkCopyDescriptorSet> CopyDescriptorSets = {
	};
	vkUpdateDescriptorSets(Device, 
		static_cast<uint32_t>(WriteDescriptorSets.size()), WriteDescriptorSets.data(), 
		static_cast<uint32_t>(CopyDescriptorSets.size()), CopyDescriptorSets.data());

#ifdef DEBUG_STDOUT
	std::cout << "CreateUniformBuffer" << COUT_OK << std::endl << std::endl;
#endif
}

/**
@brief �V�F�[�_�Ƃ̃o�C���f�B���O�̃��C�A�E�g
@note DescriptorSetLayout �́u�^�v�̂悤�Ȃ���
# TODO �����̎����͏����AExt�֎����Ă���
*/
void VK::CreateDescriptorSetLayout()
{
	std::vector<VkDescriptorSetLayoutBinding> DescriptorSetLayoutBindings = {
		/**
		uint32_t              binding;
		VkDescriptorType      descriptorType; ... VK_DESCRIPTOR_TYPE_[UNIFORM_BUFFER, SAMPLER, COMBINED_IMAGE_SAMPLER, SAMPLED_IMAGE, ...]
		uint32_t              descriptorCount;
		VkShaderStageFlags    stageFlags; ... VK_SHADER_STAGE_[VERTEX_BIT, TESSELLATION_CONTROL_BIT, TESSELLATION_EVALUATION_BIT, GEOMETRY_BIT, FRAGMENT_BIT, COMPUTE_BIT, ALL_GRAPHICS, ALL]
		const VkSampler*      pImmutableSamplers;
		*/
	};
	CreateDescriptorSetLayoutBindings(DescriptorSetLayoutBindings);

	const VkDescriptorSetLayoutCreateInfo DescriptorSetLayoutCreateInfo = {
		VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
		nullptr,
		0,
		static_cast<uint32_t>(DescriptorSetLayoutBindings.size()), DescriptorSetLayoutBindings.data()
	};
	VkDescriptorSetLayout DescriptorSetLayout = VK_NULL_HANDLE;
	VERIFY_SUCCEEDED(vkCreateDescriptorSetLayout(Device, &DescriptorSetLayoutCreateInfo, nullptr, &DescriptorSetLayout));
	DescriptorSetLayouts.push_back(DescriptorSetLayout);

#ifdef DEBUG_STDOUT
	std::cout << "CreateDescriptorSetLayout" << COUT_OK << std::endl << std::endl;
#endif
}
/**
@brief �V�F�[�_�Ƃ̃o�C���f�B���O�̃��C�A�E�g
@note DescriptorSet �́uDescriptorSetLayt �^�v�̃C���X�^���X�̂悤�Ȃ���
@note �X�V�� vkUpdateDescriptorSets() �ōs��
*/
void VK::CreateDescriptorSet()
{
	std::vector<VkDescriptorPoolSize> DescriptorPoolSizes = {
		/**
		VkDescriptorType    type; ... VK_DESCRIPTOR_TYPE_[SAMPLER, SAMPLED_IMAGE, UNIFORM_BUFFER, ...]
		uint32_t            descriptorCount;
		*/
	};
	CreateDescriptorPoolSizes(DescriptorPoolSizes);

	if (!DescriptorPoolSizes.empty()) {
		const uint32_t MaxSets = [&]() {
			uint32_t MaxDescriptorCount = 0;
			for (const auto& i : DescriptorPoolSizes) {
				MaxDescriptorCount = std::max(MaxDescriptorCount, i.descriptorCount);
			}
			return MaxDescriptorCount;
		}();
		const VkDescriptorPoolCreateInfo DescriptorPoolCreateInfo = {
			VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
			nullptr,
			VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT,
			MaxSets,
			static_cast<uint32_t>(DescriptorPoolSizes.size()), DescriptorPoolSizes.data()
		};
		VERIFY_SUCCEEDED(vkCreateDescriptorPool(Device, &DescriptorPoolCreateInfo, nullptr, &DescriptorPool));

		if (VK_NULL_HANDLE != DescriptorPool) {
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
}

void VK::UpdateDescriptorSet()
{
	VkDescriptorImageInfo DescriptorImageInfo;
	VkDescriptorBufferInfo DescriptorBufferInfo;
	VkBufferView BufferView;
	std::vector<VkWriteDescriptorSet> WriteDescriptorSets = {
	};
	CreateWriteDescriptorSets(WriteDescriptorSets, &DescriptorImageInfo, &DescriptorBufferInfo, &BufferView);

	std::vector<VkCopyDescriptorSet> CopyDescriptorSets = {
	};
	CreateCopyDescriptorSets(CopyDescriptorSets);

	vkUpdateDescriptorSets(Device,
		static_cast<uint32_t>(WriteDescriptorSets.size()), WriteDescriptorSets.data(),
		static_cast<uint32_t>(CopyDescriptorSets.size()), CopyDescriptorSets.data());
}

void VK::DestroyFramebuffer()
{
	for (auto i : Framebuffers) {
		vkDestroyFramebuffer(Device, i, nullptr);
	}
	Framebuffers.clear();
}

VkShaderModule VK::CreateShaderModule(const std::wstring& Path) const
{
	VkShaderModule ShaderModule = VK_NULL_HANDLE;

	std::ifstream In(Path.c_str(), std::ios::in | std::ios::binary);
	if (!In.fail()) {
		In.seekg(0, std::ios_base::end);
		const auto Size = In.tellg();
		In.seekg(0, std::ios_base::beg);

		if (Size) {
			auto Data = new char[Size];
			In.read(Data, Size);

			const VkShaderModuleCreateInfo ModuleCreateInfo = {
				VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
				nullptr,
				0,
				Size, reinterpret_cast<uint32_t*>(Data)
			};
			VERIFY_SUCCEEDED(vkCreateShaderModule(Device, &ModuleCreateInfo, nullptr, &ShaderModule));

			delete[] Data;
		}
		In.close();
	}
	return ShaderModule;
}

VkPipelineCache VK::LoadPipelineCache(const std::wstring& Path) const
{
	VkPipelineCache PipelineCache = VK_NULL_HANDLE;

	std::ifstream In(Path.c_str(), std::ios::in | std::ios::binary);
	if (!In.fail()) {
		In.seekg(0, std::ios_base::end);
		const auto Size = In.tellg();
		In.seekg(0, std::ios_base::beg);

		if (Size) {
			auto Data = new char[Size];
			In.read(Data, Size);

			const VkPipelineCacheCreateInfo PipelineCacheCreateInfo = {
				VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO,
				nullptr,
				0,
				Size, Data
			};
			VERIFY_SUCCEEDED(vkCreatePipelineCache(Device, &PipelineCacheCreateInfo, nullptr, &PipelineCache));

			delete[] Data;
		}
		In.close();
	}

	return PipelineCache;
}
void VK::StorePipelineCache(const std::wstring& Path, const VkPipelineCache PipelineCache) const
{
	if (VK_NULL_HANDLE != PipelineCache) {
		size_t Size;
		VERIFY_SUCCEEDED(vkGetPipelineCacheData(Device, PipelineCache, &Size, nullptr));
		if (Size) {
			auto Data = new char[Size];
			VERIFY_SUCCEEDED(vkGetPipelineCacheData(Device, PipelineCache, &Size, Data));

			std::ofstream Out(Path.c_str(), std::ios::out | std::ios::binary);
			if (!Out.fail()) {

				Out.write(Data, Size);

				delete[] Data;

				Out.close();
			}
		}
	}
}

//!< �p�C�v���C���L���V���̃}�[�W������ꍇ
//const std::vector<VkPipelineCache> PipelineCaches = { PipelineCache };
//VkPipelineCache DestinationPipelineCache = VK_NULL_HANDLE;
//VERIFY_SUCCEEDED(vkMergePipelineCaches(Device, DestinationPipelineCache, static_cast<uint32_t>(PipelineCaches.size()), PipelineCaches.data()));

void VK::CreateGraphicsPipeline()
{
#ifdef _DEBUG
	PerformanceCounter PC("CreateGraphicsPipeline : ");
#endif

	//!< �V�F�[�_
	std::vector<VkShaderModule> ShaderModules;
	std::vector<VkPipelineShaderStageCreateInfo> PipelineShaderStageCreateInfos;
	CreateShader(ShaderModules, PipelineShaderStageCreateInfos);

	//!< �o�[�e�b�N�X�C���v�b�g
	std::vector<VkVertexInputBindingDescription> VertexInputBindingDescriptions;
	std::vector<VkVertexInputAttributeDescription> VertexInputAttributeDescriptions;
	CreateVertexInput(VertexInputBindingDescriptions, VertexInputAttributeDescriptions, 0);
	const VkPipelineVertexInputStateCreateInfo PipelineVertexInputStateCreateInfo = {
		VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
		nullptr,
		0,
		static_cast<uint32_t>(VertexInputBindingDescriptions.size()), VertexInputBindingDescriptions.data(),
		static_cast<uint32_t>(VertexInputAttributeDescriptions.size()), VertexInputAttributeDescriptions.data()
	};

	//!< �C���v�b�g�A�Z���u�� (�g�|���W)
	VkPipelineInputAssemblyStateCreateInfo PipelineInputAssemblyStateCreateInfo = {
		VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
		nullptr,
		0,
		VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP,
		VK_FALSE
	};
	CreateInputAssembly(PipelineInputAssemblyStateCreateInfo);

	//!< VkDynamicState �ɂ���̂ŁA�����ł� nullptr ���w�肵�Ă���A���������͎w�肵�Ă����K�v������̂Œ���!
	const VkPipelineViewportStateCreateInfo PipelineViewportStateCreateInfo = {
		VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
		nullptr,
		0,
		1, nullptr,
		1, nullptr
	};

	const VkPipelineRasterizationStateCreateInfo PipelineRasterizationStateCreateInfo = {
		VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
		nullptr,
		0,
		VK_FALSE,
		VK_FALSE,
		VK_POLYGON_MODE_FILL,
		VK_CULL_MODE_BACK_BIT,
		VK_FRONT_FACE_COUNTER_CLOCKWISE,
		VK_FALSE, 0.0f, 0.0f, 0.0f,
		1.0f
	};

	const VkPipelineMultisampleStateCreateInfo PipelineMultisampleStateCreateInfo = {
		VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
		nullptr,
		0,
		VK_SAMPLE_COUNT_1_BIT,
		VK_FALSE, 0.0f,
		nullptr,
		VK_FALSE, VK_FALSE
	};

	const VkStencilOpState StencilOpState_Front = {
		VK_STENCIL_OP_KEEP,
		VK_STENCIL_OP_KEEP,
		VK_STENCIL_OP_KEEP,
		VK_COMPARE_OP_NEVER, 0, 0, 0
	};
	const VkStencilOpState StencilOpState_Back = {
		VK_STENCIL_OP_KEEP,
		VK_STENCIL_OP_KEEP,
		VK_STENCIL_OP_KEEP,
		VK_COMPARE_OP_ALWAYS, 0, 0, 0
	};
	const VkPipelineDepthStencilStateCreateInfo PipelineDepthStencilStateCreateInfo = {
		VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
		nullptr,
		0,
		VK_TRUE,
		VK_TRUE,
		VK_COMPARE_OP_LESS_OR_EQUAL,
		VK_FALSE,
		VK_FALSE,
		StencilOpState_Front,
		StencilOpState_Back,
		0.0f, 0.0f
	};

	//!< (��{) �z��ɂȂ��Ă���ɂ��ւ�炸�A�S�v�f�����l�łȂ��ƃ_��
	//!< �v�f���ɈقȂ�ݒ���������ꍇ�́A�f�o�C�X�ŗL���ɂȂ��Ă��Ȃ��ƃ_�� (VkPhysicalDeviceFeatures.independentBlend)
	const std::vector<VkPipelineColorBlendAttachmentState> PipelineColorBlendAttachmentStates = {
		{
			VK_FALSE,
			VK_BLEND_FACTOR_ZERO, VK_BLEND_FACTOR_ZERO,
			VK_BLEND_OP_ADD,
			VK_BLEND_FACTOR_ZERO, VK_BLEND_FACTOR_ZERO,
			VK_BLEND_OP_ADD,
			VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
		}
	};
	const VkPipelineColorBlendStateCreateInfo PipelineColorBlendStateCreateInfo = {
		VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
		nullptr,
		0,
		VK_FALSE, VK_LOGIC_OP_CLEAR,
		static_cast<uint32_t>(PipelineColorBlendAttachmentStates.size()), PipelineColorBlendAttachmentStates.data(),
		{ 0.0f, 0.0f, 0.0f, 0.0f } //!< float blendConstants[4];
	};

	//!< DirectX12 �ɍ��킹��ׁAViewport �� Scissor �� VkDynamicState �Ƃ���
	const std::vector<VkDynamicState> DynamicStates = {
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR
	};
	const VkPipelineDynamicStateCreateInfo PipelineDynamicStateCreateInfo = {
		VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
		nullptr,
		0,
		static_cast<uint32_t>(DynamicStates.size()), DynamicStates.data()
	};

	//!< �p�C�v���C�����C�A�E�g
	const std::vector<VkPushConstantRange> PushConstantRanges = {
	};
	const VkPipelineLayoutCreateInfo PipelineLayoutCreateInfo = {
		VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
		nullptr,
		0,
		static_cast<uint32_t>(DescriptorSetLayouts.size()), DescriptorSetLayouts.data(),
		static_cast<uint32_t>(PushConstantRanges.size()), PushConstantRanges.data()
	};
	VERIFY_SUCCEEDED(vkCreatePipelineLayout(Device, &PipelineLayoutCreateInfo, nullptr, &PipelineLayout));
	//!< PipelineLayout ���쐬������ADescritptorSetLayout �͔j�����Ă��ǂ��B(DescriptorSet ���č쐬����ꍇ�ɕK�v�ɂȂ�̂łƂ��Ă����ׂ����H)
	//for (auto i : DescriptorSetLayouts) {
	//	vkDestroyDescriptorSetLayout(Device, i, nullptr);
	//}
	//DescriptorSetLayouts.clear();

	/**
	basePipelineHandle �� basePipelineIndex �͓����Ɏg�p�ł��Ȃ�(�r��)
	�e�ɂ� VK_PIPELINE_CREATE_ALLOW_DERIVATIVES_BIT �t���O���K�v�A�q�ɂ� VK_PIPELINE_CREATE_DERIVATIVE_BIT �t���O���K�v

	�EbasePipelineHandle ... ���ɑ��݂���ꍇ�A�e�p�C�v���C�����w��
	�EbasePipelineIndex ... GraphicsPipelineCreateInfos �z��Őe�p�C�v���C���������ɍ쐬����ꍇ�A�z����ł̐e�p�C�v���C���̓Y���B�e�̓Y���̕����Ⴂ�l�łȂ��Ƃ����Ȃ��B
	*/
	const std::vector<VkGraphicsPipelineCreateInfo> GraphicsPipelineCreateInfos = {
		{
			VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
			nullptr,
#ifdef _DEBUG
			VK_PIPELINE_CREATE_DISABLE_OPTIMIZATION_BIT/*| VK_PIPELINE_CREATE_ALLOW_DERIVATIVES_BIT*/,
#else
			0/*| VK_PIPELINE_CREATE_ALLOW_DERIVATIVES_BIT*/,
#endif
			static_cast<uint32_t>(PipelineShaderStageCreateInfos.size()), PipelineShaderStageCreateInfos.data(),
			&PipelineVertexInputStateCreateInfo,
			&PipelineInputAssemblyStateCreateInfo,
			nullptr,
			&PipelineViewportStateCreateInfo,
			&PipelineRasterizationStateCreateInfo,
			&PipelineMultisampleStateCreateInfo,
			&PipelineDepthStencilStateCreateInfo,
			&PipelineColorBlendStateCreateInfo,
			&PipelineDynamicStateCreateInfo,
			PipelineLayout,
			RenderPass,
			0,
			VK_NULL_HANDLE, 0 //!< basePipelineHandle, basePipelineIndex
		}
	};

	//!< �p�C�v���C���L���b�V�� (�t�@�C������ǂݍ���)
	const auto BasePath = GetBasePath();
	auto PipelineCache = LoadPipelineCache(BasePath + L".pco");
	if (VK_NULL_HANDLE == PipelineCache) {
#ifdef _DEBUG
		std::cout << "LoadPipelineCache" << COUT_NG << std::endl << std::endl;
#endif
		const VkPipelineCacheCreateInfo PipelineCacheCreateInfo = {
			VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO,
			nullptr,
			0,
			0, nullptr
		};
		//!< �ǂݍ��߂Ȃ������̂ō쐬
		VERIFY_SUCCEEDED(vkCreatePipelineCache(Device, &PipelineCacheCreateInfo, nullptr, &PipelineCache));
	}
	else {
#ifdef _DEBUG
		std::cout << "LoadPipelineCache" << COUT_OK << std::endl << std::endl;
#endif
	}

	//!< (�O���t�B�b�N)�p�C�v���C��
	VERIFY_SUCCEEDED(vkCreateGraphicsPipelines(Device, PipelineCache, static_cast<uint32_t>(GraphicsPipelineCreateInfos.size()), GraphicsPipelineCreateInfos.data(), nullptr, &Pipeline));

	//!< �p�C�v���C�� ���쐬������A�V�F�[�_���W���[�� �͔j�����ėǂ�
	for (auto i : ShaderModules) {
		vkDestroyShaderModule(Device, i, nullptr);
	}
	ShaderModules.clear();

	//!< �p�C�v���C�� ���쐬������A�p�C�v���C�����C�A�E�g �͔j�����ėǂ� �� �_�� vkCmdBindDescriptorSets() ���ň����Ɏ�鎖������
	//if (VK_NULL_HANDLE != PipelineLayout) {
	//	vkDestroyPipelineLayout(Device, PipelineLayout, nullptr);
	//	PipelineLayout = VK_NULL_HANDLE;
	//}

	//!< �p�C�v���C���L���b�V�����t�@�C���֕ۑ�
	if (VK_NULL_HANDLE != PipelineCache) {
		const auto BasePath = GetBasePath();
		StorePipelineCache(BasePath + L".pco", PipelineCache);

		vkDestroyPipelineCache(Device, PipelineCache, nullptr);
		PipelineCache = VK_NULL_HANDLE;
	}

#ifdef DEBUG_STDOUT
	std::cout << "CreateGraphicsPipeline" << COUT_OK << std::endl << std::endl;
#endif
}
void VK::CreateComputePipeline()
{
	std::vector<VkShaderModule> ShaderModules;
	{
		const auto ShaderPath = GetBasePath();
		ShaderModules.push_back(CreateShaderModule((ShaderPath + L".comp.spv").data()));

		//#TODO
	}
	for (auto i : ShaderModules) {
		vkDestroyShaderModule(Device, i, nullptr);
	}

#ifdef DEBUG_STDOUT
	std::cout << "CreateComputePipeline" << COUT_OK << std::endl << std::endl;
#endif
}

void VK::PopulateCommandBuffer(const VkCommandBuffer CommandBuffer)
{
	//!< VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT ... ���������R�}���h�v�[���֕Ԃ�
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
	const VkCommandBufferBeginInfo CommandBufferBeginInfo = {
		VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		nullptr,
#if 1
		VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, //!< ����j�� or ���Z�b�g���s����ꍇ
#else
		VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT, //!< �O��̃T�u�~�b�g���������Ă��Ȃ��Ă��A�ēx�T�u�~�b�g���꓾��ꍇ
#endif
		nullptr//&CommandBufferInheritanceInfo
	};
	VERIFY_SUCCEEDED(vkBeginCommandBuffer(CommandBuffer, &CommandBufferBeginInfo)); {
		//!< �r���[�|�[�g�A�V�U�[
		vkCmdSetViewport(CommandBuffer, 0, static_cast<uint32_t>(Viewports.size()), Viewports.data());
		vkCmdSetScissor(CommandBuffer, 0, static_cast<uint32_t>(ScissorRects.size()), ScissorRects.data());

		auto Image = SwapchainImages[SwapchainImageIndex];

		//!< �N���A
		vkCmdClearColorImage(CommandBuffer, Image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &Colors::SkyBlue, 1, &ImageSubresourceRange_Color);

#ifdef _DEBUG
		//!< �����_�[�G���A�̍Œᗱ�x���m��
		VkExtent2D Granularity;
		vkGetRenderAreaGranularity(Device, RenderPass, &Granularity);
		assert(ScissorRects[0].extent.width >= Granularity.width && ScissorRects[0].extent.height >= Granularity.height && "ScissorRect is too small");
#endif
		//!< �o���A�̐ݒ�� RenderPass
		VkClearValue ClearValue = { Colors::SkyBlue };
		const VkRenderPassBeginInfo RenderPassBeginInfo = {
			VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
			nullptr,
			RenderPass,
			Framebuffers[SwapchainImageIndex],
			ScissorRects[0],
			0, nullptr//!< 1, &ClearValue
		};
		vkCmdBeginRenderPass(CommandBuffer, &RenderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE); {
		} vkCmdEndRenderPass(CommandBuffer);
	} VERIFY_SUCCEEDED(vkEndCommandBuffer(CommandBuffer));
}
void VK::Draw()
{
	//!< ���̃C���[�W���擾�ł�����Z�}�t�H���ʒm�����
	VERIFY_SUCCEEDED(vkAcquireNextImageKHR(Device, Swapchain, UINT64_MAX, NextImageAcquiredSemaphore, nullptr, &SwapchainImageIndex));

	auto CommandPool = CommandPools[0];
	VERIFY_SUCCEEDED(vkResetCommandPool(Device, CommandPool, VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT));

	auto CommandBuffer = CommandBuffers[0];
	PopulateCommandBuffer(CommandBuffer);

	VERIFY_SUCCEEDED(vkDeviceWaitIdle(Device));
	const VkPipelineStageFlags PipelineStageFlags = VK_PIPELINE_STAGE_TRANSFER_BIT;
	const VkSubmitInfo SubmitInfo = {
		VK_STRUCTURE_TYPE_SUBMIT_INFO,
		nullptr,
		1, &NextImageAcquiredSemaphore, &PipelineStageFlags,	//!< ���C���[�W���擾�ł���(�v���[���g����)�܂ŃE�G�C�g
		1,  &CommandBuffer,
		1, &RenderFinishedSemaphore								//!< �`�抮����ʒm����
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
		1, &RenderFinishedSemaphore, //!< �`�悪��������܂ő҂�
		1, &Swapchain, &SwapchainImageIndex,
		nullptr
	};
	VERIFY_SUCCEEDED(vkQueuePresentKHR(GraphicsQueue, &PresentInfo));

#ifdef DEBUG_STDOUT
	//std::cout << "\t" << "SwapchainImageIndex = " << SwapchainImageIndex << std::endl;
#endif
}
void VK::WaitForFence()
{
	VkResult Result;
	const uint64_t TimeOut = 100000000;
	do {
		//!< ���\�[�X�g�p�\�̃V�O�i���܂ő҂�
		Result = vkWaitForFences(Device, 1, &Fence, VK_TRUE, TimeOut);
		if (VK_TIMEOUT == Result) {
#ifdef _DEBUG
			std::cout << "TIMEOUT" << std::endl;
#endif
		}
	} while (VK_SUCCESS != Result);
	VERIFY_SUCCEEDED(Result);

	//!< �V�O�i�������Z�b�g
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
(����̂�)RenderDoc ���N���AWarning ���o�Ă�����N���b�N���� VulkanCapture ��L���ɂ��Ă������ƁAWindows �̃��W�X�g�����쐬�����
RenderDoc ������s�����ꍇ�ɂ��� VK_EXT_DEBUG_MARKER_EXTENSION_NAME �͗L���ɂȂ�Ȃ��̂Œ���
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


