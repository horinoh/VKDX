#pragma once

#include "resource.h"

#pragma region Code
#include "../DXExt.h"

//!< パラメトリックサーフェス(ParametricSurface) http://www.3d-meier.de/tut3/Seite0.html

class ParametricSurfaceDX : public DXExt
{
private:
	using Super = DXExt;
public:
	ParametricSurfaceDX() : Super() {}
	virtual ~ParametricSurfaceDX() {}

protected:
#ifndef USE_BUNDLE
	//!< デフォルト実装をバンドルを作成する実装にしているので、オーバーライドしてバンドルを作成しないようにしている
	virtual void CreateCommandList() override {
		CommandAllocators.emplace_back(COM_PTR<ID3D12CommandAllocator>());
		VERIFY_SUCCEEDED(Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, COM_PTR_UUIDOF_PUTVOID(CommandAllocators.back())));

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
	virtual void CreateGeometry() override { 
		//!< 最低でもインデックス数1が必要 (At least index count must be 1)
		constexpr D3D12_DRAW_INDEXED_ARGUMENTS DIA = { .IndexCountPerInstance = 1, .InstanceCount = 1, .StartIndexLocation = 0, .BaseVertexLocation = 0, .StartInstanceLocation = 0 };
		IndirectBuffers.emplace_back().Create(COM_PTR_GET(Device), DIA).ExecuteCopyCommand(COM_PTR_GET(Device), COM_PTR_GET(CommandAllocators[0]), COM_PTR_GET(GraphicsCommandLists[0]), COM_PTR_GET(GraphicsCommandQueue), COM_PTR_GET(Fence), sizeof(DIA), &DIA);
	}
	virtual void CreatePipelineState() override { 
		const auto ShaderPath = GetBasePath();
		std::vector<COM_PTR<ID3DBlob>> SBs = {};
		VERIFY_SUCCEEDED(D3DReadFileToBlob(data(ShaderPath + TEXT(".vs.cso")), COM_PTR_PUT(SBs.emplace_back())));
		VERIFY_SUCCEEDED(D3DReadFileToBlob(data(ShaderPath + TEXT(".ps.cso")), COM_PTR_PUT(SBs.emplace_back())));
		VERIFY_SUCCEEDED(D3DReadFileToBlob(data(ShaderPath + TEXT(".ds.cso")), COM_PTR_PUT(SBs.emplace_back())));
		VERIFY_SUCCEEDED(D3DReadFileToBlob(data(ShaderPath + TEXT(".hs.cso")), COM_PTR_PUT(SBs.emplace_back())));
		VERIFY_SUCCEEDED(D3DReadFileToBlob(data(ShaderPath + TEXT(".gs.cso")), COM_PTR_PUT(SBs.emplace_back())));
		const std::array SBCs = {
			D3D12_SHADER_BYTECODE({.pShaderBytecode = SBs[0]->GetBufferPointer(), .BytecodeLength = SBs[0]->GetBufferSize() }),
			D3D12_SHADER_BYTECODE({.pShaderBytecode = SBs[1]->GetBufferPointer(), .BytecodeLength = SBs[1]->GetBufferSize() }),
			D3D12_SHADER_BYTECODE({.pShaderBytecode = SBs[2]->GetBufferPointer(), .BytecodeLength = SBs[2]->GetBufferSize() }),
			D3D12_SHADER_BYTECODE({.pShaderBytecode = SBs[3]->GetBufferPointer(), .BytecodeLength = SBs[3]->GetBufferSize() }),
			D3D12_SHADER_BYTECODE({.pShaderBytecode = SBs[4]->GetBufferPointer(), .BytecodeLength = SBs[4]->GetBufferSize() }),
		};

		constexpr D3D12_RASTERIZER_DESC RD = {
			.FillMode = D3D12_FILL_MODE_SOLID,
			.CullMode = D3D12_CULL_MODE_BACK, .FrontCounterClockwise = TRUE,
			.DepthBias = D3D12_DEFAULT_DEPTH_BIAS, .DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP, .SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS,
			.DepthClipEnable = TRUE,
			.MultisampleEnable = FALSE, .AntialiasedLineEnable = FALSE, .ForcedSampleCount = 0,
			.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF
		};
		CreatePipelineState_VsPsDsHsGs(D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH, RD, FALSE, SBCs); 
	}

	virtual void PopulateCommandList(const size_t i) override {
		const auto PS = COM_PTR_GET(PipelineStates[0]);

#ifdef USE_BUNDLE
		const auto BGCL = COM_PTR_GET(BundleGraphicsCommandLists[i]);
		const auto BCA = COM_PTR_GET(BundleCommandAllocators[0]);
		VERIFY_SUCCEEDED(BGCL->Reset(BCA, PS));
		{
			BGCL->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_1_CONTROL_POINT_PATCHLIST);
			BGCL->ExecuteIndirect(COM_PTR_GET(IndirectBuffers[0].CommandSignature), 1, COM_PTR_GET(IndirectBuffers[0].Resource), 0, nullptr, 0);
		}
		VERIFY_SUCCEEDED(BGCL->Close());
#endif

		const auto GCL = COM_PTR_GET(GraphicsCommandLists[i]);
		const auto CA = COM_PTR_GET(CommandAllocators[0]);
		VERIFY_SUCCEEDED(GCL->Reset(CA, PS));
		{
			GCL->SetGraphicsRootSignature(COM_PTR_GET(RootSignatures[0]));

			GCL->RSSetViewports(static_cast<UINT>(size(Viewports)), data(Viewports));
			GCL->RSSetScissorRects(static_cast<UINT>(size(ScissorRects)), data(ScissorRects));

			const auto SCR = COM_PTR_GET(SwapChainResources[i]);
			ResourceBarrier(GCL, SCR, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
			{
				auto SCCDH = SwapChainDescriptorHeap->GetCPUDescriptorHandleForHeapStart(); SCCDH.ptr += i * Device->GetDescriptorHandleIncrementSize(SwapChainDescriptorHeap->GetDesc().Type);

				constexpr std::array<D3D12_RECT, 0> Rects = {};
				GCL->ClearRenderTargetView(SCCDH, DirectX::Colors::SkyBlue, static_cast<UINT>(size(Rects)), data(Rects));

				const std::array RTCDHs = { SCCDH };
				GCL->OMSetRenderTargets(static_cast<UINT>(size(RTCDHs)), data(RTCDHs), FALSE, nullptr);

#ifdef USE_BUNDLE
				GCL->ExecuteBundle(BGCL);
#else
				//!< トポロジ (VK では Pipline 作成時に InputAssembly で指定している)
				GCL->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_1_CONTROL_POINT_PATCHLIST);
				GCL->ExecuteIndirect(COM_PTR_GET(IndirectBuffers[0].CommandSignature), 1, COM_PTR_GET(IndirectBuffers[0].Resource), 0, nullptr, 0);
#endif
			}
			ResourceBarrier(GCL, SCR, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
		}
		VERIFY_SUCCEEDED(GCL->Close());
	}
};
#pragma endregion