#pragma once

#include <fstream>

//!< VK_NO_PROTOYYPES が定義されてる場合は DLL を使用する。(If VK_NO_PROTOYYPES is defined, using DLL)
//!< VK_NO_PROTOYYPES は VK.props 内に定義してある (VK_NO_PROTOYYPES is defined is in VK.props here)

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

#ifndef BREAK_ON_FAILED
#define BREAK_ON_FAILED(vr) if(VK_SUCCESS != (vr)) { Log(VK::GetVkResultChar(vr)); DEBUG_BREAK(); }
#endif
#ifndef THROW_ON_FAILED
#define THROW_ON_FAILED(vr) if(VK_SUCCESS != (vr)) { throw std::runtime_error("VERIFY_SUCCEEDED failed : " + std::string(VK::GetVkResultChar(vr));; }
#endif
#ifndef MESSAGEBOX_ON_FAILED
#define MESSAGEBOX_ON_FAILED(vr) if(VK_SUCCESS != (vr)) { Win::ShowMessageBox(nullptr, VK::GetVkResultChar(vr)); }
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
//#define USE_SUBPASS

#ifdef _DEBUG
#define USE_DEBUG_REPORT
#define USE_RENDERDOC
#ifdef USE_RENDERDOC
#define USE_DEBUG_MARKER
#endif
#endif //!< _DEBUG

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
		void Create(const VkDevice Device, const VkPhysicalDeviceMemoryProperties PDMP, const VkBufferUsageFlags BUF, const size_t Size, const VkMemoryPropertyFlags MPF, const void* Source = nullptr) {
			VK::CreateBufferMemory(&Buffer, &DeviceMemory, Device, PDMP, BUF, Size, MPF, Source);
		}
		virtual void Destroy(const VkDevice Device) override {
			Super::Destroy(Device);
			if (VK_NULL_HANDLE != Buffer) { vkDestroyBuffer(Device, Buffer, GetAllocationCallbacks()); }
		}
	};
	class VertexBuffer : public BufferMemory
	{
	private:
		using Super = BufferMemory;
	public:
		VertexBuffer& Create(const VkDevice Device, const VkPhysicalDeviceMemoryProperties PDMP, const size_t Size) {
			VK::CreateBufferMemory(&Buffer, &DeviceMemory, Device, PDMP, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, Size, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
			return *this;
		}
		void PopulateCopyCommand(const VkCommandBuffer CB, const size_t Size, const VkBuffer Staging) {
			PopulateCommandBuffer_CopyBufferToBuffer(CB, Staging, Buffer, VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT, VK_PIPELINE_STAGE_VERTEX_INPUT_BIT, Size);
		}
		void SubmitCopyCommand(const VkDevice Device, const VkPhysicalDeviceMemoryProperties PDMP, const VkCommandBuffer CB, const VkQueue Queue, const size_t Size, const void* Source) {
			VK::Scoped<BufferMemory> StagingBuffer(Device);
			StagingBuffer.Create(Device, PDMP, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, Size, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, Source);
			constexpr VkCommandBufferBeginInfo CBBI = { .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO, .pNext = nullptr, .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, .pInheritanceInfo = nullptr };
			VERIFY_SUCCEEDED(vkBeginCommandBuffer(CB, &CBBI)); {
				PopulateCopyCommand(CB, Size, StagingBuffer.Buffer);
			} VERIFY_SUCCEEDED(vkEndCommandBuffer(CB));
			VK::SubmitAndWait(Queue, CB);
		}
	};
	class IndexBuffer : public BufferMemory
	{
	private:
		using Super = BufferMemory;
	public:
		IndexBuffer& Create(const VkDevice Device, const VkPhysicalDeviceMemoryProperties PDMP, const size_t Size) {
			VK::CreateBufferMemory(&Buffer, &DeviceMemory, Device, PDMP, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, Size, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
			return *this;
		}
		void PopulateCopyCommand(const VkCommandBuffer CB, const size_t Size, const VkBuffer Staging) {
			PopulateCommandBuffer_CopyBufferToBuffer(CB, Staging, Buffer, VK_ACCESS_INDEX_READ_BIT, VK_PIPELINE_STAGE_VERTEX_INPUT_BIT, Size);
		}
		void SubmitCopyCommand(const VkDevice Device, const VkPhysicalDeviceMemoryProperties PDMP, const VkCommandBuffer CB, const VkQueue Queue, const size_t Size, const void* Source) {
			VK::Scoped<BufferMemory> StagingBuffer(Device);
			StagingBuffer.Create(Device, PDMP, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, Size, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, Source);
			constexpr VkCommandBufferBeginInfo CBBI = { .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO, .pNext = nullptr, .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, .pInheritanceInfo = nullptr };
			VERIFY_SUCCEEDED(vkBeginCommandBuffer(CB, &CBBI)); {
				PopulateCopyCommand(CB, Size, StagingBuffer.Buffer);
			} VERIFY_SUCCEEDED(vkEndCommandBuffer(CB));
			VK::SubmitAndWait(Queue, CB);
		}
	};
	class IndirectBuffer : public BufferMemory
	{
	private:
		using Super = BufferMemory;
	public:
		IndirectBuffer& Create(const VkDevice Device, const VkPhysicalDeviceMemoryProperties PDMP, const VkDrawIndexedIndirectCommand& DIIC) {
			VK::CreateBufferMemory(&Buffer, &DeviceMemory, Device, PDMP, VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, sizeof(DIIC), VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
			return *this;
		}
		IndirectBuffer& Create(const VkDevice Device, const VkPhysicalDeviceMemoryProperties PDMP, const VkDrawIndirectCommand& DIC) {
			VK::CreateBufferMemory(&Buffer, &DeviceMemory, Device, PDMP, VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, sizeof(DIC), VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
			return *this;
		}
		IndirectBuffer& Create(const VkDevice Device, const VkPhysicalDeviceMemoryProperties PDMP, const VkDispatchIndirectCommand& DIC) {
			VK::CreateBufferMemory(&Buffer, &DeviceMemory, Device, PDMP, VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, sizeof(DIC), VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
			return *this;
		}
#pragma region MESH_SHADER
		IndirectBuffer& Create(const VkDevice Device, const VkPhysicalDeviceMemoryProperties PDMP, const VkDrawMeshTasksIndirectCommandNV& DMTIC) {
			VK::CreateBufferMemory(&Buffer, &DeviceMemory, Device, PDMP, VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, sizeof(DMTIC), VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
			return *this;
		}
#pragma endregion
		void PopulateCopyCommand(const VkCommandBuffer CB, const size_t Size, const VkBuffer Staging) {
			PopulateCommandBuffer_CopyBufferToBuffer(CB, Staging, Buffer, VK_ACCESS_INDIRECT_COMMAND_READ_BIT, VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT, Size);
		}
		void SubmitCopyCommand(const VkDevice Device, const VkPhysicalDeviceMemoryProperties PDMP, const VkCommandBuffer CB, const VkQueue Queue, const size_t Size, const void* Source) {
			VK::Scoped<BufferMemory> StagingBuffer(Device);
			StagingBuffer.Create(Device, PDMP, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, Size, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, Source);
			constexpr VkCommandBufferBeginInfo CBBI = { .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO, .pNext = nullptr, .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, .pInheritanceInfo = nullptr };
			VERIFY_SUCCEEDED(vkBeginCommandBuffer(CB, &CBBI)); {
				PopulateCopyCommand(CB, Size, StagingBuffer.Buffer);
			} VERIFY_SUCCEEDED(vkEndCommandBuffer(CB));
			VK::SubmitAndWait(Queue, CB);
		}
	};
	class UniformBuffer : public BufferMemory
	{
	private:
		using Super = BufferMemory;
	public:
		void Create(const VkDevice Device, const VkPhysicalDeviceMemoryProperties PDMP, const size_t Size, const void* Source = nullptr) {
			VK::CreateBufferMemory(&Buffer, &DeviceMemory, Device, PDMP, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, Size, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, Source);
		}
	};
	using StorageBuffer = BufferMemory;

	class Texture : public DeviceMemoryBase
	{
	private:
		using Super = DeviceMemoryBase;
	public:
		VkImage Image = VK_NULL_HANDLE;
		VkImageView View = VK_NULL_HANDLE;
		void Create(const VkDevice Device, const VkPhysicalDeviceMemoryProperties PDMP, const VkFormat Format, const VkExtent3D& Extent, const uint32_t MipLevels, const uint32_t ArrayLayers, const VkImageAspectFlags IAF) {
			VK::CreateImageMemory(&Image, &DeviceMemory, Device, PDMP, 0, VK_IMAGE_TYPE_2D, Format, Extent, MipLevels, ArrayLayers, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);
			//!< 基本 TYPE_2D, _TYPE_2D_ARRAY として扱う、それ以外で使用する場合は明示的に上書きして使う
			const VkImageViewCreateInfo IVCI = {
				.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
				.pNext = nullptr,
				.flags = 0,
				.image = Image,
				.viewType = ArrayLayers == 1 ? VK_IMAGE_VIEW_TYPE_2D : VK_IMAGE_VIEW_TYPE_2D_ARRAY,
				.format = Format,
				.components = VkComponentMapping({.r = VK_COMPONENT_SWIZZLE_R, .g = VK_COMPONENT_SWIZZLE_G, .b = VK_COMPONENT_SWIZZLE_B, .a = VK_COMPONENT_SWIZZLE_A }),
				.subresourceRange = VkImageSubresourceRange({.aspectMask = IAF, .baseMipLevel = 0, .levelCount = VK_REMAINING_MIP_LEVELS, .baseArrayLayer = 0, .layerCount = VK_REMAINING_ARRAY_LAYERS })
			};
			VERIFY_SUCCEEDED(vkCreateImageView(Device, &IVCI, GetAllocationCallbacks(), &View));
		}
		virtual void Destroy(const VkDevice Device) override {
			Super::Destroy(Device);
			if (VK_NULL_HANDLE != Image) { vkDestroyImage(Device, Image, GetAllocationCallbacks()); }
			if (VK_NULL_HANDLE != View) { vkDestroyImageView(Device, View, GetAllocationCallbacks()); }
		}
	};
	class DepthTexture : public Texture
	{
	private:
		using Super = Texture;
	public:
		void Create(const VkDevice Device, const VkPhysicalDeviceMemoryProperties PDMP, const VkFormat Format, const VkExtent3D& Extent) {
			VK::CreateImageMemory(&Image, &DeviceMemory, Device, PDMP, 0, VK_IMAGE_TYPE_2D, Format, Extent, 1, 1, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT/*| VK_IMAGE_USAGE_SAMPLED_BIT*/);
			const VkImageViewCreateInfo IVCI = {
				.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
				.pNext = nullptr,
				.flags = 0,
				.image = Image,
				.viewType = VK_IMAGE_VIEW_TYPE_2D,
				.format = Format,
				.components = VkComponentMapping({.r = VK_COMPONENT_SWIZZLE_R, .g = VK_COMPONENT_SWIZZLE_G, .b = VK_COMPONENT_SWIZZLE_B, .a = VK_COMPONENT_SWIZZLE_A }),
				.subresourceRange = VkImageSubresourceRange({.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT, .baseMipLevel = 0, .levelCount = VK_REMAINING_MIP_LEVELS, .baseArrayLayer = 0, .layerCount = VK_REMAINING_ARRAY_LAYERS })
			};
			VERIFY_SUCCEEDED(vkCreateImageView(Device, &IVCI, GetAllocationCallbacks(), &View));
		}
	};
	class RenderTexture : public Texture
	{
	private:
		using Super = Texture;
	public:
		void Create(const VkDevice Device, const VkPhysicalDeviceMemoryProperties PDMP, const VkFormat Format, const VkExtent3D& Extent) {
			VK::CreateImageMemory(&Image, &DeviceMemory, Device, PDMP, 0, VK_IMAGE_TYPE_2D, Format, Extent, 1, 1, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
			const VkImageViewCreateInfo IVCI = {
				.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
				.pNext = nullptr,
				.flags = 0,
				.image = Image,
				.viewType = VK_IMAGE_VIEW_TYPE_2D,
				.format = Format,
				.components = VkComponentMapping({.r = VK_COMPONENT_SWIZZLE_R, .g = VK_COMPONENT_SWIZZLE_G, .b = VK_COMPONENT_SWIZZLE_B, .a = VK_COMPONENT_SWIZZLE_A }),
				.subresourceRange = VkImageSubresourceRange({.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .baseMipLevel = 0, .levelCount = VK_REMAINING_MIP_LEVELS, .baseArrayLayer = 0, .layerCount = VK_REMAINING_ARRAY_LAYERS })
			};
			VERIFY_SUCCEEDED(vkCreateImageView(Device, &IVCI, GetAllocationCallbacks(), &View));
		}
	};

#pragma region RAYTRACING
	class AccelerationStructureBuffer : public BufferMemory
	{
	public:
		VkAccelerationStructureKHR AccelerationStructure;
		void Create(const VkDevice Device, const VkPhysicalDeviceMemoryProperties PDMP, const VkAccelerationStructureTypeKHR Type, const size_t Size) {
			VK::CreateBufferMemory(&Buffer, &DeviceMemory, Device, PDMP, VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, Size, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
			const VkAccelerationStructureCreateInfoKHR ASCI = {
				.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR,
				.pNext = nullptr,
				.createFlags = 0,
				.buffer = Buffer,
				.offset = 0,
				.size = Size,
				.type = Type,
				.deviceAddress = 0
			};
			vkCreateAccelerationStructureKHR(Device, &ASCI, GetAllocationCallbacks(), &AccelerationStructure);
		}
		void Destroy(const VkDevice Device) {
			if (VK_NULL_HANDLE != AccelerationStructure) { vkDestroyAccelerationStructureKHR(Device, AccelerationStructure, GetAllocationCallbacks()); }
			BufferMemory::Destroy(Device);
		}
	};
	class ScratchBuffer : public BufferMemory
	{
	public:
		void Create(const VkDevice Device, const VkPhysicalDeviceMemoryProperties PDMP, const size_t Size) {
			VK::CreateBufferMemory(&Buffer, &DeviceMemory, Device, PDMP, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, Size, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		}
	};
	using ShaderBindingTable = BufferMemory;
#pragma endregion

#ifdef _WINDOWS
	virtual void OnCreate(HWND hWnd, HINSTANCE hInstance, LPCWSTR Title) override;
	virtual void OnExitSizeMove(HWND hWnd, HINSTANCE hInstance) override;
	virtual void OnPaint(HWND hWnd, HINSTANCE hInstance) override { Super::OnPaint(hWnd, hInstance); Draw(); }
	virtual void OnPreDestroy(HWND hWnd, HINSTANCE hInstance) override;
	virtual void OnDestroy(HWND hWnd, HINSTANCE hInstance) override;
#endif

	static [[nodiscard]] const char* GetVkResultChar(const VkResult Result);
	static [[nodiscard]] const char* GetFormatChar(const VkFormat Format);
	static [[nodiscard]] const char* GetColorSpaceChar(const VkColorSpaceKHR ColorSpace);
	static [[nodiscard]] const char* GetImageViewTypeChar(const VkImageViewType ImageViewType);
	static [[nodiscard]] const char* GetComponentSwizzleChar(const VkComponentSwizzle ComponentSwizzle);
	static [[nodiscard]] std::string GetComponentMappingString(const VkComponentMapping& CM) {
		return std::string(GetComponentSwizzleChar(CM.r)) + ", " + GetComponentSwizzleChar(CM.g) + ", " + GetComponentSwizzleChar(CM.b) + ", " + GetComponentSwizzleChar(CM.a);
	}
	static [[nodiscard]] const char* GetSystemAllocationScopeChar(const VkSystemAllocationScope SAS);

	static [[nodiscard]] std::array<float, 3> Lerp(const std::array<float, 3>& lhs, const std::array<float, 3>& rhs, const float t) {
		const auto v = glm::mix(*reinterpret_cast<const glm::vec3*>(data(lhs)), *reinterpret_cast<const glm::vec3*>(data(rhs)), t);
		return { v.x, v.y, v.z };
	}
	static [[nodiscard]] std::array<float, 4> Lerp(const std::array<float, 4>& lhs, const std::array<float, 4>& rhs, const float t) {
		const auto v = glm::lerp(*reinterpret_cast<const glm::quat*>(data(lhs)), *reinterpret_cast<const glm::quat*>(data(rhs)), t);
		return { v.x, v.y, v.z, v.w };
	}

	static [[nodiscard]] bool IsSupportedColorFormat(VkPhysicalDevice PD, const VkFormat Format) {
		VkFormatProperties FP;
		vkGetPhysicalDeviceFormatProperties(PD, Format, &FP);
		if (!(FP.optimalTilingFeatures & VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT)) {
			Error("VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT not supported\n");
			return false;
		}
		return true;
	}
	static [[nodiscard]] bool IsSupportedDepthFormat(VkPhysicalDevice PD, const VkFormat Format) {
		VkFormatProperties FP;
		vkGetPhysicalDeviceFormatProperties(PD, Format, &FP);
		if (!(FP.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)) {
			Error("VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT not supported\n");
			return false;
		}
		return true;
	}
	static [[nodiscard]] bool IsSupportedSampledImage(VkPhysicalDevice PD, const VkFormat Format, const VkFilter Mag = VK_FILTER_LINEAR, const VkFilter Min = VK_FILTER_LINEAR, const VkSamplerMipmapMode Mip = VK_SAMPLER_MIPMAP_MODE_LINEAR) {
		VkFormatProperties FP;
		vkGetPhysicalDeviceFormatProperties(PD, Format, &FP);
		if (!(FP.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT)) {
			Error("VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT not supported\n");
			return false;
		}
		if (VK_FILTER_LINEAR == Mag || VK_FILTER_LINEAR == Min || VK_SAMPLER_MIPMAP_MODE_LINEAR == Mip) {
			if (!(FP.linearTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT)) {
				Error("VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT not supported\n");
				return false;
			}
		}
		return true;
	}
#pragma region RAYTRACING
	static [[nodiscard]] bool HasRayTracingSupport(const VkPhysicalDevice PD) {
		VkPhysicalDeviceBufferDeviceAddressFeatures PDBDAF = { .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES, .pNext = nullptr };
		VkPhysicalDeviceRayTracingPipelineFeaturesKHR PDRTPF = { .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR, .pNext = &PDBDAF };
		VkPhysicalDeviceAccelerationStructureFeaturesKHR PDASF = { .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR, .pNext = &PDRTPF };
		VkPhysicalDeviceFeatures2 PDF2 = { .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2, .pNext = &PDASF };
		vkGetPhysicalDeviceFeatures2(PD, &PDF2);
		return PDBDAF.bufferDeviceAddress && PDRTPF.rayTracingPipeline && PDASF.accelerationStructure;
	}
#pragma endregion
#pragma region MESH_SHADER
	static [[nodiscard]] bool HasMeshShaderSupport(const VkPhysicalDevice PD) {
		VkPhysicalDeviceMeshShaderFeaturesNV PDMSF = { .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_FEATURES_NV, .pNext = nullptr };
		VkPhysicalDeviceFeatures2 PDF2 = { .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2, .pNext = &PDMSF };
		vkGetPhysicalDeviceFeatures2(PD, &PDF2);
		return PDMSF.taskShader && PDMSF.meshShader;
	}
#pragma endregion

protected:	
	static [[nodiscard]] uint32_t GetMemoryTypeIndex(const VkPhysicalDeviceMemoryProperties& PDMP, const uint32_t TypeBits, const VkMemoryPropertyFlags MPF);

	void AllocateDeviceMemory(VkDeviceMemory* DM, const VkMemoryRequirements& MR, const VkMemoryPropertyFlags MPF);
	void AllocateDeviceMemory(VkDeviceMemory* DM, const VkBuffer Buffer, const VkMemoryPropertyFlags MPF) { VkMemoryRequirements MR; vkGetBufferMemoryRequirements(Device, Buffer, &MR); AllocateDeviceMemory(DM, MR, MPF); }
	void AllocateDeviceMemory(VkDeviceMemory* DM, const VkImage Image, const VkMemoryPropertyFlags MPF) { VkMemoryRequirements MR; vkGetImageMemoryRequirements(Device, Image, &MR); AllocateDeviceMemory(DM, MR, MPF); }

	//virtual void CreateBuffer(VkBuffer* Buffer, const VkBufferUsageFlags Usage, const size_t Size) const;
	//virtual void CreateImage(VkImage* Image, const VkImageCreateFlags CreateFlags, const VkImageType ImageType, const VkFormat Format, const VkExtent3D& Extent3D, const uint32_t MipLevels, const uint32_t ArrayLayers, const VkSampleCountFlagBits SampleCount, const VkImageUsageFlags Usage) const;

	[[deprecated("")]]
	virtual void CopyToHostVisibleDeviceMemory(const VkDeviceMemory DeviceMemory, const VkDeviceSize Offset, const VkDeviceSize Size, const void* Source, const VkDeviceSize MappedRangeOffset = 0, const VkDeviceSize MappedRangeSize = VK_WHOLE_SIZE);

#pragma region COMMAND
	static void PopulateCommandBuffer_CopyBufferToBuffer(const VkCommandBuffer CB, const VkBuffer Src, const VkBuffer Dst, const VkAccessFlags AF, const VkPipelineStageFlagBits PSF, const size_t Size);
	static void PopulateCommandBuffer_CopyBufferToImage(const VkCommandBuffer CB, const VkBuffer Src, const VkImage Dst, const VkAccessFlags AF, const VkImageLayout IL, const VkPipelineStageFlags PSF, const std::vector<VkBufferImageCopy>& BICs, const uint32_t Levels, const uint32_t Layers);
	static void PopulateCommandBuffer_CopyImageToBuffer(const VkCommandBuffer CB, const VkImage Src, const VkBuffer Dst, const VkAccessFlags AF, const VkImageLayout IL, const VkPipelineStageFlags PSF, const std::vector<VkBufferImageCopy>& BICs, const uint32_t Levels, const uint32_t Layers);
	static void SubmitAndWait(const VkQueue Queue, const VkCommandBuffer CB);
#pragma endregion

	static void EnumerateMemoryRequirements(const VkMemoryRequirements& MR, const VkPhysicalDeviceMemoryProperties& PDMP);

	//virtual void ValidateFormatProperties(VkPhysicalDevice PD, const VkFormat Format, const VkImageUsageFlags Usage) const;
	//virtual void ValidateFormatProperties_SampledImage(VkPhysicalDevice PD, const VkFormat Format, const VkImageUsageFlags Usage, const VkFilter Mag, const VkFilter Min, const VkSamplerMipmapMode Mip) const;
	//virtual void ValidateFormatProperties_StorageImage(VkPhysicalDevice PD, const VkFormat Format, const VkImageUsageFlags Usage, const bool Atomic) const;

#pragma region MARKER
	static void MarkerInsert(VkCommandBuffer CB, const glm::vec4& Color, const char* Name);
	static void MarkerInsert(VkCommandBuffer CB, const glm::vec4& Color, const std::string_view Name) { MarkerInsert(CB, Color, data(Name)); }
	static void MarkerBegin(VkCommandBuffer CB, const glm::vec4& Color, const char* Name);
	static void MarkerBegin(VkCommandBuffer CB, const glm::vec4& Color, const std::string_view Name) { MarkerBegin(CB, Color, data(Name)); }
	static void MarkerEnd(VkCommandBuffer CB);
	class ScopedMarker
	{
	public:
		ScopedMarker(VkCommandBuffer CB, const glm::vec4& Color, const std::string_view Name) : CommandBuffer(CB) { MarkerBegin(CommandBuffer, Color, Name); }
		~ScopedMarker() { MarkerEnd(CommandBuffer); }
	private:
		VkCommandBuffer CommandBuffer;
	};
	static void MarkerSetName([[maybe_unused]]VkDevice Device, [[maybe_unused]] const VkDebugReportObjectTypeEXT Type, [[maybe_unused]] const uint64_t Object, [[maybe_unused]] std::string_view Name) {
#ifdef USE_DEBUG_MARKER
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
#ifdef USE_DEBUG_MARKER
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
#pragma endregion
#pragma region MARKER_TEMPLATE
	template<typename T> static void MarkerSetObjectName(VkDevice Device, T Object, const std::string_view Name) { DEBUG_BREAK(); /* テンプレート特殊化されていない (Not template specialized) */ }
	template<typename T> static void MarkerSetObjectTag(VkDevice Device, T Object, const uint64_t TagName, const size_t TagSize, const void* TagData) { DEBUG_BREAK(); /* テンプレート特殊化されていない (Not template specialized) */ }
	//!< ↓ここでテンプレート特殊化している (Template specialization here)
#include "VKDebugMarker.inl"
#pragma endregion

	virtual void EnumerateInstanceLayerProperties();
	virtual void EnumerateInstanceExtensionProperties(const char* LayerName);

#ifdef VK_NO_PROTOYYPES
	void LoadVulkanLibrary();
#endif

	virtual void CreateInstance();
#ifdef USE_DEBUG_REPORT
	virtual void CreateDebugReportCallback();
#endif

	virtual void EnumeratePhysicalDeviceProperties(const VkPhysicalDeviceProperties& PDP);
	virtual void EnumeratePhysicalDeviceFeatures(const VkPhysicalDeviceFeatures& PDF);
	virtual void EnumeratePhysicalDeviceMemoryProperties(const VkPhysicalDeviceMemoryProperties& PDMP);
	virtual void EnumeratePhysicalDevice(VkInstance Instance);
	virtual void EnumeratePhysicalDeviceLayerProperties(VkPhysicalDevice PD);
	virtual void EnumeratePhysicalDeviceExtensionProperties(VkPhysicalDevice PD, const char* LayerName);
	//virtual void EnumerateQueueFamilyProperties(VkPhysicalDevice PD, VkSurfaceKHR Surface, std::vector<VkQueueFamilyProperties>& QFPs);
	//virtual void OverridePhysicalDeviceFeatures(VkPhysicalDeviceFeatures& PDF) const;
	static [[nodiscard]] uint32_t FindQueueFamilyPropertyIndex(const VkQueueFlags QF, const std::vector<VkQueueFamilyProperties>& QFPs);
	static [[nodiscard]] uint32_t FindQueueFamilyPropertyIndex(const VkPhysicalDevice PD, const VkSurfaceKHR Sfc, const std::vector<VkQueueFamilyProperties>& QFPs);
	//virtual void CreateQueueFamilyPriorities(VkPhysicalDevice PD, VkSurfaceKHR Surface, const std::vector<VkQueueFamilyProperties>& QFPs, std::vector<std::vector<float>>& QueueFamilyPriorites);
	virtual void CreateDevice(HWND hWnd, HINSTANCE hInstance, void* pNext = nullptr, const std::vector<const char*>& AdditionalExtensions = {});

	virtual void CreateFence(VkDevice Dev);

	virtual void AllocateCommandBuffer();

	virtual [[nodiscard]] VkSurfaceFormatKHR SelectSurfaceFormat(VkPhysicalDevice PD, VkSurfaceKHR Surface);
	virtual [[nodiscard]] VkPresentModeKHR SelectSurfacePresentMode(VkPhysicalDevice PD, VkSurfaceKHR Surface);

	virtual void CreateSwapchain() { CreateSwapchain(GetCurrentPhysicalDevice(), Surface, GetClientRectWidth(), GetClientRectHeight()); }
	virtual void CreateSwapchain(VkPhysicalDevice PD, VkSurfaceKHR Sfc, const uint32_t Width, const uint32_t Height, const VkImageUsageFlags AdditionalUsage = 0);
	virtual void ResizeSwapchain(const uint32_t Width, const uint32_t Height);

	virtual void CreateViewport(const float Width, const float Height, const float MinDepth = 0.0f, const float MaxDepth = 1.0f);

	virtual void LoadScene() {}

	static void CreateBufferMemory(VkBuffer* Buffer, VkDeviceMemory* DeviceMemory, const VkDevice Device, const VkPhysicalDeviceMemoryProperties PDMP, const VkBufferUsageFlags BUF, const size_t Size, const VkMemoryPropertyFlags MPF, const void* Source = nullptr);
	static void CreateImageMemory(VkImage* Image, VkDeviceMemory* DM, const VkDevice Device, const VkPhysicalDeviceMemoryProperties PDMP, const VkImageCreateFlags ICF, const VkImageType IT, const VkFormat Format, const VkExtent3D& Extent, const uint32_t Levels, const uint32_t Layers, const VkImageUsageFlags IUF);

	virtual void SubmitStagingCopy(const VkBuffer Buf, const VkQueue Queue, const VkCommandBuffer CB, const VkAccessFlagBits AF, const VkPipelineStageFlagBits PSF, const VkDeviceSize Size, const void* Source);
	static void CreateBufferMemoryAndSubmitTransferCommand(VkBuffer* Buffer, VkDeviceMemory* DeviceMemory,
		const VkDevice Device, const VkPhysicalDeviceMemoryProperties PDMP, const VkBufferUsageFlags BUF, const size_t Size,
		const void* Source, const VkCommandBuffer CB, const VkAccessFlagBits AF, const VkPipelineStageFlagBits PSF, const VkQueue Queue);

	static VkDeviceAddress GetDeviceAddress(const VkDevice Device, const VkBuffer Buffer) {
		const VkBufferDeviceAddressInfo BDAI = { .sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO, .pNext = nullptr, .buffer = Buffer };
		return vkGetBufferDeviceAddress(Device, &BDAI);
	}

#pragma region RAYTRACING
	static void BuildAccelerationStructure(const VkDevice Device, const VkPhysicalDeviceMemoryProperties PDMP, VkQueue Queue, const VkCommandBuffer CB, const VkAccelerationStructureKHR AS, const VkAccelerationStructureTypeKHR Type, const VkDeviceSize Size, const std::vector<VkAccelerationStructureGeometryKHR>& ASGs);
#pragma endregion

	virtual void CreateGeometry() {}

	virtual void CreateUniformBuffer() {}
	virtual void CreateStorageBuffer() {}
	virtual void CreateUniformTexelBuffer() {}
	virtual void CreateStorageTexelBuffer() {}

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

	//[[deprecated("this is example code")]]
	//virtual void CreateUniformBuffer_Example();
	//[[deprecated("this is example code")]]
	//virtual void CreateStorageBuffer_Example();
	//[[deprecated("this is example code")]]
	//virtual void CreateUniformTexelBuffer_Example();
	//[[deprecated("this is example code")]]
	//virtual void CreateStorageTexelBuffer_Example();

	virtual void CreateDescriptorSetLayout(VkDescriptorSetLayout& DSL, const VkDescriptorSetLayoutCreateFlags Flags, const std::vector<VkDescriptorSetLayoutBinding>& DSLBs);
	virtual void CreatePipelineLayout(VkPipelineLayout& PL, const std::vector<VkDescriptorSetLayout>& DSLs, const std::vector<VkPushConstantRange>& PCRs);
	virtual void CreatePipelineLayout() { PipelineLayouts.emplace_back(VkPipelineLayout()); CreatePipelineLayout(PipelineLayouts.back(), {}, {}); }

	virtual void CreateDescriptorSet() {}
	virtual void CreateDescriptorPool(VkDescriptorPool& DP, const VkDescriptorPoolCreateFlags Flags, const std::vector<VkDescriptorPoolSize>& DPSs);

	virtual void CreateDescriptorUpdateTemplate(VkDescriptorUpdateTemplate& DUT, const std::vector<VkDescriptorUpdateTemplateEntry>& DUTEs, const VkDescriptorSetLayout DSL);
	virtual void UpdateDescriptorSet() {}

	virtual void CreateTexture() {}
	virtual void CreateTextureArray1x1(const std::vector<uint32_t>& Colors, const VkPipelineStageFlags PSF);
	virtual void CreateImmutableSampler() {}
	virtual void CreateSampler() {}

	virtual void CreateRenderPass(VkRenderPass& RP, const std::vector<VkAttachmentDescription>& ADs, const std::vector<VkSubpassDescription> &SDs, const std::vector<VkSubpassDependency>& Deps);
	virtual void CreateRenderPass();

	virtual void CreateFramebuffer(VkFramebuffer& FB, const VkRenderPass RP, const uint32_t Width, const uint32_t Height, const uint32_t Layers, const std::vector<VkImageView>& IVs);
	virtual void CreateFramebuffer() {
		const auto RP = RenderPasses[0];
		for (auto i : SwapchainImageViews) {
			Framebuffers.emplace_back(VkFramebuffer());
			VK::CreateFramebuffer(Framebuffers.back(), RP, SurfaceExtent2D.width, SurfaceExtent2D.height, 1, { i });
		}
	}

	virtual [[nodiscard]] VkShaderModule CreateShaderModule(const std::wstring& Path) const;

#include "VKPipelineCache.inl"
	virtual void CreatePipeline() {}
	static void CreatePipeline_(VkPipeline& PL, const VkDevice Dev, const VkPipelineLayout PLL, const VkRenderPass RP,
		const VkPrimitiveTopology Topology, const uint32_t PatchControlPoints,
		const VkPipelineRasterizationStateCreateInfo& PRSCI,
		const VkPipelineDepthStencilStateCreateInfo& PDSSCI,
		const VkPipelineShaderStageCreateInfo* VS, const VkPipelineShaderStageCreateInfo* FS, const VkPipelineShaderStageCreateInfo* TES, const VkPipelineShaderStageCreateInfo* TCS, const VkPipelineShaderStageCreateInfo* GS,
		const std::vector<VkVertexInputBindingDescription>& VIBDs, const std::vector<VkVertexInputAttributeDescription>& VIADs,
		const std::vector<VkPipelineColorBlendAttachmentState>& PCBASs,
		VkPipelineCache PC = VK_NULL_HANDLE);
	//virtual void CreatePipeline_Compute();

	//virtual void ClearColorAttachment(const VkCommandBuffer CommandBuffer, const VkClearColorValue& Color);
	//virtual void ClearDepthStencilAttachment(const VkCommandBuffer CommandBuffer, const VkClearDepthStencilValue& DepthStencil);
	
	virtual void PopulateCommandBuffer([[maybe_unused]] const size_t i) {}

	virtual uint32_t GetCurrentBackBufferIndex() const { return SwapchainImageIndex; }
	virtual void DrawFrame([[maybe_unused]] const uint32_t i) {}
	virtual void Draw();
	virtual void Dispatch();
	virtual void WaitForFence();
	virtual void Submit();
	virtual void Present();

#pragma region Allocation
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
	static const [[nodiscard]] VkAllocationCallbacks* GetAllocationCallbacks() { 
		return nullptr;
		//return &AllocationCallbacks;
	}
#pragma endregion

	virtual [[nodiscard]] VkPhysicalDevice GetCurrentPhysicalDevice() const { return CurrentPhysicalDevice; };
	virtual [[nodiscard]] VkPhysicalDeviceMemoryProperties GetCurrentPhysicalDeviceMemoryProperties() const { return CurrentPhysicalDeviceMemoryProperties; }

#ifdef VK_NO_PROTOYYPES

protected:
#ifdef _WINDOWS
	HMODULE VulkanLibrary = nullptr;
#else
	void* VulkanLibrary = nullptr;
#endif

public:
	//!< グローバルレベル関数 (Global level functions)
#define VK_GLOBAL_PROC_ADDR(proc) static PFN_vk ## proc vk ## proc;
#include "VKGlobalProcAddr.h"
#undef VK_GLOBAL_PROC_ADDR

	//!< インスタンスレベル関数 (Instance level functions)
#define VK_INSTANCE_PROC_ADDR(proc) static PFN_vk ## proc vk ## proc;
#include "VKInstanceProcAddr.h"
#undef VK_INSTANCE_PROC_ADDR

	//!< デバイスレベル関数 (Device level functions)
#define VK_DEVICE_PROC_ADDR(proc) static PFN_vk ## proc vk ## proc;
#include "VKDeviceProcAddr.h"
#include "VKDeviceProcAddr_RayTracing.h"
#include "VKDeviceProcAddr_MeshShader.h"
#undef VK_DEVICE_PROC_ADDR

#endif //!< VK_NO_PROTOYYPES

public:
#ifdef USE_DEBUG_REPORT
	//!< インスタンスレベル関数、デバッグ用 (Instance level functions for debug)
#define VK_INSTANCE_PROC_ADDR(proc) static PFN_vk ## proc ## EXT vk ## proc;
#include "VKInstanceProcAddr_DebugReport.h"
#undef VK_INSTANCE_PROC_ADDR
#endif

#ifdef USE_DEBUG_MARKER
	//!< デバイスレベル関数、デバッグ用 (Device level functions for debug)
#define VK_DEVICE_PROC_ADDR(proc) static PFN_vk ## proc ## EXT vk ## proc;
#include "VKDeviceProcAddr_DebugMarker.h"
#undef VK_DEVICE_PROC_ADDR
#endif

protected:
	VkInstance Instance = VK_NULL_HANDLE;
#ifdef USE_DEBUG_REPORT
	VkDebugReportCallbackEXT DebugReportCallback = VK_NULL_HANDLE;
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
	//uint32_t GraphicsQueueIndexInFamily = UINT32_MAX; //!< ファミリ内でのインデックス (必ずしも覚えておく必要は無いが一応覚えておく)
	uint32_t PresentQueueFamilyIndex = UINT32_MAX;
	//uint32_t PresentQueueIndexInFamily = UINT32_MAX;
	uint32_t ComputeQueueFamilyIndex = UINT32_MAX;
	//uint32_t ComputeQueueIndexInFamily = UINT32_MAX;
	//uint32_t TransferQueueFamilyIndex = UINT32_MAX;
	//uint32_t TransferQueueIndex = UINT32_MAX;;
	//uint32_t SparceBindingQueueFamilyIndex = UINT32_MAX;
	//uint32_t SparceBindingQueueIndex = UINT32_MAX;;

	/**
	フェンス		... デバイスとホストの同期(GPUとCPUの同期)
	セマフォ		... 複数キュー間の同期
	イベント		... コマンドバッファ間の同期(同一キューファミリ)
	バリア		... コマンドバッファ内の同期
	*/
	VkFence Fence = VK_NULL_HANDLE;
	VkFence ComputeFence = VK_NULL_HANDLE;
	VkSemaphore NextImageAcquiredSemaphore = VK_NULL_HANDLE;	//!< プレゼント完了までウエイト
	VkSemaphore RenderFinishedSemaphore = VK_NULL_HANDLE;		//!< 描画完了するまでウエイト

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

	std::vector<VertexBuffer> VertexBuffers;
	std::vector<IndexBuffer> IndexBuffers;
	std::vector<IndirectBuffer> IndirectBuffers;
#pragma region RAYTRACING
	std::vector<AccelerationStructureBuffer> BLASs, TLASs;
	std::vector<ShaderBindingTable> ShaderBindingTables;
#pragma endregion

	std::vector<UniformBuffer> UniformBuffers;

	VkFormat DepthFormat = VK_FORMAT_D24_UNORM_S8_UINT;
	std::vector<Texture> Textures;
	std::vector<DepthTexture> DepthTextures;
	std::vector<RenderTexture> RenderTextures;

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
static std::ostream& operator<<(std::ostream& rhs, const glm::vec2& Value) { for (auto i = 0; i < Value.length(); ++i) { rhs << Value[i] << ", "; } rhs << std::endl; return rhs; }
static std::ostream& operator<<(std::ostream& rhs, const glm::vec3& Value) { for (auto i = 0; i < Value.length(); ++i) { rhs << Value[i] << ", "; } rhs << std::endl; return rhs; }
static std::ostream& operator<<(std::ostream& rhs, const glm::vec4& Value) { for (auto i = 0; i < Value.length(); ++i) { rhs << Value[i] << ", "; } rhs << std::endl; return rhs; }
static std::ostream& operator<<(std::ostream& rhs, const glm::quat& Value) { for (auto i = 0; i < Value.length(); ++i) { rhs << Value[i] << ", "; } rhs << std::endl; return rhs; }
static std::ostream& operator<<(std::ostream& rhs, const glm::mat2& Value) { for (auto i = 0; i < 2; ++i) { rhs << Value[i]; } rhs << std::endl; return rhs; }
static std::ostream& operator<<(std::ostream& rhs, const glm::mat3& Value) { for (auto i = 0; i < 3; ++i) { rhs << Value[i]; } rhs << std::endl; return rhs; }
static std::ostream& operator<<(std::ostream& rhs, const glm::mat4& Value) { for (auto i = 0; i < 4; ++i) { rhs << Value[i]; } rhs << std::endl; return rhs; }
#endif