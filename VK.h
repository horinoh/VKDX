#pragma once

#include <fstream>

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
#include <glm/gtx/quaternion.hpp>
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
#define USE_RENDERDOC
#endif

#include "Cmn.h"
#ifdef _WINDOWS
#include "Win.h"
#endif

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
	static constexpr glm::vec3 Min(const glm::vec3& lhs, const glm::vec3& rhs) { return glm::vec3((std::min)(lhs.x, rhs.x), (std::min)(lhs.y, rhs.y), (std::min)(lhs.z, rhs.z)); }
	static constexpr glm::vec3 Max(const glm::vec3& lhs, const glm::vec3& rhs) { return glm::vec3((std::max)(lhs.x, rhs.x), (std::max)(lhs.y, rhs.y), (std::max)(lhs.z, rhs.z)); }

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
			constexpr std::array<VkMemoryBarrier, 0> MBs = {};
			constexpr std::array<VkImageMemoryBarrier, 0> IMBs = {};
			{
				const std::array BMBs = {
					VkBufferMemoryBarrier({
						.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
						.pNext = nullptr,
						.srcAccessMask = 0, .dstAccessMask = VK_ACCESS_MEMORY_WRITE_BIT,
						.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED, .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
						.buffer = Buffer, .offset = 0, .size = VK_WHOLE_SIZE
					}),
				};
				vkCmdPipelineBarrier(CB, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
					static_cast<uint32_t>(size(MBs)), data(MBs),
					static_cast<uint32_t>(size(BMBs)), data(BMBs),
					static_cast<uint32_t>(size(IMBs)), data(IMBs));
			}
			const std::array BCs = { VkBufferCopy({.srcOffset = 0, .dstOffset = 0, .size = Size }), };
			vkCmdCopyBuffer(CB, Staging, Buffer, static_cast<uint32_t>(size(BCs)), data(BCs));
			{
				const std::array BMBs = {
					VkBufferMemoryBarrier({
						.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
						.pNext = nullptr,
						.srcAccessMask = VK_ACCESS_MEMORY_WRITE_BIT, .dstAccessMask = AF,
						.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED, .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
						.buffer = Buffer, .offset = 0, .size = VK_WHOLE_SIZE
					}),
				};
				vkCmdPipelineBarrier(CB, VK_PIPELINE_STAGE_TRANSFER_BIT, PSF, 0,
					static_cast<uint32_t>(size(MBs)), data(MBs),
					static_cast<uint32_t>(size(BMBs)), data(BMBs),
					static_cast<uint32_t>(size(IMBs)), data(IMBs));
			}
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
			VK::CreateBufferMemory(&Buffer, &DeviceMemory, Device, PDMP, Size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,  VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, Source);
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
		DeviceLocalUniformTexelBuffer& Create(const VkDevice Device, const VkPhysicalDeviceMemoryProperties PDMP, const size_t Size, const VkBufferUsageFlagBits AdditionalUsage = static_cast< VkBufferUsageFlagBits>(0)) {
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
			const std::array IMBs = {
			   //!< UNDEFINED -> Layout
			   VkImageMemoryBarrier({
				   .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
				   .pNext = nullptr,
				   .srcAccessMask = 0, .dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
				   .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED, .newLayout = Layout,
				   .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED, .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
				   .image = Image,
				   .subresourceRange = VkImageSubresourceRange({.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .baseMipLevel = 0, .levelCount = 1, .baseArrayLayer = 0, .layerCount = 1 })
			   }),
			};
			vkCmdPipelineBarrier(CB, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, nullptr, 0, nullptr, static_cast<uint32_t>(size(IMBs)), data(IMBs));
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
			Super::Create(Device, PDMP, Format, Extent, 1, 1, VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT, VK_IMAGE_ASPECT_COLOR_BIT);
			return *this;
		}
	};

#ifdef _WINDOWS
	virtual void OnCreate(HWND hWnd, HINSTANCE hInstance, LPCWSTR Title) override;
	virtual void OnExitSizeMove(HWND hWnd, HINSTANCE hInstance) override;
	virtual void OnPaint(HWND hWnd, HINSTANCE hInstance) override { Super::OnPaint(hWnd, hInstance); Draw(); }
	virtual void OnPreDestroy(HWND hWnd, HINSTANCE hInstance) override;
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
	virtual void CopyToHostVisibleDeviceMemory(const VkDeviceMemory DeviceMemory, const VkDeviceSize Offset, const VkDeviceSize Size, const void* Source, const VkDeviceSize MappedRangeOffset = 0, const VkDeviceSize MappedRangeSize = VK_WHOLE_SIZE);

#pragma region COMMAND
	static void PopulateCopyBufferToImageCommand(const VkCommandBuffer CB, const VkBuffer Src, const VkImage Dst, const VkAccessFlags AF, const VkImageLayout IL, const VkPipelineStageFlags PSF, const std::vector<VkBufferImageCopy>& BICs, const uint32_t Levels, const uint32_t Layers);
	//static void PopulateCopyImageToBufferCommand(const VkCommandBuffer CB, const VkImage Src, const VkBuffer Dst, const VkAccessFlags AF, const VkImageLayout IL, const VkPipelineStageFlags PSF, const std::vector<VkBufferImageCopy>& BICs, const uint32_t Levels, const uint32_t Layers);
	static void SubmitAndWait(const VkQueue Queue, const VkCommandBuffer CB);
	static void PopulateBeginRenderTargetCommand(const VkCommandBuffer CB, const VkImage RenderTarget) {
		constexpr auto ISR = VkImageSubresourceRange({ .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .baseMipLevel = 0, .levelCount = 1, .baseArrayLayer = 0, .layerCount = 1 });
		const std::array IMBs = {
			//!< RenderTarget : TRANSFER_SRC_OPTIMAL -> GENERAL
			VkImageMemoryBarrier({
				.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
				.pNext = nullptr,
				.srcAccessMask = 0, .dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT,
				.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, .newLayout = VK_IMAGE_LAYOUT_GENERAL,
				.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED, .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
				.image = RenderTarget,
				.subresourceRange = ISR
			})
		};
		vkCmdPipelineBarrier(CB, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, nullptr, 0, nullptr, static_cast<uint32_t>(size(IMBs)), data(IMBs));
	}
	static void PopulateEndRenderTargetCommand(const VkCommandBuffer CB, const VkImage RenderTarget, const VkImage Swapchain, const uint32_t Width, const uint32_t Height) {
		constexpr auto ISR = VkImageSubresourceRange({ .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .baseMipLevel = 0, .levelCount = 1, .baseArrayLayer = 0, .layerCount = 1 });
		{
			const std::array IMBs = {
				//!< RenderTarget : GENERAL -> TRANSFER_SRC_OPTIMAL
				VkImageMemoryBarrier({
					.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
					.pNext = nullptr,
					.srcAccessMask = 0, .dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT,
					.oldLayout = VK_IMAGE_LAYOUT_GENERAL, .newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
					.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED, .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
					.image = RenderTarget,
					.subresourceRange = ISR
				}),
				//!< Swapchain : PRESENT_SRC_KHR -> TRANSFER_DST_OPTIMAL
				VkImageMemoryBarrier({
					.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
					.pNext = nullptr,
					.srcAccessMask = 0, .dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
					.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED, .newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
					.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED, .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
					.image = Swapchain,
					.subresourceRange = ISR
				}),
			};
			vkCmdPipelineBarrier(CB, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, nullptr, 0, nullptr, static_cast<uint32_t>(size(IMBs)), data(IMBs));
		}

		const std::array ICs = {
			VkImageCopy({
				.srcSubresource = VkImageSubresourceLayers({.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .mipLevel = 0, .baseArrayLayer = 0, .layerCount = 1 }),
				.srcOffset = VkOffset3D({.x = 0, .y = 0, .z = 0}),
				.dstSubresource = VkImageSubresourceLayers({.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .mipLevel = 0, .baseArrayLayer = 0, .layerCount = 1 }),
				.dstOffset = VkOffset3D({.x = 0, .y = 0, .z = 0}),
				.extent = VkExtent3D({.width = Width, .height = Height, .depth = 1 }),
			}),
		};
		vkCmdCopyImage(CB, RenderTarget, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, Swapchain, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, static_cast<uint32_t>(size(ICs)), data(ICs));

		{
			const std::array IMBs = {
				//!< Swapchain : TRANSFER_DST_OPTIMAL -> PRESENT_SRC_KHR, 
				VkImageMemoryBarrier({
					.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
					.pNext = nullptr,
					.srcAccessMask = 0, .dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
					.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, .newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
					.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED, .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
					.image = Swapchain,
					.subresourceRange = ISR
				}),
			};
			vkCmdPipelineBarrier(CB, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, nullptr, 0, nullptr, static_cast<uint32_t>(size(IMBs)), data(IMBs));
		}
	}
#pragma endregion

	static void EnumerateMemoryRequirements(const VkMemoryRequirements& MR, const VkPhysicalDeviceMemoryProperties& PDMP);

#pragma region MARKER
	static void MarkerInsert([[maybe_unused]] VkCommandBuffer CB, [[maybe_unused]] const glm::vec4& Color, [[maybe_unused]] std::string_view Name) {
#ifdef USE_RENDERDOC
		if (VK_NULL_HANDLE != vkCmdDebugMarkerInsert) {
			const VkDebugMarkerMarkerInfoEXT DMMI = {
				.sType = VK_STRUCTURE_TYPE_DEBUG_MARKER_MARKER_INFO_EXT,
				.pNext = nullptr,
				.pMarkerName = data(Name),
				.color = { Color.r, Color.g, Color.b, Color.a }
			};
			vkCmdDebugMarkerInsert(CB, &DMMI);
		}
#endif
	}
	static void MarkerBegin([[maybe_unused]] VkCommandBuffer CB, [[maybe_unused]] const glm::vec4& Color, [[maybe_unused]] std::string_view Name) {
#ifdef USE_RENDERDOC
		if (VK_NULL_HANDLE != vkCmdDebugMarkerBegin) {
			const VkDebugMarkerMarkerInfoEXT DMMI = {
				.sType = VK_STRUCTURE_TYPE_DEBUG_MARKER_MARKER_INFO_EXT,
				.pNext = nullptr,
				.pMarkerName = data(Name),
				.color = { Color.r, Color.g, Color.b, Color.a }
			};
			vkCmdDebugMarkerBegin(CB, &DMMI);
		}
#endif
	}
	static void MarkerEnd([[maybe_unused]] VkCommandBuffer CB) {
#ifdef USE_RENDERDOC
		if (VK_NULL_HANDLE != vkCmdDebugMarkerEnd) { vkCmdDebugMarkerEnd(CB); }
#endif
	}
	class ScopedMarker
	{
	public:
		ScopedMarker(VkCommandBuffer CB, const glm::vec4& Color, const std::string_view Name) : CommandBuffer(CB) { MarkerBegin(CommandBuffer, Color, Name); }
		~ScopedMarker() { MarkerEnd(CommandBuffer); }
	private:
		VkCommandBuffer CommandBuffer;
	};
	static void MarkerSetName([[maybe_unused]]VkDevice Device, [[maybe_unused]] const VkDebugReportObjectTypeEXT Type, [[maybe_unused]] const uint64_t Object, [[maybe_unused]] std::string_view Name) {
#ifdef USE_RENDERDOC
		if (VK_NULL_HANDLE != vkDebugMarkerSetObjectName) {
			const VkDebugMarkerObjectNameInfoEXT DMONI = {
				.sType = VK_STRUCTURE_TYPE_DEBUG_MARKER_OBJECT_NAME_INFO_EXT,
				.pNext = nullptr,
				.objectType = Type, .object = Object, .pObjectName = data(Name)
			};
			VERIFY_SUCCEEDED(vkDebugMarkerSetObjectName(Device, &DMONI));
		}
#endif
	}
	static void MarkerSetTag([[maybe_unused]] VkDevice Device, [[maybe_unused]] const VkDebugReportObjectTypeEXT Type, [[maybe_unused]] const uint64_t Object, [[maybe_unused]] const uint64_t TagName, [[maybe_unused]] const size_t TagSize, [[maybe_unused]] const void* TagData) {
#ifdef USE_RENDERDOC
		if (VK_NULL_HANDLE != vkDebugMarkerSetObjectTag) {
			const VkDebugMarkerObjectTagInfoEXT DMOTI = {
				.sType = VK_STRUCTURE_TYPE_DEBUG_MARKER_OBJECT_TAG_INFO_EXT,
				.pNext = nullptr,
				.objectType = Type, .object = Object,
				.tagName = TagName, .tagSize = TagSize, .pTag = TagData
			};
			VERIFY_SUCCEEDED(vkDebugMarkerSetObjectTag(Device, &DMOTI));
		}
#endif
	}
	static void MarkerSetTag(VkDevice Device, const VkDebugReportObjectTypeEXT Type, const uint64_t Object, const uint64_t TagName, const std::vector<std::byte>& TagData) { MarkerSetTag(Device, Type, Object, TagName, size(TagData), data(TagData)); }
#pragma region MARKER_TEMPLATE
	template<typename T> static void MarkerSetObjectName(VkDevice Device, T Object, const std::string_view Name) { DEBUG_BREAK(); /* テンプレート特殊化されていない (Not template specialized) */ }
	template<typename T> static void MarkerSetObjectTag(VkDevice Device, T Object, const uint64_t TagName, const size_t TagSize, const void* TagData) { DEBUG_BREAK(); /* テンプレート特殊化されていない (Not template specialized) */ }
	//!< ↓ここでテンプレート特殊化している (Template specialization here)
#include "VKDebugMarker.inl"
#pragma endregion
#pragma endregion

protected:
	void LoadVulkanLibrary();

	virtual void CreateDebugReportCallback();
	virtual void CreateInstance(const std::vector<const char*>& AdditionalLayers = {}, const std::vector<const char*>& AdditionalExtensions = {});

	virtual void SelectPhysicalDevice(VkInstance Instance);
	virtual void CreateDevice(HWND hWnd, HINSTANCE hInstance, void* pNext = nullptr, const std::vector<const char*>& AdditionalExtensions = {});

	virtual void CreateFence(VkDevice Dev);

	virtual void AllocatePrimaryCommandBuffer();
	virtual void AllocateSecondaryCommandBuffer();
	virtual void AllocateComputeCommandBuffer();
	virtual void AllocateCommandBuffer() {
		AllocatePrimaryCommandBuffer();
		AllocateSecondaryCommandBuffer();
		AllocateComputeCommandBuffer();
	}

	[[nodiscard]] virtual VkSurfaceFormatKHR SelectSurfaceFormat(VkPhysicalDevice PD, VkSurfaceKHR Surface);
	[[nodiscard]] virtual VkPresentModeKHR SelectSurfacePresentMode(VkPhysicalDevice PD, VkSurfaceKHR Surface);

	virtual void CreateSwapchain() { CreateSwapchain(GetCurrentPhysicalDevice(), Surface, GetClientRectWidth(), GetClientRectHeight()); }
	virtual void CreateSwapchain(VkPhysicalDevice PD, VkSurfaceKHR Sfc, const uint32_t Width, const uint32_t Height, const VkImageUsageFlags AdditionalUsage = 0);
	virtual void ResizeSwapchain(const uint32_t Width, const uint32_t Height);

	virtual void CreateViewport(const float Width, const float Height, const float MinDepth = 0.0f, const float MaxDepth = 1.0f);

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
	virtual void CreatePipelineLayout() { PipelineLayouts.emplace_back(VkPipelineLayout()); CreatePipelineLayout(PipelineLayouts.back(), {}, {}); }

	virtual void CreateDescriptorPool(VkDescriptorPool& DP, const VkDescriptorPoolCreateFlags Flags, const std::vector<VkDescriptorPoolSize>& DPSs);
	virtual void CreateDescriptorUpdateTemplate(VkDescriptorUpdateTemplate& DUT, const VkPipelineBindPoint PBP, const std::vector<VkDescriptorUpdateTemplateEntry>& DUTEs, const VkDescriptorSetLayout DSL);
	virtual void CreateDescriptor() {}
	virtual void CreateShaderBindingTable() {}

	virtual void CreateTexture() {}
	virtual void CreateTextureArray1x1(const std::vector<uint32_t>& Colors, const VkPipelineStageFlags PSF);
	virtual void CreateImmutableSampler() {}

	virtual void CreateRenderPass(VkRenderPass& RP, const std::vector<VkAttachmentDescription>& ADs, const std::vector<VkSubpassDescription> &SDs, const std::vector<VkSubpassDependency>& Deps);
	virtual void CreateRenderPass();

	virtual void CreateFramebuffer(VkFramebuffer& FB, const VkRenderPass RP, const uint32_t Width, const uint32_t Height, const uint32_t Layers, const std::vector<VkImageView>& IVs);
	virtual void CreateFramebuffer() {
		const auto RP = RenderPasses[0];
		for (auto i : SwapchainImageViews) {
			VK::CreateFramebuffer(Framebuffers.emplace_back(), RP, SurfaceExtent2D.width, SurfaceExtent2D.height, 1, { i });
		}
	}

	[[nodiscard]] virtual VkShaderModule CreateShaderModule(const std::filesystem::path& Path) const;

#ifdef USE_PIPELINE_SERIALIZE
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
					auto Data = new char[Size];
					In.read(Data, Size);
					assert(Size == *reinterpret_cast<const uint32_t*>(Data) && "");
					assert(VK_PIPELINE_CACHE_HEADER_VERSION_ONE == *(reinterpret_cast<const uint32_t*>(Data) + 1) && "");
					{
//#ifdef DEBUG_STDOUT
//						std::cout << *reinterpret_cast<const PipelineCacheData*>(Data);
//#endif
						const auto PCD = reinterpret_cast<const PipelineCacheData*>(Data);
						Win::Log("PipelineCacheSerializer\n");
						Win::Logf("\tSize = %d\n", PCD->Size);
						Win::Logf("\tVersion = %s\n", PCD->Version == VK_PIPELINE_CACHE_HEADER_VERSION_ONE ? "VK_PIPELINE_CACHE_HEADER_VERSION_ONE" : "Unknown");
						Win::Logf("\tVenderID = %d\n", PCD->VenderID);
						Win::Logf("\tDeviceID = %d\n", PCD->DeviceID);
						Win::Log("\tUUID = "); for (auto i : PCD->UUID) { Win::Logf("%c", i); } Win::Log("\n");
					}
					PipelineCaches.resize(1); //!< 書き込む際にマージされているはずなので読み込めた場合は1つで良い
					const VkPipelineCacheCreateInfo PCCI = { .sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO, .pNext = nullptr, .flags = 0, .initialDataSize = Size, .pInitialData = Data };
					VERIFY_SUCCEEDED(vkCreatePipelineCache(Device, &PCCI, nullptr, data(PipelineCaches)));
					delete[] Data;
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
					auto Data = new char[Size];
					VERIFY_SUCCEEDED(vkGetPipelineCacheData(Device, PipelineCaches.back(), &Size, Data));
					std::ofstream Out(data(FilePath.string()), std::ios::out | std::ios::binary);
					if (!Out.fail()) {
						Out.write(Data, Size);
						Out.close();
					}
					delete[] Data;
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
#endif
	virtual void CreatePipeline() {}
	static void CreatePipeline_(VkPipeline& PL,
		const VkDevice Dev, const VkPipelineLayout PLL, const VkRenderPass RP,
		const VkPrimitiveTopology Topology, const uint32_t PatchControlPoints,
		const VkPipelineRasterizationStateCreateInfo& PRSCI,
		const VkPipelineDepthStencilStateCreateInfo& PDSSCI,
		const VkPipelineShaderStageCreateInfo* VS, const VkPipelineShaderStageCreateInfo* FS, const VkPipelineShaderStageCreateInfo* TES, const VkPipelineShaderStageCreateInfo* TCS, const VkPipelineShaderStageCreateInfo* GS,
		const std::vector<VkVertexInputBindingDescription>& VIBDs, const std::vector<VkVertexInputAttributeDescription>& VIADs,
		const std::vector<VkPipelineColorBlendAttachmentState>& PCBASs,
		VkPipelineCache PC = VK_NULL_HANDLE);
	static void CreatePipeline__(VkPipeline& PL,
		const VkDevice Dev, const VkPipelineLayout PLL, const VkRenderPass RP,
		const VkPipelineRasterizationStateCreateInfo& PRSCI,
		const VkPipelineDepthStencilStateCreateInfo& PDSSCI,
		const VkPipelineShaderStageCreateInfo* TS, const VkPipelineShaderStageCreateInfo* MS, const VkPipelineShaderStageCreateInfo* FS,
		const std::vector<VkPipelineColorBlendAttachmentState>& PCBASs,
		VkPipelineCache PC = VK_NULL_HANDLE);

	virtual void PopulateCommandBuffer([[maybe_unused]] const size_t i) {}

	virtual uint32_t GetCurrentBackBufferIndex() const { return SwapchainImageIndex; }
	virtual void DrawFrame([[maybe_unused]] const uint32_t i) {}
	static void WaitForFence(VkDevice Device, VkFence Fence);
	virtual void SubmitGrapphics(const uint32_t i);
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

#define VK_PROC_ADDR(proc) static PFN_vk ## proc ## EXT vk ## proc;
#include "VKInstanceProcAddr_DebugReport.h"
#include "VKDeviceProcAddr_DebugMarker.h"
#undef VK_PROC_ADDR

#pragma region DebugUtils
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
#pragma endregion

protected:
	VkInstance Instance = VK_NULL_HANDLE;
	VkDebugReportCallbackEXT DebugReportCallback = VK_NULL_HANDLE;
	VkDebugUtilsMessengerEXT DebugUtilsMessenger = VK_NULL_HANDLE;
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

	VkExtent2D SurfaceExtent2D;
	VkFormat ColorFormat = VK_FORMAT_B8G8R8A8_UNORM;
	VkSwapchainKHR Swapchain = VK_NULL_HANDLE;
	std::vector<VkImage> SwapchainImages;
	uint32_t SwapchainImageIndex = 0;
	std::vector<VkImageView> SwapchainImageViews;

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

	std::vector<VkDescriptorSetLayout> DescriptorSetLayouts;
	std::vector<VkPipelineLayout> PipelineLayouts;

	std::vector<VkRenderPass> RenderPasses;
	std::vector<VkPipeline> Pipelines;
	std::vector<VkFramebuffer> Framebuffers;

	std::vector<VkDescriptorPool> DescriptorPools;
	std::vector<VkDescriptorSet> DescriptorSets;

	//!< VKの場合、通常サンプラ、イミュータブルサンプラとも同様に VkSampler を作成する、デスクリプタセットの指定が異なるだけ
	std::vector<VkSampler> Samplers;

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