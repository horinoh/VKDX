#include "VK.h"

#ifdef USE_DEBUG_UTILS
VKAPI_ATTR VkBool32 VKAPI_CALL VK::MessengerCallback(VkDebugUtilsMessageSeverityFlagBitsEXT MessageSeverity, VkDebugUtilsMessageTypeFlagsEXT MessageTypes, const VkDebugUtilsMessengerCallbackDataEXT* CallbackData, void* UserData)
{
	if (MessageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT) {}
	if (MessageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT) {}
	if (MessageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {}
	if (MessageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {}
	if (MessageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
		std::cout << "WARNING or ERROR" << std::endl;
	}

	if (MessageTypes & VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT) {}
	if (MessageTypes & VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT) {}
	if (MessageTypes & VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT) {}
	if (MessageTypes & VK_DEBUG_UTILS_MESSAGE_TYPE_DEVICE_ADDRESS_BINDING_BIT_EXT) {}

	if (nullptr != CallbackData) {
		//!< vk_validation_error_messages.h
		//UNIQUE_VALIDATION_ERROR_CODE;

		//if (nullptr != pCallbackData->pMessageIdName) {
		//	std::cout << "MessageIdName = " << pCallbackData->pMessageIdName << std::endl;
		//}
		//std::cout << "MessageIdNumber = " << pCallbackData->messageIdNumber << std::endl;
		std::cout << CallbackData->pMessage << std::endl;

		//!< キューに適用されたラベル ([0]に最新が入るので、ここでは逆順に出力している)
		if (0 < CallbackData->queueLabelCount) {
			std::cout << "\t" << "QueueLabel" << std::endl;
			for (int i = CallbackData->queueLabelCount - 1; i >= 0; --i) {
				const auto& Label = CallbackData->pQueueLabels[i];
				std::cout << "\t\t" << "[" << i << "]" << Label.pLabelName << "(" << Label.color[0] << ", " << Label.color[1] << ", " << Label.color[2] << ", " << Label.color[3] << ")";
			}
		}

		//!< コマンドバッファに適用されたラベル ([0]に最新が入るので、ここでは逆順に出力している)
		if (0 < CallbackData->cmdBufLabelCount) {
			std::cout << "\t" << "CmdBufLabel" << std::endl;
			for (int i = CallbackData->cmdBufLabelCount - 1; i >= 0; --i) {
				const auto& Label = CallbackData->pCmdBufLabels[i];
				std::cout << "\t\t" << "[" << i << "]" << Label.pLabelName << "(" << Label.color[0] << ", " << Label.color[1] << ", " << Label.color[2] << ", " << Label.color[3] << ")";
			}
		}

		//!< オブジェクト ([0]に最新が入るので、ここでは逆順に出力している)
		if (0 < CallbackData->objectCount) {
			std::cout << "\t" << "Object" << std::endl;
			for (int i = CallbackData->objectCount - 1; i >= 0; --i) {
				const auto& ObjectNameInfo = CallbackData->pObjects[i];
				std::cout << "\t\t" << "[" << i << "] " << (nullptr != ObjectNameInfo.pObjectName ? ObjectNameInfo.pObjectName : "----") << std::endl;

				std::cout << "\t\t\t";
				switch (ObjectNameInfo.objectType) {
				case VK_OBJECT_TYPE_INSTANCE:
					std::cout << "INSTANCE" << std::endl;
					ObjectNameInfo.objectHandle;
					break;
				case VK_OBJECT_TYPE_COMMAND_BUFFER:
					std::cout << "COMMAND_BUFFER" << std::endl;
					break;
				default:
					std::cout << ObjectNameInfo.objectType << std::endl;
					break;
				}
			}
		}
	}

	if (nullptr != UserData) {}

	return VK_FALSE;
}

void VK::SetObjectName([[maybe_unused]] VkDevice Device, [[maybe_unused]] const VkObjectType ObjectType, [[maybe_unused]] const uint64_t ObjectHandle, [[maybe_unused]] std::string_view ObjectName) 
{
#ifdef USE_RENDERDOC
	if (VK_NULL_HANDLE != vkSetDebugUtilsObjectName) {
		const VkDebugUtilsObjectNameInfoEXT DUONI = {
			.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
			.pNext = nullptr,
			.objectType = ObjectType, .objectHandle = ObjectHandle,
			.pObjectName = data(ObjectName)
		};
		VERIFY_SUCCEEDED(vkSetDebugUtilsObjectName(Device, &DUONI));
	}
#endif
}
void VK::SetObjectTag([[maybe_unused]] VkDevice Device, [[maybe_unused]] const VkObjectType ObjectType, [[maybe_unused]] const uint64_t ObjectHandle, [[maybe_unused]] const uint64_t TagName, [[maybe_unused]] const size_t TagSize, [[maybe_unused]] const void* TagData) 
{
#ifdef USE_RENDERDOC
	if (VK_NULL_HANDLE != vkSetDebugUtilsObjectTag) {
		const VkDebugUtilsObjectTagInfoEXT DUOTI = {
			.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_TAG_INFO_EXT,
			.pNext = nullptr,
			.objectType = ObjectType, .objectHandle = ObjectHandle,
			.tagName = TagName, .tagSize = TagSize, .pTag = TagData
		};
		VERIFY_SUCCEEDED(vkSetDebugUtilsObjectTag(Device, &DUOTI));
	}
#endif
}
#endif
