#pragma once

#include "resource.h"

#pragma region Code
#include "../VKExt.h"

class MeshletVK : public VKExt
{
private:
	using Super = VKExt;
public:
	MeshletVK() : Super() {}
	virtual ~MeshletVK() {}

	void CreateGeometry() override {
		const auto& CB = CommandBuffers[0];
		const auto PDMP = GetCurrentPhysicalDeviceMemoryProperties();
		constexpr VkDrawMeshTasksIndirectCommandNV DMTIC = { .taskCount = 3, .firstTask = 0 };
		IndirectBuffers.emplace_back().Create(Device, PDMP, DMTIC).SubmitCopyCommand(Device, PDMP, CB, GraphicsQueue, sizeof(DMTIC), &DMTIC);
	}
	virtual void CreateDevice(HWND hWnd, HINSTANCE hInstance, [[maybe_unused]] void* pNext, [[maybe_unused]] const std::vector<const char*>& AddExtensions) override {
		if (HasMeshShaderSupport(GetCurrentPhysicalDevice())) {
			VkPhysicalDeviceMeshShaderFeaturesNV PDMSF = { .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_FEATURES_NV, .pNext = nullptr, .taskShader = VK_TRUE, .meshShader = VK_TRUE, };
			Super::CreateDevice(hWnd, hInstance, &PDMSF, {
				VK_NV_MESH_SHADER_EXTENSION_NAME
				});
		}
		else {
			Super::CreateDevice(hWnd, hInstance);
		}
	}
	virtual void CreateRenderPass() { VKExt::CreateRenderPass_Clear(); }
	virtual void CreatePipeline() override {
		if (HasMeshShaderSupport(GetCurrentPhysicalDevice())) {
			const auto ShaderPath = GetBasePath();
			const std::array SMs = {
				VK::CreateShaderModule(data(ShaderPath + TEXT(".task.spv"))),
				VK::CreateShaderModule(data(ShaderPath + TEXT(".mesh.spv"))),
				VK::CreateShaderModule(data(ShaderPath + TEXT(".frag.spv"))),
			};
			const std::array PSSCIs = {
				VkPipelineShaderStageCreateInfo({.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, .pNext = nullptr, .flags = 0, .stage = VK_SHADER_STAGE_TASK_BIT_NV, .module = SMs[0], .pName = "main", .pSpecializationInfo = nullptr }),
				VkPipelineShaderStageCreateInfo({.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, .pNext = nullptr, .flags = 0, .stage = VK_SHADER_STAGE_MESH_BIT_NV, .module = SMs[1], .pName = "main", .pSpecializationInfo = nullptr }),
				VkPipelineShaderStageCreateInfo({.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, .pNext = nullptr, .flags = 0, .stage = VK_SHADER_STAGE_FRAGMENT_BIT, .module = SMs[2], .pName = "main", .pSpecializationInfo = nullptr }),
			};
			CreatePipeline_TsMsFs(VK_FALSE, PSSCIs);
			for (auto i : SMs) { vkDestroyShaderModule(Device, i, GetAllocationCallbacks()); }
		}
	}
	virtual void PopulateCommandBuffer(const size_t i) override {
		const auto CB = CommandBuffers[i];
		constexpr VkCommandBufferBeginInfo CBBI = {
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
			.pNext = nullptr,
			.flags = 0,
			.pInheritanceInfo = nullptr
		};
		VERIFY_SUCCEEDED(vkBeginCommandBuffer(CB, &CBBI)); {
			vkCmdSetViewport(CB, 0, static_cast<uint32_t>(size(Viewports)), data(Viewports));
			vkCmdSetScissor(CB, 0, static_cast<uint32_t>(size(ScissorRects)), data(ScissorRects));

			vkCmdBindPipeline(CB, VK_PIPELINE_BIND_POINT_GRAPHICS, Pipelines[0]);

			constexpr std::array CVs = { VkClearValue({.color = Colors::SkyBlue }) };
			const VkRenderPassBeginInfo RPBI = {
				.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
				.pNext = nullptr,
				.renderPass = RenderPasses[0],
				.framebuffer = Framebuffers[i],
				.renderArea = VkRect2D({.offset = VkOffset2D({.x = 0, .y = 0 }), .extent = SurfaceExtent2D }),
				.clearValueCount = static_cast<uint32_t>(size(CVs)), .pClearValues = data(CVs)
			};
			vkCmdBeginRenderPass(CB, &RPBI, VK_SUBPASS_CONTENTS_INLINE); {
				if (HasMeshShaderSupport(GetCurrentPhysicalDevice())) {
					vkCmdDrawMeshTasksIndirectNV(CB, IndirectBuffers[0].Buffer, 0, 1, 0);
				}
			} vkCmdEndRenderPass(CB);
		} VERIFY_SUCCEEDED(vkEndCommandBuffer(CB));
	}
};
#pragma endregion