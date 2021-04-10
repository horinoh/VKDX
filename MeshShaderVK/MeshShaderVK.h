#pragma once

#include "resource.h"

#pragma region Code
#include "../VKExt.h"

class MeshShaderVK : public VKExt
{
private:
	using Super = VKExt;
public:
	MeshShaderVK() : Super() {}
	virtual ~MeshShaderVK() {}

	virtual void CreateDevice(HWND hWnd, HINSTANCE hInstance, [[maybe_unused]] void* pNext) override {
		VkPhysicalDeviceMeshShaderFeaturesNV PDMSF = {
			.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_FEATURES_NV,
			.pNext = nullptr,
			.taskShader = VK_TRUE,
			.meshShader = VK_TRUE,
		};
		Super::CreateDevice(hWnd, hInstance, &PDMSF);
	}
	virtual void CreatePipelineLayout() override {}
	virtual void CreatePipeline() override;
};
#pragma endregion