#pragma once

//!< テンプレート特殊化 (template specialization)

//!< MemoryRequirements の取得の仕方がバッファ、イメージで異なるのでテンプレート化している (Because vkGetImageMemoryRequirements is different, using template specialization)
template<> void CreateDeviceMemory(VkDeviceMemory* DeviceMemory, const VkBuffer Object, const VkMemoryPropertyFlags MPF) { CreateBufferMemory(DeviceMemory, Object, MPF); }
template<> void BindDeviceMemory(const VkBuffer Object, const VkDeviceMemory DeviceMemory, const VkDeviceSize Offset) { BindBufferMemory(Object, DeviceMemory, Offset); }

template<> void CreateDeviceMemory(VkDeviceMemory* DeviceMemory, const VkImage Object, const VkMemoryPropertyFlags MPF) { CreateImageMemory(DeviceMemory, Object, MPF); }
template<> void BindDeviceMemory(const VkImage Object, const VkDeviceMemory DeviceMemory, const VkDeviceSize Offset) { BindImageMemory(Object, DeviceMemory, Offset); }
