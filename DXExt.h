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
	
	void CreateTexture_Depth(const UINT64 Width, const UINT Height) {
		DepthTextures.emplace_back().Create(COM_PTR_GET(Device), Width, Height, 1, D3D12_CLEAR_VALUE({ .Format = DXGI_FORMAT_D24_UNORM_S8_UINT, .DepthStencil = D3D12_DEPTH_STENCIL_VALUE({.Depth = 1.0f, .Stencil = 0 }) }));
	}
	void CreateTexture_Depth() {
		CreateTexture_Depth(static_cast<UINT64>(GetClientRectWidth()), static_cast<UINT>(GetClientRectHeight()));
	}
	void CreateTexture_Render(const UINT64 Width, const UINT Height) {
		RenderTextures.emplace_back().Create(COM_PTR_GET(Device), Width, Height, 1, D3D12_CLEAR_VALUE({ .Format = DXGI_FORMAT_R8G8B8A8_UNORM, .Color = { DirectX::Colors::SkyBlue.f[0], DirectX::Colors::SkyBlue.f[1], DirectX::Colors::SkyBlue.f[2], DirectX::Colors::SkyBlue.f[3] } }));
	}
	void CreateTexture_Render() {
		CreateTexture_Render(static_cast<UINT64>(GetClientRectWidth()), static_cast<UINT>(GetClientRectHeight()));
	}

	void CreatePipelineState_VsPs_Input(COM_PTR<ID3D12PipelineState>& PST, ID3D12RootSignature* RS, const D3D12_PRIMITIVE_TOPOLOGY_TYPE PTT, const D3D12_RASTERIZER_DESC& RD, const BOOL DepthEnable, const std::vector<D3D12_INPUT_ELEMENT_DESC>& IEDs, const std::array<D3D12_SHADER_BYTECODE, 2>& SBCs);
	void CreatePipelineState_VsPs(COM_PTR<ID3D12PipelineState>& PST, ID3D12RootSignature* RS, const D3D12_PRIMITIVE_TOPOLOGY_TYPE PTT, const D3D12_RASTERIZER_DESC& RD, const BOOL DepthEnable, const std::array<D3D12_SHADER_BYTECODE, 2>& SBCs) { CreatePipelineState_VsPs_Input(PST, RS, PTT, RD, DepthEnable, {}, SBCs); }
	void CreatePipelineState_VsPsDsHsGs_Input(COM_PTR<ID3D12PipelineState>& PST, ID3D12RootSignature* RS, const D3D12_PRIMITIVE_TOPOLOGY_TYPE PTT, const D3D12_RASTERIZER_DESC& RD, const BOOL DepthEnable, const std::vector<D3D12_INPUT_ELEMENT_DESC>& IEDs, const std::array<D3D12_SHADER_BYTECODE, 5>& SBCs);
	void CreatePipelineState_VsPsDsHsGs(COM_PTR<ID3D12PipelineState>& PST, ID3D12RootSignature* RS, const D3D12_PRIMITIVE_TOPOLOGY_TYPE PTT, const D3D12_RASTERIZER_DESC& RD, const BOOL DepthEnable, const std::array<D3D12_SHADER_BYTECODE, 5>& SBCs) { CreatePipelineState_VsPsDsHsGs_Input(PST, RS, PTT, RD, DepthEnable, {}, SBCs); }
	void CreatePipelineState_VsPsGs_Input(COM_PTR<ID3D12PipelineState>& PST, ID3D12RootSignature* RS, const D3D12_PRIMITIVE_TOPOLOGY_TYPE PTT, const D3D12_RASTERIZER_DESC& RD, const BOOL DepthEnable, const std::vector<D3D12_INPUT_ELEMENT_DESC>& IEDs, const std::array<D3D12_SHADER_BYTECODE, 3>& SBCs);
	void CreatePipelineState_VsPsGs(COM_PTR<ID3D12PipelineState>& PST, ID3D12RootSignature* RS, const D3D12_PRIMITIVE_TOPOLOGY_TYPE PTT, const D3D12_RASTERIZER_DESC& RD, const BOOL DepthEnable, const std::array<D3D12_SHADER_BYTECODE, 3>& SBCs) { CreatePipelineState_VsPsGs_Input(PST, RS, PTT, RD, DepthEnable, {}, SBCs); }
	//void CreatePipelineState_VsPsDsHs();

	void CreatePipelineState_AsMsPs(COM_PTR<ID3D12PipelineState>& PST, ID3D12RootSignature* RS, const BOOL DepthEnable, const std::array<D3D12_SHADER_BYTECODE, 3>& SBCs);
	void CreatePipelineState_MsPs(COM_PTR<ID3D12PipelineState>& PST, ID3D12RootSignature* RS, const BOOL DepthEnable, const std::array<D3D12_SHADER_BYTECODE, 2>& SBCs) { CreatePipelineState_AsMsPs(PST, RS, DepthEnable, { NullSBC, SBCs[0], SBCs[1] }); }

	void CreateStaticSampler_LinearWrap(const UINT ShaderRegister, const UINT RegisterSpace, const D3D12_SHADER_VISIBILITY ShaderVisibility)  {
		StaticSamplerDescs.emplace_back(D3D12_STATIC_SAMPLER_DESC({
			.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR,
			.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP, .AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP, .AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP,
			.MipLODBias = 0.0f,
			.MaxAnisotropy = 0,
			.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER,
			.BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE,
			.MinLOD = 0.0f, .MaxLOD = 1.0f,
			.ShaderRegister = ShaderRegister, .RegisterSpace = RegisterSpace, .ShaderVisibility = ShaderVisibility
		}));
	}
	void CreateStaticSampler_PointWrap(const UINT ShaderRegister, const UINT RegisterSpace, const D3D12_SHADER_VISIBILITY ShaderVisibility) {
		StaticSamplerDescs.emplace_back(D3D12_STATIC_SAMPLER_DESC({
			.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT,
			.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP, .AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP, .AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP,
			.MipLODBias = 0.0f,
			.MaxAnisotropy = 0,
			.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER,
			.BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE,
			.MinLOD = 0.0f, .MaxLOD = 1.0f,
			.ShaderRegister = ShaderRegister, .RegisterSpace = RegisterSpace, .ShaderVisibility = ShaderVisibility
		}));
	}
	void CreateStaticSampler_LinearClamp(const UINT ShaderRegister, const UINT RegisterSpace, const D3D12_SHADER_VISIBILITY ShaderVisibility) {
		StaticSamplerDescs.emplace_back(D3D12_STATIC_SAMPLER_DESC({
			.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR,
			.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP, .AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP, .AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
			.MipLODBias = 0.0f,
			.MaxAnisotropy = 0,
			.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER,
			.BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE,
			.MinLOD = 0.0f, .MaxLOD = 1.0f,
			.ShaderRegister = ShaderRegister, .RegisterSpace = RegisterSpace, .ShaderVisibility = ShaderVisibility
		}));
	}
	void CreateStaticSampler_PointClamp(const UINT ShaderRegister, const UINT RegisterSpace, const D3D12_SHADER_VISIBILITY ShaderVisibility) {
		StaticSamplerDescs.emplace_back(D3D12_STATIC_SAMPLER_DESC({
			.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT,
			.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP, .AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP, .AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
			.MipLODBias = 0.0f,
			.MaxAnisotropy = 0,
			.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER,
			.BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE,
			.MinLOD = 0.0f, .MaxLOD = 1.0f,
			.ShaderRegister = ShaderRegister, .RegisterSpace = RegisterSpace, .ShaderVisibility = ShaderVisibility
		}));
	}
	void PopulateCommandList_Clear(const size_t i, const DirectX::XMVECTORF32& Color) {
		const auto CL = COM_PTR_GET(DirectCommandLists[i]);
		const auto CA = COM_PTR_GET(DirectCommandAllocators[0]);
		VERIFY_SUCCEEDED(CL->Reset(CA, nullptr)); {
			CL->RSSetViewports(static_cast<UINT>(size(Viewports)), data(Viewports));
			CL->RSSetScissorRects(static_cast<UINT>(size(ScissorRects)), data(ScissorRects));
			const auto SCR = COM_PTR_GET(SwapchainBackBuffers[i].Resource);
			ResourceBarrier(CL, SCR, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
			{
				constexpr std::array<D3D12_RECT, 0> Rects = {};
				CL->ClearRenderTargetView(SwapchainBackBuffers[i].Handle, Color, static_cast<UINT>(size(Rects)), data(Rects));
			}
			ResourceBarrier(CL, SCR, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
		} VERIFY_SUCCEEDED(CL->Close());
	}
};

class DXExtDepth : public DXExt
{
private:
	using Super = DXExt;
protected:
	virtual void CreateTexture() override {
		CreateTexture_Depth();
	}
	//!< パイプラインステートを深度を有効にして作成すること
	//virtual void CreatePipelineState() override {
	//	CreatePipelineState_XXX(TRUE, ...);
	//}
	virtual void CreateDescriptor() override {
		auto& Desc = DsvDescs.emplace_back();
		auto& Heap = Desc.first;
		auto& Handle = Desc.second;

		const D3D12_DESCRIPTOR_HEAP_DESC DHD = { .Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV, .NumDescriptors = 1, .Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE, .NodeMask = 0 };
		VERIFY_SUCCEEDED(Device->CreateDescriptorHeap(&DHD, COM_PTR_UUIDOF_PUTVOID(Heap)));
		
		auto CDH = Heap->GetCPUDescriptorHandleForHeapStart();
		const auto IncSize = Device->GetDescriptorHandleIncrementSize(Heap->GetDesc().Type);

		const auto& Tex = DepthTextures[0];
		Device->CreateDepthStencilView(COM_PTR_GET(Tex.Resource), &Tex.DSV, CDH);
		Handle.emplace_back(CDH);
		CDH.ptr += IncSize;
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