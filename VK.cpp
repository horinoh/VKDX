#include "VK.h"

#ifdef _WINDOWS
#pragma comment(lib, "vulkan-1.lib")
#else
// "libvulkan.so.1"
#endif

#ifdef VK_NO_PROTOYYPES
	//!< グローバルレベル関数 (Global level functions)
#define VK_GLOBAL_PROC_ADDR(proc) PFN_vk ## proc VK::vk ## proc = VK_NULL_HANDLE;
#include "VKGlobalProcAddr.h"
#undef VK_GLOBAL_PROC_ADDR

	//!< インスタンスレベル関数 (Instance level functions)
#define VK_INSTANCE_PROC_ADDR(proc) PFN_vk ## proc VK::vk ## proc = VK_NULL_HANDLE;
#include "VKInstanceProcAddr.h"
#undef VK_INSTANCE_PROC_ADDR

	//!< デバイスレベル関数 (Device level functions)
#define VK_DEVICE_PROC_ADDR(proc) PFN_vk ## proc VK::vk ## proc = VK_NULL_HANDLE;
#include "VKDeviceProcAddr.h"
#undef VK_DEVICE_PROC_ADDR
#define VK_DEVICE_PROC_ADDR(proc) PFN_vk ## proc VK::vk ## proc = VK_NULL_HANDLE;
#include "VKDeviceProcAddr_RayTracing.h"
#undef VK_DEVICE_PROC_ADDR
#endif //!< VK_NO_PROTOYYPES

#ifdef USE_DEBUG_REPORT
	//!< インスタンスレベル関数(Debug) (Instance level functions(Debug))
#define VK_INSTANCE_PROC_ADDR(proc) PFN_vk ## proc ## EXT VK::vk ## proc = VK_NULL_HANDLE;
#include "VKInstanceProcAddr_DebugReport.h"
#undef VK_INSTANCE_PROC_ADDR
#endif

	//!< デバイスレベル関数(Debug) (Device level functions(Debug))
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

	//!< インスタンス、デバイス
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

	//!< ジオメトリ (バーテックスバッファ、インデックスバッファ、アクセラレーションストラクチャ等)
	CreateGeometry();

	//!< ユニフォームバッファ (コンスタントバッファ相当)
	CreateUniformBuffer();

	//!< テクスチャ(レンダーターゲットテクスチャの場合フレームバッファよりも前に必要になる)
	CreateTexture();

	//!< イミュータブルサンプラはこの時点(CreateDescriptorSetLayout()より前)で必要
	CreateImmutableSampler();
	//!< パイプラインレイアウト (ルートシグネチャ相当)
	CreatePipelineLayout();

	//!< レンダーパス (DXには存在しない)
	CreateRenderPass();

	//!< パイプライン
	CreatePipeline();

	//!< フレームバッファ (DXには存在しない)
	CreateFramebuffer();

	//!< デスクリプタセット
	CreateDescriptorSet();
	//!< サンプラ ... DXがデスクリプタを必要とするので付き合いでここにする
	CreateSampler();

	//!< デスクリプタセット更新 ... この時点でデスクリプタセット、ユニフォームバッファ、イメージビュー、サンプラ等が必要
	UpdateDescriptorSet();

	SetTimer(hWnd, NULL, Elapse, nullptr);

	//!< ウインドウサイズ変更時に作り直すもの
	OnExitSizeMove(hWnd, hInstance);
}

/**
@note 殆どのものを壊して作り直さないとダメ #VK_TODO
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

	//!< デバイスがアイドルになるまで待つ
	if (VK_NULL_HANDLE != Device) [[likely]] {
		VERIFY_SUCCEEDED(vkDeviceWaitIdle(Device));
	}

#if 0
	//!< スワップチェインの作り直し
	CreateSwapchain(GetCurrentPhysicalDevice(), Surface, Rect);

	//!< デプスバッファの破棄、作成

	//!< フレームバッファの破棄、作成
	for (auto i : Framebuffers) {
		vkDestroyFramebuffer(Device, i, GetAllocationCallbacks());
	}
	Framebuffers.clear();
	//CreateFramebuffer();
#endif

	CreateViewport(static_cast<float>(SurfaceExtent2D.width), static_cast<float>(SurfaceExtent2D.height));

#if 0
	//!< コマンドバッファのリセット vkBeginCommandBuffer() で暗黙的にリセットされるので不要？
	for (auto i : SecondaryCommandBuffers) { VERIFY_SUCCEEDED(vkResetCommandBuffer(i, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT)); }
	for (auto i : CommandBuffers) { VERIFY_SUCCEEDED(vkResetCommandBuffer(i, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT)); }
#endif

	//!< ビューポートサイズが決定してから
	LoadScene();
	for (auto i = 0; i < size(CommandBuffers); ++i) {
		PopulateCommandBuffer(i);
	}
}

void VK::OnDestroy(HWND hWnd, HINSTANCE hInstance)
{
	Super::OnDestroy(hWnd, hInstance);

	if (VK_NULL_HANDLE != Device) [[likely]] {
		//!< デバイスのキューにサブミットされた全コマンドが完了するまでブロッキング、主に終了処理に使う (Wait for all command submitted to queue, usually used on finalize)
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

	//!< VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT を指定した場合のみ個別に開放できる (Only if VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT is used, can be release individually)
	//if (!empty(DescriptorSets))  [[likely]] {
	//	vkFreeDescriptorSets(Device, DescriptorPool, static_cast<uint32_t>(size(DescriptorSets)), data(DescriptorSets));
	//}
	DescriptorSets.clear();

	//!< このプールから確保された全てのデスクリプタセットを解放する、ここでは次のステップでプール自体を破棄しているのでやらなくても良い
	//!< (デスクリプタセットを個々に解放するのが面倒な場合や、プール自体は破棄したくない場合に使用)
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

	//!< SwapchainImages は取得したもの、破棄しない
	
	if (VK_NULL_HANDLE != Swapchain) [[likely]] {
		vkDestroySwapchainKHR(Device, Swapchain, GetAllocationCallbacks());
		Swapchain = VK_NULL_HANDLE;
	}

	//!< コマンドプール破棄時にコマンドバッファは暗黙的に解放されるのでやらなくても良い (Command buffers will be released implicitly, when command pool released)
	//if(!empty(SecondaryCommandBuffers)) [[likely]] { vkFreeCommandBuffers(Device, SecondaryCommandPools[0], static_cast<uint32_t>(size(SecondaryCommandBuffers)), data(SecondaryCommandBuffers)); SecondaryCommandBuffers.clear(); }	
	for (auto i : SecondaryCommandPools) {
		vkDestroyCommandPool(Device, i, GetAllocationCallbacks());
	}
	SecondaryCommandPools.clear();

	//!< コマンドプール破棄時にコマンドバッファは暗黙的に解放されるのでやらなくても良い (Command buffers will be released implicitly, when command pool released)
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

	//!< キューは論理デバイスと共に破棄される

	if (VK_NULL_HANDLE != Device) [[likely]] {
		vkDestroyDevice(Device, GetAllocationCallbacks());
		Device = VK_NULL_HANDLE;
	}
	
	//!< PhysicalDevice は vkEnumeratePhysicalDevices() で取得したもの、破棄しない

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

//!< @brief メモリプロパティフラグを(サポートされているかチェックしつつ)メモリタイプへ変換
//!< @param 物理デバイスのメモリプロパティ
//!< @param バッファやイメージの要求するメモリタイプ
//!< @param 希望のメモリプロパティフラグ
//!< @return メモリタイプ
uint32_t VK::GetMemoryTypeIndex(const VkPhysicalDeviceMemoryProperties& PDMP, const uint32_t TypeBits, const VkMemoryPropertyFlags MPF)
{
	for (uint32_t i = 0; i < PDMP.memoryTypeCount; ++i) {
		if (TypeBits & (1 << i)) {
			if ((PDMP.memoryTypes[i].propertyFlags & MPF) == MPF) { //!< 指定フラグが「全て」立っていないとダメ
			//if (PDMP.memoryTypes[i].propertyFlags & MPF) {		//!< 指定フラグが「一つでも」立っていれば良い
				return i;
			}
		}
	}
	DEBUG_BREAK();
	return 0xffff;
}

void VK::CreateBuffer(VkBuffer* Buffer, const VkBufferUsageFlags Usage, const size_t Size) const
{
	//!< バッファは作成時に指定した使用法でしか使用できない、ここでは VK_SHARING_MODE_EXCLUSIVE 決め打ちにしている #VK_TODO (Using VK_SHARING_MODE_EXCLUSIVE here)
	//!< VK_SHARING_MODE_EXCLUSIVE	: 複数ファミリのキューが同時アクセスできない、他のファミリからアクセスしたい場合はオーナーシップの移譲が必要
	//!< VK_SHARING_MODE_CONCURRENT	: 複数ファミリのキューが同時アクセス可能、オーナーシップの移譲も必要無し、パフォーマンスは悪い
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
	//!< Usage に VK_IMAGE_USAGE_SAMPLED_BIT が指定されいる場合、フォーマットやフィルタが使用可能かチェック #VK_TODO ここではリニアフィルタ決め打ち
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
		.tiling = VK_IMAGE_TILING_OPTIMAL, //!< リニアはパフォーマンスが悪いので、ここでは OPTIMAL に決め打ちしている
		.usage = Usage,
		.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
		.queueFamilyIndexCount = static_cast<uint32_t>(size(QueueFamilyIndices)), .pQueueFamilyIndices = data(QueueFamilyIndices),
		.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED //!< 作成時に指定できるのは UNDEFINED, PREINITIALIZED のみ、実際に使用する前にレイアウトを変更する必要がある
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
			//!< メモリコンテンツが変更されたことをドライバへ知らせる(vkMapMemory()した状態でやること)
			//!< デバイスメモリ確保時に VK_MEMORY_PROPERTY_HOST_COHERENT_BIT を指定した場合は必要ない CreateDeviceMemory(..., VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
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
	//!< vkCmdClearColorImage() はレンダーパス内では使用できない
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

//!< @param コマンドバッファ
//!< @param コピー元バッファ
//!< @param コピー先バッファ
//!< @param (コピー後の)バッファのアクセスフラグ ex) VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT, VK_ACCESS_INDEX_READ_BIT, VK_ACCESS_INDIRECT_READ_BIT,...等
//!< @param (コピー後に)バッファが使われるパイプラインステージ ex) VK_PIPELINE_STAGE_VERTEX_INPUT_BIT, VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT,...等
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
	//!< コマンド開始 (Begin command)
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
		//!< イメージメモリバリア (Image memory barrier)
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
			//!< イメージバッファ間コピーコマンド (Image to buffer copy command)
			vkCmdCopyImageToBuffer(CB, Src, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, Dst, static_cast<uint32_t>(size(BICs)), data(BICs));
		}
		//!< イメージメモリバリア (Image memory barrier)
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
	//!< キューにサブミットされたコマンドが完了するまでブロッキング (フェンスを用いないブロッキング方法)
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

//!< Cubemapを作る場合、まずレイヤ化されたイメージを作成し、(イメージビューを用いて)レイヤをフェイスとして扱うようハードウエアに伝える
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
		//!< カラーの場合 (In case color)
		if (!(FP.optimalTilingFeatures & VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT)) {
			Error("VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT not supported\n");
			DEBUG_BREAK();
		}
		//!< デプスステンシルの場合 (In case depth stencil)
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
		//!< #VK_TODO アトミック使用時のみチェックする
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
			//!< アトミック操作が保証されているのは VK_FORMAT_R32_UINT, VK_FORMAT_R32_SINT のみ、それ以外でやりたい場合はサポートされているか調べる必要がある
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
//!< Vulkanローダーが(引数で渡したデバイスに基いて)適切な実装へ関数コールをリダイレクトする必要がある、このリダイレクトには時間がかかりパフォーマンスに影響する
//!< 以下のようにすると、使用したいデバイスから直接関数をロードするため、リダイレクトをスキップできパフォーマンスを改善できる
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
	
	//!< グローバルレベルの関数をロードする Load global level functions
#define VK_GLOBAL_PROC_ADDR(proc) vk ## proc = reinterpret_cast<PFN_vk ## proc>(vkGetInstanceProcAddr(nullptr, "vk" #proc)); assert(nullptr != vk ## proc && #proc);
#include "VKGlobalProcAddr.h"
#undef VK_GLOBAL_PROC_ADDR
}
#endif

void VK::CreateInstance()
{
	//!< ここでは最新バージョンで動くようにしておく (Use latest version here)
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
	
	//!< インスタンスレベルのレイヤー、エクステンションの列挙
	EnumerateInstanceLayerProperties();

	const std::array Layers = {
		"VK_LAYER_KHRONOS_validation", //!< "VK_LAYER_LUNARG_standard_validation" は非推奨となった (is deprecated)
		//"VK_LAYER_LUNARG_api_dump",
#ifdef USE_RENDERDOC
		"VK_LAYER_RENDERDOC_Capture",
#endif
		"VK_LAYER_LUNARG_monitor", //!< タイトルバーにFPSを表示
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
		//!< デバッグレポート用
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

	//!< インスタンスレベルの関数をロードする Load instance level functions
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

	//!< バージョンチェック Check version
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
	//!< 物理デバイス(GPU)の列挙
	uint32_t Count = 0;
	VERIFY_SUCCEEDED(vkEnumeratePhysicalDevices(Inst, &Count, nullptr));
	assert(Count && "Physical device not found");
	PhysicalDevices.resize(Count);
	VERIFY_SUCCEEDED(vkEnumeratePhysicalDevices(Inst, &Count, data(PhysicalDevices)));

	Log("\tPhysicalDevices\n");
	for (const auto& i : PhysicalDevices) {
		//!< プロパティ (Property)
		VkPhysicalDeviceProperties PDP;
		vkGetPhysicalDeviceProperties(i, &PDP);
		EnumeratePhysicalDeviceProperties(PDP);

#if 0
		//!< フィーチャー (Feature)
		VkPhysicalDeviceFeatures PDF;
		vkGetPhysicalDeviceFeatures(i, &PDF);
		EnumeratePhysicalDeviceFeatures(PDF);
#else
		//!< フィーチャー2 (Feature2)
		{
			//!< 取得したい全てのフィーチャーを VkPhysicalDeviceFeatures2.pNext へチェイン指定する
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

		//!< メモリプロパティ (MemoryProperty)
		VkPhysicalDeviceMemoryProperties PDMP;
		vkGetPhysicalDeviceMemoryProperties(i, &PDMP);
		EnumeratePhysicalDeviceMemoryProperties(PDMP);

		//!< デバイスレベルのレイヤー、エクステンションの列挙
		EnumeratePhysicalDeviceLayerProperties(i);

		Log("\n");
	}

	PhysicalDeviceProperties.resize(size(PhysicalDevices));
	for (size_t i = 0; i < size(PhysicalDevices); ++i) {
		vkGetPhysicalDeviceMemoryProperties(PhysicalDevices[i], &PhysicalDeviceProperties[i]);
	}

	//!< ここでは最初の物理デバイスを選択することにする (Use first device here) #VK_TODO
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
//	//!< VkPhysicalDeviceFeatures には可能なフィーチャーが全て true になったものが渡されてくる
//	//!< 不要な項目を false にオーバーライドするとパフォーマンスの改善が期待できるかもしれない
//	//!< false で渡されてくる項目は使えないフィーチャーなので true に変えても使えない
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

	//!< キューファミリプロパティの列挙
	uint32_t Count = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(PD, &Count, nullptr);
	assert(Count && "QueueFamilyProperty not found");
	QFPs.resize(Count);
	vkGetPhysicalDeviceQueueFamilyProperties(PD, &Count, data(QFPs));

	Log("\tQueueFamilyProperties\n");
#define QUEUE_FLAG_ENTRY(entry) if(VK_QUEUE_##entry##_BIT & QFPs[i].queueFlags) { Logf("%s | ", #entry); }
	//!< インデックスを使うので範囲ベースforにはしない
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

	//!< 機能を持つキューファミリインデックスを見つける (Find queue family index for each functions)
	GraphicsQueueFamilyIndex = FindQueueFamilyPropertyIndex(VK_QUEUE_GRAPHICS_BIT, QFPs);
	PresentQueueFamilyIndex = FindQueueFamilyPropertyIndex(PD, Sfc, QFPs);
	ComputeQueueFamilyIndex = FindQueueFamilyPropertyIndex(VK_QUEUE_COMPUTE_BIT, QFPs);

	//!< キューファミリ内でのインデックス及びプライオリティ、ここではグラフィック、プレゼント、コンピュートの分をプライオリティ0.5fで追加している
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

	//!< キュー作成情報 (Queue create information)
	std::vector<VkDeviceQueueCreateInfo> DQCIs;
	//!< インデックスを使うので範囲ベースforにはしない
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
		//!< スワップチェインはプラットフォームに特有の機能なのでデバイス作製時に VK_KHR_SWAPCHAIN_EXTENSION_NAME エクステンションを有効にして作成しておく
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
	//!< レイトレーシング拡張は動的に判断する(サポートしないGPUがまだ多く存在するため)、他の拡張はコンパイルディレクティブ対応でまあいいか(大抵のGPUでサポートされるだろう)
	if (HasRayTracingSupport(PD)) {
		Extensions.emplace_back(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME);
		Extensions.emplace_back(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
		Extensions.emplace_back(VK_KHR_RAY_QUERY_EXTENSION_NAME);
	}
#pragma endregion

	//!< サポートされるフィーチャーを全て有効にしている、パフォーマンス的には不必要なものはオフにした方が良い #PERFORMANCE_TODO
	VkPhysicalDeviceFeatures PDF; vkGetPhysicalDeviceFeatures(PD, &PDF);
	//constexpr VkPhysicalDeviceFeatures PDF = { .geometryShader = VK_TRUE, .tessellationShader = VK_TRUE };
	const VkDeviceCreateInfo DCI = {
		.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
		.queueCreateInfoCount = static_cast<uint32_t>(size(DQCIs)), .pQueueCreateInfos = data(DQCIs),
		.enabledLayerCount = 0, .ppEnabledLayerNames = nullptr, //!< デバイスでレイヤーの有効化は非推奨 (Device layer is deprecated)
		.enabledExtensionCount = static_cast<uint32_t>(size(Extensions)), .ppEnabledExtensionNames = data(Extensions),
		.pEnabledFeatures = &PDF
	};
	VERIFY_SUCCEEDED(vkCreateDevice(PD, &DCI, GetAllocationCallbacks(), &Device));

	//!< デバイスレベルの関数をロードする (Load device level functions)
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

	//!< グラフィック、プレゼントキューは同じインデックスの場合もあるが別の変数に取得しておく (Graphics and presentation index may be same, but save to individual variables)
	vkGetDeviceQueue(Device, GraphicsQueueFamilyIndex, GraphicsQueueIndexInFamily, &GraphicsQueue);
	vkGetDeviceQueue(Device, PresentQueueFamilyIndex, PresentQueueIndexInFamily, &PresentQueue);
	vkGetDeviceQueue(Device, ComputeQueueFamilyIndex, ComputeQueueIndexInFamily, &ComputeQueue);
	//vkGetDeviceQueue(Device, TransferQueueFamilyIndex, TransferQueueIndex, &TransferQueue);
	//vkGetDeviceQueue(Device, SparceBindingQueueFamilyIndex, SparceBindingQueueIndex, &SparceBindingQueue);

	LOG_OK();
}

#if 0
//!< 実装の要求するアラインを保証して確保する(イメージに128byte、バッファに64byteのアラインが必要な環境なら、128byteアラインで確保される)
//!< 同じメモリオブジェクトから、(アラインの)異なるタイプ再確保が可能
void VK::AllocateDeviceMemory()
{
	VkPhysicalDeviceMemoryProperties PDMP;
	vkGetPhysicalDeviceMemoryProperties(GetCurrentPhysicalDevice(), &PDMP);
	if (PDMP.memoryHeapCount) {
		DeviceMemories.assign(PDMP.memoryHeapCount, VK_NULL_HANDLE);

		for (uint32_t i = 0; i < PDMP.memoryTypeCount; ++i) {
			//!< 取り敢えずプロパティフラグが0のものは対象としない(環境依存がありそう注意 #VK_TODO) (Targeting propertyFlags!=0 here)
			if (PDMP.memoryTypes[i].propertyFlags) {
				const auto HeapIndex = PDMP.memoryTypes[i].heapIndex;
				//!< 確保するデバイスメモリサイズ (Device memory size to use) #VK_TODO
				//!< 最大値で取るとVK_ERROR_OUT_OF_DEVICE_MEMORYになる、あまりに大きく取ると重くなる
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

//!< ホストとデバイスの同期 (Synchronization between host and device)
//!< サブミット(vkQueueSubmit) に使用し、Draw()やDispatch()の頭でシグナル(サブミットされたコマンドの完了)を待つ (Used when submit, and wait signal on top of Draw())
//!< 初回と２回目以降を同じに扱う為に、シグナル済み状態(VK_FENCE_CREATE_SIGNALED_BIT)で作成している (Create with signaled state, to do same operation on first time and second time)
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

//!< キュー内での同期(異なるキュー間の同期も可能) (Synchronization internal queue)
//!< イメージ取得(vkAcquireNextImageKHR)、サブミット(VkSubmitInfo)、プレゼンテーション(VkPresentInfoKHR)に使用する (Use when image acquire, submit, presentation) 
void VK::CreateSemaphore(VkDevice Dev)
{
	const VkSemaphoreCreateInfo SCI = {
		.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0
	};

	//!< プレゼント完了同期用 (Wait for presentation finish)
	VERIFY_SUCCEEDED(vkCreateSemaphore(Dev, &SCI, GetAllocationCallbacks(), &NextImageAcquiredSemaphore));

	//!< 描画完了同期用 (Wait for render finish)
	VERIFY_SUCCEEDED(vkCreateSemaphore(Dev, &SCI, GetAllocationCallbacks(), &RenderFinishedSemaphore));

	LOG_OK();
}

void VK::AllocateCommandBuffer()
{
	//!< キューファミリが異なる場合は別のコマンドプールを用意する必要がある、そのキューにのみサブミットできる
	//!< 複数スレッドで同時にレコーディングするには、別のコマンドプールからアロケートされたコマンドバッファである必要がある (コマンドプールは複数スレッドからアクセス不可)
	//!< VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT	: コマンドバッファ毎にリセットが可能、指定しない場合はプール毎にまとめてリセット (コマンドバッファのレコーディング開始時に暗黙的にリセットされるので注意)
	//!< VK_COMMAND_POOL_CREATE_TRANSIENT_BIT				: 短命で、何度もサブミットしない、すぐにリセットやリリースされる場合に指定
	//!< (ここでは)プライマリ用1つ、セカンダリ用1つのコマンドプール作成をデフォルト実装とする
	const VkCommandPoolCreateInfo CPCI = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
		.pNext = nullptr,
		.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
		.queueFamilyIndex = GraphicsQueueFamilyIndex
	};
	VERIFY_SUCCEEDED(vkCreateCommandPool(Device, &CPCI, GetAllocationCallbacks(), &CommandPools.emplace_back()));
	//!< セカンダリ用 : 必ずしも別プールにする必要は無いがここでは別プールとしておく
	VERIFY_SUCCEEDED(vkCreateCommandPool(Device, &CPCI, GetAllocationCallbacks(), &SecondaryCommandPools.emplace_back()));

	//!< VK_COMMAND_BUFFER_LEVEL_PRIMARY	: 直接キューにサブミットできる、セカンダリをコールできる (Can be submit, can execute secondary)
	//!< VK_COMMAND_BUFFER_LEVEL_SECONDARY	: サブミットできない、プライマリから実行されるのみ (Cannot submit, only executed from primary)
	//!< ここではデフォルト実装として、プライマリ、セカンダリ共にスワップチェイン数分用意することとする
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

	//!< ここでは最初に見つかった UNDEFINED でないものを選択している (Select first format but UNDEFINED here)
	const auto SelectedIndex = [&]() {
		//!< 要素が 1 つのみで UNDEFINED の場合、制限は無く好きなものを選択できる (If there is only 1 element and which is UNDEFINED, we can choose any)
		if (1 == size(SFs) && VK_FORMAT_UNDEFINED == SFs[0].format) {
			return -1;
		}
		for (auto i = 0; i < size(SFs); ++i) {
#ifdef USE_HDR
			switch (SFs[i].colorSpace)
			{
			default: break;
				//!< HDR対応のディスプレイの場合、以下が返ることが期待される
			case VK_COLOR_SPACE_HDR10_HLG_EXT:
			case VK_COLOR_SPACE_HDR10_ST2084_EXT:
			case VK_COLOR_SPACE_DOLBYVISION_EXT:
				if (VK_FORMAT_UNDEFINED != SFs[i].format) {
					return i;
				}
			}
#else
			//!< VK_FORMAT_UNDEFINED でない最初のもの
			if (VK_FORMAT_UNDEFINED != SFs[i].format) {
				return i;
			}
#endif
		}

		//!< ここに来てはいけない
#ifdef USE_HDR
		assert(false && "HDR not supported");
#else
		assert(false && "Valid surface format not found");
#endif
		return 0;
	}();

	//!< ColorSpace はハードウェア上でのカラーコンポーネントの表現(リニア、ノンリニア、エンコード、デコード等)
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
		//!< 0xffffffff の場合はスワップチェインイメージサイズがウインドウサイズを決定することになる (If 0xffffffff, size of swapchain image determines the size of the window)
		//!< 引数の Width, Height を使用する (In this case, use argument Width and Height) 
		return VkExtent2D({ .width = (std::clamp)(Width, SC.maxImageExtent.width, SC.minImageExtent.width), .height = (std::clamp)(Height, SC.minImageExtent.height, SC.minImageExtent.height) });
	}
	else {
		//!< そうでない場合は currentExtent を使用する (Otherwise, use currentExtent)
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

	//!< 可能なら VK_PRESENT_MODE_MAILBOX_KHR を選択、そうでなければ VK_PRESENT_MODE_FIFO_KHR を選択 (Want to select VK_PRESENT_MODE_MAILBOX_KHR, or select VK_PRESENT_MODE_FIFO_KHR)
	/**
	@brief VkPresentModeKHR
	* VK_PRESENT_MODE_IMMEDIATE_KHR		... vsyncを待たないのでテアリングが起こる (Tearing happen, no vsync wait)
	* VK_PRESENT_MODE_MAILBOX_KHR		... キューは 1 つで常に最新で上書きされる、vsyncで更新される (Queue is 1, and always update to new image and updated on vsync)
	* VK_PRESENT_MODE_FIFO_KHR			... VulkanAPI が必ずサポートする vsyncで更新 (VulkanAPI always support this, updated on vsync)
	* VK_PRESENT_MODE_FIFO_RELAXED_KHR	... FIFOに在庫がある場合は vsyncを待つが、間に合わない場合は即座に更新されテアリングが起こる (If FIFO is not empty wait vsync. but if empty, updated immediately and tearing will happen)
	*/
	const VkPresentModeKHR SelectedPresentMode = [&]() {
		for (auto i : PMs) {
			if (VK_PRESENT_MODE_MAILBOX_KHR == i) {
				//!< 可能なら MAILBOX (If possible, want to use MAILBOX)
				return i;
			}
		}
		for (auto i : PMs) {
			if (VK_PRESENT_MODE_FIFO_KHR == i) {
				//!< FIFO は VulkanAPI が必ずサポートする (VulkanAPI always support FIFO)
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

//!< 手動でクリアする場合には VkImageUsageFlags に追加で VK_IMAGE_USAGE_TRANSFER_DST_BIT の指定が必要
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

	//!< 最低よりも1枚多く取りたい、ただし最大値でクランプする(maxImageCount が0の場合は上限無し)
	const auto ImageCount = (std::min)(SC.minImageCount + 1, 0 == SC.maxImageCount ? UINT32_MAX : SC.maxImageCount);
	Logf("\t\t\tImagCount = %d\n", ImageCount);

	//!< サーフェスのフォーマットを選択
	const auto SurfaceFormat = SelectSurfaceFormat(PD, Sfc);
	ColorFormat = SurfaceFormat.format; //!< カラーファーマットは覚えておく

	//!< サーフェスのサイズを選択
	SurfaceExtent2D = SelectSurfaceExtent(SC, Width, Height);
	Logf("\t\t\tSurfaceExtent = %d x %d\n", SurfaceExtent2D.width, SurfaceExtent2D.height);

	//!< レイヤー、ステレオレンダリング等をしたい場合は1以上になるが、ここでは1
	uint32_t ImageArrayLayers = 1;

	//!< サポートされないImageUsageFlagが指定された
	assert((IUF & SC.supportedUsageFlags) && "Specified ImageUsageFlag is not supported");

	//!< グラフィックとプレゼントのキューファミリが異なる場合はキューファミリインデックスの配列が必要、また VK_SHARING_MODE_CONCURRENT を指定すること
	//!< (ただし VK_SHARING_MODE_CONCURRENT にするとパフォーマンスが落ちる場合がある)
	std::vector<uint32_t> QueueFamilyIndices;
	if (GraphicsQueueFamilyIndex != PresentQueueFamilyIndex) {
		QueueFamilyIndices.emplace_back(GraphicsQueueFamilyIndex);
		QueueFamilyIndices.emplace_back(PresentQueueFamilyIndex);
	}

	//!< サーフェスを回転、反転等させるかどうか (Rotate, mirror surface or not)
	const auto SurfaceTransform = (VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR & SC.supportedTransforms) ? VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR : SC.currentTransform;

	//!< サーフェスのプレゼントモードを選択
	const auto SurfacePresentMode = SelectSurfacePresentMode(PD, Sfc);

	//!< 既存のは後で開放するので OldSwapchain に覚えておく (セッティングを変更してスワップチェインを再作成する場合等に備える)
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

	//!< (あれば)前のやつは破棄
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
	//!< #VK_TODO スワップチェインのリサイズ対応
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
	//!< プレゼンテーションエンジンからイメージを取得する際、ハンドルではなく vkGetSwapchainImagesKHR() で取得したインデックスが返るので、ここで取得した順序は重要になる
	uint32_t Count;
	VERIFY_SUCCEEDED(vkGetSwapchainImagesKHR(Dev, SC, &Count, nullptr));
	assert(Count && "Swapchain image count is zero");
	SwapchainImages.clear();
	SwapchainImages.resize(Count);
	VERIFY_SUCCEEDED(vkGetSwapchainImagesKHR(Dev, SC, &Count, data(SwapchainImages)));

	LOG_OK();
}
/**
@note Vulaknでは、1つのコマンドバッファで複数のスワップチェインイメージをまとめて処理できるっぽい
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
					//!<「現在のレイアウト」または「UNDEFINED」を指定すること、イメージコンテンツを保持したい場合は「UNDEFINED」はダメ、プレゼンテーション可能な VK_IMAGE_LAYOUT_PRESENT_SRC_KHR へ    
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
				//!< クリアカラーが指定されている場合 (If clear color is specified)
				const VkImageMemoryBarrier ImageMemoryBarrier_UndefinedToTransfer = {
					.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
					.pNext = nullptr,
					.srcAccessMask = 0, .dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
					//!<「現在のレイアウト」または「UNDEFINED」を指定すること、デスティネーションへ
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
					//!< デスティネーションから、プレゼンテーション可能な VK_IMAGE_LAYOUT_PRESENT_SRC_KHR へ
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

	//!< VK_IMAGE_LAYOUT_UNDEFINED で作成されているので、レイアウトを変更する必要がある
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
			ImageSubresourceRange_DepthStencil/*ImageSubresourceRange_Depth : デプスのみのフォーマットの場合*/
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
	//!< Vulkan はデフォルトで「左上」が原点 (DirectX、OpenGLは「左下」が原点)
	Viewports = {
		VkViewport({
			//!< USE_VIEWPORT_Y_UP
			//!< VKではデフォルトで「Yが下」を向くが、高さに負の値を指定すると「Yが上」を向き、DXと同様になる (In VK, by specifying negative height, Y become up. same as DX)
			//!< 通常基点は「左上」を指定するが、高さに負の値を指定する場合は「左下」を指定すること (When negative height, specify left bottom as base, otherwise left up)
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
	//!< offset, extent で指定 (left, top, right, bottomで指定のDXとは異なるので注意)
	ScissorRects = {
		VkRect2D({.offset = VkOffset2D({.x = 0, .y = 0 }), .extent = VkExtent2D({.width = static_cast<uint32_t>(Width), .height = static_cast<uint32_t>(Height) }) }),
	};

	LOG_OK();
}
void VK::CreateBufferMemory(VkBuffer* Buffer, VkDeviceMemory* DeviceMemory, const VkDevice Device, const VkPhysicalDeviceMemoryProperties PDMP, const VkBufferUsageFlags BUF, const size_t Size, const VkMemoryPropertyFlags MPF, const void* Source)
{
	assert(Size && "");
#pragma region BUFFER
	//!< バッファは作成時に指定した使用法でしか使用できない、ここでは VK_SHARING_MODE_EXCLUSIVE 決め打ちにしている #VK_TODO (Using VK_SHARING_MODE_EXCLUSIVE here)
	//!< VK_SHARING_MODE_EXCLUSIVE	: 複数ファミリのキューが同時アクセスできない、他のファミリからアクセスしたい場合はオーナーシップの移譲が必要
	//!< VK_SHARING_MODE_CONCURRENT	: 複数ファミリのキューが同時アクセス可能、オーナーシップの移譲も必要無し、パフォーマンスは悪い
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
	//!< VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT が指定された場合に pNext へ指定する
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

			//!< メモリコンテンツが変更されたことを明示的にドライバへ知らせる(vkMapMemory()した状態でやること)
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
	//!< ホストビジブルバッファ、デバイスメモリを作成 (Create host visible buffer, device memory)
	StagingBuffer.Create(Device, GetCurrentPhysicalDeviceMemoryProperties(), VK_BUFFER_USAGE_TRANSFER_SRC_BIT, Size, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, Source);	

	
	{
		constexpr VkCommandBufferBeginInfo CBBI = {
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
			.pNext = nullptr,
			.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
			.pInheritanceInfo = nullptr
		};
		VERIFY_SUCCEEDED(vkBeginCommandBuffer(CB, &CBBI)); {
			//!< ホストビジブルからデバイスローカルへのコピーコマンドを発行 (Submit host visible to device local copy command)
			PopulateCommandBuffer_CopyBufferToBuffer(CB, StagingBuffer.Buffer, Buf, AF, PSF, Size);
		} VERIFY_SUCCEEDED(vkEndCommandBuffer(CB));
		SubmitAndWait(Queue, CB);
	}
}
void VK::CreateBufferMemoryAndSubmitTransferCommand(VkBuffer* Buffer, VkDeviceMemory* DeviceMemory, const VkDevice Device, const VkPhysicalDeviceMemoryProperties PDMP, const VkBufferUsageFlags BUF, const size_t Size, const void* Source, 
	const VkCommandBuffer CB, const VkAccessFlagBits AF, const VkPipelineStageFlagBits PSF, const VkQueue Queue)
{
	//!< デバイスローカルバッファ、デバイスメモリを作成 (Create device local buffer, device memory)
	CreateBufferMemory(Buffer, DeviceMemory, Device, PDMP, BUF | VK_BUFFER_USAGE_TRANSFER_DST_BIT, Size, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	Scoped<BufferMemory> StagingBuffer(Device);
	//!< ホストビジブルバッファ、デバイスメモリを作成 (Create host visible buffer, device memory)
	StagingBuffer.Create(Device, PDMP, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, Size, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, Source);
	{
		constexpr VkCommandBufferBeginInfo CBBI = {
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
			.pNext = nullptr,
			.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
			.pInheritanceInfo = nullptr
		};
		VERIFY_SUCCEEDED(vkBeginCommandBuffer(CB, &CBBI)); {
			//!< ホストビジブルからデバイスローカルへのコピーコマンドを発行 (Submit host visible to device local copy command)
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
	//!< これはサンプルコード

	VkBuffer Buffer; //!< #VK_TODO
	VkDeviceSize Size = 0; //!< #VK_TODO
	CreateBuffer(&Buffer, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, Size);

	VkDeviceMemory DM; //!< #VK_TODO
	AllocateDeviceMemory(&DM, Buffer, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	VERIFY_SUCCEEDED(vkBindBufferMemory(Device, Buffer, DM, 0));
}
void VK::CreateStorageBuffer_Example()
{
	//!< これはサンプルコード

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
	//!< これはサンプルコード

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

	//!< サポートされているかのチェック
	VkFormatProperties FP;
	vkGetPhysicalDeviceFormatProperties(GetCurrentPhysicalDevice(), BVCI.format, &FP);
	assert((FP.bufferFeatures & VK_FORMAT_FEATURE_UNIFORM_TEXEL_BUFFER_BIT) && "");
}
void VK::CreateStorageTexelBuffer_Example()
{
	//!< これはサンプルコード

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

	//!< サポートされているかのチェック
	VkFormatProperties FP;
	vkGetPhysicalDeviceFormatProperties(GetCurrentPhysicalDevice(), BVCI.format, &FP);
	assert((FP.bufferFeatures & VK_FORMAT_FEATURE_STORAGE_TEXEL_BUFFER_BIT) && "");
	//!< #VK_TODO アトミック操作をする場合
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

//!< デスクリプタセットを個々に解放したい場合には .flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT を指定する、
//!< その場合断片化は自分で管理しなくてはならない (指定しない場合はプール毎にまとめて解放しかできない)
//!< 1つのブールに対して、複数スレッドで同時にデスクリプタセットを確保することはできない (スレッド毎に別プールにすること)
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
		.pipelineLayout = VK_NULL_HANDLE, .set = 0 //!< VK_DESCRIPTOR_UPDATE_TEMPLATE_TYPE_PUSH_DESCRIPTORS_KHR使用時はパイプラインレイアウトを指定 (使用例はUSE_PUSH_DESCRIPTOR参照)
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

	//!< ビューの作成 (Create view)
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
			//!< アップロード用バッファ (Buffer for upload)
			CreateBuffer(&Buffer, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, TotalSize);
			AllocateDeviceMemory(&DeviceMemory, Buffer, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
			VERIFY_SUCCEEDED(vkBindBufferMemory(Device, Buffer, DeviceMemory, 0));
			CopyToHostVisibleDeviceMemory(DeviceMemory, 0, TotalSize, data(Colors));

			//!< バッファ->テクスチャ転送コマンド (Buffer to image copy command)
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
			//!< コマンドの実行 (Submit command)
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
	//!< インプットアタッチメント (InputAttachment)
	constexpr std::array<VkAttachmentReference, 0> IAs = {};
	//!< カラーアタッチメント (ColorAttachment)
	constexpr std::array CAs = { VkAttachmentReference({.attachment = 0, .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL }), };
	//!< リゾルブアタッチメント (ResolveAttachment) : マルチサンプル → シングルサンプルへリゾルブするような場合
	constexpr std::array RAs = { VkAttachmentReference({.attachment = VK_ATTACHMENT_UNUSED, .layout = VK_IMAGE_LAYOUT_UNDEFINED }), };
	assert(size(CAs) == size(RAs) && "");
	//!< プリザーブアタッチメント (PreserveAttachment) : サブパス全体において保持しなくてはならないコンテンツのインデックス
	constexpr std::array<uint32_t, 0> PAs = {};

	CreateRenderPass(RenderPasses.emplace_back(), {
		VkAttachmentDescription({
			.flags = 0,
			.format = ColorFormat,
			.samples = VK_SAMPLE_COUNT_1_BIT,
			.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE, .storeOp = VK_ATTACHMENT_STORE_OP_STORE,						//!< 「開始時に何もしない」「終了時に保存」
			.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE, .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,	//!< ステンシルのロードストア : (ここでは)開始時、終了時ともに「使用しない」
			.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED, .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR				//!< レンダーパスのレイアウト : 「開始時未定義」「終了時プレゼンテーションソース」
		}),
	}, {
		VkSubpassDescription({
			.flags = 0,
			.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
			.inputAttachmentCount = static_cast<uint32_t>(size(IAs)), .pInputAttachments = data(IAs), 									//!< インプットアタッチメント(読み取り用) シェーダ内で次のように使用 layout (input_attachment_index=0, set=0, binding=0) uniform XXX YYY;
			.colorAttachmentCount = static_cast<uint32_t>(size(CAs)), .pColorAttachments = data(CAs), .pResolveAttachments = data(RAs),	//!< カラーアタッチメント(書き込み用)、リゾルブアタッチメント(マルチサンプルのリゾルブ)
			.pDepthStencilAttachment = nullptr,																							//!< デプスアタッチメント(書き込み用)
			.preserveAttachmentCount = static_cast<uint32_t>(size(PAs)), .pPreserveAttachments = data(PAs)								//!< プリザーブアタッチメント(サブパス全体において保持するコンテンツのインデックス)
		}),
	}, {
#if 1
		//!< サブパス依存 (敢えて書く場合)
		VkSubpassDependency({
			.srcSubpass = VK_SUBPASS_EXTERNAL, .dstSubpass = 0,																		//!< サブパス外からサブパス0へ
			.srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,	//!< パイプラインの最終ステージからカラー出力ステージへ
			.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT, .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,						//!< 読み込みからカラー書き込みへ
			.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT,																			//!< 同じメモリ領域に対する書き込みが完了してから読み込み (指定しない場合は自前で書き込み完了を管理)
		}),
		VkSubpassDependency({
			.srcSubpass = 0, .dstSubpass = VK_SUBPASS_EXTERNAL,																		//!< サブパス0からサブパス外へ
			.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, .dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,	//!< カラー出力ステージからパイプラインの最終ステージへ
			.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, .dstAccessMask = VK_ACCESS_MEMORY_READ_BIT,						//!< カラー書き込みから読み込みへ
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
		.renderPass = RP, //!< ここで指定するレンダーパスは「互換性のあるもの」なら可
		.attachmentCount = static_cast<uint32_t>(size(IVs)), .pAttachments = data(IVs),
		.width = Width, .height = Height,
		.layers = Layers
	};
	VERIFY_SUCCEEDED(vkCreateFramebuffer(Device, &FCI, GetAllocationCallbacks(), &FB));
}

/**
@brief シェーダコンパイル、リンクはパイプラインオブジェクト作成時に行われる Shader compilation and linkage is performed during the pipeline object creation
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

	//!< バーテックスインプット (VertexInput)
	const VkPipelineVertexInputStateCreateInfo PVISCI = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
		.vertexBindingDescriptionCount = static_cast<uint32_t>(size(VIBDs)), .pVertexBindingDescriptions = data(VIBDs),
		.vertexAttributeDescriptionCount = static_cast<uint32_t>(size(VIADs)), .pVertexAttributeDescriptions = data(VIADs)
	};

	//!< DXでは「トポロジ」と「パッチコントロールポイント」の指定はIASetPrimitiveTopology()の引数としてコマンドリストへ指定する、VKとは結構異なるので注意
	//!< (「パッチコントロールポイント」の数も何を指定するかにより決まる)
	//!< CommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	//!< CommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_1_CONTROL_POINT_PATCHLIST);

	//!< インプットアセンブリ (InputAssembly)
	const VkPipelineInputAssemblyStateCreateInfo PIASCI = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
		.topology = Topology,
		.primitiveRestartEnable = VK_FALSE
	};
	//!< WITH_ADJACENCY 系使用時には デバイスフィーチャー geometryShader が有効であること
	//assert((
	//	(PIASCI.topology != VK_PRIMITIVE_TOPOLOGY_LINE_LIST_WITH_ADJACENCY || PIASCI.topology != VK_PRIMITIVE_TOPOLOGY_LINE_STRIP_WITH_ADJACENCY || PIASCI.topology != VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST_WITH_ADJACENCY || PIASCI.topology != VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP_WITH_ADJACENCY)
	//	|| PDF.geometryShader) /*&& ""*/);
	//!< PATCH_LIST 使用時には デバイスフィーチャー tessellationShader が有効であること
	//assert((PIASCI.topology != VK_PRIMITIVE_TOPOLOGY_PATCH_LIST || PDF.tessellationShader) && "");
	//!< インデックス 0xffffffff(VK_INDEX_TYPE_UINT32), 0xffff(VK_INDEX_TYPE_UINT16) をプリミティブのリスタートとする、インデックス系描画の場合(vkCmdDrawIndexed, vkCmdDrawIndexedIndirect)のみ有効
	//!< LIST 系使用時 primitiveRestartEnable 無効であること
	assert((
		(PIASCI.topology != VK_PRIMITIVE_TOPOLOGY_POINT_LIST || PIASCI.topology != VK_PRIMITIVE_TOPOLOGY_LINE_LIST || PIASCI.topology != VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST || PIASCI.topology != VK_PRIMITIVE_TOPOLOGY_LINE_LIST_WITH_ADJACENCY || PIASCI.topology != VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST_WITH_ADJACENCY || PIASCI.topology != VK_PRIMITIVE_TOPOLOGY_PATCH_LIST)
		|| PIASCI.primitiveRestartEnable == VK_FALSE) /*&& ""*/);

	//!< テセレーション (Tessellation)
	assert((Topology != VK_PRIMITIVE_TOPOLOGY_PATCH_LIST || PatchControlPoints != 0) && "");
	const VkPipelineTessellationStateCreateInfo PTSCI = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
		.patchControlPoints = PatchControlPoints //!< パッチコントロールポイント
	};

	//!< ビューポート (Viewport)
	//!< VkDynamicState を使用するため、ここではビューポート(シザー)の個数のみ指定している (To use VkDynamicState, specify only count of viewport(scissor) here)
	//!< 後に vkCmdSetViewport(), vkCmdSetScissor() で指定する (Use vkCmdSetViewport(), vkCmdSetScissor() later)
	constexpr VkPipelineViewportStateCreateInfo PVSCI = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
		.viewportCount = 1, .pViewports = nullptr,
		.scissorCount = 1, .pScissors = nullptr
	};
	//!< 2つ以上のビューポートを使用するにはデバイスフィーチャー multiViewport が有効であること (If use 2 or more viewport device feature multiViewport must be enabled)
	//!< ビューポートのインデックスはジオメトリシェーダで指定する (Viewport index is specified in geometry shader)
	//assert((PVSCI.viewportCount <= 1 || PDF.multiViewport) && "");

	//!< PRSCI
	//!< FILL以外使用時には、デバイスフィーチャーfillModeNonSolidが有効であること
	assert(PRSCI.polygonMode == VK_POLYGON_MODE_FILL && "");
	//!< 1.0f より大きな値には、デバイスフィーチャーwidelines が有効であること
	assert(PRSCI.lineWidth <= 1.0f&& "");

	//!< マルチサンプル (Multisample)
	constexpr VkSampleMask SM = 0xffffffff; //!< 0xffffffff を指定する場合は、代わりに nullptr でもよい
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

	//!< VK_BLEND_FACTOR_SRC1 系をを使用するには、デバイスフィーチャー dualSrcBlend が有効であること
	///!< SRCコンポーネント * SRCファクタ OP DSTコンポーネント * DSTファクタ

	//!< デバイスフィーチャー independentBlend が有効で無い場合は、配列の各要素は「完全に同じ値」であること (If device feature independentBlend is not enabled, each array element must be exactly same)
	//if (!PDF.independentBlend) {
	//	for (auto i : PCBASs) {
	//		assert(memcmp(&i, &PCBASs[0], sizeof(PCBASs[0])) == 0 && ""); //!< 最初の要素は比べる必要無いがまあいいや
	//	}
	//}
	const VkPipelineColorBlendStateCreateInfo PCBSCI = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
		.logicOpEnable = VK_FALSE, .logicOp = VK_LOGIC_OP_COPY, //!< ブレンド時に論理オペレーションを行う (ブレンドは無効になる) (整数型アタッチメントに対してのみ)
		.attachmentCount = static_cast<uint32_t>(size(PCBASs)), .pAttachments = data(PCBASs),
		.blendConstants = { 1.0f, 1.0f, 1.0f, 1.0f }
	};

	//!< ダイナミックステート (DynamicState)
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
	@brief 継承 ... 共通部分が多い場合、親パイプラインを指定して作成するとより高速に作成できる、親子間でのスイッチやバインドが有利
	(DX の D3D12_CACHED_PIPELINE_STATE 相当?)
	basePipelineHandle, basePipelineIndex は同時に使用できない、使用しない場合はそれぞれ VK_NULL_HANDLE, -1 を指定すること
	親には VK_PIPELINE_CREATE_ALLOW_DERIVATIVES_BIT フラグが必要、子には VK_PIPELINE_CREATE_DERIVATIVE_BIT フラグが必要
	・basePipelineHandle ... 既に親とするパイプライン(ハンドル)が存在する場合に指定
	・basePipelineIndex ... 同配列内で親パイプラインも同時に作成する場合、配列内での親パイプラインの添字(親の添字の方が若い値であること)
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
			.renderPass = RP, .subpass = 0, //!< ここで指定するレンダーパスは「互換性のあるもの」なら可
			.basePipelineHandle = VK_NULL_HANDLE, .basePipelineIndex = -1
		})
	};
	//!< VKでは1コールで複数のパイプラインを作成することもできるが、DXに合わせて1つしか作らないことにしておく
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
@brief クリア Clear
@note 「レンダーパス外」でクリア Out of renderpass ... vkCmdClearColorImage()
@note 「レンダーパス開始時」にクリア Begining of renderpass ... VkAttachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR, VkRenderPassBeginInfo.pClearValues
@note 「各々のサブパス」でクリア Each subpass ... vkCmdClearAttachments()
*/
//!< 「レンダーパス外」にてクリアを行う
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
		//!< vkCmdClearColorImage() はレンダーパス内では使用できない
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

//!< 非推奨
#if 0
//!< これを用いてデプスをクリアする場合は、イメージ作成時に VK_IMAGE_USAGE_TRANSFER_DST_BIT を指定すること
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
		//!< vkCmdClearDepthStencilImage() はレンダーパス内では使用できない
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
//!<「サブパス」にてクリアするときに使う
//!< Drawコール前に使用すると、「それなら VkAttachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR を使え」と Warnning が出るので注意
void VK::ClearColorAttachment(const VkCommandBuffer CB, const VkClearColorValue& Color)
{
	const VkClearValue ClearValue = { .color = Color };
	const std::array ClearAttachments = {
		VkClearAttachment({
			VK_IMAGE_ASPECT_COLOR_BIT,
			0, //!< カラーアタッチメントのインデックス #VK_TODO 現状決め打ち
			ClearValue
		}),
	};
	const std::array ClearRects = {
		VkClearRect({
			ScissorRects[0],
			0, 1 //!< 開始レイヤとレイヤ数 #VK_TODO 現状決め打ち
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
			0, //!< ここでは無視される
			ClearValue
		},
	};
	const VkRect2D ClearArea = { { 0, 0 }, SurfaceExtent2D };
	const std::array<VkClearRect, 1> ClearRects = {
		{
			ClearArea,
			0, 1 //!< 開始レイヤとレイヤ数 #VK_TODO 現状決め打ち
		},
	};
	vkCmdClearAttachments(CB,
		static_cast<uint32_t>(size(ClearAttachments)), data(ClearAttachments),
		static_cast<uint32_t>(size(ClearRects)), data(ClearRects));
}

#if 0
void VK::PopulateCommandBuffer(const size_t i)
{
	//!< vkBeginCommandBuffer() で暗黙的にリセットされるが、明示的にリセットする場合には「メモリをプールへリリースするかどうかを指定できる」
	//VERIFY_SUCCEEDED(vkResetCommandBuffer(CB, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT));

	//!< @brief VkCommandBufferUsageFlags
	//!< * VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT		... 一度だけ使用する場合や毎回リセットする場合に指定、何度もサブミットするものには指定しない
	//!< * VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT		... デバイスでまだ実行されている間に、コマンドバッファを再度サブミットする必要がある場合に指定 (パフォーマンスの観点からは避けるべき)
	//!< * VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT	... セカンダリコマンドバッファでかつレンダーパス内の場合に指定する
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

		//!< ビューポート、シザー
		vkCmdSetViewport(CB, 0, static_cast<uint32_t>(size(Viewports)), data(Viewports));
		vkCmdSetScissor(CB, 0, static_cast<uint32_t>(size(ScissorRects)), data(ScissorRects));

#ifdef _DEBUG
		//!< レンダーエリアの最低粒度を確保
		VkExtent2D Granularity;
		vkGetRenderAreaGranularity(Device, RP, &Granularity);
		//!<「自分の環境では」 Granularity = { 1, 1 } だったのでほぼなんでも大丈夫みたい、環境によっては注意が必要
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
			.renderArea = VkRect2D({.offset = VkOffset2D({.x = 0, .y = 0 }), .extent = SurfaceExtent2D }), //!< フレームバッファのサイズ以下を指定できる
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

	//!< 次のイメージが取得できるまでブロック(タイムアウトは指定可能)、取得できたからといってイメージは直ぐに目的に使用可能とは限らない
	//!< (引数で指定した場合)使用可能になるとフェンスやセマフォがシグナルされる
	//!< ここではセマフォを指定し、このセマフォはサブミット時に使用する(サブミットしたコマンドがプレゼンテーションを待つように指示している)
	//!<	VK_SUBOPTIMAL_KHR : イメージは使用可能ではあるがプレゼンテーションエンジンにとってベストではない状態
	//!<	VK_ERROR_OUT_OF_DATE_KHR : イメージは使用不可で再作成が必要
	VERIFY_SUCCEEDED(vkAcquireNextImageKHR(Device, Swapchain, UINT64_MAX, NextImageAcquiredSemaphore, VK_NULL_HANDLE, &SwapchainImageIndex));

	DrawFrame(GetCurrentBackBufferIndex());
	
	Submit();
	
	Present();
}
void VK::Dispatch()
{
	//!< (Fenceを指定して)サブミットしたコマンドが完了するまでブロッキングして待つ
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
	//!< サブミットしたコマンドの完了を待つ
	const std::array Fences = { Fence };
	VERIFY_SUCCEEDED(vkWaitForFences(Device, static_cast<uint32_t>(size(Fences)), data(Fences), VK_TRUE, (std::numeric_limits<uint64_t>::max)()));
	vkResetFences(Device, static_cast<uint32_t>(size(Fences)), data(Fences));
}
void VK::Submit()
{
	//!< コマンドは指定のパイプラインステージに到達するまで実行され、そこでセマフォがシグナルされるまで待つ
	const std::array WaitSems = { NextImageAcquiredSemaphore };
	const std::array WaitStages = { VkPipelineStageFlags(VK_PIPELINE_STAGE_TRANSFER_BIT) };
	assert(size(WaitSems) == size(WaitStages) && "Must be same size");
	//!< 実行するコマンドバッファ
	const std::array CBs = { CommandBuffers[GetCurrentBackBufferIndex()], };
	//!< 完了時にシグナルされるセマフォ(RenderFinishedSemaphore)
	const std::array SigSems = { RenderFinishedSemaphore };
	const std::array SIs = {
		VkSubmitInfo({
			.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
			.pNext = nullptr,
			.waitSemaphoreCount = static_cast<uint32_t>(size(WaitSems)), .pWaitSemaphores = data(WaitSems), .pWaitDstStageMask = data(WaitStages), //!< 次イメージが取得できる(プレゼント完了)までウエイト
			.commandBufferCount = static_cast<uint32_t>(size(CBs)), .pCommandBuffers = data(CBs),
			.signalSemaphoreCount = static_cast<uint32_t>(size(SigSems)), .pSignalSemaphores = data(SigSems) //!< 描画完了を通知する
		}),
	};
	VERIFY_SUCCEEDED(vkQueueSubmit(GraphicsQueue, static_cast<uint32_t>(size(SIs)), data(SIs), Fence));
}
void VK::Present()
{
	//!< 同時に複数のプレゼントが可能だが、1つのスワップチェインからは1つのみ
	const std::array Swapchains = { Swapchain };
	const std::array ImageIndices = { GetCurrentBackBufferIndex() };
	assert(size(Swapchains) == size(ImageIndices) && "Must be same");

	//!< サブミット時に指定したセマフォ(RenderFinishedSemaphore)を待ってからプレゼントが行なわれる
	const VkPresentInfoKHR PresentInfo = {
		.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
		.pNext = nullptr,
		.waitSemaphoreCount = 1, .pWaitSemaphores = &RenderFinishedSemaphore,
		.swapchainCount = static_cast<uint32_t>(size(Swapchains)), .pSwapchains = data(Swapchains), .pImageIndices = data(ImageIndices),
		.pResults = nullptr
	};
	VERIFY_SUCCEEDED(vkQueuePresentKHR(PresentQueue, &PresentInfo));
}
