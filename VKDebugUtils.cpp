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
				std::cout << "\t\t" << "[" << i << "]" << Label.pLabelName << "(" << Label.color[0] << ", " << Label.color[1] << ", " << Label.color[2] << ", " << Label.color[3] << ")";
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
						VK_OBJECT_TYPE_ENTRY(UNKNOWN);
						VK_OBJECT_TYPE_ENTRY(INSTANCE);
						VK_OBJECT_TYPE_ENTRY(PHYSICAL_DEVICE);
						VK_OBJECT_TYPE_ENTRY(DEVICE);
						VK_OBJECT_TYPE_ENTRY(QUEUE);
						VK_OBJECT_TYPE_ENTRY(SEMAPHORE);
						VK_OBJECT_TYPE_ENTRY(COMMAND_BUFFER);
						VK_OBJECT_TYPE_ENTRY(FENCE);
						VK_OBJECT_TYPE_ENTRY(DEVICE_MEMORY);
						VK_OBJECT_TYPE_ENTRY(BUFFER);
						VK_OBJECT_TYPE_ENTRY(IMAGE);
						VK_OBJECT_TYPE_ENTRY(EVENT);
						VK_OBJECT_TYPE_ENTRY(QUERY_POOL);
						VK_OBJECT_TYPE_ENTRY(BUFFER_VIEW);
						VK_OBJECT_TYPE_ENTRY(IMAGE_VIEW);
						VK_OBJECT_TYPE_ENTRY(SHADER_MODULE);
						VK_OBJECT_TYPE_ENTRY(PIPELINE_CACHE);
						VK_OBJECT_TYPE_ENTRY(PIPELINE_LAYOUT);
						VK_OBJECT_TYPE_ENTRY(RENDER_PASS);
						VK_OBJECT_TYPE_ENTRY(PIPELINE);
						VK_OBJECT_TYPE_ENTRY(DESCRIPTOR_SET_LAYOUT);
						VK_OBJECT_TYPE_ENTRY(SAMPLER);
						VK_OBJECT_TYPE_ENTRY(DESCRIPTOR_POOL);
						VK_OBJECT_TYPE_ENTRY(DESCRIPTOR_SET);
						VK_OBJECT_TYPE_ENTRY(FRAMEBUFFER);
						VK_OBJECT_TYPE_ENTRY(COMMAND_POOL);
						VK_OBJECT_TYPE_ENTRY(SAMPLER_YCBCR_CONVERSION);
						VK_OBJECT_TYPE_ENTRY(DESCRIPTOR_UPDATE_TEMPLATE);
						VK_OBJECT_TYPE_ENTRY(PRIVATE_DATA_SLOT);
						VK_OBJECT_TYPE_ENTRY(SURFACE_KHR);
						VK_OBJECT_TYPE_ENTRY(SWAPCHAIN_KHR);
						VK_OBJECT_TYPE_ENTRY(DISPLAY_KHR);
						VK_OBJECT_TYPE_ENTRY(DISPLAY_MODE_KHR);
						VK_OBJECT_TYPE_ENTRY(DEBUG_REPORT_CALLBACK_EXT);
						VK_OBJECT_TYPE_ENTRY(VIDEO_SESSION_KHR);
						VK_OBJECT_TYPE_ENTRY(VIDEO_SESSION_PARAMETERS_KHR);
						VK_OBJECT_TYPE_ENTRY(CU_MODULE_NVX);
						VK_OBJECT_TYPE_ENTRY(CU_FUNCTION_NVX);
						VK_OBJECT_TYPE_ENTRY(DEBUG_UTILS_MESSENGER_EXT);
						VK_OBJECT_TYPE_ENTRY(ACCELERATION_STRUCTURE_KHR);
						VK_OBJECT_TYPE_ENTRY(VALIDATION_CACHE_EXT);
						VK_OBJECT_TYPE_ENTRY(ACCELERATION_STRUCTURE_NV);
						VK_OBJECT_TYPE_ENTRY(PERFORMANCE_CONFIGURATION_INTEL);
						VK_OBJECT_TYPE_ENTRY(DEFERRED_OPERATION_KHR);
						VK_OBJECT_TYPE_ENTRY(INDIRECT_COMMANDS_LAYOUT_NV);
						VK_OBJECT_TYPE_ENTRY(BUFFER_COLLECTION_FUCHSIA);
						VK_OBJECT_TYPE_ENTRY(MICROMAP_EXT);
						VK_OBJECT_TYPE_ENTRY(OPTICAL_FLOW_SESSION_NV);
						VK_OBJECT_TYPE_ENTRY(SHADER_EXT);

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
