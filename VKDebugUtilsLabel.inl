#pragma once

#pragma region VkQueue
template<> void InsertLabel([[maybe_unused]] VkQueue Queue, [[maybe_unused]] const glm::vec4& Color, [[maybe_unused]] std::string_view Name) {
#ifdef USE_RENDERDOC
	if (VK_NULL_HANDLE != vkQueueInsertDebugUtilsLabel) {
		const VkDebugUtilsLabelEXT DUL = {
			.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT,
			.pNext = nullptr,
			.pLabelName = data(Name),
			.color = { Color.r, Color.g, Color.b, Color.a },
		};
		vkQueueInsertDebugUtilsLabel(Queue, &DUL);
	}
#endif
}
template<> void BeginLabel([[maybe_unused]] VkQueue Queue, [[maybe_unused]] const glm::vec4& Color, [[maybe_unused]] std::string_view Name) {
#ifdef USE_RENDERDOC
	if (VK_NULL_HANDLE != vkQueueBeginDebugUtilsLabel) {
		const VkDebugUtilsLabelEXT DUL = {
			.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT,
			.pNext = nullptr,
			.pLabelName = data(Name),
			.color = { Color.r, Color.g, Color.b, Color.a }
		};
		vkQueueBeginDebugUtilsLabel(Queue, &DUL);
	}
#endif
}
template<> void EndLabel([[maybe_unused]] VkQueue Queue) {
#ifdef USE_RENDERDOC
	if (VK_NULL_HANDLE != vkQueueEndDebugUtilsLabel) { vkQueueEndDebugUtilsLabel(Queue); }
#endif
}
#pragma endregion

#pragma region VkCommandBuffer
template<> void InsertLabel([[maybe_unused]] VkCommandBuffer CB, [[maybe_unused]] const glm::vec4& Color, [[maybe_unused]] std::string_view Name) {
#ifdef USE_RENDERDOC
	if (VK_NULL_HANDLE != vkCmdInsertDebugUtilsLabel) {
		const VkDebugUtilsLabelEXT DUL = {
			.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT,
			.pNext = nullptr,
			.pLabelName = data(Name),
			.color = { Color.r, Color.g, Color.b, Color.a },
		};
		vkCmdInsertDebugUtilsLabel(CB, &DUL);
	}
#endif
}
template<> void BeginLabel([[maybe_unused]] VkCommandBuffer CB, [[maybe_unused]] const glm::vec4& Color, [[maybe_unused]] std::string_view Name) {
#ifdef USE_RENDERDOC
	if (VK_NULL_HANDLE != vkCmdBeginDebugUtilsLabel) {
		const VkDebugUtilsLabelEXT DUL = {
			.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT,
			.pNext = nullptr,
			.pLabelName = data(Name),
			.color = { Color.r, Color.g, Color.b, Color.a }
		};
		vkCmdBeginDebugUtilsLabel(CB, &DUL);
	}
#endif
}
template<> void EndLabel([[maybe_unused]] VkCommandBuffer CB) {
#ifdef USE_RENDERDOC
	if (VK_NULL_HANDLE != vkCmdEndDebugUtilsLabel) { vkCmdEndDebugUtilsLabel(CB); }
#endif
}
#pragma endregion