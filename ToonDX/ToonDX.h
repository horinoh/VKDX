#pragma once

#include "resource.h"

#pragma region Code
#include "../DXExt.h"

class ToonDX : public DXExt
{
private:
	using Super = DXExt;
public:
	ToonDX() : Super() {}
	virtual ~ToonDX() {}

protected:
	virtual void OnTimer(HWND hWnd, HINSTANCE hInstance) override {
		Super::OnTimer(hWnd, hInstance);
		
		DirectX::XMStoreFloat4x4(&Tr.World, DirectX::XMMatrixRotationX(DirectX::XMConvertToRadians(Degree)));
		Degree += 1.0f;

#pragma region FRAME_OBJECT
		CopyToUploadResource(COM_PTR_GET(ConstantBuffers[GetCurrentBackBufferIndex()].Resource), RoundUp256(sizeof(Tr)), &Tr);
#pragma endregion
	}

	virtual void CreateGeometry() override { 
		constexpr D3D12_DRAW_INDEXED_ARGUMENTS DIA = { .IndexCountPerInstance = 1, .InstanceCount = 1, .StartIndexLocation = 0, .BaseVertexLocation = 0, .StartInstanceLocation = 0 };
		IndirectBuffers.emplace_back().Create(COM_PTR_GET(Device), DIA).ExecuteCopyCommand(COM_PTR_GET(Device), COM_PTR_GET(CommandAllocators[0]), COM_PTR_GET(GraphicsCommandLists[0]), COM_PTR_GET(GraphicsCommandQueue), COM_PTR_GET(Fence), sizeof(DIA), &DIA);
	}
	virtual void CreateConstantBuffer() override {
		constexpr auto Fov = 0.16f * std::numbers::pi_v<float>;
		const auto Aspect = GetAspectRatioOfClientRect();
		constexpr auto ZFar = 100.0f;
		constexpr auto ZNear = ZFar * 0.0001f;
		const auto CamPos = DirectX::XMVectorSet(0.0f, 0.0f, 3.0f, 1.0f);
		const auto CamTag = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
		const auto CamUp = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
		const auto Projection = DirectX::XMMatrixPerspectiveFovRH(Fov, Aspect, ZNear, ZFar);
		const auto View = DirectX::XMMatrixLookAtRH(CamPos, CamTag, CamUp);
		const auto World = DirectX::XMMatrixIdentity();
		DirectX::XMStoreFloat4x4(&Tr.Projection, Projection);
		DirectX::XMStoreFloat4x4(&Tr.View, View);
		DirectX::XMStoreFloat4x4(&Tr.World, World);
#pragma region FRAME_OBJECT
		DXGI_SWAP_CHAIN_DESC1 SCD;
		SwapChain->GetDesc1(&SCD);
		for (UINT i = 0; i < SCD.BufferCount; ++i) {
			ConstantBuffers.emplace_back().Create(COM_PTR_GET(Device), sizeof(Tr));
		}
#pragma endregion
	}
#ifdef USE_DEPTH
	//!< 深度テクスチャ
	virtual void CreateTexture() override {
		DepthTextures.emplace_back().Create(COM_PTR_GET(Device), static_cast<UINT64>(GetClientRectWidth()), static_cast<UINT>(GetClientRectHeight()), 1, D3D12_CLEAR_VALUE({ .Format = DXGI_FORMAT_D24_UNORM_S8_UINT, .DepthStencil = D3D12_DEPTH_STENCIL_VALUE({.Depth = 1.0f, .Stencil = 0 }) }));
	}
#endif
	virtual void CreateRootSignature() override {
		COM_PTR<ID3DBlob> Blob;
#ifdef USE_HLSL_ROOTSIGNATRUE
		GetRootSignaturePartFromShader(Blob, data(GetBasePath() + TEXT(".rs.cso")));
#else
		constexpr std::array DRs = {
			D3D12_DESCRIPTOR_RANGE({.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV, .NumDescriptors = 1, .BaseShaderRegister = 0, .RegisterSpace = 0, .OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND })
		};
		DX::SerializeRootSignature(Blob, {
			D3D12_ROOT_PARAMETER({
				.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE,
				.DescriptorTable = D3D12_ROOT_DESCRIPTOR_TABLE({.NumDescriptorRanges = static_cast<UINT>(size(DRs)), .pDescriptorRanges = data(DRs) }), 
				.ShaderVisibility = D3D12_SHADER_VISIBILITY_GEOMETRY 
			}),
		}, {}, SHADER_ROOT_ACCESS_GS);
#endif
		VERIFY_SUCCEEDED(Device->CreateRootSignature(0, Blob->GetBufferPointer(), Blob->GetBufferSize(), COM_PTR_UUIDOF_PUTVOID(RootSignatures.emplace_back())));
		LOG_OK();
	}
	virtual void CreatePipelineState() override {
		const auto ShaderPath = GetBasePath();
		std::vector<COM_PTR<ID3DBlob>> SBs;
#ifdef USE_SCREENSPACE_WIREFRAME
		VERIFY_SUCCEEDED(D3DReadFileToBlob(data(ShaderPath + TEXT(".vs.cso")), COM_PTR_PUT(SBs.emplace_back())));
		VERIFY_SUCCEEDED(D3DReadFileToBlob(data(ShaderPath + TEXT("_wf.ps.cso")), COM_PTR_PUT(SBs.emplace_back())));
		VERIFY_SUCCEEDED(D3DReadFileToBlob(data(ShaderPath + TEXT(".ds.cso")), COM_PTR_PUT(SBs.emplace_back())));
		VERIFY_SUCCEEDED(D3DReadFileToBlob(data(ShaderPath + TEXT(".hs.cso")), COM_PTR_PUT(SBs.emplace_back())));
		VERIFY_SUCCEEDED(D3DReadFileToBlob(data(ShaderPath + TEXT("_wf.gs.cso")), COM_PTR_PUT(SBs.emplace_back())));
#else
		VERIFY_SUCCEEDED(D3DReadFileToBlob(data(ShaderPath + TEXT(".vs.cso")), COM_PTR_PUT(SBs.emplace_back())));
		VERIFY_SUCCEEDED(D3DReadFileToBlob(data(ShaderPath + TEXT(".ps.cso")), COM_PTR_PUT(SBs.emplace_back())));
		VERIFY_SUCCEEDED(D3DReadFileToBlob(data(ShaderPath + TEXT(".ds.cso")), COM_PTR_PUT(SBs.emplace_back())));
		VERIFY_SUCCEEDED(D3DReadFileToBlob(data(ShaderPath + TEXT(".hs.cso")), COM_PTR_PUT(SBs.emplace_back())));
		VERIFY_SUCCEEDED(D3DReadFileToBlob(data(ShaderPath + TEXT(".gs.cso")), COM_PTR_PUT(SBs.emplace_back())));
#endif
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
#ifdef USE_DEPTH
		CreatePipelineState_VsPsDsHsGs(D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH, RD, TRUE, SBCs);
#else
		CreatePipelineState_VsPsDsHsGs(D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH, RD, FALSE, SBCs);
#endif
	}
	virtual void CreateDescriptor() override {
		{
			DXGI_SWAP_CHAIN_DESC1 SCD;
			SwapChain->GetDesc1(&SCD);
#pragma region FRAME_OBJECT
			const D3D12_DESCRIPTOR_HEAP_DESC DHD = {.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, .NumDescriptors = SCD.BufferCount, .Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE, .NodeMask = 0 }; //!< CBV * N
#pragma endregion
			VERIFY_SUCCEEDED(Device->CreateDescriptorHeap(&DHD, COM_PTR_UUIDOF_PUTVOID(CbvSrvUavDescriptorHeaps.emplace_back())));
		}
#ifdef USE_DEPTH
		{
			const D3D12_DESCRIPTOR_HEAP_DESC DHD = {.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV, .NumDescriptors = 1, .Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE, .NodeMask = 0 };
			VERIFY_SUCCEEDED(Device->CreateDescriptorHeap(&DHD, COM_PTR_UUIDOF_PUTVOID(DsvDescriptorHeaps.emplace_back())));
		}
#endif

		{
			DXGI_SWAP_CHAIN_DESC1 SCD;
			SwapChain->GetDesc1(&SCD);
			const auto& DH = CbvSrvUavDescriptorHeaps[0];
			auto CDH = DH->GetCPUDescriptorHandleForHeapStart();
#pragma region FRAME_OBJECT
			for (UINT i = 0; i < SCD.BufferCount; ++i) {
				const D3D12_CONSTANT_BUFFER_VIEW_DESC CBVD = { .BufferLocation = ConstantBuffers[i].Resource->GetGPUVirtualAddress(), .SizeInBytes = static_cast<UINT>(ConstantBuffers[i].Resource->GetDesc().Width) };
				Device->CreateConstantBufferView(&CBVD, CDH); CDH.ptr += Device->GetDescriptorHandleIncrementSize(DH->GetDesc().Type);
			}
#pragma endregion
		}
#ifdef USE_DEPTH
		{
			const auto& DH = DsvDescriptorHeaps[0];
			auto CDH = DH->GetCPUDescriptorHandleForHeapStart();
			Device->CreateDepthStencilView(COM_PTR_GET(DepthTextures.back().Resource), &DepthTextures.back().DSV, CDH); CDH.ptr += Device->GetDescriptorHandleIncrementSize(DH->GetDesc().Type);
		}
#endif
	}

	virtual void PopulateCommandList(const size_t i) override {
		const auto PS = COM_PTR_GET(PipelineStates[0]);

		const auto BGCL = COM_PTR_GET(BundleGraphicsCommandLists[i]);
		const auto BCA = COM_PTR_GET(BundleCommandAllocators[0]);
		VERIFY_SUCCEEDED(BGCL->Reset(BCA, PS));
		{
			BGCL->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_1_CONTROL_POINT_PATCHLIST);
			BGCL->ExecuteIndirect(COM_PTR_GET(IndirectBuffers[0].CommandSignature), 1, COM_PTR_GET(IndirectBuffers[0].Resource), 0, nullptr, 0);
		}
		VERIFY_SUCCEEDED(BGCL->Close());

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
#ifdef USE_DEPTH
				const auto DCDH = DsvDescriptorHeaps[0]->GetCPUDescriptorHandleForHeapStart();
				GCL->ClearDepthStencilView(DCDH, D3D12_CLEAR_FLAG_DEPTH/*| D3D12_CLEAR_FLAG_STENCIL*/, 1.0f, 0, static_cast<UINT>(size(Rects)), data(Rects));
#endif

				const std::array RTCDHs = { SCCDH };
#ifdef USE_DEPTH
				GCL->OMSetRenderTargets(static_cast<UINT>(size(RTCDHs)), data(RTCDHs), FALSE, &DCDH);
#else
				GCL->OMSetRenderTargets(static_cast<UINT>(size(RTCDHs)), data(RTCDHs), FALSE, nullptr);
#endif
				{
					const auto& DH = CbvSrvUavDescriptorHeaps[0];
					const std::array DHs = { COM_PTR_GET(DH) };
					GCL->SetDescriptorHeaps(static_cast<UINT>(size(DHs)), data(DHs));

					auto GDH = DH->GetGPUDescriptorHandleForHeapStart();
#pragma region FRAME_OBJECT
					GDH.ptr += Device->GetDescriptorHandleIncrementSize(DH->GetDesc().Type) * i;
					GCL->SetGraphicsRootDescriptorTable(0, GDH);
#pragma endregion
				}
				GCL->ExecuteBundle(BGCL);
			}
			ResourceBarrier(GCL, SCR, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
		}
		VERIFY_SUCCEEDED(GCL->Close());
	}

private:
	FLOAT Degree = 0.0f;

	struct Transform
	{
		DirectX::XMFLOAT4X4 Projection;
		DirectX::XMFLOAT4X4 View;
		DirectX::XMFLOAT4X4 World;
	};
	using Transform = struct Transform;
	Transform Tr;
};
#pragma endregion