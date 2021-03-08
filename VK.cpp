#include "VK.h"

#ifdef _WINDOWS
#pragma comment(lib, "vulkan-1.lib")
#else
// "libvulkan.so.1"
#endif

#ifdef VK_NO_PROTOYYPES
	//!< �O���[�o�����x���֐� (Global level functions)
#define VK_GLOBAL_PROC_ADDR(proc) PFN_vk ## proc VK::vk ## proc = VK_NULL_HANDLE;
#include "VKGlobalProcAddr.h"
#undef VK_GLOBAL_PROC_ADDR

	//!< �C���X�^���X���x���֐� (Instance level functions)
#define VK_INSTANCE_PROC_ADDR(proc) PFN_vk ## proc VK::vk ## proc = VK_NULL_HANDLE;
#include "VKInstanceProcAddr.h"
#undef VK_INSTANCE_PROC_ADDR

	//!< �f�o�C�X���x���֐� (Device level functions)
#define VK_DEVICE_PROC_ADDR(proc) PFN_vk ## proc VK::vk ## proc = VK_NULL_HANDLE;
#include "VKDeviceProcAddr.h"
#undef VK_DEVICE_PROC_ADDR
#define VK_DEVICE_PROC_ADDR(proc) PFN_vk ## proc VK::vk ## proc = VK_NULL_HANDLE;
#include "VKDeviceProcAddr_RayTracing.h"
#undef VK_DEVICE_PROC_ADDR
#endif //!< VK_NO_PROTOYYPES

#ifdef USE_DEBUG_REPORT
	//!< �C���X�^���X���x���֐�(Debug) (Instance level functions(Debug))
#define VK_INSTANCE_PROC_ADDR(proc) PFN_vk ## proc ## EXT VK::vk ## proc = VK_NULL_HANDLE;
#include "VKInstanceProcAddr_DebugReport.h"
#undef VK_INSTANCE_PROC_ADDR
#endif

	//!< �f�o�C�X���x���֐�(Debug) (Device level functions(Debug))
#ifdef USE_DEBUG_MARKER
#define VK_DEVICE_PROC_ADDR(proc) PFN_vk ## proc ## EXT VK::vk ## proc = VK_NULL_HANDLE;
#include "VKDeviceProcAddr_DebugMarker.h"
#undef VK_DEVICE_PROC_ADDR
#endif

#ifdef _WINDOWS
void VK::OnCreate(HWND hWnd, HINSTANCE hInstance, LPCWSTR Title)
{
	PERFORMANCE_COUNTER();

	Super::OnCreate(hWnd, hInstance, Title);

#ifdef VK_NO_PROTOYYPES
	LoadVulkanLibrary();
#endif

	//!< �C���X�^���X�A�f�o�C�X
	CreateInstance();
	CreateSurface(hWnd, hInstance);
	EnumeratePhysicalDevice(Instance);
	CreateDevice(GetCurrentPhysicalDevice(), Surface);
	
	CreateFence(Device);
	CreateSemaphore(Device);

	CreateSwapchain();
	GetSwapchainImage(Device, Swapchain);
	CreateSwapchainImageView();

	AllocateCommandBuffer();

#ifdef USE_MANUAL_CLEAR
	InitializeSwapchainImage(CommandBuffers[0], &Colors::Red);
#endif

	//!< �W�I���g�� (�o�[�e�b�N�X�o�b�t�@�A�C���f�b�N�X�o�b�t�@�A�A�N�Z�����[�V�����X�g���N�`����)
	CreateGeometry();

	//!< ���j�t�H�[���o�b�t�@ (�R���X�^���g�o�b�t�@����)
	CreateUniformBuffer();

	//!< �e�N�X�`��(�����_�[�^�[�Q�b�g�e�N�X�`���̏ꍇ�t���[���o�b�t�@�����O�ɕK�v�ɂȂ�)
	CreateTexture();

	//!< �C�~���[�^�u���T���v���͂��̎��_(CreateDescriptorSetLayout()���O)�ŕK�v
	CreateImmutableSampler();
	//!< �p�C�v���C�����C�A�E�g (���[�g�V�O�l�`������)
	CreatePipelineLayout();

	//!< �����_�[�p�X (DX�ɂ͑��݂��Ȃ�)
	CreateRenderPass();

	//!< �p�C�v���C��
	CreatePipeline();

	//!< �t���[���o�b�t�@ (DX�ɂ͑��݂��Ȃ�)
	CreateFramebuffer();

	//!< �f�X�N���v�^�Z�b�g
	CreateDescriptorSet();
	//!< �T���v�� ... DX���f�X�N���v�^��K�v�Ƃ���̂ŕt�������ł����ɂ���
	CreateSampler();

	//!< �f�X�N���v�^�Z�b�g�X�V ... ���̎��_�Ńf�X�N���v�^�Z�b�g�A���j�t�H�[���o�b�t�@�A�C���[�W�r���[�A�T���v�������K�v
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
	PERFORMANCE_COUNTER();

	Super::OnExitSizeMove(hWnd, hInstance);

	//!< �f�o�C�X���A�C�h���ɂȂ�܂ő҂�
	if (VK_NULL_HANDLE != Device) [[likely]] {
		VERIFY_SUCCEEDED(vkDeviceWaitIdle(Device));
	}

#if 0
	//!< �X���b�v�`�F�C���̍�蒼��
	CreateSwapchain(GetCurrentPhysicalDevice(), Surface, Rect);

	//!< �f�v�X�o�b�t�@�̔j���A�쐬

	//!< �t���[���o�b�t�@�̔j���A�쐬
	for (auto i : Framebuffers) {
		vkDestroyFramebuffer(Device, i, GetAllocationCallbacks());
	}
	Framebuffers.clear();
	//CreateFramebuffer();
#endif

	CreateViewport(static_cast<float>(SurfaceExtent2D.width), static_cast<float>(SurfaceExtent2D.height));

#if 0
	//!< �R�}���h�o�b�t�@�̃��Z�b�g vkBeginCommandBuffer() �ňÖٓI�Ƀ��Z�b�g�����̂ŕs�v�H
	for (auto i : SecondaryCommandBuffers) { VERIFY_SUCCEEDED(vkResetCommandBuffer(i, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT)); }
	for (auto i : CommandBuffers) { VERIFY_SUCCEEDED(vkResetCommandBuffer(i, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT)); }
#endif

	//!< �r���[�|�[�g�T�C�Y�����肵�Ă���
	LoadScene();
	for (auto i = 0; i < size(CommandBuffers); ++i) {
		PopulateCommandBuffer(i);
	}
}

void VK::OnDestroy(HWND hWnd, HINSTANCE hInstance)
{
	Super::OnDestroy(hWnd, hInstance);

	if (VK_NULL_HANDLE != Device) [[likely]] {
		//!< �f�o�C�X�̃L���[�ɃT�u�~�b�g���ꂽ�S�R�}���h����������܂Ńu���b�L���O�A��ɏI�������Ɏg�� (Wait for all command submitted to queue, usually used on finalize)
		VERIFY_SUCCEEDED(vkDeviceWaitIdle(Device));
	}

	for (auto i : Framebuffers) {
		vkDestroyFramebuffer(Device, i, GetAllocationCallbacks());
	}
	Framebuffers.clear();

	for (auto i : RenderPasses) {
		vkDestroyRenderPass(Device, i, GetAllocationCallbacks());
	}

	for (auto i : Pipelines) {
		vkDestroyPipeline(Device, i, GetAllocationCallbacks());
	}
	for (auto i : PipelineLayouts) {
		vkDestroyPipelineLayout(Device, i, GetAllocationCallbacks());
	}
	PipelineLayouts.clear();

	for (auto i : DescriptorUpdateTemplates) {
		vkDestroyDescriptorUpdateTemplate(Device, i, GetAllocationCallbacks());
	}
	DescriptorUpdateTemplates.clear();

	//!< VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT ���w�肵���ꍇ�̂݌ʂɊJ���ł��� (Only if VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT is used, can be release individually)
	//if (!empty(DescriptorSets))  [[likely]] {
	//	vkFreeDescriptorSets(Device, DescriptorPool, static_cast<uint32_t>(size(DescriptorSets)), data(DescriptorSets));
	//}
	DescriptorSets.clear();

	//!< ���̃v�[������m�ۂ��ꂽ�S�Ẵf�X�N���v�^�Z�b�g���������A�����ł͎��̃X�e�b�v�Ńv�[�����̂�j�����Ă���̂ł��Ȃ��Ă��ǂ�
	//!< (�f�X�N���v�^�Z�b�g���X�ɉ������̂��ʓ|�ȏꍇ��A�v�[�����͔̂j���������Ȃ��ꍇ�Ɏg�p)
	for (auto i : DescriptorPools) {
		vkResetDescriptorPool(Device, i, 0);
	}
	for (auto i : DescriptorPools) {
		vkDestroyDescriptorPool(Device, i, GetAllocationCallbacks());
	}
	DescriptorPools.clear();
	
	for (auto i : DescriptorSetLayouts) {
		vkDestroyDescriptorSetLayout(Device, i, GetAllocationCallbacks());
	}
	DescriptorSetLayouts.clear();

	for (auto& i : RenderTextures) { i.Destroy(Device); } RenderTextures.clear();
	for (auto& i : DepthTextures) { i.Destroy(Device); } DepthTextures.clear();
	for (auto& i : Textures) { i.Destroy(Device); } Textures.clear();
	for (auto& i : ImageViews) {
		vkDestroyImageView(Device, i, GetAllocationCallbacks());
	}
	ImageViews.clear();

	for (auto& i : Images) {
		vkDestroyImage(Device, i.Image, GetAllocationCallbacks());
		vkFreeMemory(Device, i.DeviceMemory, GetAllocationCallbacks());
	}
	Images.clear();

	for (auto& i : BufferViews) {
		vkDestroyBufferView(Device, i, GetAllocationCallbacks());
	}
	BufferViews.clear();

	for (auto i : StorageTexelBuffers) { i.Destroy(Device); } StorageTexelBuffers.clear();
	for (auto i : UniformTexelBuffers) { i.Destroy(Device); } UniformTexelBuffers.clear();
	for (auto i : StorageBuffers) { i.Destroy(Device); } StorageBuffers.clear();
	for (auto i : UniformBuffers) { i.Destroy(Device); } UniformBuffers.clear();
#pragma region RAYTRACING
	for (auto i : ShaderBindingTables) { i.Destroy(Device); } ShaderBindingTables.clear();
	for (auto i : BLASs) { i.Destroy(Device); } BLASs.clear();
	for (auto i : TLASs) { i.Destroy(Device); } TLASs.clear();
#pragma endregion
	for (auto i : IndirectBuffers) { i.Destroy(Device); } IndirectBuffers.clear();
	for (auto i : IndexBuffers) { i.Destroy(Device); } IndexBuffers.clear();
	for (auto i : VertexBuffers) { i.Destroy(Device); } VertexBuffers.clear();

	for (auto i : Samplers) {
		vkDestroySampler(Device, i, GetAllocationCallbacks());
	}

	for (auto i : SwapchainImageViews) {
		vkDestroyImageView(Device, i, GetAllocationCallbacks());
	}
	SwapchainImageViews.clear();

	//!< SwapchainImages �͎擾�������́A�j�����Ȃ�
	
	if (VK_NULL_HANDLE != Swapchain) [[likely]] {
		vkDestroySwapchainKHR(Device, Swapchain, GetAllocationCallbacks());
		Swapchain = VK_NULL_HANDLE;
	}

	//!< �R�}���h�v�[���j�����ɃR�}���h�o�b�t�@�͈ÖٓI�ɉ�������̂ł��Ȃ��Ă��ǂ� (Command buffers will be released implicitly, when command pool released)
	//if(!empty(SecondaryCommandBuffers)) [[likely]] { vkFreeCommandBuffers(Device, SecondaryCommandPools[0], static_cast<uint32_t>(size(SecondaryCommandBuffers)), data(SecondaryCommandBuffers)); SecondaryCommandBuffers.clear(); }	
	for (auto i : SecondaryCommandPools) {
		vkDestroyCommandPool(Device, i, GetAllocationCallbacks());
	}
	SecondaryCommandPools.clear();

	//!< �R�}���h�v�[���j�����ɃR�}���h�o�b�t�@�͈ÖٓI�ɉ�������̂ł��Ȃ��Ă��ǂ� (Command buffers will be released implicitly, when command pool released)
	//if(!empty(CommandBuffers)) [[likely]] { vkFreeCommandBuffers(Device, CommandPool[0], static_cast<uint32_t>(size(CommandBuffers)), data(CommandBuffers)); CommandBuffers.clear(); }
	for (auto i : CommandPools) {
		vkDestroyCommandPool(Device, i, GetAllocationCallbacks());
	}
	CommandPools.clear();

	if (VK_NULL_HANDLE != RenderFinishedSemaphore) [[likely]] {
		vkDestroySemaphore(Device, RenderFinishedSemaphore, GetAllocationCallbacks());
		RenderFinishedSemaphore = VK_NULL_HANDLE;
	}
	if (VK_NULL_HANDLE != NextImageAcquiredSemaphore) [[likely]] {
		vkDestroySemaphore(Device, NextImageAcquiredSemaphore, GetAllocationCallbacks());
		NextImageAcquiredSemaphore = VK_NULL_HANDLE;
	}
	if (VK_NULL_HANDLE != Fence) [[likely]] {
		vkDestroyFence(Device, Fence, GetAllocationCallbacks());
		Fence = VK_NULL_HANDLE;
	}
	if (VK_NULL_HANDLE != ComputeFence) [[likely]] {
		vkDestroyFence(Device, ComputeFence, GetAllocationCallbacks());
		ComputeFence = VK_NULL_HANDLE;
	}

	//!< �L���[�͘_���f�o�C�X�Ƌ��ɔj�������

	if (VK_NULL_HANDLE != Device) [[likely]] {
		vkDestroyDevice(Device, GetAllocationCallbacks());
		Device = VK_NULL_HANDLE;
	}
	
	//!< PhysicalDevice �� vkEnumeratePhysicalDevices() �Ŏ擾�������́A�j�����Ȃ�

	if (VK_NULL_HANDLE != Surface) [[likely]] {
		vkDestroySurfaceKHR(Instance, Surface, GetAllocationCallbacks());
		Surface = VK_NULL_HANDLE;
	}

#ifdef USE_DEBUG_REPORT
	if (VK_NULL_HANDLE != DebugReportCallback) [[likely]] {
		vkDestroyDebugReportCallback(Instance, DebugReportCallback, nullptr);
		DebugReportCallback = VK_NULL_HANDLE;
	}
#endif

	if (VK_NULL_HANDLE != Instance) [[likely]] {
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
		) [[likely]] {
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

const char* VK::GetImageViewTypeChar(const VkImageViewType IVT)
{
#define VK_IMAGE_VIEW_TYPE_ENTRY(ivt) case VK_IMAGE_VIEW_TYPE_##ivt: return #ivt;
	switch (IVT)
	{
	default: DEBUG_BREAK(); return "Not found";
		VK_IMAGE_VIEW_TYPE_ENTRY(1D)
		VK_IMAGE_VIEW_TYPE_ENTRY(2D)
		VK_IMAGE_VIEW_TYPE_ENTRY(3D)
		VK_IMAGE_VIEW_TYPE_ENTRY(CUBE)
		VK_IMAGE_VIEW_TYPE_ENTRY(1D_ARRAY)
		VK_IMAGE_VIEW_TYPE_ENTRY(2D_ARRAY)
		VK_IMAGE_VIEW_TYPE_ENTRY(CUBE_ARRAY)
	}
#undef VK_IMAGE_VIEW_TYPE_ENTRY
}

const char* VK::GetComponentSwizzleChar(const VkComponentSwizzle CS)
{
#define VK_COMPONENT_SWIZZLE_ENTRY(cs) case VK_COMPONENT_SWIZZLE_##cs: return #cs;
	switch (CS)
	{
	default: DEBUG_BREAK(); return "Not found";
		VK_COMPONENT_SWIZZLE_ENTRY(IDENTITY)
		VK_COMPONENT_SWIZZLE_ENTRY(ZERO)
		VK_COMPONENT_SWIZZLE_ENTRY(ONE)
		VK_COMPONENT_SWIZZLE_ENTRY(R)
		VK_COMPONENT_SWIZZLE_ENTRY(G)
		VK_COMPONENT_SWIZZLE_ENTRY(B)
		VK_COMPONENT_SWIZZLE_ENTRY(A)
	}
#undef VK_COMPONENT_SWIZZLE_ENTRY
}

const char* VK::GetSystemAllocationScopeChar(const VkSystemAllocationScope SAS)
{
#define VK_SYSTEM_ALLOCATION_SCOPE_ENTRY(sas) case VK_SYSTEM_ALLOCATION_SCOPE_##sas: return #sas;
	switch (SAS)
	{
	default: DEBUG_BREAK(); return "Not found";
		VK_SYSTEM_ALLOCATION_SCOPE_ENTRY(COMMAND)
		VK_SYSTEM_ALLOCATION_SCOPE_ENTRY(OBJECT)
		VK_SYSTEM_ALLOCATION_SCOPE_ENTRY(CACHE)
		VK_SYSTEM_ALLOCATION_SCOPE_ENTRY(DEVICE)
		VK_SYSTEM_ALLOCATION_SCOPE_ENTRY(INSTANCE)
	}
#undef VK_SYSTEM_ALLOCATION_SCOPE_ENTRY
}

bool VK::IsSupportedDepthFormat(VkPhysicalDevice PhysicalDevice, const VkFormat DepthFormat)
{
	VkFormatProperties FP;
	vkGetPhysicalDeviceFormatProperties(PhysicalDevice, DepthFormat, &FP);
	if (FP.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) {
		return true;
	}
	return false;
}

//!< @brief �������v���p�e�B�t���O��(�T�|�[�g����Ă��邩�`�F�b�N����)�������^�C�v�֕ϊ�
//!< @param �����f�o�C�X�̃������v���p�e�B
//!< @param �o�b�t�@��C���[�W�̗v�����郁�����^�C�v
//!< @param ��]�̃������v���p�e�B�t���O
//!< @return �������^�C�v
uint32_t VK::GetMemoryTypeIndex(const VkPhysicalDeviceMemoryProperties& PDMP, const uint32_t TypeBits, const VkMemoryPropertyFlags MPF)
{
	for (uint32_t i = 0; i < PDMP.memoryTypeCount; ++i) {
		if (TypeBits & (1 << i)) {
			if ((PDMP.memoryTypes[i].propertyFlags & MPF) == MPF) { //!< �w��t���O���u�S�āv�����Ă��Ȃ��ƃ_��
			//if (PDMP.memoryTypes[i].propertyFlags & MPF) {		//!< �w��t���O���u��ł��v�����Ă���Ηǂ�
				return i;
			}
		}
	}
	DEBUG_BREAK();
	return 0xffff;
}

void VK::CreateBuffer(VkBuffer* Buffer, const VkBufferUsageFlags Usage, const size_t Size) const
{
	//!< �o�b�t�@�͍쐬���Ɏw�肵���g�p�@�ł����g�p�ł��Ȃ��A�����ł� VK_SHARING_MODE_EXCLUSIVE ���ߑł��ɂ��Ă��� #VK_TODO (Using VK_SHARING_MODE_EXCLUSIVE here)
	//!< VK_SHARING_MODE_EXCLUSIVE	: �����t�@�~���̃L���[�������A�N�Z�X�ł��Ȃ��A���̃t�@�~������A�N�Z�X�������ꍇ�̓I�[�i�[�V�b�v�̈ڏ����K�v
	//!< VK_SHARING_MODE_CONCURRENT	: �����t�@�~���̃L���[�������A�N�Z�X�\�A�I�[�i�[�V�b�v�̈ڏ����K�v�����A�p�t�H�[�}���X�͈���
	const std::array<uint32_t, 0> QFI = {};
	const VkBufferCreateInfo BCI = {
		.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
		.size = Size,
		.usage = Usage,
		.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
		.queueFamilyIndexCount = static_cast<uint32_t>(size(QFI)), .pQueueFamilyIndices = data(QFI)
	};
	VERIFY_SUCCEEDED(vkCreateBuffer(Device, &BCI, GetAllocationCallbacks(), Buffer));
}

void VK::CreateImage(VkImage* Img, const VkImageCreateFlags CreateFlags, const VkImageType ImageType, const VkFormat Format, const VkExtent3D& Extent3D, const uint32_t MipLevels, const uint32_t ArrayLayers, const VkSampleCountFlagBits SampleCount, const VkImageUsageFlags Usage) const
{
	//!< Usage �� VK_IMAGE_USAGE_SAMPLED_BIT ���w�肳�ꂢ��ꍇ�A�t�H�[�}�b�g��t�B���^���g�p�\���`�F�b�N #VK_TODO �����ł̓��j�A�t�B���^���ߑł�
	ValidateFormatProperties_SampledImage(GetCurrentPhysicalDevice(), Format, Usage, VK_FILTER_LINEAR, VK_FILTER_LINEAR, VK_SAMPLER_MIPMAP_MODE_LINEAR);

	const std::array<uint32_t, 0> QueueFamilyIndices = {};
	const VkImageCreateInfo ICI = {
		.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
		.pNext = nullptr,
		.flags = CreateFlags,
		.imageType = ImageType,
		.format = Format,
		.extent = Extent3D,
		.mipLevels = MipLevels,
		.arrayLayers = ArrayLayers,
		.samples = SampleCount,
		.tiling = VK_IMAGE_TILING_OPTIMAL, //!< ���j�A�̓p�t�H�[�}���X�������̂ŁA�����ł� OPTIMAL �Ɍ��ߑł����Ă���
		.usage = Usage,
		.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
		.queueFamilyIndexCount = static_cast<uint32_t>(size(QueueFamilyIndices)), .pQueueFamilyIndices = data(QueueFamilyIndices),
		.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED //!< �쐬���Ɏw��ł���̂� UNDEFINED, PREINITIALIZED �̂݁A���ۂɎg�p����O�Ƀ��C�A�E�g��ύX����K�v������
	};

#ifdef _DEBUG
	if (ICI.flags & VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT) {
		assert(ICI.samples == VK_SAMPLE_COUNT_1_BIT && "Must be VK_SAMPLE_COUNT_1_BIT");
		assert(ICI.extent.width == ICI.extent.height && "Must be square");
		assert(ICI.arrayLayers >= 6 && "Invalid ArrayLayers");
	}
	else {
		assert(ICI.arrayLayers >= 1 && "Invalid ArrayLayers");
	}
#endif

	VERIFY_SUCCEEDED(vkCreateImage(Device, &ICI, GetAllocationCallbacks(), Img));
}

void VK::CopyToHostVisibleDeviceMemory(const VkDeviceMemory DM, const VkDeviceSize Offset, const VkDeviceSize Size, const void* Source, [[maybe_unused]]const VkDeviceSize MappedRangeOffset, [[maybe_unused]]const VkDeviceSize MappedRangeSize)
{
	if (Size && nullptr != Source) [[likely]] {
		const std::array MMRs = {
			VkMappedMemoryRange({
				.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE,
				.pNext = nullptr,
				.memory = DM,
				.offset = MappedRangeOffset,
				.size = MappedRangeSize
			})
		};
		void* Data;
		VERIFY_SUCCEEDED(vkMapMemory(Device, DM, Offset, Size, static_cast<VkMemoryMapFlags>(0), &Data)); {
			memcpy(Data, Source, Size);
			//!< �������R���e���c���ύX���ꂽ���Ƃ��h���C�o�֒m�点��(vkMapMemory()������Ԃł�邱��)
			//!< �f�o�C�X�������m�ێ��� VK_MEMORY_PROPERTY_HOST_COHERENT_BIT ���w�肵���ꍇ�͕K�v�Ȃ� CreateDeviceMemory(..., VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
			VERIFY_SUCCEEDED(vkFlushMappedMemoryRanges(Device, static_cast<uint32_t>(size(MMRs)), data(MMRs)));
			//VERIFY_SUCCEEDED(vkInvalidateMappedMemoryRanges(Device, static_cast<uint32_t>(size(MMRs)), data(MMRs)));
		} vkUnmapMemory(Device, DM);
	}
}

void VK::PopulateCommandBuffer_ClearColor(const VkCommandBuffer CB, const VkImage Img, const VkClearColorValue& Color)
{
	constexpr VkImageSubresourceRange ISR = {
		.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
		.baseMipLevel = 0, .levelCount = VK_REMAINING_MIP_LEVELS,
		.baseArrayLayer = 0, .layerCount = VK_REMAINING_ARRAY_LAYERS
	};
	constexpr std::array<VkMemoryBarrier, 0> MBs = {};
	constexpr std::array<VkBufferMemoryBarrier, 0> BMBs = {};
	{
		const std::array IMBs = {
			VkImageMemoryBarrier({
				.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
				.pNext = nullptr,
				.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT, .dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
				.oldLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, .newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED, .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
				.image = Img,
				.subresourceRange = ISR
			}),
		};
		vkCmdPipelineBarrier(CB, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
			static_cast<uint32_t>(size(MBs)), data(MBs),
			static_cast<uint32_t>(size(BMBs)), data(BMBs),
			static_cast<uint32_t>(size(IMBs)), data(IMBs));
	}
	//!< vkCmdClearColorImage() �̓����_�[�p�X���ł͎g�p�ł��Ȃ�
	constexpr std::array ISRs = { ISR };
	vkCmdClearColorImage(CB, Img, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &Color, static_cast<uint32_t>(size(ISRs)), data(ISRs));
	{
		const std::array IMBs = {
			VkImageMemoryBarrier({
				.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
				.pNext = nullptr,
				.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT, .dstAccessMask = VK_ACCESS_MEMORY_READ_BIT,
				.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, .newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
				.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED, .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
				.image = Img,
				.subresourceRange = ISR
			}),
		};
		vkCmdPipelineBarrier(CB, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
			static_cast<uint32_t>(size(MBs)), data(MBs),
			static_cast<uint32_t>(size(BMBs)), data(BMBs),
			static_cast<uint32_t>(size(IMBs)), data(IMBs));
	}
}

//!< @param �R�}���h�o�b�t�@
//!< @param �R�s�[���o�b�t�@
//!< @param �R�s�[��o�b�t�@
//!< @param (�R�s�[���)�o�b�t�@�̃A�N�Z�X�t���O ex) VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT, VK_ACCESS_INDEX_READ_BIT, VK_ACCESS_INDIRECT_READ_BIT,...��
//!< @param (�R�s�[���)�o�b�t�@���g����p�C�v���C���X�e�[�W ex) VK_PIPELINE_STAGE_VERTEX_INPUT_BIT, VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT,...��
void VK::PopulateCommandBuffer_CopyBufferToBuffer(const VkCommandBuffer CB, const VkBuffer Src, const VkBuffer Dst, const VkAccessFlags AF, const VkPipelineStageFlagBits PSF, const size_t Size)
{
	constexpr std::array<VkMemoryBarrier, 0> MBs = {};
	constexpr std::array<VkImageMemoryBarrier, 0> IMBs = {};
	{
		const std::array BMBs = {
			VkBufferMemoryBarrier({
				.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
				.pNext = nullptr,
				.srcAccessMask = 0, .dstAccessMask = VK_ACCESS_MEMORY_WRITE_BIT,
				.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED, .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
				.buffer = Dst, .offset = 0, .size = VK_WHOLE_SIZE
			}),
		};
		vkCmdPipelineBarrier(CB, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
			static_cast<uint32_t>(size(MBs)), data(MBs),
			static_cast<uint32_t>(size(BMBs)), data(BMBs),
			static_cast<uint32_t>(size(IMBs)), data(IMBs));
	}
	const std::array BCs = { VkBufferCopy({.srcOffset = 0, .dstOffset = 0, .size = Size }), };
	vkCmdCopyBuffer(CB, Src, Dst, static_cast<uint32_t>(size(BCs)), data(BCs));
	{
		const std::array BMBs = {
			VkBufferMemoryBarrier({
				.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
				.pNext = nullptr,
				.srcAccessMask = VK_ACCESS_MEMORY_WRITE_BIT, .dstAccessMask = AF,
				.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED, .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
				.buffer = Dst, .offset = 0, .size = VK_WHOLE_SIZE
			}),
		};
		vkCmdPipelineBarrier(CB, VK_PIPELINE_STAGE_TRANSFER_BIT, PSF, 0,
			static_cast<uint32_t>(size(MBs)), data(MBs),
			static_cast<uint32_t>(size(BMBs)), data(BMBs),
			static_cast<uint32_t>(size(IMBs)), data(IMBs));
	}
}
void VK::PopulateCommandBuffer_CopyBufferToImage(const VkCommandBuffer CB, const VkBuffer Src, const VkImage Dst, const VkAccessFlags AF, const VkImageLayout IL, const VkPipelineStageFlags PSF, const std::vector<VkBufferImageCopy>& BICs, const uint32_t Levels, const uint32_t Layers)
{	
	constexpr std::array<VkMemoryBarrier, 0> MBs = {};
	constexpr std::array<VkBufferMemoryBarrier, 0> BMBs = {};
	assert(!empty(BICs) && "BufferImageCopy is empty");
	const VkImageSubresourceRange ISR = {
		.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
		.baseMipLevel = 0, .levelCount = Levels,
		.baseArrayLayer = 0, .layerCount = Layers
	};
	{
		const std::array IMBs = {
			VkImageMemoryBarrier({
				.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
				.pNext = nullptr,
				.srcAccessMask = 0, .dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
				.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED, .newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED, .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
				.image = Dst,
				.subresourceRange = ISR
			})
		};
		vkCmdPipelineBarrier(CB, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
			static_cast<uint32_t>(size(MBs)), data(MBs),
			static_cast<uint32_t>(size(BMBs)), data(BMBs),
			static_cast<uint32_t>(size(IMBs)), data(IMBs));
	}
	vkCmdCopyBufferToImage(CB, Src, Dst, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, static_cast<uint32_t>(size(BICs)), data(BICs));
	{
		const std::array IMBs = {
			VkImageMemoryBarrier({
				.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
				.pNext = nullptr,
				.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT, .dstAccessMask = AF,
				.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, .newLayout = IL,
				.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED, .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
				.image = Dst,
				.subresourceRange = ISR
			})
		};
		vkCmdPipelineBarrier(CB, VK_PIPELINE_STAGE_TRANSFER_BIT, PSF, 0,
			static_cast<uint32_t>(size(MBs)), data(MBs),
			static_cast<uint32_t>(size(BMBs)), data(BMBs),
			static_cast<uint32_t>(size(IMBs)), data(IMBs));
	}
}

void VK::PopulateCommandBuffer_CopyImageToBuffer(const VkCommandBuffer CB, const VkImage Src, const VkBuffer Dst, const VkAccessFlags AF, const VkImageLayout IL, const VkPipelineStageFlags PSF, const std::vector<VkBufferImageCopy>& BICs, const uint32_t Levels, const uint32_t Layers)
{
	//!< �R�}���h�J�n (Begin command)
	const VkCommandBufferBeginInfo CBBI = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		.pNext = nullptr,
		.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
		.pInheritanceInfo = nullptr
	};
	VERIFY_SUCCEEDED(vkBeginCommandBuffer(CB, &CBBI)); {
		assert(!empty(BICs) && "BufferImageCopy is empty");
		const VkImageSubresourceRange ISR = {
			.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
			.baseMipLevel = 0, .levelCount = Levels,
			.baseArrayLayer = 0, .layerCount = Layers
		};
		//!< �C���[�W�������o���A (Image memory barrier)
		{
			const std::array IMBs = {
				VkImageMemoryBarrier({
					.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
					.pNext = nullptr,
					.srcAccessMask = 0, .dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT,
					.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED, .newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
					.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED, .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
					.image = Src,
					.subresourceRange = ISR})
			};
			vkCmdPipelineBarrier(CB,
				VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
				0,
				0, nullptr,
				0, nullptr,
				static_cast<uint32_t>(size(IMBs)), data(IMBs));
		}
		{
			//!< �C���[�W�o�b�t�@�ԃR�s�[�R�}���h (Image to buffer copy command)
			vkCmdCopyImageToBuffer(CB, Src, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, Dst, static_cast<uint32_t>(size(BICs)), data(BICs));
		}
		//!< �C���[�W�������o���A (Image memory barrier)
		{
			const std::array IMBs = {
				VkImageMemoryBarrier({
					.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
					.pNext = nullptr,
					.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT, .dstAccessMask = AF,
					.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, .newLayout = IL,
					.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED, .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
					.image = Src,
					.subresourceRange = ISR})
			};
			vkCmdPipelineBarrier(CB,
				VK_PIPELINE_STAGE_TRANSFER_BIT, PSF,
				0,
				0, nullptr,
				0, nullptr,
				static_cast<uint32_t>(size(IMBs)), data(IMBs));
		}
	} VERIFY_SUCCEEDED(vkEndCommandBuffer(CB));
}

void VK::SubmitAndWait(const VkQueue Queue, const VkCommandBuffer CB)
{
	const std::array<VkSemaphore, 0> WaitSems = {};
	const std::array<VkPipelineStageFlags, 0> StageFlags = {};
	const std::array CBs = { CB };
	const std::array<VkSemaphore, 0> SignalSems = {};
	const std::array SIs = {
		VkSubmitInfo({
			.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
			.pNext = nullptr,
			.waitSemaphoreCount = static_cast<uint32_t>(size(WaitSems)), .pWaitSemaphores = data(WaitSems), .pWaitDstStageMask = data(StageFlags),
			.commandBufferCount = static_cast<uint32_t>(size(CBs)), .pCommandBuffers = data(CBs),
			.signalSemaphoreCount = static_cast<uint32_t>(size(SignalSems)), .pSignalSemaphores = data(SignalSems)
		})
	};
	VERIFY_SUCCEEDED(vkQueueSubmit(Queue, static_cast<uint32_t>(size(SIs)), data(SIs), VK_NULL_HANDLE));
	//!< �L���[�ɃT�u�~�b�g���ꂽ�R�}���h����������܂Ńu���b�L���O (�t�F���X��p���Ȃ��u���b�L���O���@)
	VERIFY_SUCCEEDED(vkQueueWaitIdle(Queue));
}

void VK::EnumerateMemoryRequirements(const VkMemoryRequirements& MR, const VkPhysicalDeviceMemoryProperties& PDMP)
{
//	const auto& PDMP = GetCurrentPhysicalDeviceMemoryProperties();
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

//!< Cubemap�����ꍇ�A�܂����C�������ꂽ�C���[�W���쐬���A(�C���[�W�r���[��p����)���C�����t�F�C�X�Ƃ��Ĉ����悤�n�[�h�E�G�A�ɓ`����
void VK::CreateImageView(VkImageView* IV, const VkImage Img, const VkImageViewType ImageViewType, const VkFormat Format, const VkComponentMapping& ComponentMapping, const VkImageSubresourceRange& ImageSubresourceRange)
{
	const VkImageViewCreateInfo IVCI = {
		.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
		.image = Img,
		.viewType = ImageViewType,
		.format = Format,
		.components = ComponentMapping,
		.subresourceRange = ImageSubresourceRange
	};
	VERIFY_SUCCEEDED(vkCreateImageView(Device, &IVCI, GetAllocationCallbacks(), IV));
	Logf("\t\tImageViewType = %s\n", GetImageViewTypeChar(ImageViewType));
	Logf("\t\tFormat = %s\n", GetFormatChar(Format));
	Logf("\t\tComponentMapping = (%s)\n", data(GetComponentMappingString(ComponentMapping)));

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
	if (Count) [[likely]] {
		std::vector<VkLayerProperties> LPs(Count);
		VERIFY_SUCCEEDED(vkEnumerateInstanceLayerProperties(&Count, data(LPs)));
		for (const auto& i : LPs) {
			Logf("\t\"%s\" (%s)\n", i.layerName, i.description);
			EnumerateInstanceExtensionProperties(i.layerName);
		}
	}
}
void VK::EnumerateInstanceExtensionProperties(const char* LayerName)
{
	uint32_t Count = 0;
	VERIFY_SUCCEEDED(vkEnumerateInstanceExtensionProperties(LayerName, &Count, nullptr));
	if (Count) [[likely]] {
		std::vector<VkExtensionProperties> ExtensionProp(Count);
		VERIFY_SUCCEEDED(vkEnumerateInstanceExtensionProperties(LayerName, &Count, data(ExtensionProp)));
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
#endif

void VK::CreateInstance()
{
	//!< �����ł͍ŐV�o�[�W�����œ����悤�ɂ��Ă��� (Use latest version here)
	uint32_t APIVersion/*= VK_API_VERSION_1_2*/;
	VERIFY_SUCCEEDED(vkEnumerateInstanceVersion(&APIVersion));
	Logf("API Version = %d.%d.(Header = %d)(Patch = %d)\n", VK_VERSION_MAJOR(APIVersion), VK_VERSION_MINOR(APIVersion), VK_HEADER_VERSION, VK_VERSION_PATCH(APIVersion));

	const auto ApplicationName = GetTitle();
	const VkApplicationInfo AI = {
		.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
		.pNext = nullptr,
		.pApplicationName = data(ApplicationName), .applicationVersion = APIVersion,
		.pEngineName = "VKDX Engine Name", .engineVersion = APIVersion,
		.apiVersion = APIVersion
	};
	
	//!< �C���X�^���X���x���̃��C���[�A�G�N�X�e���V�����̗�
	EnumerateInstanceLayerProperties();

	const std::array Layers = {
		"VK_LAYER_KHRONOS_validation", //!< "VK_LAYER_LUNARG_standard_validation" �͔񐄏��ƂȂ��� (is deprecated)
		//"VK_LAYER_LUNARG_api_dump",
#ifdef USE_RENDERDOC
		"VK_LAYER_RENDERDOC_Capture",
#endif
		"VK_LAYER_LUNARG_monitor", //!< �^�C�g���o�[��FPS��\��
	};
	const std::array Extensions = {
#ifdef _DEBUG
		VK_EXT_VALIDATION_FEATURES_EXTENSION_NAME,
#endif
		VK_KHR_SURFACE_EXTENSION_NAME,
#ifdef VK_USE_PLATFORM_WIN32_KHR
		VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
#elif defined(VK_USE_PLATFORM_XLIB_KHR)
		VK_KHR_XLIB_SURFACE_EXTENSION_NAME,
#else
		VK_KHR_XCB_SURFACE_EXTENSION_NAME,
#endif
#ifdef USE_DEBUG_REPORT
		//!< �f�o�b�O���|�[�g�p
		VK_EXT_DEBUG_REPORT_EXTENSION_NAME,
#endif
#ifdef USE_RENDERDOC
		VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
#endif
	};
	const VkInstanceCreateInfo ICI = {
	 	.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
		.pApplicationInfo = &AI,
		.enabledLayerCount = static_cast<uint32_t>(size(Layers)), .ppEnabledLayerNames = data(Layers),
		.enabledExtensionCount = static_cast<uint32_t>(size(Extensions)), .ppEnabledExtensionNames = data(Extensions)
	};
	VERIFY_SUCCEEDED(vkCreateInstance(&ICI, GetAllocationCallbacks(), &Instance));

	//!< �C���X�^���X���x���̊֐������[�h���� Load instance level functions
#ifdef VK_NO_PROTOYYPES
#define VK_INSTANCE_PROC_ADDR(proc) vk ## proc = reinterpret_cast<PFN_vk ## proc>(vkGetInstanceProcAddr(Instance, "vk" #proc)); assert(nullptr != vk ## proc && #proc);
#include "VKInstanceProcAddr.h"
#undef VK_INSTANCE_PROC_ADDR

#ifdef USE_DEBUG_REPORT
#define VK_INSTANCE_PROC_ADDR(proc) vk ## proc = reinterpret_cast<PFN_vk ## proc ## EXT>(vkGetInstanceProcAddr(Instance, "vk" #proc "EXT")); assert(nullptr != vk ## proc && #proc);
#include "VKInstanceProcAddr_DebugReport.h"
#undef VK_INSTANCE_PROC_ADDR
#endif //!< USE_DEBUG_REPORT

#endif //!< VK_NO_PROTOYYPES

#ifdef USE_DEBUG_REPORT
	CreateDebugReportCallback();
#endif

	LOG_OK();
}

#ifdef USE_DEBUG_REPORT
void VK::CreateDebugReportCallback()
{
	const auto Flags = VK_DEBUG_REPORT_INFORMATION_BIT_EXT
		| VK_DEBUG_REPORT_WARNING_BIT_EXT
		| VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT
		| VK_DEBUG_REPORT_ERROR_BIT_EXT
		| VK_DEBUG_REPORT_DEBUG_BIT_EXT;

	if (VK_NULL_HANDLE != vkCreateDebugReportCallback) [[likely]] {
		const VkDebugReportCallbackCreateInfoEXT DebugReportCallbackCreateInfo = {
			.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT,
			.pNext = nullptr,
			.flags = Flags,
			.pfnCallback = [](VkDebugReportFlagsEXT flags, [[maybe_unused]] VkDebugReportObjectTypeEXT objectType, [[maybe_unused]] uint64_t object, [[maybe_unused]] size_t location, [[maybe_unused]] int32_t messageCode, [[maybe_unused]] const char* pLayerPrefix, const char* pMessage, [[maybe_unused]] void* pUserData) -> VkBool32 {
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
			.pUserData = nullptr
		};
		vkCreateDebugReportCallback(Instance, &DebugReportCallbackCreateInfo, nullptr, &DebugReportCallback);
	}
}
#endif

void VK::MarkerInsert([[maybe_unused]] VkCommandBuffer CB, [[maybe_unused]] const glm::vec4& Color, [[maybe_unused]] const char* Name) 
{
#ifdef USE_DEBUG_MARKER
	if (VK_NULL_HANDLE != vkCmdDebugMarkerInsert) {
		VkDebugMarkerMarkerInfoEXT DMMI = {
			.sType = VK_STRUCTURE_TYPE_DEBUG_MARKER_MARKER_INFO_EXT,
			.pNext = nullptr,
			.pMarkerName = Name,
			.color = { Color.r, Color.g, Color.b, Color.a }
		};
		vkCmdDebugMarkerInsert(CB, &DMMI);
	}
#endif
}

void VK::MarkerBegin([[maybe_unused]] VkCommandBuffer CB, [[maybe_unused]] const glm::vec4& Color, [[maybe_unused]] const char* Name)
{
#ifdef USE_DEBUG_MARKER
	if (VK_NULL_HANDLE != vkCmdDebugMarkerBegin) {
		VkDebugMarkerMarkerInfoEXT DMMI = {
			.sType = VK_STRUCTURE_TYPE_DEBUG_MARKER_MARKER_INFO_EXT,
			.pNext = nullptr,
			.pMarkerName = Name,
			.color = { Color.r, Color.g, Color.b, Color.a }
		};
		vkCmdDebugMarkerBegin(CB, &DMMI);
	}
#endif
}
void VK::MarkerEnd([[maybe_unused]] VkCommandBuffer CB)
{
#ifdef USE_DEBUG_MARKER
	if (VK_NULL_HANDLE != vkCmdDebugMarkerEnd) {
		vkCmdDebugMarkerEnd(CB);
	}
#endif
}
void VK::MarkerSetName([[maybe_unused]] VkDevice Device, [[maybe_unused]] const VkDebugReportObjectTypeEXT Type, [[maybe_unused]] const uint64_t Object, [[maybe_unused]] const char* Name)
{
#ifdef USE_DEBUG_MARKER
	if (VK_NULL_HANDLE != vkDebugMarkerSetObjectName) {
		VkDebugMarkerObjectNameInfoEXT DMONI = {
			.sType = VK_STRUCTURE_TYPE_DEBUG_MARKER_OBJECT_NAME_INFO_EXT,
			.pNext = nullptr,
			.objectType = Type, .object = Object, .pObjectName = Name
		};
		VERIFY_SUCCEEDED(vkDebugMarkerSetObjectName(Device, &DMONI));
	}
#endif
}
void VK::MarkerSetTag([[maybe_unused]] VkDevice Device, [[maybe_unused]] const VkDebugReportObjectTypeEXT Type, [[maybe_unused]] const uint64_t Object, [[maybe_unused]] const uint64_t TagName, [[maybe_unused]] const size_t TagSize, [[maybe_unused]] const void* TagData)
{
#ifdef USE_DEBUG_MARKER
	if (VK_NULL_HANDLE != vkDebugMarkerSetObjectTag) {
		VkDebugMarkerObjectTagInfoEXT DMOTI = {
			.sType = VK_STRUCTURE_TYPE_DEBUG_MARKER_OBJECT_TAG_INFO_EXT,
			.pNext = nullptr,
			.objectType = Type, .object = Object,
			.tagName = TagName, .tagSize = TagSize, .pTag = TagData
		};
		VERIFY_SUCCEEDED(vkDebugMarkerSetObjectTag(Device, &DMOTI));
	}
#endif
}

#ifdef VK_USE_PLATFORM_WIN32_KHR
void VK::CreateSurface(HWND hWnd, HINSTANCE hInstance)
{
	const VkWin32SurfaceCreateInfoKHR SCI = {
		.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
		.pNext = nullptr,
		.flags = 0,
		.hinstance = hInstance,
		.hwnd = hWnd
	};
	VERIFY_SUCCEEDED(vkCreateWin32SurfaceKHR(Instance, &SCI, GetAllocationCallbacks(), &Surface));

	LOG_OK();
}
#endif
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
	PROPERTY_LIMITS_ENTRY(maxViewports);
#undef PROPERTY_LIMITS_ENTRY
}
void VK::EnumeratePhysicalDeviceFeatures(const VkPhysicalDeviceFeatures& PDF)
{
	assert(PDF.tessellationShader && "tessellationShader is not supported");

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
	VERIFY_SUCCEEDED(vkEnumeratePhysicalDevices(Inst, &Count, data(PhysicalDevices)));

	Log("\tPhysicalDevices\n");
	for (const auto& i : PhysicalDevices) {
		//!< �v���p�e�B (Property)
		VkPhysicalDeviceProperties PDP;
		vkGetPhysicalDeviceProperties(i, &PDP);
		EnumeratePhysicalDeviceProperties(PDP);

#if 0
		//!< �t�B�[�`���[ (Feature)
		VkPhysicalDeviceFeatures PDF;
		vkGetPhysicalDeviceFeatures(i, &PDF);
		EnumeratePhysicalDeviceFeatures(PDF);
#else
		//!< �t�B�[�`���[2 (Feature2)
		{
			//!< �擾�������S�Ẵt�B�[�`���[�� VkPhysicalDeviceFeatures2.pNext �փ`�F�C���w�肷��
			VkPhysicalDeviceBufferDeviceAddressFeatures PDBDAF = { .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES, .pNext = nullptr };
			VkPhysicalDeviceRayTracingPipelineFeaturesKHR PDRTPF = { .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR, .pNext = &PDBDAF };
			VkPhysicalDeviceAccelerationStructureFeaturesKHR PDASF = { .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR, .pNext = &PDRTPF };
			VkPhysicalDeviceFeatures2 PDF2 = { .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2, .pNext = &PDASF };
			vkGetPhysicalDeviceFeatures2(i, &PDF2);
			EnumeratePhysicalDeviceFeatures(PDF2.features);
			Log("\t\t\tVkPhysicalDeviceBufferDeviceAddressFeatures\n");
			if (PDBDAF.bufferDeviceAddress) { Log("\t\t\t\tbufferDeviceAddress\n"); }
			if (PDBDAF.bufferDeviceAddressCaptureReplay) { Log("\t\t\t\tbufferDeviceAddressCaptureReplay\n"); }
			if (PDBDAF.bufferDeviceAddressMultiDevice) { Log("\t\t\t\tbufferDeviceAddressMultiDevice\n"); }
			
			Log("\t\t\tVkPhysicalDeviceRayTracingPipelineFeaturesKHR\n");
			if (PDRTPF.rayTracingPipeline) { Log("\t\t\t\trayTracingPipeline\n"); }
			if (PDRTPF.rayTracingPipelineShaderGroupHandleCaptureReplay) { Log("\t\t\t\trayTracingPipelineShaderGroupHandleCaptureReplay\n"); }
			if (PDRTPF.rayTracingPipelineShaderGroupHandleCaptureReplayMixed) { Log("\t\t\t\trayTracingPipelineShaderGroupHandleCaptureReplayMixed\n"); }
			if (PDRTPF.rayTracingPipelineTraceRaysIndirect) { Log("\t\t\t\trayTracingPipelineTraceRaysIndirect\n"); }
			if (PDRTPF.rayTraversalPrimitiveCulling) { Log("\t\t\t\trayTraversalPrimitiveCulling\n"); }
			
			Log("\t\t\tVkPhysicalDeviceAccelerationStructureFeaturesKHR\n");
			if (PDASF.accelerationStructure) { Log("\t\t\t\taccelerationStructure\n"); }
			if (PDASF.accelerationStructureCaptureReplay) { Log("\t\t\t\taccelerationStructureCaptureReplay\n"); }
			if (PDASF.accelerationStructureIndirectBuild) { Log("\t\t\t\taccelerationStructureIndirectBuild\n"); }
			if (PDASF.accelerationStructureHostCommands) { Log("\t\t\t\taccelerationStructureHostCommands\n"); }
			if (PDASF.descriptorBindingAccelerationStructureUpdateAfterBind) { Log("\t\t\t\tdescriptorBindingAccelerationStructureUpdateAfterBind\n"); }
		}
#endif

		//!< �������v���p�e�B (MemoryProperty)
		VkPhysicalDeviceMemoryProperties PDMP;
		vkGetPhysicalDeviceMemoryProperties(i, &PDMP);
		EnumeratePhysicalDeviceMemoryProperties(PDMP);

		//!< �f�o�C�X���x���̃��C���[�A�G�N�X�e���V�����̗�
		EnumeratePhysicalDeviceLayerProperties(i);

		Log("\n");
	}

	PhysicalDeviceProperties.resize(size(PhysicalDevices));
	for (size_t i = 0; i < size(PhysicalDevices); ++i) {
		vkGetPhysicalDeviceMemoryProperties(PhysicalDevices[i], &PhysicalDeviceProperties[i]);
	}

	//!< �����ł͍ŏ��̕����f�o�C�X��I�����邱�Ƃɂ��� (Use first device here) #VK_TODO
	CurrentPhysicalDevice = PhysicalDevices[0];
	CurrentPhysicalDeviceMemoryProperties = PhysicalDeviceProperties[0];
}
void VK::EnumeratePhysicalDeviceLayerProperties(VkPhysicalDevice PD)
{
	Logf("Device Layer Properties\n");

	uint32_t Count = 0;
	VERIFY_SUCCEEDED(vkEnumerateDeviceLayerProperties(PD, &Count, nullptr));
	if (Count) {
		std::vector<VkLayerProperties> LPs(Count);
		VERIFY_SUCCEEDED(vkEnumerateDeviceLayerProperties(PD, &Count, data(LPs)));
		for (const auto& i : LPs) {
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
		std::vector<VkExtensionProperties> EPs(Count);
		VERIFY_SUCCEEDED(vkEnumerateDeviceExtensionProperties(PD, LayerName, &Count, data(EPs)));
		for (const auto& i : EPs) {
			Logf("\t\t\"%s\"\n", i.extensionName);
		}
	}
}

//void VK::OverridePhysicalDeviceFeatures(VkPhysicalDeviceFeatures& PDF) const
//{
//	//!< VkPhysicalDeviceFeatures �ɂ͉\�ȃt�B�[�`���[���S�� true �ɂȂ������̂��n����Ă���
//	//!< �s�v�ȍ��ڂ� false �ɃI�[�o�[���C�h����ƃp�t�H�[�}���X�̉��P�����҂ł��邩������Ȃ�
//	//!< false �œn����Ă��鍀�ڂ͎g���Ȃ��t�B�[�`���[�Ȃ̂� true �ɕς��Ă��g���Ȃ�
//
//	Log("\tPhysicalDeviceFeatures (Override)\n");
//#define VK_DEVICEFEATURE_ENTRY(entry) if(PDF.entry) { Logf("\t\t%s\n", #entry); }
//#include "VKDeviceFeature.h"
//#undef VK_DEVICEFEATURE_ENTRY
//}

uint32_t VK::FindQueueFamilyPropertyIndex(const VkQueueFlags QF, const std::vector<VkQueueFamilyProperties>& QFPs)
{
	for (auto i = 0; i < size(QFPs); ++i) {
		if (QF & QFPs[i].queueFlags) {
			return i;
		}
	}
	assert(false && "not found");
	return 0xffff;
}
uint32_t VK::FindQueueFamilyPropertyIndex(const VkPhysicalDevice PD, const VkSurfaceKHR Sfc, const std::vector<VkQueueFamilyProperties>& QFPs)
{
	for (auto i = 0; i < size(QFPs); ++i) {
		VkBool32 b = VK_FALSE;
		VERIFY_SUCCEEDED(vkGetPhysicalDeviceSurfaceSupportKHR(PD, i, Sfc, &b));
		if (b) {
			return i;
		}
	}
	assert(false && "not found");
	return 0xffff;
}

void VK::CreateDevice(VkPhysicalDevice PD, VkSurfaceKHR Sfc)
{
	std::vector<VkQueueFamilyProperties> QFPs;

	//!< �L���[�t�@�~���v���p�e�B�̗�
	uint32_t Count = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(PD, &Count, nullptr);
	assert(Count && "QueueFamilyProperty not found");
	QFPs.resize(Count);
	vkGetPhysicalDeviceQueueFamilyProperties(PD, &Count, data(QFPs));

	Log("\tQueueFamilyProperties\n");
#define QUEUE_FLAG_ENTRY(entry) if(VK_QUEUE_##entry##_BIT & QFPs[i].queueFlags) { Logf("%s | ", #entry); }
	//!< �C���f�b�N�X���g���̂Ŕ͈̓x�[�Xfor�ɂ͂��Ȃ�
	for (uint32_t i = 0; i < size(QFPs); ++i) {
		Logf("\t\t[%d] QueueCount = %d, ", i, QFPs[i].queueCount);
		Log("QueueFlags = ");
		QUEUE_FLAG_ENTRY(GRAPHICS);
		QUEUE_FLAG_ENTRY(COMPUTE);
		QUEUE_FLAG_ENTRY(TRANSFER);
		QUEUE_FLAG_ENTRY(SPARSE_BINDING);
		QUEUE_FLAG_ENTRY(PROTECTED);
		Log("\n");
		//QFPs[i].timestampValidBits;
		//QFPs[i].minImageTransferGranularity;

		VkBool32 b = VK_FALSE;
		VERIFY_SUCCEEDED(vkGetPhysicalDeviceSurfaceSupportKHR(PD, i, Sfc, &b));
		if (b) { Logf("\t\t\t\tSurface(Present) Supported\n"); }
	}
#undef QUEUE_FLAG_ENTRY

	//!< �@�\�����L���[�t�@�~���C���f�b�N�X�������� (Find queue family index for each functions)
	GraphicsQueueFamilyIndex = FindQueueFamilyPropertyIndex(VK_QUEUE_GRAPHICS_BIT, QFPs);
	PresentQueueFamilyIndex = FindQueueFamilyPropertyIndex(PD, Sfc, QFPs);
	ComputeQueueFamilyIndex = FindQueueFamilyPropertyIndex(VK_QUEUE_COMPUTE_BIT, QFPs);

	//!< �L���[�t�@�~�����ł̃C���f�b�N�X�y�уv���C�I���e�B�A�����ł̓O���t�B�b�N�A�v���[���g�A�R���s���[�g�̕����v���C�I���e�B0.5f�Œǉ����Ă���
	std::vector<std::vector<float>> Priorites(size(QFPs));
	const uint32_t GraphicsQueueIndexInFamily = static_cast<uint32_t>(size(Priorites[GraphicsQueueFamilyIndex])); Priorites[GraphicsQueueFamilyIndex].emplace_back(0.5f);
	const uint32_t PresentQueueIndexInFamily = static_cast<uint32_t>(size(Priorites[PresentQueueFamilyIndex])); Priorites[PresentQueueFamilyIndex].emplace_back(0.5f);
	const uint32_t ComputeQueueIndexInFamily = static_cast<uint32_t>(size(Priorites[ComputeQueueFamilyIndex])); Priorites[ComputeQueueFamilyIndex].emplace_back(0.5f);
	Log("\n");
	Logf("\t\tGraphics QueueFamilyIndex = %d, QueueIndex = %d\n", GraphicsQueueFamilyIndex, GraphicsQueueIndexInFamily);
	Logf("\t\tPresent QueueFamilyIndex = %d, QueueIndex = %d\n", PresentQueueFamilyIndex, PresentQueueIndexInFamily);
	Logf("\t\tCompute QueueFamilyIndex = %d, QueueIndex = %d\n", ComputeQueueFamilyIndex, ComputeQueueIndexInFamily);
	Log("\t\tPriorites = \n");
	for (const auto& i : Priorites) {
		Log("\t\t\t");
		for (const auto j : i) {
			Logf("%f, ", j);
		}
		Log("\n");
	}

	//!< �L���[�쐬��� (Queue create information)
	std::vector<VkDeviceQueueCreateInfo> DQCIs;
	//!< �C���f�b�N�X���g���̂Ŕ͈̓x�[�Xfor�ɂ͂��Ȃ�
	for (size_t i = 0; i < size(Priorites);++i) {
		if (!empty(Priorites[i])) {
			DQCIs.emplace_back(
				VkDeviceQueueCreateInfo({
					.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
					.pNext = nullptr,
					.flags = 0,
					.queueFamilyIndex = static_cast<uint32_t>(i),
					.queueCount = static_cast<uint32_t>(size(Priorites[i])), .pQueuePriorities = data(Priorites[i])
				})
			);
		}
	}	

	std::vector Extensions = {
		//!< �X���b�v�`�F�C���̓v���b�g�t�H�[���ɓ��L�̋@�\�Ȃ̂Ńf�o�C�X�쐻���� VK_KHR_SWAPCHAIN_EXTENSION_NAME �G�N�X�e���V������L���ɂ��č쐬���Ă���
		VK_KHR_SWAPCHAIN_EXTENSION_NAME,
#ifdef USE_PUSH_DESCRIPTOR
		VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME,
#endif
#ifdef USE_RENDERDOC
		VK_EXT_DEBUG_MARKER_EXTENSION_NAME,
#endif
#ifdef USE_HDR
		VK_EXT_SWAPCHAIN_COLOR_SPACE_EXTENSION_NAME,
#endif
		VK_EXT_VALIDATION_CACHE_EXTENSION_NAME,
	};

#pragma region RAYTRACING
	//!< ���C�g���[�V���O�g���͓��I�ɔ��f����(�T�|�[�g���Ȃ�GPU���܂��������݂��邽��)�A���̊g���̓R���p�C���f�B���N�e�B�u�Ή��ł܂�������(����GPU�ŃT�|�[�g����邾�낤)
	if (HasRayTracingSupport(PD)) {
		Extensions.emplace_back(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME);
		Extensions.emplace_back(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
		Extensions.emplace_back(VK_KHR_RAY_QUERY_EXTENSION_NAME);
	}
#pragma endregion

	//!< �T�|�[�g�����t�B�[�`���[��S�ėL���ɂ��Ă���A�p�t�H�[�}���X�I�ɂ͕s�K�v�Ȃ��̂̓I�t�ɂ��������ǂ� #PERFORMANCE_TODO
	VkPhysicalDeviceFeatures PDF; vkGetPhysicalDeviceFeatures(PD, &PDF);
	//constexpr VkPhysicalDeviceFeatures PDF = { .geometryShader = VK_TRUE, .tessellationShader = VK_TRUE };
	const VkDeviceCreateInfo DCI = {
		.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
		.queueCreateInfoCount = static_cast<uint32_t>(size(DQCIs)), .pQueueCreateInfos = data(DQCIs),
		.enabledLayerCount = 0, .ppEnabledLayerNames = nullptr, //!< �f�o�C�X�Ń��C���[�̗L�����͔񐄏� (Device layer is deprecated)
		.enabledExtensionCount = static_cast<uint32_t>(size(Extensions)), .ppEnabledExtensionNames = data(Extensions),
		.pEnabledFeatures = &PDF
	};
	VERIFY_SUCCEEDED(vkCreateDevice(PD, &DCI, GetAllocationCallbacks(), &Device));

	//!< �f�o�C�X���x���̊֐������[�h���� (Load device level functions)
#ifdef VK_NO_PROTOYYPES

#define VK_DEVICE_PROC_ADDR(proc) vk ## proc = reinterpret_cast<PFN_vk ## proc>(vkGetDeviceProcAddr(Device, "vk" #proc)); assert(nullptr != vk ## proc && #proc && #proc);
#include "VKDeviceProcAddr.h"
#undef VK_DEVICE_PROC_ADDR

#define VK_DEVICE_PROC_ADDR(proc) vk ## proc = reinterpret_cast<PFN_vk ## proc>(vkGetDeviceProcAddr(Device, "vk" #proc));
#include "VKDeviceProcAddr_RayTracing.h"
#undef VK_DEVICE_PROC_ADDR

#ifdef USE_DEBUG_MARKER
#define VK_DEVICE_PROC_ADDR(proc) vk ## proc = reinterpret_cast<PFN_vk ## proc ## EXT>(vkGetDeviceProcAddr(Device, "vk" #proc "EXT")); assert(nullptr != vk ## proc && #proc);
#include "VKDeviceProcAddr_DebugMarker.h"
#undef VK_DEVICE_PROC_ADDR
#endif //!< USE_DEBUG_MARKER

#endif //!< VK_NO_PROTOYYPES

	//!< �O���t�B�b�N�A�v���[���g�L���[�͓����C���f�b�N�X�̏ꍇ�����邪�ʂ̕ϐ��Ɏ擾���Ă��� (Graphics and presentation index may be same, but save to individual variables)
	vkGetDeviceQueue(Device, GraphicsQueueFamilyIndex, GraphicsQueueIndexInFamily, &GraphicsQueue);
	vkGetDeviceQueue(Device, PresentQueueFamilyIndex, PresentQueueIndexInFamily, &PresentQueue);
	vkGetDeviceQueue(Device, ComputeQueueFamilyIndex, ComputeQueueIndexInFamily, &ComputeQueue);
	//vkGetDeviceQueue(Device, TransferQueueFamilyIndex, TransferQueueIndex, &TransferQueue);
	//vkGetDeviceQueue(Device, SparceBindingQueueFamilyIndex, SparceBindingQueueIndex, &SparceBindingQueue);

	LOG_OK();
}

#if 0
//!< �����̗v������A���C����ۏ؂��Ċm�ۂ���(�C���[�W��128byte�A�o�b�t�@��64byte�̃A���C�����K�v�Ȋ��Ȃ�A128byte�A���C���Ŋm�ۂ����)
//!< �����������I�u�W�F�N�g����A(�A���C����)�قȂ�^�C�v�Ċm�ۂ��\
void VK::AllocateDeviceMemory()
{
	VkPhysicalDeviceMemoryProperties PDMP;
	vkGetPhysicalDeviceMemoryProperties(GetCurrentPhysicalDevice(), &PDMP);
	if (PDMP.memoryHeapCount) {
		DeviceMemories.assign(PDMP.memoryHeapCount, VK_NULL_HANDLE);

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
						.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
						.pNext = nullptr,
						.allocationSize = HeapSize,
						.memoryTypeIndex = i
					};
					Logf("\t\tAllocateDeviceMemory = %llu / %llu (HeapIndex = %d)\n", HeapSize, PDMP.memoryHeaps[HeapIndex].size, PDMP.memoryTypes[i].heapIndex);
					VERIFY_SUCCEEDED(vkAllocateMemory(Device, &MAI, GetAllocationCallbacks(), &DeviceMemories[HeapIndex]));
				}
			}
		}
	}
}
#endif

void VK::AllocateDeviceMemory(VkDeviceMemory* DM, const VkMemoryRequirements& MR, const VkMemoryPropertyFlags MPF)
{
#ifdef _DEBUG
	EnumerateMemoryRequirements(MR, GetCurrentPhysicalDeviceMemoryProperties());
	const auto PDMP = GetCurrentPhysicalDeviceMemoryProperties();
	const auto TypeIndex = GetMemoryTypeIndex(PDMP, MR.memoryTypeBits, MPF);
	Logf("\t\tAllocateDeviceMemory = %llu / %llu (HeapIndex = %d, Align = %llu)\n", MR.size, PDMP.memoryHeaps[PDMP.memoryTypes[TypeIndex].heapIndex].size, PDMP.memoryTypes[TypeIndex].heapIndex, MR.alignment);
#endif
	const VkMemoryAllocateInfo MAI = {
		.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
		.pNext = nullptr,
		.allocationSize = MR.size,
		.memoryTypeIndex = GetMemoryTypeIndex(GetCurrentPhysicalDeviceMemoryProperties(), MR.memoryTypeBits, MPF)
	};
	VERIFY_SUCCEEDED(vkAllocateMemory(Device, &MAI, GetAllocationCallbacks(), DM));
}

//!< �z�X�g�ƃf�o�C�X�̓��� (Synchronization between host and device)
//!< �T�u�~�b�g(vkQueueSubmit) �Ɏg�p���ADraw()��Dispatch()�̓��ŃV�O�i��(�T�u�~�b�g���ꂽ�R�}���h�̊���)��҂� (Used when submit, and wait signal on top of Draw())
//!< ����ƂQ��ڈȍ~�𓯂��Ɉ����ׂɁA�V�O�i���ςݏ��(VK_FENCE_CREATE_SIGNALED_BIT)�ō쐬���Ă��� (Create with signaled state, to do same operation on first time and second time)
void VK::CreateFence(VkDevice Dev)
{
	const VkFenceCreateInfo FCI = {
		.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
		.pNext = nullptr,
		.flags = VK_FENCE_CREATE_SIGNALED_BIT
	};
	VERIFY_SUCCEEDED(vkCreateFence(Dev, &FCI, GetAllocationCallbacks(), &Fence));
	VERIFY_SUCCEEDED(vkCreateFence(Dev, &FCI, GetAllocationCallbacks(), &ComputeFence));

	LOG_OK();
}

//!< �L���[���ł̓���(�قȂ�L���[�Ԃ̓������\) (Synchronization internal queue)
//!< �C���[�W�擾(vkAcquireNextImageKHR)�A�T�u�~�b�g(VkSubmitInfo)�A�v���[���e�[�V����(VkPresentInfoKHR)�Ɏg�p���� (Use when image acquire, submit, presentation) 
void VK::CreateSemaphore(VkDevice Dev)
{
	const VkSemaphoreCreateInfo SCI = {
		.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0
	};

	//!< �v���[���g���������p (Wait for presentation finish)
	VERIFY_SUCCEEDED(vkCreateSemaphore(Dev, &SCI, GetAllocationCallbacks(), &NextImageAcquiredSemaphore));

	//!< �`�抮�������p (Wait for render finish)
	VERIFY_SUCCEEDED(vkCreateSemaphore(Dev, &SCI, GetAllocationCallbacks(), &RenderFinishedSemaphore));

	LOG_OK();
}

void VK::AllocateCommandBuffer()
{
	//!< �L���[�t�@�~�����قȂ�ꍇ�͕ʂ̃R�}���h�v�[����p�ӂ���K�v������A���̃L���[�ɂ̂݃T�u�~�b�g�ł���
	//!< �����X���b�h�œ����Ƀ��R�[�f�B���O����ɂ́A�ʂ̃R�}���h�v�[������A���P�[�g���ꂽ�R�}���h�o�b�t�@�ł���K�v������ (�R�}���h�v�[���͕����X���b�h����A�N�Z�X�s��)
	//!< VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT	: �R�}���h�o�b�t�@���Ƀ��Z�b�g���\�A�w�肵�Ȃ��ꍇ�̓v�[�����ɂ܂Ƃ߂ă��Z�b�g (�R�}���h�o�b�t�@�̃��R�[�f�B���O�J�n���ɈÖٓI�Ƀ��Z�b�g�����̂Œ���)
	//!< VK_COMMAND_POOL_CREATE_TRANSIENT_BIT				: �Z���ŁA���x���T�u�~�b�g���Ȃ��A�����Ƀ��Z�b�g�⃊���[�X�����ꍇ�Ɏw��
	//!< (�����ł�)�v���C�}���p1�A�Z�J���_���p1�̃R�}���h�v�[���쐬���f�t�H���g�����Ƃ���
	const VkCommandPoolCreateInfo CPCI = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
		.pNext = nullptr,
		.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
		.queueFamilyIndex = GraphicsQueueFamilyIndex
	};
	VERIFY_SUCCEEDED(vkCreateCommandPool(Device, &CPCI, GetAllocationCallbacks(), &CommandPools.emplace_back()));
	//!< �Z�J���_���p : �K�������ʃv�[���ɂ���K�v�͖����������ł͕ʃv�[���Ƃ��Ă���
	VERIFY_SUCCEEDED(vkCreateCommandPool(Device, &CPCI, GetAllocationCallbacks(), &SecondaryCommandPools.emplace_back()));

	//!< VK_COMMAND_BUFFER_LEVEL_PRIMARY	: ���ڃL���[�ɃT�u�~�b�g�ł���A�Z�J���_�����R�[���ł��� (Can be submit, can execute secondary)
	//!< VK_COMMAND_BUFFER_LEVEL_SECONDARY	: �T�u�~�b�g�ł��Ȃ��A�v���C�}��������s�����̂� (Cannot submit, only executed from primary)
	//!< �����ł̓f�t�H���g�����Ƃ��āA�v���C�}���A�Z�J���_�����ɃX���b�v�`�F�C�������p�ӂ��邱�ƂƂ���
	const auto SCCount = static_cast<uint32_t>(size(SwapchainImages));
	{
		const auto PrevCount = size(CommandBuffers);
		CommandBuffers.resize(PrevCount + SCCount);
		const VkCommandBufferAllocateInfo CBAI = {
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
			.pNext = nullptr,
			.commandPool = CommandPools[0],
			.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
			.commandBufferCount = SCCount
		};
		VERIFY_SUCCEEDED(vkAllocateCommandBuffers(Device, &CBAI, &CommandBuffers[PrevCount]));
	}
	{
		const auto PrevCount = size(SecondaryCommandBuffers);
		SecondaryCommandBuffers.resize(PrevCount + SCCount);
		const VkCommandBufferAllocateInfo CBAI = {
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
			.pNext = nullptr,
			.commandPool = SecondaryCommandPools[0],
			.level = VK_COMMAND_BUFFER_LEVEL_SECONDARY,
			.commandBufferCount = SCCount
		};
		VERIFY_SUCCEEDED(vkAllocateCommandBuffers(Device, &CBAI, &SecondaryCommandBuffers[PrevCount]));
	}
	LOG_OK();
}

VkSurfaceFormatKHR VK::SelectSurfaceFormat(VkPhysicalDevice PD, VkSurfaceKHR Sfc)
{
	uint32_t Count = 0;
	VERIFY_SUCCEEDED(vkGetPhysicalDeviceSurfaceFormatsKHR(PD, Sfc, &Count, nullptr));
	assert(Count && "Surface format count is zero");
	std::vector<VkSurfaceFormatKHR> SFs(Count);
	VERIFY_SUCCEEDED(vkGetPhysicalDeviceSurfaceFormatsKHR(PD, Sfc, &Count, data(SFs)));

	//!< �����ł͍ŏ��Ɍ������� UNDEFINED �łȂ����̂�I�����Ă��� (Select first format but UNDEFINED here)
	const auto SelectedIndex = [&]() {
		//!< �v�f�� 1 �݂̂� UNDEFINED �̏ꍇ�A�����͖����D���Ȃ��̂�I���ł��� (If there is only 1 element and which is UNDEFINED, we can choose any)
		if (1 == size(SFs) && VK_FORMAT_UNDEFINED == SFs[0].format) {
			return -1;
		}
		for (auto i = 0; i < size(SFs); ++i) {
#ifdef USE_HDR
			switch (SFs[i].colorSpace)
			{
			default: break;
				//!< HDR�Ή��̃f�B�X�v���C�̏ꍇ�A�ȉ����Ԃ邱�Ƃ����҂����
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
	for (auto i = 0; i < size(SFs); ++i) {
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
VkExtent2D VK::SelectSurfaceExtent(const VkSurfaceCapabilitiesKHR& SC, const uint32_t Width, const uint32_t Height)
{
	if (0xffffffff == SC.currentExtent.width) {
		//!< 0xffffffff �̏ꍇ�̓X���b�v�`�F�C���C���[�W�T�C�Y���E�C���h�E�T�C�Y�����肷�邱�ƂɂȂ� (If 0xffffffff, size of swapchain image determines the size of the window)
		//!< ������ Width, Height ���g�p���� (In this case, use argument Width and Height) 
		return VkExtent2D({ .width = (std::clamp)(Width, SC.maxImageExtent.width, SC.minImageExtent.width), .height = (std::clamp)(Height, SC.minImageExtent.height, SC.minImageExtent.height) });
	}
	else {
		//!< �����łȂ��ꍇ�� currentExtent ���g�p���� (Otherwise, use currentExtent)
		return SC.currentExtent;
	}
}
VkPresentModeKHR VK::SelectSurfacePresentMode(VkPhysicalDevice PD, VkSurfaceKHR Sfc)
{
	uint32_t Count;
	VERIFY_SUCCEEDED(vkGetPhysicalDeviceSurfacePresentModesKHR(PD, Sfc, &Count, nullptr));
	assert(Count && "Present mode count is zero");
	std::vector<VkPresentModeKHR> PMs(Count);
	VERIFY_SUCCEEDED(vkGetPhysicalDeviceSurfacePresentModesKHR(PD, Sfc, &Count, data(PMs)));

	//!< �\�Ȃ� VK_PRESENT_MODE_MAILBOX_KHR ��I���A�����łȂ���� VK_PRESENT_MODE_FIFO_KHR ��I�� (Want to select VK_PRESENT_MODE_MAILBOX_KHR, or select VK_PRESENT_MODE_FIFO_KHR)
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

//!< �蓮�ŃN���A����ꍇ�ɂ� VkImageUsageFlags �ɒǉ��� VK_IMAGE_USAGE_TRANSFER_DST_BIT �̎w�肪�K�v
void VK::CreateSwapchain(VkPhysicalDevice PD, VkSurfaceKHR Sfc, const uint32_t Width, const uint32_t Height, const VkImageUsageFlags IUF)
{
	VkSurfaceCapabilitiesKHR SC;
	VERIFY_SUCCEEDED(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(PD, Sfc, &SC));

	Log("\tSurfaceCapabilities\n");
	Logf("\t\tminImageCount = %d\n", SC.minImageCount);
	Logf("\t\tmaxImageCount = %d\n", SC.maxImageCount);
	Logf("\t\tcurrentExtent = %d x %d\n", SC.minImageExtent.width, SC.currentExtent.height);
	Logf("\t\tminImageExtent = %d x %d\n", SC.currentExtent.width, SC.minImageExtent.height);
	Logf("\t\tmaxImageExtent = %d x %d\n", SC.maxImageExtent.width, SC.maxImageExtent.height);
	Logf("\t\tmaxImageArrayLayers = %d\n", SC.maxImageArrayLayers);
	Log("\t\tsupportedTransforms = ");
#define VK_SURFACE_TRANSFORM_ENTRY(entry) if(SC.supportedTransforms & VK_SURFACE_TRANSFORM_##entry##_BIT_KHR) { Logf("%s | ", #entry); }
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
#define VK_SURFACE_TRANSFORM_ENTRY(entry) if(SC.currentTransform == VK_SURFACE_TRANSFORM_##entry##_BIT_KHR) { Logf("%s\n", #entry); }
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
#define VK_COMPOSITE_ALPHA_ENTRY(entry) if(SC.supportedCompositeAlpha & VK_COMPOSITE_ALPHA_##entry##_BIT_KHR) { Logf("%s | ", #entry); }
	VK_COMPOSITE_ALPHA_ENTRY(OPAQUE);
	VK_COMPOSITE_ALPHA_ENTRY(PRE_MULTIPLIED);
	VK_COMPOSITE_ALPHA_ENTRY(POST_MULTIPLIED);
	VK_COMPOSITE_ALPHA_ENTRY(INHERIT);
#undef VK_COMPOSITE_ALPHA_ENTRY
	Log("\n");
	Log("\t\tsupportedUsageFlags = ");
#define VK_IMAGE_USAGE_ENTRY(entry) if(SC.supportedUsageFlags & VK_IMAGE_USAGE_##entry) { Logf("%s | ", #entry); }
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
	const auto ImageCount = (std::min)(SC.minImageCount + 1, 0 == SC.maxImageCount ? UINT32_MAX : SC.maxImageCount);
	Logf("\t\t\tImagCount = %d\n", ImageCount);

	//!< �T�[�t�F�X�̃t�H�[�}�b�g��I��
	const auto SurfaceFormat = SelectSurfaceFormat(PD, Sfc);
	ColorFormat = SurfaceFormat.format; //!< �J���[�t�@�[�}�b�g�͊o���Ă���

	//!< �T�[�t�F�X�̃T�C�Y��I��
	SurfaceExtent2D = SelectSurfaceExtent(SC, Width, Height);
	Logf("\t\t\tSurfaceExtent = %d x %d\n", SurfaceExtent2D.width, SurfaceExtent2D.height);

	//!< ���C���[�A�X�e���I�����_�����O�����������ꍇ��1�ȏ�ɂȂ邪�A�����ł�1
	uint32_t ImageArrayLayers = 1;

	//!< �T�|�[�g����Ȃ�ImageUsageFlag���w�肳�ꂽ
	assert((IUF & SC.supportedUsageFlags) && "Specified ImageUsageFlag is not supported");

	//!< �O���t�B�b�N�ƃv���[���g�̃L���[�t�@�~�����قȂ�ꍇ�̓L���[�t�@�~���C���f�b�N�X�̔z�񂪕K�v�A�܂� VK_SHARING_MODE_CONCURRENT ���w�肷�邱��
	//!< (������ VK_SHARING_MODE_CONCURRENT �ɂ���ƃp�t�H�[�}���X��������ꍇ������)
	std::vector<uint32_t> QueueFamilyIndices;
	if (GraphicsQueueFamilyIndex != PresentQueueFamilyIndex) {
		QueueFamilyIndices.emplace_back(GraphicsQueueFamilyIndex);
		QueueFamilyIndices.emplace_back(PresentQueueFamilyIndex);
	}

	//!< �T�[�t�F�X����]�A���]�������邩�ǂ��� (Rotate, mirror surface or not)
	const auto SurfaceTransform = (VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR & SC.supportedTransforms) ? VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR : SC.currentTransform;

	//!< �T�[�t�F�X�̃v���[���g���[�h��I��
	const auto SurfacePresentMode = SelectSurfacePresentMode(PD, Sfc);

	//!< �����̂͌�ŊJ������̂� OldSwapchain �Ɋo���Ă��� (�Z�b�e�B���O��ύX���ăX���b�v�`�F�C�����č쐬����ꍇ���ɔ�����)
	auto OldSwapchain = Swapchain;
	const VkSwapchainCreateInfoKHR SCI = {
		.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
		.pNext = nullptr,
		.flags = 0,
		.surface = Sfc,
		.minImageCount = ImageCount,
		.imageFormat = SurfaceFormat.format, .imageColorSpace = SurfaceFormat.colorSpace,
		.imageExtent = SurfaceExtent2D,
		.imageArrayLayers = ImageArrayLayers,
		.imageUsage = IUF,
		.imageSharingMode = empty(QueueFamilyIndices) ? VK_SHARING_MODE_EXCLUSIVE : VK_SHARING_MODE_CONCURRENT, 
		.queueFamilyIndexCount = static_cast<uint32_t>(size(QueueFamilyIndices)), .pQueueFamilyIndices = data(QueueFamilyIndices),
		.preTransform = SurfaceTransform,
		.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
		.presentMode = SurfacePresentMode,
		.clipped = VK_TRUE,
		.oldSwapchain = OldSwapchain
	};
	VERIFY_SUCCEEDED(vkCreateSwapchainKHR(Device, &SCI, GetAllocationCallbacks(), &Swapchain));

#ifdef USE_HDR
	const std::array SCs = { Swapchain };
	const std::array MDs = { 
		VkHdrMetadataEXT({
			.sType = VK_STRUCTURE_TYPE_HDR_METADATA_EXT,
			.pNext = nullptr,
			.displayPrimaryRed = VkXYColorEXT({ .x = 0.708f, .y = 0.292f }), 
			.displayPrimaryGreen = VkXYColorEXT({ .x = 0.17f, .y = 0.797f }),
			.displayPrimaryBlue = VkXYColorEXT({ .x = 0.131f, .y = 0.046f }),
			.whitePoint = VkXYColorEXT({ .x = 0.3127f, .y = 0.329f }),	
			.maxLuminance = 1000.0f, .minLuminance = 0.001f,
			.maxContentLightLevel = 2000.0f,
			.maxFrameAverageLightLevel = 500.0f
		})
	};
	assert(size(SCs) == size(MDs) && "");
	vkSetHdrMetadataEXT(Device, static_cast<uint32_t>(size(SCs)), data(SCs), data(MDs));
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
	//	//if (!empty(i.second)) {
	//	//	vkFreeCommandBuffers(Device, i.first, static_cast<uint32_t>(size(i.second)), data(i.second));
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
	VERIFY_SUCCEEDED(vkGetSwapchainImagesKHR(Dev, SC, &Count, data(SwapchainImages)));

	LOG_OK();
}
/**
@note Vulakn�ł́A1�̃R�}���h�o�b�t�@�ŕ����̃X���b�v�`�F�C���C���[�W���܂Ƃ߂ď����ł�����ۂ�
*/
void VK::InitializeSwapchainImage(const VkCommandBuffer CB, const VkClearColorValue* CCV)
{
	const VkCommandBufferBeginInfo CBBI = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		.pNext = nullptr,
		.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
		.pInheritanceInfo = nullptr
	};
	VERIFY_SUCCEEDED(vkBeginCommandBuffer(CB, &CBBI)); {
		const VkImageSubresourceRange ISR = {
			.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
			.baseMipLevel = 0, .levelCount = 1,
			.baseArrayLayer = 0, .layerCount = 1
		};
		for (auto& i : SwapchainImages) {
			if (nullptr == CCV) {
				const VkImageMemoryBarrier ImageMemoryBarrier_UndefinedToPresent = {
					.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
					.pNext = nullptr,
					.srcAccessMask = 0, .dstAccessMask = VK_ACCESS_MEMORY_READ_BIT,
					//!<�u���݂̃��C�A�E�g�v�܂��́uUNDEFINED�v���w�肷�邱�ƁA�C���[�W�R���e���c��ێ��������ꍇ�́uUNDEFINED�v�̓_���A�v���[���e�[�V�����\�� VK_IMAGE_LAYOUT_PRESENT_SRC_KHR ��    
					.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED, .newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
					.srcQueueFamilyIndex = PresentQueueFamilyIndex, .dstQueueFamilyIndex = PresentQueueFamilyIndex,
					.image = i,
					.subresourceRange = ISR
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
					.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
					.pNext = nullptr,
					.srcAccessMask = 0, .dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
					//!<�u���݂̃��C�A�E�g�v�܂��́uUNDEFINED�v���w�肷�邱�ƁA�f�X�e�B�l�[�V������
					.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED, .newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
					.srcQueueFamilyIndex = PresentQueueFamilyIndex, .dstQueueFamilyIndex = PresentQueueFamilyIndex,
					.image = i,
					.subresourceRange = ISR
				};
				vkCmdPipelineBarrier(CB,
					VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
					0,
					0, nullptr,
					0, nullptr,
					1, &ImageMemoryBarrier_UndefinedToTransfer);
				{
					vkCmdClearColorImage(CB, i, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, CCV, 1, &ISR);
				}
				const VkImageMemoryBarrier ImageMemoryBarrier_TransferToPresent = {
					.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
					.pNext = nullptr,
					.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT, .dstAccessMask = VK_ACCESS_MEMORY_READ_BIT,
					//!< �f�X�e�B�l�[�V��������A�v���[���e�[�V�����\�� VK_IMAGE_LAYOUT_PRESENT_SRC_KHR ��
					.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, .newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
					.srcQueueFamilyIndex = PresentQueueFamilyIndex, .dstQueueFamilyIndex = PresentQueueFamilyIndex,
					.image = i,
					.subresourceRange = ISR
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

	SubmitAndWait(GraphicsQueue, CB);

	LOG_OK();
}

void VK::CreateSwapchainImageView()
{
	for(auto i : SwapchainImages) {
		SwapchainImageViews.emplace_back(VkImageView());
		const VkComponentMapping CM = { .r = VK_COMPONENT_SWIZZLE_IDENTITY, .g = VK_COMPONENT_SWIZZLE_IDENTITY, .b = VK_COMPONENT_SWIZZLE_IDENTITY, .a = VK_COMPONENT_SWIZZLE_IDENTITY, };
		const VkImageSubresourceRange ISR = {
			.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
			.baseMipLevel = 0, .levelCount = 1,
			.baseArrayLayer = 0, .layerCount = 1
		};
		CreateImageView(&SwapchainImageViews.back(), i, VK_IMAGE_VIEW_TYPE_2D, ColorFormat, CM, ISR);
	}

	LOG_OK();
}

#if 0
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
	const std::array<VkSubmitInfo, 1> SIs = {
		{
			VK_STRUCTURE_TYPE_SUBMIT_INFO,
			nullptr,
			0, nullptr, nullptr,
			1,  &CB,
			0, nullptr
		}
	};
	VERIFY_SUCCEEDED(vkQueueSubmit(GraphicsQueue, static_cast<uint32_t>(size(SIs)), data(SIs), VK_NULL_HANDLE));
	VERIFY_SUCCEEDED(vkQueueWaitIdle(GraphicsQueue));
}
#endif

void VK::CreateViewport(const float Width, const float Height, const float MinDepth, const float MaxDepth)
{
	//!< Vulkan �̓f�t�H���g�Łu����v�����_ (DirectX�AOpenGL�́u�����v�����_)
	Viewports = {
		VkViewport({
			//!< USE_VIEWPORT_Y_UP
			//!< VK�ł̓f�t�H���g�ŁuY�����v���������A�����ɕ��̒l���w�肷��ƁuY����v�������ADX�Ɠ��l�ɂȂ� (In VK, by specifying negative height, Y become up. same as DX)
			//!< �ʏ��_�́u����v���w�肷�邪�A�����ɕ��̒l���w�肷��ꍇ�́u�����v���w�肷�邱�� (When negative height, specify left bottom as base, otherwise left up)
#ifdef USE_VIEWPORT_Y_UP
			.x = 0.0f, .y = Height,
			.width = Width, .height = -Height,
#else
			.x = 0.0f, .y = 0.0f,
			.width = Width, .height = Height,
#endif
			.minDepth = MinDepth, .maxDepth = MaxDepth
		})
	};
	//!< offset, extent �Ŏw�� (left, top, right, bottom�Ŏw���DX�Ƃ͈قȂ�̂Œ���)
	ScissorRects = {
		VkRect2D({.offset = VkOffset2D({.x = 0, .y = 0 }), .extent = VkExtent2D({.width = static_cast<uint32_t>(Width), .height = static_cast<uint32_t>(Height) }) }),
	};

	LOG_OK();
}
void VK::CreateBufferMemory(VkBuffer* Buffer, VkDeviceMemory* DeviceMemory, const VkDevice Device, const VkPhysicalDeviceMemoryProperties PDMP, const VkBufferUsageFlags BUF, const size_t Size, const VkMemoryPropertyFlags MPF, const void* Source)
{
	assert(Size && "");
#pragma region BUFFER
	//!< �o�b�t�@�͍쐬���Ɏw�肵���g�p�@�ł����g�p�ł��Ȃ��A�����ł� VK_SHARING_MODE_EXCLUSIVE ���ߑł��ɂ��Ă��� #VK_TODO (Using VK_SHARING_MODE_EXCLUSIVE here)
	//!< VK_SHARING_MODE_EXCLUSIVE	: �����t�@�~���̃L���[�������A�N�Z�X�ł��Ȃ��A���̃t�@�~������A�N�Z�X�������ꍇ�̓I�[�i�[�V�b�v�̈ڏ����K�v
	//!< VK_SHARING_MODE_CONCURRENT	: �����t�@�~���̃L���[�������A�N�Z�X�\�A�I�[�i�[�V�b�v�̈ڏ����K�v�����A�p�t�H�[�}���X�͈���
	const std::array<uint32_t, 0> QFI = {};
	const VkBufferCreateInfo BCI = {
		.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
		.size = Size,
		.usage = BUF,
		.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
		.queueFamilyIndexCount = static_cast<uint32_t>(size(QFI)), .pQueueFamilyIndices = data(QFI)
	};
	VERIFY_SUCCEEDED(vkCreateBuffer(Device, &BCI, GetAllocationCallbacks(), Buffer));
#pragma endregion

#pragma  region MEMORY
	VkMemoryRequirements MR; 
	vkGetBufferMemoryRequirements(Device, *Buffer, &MR);
#ifdef _DEBUG
	EnumerateMemoryRequirements(MR, PDMP);
	const auto TypeIndex = GetMemoryTypeIndex(PDMP, MR.memoryTypeBits, MPF);
	Logf("\t\tAllocateDeviceMemory = %llu / %llu (HeapIndex = %d, Align = %llu)\n", MR.size, PDMP.memoryHeaps[PDMP.memoryTypes[TypeIndex].heapIndex].size, PDMP.memoryTypes[TypeIndex].heapIndex, MR.alignment);
#endif
	//!< VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT ���w�肳�ꂽ�ꍇ�� pNext �֎w�肷��
	const VkMemoryAllocateFlagsInfo MAFI = {.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO, .pNext = nullptr, .flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT, .deviceMask = 0 };
	const VkMemoryAllocateInfo MAI = {
		.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
		.pNext = (VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT & BUF) ? &MAFI : nullptr,
		.allocationSize = MR.size,
		.memoryTypeIndex = GetMemoryTypeIndex(PDMP, MR.memoryTypeBits, MPF)
	};
	VERIFY_SUCCEEDED(vkAllocateMemory(Device, &MAI, GetAllocationCallbacks(), DeviceMemory));
#pragma endregion

	VERIFY_SUCCEEDED(vkBindBufferMemory(Device, *Buffer, *DeviceMemory, 0));

#pragma region COPY
	if (nullptr != Source) {
		void* Data;
		VERIFY_SUCCEEDED(vkMapMemory(Device, *DeviceMemory, 0, Size, static_cast<VkMemoryMapFlags>(0), &Data)); {
			memcpy(Data, Source, Size);

			//!< �������R���e���c���ύX���ꂽ���Ƃ𖾎��I�Ƀh���C�o�֒m�点��(vkMapMemory()������Ԃł�邱��)
			if (!(VK_MEMORY_PROPERTY_HOST_COHERENT_BIT & BUF)) {
				const std::array MMRs = { 
					VkMappedMemoryRange({.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE, .pNext = nullptr, .memory = *DeviceMemory, .offset = 0, .size = VK_WHOLE_SIZE }),
				};
				VERIFY_SUCCEEDED(vkFlushMappedMemoryRanges(Device, static_cast<uint32_t>(size(MMRs)), data(MMRs)));
			}

		} vkUnmapMemory(Device, *DeviceMemory);
	}
#pragma endregion
}

void VK::CreateImageMemory(VkImage* Image, VkDeviceMemory* DM, const VkDevice Device, const VkPhysicalDeviceMemoryProperties PDMP, const VkFormat Format, const VkExtent3D& Extent, const VkImageUsageFlags IUF)
{
	constexpr std::array<uint32_t, 0> QueueFamilyIndices = {};
	const VkImageCreateInfo ICI = {
		.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
		.imageType = VK_IMAGE_TYPE_2D,
		.format = Format,
		.extent = Extent,
		.mipLevels = 1,
		.arrayLayers = 1,
		.samples = VK_SAMPLE_COUNT_1_BIT,
		.tiling = VK_IMAGE_TILING_OPTIMAL,
		.usage = IUF,
		.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
		.queueFamilyIndexCount = static_cast<uint32_t>(size(QueueFamilyIndices)), .pQueueFamilyIndices = data(QueueFamilyIndices),
		.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED
	};
	VERIFY_SUCCEEDED(vkCreateImage(Device, &ICI, GetAllocationCallbacks(), Image));

	VkMemoryRequirements MR;
	vkGetImageMemoryRequirements(Device, *Image, &MR);
	const VkMemoryAllocateInfo MAI = {
		.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
		.pNext = nullptr,
		.allocationSize = MR.size,
		.memoryTypeIndex = VK::GetMemoryTypeIndex(PDMP, MR.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
	};
	VERIFY_SUCCEEDED(vkAllocateMemory(Device, &MAI, GetAllocationCallbacks(), DM));
	VERIFY_SUCCEEDED(vkBindImageMemory(Device, *Image, *DM, 0));
}

void VK::SubmitStagingCopy(const VkBuffer Buf, const VkQueue Queue, const VkCommandBuffer CB, const VkAccessFlagBits AF, const VkPipelineStageFlagBits PSF, const VkDeviceSize Size, const void* Source)
{
	Scoped<BufferMemory> StagingBuffer(Device);
	//!< �z�X�g�r�W�u���o�b�t�@�A�f�o�C�X���������쐬 (Create host visible buffer, device memory)
	StagingBuffer.Create(Device, GetCurrentPhysicalDeviceMemoryProperties(), VK_BUFFER_USAGE_TRANSFER_SRC_BIT, Size, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, Source);	

	
	{
		constexpr VkCommandBufferBeginInfo CBBI = {
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
			.pNext = nullptr,
			.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
			.pInheritanceInfo = nullptr
		};
		VERIFY_SUCCEEDED(vkBeginCommandBuffer(CB, &CBBI)); {
			//!< �z�X�g�r�W�u������f�o�C�X���[�J���ւ̃R�s�[�R�}���h�𔭍s (Submit host visible to device local copy command)
			PopulateCommandBuffer_CopyBufferToBuffer(CB, StagingBuffer.Buffer, Buf, AF, PSF, Size);
		} VERIFY_SUCCEEDED(vkEndCommandBuffer(CB));
		SubmitAndWait(Queue, CB);
	}
}
void VK::CreateBufferMemoryAndSubmitTransferCommand(VkBuffer* Buffer, VkDeviceMemory* DeviceMemory, const VkDevice Device, const VkPhysicalDeviceMemoryProperties PDMP, const VkBufferUsageFlags BUF, const size_t Size, const void* Source, 
	const VkCommandBuffer CB, const VkAccessFlagBits AF, const VkPipelineStageFlagBits PSF, const VkQueue Queue)
{
	//!< �f�o�C�X���[�J���o�b�t�@�A�f�o�C�X���������쐬 (Create device local buffer, device memory)
	CreateBufferMemory(Buffer, DeviceMemory, Device, PDMP, BUF | VK_BUFFER_USAGE_TRANSFER_DST_BIT, Size, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	Scoped<BufferMemory> StagingBuffer(Device);
	//!< �z�X�g�r�W�u���o�b�t�@�A�f�o�C�X���������쐬 (Create host visible buffer, device memory)
	StagingBuffer.Create(Device, PDMP, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, Size, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, Source);
	{
		constexpr VkCommandBufferBeginInfo CBBI = {
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
			.pNext = nullptr,
			.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
			.pInheritanceInfo = nullptr
		};
		VERIFY_SUCCEEDED(vkBeginCommandBuffer(CB, &CBBI)); {
			//!< �z�X�g�r�W�u������f�o�C�X���[�J���ւ̃R�s�[�R�}���h�𔭍s (Submit host visible to device local copy command)
			PopulateCommandBuffer_CopyBufferToBuffer(CB, StagingBuffer.Buffer, *Buffer, AF, PSF, Size);
		} VERIFY_SUCCEEDED(vkEndCommandBuffer(CB));

		SubmitAndWait(Queue, CB);
	}
}
#pragma region RAYTRACING
bool VK::HasRayTracingSupport(const VkPhysicalDevice PD)
{
	VkPhysicalDeviceBufferDeviceAddressFeatures PDBDAF = { .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES, .pNext = nullptr };
	VkPhysicalDeviceRayTracingPipelineFeaturesKHR PDRTPF = { .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR, .pNext = &PDBDAF };
	VkPhysicalDeviceAccelerationStructureFeaturesKHR PDASF = { .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR, .pNext = &PDRTPF };
	VkPhysicalDeviceFeatures2 PDF2 = { .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2, .pNext = &PDASF };
	vkGetPhysicalDeviceFeatures2(PD, &PDF2);
	return PDBDAF.bufferDeviceAddress && PDRTPF.rayTracingPipeline && PDASF.accelerationStructure;
}
void VK::BuildAccelerationStructure(const VkDevice Device, const VkPhysicalDeviceMemoryProperties PDMP, const VkQueue Queue, const VkCommandBuffer CB, const VkAccelerationStructureKHR AS, const VkAccelerationStructureTypeKHR Type, const VkDeviceSize Size, const std::vector<VkAccelerationStructureGeometryKHR>& ASGs)
{
	Scoped<ScratchBuffer> SB1(Device);
	ScratchBuffer SB;
	SB.Create(Device, PDMP, Size); {
		const std::array ASBGIs = {
			VkAccelerationStructureBuildGeometryInfoKHR({
				.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR,
				.pNext = nullptr,
				.type = Type,
				.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR,
				.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR,
				.srcAccelerationStructure = VK_NULL_HANDLE, .dstAccelerationStructure = AS,
				.geometryCount = static_cast<uint32_t>(size(ASGs)),.pGeometries = data(ASGs), .ppGeometries = nullptr,
				.scratchData = VkDeviceOrHostAddressKHR({.deviceAddress = GetDeviceAddress(Device, SB.Buffer)})
			}),
		};
		const std::array ASBRIs = { VkAccelerationStructureBuildRangeInfoKHR({.primitiveCount = 1, .primitiveOffset = 0, .firstVertex = 0, .transformOffset = 0 }), };
		const std::array ASBRIss = { data(ASBRIs) };
		vkCmdBuildAccelerationStructuresKHR(CB, static_cast<uint32_t>(size(ASBGIs)), data(ASBGIs), data(ASBRIss));

		SubmitAndWait(Queue, CB);
	} SB.Destroy(Device);
}
#pragma endregion

void VK::CreateUniformBuffer_Example()
{
	//!< ����̓T���v���R�[�h

	VkBuffer Buffer; //!< #VK_TODO
	VkDeviceSize Size = 0; //!< #VK_TODO
	CreateBuffer(&Buffer, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, Size);

	VkDeviceMemory DM; //!< #VK_TODO
	AllocateDeviceMemory(&DM, Buffer, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	VERIFY_SUCCEEDED(vkBindBufferMemory(Device, Buffer, DM, 0));
}
void VK::CreateStorageBuffer_Example()
{
	//!< ����̓T���v���R�[�h

	VkBuffer Buffer; //!< #VK_TODO
	VkDeviceSize Size = 0; //!< #VK_TODO
	CreateBuffer(&Buffer, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, Size);
	//CreateBuffer(&Buffer, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, Size);

	VkDeviceMemory DM; //!< #VK_TODO
	AllocateDeviceMemory(&DM, Buffer, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	VERIFY_SUCCEEDED(vkBindBufferMemory(Device, Buffer, DM, 0));
}
void VK::CreateUniformTexelBuffer_Example()
{
	//!< ����̓T���v���R�[�h

	VkBuffer Buffer; //!< #VK_TODO
	const VkDeviceSize Size = 0; //!< #VK_TODO
	CreateBuffer(&Buffer, VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT, Size);

	VkDeviceMemory DM; //!< #VK_TODO
	AllocateDeviceMemory(&DM, Buffer, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	VERIFY_SUCCEEDED(vkBindBufferMemory(Device, Buffer, DM, 0));

	VkBufferView View; //!< #VK_TODO
	const VkBufferViewCreateInfo BVCI = {
		.sType = VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
		.buffer = Buffer,
		.format = VK_FORMAT_R8G8B8A8_UINT,
		.offset = 0,
		.range = VK_WHOLE_SIZE
	};
	VERIFY_SUCCEEDED(vkCreateBufferView(Device, &BVCI, GetAllocationCallbacks(), &View)); 

	//!< �T�|�[�g����Ă��邩�̃`�F�b�N
	VkFormatProperties FP;
	vkGetPhysicalDeviceFormatProperties(GetCurrentPhysicalDevice(), BVCI.format, &FP);
	assert((FP.bufferFeatures & VK_FORMAT_FEATURE_UNIFORM_TEXEL_BUFFER_BIT) && "");
}
void VK::CreateStorageTexelBuffer_Example()
{
	//!< ����̓T���v���R�[�h

	VkBuffer Buffer; //!< #VK_TODO
	const VkDeviceSize Size = 0; //!< #VK_TODO
	CreateBuffer(&Buffer, VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT, Size);

	VkDeviceMemory DM; //!< #VK_TODO
	AllocateDeviceMemory(&DM, Buffer, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	VERIFY_SUCCEEDED(vkBindBufferMemory(Device, Buffer, DM, 0));

	VkBufferView View; //!< #VK_TODO
	const VkBufferViewCreateInfo BVCI = {
		.sType = VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
		.buffer = Buffer,
		.format = VK_FORMAT_R8G8B8A8_UINT,
		.offset = 0,
		.range = VK_WHOLE_SIZE
	};
	VERIFY_SUCCEEDED(vkCreateBufferView(Device, &BVCI, GetAllocationCallbacks(), &View));

	//!< �T�|�[�g����Ă��邩�̃`�F�b�N
	VkFormatProperties FP;
	vkGetPhysicalDeviceFormatProperties(GetCurrentPhysicalDevice(), BVCI.format, &FP);
	assert((FP.bufferFeatures & VK_FORMAT_FEATURE_STORAGE_TEXEL_BUFFER_BIT) && "");
	//!< #VK_TODO �A�g�~�b�N���������ꍇ
	if (false/*bUseAtomic*/) {
		assert((FP.linearTilingFeatures & VK_FORMAT_FEATURE_STORAGE_TEXEL_BUFFER_ATOMIC_BIT) && "");
	}
}

void VK::CreateDescriptorSetLayout(VkDescriptorSetLayout& DSL, const VkDescriptorSetLayoutCreateFlags Flags, const std::vector<VkDescriptorSetLayoutBinding>& DSLBs)
{
	const VkDescriptorSetLayoutCreateInfo DSLCI = {
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
		.pNext = nullptr,
		.flags = Flags,
		.bindingCount = static_cast<uint32_t>(size(DSLBs)), .pBindings = data(DSLBs)
	};
	VERIFY_SUCCEEDED(vkCreateDescriptorSetLayout(Device, &DSLCI, GetAllocationCallbacks(), &DSL));

	LOG_OK();
}

void VK::CreatePipelineLayout(VkPipelineLayout& PL, const std::vector<VkDescriptorSetLayout>& DSLs, const std::vector<VkPushConstantRange>& PCRs)
{
	const VkPipelineLayoutCreateInfo PLCI = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
		.setLayoutCount = static_cast<uint32_t>(size(DSLs)), .pSetLayouts = data(DSLs),
		.pushConstantRangeCount = static_cast<uint32_t>(size(PCRs)), .pPushConstantRanges = data(PCRs)
	};
	VERIFY_SUCCEEDED(vkCreatePipelineLayout(Device, &PLCI, GetAllocationCallbacks(), &PL));

	LOG_OK();
}

//!< �f�X�N���v�^�Z�b�g���X�ɉ���������ꍇ�ɂ� .flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT ���w�肷��A
//!< ���̏ꍇ�f�Љ��͎����ŊǗ����Ȃ��Ă͂Ȃ�Ȃ� (�w�肵�Ȃ��ꍇ�̓v�[�����ɂ܂Ƃ߂ĉ�������ł��Ȃ�)
//!< 1�̃u�[���ɑ΂��āA�����X���b�h�œ����Ƀf�X�N���v�^�Z�b�g���m�ۂ��邱�Ƃ͂ł��Ȃ� (�X���b�h���ɕʃv�[���ɂ��邱��)
void VK::CreateDescriptorPool(VkDescriptorPool& DP, const VkDescriptorPoolCreateFlags Flags, const std::vector<VkDescriptorPoolSize>& DPSs)
{
	uint32_t MaxSets = 0;
	for (const auto& i : DPSs) {
		MaxSets = (std::max)(MaxSets, i.descriptorCount);
	}

	const VkDescriptorPoolCreateInfo DPCI = {
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
		.pNext = nullptr,
		.flags = Flags,
		.maxSets = MaxSets,
		.poolSizeCount = static_cast<uint32_t>(size(DPSs)), .pPoolSizes = data(DPSs)
	};
	VERIFY_SUCCEEDED(vkCreateDescriptorPool(Device, &DPCI, GetAllocationCallbacks(), &DP));
}

void VK::CreateDescriptorUpdateTemplate(VkDescriptorUpdateTemplate& DUT, const std::vector<VkDescriptorUpdateTemplateEntry>& DUTEs, const VkDescriptorSetLayout DSL)
{
	const VkDescriptorUpdateTemplateCreateInfo DUTCI = {
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_UPDATE_TEMPLATE_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
		.descriptorUpdateEntryCount = static_cast<uint32_t>(size(DUTEs)), .pDescriptorUpdateEntries = data(DUTEs),
		.templateType = VK_DESCRIPTOR_UPDATE_TEMPLATE_TYPE_DESCRIPTOR_SET,
		.descriptorSetLayout = DSL,
		.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS, 
		.pipelineLayout = VK_NULL_HANDLE, .set = 0 //!< VK_DESCRIPTOR_UPDATE_TEMPLATE_TYPE_PUSH_DESCRIPTORS_KHR�g�p���̓p�C�v���C�����C�A�E�g���w�� (�g�p���USE_PUSH_DESCRIPTOR�Q��)
	};
	VERIFY_SUCCEEDED(vkCreateDescriptorUpdateTemplate(Device, &DUTCI, GetAllocationCallbacks(), &DUT));
}

void VK::CreateTexture1x1(const uint32_t Color, const VkPipelineStageFlags PSF)
{
	constexpr auto Format = VK_FORMAT_R8G8B8A8_UNORM;
	constexpr auto Extent = VkExtent3D({ .width = 1, .height = 1, .depth = 1 });

	Images.emplace_back(Image());
	CreateImage(&Images.back().Image, 0, VK_IMAGE_TYPE_2D, Format, Extent, 1, 1, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);
	AllocateDeviceMemory(&Images.back().DeviceMemory, Images.back().Image, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	VERIFY_SUCCEEDED(vkBindImageMemory(Device, Images.back().Image, Images.back().DeviceMemory, 0));

	//!< �r���[�̍쐬 (Create view)
	ImageViews.emplace_back(VkImageView());
	const VkImageViewCreateInfo IVCI = {
		.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
		.image = Images.back().Image,
		.viewType = VK_IMAGE_VIEW_TYPE_2D,
		.format = Format,
		.components = VkComponentMapping({.r = VK_COMPONENT_SWIZZLE_R, .g = VK_COMPONENT_SWIZZLE_G, .b = VK_COMPONENT_SWIZZLE_B, .a = VK_COMPONENT_SWIZZLE_A }),
		.subresourceRange = VkImageSubresourceRange({.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .baseMipLevel = 0, .levelCount = VK_REMAINING_MIP_LEVELS, .baseArrayLayer = 0, .layerCount = VK_REMAINING_ARRAY_LAYERS }),
	};
	VERIFY_SUCCEEDED(vkCreateImageView(Device, &IVCI, GetAllocationCallbacks(), &ImageViews.back()));

	{
		const std::array Colors = { Color };
		constexpr auto LayerSize = sizeof(Colors[0]);
		constexpr auto TotalSize = 1 * LayerSize;

		VkBuffer Buffer;
		VkDeviceMemory DeviceMemory;
		{
			//!< �A�b�v���[�h�p�o�b�t�@ (Buffer for upload)
			CreateBuffer(&Buffer, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, TotalSize);
			AllocateDeviceMemory(&DeviceMemory, Buffer, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
			VERIFY_SUCCEEDED(vkBindBufferMemory(Device, Buffer, DeviceMemory, 0));
			CopyToHostVisibleDeviceMemory(DeviceMemory, 0, TotalSize, data(Colors));

			//!< �o�b�t�@->�e�N�X�`���]���R�}���h (Buffer to image copy command)
			const std::vector BICs = {
				VkBufferImageCopy({
					.bufferOffset = 0, .bufferRowLength = 0, .bufferImageHeight = 0,
					.imageSubresource = VkImageSubresourceLayers({.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .mipLevel = 0, .baseArrayLayer = 0, .layerCount = 1 }),
					.imageOffset = VkOffset3D({.x = 0, .y = 0, .z = 0 }),
					.imageExtent = Extent })
			};
			const auto& CB = CommandBuffers[0];

			constexpr VkCommandBufferBeginInfo CBBI = {
				.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
				.pNext = nullptr,
				.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
				.pInheritanceInfo = nullptr
			};
			VERIFY_SUCCEEDED(vkBeginCommandBuffer(CB, &CBBI)); {
				PopulateCommandBuffer_CopyBufferToImage(CB, Buffer, Images.back().Image, VK_ACCESS_SHADER_READ_BIT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, PSF, BICs, 1, 1);
			} VERIFY_SUCCEEDED(vkEndCommandBuffer(CB));
			//!< �R�}���h�̎��s (Submit command)
			SubmitAndWait(GraphicsQueue, CB);
		}
		vkFreeMemory(Device, DeviceMemory, GetAllocationCallbacks());
		vkDestroyBuffer(Device, Buffer, GetAllocationCallbacks());
	}
}

//!< ABRG
void VK::CreateTextureArray1x1(const std::vector<uint32_t>& Colors, const VkPipelineStageFlags PSF)
{
#pragma region TEX_ARRAY
	const auto Layers = static_cast<uint32_t>(size(Colors));
#pragma endregion
	constexpr auto Format = VK_FORMAT_R8G8B8A8_UNORM;
	constexpr auto Extent = VkExtent3D({ .width = 1, .height = 1, .depth = 1 });

	Images.emplace_back();
#pragma region TEX_ARRAY
	CreateImage(&Images.back().Image, 0, VK_IMAGE_TYPE_2D, Format, Extent, 1, Layers, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);
#pragma endregion
	AllocateDeviceMemory(&Images.back().DeviceMemory, Images.back().Image, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	VERIFY_SUCCEEDED(vkBindImageMemory(Device, Images.back().Image, Images.back().DeviceMemory, 0));

	const VkImageViewCreateInfo IVCI = {
		.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
		.image = Images.back().Image,
#pragma region TEX_ARRAY
		.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY,
#pragma endregion
		.format = Format,
		.components = VkComponentMapping({.r = VK_COMPONENT_SWIZZLE_R, .g = VK_COMPONENT_SWIZZLE_G, .b = VK_COMPONENT_SWIZZLE_B, .a = VK_COMPONENT_SWIZZLE_A }),
		.subresourceRange = VkImageSubresourceRange({.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .baseMipLevel = 0, .levelCount = VK_REMAINING_MIP_LEVELS, .baseArrayLayer = 0, .layerCount = VK_REMAINING_ARRAY_LAYERS }),
	};
	VERIFY_SUCCEEDED(vkCreateImageView(Device, &IVCI, GetAllocationCallbacks(), &ImageViews.emplace_back()));

	//Textures.emplace_back().Create(Device, GetCurrentPhysicalDeviceMemoryProperties(), VK_FORMAT_R8G8B8A8_UNORM, VkExtent3D({.width = 1, .height = 1, .depth = 1}), VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT, VK_IMAGE_ASPECT_COLOR_BIT);

	{
		constexpr auto LayerSize = static_cast<uint32_t>(sizeof(Colors[0]));
		const auto TotalSize = Layers * LayerSize;

		VkBuffer Buffer;
		VkDeviceMemory DeviceMemory;
		{
			CreateBuffer(&Buffer, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, TotalSize);
			AllocateDeviceMemory(&DeviceMemory, Buffer, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
			VERIFY_SUCCEEDED(vkBindBufferMemory(Device, Buffer, DeviceMemory, 0));
			CopyToHostVisibleDeviceMemory(DeviceMemory, 0, TotalSize, data(Colors));

#pragma region TEX_ARRAY
			std::vector<VkBufferImageCopy> BICs;
			for (uint32_t i = 0; i < Layers; ++i) {
				BICs.emplace_back(VkBufferImageCopy({
					.bufferOffset = i * LayerSize, .bufferRowLength = 0, .bufferImageHeight = 0,
					.imageSubresource = VkImageSubresourceLayers({.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .mipLevel = 0, .baseArrayLayer = static_cast<uint32_t>(i), .layerCount = 1 }),
					.imageOffset = VkOffset3D({.x = 0, .y = 0, .z = 0 }),.imageExtent = Extent }));
			}
#pragma endregion

			const auto& CB = CommandBuffers[0];
			constexpr VkCommandBufferBeginInfo CBBI = {
				.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
				.pNext = nullptr,
				.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
				.pInheritanceInfo = nullptr
			};
			VERIFY_SUCCEEDED(vkBeginCommandBuffer(CB, &CBBI)); {
				PopulateCommandBuffer_CopyBufferToImage(CB, Buffer, Images.back().Image, VK_ACCESS_SHADER_READ_BIT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, PSF, BICs, 1, Layers);
			} VERIFY_SUCCEEDED(vkEndCommandBuffer(CB));
			SubmitAndWait(GraphicsQueue, CB);
		}
		vkFreeMemory(Device, DeviceMemory, GetAllocationCallbacks());
		vkDestroyBuffer(Device, Buffer, GetAllocationCallbacks());
	}
}

void VK::CreateRenderPass(VkRenderPass& RP, const std::vector<VkAttachmentDescription>& ADs, const std::vector<VkSubpassDescription>& SDs, const std::vector<VkSubpassDependency>& Deps)
{
	const VkRenderPassCreateInfo RPCI = {
		.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
		.attachmentCount = static_cast<uint32_t>(size(ADs)), .pAttachments = data(ADs),
		.subpassCount = static_cast<uint32_t>(size(SDs)), .pSubpasses = data(SDs),
		.dependencyCount = static_cast<uint32_t>(size(Deps)), .pDependencies = data(Deps)
	};
	VERIFY_SUCCEEDED(vkCreateRenderPass(Device, &RPCI, GetAllocationCallbacks(), &RP));
}
void VK::CreateRenderPass()
{
	//!< �C���v�b�g�A�^�b�`�����g (InputAttachment)
	constexpr std::array<VkAttachmentReference, 0> IAs = {};
	//!< �J���[�A�^�b�`�����g (ColorAttachment)
	constexpr std::array CAs = { VkAttachmentReference({.attachment = 0, .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL }), };
	//!< ���]���u�A�^�b�`�����g (ResolveAttachment) : �}���`�T���v�� �� �V���O���T���v���փ��]���u����悤�ȏꍇ
	constexpr std::array RAs = { VkAttachmentReference({.attachment = VK_ATTACHMENT_UNUSED, .layout = VK_IMAGE_LAYOUT_UNDEFINED }), };
	assert(size(CAs) == size(RAs) && "");
	//!< �v���U�[�u�A�^�b�`�����g (PreserveAttachment) : �T�u�p�X�S�̂ɂ����ĕێ����Ȃ��Ă͂Ȃ�Ȃ��R���e���c�̃C���f�b�N�X
	constexpr std::array<uint32_t, 0> PAs = {};

	CreateRenderPass(RenderPasses.emplace_back(), {
		VkAttachmentDescription({
			.flags = 0,
			.format = ColorFormat,
			.samples = VK_SAMPLE_COUNT_1_BIT,
			.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE, .storeOp = VK_ATTACHMENT_STORE_OP_STORE,						//!< �u�J�n���ɉ������Ȃ��v�u�I�����ɕۑ��v
			.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE, .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,	//!< �X�e���V���̃��[�h�X�g�A : (�����ł�)�J�n���A�I�����Ƃ��Ɂu�g�p���Ȃ��v
			.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED, .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR				//!< �����_�[�p�X�̃��C�A�E�g : �u�J�n������`�v�u�I�����v���[���e�[�V�����\�[�X�v
		}),
	}, {
		VkSubpassDescription({
			.flags = 0,
			.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
			.inputAttachmentCount = static_cast<uint32_t>(size(IAs)), .pInputAttachments = data(IAs), 									//!< �C���v�b�g�A�^�b�`�����g(�ǂݎ��p) �V�F�[�_���Ŏ��̂悤�Ɏg�p layout (input_attachment_index=0, set=0, binding=0) uniform XXX YYY;
			.colorAttachmentCount = static_cast<uint32_t>(size(CAs)), .pColorAttachments = data(CAs), .pResolveAttachments = data(RAs),	//!< �J���[�A�^�b�`�����g(�������ݗp)�A���]���u�A�^�b�`�����g(�}���`�T���v���̃��]���u)
			.pDepthStencilAttachment = nullptr,																							//!< �f�v�X�A�^�b�`�����g(�������ݗp)
			.preserveAttachmentCount = static_cast<uint32_t>(size(PAs)), .pPreserveAttachments = data(PAs)								//!< �v���U�[�u�A�^�b�`�����g(�T�u�p�X�S�̂ɂ����ĕێ�����R���e���c�̃C���f�b�N�X)
		}),
	}, {
#if 1
		//!< �T�u�p�X�ˑ� (�����ď����ꍇ)
		VkSubpassDependency({
			.srcSubpass = VK_SUBPASS_EXTERNAL, .dstSubpass = 0,																		//!< �T�u�p�X�O����T�u�p�X0��
			.srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,	//!< �p�C�v���C���̍ŏI�X�e�[�W����J���[�o�̓X�e�[�W��
			.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT, .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,						//!< �ǂݍ��݂���J���[�������݂�
			.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT,																			//!< �����������̈�ɑ΂��鏑�����݂��������Ă���ǂݍ��� (�w�肵�Ȃ��ꍇ�͎��O�ŏ������݊������Ǘ�)
		}),
		VkSubpassDependency({
			.srcSubpass = 0, .dstSubpass = VK_SUBPASS_EXTERNAL,																		//!< �T�u�p�X0����T�u�p�X�O��
			.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, .dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,	//!< �J���[�o�̓X�e�[�W����p�C�v���C���̍ŏI�X�e�[�W��
			.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, .dstAccessMask = VK_ACCESS_MEMORY_READ_BIT,						//!< �J���[�������݂���ǂݍ��݂�
			.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT,
		}),
#endif
	});
}

void VK::CreateFramebuffer(VkFramebuffer& FB, const VkRenderPass RP, const uint32_t Width, const uint32_t Height, const uint32_t Layers, const std::vector<VkImageView>& IVs)
{
	const VkFramebufferCreateInfo FCI = {
		.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
		.renderPass = RP, //!< �����Ŏw�肷�郌���_�[�p�X�́u�݊����̂�����́v�Ȃ��
		.attachmentCount = static_cast<uint32_t>(size(IVs)), .pAttachments = data(IVs),
		.width = Width, .height = Height,
		.layers = Layers
	};
	VERIFY_SUCCEEDED(vkCreateFramebuffer(Device, &FCI, GetAllocationCallbacks(), &FB));
}

/**
@brief �V�F�[�_�R���p�C���A�����N�̓p�C�v���C���I�u�W�F�N�g�쐬���ɍs���� Shader compilation and linkage is performed during the pipeline object creation
*/
VkShaderModule VK::CreateShaderModule(const std::wstring& Path) const
{
	VkShaderModule ShaderModule = VK_NULL_HANDLE;

	std::ifstream In(data(Path), std::ios::in | std::ios::binary);
	if (!In.fail()) {
		In.seekg(0, std::ios_base::end);
		const auto CodeSize = In.tellg();
		if (CodeSize) {
			In.seekg(0, std::ios_base::beg);

			std::vector<std::byte> Code(CodeSize);
			In.read(reinterpret_cast<char*>(data(Code)), size(Code));
			const VkShaderModuleCreateInfo SMCI = {
				.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
				.pNext = nullptr,
				.flags = 0,
				.codeSize = static_cast<size_t>(size(Code)), .pCode = reinterpret_cast<uint32_t*>(data(Code))
			};
			VERIFY_SUCCEEDED(vkCreateShaderModule(Device, &SMCI, GetAllocationCallbacks(), &ShaderModule));
		}
		In.close();
	}
	return ShaderModule;
}

void VK::CreatePipeline_(VkPipeline& PL, const VkDevice Dev, const VkPipelineLayout PLL, const VkRenderPass RP, 
	const VkPrimitiveTopology Topology, const uint32_t PatchControlPoints, 
	const VkPipelineRasterizationStateCreateInfo& PRSCI,
	const VkPipelineDepthStencilStateCreateInfo& PDSSCI,
	const VkPipelineShaderStageCreateInfo* VS, const VkPipelineShaderStageCreateInfo* FS, const VkPipelineShaderStageCreateInfo* TES, const VkPipelineShaderStageCreateInfo* TCS, const VkPipelineShaderStageCreateInfo* GS,
	const std::vector<VkVertexInputBindingDescription>& VIBDs, const std::vector<VkVertexInputAttributeDescription>& VIADs,
	const std::vector<VkPipelineColorBlendAttachmentState>& PCBASs,
	VkPipelineCache PC)
{
	PERFORMANCE_COUNTER();

	std::vector<VkPipelineShaderStageCreateInfo> PSSCIs;
	if (nullptr != VS) { PSSCIs.emplace_back(*VS); }
	if (nullptr != FS) { PSSCIs.emplace_back(*FS); }
	if (nullptr != TES) { PSSCIs.emplace_back(*TES); }
	if (nullptr != TCS) { PSSCIs.emplace_back(*TCS); }
	if (nullptr != GS) { PSSCIs.emplace_back(*GS); }
	assert(!empty(PSSCIs) && "");

	//!< �o�[�e�b�N�X�C���v�b�g (VertexInput)
	const VkPipelineVertexInputStateCreateInfo PVISCI = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
		.vertexBindingDescriptionCount = static_cast<uint32_t>(size(VIBDs)), .pVertexBindingDescriptions = data(VIBDs),
		.vertexAttributeDescriptionCount = static_cast<uint32_t>(size(VIADs)), .pVertexAttributeDescriptions = data(VIADs)
	};

	//!< DX�ł́u�g�|���W�v�Ɓu�p�b�`�R���g���[���|�C���g�v�̎w���IASetPrimitiveTopology()�̈����Ƃ��ăR�}���h���X�g�֎w�肷��AVK�Ƃ͌��\�قȂ�̂Œ���
	//!< (�u�p�b�`�R���g���[���|�C���g�v�̐��������w�肷�邩�ɂ�茈�܂�)
	//!< CommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	//!< CommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_1_CONTROL_POINT_PATCHLIST);

	//!< �C���v�b�g�A�Z���u�� (InputAssembly)
	const VkPipelineInputAssemblyStateCreateInfo PIASCI = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
		.topology = Topology,
		.primitiveRestartEnable = VK_FALSE
	};
	//!< WITH_ADJACENCY �n�g�p���ɂ� �f�o�C�X�t�B�[�`���[ geometryShader ���L���ł��邱��
	//assert((
	//	(PIASCI.topology != VK_PRIMITIVE_TOPOLOGY_LINE_LIST_WITH_ADJACENCY || PIASCI.topology != VK_PRIMITIVE_TOPOLOGY_LINE_STRIP_WITH_ADJACENCY || PIASCI.topology != VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST_WITH_ADJACENCY || PIASCI.topology != VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP_WITH_ADJACENCY)
	//	|| PDF.geometryShader) /*&& ""*/);
	//!< PATCH_LIST �g�p���ɂ� �f�o�C�X�t�B�[�`���[ tessellationShader ���L���ł��邱��
	//assert((PIASCI.topology != VK_PRIMITIVE_TOPOLOGY_PATCH_LIST || PDF.tessellationShader) && "");
	//!< �C���f�b�N�X 0xffffffff(VK_INDEX_TYPE_UINT32), 0xffff(VK_INDEX_TYPE_UINT16) ���v���~�e�B�u�̃��X�^�[�g�Ƃ���A�C���f�b�N�X�n�`��̏ꍇ(vkCmdDrawIndexed, vkCmdDrawIndexedIndirect)�̂ݗL��
	//!< LIST �n�g�p�� primitiveRestartEnable �����ł��邱��
	assert((
		(PIASCI.topology != VK_PRIMITIVE_TOPOLOGY_POINT_LIST || PIASCI.topology != VK_PRIMITIVE_TOPOLOGY_LINE_LIST || PIASCI.topology != VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST || PIASCI.topology != VK_PRIMITIVE_TOPOLOGY_LINE_LIST_WITH_ADJACENCY || PIASCI.topology != VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST_WITH_ADJACENCY || PIASCI.topology != VK_PRIMITIVE_TOPOLOGY_PATCH_LIST)
		|| PIASCI.primitiveRestartEnable == VK_FALSE) /*&& ""*/);

	//!< �e�Z���[�V���� (Tessellation)
	assert((Topology != VK_PRIMITIVE_TOPOLOGY_PATCH_LIST || PatchControlPoints != 0) && "");
	const VkPipelineTessellationStateCreateInfo PTSCI = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
		.patchControlPoints = PatchControlPoints //!< �p�b�`�R���g���[���|�C���g
	};

	//!< �r���[�|�[�g (Viewport)
	//!< VkDynamicState ���g�p���邽�߁A�����ł̓r���[�|�[�g(�V�U�[)�̌��̂ݎw�肵�Ă��� (To use VkDynamicState, specify only count of viewport(scissor) here)
	//!< ��� vkCmdSetViewport(), vkCmdSetScissor() �Ŏw�肷�� (Use vkCmdSetViewport(), vkCmdSetScissor() later)
	constexpr VkPipelineViewportStateCreateInfo PVSCI = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
		.viewportCount = 1, .pViewports = nullptr,
		.scissorCount = 1, .pScissors = nullptr
	};
	//!< 2�ȏ�̃r���[�|�[�g���g�p����ɂ̓f�o�C�X�t�B�[�`���[ multiViewport ���L���ł��邱�� (If use 2 or more viewport device feature multiViewport must be enabled)
	//!< �r���[�|�[�g�̃C���f�b�N�X�̓W�I���g���V�F�[�_�Ŏw�肷�� (Viewport index is specified in geometry shader)
	//assert((PVSCI.viewportCount <= 1 || PDF.multiViewport) && "");

	//!< PRSCI
	//!< FILL�ȊO�g�p���ɂ́A�f�o�C�X�t�B�[�`���[fillModeNonSolid���L���ł��邱��
	assert(PRSCI.polygonMode == VK_POLYGON_MODE_FILL && "");
	//!< 1.0f ���傫�Ȓl�ɂ́A�f�o�C�X�t�B�[�`���[widelines ���L���ł��邱��
	assert(PRSCI.lineWidth <= 1.0f&& "");

	//!< �}���`�T���v�� (Multisample)
	constexpr VkSampleMask SM = 0xffffffff; //!< 0xffffffff ���w�肷��ꍇ�́A����� nullptr �ł��悢
	constexpr VkPipelineMultisampleStateCreateInfo PMSCI = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
		.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
		.sampleShadingEnable = VK_FALSE, .minSampleShading = 0.0f,
		.pSampleMask = &SM,
		.alphaToCoverageEnable = VK_FALSE, .alphaToOneEnable = VK_FALSE
	};
	//assert((PMSCI.sampleShadingEnable == VK_FALSE || PDF.sampleRateShading) && "");
	assert((PMSCI.minSampleShading >= 0.0f && PMSCI.minSampleShading <= 1.0f) && "");
	//assert((PMSCI.alphaToOneEnable == VK_FALSE || PDF.alphaToOne) && "");

	//!< VK_BLEND_FACTOR_SRC1 �n�����g�p����ɂ́A�f�o�C�X�t�B�[�`���[ dualSrcBlend ���L���ł��邱��
	///!< SRC�R���|�[�l���g * SRC�t�@�N�^ OP DST�R���|�[�l���g * DST�t�@�N�^

	//!< �f�o�C�X�t�B�[�`���[ independentBlend ���L���Ŗ����ꍇ�́A�z��̊e�v�f�́u���S�ɓ����l�v�ł��邱�� (If device feature independentBlend is not enabled, each array element must be exactly same)
	//if (!PDF.independentBlend) {
	//	for (auto i : PCBASs) {
	//		assert(memcmp(&i, &PCBASs[0], sizeof(PCBASs[0])) == 0 && ""); //!< �ŏ��̗v�f�͔�ׂ�K�v�������܂�������
	//	}
	//}
	const VkPipelineColorBlendStateCreateInfo PCBSCI = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
		.logicOpEnable = VK_FALSE, .logicOp = VK_LOGIC_OP_COPY, //!< �u�����h���ɘ_���I�y���[�V�������s�� (�u�����h�͖����ɂȂ�) (�����^�A�^�b�`�����g�ɑ΂��Ă̂�)
		.attachmentCount = static_cast<uint32_t>(size(PCBASs)), .pAttachments = data(PCBASs),
		.blendConstants = { 1.0f, 1.0f, 1.0f, 1.0f }
	};

	//!< �_�C�i�~�b�N�X�e�[�g (DynamicState)
	constexpr std::array DSs = {
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR,
		//VK_DYNAMIC_STATE_DEPTH_BIAS,
	};
	constexpr VkPipelineDynamicStateCreateInfo PDSCI = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
		.dynamicStateCount = static_cast<uint32_t>(size(DSs)), .pDynamicStates = data(DSs)
	};

	/**
	@brief �p�� ... ���ʕ����������ꍇ�A�e�p�C�v���C�����w�肵�č쐬����Ƃ�荂���ɍ쐬�ł���A�e�q�Ԃł̃X�C�b�`��o�C���h���L��
	(DX �� D3D12_CACHED_PIPELINE_STATE ����?)
	basePipelineHandle, basePipelineIndex �͓����Ɏg�p�ł��Ȃ��A�g�p���Ȃ��ꍇ�͂��ꂼ�� VK_NULL_HANDLE, -1 ���w�肷�邱��
	�e�ɂ� VK_PIPELINE_CREATE_ALLOW_DERIVATIVES_BIT �t���O���K�v�A�q�ɂ� VK_PIPELINE_CREATE_DERIVATIVE_BIT �t���O���K�v
	�EbasePipelineHandle ... ���ɐe�Ƃ���p�C�v���C��(�n���h��)�����݂���ꍇ�Ɏw��
	�EbasePipelineIndex ... ���z����Őe�p�C�v���C���������ɍ쐬����ꍇ�A�z����ł̐e�p�C�v���C���̓Y��(�e�̓Y���̕����Ⴂ�l�ł��邱��)
	*/
	const std::array GPCIs = {
		VkGraphicsPipelineCreateInfo({
			.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
			.pNext = nullptr,
#ifdef _DEBUG
			.flags = VK_PIPELINE_CREATE_DISABLE_OPTIMIZATION_BIT,
#else
			.flags = 0,
#endif
			.stageCount = static_cast<uint32_t>(size(PSSCIs)), .pStages = data(PSSCIs),
			.pVertexInputState = &PVISCI,
			.pInputAssemblyState = &PIASCI,
			.pTessellationState = &PTSCI,
			.pViewportState = &PVSCI,
			.pRasterizationState = &PRSCI,
			.pMultisampleState = &PMSCI,
			.pDepthStencilState = &PDSSCI,
			.pColorBlendState = &PCBSCI,
			.pDynamicState = &PDSCI,
			.layout = PLL,
			.renderPass = RP, .subpass = 0, //!< �����Ŏw�肷�郌���_�[�p�X�́u�݊����̂�����́v�Ȃ��
			.basePipelineHandle = VK_NULL_HANDLE, .basePipelineIndex = -1
		})
	};
	//!< VK�ł�1�R�[���ŕ����̃p�C�v���C�����쐬���邱�Ƃ��ł��邪�ADX�ɍ��킹��1�������Ȃ����Ƃɂ��Ă���
	VERIFY_SUCCEEDED(vkCreateGraphicsPipelines(Dev, PC, static_cast<uint32_t>(size(GPCIs)), data(GPCIs), GetAllocationCallbacks(), &PL));

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

	const std::array<VkComputePipelineCreateInfo, 1> ComputePipelineCreateInfos = {
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
		static_cast<uint32_t>(size(ComputePipelineCreateInfos)), data(ComputePipelineCreateInfos),
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
	constexpr VkImageSubresourceRange ISR = {
		.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
		.baseMipLevel = 0, .levelCount = VK_REMAINING_MIP_LEVELS,
		.baseArrayLayer = 0, .layerCount = VK_REMAINING_ARRAY_LAYERS
	};
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
		ISR
	};
	vkCmdPipelineBarrier(CB,
		VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
		0,
		0, nullptr,
		0, nullptr,
		1, &ImageMemoryBarrier_PresentToTransfer);
	{
		//!< vkCmdClearColorImage() �̓����_�[�p�X���ł͎g�p�ł��Ȃ�
		vkCmdClearColorImage(CB, Img, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &Color, 1, &ISR);
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
		ISR
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
	const VkClearValue ClearValue = { .color = Color };
	const std::array ClearAttachments = {
		VkClearAttachment({
			VK_IMAGE_ASPECT_COLOR_BIT,
			0, //!< �J���[�A�^�b�`�����g�̃C���f�b�N�X #VK_TODO ���󌈂ߑł�
			ClearValue
		}),
	};
	const std::array ClearRects = {
		VkClearRect({
			ScissorRects[0],
			0, 1 //!< �J�n���C���ƃ��C���� #VK_TODO ���󌈂ߑł�
		}),
	};
	vkCmdClearAttachments(CB,
		static_cast<uint32_t>(size(ClearAttachments)), data(ClearAttachments),
		static_cast<uint32_t>(size(ClearRects)), data(ClearRects));
}
void VK::ClearDepthStencilAttachment(const VkCommandBuffer CB, const VkClearDepthStencilValue& DepthStencil)
{
	VkClearValue ClearValue;
	ClearValue.depthStencil = DepthStencil;
	const std::array<VkClearAttachment, 1> ClearAttachments = {
		{
			VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT,
			0, //!< �����ł͖��������
			ClearValue
		},
	};
	const VkRect2D ClearArea = { { 0, 0 }, SurfaceExtent2D };
	const std::array<VkClearRect, 1> ClearRects = {
		{
			ClearArea,
			0, 1 //!< �J�n���C���ƃ��C���� #VK_TODO ���󌈂ߑł�
		},
	};
	vkCmdClearAttachments(CB,
		static_cast<uint32_t>(size(ClearAttachments)), data(ClearAttachments),
		static_cast<uint32_t>(size(ClearRects)), data(ClearRects));
}

#if 0
void VK::PopulateCommandBuffer(const size_t i)
{
	//!< vkBeginCommandBuffer() �ňÖٓI�Ƀ��Z�b�g����邪�A�����I�Ƀ��Z�b�g����ꍇ�ɂ́u���������v�[���փ����[�X���邩�ǂ������w��ł���v
	//VERIFY_SUCCEEDED(vkResetCommandBuffer(CB, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT));

	//!< @brief VkCommandBufferUsageFlags
	//!< * VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT		... ��x�����g�p����ꍇ�█�񃊃Z�b�g����ꍇ�Ɏw��A���x���T�u�~�b�g������̂ɂ͎w�肵�Ȃ�
	//!< * VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT		... �f�o�C�X�ł܂����s����Ă���ԂɁA�R�}���h�o�b�t�@���ēx�T�u�~�b�g����K�v������ꍇ�Ɏw�� (�p�t�H�[�}���X�̊ϓ_����͔�����ׂ�)
	//!< * VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT	... �Z�J���_���R�}���h�o�b�t�@�ł������_�[�p�X���̏ꍇ�Ɏw�肷��
	const auto CB = CommandBuffers[i];
	constexpr VkCommandBufferBeginInfo CBBI = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		.pNext = nullptr,
		.flags = 0,
		.pInheritanceInfo = nullptr
	};
	VERIFY_SUCCEEDED(vkBeginCommandBuffer(CB, &CBBI)); {
		const auto RP = RenderPasses[0];
		const auto FB = Framebuffers[i];

		//!< �r���[�|�[�g�A�V�U�[
		vkCmdSetViewport(CB, 0, static_cast<uint32_t>(size(Viewports)), data(Viewports));
		vkCmdSetScissor(CB, 0, static_cast<uint32_t>(size(ScissorRects)), data(ScissorRects));

#ifdef _DEBUG
		//!< �����_�[�G���A�̍Œᗱ�x���m��
		VkExtent2D Granularity;
		vkGetRenderAreaGranularity(Device, RP, &Granularity);
		//!<�u�����̊��ł́v Granularity = { 1, 1 } �������̂łقڂȂ�ł����v�݂����A���ɂ���Ă͒��ӂ��K�v
		assert(ScissorRects[0].extent.width >= Granularity.width && ScissorRects[0].extent.height >= Granularity.height && "ScissorRect is too small");
#endif

#ifdef USE_MANUAL_CLEAR
		constexpr std::array CVs = { VkClearValue({}), VkClearValue({.depthStencil = {.depth = 1.0f, .stencil = 0 } }) };
#else
		constexpr std::array CVs = { VkClearValue({.color = Colors::SkyBlue }), VkClearValue({.depthStencil = {.depth = 1.0f, .stencil = 0 } }) };
#endif
		const VkRenderPassBeginInfo RPBI = {
			.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
			.pNext = nullptr,
			.renderPass = RP,
			.framebuffer = FB,
			.renderArea = VkRect2D({.offset = VkOffset2D({.x = 0, .y = 0 }), .extent = SurfaceExtent2D }), //!< �t���[���o�b�t�@�̃T�C�Y�ȉ����w��ł���
			.clearValueCount = static_cast<uint32_t>(size(CVs)), .pClearValues = data(CVs)
		};
		vkCmdBeginRenderPass(CB, &RPBI, VK_SUBPASS_CONTENTS_INLINE); {
		} vkCmdEndRenderPass(CB);

#ifdef USE_MANUAL_CLEAR
		ClearColor(CB, SwapchainImages[i], Colors::Blue);
#endif
	} VERIFY_SUCCEEDED(vkEndCommandBuffer(CB));
}
#endif

void VK::Draw()
{
	WaitForFence();

	//!< ���̃C���[�W���擾�ł���܂Ńu���b�N(�^�C���A�E�g�͎w��\)�A�擾�ł�������Ƃ����ăC���[�W�͒����ɖړI�Ɏg�p�\�Ƃ͌���Ȃ�
	//!< (�����Ŏw�肵���ꍇ)�g�p�\�ɂȂ�ƃt�F���X��Z�}�t�H���V�O�i�������
	//!< �����ł̓Z�}�t�H���w�肵�A���̃Z�}�t�H�̓T�u�~�b�g���Ɏg�p����(�T�u�~�b�g�����R�}���h���v���[���e�[�V������҂悤�Ɏw�����Ă���)
	//!<	VK_SUBOPTIMAL_KHR : �C���[�W�͎g�p�\�ł͂��邪�v���[���e�[�V�����G���W���ɂƂ��ăx�X�g�ł͂Ȃ����
	//!<	VK_ERROR_OUT_OF_DATE_KHR : �C���[�W�͎g�p�s�ōč쐬���K�v
	VERIFY_SUCCEEDED(vkAcquireNextImageKHR(Device, Swapchain, UINT64_MAX, NextImageAcquiredSemaphore, VK_NULL_HANDLE, &SwapchainImageIndex));

	DrawFrame(GetCurrentBackBufferIndex());
	
	Submit();
	
	Present();
}
void VK::Dispatch()
{
	//!< (Fence���w�肵��)�T�u�~�b�g�����R�}���h����������܂Ńu���b�L���O���đ҂�
	const std::array Fences = { ComputeFence };
	VERIFY_SUCCEEDED(vkWaitForFences(Device, static_cast<uint32_t>(size(Fences)), data(Fences), VK_TRUE, (std::numeric_limits<uint64_t>::max)()));
	vkResetFences(Device, static_cast<uint32_t>(size(Fences)), data(Fences));

	const auto& CB = CommandBuffers[0];
	const std::array SIs = {
		VkSubmitInfo({
			.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
			.pNext = nullptr,
			.waitSemaphoreCount = 0, .pWaitSemaphores = nullptr, .pWaitDstStageMask = nullptr,
			.commandBufferCount = 1, .pCommandBuffers = &CB/*ComputeCommandBuffers[0]*/,
			.signalSemaphoreCount = 0, .pSignalSemaphores = nullptr,
		}),
	};
	VERIFY_SUCCEEDED(vkQueueSubmit(ComputeQueue, static_cast<uint32_t>(size(SIs)), data(SIs), ComputeFence));
}
void VK::WaitForFence()
{
	//!< �T�u�~�b�g�����R�}���h�̊�����҂�
	const std::array Fences = { Fence };
	VERIFY_SUCCEEDED(vkWaitForFences(Device, static_cast<uint32_t>(size(Fences)), data(Fences), VK_TRUE, (std::numeric_limits<uint64_t>::max)()));
	vkResetFences(Device, static_cast<uint32_t>(size(Fences)), data(Fences));
}
void VK::Submit()
{
	//!< �R�}���h�͎w��̃p�C�v���C���X�e�[�W�ɓ��B����܂Ŏ��s����A�����ŃZ�}�t�H���V�O�i�������܂ő҂�
	const std::array WaitSems = { NextImageAcquiredSemaphore };
	const std::array WaitStages = { VkPipelineStageFlags(VK_PIPELINE_STAGE_TRANSFER_BIT) };
	assert(size(WaitSems) == size(WaitStages) && "Must be same size");
	//!< ���s����R�}���h�o�b�t�@
	const std::array CBs = { CommandBuffers[GetCurrentBackBufferIndex()], };
	//!< �������ɃV�O�i�������Z�}�t�H(RenderFinishedSemaphore)
	const std::array SigSems = { RenderFinishedSemaphore };
	const std::array SIs = {
		VkSubmitInfo({
			.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
			.pNext = nullptr,
			.waitSemaphoreCount = static_cast<uint32_t>(size(WaitSems)), .pWaitSemaphores = data(WaitSems), .pWaitDstStageMask = data(WaitStages), //!< ���C���[�W���擾�ł���(�v���[���g����)�܂ŃE�G�C�g
			.commandBufferCount = static_cast<uint32_t>(size(CBs)), .pCommandBuffers = data(CBs),
			.signalSemaphoreCount = static_cast<uint32_t>(size(SigSems)), .pSignalSemaphores = data(SigSems) //!< �`�抮����ʒm����
		}),
	};
	VERIFY_SUCCEEDED(vkQueueSubmit(GraphicsQueue, static_cast<uint32_t>(size(SIs)), data(SIs), Fence));
}
void VK::Present()
{
	//!< �����ɕ����̃v���[���g���\�����A1�̃X���b�v�`�F�C�������1�̂�
	const std::array Swapchains = { Swapchain };
	const std::array ImageIndices = { GetCurrentBackBufferIndex() };
	assert(size(Swapchains) == size(ImageIndices) && "Must be same");

	//!< �T�u�~�b�g���Ɏw�肵���Z�}�t�H(RenderFinishedSemaphore)��҂��Ă���v���[���g���s�Ȃ���
	const VkPresentInfoKHR PresentInfo = {
		.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
		.pNext = nullptr,
		.waitSemaphoreCount = 1, .pWaitSemaphores = &RenderFinishedSemaphore,
		.swapchainCount = static_cast<uint32_t>(size(Swapchains)), .pSwapchains = data(Swapchains), .pImageIndices = data(ImageIndices),
		.pResults = nullptr
	};
	VERIFY_SUCCEEDED(vkQueuePresentKHR(PresentQueue, &PresentInfo));
}
