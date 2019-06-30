#include "stdafx.h"

#include <fstream>

#include "VK.h"

#ifdef _WINDOWS
#pragma comment(lib, "vulkan-1.lib")
#else
// "libvulkan.so.1"
#endif

#ifdef VK_NO_PROTOYYPES
	//!< グローバルレベル関数 Global level functions
#define VK_GLOBAL_PROC_ADDR(proc) PFN_vk ## proc VK::vk ## proc = VK_NULL_HANDLE;
#include "VKGlobalProcAddr.h"
#undef VK_GLOBAL_PROC_ADDR

	//!< インスタンスレベル関数 Instance level functions
#define VK_INSTANCE_PROC_ADDR(proc) PFN_vk ## proc VK::vk ## proc = VK_NULL_HANDLE;
#include "VKInstanceProcAddr.h"
#undef VK_INSTANCE_PROC_ADDR

	//!< デバイスレベル関数 Device level functions
#define VK_DEVICE_PROC_ADDR(proc) PFN_vk ## proc VK::vk ## proc = VK_NULL_HANDLE;
#include "VKDeviceProcAddr.h"
#undef VK_DEVICE_PROC_ADDR
#endif //!< VK_NO_PROTOYYPES

#ifdef _DEBUG
	//!< インスタンスレベル関数(Debug) Instance level functions(Debug)
#define VK_INSTANCE_PROC_ADDR(proc) PFN_vk ## proc ## EXT VK::vk ## proc = VK_NULL_HANDLE;
#include "VKDebugReport.h"
#undef VK_INSTANCE_PROC_ADDR

	//!< デバイスレベル関数(Debug) Device level functions(Debug)
#define VK_DEVICE_PROC_ADDR(proc) PFN_vk ## proc ## EXT VK::vk ## proc = VK_NULL_HANDLE;
#include "VKDebugMarker.h"
#undef VK_DEVICE_PROC_ADDR
#endif //!< _DEBUG

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

	//!< インスタンス、デバイス
	CreateInstance();
	CreateSurface(hWnd, hInstance);
	EnumeratePhysicalDevice(Instance);
	CreateDevice(GetCurrentPhysicalDevice(), Surface);
	
	//!< 同期
	CreateFence(Device);
	CreateSemaphore(Device);

	//!< スワップチェイン
	CreateSwapchain(GetCurrentPhysicalDevice(), Surface, Rect);
	GetSwapchainImage(Device, Swapchain);
	CreateSwapchainImageView();

	//!< コマンド
	CreateCommandBuffer();
	InitializeSwapchainImage(CommandPools[0].second[0], &Colors::Red);

	//!< デプス
	CreateDepthStencil();
	InitializeDepthStencilImage(CommandPools[0].second[0]);

	//!< 頂点
	CreateVertexBuffer();
	CreateIndexBuffer();
	CreateIndirectBuffer();

	CreateTexture();

	//!< パイプラインレイアウト (ルートシグネチャ相当)
	CreatePipelineLayout();
	//!< レンダーパス
	CreateRenderPass();
	//!< パイプライン
	CreatePipeline();
	//!< フレームバッファ
	CreateFramebuffer();

	//!< ユニフォームバッファ (コンスタントバッファ相当)
	CreateUniformBuffer();
	//!< デスクリプタプール (デスクリプタヒープ相当)
	CreateDescriptorPool();
	//!< デスクリプタセット (デスクリプタビュー相当)
	CreateDescriptorSet();
	UpdateDescriptorSet();

	SetTimer(hWnd, NULL, 1000 / 60, nullptr);

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
#ifdef _DEBUG
	PerformanceCounter PC(__func__);
#endif

	Super::OnExitSizeMove(hWnd, hInstance);

	std::vector<VkFence> Fences = { Fence };
	VERIFY_SUCCEEDED(vkWaitForFences(Device, static_cast<uint32_t>(Fences.size()), Fences.data(), VK_TRUE, (std::numeric_limits<uint64_t>::max)()));
	//vkResetFences(Device, static_cast<uint32_t>(Fences.size()), Fences.data());

	//ResizeSwapChain(Rect);

	CreateViewport(static_cast<float>(SurfaceExtent2D.width), static_cast<float>(SurfaceExtent2D.height));

	//DestroyFramebuffer();
	//CreateFramebuffer();

	//for (auto i = 0; i < CommandBuffers.size(); ++i) {
	//	PopulateCommandBuffer(i);
	//}
	for (auto i : CommandPools) {
		for (auto j = 0; j < i.second.size(); ++j) {
			PopulateCommandBuffer(j);
		}
	}
}

void VK::OnDestroy(HWND hWnd, HINSTANCE hInstance)
{
	Super::OnDestroy(hWnd, hInstance);

	if (VK_NULL_HANDLE != Device) {
		//!< デバイスのキューにサブミットされた全コマンドが完了するまでブロッキング、主に終了処理に使う (Wait for all command submitted to queue, usually used on finalize)
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

	//!< VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT の場合のみ個別に開放できる (Only if VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT is used, can be release individually)
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

	//!< SwapchainImages は取得したもの、破棄しない
	
	if (VK_NULL_HANDLE != Swapchain) {
		vkDestroySwapchainKHR(Device, Swapchain, GetAllocationCallbacks());
		Swapchain = VK_NULL_HANDLE;
	}

	for (auto i : CommandPools) {
		//VERIFY_SUCCEEDED(vkResetCommandPool(Device, i.first, VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT));

		//!< コマンドプール破棄時にコマンドバッファは暗黙的に解放されるので必要は無いが、ここでは一応コールしている
		if (!i.second.empty()) {
			vkFreeCommandBuffers(Device, i.first, static_cast<uint32_t>(i.second.size()), i.second.data());
			i.second.clear();
		}

		vkDestroyCommandPool(Device, i.first, GetAllocationCallbacks());
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

	//!< キューは論理デバイスと共に破棄される
	if (VK_NULL_HANDLE != Device) {
		vkDestroyDevice(Device, GetAllocationCallbacks());
		Device = VK_NULL_HANDLE;
	}
	
	//!< PhysicalDevice は vkEnumeratePhysicalDevices() で取得したもの、破棄しない

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

char* VK::GetVkResultChar(const VkResult Result)
{
#define VK_RESULT_ENTRY(vr) case VK_##vr: return #vr;
	switch (Result)
	{
	default: DEBUG_BREAK(); return "Not found";
#include "VkResult.h"
	}
#undef VK_RESULT_ENTRY
}

char* VK::GetFormatChar(const VkFormat Format)
{
#define VK_FORMAT_ENTRY(vf) case VK_FORMAT_##vf: return #vf;
	switch (Format)
	{
	default: DEBUG_BREAK(); return "Not found";
#include "VKFormat.h"
	}
#undef VK_FORMAT_ENTRY
}

char* VK::GetColorSpaceChar(const VkColorSpaceKHR ColorSpace)
{
#define VK_COLOR_SPACE_ENTRY(vcs) case VK_COLOR_SPACE_##vcs: return #vcs;
	switch (ColorSpace)
	{
	default: DEBUG_BREAK(); return "Not found";
#include "VKColorSpace.h"
	}
#undef VK_COLOR_SPACE_ENTRY
}

char* VK::GetImageViewTypeChar(const VkImageViewType ImageViewType)
{
#define VK_IMAGE_VIEW_TYPE_ENTRY(vivt) case VK_IMAGE_VIEW_TYPE_##vivt: return #vivt;
	switch (ImageViewType)
	{
	default: DEBUG_BREAK(); return "Not found";
#include "VKImageViewType.h"
	}
#undef VK_IMAGE_VIEW_TYPE_ENTRY
}

char* VK::GetComponentSwizzleChar(const VkComponentSwizzle ComponentSwizzle)
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

//!< @brief メモリプロパティフラグを(サポートされているかチェックしつつ)メモリタイプへ変換
//!< @param 物理デバイスのメモリプロパティ
//!< @param バッファ(イメージ)の要求するメモリタイプ
//!< @param 希望のメモリプロパティフラグ
//!< @return メモリタイプ
uint32_t VK::GetMemoryType(const VkPhysicalDeviceMemoryProperties& PDMP, const VkMemoryRequirements& MR, const VkFlags Properties)
{
	for (uint32_t i = 0; i < PDMP.memoryTypeCount; ++i) {
		if (MR.memoryTypeBits & (1 << i)) {
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
	//!< バッファは作成時に指定した使用法でしか使用できない、ここでは VK_SHARING_MODE_EXCLUSIVE 決め打ちにしている #VK_TODO (Using VK_SHARING_MODE_EXCLUSIVE here)
	//!< VK_SHARING_MODE_EXCLUSIVE	: 複数ファミリのキューが同時アクセスできない、他のファミリからアクセスしたい場合はオーナーシップの移譲が必要
	//!< VK_SHARING_MODE_CONCURRENT	: 複数ファミリのキューが同時アクセス可能、オーナーシップの移譲も必要無し、パフォーマンスは悪い
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

void VK::CreateImage(VkImage* Image, const VkImageCreateFlags CreateFlags, const VkImageType ImageType, const VkFormat Format, const VkExtent3D& Extent3D, const uint32_t MipLevels, const uint32_t ArrayLayers, const VkSampleCountFlagBits SampleCount, const VkImageUsageFlags Usage) const
{
	//!< Usage に VK_IMAGE_USAGE_SAMPLED_BIT が指定されいる場合、フォーマットやフィルタが使用可能かチェック #VK_TODO ここではリニアフィルタ決め打ち
	ValidateFormatProperties_Sampled(GetCurrentPhysicalDevice(), Format, Usage, VK_FILTER_LINEAR, VK_FILTER_LINEAR, VK_SAMPLER_MIPMAP_MODE_LINEAR);

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
		VK_IMAGE_TILING_OPTIMAL, //!< リニアはパフォーマンスが悪いので、ここでは OPTIMAL に決め打ちしている
		Usage,
		VK_SHARING_MODE_EXCLUSIVE,
		0, nullptr,
		VK_IMAGE_LAYOUT_UNDEFINED //!< 作成時に指定できるのは UNDEFINED, PREINITIALIZED のみ、実際に使用する前にレイアウトを変更する必要がある
	};
	ValidateImageCreateInfo(ICI);
	VERIFY_SUCCEEDED(vkCreateImage(Device, &ICI, GetAllocationCallbacks(), Image));
}

void VK::CopyToDeviceMemory(const VkDeviceMemory DeviceMemory, const size_t Size, const void* Source, const VkDeviceSize Offset)
{
	if (Size && nullptr != Source) {
		void *Data;
		VERIFY_SUCCEEDED(vkMapMemory(Device, DeviceMemory, Offset, Size, static_cast<VkMemoryMapFlags>(0), &Data)); {
			memcpy(Data, Source, Size);

			const std::vector<VkMappedMemoryRange> MMRs = {
				{
					VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE,
					nullptr,
					DeviceMemory,
					0,
					VK_WHOLE_SIZE
				}
			};

			//!< デバスメモリ確保時に VK_MEMORY_PROPERTY_HOST_COHERENT_BIT を指定した場合は必要ない CreateDeviceMemory(..., VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
			//!< メモリコンテンツが変更されたことをドライバへ知らせる
			VERIFY_SUCCEEDED(vkFlushMappedMemoryRanges(Device, static_cast<uint32_t>(MMRs.size()), MMRs.data()));
			//VERIFY_SUCCEEDED(vkInvalidateMappedMemoryRanges(Device, static_cast<uint32_t>(MMRs.size()), MMRs.data()));
		} vkUnmapMemory(Device, DeviceMemory);
	}
}
//!< @param コマンドバッファ
//!< @param コピー元バッファ
//!< @param コピー先バッファ
//!< @param (コピー後の)バッファのアクセスフラグ ex) VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT, VK_ACCESS_INDEX_READ_BIT, VK_ACCESS_INDIRECT_READ_BIT,...等
//!< @param (コピー後に)バッファが使われるパイプラインステージ ex) VK_PIPELINE_STAGE_VERTEX_INPUT_BIT, VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT,...等
void VK::CopyBufferToBuffer(const VkCommandBuffer CB, const VkBuffer Src, const VkBuffer Dst, const VkAccessFlags AF, const VkPipelineStageFlagBits PSF, const size_t Size)
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
				//!< バッファがどう扱われるかのフラグ「これまで」と「これから」、0から書き込み(VK_ACCESS_MEMORY_WRITE_BIT)へ
				0, VK_ACCESS_MEMORY_WRITE_BIT,
				//!< (VK_SHARING_MODE_EXCLUSIVEで作成されたバッファを)参照しているキューファミリを変更「これまで」と「これから」、ここでは変更しないのでVK_QUEUE_FAMILY_IGNORED
				VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED,
				Dst,
				0,
				VK_WHOLE_SIZE
			},
		};
		vkCmdPipelineBarrier(CB,
			//!< バッファが使われるパイプラインステージ「これまで」と「これから」、VK_PIPELINE_STAGE_TOP_OF_PIPE_BITから転送(VK_PIPELINE_STAGE_TRANSFER_BIT)へ
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
				//!< バッファがどう扱われるかのフラグ「これまで」と「これから」、例えば(VK_ACCESS_MEMORY_WRITE_BIT)から頂点(VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT)へ等
				VK_ACCESS_MEMORY_WRITE_BIT, AF,
				VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED,
				Dst,
				0,
				VK_WHOLE_SIZE
			},
		};
		assert(BMBs_Pre[0].dstAccessMask == BMBs_Post[0].srcAccessMask);
		vkCmdPipelineBarrier(CB, 
			//!< バッファが使われるパイプラインステージ「これまで」と「これから」、例えば転送先(VK_PIPELINE_STAGE_TRANSFER_BIT)から頂点バッファ(VK_PIPELINE_STAGE_VERTEX_INPUT_BIT)へ等
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
		}
	}
}

void VK::CreateBufferMemory(VkDeviceMemory* DeviceMemory, const VkBuffer Buffer, const VkMemoryPropertyFlags MPF)
{
	const auto& PDMP = GetCurrentPhysicalDeviceMemoryProperties();

	VkMemoryRequirements MR;
	vkGetBufferMemoryRequirements(Device, Buffer, &MR);
	EnumerateMemoryRequirements(MR);

	const VkMemoryAllocateInfo MAI = {
		VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
		nullptr,
		MR.size,
		GetMemoryType(PDMP, MR, MPF)
	};
	VERIFY_SUCCEEDED(vkAllocateMemory(Device, &MAI, nullptr, DeviceMemory));
}
void VK::CreateImageMemory(VkDeviceMemory* DeviceMemory, const VkImage Image, const VkMemoryPropertyFlags MPF)
{
	const auto& PDMP = GetCurrentPhysicalDeviceMemoryProperties();

	VkMemoryRequirements MR;
	vkGetImageMemoryRequirements(Device, Image, &MR);
	EnumerateMemoryRequirements(MR);

	const VkMemoryAllocateInfo MAI = {
		VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
		nullptr,
		MR.size,
		GetMemoryType(PDMP, MR, MPF)
	};
	VERIFY_SUCCEEDED(vkAllocateMemory(Device, &MAI, nullptr, DeviceMemory));
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
//!< Cubemapを作る場合、まずレイヤ化されたイメージを作成し、(イメージビューを用いて)レイヤをフェイスとして扱うようハードウエアに伝える
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
void VK::ValidateFormatProperties_Sampled(VkPhysicalDevice PD, const VkFormat Format, const VkImageUsageFlags Usage, const VkFilter Mag, const VkFilter Min, const VkSamplerMipmapMode Mip) const
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

void VK::ValidateFormatProperties_Storage(VkPhysicalDevice PD, const VkFormat Format, const VkImageUsageFlags Usage, const bool UseAtomic) const
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
#endif //!< VK_NO_PROTOYYPES

void VK::CreateInstance()
{
	//!< インスタンスレベルのレイヤー、エクステンションの列挙
	EnumerateInstanceLayerProperties();

	uint32_t APIVersion; //= VK_API_VERSION_1_1;
	//!< ここでは、最新バージョンでのみ動くようにしておく Use latest version here
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
		//!< ↓標準的なバリデーションレイヤセットを最適な順序でロードする指定
		//!< (プログラムからやらない場合は環境変数 VK_INSTANCE_LAYERS へセットしておいてもよい)
		"VK_LAYER_LUNARG_standard_validation",
		//!< 
		"VK_LAYER_LUNARG_object_tracker",
		//!< API 呼び出しとパラメータをコンソール出力する (出力がうるさいのでここでは指定しない)
		//"VK_LAYER_LUNARG_api_dump",
#else
		"VK_LAYER_LUNARG_core_validation",
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
		//!< デバッグレポート用
		VK_EXT_DEBUG_REPORT_EXTENSION_NAME,
#endif
#ifdef USE_RENDERDOC
		VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
#endif
	};

	//!< #VK_TODO 有効にしているものが、InstanceLayerProperties に含まれているかチェックする

	const VkInstanceCreateInfo InstanceCreateInfo = {
		VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
		nullptr,
		0,
		&ApplicationInfo,
		static_cast<uint32_t>(EnabledLayerNames.size()), EnabledLayerNames.data(),
		static_cast<uint32_t>(EnabledExtensions.size()), EnabledExtensions.data()
	};
	VERIFY_SUCCEEDED(vkCreateInstance(&InstanceCreateInfo, GetAllocationCallbacks(), &Instance));

	//!< インスタンスレベルの関数をロードする Load instance level functions
#ifdef VK_NO_PROTOYYPES
#define VK_INSTANCE_PROC_ADDR(proc) vk ## proc = reinterpret_cast<PFN_vk ## proc>(vkGetInstanceProcAddr(Instance, "vk" #proc)); assert(nullptr != vk ## proc && #proc);
#include "VKInstanceProcAddr.h"
#undef VK_INSTANCE_PROC_ADDR
#endif //!< VK_NO_PROTOYYPES

#ifdef _DEBUG
	CreateDebugReportCallback();
#endif

	LOG_OK();
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
			MEMORY_PROPERTY_ENTRY(PROTECTED);
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
	//!< 物理デバイス(GPU)の列挙
	uint32_t Count = 0;
	VERIFY_SUCCEEDED(vkEnumeratePhysicalDevices(Instance, &Count, nullptr));
	assert(Count && "Physical device not found");
	PhysicalDevices.resize(Count);
	VERIFY_SUCCEEDED(vkEnumeratePhysicalDevices(Instance, &Count, PhysicalDevices.data()));

	Log("\tPhysicalDevices\n");
	for (const auto& i : PhysicalDevices) {
		//!< プロパティ
		VkPhysicalDeviceProperties PDP;
		vkGetPhysicalDeviceProperties(i, &PDP);
		EnumeratePhysicalDeviceProperties(PDP);

		//!< フィーチャー
		VkPhysicalDeviceFeatures PDF;
		vkGetPhysicalDeviceFeatures(i, &PDF);
		EnumeratePhysicalDeviceFeatures(PDF);

		//!< メモリプロパティ
		VkPhysicalDeviceMemoryProperties PDMP;
		vkGetPhysicalDeviceMemoryProperties(i, &PDMP);
		EnumeratePhysicalDeviceMemoryProperties(PDMP);

		//!< デバイスレベルのレイヤー、エクステンションの列挙
		EnumeratePhysicalDeviceLayerProperties(i);

		Log("\n");
	}

	assert(!PhysicalDevices.empty() && "No physical device found");
	SetCurrentPhysicalDevice(PhysicalDevices[0]); //!< ここでは最初の物理デバイスを選択することにする #VK_TODO
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

void VK::EnumerateQueueFamilyProperties(VkPhysicalDevice PD, VkSurfaceKHR Surface, std::vector<VkQueueFamilyProperties>& QFPs)
{
	//!< 同じ能力を持つキューはファミリにグループ化される
	uint32_t Count = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(PD, &Count, nullptr);
	assert(Count && "QueueFamilyProperty not found");
	QFPs.resize(Count);
	vkGetPhysicalDeviceQueueFamilyProperties(PD, &Count, QFPs.data());

	//!< Geforce970Mだと以下のような状態だった (In case of Geforce970M)
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

	//!< 専用キューが存在する場合は専用キューを使用したほうが良い
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

	//!< グラフィックとプレゼンテーションを「同時に」サポートするキューがあるか Prioritize queue which support both of graphics and presentation
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
	//!< VkPhysicalDeviceFeatures には可能なフィーチャーが全て true になったものが渡されてくるので、
	//!< 不要な項目を false にするようにオーバーライドするとパフォーマンスが良くなる
	//!< VkPhysicalDeviceFeatures が false で渡されてくる項目を true に変えても無理

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

	//!< キュー作成情報 Queue create information
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
		//!< スワップチェインはプラットフォームに特有の機能なのでデバイス作製時に VK_KHR_SWAPCHAIN_EXTENSION_NAME エクステンションを有効にして作成しておく
		VK_KHR_SWAPCHAIN_EXTENSION_NAME,
#ifdef USE_RENDERDOC
		VK_EXT_DEBUG_MARKER_EXTENSION_NAME,
#endif
		VK_EXT_VALIDATION_CACHE_EXTENSION_NAME,
	};

	//!< vkGetPhysicalDeviceFeatures() で可能なフィーチャーが全て VkPhysicalDeviceFeatures になったPDFが返る
	//!< このままでは可能なだけ有効になってしまうのでパフォーマンス的には良くない(必要な項目だけ true にし、それ以外は false にするのが本来は良い)
	//!< デバイスフィーチャーを「有効にしないと」と使用できない機能が多々あるのでここでは返った値をそのまま使っている (パフォーマンスは良くない)
	VkPhysicalDeviceFeatures PDF;
	vkGetPhysicalDeviceFeatures(PD, &PDF);
	//!< 必要ならここでオーバーライドして不要な項目を false にする
	OverridePhysicalDeviceFeatures(PDF);
	const VkDeviceCreateInfo DeviceCreateInfo = {
		VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
		nullptr,
		0,
		static_cast<uint32_t>(QueueCreateInfos.size()), QueueCreateInfos.data(),
		0, nullptr, //!< デバイスのレイヤは非推奨 (Device layer is deprecated)
		static_cast<uint32_t>(EnabledExtensions.size()), EnabledExtensions.data(),
		&PDF
	};
	VERIFY_SUCCEEDED(vkCreateDevice(PD, &DeviceCreateInfo, GetAllocationCallbacks(), &Device));

	//!< デバイスレベルの関数をロードする Load device level functions
#ifdef VK_NO_PROTOYYPES
#define VK_DEVICE_PROC_ADDR(proc) vk ## proc = reinterpret_cast<PFN_vk ## proc>(vkGetDeviceProcAddr(Device, "vk" #proc)); assert(nullptr != vk ## proc && #proc && #proc);
#include "VKDeviceProcAddr.h"
#undef VK_DEVICE_PROC_ADDR
#endif //!< VK_NO_PROTOYYPES

	//!< キューの取得 (グラフィック、プレゼントキューは同じインデックスの場合もあるが別の変数に取得しておく) Graphics and presentation index may be same, but save to individual variables
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

	LOG_OK();
}

//!< ホストとデバイスの同期 (Synchronization between host and device)
//!< サブミット(vkQueueSubmit) に使用し、Draw()やDispatch()の頭でシグナル(サブミットされたコマンドの完了)を待つ (Used when submit, and wait signal on top of Draw())
//!< 初回と２回目以降を同じに扱う為に、シグナル済み状態(VK_FENCE_CREATE_SIGNALED_BIT)で作成している (Create with signaled state, to do same operation on first time and second time)
void VK::CreateFence(VkDevice Device)
{
	const VkFenceCreateInfo FenceCreateInfo = {
		VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
		nullptr,
		VK_FENCE_CREATE_SIGNALED_BIT
	};
	VERIFY_SUCCEEDED(vkCreateFence(Device, &FenceCreateInfo, GetAllocationCallbacks(), &Fence));
	VERIFY_SUCCEEDED(vkCreateFence(Device, &FenceCreateInfo, GetAllocationCallbacks(), &ComputeFence));

	LOG_OK();
}

//!< キュー内での同期(異なるキュー間の同期も可能) (Synchronization internal queue)
//!< イメージ取得(vkAcquireNextImageKHR)、サブミット(VkSubmitInfo)、プレゼンテーション(VkPresentInfoKHR)に使用する (Use when image acquire, submit, presentation) 
void VK::CreateSemaphore(VkDevice Device)
{
	const VkSemaphoreCreateInfo SemaphoreCreateInfo = {
		VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
		nullptr,
		0
	};

	//!< プレゼント完了同期用 (Wait for presentation finish)
	VERIFY_SUCCEEDED(vkCreateSemaphore(Device, &SemaphoreCreateInfo, GetAllocationCallbacks(), &NextImageAcquiredSemaphore));

	//!< 描画完了同期用 (Wait for render finish)
	VERIFY_SUCCEEDED(vkCreateSemaphore(Device, &SemaphoreCreateInfo, GetAllocationCallbacks(), &RenderFinishedSemaphore));

	LOG_OK();
}

//!< キューファミリが異なる場合は別のコマンドプールを用意する必要がある、そのキューにのみサブミットできる
//!< 複数スレッドで同時にレコーディングするには、別のコマンドプールからアロケートされたコマンドバッファである必要がある (コマンドプールは複数スレッドからアクセス不可)
void VK::CreateCommandPool(VkDevice Device, const uint32_t QueueFamilyIndex)
{
	CommandPools.push_back(COMMAND_POOL());
	//!< VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT	... コマンドバッファ毎にリセットが可能、指定しない場合はこのプールからアロケートされた全コマンドバッファをまとめてリセットすることになる
	//!<														コマンドバッファのレコーディング開始時には暗黙的にリセットが行なわれるので注意
	//!< VK_COMMAND_POOL_CREATE_TRANSIENT_BIT				... 短命で、何度もサブミットしない、すぐにリセットやリリースされる場合に指定
	const VkCommandPoolCreateInfo CommandPoolInfo = {
		VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
		nullptr,
		VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
		QueueFamilyIndex
	};
	VERIFY_SUCCEEDED(vkCreateCommandPool(Device, &CommandPoolInfo, GetAllocationCallbacks(), &CommandPools.back().first));

	LOG_OK();
}

void VK::AllocateCommandBuffer(const VkCommandBufferLevel Level, const size_t Count, COMMAND_POOL& Command)
{
	if (Count) {
		const auto PrevCount = Command.second.size();
		Command.second.resize(PrevCount + Count);
		//!< VkCommandBufferLevel 
		//!< VK_COMMAND_BUFFER_LEVEL_PRIMARY	: 直接キューにサブミットできる、セカンダリをコールできる (Can be submit, can execute secondary)
		//!< VK_COMMAND_BUFFER_LEVEL_SECONDARY	: サブミットできない、プライマリから実行されるのみ (Cannot submit, only executed from primary)
		const VkCommandBufferAllocateInfo AllocateInfo = {
			VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
			nullptr,
			Command.first,
			Level,
			static_cast<uint32_t>(Count)
		};
		VERIFY_SUCCEEDED(vkAllocateCommandBuffers(Device, &AllocateInfo, &Command.second[PrevCount]));
	}

	LOG_OK();
}

void VK::CreateCommandBuffer()
{
	CreateCommandPool(Device, GraphicsQueueFamilyIndex);
	{
		AllocateCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, SwapchainImages.size(), CommandPools.back());

		//!< 例えばセカンダリを追加する場合
		//AllocateCommandBuffer(VK_COMMAND_BUFFER_LEVEL_SECONDARY, 1, Commands.back());
		//...
	}

	//!< キューファミリが異なる場合は、別のコマンドプールを用意する必要がある
	if (GraphicsQueueFamilyIndex != ComputeQueueFamilyIndex) {
		CreateCommandPool(Device, ComputeQueueFamilyIndex);
		{
			AllocateCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, 1, CommandPools.back());
		}
	}
}

VkSurfaceFormatKHR VK::SelectSurfaceFormat(VkPhysicalDevice PD, VkSurfaceKHR Surface)
{
	uint32_t Count;
	VERIFY_SUCCEEDED(vkGetPhysicalDeviceSurfaceFormatsKHR(PD, Surface, &Count, nullptr));
	assert(Count && "Surface format count is zero");
	std::vector<VkSurfaceFormatKHR> SFs(Count);
	VERIFY_SUCCEEDED(vkGetPhysicalDeviceSurfaceFormatsKHR(PD, Surface, &Count, SFs.data()));

	//!< ここでは最初に見つかった UNDEFINED でないものを選択している (Select first format but UNDEFINED here)
	const auto SelectedIndex = [&]() {
		//!< 要素が 1 つのみで UNDEFINED の場合、制限は無く好きなものを選択できる (If there is only 1 element and which is UNDEFINED, we can choose any)
		if (1 == SFs.size() && VK_FORMAT_UNDEFINED == SFs[0].format) {
			return -1;
		}
		for (auto i = 0; i < SFs.size(); ++i) {
			if (VK_FORMAT_UNDEFINED != SFs[i].format) {
				return i;
			}
		}
		//!< ここに来てはいけない
		assert(false && "Valid surface format not found");
		return 0;
	}();

	//!< ColorSpace はハードウェア上でのカラーコンポーネントの表現(リニア、ノンリニア、エンコード、デコード等)
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
		//!< 0xffffffff の場合はスワップチェインイメージサイズがウインドウサイズを決定することになる (If 0xffffffff, size of swapchain image determines the size of the window)
		//!< (クランプした)引数のWidth, Heightを使用する (In this case, use argument (clamped) Width and Heigt) 
		return VkExtent2D({ std::max(std::min(Width, Cap.maxImageExtent.width), Cap.minImageExtent.width), std::max(std::min(Height, Cap.minImageExtent.height), Cap.minImageExtent.height) });
	}
	else {
		//!< そうでない場合はcurrentExtentを使用する (Otherwise, use currentExtent)
		return Cap.currentExtent;
	}
}
VkImageUsageFlags VK::SelectImageUsage(const VkSurfaceCapabilitiesKHR& Cap)
{
	//!< (サポートされていれば)イメージクリア用として VK_IMAGE_USAGE_TRANSFER_DST_BIT も立てている (If supported, set VK_IMAGE_USAGE_TRANSFER_DST_BIT for image clear)
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

	//!< 可能なら VK_PRESENT_MODE_MAILBOX_KHR を選択、そうでなければ VK_PRESENT_MODE_FIFO_KHR を選択
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
//void VK::CreateSwapchain(VkPhysicalDevice PD, VkSurfaceKHR Surface)
//{
//	CreateSwapchain(PD, Surface, Rect);
//	GetSwapchainImage(Device, Swapchain);
//	CreateSwapchainImageView();
//}
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

	//!< 最低よりも1枚多く取りたい、ただし最大値でクランプする(maxImageCount が0の場合は上限無し)
	const auto ImageCount = (std::min)(SurfaceCap.minImageCount + 1, 0 == SurfaceCap.maxImageCount ? UINT32_MAX : SurfaceCap.maxImageCount);
	Logf("\t\t\tImagCount = %d\n", ImageCount);

	//!< サーフェスのフォーマットを選択
	const auto SurfaceFormat = SelectSurfaceFormat(PD, Surface);
	ColorFormat = SurfaceFormat.format; //!< カラーファーマットは覚えておく

	//!< サーフェスのサイズを選択
	SurfaceExtent2D = SelectSurfaceExtent(SurfaceCap, Width, Height);
	Logf("\t\t\tSurfaceExtent = %d x %d\n", SurfaceExtent2D.width, SurfaceExtent2D.height);

	//!< レイヤー、ステレオレンダリング等をしたい場合は1以上になるが、ここでは1
	uint32_t ImageArrayLayers = 1;

	//!< サーフェスの使用法 (Surface usage)
	const auto ImageUsage = SelectImageUsage(SurfaceCap);

	//!< グラフィックとプレゼントのキューファミリが異なる場合はキューファミリインデックスの配列が必要、また VK_SHARING_MODE_CONCURRENT を指定すること
	//!< (ただし VK_SHARING_MODE_CONCURRENT にするとパフォーマンスが落ちる場合がある)
	std::vector<uint32_t> QueueFamilyIndices;
	if (GraphicsQueueFamilyIndex != PresentQueueFamilyIndex) {
		QueueFamilyIndices.push_back(GraphicsQueueFamilyIndex);
		QueueFamilyIndices.push_back(PresentQueueFamilyIndex);
	}

	//!< サーフェスを回転、反転等させるかどうか (Rotate, mirror surface or not)
	const auto SurfaceTransform = SelectSurfaceTransform(SurfaceCap);
	
	//!< サーフェスのプレゼントモードを選択
	const auto SurfacePresentMode = SelectSurfacePresentMode(PD, Surface);

	//!< 既存のは後で開放するので OldSwapchain に覚えておく (セッティングを変更してスワップチェインを再作成する場合等に備える)
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
	//	//if (!i.second.empty()) {
	//	//	vkFreeCommandBuffers(Device, i.first, static_cast<uint32_t>(i.second.size()), i.second.data());
	//	//	i.second.clear();
	//	//}
	//	vkDestroyCommandPool(Device, i.first, GetAllocationCallbacks());
	//}
	//CommandPools.clear();

	CreateSwapchain(GetCurrentPhysicalDevice(), Surface, Rect);
	GetSwapchainImage(Device, Swapchain);
	CreateSwapchainImageView();
}
void VK::GetSwapchainImage(VkDevice Device, VkSwapchainKHR Swapchain)
{
	//!< プレゼンテーションエンジンからイメージを取得する際、ハンドルではなく vkGetSwapchainImagesKHR() で取得したインデックスが返るので、ここで取得した順序は重要になる
	uint32_t Count;
	VERIFY_SUCCEEDED(vkGetSwapchainImagesKHR(Device, Swapchain, &Count, nullptr));
	assert(Count && "Swapchain image count is zero");
	SwapchainImages.clear();
	SwapchainImages.resize(Count);
	VERIFY_SUCCEEDED(vkGetSwapchainImagesKHR(Device, Swapchain, &Count, SwapchainImages.data()));

	LOG_OK();
}
/**
@note Vulaknでは、1つのコマンドバッファで複数のスワップチェインイメージをまとめて処理できるっぽい
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
					VK_IMAGE_LAYOUT_UNDEFINED, //!<「現在のレイアウト」または「UNDEFINED」を指定すること、イメージコンテンツを保持したい場合は「UNDEFINED」はダメ         
					VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, //!< プレゼンテーション可能な VK_IMAGE_LAYOUT_PRESENT_SRC_KHR へ
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
				//!< クリアカラーが指定されている場合 (If clear color is specified)
				const VkImageMemoryBarrier ImageMemoryBarrier_UndefinedToTransfer = {
					VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
					nullptr,
					0,
					VK_ACCESS_TRANSFER_WRITE_BIT,
					VK_IMAGE_LAYOUT_UNDEFINED, //!<「現在のレイアウト」または「UNDEFINED」を指定すること        
					VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, //!< デスティネーションへ
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
					VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, //!< デスティネーションから
					VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, //!< プレゼンテーション可能な VK_IMAGE_LAYOUT_PRESENT_SRC_KHR へ
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

	//!< キューにサブミットされたコマンドが完了するまでブロッキング (フェンスを用いないブロッキング方法)
	VERIFY_SUCCEEDED(vkQueueWaitIdle(GraphicsQueue));

	LOG_OK();
}

void VK::CreateSwapchainImageView()
{
	for(auto i : SwapchainImages) {
		VkImageView ImageView;
		CreateImageView(&ImageView, i, VK_IMAGE_VIEW_TYPE_2D, ColorFormat, ComponentMapping_Identity, ImageSubresourceRange_Color);
		
		SwapchainImageViews.push_back(ImageView);
	}

	LOG_OK();
}

void VK::CreateDepthStencil(const uint32_t Width, const uint32_t Height, const VkFormat DepthFormat)
{
	assert(IsSupportedDepthFormat(GetCurrentPhysicalDevice(), DepthFormat) && "Not supported depth format");

	const VkExtent3D Extent3D = { Width, Height, 1 };
	CreateImage(&DepthStencilImage, 0, VK_IMAGE_TYPE_2D, DepthFormat, Extent3D, 1, 1, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);
	CreateDeviceMemory(&DepthStencilDeviceMemory, DepthStencilImage, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	BindDeviceMemory(DepthStencilImage, DepthStencilDeviceMemory);
	CreateImageView(&DepthStencilImageView, DepthStencilImage, VK_IMAGE_VIEW_TYPE_2D, DepthFormat, ComponentMapping_Identity, ImageSubresourceRange_DepthStencil);

	LOG_OK();
}

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
			ImageSubresourceRange_DepthStencil
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
			//!< VKではデフォルトで「Yが下」を向くが、高さに負の値を指定すると「Yが上」を向きDXと同様になる
			//!< 通常基点は「左上」を指定するが、高さに負の値を指定する場合は「左下」を指定すること
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

void VK::CreateIndirectBuffer(VkBuffer* Buffer, VkDeviceMemory* DeviceMemory, const VkDeviceSize Size, const void* Source, const VkCommandBuffer CB)
{
	VkBuffer StagingBuffer = VK_NULL_HANDLE;
	VkDeviceMemory StagingDeviceMemory = VK_NULL_HANDLE;
	
	//!< ホストビジブルのバッファとメモリを作成し、そこへデータをコピーする (Create host visible buffer and memory, and copy data)
	CreateBuffer(&StagingBuffer, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, Size);
	CreateDeviceMemory(&StagingDeviceMemory, StagingBuffer, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
	CopyToDeviceMemory(StagingDeviceMemory, Size, Source);
	BindDeviceMemory(StagingBuffer, StagingDeviceMemory);

	//!< デバイスローカルのバッファとメモリを作成 (Create device local buffer and memory)
	CreateBuffer(Buffer, VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, Size);
	CreateDeviceMemory(DeviceMemory, *Buffer, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	BindDeviceMemory(*Buffer, *DeviceMemory);

	//!< View は必要ない (No need view)

	//!< ホストビジブルからデバイスローカルへのコピーコマンドを発行 Submit copy command host visible to device local
	CopyBufferToBuffer(CB, StagingBuffer, *Buffer, VK_ACCESS_INDIRECT_COMMAND_READ_BIT, VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT, Size);
	const std::vector<VkSubmitInfo> SIs = {
		{
			VK_STRUCTURE_TYPE_SUBMIT_INFO,
			nullptr,
			0, nullptr,
			nullptr,
			1, &CB,
			0, nullptr
		}
	};
	VERIFY_SUCCEEDED(vkQueueSubmit(GraphicsQueue, static_cast<uint32_t>(SIs.size()), SIs.data(), VK_NULL_HANDLE));
	VERIFY_SUCCEEDED(vkQueueWaitIdle(GraphicsQueue));

	if (VK_NULL_HANDLE != StagingDeviceMemory) {
		vkFreeMemory(Device, StagingDeviceMemory, GetAllocationCallbacks());
	}
	if (VK_NULL_HANDLE != StagingBuffer) {
		vkDestroyBuffer(Device, StagingBuffer, GetAllocationCallbacks());
	}
}

void VK::CreateUniformBuffer(VkBuffer* Buffer, VkDeviceMemory* DeviceMemory, const VkDeviceSize Size, const void* Source)
{
	const auto Usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;

	CreateBuffer(Buffer, Usage, Size);

	//!< #VK_TODO_PERF 本来はバッファ毎にメモリを確保するのではなく、予め大きなメモリを作成しておいてその一部を複数のバッファへ割り当てる方がよい
	CreateDeviceMemory(DeviceMemory, *Buffer, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
	CopyToDeviceMemory(*DeviceMemory, Size, Source);
	BindDeviceMemory(*Buffer, *DeviceMemory);

	//!< View は必要ない (No need view)

//#ifdef DEBUG_STDOUT
//	std::cout << "CreateUniformBuffer" << COUT_OK << std::endl << std::endl;
//#endif
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
		CreateDeviceMemory(DeviceMemory, *Buffer, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		BindMemory(*Buffer, *DeviceMemory);

		//!< View は必要ない (No need view)

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
		CreateDeviceMemory(DeviceMemory, *Buffer, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		BindDeviceMemory(*Buffer, *DeviceMemory);

		//!< UniformTexelBuffer の場合は、フォーマットを指定する必要があるため、ビューを作成する UniformTexelBuffer need format, so create view
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
		CreateDeviceMemory(DeviceMemory, *Buffer, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		BindDeviceMemory(*Buffer, *DeviceMemory);

		//!< UniformStorageBuffer の場合は、フォーマットを指定する必要があるため、ビューを作成する UniformStorageBuffer need format, so create view
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
@brief シェーダとのバインディング (DX::CreateRootSignature()相当)
@note デスクリプタを使用しない場合でも、デスクリプタセットレイアウト自体は作成しなくてはならない
@note シェーダからのアクセス時は set がVkDescriptorSetLayout番号、binding が(VkDescriptorSetLayout内の)VkDescriptorSetLayoutBinding番号 に相当する
(set = VkDescriptorSetLayout番号, binding = (VkDescriptorSetLayout内の)VkDescriptorSetLayoutBinding番号)
*/
void VK::CreateDescriptorSetLayout_deprecated()
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

	LOG_OK();
}

void VK::CreatePipelineLayout()
{
	const std::array<VkDescriptorSetLayoutBinding, 0> DSLBs = {};

	const VkDescriptorSetLayoutCreateInfo DSLCI = {
		VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
		nullptr,
		0,
		static_cast<uint32_t>(DSLBs.size()), DSLBs.data()
	};

	VkDescriptorSetLayout DSL = VK_NULL_HANDLE;
	VERIFY_SUCCEEDED(vkCreateDescriptorSetLayout(Device, &DSLCI, GetAllocationCallbacks(), &DSL));
	DescriptorSetLayouts.push_back(DSL);

	//!< デスクリプタセットよりも高速
	//!< パイプラインレイアウト全体で128 Byte(ハードが許せばこれ以上使える場合もある ex)GTX970M ... 256byte)
	//!< 各シェーダステージは1つのプッシュコンスタントにしかアクセスできない
	//!< 各々のシェーダステージが共通のレンジを持たないような「ワーストケース」では 128 / 5(シェーダステージ)で 1シェーダステージで 25 - 6 Byte程度になる
	const std::array<VkPushConstantRange, 0> PCRs = {
		//{ VK_SHADER_STAGE_VERTEX_BIT, 0, 64 },
		//{ VK_SHADER_STAGE_FRAGMENT_BIT, 64, 64 },
	};

	const VkPipelineLayoutCreateInfo PipelineLayoutCreateInfo = {
		VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
		nullptr,
		0,
		static_cast<uint32_t>(DescriptorSetLayouts.size()), DescriptorSetLayouts.data(),
		static_cast<uint32_t>(PCRs.size()), PCRs.data()
	};
	VERIFY_SUCCEEDED(vkCreatePipelineLayout(Device, &PipelineLayoutCreateInfo, GetAllocationCallbacks(), &PipelineLayout));

	LOG_OK();
}
void VK::CreateDescriptorSet()
{
	if (!DescriptorSetLayouts.empty() && VK_NULL_HANDLE != DescriptorPool) {
		const VkDescriptorSetAllocateInfo DSAI = {
			VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
			nullptr,
			DescriptorPool,
			static_cast<uint32_t>(DescriptorSetLayouts.size()), DescriptorSetLayouts.data()
		};
		DescriptorSets.resize(DescriptorSetLayouts.size());
		VERIFY_SUCCEEDED(vkAllocateDescriptorSets(Device, &DSAI, DescriptorSets.data()));
	}

	LOG_OK();
}
/**
@brief シェーダとのバインディングのレイアウト
@note DescriptorSet は「DescriptorSetLayt 型」のインスタンスのようなもの
@note 更新は vkUpdateDescriptorSets() で行う
*/
void VK::CreateDescriptorSet_deprecated()
{
	std::vector<VkDescriptorPoolSize> DescriptorPoolSizes = {
		/**
		VkDescriptorType    type; ... VK_DESCRIPTOR_TYPE_[SAMPLER, SAMPLED_IMAGE, UNIFORM_BUFFER, ...]
		uint32_t            descriptorCount;
		*/
	};
	CreateDescriptorPoolSizes(DescriptorPoolSizes);

	if (!DescriptorPoolSizes.empty()) {
		//!< プールを作成 Create pool
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
			0/*VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT*/, //!< デスクリプタセットを個々に解放したい場合に指定(プールごとの場合は不要)
			MaxSets,
			static_cast<uint32_t>(DescriptorPoolSizes.size()), DescriptorPoolSizes.data()
		};
		VERIFY_SUCCEEDED(vkCreateDescriptorPool(Device, &DescriptorPoolCreateInfo, GetAllocationCallbacks(), &DescriptorPool));
		assert(VK_NULL_HANDLE != DescriptorPool && "Failed to create descriptor pool");

		//!< プールからデスクリプタセットを作成 Create descriptor set from pool
		const VkDescriptorSetAllocateInfo DescriptorSetAllocateInfo = {
			VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
			nullptr,
			DescriptorPool,
			static_cast<uint32_t>(DescriptorSetLayouts.size()), DescriptorSetLayouts.data()
		};
		DescriptorSets.resize(DescriptorSetLayouts.size());
		VERIFY_SUCCEEDED(vkAllocateDescriptorSets(Device, &DescriptorSetAllocateInfo, DescriptorSets.data()));

		LOG_OK();
	}
}

void VK::DestroyFramebuffer()
{
	for (auto i : Framebuffers) {
		vkDestroyFramebuffer(Device, i, GetAllocationCallbacks());
	}
	Framebuffers.clear();
}

/**
@brief シェーダコンパイル、リンクはパイプラインオブジェクト作成時に行われる Shader compilation and linkage is performed during the pipeline object creation
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
@brief DXに合わせるため、ここでは VkDynamicState を使用している For compatibility with DX, we use VkDynamicState
ここでは個数のみを指定して nullptr を指定している、後で vkCmdSetViewport(), vkCmdSetScissor() で指定すること We specifying only count, and set nulloptr, later use vkCmdSetViewport(), vkCmdSetScissor()
2つ以上のビューポートを使用するにはデバイスフィーチャー multiViewport が有効であること If we use 2 or more viewport device feature multiViewport must be enabled
ビューポートのインデックスはジオメトリシェーダで指定する Viewport index is specified in geometry shader
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

bool VK::ValidatePipelineCache(const size_t Size, const void* Data) const
{
	VkPhysicalDeviceProperties PDP;
	vkGetPhysicalDeviceProperties(GetCurrentPhysicalDevice(), &PDP);

	auto Ptr = reinterpret_cast<const uint32_t*>(Data);
	const auto PCSize = *Ptr++;
	const auto PCVersion = *Ptr++;
	const auto PCVenderID = *Ptr++;
	const auto PCDeviceID = *Ptr++;
	uint8_t PCUUID[VK_UUID_SIZE];
	memcpy(PCUUID, Ptr, sizeof(PCUUID));

	assert(Size == PCSize && "");
	assert(VK_PIPELINE_CACHE_HEADER_VERSION_ONE == PCVersion && "");
	assert(PDP.vendorID == PCVenderID && "");
	assert(PDP.deviceID == PCDeviceID && "");
	//assert(0 == memcmp(PDP.pipelineCacheUUID, PCUUID, sizeof(PDP.pipelineCacheUUID)) && "");

	Log("Validate PipelineCache\n");
	Logf("\tSize = %d\n", PCSize);
	Logf("\tVersion = %s\n", PCVersion == VK_PIPELINE_CACHE_HEADER_VERSION_ONE ? "VK_PIPELINE_CACHE_HEADER_VERSION_ONE" : "Unknown");
	Logf("\tVenderID = %d\n", PCVenderID);
	Logf("\tDeviceID = %d\n", PCDeviceID);
	Log("\tUUID = "); for (auto i = 0; i < sizeof(PCUUID); ++i) { Logf("%c", PCUUID[i]); } Log("\n");
	//Log("\tUUID = "); for (auto i = 0; i < sizeof(PDP.pipelineCacheUUID); ++i) { Logf("%c", PDP.pipelineCacheUUID[i]); } Log("\n");

	return Size == PCSize && VK_PIPELINE_CACHE_HEADER_VERSION_ONE == PCVersion && PDP.vendorID == PCVenderID && PDP.deviceID == PCDeviceID/*&& 0 == memcmp(PDP.pipelineCacheUUID, PCUUID, sizeof(PDP.pipelineCacheUUID))*/;
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

		char* Data = nullptr;
		if (Size) {
			Data = new char[Size];
			In.read(Data, Size);

			ValidatePipelineCache(Size, Data);

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
		}

		In.close();
	}
	else {
		const VkPipelineCacheCreateInfo PipelineCacheCreateInfo = {
				VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO,
				nullptr,
				0,
				0, nullptr
		};
		VERIFY_SUCCEEDED(vkCreatePipelineCache(Device, &PipelineCacheCreateInfo, GetAllocationCallbacks(), &PipelineCache));
	}

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

	ValidatePipelineCache(Size, Data);

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
	//!< スレッドでパイプライン (ThreadCount * PipelineCountPerThread 個) を作成する例 (パイプラインキャッシュを活用する)
	const auto ThreadCount = 10;
	const auto PipelineCountPerThread = 1;

	//!< パイプラインキャッシュをファイルから読み込む (スレッド数分だけ(同じものを)用意する)
	std::vector<VkPipelineCache> PipelineCaches(ThreadCount);
	const auto PCOPath = GetBasePath() + TEXT(".pco");
	LoadPipelineCaches(PCOPath, PipelineCaches);

	//!< パイプライン格納先
	std::vector<std::vector<VkPipeline>> Pipelines(ThreadCount);
	for (auto i : Pipelines) { 
		i.resize(PipelineCountPerThread); 
	}

	//!< パイプラインキャッシュを使用して、各々のスレッドでパイプラインを作成する
	std::vector<std::thread> Threads(ThreadCount);
	for (auto i = 0; i < Threads.size(); ++i) {
		Threads[i] = std::thread::thread([&](std::vector<VkPipeline>& Pipelines, VkPipelineCache PipelineCache) {
			//!< #VK_TODO
			//!< パイプラインを作成の処理をここに書く Create pipeline here 
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

	//!< 各々のスレッドが作成したパイプラインキャッシュをマージする (ここでは最後の要素へマージしている)
	//!< デスティネーションがソース群に「含まれていてはならない」ので注意 (ソースの個数をPipelineCaches.size() - 1として、最後の要素を含めないようにしている)
	VERIFY_SUCCEEDED(vkMergePipelineCaches(Device, PipelineCaches.back(), static_cast<uint32_t>(PipelineCaches.size() - 1), PipelineCaches.data()));
	//!< マージ後のパイプラインキャッシュをファイルへ保存
	StorePipelineCache(PCOPath, PipelineCaches.back());
	//!< もう破棄して良い
	for (auto& i : PipelineCaches) {
		vkDestroyPipelineCache(Device, i, GetAllocationCallbacks());
	}
#else

#ifdef LOAD_PIPELINE
	const auto PCOPath = GetBasePath() + TEXT(".pco");
	PipelineCache = LoadPipelineCache(PCOPath);
#endif

#if 1
	auto Thread = std::thread::thread([&]() { CreatePipeline_Graphics(); });
#else
	ShaderModules.resize(5);
	const auto ShaderPath = GetBasePath();
	ShaderModules[0] = CreateShaderModule((ShaderPath + TEXT(".vert.spv")).data());
	ShaderModules[1] = CreateShaderModule((ShaderPath + TEXT(".frag.spv")).data());

	auto Thread = std::thread::thread([&](VkPipeline& P, const VkPipelineLayout PL,
		const VkShaderModule VS, const VkShaderModule FS, const VkShaderModule TES, const VkShaderModule TCS, const VkShaderModule GS, 
		const VkRenderPass RP)
		{ CreatePipeline_Default(P, PL, VS, FS, TES, TCS, GS, RP); }, 
		std::ref(Pipeline), PipelineLayout, NullShaderModule, NullShaderModule, NullShaderModule, NullShaderModule, NullShaderModule, RenderPass);
#endif

	Thread.join();

#ifdef LOAD_PIPELINE
	if (VK_NULL_HANDLE != PipelineCache) {
		VkPipelineCache MergedPipelineCache = VK_NULL_HANDLE;
		const VkPipelineCacheCreateInfo PCCI = {
				VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO,
				nullptr,
				0,
				0, nullptr
		};
		VERIFY_SUCCEEDED(vkCreatePipelineCache(Device, &PCCI, GetAllocationCallbacks(), &MergedPipelineCache));

		const std::array<VkPipelineCache, 1> PipelineCaches = { PipelineCache };
		VERIFY_SUCCEEDED(vkMergePipelineCaches(Device, MergedPipelineCache, static_cast<uint32_t>(PipelineCaches.size()), PipelineCaches.data()));
		StorePipelineCache(PCOPath, MergedPipelineCache);

		vkDestroyPipelineCache(Device, MergedPipelineCache, GetAllocationCallbacks());
		vkDestroyPipelineCache(Device, PipelineCache, GetAllocationCallbacks());
	}
#endif
#endif
}

void VK::CreatePipeline_Default(VkPipeline& Pipeline, const VkPipelineLayout PipelineLayout,
	const VkShaderModule VS, const VkShaderModule FS, const VkShaderModule TES, const VkShaderModule TCS, const VkShaderModule GS, 
	const VkRenderPass RenderPass)
{
	PERFORMANCE_COUNTER();

	//!< (バリデーションの為に)デバイスフィーチャーを取得
	VkPhysicalDeviceFeatures PDF;
	vkGetPhysicalDeviceFeatures(GetCurrentPhysicalDevice(), &PDF);

	//!< シェーダ (Shader)
	std::vector<VkPipelineShaderStageCreateInfo> PSSCI;
	//!< シェーダ内のコンスタント変数をパイプライン作成時に変更する場合に使用
	//const std::array<VkSpecializationMapEntry, 0> SMEs = { /*{ uint32_t constantID, uint32_t offset, size_t size },*/ };
	//const VkSpecializationInfo SI = { static_cast<uint32_t>(SMEs.size()), SMEs.data() };
	if (VK_NULL_HANDLE != VS) {
		PSSCI.push_back({
			VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
			nullptr,
			0,
			VK_SHADER_STAGE_VERTEX_BIT, VS,
			"main",
			nullptr//&SI
		});
	}
	if (VK_NULL_HANDLE != FS) {
		PSSCI.push_back({
			VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
			nullptr,
			0,
			VK_SHADER_STAGE_FRAGMENT_BIT, FS,
			"main",
			nullptr
		});
	}
	if (VK_NULL_HANDLE != TES) {
		PSSCI.push_back({
			VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
			nullptr,
			0,
			VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT, TES,
			"main",
			nullptr
		});
	}
	if (VK_NULL_HANDLE != TCS) {
		PSSCI.push_back({
			VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
			nullptr,
			0,
			VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT, TCS,
			"main",
			nullptr
		});
	}
	if (VK_NULL_HANDLE != GS) {
		PSSCI.push_back({
			VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
			nullptr,
			0,
			VK_SHADER_STAGE_GEOMETRY_BIT, GS,
			"main",
			nullptr
		});
	}
	assert(!PSSCI.empty() && "");

	//!< バーテックスインプット (VertexInput)
	const std::array<VkVertexInputBindingDescription, 0> VIBDs = {};
	const std::array<VkVertexInputAttributeDescription, 0> VIADs = {};
	const VkPipelineVertexInputStateCreateInfo PVISCI = {
		VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
		nullptr,
		0,
		static_cast<uint32_t>(VIBDs.size()), VIBDs.data(),
		static_cast<uint32_t>(VIADs.size()), VIADs.data()
	};

	//!< インプットアセンブリ (InputAssembly)
	const VkPipelineInputAssemblyStateCreateInfo PIASCI = {
		VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
		nullptr,
		0,
		VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP,
		VK_FALSE
	};
	//!< WITH_ADJACENCY 系使用時には デバイスフィーチャー geometryShader が有効であること
	assert((
		(PIASCI.topology != VK_PRIMITIVE_TOPOLOGY_LINE_LIST_WITH_ADJACENCY || PIASCI.topology != VK_PRIMITIVE_TOPOLOGY_LINE_STRIP_WITH_ADJACENCY || PIASCI.topology != VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST_WITH_ADJACENCY || PIASCI.topology != VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP_WITH_ADJACENCY)
		|| PDF.geometryShader) && "");
	//!< PATCH_LIST 使用時には デバイスフィーチャー tessellationShader が有効であること
	assert((PIASCI.topology != VK_PRIMITIVE_TOPOLOGY_PATCH_LIST || PDF.tessellationShader) && "");
	//!< インデックス 0xffffffff(VK_INDEX_TYPE_UINT32), 0xffff(VK_INDEX_TYPE_UINT16) をプリミティブのリスタートとする、インデックス系描画の場合(vkCmdDrawIndexed, vkCmdDrawIndexedIndirect)のみ有効
	//!< LIST 系使用時 primitiveRestartEnable 無効であること
	assert((
		(PIASCI.topology != VK_PRIMITIVE_TOPOLOGY_POINT_LIST || PIASCI.topology != VK_PRIMITIVE_TOPOLOGY_LINE_LIST || PIASCI.topology != VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST || PIASCI.topology != VK_PRIMITIVE_TOPOLOGY_LINE_LIST_WITH_ADJACENCY || PIASCI.topology != VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST_WITH_ADJACENCY || PIASCI.topology != VK_PRIMITIVE_TOPOLOGY_PATCH_LIST)
		|| PIASCI.primitiveRestartEnable == VK_FALSE) && "");

	//!< テセレーション (Tessellation)
	const VkPipelineTessellationStateCreateInfo PTSCI = {
		VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO,
		nullptr, 
		0, 
		0//!< patchControlPoints
	};

	//!< ビューポート (Viewport)
	//!< VkDynamicState を使用するため、ここではビューポート(シザー)の個数のみ指定している (To use VkDynamicState, specify only count of viewport(scissor) here)
	//!< 後に vkCmdSetViewport(), vkCmdSetScissor() で指定する (Use vkCmdSetViewport(), vkCmdSetScissor() later)
	const VkPipelineViewportStateCreateInfo PVSCI = {
		VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
		nullptr,
		0,
		1, nullptr, //!< Viewport
		1, nullptr	//!< Scissor
	};
	//!< 2つ以上のビューポートを使用するにはデバイスフィーチャー multiViewport が有効であること (If use 2 or more viewport device feature multiViewport must be enabled)
	//!< ビューポートのインデックスはジオメトリシェーダで指定する (Viewport index is specified in geometry shader)
	assert((PVSCI.viewportCount <= 1 || PDF.multiViewport) && "");

	//!< ラスタライゼーション (Rasterization)
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
	//!< depthClampEnable にはデバイスフィーチャー depthClamp が有効であること
	assert((!PRSCI.depthClampEnable || PDF.depthClamp) && "");
	//!< FILL 以外にはデバイスフィーチャー fillModeNonSolid が有効であること
	assert((PRSCI.polygonMode != VK_POLYGON_MODE_FILL || PDF.fillModeNonSolid) && "");
	//!< 1.0f より大きな値にはデバイスフィーチャー widelines が有効であること
	assert((PRSCI.lineWidth <= 1.0f || PDF.wideLines) && "");

	//!< マルチサンプル (Multisample)
	const VkSampleMask SM = 0xffffffff; //!< 0xffffffff 指定の場合は nullptr でもよい
	const VkPipelineMultisampleStateCreateInfo PMSCI = {
		VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
		nullptr,
		0,
		VK_SAMPLE_COUNT_1_BIT,
		VK_FALSE, 0.0f, //! VK_TRUE にするにはデバイスフィーチャー minSampleShading が有効であること
		&SM,
		VK_FALSE, VK_FALSE //!< alphaToOneEnable を VK_TRUE にするにはデバイスフィーチャー alphaToOne が有効であること
	};

	//!< デプスステンシル (DepthStencil)
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

	//!< カラーブレンド (ColorBlend)
	//!< デバイスフィーチャー independentBlend が有効で無い場合は、配列の各要素は「完全に同じ値」であること If device feature independentBlend is not enabled, each array element must be exactly same
	//!< VK_BLEND_FACTOR_SRC1 系をを使用するには、デバイスフィーチャー dualSrcBlend が有効であること
	///!< SRCコンポーネント * SRCファクタ OP DSTコンポーネント * DSTファクタ
	const std::array<VkPipelineColorBlendAttachmentState, 1> PCBASs = {
		{
			VK_FALSE,
			VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ONE, VK_BLEND_OP_ADD,
			VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ONE, VK_BLEND_OP_ADD,
			VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
		}
	};
	const VkPipelineColorBlendStateCreateInfo PCBSCI = {
		VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
		nullptr,
		0,
		VK_FALSE, VK_LOGIC_OP_COPY, //!< ブレンド時に論理オペレーションを行う (ブレンドは無効になる) (整数型アタッチメントに対してのみ)
		static_cast<uint32_t>(PCBASs.size()), PCBASs.data(),
		{ 1.0f, 1.0f, 1.0f, 1.0f }
	};

	//!< ダイナミックステート (DynamicState)
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
	@brief 継承
	basePipelineHandle, basePipelineIndex は同時に使用できない、それぞれ使用しない場合は VK_NULL_HANDLE, -1 を指定	
	親には VK_PIPELINE_CREATE_ALLOW_DERIVATIVES_BIT フラグが必要、子には VK_PIPELINE_CREATE_DERIVATIVE_BIT フラグが必要		
	・basePipelineHandle ... 既に親とするパイプラインが存在する場合に指定
	・basePipelineIndex ... 同配列内で親パイプラインも同時に作成する場合、配列内での親パイプラインの添字、親の添字の方が若い値でないといけない
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
			&PVISCI,
			&PIASCI,
			&PTSCI,
			&PVSCI,
			&PRSCI,
			&PMSCI,
			&PDSSCI,
			&PCBSCI,
			&PDSCI,
			PipelineLayout,
			RenderPass, 0, //!< 指定したレンダーパス限定ではなく、互換性のある他のレンダーパスでも使用可能
			VK_NULL_HANDLE, -1 //!< basePipelineHandle, basePipelineIndex
		}
	};

	VERIFY_SUCCEEDED(vkCreateGraphicsPipelines(Device,
		/*PipelineCache*/VK_NULL_HANDLE, //!< #VK_TODO
		static_cast<uint32_t>(GPCIs.size()), GPCIs.data(),
		GetAllocationCallbacks(),
		&Pipeline));

	LOG_OK();
}

void VK::CreatePipeline_Graphics()
{
#ifdef _DEBUG
	PerformanceCounter PC(__func__);
#endif

	//!< シェーダ
	std::vector<VkShaderModule> SMs;
	std::vector<VkPipelineShaderStageCreateInfo> PSSCIs;
	CreateShader(SMs, PSSCIs);

	//!< バーテックスインプット
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

	//!< インプットアセンブリ (トポロジ)
	VkPipelineInputAssemblyStateCreateInfo PipelineInputAssemblyStateCreateInfo;
	CreateInputAssembly(PipelineInputAssemblyStateCreateInfo);

	//!< テセレーションステート (テセレーションを使用する場合には、不定値のままにしないこと)
	VkPipelineTessellationStateCreateInfo PipelineTessellationStateCreateInfo;
	CreateTessellationState(PipelineTessellationStateCreateInfo);

	//!< ビューポート(シザー)
	VkPipelineViewportStateCreateInfo PipelineViewportStateCreateInfo;
	CreateViewportState(PipelineViewportStateCreateInfo);

	const VkPipelineRasterizationStateCreateInfo PipelineRasterizationStateCreateInfo = {
		VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
		nullptr,
		0,
		VK_FALSE, //!< VK_TRUE にするにはデバイスフィーチャー depthClampEnable が有効であること
		VK_FALSE,
		VK_POLYGON_MODE_FILL, //!< LINE や POINT を有効にするにはデバイスフィーチャー fillModeNonSolid が有効であること
		VK_CULL_MODE_BACK_BIT,
		VK_FRONT_FACE_COUNTER_CLOCKWISE,
		VK_FALSE, 0.0f, 0.0f, 0.0f,
		1.0f //!< 1.0f より大きな値を指定するにはデバイスフィーチャー widelines が有効であること
	};

	//const VkSampleMask SampleMask = 0xffffffff;
	const VkPipelineMultisampleStateCreateInfo PipelineMultisampleStateCreateInfo = {
		VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
		nullptr,
		0,
		VK_SAMPLE_COUNT_1_BIT,
		VK_FALSE, 0.0f, //! VK_TRUE にするにはデバイスフィーチャー minSampleShading が有効であること
		nullptr/*&SampleMask*/, //!< 0xffffffff の場合 nullptr でよい
		VK_FALSE, VK_FALSE //!< alphaToOneEnable を VK_TRUE にするにはデバイスフィーチャー alphaToOne が有効であること
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

	//!< デバイスフィーチャー independentBlend が有効で無い場合は、配列の各要素は「完全に同じ値」であること If device feature independentBlend is not enabled, each array element must be exactly same
	//!< VK_BLEND_FACTOR_SRC1 系をを使用するには、デバイスフィーチャー dualSrcBlend が有効であること
	///!< SRCコンポーネント * SRCファクタ OP DSTコンポーネント * DSTファクタ
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
		VK_FALSE, VK_LOGIC_OP_COPY, //!< ブレンド時に論理オペレーションを行う (ブレンドは無効になる) (整数型アタッチメントに対してのみ)
		static_cast<uint32_t>(PipelineColorBlendAttachmentStates.size()), PipelineColorBlendAttachmentStates.data(),
		{ 1.0f, 1.0f, 1.0f, 1.0f }
	};

	//!< ダイナミックステート
	std::vector<VkDynamicState> DynamicStates;
	CreateDynamicState(DynamicStates);
	const VkPipelineDynamicStateCreateInfo PipelineDynamicStateCreateInfo = {
		VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
		nullptr,
		0,
		static_cast<uint32_t>(DynamicStates.size()), DynamicStates.data()
	};

	/**
	@brief 継承
	basePipelineHandle, basePipelineIndex は同時に使用できない、それぞれ使用しない場合は VK_NULL_HANDLE, -1 を指定
	親には VK_PIPELINE_CREATE_ALLOW_DERIVATIVES_BIT フラグが必要、子には VK_PIPELINE_CREATE_DERIVATIVE_BIT フラグが必要
	・basePipelineHandle ... 既に親とするパイプラインが存在する場合に指定
	・basePipelineIndex ... 同配列内で親パイプラインも同時に作成する場合、配列内での親パイプラインの添字、親の添字の方が若い値でないといけない
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
			static_cast<uint32_t>(PSSCIs.size()), PSSCIs.data(),
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
			RenderPass, 0, //!< 指定したレンダーパス限定ではなく、互換性のある他のレンダーパスでも使用可能
			VK_NULL_HANDLE, -1 //!< basePipelineHandle, basePipelineIndex
		}
	};

#if 0
	//!< パイプラインキャッシュオブジェクト Pipeline Cache Object
	const auto PCOPath = GetBasePath() + TEXT(".pco");
	PipelineCache = LoadPipelineCache(PCOPath);
	//!< パイプライン作成時にキャッシュが活用される、また今回の作成結果もキャッシュされる
	VERIFY_SUCCEEDED(vkCreateGraphicsPipelines(Device,
		PipelineCache,
		static_cast<uint32_t>(GraphicsPipelineCreateInfos.size()), GraphicsPipelineCreateInfos.data(), 
		GetAllocationCallbacks(), 
		&Pipeline));
	if (VK_NULL_HANDLE != PipelineCache) {
		StorePipelineCache(PCOPath, PipelineCache);
		vkDestroyPipelineCache(Device, PipelineCache, GetAllocationCallbacks());
	}
#else
	VERIFY_SUCCEEDED(vkCreateGraphicsPipelines(Device,
		PipelineCache,
		static_cast<uint32_t>(GraphicsPipelineCreateInfos.size()), GraphicsPipelineCreateInfos.data(),
		GetAllocationCallbacks(),
		&Pipeline));
#endif

	//!< パイプライン を作成したら、シェーダモジュール は破棄して良い
	for (auto i : SMs) {
		vkDestroyShaderModule(Device, i, GetAllocationCallbacks());
	}
	SMs.clear();

	LOG_OK();
}
void VK::CreatePipeline_Compute()
{
#ifdef _DEBUG
	PerformanceCounter PC(__func__);
#endif

	std::vector<VkShaderModule> ShaderModules;
	std::vector<VkPipelineShaderStageCreateInfo> PipelineShaderStageCreateInfos;
	CreateShader(ShaderModules, PipelineShaderStageCreateInfos);
	
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

#if 0
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
#else
	VERIFY_SUCCEEDED(vkCreateComputePipelines(Device,
		PipelineCache,
		static_cast<uint32_t>(ComputePipelineCreateInfos.size()), ComputePipelineCreateInfos.data(),
		GetAllocationCallbacks(),
		&Pipeline));
#endif

	for (auto i : ShaderModules) {
		vkDestroyShaderModule(Device, i, GetAllocationCallbacks());
	}
	ShaderModules.clear();

	LOG_OK();
}

/**
@brief クリア Clear
@note 「レンダーパス外」でクリア Out of renderpass ... vkCmdClearColorImage()
@note 「レンダーパス開始時」にクリア Begining of renderpass ... VkAttachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR, VkRenderPassBeginInfo.pClearValues
@note 「各々のサブパス」でクリア Each subpass ... vkCmdClearAttachments()
*/
//!< 「レンダーパス外」にてクリアを行う
void VK::ClearColor(const VkCommandBuffer CB, const VkImage Image, const VkClearColorValue& Color)
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
	vkCmdPipelineBarrier(CB,
		VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
		0,
		0, nullptr,
		0, nullptr,
		1, &ImageMemoryBarrier_PresentToTransfer);
	{
		//!< vkCmdClearColorImage() はレンダーパス内では使用できない
		vkCmdClearColorImage(CB, Image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &Color, 1, &ImageSubresourceRange_Color);
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
	vkCmdPipelineBarrier(CB,
		VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
		0,
		0, nullptr,
		0, nullptr,
		1, &ImageMemoryBarrier_TransferToPresent);
}
void VK::ClearDepthStencil(const VkCommandBuffer CB, const VkImage Image, const VkClearDepthStencilValue& DepthStencil)
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
	vkCmdPipelineBarrier(CB,
		VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
		0,
		0, nullptr,
		0, nullptr,
		1, &ImageMemoryBarrier_DepthToTransfer);
	{
		//!< vkCmdClearDepthStencilImage() はレンダーパス内では使用できない
		vkCmdClearDepthStencilImage(CB, Image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &ClearDepthStencilValue, 1, &ImageSubresourceRange_DepthStencil);
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
	vkCmdPipelineBarrier(CB,
		VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
		0,
		0, nullptr,
		0, nullptr,
		1, &ImageMemoryBarrier_TransferToDepth);
}
//!<「サブパス」にてクリアするときに使う
//!< Drawコール前に使用すると、「それなら VkAttachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR を使え」と Warnning が出るので注意
void VK::ClearColorAttachment(const VkCommandBuffer CB, const VkClearColorValue& Color)
{
	const VkClearValue ClearValue = { Color };
	const std::vector<VkClearAttachment> ClearAttachments = {
		{
			VK_IMAGE_ASPECT_COLOR_BIT,
			0, //!< カラーアタッチメントのインデックス #VK_TODO 現状決め打ち
			ClearValue
		},
	};
	const std::vector<VkClearRect> ClearRects = {
		{
			ScissorRects[0],
			0, 1 //!< 開始レイヤとレイヤ数 #VK_TODO 現状決め打ち
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
			0, //!< ここでは無視される
			ClearValue
		},
	};
	const std::vector<VkClearRect> ClearRects = {
		{
			ScissorRects[0],
			0, 1 //!< 開始レイヤとレイヤ数 #VK_TODO 現状決め打ち
		},
	};
	vkCmdClearAttachments(CB,
		static_cast<uint32_t>(ClearAttachments.size()), ClearAttachments.data(),
		static_cast<uint32_t>(ClearRects.size()), ClearRects.data());
}

void VK::PopulateCommandBuffer(const size_t i)
{
	const auto CB = CommandPools[0].second[i];
	const auto FB = Framebuffers[i];
	const auto Image = SwapchainImages[i];

	//!< vkBeginCommandBuffer() で暗黙的にリセットされるが、明示的にリセットする場合には「メモリをプールへリリースするかどうかを指定できる」
	//VERIFY_SUCCEEDED(vkResetCommandBuffer(CB, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT));

	//!< @brief VkCommandBufferUsageFlags
	//!< * VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT		... 一度だけ使用する場合や毎回リセットする場合に指定、何度もサブミットするものには指定しない
	//!< * VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT		... デバイスでまだ実行されている間に、コマンドバッファを再度サブミットする必要がある場合に指定 (パフォーマンスの観点からは避けるべき)
	//!< * VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT	... セカンダリコマンドバッファでかつレンダーパス内の場合に指定する
	const VkCommandBufferBeginInfo CBBI = {
		VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		nullptr,
		0,
		nullptr
	};
	VERIFY_SUCCEEDED(vkBeginCommandBuffer(CB, &CBBI)); {
		//!< ビューポート、シザー
		vkCmdSetViewport(CB, 0, static_cast<uint32_t>(Viewports.size()), Viewports.data());
		vkCmdSetScissor(CB, 0, static_cast<uint32_t>(ScissorRects.size()), ScissorRects.data());

#if 1
		//!< クリア
		ClearColor(CB, Image, Colors::SkyBlue);
#endif

#ifdef _DEBUG
		//!< レンダーエリアの最低粒度を確保
		VkExtent2D Granularity;
		vkGetRenderAreaGranularity(Device, RenderPass, &Granularity);
		//!<「自分の環境では」 Granularity = { 1, 1 } だったのでほぼなんでも大丈夫みたい、環境によっては注意が必要
		assert(ScissorRects[0].extent.width >= Granularity.width && ScissorRects[0].extent.height >= Granularity.height && "ScissorRect is too small");
#endif
		//!< (ここでは)レンダーパス開始時にカラーはクリアせず、デプスはクリアしている (In this case, not clear color, but clear depth on begining of renderpas)
		std::vector<VkClearValue> ClearValues(2);
		//ClearValues[0].color = Colors::SkyBlue; //!< If VkAttachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR, need this
		ClearValues[1].depthStencil = ClearDepthStencilValue;
		const VkRenderPassBeginInfo RPBI = {
			VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
			nullptr,
			RenderPass,
			FB,
			ScissorRects[0], //!< フレームバッファのサイズ以下を指定できる
			static_cast<uint32_t>(ClearValues.size()), ClearValues.data()
		};

		vkCmdBeginRenderPass(CB, &RPBI, VK_SUBPASS_CONTENTS_INLINE); {
			//vkCmdBindPipeline();
			//vkCmdBindDescriptorSets();
			//vkCmdBindVertexBuffers();
			//vkCmdBindIndexBuffer();
			//vkCmdDrawIndirect();

			//!< vkCmdNextSubpass(CB, VK_SUBPASS_CONTENTS_INLINE);

#pragma region SecondaryCB
			//!< セカンダリコマンドバッファを使用する場合 (In case use secondary command buffer)
			std::vector<VkCommandBuffer> SCBs = {}; //!< ここでは空なので何もしない (In this case, vector is empty so do nothing)
			if (!SCBs.empty()) {
				//!< * 基本的に、セカンダリ(コマンドバッファ)はプライマリ(コマンドバッファ)のステートを継承しない
				//!< * セカンダリ記録後のプライマリのステートも未定義、プライマリに戻って再記録する場合はステートを再設定しなくてはならない
				//!< * 例外) プライマリがレンダーパス内でそこからセカンダリを呼び出す場合には、プライマリのレンダーパス、サプバスステートは継承される
				//!< * 全てのコマンドがプライマリ、セカンダリの両方で記録できるわけではない
				//!< * セカンダリは直接サブミットできない、プライマリから呼び出される ... vkCmdExecuteCommands(プライマリ, セカンダリ個数, セカンダリ配列);
				//!< * セカンダリの場合は VK_SUBPASS_CONTENTS_INLINE の代わりに VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS を指定する ... vkCmdBeginRenderPass(, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS), vkCmdNextSubpass(, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);
				for (auto i : SCBs) {
					const VkCommandBufferInheritanceInfo CBII = {}; //!< #VK_TODO
					const VkCommandBufferBeginInfo SCBBI = {
						VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
						nullptr,
						VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT, //!< セカンダリコマンドバッファでかつレンダーパス内の場合に指定する
						&CBII
					};
					VERIFY_SUCCEEDED(vkBeginCommandBuffer(i, &SCBBI)); {
						const VkRenderPassBeginInfo RPBI = {}; //!< #VK_TODO
						vkCmdBeginRenderPass(i, &RPBI, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS); { //!< セカンダリなので VK_SUBPASS_CONTENTS_INLINE ではなく VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS を指定する
						} vkCmdEndRenderPass(i);
						vkCmdNextSubpass(i, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS); //!< セカンダリなので VK_SUBPASS_CONTENTS_INLINE ではなく VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS を指定する
					} VERIFY_SUCCEEDED(vkEndCommandBuffer(i));
				}
				vkCmdExecuteCommands(CB, static_cast<uint32_t>(SCBs.size()), SCBs.data());
			}
#pragma endregion

		} vkCmdEndRenderPass(CB);
	} VERIFY_SUCCEEDED(vkEndCommandBuffer(CB));
}

void VK::Draw()
{
	//!< サブミットしたコマンドの完了を待つ
	std::vector<VkFence> Fences = { Fence };
#if 1
	VERIFY_SUCCEEDED(vkWaitForFences(Device, static_cast<uint32_t>(Fences.size()), Fences.data(), VK_TRUE, (std::numeric_limits<uint64_t>::max)()));
#else
	while (VK_TIMEOUT == vkWaitForFences(Device, static_cast<uint32_t>(Fences.size()), Fences.data(), VK_TRUE, 0)) { Log("vkWaitForFences() == VK_TIMEOUT\n"); }
#endif
	vkResetFences(Device, static_cast<uint32_t>(Fences.size()), Fences.data());

	//!< 次のイメージが取得できるまでブロック(タイムアウトは指定可能)、取得できたからといってイメージは直ぐに目的に使用可能とは限らない
	//!< (引数で指定した場合)使用可能になるとフェンスやセマフォがシグナルされる
	//!< ここではセマフォを指定し、このセマフォはサブミット時に使用する(サブミットしたコマンドがプレゼンテーションを待つように指示している)
	//!< (ハンドルではなく)SwapchainImages のインデックスが　SwapchainImageIndex に返る (Index(not handle) of SwapchainImages will return to SwapchainImageIndex)
	VERIFY_SUCCEEDED(vkAcquireNextImageKHR(Device, Swapchain, UINT64_MAX, NextImageAcquiredSemaphore, VK_NULL_HANDLE, &SwapchainImageIndex));
	//!< vkAcquireNextImageKHR が VK_SUBOPTIMAL_KHR を返した場合、イメージは使用可能ではあるがプレゼンテーションエンジンにとってベストではない状態
	//!< vkAcquireNextImageKHR が VK_ERROR_OUT_OF_DATE_KHR を返した場合、イメージは使用不可で再作成が必要

	//!< デスクリプタセットを更新したら、コマンドバッファを記録し直さないとダメ？
	//UpdateDescriptorSet();
	
	//!< コマンドは指定のパイプラインステージに到達するまで実行され、そこでセマフォがシグナルされるまで待つ
	const std::vector<VkSemaphore> SemaphoresToWait = { NextImageAcquiredSemaphore };
	const std::vector<VkPipelineStageFlags> PipelineStagesToWait = { VK_PIPELINE_STAGE_TRANSFER_BIT };
	assert(SemaphoresToWait.size() == PipelineStagesToWait.size() && "Must be same size()");
	//!< 実行するコマンドバッファ
	const std::vector<VkCommandBuffer> CBs = { CommandPools[0].second[SwapchainImageIndex] };
	//!< 完了時にシグナルされるセマフォ(RenderFinishedSemaphore)
	const std::vector<VkSemaphore> SemaphoresToSignal = { RenderFinishedSemaphore };
	const std::vector<VkSubmitInfo> SIs = {
		{
			VK_STRUCTURE_TYPE_SUBMIT_INFO,
			nullptr,
			static_cast<uint32_t>(SemaphoresToWait.size()), SemaphoresToWait.data(), PipelineStagesToWait.data(), //!< 次イメージが取得できる(プレゼント完了)までウエイト
			static_cast<uint32_t>(CBs.size()), CBs.data(),
			static_cast<uint32_t>(SemaphoresToSignal.size()), SemaphoresToSignal.data() //!< 描画完了を通知する
		},
	};
	VERIFY_SUCCEEDED(vkQueueSubmit(GraphicsQueue, static_cast<uint32_t>(SIs.size()), SIs.data(), Fence));

	Present();
}
void VK::Dispatch()
{
	//!< (Fenceを指定して)サブミットしたコマンドが完了するまでブロッキングして待つ
	std::vector<VkFence> Fences = { ComputeFence };
	VERIFY_SUCCEEDED(vkWaitForFences(Device, static_cast<uint32_t>(Fences.size()), Fences.data(), VK_TRUE, (std::numeric_limits<uint64_t>::max)()));
	vkResetFences(Device, static_cast<uint32_t>(Fences.size()), Fences.data());

	const auto& CB = CommandPools[0].second[0];
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
	//!< 同時に複数のプレゼントが可能だが、1つのスワップチェインからは1つのみ
	const std::vector<VkSwapchainKHR> Swapchains = { Swapchain };
	const std::vector<uint32_t> ImageIndices = { SwapchainImageIndex };
	assert(Swapchains.size() == ImageIndices.size() && "Must be same");

	//!< サブミット時に指定したセマフォ(RenderFinishedSemaphore)を待ってからプレゼントが行なわれる
	const VkPresentInfoKHR PresentInfo = {
		VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
		nullptr,
		1, &RenderFinishedSemaphore,
		static_cast<uint32_t>(Swapchains.size()), Swapchains.data(), ImageIndices.data(),
		nullptr
	};
	VERIFY_SUCCEEDED(vkQueuePresentKHR(PresentQueue, &PresentInfo));
}
