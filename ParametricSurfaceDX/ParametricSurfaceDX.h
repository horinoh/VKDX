#pragma once

#include "resource.h"

#pragma region Code
#include "../DXExt.h"

class ParametricSurfaceDX : public DXExt
{
private:
	using Super = DXExt;
public:
	ParametricSurfaceDX() : Super() {}
	virtual ~ParametricSurfaceDX() {}

protected:
#ifndef USE_BUNDLE
	//!< �f�t�H���g�̓o���h�����쐬��������Ȃ̂ŃI�[�o�[���C�h����
	virtual void CreateCommandAllocator() override {
		CommandAllocators.resize(1);
		VERIFY_SUCCEEDED(Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, COM_PTR_UUIDOF_PUTVOID(CommandAllocators[0])));
		LOG_OK();
	}
	virtual void CreateCommandList() override {
		DXGI_SWAP_CHAIN_DESC1 SCD;
		SwapChain->GetDesc1(&SCD);
		for (UINT i = 0; i < SCD.BufferCount; ++i) {
			GraphicsCommandLists.push_back(COM_PTR<ID3D12GraphicsCommandList>());
			VERIFY_SUCCEEDED(Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, COM_PTR_GET(CommandAllocators[0]), nullptr, COM_PTR_UUIDOF_PUTVOID(GraphicsCommandLists.back())));
			VERIFY_SUCCEEDED(GraphicsCommandLists[i]->Close());
		}
		LOG_OK();
	}
#endif
	virtual void CreateIndirectBuffer() override { CreateIndirectBuffer_DrawIndexed(1, 1); } //!< �Œ�ł��C���f�b�N�X��1���K�v (At least index count must be 1)
	virtual void CreateShaderBlobs() override { CreateShaderBlob_VsPsDsHsGs(); }
	virtual void CreatePipelineStates() override { CreatePipelineState_VsPsDsHsGs(D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH, FALSE); }
	virtual void PopulateCommandList(const size_t i) override;
};
#pragma endregion