#include "VK.h"

#ifdef USE_DEBUG_UTILS
VKAPI_ATTR VkBool32 VKAPI_CALL VK::MessengerCallback(VkDebugUtilsMessageSeverityFlagBitsEXT MessageSeverity, VkDebugUtilsMessageTypeFlagsEXT MessageTypes, const VkDebugUtilsMessengerCallbackDataEXT* CallbackData, void* UserData)
{
	if (MessageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT) {}
	if (MessageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT) {}
	if (MessageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {}
	if (MessageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {}

	if (MessageTypes & VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT) {}
	if (MessageTypes & VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT) {}
	if (MessageTypes & VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT) {}
	if (MessageTypes & VK_DEBUG_UTILS_MESSAGE_TYPE_DEVICE_ADDRESS_BINDING_BIT_EXT) {}

	if (nullptr != CallbackData) {
		//!< vk_validation_error_messages.h
		//UNIQUE_VALIDATION_ERROR_CODE;

		Logf("[%d]%s = %s\n", CallbackData->messageIdNumber, (nullptr != CallbackData->pMessageIdName ? CallbackData->pMessageIdName:""), CallbackData->pMessage);

		//!< キューに適用されたラベル ([0]が最新)
		if (0 < CallbackData->queueLabelCount) {
			Log("\tQueueLabel\n");
			for (uint32_t i = 0; i < CallbackData->queueLabelCount; ++i) {
				const auto& Label = CallbackData->pQueueLabels[i];
				Logf("\t\t[%d]%s\n", i, (nullptr != Label.pLabelName ? Label.pLabelName : ""));
				//Logf("\t\tColor=(%2.1f, %2.1f, %2.1f, %2.1f)\n", Label.color[0], Label.color[1], Label.color[2], Label.color[3]);
			}
		}

		//!< コマンドバッファに適用されたラベル ([0]が最新)
		if (0 < CallbackData->cmdBufLabelCount) {
			Log("\tCmdBufLabel\n");
			for (uint32_t i = 0; i < CallbackData->cmdBufLabelCount; ++i) {
				const auto& Label = CallbackData->pCmdBufLabels[i];
				Logf("\t\t[%d]%s\n", i, (nullptr != Label.pLabelName ? Label.pLabelName : ""));
				//Logf("\t\tColor=(%2.1f, %2.1f, %2.1f, %2.1f)\n", Label.color[0], Label.color[1], Label.color[2], Label.color[3]);
			}
		}

		//!< オブジェクト ([0]が最新)
		if (0 < CallbackData->objectCount) {
			Log("\tObject\n");
			for (uint32_t i = 0; i < CallbackData->objectCount; ++i) {
				const auto& ObjectNameInfo = CallbackData->pObjects[i];

#define VK_OBJECT_TYPE_ENTRY(entry) case VK_OBJECT_TYPE_##entry: Logf("\t\t[%d]%s = %s\n", i, (nullptr != ObjectNameInfo.pObjectName ? ObjectNameInfo.pObjectName : ""), #entry); break
					switch (ObjectNameInfo.objectType) {
					default: assert(0 && "Unknown VkObjectType"); break;
#include "VKObjectType.h"
					}
#undef VK_OBJECT_TYPE_ENTRY
			}
		}
	}

	if (nullptr != UserData) {}

	//!< WARNING 以上の場合に出力
	if (MessageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
		Log("ERROR : ");
		if (MessageTypes & VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT) { Log("\tGENERAL\n"); }
		if (MessageTypes & VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT) { Log("\tVALIDATION\n"); }
		if (MessageTypes & VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT) { Log("\tPERFORMANCE\n"); }
		if (MessageTypes & VK_DEBUG_UTILS_MESSAGE_TYPE_DEVICE_ADDRESS_BINDING_BIT_EXT) { Log("\tDEVICE_ADDRESS_BINDING\n"); }
	
		return VK_TRUE;
	}

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
