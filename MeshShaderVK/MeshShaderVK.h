#pragma once

#include "resource.h"

#pragma region Code
#include "../VKMS.h"

class MeshShaderVK : public VKMS
{
private:
	using Super = VKMS;
public:
	MeshShaderVK() : Super() {}
	virtual ~MeshShaderVK() {}

#ifdef USE_INDIRECT
	void CreateGeometry() override {
		if (HasMeshShaderSupport(GetCurrentPhysicalDevice())) {
			const auto& CB = CommandBuffers[0];
			const auto PDMP = GetCurrentPhysicalDeviceMemoryProperties();
#ifdef USE_NV_MESH_SHADER
			constexpr VkDrawMeshTasksIndirectCommandNV DMTIC = { .taskCount = 1, .firstTask = 0 };
#else
			constexpr VkDrawMeshTasksIndirectCommandEXT DMTIC = { .groupCountX = 1, .groupCountY = 1, .groupCountZ = 1 };
#endif
			IndirectBuffers.emplace_back().Create(Device, PDMP, DMTIC).SubmitCopyCommand(Device, PDMP, CB, GraphicsQueue, sizeof(DMTIC), &DMTIC);
		}
	}
#endif
	virtual void CreatePipeline() override {
		if (HasMeshShaderSupport(GetCurrentPhysicalDevice())) {
			const auto ShaderPath = GetBasePath();
			const std::array SMs = {
				VK::CreateShaderModule(data(ShaderPath + TEXT(".mesh.spv"))),
				VK::CreateShaderModule(data(ShaderPath + TEXT(".frag.spv"))),
			};
			const std::array PSSCIs = {
#ifdef USE_NV_MESH_SHADER
				VkPipelineShaderStageCreateInfo({.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, .pNext = nullptr, .flags = 0, .stage = VK_SHADER_STAGE_MESH_BIT_NV, .module = SMs[0], .pName = "main", .pSpecializationInfo = nullptr }),
#else
				VkPipelineShaderStageCreateInfo({.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, .pNext = nullptr, .flags = 0, .stage = VK_SHADER_STAGE_MESH_BIT_EXT, .module = SMs[0], .pName = "main", .pSpecializationInfo = nullptr }),
#endif
				VkPipelineShaderStageCreateInfo({.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, .pNext = nullptr, .flags = 0, .stage = VK_SHADER_STAGE_FRAGMENT_BIT, .module = SMs[1], .pName = "main", .pSpecializationInfo = nullptr }),
			};
			CreatePipeline_MsFs(VK_FALSE, PSSCIs);
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
#ifdef USE_INDIRECT
#ifdef USE_NV_MESH_SHADER
					vkCmdDrawMeshTasksIndirectNV(CB, IndirectBuffers[0].Buffer, 0, 1, 0);
#else
					vkCmdDrawMeshTasksIndirectEXT(CB, IndirectBuffers[0].Buffer, 0, 1, 0);
#endif
#else
#ifdef USE_NV_MESH_SHADER
					vkCmdDrawMeshTasksNV(CB, 1, 0);
#else
					vkCmdDrawMeshTasksEXT(CB, 1, 1, 1);
#endif
#endif
				}
			} vkCmdEndRenderPass(CB);
		} VERIFY_SUCCEEDED(vkEndCommandBuffer(CB));
	}
};
#pragma endregion