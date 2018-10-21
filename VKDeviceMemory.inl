#pragma once

//!< テンプレート特殊化
//!< template specialization

template<> void CreateHostVisibleMemory(VkDeviceMemory* DeviceMemory, const VkBuffer Object) { CreateHostVisibleBufferMemory(DeviceMemory, Object); }
template<> void CreateDeviceLocalMemory(VkDeviceMemory* DeviceMemory, const VkBuffer Object) { CreateDeviceLocalBufferMemory(DeviceMemory, Object); }
template<> void BindMemory(const VkBuffer Object, const VkDeviceMemory DeviceMemory, const VkDeviceSize Offset) { BindBufferMemory(Object, DeviceMemory, Offset); }

template<> void CreateHostVisibleMemory(VkDeviceMemory* DeviceMemory, const VkImage Object) { CreateHostVisibleImageMemory(DeviceMemory, Object); }
template<> void CreateDeviceLocalMemory(VkDeviceMemory* DeviceMemory, const VkImage Object) { CreateDeviceLocalImageMemory(DeviceMemory, Object); }
template<> void BindMemory(const VkImage Object, const VkDeviceMemory DeviceMemory, const VkDeviceSize Offset) { BindImageMemory(Object, DeviceMemory, Offset); }
