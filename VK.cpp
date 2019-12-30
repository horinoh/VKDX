//#include "stdafx.h"
//#include "framework.h"

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

#ifdef USE_DEBUG_REPORT
	//!< インスタンスレベル関数(Debug) Instance level functions(Debug)
#define VK_INSTANCE_PROC_ADDR(proc) PFN_vk ## proc ## EXT VK::vk ## proc = VK_NULL_HANDLE;
#include "VKDebugReport.h"
#undef VK_INSTANCE_PROC_ADDR
#endif

	//!< デバイスレベル関数(Debug) Device level functions(Debug)
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

	//!< インスタンス、デバイス
	CreateInstance();
	CreateSurface(hWnd, hInstance);
	EnumeratePhysicalDevice(Instance);
	CreateDevice(GetCurrentPhysicalDevice(), Surface);
	
	//!< デバイスメモリをまとめて確保
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
	//!< パイプラインレイアウト (ルートシグネチャ相当)
	CreatePipelineLayout();
	//!< レンダーパス
	CreateRenderPass();
	CreateShaderModule();
	//!< パイプライン
	CreatePipeline();
	//!< フレームバッファ
	CreateFramebuffer();

	//!< ユニフォームバッファ (コンスタントバッファ相当)
	CreateUniformBuffer();

	//!< デスクリプタアップデートテンプレート
	CreateDescriptorUpdateTemplate();
	//!< デスクリプタプール (デスクリプタヒープ相当)
	CreateDescriptorPool();
	//!< デスクリプタセット (デスクリプタビュー相当)
	AllocateDescriptorSet();
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
#ifdef _DEBUG
	PerformanceCounter PC(__func__);
#endif

	Super::OnExitSizeMove(hWnd, hInstance);

	//!< デバイスがアイドルになるまで待つ
	if (VK_NULL_HANDLE != Device) {
		VERIFY_SUCCEEDED(vkDeviceWaitIdle(Device));
	}

#if 0
	//!< スワップチェインの作り直し
	CreateSwapchain(GetCurrentPhysicalDevice(), Surface, Rect);

	//!< デプスバッファの破棄、作成

	//!< フレームバッファの破棄、作成
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

	//!< ビューポートサイズが決定してから
	LoadScene();
	for (auto i = 0; i < CommandBuffers.size(); ++i) {
		PopulateCommandBuffer(i);
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
	//!< VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT の場合のみ個別に開放できる (Only if VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT is used, can be release individually)
	if (!DescriptorSets.empty()) {
		vkFreeDescriptorSets(Device, DescriptorPool, static_cast<uint32_t>(DescriptorSets.size()), DescriptorSets.data());
	}
#else
	//!< このプールから確保された全てのデスクリプタセットを解放する (ここでは次のステップでプール自体を破棄しているのでやらなくても良い)
	//!< (デスクリプタセットを個々に解放するのが面倒な場合、プール自体は破棄したくない場合)
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

	//!< SwapchainImages は取得したもの、破棄しない
	
	if (VK_NULL_HANDLE != Swapchain) {
		vkDestroySwapchainKHR(Device, Swapchain, GetAllocationCallbacks());
		Swapchain = VK_NULL_HANDLE;
	}

	//!< コマンドプール破棄時にコマンドバッファは暗黙的に解放されるので無くても良い
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

//!< @brief メモリプロパティフラグを(サポートされているかチェックしつつ)メモリタイプへ変換
//!< @param 物理デバイスのメモリプロパティ
//!< @param バッファやイメージの要求するメモリタイプ
//!< @param 希望のメモリプロパティフラグ
//!< @return メモリタイプ
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

void VK::CreateImage(VkImage* Img, const VkImageCreateFlags /*CreateFlags*/, const VkImageType ImageType, const VkFormat Format, const VkExtent3D& Extent3D, const uint32_t MipLevels, const uint32_t ArrayLayers, const VkSampleCountFlagBits SampleCount, const VkImageUsageFlags Usage) const
{
	//!< Usage に VK_IMAGE_USAGE_SAMPLED_BIT が指定されいる場合、フォーマットやフィルタが使用可能かチェック #VK_TODO ここではリニアフィルタ決め打ち
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
		VK_IMAGE_TILING_OPTIMAL, //!< リニアはパフォーマンスが悪いので、ここでは OPTIMAL に決め打ちしている
		Usage,
		VK_SHARING_MODE_EXCLUSIVE,
		0, nullptr,
		VK_IMAGE_LAYOUT_UNDEFINED //!< 作成時に指定できるのは UNDEFINED, PREINITIALIZED のみ、実際に使用する前にレイアウトを変更する必要がある
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

			//!< メモリコンテンツが変更されたことをドライバへ知らせる(vkMapMemory()した状態でやること)
			//!< デバスメモリ確保時に VK_MEMORY_PROPERTY_HOST_COHERENT_BIT を指定した場合は必要ない CreateDeviceMemory(..., VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
			if (nullptr != Range) {
				//!< 更新するレンジ(一部)が明示的に指定された場合、指定レンジのみを更新する
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
				//!< レンジ指定が無い場合は全体を更新する
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
//!< @param コマンドバッファ
//!< @param コピー元バッファ
//!< @param コピー先バッファ
//!< @param (コピー後の)バッファのアクセスフラグ ex) VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT, VK_ACCESS_INDEX_READ_BIT, VK_ACCESS_INDIRECT_READ_BIT,...等
//!< @param (コピー後に)バッファが使われるパイプラインステージ ex) VK_PIPELINE_STAGE_VERTEX_INPUT_BIT, VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT,...等
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
//!< Cubemapを作る場合、まずレイヤ化されたイメージを作成し、(イメージビューを用いて)レイヤをフェイスとして扱うようハードウエアに伝える
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
	
	//!< #VK_TODO 有効にしているものが、InstanceLayerProperties に含まれているかチェックする

	const VkInstanceCreateInfo InstanceCreateInfo = {
		VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
		nullptr,
		0,
		&ApplicationInfo,
		static_cast<uint32_t>(InstanceLayers.size()), InstanceLayers.data(),
		static_cast<uint32_t>(InstanceExtensions.size()), InstanceExtensions.data()
	};
	VERIFY_SUCCEEDED(vkCreateInstance(&InstanceCreateInfo, GetAllocationCallbacks(), &Instance));

	//!< インスタンスレベルの関数をロードする Load instance level functions
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
	VERIFY_SUCCEEDED(vkEnumeratePhysicalDevices(Inst, &Count, PhysicalDevices.data()));

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

void VK::EnumerateQueueFamilyProperties(VkPhysicalDevice PD, VkSurfaceKHR Sfc, std::vector<VkQueueFamilyProperties>& QFPs)
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
		VERIFY_SUCCEEDED(vkGetPhysicalDeviceSurfaceSupportKHR(PD, i, Sfc, &Supported));
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
		VERIFY_SUCCEEDED(vkGetPhysicalDeviceSurfaceSupportKHR(PD, i, Sfc, &Supported));
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
	//!< VkPhysicalDeviceFeatures には可能なフィーチャーが全て true になったものが渡されてくる
	//!< 不要な項目を false にオーバーライドするとパフォーマンスの改善が期待できるかもしれない
	//!< false で渡されてくる項目は使えないフィーチャーなので true に変えても使えない

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

	//!< vkGetPhysicalDeviceFeatures() で可能なフィーチャーが全て有効になった VkPhysicalDeviceFeatures が返る
	//!< このままでは可能なだけ有効になってしまうのでパフォーマンス的には良くない(必要な項目だけ true にし、それ以外は false にするのが本来は良い)
	//!< デバイスフィーチャーを「有効にしないと」と使用できない機能が多々あり面倒なので、ここでは返った値をそのまま使ってしまっている (パフォーマンス的には良くない)
	VkPhysicalDeviceFeatures PDF;
	vkGetPhysicalDeviceFeatures(PD, &PDF);
	//!< 必要に応じて OverridePhysicalDeviceFeatures() をオーバーライドし、不要な項目を無効にする(パフォーマンスを考える場合)
	OverridePhysicalDeviceFeatures(PDF);
	const VkDeviceCreateInfo DeviceCreateInfo = {
		VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
		nullptr,
		0,
		static_cast<uint32_t>(QueueCreateInfos.size()), QueueCreateInfos.data(),
		0, nullptr, //!< デバイスでレイヤーの有効化は非推奨 (Device layer is deprecated)
		static_cast<uint32_t>(DeviceExtensions.size()), DeviceExtensions.data(),
		&PDF
	};
	VERIFY_SUCCEEDED(vkCreateDevice(PD, &DeviceCreateInfo, GetAllocationCallbacks(), &Device));

	//!< デバイスレベルの関数をロードする Load device level functions
#ifdef VK_NO_PROTOYYPES
#define VK_DEVICE_PROC_ADDR(proc) vk ## proc = reinterpret_cast<PFN_vk ## proc>(vkGetDeviceProcAddr(Device, "vk" #proc)); assert(nullptr != vk ## proc && #proc && #proc);
#include "VKDeviceProcAddr.h"
#undef VK_DEVICE_PROC_ADDR
#endif //!< VK_NO_PROTOYYPES

	//!< キューの取得 (グラフィック、プレゼントキューは同じインデックスの場合もあるが別の変数に取得しておく) (Graphics and presentation index may be same, but save to individual variables)
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

//!< 実装の要求するアラインを保証して確保する(イメージに128byte、バッファに64byteのアラインが必要な環境なら、128byteアラインで確保される)
//!< 同じメモリオブジェクトから、(アラインの)異なるタイプ再確保が可能
void VK::AllocateDeviceMemory()
{
	VkPhysicalDeviceMemoryProperties PDMP;
	vkGetPhysicalDeviceMemoryProperties(GetCurrentPhysicalDevice(), &PDMP);
	if (PDMP.memoryHeapCount) {
		DeviceMemories.assign(PDMP.memoryHeapCount, VK_NULL_HANDLE);
		DeviceMemoryOffsets.assign(PDMP.memoryHeapCount, 0);

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

//!< ホストとデバイスの同期 (Synchronization between host and device)
//!< サブミット(vkQueueSubmit) に使用し、Draw()やDispatch()の頭でシグナル(サブミットされたコマンドの完了)を待つ (Used when submit, and wait signal on top of Draw())
//!< 初回と２回目以降を同じに扱う為に、シグナル済み状態(VK_FENCE_CREATE_SIGNALED_BIT)で作成している (Create with signaled state, to do same operation on first time and second time)
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

//!< キュー内での同期(異なるキュー間の同期も可能) (Synchronization internal queue)
//!< イメージ取得(vkAcquireNextImageKHR)、サブミット(VkSubmitInfo)、プレゼンテーション(VkPresentInfoKHR)に使用する (Use when image acquire, submit, presentation) 
void VK::CreateSemaphore(VkDevice Dev)
{
	const VkSemaphoreCreateInfo SemaphoreCreateInfo = {
		VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
		nullptr,
		0
	};

	//!< プレゼント完了同期用 (Wait for presentation finish)
	VERIFY_SUCCEEDED(vkCreateSemaphore(Dev, &SemaphoreCreateInfo, GetAllocationCallbacks(), &NextImageAcquiredSemaphore));

	//!< 描画完了同期用 (Wait for render finish)
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
//!< VK_COMMAND_BUFFER_LEVEL_PRIMARY	: 直接キューにサブミットできる、セカンダリをコールできる (Can be submit, can execute secondary)
//!< VK_COMMAND_BUFFER_LEVEL_SECONDARY	: サブミットできない、プライマリから実行されるのみ (Cannot submit, only executed from primary)
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

//!< キューファミリが異なる場合は別のコマンドプールを用意する必要がある、そのキューにのみサブミットできる
//!< 複数スレッドで同時にレコーディングするには、別のコマンドプールからアロケートされたコマンドバッファである必要がある (コマンドプールは複数スレッドからアクセス不可)
//!< VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT	: コマンドバッファ毎にリセットが可能、指定しない場合はプール毎にまとめてリセット (コマンドバッファのレコーディング開始時に暗黙的にリセットされるので注意)
//!< VK_COMMAND_POOL_CREATE_TRANSIENT_BIT				: 短命で、何度もサブミットしない、すぐにリセットやリリースされる場合に指定
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
		//!< キューファミリが異なる場合は、別のコマンドプールを用意する必要がある
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

	//!< セカンダリ用に別プールにする必要は無いが、ここでは別プールとしておく
#ifdef USE_SECONDARY_COMMAND_BUFFER
	SecondaryCommandPools.resize(1);
	VERIFY_SUCCEEDED(vkCreateCommandPool(Device, &CPCI, GetAllocationCallbacks(), &SecondaryCommandPools[0]));
#endif
}

//!< VK_COMMAND_BUFFER_LEVEL_PRIMARY	: 直接キューにサブミットできる、セカンダリをコールできる (Can be submit, can execute secondary)
//!< VK_COMMAND_BUFFER_LEVEL_SECONDARY	: サブミットできない、プライマリから実行されるのみ (Cannot submit, only executed from primary)
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

	//!< ここでは最初に見つかった UNDEFINED でないものを選択している (Select first format but UNDEFINED here)
	const auto SelectedIndex = [&]() {
		//!< 要素が 1 つのみで UNDEFINED の場合、制限は無く好きなものを選択できる (If there is only 1 element and which is UNDEFINED, we can choose any)
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
VkPresentModeKHR VK::SelectSurfacePresentMode(VkPhysicalDevice PD, VkSurfaceKHR Sfc)
{
	uint32_t Count;
	VERIFY_SUCCEEDED(vkGetPhysicalDeviceSurfacePresentModesKHR(PD, Sfc, &Count, nullptr));
	assert(Count && "Present mode count is zero");
	std::vector<VkPresentModeKHR> PMs(Count);
	VERIFY_SUCCEEDED(vkGetPhysicalDeviceSurfacePresentModesKHR(PD, Sfc, &Count, PMs.data()));

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

	//!< 最低よりも1枚多く取りたい、ただし最大値でクランプする(maxImageCount が0の場合は上限無し)
	const auto ImageCount = (std::min)(SurfaceCap.minImageCount + 1, 0 == SurfaceCap.maxImageCount ? UINT32_MAX : SurfaceCap.maxImageCount);
	Logf("\t\t\tImagCount = %d\n", ImageCount);

	//!< サーフェスのフォーマットを選択
	const auto SurfaceFormat = SelectSurfaceFormat(PD, Sfc);
	ColorFormat = SurfaceFormat.format; //!< カラーファーマットは覚えておく

	//!< サーフェスのサイズを選択
	SurfaceExtent2D = SelectSurfaceExtent(SurfaceCap, Width, Height);
	Logf("\t\t\tSurfaceExtent = %d x %d\n", SurfaceExtent2D.width, SurfaceExtent2D.height);

	//!< レイヤー、ステレオレンダリング等をしたい場合は1以上になるが、ここでは1
	uint32_t ImageArrayLayers = 1;

#ifdef USE_RENDER_PASS_CLEAR
	const auto ImageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
#else
	//!< 自前でクリアする場合は VK_IMAGE_USAGE_TRANSFER_DST_BIT フラグが必要
	const auto ImageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | (VK_IMAGE_USAGE_TRANSFER_DST_BIT & SurfaceCap.supportedUsageFlags);
#endif

	//!< グラフィックとプレゼントのキューファミリが異なる場合はキューファミリインデックスの配列が必要、また VK_SHARING_MODE_CONCURRENT を指定すること
	//!< (ただし VK_SHARING_MODE_CONCURRENT にするとパフォーマンスが落ちる場合がある)
	std::vector<uint32_t> QueueFamilyIndices;
	if (GraphicsQueueFamilyIndex != PresentQueueFamilyIndex) {
		QueueFamilyIndices.push_back(GraphicsQueueFamilyIndex);
		QueueFamilyIndices.push_back(PresentQueueFamilyIndex);
	}

	//!< サーフェスを回転、反転等させるかどうか (Rotate, mirror surface or not)
	const auto SurfaceTransform = (VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR & SurfaceCap.supportedTransforms) ? VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR : SurfaceCap.currentTransform;

	//!< サーフェスのプレゼントモードを選択
	const auto SurfacePresentMode = SelectSurfacePresentMode(PD, Sfc);

	//!< 既存のは後で開放するので OldSwapchain に覚えておく (セッティングを変更してスワップチェインを再作成する場合等に備える)
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
	VERIFY_SUCCEEDED(vkGetSwapchainImagesKHR(Dev, SC, &Count, SwapchainImages.data()));

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
	//!< vkCmdClearDepthStencilImage を用いてクリアする場合には VK_IMAGE_USAGE_TRANSFER_DST_BIT を指定すること (ここではレンダーパスでクリアする想定で指定しない)
	CreateImage(&DepthStencilImage, 0, VK_IMAGE_TYPE_2D, DepthFormat, Extent3D, 1, 1, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT /*| VK_IMAGE_USAGE_TRANSFER_DST_BIT*/);

	AllocateImageMemory(&DepthStencilDeviceMemory, DepthStencilImage, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	VERIFY_SUCCEEDED(vkBindImageMemory(Device, DepthStencilImage, DepthStencilDeviceMemory, 0));
	
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
			ImageSubresourceRange_DepthStencil/*ImageSubresourceRange_Depth : デプスのみのフォーマットの場合*/
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
			//!< VKではデフォルトで「Yが下」を向くが、高さに負の値を指定すると「Yが上」を向きDXと同様になる (In VK, by specifying negative height, Y become up. same as DX)
			//!< 通常基点は「左上」を指定するが、高さに負の値を指定する場合は「左下」を指定すること (When negative height, specify left bottom as base, otherwise left up)
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
	//!< ホストビジブルバッファ(HVB)を作成 (Create host visible buffer(HVB))
	CreateBuffer(&StagingBuffer, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, Size);

#ifdef USE_SUBALLOC
	//!< デバイスローカルメモリ(DLM)をサブアロケート (Suballocate device local memory(DLM))
	uint32_t StagingHeapIndex;
	VkDeviceSize StagingOffset;
	SuballocateBufferMemory(StagingHeapIndex, StagingOffset, StagingBuffer, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
	CopyToHostVisibleDeviceMemory(DeviceMemories[StagingHeapIndex], Size, Source, 0);
	const auto StagingSize = DeviceMemoryOffsets[StagingHeapIndex] - StagingOffset;
#else
	VkDeviceMemory StagingDeviceMemory = VK_NULL_HANDLE;

	//!< デバイスローカルメモリ(DLM)をアロケート (Allocate device local memory(DLM))
	AllocateBufferMemory(&StagingDeviceMemory, StagingBuffer, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
	VERIFY_SUCCEEDED(vkBindBufferMemory(Device, StagingBuffer, StagingDeviceMemory, 0));
	CopyToHostVisibleDeviceMemory(StagingDeviceMemory, Size, Source, 0);
#endif

	//!< HVBからDLBへのコピーコマンドを発行 (Submit HVB to DLB copy command)
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
	//!< サブアロケートしたオフセット分戻す (Revert suballocated memory offset)
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

//!< 適切なオフセットに配置する(ダイナミックではオフセット指定が変わる)
//!< デスクリプタタイプは VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC
//!< シェーダから読み書き可能
//!< アトミックな操作が可能
//!< シェーダ例 layout (set=0, binding=0) buffer MyBuffer { vec4 MyVec4; mat4 MyMat4; }
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

//!< イメージよりも大きなデータへアクセス可能(1Dイメージは最低限4096テクセルだが、ユニフォームテクセルバッファは最低限65536テクセル)
//!< デスクリプタタイプは VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER
//!< (TexelBuffer系は)ビューが必要
//!< シェーダ例 layout (set=0, binding=0) uniform samplerBuffer MySamplerBuffer;
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

	//!< サポートされているかのチェック
	VkFormatProperties FP;
	vkGetPhysicalDeviceFormatProperties(GetCurrentPhysicalDevice(), Format, &FP);
	assert((FP.bufferFeatures & VK_FORMAT_FEATURE_UNIFORM_TEXEL_BUFFER_BIT) && "");
#endif
}

//!< シェーダから書き込み可能
//!< デスクリプタタイプは VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER
//!< (TexelBuffer系は)ビューが必要
//!< アトミックな操作が可能
//!< シェーダ例 layout (set=0, binding=0, r32f) uniform imageBuffer MyImageBuffer;
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

	//!< サポートされているかのチェック
	VkFormatProperties FP;
	vkGetPhysicalDeviceFormatProperties(GetCurrentPhysicalDevice(), Format, &FP);
	assert((FP.bufferFeatures & VK_FORMAT_FEATURE_STORAGE_TEXEL_BUFFER_BIT) && "");
	//!< #VK_TODO アトミック操作をする場合
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

	//!< プッシュコンスタントレンジ : デスクリプタセットよりも高速
	//!< パイプラインレイアウト全体で128byte (ハードによりこれ以上使える場合もある GTX970M : 256byte)
	//!< 各シェーダステージは1つのプッシュコンスタントレンジにしかアクセスできない
	//!< 各シェーダステージが「共通のレンジを持たない」ような「ワーストケース」では 128/5==25.6、1シェーダステージで25byte程度となる
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

//!< デスクリプタセットを個々に解放したい場合には VkDescriptorPoolCreateInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT を指定する、
//!< その場合断片化は自分で管理しなくてはならない (指定しない場合はプール毎にまとめて解放しかできない)
//!< 1つのブールに対して、複数スレッドで同時にデスクリプタセットを確保することはできない (スレッド毎に別プールにすること)
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

//!< シェーダリソースを1つのコンテナオブジェクトにまとめる (型や数はセットレイアウトで定義され、ストレージはプールから確保される)
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
	//!< dstArrayElement ... バインディング内での配列の開始添字 (VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK_EXT指定の場合は開始バイトオフセット)
	//!< descriptorCount ... 更新するデスクリプタセット個数 (VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK_EXT指定の場合は更新するバイト)
	//!< 指定 descriptorType に従って、pImageInfo, pBufferInfo, pTexelBufferView の適切な箇所へ指定すること
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
	//!< アタッチメント ... レンダーパスでの描画先
	const std::array<VkAttachmentDescription, 1> ADs = {
		{
			0,
			Color,
			VK_SAMPLE_COUNT_1_BIT,
			//!< アタッチメントのロードストア
#ifdef USE_RENDER_PASS_CLEAR
			//!<「開始時にクリア」「終了時に保存」
			VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE,
#else
			//!<「開始時に何もしない」「終了時に保存」
			VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_STORE,
#endif
			//!< ステンシルのロードストア : (ここでは)開始時、終了時ともに「使用しない」
			VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
			//!< レンダーパスのレイアウト : 「開始時未定義」「終了時プレゼンテーションソース」
			VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
		}
	};

	//!< layout (input_attachment_index=0, set=0, binding=0) uniform SubpassInput XXX;
	//!< インプットアタッチメント : 読み取り用
	const std::array<VkAttachmentReference, 0> Input_Pass0 = {};
	//!< カラーアタッチメント : 書き込み用
	const std::array<VkAttachmentReference, 1> Color_Pass0 = { { 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL }, };
	//!< リゾルブアタッチメント : マルチサンプル → シングルサンプル
	const std::array<VkAttachmentReference, 1> Resolve_Pass0 = { { VK_ATTACHMENT_UNUSED }, };
	//assert(ColorARs.size() == ResolveARs.size() && "Size must be same");
	//!< デプスアタッチメント : 深度用
	const VkAttachmentReference* Depth_Pass0 = nullptr;
	//!< プリザーブアタッチメント : サブパス全体において保持しなくてはならないコンテンツ(のインデックス)
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

	//!< サブパス
#if 0
	const std::array<VkSubpassDependency, 0> SubpassDepends = {};
#else
	const std::array<VkSubpassDependency, 2> SubpassDepends = { {
			//!< 必要無いが、あえて書くならこんな感じ (No need this code, but if dare to write like this)
			{
				VK_SUBPASS_EXTERNAL, 0,																	//!< サブパス外からサブパス0へ
				VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,	//!< パイプラインの最終ステージからカラー出力ステージへ
				VK_ACCESS_MEMORY_READ_BIT, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,						//!< 読み込みからカラー書き込みへ
				VK_DEPENDENCY_BY_REGION_BIT,															//!< 同じメモリ領域に対する書き込みが完了してから読み込み (指定しない場合は自前で書き込み完了を管理)
			},
			{
				0, VK_SUBPASS_EXTERNAL,																	//!< サブパス0からサブパス外へ
				VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,	//!< カラー出力ステージからパイプラインの最終ステージへ
				VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_ACCESS_MEMORY_READ_BIT,						//!< カラー書き込みから読み込みへ
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
		RP, //!< ここで指定するレンダーパスは互換性のあるものなら可
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

void VK::CreatePipeline(VkPipeline& PL, const VkPipelineLayout PLL, const VkRenderPass RP, 
	const VkShaderModule VS, const VkShaderModule FS, const VkShaderModule TES, const VkShaderModule TCS, const VkShaderModule GS, 
	const std::vector<VkVertexInputBindingDescription>& VIBDs, const std::vector<VkVertexInputAttributeDescription>& VIADs, const VkPrimitiveTopology PT, const uint32_t PatchControlPoints,
	VkPipelineCache PC)
{
	PERFORMANCE_COUNTER();

	//!< (バリデーションの為に)デバイスフィーチャーを取得
	VkPhysicalDeviceFeatures PDF;
	vkGetPhysicalDeviceFeatures(GetCurrentPhysicalDevice(), &PDF);

	//!< パイプライン作成時にシェーダ内の定数値を上書き指定できる
#if 0
	//!< シェーダには以下のような記述(扱えるのはスカラ値のみ)
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

	//!< シェーダ (Shader)
	std::vector<VkPipelineShaderStageCreateInfo> PSSCI;
	//!< シェーダ内のコンスタント変数をパイプライン作成時に変更したい場合に使用する
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

	//!< バーテックスインプット (VertexInput)
	const VkPipelineVertexInputStateCreateInfo PVISCI = {
		VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
		nullptr,
		0,
		static_cast<uint32_t>(VIBDs.size()), VIBDs.data(),
		static_cast<uint32_t>(VIADs.size()), VIADs.data()
	};

	//!< DXでは「トポロジ」と「パッチコントロールポイント」の指定はIASetPrimitiveTopology()の引数としてコマンドリストへ指定する、VKとは結構異なるので注意
	//!< (「パッチコントロールポイント」の数も何を指定するかにより決まる)
	//!< CommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	//!< CommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_1_CONTROL_POINT_PATCHLIST);

	//!< インプットアセンブリ (InputAssembly)
	const VkPipelineInputAssemblyStateCreateInfo PIASCI = {
		VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
		nullptr,
		0,
		PT,
		VK_FALSE
	};
	//!< WITH_ADJACENCY 系使用時には デバイスフィーチャー geometryShader が有効であること
	assert((
		(PIASCI.topology != VK_PRIMITIVE_TOPOLOGY_LINE_LIST_WITH_ADJACENCY || PIASCI.topology != VK_PRIMITIVE_TOPOLOGY_LINE_STRIP_WITH_ADJACENCY || PIASCI.topology != VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST_WITH_ADJACENCY || PIASCI.topology != VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP_WITH_ADJACENCY)
		|| PDF.geometryShader) /*&& ""*/);
	//!< PATCH_LIST 使用時には デバイスフィーチャー tessellationShader が有効であること
	assert((PIASCI.topology != VK_PRIMITIVE_TOPOLOGY_PATCH_LIST || PDF.tessellationShader) && "");
	//!< インデックス 0xffffffff(VK_INDEX_TYPE_UINT32), 0xffff(VK_INDEX_TYPE_UINT16) をプリミティブのリスタートとする、インデックス系描画の場合(vkCmdDrawIndexed, vkCmdDrawIndexedIndirect)のみ有効
	//!< LIST 系使用時 primitiveRestartEnable 無効であること
	assert((
		(PIASCI.topology != VK_PRIMITIVE_TOPOLOGY_POINT_LIST || PIASCI.topology != VK_PRIMITIVE_TOPOLOGY_LINE_LIST || PIASCI.topology != VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST || PIASCI.topology != VK_PRIMITIVE_TOPOLOGY_LINE_LIST_WITH_ADJACENCY || PIASCI.topology != VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST_WITH_ADJACENCY || PIASCI.topology != VK_PRIMITIVE_TOPOLOGY_PATCH_LIST)
		|| PIASCI.primitiveRestartEnable == VK_FALSE) /*&& ""*/);

	//!< テセレーション (Tessellation)
	assert((PT != VK_PRIMITIVE_TOPOLOGY_PATCH_LIST || PatchControlPoints != 0) && "");
	const VkPipelineTessellationStateCreateInfo PTSCI = {
		VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO,
		nullptr,
		0,
		PatchControlPoints//!< パッチコントロールポイント
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
		VK_FALSE, 0.0f,
		&SM,
		VK_FALSE, VK_FALSE
	};
	assert((PMSCI.sampleShadingEnable == VK_FALSE || PDF.sampleRateShading) && "");
	assert((PMSCI.minSampleShading >= 0.0f && PMSCI.minSampleShading <= 1.0f) && "");
	assert((PMSCI.alphaToOneEnable == VK_FALSE || PDF.alphaToOne) && "");

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
	//!< デバイスフィーチャー independentBlend が有効で無い場合は、配列の各要素は「完全に同じ値」であること (If device feature independentBlend is not enabled, each array element must be exactly same)
	if (!PDF.independentBlend) {
		for (auto i : PCBASs) {
			assert(memcmp(&i, &PCBASs[0], sizeof(PCBASs[0])) == 0 && ""); //!< 最初の要素は比べる必要無いがまあいいや
		}
	}
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
	@brief 継承 ... 共通部分が多い場合、親パイプラインを指定して作成するとより高速に作成できる、親子間でのスイッチやバインドが有利
	(DX の D3D12_CACHED_PIPELINE_STATE 相当?)
	basePipelineHandle, basePipelineIndex は同時に使用できない、使用しない場合はそれぞれ VK_NULL_HANDLE, -1 を指定すること
	親には VK_PIPELINE_CREATE_ALLOW_DERIVATIVES_BIT フラグが必要、子には VK_PIPELINE_CREATE_DERIVATIVE_BIT フラグが必要
	・basePipelineHandle ... 既に親とするパイプライン(ハンドル)が存在する場合に指定
	・basePipelineIndex ... 同配列内で親パイプラインも同時に作成する場合、配列内での親パイプラインの添字(親の添字の方が若い値であること)
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
			RP, 0, //!< 指定したレンダーパス限定ではなく、互換性のある他のレンダーパスでも使用可能
			VK_NULL_HANDLE, -1 //!< basePipelineHandle, basePipelineIndex
		}
	};
	//!< VKでは1コールで複数のパイプラインを作成することもできるが、DXに合わせて1つしか作らないことにしておく
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
@brief クリア Clear
@note 「レンダーパス外」でクリア Out of renderpass ... vkCmdClearColorImage()
@note 「レンダーパス開始時」にクリア Begining of renderpass ... VkAttachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR, VkRenderPassBeginInfo.pClearValues
@note 「各々のサブパス」でクリア Each subpass ... vkCmdClearAttachments()
*/
//!< 「レンダーパス外」にてクリアを行う
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
		//!< vkCmdClearColorImage() はレンダーパス内では使用できない
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
	const auto CB = CommandBuffers[i];//CommandPools[0].second[i];
	const auto FB = Framebuffers[i];

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

#ifndef USE_RENDER_PASS_CLEAR
		ClearColor(CB, SwapchainImages[i], Colors::SkyBlue);
#endif

		const auto RP = RenderPasses[0];
#ifdef _DEBUG
		//!< レンダーエリアの最低粒度を確保
		VkExtent2D Granularity;
		vkGetRenderAreaGranularity(Device, RP, &Granularity);
		//!<「自分の環境では」 Granularity = { 1, 1 } だったのでほぼなんでも大丈夫みたい、環境によっては注意が必要
		assert(ScissorRects[0].extent.width >= Granularity.width && ScissorRects[0].extent.height >= Granularity.height && "ScissorRect is too small");
#endif
		//!< (ここでは)レンダーパス開始時にカラーはクリアせず、デプスはクリアしている (In this case, not clear color, but clear depth on begining of renderpas)
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
			ScissorRects[0], //!< フレームバッファのサイズ以下を指定できる
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

	//!< サブミットしたコマンドの完了を待つ
	const std::array<VkFence, 1> Fences = { Fence };
	VERIFY_SUCCEEDED(vkWaitForFences(Device, static_cast<uint32_t>(Fences.size()), Fences.data(), VK_TRUE, (std::numeric_limits<uint64_t>::max)()));
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
	//UpdateDescriptorSet(SwapchainImageIndex);
	
	//!< コマンドは指定のパイプラインステージに到達するまで実行され、そこでセマフォがシグナルされるまで待つ
	const std::vector<VkSemaphore> SemaphoresToWait = { NextImageAcquiredSemaphore };
	const std::vector<VkPipelineStageFlags> PipelineStagesToWait = { VK_PIPELINE_STAGE_TRANSFER_BIT };
	assert(SemaphoresToWait.size() == PipelineStagesToWait.size() && "Must be same size()");
	//!< 実行するコマンドバッファ
	const std::vector<VkCommandBuffer> CBs = { 
		CommandBuffers[SwapchainImageIndex],
	};
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
