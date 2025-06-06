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
		DirectCommandAllocators.emplace_back(COM_PTR<ID3D12CommandAllocator>());
		VERIFY_SUCCEEDED(Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, COM_PTR_UUIDOF_PUTVOID(DirectCommandAllocators.back())));

		DXGI_SWAP_CHAIN_DESC1 SCD;
		SwapChain.DxSwapChain->GetDesc1(&SCD);
		for (UINT i = 0; i < SCD.BufferCount; ++i) {
			DirectCommandLists.emplace_back(COM_PTR<ID3D12GraphicsCommandList>());
			VERIFY_SUCCEEDED(Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, COM_PTR_GET(DirectCommandAllocators[0]), nullptr, COM_PTR_UUIDOF_PUTVOID(DirectCommandLists.back())));
			VERIFY_SUCCEEDED(DirectCommandLists.back()->Close());
		}
		LOG_OK();
	}
#endif
	virtual void CreateGeometry() override { 
		//!< 最低でもインデックス数1が必要 (At least index count must be 1)
		constexpr D3D12_DRAW_INDEXED_ARGUMENTS DIA = { .IndexCountPerInstance = 1, .InstanceCount = 1, .StartIndexLocation = 0, .BaseVertexLocation = 0, .StartInstanceLocation = 0 };
		IndirectBuffers.emplace_back().Create(COM_PTR_GET(Device), DIA).ExecuteCopyCommand(COM_PTR_GET(Device), COM_PTR_GET(DirectCommandAllocators[0]), COM_PTR_GET(DirectCommandLists[0]), COM_PTR_GET(GraphicsCommandQueue), COM_PTR_GET(GraphicsFence), sizeof(DIA), &DIA);
	}
	virtual void CreatePipelineState() override { 
		PipelineStates.emplace_back();

		std::vector<COM_PTR<ID3DBlob>> SBs = {};
		VERIFY_SUCCEEDED(D3DReadFileToBlob(data(GetFilePath(".vs.cso").wstring()), COM_PTR_PUT(SBs.emplace_back())));
		VERIFY_SUCCEEDED(D3DReadFileToBlob(data(GetFilePath(".ps.cso").wstring()), COM_PTR_PUT(SBs.emplace_back())));
		VERIFY_SUCCEEDED(D3DReadFileToBlob(data(GetFilePath(".ds.cso").wstring()), COM_PTR_PUT(SBs.emplace_back())));
		VERIFY_SUCCEEDED(D3DReadFileToBlob(data(GetFilePath(".hs.cso").wstring()), COM_PTR_PUT(SBs.emplace_back())));
		VERIFY_SUCCEEDED(D3DReadFileToBlob(data(GetFilePath(".gs.cso").wstring()), COM_PTR_PUT(SBs.emplace_back())));
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
		CreatePipelineState_VsPsDsHsGs(PipelineStates[0], COM_PTR_GET(RootSignatures[0]), D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH, RD, FALSE, SBCs);
		
		for (auto& i : Threads) { i.join(); }
		Threads.clear();
	}

	virtual void PopulateCommandList(const size_t i) override {
		const auto PS = COM_PTR_GET(PipelineStates[0]);

#ifdef USE_BUNDLE
		const auto BCL = COM_PTR_GET(BundleCommandLists[i]);
		const auto BCA = COM_PTR_GET(BundleCommandAllocators[0]);
		VERIFY_SUCCEEDED(BCL->Reset(BCA, PS));
		{
			BCL->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_1_CONTROL_POINT_PATCHLIST);
			BCL->ExecuteIndirect(COM_PTR_GET(IndirectBuffers[0].CommandSignature), 1, COM_PTR_GET(IndirectBuffers[0].Resource), 0, nullptr, 0);
		}
		VERIFY_SUCCEEDED(BCL->Close());
#endif

		const auto CL = COM_PTR_GET(DirectCommandLists[i]);
		const auto CA = COM_PTR_GET(DirectCommandAllocators[0]);
		VERIFY_SUCCEEDED(CL->Reset(CA, PS));
		{
			CL->SetGraphicsRootSignature(COM_PTR_GET(RootSignatures[0]));

			CL->RSSetViewports(static_cast<UINT>(std::size(Viewports)), std::data(Viewports));
			CL->RSSetScissorRects(static_cast<UINT>(std::size(ScissorRects)), std::data(ScissorRects));

			const auto& RAH = SwapChain.ResourceAndHandles[i];

			const auto SCR = COM_PTR_GET(RAH.first);
			ResourceBarrier(CL, SCR, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
			{
				constexpr std::array<D3D12_RECT, 0> Rects = {};
				CL->ClearRenderTargetView(RAH.second, DirectX::Colors::SkyBlue, static_cast<UINT>(std::size(Rects)), std::data(Rects));

				const std::array CHs = { RAH.second };
				CL->OMSetRenderTargets(static_cast<UINT>(std::size(CHs)), std::data(CHs), FALSE, nullptr);

#ifdef USE_BUNDLE
				CL->ExecuteBundle(BCL);
#else
				//!< トポロジ (VK では Pipeline 作成時に InputAssembly で指定している)
				CL->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_1_CONTROL_POINT_PATCHLIST);
				CL->ExecuteIndirect(COM_PTR_GET(IndirectBuffers[0].CommandSignature), 1, COM_PTR_GET(IndirectBuffers[0].Resource), 0, nullptr, 0);
#endif
			}
			ResourceBarrier(CL, SCR, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
		}
		VERIFY_SUCCEEDED(CL->Close());
	}
};
#pragma endregion