#pragma once

#include "VKExt.h"

class VKMS : public VKExt
{
private:
	using Super = VKExt;

protected:
	[[nodiscard]] static bool HasMeshShaderSupport(const VkPhysicalDevice PD) {
#ifdef USE_NV_MESH_SHADER
		return HasMeshShaderNVSupport(PD);
#else
		return HasMeshShaderEXTSupport(PD);
#endif
	}

	virtual void CreateInstance([[maybe_unused]] const std::vector<const char*>& AdditionalLayers, const std::vector<const char*>& AdditionalExtensions) override {
		//Super::CreateInstance(AdditionalLayers, AdditionalExtensions);	//!< VKExt	: VK_LAYER_RENDERDOC_Capture を使用する
		VK::CreateInstance(AdditionalLayers, AdditionalExtensions);			//!< VK		: VK_LAYER_RENDERDOC_Capture を使用しない
	}
	virtual void CreateDevice(HWND hWnd, HINSTANCE hInstance, [[maybe_unused]] void* pNext, [[maybe_unused]] const std::vector<const char*>& AddExtensions) override {
		if (HasMeshShaderSupport(GetCurrentPhysicalDevice())) {
			//!< VkPhysicalDeviceMeshShaderPropertiesEXT PDMSP はよく使うのでメンバとして覚えておく
			VkPhysicalDeviceProperties2 PDP2 = { .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2, .pNext = &PDMSP, };
			vkGetPhysicalDeviceProperties2(GetCurrentPhysicalDevice(), &PDP2);

			VulkanFeature VF;
#ifdef USE_NV_MESH_SHADER
			VkPhysicalDeviceMeshShaderFeaturesNV PDMSF = { .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_FEATURES_NV, .pNext = VF.GetPtr(), .taskShader = VK_TRUE, .meshShader = VK_TRUE, };
			Super::CreateDevice(hWnd, hInstance, &PDMSF, { VK_NV_MESH_SHADER_EXTENSION_NAME });
#else
			VkPhysicalDeviceMeshShaderFeaturesEXT PDMSF = { .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_FEATURES_EXT, .pNext = VF.GetPtr(), .taskShader = VK_TRUE, .meshShader = VK_TRUE, .multiviewMeshShader = VK_FALSE, .primitiveFragmentShadingRateMeshShader = VK_FALSE, .meshShaderQueries = VK_FALSE};
			Super::CreateDevice(hWnd, hInstance, &PDMSF, { VK_EXT_MESH_SHADER_EXTENSION_NAME });
#endif
		}
		else {
			Super::CreateDevice(hWnd, hInstance, pNext, AddExtensions);
		}
	}
	virtual void CreateRenderPass() { VKExt::CreateRenderPass_Clear(); }

#ifdef USE_NV_MESH_SHADER
	VkPhysicalDeviceMeshShaderPropertiesNV PDMSP = { .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_PROPERTIES_NV, .pNext = nullptr };
#else
	VkPhysicalDeviceMeshShaderPropertiesEXT PDMSP = { .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_PROPERTIES_EXT, .pNext = nullptr };
#endif
};

class VKMSDepth : public VKExtDepth
{
private:
	using Super = VKExtDepth;

protected:
	[[nodiscard]] static bool HasMeshShaderSupport(const VkPhysicalDevice PD) {
#ifdef USE_NV_MESH_SHADER
		return HasMeshShaderNVSupport(PD);
#else
		return HasMeshShaderEXTSupport(PD);
#endif
	}
	virtual void CreateInstance([[maybe_unused]] const std::vector<const char*>& AdditionalLayers, const std::vector<const char*>& AdditionalExtensions) override {
		Super::CreateInstance(AdditionalLayers, AdditionalExtensions);
	}
	virtual void CreateDevice(HWND hWnd, HINSTANCE hInstance, [[maybe_unused]] void* pNext, [[maybe_unused]] const std::vector<const char*>& AddExtensions) override {
		if (HasMeshShaderSupport(GetCurrentPhysicalDevice())) {
			//!< VkPhysicalDeviceMeshShaderPropertiesEXT PDMSP はよく使うのでメンバとして覚えておく
			VkPhysicalDeviceProperties2 PDP2 = { .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2, .pNext = &PDMSP, };
			vkGetPhysicalDeviceProperties2(GetCurrentPhysicalDevice(), &PDP2);

			VulkanFeature VF;
#ifdef USE_NV_MESH_SHADER
			VkPhysicalDeviceMeshShaderFeaturesNV PDMSF = { .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_FEATURES_NV, .pNext = VF.GetPtr(), .taskShader = VK_TRUE, .meshShader = VK_TRUE, };
			Super::CreateDevice(hWnd, hInstance, &PDMSF, { VK_NV_MESH_SHADER_EXTENSION_NAME });
#else
			VkPhysicalDeviceMeshShaderFeaturesEXT PDMSF = { .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_FEATURES_EXT, .pNext = VF.GetPtr(), .taskShader = VK_TRUE, .meshShader = VK_TRUE, .multiviewMeshShader = VK_FALSE, .primitiveFragmentShadingRateMeshShader = VK_FALSE, .meshShaderQueries = VK_FALSE };
			Super::CreateDevice(hWnd, hInstance, &PDMSF, { VK_EXT_MESH_SHADER_EXTENSION_NAME });
#endif
		}
		else {
			Super::CreateDevice(hWnd, hInstance, pNext, AddExtensions);
		}
	}

#ifdef USE_NV_MESH_SHADER
	VkPhysicalDeviceMeshShaderPropertiesNV PDMSP = { .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_PROPERTIES_NV, .pNext = nullptr };
#else
	VkPhysicalDeviceMeshShaderPropertiesEXT PDMSP = { .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_PROPERTIES_EXT, .pNext = nullptr };
#endif
};