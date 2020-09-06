#pragma once

template<> /*static*/ void MarkerSetObjectName(VkDevice Device, VkBuffer Object, const char* Name) { MarkerSetName(Device, VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_EXT, reinterpret_cast<uint64_t>(Object), Name); }
template<> /*static*/ void MarkerSetObjectTag(VkDevice Device, VkBuffer Object, const uint64_t TagName, const size_t TagSize, const void* TagData) { MarkerSetTag(Device, VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_EXT, reinterpret_cast<uint64_t>(Object), TagName, TagSize, TagData); }

template<> /*static*/ void MarkerSetObjectName(VkDevice Device, VkBufferView Object, const char* Name) { MarkerSetName(Device, VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_VIEW_EXT, reinterpret_cast<uint64_t>(Object), Name); }
template<> /*static*/ void MarkerSetObjectTag(VkDevice Device, VkBufferView Object, const uint64_t TagName, const size_t TagSize, const void* TagData) { MarkerSetTag(Device, VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_VIEW_EXT, reinterpret_cast<uint64_t>(Object), TagName, TagSize, TagData); }

template<> /*static*/ void MarkerSetObjectName(VkDevice Device, VkSwapchainKHR Object, const char* Name) { MarkerSetName(Device, VK_DEBUG_REPORT_OBJECT_TYPE_SWAPCHAIN_KHR_EXT, reinterpret_cast<uint64_t>(Object), Name); }
template<> /*static*/ void MarkerSetObjectTag(VkDevice Device, VkSwapchainKHR Object, const uint64_t TagName, const size_t TagSize, const void* TagData) { MarkerSetTag(Device, VK_DEBUG_REPORT_OBJECT_TYPE_SWAPCHAIN_KHR_EXT, reinterpret_cast<uint64_t>(Object), TagName, TagSize, TagData); }
