#pragma once

//!< �e���v���[�g���ꉻ
template<>
static void SetName(VkDevice Device, VkBuffer Object, const char* Name)
{
#if 0
	if (VK_NULL_HANDLE != vkDebugMarkerSetObjectName) {
		VkDebugMarkerObjectNameInfoEXT DebugMarkerObjectNameInfo = {
			VK_STRUCTURE_TYPE_DEBUG_MARKER_OBJECT_NAME_INFO_EXT,
			nullptr,
			VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_EXT,
			reinterpret_cast<uint64_t>(Object),
			Name
		};
#if 0
		//!< #TODO �ʂ�Ȃ��Ȃ����c
		VERIFY_SUCCEEDED(vkDebugMarkerSetObjectName(Device, &DebugMarkerObjectNameInfo));
#else
		std::cout << Red << "�ʂ�Ȃ��Ȃ����̂� vkDebugMarkerSetObjectName �̓R�����g�A�E�g" << White << std::endl;
#endif
	}
#endif
}
template<>
static void SetTag(VkDevice Device, VkBuffer Object, const uint64_t TagName, const size_t TagSize, const void* TagData)
{
#if 0
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
#endif
}

template<>
static void SetName(VkDevice Device, VkBufferView Object, const char* Name)
{
#if 0
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
#endif
}
template<>
static void SetTag(VkDevice Device, VkBufferView Object, const uint64_t TagName, const size_t TagSize, const void* TagData)
{
#if 0
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
#endif
}

template<>
static void SetName(VkDevice Device, VkSwapchainKHR Object, const char* Name)
{
#if 0
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
#endif
}
template<>
static void SetTag(VkDevice Device, VkSwapchainKHR Object, const uint64_t TagName, const size_t TagSize, const void* TagData)
{
#if 0
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
#endif
}