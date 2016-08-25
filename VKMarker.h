#pragma once

#ifdef _DEBUG
void MarkerInsert(VkCommandBuffer CommandBuffer, const char* Name, const float* Color = nullptr)
{
	VkDebugMarkerMarkerInfoEXT DebugMarkerMarkerInfo = {
		VK_STRUCTURE_TYPE_DEBUG_MARKER_MARKER_INFO_EXT,
		nullptr,
		Name,
		{ nullptr == Color ? 0.0f : Color[0], nullptr == Color ? 0.0f : Color[1], nullptr == Color ? 0.0f : Color[2], nullptr == Color ? 0.0f : Color[3] }
	};
	vkCmdDebugMarkerInsert(CommandBuffer, &DebugMarkerMarkerInfo);
}
void MarkerBegin(VkCommandBuffer CommandBuffer, const char* Name, const float* Color = nullptr)
{
	VkDebugMarkerMarkerInfoEXT DebugMarkerMarkerInfo = {
		VK_STRUCTURE_TYPE_DEBUG_MARKER_MARKER_INFO_EXT,
		nullptr,
		Name,
		{ nullptr == Color ? 0.0f : Color[0], nullptr == Color ? 0.0f : Color[1], nullptr == Color ? 0.0f : Color[2], nullptr == Color ? 0.0f : Color[3] }
	};
	vkCmdDebugMarkerBegin(CommandBuffer, &DebugMarkerMarkerInfo);
}
void MarkerEnd(VkCommandBuffer CommandBuffer)
{
	vkCmdDebugMarkerEnd(CommandBuffer);
}

//!< テンプレート
template<typename T> void SetObjectName(VkDevice Device, T Object, const char* Name) {}
template<typename T> void SetObjectTag(VkDevice Device, T Object, const uint64_t TagName, const size_t TagSize, const void* Tag) {}
//!< 以下テンプレート特殊化
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
#endif //!< _DEBUG