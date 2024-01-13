#pragma once

#include "resource.h"

#pragma region Code
#include "../DXMS.h"

class MeshletDX : public DXMS
{
private:
	using Super = DXMS;
public:
	MeshletDX() : Super() {}
	virtual ~MeshletDX() {}

	virtual void CreateGeometry() override {
		if (HasMeshShaderSupport(COM_PTR_GET(Device))) {
			const auto CA = COM_PTR_GET(DirectCommandAllocators[0]);
			const auto CL = COM_PTR_GET(DirectCommandLists[0]);
			const auto CQ = COM_PTR_GET(GraphicsCommandQueue);
			//!< ƒg[ƒ‰ƒX16ŒÂ (16 of torus)
			constexpr D3D12_DISPATCH_MESH_ARGUMENTS DMA = { .ThreadGroupCountX = 16, .ThreadGroupCountY = 1, .ThreadGroupCountZ = 1 };
			IndirectBuffers.emplace_back().Create(COM_PTR_GET(Device), DMA).ExecuteCopyCommand(COM_PTR_GET(Device), CA, CL, CQ, COM_PTR_GET(GraphicsFence), sizeof(DMA), &DMA);
		}
	}
	virtual void CreatePipelineState() override {
		if (HasMeshShaderSupport(COM_PTR_GET(Device))) {
			PipelineStates.emplace_back();

			std::vector<COM_PTR<ID3DBlob>> SBs;
			VERIFY_SUCCEEDED(D3DReadFileToBlob(data(GetFilePath(".as.cso").wstring()), COM_PTR_PUT(SBs.emplace_back())));
			VERIFY_SUCCEEDED(D3DReadFileToBlob(data(GetFilePath(".ms.cso").wstring()), COM_PTR_PUT(SBs.emplace_back())));
			VERIFY_SUCCEEDED(D3DReadFileToBlob(data(GetFilePath(".ps.cso").wstring()), COM_PTR_PUT(SBs.emplace_back())));
			const std::array SBCs = {
				D3D12_SHADER_BYTECODE({.pShaderBytecode = SBs[0]->GetBufferPointer(), .BytecodeLength = SBs[0]->GetBufferSize() }),
				D3D12_SHADER_BYTECODE({.pShaderBytecode = SBs[1]->GetBufferPointer(), .BytecodeLength = SBs[1]->GetBufferSize() }),
				D3D12_SHADER_BYTECODE({.pShaderBytecode = SBs[2]->GetBufferPointer(), .BytecodeLength = SBs[2]->GetBufferSize() }),
			};
			CreatePipelineState_AsMsPs(PipelineStates[0], COM_PTR_GET(RootSignatures[0]), FALSE, SBCs);

			for (auto& i : Threads) { i.join(); }
			Threads.clear();
		}
	}
	virtual void PopulateCommandList(const size_t i) override {
		const auto HasMS = HasMeshShaderSupport(COM_PTR_GET(Device));
		const auto PS = HasMS ? COM_PTR_GET(PipelineStates[0]) : nullptr;
		const auto CL = COM_PTR_GET(DirectCommandLists[i]);
		const auto CA = COM_PTR_GET(DirectCommandAllocators[0]);

		VERIFY_SUCCEEDED(CL->Reset(CA, PS));
		{
			CL->SetGraphicsRootSignature(COM_PTR_GET(RootSignatures[0]));

			CL->RSSetViewports(static_cast<UINT>(size(Viewports)), data(Viewports));
			CL->RSSetScissorRects(static_cast<UINT>(size(ScissorRects)), data(ScissorRects));

			const auto SCR = COM_PTR_GET(SwapchainBackBuffers[i].Resource);
			ResourceBarrier(CL, SCR, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
			{
				if (HasMS) {
					constexpr std::array<D3D12_RECT, 0> Rects = {};
					CL->ClearRenderTargetView(SwapchainBackBuffers[i].Handle, DirectX::Colors::SkyBlue, static_cast<UINT>(size(Rects)), data(Rects));

					const std::array CHs = { SwapchainBackBuffers[i].Handle };
					CL->OMSetRenderTargets(static_cast<UINT>(size(CHs)), data(CHs), FALSE, nullptr);

					CL->ExecuteIndirect(COM_PTR_GET(IndirectBuffers[0].CommandSignature), 1, COM_PTR_GET(IndirectBuffers[0].Resource), 0, nullptr, 0);
				}
				else {
					constexpr std::array<D3D12_RECT, 0> Rects = {};
					CL->ClearRenderTargetView(SwapchainBackBuffers[i].Handle, DirectX::Colors::SkyBlue, static_cast<UINT>(size(Rects)), data(Rects));
				}
			}
			ResourceBarrier(CL, SCR, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
		}
		VERIFY_SUCCEEDED(CL->Close());
	}
};
#pragma endregion