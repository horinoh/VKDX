#pragma once

//!< テンプレート特殊化
template<>
static void SetName(VkDevice Device, VkBuffer Object, const char* Name)
{
	if (VK_NULL_HANDLE != vkDebugMarkerSetObjectName) {
		VkDebugMarkerObjectNameInfoEXT DebugMarkerObjectNameInfo = {
			VK_STRUCTURE_TYPE_DEBUG_MARKER_OBJECT_NAME_INFO_EXT,
			nullptr,
			VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_EXT,
			reinterpret_cast<uint64_t>(Object),
			Name
		};
		VERIFY_SUCCEEDED(vkDebugMarkerSetObjectName(Device, &DebugMarkerObjectNameInfo));
	}
}
template<>
static void SetTag(VkDevice Device, VkBuffer Object, const uint64_t TagName, const size_t TagSize, const void* TagData)
{
	if (VK_NULL_HANDLE != vkDebugMarkerSetObjectTag) {
		VkDebugMarkerObjectTagInfoEXT DebugMarkerObjectTagInfo = {
			VK_STRUCTURE_TYPE_DEBUG_MARKER_OBJECT_TAG_INFO_EXT,
			nullptr,
			VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_EXT,
			reinterpret_cast<uint64_t>(Object),
			TagName,
			TagSize,
			TagData
		};
		VERIFY_SUCCEEDED(vkDebugMarkerSetObjectTag(Device, &DebugMarkerObjectTagInfo));
	}
}

template<>
static void SetName(VkDevice Device, VkBufferView Object, const char* Name)
{
	if (VK_NULL_HANDLE != vkDebugMarkerSetObjectName) {
		VkDebugMarkerObjectNameInfoEXT DebugMarkerObjectNameInfo = {
		VK_STRUCTURE_TYPE_DEBUG_MARKER_OBJECT_NAME_INFO_EXT,
		nullptr,
		VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_VIEW_EXT,
		reinterpret_cast<uint64_t>(Object),
		Name
		};
		VERIFY_SUCCEEDED(vkDebugMarkerSetObjectName(Device, &DebugMarkerObjectNameInfo));
	}
}
template<>
static void SetTag(VkDevice Device, VkBufferView Object, const uint64_t TagName, const size_t TagSize, const void* TagData)
{
	if (VK_NULL_HANDLE != vkDebugMarkerSetObjectTag) {
		VkDebugMarkerObjectTagInfoEXT DebugMarkerObjectTagInfo = {
			VK_STRUCTURE_TYPE_DEBUG_MARKER_OBJECT_TAG_INFO_EXT,
			nullptr,
			VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_VIEW_EXT,
			reinterpret_cast<uint64_t>(Object),
			TagName,
			TagSize,
			TagData
		};
		VERIFY_SUCCEEDED(vkDebugMarkerSetObjectTag(Device, &DebugMarkerObjectTagInfo));
	}
}

template<>
static void SetName(VkDevice Device, VkSwapchainKHR Object, const char* Name)
{
	if (VK_NULL_HANDLE != vkDebugMarkerSetObjectName) {
		VkDebugMarkerObjectNameInfoEXT DebugMarkerObjectNameInfo = {
			VK_STRUCTURE_TYPE_DEBUG_MARKER_OBJECT_NAME_INFO_EXT,
			nullptr,
			VK_DEBUG_REPORT_OBJECT_TYPE_SWAPCHAIN_KHR_EXT,
			reinterpret_cast<uint64_t>(Object),
			Name
		};
		VERIFY_SUCCEEDED(vkDebugMarkerSetObjectName(Device, &DebugMarkerObjectNameInfo));
	}
}
template<>
static void SetTag(VkDevice Device, VkSwapchainKHR Object, const uint64_t TagName, const size_t TagSize, const void* TagData)
{
	if (VK_NULL_HANDLE != vkDebugMarkerSetObjectTag) {
		VkDebugMarkerObjectTagInfoEXT DebugMarkerObjectTagInfo = {
			VK_STRUCTURE_TYPE_DEBUG_MARKER_OBJECT_TAG_INFO_EXT,
			nullptr,
			VK_DEBUG_REPORT_OBJECT_TYPE_SWAPCHAIN_KHR_EXT,
			reinterpret_cast<uint64_t>(Object),
			TagName,
			TagSize,
			TagData
		};
		VERIFY_SUCCEEDED(vkDebugMarkerSetObjectTag(Device, &DebugMarkerObjectTagInfo));
	}
}
