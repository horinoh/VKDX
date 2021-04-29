#pragma once

#include "resource.h"

#pragma region Code
#include "../VKExt.h"

class TriangleVK : public VKExt
{
private:
	using Super = VKExt;
public:
	TriangleVK() : Super() {}
	virtual ~TriangleVK() {}

protected:
#ifdef USE_RENDERDOC
	virtual void CreateInstance([[maybe_unused]] const std::vector<const char*>& AdditionalLayers, const std::vector<const char*>& AdditionalExtensions) override {
		//!< #TIPS�@���C�g���[�V���O�⃁�b�V���V�F�[�_�[�Ɠ����� "VK_LAYER_RENDERDOC_Capture" ���g�p�����ꍇ�� vkCreateDevice() �ŃR�P�Ă����̂ŁA�p�r�ɂ���Ă͒��ӂ��K�v
		Super::CreateInstance({ "VK_LAYER_RENDERDOC_Capture" }, AdditionalExtensions);
	}
	virtual void CreateDevice(HWND hWnd, HINSTANCE hInstance, void* pNext, [[maybe_unused]] const std::vector<const char*>& AdditionalExtensions) override { 
		Super::CreateDevice(hWnd, hInstance, pNext, { VK_EXT_DEBUG_MARKER_EXTENSION_NAME }); 
	}
#endif
	virtual void CreateGeometry() override;
	virtual void CreatePipelineLayout() override {
		//!< �y�v�b�V���R���X�^���g�z : �f�X�N���v�^�Z�b�g��������
		//!< �p�C�v���C�����C�A�E�g�S�̂�128byte (�n�[�h�ɂ�肱��ȏ�g����ꍇ������AGTX970M�̏ꍇ��256byte������)
		//!< �e�V�F�[�_�X�e�[�W��1�̃v�b�V���R���X�^���g�����W�ɂ����A�N�Z�X�ł��Ȃ�
		//!< �e�V�F�[�_�X�e�[�W���u���ʂ̃����W�������Ȃ��v�悤�ȁu���[�X�g�P�[�X�v�ł� 128/5==25.6�A1�V�F�[�_�X�e�[�W��25byte���x�ƂȂ�
		VK::CreatePipelineLayout(PipelineLayouts.emplace_back(), {}, { 
#ifdef USE_PUSH_CONSTANTS
			VkPushConstantRange({.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT, .offset = 0, .size = static_cast<uint32_t>(size(Color) * sizeof(Color[0])) })
#endif
		});
	}
	virtual void CreateRenderPass() override { VKExt::CreateRenderPass_Clear(); }
	virtual void CreatePipeline() override {
		const auto ShaderPath = GetBasePath();
		const std::array SMs = {
#ifdef USE_PUSH_CONSTANTS
			VK::CreateShaderModule(data(ShaderPath + TEXT(".vert.spv"))),
			VK::CreateShaderModule(data(ShaderPath + TEXT("_pc.frag.spv"))) 
#else
			VK::CreateShaderModule(data(ShaderPath + TEXT(".vert.spv"))),
			VK::CreateShaderModule(data(ShaderPath + TEXT(".frag.spv"))) 
#endif
		};
		const std::array PSSCIs = {
			VkPipelineShaderStageCreateInfo({ .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, .pNext = nullptr, .flags = 0, .stage = VK_SHADER_STAGE_VERTEX_BIT, .module = SMs[0], .pName = "main", .pSpecializationInfo = nullptr }),
			VkPipelineShaderStageCreateInfo({ .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, .pNext = nullptr, .flags = 0, .stage = VK_SHADER_STAGE_FRAGMENT_BIT, .module = SMs[1], .pName = "main", .pSpecializationInfo = nullptr }),
		};
		//!< �o�C���f�B���O0�ɂ܂Ƃ߂ē����C���^�[���[�u�A�Z�}���e�B�b�N�X���Ƀo�C���f�B���O�𕪂���Ɣ�C���^�[���[�u�ƂȂ�
		const std::vector VIBDs = { 
			VkVertexInputBindingDescription({.binding = 0, .stride = sizeof(Vertex_PositionColor), .inputRate = VK_VERTEX_INPUT_RATE_VERTEX }),
		};
		//!< �l�܂��Ă��Ă��ADX �� D3D12_APPEND_ALIGNED_ELEMENT �̂悤�� offsetof() ����������i�͖���? (Is there no D3D12_APPEND_ALIGNED_ELEMENT equivalent?)
		const std::vector VIADs = { 
			VkVertexInputAttributeDescription({.location = 0, .binding = 0, .format = VK_FORMAT_R32G32B32_SFLOAT, .offset = offsetof(Vertex_PositionColor, Position) }),
			VkVertexInputAttributeDescription({.location = 1, .binding = 0, .format = VK_FORMAT_R32G32B32A32_SFLOAT, .offset = offsetof(Vertex_PositionColor, Color) }),
		};
		VKExt::CreatePipeline_VsFs_Input(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP, 0, VK_FALSE, VIBDs, VIADs, PSSCIs);
		
		for (auto i : SMs) { vkDestroyShaderModule(Device, i, GetAllocationCallbacks()); }
	}

	virtual void PopulateCommandBuffer(const size_t i) override;

#ifdef USE_PUSH_CONSTANTS
	const std::array<float, 4> Color = { 1.0f, 0.0f, 0.0f, 1.0f };
#endif
};
#pragma endregion