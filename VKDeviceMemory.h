#pragma once

//!< �e���v���[�g���ꉻ
template<> void CreateHostVisibleMemory(VkDeviceMemory* DeviceMemory, const VkBuffer Object) {
	VkMemoryRequirements MemoryRequirements;
	//!< MemoryRequirements �̎擾�̎d�����o�b�t�@���ɈقȂ�̂Ńe���v���[�g�����Ă���
	vkGetBufferMemoryRequirements(Device, Object, &MemoryRequirements);
	const VkMemoryAllocateInfo MemoryAllocateInfo = {
		VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
		nullptr,
		MemoryRequirements.size,
		GetMemoryType(MemoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT/*| VK_MEMORY_PROPERTY_HOST_COHERENT_BIT*/) //!< HOST_VISIBLE �ɂ��邱��
	};
	VERIFY_SUCCEEDED(vkAllocateMemory(Device, &MemoryAllocateInfo, nullptr, DeviceMemory));
}

template<> void CreateDeviceLocalMemory(VkDeviceMemory* DeviceMemory, const VkBuffer Object) {
	VkMemoryRequirements MemoryRequirements;
	//!< MemoryRequirements �̎擾�̎d�����o�b�t�@���ɈقȂ�̂Ńe���v���[�g�����Ă���
	vkGetBufferMemoryRequirements(Device, Object, &MemoryRequirements);
	const VkMemoryAllocateInfo MemoryAllocateInfo = {
		VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
		nullptr,
		MemoryRequirements.size,
		GetMemoryType(MemoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
	};
	VERIFY_SUCCEEDED(vkAllocateMemory(Device, &MemoryAllocateInfo, nullptr, DeviceMemory));
}
template<> void CreateDeviceLocalMemory(VkDeviceMemory* DeviceMemory, const VkImage Object) {
	VkMemoryRequirements MemoryRequirements;
	//!< MemoryRequirements �̎擾�̎d�����o�b�t�@���ɈقȂ�̂Ńe���v���[�g�����Ă���
	vkGetImageMemoryRequirements(Device, Object, &MemoryRequirements);
	const VkMemoryAllocateInfo MemoryAllocateInfo = {
		VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
		nullptr,
		MemoryRequirements.size,
		GetMemoryType(MemoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
	};
	VERIFY_SUCCEEDED(vkAllocateMemory(Device, &MemoryAllocateInfo, nullptr, DeviceMemory));
}

template<> void BindDeviceMemory(const VkBuffer Object, const VkDeviceMemory DeviceMemory, const VkDeviceSize Offset) 
{
	//!< �o�C���h�̎d�����o�b�t�@���ɈقȂ�̂Ńe���v���[�g�����Ă���
	VERIFY_SUCCEEDED(vkBindBufferMemory(Device, Object, DeviceMemory, Offset));
}
template<> void BindDeviceMemory(const VkImage Object, const VkDeviceMemory DeviceMemory, const VkDeviceSize Offset) 
{
	//!< �o�C���h�̎d�����o�b�t�@���ɈقȂ�̂Ńe���v���[�g�����Ă���
	VERIFY_SUCCEEDED(vkBindImageMemory(Device, Object, DeviceMemory, Offset));
}
