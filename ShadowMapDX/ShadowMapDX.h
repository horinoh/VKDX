#pragma once

#include "resource.h"

#pragma region Code
#include "../DXExt.h"

class ShadowMapDX : public DXExt
{
private:
	using Super = DXExt;
public:
	ShadowMapDX() : Super() {}
	virtual ~ShadowMapDX() {}

protected:
	virtual void OnTimer(HWND hWnd, HINSTANCE hInstance) override {
		Super::OnTimer(hWnd, hInstance);

		DirectX::XMStoreFloat4x4(&Tr.World, DirectX::XMMatrixRotationY(DirectX::XMConvertToRadians(Degree)));
		Degree += 1.0f;

#pragma region FRAME_OBJECT
		CopyToUploadResource(COM_PTR_GET(ConstantBuffers[GetCurrentBackBufferIndex()].Resource), RoundUp256(sizeof(Tr)), &Tr);
#pragma endregion
	}
	virtual void CreateCommandList() override {
		Super::CreateCommandList();
		//!< パス1 : バンドルコマンドリスト
		DXGI_SWAP_CHAIN_DESC1 SCD;
		SwapChain->GetDesc1(&SCD);
		for (UINT i = 0; i < SCD.BufferCount; ++i) {
			VERIFY_SUCCEEDED(Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_BUNDLE, COM_PTR_GET(BundleCommandAllocators[0]), nullptr, COM_PTR_UUIDOF_PUTVOID(BundleGraphicsCommandLists.emplace_back())));
			VERIFY_SUCCEEDED(BundleGraphicsCommandLists.back()->Close());
		}
	}
	virtual void CreateGeometry() override {
		const auto CA = COM_PTR_GET(CommandAllocators[0]);
		const auto GCL = COM_PTR_GET(GraphicsCommandLists[0]);
		const auto GCQ = COM_PTR_GET(GraphicsCommandQueue);
		//!< パス0 : インダイレクトバッファ(シャドウキャスタ描画用 : トーラス)
		{
			constexpr D3D12_DRAW_INDEXED_ARGUMENTS DIA = { .IndexCountPerInstance = 1, .InstanceCount = 1, .StartIndexLocation = 0, .BaseVertexLocation = 0, .StartInstanceLocation = 0 };
			IndirectBuffers.emplace_back().Create(COM_PTR_GET(Device), DIA).ExecuteCopyCommand(COM_PTR_GET(Device), CA, GCL, GCQ, COM_PTR_GET(Fence), sizeof(DIA), &DIA);
		}
#ifdef USE_SHADOWMAP_VISUALIZE
		//!< パス1 : インダイレクトバッファ(シャドウマップ描画用 : フルスクリーン)
		{
			constexpr D3D12_DRAW_ARGUMENTS DA = { .VertexCountPerInstance = 4, .InstanceCount = 1, .StartVertexLocation = 0, .StartInstanceLocation = 0 };
			IndirectBuffers.emplace_back().Create(COM_PTR_GET(Device), DA).ExecuteCopyCommand(COM_PTR_GET(Device), CA, GCL, GCQ, COM_PTR_GET(Fence), sizeof(DA), &DA);
		}
#else
		//!< パス1 : インダイレクトバッファ(シャドウレシーバ描画用 : トーラス、平面)
		{
			constexpr D3D12_DRAW_INDEXED_ARGUMENTS DIA = { .IndexCountPerInstance = 1, .InstanceCount = 2, .StartIndexLocation = 0, .BaseVertexLocation = 0, .StartInstanceLocation = 0 };
			IndirectBuffers.emplace_back().Create(COM_PTR_GET(Device), DIA).ExecuteCopyCommand(COM_PTR_GET(Device), CA, GCL, GCQ, COM_PTR_GET(Fence), sizeof(DIA), &DIA);
		}
#endif
	}
	virtual void CreateConstantBuffer() override {
		{
			const auto Direction = DirectX::XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
			const auto Z = DirectX::XMVector3Normalize(DirectX::XMVectorNegate(Direction));
			const auto X = DirectX::XMVector3Normalize(DirectX::XMVector3Cross(Z, DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f)));
			const auto Y = DirectX::XMVector3Cross(X, Z);
			//!< シャドウキャスタのAABB
			const auto Center = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
			const auto Radius = DirectX::XMVectorSet(1.0f, 1.0f, 1.0f, 1.0f);
			const std::array Points = {
				DirectX::XMVectorAdd(Center, DirectX::XMVectorSet(Radius.m128_f32[0],  Radius.m128_f32[1],  Radius.m128_f32[2], 0.0f)),
				DirectX::XMVectorAdd(Center, DirectX::XMVectorSet(Radius.m128_f32[0],  Radius.m128_f32[1], -Radius.m128_f32[2], 0.0f)),
				DirectX::XMVectorAdd(Center, DirectX::XMVectorSet(Radius.m128_f32[0], -Radius.m128_f32[1],  Radius.m128_f32[2], 0.0f)),
				DirectX::XMVectorAdd(Center, DirectX::XMVectorSet(Radius.m128_f32[0], -Radius.m128_f32[1], -Radius.m128_f32[2], 0.0f)),
				DirectX::XMVectorAdd(Center, DirectX::XMVectorSet(-Radius.m128_f32[0],  Radius.m128_f32[1],  Radius.m128_f32[2], 0.0f)),
				DirectX::XMVectorAdd(Center, DirectX::XMVectorSet(-Radius.m128_f32[0],  Radius.m128_f32[1], -Radius.m128_f32[2], 0.0f)),
				DirectX::XMVectorAdd(Center, DirectX::XMVectorSet(-Radius.m128_f32[0], -Radius.m128_f32[1],  Radius.m128_f32[2], 0.0f)),
				DirectX::XMVectorAdd(Center, DirectX::XMVectorSet(-Radius.m128_f32[0], -Radius.m128_f32[1], -Radius.m128_f32[2], 0.0f)),
			};

			auto Mn = (std::numeric_limits<FLOAT>::max)();
			auto Mx = (std::numeric_limits<FLOAT>::min)();
			for (auto i : Points) {
				const auto t = DirectX::XMVector3Dot(i, Z).m128_f32[0];
				Mn = (std::min)(Mn, t);
				Mx = (std::max)(Mx, t);
			}
			const auto ZRadius = (Mx - Mn) * 0.5f;

			Mn = (std::numeric_limits<FLOAT>::max)();
			Mx = (std::numeric_limits<FLOAT>::min)();
			for (auto i : Points) {
				const auto t = DirectX::XMVector3Dot(i, X).m128_f32[0];
				Mn = (std::min)(Mn, t);
				Mx = (std::max)(Mx, t);
			}
			const auto XRadius = (Mx - Mn) * 0.5f;

			Mn = (std::numeric_limits<FLOAT>::max)();
			Mx = (std::numeric_limits<FLOAT>::min)();
			for (auto i : Points) {
				const auto t = DirectX::XMVector3Dot(i, Y).m128_f32[0];
				Mn = (std::min)(Mn, t);
				Mx = (std::max)(Mx, t);
			}
			const auto YRadius = (Mx - Mn) * 0.5f;

			const auto Projection = DirectX::XMMatrixOrthographicRH(XRadius * 2.0f, YRadius * 2.0f, 1.0f, 1.0f + ZRadius * 2.0f);
			const auto View = DirectX::XMMatrixLookAtRH(DirectX::XMVectorSubtract(Center, DirectX::XMVectorScale(Z, (1.0f + ZRadius))), Center, Y);
			DirectX::XMStoreFloat4x4(&Tr.LightProjection, Projection);
			DirectX::XMStoreFloat4x4(&Tr.LightView, View);
#ifdef USE_SHADOWMAP_VISUALIZE
			DirectX::XMStoreFloat4x4(&Tr.Projection, Projection);
			DirectX::XMStoreFloat4x4(&Tr.View, View);
#endif
		}
#ifndef USE_SHADOWMAP_VISUALIZE
		{
			constexpr auto Fov = 0.16f * std::numbers::pi_v<float>;
			const auto Aspect = GetAspectRatioOfClientRect();
			constexpr auto ZFar = 4.0f;
			constexpr auto ZNear = 2.0f;
			const auto CamPos = DirectX::XMVectorSet(0.0f, 0.0f, 3.0f, 1.0f);
			const auto CamTag = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
			const auto CamUp = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
			const auto Projection = DirectX::XMMatrixPerspectiveFovRH(Fov, Aspect, ZNear, ZFar);
			const auto View = DirectX::XMMatrixLookAtRH(CamPos, CamTag, CamUp);
			DirectX::XMStoreFloat4x4(&Tr.Projection, Projection);
			DirectX::XMStoreFloat4x4(&Tr.View, View);
		}
#endif
		const auto World = DirectX::XMMatrixIdentity();
		DirectX::XMStoreFloat4x4(&Tr.World, World);

#pragma region FRAME_OBJECT
		DXGI_SWAP_CHAIN_DESC1 SCD;
		SwapChain->GetDesc1(&SCD);
		for (UINT i = 0; i < SCD.BufferCount; ++i) {
			ConstantBuffers.emplace_back().Create(COM_PTR_GET(Device), sizeof(Tr));
		}
#pragma endregion
	}
	virtual void CreateTexture()
	{
		//!< パス0
		DepthTextures.emplace_back().Create(COM_PTR_GET(Device), static_cast<UINT64>(ShadowMapExtentW), static_cast<UINT>(ShadowMapExtentH), 1, D3D12_CLEAR_VALUE({ .Format = DXGI_FORMAT_D24_UNORM_S8_UINT, .DepthStencil = D3D12_DEPTH_STENCIL_VALUE({.Depth = 1.0f, .Stencil = 0 }) }));
		//!< フォーマットを_TYPELESSにしなくてはならない
		//!< DXGI_FORMAT_D24_UNORM_S8_UINT -> DXGI_FORMAT_R24_UNORM_X8_TYPELESS : 基本的に D -> R, UINT -> TYPELESS のように置換したフォーマットを指定
		//DepthTextures.back().SRV = D3D12_SHADER_RESOURCE_VIEW_DESC({
		//	.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS,
		//	.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D,
		//	.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING,
		//	.Texture2D = D3D12_TEX2D_SRV({.MostDetailedMip = 0, .MipLevels = DepthTextures.back().Resource->GetDesc().MipLevels, .PlaneSlice = 0, .ResourceMinLODClamp = 0.0f })
		//});
	}
	virtual void CreateStaticSampler() override {
		//!< パス1 : スタティックサンプラ
		//!< シェーダ内で SamplerComparisonState を使用する場合は、比較方法(D3D12_FILTER_COMPARISON_..., D3D12_COMPARISON_FUNC_...)を指定すること
		StaticSamplerDescs.emplace_back(D3D12_STATIC_SAMPLER_DESC({
			.Filter = D3D12_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR,
			.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP, .AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP, .AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
			.MipLODBias = 0.0f,
			.MaxAnisotropy = 0,
			.ComparisonFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL,
			.BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE,
			.MinLOD = 0.0f, .MaxLOD = 1.0f,
			.ShaderRegister = 0, .RegisterSpace = 0, .ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL
		}));
	}
	virtual void CreateRootSignature() override {
		//!< パス0 : ルートシグネチャ
		{
			COM_PTR<ID3DBlob> Blob;
#ifdef USE_HLSL_ROOTSIGNATRUE
			GetRootSignaturePartFromShader(Blob, data(GetBasePath() + TEXT(".rs.cso")));
#else
			constexpr std::array DRs = { 
				D3D12_DESCRIPTOR_RANGE({ .RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV, .NumDescriptors = 1, .BaseShaderRegister = 0, .RegisterSpace = 0, .OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND }),
			};
			SerializeRootSignature(Blob, {
				D3D12_ROOT_PARAMETER({.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE, .DescriptorTable = D3D12_ROOT_DESCRIPTOR_TABLE({.NumDescriptorRanges = static_cast<UINT>(size(DRs)), .pDescriptorRanges = data(DRs) }), .ShaderVisibility = D3D12_SHADER_VISIBILITY_GEOMETRY })
			}, {}, SHADER_ROOT_ACCESS_GS);
#endif
			VERIFY_SUCCEEDED(Device->CreateRootSignature(0, Blob->GetBufferPointer(), Blob->GetBufferSize(), COM_PTR_UUIDOF_PUTVOID(RootSignatures.emplace_back())));
		}
		//!< パス1 : ルートシグネチャ
		{
			COM_PTR<ID3DBlob> Blob;
#ifdef USE_HLSL_ROOTSIGNATRUE 
			GetRootSignaturePartFromShader(Blob, data(GetBasePath() + TEXT("_1.rs.cso")));
#else
			assert(!empty(StaticSamplerDescs) && "");
#ifdef USE_SHADOWMAP_VISUALIZE
			constexpr std::array DRs = { 
				D3D12_DESCRIPTOR_RANGE({ .RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV, .NumDescriptors = 1, .BaseShaderRegister = 0, .RegisterSpace = 0, .OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND }),
			};
			DX::SerializeRootSignature(Blob, {
				D3D12_ROOT_PARAMETER({ .ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE, .DescriptorTable = D3D12_ROOT_DESCRIPTOR_TABLE({ .NumDescriptorRanges = static_cast<uint32_t>(size(DRs)), .pDescriptorRanges = data(DRs) }), .ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL }),
			}, {
				StaticSamplerDescs[0],
			}, SHADER_ROOT_ACCESS_PS);
#else
			constexpr std::array DRs_Srv = { D3D12_DESCRIPTOR_RANGE({ D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND }) };
			constexpr std::array DRs_Cbv = { D3D12_DESCRIPTOR_RANGE({ D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND }) };
			DX::SerializeRootSignature(Blob, {
				{ D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE, { static_cast<uint32_t>(size(DRs_Srv)), data(DRs_Srv) }, D3D12_SHADER_VISIBILITY_PIXEL },
				{ D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE, { static_cast<uint32_t>(size(DRs_Cbv)), data(DRs_Cbv) }, D3D12_SHADER_VISIBILITY_GEOMETRY },
			}, {
				StaticSamplerDescs[0],
			}, SHADER_ROOT_ACCESS_GS_PS);
#endif //!< USE_SHADOWMAP_VISUALIZE
#endif //!< USE_HLSL_ROOTSIGNATRUE
			VERIFY_SUCCEEDED(Device->CreateRootSignature(0, Blob->GetBufferPointer(), Blob->GetBufferSize(), COM_PTR_UUIDOF_PUTVOID(RootSignatures.emplace_back())));
		}
		LOG_OK();
	}
	virtual void CreatePipelineState() override {
		const auto ShaderPath = GetBasePath();
		std::vector<COM_PTR<ID3DBlob>> SBs;
		//!< パス0 : シェーダブロブ
		VERIFY_SUCCEEDED(D3DReadFileToBlob(data(ShaderPath + TEXT(".vs.cso")), COM_PTR_PUT(SBs.emplace_back())));
		VERIFY_SUCCEEDED(D3DReadFileToBlob(data(ShaderPath + TEXT(".ds.cso")), COM_PTR_PUT(SBs.emplace_back())));
		VERIFY_SUCCEEDED(D3DReadFileToBlob(data(ShaderPath + TEXT(".hs.cso")), COM_PTR_PUT(SBs.emplace_back())));
		VERIFY_SUCCEEDED(D3DReadFileToBlob(data(ShaderPath + TEXT(".gs.cso")), COM_PTR_PUT(SBs.emplace_back())));
		//!< パス1 : シェーダブロブ
#ifdef USE_SHADOWMAP_VISUALIZE
		VERIFY_SUCCEEDED(D3DReadFileToBlob(data(ShaderPath + TEXT("_sm_1.vs.cso")), COM_PTR_PUT(SBs.emplace_back())));
		VERIFY_SUCCEEDED(D3DReadFileToBlob(data(ShaderPath + TEXT("_sm_1.ps.cso")), COM_PTR_PUT(SBs.emplace_back())));
#else
		VERIFY_SUCCEEDED(D3DReadFileToBlob(data(ShaderPath + TEXT("_1.ps.cso")), COM_PTR_PUT(SBs.emplace_back())));
		VERIFY_SUCCEEDED(D3DReadFileToBlob(data(ShaderPath + TEXT("_1.gs.cso")), COM_PTR_PUT(SBs.emplace_back())));
#endif

		PipelineStates.resize(2);
		std::vector<std::thread> Threads;
		const std::vector RTBDs = {
			D3D12_RENDER_TARGET_BLEND_DESC({
				.BlendEnable = FALSE, .LogicOpEnable = FALSE,
				.SrcBlend = D3D12_BLEND_ONE, .DestBlend = D3D12_BLEND_ZERO, .BlendOp = D3D12_BLEND_OP_ADD,
				.SrcBlendAlpha = D3D12_BLEND_ONE, .DestBlendAlpha = D3D12_BLEND_ZERO, .BlendOpAlpha = D3D12_BLEND_OP_ADD,
				.LogicOp = D3D12_LOGIC_OP_NOOP,
				.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL,
			}),
		};
		//!< RD_0 : デプスバイアス有り (With depth bias)
		constexpr D3D12_RASTERIZER_DESC RD_0 = {
			.FillMode = D3D12_FILL_MODE_SOLID,
			.CullMode = D3D12_CULL_MODE_BACK, .FrontCounterClockwise = TRUE,
			//!< シャドウキャスタの描画でデプスバイアスを有効にする
			//!< DepthBias, DepthBiasClamp, SlopeScaledDepthBias
			//!< r * DepthBias + m * SlopeScaledDepthBias
			//!< DepthBiasClamp : 非0.0fを指定の場合クランプが有効になる(絶対値がDepthBiasClamp以下になるようにクランプされる)
			.DepthBias = 1, .DepthBiasClamp = 0.0f, .SlopeScaledDepthBias = 1.75f,
			.DepthClipEnable = TRUE,
			.MultisampleEnable = FALSE, .AntialiasedLineEnable = FALSE, .ForcedSampleCount = 0,
			.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF
		};
		//!< RD_1 : デプスバイアス無し (No depth bias)
		constexpr D3D12_RASTERIZER_DESC RD_1 = {
			.FillMode = D3D12_FILL_MODE_SOLID,
			.CullMode = D3D12_CULL_MODE_BACK, .FrontCounterClockwise = TRUE,
			.DepthBias = D3D12_DEFAULT_DEPTH_BIAS, .DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP, .SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS,
			.DepthClipEnable = TRUE,
			.MultisampleEnable = FALSE, .AntialiasedLineEnable = FALSE, .ForcedSampleCount = 0,
			.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF
		};
		constexpr D3D12_DEPTH_STENCILOP_DESC DSOD = { .StencilFailOp = D3D12_STENCIL_OP_KEEP, .StencilDepthFailOp = D3D12_STENCIL_OP_KEEP, .StencilPassOp = D3D12_STENCIL_OP_KEEP, .StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS };
		constexpr D3D12_DEPTH_STENCIL_DESC DSD_0 = {
			TRUE, D3D12_DEPTH_WRITE_MASK_ALL, D3D12_COMPARISON_FUNC_LESS,
			FALSE, D3D12_DEFAULT_STENCIL_READ_MASK, D3D12_DEFAULT_STENCIL_WRITE_MASK,
			DSOD, DSOD
		};
		constexpr D3D12_DEPTH_STENCIL_DESC DSD_1 = {
#ifdef USE_SHADOWMAP_VISUALIZE
			.DepthEnable = FALSE, .DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL, .DepthFunc = D3D12_COMPARISON_FUNC_LESS,
#else
			.DepthEnable = TRUE, .DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL, .DepthFunc = D3D12_COMPARISON_FUNC_LESS,
#endif
			.StencilEnable = FALSE, .StencilReadMask = D3D12_DEFAULT_STENCIL_READ_MASK, .StencilWriteMask = D3D12_DEFAULT_STENCIL_WRITE_MASK,
			.FrontFace = DSOD, .BackFace = DSOD
		};
		const std::array SBCs_0 = {
			D3D12_SHADER_BYTECODE({.pShaderBytecode = SBs[0]->GetBufferPointer(), .BytecodeLength = SBs[0]->GetBufferSize() }),
			D3D12_SHADER_BYTECODE({.pShaderBytecode = SBs[1]->GetBufferPointer(), .BytecodeLength = SBs[1]->GetBufferSize() }),
			D3D12_SHADER_BYTECODE({.pShaderBytecode = SBs[2]->GetBufferPointer(), .BytecodeLength = SBs[2]->GetBufferSize() }),
			D3D12_SHADER_BYTECODE({.pShaderBytecode = SBs[3]->GetBufferPointer(), .BytecodeLength = SBs[3]->GetBufferSize() }),
		};
#ifdef USE_SHADOWMAP_VISUALIZE
		const std::array SBCs_1 = {
			D3D12_SHADER_BYTECODE({.pShaderBytecode = SBs[4]->GetBufferPointer(), .BytecodeLength = SBs[4]->GetBufferSize() }),
			D3D12_SHADER_BYTECODE({.pShaderBytecode = SBs[5]->GetBufferPointer(), .BytecodeLength = SBs[5]->GetBufferSize() }),
		};
#else
		const std::array SBCs_1 = {
			D3D12_SHADER_BYTECODE({.pShaderBytecode = SBs[0]->GetBufferPointer(), .BytecodeLength = SBs[0]->GetBufferSize() }),
			D3D12_SHADER_BYTECODE({.pShaderBytecode = SBs[4]->GetBufferPointer(), .BytecodeLength = SBs[4]->GetBufferSize() }), //!< 
			D3D12_SHADER_BYTECODE({.pShaderBytecode = SBs[1]->GetBufferPointer(), .BytecodeLength = SBs[1]->GetBufferSize() }),
			D3D12_SHADER_BYTECODE({.pShaderBytecode = SBs[2]->GetBufferPointer(), .BytecodeLength = SBs[2]->GetBufferSize() }),
			D3D12_SHADER_BYTECODE({.pShaderBytecode = SBs[5]->GetBufferPointer(), .BytecodeLength = SBs[5]->GetBufferSize() }), //!< 
		};
#endif
		const std::vector<D3D12_INPUT_ELEMENT_DESC> IEDs = {};
		const std::vector<DXGI_FORMAT> RTVs_0 = {};
		const std::vector RTVs_1 = { DXGI_FORMAT_R8G8B8A8_UNORM };
#ifdef USE_PIPELINE_SERIALIZE
		PipelineLibrarySerializer PLS(COM_PTR_GET(Device), GetBasePath() + TEXT(".plo"));
		//!< パス0 : パイプラインステート
		Threads.emplace_back(std::thread::thread(DX::CreatePipelineState_, std::ref(PipelineStates[0]), COM_PTR_GET(Device), COM_PTR_GET(RootSignatures[0]), D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH, RTBDs, RD_0, DSD_0, SBCs_0[0], NullSBC, SBCs_0[1], SBCs_0[2], SBCs_0[3], IEDs, RTVs_0, &PLS, TEXT("0")));
		//!< パス1 : パイプラインステート
#ifdef USE_SHADOWMAP_VISUALIZE
		Threads.emplace_back(std::thread::thread(DX::CreatePipelineState_, std::ref(PipelineStates[1]), COM_PTR_GET(Device), COM_PTR_GET(RootSignatures[1]), D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE, RTBDs, RD_1, DSD_1, SBCs_1[0], SBCs_1[1], NullSBC, NullSBC, NullSBC, IEDs, RTVs_1, &PLS, TEXT("1")));
#else
		Threads.emplace_back(std::thread::thread(DX::CreatePipelineState_, std::ref(PipelineStates[1]), COM_PTR_GET(Device), COM_PTR_GET(RootSignatures[1]), D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH, RTBDs, RD_1, DSD_1, SBCs_1[0], SBCs_1[1], SBCs_1[2], SBCs_1[3], SBCs_1[4], IEDs, RTVs_1, &PLS, TEXT("1")));
#endif
#else
		Threads.emplace_back(std::thread::thread(DX::CreatePipelineState_, std::ref(PipelineStates[0]), COM_PTR_GET(Device), COM_PTR_GET(RootSignatures[0]), D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH, RTBDs, RD_0, DSD_0, SBCs_0[0], NullSBC, SBCs_0[1], SBCs_0[2], SBCs_0[3], IEDs, RTVs_0, nullptr, nullptr));
#ifdef USE_SHADOWMAP_VISUALIZE
		Threads.emplace_back(std::thread::thread(DX::CreatePipelineState_, std::ref(PipelineStates[1]), COM_PTR_GET(Device), COM_PTR_GET(RootSignatures[1]), D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE, RTBDs, RD_1, DSD_1, SBCs_1[0], SBCs_1[1], NullSBC, NullSBC, NullSBC, IEDs, RTVs_1, nullptr, nullptr));
#else
		Threads.emplace_back(std::thread::thread(DX::CreatePipelineState_, std::ref(PipelineStates[1]), COM_PTR_GET(Device), COM_PTR_GET(RootSignatures[1]), D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH, RTBDs, RD_1, DSD_1, SBCs_1[0], SBCs_1[1], SBCs_1[2], SBCs_1[3], SBCs_1[4], IEDs, RTVs_1, nullptr, nullptr));
#endif
#endif	
		for (auto& i : Threads) { i.join(); }
	}
	virtual void CreateDescriptorHeap() override {
		{
			//!< パス0 + パス1
#pragma region FRAME_OBJECT
			DXGI_SWAP_CHAIN_DESC1 SCD;
			SwapChain->GetDesc1(&SCD);
#ifdef USE_SHADOWMAP_VISUALIZE
			const D3D12_DESCRIPTOR_HEAP_DESC DHD = { .Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, .NumDescriptors = SCD.BufferCount + 1, .Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE, .NodeMask = 0 }; //!< CBV * N + SRV
#else
			const D3D12_DESCRIPTOR_HEAP_DESC DHD = { .Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, .NumDescriptors = SCD.BufferCount * 2 + 1, .Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE, .NodeMask = 0 }; //!< CBV * N * 2 + SRV
#endif
#pragma endregion
			VERIFY_SUCCEEDED(Device->CreateDescriptorHeap(&DHD, COM_PTR_UUIDOF_PUTVOID(CbvSrvUavDescriptorHeaps.emplace_back())));
		}
		{
			//!< パス0 + パス1
#ifdef USE_SHADOWMAP_VISUALIZE
			const D3D12_DESCRIPTOR_HEAP_DESC DHD = { .Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV, .NumDescriptors = 1 + 0, .Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE, .NodeMask = 0 }; //!< DSV
#else
			const D3D12_DESCRIPTOR_HEAP_DESC DHD = { .Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV, .NumDescriptors = 1 + 1, .Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE, .NodeMask = 0 }; //!< DSV + DSV
#endif
			VERIFY_SUCCEEDED(Device->CreateDescriptorHeap(&DHD, COM_PTR_UUIDOF_PUTVOID(DsvDescriptorHeaps.emplace_back())));
		}
	}
	virtual void CreateDescriptorView() override {
		{
			const auto& DH = CbvSrvUavDescriptorHeaps[0];
			auto CDH = DH->GetCPUDescriptorHandleForHeapStart();

			DXGI_SWAP_CHAIN_DESC1 SCD;
			SwapChain->GetDesc1(&SCD);

			//!< パス0
#pragma region FRAME_OBJECT
			for (UINT i = 0; i < SCD.BufferCount; ++i) {
				const D3D12_CONSTANT_BUFFER_VIEW_DESC CBVD = { .BufferLocation = ConstantBuffers[i].Resource->GetGPUVirtualAddress(), .SizeInBytes = static_cast<UINT>(ConstantBuffers[i].Resource->GetDesc().Width) };
				Device->CreateConstantBufferView(&CBVD, CDH); CDH.ptr += Device->GetDescriptorHandleIncrementSize(DH->GetDesc().Type); //!< CBV
			}
#pragma endregion

			//!< パス1
			const D3D12_SHADER_RESOURCE_VIEW_DESC SRVD = {
				.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS,
				.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D,
				.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING,
				.Texture2D = D3D12_TEX2D_SRV({ .MostDetailedMip = 0, .MipLevels = DepthTextures[0].Resource->GetDesc().MipLevels, .PlaneSlice = 0, .ResourceMinLODClamp = 0.0f })
			};
			Device->CreateShaderResourceView(COM_PTR_GET(DepthTextures[0].Resource), &SRVD, CDH); CDH.ptr += Device->GetDescriptorHandleIncrementSize(DH->GetDesc().Type); //!< SRV

#ifndef USE_SHADOWMAP_VISUALIZE
#pragma region FRAME_OBJECT
			for (UINT i = 0; i < SCD.BufferCount; ++i) {
				const D3D12_CONSTANT_BUFFER_VIEW_DESC CBVD = { .BufferLocation = ConstantBuffers[i].Resource->GetGPUVirtualAddress(), .SizeInBytes = static_cast<UINT>(ConstantBuffers[i].Resource->GetDesc().Width) };
				Device->CreateConstantBufferView(&CBVD, CDH); CDH.ptr += Device->GetDescriptorHandleIncrementSize(DH->GetDesc().Type); //!< CBV
			}
#pragma endregion
#endif
		}
		{
			const auto& DH = DsvDescriptorHeaps[0];
			auto CDH = DH->GetCPUDescriptorHandleForHeapStart();

			//!< パス0
			Device->CreateDepthStencilView(COM_PTR_GET(DepthTextures[0].Resource), &DepthTextures[0].DSV, CDH); CDH.ptr += Device->GetDescriptorHandleIncrementSize(DH->GetDesc().Type); //!< DSV

			//!< パス1
			const auto SRV = D3D12_SHADER_RESOURCE_VIEW_DESC({
			.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS,
			.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D,
			.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING,
			.Texture2D = D3D12_TEX2D_SRV({.MostDetailedMip = 0, .MipLevels = DepthTextures.back().Resource->GetDesc().MipLevels, .PlaneSlice = 0, .ResourceMinLODClamp = 0.0f })
			});
#ifndef USE_SHADOWMAP_VISUALIZE
			Device->CreateDepthStencilView(COM_PTR_GET(DepthTextures[0].Resource), &SRV, CDH); CDH.ptr += Device->GetDescriptorHandleIncrementSize(DH->GetDesc().Type); //!< SRV
#endif
		}
	}

	virtual void PopulateCommandList(const size_t i) override;

private:
	FLOAT Degree = 0.0f;
	struct Transform
	{
		DirectX::XMFLOAT4X4 Projection;
		DirectX::XMFLOAT4X4 View;
		DirectX::XMFLOAT4X4 World;
		DirectX::XMFLOAT4X4 LightProjection;
		DirectX::XMFLOAT4X4 LightView;
	};
	using Transform = struct Transform;
	Transform Tr;

	UINT ShadowMapExtentW = 2048, ShadowMapExtentH = 2048;
};
#pragma endregion