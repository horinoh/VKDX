#pragma once

//!< �e���v���[�g���ꉻ (template specialization)

//!< MemoryRequirements �̎擾�̎d�����o�b�t�@�A�C���[�W�ňقȂ�̂Ńe���v���[�g�����Ă��� (Because vkGetImageMemoryRequirements is different, using template specialization)
template<> void CreateDeviceMemory(VkDeviceMemory* DeviceMemory, const VkBuffer Object, const VkMemoryPropertyFlags MPF) { CreateBufferMemory(DeviceMemory, Object, MPF); }
template<> void BindDeviceMemory(const VkBuffer Object, const VkDeviceMemory DeviceMemory, const VkDeviceSize Offset) { BindBufferMemory(Object, DeviceMemory, Offset); }

template<> void CreateDeviceMemory(VkDeviceMemory* DeviceMemory, const VkImage Object, const VkMemoryPropertyFlags MPF) { CreateImageMemory(DeviceMemory, Object, MPF); }
template<> void BindDeviceMemory(const VkImage Object, const VkDeviceMemory DeviceMemory, const VkDeviceSize Offset) { BindImageMemory(Object, DeviceMemory, Offset); }
