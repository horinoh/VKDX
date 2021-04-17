#pragma once

#include "resource.h"

#pragma region Code
#include "../DXExt.h"

class MeshShaderDX : public DXExt
{
private:
	using Super = DXExt;
public:
	MeshShaderDX() : Super() {}
	virtual ~MeshShaderDX() {}

	virtual void CreatePipelineState() override {
		if (HasMeshShaderSupport(COM_PTR_GET(Device))) {
			const auto ShaderPath = GetBasePath();
				std::vector<COM_PTR<ID3DBlob>> SBs;
				//VERIFY_SUCCEEDED(D3DReadFileToBlob(data(ShaderPath + TEXT(".as.cso")), COM_PTR_PUT(SBs.emplace_back())));
				VERIFY_SUCCEEDED(D3DReadFileToBlob(data(ShaderPath + TEXT(".ms.cso")), COM_PTR_PUT(SBs.emplace_back())));
				VERIFY_SUCCEEDED(D3DReadFileToBlob(data(ShaderPath + TEXT(".ps.cso")), COM_PTR_PUT(SBs.emplace_back())));

				constexpr D3D12_RASTERIZER_DESC RD = {
					.FillMode = D3D12_FILL_MODE_SOLID,
					.CullMode = D3D12_CULL_MODE_BACK, .FrontCounterClockwise = TRUE,
					.DepthBias = D3D12_DEFAULT_DEPTH_BIAS, .DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP, .SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS,
					.DepthClipEnable = TRUE,
					.MultisampleEnable = FALSE, .AntialiasedLineEnable = FALSE, .ForcedSampleCount = 0,
					.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF
			};
			constexpr D3D12_DEPTH_STENCILOP_DESC DSOD = {
				.StencilFailOp = D3D12_STENCIL_OP_KEEP,
				.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP,
				.StencilPassOp = D3D12_STENCIL_OP_KEEP,
				.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS
			};
			constexpr D3D12_DEPTH_STENCIL_DESC DSD = {
				.DepthEnable = TRUE, .DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL, .DepthFunc = D3D12_COMPARISON_FUNC_LESS,
				.StencilEnable = FALSE, .StencilReadMask = D3D12_DEFAULT_STENCIL_READ_MASK, .StencilWriteMask = D3D12_DEFAULT_STENCIL_WRITE_MASK,
				.FrontFace = DSOD, .BackFace = DSOD
			};
			const std::array RTVs = { DXGI_FORMAT_R8G8B8A8_UNORM };

			MESH_SHADER_PIPELINE_STATE_DESC MSPSD = {
				.pRootSignature = COM_PTR_GET(RootSignatures[0]),
				.AS = D3D12_SHADER_BYTECODE({.pShaderBytecode = nullptr, .BytecodeLength = 0 }),
				.MS = D3D12_SHADER_BYTECODE({.pShaderBytecode = SBs[0]->GetBufferPointer(), .BytecodeLength = SBs[0]->GetBufferSize() }),
				.PS = D3D12_SHADER_BYTECODE({.pShaderBytecode = SBs[1]->GetBufferPointer(), .BytecodeLength = SBs[1]->GetBufferSize() }),
				.BlendState = D3D12_BLEND_DESC({.AlphaToCoverageEnable = TRUE, .IndependentBlendEnable = FALSE, .RenderTarget = {}}),
				.SampleMask = D3D12_DEFAULT_SAMPLE_MASK,
				.RasterizerState = RD,
				.DepthStencilState = DSD,
				.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE,
				.NumRenderTargets = static_cast<UINT>(size(RTVs)), .RTVFormats = {},
				.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT,
				.SampleDesc = DXGI_SAMPLE_DESC({.Count = 1, .Quality = 0 }),
				.NodeMask = 0,
				.CachedPSO = D3D12_CACHED_PIPELINE_STATE({.pCachedBlob = nullptr, .CachedBlobSizeInBytes = 0 }),
		#if defined(_DEBUG) && defined(USE_WARP)
				.Flags = D3D12_PIPELINE_STATE_FLAG_TOOL_DEBUG
		#else
				.Flags = D3D12_PIPELINE_STATE_FLAG_NONE
		#endif
			};
			std::ranges::copy(RTVs, MSPSD.RTVFormats);

			const D3D12_PIPELINE_STATE_STREAM_DESC PSSD = { .SizeInBytes = sizeof(MSPSD), .pPipelineStateSubobjectStream = &MSPSD };
			COM_PTR<ID3D12Device2> Device2;
			VERIFY_SUCCEEDED(Device->QueryInterface(COM_PTR_UUIDOF_PUTVOID(Device2)));
			VERIFY_SUCCEEDED(Device2->CreatePipelineState(&PSSD, COM_PTR_UUIDOF_PUTVOID(PipelineStates.emplace_back())));
		}
	}
	virtual void PopulateCommandList(const size_t i) override {
		const auto PS = HasMeshShaderSupport(COM_PTR_GET(Device)) ? COM_PTR_GET(PipelineStates[0]) : nullptr;
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
				if (HasMeshShaderSupport(COM_PTR_GET(Device))) {
					auto SCCDH = SwapChainDescriptorHeap->GetCPUDescriptorHandleForHeapStart(); SCCDH.ptr += i * Device->GetDescriptorHandleIncrementSize(SwapChainDescriptorHeap->GetDesc().Type);

					constexpr std::array<D3D12_RECT, 0> Rects = {};
					GCL->ClearRenderTargetView(SCCDH, DirectX::Colors::SkyBlue, static_cast<UINT>(size(Rects)), data(Rects));

					const std::array RTCDHs = { SCCDH };
					GCL->OMSetRenderTargets(static_cast<UINT>(size(RTCDHs)), data(RTCDHs), FALSE, nullptr);

					COM_PTR<ID3D12GraphicsCommandList6> GCL6;
					VERIFY_SUCCEEDED(GCL->QueryInterface(COM_PTR_UUIDOF_PUTVOID(GCL6)));
					GCL6->DispatchMesh(1, 1, 1);
				}
			}
			ResourceBarrier(GCL, SCR, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
		}
		VERIFY_SUCCEEDED(GCL->Close());
	}
};
#pragma endregion