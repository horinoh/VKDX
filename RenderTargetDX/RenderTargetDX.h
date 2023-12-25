#pragma once

#include "resource.h"

#pragma region Code
#include "../DXExt.h"

#ifdef USE_DEPTH
class RenderTargetDX : public DXExtDepth
#else
class RenderTargetDX : public DXExt
#endif
{
private:
#ifdef USE_DEPTH
	using Super = DXExtDepth;
#else
	using Super = DXExt;
#endif
public:
	RenderTargetDX() : Super() {}
	virtual ~RenderTargetDX() {}

protected:
	virtual void CreateCommandList() override {
		Super::CreateCommandList();
#pragma region PASS1
		DXGI_SWAP_CHAIN_DESC1 SCD;
		SwapChain->GetDesc1(&SCD);
		for (UINT i = 0; i < SCD.BufferCount; ++i) {
			VERIFY_SUCCEEDED(Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_BUNDLE, COM_PTR_GET(BundleCommandAllocators[0]), nullptr, COM_PTR_UUIDOF_PUTVOID(BundleCommandLists.emplace_back())));
			VERIFY_SUCCEEDED(BundleCommandLists.back()->Close());
		}
#pragma endregion
	}

	virtual void CreateGeometry() override {
		const auto CA = COM_PTR_GET(DirectCommandAllocators[0]);
		const auto CL = COM_PTR_GET(DirectCommandLists[0]);
		const auto GCQ = COM_PTR_GET(GraphicsCommandQueue);

#pragma region PASS0
		//!< メッシュ描画用
		constexpr D3D12_DRAW_INDEXED_ARGUMENTS DIA = { .IndexCountPerInstance = 1, .InstanceCount = 1, .StartIndexLocation = 0, .BaseVertexLocation = 0, .StartInstanceLocation = 0 };
		IndirectBuffers.emplace_back().Create(COM_PTR_GET(Device), DIA).ExecuteCopyCommand(COM_PTR_GET(Device), CA, CL, GCQ, COM_PTR_GET(GraphicsFence), sizeof(DIA), &DIA);
		UploadResource Upload_Indirect0;
		Upload_Indirect0.Create(COM_PTR_GET(Device), sizeof(DIA), &DIA);
#pragma endregion

#pragma region PASS1
		//!< フルスクリーン描画用
		constexpr D3D12_DRAW_ARGUMENTS DA = { .VertexCountPerInstance = 4, .InstanceCount = 1, .StartVertexLocation = 0, .StartInstanceLocation = 0 };
		IndirectBuffers.emplace_back().Create(COM_PTR_GET(Device), DA).ExecuteCopyCommand(COM_PTR_GET(Device), CA, CL, GCQ, COM_PTR_GET(GraphicsFence), sizeof(DA), &DA);
		UploadResource Upload_Indirect1;
		Upload_Indirect1.Create(COM_PTR_GET(Device), sizeof(DA), &DA);
#pragma endregion

		VERIFY_SUCCEEDED(CL->Reset(CA, nullptr)); {
			IndirectBuffers[0].PopulateCopyCommand(CL, sizeof(DIA), COM_PTR_GET(Upload_Indirect0.Resource));
			IndirectBuffers[1].PopulateCopyCommand(CL, sizeof(DA), COM_PTR_GET(Upload_Indirect1.Resource));
		} VERIFY_SUCCEEDED(CL->Close());
		DX::ExecuteAndWait(GCQ, CL, COM_PTR_GET(GraphicsFence));
	}
	virtual void CreateTexture() {
		const auto W = static_cast<UINT64>(GetClientRectWidth());
		const auto H = static_cast<UINT>(GetClientRectHeight());
		RenderTextures.emplace_back().Create(COM_PTR_GET(Device), W, H, 1, D3D12_CLEAR_VALUE({.Format = DXGI_FORMAT_R8G8B8A8_UNORM, .Color = { DirectX::Colors::SkyBlue.f[0], DirectX::Colors::SkyBlue.f[1], DirectX::Colors::SkyBlue.f[2], DirectX::Colors::SkyBlue.f[3] } }));
#ifdef USE_DEPTH
		Super::CreateTexture();
#endif
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
			SerializeRootSignature(Blob, {}, {}, SHADER_ROOT_ACCESS_DENY_ALL);
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
			DX::SerializeRootSignature(Blob, {
				D3D12_ROOT_PARAMETER({
					.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE, 
					.DescriptorTable = D3D12_ROOT_DESCRIPTOR_TABLE({.NumDescriptorRanges = static_cast<uint32_t>(size(DRs_Srv)), .pDescriptorRanges = data(DRs_Srv) }), 
					.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL 
				})
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
#ifdef USE_PIPELINE_SERIALIZE
		PipelineLibrarySerializer PLS(COM_PTR_GET(Device), GetFilePath(".plo"));
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
		constexpr D3D12_DEPTH_STENCILOP_DESC DSOD = {.StencilFailOp = D3D12_STENCIL_OP_KEEP, .StencilDepthFailOp = D3D12_STENCIL_OP_KEEP, .StencilPassOp = D3D12_STENCIL_OP_KEEP, .StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS };
		const std::vector<D3D12_INPUT_ELEMENT_DESC> IEDs = {};
		const std::vector RTVs = { DXGI_FORMAT_R8G8B8A8_UNORM };

#pragma region PASS0
		constexpr D3D12_DEPTH_STENCIL_DESC DSD0 = {
#ifdef USE_DEPTH
			.DepthEnable = TRUE, .DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL, .DepthFunc = D3D12_COMPARISON_FUNC_LESS,
#else
			.DepthEnable = FALSE, .DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL, .DepthFunc = D3D12_COMPARISON_FUNC_LESS,
#endif
			.StencilEnable = FALSE, .StencilReadMask = D3D12_DEFAULT_STENCIL_READ_MASK, .StencilWriteMask = D3D12_DEFAULT_STENCIL_WRITE_MASK,
			.FrontFace = DSOD, .BackFace = DSOD
		};
		std::vector<COM_PTR<ID3DBlob>> SBs0;
		VERIFY_SUCCEEDED(D3DReadFileToBlob(data(GetFilePath(".vs.cso").wstring()), COM_PTR_PUT(SBs0.emplace_back())));
		VERIFY_SUCCEEDED(D3DReadFileToBlob(data(GetFilePath(".ps.cso").wstring()), COM_PTR_PUT(SBs0.emplace_back())));
		VERIFY_SUCCEEDED(D3DReadFileToBlob(data(GetFilePath(".ds.cso").wstring()), COM_PTR_PUT(SBs0.emplace_back())));
		VERIFY_SUCCEEDED(D3DReadFileToBlob(data(GetFilePath(".hs.cso").wstring()), COM_PTR_PUT(SBs0.emplace_back())));
		VERIFY_SUCCEEDED(D3DReadFileToBlob(data(GetFilePath(".gs.cso").wstring()), COM_PTR_PUT(SBs0.emplace_back())));
		const std::array SBCs0 = {
			D3D12_SHADER_BYTECODE({.pShaderBytecode = SBs0[0]->GetBufferPointer(), .BytecodeLength = SBs0[0]->GetBufferSize() }),
			D3D12_SHADER_BYTECODE({.pShaderBytecode = SBs0[1]->GetBufferPointer(), .BytecodeLength = SBs0[1]->GetBufferSize() }),
			D3D12_SHADER_BYTECODE({.pShaderBytecode = SBs0[2]->GetBufferPointer(), .BytecodeLength = SBs0[2]->GetBufferSize() }),
			D3D12_SHADER_BYTECODE({.pShaderBytecode = SBs0[3]->GetBufferPointer(), .BytecodeLength = SBs0[3]->GetBufferSize() }),
			D3D12_SHADER_BYTECODE({.pShaderBytecode = SBs0[4]->GetBufferPointer(), .BytecodeLength = SBs0[4]->GetBufferSize() }),
		};
#ifdef USE_PIPELINE_SERIALIZE
		Threads.emplace_back(std::thread::thread(DX::CreatePipelineStateVsPsDsHsGs, std::ref(PipelineStates[0]), COM_PTR_GET(Device), COM_PTR_GET(RootSignatures[0]), D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH, RTBDs, RD, DSD0, SBCs0[0], SBCs0[1], SBCs0[2], SBCs0[3], SBCs0[4], IEDs, RTVs, &PLS, TEXT("0")));
#else
		Threads.emplace_back(std::thread::thread(DX::CreatePipelineStateVsPsDsHsGs, std::ref(PipelineStates[0]), COM_PTR_GET(Device), COM_PTR_GET(RootSignatures[0]), D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH, RTBDs, RD, DSD0, SBCs0[0], SBCs0[1], SBCs0[2], SBCs0[3], SBCs0[4], IEDs, RTVs, nullptr, nullptr));
#endif	
#pragma endregion

#pragma region PASS1
		constexpr D3D12_DEPTH_STENCIL_DESC DSD1 = {
			.DepthEnable = FALSE, .DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL, .DepthFunc = D3D12_COMPARISON_FUNC_LESS,
			.StencilEnable = FALSE, .StencilReadMask = D3D12_DEFAULT_STENCIL_READ_MASK, .StencilWriteMask = D3D12_DEFAULT_STENCIL_WRITE_MASK,
			.FrontFace = DSOD, .BackFace = DSOD
		};
		std::vector<COM_PTR<ID3DBlob>> SBs1;
		VERIFY_SUCCEEDED(D3DReadFileToBlob(data(GetFilePath("_1.vs.cso").wstring()), COM_PTR_PUT(SBs1.emplace_back())));
		VERIFY_SUCCEEDED(D3DReadFileToBlob(data(GetFilePath("_1.ps.cso").wstring()), COM_PTR_PUT(SBs1.emplace_back())));
		const std::array SBCs1 = {
			D3D12_SHADER_BYTECODE({.pShaderBytecode = SBs1[0]->GetBufferPointer(), .BytecodeLength = SBs1[0]->GetBufferSize() }),
			D3D12_SHADER_BYTECODE({.pShaderBytecode = SBs1[1]->GetBufferPointer(), .BytecodeLength = SBs1[1]->GetBufferSize() }),
		};
#ifdef USE_PIPELINE_SERIALIZE
		Threads.emplace_back(std::thread::thread(DX::CreatePipelineStateVsPsDsHsGs, std::ref(PipelineStates[1]), COM_PTR_GET(Device), COM_PTR_GET(RootSignatures[1]), D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE, RTBDs, RD, DSD1, SBCs1[0], SBCs1[1], NullSBC, NullSBC, NullSBC, IEDs, RTVs, &PLS, TEXT("1")));
#else
		Threads.emplace_back(std::thread::thread(DX::CreatePipelineStateVsPsDsHsGs, std::ref(PipelineStates[1]), COM_PTR_GET(Device), COM_PTR_GET(RootSignatures[1]), D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE, RTBDs, RD, DSD1, SBCs1[0], SBCs1[1], NullSBC, NullSBC, NullSBC, IEDs, RTVs, nullptr, nullptr));
#endif	
#pragma endregion
		for (auto& i : Threads) { i.join(); }
	}
	virtual void CreateDescriptor() override {
#pragma region PASS0
		{
			const D3D12_DESCRIPTOR_HEAP_DESC DHD = {.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV, .NumDescriptors = 1, .Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE, .NodeMask = 0 };
			VERIFY_SUCCEEDED(Device->CreateDescriptorHeap(&DHD, COM_PTR_UUIDOF_PUTVOID(RtvDescriptorHeaps.emplace_back())));
		}
#pragma endregion

#pragma region PASS1
		{
			const D3D12_DESCRIPTOR_HEAP_DESC DHD = {.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, .NumDescriptors = 1, .Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE, .NodeMask = 0 };
			VERIFY_SUCCEEDED(Device->CreateDescriptorHeap(&DHD, COM_PTR_UUIDOF_PUTVOID(CbvSrvUavDescriptorHeaps.emplace_back())));
		}
#pragma endregion

#pragma region PASS0
		//!< RTV
		{
			RtvCPUHandles.emplace_back();
			auto CDH = RtvDescriptorHeaps[0]->GetCPUDescriptorHandleForHeapStart();
			Device->CreateRenderTargetView(COM_PTR_GET(RenderTextures.back().Resource), &RenderTextures.back().RTV, CDH); 
			RtvCPUHandles.back().emplace_back(CDH);
		}
#ifdef USE_DEPTH
		//!< DSV
		Super::CreateDescriptor();
#endif
#pragma endregion

#pragma region PASS1
		//!< SRV
		{
			CbvSrvUavGPUHandles.emplace_back();
			auto CDH = CbvSrvUavDescriptorHeaps[0]->GetCPUDescriptorHandleForHeapStart();
			auto GDH = CbvSrvUavDescriptorHeaps[0]->GetGPUDescriptorHandleForHeapStart();
			Device->CreateShaderResourceView(COM_PTR_GET(RenderTextures.back().Resource), &RenderTextures.back().SRV, CDH);
			CbvSrvUavGPUHandles.back().emplace_back(GDH);
		}
#pragma endregion
	}

	virtual void PopulateCommandList(const size_t i) override {
		const auto BCA = COM_PTR_GET(BundleCommandAllocators[0]);

#pragma region PASS0
		//!< バンドルコマンドリスト(メッシュ描画用
		const auto PS0 = COM_PTR_GET(PipelineStates[0]);
		const auto BCL0 = COM_PTR_GET(BundleCommandLists[i]);
		VERIFY_SUCCEEDED(BCL0->Reset(BCA, PS0));
		{
			BCL0->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_1_CONTROL_POINT_PATCHLIST);
			BCL0->ExecuteIndirect(COM_PTR_GET(IndirectBuffers[0].CommandSignature), 1, COM_PTR_GET(IndirectBuffers[0].Resource), 0, nullptr, 0);
		}
		VERIFY_SUCCEEDED(BCL0->Close());
#pragma endregion

#pragma region PASS1
		//!< レンダーテクスチャ描画用
		const auto PS1 = COM_PTR_GET(PipelineStates[1]);
		DXGI_SWAP_CHAIN_DESC1 SCD;
		SwapChain->GetDesc1(&SCD);
		const auto BCL1 = COM_PTR_GET(BundleCommandLists[i + SCD.BufferCount]); //!< オフセットさせる(ここでは2つのバンドルコマンドリストがぞれぞれスワップチェインイメージ数だけある)
		VERIFY_SUCCEEDED(BCL1->Reset(BCA, PS1));
		{
			BCL1->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
			BCL1->ExecuteIndirect(COM_PTR_GET(IndirectBuffers[1].CommandSignature), 1, COM_PTR_GET(IndirectBuffers[1].Resource), 0, nullptr, 0);
		}
		VERIFY_SUCCEEDED(BCL1->Close());
#pragma endregion

		const auto CL = COM_PTR_GET(DirectCommandLists[i]);
		const auto CA = COM_PTR_GET(DirectCommandAllocators[0]);
		VERIFY_SUCCEEDED(CL->Reset(CA, PS1));
		{
			const auto SCR = COM_PTR_GET(SwapChainResources[i]);
			const auto RT = COM_PTR_GET(RenderTextures.back().Resource);

			CL->RSSetViewports(static_cast<UINT>(size(Viewports)), data(Viewports));
			CL->RSSetScissorRects(static_cast<UINT>(size(ScissorRects)), data(ScissorRects));

#pragma region PASS0
			//!< メッシュ描画用
			{
				const auto& HandleDSV = DsvDescs[0].second;

				CL->SetGraphicsRootSignature(COM_PTR_GET(RootSignatures[0]));

				constexpr std::array<D3D12_RECT, 0> Rects = {};
				CL->ClearRenderTargetView(RtvCPUHandles.back()[0], DirectX::Colors::SkyBlue, static_cast<UINT>(size(Rects)), data(Rects));
#ifdef USE_DEPTH
				CL->ClearDepthStencilView(HandleDSV[0], D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, static_cast<UINT>(size(Rects)), data(Rects));

				const std::array RTCHs = { RtvCPUHandles.back()[0] };
				CL->OMSetRenderTargets(static_cast<UINT>(size(RTCHs)), data(RTCHs), FALSE, &HandleDSV[0]);
#else			
				const std::array RTCHs = { RtvCPUHandles[0] };
				CL->OMSetRenderTargets(static_cast<UINT>(size(RTCHs)), data(RTCHs), FALSE, nullptr);
#endif

				CL->ExecuteBundle(BCL0);
			}
#pragma endregion

			//!< リソースバリア
			{
				const std::array RBs = {
					//!< PRESENT -> RENDER_TARGET
					D3D12_RESOURCE_BARRIER({
						.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION,
						.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE,
						.Transition = D3D12_RESOURCE_TRANSITION_BARRIER({.pResource = SCR, .Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES, .StateBefore = D3D12_RESOURCE_STATE_PRESENT, .StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET })
					}),
					//!< RENDER_TARGET -> PIXEL_SHADER_RESOURCE
					D3D12_RESOURCE_BARRIER({
						.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION,
						.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE,
						.Transition = D3D12_RESOURCE_TRANSITION_BARRIER({.pResource = RT, .Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES, .StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET, .StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE })
					}),
				};
				CL->ResourceBarrier(static_cast<UINT>(size(RBs)), data(RBs));
			}

#pragma region PASS1
			//!< レンダーテクスチャ描画用
			{
				CL->SetGraphicsRootSignature(COM_PTR_GET(RootSignatures[1]));

				const std::array CHs = { SwapChainCPUHandles[i] };
				CL->OMSetRenderTargets(static_cast<UINT>(size(CHs)), data(CHs), FALSE, nullptr);

				const std::array DHs = { COM_PTR_GET(CbvSrvUavDescriptorHeaps[0]) };
				CL->SetDescriptorHeaps(static_cast<UINT>(size(DHs)), data(DHs));
				//!< SRV
				CL->SetGraphicsRootDescriptorTable(0, CbvSrvUavGPUHandles.back()[0]); 

				CL->ExecuteBundle(BCL1);
			}

			//!< リソースバリア : D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE -> D3D12_RESOURCE_STATE_RENDER_TARGET
			{
				const std::array RBs = {
					//!< RENDER_TARGET -> PRESENT
					D3D12_RESOURCE_BARRIER({
						.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION,
						.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE,
						.Transition = D3D12_RESOURCE_TRANSITION_BARRIER({.pResource = SCR, .Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES, .StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET, .StateAfter = D3D12_RESOURCE_STATE_PRESENT })
					}),
					//!< PIXEL_SHADER_RESOURCE -> RENDER_TARGET
					D3D12_RESOURCE_BARRIER({
						.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION,
						.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE,
						.Transition = D3D12_RESOURCE_TRANSITION_BARRIER({.pResource = RT, .Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES, .StateBefore = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, .StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET })
					}),
				};
				CL->ResourceBarrier(static_cast<UINT>(size(RBs)), data(RBs));
			}
#pragma endregion
		}
		VERIFY_SUCCEEDED(CL->Close());
	}
};
#pragma endregion