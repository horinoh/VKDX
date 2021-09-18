#pragma once

#include "resource.h"

#pragma region Code
#include "../DXExt.h"

class MeshletDX : public DXExt
{
private:
	using Super = DXExt;
public:
	MeshletDX() : Super() {}
	virtual ~MeshletDX() {}

	virtual void CreateGeometry() override {
		if (HasMeshShaderSupport(COM_PTR_GET(Device))) {
			const auto CA = COM_PTR_GET(DirectCommandAllocators[0]);
			const auto GCL = COM_PTR_GET(DirectCommandLists[0]);
			const auto CQ = COM_PTR_GET(GraphicsCommandQueue);
			//!< ƒg[ƒ‰ƒX16ŒÂ (16 of torus)
			constexpr D3D12_DISPATCH_MESH_ARGUMENTS DMA = { .ThreadGroupCountX = 16, .ThreadGroupCountY = 1, .ThreadGroupCountZ = 1 };
			IndirectBuffers.emplace_back().Create(COM_PTR_GET(Device), DMA).ExecuteCopyCommand(COM_PTR_GET(Device), CA, GCL, CQ, COM_PTR_GET(Fence), sizeof(DMA), &DMA);
		}
	}
	virtual void CreatePipelineState() override {
		if (HasMeshShaderSupport(COM_PTR_GET(Device))) {
			const auto ShaderPath = GetBasePath();
			std::vector<COM_PTR<ID3DBlob>> SBs;
			VERIFY_SUCCEEDED(D3DReadFileToBlob(data(ShaderPath + TEXT(".as.cso")), COM_PTR_PUT(SBs.emplace_back())));
			VERIFY_SUCCEEDED(D3DReadFileToBlob(data(ShaderPath + TEXT(".ms.cso")), COM_PTR_PUT(SBs.emplace_back())));
			VERIFY_SUCCEEDED(D3DReadFileToBlob(data(ShaderPath + TEXT(".ps.cso")), COM_PTR_PUT(SBs.emplace_back())));
			const std::array SBCs = {
				D3D12_SHADER_BYTECODE({.pShaderBytecode = SBs[0]->GetBufferPointer(), .BytecodeLength = SBs[0]->GetBufferSize() }),
				D3D12_SHADER_BYTECODE({.pShaderBytecode = SBs[1]->GetBufferPointer(), .BytecodeLength = SBs[1]->GetBufferSize() }),
				D3D12_SHADER_BYTECODE({.pShaderBytecode = SBs[2]->GetBufferPointer(), .BytecodeLength = SBs[2]->GetBufferSize() }),
			};
			CreatePipelineState_AsMsPs(FALSE, SBCs);
		}
	}
	virtual void PopulateCommandList(const size_t i) override {
		const auto HasMS = HasMeshShaderSupport(COM_PTR_GET(Device));
		const auto PS = HasMS ? COM_PTR_GET(PipelineStates[0]) : nullptr;
		const auto GCL = COM_PTR_GET(DirectCommandLists[i]);
		const auto CA = COM_PTR_GET(DirectCommandAllocators[0]);

		VERIFY_SUCCEEDED(GCL->Reset(CA, PS));
		{
			GCL->SetGraphicsRootSignature(COM_PTR_GET(RootSignatures[0]));

			GCL->RSSetViewports(static_cast<UINT>(size(Viewports)), data(Viewports));
			GCL->RSSetScissorRects(static_cast<UINT>(size(ScissorRects)), data(ScissorRects));

			const auto SCR = COM_PTR_GET(SwapChainResources[i]);
			ResourceBarrier(GCL, SCR, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
			{
				if (HasMS) {
					constexpr std::array<D3D12_RECT, 0> Rects = {};
					GCL->ClearRenderTargetView(SwapChainCPUHandles[i], DirectX::Colors::SkyBlue, static_cast<UINT>(size(Rects)), data(Rects));

					const std::array CHs = { SwapChainCPUHandles[i] };
					GCL->OMSetRenderTargets(static_cast<UINT>(size(CHs)), data(CHs), FALSE, nullptr);

					GCL->ExecuteIndirect(COM_PTR_GET(IndirectBuffers[0].CommandSignature), 1, COM_PTR_GET(IndirectBuffers[0].Resource), 0, nullptr, 0);
				}
				else {
					constexpr std::array<D3D12_RECT, 0> Rects = {};
					GCL->ClearRenderTargetView(SwapChainCPUHandles[i], DirectX::Colors::SkyBlue, static_cast<UINT>(size(Rects)), data(Rects));
				}
			}
			ResourceBarrier(GCL, SCR, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
		}
		VERIFY_SUCCEEDED(GCL->Close());
	}
};
#pragma endregion