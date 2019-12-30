//#include "stdafx.h"
//#include "framework.h"

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

#ifdef USE_DEBUG_REPORT
	//!< �C���X�^���X���x���֐�(Debug) Instance level functions(Debug)
#define VK_INSTANCE_PROC_ADDR(proc) PFN_vk ## proc ## EXT VK::vk ## proc = VK_NULL_HANDLE;
#include "VKDebugReport.h"
#undef VK_INSTANCE_PROC_ADDR
#endif

	//!< �f�o�C�X���x���֐�(Debug) Device level functions(Debug)
#ifdef USE_DEBUG_MARKER
#define VK_DEVICE_PROC_ADDR(proc) PFN_vk ## proc ## EXT VK::vk ## proc = VK_NULL_HANDLE;
#include "VKDebugMarker.h"
#undef VK_DEVICE_PROC_ADDR
#endif

#ifdef _WINDOWS
void VK::OnCreate(HWND hWnd, HINSTANCE hInstance, LPCWSTR Title)
{
#ifdef _DEBUG
	PerformanceCounter PC(__func__);
#endif

	Super::OnCreate(hWnd, hInstance, Title);

#ifdef VK_NO_PROTOYYPES
	LoadVulkanLibrary();
#endif

	//!< �C���X�^���X�A�f�o�C�X
	CreateInstance();
	CreateSurface(hWnd, hInstance);
	EnumeratePhysicalDevice(Instance);
	CreateDevice(GetCurrentPhysicalDevice(), Surface);
	
	//!< �f�o�C�X���������܂Ƃ߂Ċm��
	AllocateDeviceMemory();

	CreateFence(Device);
	CreateSemaphore(Device);

	CreateSwapchain(GetCurrentPhysicalDevice(), Surface, GetClientRectWidth(), GetClientRectHeight());
	GetSwapchainImage(Device, Swapchain);
	CreateSwapchainImageView();

	CreateCommandPool();
	AllocateCommandBuffer();
	AllocateSecondaryCommandBuffer();

#ifndef USE_RENDER_PASS_CLEAR
	InitializeSwapchainImage(CommandBuffers[0], &Colors::Red);
#endif

	CreateDepthStencil();
	InitializeDepthStencilImage(CommandBuffers[0]);
	CreateRenderTarget();

	CreateVertexBuffer();
	CreateIndexBuffer();
	CreateIndirectBuffer();

	CreateTexture();
	CreateSampler();

	CreateDescriptorSetLayout();
	//!< �p�C�v���C�����C�A�E�g (���[�g�V�O�l�`������)
	CreatePipelineLayout();
	//!< �����_�[�p�X
	CreateRenderPass();
	CreateShaderModule();
	//!< �p�C�v���C��
	CreatePipeline();
	//!< �t���[���o�b�t�@
	CreateFramebuffer();

	//!< ���j�t�H�[���o�b�t�@ (�R���X�^���g�o�b�t�@����)
	CreateUniformBuffer();

	//!< �f�X�N���v�^�A�b�v�f�[�g�e���v���[�g
	CreateDescriptorUpdateTemplate();
	//!< �f�X�N���v�^�v�[�� (�f�X�N���v�^�q�[�v����)
	CreateDescriptorPool();
	//!< �f�X�N���v�^�Z�b�g (�f�X�N���v�^�r���[����)
	AllocateDescriptorSet();
	UpdateDescriptorSet();

	SetTimer(hWnd, NULL, Elapse, nullptr);

	//!< �E�C���h�E�T�C�Y�ύX���ɍ�蒼������
	OnExitSizeMove(hWnd, hInstance);
}

/**
@note �w�ǂ̂��̂��󂵂č�蒼���Ȃ��ƃ_�� #VK_TODO
almost every thing must be recreated

LEARNING VULKAN : p367
need to destroy and recreate the framebuffer,
command pool, graphics pipeline, Render Pass, depth buffer image, image view, vertex
buffer, and so on.
*/
void VK::OnExitSizeMove(HWND hWnd, HINSTANCE hInstance)
{
#ifdef _DEBUG
	PerformanceCounter PC(__func__);
#endif

	Super::OnExitSizeMove(hWnd, hInstance);

	//!< �f�o�C�X���A�C�h���ɂȂ�܂ő҂�
	if (VK_NULL_HANDLE != Device) {
		VERIFY_SUCCEEDED(vkDeviceWaitIdle(Device));
	}

#if 0
	//!< �X���b�v�`�F�C���̍�蒼��
	CreateSwapchain(GetCurrentPhysicalDevice(), Surface, Rect);

	//!< �f�v�X�o�b�t�@�̔j���A�쐬

	//!< �t���[���o�b�t�@�̔j���A�쐬
	//DestroyFramebuffer();
	//CreateFramebuffer();
#endif

	CreateViewport(static_cast<float>(SurfaceExtent2D.width), static_cast<float>(SurfaceExtent2D.height));

	//DestroyFramebuffer();
	//CreateFramebuffer();

	for (auto i : SecondaryCommandBuffers) {
		VERIFY_SUCCEEDED(vkResetCommandBuffer(i, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT));
	}
	for (auto i : CommandBuffers) {
		VERIFY_SUCCEEDED(vkResetCommandBuffer(i, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT));
	}

	//!< �r���[�|�[�g�T�C�Y�����肵�Ă���
	LoadScene();
	for (auto i = 0; i < CommandBuffers.size(); ++i) {
		PopulateCommandBuffer(i);
	}
}

void VK::OnDestroy(HWND hWnd, HINSTANCE hInstance)
{
	Super::OnDestroy(hWnd, hInstance);

	if (VK_NULL_HANDLE != Device) {
		//!< �f�o�C�X�̃L���[�ɃT�u�~�b�g���ꂽ�S�R�}���h����������܂Ńu���b�L���O�A��ɏI�������Ɏg�� (Wait for all command submitted to queue, usually used on finalize)
		VERIFY_SUCCEEDED(vkDeviceWaitIdle(Device));
	}

	DestroyFramebuffer();

	for (auto i : RenderPasses) {
		vkDestroyRenderPass(Device, i, GetAllocationCallbacks());
	}

	for (auto i : Pipelines) {
		vkDestroyPipeline(Device, i, GetAllocationCallbacks());
	}

	for (auto i : ShaderModules) {
		vkDestroyShaderModule(Device, i, GetAllocationCallbacks());
	}
	ShaderModules.clear();

	for (auto i : PipelineLayouts) {
		vkDestroyPipelineLayout(Device, i, GetAllocationCallbacks());
	}
	PipelineLayouts.clear();

	for (auto i : DescriptorUpdateTemplates) {
		vkDestroyDescriptorUpdateTemplate(Device, i, GetAllocationCallbacks());
	}
	DescriptorUpdateTemplates.clear();

#if 0
	//!< VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT �̏ꍇ�̂݌ʂɊJ���ł��� (Only if VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT is used, can be release individually)
	if (!DescriptorSets.empty()) {
		vkFreeDescriptorSets(Device, DescriptorPool, static_cast<uint32_t>(DescriptorSets.size()), DescriptorSets.data());
	}
#else
	//!< ���̃v�[������m�ۂ��ꂽ�S�Ẵf�X�N���v�^�Z�b�g��������� (�����ł͎��̃X�e�b�v�Ńv�[�����̂�j�����Ă���̂ł��Ȃ��Ă��ǂ�)
	//!< (�f�X�N���v�^�Z�b�g���X�ɉ������̂��ʓ|�ȏꍇ�A�v�[�����͔̂j���������Ȃ��ꍇ)
	for (auto i : DescriptorPools) {
		vkResetDescriptorPool(Device, i, 0);
	}
#endif
	DescriptorSets.clear();

	for (auto i : DescriptorPools) {
		vkDestroyDescriptorPool(Device, i, GetAllocationCallbacks());
	}
	DescriptorPools.clear();
	
	for (auto i : DescriptorSetLayouts) {
		vkDestroyDescriptorSetLayout(Device, i, GetAllocationCallbacks());
	}
	DescriptorSetLayouts.clear();

	for (auto i : UniformBuffers) {
		vkDestroyBuffer(Device, i, GetAllocationCallbacks());
	}
	UniformBuffers.clear();

	for (auto i : IndirectBuffers) {
		vkDestroyBuffer(Device, i, GetAllocationCallbacks());
	}
	IndirectBuffers.clear();
	for (auto i : IndexBuffers) {
		vkDestroyBuffer(Device, i, GetAllocationCallbacks());
	}
	IndexBuffers.clear();
	for (auto i : VertexBuffers) {
		vkDestroyBuffer(Device, i, GetAllocationCallbacks());
	}
	VertexBuffers.clear();

	for (auto i : DeviceMemories) {
		vkFreeMemory(Device, i, GetAllocationCallbacks());
	}
	DeviceMemories.clear();
	DeviceMemoryOffsets.clear();

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

	if (VK_NULL_HANDLE != RenderTargetImageView) {
		vkDestroyImageView(Device, RenderTargetImageView, GetAllocationCallbacks());
		RenderTargetImageView = VK_NULL_HANDLE;
	}
	if (VK_NULL_HANDLE != RenderTargetDeviceMemory) {
		vkFreeMemory(Device, RenderTargetDeviceMemory, GetAllocationCallbacks());
		RenderTargetDeviceMemory = VK_NULL_HANDLE;
	}
	if (VK_NULL_HANDLE != RenderTargetImage) {
		vkDestroyImage(Device, RenderTargetImage, GetAllocationCallbacks());
		RenderTargetImage = VK_NULL_HANDLE;
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

	//!< �R�}���h�v�[���j�����ɃR�}���h�o�b�t�@�͈ÖٓI�ɉ�������̂Ŗ����Ă��ǂ�
	//if(!SecondaryCommandBuffers.empty()) {
	//	vkFreeCommandBuffers(Device, SecondaryCommandPools[0], static_cast<uint32_t>(SecondaryCommandBuffers.size()), SecondaryCommandBuffers.data());
	//	SecondaryCommandBuffers.clear();
	//}	
	for (auto i : SecondaryCommandPools) {
		vkDestroyCommandPool(Device, i, GetAllocationCallbacks());
	}
	SecondaryCommandPools.clear();

	//if(!CommandBuffers.empty()) {
	//	vkFreeCommandBuffers(Device, CommandPool[0], static_cast<uint32_t>(CommandBuffers.size()), CommandBuffers.data());
	//	CommandBuffers.clear();
	//}
	for (auto i : CommandPools) {
		vkDestroyCommandPool(Device, i, GetAllocationCallbacks());
	}
	CommandPools.clear();

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

#ifdef USE_DEBUG_REPORT
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

const char* VK::GetVkResultChar(const VkResult Result)
{
#define VK_RESULT_ENTRY(vr) case VK_##vr: return #vr;
	switch (Result)
	{
	default: DEBUG_BREAK(); return "Not found";
#include "VkResult.h"
	}
#undef VK_RESULT_ENTRY
}

const char* VK::GetFormatChar(const VkFormat Format)
{
#define VK_FORMAT_ENTRY(vf) case VK_FORMAT_##vf: return #vf;
	switch (Format)
	{
	default: DEBUG_BREAK(); return "Not found";
#include "VKFormat.h"
	}
#undef VK_FORMAT_ENTRY
}

const char* VK::GetColorSpaceChar(const VkColorSpaceKHR ColorSpace)
{
#define VK_COLOR_SPACE_ENTRY(vcs) case VK_COLOR_SPACE_##vcs: return #vcs;
	switch (ColorSpace)
	{
	default: DEBUG_BREAK(); return "Not found";
#include "VKColorSpace.h"
	}
#undef VK_COLOR_SPACE_ENTRY
}

const char* VK::GetImageViewTypeChar(const VkImageViewType ImageViewType)
{
#define VK_IMAGE_VIEW_TYPE_ENTRY(vivt) case VK_IMAGE_VIEW_TYPE_##vivt: return #vivt;
	switch (ImageViewType)
	{
	default: DEBUG_BREAK(); return "Not found";
#include "VKImageViewType.h"
	}
#undef VK_IMAGE_VIEW_TYPE_ENTRY
}

const char* VK::GetComponentSwizzleChar(const VkComponentSwizzle ComponentSwizzle)
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

//!< @brief �������v���p�e�B�t���O��(�T�|�[�g����Ă��邩�`�F�b�N����)�������^�C�v�֕ϊ�
//!< @param �����f�o�C�X�̃������v���p�e�B
//!< @param �o�b�t�@��C���[�W�̗v�����郁�����^�C�v
//!< @param ��]�̃������v���p�e�B�t���O
//!< @return �������^�C�v
uint32_t VK::GetMemoryTypeIndex(const VkPhysicalDeviceMemoryProperties& PDMP, const uint32_t TypeBits, const VkFlags Properties)
{
	for (uint32_t i = 0; i < PDMP.memoryTypeCount; ++i) {
		if (TypeBits & (1 << i)) {
			if ((PDMP.memoryTypes[i].propertyFlags & Properties) == Properties) {
				return i;
			}
		}
	}
	DEBUG_BREAK();
	return 0;
}

void VK::CreateBuffer(VkBuffer* Buffer, const VkBufferUsageFlags Usage, const size_t Size) const
{
	//!< �o�b�t�@�͍쐬���Ɏw�肵���g�p�@�ł����g�p�ł��Ȃ��A�����ł� VK_SHARING_MODE_EXCLUSIVE ���ߑł��ɂ��Ă��� #VK_TODO (Using VK_SHARING_MODE_EXCLUSIVE here)
	//!< VK_SHARING_MODE_EXCLUSIVE	: �����t�@�~���̃L���[�������A�N�Z�X�ł��Ȃ��A���̃t�@�~������A�N�Z�X�������ꍇ�̓I�[�i�[�V�b�v�̈ڏ����K�v
	//!< VK_SHARING_MODE_CONCURRENT	: �����t�@�~���̃L���[�������A�N�Z�X�\�A�I�[�i�[�V�b�v�̈ڏ����K�v�����A�p�t�H�[�}���X�͈���
	const VkBufferCreateInfo BCI = {
		VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
		nullptr,
		0,
		Size,
		Usage,
		VK_SHARING_MODE_EXCLUSIVE,
		0, nullptr
	};
	VERIFY_SUCCEEDED(vkCreateBuffer(Device, &BCI, GetAllocationCallbacks(), Buffer));
}

void VK::CreateImage(VkImage* Img, const VkImageCreateFlags /*CreateFlags*/, const VkImageType ImageType, const VkFormat Format, const VkExtent3D& Extent3D, const uint32_t MipLevels, const uint32_t ArrayLayers, const VkSampleCountFlagBits SampleCount, const VkImageUsageFlags Usage) const
{
	//!< Usage �� VK_IMAGE_USAGE_SAMPLED_BIT ���w�肳�ꂢ��ꍇ�A�t�H�[�}�b�g��t�B���^���g�p�\���`�F�b�N #VK_TODO �����ł̓��j�A�t�B���^���ߑł�
	ValidateFormatProperties_SampledImage(GetCurrentPhysicalDevice(), Format, Usage, VK_FILTER_LINEAR, VK_FILTER_LINEAR, VK_SAMPLER_MIPMAP_MODE_LINEAR);

	const VkImageCreateInfo ICI = {
		VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
		nullptr,
		0,
		ImageType,
		Format,
		Extent3D,
		MipLevels,
		ArrayLayers,
		SampleCount,
		VK_IMAGE_TILING_OPTIMAL, //!< ���j�A�̓p�t�H�[�}���X�������̂ŁA�����ł� OPTIMAL �Ɍ��ߑł����Ă���
		Usage,
		VK_SHARING_MODE_EXCLUSIVE,
		0, nullptr,
		VK_IMAGE_LAYOUT_UNDEFINED //!< �쐬���Ɏw��ł���̂� UNDEFINED, PREINITIALIZED �̂݁A���ۂɎg�p����O�Ƀ��C�A�E�g��ύX����K�v������
	};
	ValidateImageCreateInfo(ICI);
	VERIFY_SUCCEEDED(vkCreateImage(Device, &ICI, GetAllocationCallbacks(), Img));
}

void VK::CopyToHostVisibleDeviceMemory(const VkDeviceMemory DM, const size_t Size, const void* Source, const VkDeviceSize Offset, const std::array<VkDeviceSize, 2>* Range)
{
	if (Size && nullptr != Source) {
		void *Data;
		VERIFY_SUCCEEDED(vkMapMemory(Device, DM, Offset, Size, static_cast<VkMemoryMapFlags>(0), &Data)); {
			memcpy(Data, Source, Size);

			//!< �������R���e���c���ύX���ꂽ���Ƃ��h���C�o�֒m�点��(vkMapMemory()������Ԃł�邱��)
			//!< �f�o�X�������m�ێ��� VK_MEMORY_PROPERTY_HOST_COHERENT_BIT ���w�肵���ꍇ�͕K�v�Ȃ� CreateDeviceMemory(..., VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
			if (nullptr != Range) {
				//!< �X�V���郌���W(�ꕔ)�������I�Ɏw�肳�ꂽ�ꍇ�A�w�背���W�݂̂��X�V����
				const std::array<VkMappedMemoryRange, 1> MMRs = {
						{
							VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE,
							nullptr,
							DM,
							Offset + (*Range)[0],
							(*Range)[1]
						}
				};
				VERIFY_SUCCEEDED(vkFlushMappedMemoryRanges(Device, static_cast<uint32_t>(MMRs.size()), MMRs.data()));
				//VERIFY_SUCCEEDED(vkInvalidateMappedMemoryRanges(Device, static_cast<uint32_t>(MMRs.size()), MMRs.data()));
			}
			else {
				//!< �����W�w�肪�����ꍇ�͑S�̂��X�V����
				const std::array<VkMappedMemoryRange, 1> MMRs = {
						{
							VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE,
							nullptr,
							DM,
							Offset,
							VK_WHOLE_SIZE
						}
				};
				VERIFY_SUCCEEDED(vkFlushMappedMemoryRanges(Device, static_cast<uint32_t>(MMRs.size()), MMRs.data()));
				//VERIFY_SUCCEEDED(vkInvalidateMappedMemoryRanges(Device, static_cast<uint32_t>(MMRs.size()), MMRs.data()));
			}
		} vkUnmapMemory(Device, DM);
	}
}
//!< @param �R�}���h�o�b�t�@
//!< @param �R�s�[���o�b�t�@
//!< @param �R�s�[��o�b�t�@
//!< @param (�R�s�[���)�o�b�t�@�̃A�N�Z�X�t���O ex) VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT, VK_ACCESS_INDEX_READ_BIT, VK_ACCESS_INDIRECT_READ_BIT,...��
//!< @param (�R�s�[���)�o�b�t�@���g����p�C�v���C���X�e�[�W ex) VK_PIPELINE_STAGE_VERTEX_INPUT_BIT, VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT,...��
void VK::CmdCopyBufferToBuffer(const VkCommandBuffer CB, const VkBuffer Src, const VkBuffer Dst, const VkAccessFlags AF, const VkPipelineStageFlagBits PSF, const size_t Size)
{
	const VkCommandBufferBeginInfo CBBI = {
		VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		nullptr,
		VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
		nullptr
	};
	VERIFY_SUCCEEDED(vkBeginCommandBuffer(CB, &CBBI)); {
		const std::vector<VkBufferCopy> BCs = {
			{ 0, 0, Size },
		};
		const std::vector<VkBufferMemoryBarrier> BMBs_Pre = {
			{
				VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
				nullptr,
				//!< �o�b�t�@���ǂ������邩�̃t���O�u����܂Łv�Ɓu���ꂩ��v�A0���珑������(VK_ACCESS_MEMORY_WRITE_BIT)��
				0, VK_ACCESS_MEMORY_WRITE_BIT,
				//!< (VK_SHARING_MODE_EXCLUSIVE�ō쐬���ꂽ�o�b�t�@��)�Q�Ƃ��Ă���L���[�t�@�~����ύX�u����܂Łv�Ɓu���ꂩ��v�A�����ł͕ύX���Ȃ��̂�VK_QUEUE_FAMILY_IGNORED
				VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED,
				Dst,
				0,
				VK_WHOLE_SIZE
			},
		};
		vkCmdPipelineBarrier(CB,
			//!< �o�b�t�@���g����p�C�v���C���X�e�[�W�u����܂Łv�Ɓu���ꂩ��v�AVK_PIPELINE_STAGE_TOP_OF_PIPE_BIT����]��(VK_PIPELINE_STAGE_TRANSFER_BIT)��
			VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
			0,
			0, nullptr,
			static_cast<uint32_t>(BMBs_Pre.size()), BMBs_Pre.data(),
			0, nullptr);
		{
			vkCmdCopyBuffer(CB, Src, Dst, static_cast<uint32_t>(BCs.size()), BCs.data());
		}
		const std::vector<VkBufferMemoryBarrier> BMBs_Post = {
			{
				VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
				nullptr,
				//!< �o�b�t�@���ǂ������邩�̃t���O�u����܂Łv�Ɓu���ꂩ��v�A�Ⴆ��(VK_ACCESS_MEMORY_WRITE_BIT)���璸�_(VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT)�֓�
				VK_ACCESS_MEMORY_WRITE_BIT, AF,
				VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED,
				Dst,
				0,
				VK_WHOLE_SIZE
			},
		};
		assert(BMBs_Pre[0].dstAccessMask == BMBs_Post[0].srcAccessMask);
		vkCmdPipelineBarrier(CB, 
			//!< �o�b�t�@���g����p�C�v���C���X�e�[�W�u����܂Łv�Ɓu���ꂩ��v�A�Ⴆ�Γ]����(VK_PIPELINE_STAGE_TRANSFER_BIT)���璸�_�o�b�t�@(VK_PIPELINE_STAGE_VERTEX_INPUT_BIT)�֓�
			VK_PIPELINE_STAGE_TRANSFER_BIT, PSF,
			0, 
			0, nullptr,
			static_cast<uint32_t>(BMBs_Post.size()), BMBs_Post.data(), 
			0, nullptr);
	} VERIFY_SUCCEEDED(vkEndCommandBuffer(CB));
}

void VK::EnumerateMemoryRequirements(const VkMemoryRequirements& MR)
{
	const auto& PDMP = GetCurrentPhysicalDeviceMemoryProperties();

	Logf("\t\tsize=%d\n", MR.size);
	Logf("\t\talignment=%d\n", MR.alignment);
	for (uint32_t i = 0; i < PDMP.memoryTypeCount; ++i) {
		if (MR.memoryTypeBits & (1 << i)) {
			Logf("\t\tmemoryTypeBits[%d] = ", i);
#define MEMORY_PROPERTY_ENTRY(entry) if(VK_MEMORY_PROPERTY_##entry##_BIT & PDMP.memoryTypes[i].propertyFlags) { Log(#entry " | "); }
			MEMORY_PROPERTY_ENTRY(DEVICE_LOCAL);
			MEMORY_PROPERTY_ENTRY(HOST_VISIBLE);
			MEMORY_PROPERTY_ENTRY(HOST_COHERENT);
			MEMORY_PROPERTY_ENTRY(HOST_CACHED);
			MEMORY_PROPERTY_ENTRY(LAZILY_ALLOCATED);
			MEMORY_PROPERTY_ENTRY(PROTECTED);
#undef MEMORY_PROPERTY_ENTRY
			Log("\n");

			Logf("\t\t\tHeapIndex = %d, HeapSize = %llu\n", PDMP.memoryTypes[i].heapIndex, PDMP.memoryHeaps[PDMP.memoryTypes[i].heapIndex].size);
		}
	}
}

void VK::AllocateBufferMemory(VkDeviceMemory* DM, const VkBuffer Buffer, const VkMemoryPropertyFlags MPF)
{
	VkMemoryRequirements MR;
	vkGetBufferMemoryRequirements(Device, Buffer, &MR);
	EnumerateMemoryRequirements(MR);

	const auto PDMP = GetCurrentPhysicalDeviceMemoryProperties();
	const auto TypeIndex = GetMemoryTypeIndex(PDMP, MR.memoryTypeBits, MPF);

	Logf("\t\tAllocateBufferMemory = %llu / %llu (HeapIndex = %d)\n", MR.size, PDMP.memoryHeaps[PDMP.memoryTypes[TypeIndex].heapIndex].size, PDMP.memoryTypes[TypeIndex].heapIndex);

	const VkMemoryAllocateInfo MAI = {
		VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
		nullptr,
		MR.size,
		TypeIndex
	};
	VERIFY_SUCCEEDED(vkAllocateMemory(Device, &MAI, GetAllocationCallbacks(), DM));
}
void VK::SuballocateBufferMemory(uint32_t& HeapIndex, VkDeviceSize& Offset, const VkBuffer Buffer, const VkMemoryPropertyFlags MPF)
{
	VkMemoryRequirements MR;
	vkGetBufferMemoryRequirements(Device, Buffer, &MR);

	const auto PDMP = GetCurrentPhysicalDeviceMemoryProperties();
	const auto TypeIndex = GetMemoryTypeIndex(PDMP, MR.memoryTypeBits, MPF);
	HeapIndex = PDMP.memoryTypes[TypeIndex].heapIndex;

	Logf("\t\tSuballocateBufferMemory = %llu / %llu (HeapIndex = %d)\n", MR.size, PDMP.memoryHeaps[HeapIndex].size, HeapIndex);

	Offset = DeviceMemoryOffsets[HeapIndex];
	VERIFY_SUCCEEDED(vkBindBufferMemory(Device, Buffer, DeviceMemories[HeapIndex], Offset));

	DeviceMemoryOffsets[HeapIndex] += MR.size;
}

void VK::AllocateImageMemory(VkDeviceMemory* DM, const VkImage Img, const VkMemoryPropertyFlags MPF)
{
	VkMemoryRequirements MR;
	vkGetImageMemoryRequirements(Device, Img, &MR);
	EnumerateMemoryRequirements(MR);

	const auto PDMP = GetCurrentPhysicalDeviceMemoryProperties();
	const auto TypeIndex = GetMemoryTypeIndex(PDMP, MR.memoryTypeBits, MPF);

	Logf("\t\tAllocateImageMemory = %llu / %llu (HeapIndex = %d)\n", MR.size, PDMP.memoryHeaps[PDMP.memoryTypes[TypeIndex].heapIndex].size, PDMP.memoryTypes[TypeIndex].heapIndex);

	const VkMemoryAllocateInfo MAI = {
		VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
		nullptr,
		MR.size,
		TypeIndex
	};
	VERIFY_SUCCEEDED(vkAllocateMemory(Device, &MAI, GetAllocationCallbacks(), DM));
}
void VK::SuballocateImageMemory(uint32_t& HeapIndex, VkDeviceSize& Offset, const VkImage Img, const VkMemoryPropertyFlags MPF)
{
	VkMemoryRequirements MR;
	vkGetImageMemoryRequirements(Device, Img, &MR);

	const auto PDMP = GetCurrentPhysicalDeviceMemoryProperties();
	const auto TypeIndex = GetMemoryTypeIndex(PDMP, MR.memoryTypeBits, MPF);
	HeapIndex = PDMP.memoryTypes[TypeIndex].heapIndex;

	Logf("\t\tSuballocateImageMemory = %llu / %llu (HeapIndex = %d)\n", MR.size, PDMP.memoryHeaps[HeapIndex].size, HeapIndex);

	Offset = DeviceMemoryOffsets[HeapIndex];
	VERIFY_SUCCEEDED(vkBindImageMemory(Device, Img, DeviceMemories[HeapIndex], Offset));

	DeviceMemoryOffsets[HeapIndex] += MR.size;
}

//void VK::CreateBufferView(VkBufferView* BufferView, const VkBuffer Buffer, const VkFormat Format, const VkDeviceSize Offset, const VkDeviceSize Range)
//{
//	const VkBufferViewCreateInfo BufferViewCreateInfo = {
//		VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO,
//		nullptr,
//		0,
//		Buffer,
//		Format,
//		Offset,
//		Range
//	};
//	VERIFY_SUCCEEDED(vkCreateBufferView(Device, &BufferViewCreateInfo, GetAllocationCallbacks(), BufferView));
//}
//!< Cubemap�����ꍇ�A�܂����C�������ꂽ�C���[�W���쐬���A(�C���[�W�r���[��p����)���C�����t�F�C�X�Ƃ��Ĉ����悤�n�[�h�E�G�A�ɓ`����
void VK::CreateImageView(VkImageView* IV, const VkImage Img, const VkImageViewType ImageViewType, const VkFormat Format, const VkComponentMapping& ComponentMapping, const VkImageSubresourceRange& ImageSubresourceRange)
{
	const VkImageViewCreateInfo ImageViewCreateInfo = {
		VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
		nullptr,
		0,
		Img,
		ImageViewType,
		Format,
		ComponentMapping,
		ImageSubresourceRange
	};
	VERIFY_SUCCEEDED(vkCreateImageView(Device, &ImageViewCreateInfo, GetAllocationCallbacks(), IV));

	Logf("\t\tImageViewType = %s\n", GetImageViewTypeChar(ImageViewType));
	Logf("\t\tFormat = %s\n", GetFormatChar(Format));
	Logf("\t\tComponentMapping = (%s)\n", GetComponentMappingString(ComponentMapping).c_str());

	LOG_OK();
}

void VK::ValidateFormatProperties(VkPhysicalDevice PD, const VkFormat Format, const VkImageUsageFlags Usage) const
{
	VkFormatProperties FP;
	vkGetPhysicalDeviceFormatProperties(PD, Format, &FP);

	if (Usage & VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT) {
#if 0
		//!< �J���[�̏ꍇ (In case color)
		if (!(FP.optimalTilingFeatures & VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT)) {
			Error("VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT not supported\n");
			DEBUG_BREAK();
		}
		//!< �f�v�X�X�e���V���̏ꍇ (In case depth stencil)
		if (!(FP.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)) {
			Error("VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT not supported\n");
			DEBUG_BREAK();
		}
#endif
	}

	if (Usage & VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT) {
		if (!(FP.bufferFeatures &  VK_FORMAT_FEATURE_UNIFORM_TEXEL_BUFFER_BIT)) {
			Error("VK_FORMAT_FEATURE_UNIFORM_TEXEL_BUFFER_BIT not supported\n");
			DEBUG_BREAK();
		}
	}

	if (Usage & VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT) {
		if (!(FP.bufferFeatures &  VK_FORMAT_FEATURE_STORAGE_TEXEL_BUFFER_BIT)) {
			Error("VK_FORMAT_FEATURE_STORAGE_TEXEL_BUFFER_BIT not supported\n");
			DEBUG_BREAK();
		}
		//!< #VK_TODO �A�g�~�b�N�g�p���̂݃`�F�b�N����
		const auto bUseAtomic = false;
		if (bUseAtomic) {
			if (!(FP.linearTilingFeatures & VK_FORMAT_FEATURE_STORAGE_TEXEL_BUFFER_ATOMIC_BIT)) {
				Error("VK_FORMAT_FEATURE_STORAGE_TEXEL_BUFFER_ATOMIC_BIT not supported\n");
				DEBUG_BREAK();
			}
		}
	}
}
void VK::ValidateFormatProperties_SampledImage(VkPhysicalDevice PD, const VkFormat Format, const VkImageUsageFlags Usage, const VkFilter Mag, const VkFilter Min, const VkSamplerMipmapMode Mip) const
{
	VkFormatProperties FP;
	vkGetPhysicalDeviceFormatProperties(PD, Format, &FP);

	if (Usage & VK_IMAGE_USAGE_SAMPLED_BIT) {
		if (!(FP.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT)) {
			Error("VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT not supported\n");
			DEBUG_BREAK();
		}
		if (VK_FILTER_LINEAR == Mag || VK_FILTER_LINEAR == Min || VK_SAMPLER_MIPMAP_MODE_LINEAR == Mip) {
			if (!(FP.linearTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT)) {
				Error("VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT not supported\n");
				DEBUG_BREAK();
			}
		}
	}
}

void VK::ValidateFormatProperties_StorageImage(VkPhysicalDevice PD, const VkFormat Format, const VkImageUsageFlags Usage, const bool UseAtomic) const
{
	VkFormatProperties FP;
	vkGetPhysicalDeviceFormatProperties(PD, Format, &FP);

	if (Usage & VK_IMAGE_USAGE_STORAGE_BIT) {
		if (!(FP.optimalTilingFeatures & VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT)) {
			Error("VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT not supported\n");
			DEBUG_BREAK();
		}
		if (UseAtomic) {
			//!< �A�g�~�b�N���삪�ۏ؂���Ă���̂� VK_FORMAT_R32_UINT, VK_FORMAT_R32_SINT �̂݁A����ȊO�ł�肽���ꍇ�̓T�|�[�g����Ă��邩���ׂ�K�v������
			if (!(FP.linearTilingFeatures & VK_FORMAT_FEATURE_STORAGE_IMAGE_ATOMIC_BIT)) {
				Error("VK_FORMAT_FEATURE_STORAGE_IMAGE_ATOMIC_BIT not supported\n");
				DEBUG_BREAK();
			}
		}
	}
}

void VK::EnumerateInstanceLayerProperties()
{
	Logf("Instance Layer Properties\n");

	uint32_t Count = 0;
	VERIFY_SUCCEEDED(vkEnumerateInstanceLayerProperties(&Count, nullptr));
	if (Count) {
		std::vector<VkLayerProperties> LayerProp(Count);
		VERIFY_SUCCEEDED(vkEnumerateInstanceLayerProperties(&Count, LayerProp.data()));
		for (const auto& i : LayerProp) {
			Logf("\t\"%s\" (%s)\n", i.layerName, i.description);
			EnumerateInstanceExtensionProperties(i.layerName);
		}
	}
}
void VK::EnumerateInstanceExtensionProperties(const char* LayerName)
{
	uint32_t Count = 0;
	VERIFY_SUCCEEDED(vkEnumerateInstanceExtensionProperties(LayerName, &Count, nullptr));
	if (Count) {
		std::vector<VkExtensionProperties> ExtensionProp(Count);
		VERIFY_SUCCEEDED(vkEnumerateInstanceExtensionProperties(LayerName, &Count, ExtensionProp.data()));
		for (const auto& i : ExtensionProp) {
			Logf("\t\t\"%s\"\n", i.extensionName);
		}
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
	
	//!< #VK_TODO �L���ɂ��Ă�����̂��AInstanceLayerProperties �Ɋ܂܂�Ă��邩�`�F�b�N����

	const VkInstanceCreateInfo InstanceCreateInfo = {
		VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
		nullptr,
		0,
		&ApplicationInfo,
		static_cast<uint32_t>(InstanceLayers.size()), InstanceLayers.data(),
		static_cast<uint32_t>(InstanceExtensions.size()), InstanceExtensions.data()
	};
	VERIFY_SUCCEEDED(vkCreateInstance(&InstanceCreateInfo, GetAllocationCallbacks(), &Instance));

	//!< �C���X�^���X���x���̊֐������[�h���� Load instance level functions
#ifdef VK_NO_PROTOYYPES
#define VK_INSTANCE_PROC_ADDR(proc) vk ## proc = reinterpret_cast<PFN_vk ## proc>(vkGetInstanceProcAddr(Instance, "vk" #proc)); assert(nullptr != vk ## proc && #proc);
#include "VKInstanceProcAddr.h"
#undef VK_INSTANCE_PROC_ADDR
#endif //!< VK_NO_PROTOYYPES

#ifdef USE_DEBUG_REPORT
	CreateDebugReportCallback();
#endif

	LOG_OK();
}

#ifdef USE_DEBUG_REPORT
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
			[](VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT /*objectType*/, uint64_t /*object*/, size_t /*location*/, int32_t /*messageCode*/, const char* /*pLayerPrefix*/, const char* pMessage, void* /*pUserData*/) -> VkBool32 {
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
#endif

#ifdef USE_DEBUG_MARKER
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
#else
void VK::MarkerInsert(VkCommandBuffer /*CB*/, const glm::vec4& /*Color*/, const char* /*Name*/) {}
#endif

#ifdef USE_DEBUG_MARKER
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
#else
void VK::MarkerBegin(VkCommandBuffer /*CB*/, const glm::vec4& /*Color*/, const char* /*Name*/) {}
#endif

#ifdef USE_DEBUG_MARKER
void VK::MarkerEnd(VkCommandBuffer CB)
{
	if (VK_NULL_HANDLE != vkCmdDebugMarkerEnd) {
		vkCmdDebugMarkerEnd(CB);
	}
}
#else
void VK::MarkerEnd(VkCommandBuffer /*CB*/) {}
#endif

#ifdef USE_DEBUG_MARKER
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
#else
void VK::MarkerSetName(VkDevice /*Device*/, const VkDebugReportObjectTypeEXT /*Type*/, const uint64_t /*Object*/, const char* /*Name*/) {}
#endif

#ifdef USE_DEBUG_MARKER
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
#else
void VK::MarkerSetTag(VkDevice /*Device*/, const VkDebugReportObjectTypeEXT /*Type*/, const uint64_t /*Object*/, const uint64_t /*TagName*/, const size_t /*TagSize*/, const void* /*TagData*/){}
#endif


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

		Logf(", PropertyFlags(%d) = ", PDMP.memoryTypes[i].propertyFlags);
		if (PDMP.memoryTypes[i].propertyFlags) {
#define MEMORY_PROPERTY_ENTRY(entry) if(VK_MEMORY_PROPERTY_##entry##_BIT & PDMP.memoryTypes[i].propertyFlags) { Log(#entry " | "); }
			MEMORY_PROPERTY_ENTRY(DEVICE_LOCAL);
			MEMORY_PROPERTY_ENTRY(HOST_VISIBLE);
			MEMORY_PROPERTY_ENTRY(HOST_COHERENT);
			MEMORY_PROPERTY_ENTRY(HOST_CACHED);
			MEMORY_PROPERTY_ENTRY(LAZILY_ALLOCATED);
			MEMORY_PROPERTY_ENTRY(PROTECTED);
#undef MEMORY_PROPERTY_ENTRY
		}
		Log("\n");
	}

	Log("\t\t\tMemoryHeap\n");
	for (uint32_t i = 0; i < PDMP.memoryHeapCount; ++i) {
		Log("\t\t\t\t");
		Logf("[%d] Size = %llu", i, PDMP.memoryHeaps[i].size);

		if (PDMP.memoryHeaps[i].flags) {
			Log(", Flags = ");
			if (VK_MEMORY_HEAP_DEVICE_LOCAL_BIT & PDMP.memoryHeaps[i].flags) { Log("DEVICE_LOCAL | "); }
			if (VK_MEMORY_HEAP_MULTI_INSTANCE_BIT_KHR & PDMP.memoryHeaps[i].flags) { Log("MULTI_INSTANCE | "); }
		}
		Log("\n");
	}
}
void VK::EnumeratePhysicalDevice(VkInstance Inst)
{
	//!< �����f�o�C�X(GPU)�̗�
	uint32_t Count = 0;
	VERIFY_SUCCEEDED(vkEnumeratePhysicalDevices(Inst, &Count, nullptr));
	assert(Count && "Physical device not found");
	PhysicalDevices.resize(Count);
	VERIFY_SUCCEEDED(vkEnumeratePhysicalDevices(Inst, &Count, PhysicalDevices.data()));

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
	Logf("Device Layer Properties\n");

	uint32_t Count = 0;
	VERIFY_SUCCEEDED(vkEnumerateDeviceLayerProperties(PD, &Count, nullptr));
	if (Count) {
		std::vector<VkLayerProperties> LayerProp(Count);
		VERIFY_SUCCEEDED(vkEnumerateDeviceLayerProperties(PD, &Count, LayerProp.data()));
		for (const auto& i : LayerProp) {
			Logf("\t\"%s\" (%s)\n", i.layerName, i.description);
			EnumeratePhysicalDeviceExtensionProperties(PD, i.layerName);
		}
	}
}
void VK::EnumeratePhysicalDeviceExtensionProperties(VkPhysicalDevice PD, const char* LayerName)
{
	uint32_t Count = 0;
	VERIFY_SUCCEEDED(vkEnumerateDeviceExtensionProperties(PD, LayerName, &Count, nullptr));
	if (Count) {
		std::vector<VkExtensionProperties> ExtensionProp(Count);
		VERIFY_SUCCEEDED(vkEnumerateDeviceExtensionProperties(PD, LayerName, &Count, ExtensionProp.data()));
		for (const auto& i : ExtensionProp) {
			Logf("\t\t\"%s\"\n", i.extensionName);
		}
	}
}

void VK::EnumerateQueueFamilyProperties(VkPhysicalDevice PD, VkSurfaceKHR Sfc, std::vector<VkQueueFamilyProperties>& QFPs)
{
	//!< �����\�͂����L���[�̓t�@�~���ɃO���[�v�������
	uint32_t Count = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(PD, &Count, nullptr);
	assert(Count && "QueueFamilyProperty not found");
	QFPs.resize(Count);
	vkGetPhysicalDeviceQueueFamilyProperties(PD, &Count, QFPs.data());

	//!< Geforce970M���ƈȉ��̂悤�ȏ�Ԃ����� (In case of Geforce970M)
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
		VERIFY_SUCCEEDED(vkGetPhysicalDeviceSurfaceSupportKHR(PD, i, Sfc, &Supported));
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
		VERIFY_SUCCEEDED(vkGetPhysicalDeviceSurfaceSupportKHR(PD, i, Sfc, &Supported));
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

	Log("\n");
	Logf("\t\tGraphicsQueueFamilyBits = %s\n", GraphicsQueueFamilyBits.to_string().c_str());
	Logf("\t\tPresentQueueFamilyBits = %s\n", PresentQueueFamilyBits.to_string().c_str());
	Logf("\t\tComputeQueueFamilyBits = %s\n", ComputeQueueFamilyBits.to_string().c_str());
	Logf("\t\tTansferQueueFamilyBits = %s\n", TransferQueueFamilyBits.to_string().c_str());
	Logf("\t\tSparceBindingQueueFamilyBits = %s\n", SparceBindingQueueFamilyBits.to_string().c_str());
}
void VK::OverridePhysicalDeviceFeatures(VkPhysicalDeviceFeatures& PDF) const
{
	//!< VkPhysicalDeviceFeatures �ɂ͉\�ȃt�B�[�`���[���S�� true �ɂȂ������̂��n����Ă���
	//!< �s�v�ȍ��ڂ� false �ɃI�[�o�[���C�h����ƃp�t�H�[�}���X�̉��P�����҂ł��邩������Ȃ�
	//!< false �œn����Ă��鍀�ڂ͎g���Ȃ��t�B�[�`���[�Ȃ̂� true �ɕς��Ă��g���Ȃ�

	Log("\tPhysicalDeviceFeatures (Override)\n");
#define VK_DEVICEFEATURE_ENTRY(entry) if(PDF.entry) { Logf("\t\t%s\n", #entry); }
#include "VKDeviceFeature.h"
#undef VK_DEVICEFEATURE_ENTRY
}

void VK::CreateQueueFamilyPriorities(VkPhysicalDevice PD, VkSurfaceKHR Sfc, const std::vector<VkQueueFamilyProperties>& QFPs, std::vector<std::vector<float>>& Priorites)
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
		VERIFY_SUCCEEDED(vkGetPhysicalDeviceSurfaceSupportKHR(PD, i, Sfc, &Supported));
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
void VK::CreateDevice(VkPhysicalDevice PD, VkSurfaceKHR Sfc)
{
	std::vector<VkQueueFamilyProperties> QFPs;
	EnumerateQueueFamilyProperties(PD, Sfc, QFPs);

	std::vector<std::vector<float>> Priorites(8);
	CreateQueueFamilyPriorities(PD, Sfc, QFPs, Priorites);

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

	//!< vkGetPhysicalDeviceFeatures() �ŉ\�ȃt�B�[�`���[���S�ėL���ɂȂ��� VkPhysicalDeviceFeatures ���Ԃ�
	//!< ���̂܂܂ł͉\�Ȃ����L���ɂȂ��Ă��܂��̂Ńp�t�H�[�}���X�I�ɂ͗ǂ��Ȃ�(�K�v�ȍ��ڂ��� true �ɂ��A����ȊO�� false �ɂ���̂��{���͗ǂ�)
	//!< �f�o�C�X�t�B�[�`���[���u�L���ɂ��Ȃ��Ɓv�Ǝg�p�ł��Ȃ��@�\�����X����ʓ|�Ȃ̂ŁA�����ł͕Ԃ����l�����̂܂܎g���Ă��܂��Ă��� (�p�t�H�[�}���X�I�ɂ͗ǂ��Ȃ�)
	VkPhysicalDeviceFeatures PDF;
	vkGetPhysicalDeviceFeatures(PD, &PDF);
	//!< �K�v�ɉ����� OverridePhysicalDeviceFeatures() ���I�[�o�[���C�h���A�s�v�ȍ��ڂ𖳌��ɂ���(�p�t�H�[�}���X���l����ꍇ)
	OverridePhysicalDeviceFeatures(PDF);
	const VkDeviceCreateInfo DeviceCreateInfo = {
		VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
		nullptr,
		0,
		static_cast<uint32_t>(QueueCreateInfos.size()), QueueCreateInfos.data(),
		0, nullptr, //!< �f�o�C�X�Ń��C���[�̗L�����͔񐄏� (Device layer is deprecated)
		static_cast<uint32_t>(DeviceExtensions.size()), DeviceExtensions.data(),
		&PDF
	};
	VERIFY_SUCCEEDED(vkCreateDevice(PD, &DeviceCreateInfo, GetAllocationCallbacks(), &Device));

	//!< �f�o�C�X���x���̊֐������[�h���� Load device level functions
#ifdef VK_NO_PROTOYYPES
#define VK_DEVICE_PROC_ADDR(proc) vk ## proc = reinterpret_cast<PFN_vk ## proc>(vkGetDeviceProcAddr(Device, "vk" #proc)); assert(nullptr != vk ## proc && #proc && #proc);
#include "VKDeviceProcAddr.h"
#undef VK_DEVICE_PROC_ADDR
#endif //!< VK_NO_PROTOYYPES

	//!< �L���[�̎擾 (�O���t�B�b�N�A�v���[���g�L���[�͓����C���f�b�N�X�̏ꍇ�����邪�ʂ̕ϐ��Ɏ擾���Ă���) (Graphics and presentation index may be same, but save to individual variables)
	vkGetDeviceQueue(Device, GraphicsQueueFamilyIndex, GraphicsQueueIndex, &GraphicsQueue);
	vkGetDeviceQueue(Device, PresentQueueFamilyIndex, PresentQueueIndex, &PresentQueue);
	//vkGetDeviceQueue(Device, ComputeQueueFamilyIndex, ComputeQueueIndex, &ComputeQueue);
	//vkGetDeviceQueue(Device, TransferQueueFamilyIndex, TransferQueueIndex, &TransferQueue);
	//vkGetDeviceQueue(Device, SparceBindingQueueFamilyIndex, SparceBindingQueueIndex, &SparceBindingQueue);

#ifdef USE_DEBUG_MARKER
	//if (HasDebugMarkerExt) {
#define VK_DEVICE_PROC_ADDR(proc) vk ## proc = reinterpret_cast<PFN_vk ## proc ## EXT>(vkGetDeviceProcAddr(Device, "vk" #proc "EXT")); assert(nullptr != vk ## proc && #proc);
#include "VKDebugMarker.h"
#undef VK_DEVICE_PROC_ADDR
	//}
#endif

	LOG_OK();
}

//!< �����̗v������A���C����ۏ؂��Ċm�ۂ���(�C���[�W��128byte�A�o�b�t�@��64byte�̃A���C�����K�v�Ȋ��Ȃ�A128byte�A���C���Ŋm�ۂ����)
//!< �����������I�u�W�F�N�g����A(�A���C����)�قȂ�^�C�v�Ċm�ۂ��\
void VK::AllocateDeviceMemory()
{
	VkPhysicalDeviceMemoryProperties PDMP;
	vkGetPhysicalDeviceMemoryProperties(GetCurrentPhysicalDevice(), &PDMP);
	if (PDMP.memoryHeapCount) {
		DeviceMemories.assign(PDMP.memoryHeapCount, VK_NULL_HANDLE);
		DeviceMemoryOffsets.assign(PDMP.memoryHeapCount, 0);

		for (uint32_t i = 0; i < PDMP.memoryTypeCount; ++i) {
			//!< ��芸�����v���p�e�B�t���O��0�̂��̂͑ΏۂƂ��Ȃ�(���ˑ������肻������ #VK_TODO) (Targeting propertyFlags!=0 here)
			if (PDMP.memoryTypes[i].propertyFlags) {
				const auto HeapIndex = PDMP.memoryTypes[i].heapIndex;
				//!< �m�ۂ���f�o�C�X�������T�C�Y (Device memory size to use) #VK_TODO
				//!< �ő�l�Ŏ���VK_ERROR_OUT_OF_DEVICE_MEMORY�ɂȂ�A���܂�ɑ傫�����Əd���Ȃ�
				const auto HeapSize = PDMP.memoryHeaps[HeapIndex].size / 2;
				assert(HeapSize < PDMP.memoryHeaps[HeapIndex].size && "");
				if (VK_NULL_HANDLE == DeviceMemories[HeapIndex]) {
					const VkMemoryAllocateInfo MAI = {
						VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
						nullptr,
						HeapSize,
						i
					};
					Logf("\t\tAllocateDeviceMemory = %llu / %llu (HeapIndex = %d)\n", HeapSize, PDMP.memoryHeaps[HeapIndex].size, PDMP.memoryTypes[i].heapIndex);
					VERIFY_SUCCEEDED(vkAllocateMemory(Device, &MAI, GetAllocationCallbacks(), &DeviceMemories[HeapIndex]));
				}
			}
		}
	}
}

//!< �z�X�g�ƃf�o�C�X�̓��� (Synchronization between host and device)
//!< �T�u�~�b�g(vkQueueSubmit) �Ɏg�p���ADraw()��Dispatch()�̓��ŃV�O�i��(�T�u�~�b�g���ꂽ�R�}���h�̊���)��҂� (Used when submit, and wait signal on top of Draw())
//!< ����ƂQ��ڈȍ~�𓯂��Ɉ����ׂɁA�V�O�i���ςݏ��(VK_FENCE_CREATE_SIGNALED_BIT)�ō쐬���Ă��� (Create with signaled state, to do same operation on first time and second time)
void VK::CreateFence(VkDevice Dev)
{
	const VkFenceCreateInfo FenceCreateInfo = {
		VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
		nullptr,
		VK_FENCE_CREATE_SIGNALED_BIT
	};
	VERIFY_SUCCEEDED(vkCreateFence(Dev, &FenceCreateInfo, GetAllocationCallbacks(), &Fence));
	VERIFY_SUCCEEDED(vkCreateFence(Dev, &FenceCreateInfo, GetAllocationCallbacks(), &ComputeFence));

	LOG_OK();
}

//!< �L���[���ł̓���(�قȂ�L���[�Ԃ̓������\) (Synchronization internal queue)
//!< �C���[�W�擾(vkAcquireNextImageKHR)�A�T�u�~�b�g(VkSubmitInfo)�A�v���[���e�[�V����(VkPresentInfoKHR)�Ɏg�p���� (Use when image acquire, submit, presentation) 
void VK::CreateSemaphore(VkDevice Dev)
{
	const VkSemaphoreCreateInfo SemaphoreCreateInfo = {
		VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
		nullptr,
		0
	};

	//!< �v���[���g���������p (Wait for presentation finish)
	VERIFY_SUCCEEDED(vkCreateSemaphore(Dev, &SemaphoreCreateInfo, GetAllocationCallbacks(), &NextImageAcquiredSemaphore));

	//!< �`�抮�������p (Wait for render finish)
	VERIFY_SUCCEEDED(vkCreateSemaphore(Dev, &SemaphoreCreateInfo, GetAllocationCallbacks(), &RenderFinishedSemaphore));

	LOG_OK();
}

//void VK::CreateCommandPool(VkCommandPool& CP, VkDevice Dev, const VkCommandPoolCreateFlags Flags, const uint32_t QueueFamilyIndex)
//{
//	const VkCommandPoolCreateInfo CPCI = {
//		VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
//		nullptr,
//		Flags,
//		QueueFamilyIndex
//	};
//	VERIFY_SUCCEEDED(vkCreateCommandPool(Dev, &CPCI, GetAllocationCallbacks(), &CP));
//
//	LOG_OK();
//}
//!< VK_COMMAND_BUFFER_LEVEL_PRIMARY	: ���ڃL���[�ɃT�u�~�b�g�ł���A�Z�J���_�����R�[���ł��� (Can be submit, can execute secondary)
//!< VK_COMMAND_BUFFER_LEVEL_SECONDARY	: �T�u�~�b�g�ł��Ȃ��A�v���C�}��������s�����̂� (Cannot submit, only executed from primary)
//void VK::AllocateCommandBuffer(std::vector<VkCommandBuffer>& CB, const VkCommandPool CP, const VkCommandBufferLevel Level, const uint32_t Count)
//{
//	assert(Count && "");
//	CB.resize(Count);
//	const VkCommandBufferAllocateInfo CBAI = {
//		VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
//		nullptr,
//		CP,
//		Level,
//		Count
//	};
//	VERIFY_SUCCEEDED(vkAllocateCommandBuffers(Device, &CBAI, CB.data()));
//
//	LOG_OK();
//}

//!< �L���[�t�@�~�����قȂ�ꍇ�͕ʂ̃R�}���h�v�[����p�ӂ���K�v������A���̃L���[�ɂ̂݃T�u�~�b�g�ł���
//!< �����X���b�h�œ����Ƀ��R�[�f�B���O����ɂ́A�ʂ̃R�}���h�v�[������A���P�[�g���ꂽ�R�}���h�o�b�t�@�ł���K�v������ (�R�}���h�v�[���͕����X���b�h����A�N�Z�X�s��)
//!< VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT	: �R�}���h�o�b�t�@���Ƀ��Z�b�g���\�A�w�肵�Ȃ��ꍇ�̓v�[�����ɂ܂Ƃ߂ă��Z�b�g (�R�}���h�o�b�t�@�̃��R�[�f�B���O�J�n���ɈÖٓI�Ƀ��Z�b�g�����̂Œ���)
//!< VK_COMMAND_POOL_CREATE_TRANSIENT_BIT				: �Z���ŁA���x���T�u�~�b�g���Ȃ��A�����Ƀ��Z�b�g�⃊���[�X�����ꍇ�Ɏw��
void VK::CreateCommandPool()
{
	const VkCommandPoolCreateInfo CPCI = {
		VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
		nullptr,
		VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
		GraphicsQueueFamilyIndex
	};
	if (GraphicsQueueFamilyIndex == ComputeQueueFamilyIndex) {
		CommandPools.resize(1);
		VERIFY_SUCCEEDED(vkCreateCommandPool(Device, &CPCI, GetAllocationCallbacks(), &CommandPools[0]));
	}
	else {
		//!< �L���[�t�@�~�����قȂ�ꍇ�́A�ʂ̃R�}���h�v�[����p�ӂ���K�v������
		CommandPools.resize(2);
		VERIFY_SUCCEEDED(vkCreateCommandPool(Device, &CPCI, GetAllocationCallbacks(), &CommandPools[0]));
		const VkCommandPoolCreateInfo CPCI_Compute = {
			VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
			nullptr,
			VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
			ComputeQueueFamilyIndex
		};
		VERIFY_SUCCEEDED(vkCreateCommandPool(Device, &CPCI_Compute, GetAllocationCallbacks(), &CommandPools[1]));
	}

	//!< �Z�J���_���p�ɕʃv�[���ɂ���K�v�͖������A�����ł͕ʃv�[���Ƃ��Ă���
#ifdef USE_SECONDARY_COMMAND_BUFFER
	SecondaryCommandPools.resize(1);
	VERIFY_SUCCEEDED(vkCreateCommandPool(Device, &CPCI, GetAllocationCallbacks(), &SecondaryCommandPools[0]));
#endif
}

//!< VK_COMMAND_BUFFER_LEVEL_PRIMARY	: ���ڃL���[�ɃT�u�~�b�g�ł���A�Z�J���_�����R�[���ł��� (Can be submit, can execute secondary)
//!< VK_COMMAND_BUFFER_LEVEL_SECONDARY	: �T�u�~�b�g�ł��Ȃ��A�v���C�}��������s�����̂� (Cannot submit, only executed from primary)
uint32_t VK::AddCommandBuffer()
{
	assert(!CommandPools.empty() && "");

	const auto PrevCount = CommandBuffers.size();
	CommandBuffers.resize(PrevCount + SwapchainImages.size());
	const VkCommandBufferAllocateInfo CBAI = {
		VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		nullptr,
		CommandPools[0],
		VK_COMMAND_BUFFER_LEVEL_PRIMARY,
		static_cast<uint32_t>(SwapchainImages.size())
	};
	VERIFY_SUCCEEDED(vkAllocateCommandBuffers(Device, &CBAI, &CommandBuffers[PrevCount]));
	return CBAI.commandBufferCount;
}
uint32_t VK::AddSecondaryCommandBuffer()
{
	assert(!SecondaryCommandPools.empty() && "");

	const auto PrevCount = SecondaryCommandBuffers.size();
	SecondaryCommandBuffers.resize(PrevCount + SwapchainImages.size());
	const VkCommandBufferAllocateInfo SCBAI = {
		VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		nullptr,
		SecondaryCommandPools[0],
		VK_COMMAND_BUFFER_LEVEL_SECONDARY,
		static_cast<uint32_t>(SwapchainImages.size())
	};
	VERIFY_SUCCEEDED(vkAllocateCommandBuffers(Device, &SCBAI, &SecondaryCommandBuffers[PrevCount]));
	return SCBAI.commandBufferCount;
}
VkSurfaceFormatKHR VK::SelectSurfaceFormat(VkPhysicalDevice PD, VkSurfaceKHR Sfc)
{
	uint32_t Count;
	VERIFY_SUCCEEDED(vkGetPhysicalDeviceSurfaceFormatsKHR(PD, Sfc, &Count, nullptr));
	assert(Count && "Surface format count is zero");
	std::vector<VkSurfaceFormatKHR> SFs(Count);
	VERIFY_SUCCEEDED(vkGetPhysicalDeviceSurfaceFormatsKHR(PD, Sfc, &Count, SFs.data()));

	//!< �����ł͍ŏ��Ɍ������� UNDEFINED �łȂ����̂�I�����Ă��� (Select first format but UNDEFINED here)
	const auto SelectedIndex = [&]() {
		//!< �v�f�� 1 �݂̂� UNDEFINED �̏ꍇ�A�����͖����D���Ȃ��̂�I���ł��� (If there is only 1 element and which is UNDEFINED, we can choose any)
		if (1 == SFs.size() && VK_FORMAT_UNDEFINED == SFs[0].format) {
			return -1;
		}

		for (auto i = 0; i < SFs.size(); ++i) {
#ifdef USE_HDR
			switch (SFs[i].colorSpace)
			{
			default: break;
			case VK_COLOR_SPACE_HDR10_HLG_EXT:
			case VK_COLOR_SPACE_HDR10_ST2084_EXT:
			case VK_COLOR_SPACE_DOLBYVISION_EXT:
				if (VK_FORMAT_UNDEFINED != SFs[i].format) {
					return i;
				}
			}
#else
			//!< VK_FORMAT_UNDEFINED �łȂ��ŏ��̂���
			if (VK_FORMAT_UNDEFINED != SFs[i].format) {
				return i;
			}
#endif
		
		}

		//!< �����ɗ��Ă͂����Ȃ�
#ifdef USE_HDR
		assert(false && "HDR not supported");
#else
		assert(false && "Valid surface format not found");
#endif
		return 0;
	}();

	//!< ColorSpace �̓n�[�h�E�F�A��ł̃J���[�R���|�[�l���g�̕\��(���j�A�A�m�����j�A�A�G���R�[�h�A�f�R�[�h��)
	Log("\t\tFormats\n");
	for (auto i = 0; i < SFs.size(); ++i) {
		Log("\t\t\t");
		if (i == SelectedIndex) {
			Log("->");
		}
		Logf("Format = %s, ColorSpace = %s\n", GetFormatChar(SFs[i].format), GetColorSpaceChar(SFs[i].colorSpace));
	}
	if (-1 == SelectedIndex) {
		Log("\t\t\t->");
		Logf("Format = %s, ColorSpace = %s\n", GetFormatChar(VK_FORMAT_B8G8R8A8_UNORM), GetColorSpaceChar(VK_COLOR_SPACE_SRGB_NONLINEAR_KHR));
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
VkPresentModeKHR VK::SelectSurfacePresentMode(VkPhysicalDevice PD, VkSurfaceKHR Sfc)
{
	uint32_t Count;
	VERIFY_SUCCEEDED(vkGetPhysicalDeviceSurfacePresentModesKHR(PD, Sfc, &Count, nullptr));
	assert(Count && "Present mode count is zero");
	std::vector<VkPresentModeKHR> PMs(Count);
	VERIFY_SUCCEEDED(vkGetPhysicalDeviceSurfacePresentModesKHR(PD, Sfc, &Count, PMs.data()));

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
//void VK::CreateSwapchain(VkPhysicalDevice PD, VkSurfaceKHR Surface)
//{
//	CreateSwapchain(PD, Surface, Rect);
//	GetSwapchainImage(Device, Swapchain);
//	CreateSwapchainImageView();
//}
void VK::CreateSwapchain(VkPhysicalDevice PD, VkSurfaceKHR Sfc, const uint32_t Width, const uint32_t Height)
{
	VkSurfaceCapabilitiesKHR SurfaceCap;
	VERIFY_SUCCEEDED(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(PD, Sfc, &SurfaceCap));

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
	const auto SurfaceFormat = SelectSurfaceFormat(PD, Sfc);
	ColorFormat = SurfaceFormat.format; //!< �J���[�t�@�[�}�b�g�͊o���Ă���

	//!< �T�[�t�F�X�̃T�C�Y��I��
	SurfaceExtent2D = SelectSurfaceExtent(SurfaceCap, Width, Height);
	Logf("\t\t\tSurfaceExtent = %d x %d\n", SurfaceExtent2D.width, SurfaceExtent2D.height);

	//!< ���C���[�A�X�e���I�����_�����O�����������ꍇ��1�ȏ�ɂȂ邪�A�����ł�1
	uint32_t ImageArrayLayers = 1;

#ifdef USE_RENDER_PASS_CLEAR
	const auto ImageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
#else
	//!< ���O�ŃN���A����ꍇ�� VK_IMAGE_USAGE_TRANSFER_DST_BIT �t���O���K�v
	const auto ImageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | (VK_IMAGE_USAGE_TRANSFER_DST_BIT & SurfaceCap.supportedUsageFlags);
#endif

	//!< �O���t�B�b�N�ƃv���[���g�̃L���[�t�@�~�����قȂ�ꍇ�̓L���[�t�@�~���C���f�b�N�X�̔z�񂪕K�v�A�܂� VK_SHARING_MODE_CONCURRENT ���w�肷�邱��
	//!< (������ VK_SHARING_MODE_CONCURRENT �ɂ���ƃp�t�H�[�}���X��������ꍇ������)
	std::vector<uint32_t> QueueFamilyIndices;
	if (GraphicsQueueFamilyIndex != PresentQueueFamilyIndex) {
		QueueFamilyIndices.push_back(GraphicsQueueFamilyIndex);
		QueueFamilyIndices.push_back(PresentQueueFamilyIndex);
	}

	//!< �T�[�t�F�X����]�A���]�������邩�ǂ��� (Rotate, mirror surface or not)
	const auto SurfaceTransform = (VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR & SurfaceCap.supportedTransforms) ? VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR : SurfaceCap.currentTransform;

	//!< �T�[�t�F�X�̃v���[���g���[�h��I��
	const auto SurfacePresentMode = SelectSurfacePresentMode(PD, Sfc);

	//!< �����̂͌�ŊJ������̂� OldSwapchain �Ɋo���Ă��� (�Z�b�e�B���O��ύX���ăX���b�v�`�F�C�����č쐬����ꍇ���ɔ�����)
	auto OldSwapchain = Swapchain;
	const VkSwapchainCreateInfoKHR SwapchainCreateInfo = {
		VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
		nullptr,
		0,
		Sfc,
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

#ifdef USE_HDR
	const std::array<VkSwapchainKHR, 1> SCs = { Swapchain };
	const std::array<VkHdrMetadataEXT, 1> MDs = { {
		VK_STRUCTURE_TYPE_HDR_METADATA_EXT,
		nullptr,
		{ 0.708f, 0.292f },
		{ 0.17f, 0.797f },
		{ 0.131f, 0.046f },
		{ 0.3127f, 0.329f },
		1000.0f,
		0.001f,
		2000.0f,
		500.0f
	} };
	assert(SCs.size() == MDs.size() && "");
	vkSetHdrMetadataEXT(Device, static_cast<uint32_t>(SCs.size()), SCs.data(), MDs.data());
#endif

	//!< (�����)�O�̂�͔j��
	if (VK_NULL_HANDLE != OldSwapchain) {
		for (auto i : SwapchainImageViews) {
			vkDestroyImageView(Device, i, GetAllocationCallbacks());
		}
		SwapchainImageViews.clear();
		vkDestroySwapchainKHR(Device, OldSwapchain, GetAllocationCallbacks());
	}

	LOG_OK();
}
void VK::ResizeSwapchain(const uint32_t Width, const uint32_t Height)
{
	//!< #VK_TODO �X���b�v�`�F�C���̃��T�C�Y�Ή�
	if (VK_NULL_HANDLE != Device) {
		VERIFY_SUCCEEDED(vkDeviceWaitIdle(Device));
	}

	//for (auto i : CommandPools) {
	//	//if (!i.second.empty()) {
	//	//	vkFreeCommandBuffers(Device, i.first, static_cast<uint32_t>(i.second.size()), i.second.data());
	//	//	i.second.clear();
	//	//}
	//	vkDestroyCommandPool(Device, i.first, GetAllocationCallbacks());
	//}
	//CommandPools.clear();

	CreateSwapchain(GetCurrentPhysicalDevice(), Surface, Width, Height);
	GetSwapchainImage(Device, Swapchain);
	CreateSwapchainImageView();
}
void VK::GetSwapchainImage(VkDevice Dev, VkSwapchainKHR SC)
{
	//!< �v���[���e�[�V�����G���W������C���[�W���擾����ہA�n���h���ł͂Ȃ� vkGetSwapchainImagesKHR() �Ŏ擾�����C���f�b�N�X���Ԃ�̂ŁA�����Ŏ擾���������͏d�v�ɂȂ�
	uint32_t Count;
	VERIFY_SUCCEEDED(vkGetSwapchainImagesKHR(Dev, SC, &Count, nullptr));
	assert(Count && "Swapchain image count is zero");
	SwapchainImages.clear();
	SwapchainImages.resize(Count);
	VERIFY_SUCCEEDED(vkGetSwapchainImagesKHR(Dev, SC, &Count, SwapchainImages.data()));

	LOG_OK();
}
/**
@note Vulakn�ł́A1�̃R�}���h�o�b�t�@�ŕ����̃X���b�v�`�F�C���C���[�W���܂Ƃ߂ď����ł�����ۂ�
*/
void VK::InitializeSwapchainImage(const VkCommandBuffer CB, const VkClearColorValue* CCV)
{
	const VkCommandBufferBeginInfo CBBI = {
		VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		nullptr,
		VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
		nullptr
	};
	VERIFY_SUCCEEDED(vkBeginCommandBuffer(CB, &CBBI)); {
		for (auto& i : SwapchainImages) {
			if (nullptr == CCV) {
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
				vkCmdPipelineBarrier(CB,
					VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT/*VK_PIPELINE_STAGE_TRANSFER_BIT*/,
					0,
					0, nullptr,
					0, nullptr,
					1, &ImageMemoryBarrier_UndefinedToPresent);
			}
			else {
				//!< �N���A�J���[���w�肳��Ă���ꍇ (If clear color is specified)
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
				vkCmdPipelineBarrier(CB,
					VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
					0,
					0, nullptr,
					0, nullptr,
					1, &ImageMemoryBarrier_UndefinedToTransfer);
				{
					vkCmdClearColorImage(CB, i, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, CCV, 1, &ImageSubresourceRange_Color);
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
				vkCmdPipelineBarrier(CB,
					VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
					0,
					0, nullptr,
					0, nullptr,
					1, &ImageMemoryBarrier_TransferToPresent);
			}
		}
	} VERIFY_SUCCEEDED(vkEndCommandBuffer(CB));

	const std::vector<VkSubmitInfo> SIs = {
		{
			VK_STRUCTURE_TYPE_SUBMIT_INFO,
			nullptr,
			0, nullptr, nullptr,
			1,  &CB,
			0, nullptr
		}
	};
	VERIFY_SUCCEEDED(vkQueueSubmit(GraphicsQueue, static_cast<uint32_t>(SIs.size()), SIs.data(), VK_NULL_HANDLE));

	//!< �L���[�ɃT�u�~�b�g���ꂽ�R�}���h����������܂Ńu���b�L���O (�t�F���X��p���Ȃ��u���b�L���O���@)
	VERIFY_SUCCEEDED(vkQueueWaitIdle(GraphicsQueue));

	LOG_OK();
}

void VK::CreateSwapchainImageView()
{
	for(auto i : SwapchainImages) {
		VkImageView IV;
		CreateImageView(&IV, i, VK_IMAGE_VIEW_TYPE_2D, ColorFormat, ComponentMapping_Identity, ImageSubresourceRange_Color);
		
		SwapchainImageViews.push_back(IV);
	}

	LOG_OK();
}

void VK::CreateRenderTarget(const VkFormat Format, const uint32_t Width, const uint32_t Height)
{
	const VkExtent3D Extent3D = { Width, Height, 1 };
	CreateImage(&RenderTargetImage, 0, VK_IMAGE_TYPE_2D, Format, Extent3D, 1, 1, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);

	AllocateImageMemory(&RenderTargetDeviceMemory, RenderTargetImage, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	VERIFY_SUCCEEDED(vkBindImageMemory(Device, RenderTargetImage, RenderTargetDeviceMemory, 0));

	CreateImageView(&RenderTargetImageView, RenderTargetImage, VK_IMAGE_VIEW_TYPE_2D, Format, ComponentMapping_Identity, ImageSubresourceRange_Color);

	LOG_OK();
}

void VK::CreateDepthStencil(const VkFormat DepthFormat, const uint32_t Width, const uint32_t Height)
{
	assert(IsSupportedDepthFormat(GetCurrentPhysicalDevice(), DepthFormat) && "Not supported depth format");

	const VkExtent3D Extent3D = { Width, Height, 1 };
	//!< vkCmdClearDepthStencilImage ��p���ăN���A����ꍇ�ɂ� VK_IMAGE_USAGE_TRANSFER_DST_BIT ���w�肷�邱�� (�����ł̓����_�[�p�X�ŃN���A����z��Ŏw�肵�Ȃ�)
	CreateImage(&DepthStencilImage, 0, VK_IMAGE_TYPE_2D, DepthFormat, Extent3D, 1, 1, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT /*| VK_IMAGE_USAGE_TRANSFER_DST_BIT*/);

	AllocateImageMemory(&DepthStencilDeviceMemory, DepthStencilImage, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	VERIFY_SUCCEEDED(vkBindImageMemory(Device, DepthStencilImage, DepthStencilDeviceMemory, 0));
	
	CreateImageView(&DepthStencilImageView, DepthStencilImage, VK_IMAGE_VIEW_TYPE_2D, DepthFormat, ComponentMapping_Identity, ImageSubresourceRange_DepthStencil);

	LOG_OK();
}

void VK::InitializeDepthStencilImage(const VkCommandBuffer CB)
{
	if (VK_NULL_HANDLE == DepthStencilImage) return;

	//!< VK_IMAGE_LAYOUT_UNDEFINED �ō쐬����Ă���̂ŁA���C�A�E�g��ύX����K�v������
	const VkCommandBufferBeginInfo CBBI = {
		VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		nullptr,
		VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
		nullptr
	};
	VERIFY_SUCCEEDED(vkBeginCommandBuffer(CB, &CBBI)); {
		const VkImageMemoryBarrier IMB = {
			VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
			nullptr,
			0,
			0,
			VK_IMAGE_LAYOUT_UNDEFINED,
			VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
			VK_QUEUE_FAMILY_IGNORED,
			VK_QUEUE_FAMILY_IGNORED,
			DepthStencilImage,
			ImageSubresourceRange_DepthStencil/*ImageSubresourceRange_Depth : �f�v�X�݂̂̃t�H�[�}�b�g�̏ꍇ*/
		};
		vkCmdPipelineBarrier(CB,
			VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
			0,
			0, nullptr,
			0, nullptr,
			1, &IMB);
	} VERIFY_SUCCEEDED(vkEndCommandBuffer(CB));
	const std::vector<VkSubmitInfo> SIs = {
		{
			VK_STRUCTURE_TYPE_SUBMIT_INFO,
			nullptr,
			0, nullptr, nullptr,
			1,  &CB,
			0, nullptr
		}
	};
	VERIFY_SUCCEEDED(vkQueueSubmit(GraphicsQueue, static_cast<uint32_t>(SIs.size()), SIs.data(), VK_NULL_HANDLE));
	VERIFY_SUCCEEDED(vkQueueWaitIdle(GraphicsQueue));
}

void VK::CreateViewport(const float Width, const float Height, const float MinDepth, const float MaxDepth)
{
	Viewports = {
		{
			//!< VK�ł̓f�t�H���g�ŁuY�����v���������A�����ɕ��̒l���w�肷��ƁuY����v������DX�Ɠ��l�ɂȂ� (In VK, by specifying negative height, Y become up. same as DX)
			//!< �ʏ��_�́u����v���w�肷�邪�A�����ɕ��̒l���w�肷��ꍇ�́u�����v���w�肷�邱�� (When negative height, specify left bottom as base, otherwise left up)
#ifdef USE_VIEWPORT_Y_UP
			0, Height,
			Width, -Height,
#else
			0, 0,
			Width, Height,
#endif
			MinDepth, MaxDepth
		}
	};
	ScissorRects = {
		{
			{ 0, 0 },
			{ static_cast<uint32_t>(Width), static_cast<uint32_t>(Height) }
		}
	};

	LOG_OK();
}

void VK::SubmitStagingCopy(const VkQueue Queue, const VkCommandBuffer CB, const VkBuffer Buffer, const VkDeviceSize Size, const void* Source, const VkAccessFlagBits AF, const VkPipelineStageFlagBits PSF)
{
#define USE_SUBALLOC
	VkBuffer StagingBuffer = VK_NULL_HANDLE;
	//!< �z�X�g�r�W�u���o�b�t�@(HVB)���쐬 (Create host visible buffer(HVB))
	CreateBuffer(&StagingBuffer, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, Size);

#ifdef USE_SUBALLOC
	//!< �f�o�C�X���[�J��������(DLM)���T�u�A���P�[�g (Suballocate device local memory(DLM))
	uint32_t StagingHeapIndex;
	VkDeviceSize StagingOffset;
	SuballocateBufferMemory(StagingHeapIndex, StagingOffset, StagingBuffer, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
	CopyToHostVisibleDeviceMemory(DeviceMemories[StagingHeapIndex], Size, Source, 0);
	const auto StagingSize = DeviceMemoryOffsets[StagingHeapIndex] - StagingOffset;
#else
	VkDeviceMemory StagingDeviceMemory = VK_NULL_HANDLE;

	//!< �f�o�C�X���[�J��������(DLM)���A���P�[�g (Allocate device local memory(DLM))
	AllocateBufferMemory(&StagingDeviceMemory, StagingBuffer, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
	VERIFY_SUCCEEDED(vkBindBufferMemory(Device, StagingBuffer, StagingDeviceMemory, 0));
	CopyToHostVisibleDeviceMemory(StagingDeviceMemory, Size, Source, 0);
#endif

	//!< HVB����DLB�ւ̃R�s�[�R�}���h�𔭍s (Submit HVB to DLB copy command)
	CmdCopyBufferToBuffer(CB, StagingBuffer, Buffer, AF, PSF, Size);
	const std::array<VkCommandBuffer, 1> CBs = { CB };
	const std::array<VkSubmitInfo, 1> SIs = {
		{
			VK_STRUCTURE_TYPE_SUBMIT_INFO,
			nullptr,
			0, nullptr,
			nullptr,
			static_cast<uint32_t>(CBs.size()), CBs.data(),
			0, nullptr
		}
	};
	VERIFY_SUCCEEDED(vkQueueSubmit(Queue, static_cast<uint32_t>(SIs.size()), SIs.data(), VK_NULL_HANDLE));
	VERIFY_SUCCEEDED(vkQueueWaitIdle(Queue));
#ifdef USE_SUBALLOC
	//!< �T�u�A���P�[�g�����I�t�Z�b�g���߂� (Revert suballocated memory offset)
	DeviceMemoryOffsets[StagingHeapIndex] -= StagingSize;
	Logf("\t\tSubreleaseBufferMemory = %llu / %llu (HeapIndex = %d)\n", DeviceMemoryOffsets[StagingHeapIndex], GetCurrentPhysicalDeviceMemoryProperties().memoryHeaps[StagingHeapIndex].size, StagingHeapIndex);
#else
	if (VK_NULL_HANDLE != StagingDeviceMemory) {
		vkFreeMemory(Device, StagingDeviceMemory, GetAllocationCallbacks());
	}
#endif
	if (VK_NULL_HANDLE != StagingBuffer) {
		vkDestroyBuffer(Device, StagingBuffer, GetAllocationCallbacks());
	}
#undef USE_SUBALLOC
}

//!< �K�؂ȃI�t�Z�b�g�ɔz�u����(�_�C�i�~�b�N�ł̓I�t�Z�b�g�w�肪�ς��)
//!< �f�X�N���v�^�^�C�v�� VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC
//!< �V�F�[�_����ǂݏ����\
//!< �A�g�~�b�N�ȑ��삪�\
//!< �V�F�[�_�� layout (set=0, binding=0) buffer MyBuffer { vec4 MyVec4; mat4 MyMat4; }
void VK::CreateStorageBuffer()
{
#if 0
	VkBuffer Buffer;
	VkDeviceSize Size = 0;
	CreateBuffer(&Buffer, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, Size);
	//CreateBuffer(&Buffer, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, Size);

	uint32_t HeapIndex;
	VkDeviceSize Offset;
	SuballocateBufferMemory(HeapIndex, Offset, Buffer, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	VERIFY_SUCCEEDED(vkBindBufferMemory(Device, Buffer, DeviceMemories[HeapIndex], Offset));
#endif
}

//!< �C���[�W�����傫�ȃf�[�^�փA�N�Z�X�\(1D�C���[�W�͍Œ��4096�e�N�Z�������A���j�t�H�[���e�N�Z���o�b�t�@�͍Œ��65536�e�N�Z��)
//!< �f�X�N���v�^�^�C�v�� VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER
//!< (TexelBuffer�n��)�r���[���K�v
//!< �V�F�[�_�� layout (set=0, binding=0) uniform samplerBuffer MySamplerBuffer;
void VK::CreateUniformTexelBuffer()
{
#if 0
	VkBuffer Buffer;
	const VkDeviceSize Size = 0;
	CreateBuffer(&Buffer, VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT, Size);

	uint32_t HeapIndex;
	VkDeviceSize Offset;
	SuballocateBufferMemory(HeapIndex, Offset, Buffer, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	VERIFY_SUCCEEDED(vkBindBufferMemory(Device, Buffer, DeviceMemories[HeapIndex], Offset));

	VkBufferView View;
	const VkFormat Format = VK_FORMAT_R8G8B8A8_UINT;
	const VkBufferViewCreateInfo BVCI = {
			VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO,
			nullptr,
			0,
			Buffer,
			Format,
			0,
			VK_WHOLE_SIZE
	};
	VERIFY_SUCCEEDED(vkCreateBufferView(Device, &BVCI, GetAllocationCallbacks(), &View)); 

	//!< �T�|�[�g����Ă��邩�̃`�F�b�N
	VkFormatProperties FP;
	vkGetPhysicalDeviceFormatProperties(GetCurrentPhysicalDevice(), Format, &FP);
	assert((FP.bufferFeatures & VK_FORMAT_FEATURE_UNIFORM_TEXEL_BUFFER_BIT) && "");
#endif
}

//!< �V�F�[�_���珑�����݉\
//!< �f�X�N���v�^�^�C�v�� VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER
//!< (TexelBuffer�n��)�r���[���K�v
//!< �A�g�~�b�N�ȑ��삪�\
//!< �V�F�[�_�� layout (set=0, binding=0, r32f) uniform imageBuffer MyImageBuffer;
void VK::CreateStorageTexelBuffer()
{
#if 0
	VkBuffer Buffer;
	const VkDeviceSize Size = 0;
	CreateBuffer(&Buffer, VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT, Size);

	uint32_t HeapIndex;
	VkDeviceSize Offset;
	SuballocateBufferMemory(HeapIndex, Offset, Buffer, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	VERIFY_SUCCEEDED(vkBindBufferMemory(Device, Buffer, DeviceMemories[HeapIndex], Offset));

	VkBufferView View;
	const VkFormat Format = VK_FORMAT_R8G8B8A8_UINT;
	const VkBufferViewCreateInfo BVCI = {
		VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO,
		nullptr,
		0,
		Buffer,
		Format,
		0,
		VK_WHOLE_SIZE
	};
	VERIFY_SUCCEEDED(vkCreateBufferView(Device, &BVCI, GetAllocationCallbacks(), &View));

	//!< �T�|�[�g����Ă��邩�̃`�F�b�N
	VkFormatProperties FP;
	vkGetPhysicalDeviceFormatProperties(GetCurrentPhysicalDevice(), Format, &FP);
	assert((FP.bufferFeatures & VK_FORMAT_FEATURE_STORAGE_TEXEL_BUFFER_BIT) && "");
	//!< #VK_TODO �A�g�~�b�N���������ꍇ
	if (false/*bUseAtomic*/) {
		assert((FP.linearTilingFeatures & VK_FORMAT_FEATURE_STORAGE_TEXEL_BUFFER_ATOMIC_BIT) && "");
	}
#endif
}

void VK::CreateDescriptorSetLayout(VkDescriptorSetLayout& DSL, const std::initializer_list<VkDescriptorSetLayoutBinding> il_DSLBs)
{
	const std::vector<VkDescriptorSetLayoutBinding> DSLBs(il_DSLBs.begin(), il_DSLBs.end());

	const VkDescriptorSetLayoutCreateInfo DSLCI = {
		VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
		nullptr,
#ifdef USE_PUSH_DESCRIPTOR
		VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR,
#else
		0,
#endif
		static_cast<uint32_t>(DSLBs.size()), DSLBs.data()
	};
	VERIFY_SUCCEEDED(vkCreateDescriptorSetLayout(Device, &DSLCI, GetAllocationCallbacks(), &DSL));

	LOG_OK();
}

void VK::CreatePipelineLayout(VkPipelineLayout& PL, const std::initializer_list<VkDescriptorSetLayout> il_DSLs, const std::initializer_list<VkPushConstantRange> il_PCRs)
{
	const std::vector<VkDescriptorSetLayout> DSLs(il_DSLs.begin(), il_DSLs.end());

	//!< �v�b�V���R���X�^���g�����W : �f�X�N���v�^�Z�b�g��������
	//!< �p�C�v���C�����C�A�E�g�S�̂�128byte (�n�[�h�ɂ�肱��ȏ�g����ꍇ������ GTX970M : 256byte)
	//!< �e�V�F�[�_�X�e�[�W��1�̃v�b�V���R���X�^���g�����W�ɂ����A�N�Z�X�ł��Ȃ�
	//!< �e�V�F�[�_�X�e�[�W���u���ʂ̃����W�������Ȃ��v�悤�ȁu���[�X�g�P�[�X�v�ł� 128/5==25.6�A1�V�F�[�_�X�e�[�W��25byte���x�ƂȂ�
	//const std::array<VkPushConstantRange, 0> PCRs = { { VK_SHADER_STAGE_VERTEX_BIT, 0, 64 }, { VK_SHADER_STAGE_FRAGMENT_BIT, 64, 64 }, };
	const std::vector<VkPushConstantRange> PCRs(il_PCRs.begin(), il_PCRs.end());

	const VkPipelineLayoutCreateInfo PLCI = {
		VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
		nullptr,
		0,
		static_cast<uint32_t>(DSLs.size()), DSLs.data(),
		static_cast<uint32_t>(PCRs.size()), PCRs.data()
	};
	VERIFY_SUCCEEDED(vkCreatePipelineLayout(Device, &PLCI, GetAllocationCallbacks(), &PL));

	LOG_OK();
}

//!< �f�X�N���v�^�Z�b�g���X�ɉ���������ꍇ�ɂ� VkDescriptorPoolCreateInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT ���w�肷��A
//!< ���̏ꍇ�f�Љ��͎����ŊǗ����Ȃ��Ă͂Ȃ�Ȃ� (�w�肵�Ȃ��ꍇ�̓v�[�����ɂ܂Ƃ߂ĉ�������ł��Ȃ�)
//!< 1�̃u�[���ɑ΂��āA�����X���b�h�œ����Ƀf�X�N���v�^�Z�b�g���m�ۂ��邱�Ƃ͂ł��Ȃ� (�X���b�h���ɕʃv�[���ɂ��邱��)
void VK::CreateDescriptorPool(VkDescriptorPool& DP, const VkDescriptorPoolCreateFlags Flags, const std::initializer_list<VkDescriptorPoolSize> il_DPSs)
{
	const std::vector<VkDescriptorPoolSize> DPSs(il_DPSs.begin(), il_DPSs.end());

	uint32_t MaxSets = 0;
	for (const auto& i : DPSs) {
		MaxSets = std::max(MaxSets, i.descriptorCount);
	}

	const VkDescriptorPoolCreateInfo DPCI = {
		VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
		nullptr,
		Flags,
		MaxSets,
		static_cast<uint32_t>(DPSs.size()), DPSs.data()
	};
	VERIFY_SUCCEEDED(vkCreateDescriptorPool(Device, &DPCI, GetAllocationCallbacks(), &DP));

	LOG_OK();
}

//!< �V�F�[�_���\�[�X��1�̃R���e�i�I�u�W�F�N�g�ɂ܂Ƃ߂� (�^�␔�̓Z�b�g���C�A�E�g�Œ�`����A�X�g���[�W�̓v�[������m�ۂ����)
//void VK::AllocateDescriptorSet(std::vector<VkDescriptorSet>& DSs, const VkDescriptorPool DP, const std::initializer_list <VkDescriptorSetLayout> il_DSLs)
//{
//	const std::vector<VkDescriptorSetLayout> DSLs(il_DSLs.begin(), il_DSLs.end());
//	DSs.resize(DSLs.size());
//	const VkDescriptorSetAllocateInfo DSAI = {
//		VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
//		nullptr,
//		DP,
//		static_cast<uint32_t>(DSLs.size()), DSLs.data()
//	};
//	VERIFY_SUCCEEDED(vkAllocateDescriptorSets(Device, &DSAI, DSs.data()));
//
//	LOG_OK();
//}

void VK::UpdateDescriptorSet(const std::initializer_list <VkWriteDescriptorSet> il_WDSs, const std::initializer_list <VkCopyDescriptorSet> il_CDSs)
{
	//!< dstArrayElement ... �o�C���f�B���O���ł̔z��̊J�n�Y�� (VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK_EXT�w��̏ꍇ�͊J�n�o�C�g�I�t�Z�b�g)
	//!< descriptorCount ... �X�V����f�X�N���v�^�Z�b�g�� (VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK_EXT�w��̏ꍇ�͍X�V����o�C�g)
	//!< �w�� descriptorType �ɏ]���āApImageInfo, pBufferInfo, pTexelBufferView �̓K�؂ȉӏ��֎w�肷�邱��
	const std::vector<VkWriteDescriptorSet> WDSs(il_WDSs.begin(), il_WDSs.end());
	const std::vector<VkCopyDescriptorSet> CDSs(il_CDSs.begin(), il_CDSs.end());

	vkUpdateDescriptorSets(Device,
		static_cast<uint32_t>(WDSs.size()), WDSs.data(),
		static_cast<uint32_t>(CDSs.size()), CDSs.data());

	LOG_OK();
}

//void VK::CreateDescriptorSet()
//{
//	assert(!DescriptorPools.empty() && "");
//
//	const auto DP = DescriptorPools[0];
//	if (!DescriptorSetLayouts.empty()) {
//		const VkDescriptorSetAllocateInfo DSAI = {
//			VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
//			nullptr,
//			DP,
//			static_cast<uint32_t>(DescriptorSetLayouts.size()), DescriptorSetLayouts.data()
//		};
//		DescriptorSets.resize(DescriptorSetLayouts.size());
//		VERIFY_SUCCEEDED(vkAllocateDescriptorSets(Device, &DSAI, DescriptorSets.data()));
//	}
//
//	LOG_OK();
//}

void VK::CreateRenderPass_Default(VkRenderPass& RP, const VkFormat Color)
{
	//!< �A�^�b�`�����g ... �����_�[�p�X�ł̕`���
	const std::array<VkAttachmentDescription, 1> ADs = {
		{
			0,
			Color,
			VK_SAMPLE_COUNT_1_BIT,
			//!< �A�^�b�`�����g�̃��[�h�X�g�A
#ifdef USE_RENDER_PASS_CLEAR
			//!<�u�J�n���ɃN���A�v�u�I�����ɕۑ��v
			VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE,
#else
			//!<�u�J�n���ɉ������Ȃ��v�u�I�����ɕۑ��v
			VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_STORE,
#endif
			//!< �X�e���V���̃��[�h�X�g�A : (�����ł�)�J�n���A�I�����Ƃ��Ɂu�g�p���Ȃ��v
			VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
			//!< �����_�[�p�X�̃��C�A�E�g : �u�J�n������`�v�u�I�����v���[���e�[�V�����\�[�X�v
			VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
		}
	};

	//!< layout (input_attachment_index=0, set=0, binding=0) uniform SubpassInput XXX;
	//!< �C���v�b�g�A�^�b�`�����g : �ǂݎ��p
	const std::array<VkAttachmentReference, 0> Input_Pass0 = {};
	//!< �J���[�A�^�b�`�����g : �������ݗp
	const std::array<VkAttachmentReference, 1> Color_Pass0 = { { 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL }, };
	//!< ���]���u�A�^�b�`�����g : �}���`�T���v�� �� �V���O���T���v��
	const std::array<VkAttachmentReference, 1> Resolve_Pass0 = { { VK_ATTACHMENT_UNUSED }, };
	//assert(ColorARs.size() == ResolveARs.size() && "Size must be same");
	//!< �f�v�X�A�^�b�`�����g : �[�x�p
	const VkAttachmentReference* Depth_Pass0 = nullptr;
	//!< �v���U�[�u�A�^�b�`�����g : �T�u�p�X�S�̂ɂ����ĕێ����Ȃ��Ă͂Ȃ�Ȃ��R���e���c(�̃C���f�b�N�X)
	const std::array<uint32_t, 0> Preserve_Pass0 = {};
	const std::array<VkSubpassDescription, 1> SubpassDescs = {
		{
			0,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			static_cast<uint32_t>(Input_Pass0.size()), Input_Pass0.data(),
			static_cast<uint32_t>(Color_Pass0.size()), Color_Pass0.data(), Resolve_Pass0.data(),
			Depth_Pass0,
			static_cast<uint32_t>(Preserve_Pass0.size()), Preserve_Pass0.data()
		}
	};

	//!< �T�u�p�X
#if 0
	const std::array<VkSubpassDependency, 0> SubpassDepends = {};
#else
	const std::array<VkSubpassDependency, 2> SubpassDepends = { {
			//!< �K�v�������A�����ď����Ȃ炱��Ȋ��� (No need this code, but if dare to write like this)
			{
				VK_SUBPASS_EXTERNAL, 0,																	//!< �T�u�p�X�O����T�u�p�X0��
				VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,	//!< �p�C�v���C���̍ŏI�X�e�[�W����J���[�o�̓X�e�[�W��
				VK_ACCESS_MEMORY_READ_BIT, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,						//!< �ǂݍ��݂���J���[�������݂�
				VK_DEPENDENCY_BY_REGION_BIT,															//!< �����������̈�ɑ΂��鏑�����݂��������Ă���ǂݍ��� (�w�肵�Ȃ��ꍇ�͎��O�ŏ������݊������Ǘ�)
			},
			{
				0, VK_SUBPASS_EXTERNAL,																	//!< �T�u�p�X0����T�u�p�X�O��
				VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,	//!< �J���[�o�̓X�e�[�W����p�C�v���C���̍ŏI�X�e�[�W��
				VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_ACCESS_MEMORY_READ_BIT,						//!< �J���[�������݂���ǂݍ��݂�
				VK_DEPENDENCY_BY_REGION_BIT,
			}
		} };
#endif

	const VkRenderPassCreateInfo RPCI = {
		VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
		nullptr,
		0,
		static_cast<uint32_t>(ADs.size()), ADs.data(),
		static_cast<uint32_t>(SubpassDescs.size()), SubpassDescs.data(),
		static_cast<uint32_t>(SubpassDepends.size()), SubpassDepends.data()
	};
	VERIFY_SUCCEEDED(vkCreateRenderPass(Device, &RPCI, GetAllocationCallbacks(), &RP));
}

void VK::CreateFramebuffer(VkFramebuffer& FB, const VkRenderPass RP, const uint32_t Width, const uint32_t Height, const uint32_t Layers, const std::initializer_list<VkImageView> il_IVs)
{
	const std::vector<VkImageView> IVs(il_IVs.begin(), il_IVs.end());

	const VkFramebufferCreateInfo FCI = {
		VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
		nullptr,
		0,
		RP, //!< �����Ŏw�肷�郌���_�[�p�X�͌݊����̂�����̂Ȃ��
		static_cast<uint32_t>(IVs.size()), IVs.data(),
		Width, Height,
		Layers
	};
	VERIFY_SUCCEEDED(vkCreateFramebuffer(Device, &FCI, GetAllocationCallbacks(), &FB));
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

void VK::CreatePipeline(VkPipeline& PL, const VkPipelineLayout PLL, const VkRenderPass RP, 
	const VkShaderModule VS, const VkShaderModule FS, const VkShaderModule TES, const VkShaderModule TCS, const VkShaderModule GS, 
	const std::vector<VkVertexInputBindingDescription>& VIBDs, const std::vector<VkVertexInputAttributeDescription>& VIADs, const VkPrimitiveTopology PT, const uint32_t PatchControlPoints,
	VkPipelineCache PC)
{
	PERFORMANCE_COUNTER();

	//!< (�o���f�[�V�����ׂ̈�)�f�o�C�X�t�B�[�`���[���擾
	VkPhysicalDeviceFeatures PDF;
	vkGetPhysicalDeviceFeatures(GetCurrentPhysicalDevice(), &PDF);

	//!< �p�C�v���C���쐬���ɃV�F�[�_���̒萔�l���㏑���w��ł���
#if 0
	//!< �V�F�[�_�ɂ͈ȉ��̂悤�ȋL�q(������̂̓X�J���l�̂�)
	//layout(constant_id = 0) const int IntValue = 0;
	//layout(constant_id = 1) const float FloatValue = 0.0f;
	//layout(constant_id = 2) const bool BoolValue = false;
	struct SpecializationData {
		int IntValue;
		float FloatValue;
		bool BoolValue;
	};
	const SpecializationData SD = { 1, 1.0f, true };
	const std::vector<VkSpecializationMapEntry> SMEs = {
		{ 0, offsetof(SpecializationData, IntValue), sizeof(SD.IntValue) },
		{ 1, offsetof(SpecializationData, FloatValue), sizeof(SD.FloatValue) },
		{ 2, offsetof(SpecializationData, BoolValue), sizeof(SD.BoolValue) },
	};
	const VkSpecializationInfo SI = {
		static_cast<uint32_t>(SMEs.size()), SMEs.data(),
		sizeof(SD),& SD
	};
#endif

	//!< �V�F�[�_ (Shader)
	std::vector<VkPipelineShaderStageCreateInfo> PSSCI;
	//!< �V�F�[�_���̃R���X�^���g�ϐ����p�C�v���C���쐬���ɕύX�������ꍇ�Ɏg�p����
	//const std::array<VkSpecializationMapEntry, 0> SMEs = { /*{ uint32_t constantID, uint32_t offset, size_t size },*/ };
	//const VkSpecializationInfo SI = { static_cast<uint32_t>(SMEs.size()), SMEs.data() };
	if (VK_NULL_HANDLE != VS) {
		PSSCI.push_back({
			VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
			nullptr,
			0,
			VK_SHADER_STAGE_VERTEX_BIT, VS,
			"main",
			nullptr //!< &SI
			});
	}
	if (VK_NULL_HANDLE != FS) {
		PSSCI.push_back({
			VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
			nullptr,
			0,
			VK_SHADER_STAGE_FRAGMENT_BIT, FS,
			"main",
			nullptr //!< &SI
			});
	}
	if (VK_NULL_HANDLE != TES) {
		PSSCI.push_back({
			VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
			nullptr,
			0,
			VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT, TES,
			"main",
			nullptr //!< &SI
			});
	}
	if (VK_NULL_HANDLE != TCS) {
		PSSCI.push_back({
			VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
			nullptr,
			0,
			VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT, TCS,
			"main",
			nullptr //!< &SI
			});
	}
	if (VK_NULL_HANDLE != GS) {
		PSSCI.push_back({
			VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
			nullptr,
			0,
			VK_SHADER_STAGE_GEOMETRY_BIT, GS,
			"main",
			nullptr //!< &SI
			});
	}
	assert(!PSSCI.empty() && "");

	//!< �o�[�e�b�N�X�C���v�b�g (VertexInput)
	const VkPipelineVertexInputStateCreateInfo PVISCI = {
		VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
		nullptr,
		0,
		static_cast<uint32_t>(VIBDs.size()), VIBDs.data(),
		static_cast<uint32_t>(VIADs.size()), VIADs.data()
	};

	//!< DX�ł́u�g�|���W�v�Ɓu�p�b�`�R���g���[���|�C���g�v�̎w���IASetPrimitiveTopology()�̈����Ƃ��ăR�}���h���X�g�֎w�肷��AVK�Ƃ͌��\�قȂ�̂Œ���
	//!< (�u�p�b�`�R���g���[���|�C���g�v�̐��������w�肷�邩�ɂ�茈�܂�)
	//!< CommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	//!< CommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_1_CONTROL_POINT_PATCHLIST);

	//!< �C���v�b�g�A�Z���u�� (InputAssembly)
	const VkPipelineInputAssemblyStateCreateInfo PIASCI = {
		VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
		nullptr,
		0,
		PT,
		VK_FALSE
	};
	//!< WITH_ADJACENCY �n�g�p���ɂ� �f�o�C�X�t�B�[�`���[ geometryShader ���L���ł��邱��
	assert((
		(PIASCI.topology != VK_PRIMITIVE_TOPOLOGY_LINE_LIST_WITH_ADJACENCY || PIASCI.topology != VK_PRIMITIVE_TOPOLOGY_LINE_STRIP_WITH_ADJACENCY || PIASCI.topology != VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST_WITH_ADJACENCY || PIASCI.topology != VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP_WITH_ADJACENCY)
		|| PDF.geometryShader) /*&& ""*/);
	//!< PATCH_LIST �g�p���ɂ� �f�o�C�X�t�B�[�`���[ tessellationShader ���L���ł��邱��
	assert((PIASCI.topology != VK_PRIMITIVE_TOPOLOGY_PATCH_LIST || PDF.tessellationShader) && "");
	//!< �C���f�b�N�X 0xffffffff(VK_INDEX_TYPE_UINT32), 0xffff(VK_INDEX_TYPE_UINT16) ���v���~�e�B�u�̃��X�^�[�g�Ƃ���A�C���f�b�N�X�n�`��̏ꍇ(vkCmdDrawIndexed, vkCmdDrawIndexedIndirect)�̂ݗL��
	//!< LIST �n�g�p�� primitiveRestartEnable �����ł��邱��
	assert((
		(PIASCI.topology != VK_PRIMITIVE_TOPOLOGY_POINT_LIST || PIASCI.topology != VK_PRIMITIVE_TOPOLOGY_LINE_LIST || PIASCI.topology != VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST || PIASCI.topology != VK_PRIMITIVE_TOPOLOGY_LINE_LIST_WITH_ADJACENCY || PIASCI.topology != VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST_WITH_ADJACENCY || PIASCI.topology != VK_PRIMITIVE_TOPOLOGY_PATCH_LIST)
		|| PIASCI.primitiveRestartEnable == VK_FALSE) /*&& ""*/);

	//!< �e�Z���[�V���� (Tessellation)
	assert((PT != VK_PRIMITIVE_TOPOLOGY_PATCH_LIST || PatchControlPoints != 0) && "");
	const VkPipelineTessellationStateCreateInfo PTSCI = {
		VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO,
		nullptr,
		0,
		PatchControlPoints//!< �p�b�`�R���g���[���|�C���g
	};

	//!< �r���[�|�[�g (Viewport)
	//!< VkDynamicState ���g�p���邽�߁A�����ł̓r���[�|�[�g(�V�U�[)�̌��̂ݎw�肵�Ă��� (To use VkDynamicState, specify only count of viewport(scissor) here)
	//!< ��� vkCmdSetViewport(), vkCmdSetScissor() �Ŏw�肷�� (Use vkCmdSetViewport(), vkCmdSetScissor() later)
	const VkPipelineViewportStateCreateInfo PVSCI = {
		VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
		nullptr,
		0,
		1, nullptr, //!< Viewport
		1, nullptr	//!< Scissor
	};
	//!< 2�ȏ�̃r���[�|�[�g���g�p����ɂ̓f�o�C�X�t�B�[�`���[ multiViewport ���L���ł��邱�� (If use 2 or more viewport device feature multiViewport must be enabled)
	//!< �r���[�|�[�g�̃C���f�b�N�X�̓W�I���g���V�F�[�_�Ŏw�肷�� (Viewport index is specified in geometry shader)
	assert((PVSCI.viewportCount <= 1 || PDF.multiViewport) && "");

	//!< ���X�^���C�[�[�V���� (Rasterization)
	const VkPipelineRasterizationStateCreateInfo PRSCI = {
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
	//!< depthClampEnable �ɂ̓f�o�C�X�t�B�[�`���[ depthClamp ���L���ł��邱��
	assert((!PRSCI.depthClampEnable || PDF.depthClamp) && "");
	//!< FILL �ȊO�ɂ̓f�o�C�X�t�B�[�`���[ fillModeNonSolid ���L���ł��邱��
	assert((PRSCI.polygonMode != VK_POLYGON_MODE_FILL || PDF.fillModeNonSolid) && "");
	//!< 1.0f ���傫�Ȓl�ɂ̓f�o�C�X�t�B�[�`���[ widelines ���L���ł��邱��
	assert((PRSCI.lineWidth <= 1.0f || PDF.wideLines) && "");

	//!< �}���`�T���v�� (Multisample)
	const VkSampleMask SM = 0xffffffff; //!< 0xffffffff �w��̏ꍇ�� nullptr �ł��悢
	const VkPipelineMultisampleStateCreateInfo PMSCI = {
		VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
		nullptr,
		0,
		VK_SAMPLE_COUNT_1_BIT,
		VK_FALSE, 0.0f,
		&SM,
		VK_FALSE, VK_FALSE
	};
	assert((PMSCI.sampleShadingEnable == VK_FALSE || PDF.sampleRateShading) && "");
	assert((PMSCI.minSampleShading >= 0.0f && PMSCI.minSampleShading <= 1.0f) && "");
	assert((PMSCI.alphaToOneEnable == VK_FALSE || PDF.alphaToOne) && "");

	//!< �f�v�X�X�e���V�� (DepthStencil)
	const VkPipelineDepthStencilStateCreateInfo PDSSCI = {
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

	//!< �J���[�u�����h (ColorBlend)
	//!< VK_BLEND_FACTOR_SRC1 �n�����g�p����ɂ́A�f�o�C�X�t�B�[�`���[ dualSrcBlend ���L���ł��邱��
	///!< SRC�R���|�[�l���g * SRC�t�@�N�^ OP DST�R���|�[�l���g * DST�t�@�N�^
	const std::array<VkPipelineColorBlendAttachmentState, 1> PCBASs = {
		{
			VK_FALSE,
			VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ONE, VK_BLEND_OP_ADD,
			VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ONE, VK_BLEND_OP_ADD,
			VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
		}
	};
	//!< �f�o�C�X�t�B�[�`���[ independentBlend ���L���Ŗ����ꍇ�́A�z��̊e�v�f�́u���S�ɓ����l�v�ł��邱�� (If device feature independentBlend is not enabled, each array element must be exactly same)
	if (!PDF.independentBlend) {
		for (auto i : PCBASs) {
			assert(memcmp(&i, &PCBASs[0], sizeof(PCBASs[0])) == 0 && ""); //!< �ŏ��̗v�f�͔�ׂ�K�v�������܂�������
		}
	}
	const VkPipelineColorBlendStateCreateInfo PCBSCI = {
		VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
		nullptr,
		0,
		VK_FALSE, VK_LOGIC_OP_COPY, //!< �u�����h���ɘ_���I�y���[�V�������s�� (�u�����h�͖����ɂȂ�) (�����^�A�^�b�`�����g�ɑ΂��Ă̂�)
		static_cast<uint32_t>(PCBASs.size()), PCBASs.data(),
		{ 1.0f, 1.0f, 1.0f, 1.0f }
	};

	//!< �_�C�i�~�b�N�X�e�[�g (DynamicState)
	const std::array<VkDynamicState, 2> DSs = {
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR
	};
	const VkPipelineDynamicStateCreateInfo PDSCI = {
		VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
		nullptr,
		0,
		static_cast<uint32_t>(DSs.size()), DSs.data()
	};

	/**
	@brief �p�� ... ���ʕ����������ꍇ�A�e�p�C�v���C�����w�肵�č쐬����Ƃ�荂���ɍ쐬�ł���A�e�q�Ԃł̃X�C�b�`��o�C���h���L��
	(DX �� D3D12_CACHED_PIPELINE_STATE ����?)
	basePipelineHandle, basePipelineIndex �͓����Ɏg�p�ł��Ȃ��A�g�p���Ȃ��ꍇ�͂��ꂼ�� VK_NULL_HANDLE, -1 ���w�肷�邱��
	�e�ɂ� VK_PIPELINE_CREATE_ALLOW_DERIVATIVES_BIT �t���O���K�v�A�q�ɂ� VK_PIPELINE_CREATE_DERIVATIVE_BIT �t���O���K�v
	�EbasePipelineHandle ... ���ɐe�Ƃ���p�C�v���C��(�n���h��)�����݂���ꍇ�Ɏw��
	�EbasePipelineIndex ... ���z����Őe�p�C�v���C���������ɍ쐬����ꍇ�A�z����ł̐e�p�C�v���C���̓Y��(�e�̓Y���̕����Ⴂ�l�ł��邱��)
	*/
	const std::array<VkGraphicsPipelineCreateInfo, 1> GPCIs = {
		{
			VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
			nullptr,
#ifdef _DEBUG
			VK_PIPELINE_CREATE_DISABLE_OPTIMIZATION_BIT,
#else
			0,
#endif
			static_cast<uint32_t>(PSSCI.size()), PSSCI.data(),
			& PVISCI,
			& PIASCI,
			& PTSCI,
			& PVSCI,
			& PRSCI,
			& PMSCI,
			& PDSSCI,
			& PCBSCI,
			& PDSCI,
			PLL,
			RP, 0, //!< �w�肵�������_�[�p�X����ł͂Ȃ��A�݊����̂��鑼�̃����_�[�p�X�ł��g�p�\
			VK_NULL_HANDLE, -1 //!< basePipelineHandle, basePipelineIndex
		}
	};
	//!< VK�ł�1�R�[���ŕ����̃p�C�v���C�����쐬���邱�Ƃ��ł��邪�ADX�ɍ��킹��1�������Ȃ����Ƃɂ��Ă���
	VERIFY_SUCCEEDED(vkCreateGraphicsPipelines(Device,
		PC,
		static_cast<uint32_t>(GPCIs.size()), GPCIs.data(),
		GetAllocationCallbacks(),
		&PL));

	LOG_OK();
}

#if 0
void VK::CreatePipeline_Compute()
{
	PERFORMANCE_COUNTER();

	std::vector<VkShaderModule> ShaderModules;
	std::vector<VkPipelineShaderStageCreateInfo> PipelineShaderStageCreateInfos;
	//CreateShader(ShaderModules, PipelineShaderStageCreateInfos);
	
	const auto PL = PipelineLayouts[0];

	const std::vector<VkComputePipelineCreateInfo> ComputePipelineCreateInfos = {
		{
			VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
			VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
			nullptr,
#ifdef _DEBUG
			VK_PIPELINE_CREATE_DISABLE_OPTIMIZATION_BIT,
#else
			0,
#endif
			PipelineShaderStageCreateInfos[0],
			PL,
			VK_NULL_HANDLE, -1 //!< basePipelineHandle, basePipelineIndex
		},
	};

	VERIFY_SUCCEEDED(vkCreateComputePipelines(Device,
		PipelineCache,
		static_cast<uint32_t>(ComputePipelineCreateInfos.size()), ComputePipelineCreateInfos.data(),
		GetAllocationCallbacks(),
		&Pipeline));

	for (auto i : ShaderModules) {
		vkDestroyShaderModule(Device, i, GetAllocationCallbacks());
	}
	ShaderModules.clear();

	LOG_OK();
}
#endif

/**
@brief �N���A Clear
@note �u�����_�[�p�X�O�v�ŃN���A Out of renderpass ... vkCmdClearColorImage()
@note �u�����_�[�p�X�J�n���v�ɃN���A Begining of renderpass ... VkAttachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR, VkRenderPassBeginInfo.pClearValues
@note �u�e�X�̃T�u�p�X�v�ŃN���A Each subpass ... vkCmdClearAttachments()
*/
//!< �u�����_�[�p�X�O�v�ɂăN���A���s��
void VK::ClearColor(const VkCommandBuffer CB, const VkImage Img, const VkClearColorValue& Color)
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
		Img,
		ImageSubresourceRange_Color
	};
	vkCmdPipelineBarrier(CB,
		VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
		0,
		0, nullptr,
		0, nullptr,
		1, &ImageMemoryBarrier_PresentToTransfer);
	{
		//!< vkCmdClearColorImage() �̓����_�[�p�X���ł͎g�p�ł��Ȃ�
		vkCmdClearColorImage(CB, Img, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &Color, 1, &ImageSubresourceRange_Color);
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
		Img,
		ImageSubresourceRange_Color
	};
	vkCmdPipelineBarrier(CB,
		VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
		0,
		0, nullptr,
		0, nullptr,
		1, &ImageMemoryBarrier_TransferToPresent);
}

//!< �񐄏�
#if 0
//!< �����p���ăf�v�X���N���A����ꍇ�́A�C���[�W�쐬���� VK_IMAGE_USAGE_TRANSFER_DST_BIT ���w�肷�邱��
void VK::ClearDepthStencil(const VkCommandBuffer CB, const VkImage Img, const VkClearDepthStencilValue& /*DepthStencil*/)
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
		Img,
		ImageSubresourceRange_DepthStencil
	};
	vkCmdPipelineBarrier(CB,
		VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
		0,
		0, nullptr,
		0, nullptr,
		1, &ImageMemoryBarrier_DepthToTransfer);
	{
		//!< vkCmdClearDepthStencilImage() �̓����_�[�p�X���ł͎g�p�ł��Ȃ�
		vkCmdClearDepthStencilImage(CB, Img, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &ClearDepthStencilValue, 1, &ImageSubresourceRange_DepthStencil);
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
		Img,
		ImageSubresourceRange_DepthStencil
	};
	vkCmdPipelineBarrier(CB,
		VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
		0,
		0, nullptr,
		0, nullptr,
		1, &ImageMemoryBarrier_TransferToDepth);
}
#endif
//!<�u�T�u�p�X�v�ɂăN���A����Ƃ��Ɏg��
//!< Draw�R�[���O�Ɏg�p����ƁA�u����Ȃ� VkAttachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR ���g���v�� Warnning ���o��̂Œ���
void VK::ClearColorAttachment(const VkCommandBuffer CB, const VkClearColorValue& Color)
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
	vkCmdClearAttachments(CB,
		static_cast<uint32_t>(ClearAttachments.size()), ClearAttachments.data(),
		static_cast<uint32_t>(ClearRects.size()), ClearRects.data());
}
void VK::ClearDepthStencilAttachment(const VkCommandBuffer CB, const VkClearDepthStencilValue& DepthStencil)
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
	vkCmdClearAttachments(CB,
		static_cast<uint32_t>(ClearAttachments.size()), ClearAttachments.data(),
		static_cast<uint32_t>(ClearRects.size()), ClearRects.data());
}

void VK::PopulateCommandBuffer(const size_t i)
{
	const auto CB = CommandBuffers[i];//CommandPools[0].second[i];
	const auto FB = Framebuffers[i];

	//!< vkBeginCommandBuffer() �ňÖٓI�Ƀ��Z�b�g����邪�A�����I�Ƀ��Z�b�g����ꍇ�ɂ́u���������v�[���փ����[�X���邩�ǂ������w��ł���v
	//VERIFY_SUCCEEDED(vkResetCommandBuffer(CB, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT));

	//!< @brief VkCommandBufferUsageFlags
	//!< * VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT		... ��x�����g�p����ꍇ�█�񃊃Z�b�g����ꍇ�Ɏw��A���x���T�u�~�b�g������̂ɂ͎w�肵�Ȃ�
	//!< * VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT		... �f�o�C�X�ł܂����s����Ă���ԂɁA�R�}���h�o�b�t�@���ēx�T�u�~�b�g����K�v������ꍇ�Ɏw�� (�p�t�H�[�}���X�̊ϓ_����͔�����ׂ�)
	//!< * VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT	... �Z�J���_���R�}���h�o�b�t�@�ł������_�[�p�X���̏ꍇ�Ɏw�肷��
	const VkCommandBufferBeginInfo CBBI = {
		VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		nullptr,
		0,
		nullptr
	};
	VERIFY_SUCCEEDED(vkBeginCommandBuffer(CB, &CBBI)); {
		//!< �r���[�|�[�g�A�V�U�[
		vkCmdSetViewport(CB, 0, static_cast<uint32_t>(Viewports.size()), Viewports.data());
		vkCmdSetScissor(CB, 0, static_cast<uint32_t>(ScissorRects.size()), ScissorRects.data());

#ifndef USE_RENDER_PASS_CLEAR
		ClearColor(CB, SwapchainImages[i], Colors::SkyBlue);
#endif

		const auto RP = RenderPasses[0];
#ifdef _DEBUG
		//!< �����_�[�G���A�̍Œᗱ�x���m��
		VkExtent2D Granularity;
		vkGetRenderAreaGranularity(Device, RP, &Granularity);
		//!<�u�����̊��ł́v Granularity = { 1, 1 } �������̂łقڂȂ�ł����v�݂����A���ɂ���Ă͒��ӂ��K�v
		assert(ScissorRects[0].extent.width >= Granularity.width && ScissorRects[0].extent.height >= Granularity.height && "ScissorRect is too small");
#endif
		//!< (�����ł�)�����_�[�p�X�J�n���ɃJ���[�̓N���A�����A�f�v�X�̓N���A���Ă��� (In this case, not clear color, but clear depth on begining of renderpas)
		std::array<VkClearValue, 2> CVs = {};
#ifdef USE_RENDER_PASS_CLEAR
		CVs[0].color = Colors::SkyBlue;
#endif
		CVs[1].depthStencil = ClearDepthStencilValue;
		const VkRenderPassBeginInfo RPBI = {
			VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
			nullptr,
			RP,
			FB,
			ScissorRects[0], //!< �t���[���o�b�t�@�̃T�C�Y�ȉ����w��ł���
			static_cast<uint32_t>(CVs.size()), CVs.data()
		};

		vkCmdBeginRenderPass(CB, &RPBI, VK_SUBPASS_CONTENTS_INLINE); {
			//vkCmdBindPipeline();
			//vkCmdBindDescriptorSets();
			//vkCmdBindVertexBuffers();
			//vkCmdBindIndexBuffer();
			//vkCmdDrawIndirect();
			//vkCmdNextSubpass(CB, VK_SUBPASS_CONTENTS_INLINE);
		} vkCmdEndRenderPass(CB);
	} VERIFY_SUCCEEDED(vkEndCommandBuffer(CB));
}

void VK::Draw()
{
#ifdef _DEBUG
	//PerformanceCounter PC(__func__);
#endif

	//!< �T�u�~�b�g�����R�}���h�̊�����҂�
	const std::array<VkFence, 1> Fences = { Fence };
	VERIFY_SUCCEEDED(vkWaitForFences(Device, static_cast<uint32_t>(Fences.size()), Fences.data(), VK_TRUE, (std::numeric_limits<uint64_t>::max)()));
	vkResetFences(Device, static_cast<uint32_t>(Fences.size()), Fences.data());

	//!< ���̃C���[�W���擾�ł���܂Ńu���b�N(�^�C���A�E�g�͎w��\)�A�擾�ł�������Ƃ����ăC���[�W�͒����ɖړI�Ɏg�p�\�Ƃ͌���Ȃ�
	//!< (�����Ŏw�肵���ꍇ)�g�p�\�ɂȂ�ƃt�F���X��Z�}�t�H���V�O�i�������
	//!< �����ł̓Z�}�t�H���w�肵�A���̃Z�}�t�H�̓T�u�~�b�g���Ɏg�p����(�T�u�~�b�g�����R�}���h���v���[���e�[�V������҂悤�Ɏw�����Ă���)
	//!< (�n���h���ł͂Ȃ�)SwapchainImages �̃C���f�b�N�X���@SwapchainImageIndex �ɕԂ� (Index(not handle) of SwapchainImages will return to SwapchainImageIndex)
	VERIFY_SUCCEEDED(vkAcquireNextImageKHR(Device, Swapchain, UINT64_MAX, NextImageAcquiredSemaphore, VK_NULL_HANDLE, &SwapchainImageIndex));
	//!< vkAcquireNextImageKHR �� VK_SUBOPTIMAL_KHR ��Ԃ����ꍇ�A�C���[�W�͎g�p�\�ł͂��邪�v���[���e�[�V�����G���W���ɂƂ��ăx�X�g�ł͂Ȃ����
	//!< vkAcquireNextImageKHR �� VK_ERROR_OUT_OF_DATE_KHR ��Ԃ����ꍇ�A�C���[�W�͎g�p�s�ōč쐬���K�v

	//!< �f�X�N���v�^�Z�b�g���X�V������A�R�}���h�o�b�t�@���L�^�������Ȃ��ƃ_���H
	//UpdateDescriptorSet();
	//UpdateDescriptorSet(SwapchainImageIndex);
	
	//!< �R�}���h�͎w��̃p�C�v���C���X�e�[�W�ɓ��B����܂Ŏ��s����A�����ŃZ�}�t�H���V�O�i�������܂ő҂�
	const std::vector<VkSemaphore> SemaphoresToWait = { NextImageAcquiredSemaphore };
	const std::vector<VkPipelineStageFlags> PipelineStagesToWait = { VK_PIPELINE_STAGE_TRANSFER_BIT };
	assert(SemaphoresToWait.size() == PipelineStagesToWait.size() && "Must be same size()");
	//!< ���s����R�}���h�o�b�t�@
	const std::vector<VkCommandBuffer> CBs = { 
		CommandBuffers[SwapchainImageIndex],
	};
	//!< �������ɃV�O�i�������Z�}�t�H(RenderFinishedSemaphore)
	const std::vector<VkSemaphore> SemaphoresToSignal = { RenderFinishedSemaphore };
	const std::vector<VkSubmitInfo> SIs = {
		{
			VK_STRUCTURE_TYPE_SUBMIT_INFO,
			nullptr,
			static_cast<uint32_t>(SemaphoresToWait.size()), SemaphoresToWait.data(), PipelineStagesToWait.data(), //!< ���C���[�W���擾�ł���(�v���[���g����)�܂ŃE�G�C�g
			static_cast<uint32_t>(CBs.size()), CBs.data(),
			static_cast<uint32_t>(SemaphoresToSignal.size()), SemaphoresToSignal.data() //!< �`�抮����ʒm����
		},
	};
	VERIFY_SUCCEEDED(vkQueueSubmit(GraphicsQueue, static_cast<uint32_t>(SIs.size()), SIs.data(), Fence));

	Present();
}
void VK::Dispatch()
{
	//!< (Fence���w�肵��)�T�u�~�b�g�����R�}���h����������܂Ńu���b�L���O���đ҂�
	const std::array<VkFence, 1> Fences = { ComputeFence };
	VERIFY_SUCCEEDED(vkWaitForFences(Device, static_cast<uint32_t>(Fences.size()), Fences.data(), VK_TRUE, (std::numeric_limits<uint64_t>::max)()));
	vkResetFences(Device, static_cast<uint32_t>(Fences.size()), Fences.data());

	const auto& CB = CommandBuffers[0];
	const std::vector<VkSubmitInfo> SIs = {
		{
			VK_STRUCTURE_TYPE_SUBMIT_INFO,
			nullptr,
			0, nullptr, nullptr,
			1, &CB/*ComputeCommandBuffers[0]*/,
			0, nullptr,
		},
	};
	VERIFY_SUCCEEDED(vkQueueSubmit(ComputeQueue, static_cast<uint32_t>(SIs.size()), SIs.data(), ComputeFence));
}
void VK::Present()
{
	//!< �����ɕ����̃v���[���g���\�����A1�̃X���b�v�`�F�C�������1�̂�
	const std::vector<VkSwapchainKHR> Swapchains = { Swapchain };
	const std::vector<uint32_t> ImageIndices = { SwapchainImageIndex };
	assert(Swapchains.size() == ImageIndices.size() && "Must be same");

	//!< �T�u�~�b�g���Ɏw�肵���Z�}�t�H(RenderFinishedSemaphore)��҂��Ă���v���[���g���s�Ȃ���
	const VkPresentInfoKHR PresentInfo = {
		VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
		nullptr,
		1, &RenderFinishedSemaphore,
		static_cast<uint32_t>(Swapchains.size()), Swapchains.data(), ImageIndices.data(),
		nullptr
	};
	VERIFY_SUCCEEDED(vkQueuePresentKHR(PresentQueue, &PresentInfo));
}
