#pragma once

//!< テンプレート特殊化
template<>
void SetObjectName(VkDevice Device, VkSwapchainKHR Object, const char* Name)
{
	VkDebugMarkerObjectNameInfoEXT DebugMarkerObjectNameInfo = {
		VK_STRUCTURE_TYPE_DEBUG_MARKER_OBJECT_NAME_INFO_EXT,
		nullptr,
		VK_DEBUG_REPORT_OBJECT_TYPE_SWAPCHAIN_KHR_EXT,
		reinterpret_cast<uint64_t>(Object),
		Name
	};
	VERIFY_SUCCEEDED(vkDebugMarkerSetObjectName(Device, &DebugMarkerObjectNameInfo));
}
template<>
void SetObjectTag(VkDevice Device, VkSwapchainKHR Object, const uint64_t TagName, const size_t TagSize, const void* TagData)
{
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
