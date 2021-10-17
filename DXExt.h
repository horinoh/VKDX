#pragma once

#include "DX.h"

class DXExt : public DX
{
private:
	using Super = DX;
public:
	using Vertex_Position = struct Vertex_Position { DirectX::XMFLOAT3 Position; };
	using Vertex_PositionColor = struct Vertex_PositionColor { DirectX::XMFLOAT3 Position; DirectX::XMFLOAT4 Color; };
	using Vertex_PositionNormal = struct Vertex_PositionNormal { DirectX::XMFLOAT3 Position; DirectX::XMFLOAT3 Normal; };
	using Instance_OffsetXY = struct Instance_OffsetXY { DirectX::XMFLOAT2 Offset; };

	void CreatePipelineState_VsPs(const D3D12_PRIMITIVE_TOPOLOGY_TYPE PTT, const D3D12_RASTERIZER_DESC& RD, const BOOL DepthEnable, const std::array<D3D12_SHADER_BYTECODE, 2>& SBCs) { CreatePipelineState_VsPs_Input(PTT, RD, DepthEnable, {}, SBCs); }
	void CreatePipelineState_VsPs_Input(const D3D12_PRIMITIVE_TOPOLOGY_TYPE PTT, const D3D12_RASTERIZER_DESC& RD, const BOOL DepthEnable, const std::vector<D3D12_INPUT_ELEMENT_DESC>& IEDs, const std::array<D3D12_SHADER_BYTECODE, 2>& SBCs);
	void CreatePipelineState_VsPsDsHsGs(const D3D12_PRIMITIVE_TOPOLOGY_TYPE PTT, const D3D12_RASTERIZER_DESC& RD, const BOOL DepthEnable, const std::array<D3D12_SHADER_BYTECODE, 5>& SBCs) { CreatePipelineState_VsPsDsHsGs_Input(PTT, RD, DepthEnable, {}, SBCs); }
	void CreatePipelineState_VsPsDsHsGs_Input(const D3D12_PRIMITIVE_TOPOLOGY_TYPE PTT, const D3D12_RASTERIZER_DESC& RD, const BOOL DepthEnable, const std::vector<D3D12_INPUT_ELEMENT_DESC>& IEDs, const std::array<D3D12_SHADER_BYTECODE, 5>& SBCs);
	//void CreatePipelineState_VsGsPs();
	//void CreatePipelineState_VsPsDsHs();

	void CreatePipelineState_MsPs(const BOOL DepthEnable, const std::array<D3D12_SHADER_BYTECODE, 2>& SBCs) { CreatePipelineState_AsMsPs(DepthEnable, { NullSBC, SBCs[0], SBCs[1] }); }
	void CreatePipelineState_AsMsPs(const BOOL DepthEnable, const std::array<D3D12_SHADER_BYTECODE, 3>& SBCs);
};

class DXExtDepth : public DXExt
{
private:
	using Super = DXExt;
protected:
	virtual void CreateTexture() override {
		DepthTextures.emplace_back().Create(COM_PTR_GET(Device), static_cast<UINT64>(GetClientRectWidth()), static_cast<UINT>(GetClientRectHeight()), 1, D3D12_CLEAR_VALUE({ .Format = DXGI_FORMAT_D24_UNORM_S8_UINT, .DepthStencil = D3D12_DEPTH_STENCIL_VALUE({.Depth = 1.0f, .Stencil = 0 }) }));
	}
	//!< パイプラインステートを深度を有効にして作成すること
	//virtual void CreatePipelineState() override {}
	virtual void CreateDescriptor() override {
		const D3D12_DESCRIPTOR_HEAP_DESC DHD = { .Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV, .NumDescriptors = 1, .Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE, .NodeMask = 0 };
		VERIFY_SUCCEEDED(Device->CreateDescriptorHeap(&DHD, COM_PTR_UUIDOF_PUTVOID(DsvDescriptorHeaps.emplace_back())));
	
		DsvCPUHandles.emplace_back();
		auto CDH = DsvDescriptorHeaps[0]->GetCPUDescriptorHandleForHeapStart();
		Device->CreateDepthStencilView(COM_PTR_GET(DepthTextures.back().Resource), &DepthTextures.back().DSV, CDH);
		DsvCPUHandles.back().emplace_back(CDH);
	}
	//!< 深度クリア、レンダーターゲットへの設定をすること
	//virtual void PopulateCommandList(const size_t i) override {
	//	const auto DCL = COM_PTR_GET(DirectCommandLists[i]);
	//	constexpr std::array<D3D12_RECT, 0> Rects = {};
	//	DCL->ClearDepthStencilView(DsvCPUHandles.back()[0], D3D12_CLEAR_FLAG_DEPTH/*| D3D12_CLEAR_FLAG_STENCIL*/, 1.0f, 0, static_cast<UINT>(size(Rects)), data(Rects));
	//	const std::array CHs = { SwapChainCPUHandles[i] };
	//	DCL->OMSetRenderTargets(static_cast<UINT>(size(CHs)), data(CHs), FALSE, &DsvCPUHandles.back()[0]);
	//}
};