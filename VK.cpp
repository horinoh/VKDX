#include "stdafx.h"

#include <fstream>

#include "VK.h"

#ifdef _WINDOWS
#pragma comment(lib, "vulkan-1.lib")
#else
// "libvulkan.so.1"
#endif

#ifdef VK_NO_PROTOYYPES
	//!< �O���[�o�����x���֐� Global level functions
#define VK_GLOBAL_PROC_ADDR(proc) PFN_vk ## proc VK::vk ## proc = VK_NULL_HANDLE;
#include "VKGlobalProcAddr.h"
#undef VK_GLOBAL_PROC_ADDR

	//!< �C���X�^���X���x���֐� Instance level functions
#define VK_INSTANCE_PROC_ADDR(proc) PFN_vk ## proc VK::vk ## proc = VK_NULL_HANDLE;
#include "VKInstanceProcAddr.h"
#undef VK_INSTANCE_PROC_ADDR

	//!< �f�o�C�X���x���֐� Device level functions
#define VK_DEVICE_PROC_ADDR(proc) PFN_vk ## proc VK::vk ## proc = VK_NULL_HANDLE;
#include "VKDeviceProcAddr.h"
#undef VK_DEVICE_PROC_ADDR
#endif //!< VK_NO_PROTOYYPES

#ifdef _DEBUG
	//!< �C���X�^���X���x���֐�(Debug) Instance level functions(Debug)
#define VK_INSTANCE_PROC_ADDR(proc) PFN_vk ## proc ## EXT VK::vk ## proc = VK_NULL_HANDLE;
#include "VKDebugReport.h"
#undef VK_INSTANCE_PROC_ADDR

	//!< �f�o�C�X���x���֐�(Debug) Device level functions(Debug)
#define VK_DEVICE_PROC_ADDR(proc) PFN_vk ## proc ## EXT VK::vk ## proc = VK_NULL_HANDLE;
#include "VKDebugMarker.h"
#undef VK_DEVICE_PROC_ADDR
#endif //!< _DEBUG

#ifdef _WINDOWS
void VK::OnCreate(HWND hWnd, HINSTANCE hInstance, LPCWSTR Title)
{
#ifdef _DEBUG
	PerformanceCounter PC("OnCreate : ");
#endif

	Super::OnCreate(hWnd, hInstance, Title);

#ifdef VK_NO_PROTOYYPES
	LoadVulkanLibrary();
#endif

	CreateInstance();
	CreateSurface(hWnd, hInstance);
	EnumeratePhysicalDevice(Instance);
	CreateDevice(GetCurrentPhysicalDevice(), Surface);
	
	CreateFence();
	CreateSemaphore();

	CreateSwapchain(GetCurrentPhysicalDevice(), Surface);
	CreateCommandBuffer();
	InitializeSwapchain();

	CreateDepthStencil();

	CreateVertexBuffer();
	CreateIndexBuffer();
	CreateIndirectBuffer();

	CreateTexture();

	CreateDescriptorSetLayout();
	{
		CreateUniformBuffer();
	}
	CreateDescriptorSet();
	UpdateDescriptorSet();

	CreatePushConstantRanges();

	CreateRenderPass();
	CreateFramebuffer();

	CreatePipeline();

	SetTimer(hWnd, NULL, 1000 / 60, nullptr);
}

/**
@note �w�ǂ̂��̂��󂵂č�蒼���Ȃ��ƃ_�� #VK_TODO
almost every thing must be recreated

LEARNING VULKAN : p367
need to destroy and recreate the framebuffer,
command pool, graphics pipeline, Render Pass, depth buffer image, image view, vertex
buffer, and so on.
*/
void VK::OnSize(HWND hWnd, HINSTANCE hInstance)
{
#ifdef _DEBUG
	PerformanceCounter PC("OnSize : ");
#endif

	Super::OnSize(hWnd, hInstance);

	//ResizeSwapChain(Rect);

	CreateViewport(static_cast<float>(SurfaceExtent2D.width), static_cast<float>(SurfaceExtent2D.height));

	//DestroyFramebuffer();
	//CreateFramebuffer();

	for (auto i = 0; i < CommandBuffers.size(); ++i) {
		PopulateCommandBuffer(i);
	}
}
void VK::OnDestroy(HWND hWnd, HINSTANCE hInstance)
{
	Super::OnDestroy(hWnd, hInstance);

	if (VK_NULL_HANDLE != Device) {
		VERIFY_SUCCEEDED(vkDeviceWaitIdle(Device));
	}

	DestroyFramebuffer();

	if (VK_NULL_HANDLE != RenderPass) {
		vkDestroyRenderPass(Device, RenderPass, GetAllocationCallbacks());
		RenderPass = VK_NULL_HANDLE;
	}
	if (VK_NULL_HANDLE != Pipeline) {
		vkDestroyPipeline(Device, Pipeline, GetAllocationCallbacks());
		Pipeline = VK_NULL_HANDLE;
	}

	if (VK_NULL_HANDLE != PipelineLayout) {
		vkDestroyPipelineLayout(Device, PipelineLayout, GetAllocationCallbacks());
	}

	//!< VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT �̏ꍇ�̂݌ʂɊJ���ł��� If only VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT is used, can be release individually
#if 0
	if (!DescriptorSets.empty()) {
		vkFreeDescriptorSets(Device, DescriptorPool, static_cast<uint32_t>(DescriptorSets.size()), DescriptorSets.data());
	}
#endif
	DescriptorSets.clear();
	if (VK_NULL_HANDLE != DescriptorPool) {
		vkDestroyDescriptorPool(Device, DescriptorPool, GetAllocationCallbacks());
		DescriptorPool = VK_NULL_HANDLE;
	}
	for (auto i : DescriptorSetLayouts) {
		vkDestroyDescriptorSetLayout(Device, i, GetAllocationCallbacks());
	}
	DescriptorSetLayouts.clear();

	if (VK_NULL_HANDLE != UniformDeviceMemory) {
		vkFreeMemory(Device, UniformDeviceMemory, GetAllocationCallbacks());
		UniformDeviceMemory = VK_NULL_HANDLE;
	}
	if (VK_NULL_HANDLE != UniformBuffer) {
		vkDestroyBuffer(Device, UniformBuffer, GetAllocationCallbacks());
		UniformBuffer = VK_NULL_HANDLE;
	}

	if (VK_NULL_HANDLE != IndirectDeviceMemory) {
		vkFreeMemory(Device, IndirectDeviceMemory, GetAllocationCallbacks());
		IndirectDeviceMemory = VK_NULL_HANDLE;
	}
	if (VK_NULL_HANDLE != IndirectBuffer) {
		vkDestroyBuffer(Device, IndirectBuffer, GetAllocationCallbacks());
		IndirectBuffer = VK_NULL_HANDLE;
	}
	if (VK_NULL_HANDLE != IndexDeviceMemory) {
		vkFreeMemory(Device, IndexDeviceMemory, GetAllocationCallbacks());
		IndexDeviceMemory = VK_NULL_HANDLE;
	}
	if (VK_NULL_HANDLE != IndexBuffer) {
		vkDestroyBuffer(Device, IndexBuffer, GetAllocationCallbacks());
		IndexBuffer = VK_NULL_HANDLE;
	}
	if (VK_NULL_HANDLE != VertexDeviceMemory) {
		vkFreeMemory(Device, VertexDeviceMemory, GetAllocationCallbacks());
		VertexDeviceMemory = VK_NULL_HANDLE;
	}
	if (VK_NULL_HANDLE != VertexBuffer) {
		vkDestroyBuffer(Device, VertexBuffer, GetAllocationCallbacks());
		VertexBuffer = VK_NULL_HANDLE;
	}

	for (auto i : Samplers) {
		vkDestroySampler(Device, i, GetAllocationCallbacks());
	}

	if (VK_NULL_HANDLE != ImageView) {
		vkDestroyImageView(Device, ImageView, GetAllocationCallbacks());
		ImageView = VK_NULL_HANDLE;
	}
	if (VK_NULL_HANDLE != ImageDeviceMemory) {
		vkFreeMemory(Device, ImageDeviceMemory, GetAllocationCallbacks());
		ImageDeviceMemory = VK_NULL_HANDLE;
	}
	if (VK_NULL_HANDLE != Image) {
		vkDestroyImage(Device, Image, GetAllocationCallbacks());
		Image = VK_NULL_HANDLE;
	}

	if (VK_NULL_HANDLE != DepthStencilImageView) {
		vkDestroyImageView(Device, DepthStencilImageView, GetAllocationCallbacks());
		DepthStencilImageView = VK_NULL_HANDLE;
	}
	if (VK_NULL_HANDLE != DepthStencilDeviceMemory) {
		vkFreeMemory(Device, DepthStencilDeviceMemory, GetAllocationCallbacks());
		DepthStencilDeviceMemory = VK_NULL_HANDLE;
	}
	if (VK_NULL_HANDLE != DepthStencilImage) {
		vkDestroyImage(Device, DepthStencilImage, GetAllocationCallbacks());
		DepthStencilImage = VK_NULL_HANDLE;
	}

	for (auto i : SwapchainImageViews) {
		vkDestroyImageView(Device, i, GetAllocationCallbacks());
	}
	SwapchainImageViews.clear();

	//!< SwapchainImages �͎擾�������́A�j�����Ȃ�
	
	if (VK_NULL_HANDLE != Swapchain) {
		vkDestroySwapchainKHR(Device, Swapchain, GetAllocationCallbacks());
		Swapchain = VK_NULL_HANDLE;
	}

	if (VK_NULL_HANDLE != CommandPool) {
		const auto CP = CommandPool;
		auto& CB = CommandBuffers;

		if (!CB.empty()) {
			vkFreeCommandBuffers(Device, CP, static_cast<uint32_t>(CB.size()), CB.data());
			CB.clear();
		}
		vkDestroyCommandPool(Device, CP, GetAllocationCallbacks());
	}
	if (VK_NULL_HANDLE != ComputeCommandPool) {
		const auto CP = ComputeCommandPool;
		auto& CB = ComputeCommandBuffers;

		if (!CB.empty()) {
			vkFreeCommandBuffers(Device, CP, static_cast<uint32_t>(CB.size()), CB.data());
			CB.clear();
		}
		vkDestroyCommandPool(Device, CP, GetAllocationCallbacks());
	}
	
	if (VK_NULL_HANDLE != RenderFinishedSemaphore) {
		vkDestroySemaphore(Device, RenderFinishedSemaphore, GetAllocationCallbacks());
		RenderFinishedSemaphore = VK_NULL_HANDLE;
	}
	if (VK_NULL_HANDLE != NextImageAcquiredSemaphore) {
		vkDestroySemaphore(Device, NextImageAcquiredSemaphore, GetAllocationCallbacks());
		NextImageAcquiredSemaphore = VK_NULL_HANDLE;
	}
	if (VK_NULL_HANDLE != Fence) {
		vkDestroyFence(Device, Fence, GetAllocationCallbacks());
		Fence = VK_NULL_HANDLE;
	}
	if (VK_NULL_HANDLE != ComputeFence) {
		vkDestroyFence(Device, ComputeFence, GetAllocationCallbacks());
		ComputeFence = VK_NULL_HANDLE;
	}

	//!< �L���[�͘_���f�o�C�X�Ƌ��ɔj�������
	if (VK_NULL_HANDLE != Device) {
		vkDestroyDevice(Device, GetAllocationCallbacks());
		Device = VK_NULL_HANDLE;
	}
	
	//!< PhysicalDevice �� vkEnumeratePhysicalDevices() �Ŏ擾�������́A�j�����Ȃ�

	if (VK_NULL_HANDLE != Surface) {
		vkDestroySurfaceKHR(Instance, Surface, GetAllocationCallbacks());
		Surface = VK_NULL_HANDLE;
	}

#ifdef _DEBUG
	if (VK_NULL_HANDLE != DebugReportCallback) {
		vkDestroyDebugReportCallback(Instance, DebugReportCallback, nullptr);
		DebugReportCallback = VK_NULL_HANDLE;
	}
#endif

	if (VK_NULL_HANDLE != Instance) {
		vkDestroyInstance(Instance, GetAllocationCallbacks());
		Instance = VK_NULL_HANDLE;
	}

#ifdef VK_NO_PROTOYYPES
	if (
#ifdef _WINDOWS
		!FreeLibrary(VulkanLibrary)
#else
		!dlclose(VulkanLibrary)
#endif
		) {
		assert(false && "FreeLibrary failed");
		VulkanLibrary = nullptr;
	}
#endif
}
#endif //!< _WINDOWS

std::string VK::GetVkResultString(const VkResult Result)
{
#define VK_RESULT_ENTRY(vr) case VK_##vr: return #vr;
	switch (Result)
	{
	default: DEBUG_BREAK(); return "Not found";
#include "VkResult.h"
	}
#undef VK_RESULT_ENTRY
}
std::string VK::GetFormatString(const VkFormat Format)
{
#define VK_FORMAT_ENTRY(vf) case VK_FORMAT_##vf: return #vf;
	switch (Format)
	{
	default: DEBUG_BREAK(); return "Not found";
#include "VKFormat.h"
	}
#undef VK_FORMAT_ENTRY
}

std::string VK::GetColorSpaceString(const VkColorSpaceKHR ColorSpace)
{
#define VK_COLOR_SPACE_ENTRY(vcs) case VK_COLOR_SPACE_##vcs: return #vcs;
	switch (ColorSpace)
	{
	default: DEBUG_BREAK(); return "Not found";
#include "VKColorSpace.h"
	}
#undef VK_COLOR_SPACE_ENTRY
}

std::string VK::GetImageViewTypeString(const VkImageViewType ImageViewType)
{
#define VK_IMAGE_VIEW_TYPE_ENTRY(vivt) case VK_IMAGE_VIEW_TYPE_##vivt: return #vivt;
	switch (ImageViewType)
	{
	default: DEBUG_BREAK(); return "Not found";
#include "VKImageViewType.h"
	}
#undef VK_IMAGE_VIEW_TYPE_ENTRY
}

std::string VK::GetComponentSwizzleString(const VkComponentSwizzle ComponentSwizzle)
{
#define VK_COMPONENT_SWIZZLE_ENTRY(vcs) case VK_COMPONENT_SWIZZLE_##vcs: return #vcs;
	switch (ComponentSwizzle)
	{
	default: DEBUG_BREAK(); return "Not found";
#include "VKComponentSwizzle.h"
	}
#undef VK_COMPONENT_SWIZZLE_ENTRY
}

bool VK::IsSupportedDepthFormat(VkPhysicalDevice PhysicalDevice, const VkFormat DepthFormat)
{
	VkFormatProperties FormatProperties;
	vkGetPhysicalDeviceFormatProperties(PhysicalDevice, DepthFormat, &FormatProperties);
	if (FormatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) {
		return true;
	}
	return false;
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
	VERIFY_SUCCEEDED(vkCreateBuffer(Device, &BufferCreateInfo, GetAllocationCallbacks(), Buffer));
}

void VK::CreateImage(VkImage* Image, const VkImageUsageFlags Usage, const VkSampleCountFlagBits SampleCount, const VkImageType ImageType, const VkFormat Format, const VkExtent3D& Extent3D, const uint32_t MipLevels, const uint32_t ArrayLayers) const
{
	ValidateFormatProperties(Usage, Format);

	const VkImageCreateInfo ImageCreateInfo = {
		VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
		nullptr,
		0,
		ImageType,
		Format,
		Extent3D,
		MipLevels,
		ArrayLayers,
		SampleCount, //!< �L���[�u�}�b�v�̏ꍇ�� VK_SAMPLE_COUNT_1_BIT �����g���Ȃ�
		VK_IMAGE_TILING_OPTIMAL,
		Usage,
		VK_SHARING_MODE_EXCLUSIVE,
		0, nullptr,
		VK_IMAGE_LAYOUT_UNDEFINED
	};
	VERIFY_SUCCEEDED(vkCreateImage(Device, &ImageCreateInfo, GetAllocationCallbacks(), Image));
}

void VK::CopyToHostVisibleMemory(const VkDeviceMemory DeviceMemory, const size_t Size, const void* Source, const VkDeviceSize Offset)
{
	if (Size && nullptr != Source) {
		void *Data;
		VERIFY_SUCCEEDED(vkMapMemory(Device, DeviceMemory, Offset, Size, static_cast<VkMemoryMapFlags>(0), &Data)); {
			memcpy(Data, Source, Size);

			const std::vector<VkMappedMemoryRange> MappedMemoryRanges = {
				{
					VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE,
					nullptr,
					DeviceMemory,
					0,
					VK_WHOLE_SIZE
				}
			};
			//!< ���z�X�g������ɂ��邽�߂ɖ��������� (�����Ă��������ǕK�v�H)
			//VERIFY_SUCCEEDED(vkInvalidateMappedMemoryRanges(Device, static_cast<uint32_t>(MappedMemoryRanges.size()), MappedMemoryRanges.data()));

			//!< �t���b�V�����Ă����Ȃ��ƁA�T�u�~�b�g���ꂽ���̃R�}���h����͂����ɂ͌����Ȃ� (VK_MEMORY_PROPERTY_HOST_COHERENT_BIT �̏ꍇ�͕K�v�Ȃ�)
			VERIFY_SUCCEEDED(vkFlushMappedMemoryRanges(Device, static_cast<uint32_t>(MappedMemoryRanges.size()), MappedMemoryRanges.data()));
		} vkUnmapMemory(Device, DeviceMemory);
	}
}
void VK::SubmitCopyBuffer(const VkCommandBuffer CB, const VkBuffer Src, const VkBuffer Dst, const VkAccessFlags AccessFlag, const VkPipelineStageFlagBits PipelineStageFlag, const size_t Size)
{
	VERIFY_SUCCEEDED(vkBeginCommandBuffer(CB, &CommandBufferBeginInfo_OneTime)); {
		const VkBufferCopy BufferCopy = { 0, 0, Size };
		vkCmdCopyBuffer(CB, Src, Dst, 1, &BufferCopy);

		const VkBufferMemoryBarrier BufferMemoryBarrier = {
			VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
			nullptr,
			VK_ACCESS_MEMORY_WRITE_BIT,
			AccessFlag,
			VK_QUEUE_FAMILY_IGNORED,
			VK_QUEUE_FAMILY_IGNORED,
			Dst,
			0,
			VK_WHOLE_SIZE
		};
		//!< �o�b�t�@���u�]����(VK_PIPELINE_STAGE_TRANSFER_BIT)�v����u�ړI�̃o�b�t�@(PipelineStageFlag(�o�[�e�b�N�X�o�b�t�@��))�v�֕ύX����
		vkCmdPipelineBarrier(CB, 
			VK_PIPELINE_STAGE_TRANSFER_BIT, PipelineStageFlag,
			0, 
			0, nullptr,
			1, &BufferMemoryBarrier, 
			0, nullptr);

	} VERIFY_SUCCEEDED(vkEndCommandBuffer(CB));

	//!< �T�u�~�b�g
	const std::vector<VkSubmitInfo> SubmitInfos = {
		{
			VK_STRUCTURE_TYPE_SUBMIT_INFO,
			nullptr,
			0, nullptr,
			nullptr,
			1, &CB,
			0, nullptr
		}
	};
	VERIFY_SUCCEEDED(vkQueueSubmit(GraphicsQueue, static_cast<uint32_t>(SubmitInfos.size()), SubmitInfos.data(), VK_NULL_HANDLE));
	VERIFY_SUCCEEDED(vkQueueWaitIdle(GraphicsQueue));
}

void VK::CreateHostVisibleBufferMemory(VkDeviceMemory* DeviceMemory, const VkBuffer Object)
{
	VkMemoryRequirements MemoryRequirements;
	//!< MemoryRequirements �̎擾�̎d�����o�b�t�@���ɈقȂ�̂Ńe���v���[�g�����Ă���
	//!< Because vkGetImageMemoryRequirements is different, using template specialization
	vkGetBufferMemoryRequirements(Device, Object, &MemoryRequirements);
	const VkMemoryAllocateInfo MemoryAllocateInfo = {
		VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
		nullptr,
		MemoryRequirements.size,
		GetMemoryType(MemoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT/*| VK_MEMORY_PROPERTY_HOST_COHERENT_BIT*/) //!< HOST_VISIBLE �ɂ��邱�� Must be HOST_VISIBLE
	};
	VERIFY_SUCCEEDED(vkAllocateMemory(Device, &MemoryAllocateInfo, nullptr, DeviceMemory));
}

void VK::CreateDeviceLocalBufferMemory(VkDeviceMemory* DeviceMemory, const VkBuffer Object)
{
	VkMemoryRequirements MemoryRequirements;
	//!< MemoryRequirements �̎擾�̎d�����o�b�t�@���ɈقȂ�̂Ńe���v���[�g�����Ă���
	//!< Because vkGetImageMemoryRequirements is different, using template specialization
	vkGetBufferMemoryRequirements(Device, Object, &MemoryRequirements);
	const VkMemoryAllocateInfo MemoryAllocateInfo = {
		VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
		nullptr,
		MemoryRequirements.size,
		GetMemoryType(MemoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
	};
	VERIFY_SUCCEEDED(vkAllocateMemory(Device, &MemoryAllocateInfo, nullptr, DeviceMemory));
}

void VK::CreateHostVisibleImageMemory(VkDeviceMemory* DeviceMemory, const VkImage Object)
{
	assert(false && "Not implemented");
}

void VK::CreateDeviceLocalImageMemory(VkDeviceMemory* DeviceMemory, const VkImage Object)
{
	VkMemoryRequirements MemoryRequirements;
	//!< MemoryRequirements �̎擾�̎d�����o�b�t�@���ɈقȂ�̂Ńe���v���[�g�����Ă���
	//!< Because vkGetImageMemoryRequirements is different, using template specialization
	vkGetImageMemoryRequirements(Device, Object, &MemoryRequirements);
	const VkMemoryAllocateInfo MemoryAllocateInfo = {
		VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
		nullptr,
		MemoryRequirements.size,
		GetMemoryType(MemoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
	};
	VERIFY_SUCCEEDED(vkAllocateMemory(Device, &MemoryAllocateInfo, nullptr, DeviceMemory));
}

//!< Cubemap�����ꍇ�A�܂����C�������ꂽ�C���[�W���쐬���A(�C���[�W�r���[��p����)���C�����t�F�C�X�Ƃ��Ĉ����悤�n�[�h�E�G�A�ɓ`����
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
	VERIFY_SUCCEEDED(vkCreateImageView(Device, &ImageViewCreateInfo, GetAllocationCallbacks(), ImageView));

	Logf("\t\tImageViewType = %s\n", GetImageViewTypeString(ImageViewType).c_str());
	Logf("\t\tFormat = %s\n", GetFormatString(Format).c_str());
	Logf("\t\tComponentMapping = (%s)\n", GetComponentMappingString(ComponentMapping).c_str());

	LogOK("CreateImageView");
}

void VK::CreateBufferView(VkBufferView* BufferView, const VkBuffer Buffer, const VkFormat Format, const VkDeviceSize Offset, const VkDeviceSize Range)
{
	const VkBufferViewCreateInfo BufferViewCreateInfo = {
		VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO,
		nullptr,
		0,
		Buffer,
		Format,
		Offset,
		Range
	};
	VERIFY_SUCCEEDED(vkCreateBufferView(Device, &BufferViewCreateInfo, GetAllocationCallbacks(), BufferView));
}

void VK::ValidateFormatProperties(const VkImageUsageFlags Usage, const VkFormat Format) const
{
	VkFormatProperties FormatProperties;
	vkGetPhysicalDeviceFormatProperties(GetCurrentPhysicalDevice(), Format, &FormatProperties);

	//!< �T���v���h�C���[�W�ł͑S�Ẵt�H�[�}�b�g���T�|�[�g����Ă�킯�ł͂Ȃ� Not all formats are supported for sampled images
	if (Usage & VK_IMAGE_USAGE_SAMPLED_BIT) {
		if (!(FormatProperties.optimalTilingFeatures &  VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT)) {
			Error("VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT not supported\n");
			DEBUG_BREAK();
		}
		//!< #VK_TODO ���j�A�g�p���̂݃`�F�b�N����
		const auto bUseLiner = true;//!< VK_FILTER_LINEAR == VkSamplerCreateInfo.magFilter || VK_FILTER_LINEAR == VkSamplerCreateInfo.minFilter || VK_SAMPLER_MIPMAP_MODE_LINEAR == VkSamplerCreateInfo.mipmapMode;
		if (bUseLiner) {
			if (!(FormatProperties.linearTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT)) {
				Error("VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT not supported\n");
				DEBUG_BREAK();
			}
		}
	}

	if (Usage & VK_IMAGE_USAGE_STORAGE_BIT) {
		if (!(FormatProperties.optimalTilingFeatures &  VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT)) {
			Error("VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT not supported\n");
			DEBUG_BREAK();
		}
		//!< #VK_TODO �A�g�~�b�N�g�p���̂݃`�F�b�N����
		const auto bUseAtomic = false; 
		if (bUseAtomic) {
			if (!(FormatProperties.linearTilingFeatures & VK_FORMAT_FEATURE_STORAGE_IMAGE_ATOMIC_BIT)) {
				Error("VK_FORMAT_FEATURE_STORAGE_IMAGE_ATOMIC_BIT not supported\n");
				DEBUG_BREAK();
			}
		}
	}

	if (Usage & VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT) {
		// !< #VK_TODO ����J���[�Ɍ��ߑł����Ă���
		if (true) {
			//!< �J���[�̏ꍇ In case color
			if (!(FormatProperties.optimalTilingFeatures &  VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT)) {
				Error("VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT not supported\n");
				DEBUG_BREAK();
			}
		}
		else {
			//!< �f�v�X�X�e���V���̏ꍇ In case depth stencil
			if (!(FormatProperties.optimalTilingFeatures &  VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)) {
				Error("VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT not supported\n");
				DEBUG_BREAK();
			}
		}
	}

	if (Usage & VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT) {
		if (!(FormatProperties.bufferFeatures &  VK_FORMAT_FEATURE_UNIFORM_TEXEL_BUFFER_BIT)) {
			Error("VK_FORMAT_FEATURE_UNIFORM_TEXEL_BUFFER_BIT not supported\n");
			DEBUG_BREAK();
		}
	}

	if (Usage & VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT) {
		if (!(FormatProperties.bufferFeatures &  VK_FORMAT_FEATURE_STORAGE_TEXEL_BUFFER_BIT)) {
			Error("VK_FORMAT_FEATURE_STORAGE_TEXEL_BUFFER_BIT not supported\n");
			DEBUG_BREAK();
		}
		//!< #VK_TODO �A�g�~�b�N�g�p���̂݃`�F�b�N����
		const auto bUseAtomic = false;
		if (bUseAtomic) {
			if (!(FormatProperties.linearTilingFeatures & VK_FORMAT_FEATURE_STORAGE_TEXEL_BUFFER_ATOMIC_BIT)) {
				Error("VK_FORMAT_FEATURE_STORAGE_TEXEL_BUFFER_ATOMIC_BIT not supported\n");
				DEBUG_BREAK();
			}
		}
	}
}

void VK::EnumerateInstanceLayerProperties()
{
	InstanceLayerProperties.clear();

	uint32_t Count = 0;
	VERIFY_SUCCEEDED(vkEnumerateInstanceLayerProperties(&Count, nullptr));
	if (Count) {
		std::vector<VkLayerProperties> LayerProp(Count);
		InstanceLayerProperties.reserve(Count);
		VERIFY_SUCCEEDED(vkEnumerateInstanceLayerProperties(&Count, LayerProp.data()));
		for (const auto& i : LayerProp) {
			InstanceLayerProperties.push_back(LAYER_PROPERTY(i, {}));
			EnumerateInstanceExtensionProperties(i.layerName);
		}
	}

	Logf("Instance Layer Properties\n");
	for (const auto& i : InstanceLayerProperties) {
		if (strlen(i.first.layerName)) {
			Logf("\t\"%s\" (%s)\n", i.first.layerName, i.first.description);
			for (const auto j : i.second) {
				Logf("\t\t\"%s\"\n", j.extensionName);
			}
		}
	}
}
void VK::EnumerateInstanceExtensionProperties(const char* LayerName)
{
	uint32_t Count = 0;
	VERIFY_SUCCEEDED(vkEnumerateInstanceExtensionProperties(LayerName, &Count, nullptr));
	if (Count) {
		InstanceLayerProperties.back().second.resize(Count);
		VERIFY_SUCCEEDED(vkEnumerateInstanceExtensionProperties(LayerName, &Count, InstanceLayerProperties.back().second.data()));
	}
}

#ifdef VK_NO_PROTOYYPES
//!< Vulkan���[�_�[��(�����œn�����f�o�C�X�Ɋ��)�K�؂Ȏ����֊֐��R�[�������_�C���N�g����K�v������A���̃��_�C���N�g�ɂ͎��Ԃ�������p�t�H�[�}���X�ɉe������
//!< �ȉ��̂悤�ɂ���ƁA�g�p�������f�o�C�X���璼�ڊ֐������[�h���邽�߁A���_�C���N�g���X�L�b�v�ł��p�t�H�[�}���X�����P�ł���
void VK::LoadVulkanLibrary()
{
#ifdef _WINDOWS
	VulkanLibrary = LoadLibrary(TEXT("vulkan-1.dll")); assert(nullptr != VulkanLibrary && "LoadLibrary failed");
#else
	VulkanLibrary = dlopen("libvulkan.so.1", RTLD_NOW); assert(nullptr != VulkanLibrary && "dlopen failed");
#endif
#ifdef _WINDOWS
	//vkGetInstanceProcAddr = reinterpret_cast<PFN_vkGetInstanceProcAddr>(GetProcAddress(VulkanLibrary, "vkGetInstanceProcAddr")); assert(nullptr != vkGetInstanceProcAddr && "GetProcAddress failed");
#else
	//vkGetInstanceProcAddr = reinterpret_cast<PFN_vkGetInstanceProcAddr>(dlsym(VulkanLibrary, "vkGetInstanceProcAddr")); assert(nullptr != vkGetInstanceProcAddr && "dlsym failed");
#endif
	
	//!< �O���[�o�����x���̊֐������[�h���� Load global level functions
#define VK_GLOBAL_PROC_ADDR(proc) vk ## proc = reinterpret_cast<PFN_vk ## proc>(vkGetInstanceProcAddr(nullptr, "vk" #proc)); assert(nullptr != vk ## proc && #proc);
#include "VKGlobalProcAddr.h"
#undef VK_GLOBAL_PROC_ADDR
}
#endif //!< VK_NO_PROTOYYPES

void VK::CreateInstance()
{
	//!< �C���X�^���X���x���̃��C���[�A�G�N�X�e���V�����̗�
	EnumerateInstanceLayerProperties();

	uint32_t APIVersion; //= VK_API_VERSION_1_1;
	//!< �����ł́A�ŐV�o�[�W�����ł̂ݓ����悤�ɂ��Ă��� Use latest version here
	VERIFY_SUCCEEDED(vkEnumerateInstanceVersion(&APIVersion));
	const auto MajorVersion = VK_VERSION_MAJOR(APIVersion);
	const auto MinorVersion = VK_VERSION_MINOR(APIVersion);
	const auto PatchVersion = VK_VERSION_PATCH(APIVersion);
	Logf("API Version = %d.%d.(Header = %d)(Patch = %d)\n", MajorVersion, MinorVersion, VK_HEADER_VERSION, PatchVersion);

	const auto ApplicationName = GetTitle();
	const VkApplicationInfo ApplicationInfo = {
		VK_STRUCTURE_TYPE_APPLICATION_INFO,
		nullptr,
		ApplicationName.data(), APIVersion,
		"VKDX Engine Name", APIVersion,
		APIVersion
	};
	
	const std::vector<const char*> EnabledLayerNames = {
#ifdef _DEBUG
		//!< ���W���I�ȃo���f�[�V�������C���Z�b�g���œK�ȏ����Ń��[�h����w��
		//!< (�v���O����������Ȃ��ꍇ�͊��ϐ� VK_INSTANCE_LAYERS �փZ�b�g���Ă����Ă��悢)
		"VK_LAYER_LUNARG_standard_validation", 
		//!< 
		"VK_LAYER_LUNARG_object_tracker",
		//!< API �Ăяo���ƃp�����[�^���R���\�[���o�͂��� (�o�͂����邳���̂ł����ł͎w�肵�Ȃ�)
		//"VK_LAYER_LUNARG_api_dump",
#endif
#ifdef USE_RENDERDOC
		"VK_LAYER_RENDERDOC_Capture",
#endif
	};

	const std::vector<const char*> EnabledExtensions = {
		VK_KHR_SURFACE_EXTENSION_NAME,
#ifdef VK_USE_PLATFORM_WIN32_KHR
		VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
#elif defined(VK_USE_PLATFORM_XLIB_KHR)
		VK_KHR_XLIB_SURFACE_EXTENSION_NAME,
#else
		VK_KHR_XCB_SURFACE_EXTENSION_NAME,
#endif
#ifdef _DEBUG
		//!< �f�o�b�O���|�[�g�p
		VK_EXT_DEBUG_REPORT_EXTENSION_NAME,
#endif
#ifdef USE_RENDERDOC
		VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
#endif
	};

	//!< #VK_TODO �L���ɂ��Ă�����̂��AInstanceLayerProperties �Ɋ܂܂�Ă��邩�`�F�b�N����

	const VkInstanceCreateInfo InstanceCreateInfo = {
		VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
		nullptr,
		0,
		&ApplicationInfo,
		static_cast<uint32_t>(EnabledLayerNames.size()), EnabledLayerNames.data(),
		static_cast<uint32_t>(EnabledExtensions.size()), EnabledExtensions.data()
	};
	VERIFY_SUCCEEDED(vkCreateInstance(&InstanceCreateInfo, GetAllocationCallbacks(), &Instance));

	//!< �C���X�^���X���x���̊֐������[�h���� Load instance level functions
#ifdef VK_NO_PROTOYYPES
#define VK_INSTANCE_PROC_ADDR(proc) vk ## proc = reinterpret_cast<PFN_vk ## proc>(vkGetInstanceProcAddr(Instance, "vk" #proc)); assert(nullptr != vk ## proc && #proc);
#include "VKInstanceProcAddr.h"
#undef VK_INSTANCE_PROC_ADDR
#endif //!< VK_NO_PROTOYYPES

#ifdef _DEBUG
	CreateDebugReportCallback();
#endif

	LogOK("CreateInstance");
}

#ifdef _DEBUG
void VK::CreateDebugReportCallback()
{
#define VK_INSTANCE_PROC_ADDR(proc) vk ## proc = reinterpret_cast<PFN_vk ## proc ## EXT>(vkGetInstanceProcAddr(Instance, "vk" #proc "EXT")); assert(nullptr != vk ## proc && #proc);
#include "VKDebugReport.h"
#undef VK_INSTANCE_PROC_ADDR

	const auto Flags = VK_DEBUG_REPORT_INFORMATION_BIT_EXT
		| VK_DEBUG_REPORT_WARNING_BIT_EXT
		| VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT
		| VK_DEBUG_REPORT_ERROR_BIT_EXT
		| VK_DEBUG_REPORT_DEBUG_BIT_EXT;

	if (VK_NULL_HANDLE != vkCreateDebugReportCallback) {
		const VkDebugReportCallbackCreateInfoEXT DebugReportCallbackCreateInfo = {
			VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT,
			nullptr,
			Flags,
			[](VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objectType, uint64_t object, size_t location, int32_t messageCode, const char* pLayerPrefix, const char* pMessage, void* pUserData)->VkBool32 {
					using namespace std;
					if (VK_DEBUG_REPORT_ERROR_BIT_EXT & flags) {
						DEBUG_BREAK();
						Errorf("%s\n", pMessage);
						return VK_TRUE;
					}
					else if (VK_DEBUG_REPORT_WARNING_BIT_EXT & flags) {
						DEBUG_BREAK();
						Warningf("%s\n", pMessage);
						return VK_TRUE;
					}
					else if (VK_DEBUG_REPORT_INFORMATION_BIT_EXT & flags) {
						//Logf(" %s\n", pMessage);
					}
					else if (VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT & flags) {
						DEBUG_BREAK();
						Logf("%s\n", pMessage);
						return VK_TRUE;
					}
					else if (VK_DEBUG_REPORT_DEBUG_BIT_EXT & flags) {
						//Logf("%s\n", pMessage);
					}
					return VK_FALSE;
			},
			nullptr
		};
		vkCreateDebugReportCallback(Instance, &DebugReportCallbackCreateInfo, nullptr, &DebugReportCallback);
	}
}
#endif //!< _DEBUG

#ifdef _DEBUG
void VK::MarkerInsert(VkCommandBuffer CB, const glm::vec4& Color, const char* Name)
{
	if (VK_NULL_HANDLE != vkCmdDebugMarkerInsert) {
		VkDebugMarkerMarkerInfoEXT DebugMarkerMarkerInfo = {
			VK_STRUCTURE_TYPE_DEBUG_MARKER_MARKER_INFO_EXT,
			nullptr,
			Name,
		};
		memcpy(DebugMarkerMarkerInfo.color, &Color, sizeof(DebugMarkerMarkerInfo.color));
		vkCmdDebugMarkerInsert(CB, &DebugMarkerMarkerInfo);
	}
}
void VK::MarkerBegin(VkCommandBuffer CB, const glm::vec4& Color, const char* Name)
{
	if (VK_NULL_HANDLE != vkCmdDebugMarkerBegin) {
		VkDebugMarkerMarkerInfoEXT DebugMarkerMarkerInfo = {
			VK_STRUCTURE_TYPE_DEBUG_MARKER_MARKER_INFO_EXT,
			nullptr,
			Name,
		};
		memcpy(DebugMarkerMarkerInfo.color, &Color, sizeof(DebugMarkerMarkerInfo.color));
		vkCmdDebugMarkerBegin(CB, &DebugMarkerMarkerInfo);
	}
}
void VK::MarkerEnd(VkCommandBuffer CB)
{
	if (VK_NULL_HANDLE != vkCmdDebugMarkerEnd) {
		vkCmdDebugMarkerEnd(CB);
	}
}

void VK::MarkerSetName(VkDevice Device, const VkDebugReportObjectTypeEXT Type, const uint64_t Object, const char* Name)
{
	if (VK_NULL_HANDLE != vkDebugMarkerSetObjectName) {
		VkDebugMarkerObjectNameInfoEXT DebugMarkerObjectNameInfo = {
			VK_STRUCTURE_TYPE_DEBUG_MARKER_OBJECT_NAME_INFO_EXT,
			nullptr,
			Type,
			Object,
			Name
		};
		VERIFY_SUCCEEDED(vkDebugMarkerSetObjectName(Device, &DebugMarkerObjectNameInfo));
	}
}

void VK::MarkerSetTag(VkDevice Device, const VkDebugReportObjectTypeEXT Type, const uint64_t Object, const uint64_t TagName, const size_t TagSize, const void* TagData)
{
	if (VK_NULL_HANDLE != vkDebugMarkerSetObjectTag) {
		VkDebugMarkerObjectTagInfoEXT DebugMarkerObjectTagInfo = {
			VK_STRUCTURE_TYPE_DEBUG_MARKER_OBJECT_TAG_INFO_EXT,
			nullptr,
			Type,
			Object,
			TagName,
			TagSize,
			TagData
		};
		VERIFY_SUCCEEDED(vkDebugMarkerSetObjectTag(Device, &DebugMarkerObjectTagInfo));
	}
}

#endif //!< _DEBUG

void VK::CreateSurface(
#ifdef VK_USE_PLATFORM_WIN32_KHR
	HWND hWnd, HINSTANCE hInstance
#elif defined(VK_USE_PLATFORM_XLIB_KHR)
	Display* Dsp, Window Wnd
#else
	xcb_connection_t* Cnt, xcb_window_t Wnd
#endif
)
{
#ifdef VK_USE_PLATFORM_WIN32_KHR
	const VkWin32SurfaceCreateInfoKHR SurfaceCreateInfo = {
		VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
		nullptr,
		0,
		hInstance,
		hWnd
	};
	VERIFY_SUCCEEDED(vkCreateWin32SurfaceKHR(Instance, &SurfaceCreateInfo, GetAllocationCallbacks(), &Surface));
#elif defined(VK_USE_PLATFORM_XLIB_KHR)
	const VkXlibSurfaceCreateInfoKHR SurfaceCreateInfo = {
		VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR,
		nullptr,
		0,
		Dsp,
		Wnd,
	};
	VERIFY_SUCCEEDED(VkXlibSurfaceCreateInfoKHR(Instance, &SurfaceCreateInfo, GetAllocationCallbacks(), &Surface));
#else
	const VkXcbSurfaceCreateInfoKHR SurfaceCreateInfo = {
		VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR,
		nullptr,
		0,
		Cnt,
		Wnd,
	};
	VERIFY_SUCCEEDED(vkCreateXcbSurfaceKHR(Instance, &SurfaceCreateInfo, GetAllocationCallbacks(), &Surface));
#endif

	Log("\tCreateSurface");
}
void VK::EnumeratePhysicalDeviceProperties(const VkPhysicalDeviceProperties& PDP)
{
	Log("\t\tVersion = ");
	Logf("%d.%d(Patch = %d)\n", VK_VERSION_MAJOR(PDP.apiVersion), VK_VERSION_MINOR(PDP.apiVersion), VK_VERSION_PATCH(PDP.apiVersion));

	//!< �o�[�W�����`�F�b�N Check version
#ifdef _DEBUG
	[&](const uint32_t Version) {
		uint32_t APIVersion;
		VERIFY_SUCCEEDED(vkEnumerateInstanceVersion(&APIVersion));
		if (Version < APIVersion) {
			Log("\t\t");
			Warningf("[ DEVICE ] %d.%d(Patch = %d) < %d.%d(Patch = %d) [ INSTANCE ]\n",
				VK_VERSION_MAJOR(Version), VK_VERSION_MINOR(Version), VK_VERSION_PATCH(Version),
				VK_VERSION_MAJOR(APIVersion), VK_VERSION_MINOR(APIVersion), VK_VERSION_PATCH(APIVersion));
		}
	}(PDP.apiVersion);
#endif

#define PHYSICAL_DEVICE_TYPE_ENTRY(entry) if(VK_PHYSICAL_DEVICE_TYPE_##entry == PDP.deviceType) { Log(#entry); }
	Logf("\t\t%s, DeviceType = ", PDP.deviceName);
	PHYSICAL_DEVICE_TYPE_ENTRY(OTHER);
	PHYSICAL_DEVICE_TYPE_ENTRY(INTEGRATED_GPU);
	PHYSICAL_DEVICE_TYPE_ENTRY(DISCRETE_GPU);
	PHYSICAL_DEVICE_TYPE_ENTRY(VIRTUAL_GPU);
	PHYSICAL_DEVICE_TYPE_ENTRY(CPU);
	Log("\n");
#undef PHYSICAL_DEVICE_TYPE_ENTRY

#define PROPERTY_LIMITS_ENTRY(entry) Logf("\t\t\t\t%s = %d\n", #entry, PDP.limits.##entry);
	Log("\t\t\tPhysicalDeviceProperties.PhysicalDeviceLimits\n");
	PROPERTY_LIMITS_ENTRY(maxUniformBufferRange);
	//PROPERTY_LIMITS_ENTRY(maxStorageBufferRange);
	PROPERTY_LIMITS_ENTRY(maxPushConstantsSize);
	PROPERTY_LIMITS_ENTRY(maxFragmentOutputAttachments);
	PROPERTY_LIMITS_ENTRY(maxColorAttachments);
#undef PROPERTY_LIMITS_ENTRY
}
void VK::EnumeratePhysicalDeviceFeatures(const VkPhysicalDeviceFeatures& PDF)
{
	Log("\t\t\tPhysicalDeviceFeatures\n");
#define VK_DEVICEFEATURE_ENTRY(entry) if (PDF.##entry) { Log("\t\t\t\t" #entry "\n"); }
#include "VKDeviceFeature.h"
#undef VK_DEVICEFEATURE_ENTRY
}
void VK::EnumeratePhysicalDeviceMemoryProperties(const VkPhysicalDeviceMemoryProperties& PDMP)
{
	Log("\t\t\tMemoryType\n");
	for (uint32_t i = 0; i < PDMP.memoryTypeCount; ++i) {
		Log("\t\t\t\t");
		Logf("[%d] HeapIndex = %d", i, PDMP.memoryTypes[i].heapIndex);

		if (PDMP.memoryTypes[i].propertyFlags) {
			Log(", PropertyFlags = ");
#define MEMORY_PROPERTY_ENTRY(entry) if(VK_MEMORY_PROPERTY_##entry##_BIT & PDMP.memoryTypes[i].propertyFlags) { Log(#entry " | "); }
			MEMORY_PROPERTY_ENTRY(DEVICE_LOCAL);
			MEMORY_PROPERTY_ENTRY(HOST_VISIBLE);
			MEMORY_PROPERTY_ENTRY(HOST_COHERENT);
			MEMORY_PROPERTY_ENTRY(HOST_CACHED);
			MEMORY_PROPERTY_ENTRY(LAZILY_ALLOCATED);
#undef MEMORY_PROPERTY_ENTRY
		}
		Log("\n");
	}

	Log("\t\t\tMemoryHeap\n");
	for (uint32_t i = 0; i < PDMP.memoryHeapCount; ++i) {
		Log("\t\t\t\t");
		Logf("[%d] Size = %d", i, PDMP.memoryHeaps[i].size);

		if (PDMP.memoryHeaps[i].flags) {
			Log(", Flags = ");
			if (VK_MEMORY_HEAP_DEVICE_LOCAL_BIT & PDMP.memoryHeaps[i].flags) { Log("DEVICE_LOCAL | "); }
			if (VK_MEMORY_HEAP_MULTI_INSTANCE_BIT_KHR & PDMP.memoryHeaps[i].flags) { Log("MULTI_INSTANCE | "); }
		}
		Log("\n");
	}
}
void VK::EnumeratePhysicalDevice(VkInstance Instance)
{
	//!< �����f�o�C�X(GPU)�̗�
	uint32_t Count = 0;
	VERIFY_SUCCEEDED(vkEnumeratePhysicalDevices(Instance, &Count, nullptr));
	assert(Count && "Physical device not found");
	PhysicalDevices.resize(Count);
	VERIFY_SUCCEEDED(vkEnumeratePhysicalDevices(Instance, &Count, PhysicalDevices.data()));

	Log("\tPhysicalDevices\n");
	for (const auto& i : PhysicalDevices) {
		//!< �v���p�e�B
		VkPhysicalDeviceProperties PDP;
		vkGetPhysicalDeviceProperties(i, &PDP);
		EnumeratePhysicalDeviceProperties(PDP);

		//!< �t�B�[�`���[
		VkPhysicalDeviceFeatures PDF;
		vkGetPhysicalDeviceFeatures(i, &PDF);
		EnumeratePhysicalDeviceFeatures(PDF);

		//!< �������v���p�e�B
		VkPhysicalDeviceMemoryProperties PDMP;
		vkGetPhysicalDeviceMemoryProperties(i, &PDMP);
		EnumeratePhysicalDeviceMemoryProperties(PDMP);

		//!< �f�o�C�X���x���̃��C���[�A�G�N�X�e���V�����̗�
		EnumeratePhysicalDeviceLayerProperties(i);

		Log("\n");
	}

	assert(!PhysicalDevices.empty() && "No physical device found");
	SetCurrentPhysicalDevice(PhysicalDevices[0]); //!< �����ł͍ŏ��̕����f�o�C�X��I�����邱�Ƃɂ��� #VK_TODO
}
void VK::EnumeratePhysicalDeviceLayerProperties(VkPhysicalDevice PD)
{
	PhysicalDeviceLayerProperties.push_back(PHYSICAL_DEVICE_LAYER_PROPERTY(PD, {}));

	uint32_t Count = 0;
	VERIFY_SUCCEEDED(vkEnumerateDeviceLayerProperties(PD, &Count, nullptr));
	if (Count) {
		std::vector<VkLayerProperties> LayerProp(Count);
		PhysicalDeviceLayerProperties.back().second.reserve(Count);
		VERIFY_SUCCEEDED(vkEnumerateDeviceLayerProperties(PD, &Count, LayerProp.data()));
		for (const auto& i : LayerProp) {
			PhysicalDeviceLayerProperties.back().second.push_back(LAYER_PROPERTY(i, {}));
			EnumeratePhysicalDeviceExtensionProperties(PD, i.layerName);
		}
	}

	Logf("Device Layer Properties\n");
	for (const auto& i : PhysicalDeviceLayerProperties.back().second) {
		if (strlen(i.first.layerName)) {
			Logf("\t\"%s\" (%s)\n", i.first.layerName, i.first.description);
			for (const auto j : i.second) {
				Logf("\t\t\"%s\"\n", j.extensionName);
			}
		}
	}
}
void VK::EnumeratePhysicalDeviceExtensionProperties(VkPhysicalDevice PD, const char* LayerName)
{
	uint32_t Count = 0;
	VERIFY_SUCCEEDED(vkEnumerateDeviceExtensionProperties(PD, LayerName, &Count, nullptr));
	if (Count) {
		PhysicalDeviceLayerProperties.back().second.back().second.resize(Count);
		VERIFY_SUCCEEDED(vkEnumerateDeviceExtensionProperties(PD, LayerName, &Count, PhysicalDeviceLayerProperties.back().second.back().second.data()));
	}
}

void VK::EnumerateQueueFamilyProperties(VkPhysicalDevice PD, VkSurfaceKHR Surface, std::vector<VkQueueFamilyProperties>& QFPs)
{
	//!< �����\�͂����L���[�̓t�@�~���ɃO���[�v�������
	uint32_t Count = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(PD, &Count, nullptr);
	assert(Count && "QueueFamilyProperty not found");
	QFPs.resize(Count);
	vkGetPhysicalDeviceQueueFamilyProperties(PD, &Count, QFPs.data());

	//!< Geforce970M���ƈȉ��̂悤�ȏ�Ԃ����� In case of Geforce970M
	//!< QueueFamilyIndex == 0 : Grahics | Compute | Transfer | SparceBinding and Present
	//!< QueueFamilyIndex == 1 : Transfer
	Log("\tQueueFamilyProperties\n");
#define QUEUE_FLAG_ENTRY(entry) if(VK_QUEUE_##entry##_BIT & QFPs[i].queueFlags) { Logf("%s | ", #entry); }
	for (uint32_t i = 0; i < QFPs.size(); ++i) {
		Logf("\t\t[%d] QueueCount = %d, ", i, QFPs[i].queueCount);
		Log("QueueFlags = ");
		QUEUE_FLAG_ENTRY(GRAPHICS);
		QUEUE_FLAG_ENTRY(COMPUTE);
		QUEUE_FLAG_ENTRY(TRANSFER);
		QUEUE_FLAG_ENTRY(SPARSE_BINDING);
		QUEUE_FLAG_ENTRY(PROTECTED);
		Log("\n");

		QFPs[i].timestampValidBits;
		QFPs[i].minImageTransferGranularity;

		VkBool32 Supported = VK_FALSE;
		VERIFY_SUCCEEDED(vkGetPhysicalDeviceSurfaceSupportKHR(PD, i, Surface, &Supported));
		if (Supported) {
			Logf("\t\t\t\tSurface(Present) Supported\n");
		}
	}
#undef QUEUE_FLAG_ENTRY

	//!< ��p�L���[�����݂���ꍇ�͐�p�L���[���g�p�����ق����ǂ�
	std::bitset<8> GraphicsQueueFamilyBits;
	std::bitset<8> PresentQueueFamilyBits;
	std::bitset<8> ComputeQueueFamilyBits;
	std::bitset<8> TransferQueueFamilyBits;
	std::bitset<8> SparceBindingQueueFamilyBits;
	for (uint32_t i = 0; i < QFPs.size(); ++i) {
		const auto& QP = QFPs[i];

		if (VK_QUEUE_GRAPHICS_BIT & QP.queueFlags) {
			GraphicsQueueFamilyBits.set(i);
		}
		if (VK_QUEUE_COMPUTE_BIT & QP.queueFlags) {
			ComputeQueueFamilyBits.set(i);
		}
		if (VK_QUEUE_TRANSFER_BIT & QP.queueFlags) {
			TransferQueueFamilyBits.set(i);
		}
		if (VK_QUEUE_SPARSE_BINDING_BIT & QP.queueFlags) {
			SparceBindingQueueFamilyBits.set(i);
		}

		VkBool32 Supported = VK_FALSE;
		VERIFY_SUCCEEDED(vkGetPhysicalDeviceSurfaceSupportKHR(PD, i, Surface, &Supported));
		if (Supported) {
			PresentQueueFamilyBits.set(i);
		}
	}

	//!< �O���t�B�b�N�ƃv���[���e�[�V�������u�����Ɂv�T�|�[�g����L���[�����邩 Prioritize queue which support both of graphics and presentation
	//for (uint32_t i = 0; i < Count; ++i) {
	//	if (VK_QUEUE_GRAPHICS_BIT & QFPs[i].queueFlags) {
	//		VkBool32 Supported = VK_FALSE;
	//		VERIFY_SUCCEEDED(vkGetPhysicalDeviceSurfaceSupportKHR(PD, i, Surface, &Supported));
	//		if (Supported) {
	//			Logf("\t\t\tFound Graphics and Presentation support queue [%d]\n", i);
	//			break;
	//		}
	//	}
	//}

	assert(GraphicsQueueFamilyBits.any() && "GraphicsQueue not found");
	assert(PresentQueueFamilyBits.any() && "PresentQueue not found");
	assert(ComputeQueueFamilyBits.any() && "ComputeQueue not found");
	//assert(TransferQueueFamilyBits.any() && "TansferQueue not found");
	//assert(SparceBindingQueueFamilyBits.any() && "SparceBindingQueue not found");

	//Log("\n");
	//Logf("\t\tGraphicsQueueFamilyBits = %s\n", GraphicsQueueFamilyBits.to_string());
	//Logf("\t\tPresentQueueFamilyBits = %s\n", PresentQueueFamilyBits.to_string());
	//Logf("\t\tComputeQueueFamilyBits = %s\n", ComputeQueueFamilyBits.to_string());
	//Logf("\t\tTansferQueueFamilyBits = %s\n", TransferQueueFamilyBits.to_string());
	//Logf("\t\tSparceBindingQueueFamilyBits = %s\n", SparceBindingQueueFamilyBits.to_string());
#ifdef DEBUG_STDOUT
	std::cout << std::endl;
	std::cout << "\t" << "\t" << "GraphicsQueueFamilyBits = " << GraphicsQueueFamilyBits.to_string() << std::endl;
	std::cout << "\t" << "\t" << "PresentQueueFamilyBits = " << PresentQueueFamilyBits.to_string() << std::endl;
	std::cout << "\t" << "\t" << "ComputeQueueFamilyBits = " << ComputeQueueFamilyBits.to_string() << std::endl;
	std::cout << "\t" << "\t" << "TansferQueueFamilyBits = " << TransferQueueFamilyBits.to_string() << std::endl;
	std::cout << "\t" << "\t" << "SparceBindingQueueFamilyBits = " << SparceBindingQueueFamilyBits.to_string() << std::endl;
#endif
}
void VK::OverridePhysicalDeviceFeatures(VkPhysicalDeviceFeatures& PDF) const
{
	//!< VkPhysicalDeviceFeatures �ɂ͉\�ȃt�B�[�`���[���S�� true �ɂȂ������̂��n����Ă���̂ŁA
	//!< �s�v�ȍ��ڂ� false �ɂ���悤�ɃI�[�o�[���C�h����ƃp�t�H�[�}���X���ǂ��Ȃ�
	//!< VkPhysicalDeviceFeatures �� false �œn����Ă��鍀�ڂ� true �ɕς��Ă�����

	Log("\tPhysicalDeviceFeatures (Override)\n");
#define VK_DEVICEFEATURE_ENTRY(entry) if(PDF.entry) { Logf("\t\t%s\n", #entry); }
#include "VKDeviceFeature.h"
#undef VK_DEVICEFEATURE_ENTRY
}

void VK::CreateQueueFamilyPriorities(VkPhysicalDevice PD, VkSurfaceKHR Surface, const std::vector<VkQueueFamilyProperties>& QFPs, std::vector<std::vector<float>>& Priorites)
{
	for (auto i = 0; i < QFPs.size(); ++i) {
		const auto& QFP = QFPs[i];
		auto& Pri = Priorites[i];
		if (VK_QUEUE_GRAPHICS_BIT & QFP.queueFlags) {
			GraphicsQueueFamilyIndex = i;
			if (Pri.size() < QFP.queueCount) {
				Pri.push_back(0.5f);
			}
			GraphicsQueueIndex = static_cast<uint32_t>(Pri.size()) - 1;
			break;
		}
	}
	for (auto i = 0; i < QFPs.size(); ++i) {
		const auto& QFP = QFPs[i];
		auto& Pri = Priorites[i];
		VkBool32 Supported = VK_FALSE;
		VERIFY_SUCCEEDED(vkGetPhysicalDeviceSurfaceSupportKHR(PD, i, Surface, &Supported));
		if (Supported) {
			PresentQueueFamilyIndex = i;
			if (Pri.size() < QFP.queueCount) {
				Pri.push_back(0.5f);
			}
			PresentQueueIndex = static_cast<uint32_t>(Pri.size()) - 1;
			break;
		}
	}
	for (auto i = 0; i < QFPs.size(); ++i) {
		const auto& QFP = QFPs[i];
		auto& Pri = Priorites[i];
		if (VK_QUEUE_COMPUTE_BIT & QFP.queueFlags) {
			ComputeQueueFamilyIndex = i;
			if (Pri.size() < QFP.queueCount) {
				Pri.push_back(0.5f);
			}
			ComputeQueueIndex = static_cast<uint32_t>(Pri.size()) - 1;
			break;
		}
	}
	//for (auto i = 0; i < QFPs.size(); ++i) {
	//	const auto& QFP = QFPs[i];
	//	auto& Pri = Priorites[i];
	//	if (VK_QUEUE_TRANSFER_BIT & QFP.queueFlags) {
	//		TransferQueueFamilyIndex = i;
	//		if (Pri.size() < QFP.queueCount) {
	//			Pri.push_back(0.5f);
	//		}
	//		TransferQueueIndex = static_cast<uint32_t>(Pri.size()) - 1;
	//		break;
	//	}
	//}
	//for (auto i = 0; i < QFPs.size(); ++i) {
	//	const auto& QFP = QFPs[i];
	//	auto& Pri = Priorites[i];
	//	if (VK_QUEUE_SPARSE_BINDING_BIT & QFP.queueFlags) {
	//		SparceBindingQueueFamilyIndex = i;
	//		if (Pri.size() < QFP.queueCount) {
	//			Pri.push_back(0.5f);
	//		}
	//		SparceBindingQueueIndex = static_cast<uint32_t>(Pri.size()) - 1;
	//		break;
	//	}
	//}

	Log("\n");
	Logf("\t\tGraphics QueueFamilyIndex = %d, QueueIndex = %d\n", GraphicsQueueFamilyIndex, GraphicsQueueIndex);
	Logf("\t\tPresent QueueFamilyIndex = %d, QueueIndex = %d\n", PresentQueueFamilyIndex, PresentQueueIndex);
	Logf("\t\tCompute QueueFamilyIndex = %d, QueueIndex = %d\n", ComputeQueueFamilyIndex, ComputeQueueIndex);
	//Logf("\t\tTransfer\tQueueFamilyIndex = %d, QueueIndex = %d\n", TransferQueueFamilyIndex, TransferQueueIndex);
	//Logf("\t\tSparceBinding\tQueueFamilyIndex = %d, QueueIndex = %d\n", SparceBindingQueueFamilyIndex, SparceBindingQueueIndex);
}
void VK::CreateDevice(VkPhysicalDevice PD, VkSurfaceKHR Surface)
{
	std::vector<VkQueueFamilyProperties> QFPs;
	EnumerateQueueFamilyProperties(PD, Surface, QFPs);

	std::vector<std::vector<float>> Priorites(8);
	CreateQueueFamilyPriorities(PD, Surface, QFPs, Priorites);

	//!< �L���[�쐬��� Queue create information
	std::vector<VkDeviceQueueCreateInfo> QueueCreateInfos;
	for (auto i = 0; i < Priorites.size();++i) {
		if (!Priorites[i].empty()) {
			QueueCreateInfos.push_back(
				{
						VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
						nullptr,
						0,
						static_cast<uint32_t>(i),
						static_cast<uint32_t>(Priorites[i].size()), Priorites[i].data()
				}
			);
		}
	}	

	const std::vector<const char*> EnabledExtensions = {
		//!< �X���b�v�`�F�C���̓v���b�g�t�H�[���ɓ��L�̋@�\�Ȃ̂Ńf�o�C�X�쐻���� VK_KHR_SWAPCHAIN_EXTENSION_NAME �G�N�X�e���V������L���ɂ��č쐬���Ă���
		VK_KHR_SWAPCHAIN_EXTENSION_NAME,
#ifdef USE_RENDERDOC
		VK_EXT_DEBUG_MARKER_EXTENSION_NAME,
#endif
		VK_EXT_VALIDATION_CACHE_EXTENSION_NAME,
	};

	//!< vkGetPhysicalDeviceFeatures() �ŉ\�ȃt�B�[�`���[���S�� VkPhysicalDeviceFeatures �ɂȂ���PDF���Ԃ�
	//!< ���̂܂܂ł͉\�Ȃ����L���ɂȂ��Ă��܂��̂Ńp�t�H�[�}���X�I�ɂ͗ǂ��Ȃ�(�K�v�ȍ��ڂ��� true �ɂ��A����ȊO�� false �ɂ���̂��{���͗ǂ�)
	//!< �f�o�C�X�t�B�[�`���[���u�L���ɂ��Ȃ��Ɓv�Ǝg�p�ł��Ȃ��@�\�����X����̂ł����ł͕Ԃ����l�����̂܂܎g���Ă��� (�p�t�H�[�}���X�͗ǂ��Ȃ�)
	VkPhysicalDeviceFeatures PDF;
	vkGetPhysicalDeviceFeatures(PD, &PDF);
	//!< �K�v�Ȃ炱���ŃI�[�o�[���C�h���ĕs�v�ȍ��ڂ� false �ɂ���
	OverridePhysicalDeviceFeatures(PDF);
	const VkDeviceCreateInfo DeviceCreateInfo = {
		VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
		nullptr,
		0,
		static_cast<uint32_t>(QueueCreateInfos.size()), QueueCreateInfos.data(),
		0, nullptr, //!< �f�o�C�X�̃��C���͔񐄏� (Device layer is deprecated)
		static_cast<uint32_t>(EnabledExtensions.size()), EnabledExtensions.data(),
		&PDF
	};
	VERIFY_SUCCEEDED(vkCreateDevice(PD, &DeviceCreateInfo, GetAllocationCallbacks(), &Device));

	//!< �f�o�C�X���x���̊֐������[�h���� Load device level functions
#ifdef VK_NO_PROTOYYPES
#define VK_DEVICE_PROC_ADDR(proc) vk ## proc = reinterpret_cast<PFN_vk ## proc>(vkGetDeviceProcAddr(Device, "vk" #proc)); assert(nullptr != vk ## proc && #proc); assert(nullptr != vk ## proc && #proc);
#include "VKDeviceProcAddr.h"
#undef VK_DEVICE_PROC_ADDR
#endif //!< VK_NO_PROTOYYPES

	//!< �L���[�̎擾 (�O���t�B�b�N�A�v���[���g�L���[�͓����C���f�b�N�X�̏ꍇ�����邪�ʂ̕ϐ��Ɏ擾���Ă���) Graphics and presentation index may be same, but save to individual variables
	vkGetDeviceQueue(Device, GraphicsQueueFamilyIndex, GraphicsQueueIndex, &GraphicsQueue);
	vkGetDeviceQueue(Device, PresentQueueFamilyIndex, PresentQueueIndex, &PresentQueue);
	//vkGetDeviceQueue(Device, ComputeQueueFamilyIndex, ComputeQueueIndex, &ComputeQueue);
	//vkGetDeviceQueue(Device, TransferQueueFamilyIndex, TransferQueueIndex, &TransferQueue);
	//vkGetDeviceQueue(Device, SparceBindingQueueFamilyIndex, SparceBindingQueueIndex, &SparceBindingQueue);

#ifdef _DEBUG
	//if (HasDebugMarkerExt) {
#define VK_DEVICE_PROC_ADDR(proc) vk ## proc = reinterpret_cast<PFN_vk ## proc ## EXT>(vkGetDeviceProcAddr(Device, "vk" #proc "EXT")); assert(nullptr != vk ## proc && #proc);
#include "VKDebugMarker.h"
#undef VK_DEVICE_PROC_ADDR
	//}
#endif

	LogOK("CreateDevice");
}

void VK::CreateFence()
{
	const VkFenceCreateInfo FenceCreateInfo = {
		VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
		nullptr,
		VK_FENCE_CREATE_SIGNALED_BIT //!< ����ƂQ��ڈȍ~�𓯂��Ɉ����ׂɃV�O�i���ςݏ�Ԃō쐬 Create signaled state to do same operation on first time and second time
	};
	VERIFY_SUCCEEDED(vkCreateFence(Device, &FenceCreateInfo, GetAllocationCallbacks(), &Fence));
	VERIFY_SUCCEEDED(vkCreateFence(Device, &FenceCreateInfo, GetAllocationCallbacks(), &ComputeFence));

	LogOK("CreateFence");
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

	//!< �v���[���g���������p To wait presentation finish
	VERIFY_SUCCEEDED(vkCreateSemaphore(Device, &SemaphoreCreateInfo, GetAllocationCallbacks(), &NextImageAcquiredSemaphore));

	//!< �`�抮�������p To wait render finish
	VERIFY_SUCCEEDED(vkCreateSemaphore(Device, &SemaphoreCreateInfo, GetAllocationCallbacks(), &RenderFinishedSemaphore));

	LogOK("CreateSemaphore");
}

//!< �L���[�t�@�~�����قȂ�ꍇ�́A�e�X�ʂ̃R�}���h�v�[����p�ӂ���K�v������ #VK_TODO
void VK::CreateCommandPool(VkCommandPool& CP, const uint32_t QueueFamilyIndex)
{
	/**
	@brief VkCommandPoolCreateFlags
	* VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT	... �R�}���h�o�b�t�@���Ƀ��Z�b�g���\�A�w�薳�����ƃv�[���P�ʂł܂Ƃ߂ă��Z�b�g�����ł��Ȃ�
	* VK_COMMAND_POOL_CREATE_TRANSIENT_BIT				... �p�ɂɍX�V�����A���C�t�X�p�����Z���ꍇ�Ɏw�� (�������A���P�[�V�����̃q���g�ƂȂ�)
	*/
	const VkCommandPoolCreateInfo CommandPoolInfo = {
		VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
		nullptr,
		VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT/*| VK_COMMAND_POOL_CREATE_TRANSIENT_BIT*/,
		QueueFamilyIndex
	};

	VERIFY_SUCCEEDED(vkCreateCommandPool(Device, &CommandPoolInfo, GetAllocationCallbacks(), &CP));

	LogOK("CreateCommandPool");
}

void VK::AllocateCommandBuffer(std::vector<VkCommandBuffer>& CBs, const VkCommandPool CP, const size_t Count, const VkCommandBufferLevel Level)
{
	if (Count) {
		const VkCommandBufferAllocateInfo AllocateInfo = {
			VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
			nullptr,
			CP,
			Level,
			static_cast<uint32_t>(Count)
		};

		CBs.resize(Count);
		VERIFY_SUCCEEDED(vkAllocateCommandBuffers(Device, &AllocateInfo, CBs.data()));
	}

	LogOK("CreateCommandBuffer");
}

void VK::CreateCommandBuffer()
{	
	CreateCommandPool(CommandPool, GraphicsQueueFamilyIndex);
	AllocateCommandBuffer(CommandBuffers, CommandPool, SwapchainImages.size(), VK_COMMAND_BUFFER_LEVEL_PRIMARY);

	//!< �L���[�t�@�~�����قȂ�ꍇ�́A�ʂ̃R�}���h�v�[����p�ӂ���K�v������ #VK_TODO
//	CreateCommandPool(CommandPool, ComputeQueueFamilyIndex);
//	AllocateCommandBuffer(CommandBuffers, CommandPool, 1, VK_COMMAND_BUFFER_LEVEL_PRIMARY);
}

void VK::CreateSwapchain(VkPhysicalDevice PD, VkSurfaceKHR Surface)
{
	CreateSwapchain(PD, Surface, Rect);

	CreateSwapchainImageView();
}
VkSurfaceFormatKHR VK::SelectSurfaceFormat(VkPhysicalDevice PD, VkSurfaceKHR Surface)
{
	uint32_t Count;
	VERIFY_SUCCEEDED(vkGetPhysicalDeviceSurfaceFormatsKHR(PD, Surface, &Count, nullptr));
	assert(Count && "Surface format count is zero");
	std::vector<VkSurfaceFormatKHR> SFs(Count);
	VERIFY_SUCCEEDED(vkGetPhysicalDeviceSurfaceFormatsKHR(PD, Surface, &Count, SFs.data()));

	//!< �����ł͍ŏ��Ɍ������� UNDEFINED �łȂ����̂�I�����Ă��� (Select first format but UNDEFINED here)
	const auto SelectedIndex = [&]() {
		//!< �v�f�� 1 �݂̂� UNDEFINED �̏ꍇ�A�����͖����D���Ȃ��̂�I���ł��� (If there is only 1 element and which is UNDEFINED, we can choose any)
		if (1 == SFs.size() && VK_FORMAT_UNDEFINED == SFs[0].format) {
			return -1;
		}
		for (auto i = 0; i < SFs.size(); ++i) {
			if (VK_FORMAT_UNDEFINED != SFs[i].format) {
				return i;
			}
		}
		//!< �����ɗ��Ă͂����Ȃ�
		assert(false && "Valid surface format not found");
		return 0;
	}();

	//!< ColorSpace �̓n�[�h�E�F�A��ł̃J���[�R���|�[�l���g�̕\��(���j�A�A�m�����j�A�A�G���R�[�h�A�f�R�[�h��)
	Log("\t\tFormats\n");
	for (auto i = 0; i < SFs.size(); ++i) {
		Log("\t\t\t");
		if (i == SelectedIndex) {
			Log("->");
		}
		//Logf("Format = %s, ColorSpace = %s\n", GetFormatString(i.format), GetColorSpaceString(i.colorSpace));
		std::cout << "Format = " << GetFormatString(SFs[i].format) << ", ColorSpace = " << GetColorSpaceString(SFs[i].colorSpace) << std::endl;
	}
	if (-1 == SelectedIndex) {
		Log("\t\t\t->");
		std::cout << "Format = " << GetFormatString(VK_FORMAT_B8G8R8A8_UNORM) << ", ColorSpace = " << GetColorSpaceString(VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) << std::endl;
	}

	return -1 == SelectedIndex ? VkSurfaceFormatKHR({ VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR }) : SFs[SelectedIndex];
}
VkExtent2D VK::SelectSurfaceExtent(const VkSurfaceCapabilitiesKHR& Cap, const uint32_t Width, const uint32_t Height)
{
	if (0xffffffff == Cap.currentExtent.width) {
		//!< 0xffffffff �̏ꍇ�̓X���b�v�`�F�C���C���[�W�T�C�Y���E�C���h�E�T�C�Y�����肷�邱�ƂɂȂ� (If 0xffffffff, size of swapchain image determines the size of the window)
		//!< (�N�����v����)������Width, Height���g�p���� (In this case, use argument (clamped) Width and Heigt) 
		return VkExtent2D({ std::max(std::min(Width, Cap.maxImageExtent.width), Cap.minImageExtent.width), std::max(std::min(Height, Cap.minImageExtent.height), Cap.minImageExtent.height) });
	}
	else {
		//!< �����łȂ��ꍇ��currentExtent���g�p���� (Otherwise, use currentExtent)
		return Cap.currentExtent;
	}
}
VkImageUsageFlags VK::SelectImageUsage(const VkSurfaceCapabilitiesKHR& Cap)
{
	//!< (�T�|�[�g����Ă����)�C���[�W�N���A�p�Ƃ��� VK_IMAGE_USAGE_TRANSFER_DST_BIT �����ĂĂ��� (If supported, set VK_IMAGE_USAGE_TRANSFER_DST_BIT for image clear)
	return VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | (VK_IMAGE_USAGE_TRANSFER_DST_BIT & Cap.supportedUsageFlags);
}
VkSurfaceTransformFlagBitsKHR VK::SelectSurfaceTransform(const VkSurfaceCapabilitiesKHR& Cap)
{
	const auto Desired = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
	return (Desired & Cap.supportedTransforms) ? Desired : Cap.currentTransform;
}
VkPresentModeKHR VK::SelectSurfacePresentMode(VkPhysicalDevice PD, VkSurfaceKHR Surface)
{
	uint32_t Count;
	VERIFY_SUCCEEDED(vkGetPhysicalDeviceSurfacePresentModesKHR(PD, Surface, &Count, nullptr));
	assert(Count && "Present mode count is zero");
	std::vector<VkPresentModeKHR> PMs(Count);
	VERIFY_SUCCEEDED(vkGetPhysicalDeviceSurfacePresentModesKHR(PD, Surface, &Count, PMs.data()));

	//!< �\�Ȃ� VK_PRESENT_MODE_MAILBOX_KHR ��I���A�����łȂ���� VK_PRESENT_MODE_FIFO_KHR ��I��
	/**
	@brief VkPresentModeKHR
	* VK_PRESENT_MODE_IMMEDIATE_KHR		... vsync��҂��Ȃ��̂Ńe�A�����O���N���� (Tearing happen, no vsync wait)
	* VK_PRESENT_MODE_MAILBOX_KHR		... �L���[�� 1 �ŏ�ɍŐV�ŏ㏑�������Avsync�ōX�V����� (Queue is 1, and always update to new image and updated on vsync)
	* VK_PRESENT_MODE_FIFO_KHR			... VulkanAPI ���K���T�|�[�g���� vsync�ōX�V (VulkanAPI always support this, updated on vsync)
	* VK_PRESENT_MODE_FIFO_RELAXED_KHR	... FIFO�ɍ݌ɂ�����ꍇ�� vsync��҂��A�Ԃɍ���Ȃ��ꍇ�͑����ɍX�V����e�A�����O���N���� (If FIFO is not empty wait vsync. but if empty, updated immediately and tearing will happen)
	*/
	const VkPresentModeKHR SelectedPresentMode = [&]() {
		for (auto i : PMs) {
			if (VK_PRESENT_MODE_MAILBOX_KHR == i) {
				//!< �\�Ȃ� MAILBOX (If possible, want to use MAILBOX)
				return i;
			}
		}
		for (auto i : PMs) {
			if (VK_PRESENT_MODE_FIFO_KHR == i) {
				//!< FIFO �� VulkanAPI ���K���T�|�[�g���� (VulkanAPI always support FIFO)
				return i;
			}
		}
		assert(false && "Not foud");
		return PMs[0];
	}();

	Log("\tPresent Mode\n");
#define VK_PRESENT_MODE_ENTRY(entry) case VK_PRESENT_MODE_##entry##_KHR: Logf("%s\n", #entry); break
	for (auto i : PMs) {
		Logf("\t\t%s", SelectedPresentMode == i ? "-> " : "");
		switch (i) {
		default: assert(0 && "Unknown VkPresentMode"); break;
		VK_PRESENT_MODE_ENTRY(IMMEDIATE);	
		VK_PRESENT_MODE_ENTRY(MAILBOX);		
		VK_PRESENT_MODE_ENTRY(FIFO);		
		VK_PRESENT_MODE_ENTRY(FIFO_RELAXED);
		}
#undef VK_PRESENT_MODE_ENTRY
	}

	return SelectedPresentMode;
}
void VK::CreateSwapchain(VkPhysicalDevice PD, VkSurfaceKHR Surface, const uint32_t Width, const uint32_t Height)
{
	VkSurfaceCapabilitiesKHR SurfaceCap;
	VERIFY_SUCCEEDED(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(PD, Surface, &SurfaceCap));

	Log("\tSurfaceCapabilities\n");
	Logf("\t\tminImageCount = %d\n", SurfaceCap.minImageCount);
	Logf("\t\tmaxImageCount = %d\n", SurfaceCap.maxImageCount);
	Logf("\t\tcurrentExtent = %d x %d\n", SurfaceCap.minImageExtent.width, SurfaceCap.currentExtent.height);
	Logf("\t\tminImageExtent = %d x %d\n", SurfaceCap.currentExtent.width, SurfaceCap.minImageExtent.height);
	Logf("\t\tmaxImageExtent = %d x %d\n", SurfaceCap.maxImageExtent.width, SurfaceCap.maxImageExtent.height);
	Logf("\t\tmaxImageArrayLayers = %d\n", SurfaceCap.maxImageArrayLayers);
	Log("\t\tsupportedTransforms = ");
#define VK_SURFACE_TRANSFORM_ENTRY(entry) if(SurfaceCap.supportedTransforms & VK_SURFACE_TRANSFORM_##entry##_BIT_KHR) { Logf("%s | ", #entry); }
	VK_SURFACE_TRANSFORM_ENTRY(IDENTITY);
	VK_SURFACE_TRANSFORM_ENTRY(ROTATE_90);
	VK_SURFACE_TRANSFORM_ENTRY(ROTATE_180);
	VK_SURFACE_TRANSFORM_ENTRY(ROTATE_270);
	VK_SURFACE_TRANSFORM_ENTRY(HORIZONTAL_MIRROR);
	VK_SURFACE_TRANSFORM_ENTRY(HORIZONTAL_MIRROR_ROTATE_90);
	VK_SURFACE_TRANSFORM_ENTRY(HORIZONTAL_MIRROR_ROTATE_180);
	VK_SURFACE_TRANSFORM_ENTRY(HORIZONTAL_MIRROR_ROTATE_270);
	VK_SURFACE_TRANSFORM_ENTRY(INHERIT);
#undef VK_SURFACE_TRANSFORM_ENTRY
	Log("\n");
	Log("\t\tcurrentTransform = ");
#define VK_SURFACE_TRANSFORM_ENTRY(entry) if(SurfaceCap.currentTransform == VK_SURFACE_TRANSFORM_##entry##_BIT_KHR) { Logf("%s\n", #entry); }
	VK_SURFACE_TRANSFORM_ENTRY(IDENTITY);
	VK_SURFACE_TRANSFORM_ENTRY(ROTATE_90);
	VK_SURFACE_TRANSFORM_ENTRY(ROTATE_180);
	VK_SURFACE_TRANSFORM_ENTRY(ROTATE_270);
	VK_SURFACE_TRANSFORM_ENTRY(HORIZONTAL_MIRROR);
	VK_SURFACE_TRANSFORM_ENTRY(HORIZONTAL_MIRROR_ROTATE_90);
	VK_SURFACE_TRANSFORM_ENTRY(HORIZONTAL_MIRROR_ROTATE_180);
	VK_SURFACE_TRANSFORM_ENTRY(HORIZONTAL_MIRROR_ROTATE_270);
	VK_SURFACE_TRANSFORM_ENTRY(INHERIT);
#undef VK_SURFACE_TRANSFORM_ENTRY
	Log("\t\tsupportedCompositeAlpha = ");
#define VK_COMPOSITE_ALPHA_ENTRY(entry) if(SurfaceCap.supportedCompositeAlpha & VK_COMPOSITE_ALPHA_##entry##_BIT_KHR) { Logf("%s | ", #entry); }
	VK_COMPOSITE_ALPHA_ENTRY(OPAQUE);
	VK_COMPOSITE_ALPHA_ENTRY(PRE_MULTIPLIED);
	VK_COMPOSITE_ALPHA_ENTRY(POST_MULTIPLIED);
	VK_COMPOSITE_ALPHA_ENTRY(INHERIT);
#undef VK_COMPOSITE_ALPHA_ENTRY
	Log("\n");
	Log("\t\tsupportedUsageFlags = ");
#define VK_IMAGE_USAGE_ENTRY(entry) if(SurfaceCap.supportedUsageFlags & VK_IMAGE_USAGE_##entry) { Logf("%s | ", #entry); }
	VK_IMAGE_USAGE_ENTRY(TRANSFER_SRC_BIT);
	VK_IMAGE_USAGE_ENTRY(TRANSFER_DST_BIT);
	VK_IMAGE_USAGE_ENTRY(SAMPLED_BIT);
	VK_IMAGE_USAGE_ENTRY(STORAGE_BIT);
	VK_IMAGE_USAGE_ENTRY(COLOR_ATTACHMENT_BIT);
	VK_IMAGE_USAGE_ENTRY(DEPTH_STENCIL_ATTACHMENT_BIT);
	VK_IMAGE_USAGE_ENTRY(TRANSIENT_ATTACHMENT_BIT);
	VK_IMAGE_USAGE_ENTRY(INPUT_ATTACHMENT_BIT);
	VK_IMAGE_USAGE_ENTRY(SHADING_RATE_IMAGE_BIT_NV);
	VK_IMAGE_USAGE_ENTRY(FRAGMENT_DENSITY_MAP_BIT_EXT);
#undef VK_IMAGE_USAGE_ENTRY
	Log("\n");

	//!< �Œ����1��������肽���A�������ő�l�ŃN�����v����(maxImageCount ��0�̏ꍇ�͏������)
	const auto ImageCount = (std::min)(SurfaceCap.minImageCount + 1, 0 == SurfaceCap.maxImageCount ? UINT32_MAX : SurfaceCap.maxImageCount);
	Logf("\t\t\tImagCount = %d\n", ImageCount);

	//!< �T�[�t�F�X�̃t�H�[�}�b�g��I��
	const auto SurfaceFormat = SelectSurfaceFormat(PD, Surface);
	ColorFormat = SurfaceFormat.format; //!< �J���[�t�@�[�}�b�g�͊o���Ă���

	//!< �T�[�t�F�X�̃T�C�Y��I��
	SurfaceExtent2D = SelectSurfaceExtent(SurfaceCap, Width, Height);
	Logf("\t\t\tSurfaceExtent = %d x %d\n", SurfaceExtent2D.width, SurfaceExtent2D.height);

	//!< ���C���[�A�X�e���I�����_�����O�����������ꍇ��1�ȏ�ɂȂ邪�A�����ł�1
	uint32_t ImageArrayLayers = 1;

	//!< �T�[�t�F�X�̎g�p�@ (Surface usage)
	const auto ImageUsage = SelectImageUsage(SurfaceCap);

	//!< �O���t�B�b�N�ƃv���[���g�̃L���[�t�@�~�����قȂ�ꍇ�̓L���[�t�@�~���C���f�b�N�X�̔z�񂪕K�v�A�܂� VK_SHARING_MODE_CONCURRENT ���w�肷�邱��
	//!< (������ VK_SHARING_MODE_CONCURRENT �ɂ���ƃp�t�H�[�}���X��������ꍇ������)
	std::vector<uint32_t> QueueFamilyIndices;
	if (GraphicsQueueFamilyIndex != PresentQueueFamilyIndex) {
		QueueFamilyIndices.push_back(GraphicsQueueFamilyIndex);
		QueueFamilyIndices.push_back(PresentQueueFamilyIndex);
	}

	//!< �T�[�t�F�X����]�A���]�������邩�ǂ��� (Rotate, mirror surface or not)
	const auto SurfaceTransform = SelectSurfaceTransform(SurfaceCap);
	
	//!< �T�[�t�F�X�̃v���[���g���[�h��I��
	const auto SurfacePresentMode = SelectSurfacePresentMode(PD, Surface);

	//!< �����̂͌�ŊJ������̂� OldSwapchain �Ɋo���Ă��� (�Z�b�e�B���O��ύX���ăX���b�v�`�F�C�����č쐬����ꍇ���ɔ�����)
	auto OldSwapchain = Swapchain;
	const VkSwapchainCreateInfoKHR SwapchainCreateInfo = {
		VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
		nullptr,
		0,
		Surface,
		ImageCount,
		SurfaceFormat.format, SurfaceFormat.colorSpace,
		SurfaceExtent2D,
		ImageArrayLayers,
		ImageUsage,
		QueueFamilyIndices.empty() ? VK_SHARING_MODE_EXCLUSIVE : VK_SHARING_MODE_CONCURRENT, static_cast<uint32_t>(QueueFamilyIndices.size()), QueueFamilyIndices.data(),
		SurfaceTransform,
		VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
		SurfacePresentMode,
		VK_TRUE,
		OldSwapchain
	};
	VERIFY_SUCCEEDED(vkCreateSwapchainKHR(Device, &SwapchainCreateInfo, GetAllocationCallbacks(), &Swapchain));

	LogOK("CreateSwapchain");

	//!< (�����)�O�̂�͔j��
	if (VK_NULL_HANDLE != OldSwapchain) {
		for (auto i : SwapchainImageViews) {
			vkDestroyImageView(Device, i, GetAllocationCallbacks());
		}
		SwapchainImageViews.clear();
		vkDestroySwapchainKHR(Device, OldSwapchain, GetAllocationCallbacks());
	}

	//!< �X���b�v�`�F�C���C���[�W�̎擾
	uint32_t SwapchainImageCount;
	VERIFY_SUCCEEDED(vkGetSwapchainImagesKHR(Device, Swapchain, &SwapchainImageCount, nullptr));
	assert(SwapchainImageCount && "Swapchain image count is zero");
	SwapchainImages.resize(SwapchainImageCount);
	VERIFY_SUCCEEDED(vkGetSwapchainImagesKHR(Device, Swapchain, &SwapchainImageCount, SwapchainImages.data()));

	LogOK("GetSwapchainImages");
}

/**
@note Vulakn�ł́A1�̃R�}���h�o�b�t�@�ŕ����̃X���b�v�`�F�C���C���[�W���܂Ƃ߂ď����ł�����ۂ�
*/
void VK::InitializeSwapchainImage(const VkCommandBuffer CommandBuffer, const VkClearColorValue* ClearColorValue)
{
	VERIFY_SUCCEEDED(vkBeginCommandBuffer(CommandBuffer, &CommandBufferBeginInfo_OneTime)); {
		for (auto& i : SwapchainImages) {
			if (nullptr == ClearColorValue) {
				const VkImageMemoryBarrier ImageMemoryBarrier_UndefinedToPresent = {
					VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
					nullptr,
					0,
					VK_ACCESS_MEMORY_READ_BIT,
					VK_IMAGE_LAYOUT_UNDEFINED, //!<�u���݂̃��C�A�E�g�v�܂��́uUNDEFINED�v���w�肷�邱�ƁA�C���[�W�R���e���c��ێ��������ꍇ�́uUNDEFINED�v�̓_��         
					VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, //!< �v���[���e�[�V�����\�� VK_IMAGE_LAYOUT_PRESENT_SRC_KHR ��
					PresentQueueFamilyIndex,
					PresentQueueFamilyIndex,
					i,
					ImageSubresourceRange_Color
				};
				vkCmdPipelineBarrier(CommandBuffer,
					VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT/*VK_PIPELINE_STAGE_TRANSFER_BIT*/,
					0,
					0, nullptr,
					0, nullptr,
					1, &ImageMemoryBarrier_UndefinedToPresent);
			}
			else {
				//!< �N���A�J���[���w�肳��Ă���ꍇ If clear color is specified
				const VkImageMemoryBarrier ImageMemoryBarrier_UndefinedToTransfer = {
					VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
					nullptr,
					0,
					VK_ACCESS_TRANSFER_WRITE_BIT,
					VK_IMAGE_LAYOUT_UNDEFINED, //!<�u���݂̃��C�A�E�g�v�܂��́uUNDEFINED�v���w�肷�邱��        
					VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, //!< �f�X�e�B�l�[�V������
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
					vkCmdClearColorImage(CommandBuffer, i, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, ClearColorValue, 1, &ImageSubresourceRange_Color);
				}
				const VkImageMemoryBarrier ImageMemoryBarrier_TransferToPresent = {
					VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
					nullptr,
					VK_ACCESS_TRANSFER_WRITE_BIT,
					VK_ACCESS_MEMORY_READ_BIT,
					VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, //!< �f�X�e�B�l�[�V��������
					VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, //!< �v���[���e�[�V�����\�� VK_IMAGE_LAYOUT_PRESENT_SRC_KHR ��
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
		}
	} VERIFY_SUCCEEDED(vkEndCommandBuffer(CommandBuffer));

	const VkSubmitInfo SubmitInfo = {
		VK_STRUCTURE_TYPE_SUBMIT_INFO,
		nullptr,
		0, nullptr, nullptr,
		1,  &CommandBuffer,
		0, nullptr
	};
	VERIFY_SUCCEEDED(vkQueueSubmit(GraphicsQueue, 1, &SubmitInfo, VK_NULL_HANDLE));
	VERIFY_SUCCEEDED(vkQueueWaitIdle(GraphicsQueue));

	LogOK("InitializeSwapchainImage");
}
void VK::InitializeSwapchain()
{
#if 1
	//!< �C���[�W�̏����� Initialize images
	//InitializeSwapchainImage(CommandBuffers[0]);
	InitializeSwapchainImage(CommandBuffers[0], &Colors::Red);
#endif
}
void VK::ResizeSwapchain(const uint32_t Width, const uint32_t Height)
{
	//!< #VK_TODO �X���b�v�`�F�C���̃��T�C�Y�Ή�
	if (VK_NULL_HANDLE != Device) {
		VERIFY_SUCCEEDED(vkDeviceWaitIdle(Device));
	}

	//if (!CommandPools.empty() && !CommandBuffers.empty()) {
	//	const auto CP = CommandPools[0]; //!< �����0�Ԃ̃R�}���h�v�[�����ߑł� #VK_TODO 
	//	vkFreeCommandBuffers(Device, CP, static_cast<uint32_t>(CommandBuffers.size()), CommandBuffers.data());
	//}
	//CommandBuffers.clear();
	//std::for_each(CommandPools.begin(), CommandPools.end(), [&](const VkCommandPool rhs) { vkDestroyCommandPool(Device, rhs, GetAllocationCallbacks()); });
	//CommandPools.clear();

	CreateSwapchain(GetCurrentPhysicalDevice(), Surface);
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

	LogOK("CreateSwapchainImageView");
}

void VK::CreateDepthStencil(const uint32_t Width, const uint32_t Height, const VkFormat DepthFormat)
{
	CreateDepthStencilImage(Width, Height, DepthFormat);
	CreateDepthStencilDeviceMemory();
	CreateDepthStencilView(DepthFormat);

	LogOK("CreateDepthStencil");
}

void VK::CreateDepthStencilImage(const uint32_t Width, const uint32_t Height, const VkFormat DepthFormat)
{
	assert(IsSupportedDepthFormat(GetCurrentPhysicalDevice(), DepthFormat) && "Not supported depth format");

	const VkExtent3D Extent3D = {
		Width, Height, 1
	};
	const VkImageUsageFlags Usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
	CreateImage(&DepthStencilImage, Usage, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_TYPE_2D, DepthFormat, Extent3D, 1, 1);

#ifdef DEBUG_STDOUT
	std::cout << "\t" << "DepthStencilImage" << std::endl;
#endif
}
void VK::CreateDepthStencilDeviceMemory()
{
	CreateDeviceLocalMemory(&DepthStencilDeviceMemory, DepthStencilImage);

	BindMemory(DepthStencilImage, DepthStencilDeviceMemory/*, 0*/);

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

	LogOK("CreateViewport");
}

void VK::CreateIndirectBuffer(VkBuffer* Buffer, VkDeviceMemory* DeviceMemory, const VkDeviceSize Size, const void* Source, const VkCommandBuffer CB)
{
	VkBuffer StagingBuffer = VK_NULL_HANDLE;
	VkDeviceMemory StagingDeviceMemory = VK_NULL_HANDLE;
	
	//!< �z�X�g�r�W�u���̃o�b�t�@�ƃ��������쐬���A�����փf�[�^���R�s�[���� Create host visible buffer and memory, and copy data
	CreateBuffer(&StagingBuffer, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, Size);
	CreateHostVisibleMemory(&StagingDeviceMemory, StagingBuffer);
	CopyToHostVisibleMemory(StagingDeviceMemory, Size, Source);
	BindMemory(StagingBuffer, StagingDeviceMemory);

	//!< �f�o�C�X���[�J���̃o�b�t�@�ƃ��������쐬 Create device local buffer and memory
	CreateBuffer(Buffer, VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, Size);
	CreateDeviceLocalMemory(DeviceMemory, *Buffer);
	BindMemory(*Buffer, *DeviceMemory);

	//!< �z�X�g�r�W�u������f�o�C�X���[�J���ւ̃R�s�[�R�}���h�𔭍s Submit copy command host visible to device local
	SubmitCopyBuffer(CB, StagingBuffer, *Buffer, VK_ACCESS_INDIRECT_COMMAND_READ_BIT, VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT/*Dispatch�̏ꍇ������ŗǂ��H*/, Size);

	if (VK_NULL_HANDLE != StagingDeviceMemory) {
		vkFreeMemory(Device, StagingDeviceMemory, GetAllocationCallbacks());
	}
	if (VK_NULL_HANDLE != StagingBuffer) {
		vkDestroyBuffer(Device, StagingBuffer, GetAllocationCallbacks());
	}
}

#if 0
void VK::CreateStorageBuffer()
{
	const auto Size = 256;

	VkBuffer Buffer = VK_NULL_HANDLE;
	VkDeviceMemory DeviceMemory = VK_NULL_HANDLE;

	[&](VkBuffer* Buffer, VkDeviceMemory* DeviceMemory, const VkDeviceSize Size) {
		const auto Usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;

		CreateBuffer(Buffer, Usage, Size);
		CreateDeviceLocalMemory(DeviceMemory, *Buffer);
		BindMemory(*Buffer, *DeviceMemory);

		//!< View �͕K�v�Ȃ� No need view

	}(&Buffer, &DeviceMemory, Size);

	if (VK_NULL_HANDLE != DeviceMemory) {
		vkFreeMemory(Device, DeviceMemory, GetAllocationCallbacks());
	}
	if (VK_NULL_HANDLE != Buffer) {
		vkDestroyBuffer(Device, Buffer, GetAllocationCallbacks());
	}
}
void VK::CreateUniformTexelBuffer()
{
	const auto Size = 256;

	VkBuffer Buffer = VK_NULL_HANDLE;
	VkDeviceMemory DeviceMemory = VK_NULL_HANDLE;
	VkBufferView View = VK_NULL_HANDLE;

	[&](VkBuffer* Buffer, VkDeviceMemory* DeviceMemory, VkBufferView* View, const VkDeviceSize Size) {
		const auto Usage = VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT;

		CreateBuffer(Buffer, Usage, Size);
		CreateDeviceLocalMemory(DeviceMemory, *Buffer);

		BindDeviceMemory(*Buffer, *DeviceMemory);

		//!< UniformTexelBuffer �̏ꍇ�́A�t�H�[�}�b�g���w�肷��K�v�����邽�߁A�r���[���쐬���� UniformTexelBuffer need format, so create view
		const auto Format = VK_FORMAT_R8G8B8A8_UNORM;
		ValidateFormatProperties(Usage, Format);
		CreateBufferView(View, *Buffer, Format);
	}(&Buffer, &DeviceMemory, &View, Size);

	if (VK_NULL_HANDLE != DeviceMemory) {
		vkFreeMemory(Device, DeviceMemory, GetAllocationCallbacks());
	}
	if (VK_NULL_HANDLE != Buffer) {
		vkDestroyBuffer(Device, Buffer, GetAllocationCallbacks());
	}
	if (VK_NULL_HANDLE != View) {
		vkDestroyBufferView(Device, View, GetAllocationCallbacks());
	}
}
void VK::CreateStorageTexelBuffer()
{
	const auto Size = 256;

	VkBuffer Buffer = VK_NULL_HANDLE;
	VkDeviceMemory DeviceMemory = VK_NULL_HANDLE;
	VkBufferView View = VK_NULL_HANDLE;
	
	[&](VkBuffer* Buffer, VkDeviceMemory* DeviceMemory, VkBufferView* View, const VkDeviceSize Size) {
		const auto Usage = VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT;

		CreateBuffer(Buffer, Usage, Size);
		CreateDeviceLocalMemory(DeviceMemory, *Buffer);
		BindDeviceMemory(*Buffer, *DeviceMemory);

		//!< UniformStorageBuffer �̏ꍇ�́A�t�H�[�}�b�g���w�肷��K�v�����邽�߁A�r���[���쐬���� UniformStorageBuffer need format, so create view
		const auto Format = VK_FORMAT_R8G8B8A8_UNORM;
		ValidateFormatProperties(Usage, Format);
		CreateBufferView(View, *Buffer, Format);
	}(&Buffer, &DeviceMemory, &View, Size);
	
	if (VK_NULL_HANDLE != DeviceMemory) {
		vkFreeMemory(Device, DeviceMemory, GetAllocationCallbacks());
	}
	if (VK_NULL_HANDLE != Buffer) {
		vkDestroyBuffer(Device, Buffer, GetAllocationCallbacks());
	}
	if (VK_NULL_HANDLE != View) {
		vkDestroyBufferView(Device, View, GetAllocationCallbacks());
	}
}
#endif

/**
@brief �V�F�[�_�Ƃ̃o�C���f�B���O (DX::CreateRootSignature()����)
@note �f�X�N���v�^���g�p���Ȃ��ꍇ�ł��A�f�X�N���v�^�Z�b�g���C�A�E�g���͍̂쐬���Ȃ��Ă͂Ȃ�Ȃ�
@note �V�F�[�_����̃A�N�Z�X���� set ��VkDescriptorSetLayout�ԍ��Abinding ��(VkDescriptorSetLayout����)VkDescriptorSetLayoutBinding�ԍ� �ɑ�������
(set = VkDescriptorSetLayout�ԍ�, binding = (VkDescriptorSetLayout����)VkDescriptorSetLayoutBinding�ԍ�)
*/
void VK::CreateDescriptorSetLayout()
{
	//!< binding = [0, DescriptorSetLayoutBindings.size()-1]
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
	VERIFY_SUCCEEDED(vkCreateDescriptorSetLayout(Device, &DescriptorSetLayoutCreateInfo, GetAllocationCallbacks(), &DescriptorSetLayout));
	//!< set = [0, DescriptorSetLayouts.size()-1]
	DescriptorSetLayouts.push_back(DescriptorSetLayout);

	LogOK("CreateDescriptorSetLayout");
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
		//!< �v�[�����쐬 Create pool
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
			0/*VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT*/, //!< �f�X�N���v�^�Z�b�g���X�ɉ���������ꍇ�Ɏw��(�v�[�����Ƃ̏ꍇ�͕s�v)
			MaxSets,
			static_cast<uint32_t>(DescriptorPoolSizes.size()), DescriptorPoolSizes.data()
		};
		VERIFY_SUCCEEDED(vkCreateDescriptorPool(Device, &DescriptorPoolCreateInfo, GetAllocationCallbacks(), &DescriptorPool));
		assert(VK_NULL_HANDLE != DescriptorPool && "Failed to create descriptor pool");

		//!< �v�[������f�X�N���v�^�Z�b�g���쐬 Create descriptor set from pool
		const VkDescriptorSetAllocateInfo DescriptorSetAllocateInfo = {
			VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
			nullptr,
			DescriptorPool,
			static_cast<uint32_t>(DescriptorSetLayouts.size()), DescriptorSetLayouts.data()
		};
		DescriptorSets.resize(DescriptorSetLayouts.size());
		VERIFY_SUCCEEDED(vkAllocateDescriptorSets(Device, &DescriptorSetAllocateInfo, DescriptorSets.data()));

		LogOK("CreateDescriptorSet");
	}
}

/**
@brief �f�X�N���v�^�Z�b�g��������
�p�C�v���C�����C�A�E�g�S�̂�128 Byte (�n�[�h�������΂���ȏ�g����ꍇ������ ex)GTX970M ... 256byte)

�e�V�F�[�_�X�e�[�W��1�̃v�b�V���R���X�^���g�ɂ����A�N�Z�X�ł��Ȃ�
�e�X�̃V�F�[�_�X�e�[�W�����ʂ̃����W�������Ȃ��悤�ȁu���[�X�g�P�[�X�v�ł� 128 / 5(�V�F�[�_�X�e�[�W)�� 1�V�F�[�_�X�e�[�W�� 25-6 Byte���x�ɂȂ�
*/
void VK::CreatePushConstantRanges()
{
#if 1
	//!< �����ł̓o�[�e�b�N�X�A�t���O�����g�V�F�[�_�� 64byte �m�� (�g�p���Ȃ����ɂ͖��Ȃ�) In this case, assign 64byte for vertex and fragment shader
	PushConstantRanges = {
		{ VK_SHADER_STAGE_VERTEX_BIT, 0, 64 },
		{ VK_SHADER_STAGE_FRAGMENT_BIT, 64, 64 },
	};
#else
	PushConstantRanges = {};
#endif
}

void VK::DestroyFramebuffer()
{
	for (auto i : Framebuffers) {
		vkDestroyFramebuffer(Device, i, GetAllocationCallbacks());
	}
	Framebuffers.clear();
}

/**
@brief �V�F�[�_�R���p�C���A�����N�̓p�C�v���C���I�u�W�F�N�g�쐬���ɍs���� Shader compilation and linkage is performed during the pipeline object creation
*/
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
				static_cast<size_t>(Size), reinterpret_cast<uint32_t*>(Data)
			};
			VERIFY_SUCCEEDED(vkCreateShaderModule(Device, &ModuleCreateInfo, GetAllocationCallbacks(), &ShaderModule));

			delete[] Data;
		}
		In.close();
	}
	return ShaderModule;
}

/**
@brief DX�ɍ��킹�邽�߁A�����ł� VkDynamicState ���g�p���Ă��� For compatibility with DX, we use VkDynamicState
�����ł͌��݂̂��w�肵�� nullptr ���w�肵�Ă���A��� vkCmdSetViewport(), vkCmdSetScissor() �Ŏw�肷�邱�� We specifying only count, and set nulloptr, later use vkCmdSetViewport(), vkCmdSetScissor()
2�ȏ�̃r���[�|�[�g���g�p����ɂ̓f�o�C�X�t�B�[�`���[ multiViewport ���L���ł��邱�� If we use 2 or more viewport device feature multiViewport must be enabled
�r���[�|�[�g�̃C���f�b�N�X�̓W�I���g���V�F�[�_�Ŏw�肷�� Viewport index is specified in geometry shader
*/
void VK::CreateViewportState_Dynamic(VkPipelineViewportStateCreateInfo& PipelineViewportStateCreateInfo, const uint32_t Count) const
{
	PipelineViewportStateCreateInfo = {
		VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
		nullptr,
		0,
		Count, nullptr,
		Count, nullptr
	};
}

VkPipelineCache VK::LoadPipelineCache(const std::wstring& Path) const
{
	VkPipelineCache PipelineCache = VK_NULL_HANDLE;
	size_t Size = 0;

	std::ifstream In(Path.c_str(), std::ios::in | std::ios::binary);
	if (!In.fail()) {
		In.seekg(0, std::ios_base::end);
		Size = In.tellg();
		In.seekg(0, std::ios_base::beg);
	}

	char* Data = nullptr;
	if (Size) {
		Data = new char[Size];
		In.read(Data, Size);
	}

	const VkPipelineCacheCreateInfo PipelineCacheCreateInfo = {
		VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO,
		nullptr,
		0,
		Size, Data
	};
	VERIFY_SUCCEEDED(vkCreatePipelineCache(Device, &PipelineCacheCreateInfo, GetAllocationCallbacks(), &PipelineCache));

	if (nullptr != Data) {
		delete[] Data;
	}

	In.close();

	return PipelineCache;
}
void VK::LoadPipelineCaches(const std::wstring& Path, std::vector<VkPipelineCache>& PipelineCaches) const
{
	size_t Size = 0;

	std::ifstream In(Path.c_str(), std::ios::in | std::ios::binary);
	if (!In.fail()) {
		In.seekg(0, std::ios_base::end);
		Size = In.tellg();
		In.seekg(0, std::ios_base::beg);
	}

	char* Data = nullptr;
	if (Size) {
		Data = new char[Size];
		In.read(Data, Size);
	}

	const VkPipelineCacheCreateInfo PipelineCacheCreateInfo = {
		VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO,
		nullptr,
		0,
		Size, Data
	};
	for (auto& i : PipelineCaches) {
		VERIFY_SUCCEEDED(vkCreatePipelineCache(Device, &PipelineCacheCreateInfo, GetAllocationCallbacks(), &i));
	}

	if (nullptr != Data) {
		delete[] Data;
	}

	In.close();
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
				Out.close();
			}

			delete[] Data;
		}
	}
}

VkPipelineCache VK::CreatePipelineCache() const
{
	VkPipelineCache PipelineCache = VK_NULL_HANDLE;
	const VkPipelineCacheCreateInfo PipelineCacheCreateInfo = {
		VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO,
		nullptr,
		0,
		0, nullptr
	};
	VERIFY_SUCCEEDED(vkCreatePipelineCache(Device, &PipelineCacheCreateInfo, GetAllocationCallbacks(), &PipelineCache));
	return PipelineCache;
}
void VK::CreatePipelineCaches(std::vector<VkPipelineCache>& PipelineCaches) const
{
	const VkPipelineCacheCreateInfo PipelineCacheCreateInfo = {
		VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO,
		nullptr,
		0,
		0, nullptr
	};
	for (auto& i : PipelineCaches) {
		VERIFY_SUCCEEDED(vkCreatePipelineCache(Device, &PipelineCacheCreateInfo, GetAllocationCallbacks(), &i));
	}
}

void VK::CreatePipeline()
{
#if 0
	//!< �X���b�h�Ńp�C�v���C�� (ThreadCount * PipelineCountPerThread ��) ���쐬����� (�p�C�v���C���L���b�V�������p����)
	const auto ThreadCount = 10;
	const auto PipelineCountPerThread = 1;

	//!< �p�C�v���C���L���b�V�����t�@�C������ǂݍ��� (�X���b�h��������(�������̂�)�p�ӂ���)
	std::vector<VkPipelineCache> PipelineCaches(ThreadCount);
	const auto PCOPath = GetBasePath() + TEXT(".pco");
	LoadPipelineCaches(PCOPath, PipelineCaches);

	//!< �p�C�v���C���i�[��
	std::vector<std::vector<VkPipeline>> Pipelines(ThreadCount);
	for (auto i : Pipelines) { 
		i.resize(PipelineCountPerThread); 
	}

	//!< �p�C�v���C���L���b�V�����g�p���āA�e�X�̃X���b�h�Ńp�C�v���C�����쐬����
	std::vector<std::thread> Threads(ThreadCount);
	for (auto i = 0; i < Threads.size(); ++i) {
		Threads[i] = std::thread::thread([&](std::vector<VkPipeline>& Pipelines, VkPipelineCache PipelineCache) {
			//!< #VK_TODO
			//!< �p�C�v���C�����쐬�̏����������ɏ��� Create pipeline here 
			//std::vector<VkGraphicsPipelineCreateInfo> GraphicsPipelineCreateInfos(Pipelines.size());
			// ...
			//VERIFY_SUCCEEDED(vkCreateGraphicsPipelines(Device,
			//	PipelineCache,
			//	static_cast<uint32_t>(GraphicsPipelineCreateInfos.size()), GraphicsPipelineCreateInfos.data(), 
			//	GetAllocationCallbacks(),
			//	Pipelines.data()));

			//std::vector<VkComputePipelineCreateInfo> ComputePipelineCreateInfos(Pipelines.size());
			// ...
			//VERIFY_SUCCEEDED(vkCreateComputePipelines(Device,
			//	PipelineCache,
			//	static_cast<uint32_t>(ComputePipelineCreateInfos.size()), ComputePipelineCreateInfos.data(),
			//	GetAllocationCallbacks(),
			//	Pipelines.data()));

			std::cout << "Creating pipelines in thread\n"; 
		}, Pipelines[i], PipelineCaches[i]);
	}
	for (auto& i : Threads) {
		i.join();
	}

	//!< �e�X�̃X���b�h���쐬�����p�C�v���C���L���b�V�����}�[�W���� (�����ł͍Ō�̗v�f�փ}�[�W���Ă���)
	VERIFY_SUCCEEDED(vkMergePipelineCaches(Device, PipelineCaches.back(), static_cast<uint32_t>(PipelineCaches.size() - 1), PipelineCaches.data()));	
	//!< �}�[�W��̃p�C�v���C���L���b�V�����t�@�C���֕ۑ�
	StorePipelineCache(PCOPath, PipelineCaches.back());
	//!< �����j�����ėǂ�
	for (auto& i : PipelineCaches) {
		vkDestroyPipelineCache(Device, i, GetAllocationCallbacks());
	}
#endif

	CreatePipeline_Graphics();
}

void VK::CreatePipeline_Graphics()
{
#ifdef _DEBUG
	PerformanceCounter PC("CreatePipeline_Graphics : ");
#endif

	//!< �V�F�[�_
	std::vector<VkShaderModule> ShaderModules;
	std::vector<VkPipelineShaderStageCreateInfo> PipelineShaderStageCreateInfos;
	CreateShader(ShaderModules, PipelineShaderStageCreateInfos);

	//!< �o�[�e�b�N�X�C���v�b�g
	std::vector<VkVertexInputBindingDescription> VertexInputBindingDescriptions;
	std::vector<VkVertexInputAttributeDescription> VertexInputAttributeDescriptions;
	CreateVertexInput(VertexInputBindingDescriptions, VertexInputAttributeDescriptions);
	const VkPipelineVertexInputStateCreateInfo PipelineVertexInputStateCreateInfo = {
		VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
		nullptr,
		0,
		static_cast<uint32_t>(VertexInputBindingDescriptions.size()), VertexInputBindingDescriptions.data(),
		static_cast<uint32_t>(VertexInputAttributeDescriptions.size()), VertexInputAttributeDescriptions.data()
	};

	//!< �C���v�b�g�A�Z���u�� (�g�|���W)
	VkPipelineInputAssemblyStateCreateInfo PipelineInputAssemblyStateCreateInfo;
	CreateInputAssembly(PipelineInputAssemblyStateCreateInfo);

	//!< �e�Z���[�V�����X�e�[�g (�e�Z���[�V�������g�p����ꍇ�ɂ́A�s��l�̂܂܂ɂ��Ȃ�����)
	VkPipelineTessellationStateCreateInfo PipelineTessellationStateCreateInfo;
	CreateTessellationState(PipelineTessellationStateCreateInfo);

	//!< �r���[�|�[�g(�V�U�[)
	VkPipelineViewportStateCreateInfo PipelineViewportStateCreateInfo;
	CreateViewportState(PipelineViewportStateCreateInfo);

	const VkPipelineRasterizationStateCreateInfo PipelineRasterizationStateCreateInfo = {
		VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
		nullptr,
		0,
		VK_FALSE, //!< VK_TRUE �ɂ���ɂ̓f�o�C�X�t�B�[�`���[ depthClampEnable ���L���ł��邱��
		VK_FALSE,
		VK_POLYGON_MODE_FILL, //!< LINE �� POINT ��L���ɂ���ɂ̓f�o�C�X�t�B�[�`���[ fillModeNonSolid ���L���ł��邱��
		VK_CULL_MODE_BACK_BIT,
		VK_FRONT_FACE_COUNTER_CLOCKWISE,
		VK_FALSE, 0.0f, 0.0f, 0.0f,
		1.0f //!< 1.0f ���傫�Ȓl���w�肷��ɂ̓f�o�C�X�t�B�[�`���[ widelines ���L���ł��邱��
	};

	//const VkSampleMask SampleMask = 0xffffffff;
	const VkPipelineMultisampleStateCreateInfo PipelineMultisampleStateCreateInfo = {
		VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
		nullptr,
		0,
		VK_SAMPLE_COUNT_1_BIT,
		VK_FALSE, 0.0f, //! VK_TRUE �ɂ���ɂ̓f�o�C�X�t�B�[�`���[ minSampleShading ���L���ł��邱��
		nullptr/*&SampleMask*/, //!< 0xffffffff �̏ꍇ nullptr �ł悢
		VK_FALSE, VK_FALSE //!< alphaToOneEnable �� VK_TRUE �ɂ���ɂ̓f�o�C�X�t�B�[�`���[ alphaToOne ���L���ł��邱��
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
		{ VK_STENCIL_OP_KEEP, VK_STENCIL_OP_KEEP, VK_STENCIL_OP_KEEP, VK_COMPARE_OP_NEVER, 0, 0, 0 },
		{ VK_STENCIL_OP_KEEP, VK_STENCIL_OP_KEEP, VK_STENCIL_OP_KEEP, VK_COMPARE_OP_ALWAYS, 0, 0, 0 },
		0.0f, 1.0f
	};

	//!< �f�o�C�X�t�B�[�`���[ independentBlend ���L���Ŗ����ꍇ�́A�z��̊e�v�f�́u���S�ɓ����l�v�ł��邱�� If device feature independentBlend is not enabled, each array element must be exactly same
	//!< VK_BLEND_FACTOR_SRC1 �n�����g�p����ɂ́A�f�o�C�X�t�B�[�`���[ dualSrcBlend ���L���ł��邱��
	///!< SRC�R���|�[�l���g * SRC�t�@�N�^ OP DST�R���|�[�l���g * DST�t�@�N�^
	const std::vector<VkPipelineColorBlendAttachmentState> PipelineColorBlendAttachmentStates = {
		{
			VK_FALSE,
			VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ONE, VK_BLEND_OP_ADD,
			VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ONE, VK_BLEND_OP_ADD,
			VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
		}
	};
	const VkPipelineColorBlendStateCreateInfo PipelineColorBlendStateCreateInfo = {
		VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
		nullptr,
		0,
		VK_FALSE, VK_LOGIC_OP_COPY, //!< �u�����h���ɘ_���I�y���[�V�������s�� (�u�����h�͖����ɂȂ�) (�����^�A�^�b�`�����g�ɑ΂��Ă̂�)
		static_cast<uint32_t>(PipelineColorBlendAttachmentStates.size()), PipelineColorBlendAttachmentStates.data(),
		{ 1.0f, 1.0f, 1.0f, 1.0f }
	};

	//!< �_�C�i�~�b�N�X�e�[�g
	std::vector<VkDynamicState> DynamicStates;
	CreateDynamicState(DynamicStates);
	const VkPipelineDynamicStateCreateInfo PipelineDynamicStateCreateInfo = {
		VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
		nullptr,
		0,
		static_cast<uint32_t>(DynamicStates.size()), DynamicStates.data()
	};

	//!< �p�C�v���C�����C�A�E�g
	const VkPipelineLayoutCreateInfo PipelineLayoutCreateInfo = {
		VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
		nullptr,
		0,
		static_cast<uint32_t>(DescriptorSetLayouts.size()), DescriptorSetLayouts.data(),
		static_cast<uint32_t>(PushConstantRanges.size()), PushConstantRanges.data()
	};
	VERIFY_SUCCEEDED(vkCreatePipelineLayout(Device, &PipelineLayoutCreateInfo, GetAllocationCallbacks(), &PipelineLayout));
	//!< �{�� PipelineLayout ���쐬������ADescritptorSetLayout �͔j�����Ă��ǂ�
	//!< �������A�������C�A�E�g�� DescriptorSet ���č쐬����悤�ȏꍇ�ɕK�v�ɂȂ�̂ł����ł͎c���Ă������Ƃɂ���

	/**
	@brief �p��
	basePipelineHandle, basePipelineIndex �͓����Ɏg�p�ł��Ȃ��A���ꂼ��g�p���Ȃ��ꍇ�� VK_NULL_HANDLE, -1 ���w��
	�e�ɂ� VK_PIPELINE_CREATE_ALLOW_DERIVATIVES_BIT �t���O���K�v�A�q�ɂ� VK_PIPELINE_CREATE_DERIVATIVE_BIT �t���O���K�v
	�EbasePipelineHandle ... ���ɐe�Ƃ���p�C�v���C�������݂���ꍇ�Ɏw��
	�EbasePipelineIndex ... ���z����Őe�p�C�v���C���������ɍ쐬����ꍇ�A�z����ł̐e�p�C�v���C���̓Y���A�e�̓Y���̕����Ⴂ�l�łȂ��Ƃ����Ȃ�
	*/
	const std::vector<VkGraphicsPipelineCreateInfo> GraphicsPipelineCreateInfos = {
		{
			VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
			nullptr,
#ifdef _DEBUG
			VK_PIPELINE_CREATE_DISABLE_OPTIMIZATION_BIT,
#else
			0,
#endif
			static_cast<uint32_t>(PipelineShaderStageCreateInfos.size()), PipelineShaderStageCreateInfos.data(),
			&PipelineVertexInputStateCreateInfo,
			&PipelineInputAssemblyStateCreateInfo,
			&PipelineTessellationStateCreateInfo,
			&PipelineViewportStateCreateInfo,
			&PipelineRasterizationStateCreateInfo,
			&PipelineMultisampleStateCreateInfo,
			&PipelineDepthStencilStateCreateInfo,
			&PipelineColorBlendStateCreateInfo,
			&PipelineDynamicStateCreateInfo,
			PipelineLayout,
			RenderPass, 0, //!< �w�肵�������_�[�p�X����ł͂Ȃ��A�݊����̂��鑼�̃����_�[�p�X�ł��g�p�\
			VK_NULL_HANDLE, -1 //!< basePipelineHandle, basePipelineIndex
		}
	};

	//!< �p�C�v���C���L���b�V���I�u�W�F�N�g Pipeline Cache Object
	const auto PCOPath = GetBasePath() + TEXT(".pco");
	const auto PipelineCache = LoadPipelineCache(PCOPath);
	//!< �p�C�v���C���쐬���ɃL���b�V�������p�����A�܂�����̍쐬���ʂ��L���b�V�������
	VERIFY_SUCCEEDED(vkCreateGraphicsPipelines(Device, 
		PipelineCache, 
		static_cast<uint32_t>(GraphicsPipelineCreateInfos.size()), GraphicsPipelineCreateInfos.data(), 
		GetAllocationCallbacks(), 
		&Pipeline));
	if (VK_NULL_HANDLE != PipelineCache) {
		StorePipelineCache(PCOPath, PipelineCache);
		vkDestroyPipelineCache(Device, PipelineCache, GetAllocationCallbacks());
	}

	//!< �p�C�v���C�� ���쐬������A�V�F�[�_���W���[�� �͔j�����ėǂ�
	for (auto i : ShaderModules) {
		vkDestroyShaderModule(Device, i, GetAllocationCallbacks());
	}
	ShaderModules.clear();

	LogOK("CreatePipeline_Graphics");
}
void VK::CreatePipeline_Compute()
{
#ifdef _DEBUG
	PerformanceCounter PC("CreatePipeline_Compute : ");
#endif

	std::vector<VkShaderModule> ShaderModules;
	std::vector<VkPipelineShaderStageCreateInfo> PipelineShaderStageCreateInfos;
	CreateShader(ShaderModules, PipelineShaderStageCreateInfos);
	
	const VkPipelineLayoutCreateInfo PipelineLayoutCreateInfo = {
		VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
		nullptr,
		0,
		static_cast<uint32_t>(DescriptorSetLayouts.size()), DescriptorSetLayouts.data(),
		static_cast<uint32_t>(PushConstantRanges.size()), PushConstantRanges.data()
	};
	VERIFY_SUCCEEDED(vkCreatePipelineLayout(Device, &PipelineLayoutCreateInfo, GetAllocationCallbacks(), &PipelineLayout));

	const std::vector<VkComputePipelineCreateInfo> ComputePipelineCreateInfos = {
		{
			VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
			nullptr,
#ifdef _DEBUG
			VK_PIPELINE_CREATE_DISABLE_OPTIMIZATION_BIT,
#else
			0,
#endif
			PipelineShaderStageCreateInfos[0],
			PipelineLayout,
			VK_NULL_HANDLE, -1 //!< basePipelineHandle, basePipelineIndex
		},
	};

	const auto PCOPath = GetBasePath() + TEXT(".pco");
	const auto PipelineCache = LoadPipelineCache(PCOPath);
	VERIFY_SUCCEEDED(vkCreateComputePipelines(Device,
		PipelineCache,
		static_cast<uint32_t>(ComputePipelineCreateInfos.size()), ComputePipelineCreateInfos.data(),
		GetAllocationCallbacks(),
		&Pipeline));
	if (VK_NULL_HANDLE != PipelineCache) {
		StorePipelineCache(PCOPath, PipelineCache);
		vkDestroyPipelineCache(Device, PipelineCache, GetAllocationCallbacks());
	}

	for (auto i : ShaderModules) {
		vkDestroyShaderModule(Device, i, GetAllocationCallbacks());
	}
	ShaderModules.clear();

	LogOK("CreatePipeline_Compute");
}

/**
@brief �N���A Clear
@note �u�����_�[�p�X�O�v�ŃN���A Out of renderpass ... vkCmdClearColorImage()
@note �u�����_�[�p�X�J�n���v�ɃN���A Begining of renderpass ... VkAttachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR, VkRenderPassBeginInfo.pClearValues
@note �u�e�X�̃T�u�p�X�v�ŃN���A Each subpass ... vkCmdClearAttachments()
*/
//!< �u�����_�[�p�X�O�v�ɂăN���A���s��
void VK::ClearColor(const VkCommandBuffer CommandBuffer, const VkImage Image, const VkClearColorValue& Color)
{
	const VkImageMemoryBarrier ImageMemoryBarrier_PresentToTransfer = {
		VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
		nullptr,
		VK_ACCESS_MEMORY_READ_BIT,
		VK_ACCESS_TRANSFER_WRITE_BIT,
		VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		PresentQueueFamilyIndex,
		PresentQueueFamilyIndex,
		Image,
		ImageSubresourceRange_Color
	};
	vkCmdPipelineBarrier(CommandBuffer,
		VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
		0,
		0, nullptr,
		0, nullptr,
		1, &ImageMemoryBarrier_PresentToTransfer);
	{
		//!< vkCmdClearColorImage() �̓����_�[�p�X���ł͎g�p�ł��Ȃ�
		vkCmdClearColorImage(CommandBuffer, Image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &Color, 1, &ImageSubresourceRange_Color);
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
		Image,
		ImageSubresourceRange_Color
	};
	vkCmdPipelineBarrier(CommandBuffer,
		VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
		0,
		0, nullptr,
		0, nullptr,
		1, &ImageMemoryBarrier_TransferToPresent);
}
void VK::ClearDepthStencil(const VkCommandBuffer CommandBuffer, const VkImage Image, const VkClearDepthStencilValue& DepthStencil)
{
	const VkImageMemoryBarrier ImageMemoryBarrier_DepthToTransfer = {
		VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
		nullptr,
		VK_ACCESS_MEMORY_READ_BIT,
		VK_ACCESS_TRANSFER_WRITE_BIT,
		VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		PresentQueueFamilyIndex,
		PresentQueueFamilyIndex,
		Image,
		ImageSubresourceRange_DepthStencil
	};
	vkCmdPipelineBarrier(CommandBuffer,
		VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
		0,
		0, nullptr,
		0, nullptr,
		1, &ImageMemoryBarrier_DepthToTransfer);
	{
		//!< vkCmdClearDepthStencilImage() �̓����_�[�p�X���ł͎g�p�ł��Ȃ�
		vkCmdClearDepthStencilImage(CommandBuffer, Image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &ClearDepthStencilValue, 1, &ImageSubresourceRange_DepthStencil);
	}
	const VkImageMemoryBarrier ImageMemoryBarrier_TransferToDepth = {
		VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
		nullptr,
		VK_ACCESS_TRANSFER_WRITE_BIT,
		VK_ACCESS_MEMORY_READ_BIT,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
		PresentQueueFamilyIndex,
		PresentQueueFamilyIndex,
		Image,
		ImageSubresourceRange_DepthStencil
	};
	vkCmdPipelineBarrier(CommandBuffer,
		VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
		0,
		0, nullptr,
		0, nullptr,
		1, &ImageMemoryBarrier_TransferToDepth);
}
//!<�u�T�u�p�X�v�ɂăN���A����Ƃ��Ɏg��
//!< Draw�R�[���O�Ɏg�p����ƁA�u����Ȃ� VkAttachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR ���g���v�� Warnning ���o��̂Œ���
void VK::ClearColorAttachment(const VkCommandBuffer CommandBuffer, const VkClearColorValue& Color)
{
	const VkClearValue ClearValue = { Color };
	const std::vector<VkClearAttachment> ClearAttachments = {
		{
			VK_IMAGE_ASPECT_COLOR_BIT,
			0, //!< �J���[�A�^�b�`�����g�̃C���f�b�N�X #VK_TODO ���󌈂ߑł�
			ClearValue
		},
	};
	const std::vector<VkClearRect> ClearRects = {
		{
			ScissorRects[0],
			0, 1 //!< �J�n���C���ƃ��C���� #VK_TODO ���󌈂ߑł�
		},
	};
	vkCmdClearAttachments(CommandBuffer,
		static_cast<uint32_t>(ClearAttachments.size()), ClearAttachments.data(),
		static_cast<uint32_t>(ClearRects.size()), ClearRects.data());
}
void VK::ClearDepthStencilAttachment(const VkCommandBuffer CommandBuffer, const VkClearDepthStencilValue& DepthStencil)
{
	VkClearValue ClearValue;
	ClearValue.depthStencil = DepthStencil;
	const std::vector<VkClearAttachment> ClearAttachments = {
		{
			VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT,
			0, //!< �����ł͖��������
			ClearValue
		},
	};
	const std::vector<VkClearRect> ClearRects = {
		{
			ScissorRects[0],
			0, 1 //!< �J�n���C���ƃ��C���� #VK_TODO ���󌈂ߑł�
		},
	};
	vkCmdClearAttachments(CommandBuffer,
		static_cast<uint32_t>(ClearAttachments.size()), ClearAttachments.data(),
		static_cast<uint32_t>(ClearRects.size()), ClearRects.data());
}

void VK::PopulateCommandBuffer(const size_t i)
{
	const auto CB = CommandBuffers[i];
	const auto FB = Framebuffers[i];
	const auto Image = SwapchainImages[i];

	/**
	@brief �R�}���h�o�b�t�@�̃��Z�b�g Reset of command buffer
	* vkBeginCommandBuffer() �Ŏ����I�Ƀ��Z�b�g�����̂ŁA�����I�� vkResetCommandBuffer() ���R�[�����Ȃ��Ă��ǂ�
	* ������ vkResetCommandBuffer() �Ŗ����I�Ƀ��Z�b�g����ꍇ�ɂ́A���������J�����邩�ǂ������w��ł���
	*/
	//VERIFY_SUCCEEDED(vkResetCommandBuffer(CB, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT));
	VERIFY_SUCCEEDED(vkBeginCommandBuffer(CB, &CommandBufferBeginInfo)); {
		//!< �r���[�|�[�g�A�V�U�[
		vkCmdSetViewport(CB, 0, static_cast<uint32_t>(Viewports.size()), Viewports.data());
		vkCmdSetScissor(CB, 0, static_cast<uint32_t>(ScissorRects.size()), ScissorRects.data());

#if 1
		//!< �N���A
		ClearColor(CB, Image, Colors::SkyBlue);
#endif

#ifdef _DEBUG
		//!< �����_�[�G���A�̍Œᗱ�x���m��
		VkExtent2D Granularity;
		vkGetRenderAreaGranularity(Device, RenderPass, &Granularity);
		//!<�u�����̊��ł́v Granularity = { 1, 1 } �������̂łقڂȂ�ł����v�݂����A���ɂ���Ă͒��ӂ��K�v
		assert(ScissorRects[0].extent.width >= Granularity.width && ScissorRects[0].extent.height >= Granularity.height && "ScissorRect is too small");
#endif
		//!< (�����ł�)�����_�[�p�X�J�n���ɃJ���[�̓N���A�����A�f�v�X�̓N���A���Ă��� In this case, not clear color, but clear depth on begining of renderpas
		std::vector<VkClearValue> ClearValues(2);
		//ClearValues[0].color = Colors::SkyBlue; //!< If VkAttachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR, need this
		ClearValues[1].depthStencil = ClearDepthStencilValue;
		const VkRenderPassBeginInfo RenderPassBeginInfo = {
			VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
			nullptr,
			RenderPass,
			FB,
			ScissorRects[0], //!< �t���[���o�b�t�@�̃T�C�Y�ȉ����w��ł���
			static_cast<uint32_t>(ClearValues.size()), ClearValues.data()
		};

		vkCmdBeginRenderPass(CB, &RenderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE); {
			//vkCmdBindPipeline();
			//vkCmdBindDescriptorSets();
			//vkCmdBindVertexBuffers();
			//vkCmdBindIndexBuffer();
			//vkCmdDrawIndirect();

			//!< vkCmdNextSubpass(CommandBuffer, VK_SUBPASS_CONTENTS_INLINE);

			//if (!SCB.empty()) {
			//	vkCmdExecuteCommands(CB, static_cast<uint32_t>(SCB.size()), SCB.data());
			//}
		} vkCmdEndRenderPass(CB);
	} VERIFY_SUCCEEDED(vkEndCommandBuffer(CB));
}

void VK::Draw()
{
	WaitForFence(Fence);

	//!< ���̃C���[�W���擾�ł�����Z�}�t�H���ʒm�����
	VERIFY_SUCCEEDED(vkAcquireNextImageKHR(Device, Swapchain, UINT64_MAX, NextImageAcquiredSemaphore, VK_NULL_HANDLE, &SwapchainImageIndex));

	//!< �f�X�N���v�^�Z�b�g���X�V������A�R�}���h�o�b�t�@���L�^�������Ȃ��ƃ_���H
	//UpdateDescriptorSet();

	/**
	@brief �e�X�̃Z�}�t�H�̓p�C�v���C���X�e�[�W�Ɋ֘A�t������
	�R�}���h�͎w��̃p�C�v���C���X�e�[�W�ɓ��B����܂Ŏ��s����A�����ŃZ�}�t�H���V�O�i�������܂ő҂�
	*/
	const VkPipelineStageFlags PipelineStageFlags = VK_PIPELINE_STAGE_TRANSFER_BIT;
	const std::vector<VkSubmitInfo> SubmitInfos = {
		{
			VK_STRUCTURE_TYPE_SUBMIT_INFO,
			nullptr,
			1, &NextImageAcquiredSemaphore, &PipelineStageFlags,	//!< ���C���[�W���擾�ł���(�v���[���g����)�܂ŃE�G�C�g
			1, &CommandBuffers[SwapchainImageIndex],
			1, &RenderFinishedSemaphore								//!< �`�抮����ʒm����
		},
	};
	VERIFY_SUCCEEDED(vkQueueSubmit(GraphicsQueue, static_cast<uint32_t>(SubmitInfos.size()), SubmitInfos.data(), Fence));

	Present();
}
void VK::Dispatch()
{
	WaitForFence(ComputeFence);

	const std::vector<VkSubmitInfo> SubmitInfos = {
		{
			VK_STRUCTURE_TYPE_SUBMIT_INFO,
			nullptr,
			0, nullptr, nullptr,
			1, &ComputeCommandBuffers[0],
			0, nullptr,
		},
	};
	VERIFY_SUCCEEDED(vkQueueSubmit(ComputeQueue, static_cast<uint32_t>(SubmitInfos.size()), SubmitInfos.data(), ComputeFence));
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
	VERIFY_SUCCEEDED(vkQueuePresentKHR(PresentQueue, &PresentInfo));

#ifdef DEBUG_STDOUT
	//std::cout << "\t" << "SwapchainImageIndex = " << SwapchainImageIndex << std::endl;
#endif
}
void VK::WaitForFence(VkFence& Fence, const uint64_t TimeOut)
{
	VkResult Result;
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
}

