#pragma once

#include <fstream>
#include <memory>
#include <map>

//!< VK_NO_PROTOYYPES(VK.props) が定義されてる場合は DLL を使用する。(If VK_NO_PROTOYYPES is defined, using DLL)

#ifdef _WINDOWS
#define VK_USE_PLATFORM_WIN32_KHR
#endif
#pragma warning(push)
//#pragma warning(disable : )
#include <vulkan/vulkan.h>
#pragma warning(pop)

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#pragma warning(push)
#pragma warning(disable : 4201)
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>
//#define GLM_ENABLE_EXPERIMENTAL
//#include <glm/gtx/quaternion.hpp>
#pragma warning(pop)

#ifndef VERIFY_SUCCEEDED
#ifdef _DEBUG
//#define VERIFY_SUCCEEDED(X) { const auto VR = (X); if(FAILED(VR)) { OutputDebugStringA(data(std::string(VK::GetVkResultChar(VR)) + "\n")); DEBUG_BREAK(); } }
#define VERIFY_SUCCEEDED(X) { const auto VR = (X); if(FAILED(VR)) { MessageBoxA(nullptr, VK::GetVkResultChar(VR), "", MB_OK); DEBUG_BREAK(); } }
#else
#define VERIFY_SUCCEEDED(X) (X)
#endif
#endif

#define USE_VIEWPORT_Y_UP //!< *VK
#define USE_IMMUTABLE_SAMPLER //!< [ TextureVK ] DX:USE_STATIC_SAMPLER相当

//!< セカンダリコマンドバッファ : DXのバンドル相当
//!< 基本的にセカンダリはプライマリのステートを継承しない
//!< ただしプライマリがレンダーパス内からセカンダリを呼び出す場合には、プライマリのレンダーパス、サプバスステートは継承される
//!< 全てのコマンドがプライマリ、セカンダリの両方で記録できるわけではない
//!< セカンダリの場合は VK_SUBPASS_CONTENTS_INLINE の代わりに VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS を指定する 
//#define USE_SECONDARY_COMMAND_BUFFER //!< [ ParametricSurfaceVK ] DX::USE_BUNDLE相当

//!< パイプライン作成時にシェーダ内の定数値を上書き指定できる(スカラ値のみ)
//#define USE_SPECIALIZATION_INFO //!< [ ParametricSurfaceVK ]

//!< プッシュデスクリプタ : デスクリプタセットを確保してからコマンドバッファにバインドするのではなく、デスクリプタの更新自体をコマンドバッファに記録してしまう
//#define USE_PUSH_DESCRIPTOR //!< [ BillboardVK ]
//#define USE_PUSH_CONSTANTS //!< [ TriangleVK ] DX:USE_ROOT_CONSTANTS相当
//#define USE_UPDATE_DESCRIPTOR_SET_WITH_TEMPLATE //!< [ DisplacementVK ]
//#define USE_MANUAL_CLEAR //!< [ ClearVK ]

#define USE_SEPARATE_SAMPLER //!< [ NormalMapVK ] コンバインドイメージサンプラを使用しない

//!< 参考)https://www.saschawillems.de/blog/2018/07/19/vulkan-input-attachments-and-sub-passes/
//!< あるサブパスで書き込まれたフレームバッファアタッチメントに対して、別のサブパスで「同一のピクセル」を読み込むことができる(ただし「周辺のピクセル」は読めないので用途は要考慮)
//!< 各サブパスのアタッチメントは１つのフレームバッファにまとめてエントリする
//!< VkRenderPassBeginInfoの引数として渡すため、同一パス(サブパス)で完結するにはまとめる必要がある
//!< VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENTを指定する
//!< vkCmdNextSubpass(でコマンドを次のサブパスへ進める
//!< シェーダ内での使用例
//!<	layout(input_attachment_index = 0, set = 0, binding = 0) uniform subpassInput XXX;
//!<	vec3 Color = subpassLoad(XXX).rgb;
//#define USE_SUBPASS #VK_TODO

#ifdef _DEBUG
#define USE_DEBUG_UTILS
#endif

//!< RenderDoc を別途インストールしておくこと
//#define USE_RENDERDOC

//#define USE_DYNAMIC_RENDERING
//#define USE_TIMELINESEMAPHORE
#define USE_SYNCHRONIZATION2

#include "Cmn.h"
#ifdef _WINDOWS
#include "Win.h"
#endif

#include "VKFeature.h"

namespace Colors
{
	constexpr VkClearColorValue Black = { 0.0f, 0.0f, 0.0f, 1.0f };
	constexpr VkClearColorValue Blue = { 0.0f, 0.0f, 1.0f, 1.0f };
	constexpr VkClearColorValue Brown = { 0.647058845f, 0.164705887f, 0.164705887f, 1.0f };
	constexpr VkClearColorValue Gray = { 0.501960814f, 0.501960814f, 0.501960814f, 1.0f };
	constexpr VkClearColorValue Green = { 0.0f, 0.501960814f, 0.0f, 1.0f };
	constexpr VkClearColorValue Magenta = { 1.0f, 0.0f, 1.0f, 1.0f };
	constexpr VkClearColorValue Orange = { 1.0f, 0.647058845f, 0.0f, 1.0f };
	constexpr VkClearColorValue Pink = { 1.0f, 0.752941251f, 0.796078503f, 1.0f };
	constexpr VkClearColorValue Purple = { 0.501960814f, 0.0f, 0.501960814f, 1.0f };
	constexpr VkClearColorValue Red = { 1.0f, 0.0f, 0.0f, 1.0f };
	constexpr VkClearColorValue SkyBlue = { 0.529411793f, 0.807843208f, 0.921568692f, 1.0f };
	constexpr VkClearColorValue Transparent = { 0.0f, 0.0f, 0.0f, 0.0f };
	constexpr VkClearColorValue White = { 1.0f, 1.0f, 1.0f, 1.0f };
	constexpr VkClearColorValue Yellow = { 1.0f, 1.0f, 0.0f, 1.0f };
}

class VK : public Cmn
#ifdef _WINDOWS
	, public Win
#endif
{
private:
	using Super = Win;

public:
	static const glm::vec3 Min(const glm::vec3& lhs, const glm::vec3& rhs) { return glm::vec3((std::min)(lhs.x, rhs.x), (std::min)(lhs.y, rhs.y), (std::min)(lhs.z, rhs.z)); }
	static const glm::vec3 Max(const glm::vec3& lhs, const glm::vec3& rhs) { return glm::vec3((std::max)(lhs.x, rhs.x), (std::max)(lhs.y, rhs.y), (std::max)(lhs.z, rhs.z)); }

	template<typename T> class Scoped : public T
	{
	public:
		Scoped(VkDevice Dev) : T(), Device(Dev) {}
		virtual ~Scoped() { if (VK_NULL_HANDLE != Device) { T::Destroy(Device); } }
	private:
		VkDevice Device = VK_NULL_HANDLE;
	};
	class DeviceMemoryBase
	{
	public:
		VkDeviceMemory DeviceMemory = VK_NULL_HANDLE;
		virtual void Destroy(const VkDevice Device) {
			if (VK_NULL_HANDLE != DeviceMemory) { vkFreeMemory(Device, DeviceMemory, GetAllocationCallbacks()); }
		}
	};
	class BufferMemory : public DeviceMemoryBase
	{
	private:
		using Super = DeviceMemoryBase;
	public:
		VkBuffer Buffer = VK_NULL_HANDLE;
		BufferMemory& Create(const VkDevice Device, const VkPhysicalDeviceMemoryProperties PDMP, const size_t Size, const VkBufferUsageFlags BUF, const VkMemoryPropertyFlags MPF, const void* Source = nullptr) {
			VK::CreateBufferMemory(&Buffer, &DeviceMemory, Device, PDMP, Size, BUF, MPF, Source);
			return *this;
		}
		virtual void Destroy(const VkDevice Device) override {
			Super::Destroy(Device);
			if (VK_NULL_HANDLE != Buffer) { vkDestroyBuffer(Device, Buffer, GetAllocationCallbacks()); }
		}
	};
	class DeviceLocalBuffer : public BufferMemory
	{
	private:
		using Super = BufferMemory;
	public:
		DeviceLocalBuffer& Create(const VkDevice Device, const VkPhysicalDeviceMemoryProperties PDMP, const size_t Size, const VkBufferUsageFlags BUF) {
			Super::Create(Device, PDMP, Size, BUF, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
			return *this;
		}
		void PopulateCopyCommand(const VkCommandBuffer CB, const size_t Size, const VkBuffer Staging, const VkAccessFlags AF, const VkPipelineStageFlagBits PSF) {
			BufferMemoryBarrier(CB,
				VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
				Buffer,
				0, VK_ACCESS_MEMORY_WRITE_BIT);
			{
				const std::array BCs = { VkBufferCopy({.srcOffset = 0, .dstOffset = 0, .size = Size }), };
				vkCmdCopyBuffer(CB, Staging, Buffer, static_cast<uint32_t>(std::size(BCs)), std::data(BCs));
			}
			BufferMemoryBarrier(CB,
				VK_PIPELINE_STAGE_TRANSFER_BIT, PSF,
				Buffer,
				VK_ACCESS_MEMORY_WRITE_BIT, AF);
		}
		void SubmitCopyCommand(const VkDevice Device, const VkPhysicalDeviceMemoryProperties PDMP, const VkCommandBuffer CB, const VkQueue Queue, const size_t Size, const void* Source, const VkAccessFlags AF, const VkPipelineStageFlagBits PSF) {
			VK::Scoped<StagingBuffer> Staging(Device);
			Staging.Create(Device, PDMP, Size, Source);
			constexpr VkCommandBufferBeginInfo CBBI = { .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO, .pNext = nullptr, .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, .pInheritanceInfo = nullptr };
			VERIFY_SUCCEEDED(vkBeginCommandBuffer(CB, &CBBI)); {
				PopulateCopyCommand(CB, Size, Staging.Buffer, AF, PSF);
			} VERIFY_SUCCEEDED(vkEndCommandBuffer(CB));
			VK::SubmitAndWait(Queue, CB);
		}
	};
	class HostVisibleBuffer : public BufferMemory
	{
	private:
		using Super = BufferMemory;
	public:
		HostVisibleBuffer& Create(const VkDevice Device, const VkPhysicalDeviceMemoryProperties PDMP, const size_t Size, const VkBufferUsageFlags BUF, const void* Source = nullptr) {
			Super::Create(Device, PDMP, Size, BUF, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT/*| VK_MEMORY_PROPERTY_HOST_COHERENT_BIT*/, Source);
			return *this;
		}
	};
	class StagingBuffer : public HostVisibleBuffer
	{
	private:
		using Super = HostVisibleBuffer;
	public:
		StagingBuffer& Create(const VkDevice Device, const VkPhysicalDeviceMemoryProperties PDMP, const size_t Size, const void* Source = nullptr) {
			Super::Create(Device, PDMP, Size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, Source);
			return *this;
		}
	};
	class VertexBuffer : public DeviceLocalBuffer
	{
	private:
		using Super = DeviceLocalBuffer;
	public:
		VertexBuffer& Create(const VkDevice Device, const VkPhysicalDeviceMemoryProperties PDMP, const size_t Size) {
			Super::Create(Device, PDMP, Size, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
			return *this;
		}
		void PopulateCopyCommand(const VkCommandBuffer CB, const size_t Size, const VkBuffer Staging) {
			Super::PopulateCopyCommand(CB, Size, Staging, VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT, VK_PIPELINE_STAGE_VERTEX_INPUT_BIT);
		}
		void SubmitCopyCommand(const VkDevice Device, const VkPhysicalDeviceMemoryProperties PDMP, const VkCommandBuffer CB, const VkQueue Queue, const size_t Size, const void* Source) {
			Super::SubmitCopyCommand(Device, PDMP, CB, Queue, Size, Source, VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT, VK_PIPELINE_STAGE_VERTEX_INPUT_BIT);
		}
	};
	class IndexBuffer : public DeviceLocalBuffer
	{
	private:
		using Super = DeviceLocalBuffer;
	public:
		IndexBuffer& Create(const VkDevice Device, const VkPhysicalDeviceMemoryProperties PDMP, const size_t Size) {
			Super::Create(Device, PDMP, Size, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
			return *this;
		}
		void PopulateCopyCommand(const VkCommandBuffer CB, const size_t Size, const VkBuffer Staging) {
			Super::PopulateCopyCommand(CB, Size, Staging, VK_ACCESS_INDEX_READ_BIT, VK_PIPELINE_STAGE_VERTEX_INPUT_BIT);
		}
		void SubmitCopyCommand(const VkDevice Device, const VkPhysicalDeviceMemoryProperties PDMP, const VkCommandBuffer CB, const VkQueue Queue, const size_t Size, const void* Source) {
			Super::SubmitCopyCommand(Device, PDMP, CB, Queue, Size, Source, VK_ACCESS_INDEX_READ_BIT, VK_PIPELINE_STAGE_VERTEX_INPUT_BIT);
		}
	};
	class IndirectBuffer : public DeviceLocalBuffer
	{
	private:
		using Super = DeviceLocalBuffer;
	protected:
		IndirectBuffer& Create(const VkDevice Device, const VkPhysicalDeviceMemoryProperties PDMP, const size_t Size) {
			Super::Create(Device, PDMP, Size, VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
			return *this;
		}
	public:
		IndirectBuffer& Create(const VkDevice Device, const VkPhysicalDeviceMemoryProperties PDMP, const VkDrawIndexedIndirectCommand& DIIC) { return Create(Device, PDMP, sizeof(DIIC)); }
		IndirectBuffer& Create(const VkDevice Device, const VkPhysicalDeviceMemoryProperties PDMP, const VkDrawIndirectCommand& DIC) { return Create(Device, PDMP, sizeof(DIC)); }
		IndirectBuffer& Create(const VkDevice Device, const VkPhysicalDeviceMemoryProperties PDMP, const VkDispatchIndirectCommand& DIC) { return Create(Device, PDMP, sizeof(DIC)); }
#pragma region MESH_SHADER
		IndirectBuffer& Create(const VkDevice Device, const VkPhysicalDeviceMemoryProperties PDMP, const VkDrawMeshTasksIndirectCommandNV& DMTIC) { return Create(Device, PDMP, sizeof(DMTIC)); }
		IndirectBuffer& Create(const VkDevice Device, const VkPhysicalDeviceMemoryProperties PDMP, const VkDrawMeshTasksIndirectCommandEXT& DMTIC) { return Create(Device, PDMP, sizeof(DMTIC)); }
#pragma endregion
#pragma region RAYTRACING
		IndirectBuffer& Create(const VkDevice Device, const VkPhysicalDeviceMemoryProperties PDMP, const VkTraceRaysIndirectCommandKHR& TRIC) {
			Super::Create(Device, PDMP, sizeof(TRIC), VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT);
			return *this;
		}
#pragma endregion
		void PopulateCopyCommand(const VkCommandBuffer CB, const size_t Size, const VkBuffer Staging) {
			Super::PopulateCopyCommand(CB, Size, Staging, VK_ACCESS_INDIRECT_COMMAND_READ_BIT, VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT);
		}
		void SubmitCopyCommand(const VkDevice Device, const VkPhysicalDeviceMemoryProperties PDMP, const VkCommandBuffer CB, const VkQueue Queue, const size_t Size, const void* Source) {
			Super::SubmitCopyCommand(Device, PDMP, CB, Queue, Size, Source, VK_ACCESS_INDIRECT_COMMAND_READ_BIT, VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT);
		}
	};
	class UniformBuffer : public BufferMemory
	{
	private:
		using Super = BufferMemory;
	public:
		UniformBuffer& Create(const VkDevice Device, const VkPhysicalDeviceMemoryProperties PDMP, const size_t Size, const void* Source = nullptr) {
			VK::CreateBufferMemory(&Buffer, &DeviceMemory, Device, PDMP, Size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, Source);
			return *this;
		}
	};

	class DeviceLocalStorageBuffer : public DeviceLocalBuffer
	{
	private:
		using Super = DeviceLocalBuffer;
	public:
		VkBufferView View = VK_NULL_HANDLE;
		DeviceLocalStorageBuffer& Create(const VkDevice Device, const VkPhysicalDeviceMemoryProperties PDMP, const size_t Size) {
			Super::Create(Device, PDMP, Size, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
			return *this;
		}
		DeviceLocalStorageBuffer& Create(const VkDevice Device, const VkPhysicalDeviceMemoryProperties PDMP, const size_t Size, const VkFormat Format) {
			Create(Device, PDMP, Size);
			const VkBufferViewCreateInfo BVCI = {
				.sType = VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO,
				.pNext = nullptr,
				.flags = 0,
				.buffer = Buffer,
				.format = Format,
				.offset = 0,
				.range = VK_WHOLE_SIZE
			};
			VERIFY_SUCCEEDED(vkCreateBufferView(Device, &BVCI, GetAllocationCallbacks(), &View));
			return *this;
		}
		virtual void Destroy(const VkDevice Device) override {
			Super::Destroy(Device);
			if (VK_NULL_HANDLE != View) { vkDestroyBufferView(Device, View, GetAllocationCallbacks()); }
		}
	};
	//class HostVisibleStorageBuffer : public HostVisibleBuffer
	//{
	//private:
	//	using Super = HostVisibleBuffer;
	//public:
	//	HostVisibleStorageBuffer& Create(const VkDevice Device, const VkPhysicalDeviceMemoryProperties PDMP, const size_t Size, const void* Source = nullptr) {
	//		Super::Create(Device, PDMP, Size, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, Source);
	//		return *this;
	//	}
	//};

	class DeviceLocalUniformTexelBuffer : public DeviceLocalBuffer
	{
	private:
		using Super = DeviceLocalBuffer;
	public:
		VkBufferView View = VK_NULL_HANDLE;
		DeviceLocalUniformTexelBuffer& Create(const VkDevice Device, const VkPhysicalDeviceMemoryProperties PDMP, const size_t Size, const VkBufferUsageFlagBits AdditionalUsage = static_cast<VkBufferUsageFlagBits>(0)) {
			Super::Create(Device, PDMP, Size, VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | AdditionalUsage);
			return *this;
		}
		DeviceLocalUniformTexelBuffer& Create(const VkDevice Device, const VkPhysicalDeviceMemoryProperties PDMP, const size_t Size, const VkFormat Format, const VkBufferUsageFlagBits AdditionalUsage = static_cast<VkBufferUsageFlagBits>(0)) {
			Create(Device, PDMP, Size, AdditionalUsage);
			const VkBufferViewCreateInfo BVCI = {
				.sType = VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO,
				.pNext = nullptr,
				.flags = 0,
				.buffer = Buffer,
				.format = Format,
				.offset = 0,
				.range = VK_WHOLE_SIZE
			};
			VERIFY_SUCCEEDED(vkCreateBufferView(Device, &BVCI, GetAllocationCallbacks(), &View));
			return *this;
		}
		virtual void Destroy(const VkDevice Device) override {
			Super::Destroy(Device);
			if (VK_NULL_HANDLE != View) { vkDestroyBufferView(Device, View, GetAllocationCallbacks()); }
		}
	};
	//class HostVisibleUniformTexelBuffer : public HostVisibleBuffer
	//{
	//private:
	//	using Super = HostVisibleBuffer;
	//public:
	//	HostVisibleUniformTexelBuffer& Create(const VkDevice Device, const VkPhysicalDeviceMemoryProperties PDMP, const size_t Size, const void* Source = nullptr) {
	//		Super::Create(Device, PDMP, Size, VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT, Source);
	//		return *this;
	//	}
	//};

	class Texture : public DeviceMemoryBase
	{
	private:
		using Super = DeviceMemoryBase;
	public:
		VkImage Image = VK_NULL_HANDLE;
		VkImageView View = VK_NULL_HANDLE;
		Texture& Create(const VkDevice Device, const VkPhysicalDeviceMemoryProperties PDMP, const VkFormat Format, const VkExtent3D& Extent, const uint32_t MipLevels = 1, const uint32_t ArrayLayers = 1, const VkImageUsageFlags Usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT, const VkImageAspectFlags IAF = VK_IMAGE_ASPECT_COLOR_BIT) {
			VK::CreateImageMemory(&Image, &DeviceMemory, Device, PDMP, 0, VK_IMAGE_TYPE_2D, Format, Extent, MipLevels, ArrayLayers, Usage);
			const VkImageViewCreateInfo IVCI = {
				.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
				.pNext = nullptr,
				.flags = 0,
				.image = Image,
				//!< 基本 TYPE_2D, _TYPE_2D_ARRAY として扱う、それ以外で使用する場合は明示的に上書きして使う
				.viewType = ArrayLayers == 1 ? VK_IMAGE_VIEW_TYPE_2D : VK_IMAGE_VIEW_TYPE_2D_ARRAY,
				.format = Format,
				.components = VkComponentMapping({.r = VK_COMPONENT_SWIZZLE_R, .g = VK_COMPONENT_SWIZZLE_G, .b = VK_COMPONENT_SWIZZLE_B, .a = VK_COMPONENT_SWIZZLE_A }),
				.subresourceRange = VkImageSubresourceRange({.aspectMask = IAF, .baseMipLevel = 0, .levelCount = VK_REMAINING_MIP_LEVELS, .baseArrayLayer = 0, .layerCount = VK_REMAINING_ARRAY_LAYERS })
			};
			VERIFY_SUCCEEDED(vkCreateImageView(Device, &IVCI, GetAllocationCallbacks(), &View));
			return *this;
		}
		virtual void Destroy(const VkDevice Device) override {
			Super::Destroy(Device);
			if (VK_NULL_HANDLE != Image) { vkDestroyImage(Device, Image, GetAllocationCallbacks()); }
			if (VK_NULL_HANDLE != View) { vkDestroyImageView(Device, View, GetAllocationCallbacks()); }
		}

		//!< レイアウトを GENERAL にしておく必要がある場合に使用
		//!< 例) 頻繁にレイアウトの変更や戻しが必要になるような場合 UNDEFINED へは戻せないので、戻せるレイアウトである GENERAL を初期レイアウトとしておく
		void PopulateSetLayoutCommand(const VkCommandBuffer CB, const VkImageLayout Layout) {
			ImageMemoryBarrier(CB,
				VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
				Image,
				0, VK_ACCESS_TRANSFER_WRITE_BIT,
				VK_IMAGE_LAYOUT_UNDEFINED, Layout);
		}
		void SubmitSetLayoutCommand(const VkCommandBuffer CB, const VkQueue Queue, const VkImageLayout Layout = VK_IMAGE_LAYOUT_GENERAL) {
			constexpr VkCommandBufferBeginInfo CBBI = { .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO, .pNext = nullptr, .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, .pInheritanceInfo = nullptr };
			VERIFY_SUCCEEDED(vkBeginCommandBuffer(CB, &CBBI)); {
				PopulateSetLayoutCommand(CB, Layout);
			} VERIFY_SUCCEEDED(vkEndCommandBuffer(CB));
			VK::SubmitAndWait(Queue, CB);
		}
	};
	class DepthTexture : public Texture
	{
	private:
		using Super = Texture;
	public:
		DepthTexture& Create(const VkDevice Device, const VkPhysicalDeviceMemoryProperties PDMP, const VkFormat Format, const VkExtent3D& Extent) {
			Super::Create(Device, PDMP, Format, Extent, 1, 1, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT/*| VK_IMAGE_USAGE_SAMPLED_BIT*/, VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT);
			return *this;
		}
	};
	class RenderTexture : public Texture
	{
	private:
		using Super = Texture;
	public:
		RenderTexture& Create(const VkDevice Device, const VkPhysicalDeviceMemoryProperties PDMP, const VkFormat Format, const VkExtent3D& Extent) {
			Super::Create(Device, PDMP, Format, Extent, 1, 1, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
			return *this;
		}
	};
	class StorageTexture : public Texture
	{
	private:
		using Super = Texture;
	public:
		StorageTexture& Create(const VkDevice Device, const VkPhysicalDeviceMemoryProperties PDMP, const VkFormat Format, const VkExtent3D& Extent) {
			Super::Create(Device, PDMP, Format, Extent, 1, 1,
				VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
				//VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
				VK_IMAGE_ASPECT_COLOR_BIT);
			return *this;
		}
	};
	class AnimatedTexture : public Texture
	{
	private:
		using Super = Texture;
	public:
		AnimatedTexture& Create(const VkDevice Device, const VkPhysicalDeviceMemoryProperties PDMP, const VkFormat Format, const uint32_t Bpp, const VkExtent3D& Extent) {
			Super::Create(Device, PDMP, Format, Extent);

			//!< ステージングバッファを作る
			const auto Size = Extent.width * Extent.height * Bpp;
			StagingBuffer.Create(Device, PDMP, Size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

			return *this;
		}
		void UpdateStagingBuffer(const VkDevice Device, const size_t Size, const void* Src) {
			CopyToDeviceMemory(&StagingBuffer.DeviceMemory, Device, Size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, Src);
		}
		void PopulateStagingToImageCommand(VkCommandBuffer CB, const uint32_t Width, const uint32_t Height, const VkPipelineStageFlags PSF) {
			PopulateCopyBufferToImageCommand(CB, StagingBuffer.Buffer, Image, Width, Height, 1, PSF);
		}
		virtual void Destroy(const VkDevice Device) override {
			Super::Destroy(Device);
			StagingBuffer.Destroy(Device);
		}
	protected:
		BufferMemory StagingBuffer;
	};

	void UpdateTexture(Texture& Tex, const uint32_t Width, const uint32_t Height, const uint32_t Bpp, const void* Data, const VkPipelineStageFlags PSF) {
		const auto PDMP = CurrentPhysicalDeviceMemoryProperties;
		const auto CB = CommandBuffers[0];

		VK::Scoped<BufferMemory> StagingBuffer(Device);
		StagingBuffer.Create(Device, PDMP, Width * Height * Bpp, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, Data);

		constexpr VkCommandBufferBeginInfo CBBI = {
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
			.pNext = nullptr,
			.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
			.pInheritanceInfo = nullptr
		};
		VERIFY_SUCCEEDED(vkBeginCommandBuffer(CB, &CBBI)); {
			PopulateCopyBufferToImageCommand(CB, StagingBuffer.Buffer, Tex.Image, Width, Height, 1, PSF);
		} VERIFY_SUCCEEDED(vkEndCommandBuffer(CB));
		VK::SubmitAndWait(GraphicsQueue, CB);
	}
	void UpdateTexture2(Texture& Tex, const uint32_t Width, const uint32_t Height, const uint32_t Bpp, const void* Data, const VkPipelineStageFlags PSF,
		Texture& Tex1, const uint32_t Width1, const uint32_t Height1, const uint32_t Bpp1, const void* Data1, const VkPipelineStageFlags PSF1) {
		const auto PDMP = CurrentPhysicalDeviceMemoryProperties;
		const auto CB = CommandBuffers[0];

		VK::Scoped<BufferMemory> StagingBuffer(Device);
		StagingBuffer.Create(Device, PDMP, Width * Height * Bpp, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, Data);

		VK::Scoped<BufferMemory> StagingBuffer1(Device);
		StagingBuffer1.Create(Device, PDMP, Width1 * Height1 * Bpp1, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, Data1);

		constexpr VkCommandBufferBeginInfo CBBI = {
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
			.pNext = nullptr,
			.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
			.pInheritanceInfo = nullptr
		};
		VERIFY_SUCCEEDED(vkBeginCommandBuffer(CB, &CBBI)); {
			PopulateCopyBufferToImageCommand(CB, StagingBuffer.Buffer, Tex.Image, Width, Height, 1, PSF);
			PopulateCopyBufferToImageCommand(CB, StagingBuffer1.Buffer, Tex1.Image, Width1, Height1, 1, PSF1);
		} VERIFY_SUCCEEDED(vkEndCommandBuffer(CB));
		VK::SubmitAndWait(GraphicsQueue, CB);
	}

#ifdef _WINDOWS
	virtual void OnCreate(HWND hWnd, HINSTANCE hInstance, LPCWSTR Title) override;
	virtual void OnExitSizeMove(HWND hWnd, HINSTANCE hInstance) override;
	virtual void OnPaint(HWND hWnd, HINSTANCE hInstance) override { Super::OnPaint(hWnd, hInstance); Draw(); }
	virtual void OnPreDestroy() override;
	virtual void OnDestroy(HWND hWnd, HINSTANCE hInstance) override;
#endif

	[[nodiscard]] static const char* GetVkResultChar(const VkResult Result);
	[[nodiscard]] static const char* GetFormatChar(const VkFormat Format);
	[[nodiscard]] static const char* GetColorSpaceChar(const VkColorSpaceKHR ColorSpace);
	[[nodiscard]] static const char* GetImageViewTypeChar(const VkImageViewType ImageViewType);
	[[nodiscard]] static const char* GetComponentSwizzleChar(const VkComponentSwizzle ComponentSwizzle);
	[[nodiscard]] static std::string GetComponentMappingString(const VkComponentMapping& CM) {
		return std::string(GetComponentSwizzleChar(CM.r)) + ", " + GetComponentSwizzleChar(CM.g) + ", " + GetComponentSwizzleChar(CM.b) + ", " + GetComponentSwizzleChar(CM.a);
	}
	[[nodiscard]] static const char* GetSystemAllocationScopeChar(const VkSystemAllocationScope SAS);

	[[nodiscard]] static std::array<float, 3> Lerp(const std::array<float, 3>& lhs, const std::array<float, 3>& rhs, const float t) {
		const auto v = glm::mix(*reinterpret_cast<const glm::vec3*>(data(lhs)), *reinterpret_cast<const glm::vec3*>(data(rhs)), t);
		return { v.x, v.y, v.z };
	}
	[[nodiscard]] static std::array<float, 4> Lerp(const std::array<float, 4>& lhs, const std::array<float, 4>& rhs, const float t) {
		const auto v = glm::lerp(*reinterpret_cast<const glm::quat*>(data(lhs)), *reinterpret_cast<const glm::quat*>(data(rhs)), t);
		return { v.x, v.y, v.z, v.w };
	}

	static void BufferMemoryBarrier(const VkCommandBuffer CB,
		const VkPipelineStageFlags SrcStage, const VkPipelineStageFlags DstStage,
		const VkBuffer Buffer,
		const VkAccessFlags SrcAccess, const VkAccessFlags DstAccess)
	{
#ifdef USE_SYNCHRONIZATION2
		constexpr std::array<VkMemoryBarrier2, 0> MBs = {};
		const std::array BMBs = {
			VkBufferMemoryBarrier2({
				.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2,
				.pNext = nullptr,
				.srcStageMask = SrcStage, .srcAccessMask = SrcAccess, .dstStageMask = DstStage, .dstAccessMask = DstAccess,
				.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED, .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
				.buffer = Buffer, .offset = 0, .size = VK_WHOLE_SIZE
			}),
		};
		constexpr std::array<VkImageMemoryBarrier2, 0> IMBs = {};
		const VkDependencyInfo DI = {
			.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
			.pNext = nullptr,
			.dependencyFlags = 0,
			.memoryBarrierCount = static_cast<uint32_t>(std::size(MBs)), .pMemoryBarriers = std::data(MBs),
			.bufferMemoryBarrierCount = static_cast<uint32_t>(std::size(BMBs)),.pBufferMemoryBarriers = std::data(BMBs),
			.imageMemoryBarrierCount = static_cast<uint32_t>(std::size(IMBs)), .pImageMemoryBarriers = std::data(IMBs),
		};
		vkCmdPipelineBarrier2(CB, &DI);
#else
		constexpr std::array<VkMemoryBarrier, 0> MBs = {};
		const std::array BMBs = {
			VkBufferMemoryBarrier({
				.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
				.pNext = nullptr,
				.srcAccessMask = SrcAccess, .dstAccessMask = DstAccess,
				.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED, .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
				.buffer = Buffer, .offset = 0, .size = VK_WHOLE_SIZE
			}),
		};
		constexpr std::array<VkImageMemoryBarrier, 0> IMBs = {};
		vkCmdPipelineBarrier(CB,
			SrcStage, DstStage,
			0,
			static_cast<uint32_t>(std::size(MBs)), std::data(MBs),
			static_cast<uint32_t>(std::size(BMBs)), std::data(BMBs),
			static_cast<uint32_t>(std::size(IMBs)), std::data(IMBs));
#endif
	}
	static void ImageMemoryBarrier(const VkCommandBuffer CB,
		const VkPipelineStageFlags SrcStage, const VkPipelineStageFlags DstStage,
		const VkImage Image,
		const VkAccessFlags SrcAccess, const VkAccessFlags DstAccess,
		const VkImageLayout OldLayout, const VkImageLayout NewLayout,
		const VkImageSubresourceRange& ISR)
	{
#ifdef USE_SYNCHRONIZATION2
		//!< デバイス作成時に VkPhysicalDeviceSynchronization2Features を有効にする必要がある
		constexpr std::array<VkMemoryBarrier2, 0> MBs = {};
		constexpr std::array<VkBufferMemoryBarrier2, 0> BMBs = {};
		const std::array IMBs = {
			VkImageMemoryBarrier2({
				.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
				.pNext = nullptr,
				.srcStageMask = SrcStage, .srcAccessMask = SrcAccess, .dstStageMask = DstStage, .dstAccessMask = DstAccess,
				.oldLayout = OldLayout, .newLayout = NewLayout,
				.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED, .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
				.image = Image,
				.subresourceRange = ISR,
			})
		};
		const VkDependencyInfo DI = {
			.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
			.pNext = nullptr,
			.dependencyFlags = 0,
			.memoryBarrierCount = static_cast<uint32_t>(std::size(MBs)), .pMemoryBarriers = std::data(MBs),
			.bufferMemoryBarrierCount = static_cast<uint32_t>(std::size(BMBs)),.pBufferMemoryBarriers = std::data(BMBs),
			.imageMemoryBarrierCount = static_cast<uint32_t>(std::size(IMBs)), .pImageMemoryBarriers = std::data(IMBs),
		};
		vkCmdPipelineBarrier2(CB, &DI);
#else
		constexpr std::array<VkMemoryBarrier, 0> MBs = {};
		constexpr std::array<VkBufferMemoryBarrier, 0> BMBs = {};
		const std::array IMBs = {
			VkImageMemoryBarrier({
				.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
				.pNext = nullptr,
				.srcAccessMask = SrcAccess, .dstAccessMask = DstAccess,
				.oldLayout = OldLayout, .newLayout = NewLayout,
				.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED, .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
				.image = Image,
				.subresourceRange = ISR,
			})
		};
		vkCmdPipelineBarrier(CB,
			SrcStage, DstStage,
			0,
			static_cast<uint32_t>(std::size(MBs)), std::data(MBs),
			static_cast<uint32_t>(std::size(BMBs)), std::data(BMBs),
			static_cast<uint32_t>(std::size(IMBs)), std::data(IMBs));
#endif
	}
	static void ImageMemoryBarrier(const VkCommandBuffer CB,
		const VkPipelineStageFlags SrcStage, const VkPipelineStageFlags DstStage,
		const VkImage Image,
		const VkAccessFlags SrcAccess, const VkAccessFlags DstAccess,
		const VkImageLayout OldLayout, const VkImageLayout NewLayout)
	{
		ImageMemoryBarrier(CB,
			SrcStage, DstStage,
			Image,
			SrcAccess, DstAccess,
			OldLayout, NewLayout,
			VkImageSubresourceRange({
				.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
				.baseMipLevel = 0, .levelCount = 1,
				.baseArrayLayer = 0, .layerCount = 1
				}));
	}
	static void ImageMemoryBarrier2(const VkCommandBuffer CB,
		const VkPipelineStageFlags SrcStage, const VkPipelineStageFlags DstStage,
		const VkImage Image0,
		const VkAccessFlags SrcAccess0, const VkAccessFlags DstAccess0,
		const VkImageLayout OldLayout0, const VkImageLayout NewLayout0,
		const VkImage Image1,
		const VkAccessFlags SrcAccess1, const VkAccessFlags DstAccess1,
		const VkImageLayout OldLayout1, const VkImageLayout NewLayout1,
		const VkImageSubresourceRange& ISR)
	{
#ifdef USE_SYNCHRONIZATION2
		constexpr std::array<VkMemoryBarrier2, 0> MBs = {};
		constexpr std::array<VkBufferMemoryBarrier2, 0> BMBs = {};
		const std::array IMBs = {
			VkImageMemoryBarrier2({
				.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
				.pNext = nullptr,
				.srcStageMask = SrcStage, .srcAccessMask = SrcAccess0, .dstStageMask = DstStage, .dstAccessMask = DstAccess0,
				.oldLayout = OldLayout0, .newLayout = NewLayout0,
				.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED, .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
				.image = Image0,
				.subresourceRange = ISR,
			}),
			VkImageMemoryBarrier2({
				.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
				.pNext = nullptr,
				.srcStageMask = SrcStage, .srcAccessMask = SrcAccess1, .dstStageMask = DstStage, .dstAccessMask = DstAccess1,
				.oldLayout = OldLayout1, .newLayout = NewLayout1,
				.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED, .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
				.image = Image1,
				.subresourceRange = ISR,
			})
		};
		const VkDependencyInfo DI = {
			.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
			.pNext = nullptr,
			.dependencyFlags = 0,
			.memoryBarrierCount = static_cast<uint32_t>(std::size(MBs)), .pMemoryBarriers = std::data(MBs),
			.bufferMemoryBarrierCount = static_cast<uint32_t>(std::size(BMBs)),.pBufferMemoryBarriers = std::data(BMBs),
			.imageMemoryBarrierCount = static_cast<uint32_t>(std::size(IMBs)), .pImageMemoryBarriers = std::data(IMBs),
		};
		vkCmdPipelineBarrier2(CB, &DI);
#else
		constexpr std::array<VkMemoryBarrier, 0> MBs = {};
		constexpr std::array<VkBufferMemoryBarrier, 0> BMBs = {};
		const std::array IMBs = {
			VkImageMemoryBarrier({
				.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
				.pNext = nullptr,
				.srcAccessMask = SrcAccess0, .dstAccessMask = DstAccess0,
				.oldLayout = OldLayout0, .newLayout = NewLayout0,
				.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED, .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
				.image = Image0,
				.subresourceRange = ISR,
			}),
			VkImageMemoryBarrier({
				.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
				.pNext = nullptr,
				.srcAccessMask = SrcAccess1, .dstAccessMask = DstAccess1,
				.oldLayout = OldLayout1, .newLayout = NewLayout1,
				.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED, .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
				.image = Image1,
				.subresourceRange = ISR,
			})
		};
		vkCmdPipelineBarrier(CB,
			SrcStage, DstStage,
			0,
			static_cast<uint32_t>(std::size(MBs)), std::data(MBs),
			static_cast<uint32_t>(std::size(BMBs)), std::data(BMBs),
			static_cast<uint32_t>(std::size(IMBs)), std::data(IMBs));
#endif
	}
	static void ImageMemoryBarrier2(const VkCommandBuffer CB,
		const VkPipelineStageFlags SrcStage, const VkPipelineStageFlags DstStage,
		const VkImage Image0,
		const VkAccessFlags SrcAccess0, const VkAccessFlags DstAccess0,
		const VkImageLayout OldLayout0, const VkImageLayout NewLayout0,
		const VkImage Image1,
		const VkAccessFlags SrcAccess1, const VkAccessFlags DstAccess1,
		const VkImageLayout OldLayout1, const VkImageLayout NewLayout1)
	{
		ImageMemoryBarrier2(CB,
			SrcStage, DstStage,
			Image0,
			SrcAccess0, DstAccess0,
			OldLayout0, NewLayout0,
			Image1,
			SrcAccess1, DstAccess1,
			OldLayout1, NewLayout1,
			VkImageSubresourceRange({
				.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
				.baseMipLevel = 0, .levelCount = 1,
				.baseArrayLayer = 0, .layerCount = 1
				}));
	}

	//static [[nodiscard]] bool IsSupportedColorFormat(VkPhysicalDevice PD, const VkFormat Format) {
	//	VkFormatProperties FP;
	//	vkGetPhysicalDeviceFormatProperties(PD, Format, &FP);
	//	if (!(FP.optimalTilingFeatures & VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT)) {
	//		Error("VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT not supported\n");
	//		return false;
	//	}
	//	return true;
	//}
	//static [[nodiscard]] bool IsSupportedDepthFormat(VkPhysicalDevice PD, const VkFormat Format) {
	//	VkFormatProperties FP;
	//	vkGetPhysicalDeviceFormatProperties(PD, Format, &FP);
	//	if (!(FP.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)) {
	//		Error("VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT not supported\n");
	//		return false;
	//	}
	//	return true;
	//}
	//static [[nodiscard]] bool IsSupportedSampledImage(VkPhysicalDevice PD, const VkFormat Format, const VkFilter Mag = VK_FILTER_LINEAR, const VkFilter Min = VK_FILTER_LINEAR, const VkSamplerMipmapMode Mip = VK_SAMPLER_MIPMAP_MODE_LINEAR) {
	//	VkFormatProperties FP;
	//	vkGetPhysicalDeviceFormatProperties(PD, Format, &FP);
	//	if (!(FP.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT)) {
	//		Error("VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT not supported\n");
	//		return false;
	//	}
	//	if (VK_FILTER_LINEAR == Mag || VK_FILTER_LINEAR == Min || VK_SAMPLER_MIPMAP_MODE_LINEAR == Mip) {
	//		if (!(FP.linearTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT)) {
	//			Error("VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT not supported\n");
	//			return false;
	//		}
	//	}
	//	return true;
	//}

#pragma region MESH_SHADER
	[[nodiscard]] static bool HasMeshShaderNVSupport(const VkPhysicalDevice PD) {
		VkPhysicalDeviceMeshShaderFeaturesNV PDMSF = { .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_FEATURES_NV, .pNext = nullptr };
		VkPhysicalDeviceFeatures2 PDF2 = { .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2, .pNext = &PDMSF };
		vkGetPhysicalDeviceFeatures2(PD, &PDF2);
		return PDMSF.taskShader && PDMSF.meshShader;
	}
	[[nodiscard]] static bool HasMeshShaderEXTSupport(const VkPhysicalDevice PD) {
		VkPhysicalDeviceMeshShaderFeaturesEXT PDMSF = { .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_FEATURES_EXT, .pNext = nullptr };
		VkPhysicalDeviceFeatures2 PDF2 = { .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2, .pNext = &PDMSF };
		vkGetPhysicalDeviceFeatures2(PD, &PDF2);
		return PDMSF.taskShader && PDMSF.meshShader;
	}
#pragma endregion

public:
	[[nodiscard]] static uint32_t GetMemoryTypeIndex(const VkPhysicalDeviceMemoryProperties& PDMP, const uint32_t TypeBits, const VkMemoryPropertyFlags MPF);

	//[[deprecated("")]]
	static void CopyToHostVisibleDeviceMemory(const VkDevice Dev, const VkDeviceMemory DM, const VkDeviceSize Offset, const VkDeviceSize Size, const void* Source, const VkDeviceSize MappedRangeOffset = 0, const VkDeviceSize MappedRangeSize = VK_WHOLE_SIZE) {
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
			VERIFY_SUCCEEDED(vkMapMemory(Dev, DM, Offset, /*Size*/MappedRangeSize, static_cast<VkMemoryMapFlags>(0), &Data)); {
				memcpy(Data, Source, Size);
				//!< メモリコンテンツが変更されたことをドライバへ知らせる(vkMapMemory()した状態でやること)
				//!< デバイスメモリ確保時に VK_MEMORY_PROPERTY_HOST_COHERENT_BIT を指定した場合は必要ない CreateDeviceMemory(..., VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
				VERIFY_SUCCEEDED(vkFlushMappedMemoryRanges(Dev, static_cast<uint32_t>(size(MMRs)), data(MMRs)));
				//VERIFY_SUCCEEDED(vkInvalidateMappedMemoryRanges(Device, static_cast<uint32_t>(size(MMRs)), data(MMRs)));
			} vkUnmapMemory(Dev, DM);
			}
	}
	static void CopyToDeviceMemory(VkDeviceMemory* DeviceMemory, const VkDevice Device, const size_t Size, const VkBufferUsageFlags BUF, const void* Src) {
		constexpr auto MapSize = VK_WHOLE_SIZE;
		void* Dst;
		VERIFY_SUCCEEDED(vkMapMemory(Device, *DeviceMemory, 0, MapSize, static_cast<VkMemoryMapFlags>(0), &Dst)); {
			std::memcpy(Dst, Src, Size);
			if (!(VK_MEMORY_PROPERTY_HOST_COHERENT_BIT & BUF)) {
				const std::array MMRs = {
					VkMappedMemoryRange({
						.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE,
						.pNext = nullptr,
						.memory = *DeviceMemory,
						.offset = 0,
						.size = MapSize
					}),
				};
				VERIFY_SUCCEEDED(vkFlushMappedMemoryRanges(Device, static_cast<uint32_t>(std::size(MMRs)), std::data(MMRs)));
			}
		} vkUnmapMemory(Device, *DeviceMemory);
	}

#pragma region COMMAND
	static void PopulateCopyBufferToImageCommand(const VkCommandBuffer CB, const VkBuffer Src, const VkImage Dst, const uint32_t Width, const uint32_t Height, const uint32_t Layers, const VkPipelineStageFlags PSF) {
		std::vector<VkBufferImageCopy2> BICs;
		for (uint32_t i = 0; i < Layers; ++i) {
			BICs.emplace_back(
				VkBufferImageCopy2({
					.sType = VK_STRUCTURE_TYPE_BUFFER_IMAGE_COPY_2,
					.pNext = nullptr,
					.bufferOffset = i * 0, .bufferRowLength = 0, .bufferImageHeight = 0,
					.imageSubresource = VkImageSubresourceLayers({.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .mipLevel = 0, .baseArrayLayer = i, .layerCount = 1 }),
					.imageOffset = VkOffset3D({.x = 0, .y = 0, .z = 0 }),
					.imageExtent = VkExtent3D({.width = Width, .height = Height, .depth = 1 })
					})
			);
		}
		PopulateCopyBufferToImageCommand(CB, Src, Dst, VK_ACCESS_SHADER_READ_BIT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, PSF, BICs, 1, 1);
	}
	static void PopulateCopyBufferToImageCommand(const VkCommandBuffer CB, const VkBuffer Src, const VkImage Dst, const VkAccessFlags AF, const VkImageLayout IL, const VkPipelineStageFlags PSF, const std::vector<VkBufferImageCopy2>& BICs, const uint32_t Levels, const uint32_t Layers);
	static void SubmitAndWait(const VkQueue Queue, const VkCommandBuffer CB);
	static void PopulateBeginRenderTargetCommand(const VkCommandBuffer CB, const VkImage RenderTarget) {
		ImageMemoryBarrier(CB,
			VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
			RenderTarget,
			0, VK_ACCESS_TRANSFER_READ_BIT,
			VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_GENERAL);
	}
	static void PopulateEndRenderTargetCommand(const VkCommandBuffer CB, const VkImage RenderTarget, const VkImage Swapchain, const uint32_t Width, const uint32_t Height) {
		ImageMemoryBarrier2(CB,
			VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,

			RenderTarget,
			0, VK_ACCESS_TRANSFER_READ_BIT,
			VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,

			Swapchain,
			0, VK_ACCESS_TRANSFER_WRITE_BIT,
			VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
		{
			const std::array IC2s = {
				VkImageCopy2({
					.sType = VK_STRUCTURE_TYPE_IMAGE_COPY_2,
					.pNext = nullptr,
					.srcSubresource = VkImageSubresourceLayers({.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .mipLevel = 0, .baseArrayLayer = 0, .layerCount = 1 }),
					.srcOffset = VkOffset3D({.x = 0, .y = 0, .z = 0}),
					.dstSubresource = VkImageSubresourceLayers({.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .mipLevel = 0, .baseArrayLayer = 0, .layerCount = 1 }),
					.dstOffset = VkOffset3D({.x = 0, .y = 0, .z = 0}),
					.extent = VkExtent3D({.width = Width, .height = Height, .depth = 1 }),
				}),
			};
			const VkCopyImageInfo2 CII2 = {
				.sType = VK_STRUCTURE_TYPE_COPY_IMAGE_INFO_2,
				.pNext = nullptr,
				.srcImage = RenderTarget, .srcImageLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
				.dstImage = Swapchain, .dstImageLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				.regionCount = static_cast<uint32_t>(std::size(IC2s)), .pRegions = std::data(IC2s)
			};
			vkCmdCopyImage2(CB, &CII2);
		}
		ImageMemoryBarrier(CB,
			VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
			Swapchain,
			0, VK_ACCESS_TRANSFER_WRITE_BIT,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
	}
#pragma endregion

	static void EnumerateMemoryRequirements(const VkMemoryRequirements& MR, const VkPhysicalDeviceMemoryProperties& PDMP);

protected:
	void LoadVulkanLibrary();

	virtual void CreateInstance(const std::vector<const char*>& AdditionalLayers = {}, const std::vector<const char*>& AdditionalExtensions = {});

	virtual void SelectPhysicalDevice(VkInstance Instance);
	void CreateDevice(HWND hWnd, HINSTANCE hInstance, void* pNext, const std::vector<const char*>& ExtensionNames);
	virtual void CreateDevice(HWND hWnd, HINSTANCE hInstance) {
		VKFeature VKF;
		CreateDevice(hWnd, hInstance, VKF.GetPtr(), VKF.ExtNames);
	}

	virtual void CreateFence(VkDevice Dev) {
		//!< ホストとデバイスの同期 (Synchronization between host and device)
		//!< サブミット(vkQueueSubmit) に使用し、Draw()やDispatch()の頭でシグナル(サブミットされたコマンドの完了)を待つ (Used when submit, and wait signal on top of Draw())
		//!< 初回と２回目以降を同じに扱う為に、シグナル済み状態(VK_FENCE_CREATE_SIGNALED_BIT)で作成している (Create with signaled state, to do same operation on first time and second time)
		constexpr VkFenceCreateInfo FCI = { .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO, .pNext = nullptr, .flags = VK_FENCE_CREATE_SIGNALED_BIT };
		VERIFY_SUCCEEDED(vkCreateFence(Dev, &FCI, GetAllocationCallbacks(), &GraphicsFence));
		VERIFY_SUCCEEDED(vkCreateFence(Dev, &FCI, GetAllocationCallbacks(), &ComputeFence));
		LOG_OK();
	}
	virtual void CreateSemaphore(VkDevice Dev) {
#if false// USE_TIMELINESEMAPHORE
#include <thread>
#include <chrono>
		VkSemaphore TimelineSemaphore = VK_NULL_HANDLE;
		{
			constexpr VkSemaphoreTypeCreateInfo STCI = {
				.sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO,
				.pNext = nullptr,
				.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE,
				.initialValue = 0
			};
			const VkSemaphoreCreateInfo SCI = {
				.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
				.pNext = &STCI,
				.flags = 0
			};
			VERIFY_SUCCEEDED(vkCreateSemaphore(Device, &SCI, nullptr, &TimelineSemaphore));
		}
		//!< (1) [ GPU ] (2) で (CPU から) 値へ 2 が書き込まれれば処理が進み、完了すれば (GPU から) 値へ 3 が書き込まれる
		{
			//!< 値が >= 2 になるまで待つ
			const std::array WaitSSIs = {
				VkSemaphoreSubmitInfo({.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO, .pNext = nullptr, .semaphore = TimelineSemaphore, .value = 2, .stageMask = VK_PIPELINE_STAGE_2_NONE, .deviceIndex = 0 })
			};
			constexpr std::array<VkCommandBufferSubmitInfo, 0> CBSIs = {};
			//!< 完了時に値を 3 にする
			const std::array SignalSSIs = {
				VkSemaphoreSubmitInfo({.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO, .pNext = nullptr, .semaphore = TimelineSemaphore, .value = 3, .stageMask = VK_PIPELINE_STAGE_2_NONE, .deviceIndex = 0 })
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
			VERIFY_SUCCEEDED(vkQueueSubmit2(GraphicsQueue.first, static_cast<uint32_t>(std::size(SIs)), std::data(SIs), VK_NULL_HANDLE));
			std::cout << "[GPU] Wait timelineSemaphore value >= " << WaitSSIs.back().value << ", and set value = " << SignalSSIs.back().value << " on finished" << std::endl;
		}
		//!< (2) [CPU] 3秒待った後に、(CPU から) 値を 2 へセットする
		auto Thread = std::thread([&]() {
			std::cout << "[CPU] Sleep 3 seconds" << std::endl;
			std::this_thread::sleep_for(std::chrono::seconds(3));

			//!< 値を 2 にする
			const VkSemaphoreSignalInfo SSI = {
				.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SIGNAL_INFO,
				.pNext = nullptr,
				.semaphore = TimelineSemaphore,
				.value = 2
			};
			VERIFY_SUCCEEDED(vkSignalSemaphore(Device, &SSI));
			std::cout << "[CPU] Set timelineSemaphore value = " << SSI.value << std::endl;
			});
		//!< (3) [ CPU ] (2) で (CPU から) 値へ 2 が書き込まれれば処理が進む
		{
			//!< 値が >= 1 になるまで待つ
			const std::array WaitSemaphores = { TimelineSemaphore };
			const std::array WaitValues = { uint64_t(1) };
			const VkSemaphoreWaitInfo SWI = {
				.sType = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO,
				.pNext = nullptr,
				.flags = 0,
				.semaphoreCount = static_cast<uint32_t>(std::size(WaitSemaphores)), .pSemaphores = std::data(WaitSemaphores), .pValues = std::data(WaitValues)
			};
			VERIFY_SUCCEEDED(vkWaitSemaphores(Device, &SWI, (std::numeric_limits<uint64_t>::max)()));
			std::cout << "[CPU] Wait timelineSemaphore value >= " << WaitValues.back() << " finished" << std::endl;
		}
		//!< (4) [ CPU ] (3) の処理が終わればここに来る、理屈ではタイミングにより 2 または 3 になるが (GPU処理が速いせいかほとんど 3 になる模様)
		{
			uint64_t Value;
			VERIFY_SUCCEEDED(vkGetSemaphoreCounterValue(Device, TimelineSemaphore, &Value));
			std::cout << "[CPU] Get timelineSemaphore value = " << Value << std::endl;
		}
		Thread.join();
		if (VK_NULL_HANDLE != TimelineSemaphore) {
			vkDestroySemaphore(Device, TimelineSemaphore, nullptr);
		}
#else
		//!< キューの同期(異なるキュー間の同期も可能) (Synchronization internal queue)
		//!< イメージ取得(vkAcquireNextImageKHR)、サブミット(VkSubmitInfo)、プレゼンテーション(VkPresentInfoKHR)に使用する (Use when image acquire, submit, presentation) 
		constexpr VkSemaphoreTypeCreateInfo STCI = {
				.sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO,
				.pNext = nullptr,
				.semaphoreType = VK_SEMAPHORE_TYPE_BINARY,
				.initialValue = 0
		};
		const VkSemaphoreCreateInfo SCI = {
			.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO, 
			.pNext = &STCI,
			.flags = 0
		};
#endif
		//!< プレゼント完了同期用 (Wait for presentation finish)
		VERIFY_SUCCEEDED(vkCreateSemaphore(Dev, &SCI, GetAllocationCallbacks(), &NextImageAcquiredSemaphore));
		//!< 描画完了同期用 (Wait for render finish)
		VERIFY_SUCCEEDED(vkCreateSemaphore(Dev, &SCI, GetAllocationCallbacks(), &RenderFinishedSemaphore));
		VERIFY_SUCCEEDED(vkCreateSemaphore(Dev, &SCI, GetAllocationCallbacks(), &ComputeSemaphore));
		LOG_OK();
	}

	virtual void AllocatePrimaryCommandBuffer(const size_t Count);
	void AllocatePrimaryCommandBuffer() {
		AllocatePrimaryCommandBuffer(std::size(Swapchain.ImageAndViews));
	}
	void AllocateSecondaryCommandBuffer(const size_t Count);
	virtual void AllocateSecondaryCommandBuffer() {
		AllocateSecondaryCommandBuffer(std::size(Swapchain.ImageAndViews));
	}
	virtual void AllocateComputeCommandBuffer();
	virtual void AllocateCommandBuffer() {
		AllocatePrimaryCommandBuffer();
		AllocateSecondaryCommandBuffer();
		AllocateComputeCommandBuffer();
	}

	[[nodiscard]] virtual VkSurfaceFormatKHR SelectSurfaceFormat(VkPhysicalDevice PD, VkSurfaceKHR Surface);
	[[nodiscard]] virtual VkPresentModeKHR SelectSurfacePresentMode(VkPhysicalDevice PD, VkSurfaceKHR Surface);
	virtual void CreateSwapchain(VkPhysicalDevice PD, VkSurfaceKHR Sfc, const uint32_t Width, const uint32_t Height, const VkImageUsageFlags AdditionalUsage = 0);
	virtual void GetSwapchainImages();
	virtual void CreateSwapchain() {
		CreateSwapchain(GetCurrentPhysicalDevice(), Surface, GetClientRectWidth(), GetClientRectHeight()); 
		GetSwapchainImages();
	}
	virtual void ResizeSwapchain(const uint32_t Width, const uint32_t Height);

	virtual void CreateViewport(const float Width, const float Height, const float MinDepth = 0.0f, const float MaxDepth = 1.0f);
	virtual void CreateViewport(const VkExtent2D& Extent, const float MinDepth = 0.0f, const float MaxDepth = 1.0f) { 
		CreateViewport(static_cast<const float>(Extent.width), static_cast<const float>(Extent.height), MinDepth, MaxDepth); 
	}

	virtual void LoadScene() {}

	static void CreateBufferMemory(VkBuffer* Buffer, VkDeviceMemory* DeviceMemory, const VkDevice Device, const VkPhysicalDeviceMemoryProperties PDMP, const size_t Size, const VkBufferUsageFlags BUF, const VkMemoryPropertyFlags MPF, const void* Source = nullptr);
	static void CreateImageMemory(VkImage* Image, VkDeviceMemory* DM, const VkDevice Device, const VkPhysicalDeviceMemoryProperties PDMP, const VkImageCreateFlags ICF, const VkImageType IT, const VkFormat Format, const VkExtent3D& Extent, const uint32_t Levels, const uint32_t Layers, const VkImageUsageFlags IUF);

	//static void CreateBufferMemoryAndSubmitTransferCommand(VkBuffer* Buffer, VkDeviceMemory* DeviceMemory,
	//	const VkDevice Device, const VkPhysicalDeviceMemoryProperties PDMP, const VkBufferUsageFlags BUF, const size_t Size,
	//	const void* Source, const VkCommandBuffer CB, const VkAccessFlagBits AF, const VkPipelineStageFlagBits PSF, const VkQueue Queue);

	static VkDeviceAddress GetDeviceAddress(const VkDevice Device, const VkBuffer Buffer) {
		const VkBufferDeviceAddressInfo BDAI = { .sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO, .pNext = nullptr, .buffer = Buffer };
		return vkGetBufferDeviceAddress(Device, &BDAI);
	}
	static VkDeviceAddress GetDeviceAddress(const VkDevice Device, const VkAccelerationStructureKHR AS) {
		const VkAccelerationStructureDeviceAddressInfoKHR ASDAI = { .sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR, .pNext = nullptr, .accelerationStructure = AS };
		return vkGetAccelerationStructureDeviceAddressKHR(Device, &ASDAI);
	}

	virtual void CreateGeometry() {}

	virtual void CreateUniformBuffer() {}
	/*virtual void CreateStorageBuffer() {}
	virtual void CreateUniformTexelBuffer() {}
	virtual void CreateStorageTexStorageTexelBuffer() {}*/

	/**
	アプリ内ではサンプラとサンプルドイメージは別のオブジェクトとして扱うが、シェーダ内ではまとめた一つのオブジェクトとして扱うことができ、プラットフォームによっては効率が良い場合がある
	(コンバインドイメージサンプラ == サンプラ + サンプルドイメージ)
	デスクリプタタイプに VK_DESCRIPTOR_TYPE_SAMPLER や VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE を指定するか、VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER を指定するかの違い
	IMAGE	... VkImage
	BUFFER	... VkBuffer
		TEXEL の付いているものは BufferView が必要
		STORAGE の付いているものはシェーダから書き込み可能

	サンプラ (VkSampler)
		VK_DESCRIPTOR_TYPE_SAMPLER
		layout (set=0, binding=0) uniform sampler MySampler;
	サンプルドイメージ (VkImage)
		VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE
		layout (set=0, binding=0) uniform texture2D MyTexture2D;
	コンバインドイメージサンプラ (VkSampler + VkImage)
		VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER
		layout (set=0, binding=0) uniform sampler2D MySampler2D;

	ストレージイメージ (VkImage)
		VK_DESCRIPTOR_TYPE_STORAGE_IMAGE
		シェーダから書き込み可能、アトミックな操作が可能
		レイアウトは VK_IMAGE_LAYOUT_GENERAL にしておくこと
		layout (set=0, binding=0, r32f) uniform image2D MyImage2D;

	ユニフォームテクセルバッファ (VkBuffer)
		VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER
		1Dのイメージのように扱われる
		1Dイメージは最低限4096テクセルだが、ユニフォームテクセルバッファは最低限65536テクセル(イメージよりも大きなデータへアクセス可能)
		layout (set=0, binding=0) uniform samplerBuffer MySamplerBuffer;

	ストレージテクセルバッファ (vkBuffer)
		VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER
		シェーダから書き込み可能、アトミックな操作が可能
		layout (set=0, binding=0, r32f) uniform imageBuffer MyImageBuffer;

	ユニフォームバッファ (VkBuffer)
		VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC
		ダイナミックユニフォームバッファの場合は、コマンドバッファにオフセットを一緒に積むことができる
		layout (set=0, binding=0) uniform MyUniform { vec4 MyVec4; mat4 MyMat4; }

	ストレージバッファ (VkBuffer)
		VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC
		シェーダから書き込み可能、アトミックな操作が可能
		ダイナミックストレージバッファの場合は、コマンドバッファにオフセットを一緒に積むことができる
		layout (set=0, binding=0) buffer MyBuffer { vec4 MyVec4; mat4 MyMat4; }

	VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT ... (前レンダーパスで)レンダーターゲット(アタッチメント)として使われたものを(レンダーパス中で)入力として取る場合
		layout (input_attachment_index=0, set=0, binding=0) uniform subpassInput MySubpassInput;
	*/

	//virtual void CreateStorageTexelBuffer_Example();

	virtual void CreateDescriptorSetLayout(VkDescriptorSetLayout& DSL, const VkDescriptorSetLayoutCreateFlags Flags, const std::vector<VkDescriptorSetLayoutBinding>& DSLBs);
	virtual void CreatePipelineLayout(VkPipelineLayout& PL, const std::vector<VkDescriptorSetLayout>& DSLs, const std::vector<VkPushConstantRange>& PCRs);
	virtual void CreatePipelineLayout() { PipelineLayouts.emplace_back(); CreatePipelineLayout(PipelineLayouts.back(), {}, {}); }

	virtual void CreateDescriptorPool(VkDescriptorPool& DP, const VkDescriptorPoolCreateFlags Flags, const std::vector<VkDescriptorPoolSize>& DPSs);
	virtual void CreateDescriptorUpdateTemplate(VkDescriptorUpdateTemplate& DUT, const VkPipelineBindPoint PBP, const std::vector<VkDescriptorUpdateTemplateEntry>& DUTEs, const VkDescriptorSetLayout DSL);
	virtual void CreateDescriptor() {}
	virtual void CreateShaderBindingTable() {}

	virtual void CreateTexture() {}
	virtual void CreateTextureArray1x1(const std::vector<uint32_t>& Colors, const VkPipelineStageFlags PSF);
	virtual void CreateImmutableSampler() {}

	virtual void CreateRenderPass(VkRenderPass& RP, const std::vector<VkAttachmentDescription>& ADs, const std::vector<VkSubpassDescription>& SDs, const std::vector<VkSubpassDependency>& Deps);
	virtual void CreateRenderPass() {}

	virtual void CreateFramebuffer(VkFramebuffer& FB, const VkRenderPass RP, const uint32_t Width, const uint32_t Height, const uint32_t Layers, const std::vector<VkImageView>& IVs);
	virtual void CreateFramebuffer(VkFramebuffer& FB, const VkRenderPass RP, const VkExtent2D& Ext, const uint32_t Layers, const std::vector<VkImageView>& IVs) { CreateFramebuffer(FB, RP, Ext.width, Ext.height, Layers, IVs); }
	virtual void CreateFramebuffer() {
		const auto RP = RenderPasses[0];
		for (const auto& i : Swapchain.ImageAndViews) {
			VK::CreateFramebuffer(Framebuffers.emplace_back(), RP, Swapchain.Extent, 1, { i.second });
		}
	}

	[[nodiscard]] virtual VkShaderModule CreateShaderModule(const std::filesystem::path& Path) const;

	virtual void CreateVideo() {}

public:
	struct PipelineCacheData
	{
		uint32_t Size;
		uint32_t Version;
		uint32_t VenderID;
		uint32_t DeviceID;
		std::array<uint8_t, VK_UUID_SIZE> UUID;
	};
protected:
	class PipelineCacheSerializer
	{
	public:
		PipelineCacheSerializer(VkDevice Dev, const std::filesystem::path& Path, const size_t Count) : Device(Dev), FilePath(Path) {
#ifdef ALWAYS_REBUILD_PIPELINE
			DeleteFile(data(FilePath.wstring()));
#endif
			//!< ファイルが読めた場合は PipelineCaches[0] へ読み込む (If file is read, load to PipelineCaches[0])
			std::ifstream In(data(FilePath.string()), std::ios::in | std::ios::binary);
			if (!In.fail()) {
				Logf("PipelineCacheSerializer : Reading PipelineCache = %ls\n", data(FilePath.string()));
				In.seekg(0, std::ios_base::end);
				const size_t Size = In.tellg();
				In.seekg(0, std::ios_base::beg);
				if (Size) {
					auto Data = std::make_unique<std::byte[]>(Size);
					In.read(reinterpret_cast<char*>(Data.get()), Size);
					assert(Size == *reinterpret_cast<const uint32_t*>(Data.get()) && "");
					assert(VK_PIPELINE_CACHE_HEADER_VERSION_ONE == *(reinterpret_cast<const uint32_t*>(Data.get()) + 1) && "");
					{
//#ifdef DEBUG_STDOUT
//						std::cout << *reinterpret_cast<const PipelineCacheData*>(Data);
//#endif
						const auto PCD = reinterpret_cast<const PipelineCacheData*>(Data.get());
						Win::Log("PipelineCacheSerializer\n");
						Win::Logf("\tSize = %d\n", PCD->Size);
						Win::Logf("\tVersion = %s\n", PCD->Version == VK_PIPELINE_CACHE_HEADER_VERSION_ONE ? "VK_PIPELINE_CACHE_HEADER_VERSION_ONE" : "Unknown");
						Win::Logf("\tVenderID = %d\n", PCD->VenderID);
						Win::Logf("\tDeviceID = %d\n", PCD->DeviceID);
						Win::Log("\tUUID = "); for (auto i : PCD->UUID) { Win::Logf("%c", i); } Win::Log("\n");
					}
					PipelineCaches.resize(1); //!< 書き込む際にマージされているはずなので読み込めた場合は1つで良い
					const VkPipelineCacheCreateInfo PCCI = { .sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO, .pNext = nullptr, .flags = 0, .initialDataSize = Size, .pInitialData = Data.get() };
					VERIFY_SUCCEEDED(vkCreatePipelineCache(Device, &PCCI, nullptr, data(PipelineCaches)));
					IsLoaded = true;
				}
				In.close();
			}
			//!< ファイルが読めなかった場合は書き込み用のパイプラインキャッシュを(必要に応じて複数個)作成する (If file is not read, create PiplineCache array for write)
			if (!IsLoaded) {
				Log("PipelineCacheSerializer : Creating PipelineCache\n");
				PipelineCaches.resize(Count); //!< 書き込む場合は複数必要な可能性がある
				constexpr VkPipelineCacheCreateInfo PCCI = { .sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO, .pNext = nullptr, .flags = 0, .initialDataSize = 0, .pInitialData = nullptr };
				for (auto& i : PipelineCaches) {
					VERIFY_SUCCEEDED(vkCreatePipelineCache(Device, &PCCI, nullptr, &i));
				}
			}
		}
		virtual ~PipelineCacheSerializer() {
			if (!IsLoaded) {
				Logf("PipelineCacheSerializer : Writing PipelineCache = %ls\n", data(FilePath.string()));
				//!< パイプラインキャッシュが複数ある場合、末尾のパイプラインキャッシュへマージする (Merge PipelineCaches to the last element)
				if (PipelineCaches.size() > 1) {
					VERIFY_SUCCEEDED(vkMergePipelineCaches(Device, PipelineCaches.back(), static_cast<uint32_t>(size(PipelineCaches) - 1), data(PipelineCaches)));
				}
				//!< 末尾のパイプラインキャッシュをファイルへ書き込む (Write last element of PipelineCache to file)
				size_t Size;
				VERIFY_SUCCEEDED(vkGetPipelineCacheData(Device, PipelineCaches.back(), &Size, nullptr));
				if (Size) {
					auto Data = std::make_unique<std::byte[]>(Size);
					VERIFY_SUCCEEDED(vkGetPipelineCacheData(Device, PipelineCaches.back(), &Size, Data.get()));
					std::ofstream Out(data(FilePath.string()), std::ios::out | std::ios::binary);
					if (!Out.fail()) {
						Out.write(reinterpret_cast<char*>(Data.get()), Size);
						Out.close();
					}
				}
			}
			for (auto i : PipelineCaches) {
				vkDestroyPipelineCache(Device, i, nullptr);
			}
			PipelineCaches.clear();
			LOG_OK();
		}
		VkPipelineCache GetPipelineCache(const size_t i) const { return IsLoaded ? PipelineCaches[0] : PipelineCaches[i]; }
	private:
		VkDevice Device;
		std::filesystem::path FilePath;
		std::vector<VkPipelineCache> PipelineCaches;
		bool IsLoaded = false;
	};

	virtual void CreatePipeline() {}
	//!< VS, FS, TES, TCS, GS
	static void CreatePipelineVsFsTesTcsGs(VkPipeline& PL,
		const VkDevice Dev, const VkPipelineLayout PLL, const VkRenderPass RP,
		const VkPrimitiveTopology Topology, const uint32_t PatchControlPoints,
		const VkPipelineRasterizationStateCreateInfo& PRSCI,
		const VkPipelineDepthStencilStateCreateInfo& PDSSCI,
		const VkPipelineShaderStageCreateInfo* VS, const VkPipelineShaderStageCreateInfo* FS, const VkPipelineShaderStageCreateInfo* TES, const VkPipelineShaderStageCreateInfo* TCS, const VkPipelineShaderStageCreateInfo* GS,
		const std::vector<VkVertexInputBindingDescription>& VIBDs, const std::vector<VkVertexInputAttributeDescription>& VIADs,
		const std::vector<VkPipelineColorBlendAttachmentState>& PCBASs,
		VkPipelineCache PC = VK_NULL_HANDLE);
	//!< TS, MS, FS
	static void CreatePipelineTsMsFs(VkPipeline& PL,
		const VkDevice Dev, const VkPipelineLayout PLL, const VkRenderPass RP,
		const VkPipelineRasterizationStateCreateInfo& PRSCI,
		const VkPipelineDepthStencilStateCreateInfo& PDSSCI,
		const VkPipelineShaderStageCreateInfo* TS, const VkPipelineShaderStageCreateInfo* MS, const VkPipelineShaderStageCreateInfo* FS,
		const std::vector<VkPipelineColorBlendAttachmentState>& PCBASs,
		VkPipelineCache PC = VK_NULL_HANDLE);

	virtual void PopulateSecondaryCommandBuffer([[maybe_unused]] const size_t i) {}
	virtual void PopulateCommandBuffer([[maybe_unused]] const size_t i) {}

	virtual uint32_t GetCurrentBackBufferIndex() const { return Swapchain.Index; }

	virtual void OnUpdate([[maybe_unused]] const uint32_t i) {}
	static void WaitForFence(VkDevice Device, VkFence Fence);
	virtual void SubmitGraphics(const uint32_t i);
	virtual void SubmitCompute(const uint32_t i);
	virtual void Present();

	virtual void Draw();
	virtual void Dispatch();
	
#pragma region ALLOCATION
	static inline const VkAllocationCallbacks AllocationCallbacks = {
		.pUserData = nullptr,
		.pfnAllocation = []([[maybe_unused]] void* pUserData, size_t size, size_t alignment, [[maybe_unused]] VkSystemAllocationScope allocationScope) {
			Logf("_aligned_malloc(size = %d, align = %d) Scope = %s\n", size, alignment, GetSystemAllocationScopeChar(allocationScope));
			return _aligned_malloc(size, alignment);
		},
		.pfnReallocation = []([[maybe_unused]] void* pUserData, void* pOriginal, size_t size, size_t alignment, [[maybe_unused]] VkSystemAllocationScope allocationScope) {
			Logf("_aligned_realloc(size = %d, align = %d) Scope = %s\n", size, alignment, GetSystemAllocationScopeChar(allocationScope));
			return _aligned_realloc(pOriginal, size, alignment);
		},
		.pfnFree = []([[maybe_unused]] void* pUserData, void* pMemory) {
			Log("_aligned_free()\n");
			_aligned_free(pMemory); 
		},
		.pfnInternalAllocation = []([[maybe_unused]] void* pUserData, [[maybe_unused]] size_t size, [[maybe_unused]] VkInternalAllocationType allocationType, [[maybe_unused]] VkSystemAllocationScope allocationScope) {
		},
		.pfnInternalFree = []([[maybe_unused]] void* pUserData, [[maybe_unused]] size_t size, [[maybe_unused]] VkInternalAllocationType allocationType, [[maybe_unused]] VkSystemAllocationScope allocationScope) {
		},
	};
	[[nodiscard]] static const VkAllocationCallbacks* GetAllocationCallbacks() { return nullptr/*&AllocationCallbacks*/; }
#pragma endregion

	[[nodiscard]] virtual VkPhysicalDevice GetCurrentPhysicalDevice() const { return CurrentPhysicalDevice; };
	[[nodiscard]] virtual VkPhysicalDeviceMemoryProperties GetCurrentPhysicalDeviceMemoryProperties() const { return CurrentPhysicalDeviceMemoryProperties; }

#define VK_PROC_ADDR(proc) static PFN_vk ## proc vk ## proc;
#ifdef VK_NO_PROTOYYPES
protected:
#ifdef _WINDOWS
	HMODULE VulkanLibrary = nullptr;
#else
	void* VulkanLibrary = nullptr;
#endif
public:
#include "VKGlobalProcAddr.h"
#include "VKInstanceProcAddr.h"
#include "VKDeviceProcAddr.h"
#endif
#include "VKDeviceProcAddr_RayTracing.h"
#include "VKDeviceProcAddr_MeshShaderNV.h"
#include "VKDeviceProcAddr_MeshShader.h"
#undef VK_PROC_ADDR

#ifdef USE_DEBUG_UTILS
#define VK_PROC_ADDR(proc) static PFN_vk ## proc ## EXT vk ## proc;
#include "VKInstanceProcAddr_DebugUtils.h"
#undef VK_PROC_ADDR

	static VKAPI_ATTR VkBool32 VKAPI_CALL MessengerCallback(VkDebugUtilsMessageSeverityFlagBitsEXT MessageSeverity, VkDebugUtilsMessageTypeFlagsEXT MessageTypes, const VkDebugUtilsMessengerCallbackDataEXT* CallbackData, void* UserData);
	
	static void SetObjectName([[maybe_unused]] VkDevice Device, [[maybe_unused]] const VkObjectType ObjectType, [[maybe_unused]] const uint64_t ObjectHandle, [[maybe_unused]] std::string_view ObjectName);
	static void SetObjectTag([[maybe_unused]] VkDevice Device, [[maybe_unused]] const VkObjectType ObjectType, [[maybe_unused]] const uint64_t ObjectHandle, [[maybe_unused]] const uint64_t TagName, [[maybe_unused]] const size_t TagSize, [[maybe_unused]] const void* TagData);
	static void SetObjectTag(VkDevice Device, const VkObjectType ObjectType, const uint64_t ObjectHandle, const uint64_t TagName, const std::vector<std::byte>& TagData) { SetObjectTag(Device, ObjectType, ObjectHandle, TagName, size(TagData), data(TagData)); }
	
	template<typename T> static void SetObjectName(VkDevice Device, T Object, const std::string_view Name) { DEBUG_BREAK(); /* テンプレート特殊化されていない (Not template specialized) */ }
	template<typename T> static void SetObjectTag(VkDevice Device, T Object, const uint64_t TagName, const size_t TagSize, const void* TagData) { DEBUG_BREAK(); /* テンプレート特殊化されていない (Not template specialized) */ }
#include "VKDebugUtilsNameTag.inl"

	template<typename T> static void InsertLabel([[maybe_unused]] T Object, [[maybe_unused]] const glm::vec4& Color, [[maybe_unused]] std::string_view Name) { DEBUG_BREAK(); /* テンプレート特殊化されていない (Not template specialized) */ }
	template<typename T> static void BeginLabel([[maybe_unused]] T Object, [[maybe_unused]] const glm::vec4& Color, [[maybe_unused]] std::string_view Name) { DEBUG_BREAK(); /* テンプレート特殊化されていない (Not template specialized) */ }
	template<typename T> static void EndLabel([[maybe_unused]] T Object) { DEBUG_BREAK(); /* テンプレート特殊化されていない (Not template specialized) */ }
#include "VKDebugUtilsLabel.inl"

	template<typename T> class ScopedLabel
	{
	public:
		ScopedLabel(T Obj, const glm::vec4& Color, const std::string_view Name) : Object(Obj) { BeginLabel(Object, Color, Name); }
		~ScopedLabel() { EndLabel(Object); }
	private:
		T Object;
	};
#endif

	struct VulkanFeature {
		void* GetPtr() { return &PDV13F; }
		VkPhysicalDeviceNestedCommandBufferFeaturesEXT PDNCBF = {
			.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_NESTED_COMMAND_BUFFER_FEATURES_EXT,
			.pNext = nullptr,
			.nestedCommandBuffer = VK_TRUE,
			.nestedCommandBufferRendering = VK_TRUE,
			.nestedCommandBufferSimultaneousUse = VK_TRUE
		};

		VkPhysicalDeviceVulkan11Features PDV11F = {
			.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES,
			.pNext = &PDNCBF,
			.storageBuffer16BitAccess = VK_FALSE,
			.uniformAndStorageBuffer16BitAccess = VK_FALSE,
			.storagePushConstant16 = VK_FALSE,
			.storageInputOutput16 = VK_FALSE,
			.multiview = VK_FALSE,
			.multiviewGeometryShader = VK_FALSE,
			.multiviewTessellationShader = VK_FALSE,
			.variablePointersStorageBuffer = VK_FALSE,
			.variablePointers = VK_FALSE,
			.protectedMemory = VK_FALSE,
			.samplerYcbcrConversion = VK_FALSE,
			.shaderDrawParameters = VK_FALSE
		};
		VkPhysicalDeviceVulkan12Features PDV12F = {
			.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES,
			.pNext = &PDV11F,
			.samplerMirrorClampToEdge = VK_FALSE,
			.drawIndirectCount = VK_FALSE,
			.storageBuffer8BitAccess = VK_FALSE,
			.uniformAndStorageBuffer8BitAccess = VK_FALSE,
			.storagePushConstant8 = VK_FALSE,
			.shaderBufferInt64Atomics = VK_FALSE,
			.shaderSharedInt64Atomics = VK_FALSE,
			.shaderFloat16 = VK_FALSE,
			.shaderInt8 = VK_FALSE,
			.descriptorIndexing = VK_FALSE,
			.shaderInputAttachmentArrayDynamicIndexing = VK_FALSE,
			.shaderUniformTexelBufferArrayDynamicIndexing = VK_FALSE,
			.shaderStorageTexelBufferArrayDynamicIndexing = VK_FALSE,
			.shaderUniformBufferArrayNonUniformIndexing = VK_FALSE,
			.shaderSampledImageArrayNonUniformIndexing = VK_FALSE,
			.shaderStorageBufferArrayNonUniformIndexing = VK_FALSE,
			.shaderStorageImageArrayNonUniformIndexing = VK_FALSE,
			.shaderInputAttachmentArrayNonUniformIndexing = VK_FALSE,
			.shaderUniformTexelBufferArrayNonUniformIndexing = VK_FALSE,
			.shaderStorageTexelBufferArrayNonUniformIndexing = VK_FALSE,
			.descriptorBindingUniformBufferUpdateAfterBind = VK_FALSE,
			.descriptorBindingSampledImageUpdateAfterBind = VK_FALSE,
			.descriptorBindingStorageImageUpdateAfterBind = VK_FALSE,
			.descriptorBindingStorageBufferUpdateAfterBind = VK_FALSE,
			.descriptorBindingUniformTexelBufferUpdateAfterBind = VK_FALSE,
			.descriptorBindingStorageTexelBufferUpdateAfterBind = VK_FALSE,
			.descriptorBindingUpdateUnusedWhilePending = VK_FALSE,
			.descriptorBindingPartiallyBound = VK_FALSE,
			.descriptorBindingVariableDescriptorCount = VK_FALSE,
			.runtimeDescriptorArray = VK_FALSE,
			.samplerFilterMinmax = VK_FALSE,
			.scalarBlockLayout = VK_FALSE,
			.imagelessFramebuffer = VK_FALSE,
			.uniformBufferStandardLayout = VK_FALSE,
			.shaderSubgroupExtendedTypes = VK_FALSE,
			.separateDepthStencilLayouts = VK_FALSE,
			.hostQueryReset = VK_FALSE,
			.timelineSemaphore = VK_TRUE,
			.bufferDeviceAddress = VK_TRUE,
			.bufferDeviceAddressCaptureReplay = VK_FALSE,
			.bufferDeviceAddressMultiDevice = VK_FALSE,
			.vulkanMemoryModel = VK_FALSE,
			.vulkanMemoryModelDeviceScope = VK_FALSE,
			.vulkanMemoryModelAvailabilityVisibilityChains = VK_FALSE,
			.shaderOutputViewportIndex = VK_FALSE,
			.shaderOutputLayer = VK_FALSE,
			.subgroupBroadcastDynamicId = VK_FALSE,
		};
		VkPhysicalDeviceVulkan13Features PDV13F = {
			.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
			.pNext = &PDV12F,
			.robustImageAccess = VK_FALSE,
			.inlineUniformBlock = VK_FALSE,
			.descriptorBindingInlineUniformBlockUpdateAfterBind = VK_FALSE,
			.pipelineCreationCacheControl = VK_FALSE,
			.privateData = VK_FALSE,
			.shaderDemoteToHelperInvocation = VK_FALSE,
			.shaderTerminateInvocation = VK_FALSE,
			.subgroupSizeControl = VK_FALSE,
			.computeFullSubgroups = VK_FALSE,
			.synchronization2 = VK_TRUE,
			.textureCompressionASTC_HDR = VK_FALSE,
			.shaderZeroInitializeWorkgroupMemory = VK_FALSE,
			.dynamicRendering = VK_TRUE,
			.shaderIntegerDotProduct = VK_FALSE,
			.maintenance4 = VK_TRUE,
		};
	};

protected:
	std::vector<std::thread> Threads;

	VkInstance Instance = VK_NULL_HANDLE;
#ifdef USE_DEBUG_UTILS
	VkDebugUtilsMessengerEXT DebugUtilsMessenger = VK_NULL_HANDLE;
#endif
	VkSurfaceKHR Surface = VK_NULL_HANDLE;
	
	std::vector<VkPhysicalDevice> PhysicalDevices;
	VkPhysicalDevice CurrentPhysicalDevice = VK_NULL_HANDLE;
	VkPhysicalDeviceMemoryProperties CurrentPhysicalDeviceMemoryProperties;

	VkDevice Device = VK_NULL_HANDLE;
	VkQueue GraphicsQueue = VK_NULL_HANDLE;
	VkQueue PresentQueue = VK_NULL_HANDLE;
	VkQueue ComputeQueue = VK_NULL_HANDLE;
	//VkQueue TransferQueue = VK_NULL_HANDLE;
	//VkQueue SparceBindingQueue = VK_NULL_HANDLE;
	uint32_t GraphicsQueueFamilyIndex = UINT32_MAX;
	uint32_t PresentQueueFamilyIndex = UINT32_MAX;
	//uint32_t PresentQueueIndexInFamily = UINT32_MAX;
	uint32_t ComputeQueueFamilyIndex = UINT32_MAX;
	//uint32_t ComputeQueueIndexInFamily = UINT32_MAX;
	//uint32_t TransferQueueFamilyIndex = UINT32_MAX;
	//uint32_t TransferQueueIndex = UINT32_MAX;;
	//uint32_t SparceBindingQueueFamilyIndex = UINT32_MAX;
	//uint32_t SparceBindingQueueIndex = UINT32_MAX;;

	/**
	フェンス		... デバイスとホスト(GPUとCPU)の同期
	セマフォ		... コマンドバッファ間の同期 (異なるキューファミリでもよい) ... DXには存在せずフェンスで代用
	バリア		... コマンドバッファ内の同期
	イベント		... コマンドバッファ間の同期 (同一キューファミリであること) ... DXには存在しない
	*/
	VkFence GraphicsFence = VK_NULL_HANDLE;
	VkFence ComputeFence = VK_NULL_HANDLE;

	VkSemaphore NextImageAcquiredSemaphore = VK_NULL_HANDLE;	//!< プレゼント完了までウエイト
	VkSemaphore RenderFinishedSemaphore = VK_NULL_HANDLE;		//!< 描画完了するまでウエイト
	VkSemaphore ComputeSemaphore = VK_NULL_HANDLE;

	VkSurfaceFormatKHR SurfaceFormat;

	//!< [DX] <ID3D12Resource, D3D12_CPU_DESCRIPTOR_HANDLE> 相当
	//!< [DX] <ID3D12Resource, D3D12_CPU_DESCRIPTOR_HANDLE> 相当
	using ImageAndView = std::pair<VkImage, VkImageView>;
	struct Swapchain {
		VkSwapchainKHR VkSwapchain = VK_NULL_HANDLE;
		std::vector<ImageAndView> ImageAndViews; 
		uint32_t Index = 0;
		VkExtent2D Extent;
	};
	Swapchain Swapchain;

	std::vector<VkCommandPool> CommandPools;
	std::vector<VkCommandBuffer> CommandBuffers;
	std::vector<VkCommandPool> SecondaryCommandPools;
	std::vector<VkCommandBuffer> SecondaryCommandBuffers;
	std::vector<VkCommandPool> ComputeCommandPools;
	std::vector<VkCommandBuffer> ComputeCommandBuffers;

	std::vector<VertexBuffer> VertexBuffers;
	std::vector<IndexBuffer> IndexBuffers;
	std::vector<IndirectBuffer> IndirectBuffers;
	std::vector<UniformBuffer> UniformBuffers;

	VkFormat DepthFormat = VK_FORMAT_D24_UNORM_S8_UINT;
	std::vector<Texture> Textures;
	std::vector<DepthTexture> DepthTextures;
	std::vector<RenderTexture> RenderTextures;
	std::vector<StorageTexture> StorageTextures;
	std::vector<AnimatedTexture> AnimatedTextures;

	//!< VKの場合、通常サンプラ、イミュータブルサンプラとも同様に VkSampler を作成する、デスクリプタセットの指定が異なるだけ
	std::vector<VkSampler> Samplers;

	std::vector<VkDescriptorSetLayout> DescriptorSetLayouts;
	std::vector<VkPipelineLayout> PipelineLayouts;

	std::vector<VkRenderPass> RenderPasses;
	std::vector<VkPipeline> Pipelines;
	std::vector<VkFramebuffer> Framebuffers;

	std::vector<VkDescriptorPool> DescriptorPools;
	std::vector<VkDescriptorSet> DescriptorSets;
	std::vector<VkDescriptorUpdateTemplate> DescriptorUpdateTemplates;
	
	std::vector<VkViewport> Viewports;
	std::vector<VkRect2D> ScissorRects;

	//!< https://matthewwellings.com/blog/the-new-vulkan-coordinate-system/
	//!< VKのクリップスペースはYが反転、Zが半分 (Vulkan clip space has inverted Y and half Z)
	//!< → ビューポートの指定次第で回避できる (USE_VIEWPORT_Y_UP使用箇所参照)、Vulkan はTLが原点(DirectX、OpenGLはBLが原点)
	//static glm::mat4 GetVulkanClipSpace() {
	//	return glm::mat4(1.0f, 0.0f, 0.0f, 0.0f,
	//			0.0f, -1.0f, 0.0f, 0.0f,
	//			0.0f, 0.0f, 0.5f, 0.0f,
	//			0.0f, 0.0f, 0.5f, 1.0f);
	//}
};

#ifdef DEBUG_STDOUT
static std::ostream& operator<<(std::ostream& lhs, const glm::vec2& rhs) { for (auto i = 0; i < rhs.length(); ++i) { lhs << rhs[i] << ", "; } lhs << std::endl; return lhs; }
static std::ostream& operator<<(std::ostream& lhs, const glm::vec3& rhs) { for (auto i = 0; i < rhs.length(); ++i) { lhs << rhs[i] << ", "; } lhs << std::endl; return lhs; }
static std::ostream& operator<<(std::ostream& lhs, const glm::vec4& rhs) { for (auto i = 0; i < rhs.length(); ++i) { lhs << rhs[i] << ", "; } lhs << std::endl; return lhs; }
static std::ostream& operator<<(std::ostream& lhs, const glm::quat& rhs) { for (auto i = 0; i < rhs.length(); ++i) { lhs << rhs[i] << ", "; } lhs << std::endl; return lhs; }
static std::ostream& operator<<(std::ostream& lhs, const glm::mat2& rhs) { for (auto i = 0; i < 2; ++i) { lhs << rhs[i]; } lhs << std::endl; return lhs; }
static std::ostream& operator<<(std::ostream& lhs, const glm::mat3& rhs) { for (auto i = 0; i < 3; ++i) { lhs << rhs[i]; } lhs << std::endl; return lhs; }
static std::ostream& operator<<(std::ostream& lhs, const glm::mat4& rhs) { for (auto i = 0; i < 4; ++i) { lhs << rhs[i]; } lhs << std::endl; return lhs; }

#pragma region PROPERTY
static std::ostream& operator<<(std::ostream& lhs, const VkExtensionProperties& rhs) { Win::Logf("\t\t\t\t\"%s\"\n", rhs.extensionName); return lhs; }
static std::ostream& operator<<(std::ostream& lhs, const VkLayerProperties& rhs) { Win::Logf("\t\t\t\"%s\" (%s)\n", rhs.layerName, rhs.description); return lhs; }
#pragma endregion

#pragma region PHYSICAL_DEVICE_PROPERTY
static std::ostream& operator<<(std::ostream& lhs, const VkPhysicalDeviceProperties& rhs) {
	Win::Log("\t\tPhysical Device API Version = ");
	Win::Logf("%d.%d(Patch = %d)\n", VK_VERSION_MAJOR(rhs.apiVersion), VK_VERSION_MINOR(rhs.apiVersion), VK_VERSION_PATCH(rhs.apiVersion));
#if true
	constexpr auto APIVersion = VK_HEADER_VERSION_COMPLETE;
#else
	uint32_t APIVersion; VERIFY_SUCCEEDED(vkEnumerateInstanceVersion(&APIVersion));
#endif
	if (rhs.apiVersion != APIVersion) {
		Win::Log("\t\t");
		Win::Warningf("[ PHYSICAL DEVICE ] %d.%d(Patch = %d) != %d.%d(Patch = %d) [ INSTANCE(SDK) ]\n",
			VK_VERSION_MAJOR(rhs.apiVersion), VK_VERSION_MINOR(rhs.apiVersion), VK_VERSION_PATCH(rhs.apiVersion),
			VK_VERSION_MAJOR(APIVersion), VK_VERSION_MINOR(APIVersion), VK_VERSION_PATCH(APIVersion));
	}

#define PHYSICAL_DEVICE_TYPE_ENTRY(entry) if(VK_PHYSICAL_DEVICE_TYPE_##entry == rhs.deviceType) { Win::Log(#entry); }
	Win::Logf("\t\t[ %s ](", rhs.deviceName);
	PHYSICAL_DEVICE_TYPE_ENTRY(OTHER);
	PHYSICAL_DEVICE_TYPE_ENTRY(INTEGRATED_GPU);
	PHYSICAL_DEVICE_TYPE_ENTRY(DISCRETE_GPU);
	PHYSICAL_DEVICE_TYPE_ENTRY(VIRTUAL_GPU);
	PHYSICAL_DEVICE_TYPE_ENTRY(CPU);
	Win::Log(")\n");
#undef PHYSICAL_DEVICE_TYPE_ENTRY

#define PROPERTY_LIMITS_ENTRY(entry) Win::Logf("\t\t\t\t%s = %d\n", #entry, rhs.limits.##entry);
	Win::Log("\t\t\tPhysicalDeviceProperties.PhysicalDeviceLimits\n");
	PROPERTY_LIMITS_ENTRY(maxUniformBufferRange);
	//PROPERTY_LIMITS_ENTRY(maxStorageBufferRange);
	PROPERTY_LIMITS_ENTRY(maxPushConstantsSize);
	PROPERTY_LIMITS_ENTRY(maxFragmentOutputAttachments);
	PROPERTY_LIMITS_ENTRY(maxColorAttachments);
	PROPERTY_LIMITS_ENTRY(maxViewports);
#undef PROPERTY_LIMITS_ENTRY
	return lhs;
}

#pragma region RAYTRACING
static std::ostream& operator<<(std::ostream& lhs, const VkPhysicalDeviceRayTracingPipelinePropertiesKHR& rhs) {
	Win::Log("\t\t\tVkPhysicalDeviceRayTracingPipelinePropertiesKHR\n");
	Win::Logf("\t\t\t\tshaderGroupHandleSize = %d\n", rhs.shaderGroupHandleSize);
	Win::Logf("\t\t\t\tmaxRayRecursionDepth = %d\n", rhs.maxRayRecursionDepth);
	Win::Logf("\t\t\t\tmaxShaderGroupStride = %d\n", rhs.maxShaderGroupStride);
	Win::Logf("\t\t\t\tshaderGroupBaseAlignment = %d\n", rhs.shaderGroupBaseAlignment);
	Win::Logf("\t\t\t\tshaderGroupHandleCaptureReplaySize = %d\n", rhs.shaderGroupHandleCaptureReplaySize);
	Win::Logf("\t\t\t\tmaxRayDispatchInvocationCount = %d\n", rhs.maxRayDispatchInvocationCount);
	Win::Logf("\t\t\t\tshaderGroupHandleAlignment = %d\n", rhs.shaderGroupHandleAlignment);
	Win::Logf("\t\t\t\tmaxRayHitAttributeSize = %d\n", rhs.maxRayHitAttributeSize);
	return lhs;
}
#pragma endregion

#pragma region MESH_SHADER
static std::ostream& operator<<(std::ostream& lhs, const VkPhysicalDeviceMeshShaderPropertiesNV& rhs) {
	Win::Log("\t\t\tVkPhysicalDeviceMeshShaderPropertiesNV\n");
	Win::Logf("\t\t\t\tmaxDrawMeshTasksCount = %d\n", rhs.maxDrawMeshTasksCount);
	Win::Logf("\t\t\t\tmaxTaskWorkGroupInvocations = %d\n", rhs.maxTaskWorkGroupInvocations);
	Win::Logf("\t\t\t\tmaxTaskWorkGroupSize[] = %d, %d, %d\n", rhs.maxTaskWorkGroupSize[0], rhs.maxTaskWorkGroupSize[1], rhs.maxTaskWorkGroupSize[2]);
	Win::Logf("\t\t\t\tmaxTaskTotalMemorySize = %d\n", rhs.maxTaskTotalMemorySize);
	Win::Logf("\t\t\t\tmaxTaskOutputCount = %d\n", rhs.maxTaskOutputCount);
	Win::Logf("\t\t\t\tmaxMeshWorkGroupInvocations = %d\n", rhs.maxMeshWorkGroupInvocations);
	Win::Logf("\t\t\t\tmaxMeshWorkGroupSize[] = %d, %d, %d\n", rhs.maxMeshWorkGroupSize[0], rhs.maxMeshWorkGroupSize[1], rhs.maxMeshWorkGroupSize[2]);
	Win::Logf("\t\t\t\tmaxMeshTotalMemorySize = %d\n", rhs.maxMeshTotalMemorySize);
	Win::Logf("\t\t\t\tmaxMeshOutputVertices = %d\n", rhs.maxMeshOutputVertices);
	Win::Logf("\t\t\t\tmaxMeshOutputPrimitives = %d\n", rhs.maxMeshOutputPrimitives);
	Win::Logf("\t\t\t\tmaxMeshMultiviewViewCount = %d\n", rhs.maxMeshMultiviewViewCount);
	Win::Logf("\t\t\t\tmeshOutputPerVertexGranularity = %d\n", rhs.meshOutputPerVertexGranularity);
	Win::Logf("\t\t\t\tmeshOutputPerPrimitiveGranularity = %d\n", rhs.meshOutputPerPrimitiveGranularity);
	return lhs;
}
static std::ostream& operator<<(std::ostream& lhs, const VkPhysicalDeviceMeshShaderPropertiesEXT& rhs) {
	Win::Log("\t\t\tVkPhysicalDeviceMeshShaderPropertiesEXT\n");
	Win::Logf("\t\t\t\tmaxTaskWorkGroupTotalCount = %d\n", rhs.maxTaskWorkGroupTotalCount);
	Win::Logf("\t\t\t\tmaxTaskWorkGroupCount[] = %d, %d, %d\n", rhs.maxTaskWorkGroupCount[0], rhs.maxTaskWorkGroupCount[1], rhs.maxTaskWorkGroupCount[2]);
	Win::Logf("\t\t\t\tmaxTaskWorkGroupInvocations = %d\n", rhs.maxTaskWorkGroupInvocations);
	Win::Logf("\t\t\t\tmaxTaskWorkGroupSize[] = %d, %d, %d\n", rhs.maxTaskWorkGroupSize[0], rhs.maxTaskWorkGroupSize[1], rhs.maxTaskWorkGroupSize[2]);
	Win::Logf("\t\t\t\tmaxTaskPayloadSize = %d\n", rhs.maxTaskPayloadSize);
	Win::Logf("\t\t\t\tmaxTaskSharedMemorySize = %d\n", rhs.maxTaskSharedMemorySize);
	Win::Logf("\t\t\t\tmaxTaskPayloadAndSharedMemorySize = %d\n", rhs.maxTaskPayloadAndSharedMemorySize);
	Win::Logf("\t\t\t\tmaxMeshWorkGroupTotalCount = %d\n", rhs.maxMeshWorkGroupTotalCount);
	Win::Logf("\t\t\t\tmaxMeshWorkGroupCount[] = %d, %d, %d\n", rhs.maxMeshWorkGroupCount[0], rhs.maxMeshWorkGroupCount[1], rhs.maxMeshWorkGroupCount[2]);
	Win::Logf("\t\t\t\tmaxMeshWorkGroupInvocations = %d\n", rhs.maxMeshWorkGroupInvocations);
	Win::Logf("\t\t\t\tmaxMeshWorkGroupSize[] = %d, %d, %d\n", rhs.maxMeshWorkGroupSize[0], rhs.maxMeshWorkGroupSize[1], rhs.maxMeshWorkGroupSize[2]);
	Win::Logf("\t\t\t\tmaxMeshSharedMemorySize = %d\n", rhs.maxMeshSharedMemorySize);
	Win::Logf("\t\t\t\tmaxMeshPayloadAndSharedMemorySize = %d\n", rhs.maxMeshPayloadAndSharedMemorySize);
	Win::Logf("\t\t\t\tmaxMeshOutputMemorySize = %d\n", rhs.maxMeshOutputMemorySize);
	Win::Logf("\t\t\t\tmaxMeshPayloadAndOutputMemorySize = %d\n", rhs.maxMeshPayloadAndOutputMemorySize);
	Win::Logf("\t\t\t\tmaxMeshOutputComponents = %d\n", rhs.maxMeshOutputComponents);
	Win::Logf("\t\t\t\tmaxMeshOutputVertices = %d\n", rhs.maxMeshOutputVertices);
	Win::Logf("\t\t\t\tmaxMeshOutputPrimitives = %d\n", rhs.maxMeshOutputPrimitives);
	Win::Logf("\t\t\t\tmaxMeshOutputLayers = %d\n", rhs.maxMeshOutputLayers);
	Win::Logf("\t\t\t\tmaxMeshMultiviewViewCount = %d\n", rhs.maxMeshMultiviewViewCount);
	Win::Logf("\t\t\t\tmeshOutputPerVertexGranularity = %d\n", rhs.meshOutputPerVertexGranularity);
	Win::Logf("\t\t\t\tmeshOutputPerPrimitiveGranularity = %d\n", rhs.meshOutputPerPrimitiveGranularity);
	Win::Logf("\t\t\t\tmaxPreferredTaskWorkGroupInvocations = %d\n", rhs.maxPreferredTaskWorkGroupInvocations);
	Win::Logf("\t\t\t\tmaxPreferredMeshWorkGroupInvocations = %d\n", rhs.maxPreferredMeshWorkGroupInvocations);
	Win::Logf("\t\t\t\tprefersLocalInvocationVertexOutput = %d\n", rhs.prefersLocalInvocationVertexOutput);
	Win::Logf("\t\t\t\tprefersLocalInvocationPrimitiveOutput = %d\n", rhs.prefersLocalInvocationPrimitiveOutput);
	Win::Logf("\t\t\t\tprefersCompactVertexOutput = %d\n", rhs.prefersCompactVertexOutput);
	Win::Logf("\t\t\t\tprefersCompactPrimitiveOutput = %d\n", rhs.prefersCompactPrimitiveOutput);
	return lhs;
}
#pragma endregion
#pragma endregion

#pragma region PHYSICAL_DEVICE_FEATURE
static std::ostream& operator<<(std::ostream& lhs, const VkPhysicalDeviceFeatures& rhs) {
	assert(rhs.tessellationShader && "tessellationShader is not supported");
	Win::Log("\t\t\tPhysicalDeviceFeatures\n");
#define VK_DEVICEFEATURE_ENTRY(entry) if (rhs.##entry) { Win::Log("\t\t\t\t" #entry "\n"); }
#include "VKDeviceFeature.h"
#undef VK_DEVICEFEATURE_ENTRY
	return lhs;
}
#pragma region RAYTRACING
static std::ostream& operator<<(std::ostream& lhs, const VkPhysicalDeviceBufferDeviceAddressFeatures& rhs) {
	Win::Log("\t\t\tVkPhysicalDeviceBufferDeviceAddressFeatures\n");
	if (rhs.bufferDeviceAddress) { Win::Log("\t\t\t\tbufferDeviceAddress\n"); }
	if (rhs.bufferDeviceAddressCaptureReplay) { Win::Log("\t\t\t\tbufferDeviceAddressCaptureReplay\n"); }
	if (rhs.bufferDeviceAddressMultiDevice) { Win::Log("\t\t\t\tbufferDeviceAddressMultiDevice\n"); }
	return lhs;
}
static std::ostream& operator<<(std::ostream& lhs, const VkPhysicalDeviceRayTracingPipelineFeaturesKHR& rhs) {
	Win::Log("\t\t\tVkPhysicalDeviceRayTracingPipelineFeaturesKHR\n");
	if (rhs.rayTracingPipeline) { Win::Log("\t\t\t\trayTracingPipeline\n"); }
	if (rhs.rayTracingPipelineShaderGroupHandleCaptureReplay) { Win::Log("\t\t\t\trayTracingPipelineShaderGroupHandleCaptureReplay\n"); }
	if (rhs.rayTracingPipelineShaderGroupHandleCaptureReplayMixed) { Win::Log("\t\t\t\trayTracingPipelineShaderGroupHandleCaptureReplayMixed\n"); }
	if (rhs.rayTracingPipelineTraceRaysIndirect) { Win::Log("\t\t\t\trayTracingPipelineTraceRaysIndirect\n"); }
	if (rhs.rayTraversalPrimitiveCulling) { Win::Log("\t\t\t\trayTraversalPrimitiveCulling\n"); }
	return lhs;
}
static std::ostream& operator<<(std::ostream& lhs, const VkPhysicalDeviceAccelerationStructureFeaturesKHR& rhs) {
	Win::Log("\t\t\tVkPhysicalDeviceAccelerationStructureFeaturesKHR\n");
	if (rhs.accelerationStructure) { Win::Log("\t\t\t\taccelerationStructure\n"); }
	if (rhs.accelerationStructureCaptureReplay) { Win::Log("\t\t\t\taccelerationStructureCaptureReplay\n"); }
	if (rhs.accelerationStructureIndirectBuild) { Win::Log("\t\t\t\taccelerationStructureIndirectBuild\n"); }
	if (rhs.accelerationStructureHostCommands) { Win::Log("\t\t\t\taccelerationStructureHostCommands\n"); }
	if (rhs.descriptorBindingAccelerationStructureUpdateAfterBind) { Win::Log("\t\t\t\tdescriptorBindingAccelerationStructureUpdateAfterBind\n"); }
	return lhs;
}
#pragma endregion
#pragma region MESH_SHADER
static std::ostream& operator<<(std::ostream& lhs, const VkPhysicalDeviceMeshShaderFeaturesNV& rhs) {
	Win::Log("\t\t\tVkPhysicalDeviceMeshShaderFeaturesNV\n");
	if (rhs.taskShader) { Win::Log("\t\t\t\ttaskShader\n"); }
	if (rhs.meshShader) { Win::Log("\t\t\t\tmeshShader\n"); }
	return lhs;
}
static std::ostream& operator<<(std::ostream& lhs, const VkPhysicalDeviceMeshShaderFeaturesEXT& rhs) {
	Win::Log("\t\t\tVkPhysicalDeviceMeshShaderFeaturesEXT\n");
	if (rhs.taskShader) { Win::Log("\t\t\t\ttaskShader\n"); }
	if (rhs.meshShader) { Win::Log("\t\t\t\tmeshShader\n"); }
	if (rhs.multiviewMeshShader) { Win::Log("\t\t\t\tmultiviewMeshShader\n"); }
	if (rhs.primitiveFragmentShadingRateMeshShader) { Win::Log("\t\t\t\tprimitiveFragmentShadingRateMeshShader\n"); }
	if (rhs.meshShaderQueries) { Win::Log("\t\t\t\tmeshShaderQueries\n"); }
	return lhs;
}
#pragma endregion
#pragma endregion

static std::ostream& operator<<(std::ostream& lhs, const VkPhysicalDeviceMemoryProperties& rhs) {
	Win::Log("\t\t\tMemoryType\n");
	for (uint32_t i = 0; i < rhs.memoryTypeCount; ++i) {
		Win::Log("\t\t\t\t");
		Win::Logf("[%d] HeapIndex = %d", i, rhs.memoryTypes[i].heapIndex);

		Win::Logf(", PropertyFlags(%d) = ", rhs.memoryTypes[i].propertyFlags);
		if (rhs.memoryTypes[i].propertyFlags) {
#define MEMORY_PROPERTY_ENTRY(entry) if(VK_MEMORY_PROPERTY_##entry##_BIT & rhs.memoryTypes[i].propertyFlags) { Win::Log(#entry " | "); }
			MEMORY_PROPERTY_ENTRY(DEVICE_LOCAL);
			MEMORY_PROPERTY_ENTRY(HOST_VISIBLE);
			MEMORY_PROPERTY_ENTRY(HOST_COHERENT);
			MEMORY_PROPERTY_ENTRY(HOST_CACHED);
			MEMORY_PROPERTY_ENTRY(LAZILY_ALLOCATED);
			MEMORY_PROPERTY_ENTRY(PROTECTED);
#undef MEMORY_PROPERTY_ENTRY
		}
		Win::Log("\n");
	}

	Win::Log("\t\t\tMemoryHeap\n");
	for (uint32_t i = 0; i < rhs.memoryHeapCount; ++i) {
		Win::Log("\t\t\t\t");
		Win::Logf("[%d] Size = %llu", i, rhs.memoryHeaps[i].size);

		if (rhs.memoryHeaps[i].flags) {
			Win::Log(", Flags = ");
			if (VK_MEMORY_HEAP_DEVICE_LOCAL_BIT & rhs.memoryHeaps[i].flags) { Win::Log("DEVICE_LOCAL | "); }
			if (VK_MEMORY_HEAP_MULTI_INSTANCE_BIT_KHR & rhs.memoryHeaps[i].flags) { Win::Log("MULTI_INSTANCE | "); }
		}
		Win::Log("\n");
	}
	return lhs;
}

#pragma region PHYSICAL_DEVICE
static std::ostream& operator<<(std::ostream& lhs, const VkPhysicalDevice& rhs) {
	Win::Logf("\t\tDevice Layer Properties\n");
	uint32_t LPC = 0;
	VERIFY_SUCCEEDED(vkEnumerateDeviceLayerProperties(rhs, &LPC, nullptr));
	if (LPC) [[likely]] {
		std::vector<VkLayerProperties> LPs(LPC);
		VERIFY_SUCCEEDED(vkEnumerateDeviceLayerProperties(rhs, &LPC, data(LPs)));
		for (const auto& i : LPs) {
			std::cout << i;
			uint32_t EPC = 0;
			VERIFY_SUCCEEDED(vkEnumerateDeviceExtensionProperties(rhs, i.layerName, &EPC, nullptr));
			if (EPC) [[likely]] {
				std::vector<VkExtensionProperties> EPs(EPC);
				VERIFY_SUCCEEDED(vkEnumerateDeviceExtensionProperties(rhs, i.layerName, &EPC, data(EPs)));
				for (const auto& j : EPs) {
					std::cout << j;
				}
			}
		}
	}
	return lhs;
}
#pragma endregion

#pragma region DEVICE
static std::ostream& operator<<(std::ostream& lhs, const VkQueueFamilyProperties& rhs) {
#define QUEUE_FLAG_ENTRY(entry) if(VK_QUEUE_##entry##_BIT & rhs.queueFlags) { Win::Logf("%s | ", #entry); }
	Win::Log("QueueFlags = ");
	QUEUE_FLAG_ENTRY(GRAPHICS);
	QUEUE_FLAG_ENTRY(COMPUTE);
	QUEUE_FLAG_ENTRY(TRANSFER);
	QUEUE_FLAG_ENTRY(SPARSE_BINDING);
	QUEUE_FLAG_ENTRY(PROTECTED);
#undef QUEUE_FLAG_ENTRY
	return lhs;
}
#pragma endregion

#pragma region SWAPCHAIN
static std::ostream& operator<<(std::ostream& lhs, const VkSurfaceCapabilitiesKHR& rhs) {
	Win::Log("\tSurfaceCapabilities\n");
	Win::Logf("\t\tminImageCount = %d\n", rhs.minImageCount);
	Win::Logf("\t\tmaxImageCount = %d\n", rhs.maxImageCount);
	Win::Logf("\t\tcurrentExtent = %d x %d\n", rhs.minImageExtent.width, rhs.currentExtent.height);
	Win::Logf("\t\tminImageExtent = %d x %d\n", rhs.currentExtent.width, rhs.minImageExtent.height);
	Win::Logf("\t\tmaxImageExtent = %d x %d\n", rhs.maxImageExtent.width, rhs.maxImageExtent.height);
	Win::Logf("\t\tmaxImageArrayLayers = %d\n", rhs.maxImageArrayLayers);
	Win::Log("\t\tsupportedTransforms = ");
#define VK_SURFACE_TRANSFORM_ENTRY(entry) if(rhs.supportedTransforms & VK_SURFACE_TRANSFORM_##entry##_BIT_KHR) { Win::Logf("%s | ", #entry); }
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
	Win::Log("\n");
	Win::Log("\t\tcurrentTransform = ");
#define VK_SURFACE_TRANSFORM_ENTRY(entry) if(rhs.currentTransform == VK_SURFACE_TRANSFORM_##entry##_BIT_KHR) { Win::Logf("%s\n", #entry); }
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
	Win::Log("\t\tsupportedCompositeAlpha = ");
#define VK_COMPOSITE_ALPHA_ENTRY(entry) if(rhs.supportedCompositeAlpha & VK_COMPOSITE_ALPHA_##entry##_BIT_KHR) { Win::Logf("%s | ", #entry); }
	VK_COMPOSITE_ALPHA_ENTRY(OPAQUE);
	VK_COMPOSITE_ALPHA_ENTRY(PRE_MULTIPLIED);
	VK_COMPOSITE_ALPHA_ENTRY(POST_MULTIPLIED);
	VK_COMPOSITE_ALPHA_ENTRY(INHERIT);
#undef VK_COMPOSITE_ALPHA_ENTRY
	Win::Log("\n");
	Win::Log("\t\tsupportedUsageFlags = ");
#define VK_IMAGE_USAGE_ENTRY(entry) if(rhs.supportedUsageFlags & VK_IMAGE_USAGE_##entry) { Win::Logf("%s | ", #entry); }
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
	Win::Log("\n");
	return lhs;
}
#pragma endregion

#ifdef USE_PIPELINE_SERIALIZE
static std::ostream& operator<<(std::ostream& lhs, const VK::PipelineCacheData& rhs) {
	Win::Log("PipelineCacheSerializer\n");
	Win::Logf("\tSize = %d\n", rhs.Size);
	Win::Logf("\tVersion = %s\n", rhs.Version == VK_PIPELINE_CACHE_HEADER_VERSION_ONE ? "VK_PIPELINE_CACHE_HEADER_VERSION_ONE" : "Unknown");
	Win::Logf("\tVenderID = %d\n", rhs.VenderID);
	Win::Logf("\tDeviceID = %d\n", rhs.DeviceID);
	Win::Log("\tUUID = "); for (auto i : rhs.UUID) { Win::Logf("%c", i); } Win::Log("\n");
	return lhs;
}
#endif

#endif