#pragma once

#include "VKExt.h"

class VKMS : public VKExt
{
private:
	using Super = VKExt;

protected:
	virtual void CreateInstance([[maybe_unused]] const std::vector<const char*>& AdditionalLayers, const std::vector<const char*>& AdditionalExtensions) override {
		//Super::CreateInstance(AdditionalLayers, AdditionalExtensions);	//!< VKExt	: VK_LAYER_RENDERDOC_Capture を使用する
		VK::CreateInstance(AdditionalLayers, AdditionalExtensions);			//!< VK		: VK_LAYER_RENDERDOC_Capture を使用しない
	}
	virtual void CreateDevice(HWND hWnd, HINSTANCE hInstance, [[maybe_unused]] void* pNext, [[maybe_unused]] const std::vector<const char*>& AddExtensions) override {
		if (HasMeshShaderSupport(GetCurrentPhysicalDevice())) {
			//!< VkPhysicalDeviceMeshShaderPropertiesNV は覚えておく
			VkPhysicalDeviceProperties2 PDP2 = { .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2, .pNext = &PDMSP, };
			vkGetPhysicalDeviceProperties2(GetCurrentPhysicalDevice(), &PDP2);

			VkPhysicalDeviceMeshShaderFeaturesNV PDMSF = { .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_FEATURES_NV, .pNext = nullptr, .taskShader = VK_TRUE, .meshShader = VK_TRUE, };
			Super::CreateDevice(hWnd, hInstance, &PDMSF, { VK_NV_MESH_SHADER_EXTENSION_NAME });
		}
		else {
			Super::CreateDevice(hWnd, hInstance, pNext, AddExtensions);
		}
	}
	virtual void CreateRenderPass() { VKExt::CreateRenderPass_Clear(); }

	VkPhysicalDeviceMeshShaderPropertiesNV PDMSP = { .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_PROPERTIES_NV, .pNext = nullptr };
};

class VKMSDepth : public VKExtDepth
{
private:
	using Super = VKExtDepth;

protected:
	virtual void CreateInstance([[maybe_unused]] const std::vector<const char*>& AdditionalLayers, const std::vector<const char*>& AdditionalExtensions) override {
		//Super::CreateInstance(AdditionalLayers, AdditionalExtensions);	//!< VKExt	: VK_LAYER_RENDERDOC_Capture を使用する
		VK::CreateInstance(AdditionalLayers, AdditionalExtensions);			//!< VK		: VK_LAYER_RENDERDOC_Capture を使用しない
	}
	virtual void CreateDevice(HWND hWnd, HINSTANCE hInstance, [[maybe_unused]] void* pNext, [[maybe_unused]] const std::vector<const char*>& AddExtensions) override {
		if (HasMeshShaderSupport(GetCurrentPhysicalDevice())) {
			VkPhysicalDeviceProperties2 PDP2 = { .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2, .pNext = &PDMSP, };
			vkGetPhysicalDeviceProperties2(GetCurrentPhysicalDevice(), &PDP2);

			VkPhysicalDeviceMeshShaderFeaturesNV PDMSF = { .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_FEATURES_NV, .pNext = nullptr, .taskShader = VK_TRUE, .meshShader = VK_TRUE, };
			Super::CreateDevice(hWnd, hInstance, &PDMSF, { VK_NV_MESH_SHADER_EXTENSION_NAME });
		}
		else {
			Super::CreateDevice(hWnd, hInstance, pNext, AddExtensions);
		}
	}

	VkPhysicalDeviceMeshShaderPropertiesNV PDMSP = { .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_PROPERTIES_NV, .pNext = nullptr };
};