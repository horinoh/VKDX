#pragma once

#include "resource.h"

#pragma region Code
#include "../DXExt.h"
#include "../Holo.h"

class HoloDX : public DXExt, public Holo
{
private:
	using Super = DXExt;
public:
	HoloDX() : Super(), Holo() {
		const auto& QS = GetQuiltSetting();
		QuiltWidth = static_cast<UINT64>(QS.GetWidth());
		QuiltHeight = static_cast<UINT>(QS.GetHeight());
	}
	virtual ~HoloDX() {}

protected:
	virtual void OnCreate(HWND hWnd, HINSTANCE hInstance, LPCWSTR Title) {
		SetWindow(hWnd);
		Super::OnCreate(hWnd, hInstance, Title);
	}
	virtual void OnTimer(HWND hWnd, HINSTANCE hInstance) override { 
		Super::OnTimer(hWnd, hInstance); 
#pragma region FRAME_OBJECT
		//CopyToUploadResource(COM_PTR_GET(ConstantBuffers[GetCurrentBackBufferIndex()].Resource), RoundUp256(sizeof(Tr)), &Tr);
#pragma endregion
	}

	virtual void CreateCommandList() override {
		Super::CreateCommandList();
#pragma region PASS1
		DXGI_SWAP_CHAIN_DESC1 SCD;
		SwapChain->GetDesc1(&SCD);
		for (UINT i = 0; i < SCD.BufferCount; ++i) {
			VERIFY_SUCCEEDED(Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_BUNDLE, COM_PTR_GET(BundleCommandAllocators[0]), nullptr, COM_PTR_UUIDOF_PUTVOID(BundleGraphicsCommandLists.emplace_back())));
			VERIFY_SUCCEEDED(BundleGraphicsCommandLists.back()->Close());
		}
#pragma endregion
	}
	virtual void CreateGeometry() override {
		const auto CA = COM_PTR_GET(CommandAllocators[0]);
		const auto GCL = COM_PTR_GET(GraphicsCommandLists[0]);
		const auto GCQ = COM_PTR_GET(GraphicsCommandQueue);
#pragma region PASS0
		//!< ���b�V���`��p
		{
			constexpr D3D12_DRAW_INDEXED_ARGUMENTS DIA = { .IndexCountPerInstance = 1, .InstanceCount = 1, .StartIndexLocation = 0, .BaseVertexLocation = 0, .StartInstanceLocation = 0 };
			IndirectBuffers.emplace_back().Create(COM_PTR_GET(Device), DIA).ExecuteCopyCommand(COM_PTR_GET(Device), CA, GCL, GCQ, COM_PTR_GET(Fence), sizeof(DIA), &DIA);
		}
#pragma endregion

#pragma region PASS1
		//!< �t���X�N���[���`��p
		{
			constexpr D3D12_DRAW_ARGUMENTS DA = { .VertexCountPerInstance = 4, .InstanceCount = 1, .StartVertexLocation = 0, .StartInstanceLocation = 0 };
			IndirectBuffers.emplace_back().Create(COM_PTR_GET(Device), DA).ExecuteCopyCommand(COM_PTR_GET(Device), CA, GCL, GCQ, COM_PTR_GET(Fence), sizeof(DA), &DA);
		}
#pragma endregion
	}
	virtual void CreateConstantBuffer() override {
		constexpr auto Fov = DirectX::XMConvertToRadians(14.0f);
		//const auto Aspect = Holo::GetAspectRatio(GetDeviceIndex());
		const auto Aspect = HoloDraw.DisplayAspect;
		constexpr auto ZFar = 100.0f;
		constexpr auto ZNear = 0.1f;

		const auto CamPos = DirectX::XMVectorSet(0.0f, 0.0f, 7.0f, 1.0f);
		const auto CamTag = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
		const auto CamUp = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

		const auto Projection = DirectX::XMMatrixPerspectiveFovRH(Fov, Aspect, ZNear, ZFar);
		const auto View = DirectX::XMMatrixLookAtRH(CamPos, CamTag, CamUp);
		const auto World = DirectX::XMMatrixIdentity();

		DirectX::XMStoreFloat4x4(&Transform.World, World);
		DirectX::XMStoreFloat4x4(&Transform.View, View);
		DirectX::XMStoreFloat4x4(&Transform.Projection, Projection);

#pragma region FRAME_OBJECT
		DXGI_SWAP_CHAIN_DESC1 SCD;
		SwapChain->GetDesc1(&SCD);
		for (UINT i = 0; i < SCD.BufferCount; ++i) {
			ConstantBuffers.emplace_back().Create(COM_PTR_GET(Device), sizeof(Transform));
		}
#pragma endregion
	}
	virtual void CreateTexture()
	{
		RenderTextures.emplace_back().Create(COM_PTR_GET(Device), QuiltWidth, QuiltHeight, 1, D3D12_CLEAR_VALUE({.Format = DXGI_FORMAT_R8G8B8A8_UNORM, .Color = { DirectX::Colors::SkyBlue.f[0], DirectX::Colors::SkyBlue.f[1], DirectX::Colors::SkyBlue.f[2], DirectX::Colors::SkyBlue.f[3] } }));
		DepthTextures.emplace_back().Create(COM_PTR_GET(Device), QuiltWidth, QuiltHeight, 1, D3D12_CLEAR_VALUE({ .Format = DXGI_FORMAT_D24_UNORM_S8_UINT, .DepthStencil = D3D12_DEPTH_STENCIL_VALUE({.Depth = 1.0f, .Stencil = 0 }) }));
	}
	virtual void CreateStaticSampler() override {
#pragma region PASS1
		StaticSamplerDescs.emplace_back(D3D12_STATIC_SAMPLER_DESC({
			.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR,
			.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP, .AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP, .AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP,
			.MipLODBias = 0.0f,
			.MaxAnisotropy = 0,
			.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER,
			.BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE,
			.MinLOD = 0.0f, .MaxLOD = 1.0f,
			.ShaderRegister = 0, .RegisterSpace = 0, .ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL
		}));
#pragma endregion
	}
	virtual void CreateRootSignature() override {
#pragma region PASS0
		{
			COM_PTR<ID3DBlob> Blob;
#ifdef USE_HLSL_ROOTSIGNATRUE
			GetRootSignaturePartFromShader(Blob, data(GetBasePath() + TEXT(".rs.cso")));
#else
			constexpr std::array DRs = {
				D3D12_DESCRIPTOR_RANGE({.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV, .NumDescriptors = 1, .BaseShaderRegister = 0, .RegisterSpace = 0, .OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND })
			};
			SerializeRootSignature(Blob, {
				D3D12_ROOT_PARAMETER({
					.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE,
					.DescriptorTable = D3D12_ROOT_DESCRIPTOR_TABLE({.NumDescriptorRanges = static_cast<UINT>(size(DRs)), .pDescriptorRanges = data(DRs) }),
					.ShaderVisibility = D3D12_SHADER_VISIBILITY_GEOMETRY 
				}),
#pragma region ROOT_CONSTANT
				D3D12_ROOT_PARAMETER({
					.ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS,
					.Constants = {.ShaderRegister = 1, .RegisterSpace = 0, .Num32BitValues = static_cast<UINT>(sizeof(QuiltDraw)) }, 
					.ShaderVisibility = D3D12_SHADER_VISIBILITY_GEOMETRY 
				}),
#pragma endregion
			}, {}, SHADER_ROOT_ACCESS_GS);
#endif
			VERIFY_SUCCEEDED(Device->CreateRootSignature(0, Blob->GetBufferPointer(), Blob->GetBufferSize(), COM_PTR_UUIDOF_PUTVOID(RootSignatures.emplace_back())));
		}
#pragma endregion

#pragma region PASS1
		{
			COM_PTR<ID3DBlob> Blob;
#ifdef USE_HLSL_ROOTSIGNATRUE 
			GetRootSignaturePartFromShader(Blob, data(GetBasePath() + TEXT("_1.rs.cso")));
#else
			constexpr std::array DRs_Srv = {
				D3D12_DESCRIPTOR_RANGE({.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV, .NumDescriptors = 1, .BaseShaderRegister = 0, .RegisterSpace = 0, .OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND })
			};
			assert(!empty(StaticSamplerDescs) && "");
			DX::SerializeRootSignature(Blob, {
				D3D12_ROOT_PARAMETER({
					.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE,
					.DescriptorTable = D3D12_ROOT_DESCRIPTOR_TABLE({.NumDescriptorRanges = static_cast<uint32_t>(size(DRs_Srv)), .pDescriptorRanges = data(DRs_Srv) }), 
					.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL 
				}),
#pragma region ROOT_CONSTANT
				D3D12_ROOT_PARAMETER({
					.ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS,
					.Constants = {.ShaderRegister = 0, .RegisterSpace = 0, .Num32BitValues = static_cast<UINT>(sizeof(HoloDraw)) },
					.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL
				}),
#pragma endregion
			}, {
				StaticSamplerDescs[0],
			}, SHADER_ROOT_ACCESS_PS);
#endif
			VERIFY_SUCCEEDED(Device->CreateRootSignature(0, Blob->GetBufferPointer(), Blob->GetBufferSize(), COM_PTR_UUIDOF_PUTVOID(RootSignatures.emplace_back())));
		}
#pragma endregion
		LOG_OK();
	}
	virtual void CreatePipelineState() override {
		std::vector<std::thread> Threads;
		PipelineStates.resize(2);
		const auto ShaderPath = GetBasePath();
#ifdef USE_PIPELINE_SERIALIZE
		PipelineLibrarySerializer PLS(COM_PTR_GET(Device), GetBasePath() + TEXT(".plo"));
#endif
		const std::vector RTBDs = {
			D3D12_RENDER_TARGET_BLEND_DESC({
				.BlendEnable = FALSE, .LogicOpEnable = FALSE,
				.SrcBlend = D3D12_BLEND_ONE, .DestBlend = D3D12_BLEND_ZERO, .BlendOp = D3D12_BLEND_OP_ADD,
				.SrcBlendAlpha = D3D12_BLEND_ONE, .DestBlendAlpha = D3D12_BLEND_ZERO, .BlendOpAlpha = D3D12_BLEND_OP_ADD,
				.LogicOp = D3D12_LOGIC_OP_NOOP,
				.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL,
			}),
		};
		constexpr D3D12_RASTERIZER_DESC RD = {
			.FillMode = D3D12_FILL_MODE_SOLID,
			.CullMode = D3D12_CULL_MODE_BACK, .FrontCounterClockwise = TRUE,
			.DepthBias = D3D12_DEFAULT_DEPTH_BIAS, .DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP, .SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS,
			.DepthClipEnable = TRUE,
			.MultisampleEnable = FALSE, .AntialiasedLineEnable = FALSE, .ForcedSampleCount = 0,
			.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF
		};
		constexpr D3D12_DEPTH_STENCILOP_DESC DSOD = { .StencilFailOp = D3D12_STENCIL_OP_KEEP, .StencilDepthFailOp = D3D12_STENCIL_OP_KEEP, .StencilPassOp = D3D12_STENCIL_OP_KEEP, .StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS };
		const std::vector<D3D12_INPUT_ELEMENT_DESC> IEDs = {};
		const std::vector RTVs = { DXGI_FORMAT_R8G8B8A8_UNORM };

#pragma region PASS0
		constexpr D3D12_DEPTH_STENCIL_DESC DSD0 = {
			.DepthEnable = TRUE, .DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL, .DepthFunc = D3D12_COMPARISON_FUNC_LESS,
			.StencilEnable = FALSE, .StencilReadMask = D3D12_DEFAULT_STENCIL_READ_MASK, .StencilWriteMask = D3D12_DEFAULT_STENCIL_WRITE_MASK,
			.FrontFace = DSOD, .BackFace = DSOD
		};
		std::vector<COM_PTR<ID3DBlob>> SBs0;
		VERIFY_SUCCEEDED(D3DReadFileToBlob(data(ShaderPath + TEXT(".vs.cso")), COM_PTR_PUT(SBs0.emplace_back())));
		VERIFY_SUCCEEDED(D3DReadFileToBlob(data(ShaderPath + TEXT(".ps.cso")), COM_PTR_PUT(SBs0.emplace_back())));
		VERIFY_SUCCEEDED(D3DReadFileToBlob(data(ShaderPath + TEXT(".ds.cso")), COM_PTR_PUT(SBs0.emplace_back())));
		VERIFY_SUCCEEDED(D3DReadFileToBlob(data(ShaderPath + TEXT(".hs.cso")), COM_PTR_PUT(SBs0.emplace_back())));
		VERIFY_SUCCEEDED(D3DReadFileToBlob(data(ShaderPath + TEXT(".gs.cso")), COM_PTR_PUT(SBs0.emplace_back())));
		const std::array SBCs0 = {
			D3D12_SHADER_BYTECODE({.pShaderBytecode = SBs0[0]->GetBufferPointer(), .BytecodeLength = SBs0[0]->GetBufferSize() }),
			D3D12_SHADER_BYTECODE({.pShaderBytecode = SBs0[1]->GetBufferPointer(), .BytecodeLength = SBs0[1]->GetBufferSize() }),
			D3D12_SHADER_BYTECODE({.pShaderBytecode = SBs0[2]->GetBufferPointer(), .BytecodeLength = SBs0[2]->GetBufferSize() }),
			D3D12_SHADER_BYTECODE({.pShaderBytecode = SBs0[3]->GetBufferPointer(), .BytecodeLength = SBs0[3]->GetBufferSize() }),
			D3D12_SHADER_BYTECODE({.pShaderBytecode = SBs0[4]->GetBufferPointer(), .BytecodeLength = SBs0[4]->GetBufferSize() }),
		};
#ifdef USE_PIPELINE_SERIALIZE
		Threads.emplace_back(std::thread::thread(DX::CreatePipelineState_, std::ref(PipelineStates[0]), COM_PTR_GET(Device), COM_PTR_GET(RootSignatures[0]), D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH, RTBDs, RD, DSD0, SBCs0[0], SBCs0[1], SBCs0[2], SBCs0[3], SBCs0[4], IEDs, RTVs, &PLS, TEXT("0")));
#else
		Threads.emplace_back(std::thread::thread(DX::CreatePipelineState_, std::ref(PipelineStates[0]), COM_PTR_GET(Device), COM_PTR_GET(RootSignatures[0]), D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH, RTBDs, RD, DSD0, SBCs0[0], SBCs0[1], SBCs0[2], SBCs0[3], SBCs0[4], IEDs, RTVs, nullptr, nullptr));
#endif	
#pragma endregion

#pragma region PASS1
		constexpr D3D12_DEPTH_STENCIL_DESC DSD1 = {
			.DepthEnable = FALSE, .DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL, .DepthFunc = D3D12_COMPARISON_FUNC_LESS,
			.StencilEnable = FALSE, .StencilReadMask = D3D12_DEFAULT_STENCIL_READ_MASK, .StencilWriteMask = D3D12_DEFAULT_STENCIL_WRITE_MASK,
			.FrontFace = DSOD, .BackFace = DSOD
		};
		std::vector<COM_PTR<ID3DBlob>> SBs1;
		VERIFY_SUCCEEDED(D3DReadFileToBlob(data(ShaderPath + TEXT("_1.vs.cso")), COM_PTR_PUT(SBs1.emplace_back())));
		VERIFY_SUCCEEDED(D3DReadFileToBlob(data(ShaderPath + TEXT("_1.ps.cso")), COM_PTR_PUT(SBs1.emplace_back())));
		//VERIFY_SUCCEEDED(D3DReadFileToBlob(data(ShaderPath + TEXT("_1_Quilt.ps.cso")), COM_PTR_PUT(SBs1.emplace_back())));
		const std::array SBCs1 = {
			D3D12_SHADER_BYTECODE({.pShaderBytecode = SBs1[0]->GetBufferPointer(), .BytecodeLength = SBs1[0]->GetBufferSize() }),
			D3D12_SHADER_BYTECODE({.pShaderBytecode = SBs1[1]->GetBufferPointer(), .BytecodeLength = SBs1[1]->GetBufferSize() }),
		};
#ifdef USE_PIPELINE_SERIALIZE
		Threads.emplace_back(std::thread::thread(DX::CreatePipelineState_, std::ref(PipelineStates[1]), COM_PTR_GET(Device), COM_PTR_GET(RootSignatures[1]), D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE, RTBDs, RD, DSD1, SBCs1[0], SBCs1[1], NullSBC, NullSBC, NullSBC, IEDs, RTVs, &PLS, TEXT("1")));
#else
		Threads.emplace_back(std::thread::thread(DX::CreatePipelineState_, std::ref(PipelineStates[1]), COM_PTR_GET(Device), COM_PTR_GET(RootSignatures[1]), D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE, RTBDs, RD, DSD1, SBCs1[0], SBCs1[1], , NullSBC, NullSBC, NullSBC, IEDs, RTVs, nullptr, nullptr));
#endif	
#pragma endregion
		for (auto& i : Threads) { i.join(); }
	}
	virtual void CreateDescriptorHeap() override {
#pragma region PASS0
		{
			{
				DXGI_SWAP_CHAIN_DESC1 SCD;
				SwapChain->GetDesc1(&SCD);
#pragma region FRAME_OBJECT
				const D3D12_DESCRIPTOR_HEAP_DESC DHD = {.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, .NumDescriptors = SCD.BufferCount, .Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE, .NodeMask = 0 }; //!< CBV * N
#pragma endregion
				VERIFY_SUCCEEDED(Device->CreateDescriptorHeap(&DHD, COM_PTR_UUIDOF_PUTVOID(CbvSrvUavDescriptorHeaps.emplace_back())));
			}
			{
				const D3D12_DESCRIPTOR_HEAP_DESC DHD = {.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV, .NumDescriptors = 1, .Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE, .NodeMask = 0 };
				VERIFY_SUCCEEDED(Device->CreateDescriptorHeap(&DHD, COM_PTR_UUIDOF_PUTVOID(RtvDescriptorHeaps.emplace_back())));
			}
			{
				const D3D12_DESCRIPTOR_HEAP_DESC DHD = {.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV, .NumDescriptors = 1, .Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE, .NodeMask = 0 };
				VERIFY_SUCCEEDED(Device->CreateDescriptorHeap(&DHD, COM_PTR_UUIDOF_PUTVOID(DsvDescriptorHeaps.emplace_back())));
			}
		}
#pragma endregion

#pragma region PASS1
		{
			const D3D12_DESCRIPTOR_HEAP_DESC DHD = {.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, .NumDescriptors = 1, .Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE, .NodeMask = 0 };
			VERIFY_SUCCEEDED(Device->CreateDescriptorHeap(&DHD, COM_PTR_UUIDOF_PUTVOID(CbvSrvUavDescriptorHeaps.emplace_back())));
		}
#pragma endregion
	}
	virtual void CreateDescriptorView() override {
#pragma region PASS0
		{
			{
				DXGI_SWAP_CHAIN_DESC1 SCD;
				SwapChain->GetDesc1(&SCD);
				const auto& DH = CbvSrvUavDescriptorHeaps[0];
				auto CDH = DH->GetCPUDescriptorHandleForHeapStart();
#pragma region FRAME_OBJECT
				for (UINT i = 0; i < SCD.BufferCount; ++i) {
					const D3D12_CONSTANT_BUFFER_VIEW_DESC CBVD = { .BufferLocation = COM_PTR_GET(ConstantBuffers[i].Resource)->GetGPUVirtualAddress(), .SizeInBytes = static_cast<UINT>(ConstantBuffers[i].Resource->GetDesc().Width) };
					Device->CreateConstantBufferView(&CBVD, CDH); CDH.ptr += Device->GetDescriptorHandleIncrementSize(DH->GetDesc().Type);
				}
#pragma endregion
			}
			{
				const auto& DH = RtvDescriptorHeaps[0];
				auto CDH = DH->GetCPUDescriptorHandleForHeapStart();
				Device->CreateRenderTargetView(COM_PTR_GET(RenderTextures[0].Resource), &RenderTextures[0].RTV, CDH); CDH.ptr += Device->GetDescriptorHandleIncrementSize(DH->GetDesc().Type);
			}
			{
				const auto& DH = DsvDescriptorHeaps[0];
				auto CDH = DH->GetCPUDescriptorHandleForHeapStart();
				Device->CreateDepthStencilView(COM_PTR_GET(DepthTextures.back().Resource), &DepthTextures.back().DSV, CDH); CDH.ptr += Device->GetDescriptorHandleIncrementSize(DH->GetDesc().Type);
			}
		}
#pragma endregion

#pragma region PASS1
		{
			const auto& DH = CbvSrvUavDescriptorHeaps[1];
			auto CDH = DH->GetCPUDescriptorHandleForHeapStart();
			{
				Device->CreateShaderResourceView(COM_PTR_GET(RenderTextures[0].Resource), &RenderTextures[0].SRV, CDH); CDH.ptr += Device->GetDescriptorHandleIncrementSize(DH->GetDesc().Type);
			}
		}
#pragma endregion

#pragma region FRAME_OBJECT
		DXGI_SWAP_CHAIN_DESC1 SCD;
		SwapChain->GetDesc1(&SCD);
		for (UINT i = 0; i < SCD.BufferCount; ++i) {
			CopyToUploadResource(COM_PTR_GET(ConstantBuffers[i].Resource), RoundUp256(sizeof(Transform)), &Transform);
		}
#pragma endregion
	}

	virtual void PopulateCommandList(const size_t i) override;

	virtual void CreateViewport(const FLOAT Width, const FLOAT Height, const FLOAT MinDepth = 0.0f, const FLOAT MaxDepth = 1.0f) override;
	virtual int GetViewportMax() const override { return D3D12_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE; }

protected:
	struct TRANSFORM
	{
		DirectX::XMFLOAT4X4 Projection;
		DirectX::XMFLOAT4X4 View;
		DirectX::XMFLOAT4X4 World;
	};
	using TRANSFORM = struct TRANSFORM;
	TRANSFORM Transform;

	UINT64 QuiltWidth;
	UINT QuiltHeight;
	std::vector<D3D12_VIEWPORT> QuiltViewports;
	std::vector<D3D12_RECT> QuiltScissorRects;
};
#pragma endregion