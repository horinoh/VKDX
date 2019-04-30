#pragma once

//!< テンプレート特殊化
//!< template specialization

template<> void CreateHostVisibleMemory(VkDeviceMemory* DeviceMemory, const VkBuffer T) { CreateHostVisibleBufferMemory(DeviceMemory, T); }
template<> void CreateDeviceLocalMemory(VkDeviceMemory* DeviceMemory, const VkBuffer T) { CreateDeviceLocalBufferMemory(DeviceMemory, T); }
template<> void BindDeviceMemory(const VkBuffer T, const VkDeviceMemory DeviceMemory, const VkDeviceSize Offset) { BindBufferMemory(T, DeviceMemory, Offset); }

//template<> void CreateHostVisibleMemory(VkDeviceMemory* DeviceMemory, const VkImage T) { CreateHostVisibleImageMemory(DeviceMemory, T); }
template<> void CreateDeviceLocalMemory(VkDeviceMemory* DeviceMemory, const VkImage T) { CreateDeviceLocalImageMemory(DeviceMemory, T); }
template<> void BindDeviceMemory(const VkImage T, const VkDeviceMemory DeviceMemory, const VkDeviceSize Offset) { BindImageMemory(T, DeviceMemory, Offset); }
