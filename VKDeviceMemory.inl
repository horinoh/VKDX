#pragma once

//!< テンプレート特殊化
//!< template specialization

template<> void CreateHostVisibleMemory(VkDeviceMemory* DeviceMemory, const VkBuffer Object) {
	VkMemoryRequirements MemoryRequirements;
	//!< MemoryRequirements の取得の仕方がバッファ毎に異なるのでテンプレート化している
	//!< Because vkGetImageMemoryRequirements is different, using template specialization
	vkGetBufferMemoryRequirements(Device, Object, &MemoryRequirements);
	const VkMemoryAllocateInfo MemoryAllocateInfo = {
		VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
		nullptr,
		MemoryRequirements.size,
		GetMemoryType(MemoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT/*| VK_MEMORY_PROPERTY_HOST_COHERENT_BIT*/) //!< HOST_VISIBLE にすること Must be HOST_VISIBLE
	};
	VERIFY_SUCCEEDED(vkAllocateMemory(Device, &MemoryAllocateInfo, nullptr, DeviceMemory));
}
template<> void CreateHostVisibleMemory(VkDeviceMemory* DeviceMemory, const VkImage Object) { assert(false && "Not implemented"); }

template<> void CreateDeviceLocalMemory(VkDeviceMemory* DeviceMemory, const VkBuffer Object) {
	VkMemoryRequirements MemoryRequirements;
	//!< MemoryRequirements の取得の仕方がバッファ毎に異なるのでテンプレート化している
	//!< Because vkGetImageMemoryRequirements is different, using template specialization
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
	//!< MemoryRequirements の取得の仕方がバッファ毎に異なるのでテンプレート化している
	//!< Because vkGetImageMemoryRequirements is different, using template specialization
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
	//!< バインドの仕方がバッファ毎に異なるのでテンプレート化している
	//!< Becase vkBindBufferMemory is different, using template specialization
	VERIFY_SUCCEEDED(vkBindBufferMemory(Device, Object, DeviceMemory, Offset));
}
template<> void BindDeviceMemory(const VkImage Object, const VkDeviceMemory DeviceMemory, const VkDeviceSize Offset) 
{
	//!< バインドの仕方がバッファ毎に異なるのでテンプレート化している
	//!< Becase vkBindBufferMemory is different, using template specialization
	VERIFY_SUCCEEDED(vkBindImageMemory(Device, Object, DeviceMemory, Offset));
}
