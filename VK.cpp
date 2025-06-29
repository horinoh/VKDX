#include "VK.h"

template<> const glm::vec3 GetMin(const glm::vec3& lhs, const glm::vec3& rhs) { return glm::vec3((std::min)(lhs.x, rhs.x), (std::min)(lhs.y, rhs.y), (std::min)(lhs.z, rhs.z)); }
template<> const glm::vec3 GetMax(const glm::vec3& lhs, const glm::vec3& rhs) { return glm::vec3((std::max)(lhs.x, rhs.x), (std::max)(lhs.y, rhs.y), (std::max)(lhs.z, rhs.z)); }
template<>
void AdjustScale(std::vector<glm::vec3>& Vertices, const float Scale)
{
	auto Max = (std::numeric_limits<glm::vec3>::min)();
	auto Min = (std::numeric_limits<glm::vec3>::max)();
	for (const auto& i : Vertices) {
		Min = GetMin(Min, i);
		Max = GetMax(Max, i);
	}

	const auto Diff = Max - Min;
	const auto Bound = (std::max)((std::max)(Diff.x, Diff.y), Diff.z);
	const auto Coef = Scale / Bound;
	std::ranges::transform(Vertices, std::begin(Vertices), [&](const glm::vec3& rhs) { return glm::vec3(rhs.x, (rhs.y - Diff.y * 0.5f), rhs.z - Min.z) * Coef; });
}

#ifdef _WINDOWS
#pragma comment(lib, "vulkan-1.lib")
#else
// "libvulkan.so.1"
#endif

#define VK_PROC_ADDR(proc) PFN_vk ## proc VK::vk ## proc = VK_NULL_HANDLE;
#ifdef VK_NO_PROTOYYPES
#include "VKGlobalProcAddr.h"
#include "VKInstanceProcAddr.h"
#include "VKDeviceProcAddr.h"
#endif
#include "VKDeviceProcAddr_RayTracing.h"
#include "VKDeviceProcAddr_MeshShaderNV.h"
#include "VKDeviceProcAddr_MeshShader.h"
#undef VK_PROC_ADDR

#ifdef USE_DEBUG_UTILS
#define VK_PROC_ADDR(proc) PFN_vk ## proc ## EXT VK::vk ## proc = VK_NULL_HANDLE;
#include "VKInstanceProcAddr_DebugUtils.h"
#undef VK_PROC_ADDR
#endif

#ifdef _WINDOWS
void VK::OnCreate(HWND hWnd, HINSTANCE hInstance, LPCWSTR Title)
{
	PERFORMANCE_COUNTER_FUNC();

	Super::OnCreate(hWnd, hInstance, Title);

	LoadVulkanLibrary();

	//!< インスタンス、デバイス
	CreateInstance();
	SelectPhysicalDevice(Instance);
	CreateDevice(hWnd, hInstance);
	
	CreateFence(Device);

	CreateSwapchain();

	CreateSemaphore(Device);

	CreateCommandBuffer();

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

	//!< デスクリプタ
	CreateDescriptor();

	CreateShaderBindingTable();

	CreateVideo();

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
	constexpr UINT_PTR IDT_TIMER1 = 1000; //!< 何でも良い
	KillTimer(hWnd, IDT_TIMER1); {
		OnPreDestroy();

		PERFORMANCE_COUNTER_FUNC();

		Super::OnExitSizeMove(hWnd, hInstance);

#if 0
		//!< デバイスがアイドルになるまで待つ
		if (VK_NULL_HANDLE != Device) [[likely]] {
			VERIFY_SUCCEEDED(vkDeviceWaitIdle(Device));
			}

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

		CreateViewports();

#if 0
		//!< コマンドバッファのリセット vkBeginCommandBuffer() で暗黙的にリセットされるので不要？
		for (auto i : SecondaryCommandBuffers) { VERIFY_SUCCEEDED(vkResetCommandBuffer(i, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT)); }
		for (auto i : CommandBuffers) { VERIFY_SUCCEEDED(vkResetCommandBuffer(i, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT)); }
#endif

		//!< ビューポートサイズが決定してから
		LoadScene();
		for (auto i = 0; i < size(CommandBuffers); ++i) {
			PopulateSecondaryCommandBuffer(i);
			PopulateCommandBuffer(i);
		}
	} SetTimer(hWnd, IDT_TIMER1, DeltaMsec, nullptr);
}
void VK::OnPreDestroy()
{
	Super::OnPreDestroy();

	if (VK_NULL_HANDLE != Device) [[likely]] {
		//!< デバイスのキューにサブミットされた全コマンドが完了するまでブロッキング、主に終了処理に使う (Wait for all command submitted to queue, usually used on finalize)
		VERIFY_SUCCEEDED(vkDeviceWaitIdle(Device));
	}
}
void VK::OnDestroy(HWND hWnd, HINSTANCE hInstance)
{
	Super::OnDestroy(hWnd, hInstance);

	//!< VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT を指定した場合のみ個別に開放できる (Only if VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT is used, can be release individually)
	//if (!empty(DescriptorSets))  [[likely]] {
	//	vkFreeDescriptorSets(Device, DescriptorPool, static_cast<uint32_t>(size(DescriptorSets)), data(DescriptorSets));
	//}
	DescriptorSets.clear();
	//!< このプールから確保された全てのデスクリプタセットを解放する、ここでは次のステップでプール自体を破棄しているのでやらなくても良い
	for (auto i : DescriptorPools) {
		vkResetDescriptorPool(Device, i, 0);
	}
	for (auto i : DescriptorPools) {
		vkDestroyDescriptorPool(Device, i, GetAllocationCallbacks());
	}
	DescriptorPools.clear();

	for (auto i : Framebuffers) {
		vkDestroyFramebuffer(Device, i, GetAllocationCallbacks());
	}
	Framebuffers.clear();
	for (auto i : Pipelines) {
		vkDestroyPipeline(Device, i, GetAllocationCallbacks());
	}
	Pipelines.clear();
	for (auto i : RenderPasses) {
		vkDestroyRenderPass(Device, i, GetAllocationCallbacks());
	}
	RenderPasses.clear();

	for (auto i : PipelineLayouts) {
		vkDestroyPipelineLayout(Device, i, GetAllocationCallbacks());
	}
	PipelineLayouts.clear();
	for (auto i : DescriptorSetLayouts) {
		vkDestroyDescriptorSetLayout(Device, i, GetAllocationCallbacks());
	}
	DescriptorSetLayouts.clear();

	for (auto i : Samplers) {
		vkDestroySampler(Device, i, GetAllocationCallbacks());
	}
	Samplers.clear();

	for (auto& i : AnimatedTextures) { i.Destroy(Device); } AnimatedTextures.clear();
	for (auto& i : StorageTextures) { i.Destroy(Device); } StorageTextures.clear();
	for (auto& i : RenderTextures) { i.Destroy(Device); } RenderTextures.clear();
	for (auto& i : DepthTextures) { i.Destroy(Device); } DepthTextures.clear();
	for (auto& i : Textures) { i.Destroy(Device); } Textures.clear();

	for (auto i : UniformBuffers) { i.Destroy(Device); } UniformBuffers.clear();
	for (auto i : IndirectBuffers) { i.Destroy(Device); } IndirectBuffers.clear();
	for (auto i : IndexBuffers) { i.Destroy(Device); } IndexBuffers.clear();
	for (auto i : VertexBuffers) { i.Destroy(Device); } VertexBuffers.clear();

	for (auto i : ComputeCommandPools) {
		vkDestroyCommandPool(Device, i, GetAllocationCallbacks());
	}
	ComputeCommandPools.clear();

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
	
	for (auto& i : Swapchain.ImageAndViews) {
		vkDestroyImageView(Device, i.second, GetAllocationCallbacks()); 
	}
	//SwapchainBackBuffers.clear();
	//!< SwapchainImages は取得したもの、破棄しない
	if (VK_NULL_HANDLE != Swapchain.VkSwapchain) [[likely]] {
		vkDestroySwapchainKHR(Device, Swapchain.VkSwapchain, GetAllocationCallbacks());
		Swapchain.VkSwapchain = VK_NULL_HANDLE;
	}

	if (VK_NULL_HANDLE != ComputeSemaphore) [[likely]] {
		vkDestroySemaphore(Device, ComputeSemaphore, GetAllocationCallbacks());
	}
	for(auto i : RenderFinishedSemaphores) {
		vkDestroySemaphore(Device, i, GetAllocationCallbacks());
	}
	for (auto i : NextImageAcquiredSemaphores) {
		vkDestroySemaphore(Device, i, GetAllocationCallbacks());
	}
	
	if (VK_NULL_HANDLE != GraphicsFence) [[likely]] {
		vkDestroyFence(Device, GraphicsFence, GetAllocationCallbacks());
	}
	if (VK_NULL_HANDLE != ComputeFence) [[likely]] {
		vkDestroyFence(Device, ComputeFence, GetAllocationCallbacks());
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
#ifdef USE_DEBUG_UTILS
	if (VK_NULL_HANDLE != DebugUtilsMessenger) [[likely]] {
		vkDestroyDebugUtilsMessenger(Instance, DebugUtilsMessenger, GetAllocationCallbacks());
		DebugUtilsMessenger = VK_NULL_HANDLE;
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
			VERIFY(false);
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
	default: BREAKPOINT(); return "Not found";
#include "VkResult.h"
	}
#undef VK_RESULT_ENTRY
}

const char* VK::GetFormatChar(const VkFormat Format)
{
#define VK_FORMAT_ENTRY(vf) case VK_FORMAT_##vf: return #vf;
	switch (Format)
	{
	default: BREAKPOINT(); return "Not found";
#include "VKFormat.h"
	}
#undef VK_FORMAT_ENTRY
}

const char* VK::GetColorSpaceChar(const VkColorSpaceKHR ColorSpace)
{
#define VK_COLOR_SPACE_ENTRY(vcs) case VK_COLOR_SPACE_##vcs: return #vcs;
	switch (ColorSpace)
	{
	default: BREAKPOINT(); return "Not found";
#include "VKColorSpace.h"
	}
#undef VK_COLOR_SPACE_ENTRY
}

const char* VK::GetImageViewTypeChar(const VkImageViewType IVT)
{
#define VK_IMAGE_VIEW_TYPE_ENTRY(ivt) case VK_IMAGE_VIEW_TYPE_##ivt: return #ivt;
	switch (IVT)
	{
	default: BREAKPOINT(); return "Not found";
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
	default: BREAKPOINT(); return "Not found";
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
	default: BREAKPOINT(); return "Not found";
		VK_SYSTEM_ALLOCATION_SCOPE_ENTRY(COMMAND)
		VK_SYSTEM_ALLOCATION_SCOPE_ENTRY(OBJECT)
		VK_SYSTEM_ALLOCATION_SCOPE_ENTRY(CACHE)
		VK_SYSTEM_ALLOCATION_SCOPE_ENTRY(DEVICE)
		VK_SYSTEM_ALLOCATION_SCOPE_ENTRY(INSTANCE)
	}
#undef VK_SYSTEM_ALLOCATION_SCOPE_ENTRY
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
	BREAKPOINT();
	return 0xffff;
}

void VK::PopulateCopyBufferToImageCommand(const VkCommandBuffer CB, const VkBuffer Src, const VkImage Dst, const VkAccessFlags2 AF, const VkImageLayout IL, const VkPipelineStageFlags2 PSF, const std::vector<VkBufferImageCopy2>& BICs, const uint32_t Levels, const uint32_t Layers)
{
	VERIFY(!empty(BICs));
	const VkImageSubresourceRange ISR = {
		.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
		.baseMipLevel = 0, .levelCount = Levels,
		.baseArrayLayer = 0, .layerCount = Layers
	};
	ImageMemoryBarrier(CB, 
		VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_2_TRANSFER_BIT, 
		Dst,
		0, VK_ACCESS_2_TRANSFER_WRITE_BIT,
		VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		ISR);
	{
		const VkCopyBufferToImageInfo2 CBTII2 = {
			.sType = VK_STRUCTURE_TYPE_COPY_BUFFER_TO_IMAGE_INFO_2,
			.pNext = nullptr,
			.srcBuffer = Src, .dstImage = Dst,
			.dstImageLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			.regionCount = static_cast<uint32_t>(std::size(BICs)), .pRegions = std::data(BICs)
		};
		vkCmdCopyBufferToImage2(CB, &CBTII2);
	}
	ImageMemoryBarrier(CB,
		VK_PIPELINE_STAGE_2_TRANSFER_BIT, PSF,
		Dst,
		VK_ACCESS_2_TRANSFER_WRITE_BIT, AF,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, IL,
		ISR);
}

//void VK::PopulateCopyImageToBufferCommand(const VkCommandBuffer CB, const VkImage Src, const VkBuffer Dst, const VkAccessFlags AF, const VkImageLayout IL, const VkPipelineStageFlags PSF, const std::vector<VkBufferImageCopy>& BICs, const uint32_t Levels, const uint32_t Layers)
//{
//	//!< コマンド開始 (Begin command)
//	const VkCommandBufferBeginInfo CBBI = {
//		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
//		.pNext = nullptr,
//		.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
//		.pInheritanceInfo = nullptr
//	};
//	VERIFY_SUCCEEDED(vkBeginCommandBuffer(CB, &CBBI)); {
//		assert(!empty(BICs) && "BufferImageCopy is empty");
//		const VkImageSubresourceRange ISR = {
//			.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
//			.baseMipLevel = 0, .levelCount = Levels,
//			.baseArrayLayer = 0, .layerCount = Layers
//		};
//		//!< イメージメモリバリア (Image memory barrier)
//		{
//			const std::array IMBs = {
//				VkImageMemoryBarrier({
//					.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
//					.pNext = nullptr,
//					.srcAccessMask = 0, .dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT,
//					.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED, .newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
//					.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED, .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
//					.image = Src,
//					.subresourceRange = ISR})
//			};
//			vkCmdPipelineBarrier(CB,
//				VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
//				0,
//				0, nullptr,
//				0, nullptr,
//				static_cast<uint32_t>(size(IMBs)), data(IMBs));
//		}
//		{
//			//!< イメージバッファ間コピーコマンド (Image to buffer copy command)
//			vkCmdCopyImageToBuffer(CB, Src, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, Dst, static_cast<uint32_t>(size(BICs)), data(BICs));
//		}
//		//!< イメージメモリバリア (Image memory barrier)
//		{
//			const std::array IMBs = {
//				VkImageMemoryBarrier({
//					.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
//					.pNext = nullptr,
//					.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT, .dstAccessMask = AF,
//					.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, .newLayout = IL,
//					.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED, .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
//					.image = Src,
//					.subresourceRange = ISR})
//			};
//			vkCmdPipelineBarrier(CB,
//				VK_PIPELINE_STAGE_TRANSFER_BIT, PSF,
//				0,
//				0, nullptr,
//				0, nullptr,
//				static_cast<uint32_t>(size(IMBs)), data(IMBs));
//		}
//	} VERIFY_SUCCEEDED(vkEndCommandBuffer(CB));
//}

void VK::SubmitAndWait(const VkQueue Queue, const VkCommandBuffer CB)
{
	const std::array<VkSemaphoreSubmitInfo, 0> WaitSSIs = {};
	const std::array CBSIs = {
		VkCommandBufferSubmitInfo({
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO,
			.pNext = nullptr,
			.commandBuffer = CB,
			.deviceMask = 0
			})
	};
	const std::array<VkSemaphoreSubmitInfo, 0> SignalSSIs = {};
	const std::array SIs = {
		VkSubmitInfo2({
			.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2,
			.pNext = nullptr,
			.flags = 0,
			.waitSemaphoreInfoCount = static_cast<uint32_t>(std::size(WaitSSIs)), .pWaitSemaphoreInfos = std::data(WaitSSIs),
			.commandBufferInfoCount = static_cast<uint32_t>(std::size(CBSIs)), .pCommandBufferInfos = std::data(CBSIs),
			.signalSemaphoreInfoCount = static_cast<uint32_t>(std::size(SignalSSIs)), .pSignalSemaphoreInfos = std::data(SignalSSIs)
		})
	};
	VERIFY_SUCCEEDED(vkQueueSubmit2(Queue, static_cast<uint32_t>(std::size(SIs)), std::data(SIs), VK_NULL_HANDLE));

	//!< キューにサブミットされたコマンドが完了するまでブロッキング (フェンスを用いないブロッキング方法)
	VERIFY_SUCCEEDED(vkQueueWaitIdle(Queue));
}

void VK::EnumerateMemoryRequirements(const VkMemoryRequirements& MR, const VkPhysicalDeviceMemoryProperties& PDMP)
{
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

//!< Vulkanローダーが(引数で渡したデバイスに基いて)適切な実装へ関数コールをリダイレクトする必要がある、このリダイレクトには時間がかかりパフォーマンスに影響する
//!< 以下のようにすると、使用したいデバイスから直接関数をロードするため、リダイレクトをスキップできパフォーマンスを改善できる
void VK::LoadVulkanLibrary()
{
#ifdef VK_NO_PROTOYYPES
#ifdef _WINDOWS
	VulkanLibrary = LoadLibrary(TEXT("vulkan-1.dll")); VERIFY(nullptr != VulkanLibrary);
#else
	VulkanLibrary = dlopen("libvulkan.so.1", RTLD_NOW); VERIFY(nullptr != VulkanLibrary);
#endif
	
	//!< グローバルレベルの関数をロードする Load global level functions
#define VK_PROC_ADDR(proc) vk ## proc = reinterpret_cast<PFN_vk ## proc>(vkGetInstanceProcAddr(nullptr, "vk" #proc)); VERIFY(nullptr != vk ## proc && #proc);
#include "VKGlobalProcAddr.h"
#undef VK_PROC_ADDR
#endif
}

void VK::CreateInstance(const std::vector<const char*>& AdditionalLayers, const std::vector<const char*>& AdditionalExtensions)
{
	//!< インスタンスレベルのレイヤー、エクステンションの列挙
	{
#ifdef DEBUG_STDOUT
		Logf("Instance Layer Properties\n");
		uint32_t LC = 0;
		VERIFY_SUCCEEDED(vkEnumerateInstanceLayerProperties(&LC, nullptr));
		if (LC) [[likely]] {
			std::vector<VkLayerProperties> LPs(LC);
			VERIFY_SUCCEEDED(vkEnumerateInstanceLayerProperties(&LC, std::data(LPs)));
			for (const auto& i : LPs) {
				std::cout << i;

				uint32_t EC = 0;
				VERIFY_SUCCEEDED(vkEnumerateInstanceExtensionProperties(i.layerName, &EC, nullptr));
				if (EC) [[likely]] {
					std::vector<VkExtensionProperties> EPs(EC);
					VERIFY_SUCCEEDED(vkEnumerateInstanceExtensionProperties(i.layerName, &EC, std::data(EPs)));
					for (const auto& j : EPs) {
						std::cout << j;
					}
				}
			}
		}
#endif
	}

	//!< ここでは最新バージョンで動くようにしておく (Use latest version here)
#if true
	constexpr auto APIVersion = VK_HEADER_VERSION_COMPLETE;
#else
	uint32_t APIVersion; VERIFY_SUCCEEDED(vkEnumerateInstanceVersion(&APIVersion));
#endif
	Logf("API Version = %d.%d.(Header = %d)(Patch = %d)\n", VK_VERSION_MAJOR(APIVersion), VK_VERSION_MINOR(APIVersion), VK_HEADER_VERSION, VK_VERSION_PATCH(APIVersion));
	const auto ApplicationName = GetTitleString();
	const VkApplicationInfo AI = {
		.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
		.pNext = nullptr,
		.pApplicationName = data(ApplicationName), .applicationVersion = APIVersion,
		.pEngineName = "VKDX Engine Name", .engineVersion = APIVersion,
		.apiVersion = APIVersion
	};
	std::vector Layers = {
		"VK_LAYER_KHRONOS_validation",
		"VK_LAYER_LUNARG_monitor", //!< タイトルバーにFPSを表示 (Display FPS on titile bar)
	};
	std::ranges::copy(AdditionalLayers, std::back_inserter(Layers));

	std::vector Extensions = {
		VK_KHR_SURFACE_EXTENSION_NAME,
#ifdef VK_USE_PLATFORM_WIN32_KHR
		VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
#endif
#ifdef _DEBUG
		VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
		VK_EXT_VALIDATION_FEATURES_EXTENSION_NAME,
		//VK_EXT_DEBUG_REPORT_EXTENSION_NAME,
#endif
	};
	std::ranges::copy(AdditionalExtensions, std::back_inserter(Extensions));

	const VkInstanceCreateInfo ICI = {
		.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
		.pApplicationInfo = &AI,
		.enabledLayerCount = static_cast<uint32_t>(std::size(Layers)), .ppEnabledLayerNames = std::data(Layers),
		.enabledExtensionCount = static_cast<uint32_t>(std::size(Extensions)), .ppEnabledExtensionNames = std::data(Extensions)
	};
	VERIFY_SUCCEEDED(vkCreateInstance(&ICI, GetAllocationCallbacks(), &Instance));

#ifdef VK_NO_PROTOYYPES
#define VK_PROC_ADDR(proc) vk ## proc = reinterpret_cast<PFN_vk ## proc>(vkGetInstanceProcAddr(Instance, "vk" #proc)); VERIFY(nullptr != vk ## proc && #proc);
#include "VKInstanceProcAddr.h"
#undef VK_PROC_ADDR
#endif

#ifdef USE_DEBUG_UTILS
#define VK_PROC_ADDR(proc) vk ## proc = reinterpret_cast<PFN_vk ## proc ## EXT>(vkGetInstanceProcAddr(Instance, "vk" #proc "EXT")); VERIFY(nullptr != vk ## proc && #proc);
#include "VKInstanceProcAddr_DebugUtils.h"
#undef VK_PROC_ADDR
	constexpr VkDebugUtilsMessengerCreateInfoEXT DUMCI = {
		.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
		.pNext = nullptr,
		.flags = 0,
		.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
		.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT/*| VK_DEBUG_UTILS_MESSAGE_TYPE_DEVICE_ADDRESS_BINDING_BIT_EXT*/,
		.pfnUserCallback = MessengerCallback,
		.pUserData = nullptr
	};
	VERIFY_SUCCEEDED(vkCreateDebugUtilsMessenger(Instance, &DUMCI, GetAllocationCallbacks(), &DebugUtilsMessenger));
#endif

	LOG_OK();
}



void VK::SelectPhysicalDevice(VkInstance Inst)
{
	//!< 物理デバイス(GPU)の列挙
	uint32_t Count = 0;
	VERIFY_SUCCEEDED(vkEnumeratePhysicalDevices(Inst, &Count, nullptr));
	VERIFY(Count);
	PhysicalDevices.resize(Count);
	VERIFY_SUCCEEDED(vkEnumeratePhysicalDevices(Inst, &Count, std::data(PhysicalDevices)));

#ifdef DEBUG_STDOUT
	Log("\tPhysicalDevices\n");
	for (const auto& i : PhysicalDevices) {
#pragma region PROPERTY
		{
			VkPhysicalDeviceProperties PDP = {};
			vkGetPhysicalDeviceProperties(i, &PDP);
			std::cout << PDP;

			//!< プロパティ2 (Property2)
			{
				//!< 取得したい全てのプロパティを VkPhysicalDeviceProperties2.pNext へチェイン指定する
#pragma region MESH_SHADER
				{
					VkPhysicalDeviceMeshShaderPropertiesNV PDMSP = { .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_PROPERTIES_NV, .pNext = nullptr };
					VkPhysicalDeviceProperties2 PDP2 = { .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2, .pNext = &PDMSP, };
					vkGetPhysicalDeviceProperties2(i, &PDP2);
					std::cout << PDMSP;
				}
				{
					VkPhysicalDeviceMeshShaderPropertiesEXT PDMSP = { .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_PROPERTIES_EXT, .pNext = nullptr };
					VkPhysicalDeviceProperties2 PDP2 = { .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2, .pNext = &PDMSP, };
					vkGetPhysicalDeviceProperties2(i, &PDP2);
					std::cout << PDMSP;
				}
#pragma endregion
#pragma region RAYTRACING
				{
					VkPhysicalDeviceRayTracingPipelinePropertiesKHR PDRTPP = { .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_PROPERTIES_KHR, .pNext = nullptr };
					VkPhysicalDeviceProperties2 PDP2 = { .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2, .pNext = &PDRTPP, };
					vkGetPhysicalDeviceProperties2(i, &PDP2);
					std::cout << PDRTPP;
					//std::cout << PDP2.properties;
				}
#pragma endregion
			}
		}
#pragma endregion

#pragma region FEATURE
		{
			VkPhysicalDeviceFeatures PDF = {};
			vkGetPhysicalDeviceFeatures(i, &PDF);
			std::cout << PDF;

			//!< フィーチャー2 (Feature2)
			{
#pragma region MESH_SHADER
				{
					VkPhysicalDeviceMeshShaderFeaturesNV PDMSF = { .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_FEATURES_NV, .pNext = nullptr };
					VkPhysicalDeviceFeatures2 PDF2 = { .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2, .pNext = &PDMSF };
					vkGetPhysicalDeviceFeatures2(i, &PDF2);
					std::cout << PDMSF;
				}
				{
					VkPhysicalDeviceMeshShaderFeaturesEXT PDMSF = { .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_FEATURES_EXT, .pNext = nullptr };
					VkPhysicalDeviceFeatures2 PDF2 = { .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2, .pNext = &PDMSF };
					vkGetPhysicalDeviceFeatures2(i, &PDF2);
					std::cout << PDMSF;
				}
#pragma endregion
#pragma region RAYTRACING
				//!< VkPhysicalDeviceAccelerationStructureFeaturesKHR が上手くいかない環境がある… (サポートしない環境ではフィーチャーのチェックもできない?) #VK_TODO
				//{
				//	VkPhysicalDeviceBufferDeviceAddressFeatures PDBDAF = { .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES, .pNext = nullptr };
				//	VkPhysicalDeviceRayTracingPipelineFeaturesKHR PDRTPF = { .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR, .pNext = &PDBDAF };
				//	VkPhysicalDeviceAccelerationStructureFeaturesKHR PDASF = { .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR, .pNext = &PDRTPF };
				//	VkPhysicalDeviceFeatures2 PDF2 = { .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2, .pNext = &PDASF };
				//	vkGetPhysicalDeviceFeatures2(i, &PDF2);
				//	std::cout << PDBDAF;
				//	std::cout << PDRTPF;
				//	std::cout << PDASF;
				//}
				{
					VkPhysicalDeviceBufferDeviceAddressFeatures PDBDAF = { .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES, .pNext = nullptr };
					VkPhysicalDeviceRayTracingPipelineFeaturesKHR PDRTPF = { .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR, .pNext = &PDBDAF };
					VkPhysicalDeviceFeatures2 PDF2 = { .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2, .pNext = &PDRTPF };
					vkGetPhysicalDeviceFeatures2(i, &PDF2);
					std::cout << PDBDAF;
					std::cout << PDRTPF;
					if (PDRTPF.rayTracingPipeline) {
						VkPhysicalDeviceAccelerationStructureFeaturesKHR PDASF = { .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR, .pNext = nullptr };
						VkPhysicalDeviceFeatures2 PDF2_1 = { .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2, .pNext = &PDASF };
						vkGetPhysicalDeviceFeatures2(i, &PDF2_1);
						std::cout << PDASF;
					}
					//std::cout << PDF2.features;
				}
#pragma endregion			
				{
					VkPhysicalDeviceSwapchainMaintenance1FeaturesEXT PDSMF = { .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SWAPCHAIN_MAINTENANCE_1_FEATURES_EXT, .pNext = nullptr };
					VkPhysicalDeviceFeatures2 PDF2 = { .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2, .pNext = &PDSMF };
					vkGetPhysicalDeviceFeatures2(i, &PDF2);
					std::cout << PDSMF;
				}
			}
		}
#pragma endregion

#pragma region MEMORY_PROP
		//!< メモリプロパティ (MemoryProperty)
		VkPhysicalDeviceMemoryProperties PDMP;
		vkGetPhysicalDeviceMemoryProperties(i, &PDMP);
		std::cout << PDMP;
#pragma endregion

		//!< フィジカルデバイスのレイヤー、エクステンションの列挙 (Enumrate physical device's layers and extentions)
		std::cout << i;

		Log("\n\n");
	}
#endif

	//!< 物理デバイスの選択、ここでは最大メモリを選択することにする (Select physical device, here select max memory size)
	SelectedPhysDevice.first = *std::ranges::max_element(PhysicalDevices, [](const auto& lhs, const auto& rhs) {
		VkPhysicalDeviceMemoryProperties MemPropL, MemPropR;
		vkGetPhysicalDeviceMemoryProperties(lhs, &MemPropL);
		vkGetPhysicalDeviceMemoryProperties(rhs, &MemPropR);
		const auto MemTotalL = std::accumulate(std::data(MemPropL.memoryHeaps), &MemPropL.memoryHeaps[MemPropL.memoryHeapCount], static_cast<VkDeviceSize>(0), [](auto Sum, const auto& rhs) { return Sum + rhs.size; });
		const auto MemTotalR = std::accumulate(std::data(MemPropR.memoryHeaps), &MemPropR.memoryHeaps[MemPropR.memoryHeapCount], static_cast<VkDeviceSize>(0), [](auto Sum, const auto& rhs) { return Sum + rhs.size; });
		return MemTotalL < MemTotalR;
		});
	vkGetPhysicalDeviceProperties(SelectedPhysDevice.first, &SelectedPhysDevice.second.PDP);
	vkGetPhysicalDeviceMemoryProperties(SelectedPhysDevice.first, &SelectedPhysDevice.second.PDMP);
#ifdef DEBUG_STDOUT
	Logf("Selected GPU = %s\n", SelectedPhysDevice.second.PDP.deviceName);
#endif
}

void VK::CreateDevice(HWND hWnd, HINSTANCE hInstance, void* pNext, const std::vector<const char*>& ExtensionNames)
{
	//!< サーフェス作成
#pragma region SURFACE
	const VkWin32SurfaceCreateInfoKHR SCI = {
		.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
		.pNext = nullptr,
		.flags = 0,
		.hinstance = hInstance,
		.hwnd = hWnd
	};
	VERIFY_SUCCEEDED(vkCreateWin32SurfaceKHR(Instance, &SCI, GetAllocationCallbacks(), &Surface));
#pragma endregion

	const auto PD = SelectedPhysDevice.first;

#pragma region QUEUE_FAMILY
	std::vector<VkQueueFamilyProperties> QFPs;
	//!< キューファミリプロパティの列挙
	uint32_t Count = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(PD, &Count, nullptr);
	VERIFY(Count);
	QFPs.resize(Count);
	vkGetPhysicalDeviceQueueFamilyProperties(PD, &Count, std::data(QFPs));
	Log("\tQueueFamilyProperties\n");
	for (uint32_t i = 0; i < std::size(QFPs); ++i) {
		Logf("\t\t[%d] QueueCount = %d, ", i, QFPs[i].queueCount);
#ifdef DEBUG_STDOUT
		std::cout << QFPs[i];
#endif
		VkBool32 b = VK_FALSE;
		VERIFY_SUCCEEDED(vkGetPhysicalDeviceSurfaceSupportKHR(PD, i, Surface, &b));
		if (b) { Logf("PRESENT"); }
		Log("\n");
	}

	constexpr uint8_t QueueFamilyPropMax = 8;
	VERIFY(size(QFPs) <= QueueFamilyPropMax);
	std::bitset<QueueFamilyPropMax> GraphicsMask;
	std::bitset<QueueFamilyPropMax> PresentMask;
	std::bitset<QueueFamilyPropMax> ComputeMask;
	std::bitset<QueueFamilyPropMax> TransferMask;
	std::bitset<QueueFamilyPropMax> SparseBindingMask;
	std::bitset<QueueFamilyPropMax> ProtectedMask;
	//!< (ここでは) 機能を持つ最初のキューファミリインデックスを見つける (Find queue family index for each functions)
	GraphicsQueueFamilyIndex = (std::numeric_limits<uint32_t>::max)();
	PresentQueueFamilyIndex = (std::numeric_limits<uint32_t>::max)();
	ComputeQueueFamilyIndex = (std::numeric_limits<uint32_t>::max)();
	for (auto i = 0; i < std::size(QFPs); ++i) {
		const auto& QFP = QFPs[i];
		if (VK_QUEUE_GRAPHICS_BIT & QFP.queueFlags) {
			GraphicsMask.set(i);
			if ((std::numeric_limits<uint32_t>::max)() == GraphicsQueueFamilyIndex) {
				GraphicsQueueFamilyIndex = i;
			}
		}
		VkBool32 b = VK_FALSE;
		VERIFY_SUCCEEDED(vkGetPhysicalDeviceSurfaceSupportKHR(PD, i, Surface, &b));
		if (b) {
			PresentMask.set(i);
			if ((std::numeric_limits<uint32_t>::max)() == PresentQueueFamilyIndex) {
				PresentQueueFamilyIndex = i;
			}
		}
		if (VK_QUEUE_COMPUTE_BIT & QFP.queueFlags) {
			ComputeMask.set(i);
			//!< コンピュートはグラフィックと異なるファミリを希望
			if ((std::numeric_limits<uint32_t>::max)() == ComputeQueueFamilyIndex && GraphicsQueueFamilyIndex != static_cast<uint32_t>(i)) {
				ComputeQueueFamilyIndex = i;
			}
		}
		if (VK_QUEUE_TRANSFER_BIT & QFP.queueFlags) { TransferMask.set(i); }
		if (VK_QUEUE_SPARSE_BINDING_BIT & QFP.queueFlags) { SparseBindingMask.set(i); }
		if (VK_QUEUE_PROTECTED_BIT & QFP.queueFlags) { ProtectedMask.set(i); }
	}
	//!< 見つからない場合はグラフィックと同じファミリで良い
	if ((std::numeric_limits<uint32_t>::max)() == ComputeQueueFamilyIndex) {
		for (auto i = 0; i < size(QFPs); ++i) {
			if (VK_QUEUE_COMPUTE_BIT & QFPs[i].queueFlags) {
				if ((std::numeric_limits<uint32_t>::max)() == ComputeQueueFamilyIndex) {
					ComputeQueueFamilyIndex = i;
				}
			}
		}
	}
	std::cout << "\t\tGRAPHICS\t" << GraphicsMask << std::endl;
	std::cout << "\t\tPRESENT\t\t" << PresentMask << std::endl;
	std::cout << "\t\tCOMPUTE\t\t" << ComputeMask << std::endl;
	std::cout << "\t\tTRANSFER\t" << TransferMask << std::endl;
	std::cout << "\t\tSPARCE_BINDING\t" << SparseBindingMask << std::endl;
	std::cout << "\t\tPROTECTED\t" << ProtectedMask << std::endl;

	//!< キューファミリ内でのインデックス及びプライオリティ、ここではグラフィック、プレゼント、コンピュートの分をプライオリティ0.5fで追加している
	using FamilyIndexAndPriorities = std::map<uint32_t, std::vector<float>>;
	FamilyIndexAndPriorities FIAP;
	const auto GraphicsQueueIndexInFamily = static_cast<uint32_t>(std::size(FIAP[GraphicsQueueFamilyIndex]));
	FIAP[GraphicsQueueFamilyIndex].emplace_back(0.5f);
	const auto PresentQueueIndexInFamily = static_cast<uint32_t>(std::size(FIAP[PresentQueueFamilyIndex]));
	FIAP[PresentQueueFamilyIndex].emplace_back(0.5f);
	const auto ComputeQueueIndexInFamily = static_cast<uint32_t>(std::size(FIAP[ComputeQueueFamilyIndex]));
	FIAP[ComputeQueueFamilyIndex].emplace_back(0.5f);

	Log("\n");
	Logf("\t\tGRAPHICS : QueueFamilyIndex = %d, IndexInFamily = %d\n", GraphicsQueueFamilyIndex, GraphicsQueueIndexInFamily);
	Logf("\t\tPRESENT  : QueueFamilyIndex = %d, IndexInFamily = %d\n", PresentQueueFamilyIndex, PresentQueueIndexInFamily);
	Logf("\t\tCOMPUTE  : QueueFamilyIndex = %d, IndexInFamily = %d\n", ComputeQueueFamilyIndex, ComputeQueueIndexInFamily);
	for (const auto& i : FIAP) {
		Logf("\t\tPriorites[%d] = { ", i.first);
		for (const auto j : i.second) {
			Logf("%f, ", j);
		}
		Log("}\n");
	}

	//!< キュー作成情報 (Queue create information)
	std::vector<VkDeviceQueueCreateInfo> DQCIs;
	for (const auto& i : FIAP) {
		DQCIs.emplace_back(VkDeviceQueueCreateInfo({
							.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
							.pNext = nullptr,
							.flags = 0,
							.queueFamilyIndex = i.first,
							.queueCount = static_cast<uint32_t>(std::size(i.second)), .pQueuePriorities = std::data(i.second)
			}));
	}
#pragma endregion

	//!< ここではサポートされるフィーチャーを全て有効にしている、パフォーマンス的には不必要なものはオフにした方が良い #PERFORMANCE_TODO
	VkPhysicalDeviceFeatures PDF;
	vkGetPhysicalDeviceFeatures(PD, &PDF);
	VkPhysicalDeviceFeatures2 PDF2 = {
		.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
		.pNext = pNext,
		.features = PDF //!< PDFはここに指定
	};
	const VkDeviceCreateInfo DCI = {
		.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
		.pNext = &PDF2, //!< ここに PDF2 を指定
		.flags = 0,
		.queueCreateInfoCount = static_cast<uint32_t>(size(DQCIs)), .pQueueCreateInfos = data(DQCIs),
		.enabledLayerCount = 0, .ppEnabledLayerNames = nullptr,
		.enabledExtensionCount = static_cast<uint32_t>(std::size(ExtensionNames)), .ppEnabledExtensionNames = std::data(ExtensionNames),
		.pEnabledFeatures = nullptr //!< PDF は PDF2.features へ指定しているので、ここは nullptr
	};
	VERIFY_SUCCEEDED(vkCreateDevice(PD, &DCI, GetAllocationCallbacks(), &Device));

#define VK_PROC_ADDR(proc) vk ## proc = reinterpret_cast<PFN_vk ## proc>(vkGetDeviceProcAddr(Device, "vk" #proc)); VERIFY(nullptr != vk ## proc && #proc && #proc);
#ifdef VK_NO_PROTOYYPES
#include "VKDeviceProcAddr.h"
#endif
	if (end(ExtensionNames) != std::ranges::find(ExtensionNames, VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME)) {
#include "VKDeviceProcAddr_RayTracing.h"
	}
	if (end(ExtensionNames) != std::ranges::find(ExtensionNames, VK_NV_MESH_SHADER_EXTENSION_NAME)) {
#include "VKDeviceProcAddr_MeshShaderNV.h"
	}
	if (end(ExtensionNames) != std::ranges::find(ExtensionNames, VK_EXT_MESH_SHADER_EXTENSION_NAME)) {
#include "VKDeviceProcAddr_MeshShader.h"
	}
#undef VK_PROC_ADDR

	//!< グラフィック、プレゼントキューは同じインデックスの場合もあるが別の変数に取得しておく (Graphics and presentation index may be same, but save to individual variables)
	vkGetDeviceQueue(Device, GraphicsQueueFamilyIndex, GraphicsQueueIndexInFamily, &GraphicsQueue);
	vkGetDeviceQueue(Device, PresentQueueFamilyIndex, PresentQueueIndexInFamily, &PresentQueue);
	vkGetDeviceQueue(Device, ComputeQueueFamilyIndex, ComputeQueueIndexInFamily, &ComputeQueue);
	//vkGetDeviceQueue(Device, TransferQueueFamilyIndex, TransferQueueIndex, &TransferQueue);
	//vkGetDeviceQueue(Device, SparceBindingQueueFamilyIndex, SparceBindingQueueIndex, &SparceBindingQueue);

	LOG_OK();
}

void VK::AllocatePrimaryCommandBuffer(const size_t Count)
{
	//!< キューファミリが異なる場合は別のコマンドプールを用意する必要がある、そのキューにのみサブミットできる
	//!< 複数スレッドで同時にレコーディングするには、別のコマンドプールからアロケートされたコマンドバッファである必要がある (コマンドプールは複数スレッドからアクセス不可)
	//!< VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT	: コマンドバッファ毎にリセットが可能、指定しない場合はプール毎にまとめてリセット (コマンドバッファのレコーディング開始時に暗黙的にリセットされるので注意)
	//!< VK_COMMAND_POOL_CREATE_TRANSIENT_BIT				: 短命で、何度もサブミットしない、すぐにリセットやリリースされる場合に指定
	const VkCommandPoolCreateInfo CPCI = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
		.pNext = nullptr,
		.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
		.queueFamilyIndex = GraphicsQueueFamilyIndex
	};
	VERIFY_SUCCEEDED(vkCreateCommandPool(Device, &CPCI, GetAllocationCallbacks(), &CommandPools.emplace_back()));

	const auto CP = CommandPools[0];
	//!< VK_COMMAND_BUFFER_LEVEL_PRIMARY : 直接キューにサブミットできる、セカンダリをコールできる (Can be submit, can execute secondary)
	CommandBuffers.resize(Count);
	const VkCommandBufferAllocateInfo CBAI = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		.pNext = nullptr,
		.commandPool = CP,
		.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
		.commandBufferCount = static_cast<uint32_t>(std::size(CommandBuffers))
	};
	VERIFY_SUCCEEDED(vkAllocateCommandBuffers(Device, &CBAI, std::data(CommandBuffers)));
}
void VK::AllocateSecondaryCommandBuffer(const size_t Count)
{
	//!< セカンダリ用、必ずしも別プールにする必要は無い (DXのコマンドアロケータは DIRECT と BUNDLE で別なのでそれに合わせる形で別にしている)
	const VkCommandPoolCreateInfo CPCI = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
		.pNext = nullptr,
		.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
		.queueFamilyIndex = GraphicsQueueFamilyIndex
	};
	VERIFY_SUCCEEDED(vkCreateCommandPool(Device, &CPCI, GetAllocationCallbacks(), &SecondaryCommandPools.emplace_back()));

	const auto SCP = SecondaryCommandPools[0];
	//!< VK_COMMAND_BUFFER_LEVEL_SECONDARY	: サブミットできない、プライマリから実行されるのみ (Cannot submit, only executed from primary)
	SecondaryCommandBuffers.resize(Count);
	const VkCommandBufferAllocateInfo CBAI = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		.pNext = nullptr,
		.commandPool = SCP,
		.level = VK_COMMAND_BUFFER_LEVEL_SECONDARY,
		.commandBufferCount = static_cast<uint32_t>(Count)
	};
	VERIFY_SUCCEEDED(vkAllocateCommandBuffers(Device, &CBAI, data(SecondaryCommandBuffers)));
}
//void VK::AllocateSecondaryCommandBuffer()
//{
//	//!< セカンダリ用、必ずしも別プールにする必要は無い (DXのコマンドアロケータは DIRECT と BUNDLE で別なのでそれに合わせる形で別にしている)
//	const VkCommandPoolCreateInfo CPCI = {
//		.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
//		.pNext = nullptr,
//		.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
//		.queueFamilyIndex = GraphicsQueueFamilyIndex
//	};
//	VERIFY_SUCCEEDED(vkCreateCommandPool(Device, &CPCI, GetAllocationCallbacks(), &SecondaryCommandPools.emplace_back()));
//
//	const auto SCP = SecondaryCommandPools[0];
//	//!< VK_COMMAND_BUFFER_LEVEL_SECONDARY	: サブミットできない、プライマリから実行されるのみ (Cannot submit, only executed from primary)
//	const auto Count = size(SwapchainImages);
//	SecondaryCommandBuffers.resize(Count);
//	const VkCommandBufferAllocateInfo CBAI = {
//		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
//		.pNext = nullptr,
//		.commandPool = SCP,
//		.level = VK_COMMAND_BUFFER_LEVEL_SECONDARY,
//		.commandBufferCount = static_cast<uint32_t>(Count)
//	};
//	VERIFY_SUCCEEDED(vkAllocateCommandBuffers(Device, &CBAI, data(SecondaryCommandBuffers)));
//}
void VK::AllocateComputeCommandBuffer()
{
	//!< キューファミリが同じ場合は必ずしも別プールにする必要はない、ファミリが異なる場合や別スレッドで使用したい場合には別プールとすること
	const VkCommandPoolCreateInfo CPCI = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
		.pNext = nullptr,
		.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
		.queueFamilyIndex = ComputeQueueFamilyIndex
	};
	VERIFY_SUCCEEDED(vkCreateCommandPool(Device, &CPCI, GetAllocationCallbacks(), &ComputeCommandPools.emplace_back()));

	const VkCommandBufferAllocateInfo CBAI = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		.pNext = nullptr,
		.commandPool = ComputeCommandPools[0],
		.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
		.commandBufferCount = 1
	};
	VERIFY_SUCCEEDED(vkAllocateCommandBuffers(Device, &CBAI, &ComputeCommandBuffers.emplace_back()));
}

VkSurfaceFormatKHR VK::SelectSurfaceFormat(VkPhysicalDevice PD, VkSurfaceKHR Sfc)
{
	uint32_t Count = 0;
	VERIFY_SUCCEEDED(vkGetPhysicalDeviceSurfaceFormatsKHR(PD, Sfc, &Count, nullptr));
	VERIFY(Count);
	std::vector<VkSurfaceFormatKHR> SFs(Count);
	VERIFY_SUCCEEDED(vkGetPhysicalDeviceSurfaceFormatsKHR(PD, Sfc, &Count, std::data(SFs)));

	//!< VK_FORMAT_R8G8B8A8_UNORM があれば選択 (Select VK_FORMAT_R8G8B8A8_UNORM if possible)
	//!< (GLSL layout のカラーフォーマットが RGBA 系なので)
	const auto SelectedIndex = [&]() {
		//!< 要素が 1 つのみで UNDEFINED の場合、制限は無く好きなものを選択できる (If there is only 1 element and which is UNDEFINED, we can choose any)
		if (1 == std::size(SFs) && VK_FORMAT_UNDEFINED == SFs[0].format) {
			return -1;
		}
#ifdef USE_HDR
		auto It = std::ranges::find_if(SFs, 
			[](const auto& rhs) {
				switch (SF.colorSpace) {
				default: break;
					//!< HDR対応のディスプレイの場合は以下が返ることが期待される
				case VK_COLOR_SPACE_HDR10_HLG_EXT:
				case VK_COLOR_SPACE_HDR10_ST2084_EXT:
				case VK_COLOR_SPACE_DOLBYVISION_EXT:
					return rhs.format == VK_FORMAT_R8G8B8A8_UNORM;
				}
			});
#else
		auto It = std::ranges::find_if(SFs, [](const auto& rhs) { return rhs.format == VK_FORMAT_R8G8B8A8_UNORM; });
		if (It != std::cend(SFs)) {
			return static_cast<int>(std::distance(std::begin(SFs), It));
		}
#endif
		//!< VK_FORMAT_UNDEFINED でなければ良しとする (If not found, select but VK_FORMAT_UNDEFINED)
		It = std::ranges::find_if(SFs, [](const auto& rhs) { return rhs.format != VK_FORMAT_UNDEFINED; });
		if (It != std::cend(SFs)) {
			return static_cast<int>(std::distance(std::begin(SFs), It));
		}
		
		//!< ここに来てはいけない
		VERIFY(false);
		return 0;
	}();

	//!< ColorSpace はハードウェア上でのカラーコンポーネントの表現(リニア、ノンリニア、エンコード、デコード等)
	Log("\t\tFormats\n");
	for (auto i = 0; i < std::size(SFs); ++i) {
		const auto& SF = SFs[i];
		Log("\t\t\t");
		if (i == SelectedIndex) {
			Log("->");
		}
		Logf("Format = %s, ColorSpace = %s\n", GetFormatChar(SF.format), GetColorSpaceChar(SF.colorSpace));
	}
	if (-1 == SelectedIndex) {
		Log("\t\t\t->");
		constexpr auto SF = VkSurfaceFormatKHR({ VK_FORMAT_R8G8B8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR });
		Logf("Format = %s, ColorSpace = %s\n", GetFormatChar(SF.format), GetColorSpaceChar(SF.colorSpace));
		return SF;
	}

	return SFs[SelectedIndex];
}
VkPresentModeKHR VK::SelectSurfacePresentMode(VkPhysicalDevice PD, VkSurfaceKHR Sfc)
{
	uint32_t Count;
	VERIFY_SUCCEEDED(vkGetPhysicalDeviceSurfacePresentModesKHR(PD, Sfc, &Count, nullptr));
	VERIFY(Count);
	std::vector<VkPresentModeKHR> PMs(Count);
	VERIFY_SUCCEEDED(vkGetPhysicalDeviceSurfacePresentModesKHR(PD, Sfc, &Count, std::data(PMs)));

	//!< 可能なら VK_PRESENT_MODE_MAILBOX_KHR を選択、そうでなければ VK_PRESENT_MODE_FIFO_KHR を選択 (Want to select VK_PRESENT_MODE_MAILBOX_KHR, or select VK_PRESENT_MODE_FIFO_KHR)
	/**
	@brief VkPresentModeKHR
	* VK_PRESENT_MODE_IMMEDIATE_KHR		... vsyncを待たないのでテアリングが起こる (Tearing happen, no vsync wait)
	* VK_PRESENT_MODE_MAILBOX_KHR		... キューは 1 つで常に最新で上書きされる、vsyncで更新される (Queue is 1, and always update to new image and updated on vsync)
	* VK_PRESENT_MODE_FIFO_KHR			... VulkanAPI が必ずサポートする vsyncで更新 (VulkanAPI always support this, updated on vsync)
	* VK_PRESENT_MODE_FIFO_RELAXED_KHR	... FIFOに在庫がある場合は vsyncを待つが、間に合わない場合は即座に更新されテアリングが起こる (If FIFO is not empty wait vsync. but if empty, updated immediately and tearing will happen)
	*/
	const auto SelectedPresentMode = [&]() {
		//!< 可能なら MAILBOX (Select MAILBOX, If possible)
		const auto It = std::ranges::find(PMs, VK_PRESENT_MODE_MAILBOX_KHR);
		if(It != std::cend(PMs)) {
			return *It;
		}
		//!< FIFO は VulkanAPI が必ずサポートする (VulkanAPI always support FIFO)
		return *std::ranges::find(PMs, VK_PRESENT_MODE_FIFO_KHR);
	}();

	Log("\tPresent Mode\n");
#define VK_PRESENT_MODE_ENTRY(entry) case VK_PRESENT_MODE_##entry: Logf("%s\n", #entry); break
	for (auto i : PMs) {
		Logf("\t\t%s", SelectedPresentMode == i ? "-> " : "");
		switch (i) {
		default: VERIFY(false); break; //!< VK_PRESENT_MODE が増えた可能性がある、必要に応じてエントリを増やすこと
		VK_PRESENT_MODE_ENTRY(IMMEDIATE_KHR);
		VK_PRESENT_MODE_ENTRY(MAILBOX_KHR);
		VK_PRESENT_MODE_ENTRY(FIFO_KHR);
		VK_PRESENT_MODE_ENTRY(FIFO_RELAXED_KHR);
		VK_PRESENT_MODE_ENTRY(FIFO_LATEST_READY_EXT);
		}
#undef VK_PRESENT_MODE_ENTRY
	}

	return SelectedPresentMode;
}

void VK::DestroySwapchain() 
{
	VERIFY_SUCCEEDED(vkDeviceWaitIdle(Device));

	for (auto i : Framebuffers) {
		vkDestroyFramebuffer(Device, i, nullptr);
	}
	Framebuffers.clear();

	for (auto& i : Swapchain.ImageAndViews) {
		vkDestroyImageView(Device, i.second, GetAllocationCallbacks());
	}
	Swapchain.ImageAndViews.clear();

	if (VK_NULL_HANDLE != Swapchain.VkSwapchain) [[likely]] {
		vkDestroySwapchainKHR(Device, Swapchain.VkSwapchain, GetAllocationCallbacks());
		Swapchain.VkSwapchain = VK_NULL_HANDLE;
	}
}
bool VK::ReCreateSwapchain() 
{
	if (VK_NULL_HANDLE == Swapchain.VkSwapchain) {
		if (CreateSwapchain()) {
			CreateFramebuffer();

			//!< リサイズされた可能性があるのでやっておく
			DestroyViewports();
			CreateViewports();

			//!< フレームバッファが作り直されたので、コマンド発行をやり直す
			PopulateCommandBuffer();

			return true;
		}
		return false;
	}
	return true;
}
//!< 手動でクリアする場合には VkImageUsageFlags に追加で VK_IMAGE_USAGE_TRANSFER_DST_BIT の指定が必要
bool VK::CreateSwapchain(VkPhysicalDevice PD, VkSurfaceKHR Sfc, const uint32_t Width, const uint32_t Height, const VkImageUsageFlags AdditionalUsage)
{
	VkSurfaceCapabilitiesKHR SC;
	VERIFY_SUCCEEDED(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(PD, Sfc, &SC));
#ifdef DEBUG_STDOUT
	std::cout << SC;
#endif
	//!< VK_ERROR_OUT_OF_DATE_KHR 後、暫く Extent = {0, 0} が返る期間があるので、まともな値が返るまで失敗と扱う
	if (0 == SC.currentExtent.width) {
		return false;
	}
	//!< 最低よりも1枚多く取りたい、ただし最大値でクランプする(maxImageCount が0の場合は上限無し)
	const auto ImageCount = (std::min)(SC.minImageCount + 1, 0 == SC.maxImageCount ? (std::numeric_limits<uint32_t>::max)() : SC.maxImageCount);
	Logf("\t\t\tImagCount = %d\n", ImageCount);

	//!< サーフェスのフォーマットを選択 覚えておく
	SurfaceFormat = SelectSurfaceFormat(PD, Sfc);
	//ColorFormat = SurfaceFormat.format; //!< カラーファーマットは覚えておく

	//!< サーフェスのサイズを選択
	//!< currentExtent.width == 0xffffffff の場合はスワップチェインのサイズから決定する (If 0xffffffff, surface size will be determined by the extent of a swapchain targeting the surface)
	Swapchain.Extent = 0xffffffff != SC.currentExtent.width ? SC.currentExtent : VkExtent2D({ .width = (std::clamp)(Width, SC.maxImageExtent.width, SC.minImageExtent.width), .height = (std::clamp)(Height, SC.minImageExtent.height, SC.minImageExtent.height) });
	Logf("\t\t\tSurfaceExtent = %d x %d\n", Swapchain.Extent.width, Swapchain.Extent.height);

	//!< レイヤー、ステレオレンダリング等をしたい場合は1以上になるが、ここでは1
	uint32_t ImageArrayLayers = 1;

	VERIFY((VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT & SC.supportedUsageFlags));

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
	auto OldSwapchain = Swapchain.VkSwapchain;
	const VkSwapchainCreateInfoKHR SCI = {
		.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
		.pNext = nullptr,
		.flags = 0,
		.surface = Sfc,
		.minImageCount = ImageCount,
		.imageFormat = SurfaceFormat.format, .imageColorSpace = SurfaceFormat.colorSpace,
		.imageExtent = Swapchain.Extent,
		.imageArrayLayers = ImageArrayLayers,
		.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | AdditionalUsage,
		.imageSharingMode = std::empty(QueueFamilyIndices) ? VK_SHARING_MODE_EXCLUSIVE : VK_SHARING_MODE_CONCURRENT, 
		.queueFamilyIndexCount = static_cast<uint32_t>(std::size(QueueFamilyIndices)), .pQueueFamilyIndices = std::data(QueueFamilyIndices),
		.preTransform = SurfaceTransform,
		.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
		.presentMode = SurfacePresentMode,
		.clipped = VK_TRUE,
		.oldSwapchain = OldSwapchain
	};
	VERIFY_SUCCEEDED(vkCreateSwapchainKHR(Device, &SCI, GetAllocationCallbacks(), &Swapchain.VkSwapchain));

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
	VERIFY(size(SCs) == size(MDs));
	vkSetHdrMetadataEXT(Device, static_cast<uint32_t>(size(SCs)), data(SCs), data(MDs));
#endif
	
	//!< (あれば)前のやつは破棄
	if (VK_NULL_HANDLE != OldSwapchain) {

		vkDestroySwapchainKHR(Device, OldSwapchain, GetAllocationCallbacks());
	}

	LOG_OK();

	return true;
}
void VK::GetSwapchainImages() 
{
	//!< 既存があるならまず破棄する
	for (auto i : Swapchain.ImageAndViews) {
		vkDestroyImageView(Device, i.second, GetAllocationCallbacks());
	}
	Swapchain.ImageAndViews.clear();

	uint32_t Count;
	VERIFY_SUCCEEDED(vkGetSwapchainImagesKHR(Device, Swapchain.VkSwapchain, &Count, nullptr));
	std::vector<VkImage> Images(Count);
	VERIFY_SUCCEEDED(vkGetSwapchainImagesKHR(Device, Swapchain.VkSwapchain, &Count, std::data(Images)));
	for (auto i : Images) {
		const VkImageViewCreateInfo IVCI = {
			.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
			.pNext = nullptr,
			.flags = 0,
			.image = i,
			.viewType = VK_IMAGE_VIEW_TYPE_2D,
			.format = SurfaceFormat.format,
			.components = VkComponentMapping({.r = VK_COMPONENT_SWIZZLE_IDENTITY, .g = VK_COMPONENT_SWIZZLE_IDENTITY, .b = VK_COMPONENT_SWIZZLE_IDENTITY, .a = VK_COMPONENT_SWIZZLE_IDENTITY, }),
			.subresourceRange = VkImageSubresourceRange({.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .baseMipLevel = 0, .levelCount = 1, .baseArrayLayer = 0, .layerCount = 1 })
		};
		VkImageView IV;
		VERIFY_SUCCEEDED(vkCreateImageView(Device, &IVCI, GetAllocationCallbacks(), &IV));
		Swapchain.ImageAndViews.emplace_back(ImageAndView({ i, IV }));
	}
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

	CreateSwapchain(SelectedPhysDevice.first, Surface, Width, Height);
}
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
void VK::CreateBufferMemory(VkBuffer* Buffer, VkDeviceMemory* DeviceMemory, const VkDevice Device, const VkPhysicalDeviceMemoryProperties& PDMP, const size_t Size, const VkBufferUsageFlags BUF, const VkMemoryPropertyFlags MPF, const void* Source)
{
	VERIFY(Size);
#pragma region BUFFER
	//!< バッファは作成時に指定した使用法でしか使用できない、ここでは VK_SHARING_MODE_EXCLUSIVE 決め打ちにしている #VK_TODO (Using VK_SHARING_MODE_EXCLUSIVE here)
	//!< VK_SHARING_MODE_EXCLUSIVE	: 複数ファミリのキューが同時アクセスできない、他のファミリからアクセスしたい場合はオーナーシップの移譲が必要
	//!< VK_SHARING_MODE_CONCURRENT	: 複数ファミリのキューが同時アクセス可能、オーナーシップの移譲も必要無し、パフォーマンスは悪い
	constexpr std::array<uint32_t, 0> QFI = {};
	const VkBufferCreateInfo BCI = {
		.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
		.size = Size,
		.usage = BUF,
		.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
		.queueFamilyIndexCount = static_cast<uint32_t>(std::size(QFI)), .pQueueFamilyIndices = std::data(QFI)
	};
	VERIFY_SUCCEEDED(vkCreateBuffer(Device, &BCI, GetAllocationCallbacks(), Buffer));
#pragma endregion

#pragma  region MEMORY
	const VkBufferMemoryRequirementsInfo2 BMRI = {
		.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_REQUIREMENTS_INFO_2,
		.pNext = nullptr,
		.buffer = *Buffer
	};
	VkMemoryRequirements2 MR = {
		.sType = VK_STRUCTURE_TYPE_MEMORY_REQUIREMENTS_2,
		.pNext = nullptr,
	};
	vkGetBufferMemoryRequirements2(Device, &BMRI, &MR);
#ifdef _DEBUG
	EnumerateMemoryRequirements(MR.memoryRequirements, PDMP);
	const auto TypeIndex = GetMemoryTypeIndex(PDMP, MR.memoryRequirements.memoryTypeBits, MPF);
	Logf("\t\tAllocateDeviceMemory = %llu / %llu (HeapIndex = %d, Align = %llu)\n", MR.memoryRequirements.size, PDMP.memoryHeaps[PDMP.memoryTypes[TypeIndex].heapIndex].size, PDMP.memoryTypes[TypeIndex].heapIndex, MR.memoryRequirements.alignment);
#endif
	//!< VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT が指定された場合に pNext へ指定する
	constexpr VkMemoryAllocateFlagsInfo MAFI = {
		.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO, 
		.pNext = nullptr, 
		.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT, 
		.deviceMask = 0
	};
	const VkMemoryAllocateInfo MAI = {
		.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
		.pNext = (VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT & BUF) ? &MAFI : nullptr,
		.allocationSize = MR.memoryRequirements.size,
		.memoryTypeIndex = GetMemoryTypeIndex(PDMP, MR.memoryRequirements.memoryTypeBits, MPF)
	};
	VERIFY_SUCCEEDED(vkAllocateMemory(Device, &MAI, GetAllocationCallbacks(), DeviceMemory));
#pragma endregion

	VERIFY_SUCCEEDED(vkBindBufferMemory(Device, *Buffer, *DeviceMemory, 0));

#pragma region COPY
	if (nullptr != Source) {
		constexpr auto MapSize = VK_WHOLE_SIZE;
		void* Data;
		VERIFY_SUCCEEDED(vkMapMemory(Device, *DeviceMemory, 0, MapSize, static_cast<VkMemoryMapFlags>(0), &Data)); {
			memcpy(Data, Source, Size);
			//!< メモリコンテンツが変更されたことを明示的にドライバへ知らせる(vkMapMemory()した状態でやること)
			if (!(VK_MEMORY_PROPERTY_HOST_COHERENT_BIT & BUF)) {
				const std::array MMRs = { 
					VkMappedMemoryRange({
						.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE, 
						.pNext = nullptr,
						.memory = *DeviceMemory,
						.offset = 0, .size = MapSize
					}),
				};
				VERIFY_SUCCEEDED(vkFlushMappedMemoryRanges(Device, static_cast<uint32_t>(std::size(MMRs)), std::data(MMRs)));
			}
		} vkUnmapMemory(Device, *DeviceMemory);
	}
#pragma endregion
}

void VK::CreateImageMemory(VkImage* Image, VkDeviceMemory* DM, const VkDevice Device, const VkPhysicalDeviceMemoryProperties& PDMP, const VkImageCreateFlags ICF, const VkImageType IT, const VkFormat Format, const VkExtent3D& Extent, const uint32_t Levels, const uint32_t Layers, const VkImageUsageFlags IUF)
{
	constexpr std::array<uint32_t, 0> QueueFamilyIndices = {};
	const VkImageCreateInfo ICI = {
		.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
		.pNext = nullptr,
		.flags = ICF,
		.imageType = IT,
		.format = Format,
		.extent = Extent,
		.mipLevels = Levels,
		.arrayLayers = Layers,
		.samples = VK_SAMPLE_COUNT_1_BIT,
		.tiling = VK_IMAGE_TILING_OPTIMAL,
		.usage = IUF,
		.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
		.queueFamilyIndexCount = static_cast<uint32_t>(size(QueueFamilyIndices)), .pQueueFamilyIndices = data(QueueFamilyIndices),
		.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED
	};
	VERIFY_SUCCEEDED(vkCreateImage(Device, &ICI, GetAllocationCallbacks(), Image));

	const VkImageMemoryRequirementsInfo2 IMRI = {
		.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_REQUIREMENTS_INFO_2,
		.pNext = nullptr,
		.image = *Image
	};
	VkMemoryRequirements2 MR = {
		.sType = VK_STRUCTURE_TYPE_MEMORY_REQUIREMENTS_2,
		.pNext = nullptr,
	};
	vkGetImageMemoryRequirements2(Device, &IMRI, &MR);

	const VkMemoryAllocateInfo MAI = {
		.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
		.pNext = nullptr,
		.allocationSize = MR.memoryRequirements.size,
		.memoryTypeIndex = VK::GetMemoryTypeIndex(PDMP, MR.memoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
	};
	VERIFY_SUCCEEDED(vkAllocateMemory(Device, &MAI, GetAllocationCallbacks(), DM));
	VERIFY_SUCCEEDED(vkBindImageMemory(Device, *Image, *DM, 0));
}


//void VK::CreateStorageTexelBuffer_Example()
//{
//	//!< これはサンプルコード
//
//	VkBuffer Buffer; //!< #VK_TODO
//	const VkDeviceSize Size = 0; //!< #VK_TODO
//	CreateBuffer(&Buffer, VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT, Size);
//
//	VkDeviceMemory DM; //!< #VK_TODO
//	AllocateDeviceMemory(&DM, Buffer, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
//	VERIFY_SUCCEEDED(vkBindBufferMemory(Device, Buffer, DM, 0));
//
//	VkBufferView View; //!< #VK_TODO
//	const VkBufferViewCreateInfo BVCI = {
//		.sType = VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO,
//		.pNext = nullptr,
//		.flags = 0,
//		.buffer = Buffer,
//		.format = VK_FORMAT_R8G8B8A8_UINT,
//		.offset = 0,
//		.range = VK_WHOLE_SIZE
//	};
//	VERIFY_SUCCEEDED(vkCreateBufferView(Device, &BVCI, GetAllocationCallbacks(), &View));
//
//	//!< サポートされているかのチェック
//	VkFormatProperties FP;
//	vkGetPhysicalDeviceFormatProperties(GetCurrentPhysicalDevice(), BVCI.format, &FP);
//	assert((FP.bufferFeatures & VK_FORMAT_FEATURE_STORAGE_TEXEL_BUFFER_BIT) && "");
//	//!< #VK_TODO アトミック操作をする場合
//	if (false/*bUseAtomic*/) {
//		assert((FP.linearTilingFeatures & VK_FORMAT_FEATURE_STORAGE_TEXEL_BUFFER_ATOMIC_BIT) && "");
//	}
//}

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
		.setLayoutCount = static_cast<uint32_t>(std::size(DSLs)), .pSetLayouts = std::data(DSLs),
		.pushConstantRangeCount = static_cast<uint32_t>(std::size(PCRs)), .pPushConstantRanges = std::data(PCRs)
	};
	VERIFY_SUCCEEDED(vkCreatePipelineLayout(Device, &PLCI, GetAllocationCallbacks(), &PL));

	LOG_OK();
}

//!< デスクリプタセットを個々に解放したい場合には .flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT を指定する、
//!< その場合断片化は自分で管理しなくてはならない (指定しない場合はプール毎にまとめて解放しかできない)
//!< 1つのブールに対して、複数スレッドで同時にデスクリプタセットを確保することはできない (スレッド毎に別プールにすること)
void VK::CreateDescriptorPool(VkDescriptorPool& DP, const VkDescriptorPoolCreateFlags Flags, const std::vector<VkDescriptorPoolSize>& DPSs)
{
	const VkDescriptorPoolCreateInfo DPCI = {
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
		.pNext = nullptr,
		.flags = Flags,
		.maxSets = std::ranges::max_element(DPSs, [](const auto& lhs, const auto& rhs) { return lhs.descriptorCount < rhs.descriptorCount; })->descriptorCount,
		.poolSizeCount = static_cast<uint32_t>(size(DPSs)), .pPoolSizes = data(DPSs)
	};
	VERIFY_SUCCEEDED(vkCreateDescriptorPool(Device, &DPCI, GetAllocationCallbacks(), &DP));
}

void VK::CreateDescriptorUpdateTemplate(VkDescriptorUpdateTemplate& DUT, const VkPipelineBindPoint PBP, const std::vector<VkDescriptorUpdateTemplateEntry>& DUTEs, const VkDescriptorSetLayout DSL)
{
	const VkDescriptorUpdateTemplateCreateInfo DUTCI = {
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_UPDATE_TEMPLATE_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
		.descriptorUpdateEntryCount = static_cast<uint32_t>(size(DUTEs)), .pDescriptorUpdateEntries = data(DUTEs),
		.templateType = VK_DESCRIPTOR_UPDATE_TEMPLATE_TYPE_DESCRIPTOR_SET,
		.descriptorSetLayout = DSL,
		.pipelineBindPoint = PBP, 
		.pipelineLayout = VK_NULL_HANDLE, .set = 0
	};
	VERIFY_SUCCEEDED(vkCreateDescriptorUpdateTemplate(Device, &DUTCI, GetAllocationCallbacks(), &DUT));
}

//!< uint32_t -> A8B8R8G8
void VK::CreateTextureArray1x1(const std::vector<uint32_t>& Colors, const VkPipelineStageFlags2 PSF)
{
	constexpr auto Format = VK_FORMAT_R8G8B8A8_UNORM;
	constexpr auto Extent = VkExtent3D({ .width = 1, .height = 1, .depth = 1 });

	const auto& PDMP = SelectedPhysDevice.second.PDMP;
	Textures.emplace_back().Create(Device, PDMP, Format, Extent, 1, static_cast<uint32_t>(size(Colors)));

	const auto CB = CommandBuffers[0];
	VK::Scoped<BufferMemory> StagingBuffer(Device);
	StagingBuffer.Create(Device, PDMP, sizeof(Colors), VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, data(Colors));
	std::vector<VkBufferImageCopy2> BICs;
	for (uint32_t i = 0; i < size(Colors); ++i) {
		BICs.emplace_back(VkBufferImageCopy2({
			.sType = VK_STRUCTURE_TYPE_BUFFER_IMAGE_COPY_2,
			.pNext = nullptr,
			.bufferOffset = i * sizeof(Colors[0]), .bufferRowLength = 0, .bufferImageHeight = 0,
			.imageSubresource = VkImageSubresourceLayers({.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .mipLevel = 0, .baseArrayLayer = static_cast<uint32_t>(i), .layerCount = 1 }),
			.imageOffset = VkOffset3D({.x = 0, .y = 0, .z = 0 }),.imageExtent = Extent }));
	}
	constexpr VkCommandBufferBeginInfo CBBI = { .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO, .pNext = nullptr, .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, .pInheritanceInfo = nullptr };
	VERIFY_SUCCEEDED(vkBeginCommandBuffer(CB, &CBBI)); {
		PopulateCopyBufferToImageCommand(CB, StagingBuffer.Buffer, Textures.back().Image, VK_ACCESS_2_SHADER_READ_BIT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, PSF, BICs, 1, 2);
	} VERIFY_SUCCEEDED(vkEndCommandBuffer(CB));
	VK::SubmitAndWait(GraphicsQueue, CB);
}

void VK::CreateRenderPass(VkRenderPass& RP, const std::vector<VkAttachmentDescription>& ADs, const std::vector<VkSubpassDescription>& SDs, const std::vector<VkSubpassDependency>& Deps)
{
	const VkRenderPassCreateInfo RPCI = {
		.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
		.attachmentCount = static_cast<uint32_t>(std::size(ADs)), .pAttachments = std::data(ADs),
		.subpassCount = static_cast<uint32_t>(std::size(SDs)), .pSubpasses = std::data(SDs),
		.dependencyCount = static_cast<uint32_t>(std::size(Deps)), .pDependencies = std::data(Deps)
	};
	VERIFY_SUCCEEDED(vkCreateRenderPass(Device, &RPCI, GetAllocationCallbacks(), &RP));
}
//!< デフォルト実装は何もしないものにしている (全画面を描画するような、クリアを必要としないケース)
//void VK::CreateRenderPass()
//{
//	//!< インプットアタッチメント (InputAttachment)
//	constexpr std::array<VkAttachmentReference, 0> IAs = {};
//	//!< カラーアタッチメント (ColorAttachment)
//	constexpr std::array CAs = { VkAttachmentReference({.attachment = 0, .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL }), };
//	//!< リゾルブアタッチメント (ResolveAttachment) : マルチサンプル → シングルサンプルへリゾルブするような場合
//	constexpr std::array RAs = { VkAttachmentReference({.attachment = VK_ATTACHMENT_UNUSED, .layout = VK_IMAGE_LAYOUT_UNDEFINED }), };
//	VERIFY(size(CAs) == size(RAs));
//	//!< プリザーブアタッチメント (PreserveAttachment) : サブパス全体において保持しなくてはならないコンテンツのインデックス
//	constexpr std::array<uint32_t, 0> PAs = {};
//
//	CreateRenderPass(RenderPasses.emplace_back(), {
//		VkAttachmentDescription({
//			.flags = 0,
//			.format = ColorFormat,
//			.samples = VK_SAMPLE_COUNT_1_BIT,
//			.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE, .storeOp = VK_ATTACHMENT_STORE_OP_STORE,						//!< 「開始時に何もしない」「終了時に保存」
//			.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE, .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,	//!< ステンシルのロードストア : (ここでは)開始時、終了時ともに「使用しない」
//			.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED, .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR				//!< レンダーパスのレイアウト : 「開始時未定義」「終了時プレゼンテーションソース」
//		}),
//	}, {
//		VkSubpassDescription({
//			.flags = 0,
//			.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
//			.inputAttachmentCount = static_cast<uint32_t>(size(IAs)), .pInputAttachments = data(IAs), 									//!< インプットアタッチメント(読み取り用) シェーダ内で次のように使用 layout (input_attachment_index=0, set=0, binding=0) uniform XXX YYY;
//			.colorAttachmentCount = static_cast<uint32_t>(size(CAs)), .pColorAttachments = data(CAs), .pResolveAttachments = data(RAs),	//!< カラーアタッチメント(書き込み用)、リゾルブアタッチメント(マルチサンプルのリゾルブ)
//			.pDepthStencilAttachment = nullptr,																							//!< デプスアタッチメント(書き込み用)
//			.preserveAttachmentCount = static_cast<uint32_t>(size(PAs)), .pPreserveAttachments = data(PAs)								//!< プリザーブアタッチメント(サブパス全体において保持するコンテンツのインデックス)
//		}),
//	}, {
//#if 1
//		//!< サブパス依存 (敢えて書く場合)
//		VkSubpassDependency({
//			.srcSubpass = VK_SUBPASS_EXTERNAL, .dstSubpass = 0,																		//!< サブパス外からサブパス0へ
//			.srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,	//!< パイプラインの最終ステージからカラー出力ステージへ
//			.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT, .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,						//!< 読み込みからカラー書き込みへ
//			.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT,																			//!< 同じメモリ領域に対する書き込みが完了してから読み込み (指定しない場合は自前で書き込み完了を管理)
//		}),
//		VkSubpassDependency({
//			.srcSubpass = 0, .dstSubpass = VK_SUBPASS_EXTERNAL,																		//!< サブパス0からサブパス外へ
//			.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, .dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,	//!< カラー出力ステージからパイプラインの最終ステージへ
//			.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, .dstAccessMask = VK_ACCESS_MEMORY_READ_BIT,						//!< カラー書き込みから読み込みへ
//			.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT,
//		}),
//#endif
//	});
//}

void VK::CreateFramebuffer(VkFramebuffer& FB, const VkRenderPass RP, const uint32_t Width, const uint32_t Height, const uint32_t Layers, const std::vector<VkImageView>& IVs)
{
	const VkFramebufferCreateInfo FCI = {
		.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
		.renderPass = RP, //!< ここで指定するレンダーパスは「互換性のあるもの」なら可
		.attachmentCount = static_cast<uint32_t>(std::size(IVs)), .pAttachments = std::data(IVs),
		.width = Width, .height = Height,
		.layers = Layers
	};
	VERIFY_SUCCEEDED(vkCreateFramebuffer(Device, &FCI, GetAllocationCallbacks(), &FB));
}

/**
@brief シェーダコンパイル、リンクはパイプラインオブジェクト作成時に行われる Shader compilation and linkage is performed during the pipeline object creation
*/
VkShaderModule VK::CreateShaderModule(const std::filesystem::path& Path) const
{
	VkShaderModule SM = VK_NULL_HANDLE;
	std::ifstream In(data(Path.string()), std::ios::in | std::ios::binary);
	if (!In.fail()) {
		In.seekg(0, std::ios_base::end);
		const auto Size = In.tellg();
		if (Size) {
			In.seekg(0, std::ios_base::beg);
			std::vector<std::byte> Code(Size);
			In.read(reinterpret_cast<char*>(std::data(Code)), std::size(Code));
			const VkShaderModuleCreateInfo SMCI = {
				.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
				.pNext = nullptr,
				.flags = 0,
				.codeSize = static_cast<size_t>(std::size(Code)), .pCode = reinterpret_cast<uint32_t*>(std::data(Code))
			};
			VERIFY_SUCCEEDED(vkCreateShaderModule(Device, &SMCI, GetAllocationCallbacks(), &SM));
		}
		In.close();
	}
	return SM;
}

void VK::CreatePipelineVsFsTesTcsGs(VkPipeline& PL,
	const VkDevice Dev, 
	const VkPipelineLayout PLL,
	const VkRenderPass RP, 
	const VkPrimitiveTopology PT, const uint32_t PatchControlPoints, 
	const VkPipelineRasterizationStateCreateInfo& PRSCI,
	const VkPipelineDepthStencilStateCreateInfo& PDSSCI,
	const VkPipelineShaderStageCreateInfo* VS, const VkPipelineShaderStageCreateInfo* FS, const VkPipelineShaderStageCreateInfo* TES, const VkPipelineShaderStageCreateInfo* TCS, const VkPipelineShaderStageCreateInfo* GS,
	const std::vector<VkVertexInputBindingDescription>& VIBDs, const std::vector<VkVertexInputAttributeDescription>& VIADs,
	const std::vector<VkPipelineColorBlendAttachmentState>& PCBASs,
	VkPipelineCache PC)
{
	PERFORMANCE_COUNTER_FUNC();
	
	//!< シェーダステージ (ShaderStage)
	std::vector<VkPipelineShaderStageCreateInfo> PSSCIs;
	if (nullptr != VS) { PSSCIs.emplace_back(*VS); }
	if (nullptr != FS) { PSSCIs.emplace_back(*FS); }
	if (nullptr != TES) { PSSCIs.emplace_back(*TES); }
	if (nullptr != TCS) { PSSCIs.emplace_back(*TCS); }
	if (nullptr != GS) { PSSCIs.emplace_back(*GS); }
	VERIFY(!empty(PSSCIs));

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
		.topology = PT,
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
	VERIFY((
		(PIASCI.topology != VK_PRIMITIVE_TOPOLOGY_POINT_LIST || PIASCI.topology != VK_PRIMITIVE_TOPOLOGY_LINE_LIST || PIASCI.topology != VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST || PIASCI.topology != VK_PRIMITIVE_TOPOLOGY_LINE_LIST_WITH_ADJACENCY || PIASCI.topology != VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST_WITH_ADJACENCY || PIASCI.topology != VK_PRIMITIVE_TOPOLOGY_PATCH_LIST)
		|| PIASCI.primitiveRestartEnable == VK_FALSE));

	//!< テセレーション (Tessellation)
	VERIFY((PT != VK_PRIMITIVE_TOPOLOGY_PATCH_LIST || PatchControlPoints != 0));
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
	//assert(PRSCI.polygonMode == VK_POLYGON_MODE_FILL || PDF.fillModeNonSolid && "");
	//!< 1.0f より大きな値には、デバイスフィーチャーwidelines が有効であること
	//assert(PRSCI.lineWidth <= 1.0f || PDF.wideLines && "");

	//!< マルチサンプル (Multisample)
	constexpr VkSampleMask SM = 0xffffffff; //!< 0xffffffff を指定する場合は、代わりに nullptr でもよい
	const VkPipelineMultisampleStateCreateInfo PMSCI = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
		.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
		.sampleShadingEnable = VK_FALSE, .minSampleShading = 0.0f,
		.pSampleMask = &SM,
		.alphaToCoverageEnable = VK_FALSE, .alphaToOneEnable = VK_FALSE
	};
	//assert((PMSCI.sampleShadingEnable == VK_FALSE || PDF.sampleRateShading) && "");
	VERIFY((PMSCI.minSampleShading >= 0.0f && PMSCI.minSampleShading <= 1.0f));
	//assert((PMSCI.alphaToOneEnable == VK_FALSE || PDF.alphaToOne) && "");

	//!< カラーブレンド (ColorBlend)
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
	const VkPipelineDynamicStateCreateInfo PDSCI = {
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
#ifdef USE_DYNAMIC_RENDERING
	constexpr std::array ColorAttachments = { VK_FORMAT_B8G8R8A8_UNORM };
	const VkPipelineRenderingCreateInfo PRCI = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
		.pNext = nullptr,
		.viewMask = 0,
		.colorAttachmentCount = static_cast<uint32_t>(std::size(ColorAttachments)), .pColorAttachmentFormats = std::data(ColorAttachments),
		.depthAttachmentFormat = VK_FORMAT_D24_UNORM_S8_UINT,
		.stencilAttachmentFormat = VK_FORMAT_UNDEFINED,
	};
#endif
	const std::array GPCIs = {
		VkGraphicsPipelineCreateInfo({
			.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
#ifdef USE_DYNAMIC_RENDERING
			.pNext = &PRCI,
#else
			.pNext = nullptr,
#endif
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
	VERIFY_SUCCEEDED(vkCreateGraphicsPipelines(Dev, PC, static_cast<uint32_t>(std::size(GPCIs)), std::data(GPCIs), GetAllocationCallbacks(), &PL));

	LOG_OK();
}
void VK::CreatePipelineTsMsFs(VkPipeline& PL,
	const VkDevice Dev, 
	const VkPipelineLayout PLL,
	const VkRenderPass RP, 
	const VkPipelineRasterizationStateCreateInfo& PRSCI, 
	const VkPipelineDepthStencilStateCreateInfo& PDSSCI, 
	const VkPipelineShaderStageCreateInfo* TS, const VkPipelineShaderStageCreateInfo* MS, const VkPipelineShaderStageCreateInfo* FS, 
	const std::vector<VkPipelineColorBlendAttachmentState>& PCBASs, 
	VkPipelineCache PC)
{
	PERFORMANCE_COUNTER_FUNC();

	//!< シェーダステージ (ShaderStage)
	std::vector<VkPipelineShaderStageCreateInfo> PSSCIs;
	if (nullptr != TS) { PSSCIs.emplace_back(*TS); }
	if (nullptr != MS) { PSSCIs.emplace_back(*MS); }
	if (nullptr != FS) { PSSCIs.emplace_back(*FS); }
	VERIFY(!empty(PSSCIs));

	//!< ビューポート (Viewport)
	constexpr VkPipelineViewportStateCreateInfo PVSCI = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
		.viewportCount = 1, .pViewports = nullptr,
		.scissorCount = 1, .pScissors = nullptr
	};

	//!< マルチサンプル (Multisample)
	constexpr VkSampleMask SM = 0xffffffff;
	const VkPipelineMultisampleStateCreateInfo PMSCI = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
		.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
		.sampleShadingEnable = VK_FALSE, .minSampleShading = 0.0f,
		.pSampleMask = &SM,
		.alphaToCoverageEnable = VK_FALSE, .alphaToOneEnable = VK_FALSE
	};
	
	//!< カラーブレンド (ColorBlend)
	const VkPipelineColorBlendStateCreateInfo PCBSCI = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
		.logicOpEnable = VK_FALSE, .logicOp = VK_LOGIC_OP_COPY, 
		.attachmentCount = static_cast<uint32_t>(size(PCBASs)), .pAttachments = data(PCBASs),
		.blendConstants = { 1.0f, 1.0f, 1.0f, 1.0f }
	};

	//!< ダイナミックステート (DynamicState)
	constexpr std::array DSs = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR, };
	const VkPipelineDynamicStateCreateInfo PDSCI = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
		.dynamicStateCount = static_cast<uint32_t>(size(DSs)), .pDynamicStates = data(DSs)
	};

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
			.pVertexInputState = nullptr,
			.pInputAssemblyState = nullptr,//&PIASCI,
			.pTessellationState = nullptr,
			.pViewportState = &PVSCI,
			.pRasterizationState = &PRSCI,
			.pMultisampleState = &PMSCI,
			.pDepthStencilState = &PDSSCI,
			.pColorBlendState = &PCBSCI,
			.pDynamicState = &PDSCI,
			.layout = PLL,
			.renderPass = RP, .subpass = 0, 
			.basePipelineHandle = VK_NULL_HANDLE, .basePipelineIndex = -1
		})
	};
	VERIFY_SUCCEEDED(vkCreateGraphicsPipelines(Dev, PC, static_cast<uint32_t>(size(GPCIs)), data(GPCIs), GetAllocationCallbacks(), &PL));
	LOG_OK();
}

void VK::WaitForFence(VkDevice Device, VkFence Fence)
{
	const std::array Fences = { Fence };

	//!< https://arm-software.github.io/vulkan_best_practice_for_mobile_developers/docs/faq.html#debugging-a-device_lost 
	//!< I’m getting a DEVICE_LOST error when calling either vkQueueSubmit or vkWaitForFences. Validation is clean. Why could that be? There are two main reasons why a DEVICE_LOST might arise:
	//!<	Out of memory(OOM)
	//!<	Resource corruption
	//!<  missing synchronization does not necessarily result in a lost device.
	VERIFY_SUCCEEDED(vkWaitForFences(Device, static_cast<uint32_t>(std::size(Fences)), std::data(Fences), VK_TRUE, (std::numeric_limits<uint64_t>::max)()));
	vkResetFences(Device, static_cast<uint32_t>(std::size(Fences)), std::data(Fences));
}
void VK::SubmitGraphics(const uint32_t i)
{
	const auto& CB = CommandBuffers[i];

	const std::array WaitSSIs = {
	VkSemaphoreSubmitInfo({
		.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
		.pNext = nullptr,
		.semaphore = NextImageAcquiredSemaphores[i],
		.value = 0,
		.stageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT,
		.deviceIndex = 0
		})
	};
	const std::array CBSIs = {
		VkCommandBufferSubmitInfo({
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO,
			.pNext = nullptr,
			.commandBuffer = CB,
			.deviceMask = 0
			})
	};
	const std::array SignalSSIs = {
		VkSemaphoreSubmitInfo({
			.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
			.pNext = nullptr,
			.semaphore = RenderFinishedSemaphores[i],
			.value = 0,
			.stageMask = VK_PIPELINE_STAGE_2_NONE,
			.deviceIndex = 0
			})
	};
	const std::array SIs = {
		VkSubmitInfo2({
			.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2,
			.pNext = nullptr,
			.flags = 0,
			.waitSemaphoreInfoCount = static_cast<uint32_t>(std::size(WaitSSIs)), .pWaitSemaphoreInfos = std::data(WaitSSIs),
			.commandBufferInfoCount = static_cast<uint32_t>(std::size(CBSIs)), .pCommandBufferInfos = std::data(CBSIs),
			.signalSemaphoreInfoCount = static_cast<uint32_t>(std::size(SignalSSIs)), .pSignalSemaphoreInfos = std::data(SignalSSIs)
		})
	};
	VERIFY_SUCCEEDED(vkQueueSubmit2(GraphicsQueue, static_cast<uint32_t>(std::size(SIs)), std::data(SIs), GraphicsFence));
}
void VK::SubmitCompute(const uint32_t i)
{
	const auto& CB = ComputeCommandBuffers[i];

	const std::array<VkSemaphoreSubmitInfo, 0> WaitSSIs = {};
	const std::array CBSIs = {
		VkCommandBufferSubmitInfo({
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO,
			.pNext = nullptr,
			.commandBuffer = CB,
			.deviceMask = 0
			})
	};
	const std::array SignalSSIs = {
		VkSemaphoreSubmitInfo({
			.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
			.pNext = nullptr,
			.semaphore = ComputeSemaphore,
			.value = 0,
			.stageMask = VK_PIPELINE_STAGE_2_NONE,
			.deviceIndex = 0
			})
	};
	const std::array SIs = {
		VkSubmitInfo2({
			.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2,
			.pNext = nullptr,
			.flags = 0,
			.waitSemaphoreInfoCount = static_cast<uint32_t>(std::size(WaitSSIs)), .pWaitSemaphoreInfos = std::data(WaitSSIs),
			.commandBufferInfoCount = static_cast<uint32_t>(std::size(CBSIs)), .pCommandBufferInfos = std::data(CBSIs),
			.signalSemaphoreInfoCount = static_cast<uint32_t>(std::size(SignalSSIs)), .pSignalSemaphoreInfos = std::data(SignalSSIs)
		})
	};
	VERIFY_SUCCEEDED(vkQueueSubmit2(ComputeQueue, static_cast<uint32_t>(std::size(SIs)), std::data(SIs), ComputeFence));
}
void VK::Present()
{
	const std::array WaitSems = { RenderFinishedSemaphores[GetCurrentBackBufferIndex()]};
	//!< 同時に複数のプレゼントが可能だが、1つのスワップチェインからは1つのみ
	const std::array Swapchains = { Swapchain.VkSwapchain };
	const std::array ImageIndices = { GetCurrentBackBufferIndex() };
	VERIFY(size(Swapchains) == size(ImageIndices));

	//!< サブミット時に指定したセマフォ(RenderFinishedSemaphore)を待ってからプレゼントが行なわれる
	const VkPresentInfoKHR PI = {
		.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
		.pNext = nullptr,
		.waitSemaphoreCount = static_cast<uint32_t>(std::size(WaitSems)), .pWaitSemaphores = std::data(WaitSems),
		.swapchainCount = static_cast<uint32_t>(std::size(Swapchains)), .pSwapchains = std::data(Swapchains), .pImageIndices = std::data(ImageIndices),
		.pResults = nullptr
	};
	VERIFY_SUCCEEDED(vkQueuePresentKHR(PresentQueue, &PI));
}
void VK::Draw()
{
	WaitForFence(Device, GraphicsFence);

	//!< 次のイメージが取得できるまでブロック(タイムアウトは指定可能)、取得できたからといってイメージは直ぐに目的に使用可能とは限らない
	//!< 使用可能になるとフェンスやセマフォがシグナルされる (シグナルするように指定した場合)
	//!< ここではセマフォを指定し、このセマフォはサブミット時に使用する(サブミットしたコマンドがプレゼンテーションを待つように指示している)
	//!<	VK_SUBOPTIMAL_KHR : イメージは使用可能ではあるがプレゼンテーションエンジンにとってベストではない状態
	//!<	VK_ERROR_OUT_OF_DATE_KHR : イメージは使用不可で再作成が必要
	const auto Index = (Swapchain.Index + 1) % std::size(NextImageAcquiredSemaphores); //!< このインデックスは vkAcquireNextImageKHR() で返るであろうインデックスを先取りした値
	VERIFY_SUCCEEDED(vkAcquireNextImageKHR(Device, Swapchain.VkSwapchain, (std::numeric_limits<uint64_t>::max)(), NextImageAcquiredSemaphores[Index], VK_NULL_HANDLE, &Swapchain.Index));

	//!< ユニフォームバッファの更新等
	OnUpdate(Swapchain.Index);
	
	SubmitGraphics(Swapchain.Index);
	
	Present();
}
void VK::Dispatch()
{
	WaitForFence(Device, ComputeFence);

	SubmitCompute(0);
}
