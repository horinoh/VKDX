#pragma once

#include "resource.h"

#pragma region Code
#include "../DXMS.h"

class MeshShaderDX : public DXMS
{
private:
	using Super = DXMS;
public:
	MeshShaderDX() : Super() {}
	virtual ~MeshShaderDX() {}

#ifdef USE_INDIRECT
	virtual void CreateGeometry() override {
		if (HasMeshShaderSupport(COM_PTR_GET(Device))) {
			const auto CA = COM_PTR_GET(DirectCommandAllocators[0]);
			const auto CL = COM_PTR_GET(DirectCommandLists[0]);
			const auto CQ = COM_PTR_GET(GraphicsCommandQueue);
			constexpr D3D12_DISPATCH_MESH_ARGUMENTS DMA = { .ThreadGroupCountX = 1, .ThreadGroupCountY = 1, .ThreadGroupCountZ = 1 };
			IndirectBuffers.emplace_back().Create(COM_PTR_GET(Device), DMA).ExecuteCopyCommand(COM_PTR_GET(Device), CA, CL, CQ, COM_PTR_GET(GraphicsFence), sizeof(DMA), &DMA);
		}
	}
#endif
	virtual void CreatePipelineState() override {
		if (HasMeshShaderSupport(COM_PTR_GET(Device))) {
			std::vector<COM_PTR<ID3DBlob>> SBs;
			VERIFY_SUCCEEDED(D3DReadFileToBlob(data(GetFilePath(".ms.cso").wstring()), COM_PTR_PUT(SBs.emplace_back())));
			VERIFY_SUCCEEDED(D3DReadFileToBlob(data(GetFilePath(".ps.cso").wstring()), COM_PTR_PUT(SBs.emplace_back())));
			const std::array SBCs = {
				D3D12_SHADER_BYTECODE({.pShaderBytecode = SBs[0]->GetBufferPointer(), .BytecodeLength = SBs[0]->GetBufferSize() }),
				D3D12_SHADER_BYTECODE({.pShaderBytecode = SBs[1]->GetBufferPointer(), .BytecodeLength = SBs[1]->GetBufferSize() }),
			};
			CreatePipelineState_MsPs(FALSE, SBCs);
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

			const auto SCR = COM_PTR_GET(SwapChainResources[i]);
			ResourceBarrier(CL, SCR, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
			{
				if (HasMS) {
					constexpr std::array<D3D12_RECT, 0> Rects = {};
					CL->ClearRenderTargetView(SwapChainCPUHandles[i], DirectX::Colors::SkyBlue, static_cast<UINT>(size(Rects)), data(Rects));

					const std::array CHs = { SwapChainCPUHandles[i] };
					CL->OMSetRenderTargets(static_cast<UINT>(size(CHs)), data(CHs), FALSE, nullptr);

#ifdef USE_INDIRECT
					CL->ExecuteIndirect(COM_PTR_GET(IndirectBuffers[0].CommandSignature), 1, COM_PTR_GET(IndirectBuffers[0].Resource), 0, nullptr, 0);
#else
					COM_PTR<ID3D12GraphicsCommandList6> CL6;
					VERIFY_SUCCEEDED(CL->QueryInterface(COM_PTR_UUIDOF_PUTVOID(CL6)));
					CL6->DispatchMesh(1, 1, 1);
#endif
				}
				else {
					constexpr std::array<D3D12_RECT, 0> Rects = {};
					CL->ClearRenderTargetView(SwapChainCPUHandles[i], DirectX::Colors::SkyBlue, static_cast<UINT>(size(Rects)), data(Rects));
				}
			}
			ResourceBarrier(CL, SCR, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
		}
		VERIFY_SUCCEEDED(CL->Close());
	}
};
#pragma endregion