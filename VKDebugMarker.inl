#pragma once

#pragma region VkBuffer
template<> void MarkerSetObjectName(VkDevice Device, VkBuffer Object, std::string_view Name) { MarkerSetName(Device, VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_EXT, reinterpret_cast<uint64_t>(Object), data(Name)); }
template<> void MarkerSetObjectTag(VkDevice Device, VkBuffer Object, const uint64_t TagName, const size_t TagSize, const void* TagData) { MarkerSetTag(Device, VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_EXT, reinterpret_cast<uint64_t>(Object), TagName, TagSize, TagData); }
#pragma endregion

#pragma region VkBufferView
template<> void MarkerSetObjectName(VkDevice Device, VkBufferView Object, std::string_view Name) { MarkerSetName(Device, VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_VIEW_EXT, reinterpret_cast<uint64_t>(Object), data(Name)); }
template<> void MarkerSetObjectTag(VkDevice Device, VkBufferView Object, const uint64_t TagName, const size_t TagSize, const void* TagData) { MarkerSetTag(Device, VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_VIEW_EXT, reinterpret_cast<uint64_t>(Object), TagName, TagSize, TagData); }
#pragma endregion

#pragma region VkSwapchainKHR
template<> void MarkerSetObjectName(VkDevice Device, VkSwapchainKHR Object, std::string_view Name) { MarkerSetName(Device, VK_DEBUG_REPORT_OBJECT_TYPE_SWAPCHAIN_KHR_EXT, reinterpret_cast<uint64_t>(Object), data(Name)); }
template<> void MarkerSetObjectTag(VkDevice Device, VkSwapchainKHR Object, const uint64_t TagName, const size_t TagSize, const void* TagData) { MarkerSetTag(Device, VK_DEBUG_REPORT_OBJECT_TYPE_SWAPCHAIN_KHR_EXT, reinterpret_cast<uint64_t>(Object), TagName, TagSize, TagData); }
#pragma endregion