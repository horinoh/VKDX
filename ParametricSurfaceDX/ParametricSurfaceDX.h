#pragma once

#include "resource.h"

#pragma region Code
#include "../DXExt.h"

//!< �p�����g���b�N�T�[�t�F�X(ParametricSurface) http://www.3d-meier.de/tut3/Seite0.html

class ParametricSurfaceDX : public DXExt
{
private:
	using Super = DXExt;
public:
	ParametricSurfaceDX() : Super() {}
	virtual ~ParametricSurfaceDX() {}

protected:
#ifdef USE_NO_BUNDLE
	//!< �f�t�H���g�����̓o���h�����쐬��������Ȃ̂ŁA�I�[�o�[���C�h���č쐬���Ȃ��悤�ɂ��Ă���
	virtual void CreateCommandAllocator() override {
		CommandAllocators.emplace_back(COM_PTR<ID3D12CommandAllocator>());
		VERIFY_SUCCEEDED(Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, COM_PTR_UUIDOF_PUTVOID(CommandAllocators.back())));
		LOG_OK();
	}
	virtual void CreateCommandList() override {
		DXGI_SWAP_CHAIN_DESC1 SCD;
		SwapChain->GetDesc1(&SCD);
		for (UINT i = 0; i < SCD.BufferCount; ++i) {
			GraphicsCommandLists.emplace_back(COM_PTR<ID3D12GraphicsCommandList>());
			VERIFY_SUCCEEDED(Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, COM_PTR_GET(CommandAllocators[0]), nullptr, COM_PTR_UUIDOF_PUTVOID(GraphicsCommandLists.back())));
			VERIFY_SUCCEEDED(GraphicsCommandLists.back()->Close());
		}
		LOG_OK();
	}
#endif

	virtual void CreateBottomLevel() override { CreateIndirectBuffer_DrawIndexed(1, 1); } //!< �Œ�ł��C���f�b�N�X��1���K�v (At least index count must be 1)
	
	virtual void CreateShaderBlobs() override { CreateShaderBlob_VsPsDsHsGs(); }
	virtual void CreatePipelineStates() override { CreatePipelineState_VsPsDsHsGs(D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH, FALSE); }

	virtual void PopulateCommandList(const size_t i) override;
};
#pragma endregion