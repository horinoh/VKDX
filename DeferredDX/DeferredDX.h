#pragma once

#include "resource.h"

#pragma region Code
#include "../DXExt.h"

class DeferredDX : public DXExtDepth
{
private:
	using Super = DXExtDepth;
public:
	DeferredDX() : Super() {}
	virtual ~DeferredDX() {}

protected:
	virtual void DrawFrame(const UINT i) override {
		DirectX::XMStoreFloat4x4(&Tr.World, DirectX::XMMatrixRotationX(DirectX::XMConvertToRadians(Degree)));

		if (IsUpdate()) {
			Degree += 1.0f;
		}

#pragma region FRAME_OBJECT
		CopyToUploadResource(COM_PTR_GET(ConstantBuffers[i].Resource), RoundUp256(sizeof(Tr)), &Tr);
#pragma endregion
	}
	virtual void CreateCommandList() override {
		Super::CreateCommandList();
#pragma region PASS1 (Draw fullscreen)
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
#pragma region PASS0 (Draw mesh)
		//!< メッシュ描画用
		constexpr D3D12_DRAW_INDEXED_ARGUMENTS DIA = { .IndexCountPerInstance = 1, .InstanceCount = 1, .StartIndexLocation = 0, .BaseVertexLocation = 0, .StartInstanceLocation = 0 };
		IndirectBuffers.emplace_back().Create(COM_PTR_GET(Device), DIA);
		UploadResource Upload_Indirect0;
		Upload_Indirect0.Create(COM_PTR_GET(Device), sizeof(DIA), &DIA);
#pragma endregion

#pragma region PASS1 (Draw fullscreen)
		//!< フルスクリーン描画用
		constexpr D3D12_DRAW_ARGUMENTS DA = { .VertexCountPerInstance = 4, .InstanceCount = 1, .StartVertexLocation = 0, .StartInstanceLocation = 0 };
		IndirectBuffers.emplace_back().Create(COM_PTR_GET(Device), DA);
		UploadResource Upload_Indirect1;
		Upload_Indirect1.Create(COM_PTR_GET(Device), sizeof(DA), &DA);
#pragma endregion
	
		VERIFY_SUCCEEDED(CL->Reset(CA, nullptr)); {
			IndirectBuffers[0].PopulateCopyCommand(CL, sizeof(DIA), COM_PTR_GET(Upload_Indirect0.Resource));
			IndirectBuffers[1].PopulateCopyCommand(CL, sizeof(DA), COM_PTR_GET(Upload_Indirect1.Resource));
		} VERIFY_SUCCEEDED(CL->Close());
		DX::ExecuteAndWait(GCQ, CL, COM_PTR_GET(GraphicsFence));
	}
	virtual void CreateConstantBuffer() override {
		constexpr auto Fov = 0.16f * std::numbers::pi_v<float>;
		const auto Aspect = GetAspectRatioOfClientRect();
		constexpr auto ZFar = 4.0f;
		constexpr auto ZNear = 2.0f;
		const auto CamPos = DirectX::XMVectorSet(0.0f, 0.0f, 3.0f, 1.0f);
		const auto CamTag = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
		const auto CamUp = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
		const auto Projection = DirectX::XMMatrixPerspectiveFovRH(Fov, Aspect, ZNear, ZFar);
		const auto View = DirectX::XMMatrixLookAtRH(CamPos, CamTag, CamUp);
		const auto World = DirectX::XMMatrixIdentity();

		const auto ViewProjection = DirectX::XMMatrixMultiply(View, Projection);
		auto Det = DirectX::XMMatrixDeterminant(ViewProjection);
		const auto InverseViewProjection = DirectX::XMMatrixInverse(&Det, ViewProjection);

		DirectX::XMStoreFloat4x4(&Tr.Projection, Projection);
		DirectX::XMStoreFloat4x4(&Tr.View, View);
		DirectX::XMStoreFloat4x4(&Tr.World, World);
		DirectX::XMStoreFloat4x4(&Tr.InverseViewProjection, InverseViewProjection);

#pragma region FRAME_OBJECT
		DXGI_SWAP_CHAIN_DESC1 SCD;
		SwapChain->GetDesc1(&SCD);
		for (UINT i = 0; i < SCD.BufferCount; ++i) {
			ConstantBuffers.emplace_back().Create(COM_PTR_GET(Device), sizeof(Tr));
		}
#pragma endregion
	}
	virtual void CreateTexture() override {
		const auto W = static_cast<UINT64>(GetClientRectWidth());
		const auto H = static_cast<UINT>(GetClientRectHeight());

		//!< カラー (Color)
		RenderTextures.emplace_back().Create(COM_PTR_GET(Device), W, H, 1, D3D12_CLEAR_VALUE({ .Format = DXGI_FORMAT_R8G8B8A8_UNORM, .Color = { DirectX::Colors::SkyBlue.f[0], DirectX::Colors::SkyBlue.f[1], DirectX::Colors::SkyBlue.f[2], DirectX::Colors::SkyBlue.f[3] } }));
#pragma region MRT
		//!< 法線 (Normal)
		RenderTextures.emplace_back().Create(COM_PTR_GET(Device), W, H, 1, D3D12_CLEAR_VALUE({.Format = DXGI_FORMAT_R10G10B10A2_UNORM, .Color = { 0.5f, 0.5f, 1.0f, 1.0f} }));
		//!< 深度 (Depth)
		RenderTextures.emplace_back().Create(COM_PTR_GET(Device), W, H, 1, D3D12_CLEAR_VALUE({ .Format = DXGI_FORMAT_R32_FLOAT, .Color = { DirectX::Colors::Red.f[0], DirectX::Colors::Red.f[1], DirectX::Colors::Red.f[2], DirectX::Colors::Red.f[3] } }));
		//!< 未定 
		RenderTextures.emplace_back().Create(COM_PTR_GET(Device), W, H, 1, D3D12_CLEAR_VALUE({ .Format = DXGI_FORMAT_R8G8B8A8_UNORM, .Color = { DirectX::Colors::SkyBlue.f[0], DirectX::Colors::SkyBlue.f[1], DirectX::Colors::SkyBlue.f[2], DirectX::Colors::SkyBlue.f[3] } }));
#pragma endregion

		Super::CreateTexture();
	}
	virtual void CreateStaticSampler() override {
#pragma region PASS1 (Draw fullscreen)
		CreateStaticSampler_LinearWrap(0, 0, D3D12_SHADER_VISIBILITY_PIXEL);
#pragma endregion
	}
	virtual void CreateRootSignature() override {
#pragma region PASS0 (Draw mesh)
		{
			COM_PTR<ID3DBlob> Blob;
#ifdef USE_HLSL_ROOTSIGNATRUE
			GetRootSignaturePartFromShader(Blob, data(GetBasePath() + TEXT(".rs.cso")));
#else
			constexpr std::array DRs = { 
				D3D12_DESCRIPTOR_RANGE({ .RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV, .NumDescriptors = 1, .BaseShaderRegister = 0, .RegisterSpace = 0, .OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND }),
			};
			SerializeRootSignature(Blob, {
				D3D12_ROOT_PARAMETER({ 
					.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE, 
					.DescriptorTable = D3D12_ROOT_DESCRIPTOR_TABLE({ .NumDescriptorRanges = static_cast<UINT>(size(DRs)), .pDescriptorRanges = data(DRs) }), 
					.ShaderVisibility = D3D12_SHADER_VISIBILITY_GEOMETRY
				}),
			}, {}, SHADER_ROOT_ACCESS_GS);
#endif
			VERIFY_SUCCEEDED(Device->CreateRootSignature(0, Blob->GetBufferPointer(), Blob->GetBufferSize(), COM_PTR_UUIDOF_PUTVOID(RootSignatures.emplace_back())));
		}
#pragma endregion

#pragma region PASS1 (Draw fullscreen)
		{
			COM_PTR<ID3DBlob> Blob;
#ifdef USE_HLSL_ROOTSIGNATRUE 
			GetRootSignaturePartFromShader(Blob, data(GetBasePath() + TEXT("_1.rs.cso")));
#else
			constexpr std::array DRs = { 
#pragma region MRT
				//!< レンダーターゲット : カラー(Color), 法線(Normal), 深度(Depth), 未定
				D3D12_DESCRIPTOR_RANGE({ .RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV, .NumDescriptors = 4, .BaseShaderRegister = 0, .RegisterSpace = 0, .OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND }),
#pragma endregion
#ifndef USE_GBUFFER_VISUALIZE
				D3D12_DESCRIPTOR_RANGE({ .RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV, .NumDescriptors = 1, .BaseShaderRegister = 0, .RegisterSpace = 0, .OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND }),
#endif
			};
			DX::SerializeRootSignature(Blob, {
				D3D12_ROOT_PARAMETER({ 
					.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE,
					.DescriptorTable = D3D12_ROOT_DESCRIPTOR_TABLE({ .NumDescriptorRanges = static_cast<uint32_t>(size(DRs)), .pDescriptorRanges = data(DRs) }), 
					.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL 
				}),
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
		const D3D12_DEPTH_STENCILOP_DESC DSOD = { .StencilFailOp = D3D12_STENCIL_OP_KEEP, .StencilDepthFailOp = D3D12_STENCIL_OP_KEEP, .StencilPassOp = D3D12_STENCIL_OP_KEEP, .StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS };
		const std::vector<D3D12_INPUT_ELEMENT_DESC> IEDs = {};
#pragma region PASS0 (Draw mesh)
		const D3D12_DEPTH_STENCIL_DESC DSD0 = {
			.DepthEnable = TRUE, .DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL, .DepthFunc = D3D12_COMPARISON_FUNC_LESS,
			.StencilEnable = FALSE, .StencilReadMask = D3D12_DEFAULT_STENCIL_READ_MASK, .StencilWriteMask = D3D12_DEFAULT_STENCIL_WRITE_MASK,
			.FrontFace = DSOD, .BackFace = DSOD
		};
		const std::vector RTVs0 = {
			DXGI_FORMAT_R8G8B8A8_UNORM,
#pragma region MRT
			DXGI_FORMAT_R10G10B10A2_UNORM,
			DXGI_FORMAT_R32_FLOAT,
			DXGI_FORMAT_R8G8B8A8_UNORM
#pragma endregion
		};
		std::vector<COM_PTR<ID3DBlob>> SBs0;
		VERIFY_SUCCEEDED(D3DReadFileToBlob(data(GetFilePath(".vs.cso").wstring()), COM_PTR_PUT(SBs0.emplace_back())));
		VERIFY_SUCCEEDED(D3DReadFileToBlob(data(GetFilePath(".ps.cso").wstring()), COM_PTR_PUT(SBs0.emplace_back())));
		VERIFY_SUCCEEDED(D3DReadFileToBlob(data(GetFilePath(".ds.cso").wstring()), COM_PTR_PUT(SBs0.emplace_back())));
		VERIFY_SUCCEEDED(D3DReadFileToBlob(data(GetFilePath(".hs.cso").wstring()), COM_PTR_PUT(SBs0.emplace_back())));
		VERIFY_SUCCEEDED(D3DReadFileToBlob(data(GetFilePath(".gs.cso").wstring()), COM_PTR_PUT(SBs0.emplace_back())));
		const std::array SBCs0 = {
			D3D12_SHADER_BYTECODE({ SBs0[0]->GetBufferPointer(), SBs0[0]->GetBufferSize() }),
			D3D12_SHADER_BYTECODE({ SBs0[1]->GetBufferPointer(), SBs0[1]->GetBufferSize() }),
			D3D12_SHADER_BYTECODE({ SBs0[2]->GetBufferPointer(), SBs0[2]->GetBufferSize() }),
			D3D12_SHADER_BYTECODE({ SBs0[3]->GetBufferPointer(), SBs0[3]->GetBufferSize() }),
			D3D12_SHADER_BYTECODE({ SBs0[4]->GetBufferPointer(), SBs0[4]->GetBufferSize() }),
		};
#ifdef USE_PIPELINE_SERIALIZE
		Threads.emplace_back(std::thread::thread(DX::CreatePipelineStateVsPsDsHsGs, std::ref(PipelineStates[0]), COM_PTR_GET(Device), COM_PTR_GET(RootSignatures[0]), D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH, RTBDs, RD, DSD0, SBCs0[0], SBCs0[1], SBCs0[2], SBCs0[3], SBCs0[4], IEDs, RTVs0, &PLS, TEXT("0")));
#else
		Threads.emplace_back(std::thread::thread(DX::CreatePipelineStateVsPsDsHsGs, std::ref(PipelineStates[0]), COM_PTR_GET(Device), COM_PTR_GET(RootSignatures[0]), D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH, RTBDs, RD, DSD0, SBCs0[0], SBCs0[1], SBCs0[2], SBCs0[3], SBCs0[4], IEDs, RTVs0, nullptr, nullptr));
#endif	
#pragma endregion

#pragma region PASS1 (Draw fullscreen)
		const D3D12_DEPTH_STENCIL_DESC DSD1 = {
			.DepthEnable = FALSE, .DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL, .DepthFunc = D3D12_COMPARISON_FUNC_LESS,
			.StencilEnable = FALSE, .StencilReadMask = D3D12_DEFAULT_STENCIL_READ_MASK, .StencilWriteMask = D3D12_DEFAULT_STENCIL_WRITE_MASK,
			.FrontFace = DSOD, .BackFace = DSOD
		};
		const std::vector RTVs = { DXGI_FORMAT_R8G8B8A8_UNORM };
		std::vector<COM_PTR<ID3DBlob>> SBs1;
		VERIFY_SUCCEEDED(D3DReadFileToBlob(data(GetFilePath("_1.vs.cso").wstring()), COM_PTR_PUT(SBs1.emplace_back())));
#ifdef USE_GBUFFER_VISUALIZE
		VERIFY_SUCCEEDED(D3DReadFileToBlob(data(GetFilePath("_gb_1.ps.cso").wstring()), COM_PTR_PUT(SBs1.emplace_back())));
		VERIFY_SUCCEEDED(D3DReadFileToBlob(data(GetFilePath("_gb_1.gs.cso").wstring()), COM_PTR_PUT(SBs1.emplace_back())));
#else
		VERIFY_SUCCEEDED(D3DReadFileToBlob(data(GetFilePath("_1.ps.cso").wstring()), COM_PTR_PUT(SBs1.emplace_back())));
#endif
		const std::array SBCs1 = {
#ifdef USE_GBUFFER_VISUALIZE
			D3D12_SHADER_BYTECODE({ SBs1[0]->GetBufferPointer(), SBs1[0]->GetBufferSize() }),
			D3D12_SHADER_BYTECODE({ SBs1[1]->GetBufferPointer(), SBs1[1]->GetBufferSize() }),
			D3D12_SHADER_BYTECODE({ SBs1[2]->GetBufferPointer(), SBs1[2]->GetBufferSize() }),
#else
			D3D12_SHADER_BYTECODE({ SBs1[0]->GetBufferPointer(), SBs1[0]->GetBufferSize() }),
			D3D12_SHADER_BYTECODE({ SBs1[1]->GetBufferPointer(), SBs1[1]->GetBufferSize() }),
#endif
		};
#ifdef USE_PIPELINE_SERIALIZE
#ifdef USE_GBUFFER_VISUALIZE
		Threads.emplace_back(std::thread::thread(DX::CreatePipelineStateVsPsDsHsGs, std::ref(PipelineStates[1]), COM_PTR_GET(Device), COM_PTR_GET(RootSignatures[1]), D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE, RTBDs, RD, DSD1, SBCs1[0], SBCs1[1], NullSBC, NullSBC, SBCs1[2], IEDs, RTVs, &PLS, TEXT("1")));
#else
		Threads.emplace_back(std::thread::thread(DX::CreatePipelineStateVsPsDsHsGs, std::ref(PipelineStates[1]), COM_PTR_GET(Device), COM_PTR_GET(RootSignatures[1]), D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE, RTBDs, RD, DSD1, SBCs1[0], SBCs1[1], NullSBC, NullSBC, NullSBC, IEDs, RTVs, &PLS, TEXT("1")));
#endif
#else
#ifdef USE_GBUFFER_VISUALIZE
		Threads.emplace_back(std::thread::thread(DX::CreatePipelineStateVsPsDsHsGs, std::ref(PipelineStates[1]), COM_PTR_GET(Device), COM_PTR_GET(RootSignatures[1]), D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE, RTBDs, RD, DSD1, SBCs1[0], SBCs1[1], NullSBC, NullSBC, SBCs1[2], IEDs, RTVs, nullptr, nullptr));
#else
		Threads.emplace_back(std::thread::thread(DX::CreatePipelineStateVsPsDsHsGs, std::ref(PipelineStates[1]), COM_PTR_GET(Device), COM_PTR_GET(RootSignatures[1]), D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE, RTBDs, RD, DSD1, SBCs1[0], SBCs1[1], NullSBC, NullSBC, NullSBC, IEDs, RTVs, nullptr, nullptr));
#endif
#endif	
#pragma endregion

		for (auto& i : Threads) { i.join(); }
	}
	virtual void CreateDescriptor() override {
#pragma region FRAME_OBJECT
		DXGI_SWAP_CHAIN_DESC1 SCD;
		SwapChain->GetDesc1(&SCD);
#pragma endregion

#pragma region PASS0 (Draw mesh)
		{
			auto& DescCBV = CbvSrvUavDescs.emplace_back();
			auto& HeapCBV = DescCBV.first;
			auto& HandleCBV = DescCBV.second;

			auto& DescRTV = RtvDescs.emplace_back();
			auto& HeapRTV = DescRTV.first;
			auto& HandleRTV = DescRTV.second;

			{
#pragma region FRAME_OBJECT
				{
					const D3D12_DESCRIPTOR_HEAP_DESC DHD = { .Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, .NumDescriptors = SCD.BufferCount, .Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE, .NodeMask = 0 };
					//!< CBV * N
					VERIFY_SUCCEEDED(Device->CreateDescriptorHeap(&DHD, COM_PTR_UUIDOF_PUTVOID(HeapCBV)));
				}
#pragma endregion
#pragma region MRT
				{
					//!< レンダーターゲット : カラー(Color), 法線(Normal), 深度(Depth), 未定
					const D3D12_DESCRIPTOR_HEAP_DESC DHD = { .Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV, .NumDescriptors = 4, .Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE, .NodeMask = 0 };
					//!< RTV * 4
					VERIFY_SUCCEEDED(Device->CreateDescriptorHeap(&DHD, COM_PTR_UUIDOF_PUTVOID(HeapRTV)));
				}
#pragma endregion
			}

			{
				//!< CBV
				{
					auto CDH = HeapCBV->GetCPUDescriptorHandleForHeapStart();
					auto GDH = HeapCBV->GetGPUDescriptorHandleForHeapStart();
					const auto IncSize = Device->GetDescriptorHandleIncrementSize(HeapCBV->GetDesc().Type);
#pragma region FRAME_OBJECT
					for (UINT i = 0; i < SCD.BufferCount; ++i) {
						const D3D12_CONSTANT_BUFFER_VIEW_DESC CBVD = {
							.BufferLocation = ConstantBuffers[i].Resource->GetGPUVirtualAddress(),
							.SizeInBytes = static_cast<UINT>(ConstantBuffers[i].Resource->GetDesc().Width)
						};
						Device->CreateConstantBufferView(&CBVD, CDH);
						HandleCBV.emplace_back(GDH);
						CDH.ptr += IncSize;
						GDH.ptr += IncSize;
					}
#pragma endregion
				}
				//!< RTV
				{
					auto CDH = HeapRTV->GetCPUDescriptorHandleForHeapStart();
					const auto IncSize = Device->GetDescriptorHandleIncrementSize(HeapRTV->GetDesc().Type);
					//!< カラー(Color)
					Device->CreateRenderTargetView(COM_PTR_GET(RenderTextures[0].Resource), &RenderTextures[0].RTV, CDH);
					HandleRTV.emplace_back(CDH);
					CDH.ptr += IncSize;
#pragma region MRT
					//!< 法線(Normal)
					Device->CreateRenderTargetView(COM_PTR_GET(RenderTextures[1].Resource), &RenderTextures[1].RTV, CDH);
					HandleRTV.emplace_back(CDH);
					CDH.ptr += IncSize;
					//!< 深度(Depth)
					Device->CreateRenderTargetView(COM_PTR_GET(RenderTextures[2].Resource), &RenderTextures[2].RTV, CDH);
					HandleRTV.emplace_back(CDH);
					CDH.ptr += IncSize;
					//!< 未定
					Device->CreateRenderTargetView(COM_PTR_GET(RenderTextures[3].Resource), &RenderTextures[3].RTV, CDH);
					HandleRTV.emplace_back(CDH);
					CDH.ptr += IncSize;
#pragma endregion
				}
			}

			Super::CreateDescriptor();
		}
#pragma endregion

#pragma region PASS1 (Draw fullscreen)
		{
			auto& Desc = CbvSrvUavDescs.emplace_back();
			auto& Heap = Desc.first;
			auto& Handle = Desc.second;

#pragma region MRT
#pragma region FRAME_OBJECT
			{
				//!< レンダーターゲット : カラー(Color), 法線(Normal), 深度(Depth), 未定
				const D3D12_DESCRIPTOR_HEAP_DESC DHD = { 
					.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
#ifdef USE_GBUFFER_VISUALIZE
					.NumDescriptors = 4,
#else
					.NumDescriptors = 4 + SCD.BufferCount,
#endif
					.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE,
					.NodeMask = 0
				}; 
				//!< SRV * 4 + CBV * N
				VERIFY_SUCCEEDED(Device->CreateDescriptorHeap(&DHD, COM_PTR_UUIDOF_PUTVOID(Heap)));
			}
#pragma endregion
#pragma endregion
			//!< SRV
			{
				auto CDH = Heap->GetCPUDescriptorHandleForHeapStart();
				auto GDH = Heap->GetGPUDescriptorHandleForHeapStart();
				const auto IncSize = Device->GetDescriptorHandleIncrementSize(Heap->GetDesc().Type);
				//!< レンダーターゲット : カラー(RenderTarget : Color)
				Device->CreateShaderResourceView(COM_PTR_GET(RenderTextures[0].Resource), &RenderTextures[0].SRV, CDH); 
				Handle.emplace_back(GDH);
				CDH.ptr += IncSize;
				GDH.ptr += IncSize;
#pragma region MRT
				//!< レンダーターゲット : 法線(RenderTarget : Normal)
				Device->CreateShaderResourceView(COM_PTR_GET(RenderTextures[1].Resource), &RenderTextures[1].SRV, CDH);
				Handle.emplace_back(GDH);
				CDH.ptr += IncSize;
				GDH.ptr += IncSize;
				//!< レンダーターゲット : 深度(RenderTarget : Depth)
				Device->CreateShaderResourceView(COM_PTR_GET(RenderTextures[2].Resource), &RenderTextures[2].SRV, CDH);
				Handle.emplace_back(GDH);
				CDH.ptr += IncSize;
				GDH.ptr += IncSize;
				//!< レンダーターゲット : 未定
				Device->CreateShaderResourceView(COM_PTR_GET(RenderTextures[3].Resource), &RenderTextures[3].SRV, CDH);
				Handle.emplace_back(GDH);
				CDH.ptr += IncSize;
				GDH.ptr += IncSize; 
#pragma	endregion

#ifndef USE_GBUFFER_VISUALIZE
				//!< CBV
				{
#pragma region FRAME_OBJECT
					for (UINT i = 0; i < SCD.BufferCount; ++i) {
						const D3D12_CONSTANT_BUFFER_VIEW_DESC CBVD = {
							.BufferLocation = ConstantBuffers[i].Resource->GetGPUVirtualAddress(),
							.SizeInBytes = static_cast<UINT>(ConstantBuffers[i].Resource->GetDesc().Width)
						};
						Device->CreateConstantBufferView(&CBVD, CDH);
						Handle.emplace_back(GDH);
						CDH.ptr += IncSize;
						GDH.ptr += IncSize;
					}
#pragma endregion
				}
#endif
			}
		}
#pragma endregion
	}
	virtual void PopulateCommandList(const size_t i) override {
		const auto BCA = COM_PTR_GET(BundleCommandAllocators[0]);

#pragma region PASS0 (Draw mesh)
		//!< メッシュ描画用
		const auto PS0 = COM_PTR_GET(PipelineStates[0]);
		const auto BCL0 = COM_PTR_GET(BundleCommandLists[i]);
		VERIFY_SUCCEEDED(BCL0->Reset(BCA, PS0));
		{
			const auto IDBCS = COM_PTR_GET(IndirectBuffers[0].CommandSignature);
			const auto IDBR = COM_PTR_GET(IndirectBuffers[0].Resource);

			BCL0->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_1_CONTROL_POINT_PATCHLIST);
			BCL0->ExecuteIndirect(IDBCS, 1, IDBR, 0, nullptr, 0);
		}
		VERIFY_SUCCEEDED(BCL0->Close());
#pragma endregion

#pragma region PASS1 (Draw fullscreen)
		//!< レンダーテクスチャ描画用
		const auto PS1 = COM_PTR_GET(PipelineStates[1]);
		const auto BGCL1 = COM_PTR_GET(BundleCommandLists[i + size(BundleCommandLists) / 2]); //!< オフセットさせる(ここでは2つのバンドルコマンドリストがぞれぞれスワップチェインイメージ数だけある)
		VERIFY_SUCCEEDED(BGCL1->Reset(BCA, PS1));
		{
			const auto IDBCS = COM_PTR_GET(IndirectBuffers[1].CommandSignature);
			const auto IDBR = COM_PTR_GET(IndirectBuffers[1].Resource);

			BGCL1->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
			BGCL1->ExecuteIndirect(IDBCS, 1, IDBR, 0, nullptr, 0);
		}
		VERIFY_SUCCEEDED(BGCL1->Close());
#pragma endregion

		const auto GCL = COM_PTR_GET(DirectCommandLists[i]);
		const auto CA = COM_PTR_GET(DirectCommandAllocators[0]);
		VERIFY_SUCCEEDED(GCL->Reset(CA, PS1));
		{
			const auto SCR = COM_PTR_GET(SwapChainResources[i]);
			const auto RT = COM_PTR_GET(RenderTextures[0].Resource);

			GCL->RSSetViewports(static_cast<UINT>(size(Viewports)), data(Viewports));
			GCL->RSSetScissorRects(static_cast<UINT>(size(ScissorRects)), data(ScissorRects));

#pragma region PASS0 (Draw mesh)
			auto Pass = 0;
			//!< メッシュ描画用
			{
				GCL->SetGraphicsRootSignature(COM_PTR_GET(RootSignatures[Pass]));

				const auto& DescRTV = RtvDescs[Pass];
				const auto& HandleRTV = DescRTV.second;
				
				const auto& DescDSV = DsvDescs[Pass];
				const auto& HandleDSV = DescDSV.second;

				//!< クリア
				{
					constexpr std::array<D3D12_RECT, 0> Rects = {};
					GCL->ClearRenderTargetView(HandleRTV[0], DirectX::Colors::SkyBlue, static_cast<UINT>(size(Rects)), data(Rects));
#pragma region MRT
					GCL->ClearRenderTargetView(HandleRTV[1], data(std::array<FLOAT, 4>({ 0.5f, 0.5f, 1.0f, 1.0f })), static_cast<UINT>(size(Rects)), data(Rects));
					GCL->ClearRenderTargetView(HandleRTV[2], DirectX::Colors::Red, static_cast<UINT>(size(Rects)), data(Rects));
					GCL->ClearRenderTargetView(HandleRTV[3], DirectX::Colors::SkyBlue, static_cast<UINT>(size(Rects)), data(Rects));
#pragma endregion
					GCL->ClearDepthStencilView(HandleDSV[0], D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, static_cast<UINT>(size(Rects)), data(Rects));
				}

				//!< レンダーターゲット
				{
#if 0
					const std::array CHs = { RtvCPUHandles[Pass][0], RtvCPUHandles[Pass][1], RtvCPUHandles[Pass][2], RtvCPUHandles[Pass][3] };
					//!< RTV, DSV
					GCL->OMSetRenderTargets(static_cast<UINT>(size(CHs)), data(CHs), FALSE, &DsvCPUHandles[Pass][0]);
#else
					//!< 「連続している」場合は、「個数」と「先頭アドレス」を指定して「RTsSingleHandleToDescriptorRange==TRUE」で良い
					const std::array CHs = { HandleRTV[0] };
					//!< RTV, DSV
					GCL->OMSetRenderTargets(4, data(CHs), TRUE, &HandleDSV[0]);
#endif
				}

				{
					const auto& Desc = CbvSrvUavDescs[Pass];
					const auto& Heap = Desc.first;
					const auto& Handle = Desc.second;

					const std::array DHs = { COM_PTR_GET(Heap) };
					GCL->SetDescriptorHeaps(static_cast<UINT>(size(DHs)), data(DHs));
#pragma region FRAME_OBJECT
					//!< CBV
					GCL->SetGraphicsRootDescriptorTable(0, Handle[i]);
#pragma endregion
				}

				GCL->ExecuteBundle(BCL0);
			}
#pragma endregion

			//!< リソースバリア
			{
				const std::array RBs = {
					//!< スワップチェイン PRESENT -> RENDER_TARGET
					D3D12_RESOURCE_BARRIER({
						.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION,
						.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE,
						.Transition = D3D12_RESOURCE_TRANSITION_BARRIER({.pResource = SCR, .Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES, .StateBefore = D3D12_RESOURCE_STATE_PRESENT, .StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET })
					}),
					//!< イメージ RENDER_TARGET -> PIXEL_SHADER_RESOURCE
					D3D12_RESOURCE_BARRIER({
						.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION,
						.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE,
						.Transition = D3D12_RESOURCE_TRANSITION_BARRIER({.pResource = RT, .Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES, .StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET, .StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE })
					}),
				};
				GCL->ResourceBarrier(static_cast<UINT>(size(RBs)), data(RBs));
			}

#pragma region PASS1 (Draw fullscreen)
			Pass = 1;
			//!< レンダーテクスチャ描画用
			{
				GCL->SetGraphicsRootSignature(COM_PTR_GET(RootSignatures[Pass]));

				const std::array CHs = { SwapChainCPUHandles[i] };
				GCL->OMSetRenderTargets(static_cast<UINT>(size(CHs)), data(CHs), FALSE, nullptr); 

				{
					const auto& Desc = CbvSrvUavDescs[Pass];
					const auto& Heap = Desc.first;
					const auto& Handle = Desc.second;

					const std::array DHs = { COM_PTR_GET(Heap) };
					GCL->SetDescriptorHeaps(static_cast<UINT>(size(DHs)), data(DHs));
					
					//!< SRV
					GCL->SetGraphicsRootDescriptorTable(0, Handle[0]);
#ifndef USE_GBUFFER_VISUALIZE
#pragma region FRAME_OBJECT
					//!< CBV
					//GCL->SetGraphicsRootDescriptorTable(1, Handle[4 + i]);
#pragma endregion
#endif
				}

				GCL->ExecuteBundle(BGCL1);
			}
#pragma endregion

			//!< リソースバリア : D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE -> D3D12_RESOURCE_STATE_RENDER_TARGET
			{
				const std::array RBs = {
					//!< スワップチェイン RENDER_TARGET -> PRESENT
					D3D12_RESOURCE_BARRIER({
						.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION,
						.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE,
						.Transition = D3D12_RESOURCE_TRANSITION_BARRIER({.pResource = SCR, .Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES, .StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET, .StateAfter = D3D12_RESOURCE_STATE_PRESENT })
					}),
					//!< イメージ PIXEL_SHADER_RESOURCE -> RENDER_TARGET
					D3D12_RESOURCE_BARRIER({
						.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION,
						.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE,
						.Transition = D3D12_RESOURCE_TRANSITION_BARRIER({.pResource = RT, .Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES, .StateBefore = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, .StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET })
					}),
				};
				GCL->ResourceBarrier(static_cast<UINT>(size(RBs)), data(RBs));
			}
		}
		VERIFY_SUCCEEDED(GCL->Close());
	}

#ifdef USE_GBUFFER_VISUALIZE
	virtual void CreateViewport(const FLOAT Width, const FLOAT Height, const FLOAT MinDepth = 0.0f, const FLOAT MaxDepth = 1.0f) override {
		D3D12_FEATURE_DATA_D3D12_OPTIONS3 FDO3;
		VERIFY_SUCCEEDED(Device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS3, reinterpret_cast<void*>(&FDO3), sizeof(FDO3)));
		assert(D3D12_VIEW_INSTANCING_TIER_1 < FDO3.ViewInstancingTier && "");

		const auto W = Width * 0.5f, H = Height * 0.5f;
		Viewports = {
			//!< 全画面用
			D3D12_VIEWPORT({.TopLeftX = 0.0f, .TopLeftY = 0.0f, .Width = Width, .Height = Height, .MinDepth = MinDepth, .MaxDepth = MaxDepth }),
			//!< 分割画面用
			D3D12_VIEWPORT({.TopLeftX = 0.0f, .TopLeftY = 0.0f, .Width = W, .Height = H, .MinDepth = MinDepth, .MaxDepth = MaxDepth }),
			D3D12_VIEWPORT({.TopLeftX = W, .TopLeftY = 0.0f, .Width = W, .Height = H, .MinDepth = MinDepth, .MaxDepth = MaxDepth }),
			D3D12_VIEWPORT({.TopLeftX = 0.0f, .TopLeftY = H, .Width = W, .Height = H, .MinDepth = MinDepth, .MaxDepth = MaxDepth }),
			D3D12_VIEWPORT({.TopLeftX = W, .TopLeftY = H, .Width = W, .Height = H, .MinDepth = MinDepth, .MaxDepth = MaxDepth }),
		};
		//!< left, top, right, bottomで指定 (offset, extentで指定のVKとは異なるので注意)
		ScissorRects = {
			//!< 全画面用
			D3D12_RECT({.left = 0, .top = 0, .right = static_cast<LONG>(Width), .bottom = static_cast<LONG>(Height) }),
			//!< 分割画面用
			D3D12_RECT({.left = 0, .top = 0, .right = static_cast<LONG>(W), .bottom = static_cast<LONG>(H) }),
			D3D12_RECT({.left = static_cast<LONG>(W), .top = 0, .right = static_cast<LONG>(Width), .bottom = static_cast<LONG>(H) }),
			D3D12_RECT({.left = 0, .top = static_cast<LONG>(H), .right = static_cast<LONG>(W), .bottom = static_cast<LONG>(Height) }),
			D3D12_RECT({.left = static_cast<LONG>(W), .top = static_cast<LONG>(H), .right = static_cast<LONG>(Width), .bottom = static_cast<LONG>(Height) }),
		};
		LOG_OK();
	}
#endif

private:
	FLOAT Degree = 0.0f;
	struct Transform
	{
		DirectX::XMFLOAT4X4 Projection;
		DirectX::XMFLOAT4X4 View;
		DirectX::XMFLOAT4X4 World;
		DirectX::XMFLOAT4X4 InverseViewProjection;
	};
	using Transform = struct Transform;
	Transform Tr;
};
#pragma endregion