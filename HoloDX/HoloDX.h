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
	virtual void OnCreate(HWND hWnd, HINSTANCE hInstance, LPCWSTR Title) override {
		SetWindow(hWnd);
		Super::OnCreate(hWnd, hInstance, Title);
	}
	virtual void OnKeyDown(HWND hWnd, [[maybe_unused]] HINSTANCE hInstance, const WPARAM Param) override {
		switch (Param) {
		case VK_ESCAPE:
			SendMessage(hWnd, WM_DESTROY, 0, 0);
			break;
		case VK_PAUSE:
			TogglePause();
			break;
		case VK_RETURN:
			ToggleBorderless(hWnd);
			break;
		case VK_SPACE:
			Step();
			break;
		default:
			break;
		}
	}
	virtual void DrawFrame([[maybe_unused]] const UINT i) override {
#pragma region FRAME_OBJECT
		//CopyToUploadResource(COM_PTR_GET(ConstantBuffers[i].Resource), RoundUp256(sizeof(Tr)), &Tr);
#pragma endregion
	}

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
	virtual void CreateConstantBuffer() override {
		constexpr auto Fov = DirectX::XMConvertToRadians(14.0f);
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
		CreateTexture_Render(QuiltWidth, QuiltHeight);
		CreateTexture_Depth(QuiltWidth, QuiltHeight);
	}
	virtual void CreateStaticSampler() override {
#pragma region PASS1
		CreateStaticSampler_LinearWrap(0, 0, D3D12_SHADER_VISIBILITY_PIXEL);
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
#ifdef DRAW_QUILT
		VERIFY_SUCCEEDED(D3DReadFileToBlob(data(GetFilePath("_1_Quilt.ps.cso").wstring()), COM_PTR_PUT(SBs1.emplace_back())));
#else
		VERIFY_SUCCEEDED(D3DReadFileToBlob(data(GetFilePath("_1.ps.cso").wstring()), COM_PTR_PUT(SBs1.emplace_back())));
#endif
		const std::array SBCs1 = {
			D3D12_SHADER_BYTECODE({.pShaderBytecode = SBs1[0]->GetBufferPointer(), .BytecodeLength = SBs1[0]->GetBufferSize() }),
			D3D12_SHADER_BYTECODE({.pShaderBytecode = SBs1[1]->GetBufferPointer(), .BytecodeLength = SBs1[1]->GetBufferSize() }),
		};
#ifdef USE_PIPELINE_SERIALIZE
		Threads.emplace_back(std::thread::thread(DX::CreatePipelineStateVsPsDsHsGs, std::ref(PipelineStates[1]), COM_PTR_GET(Device), COM_PTR_GET(RootSignatures[1]), D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE, RTBDs, RD, DSD1, SBCs1[0], SBCs1[1], NullSBC, NullSBC, NullSBC, IEDs, RTVs, &PLS, TEXT("1")));
#else
		Threads.emplace_back(std::thread::thread(DX::CreatePipelineStateVsPsDsHsGs, std::ref(PipelineStates[1]), COM_PTR_GET(Device), COM_PTR_GET(RootSignatures[1]), D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE, RTBDs, RD, DSD1, SBCs1[0], SBCs1[1], , NullSBC, NullSBC, NullSBC, IEDs, RTVs, nullptr, nullptr));
#endif	
#pragma endregion
		for (auto& i : Threads) { i.join(); }
	}
	virtual void CreateDescriptor() override {
#pragma region PASS0
		{
			auto& DescCBV = CbvSrvUavDescs.emplace_back();
			auto& HeapCBV = DescCBV.first;
			auto& HandleCBV = DescCBV.second;

			auto& DescRTV = RtvDescs.emplace_back();
			auto& HeapRTV = DescRTV.first;
			auto& HandleRTV = DescRTV.second;

			auto& DescDSV = DsvDescs.emplace_back();
			auto& HeapDSV = DescDSV.first;
			auto& HandleDSV = DescDSV.second;

			{
				DXGI_SWAP_CHAIN_DESC1 SCD;
				SwapChain->GetDesc1(&SCD);
#pragma region FRAME_OBJECT
				const D3D12_DESCRIPTOR_HEAP_DESC DHD = {.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, .NumDescriptors = SCD.BufferCount, .Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE, .NodeMask = 0 }; //!< CBV * N
#pragma endregion
				VERIFY_SUCCEEDED(Device->CreateDescriptorHeap(&DHD, COM_PTR_UUIDOF_PUTVOID(HeapCBV)));

#pragma region FRAME_OBJECT
				auto CDH = HeapCBV->GetCPUDescriptorHandleForHeapStart();
				auto GDH = HeapCBV->GetGPUDescriptorHandleForHeapStart();
				const auto IncSize = Device->GetDescriptorHandleIncrementSize(HeapCBV->GetDesc().Type);
				for (UINT i = 0; i < SCD.BufferCount; ++i) {
					const D3D12_CONSTANT_BUFFER_VIEW_DESC CBVD = { .BufferLocation = ConstantBuffers[i].Resource->GetGPUVirtualAddress(), .SizeInBytes = static_cast<UINT>(ConstantBuffers[i].Resource->GetDesc().Width) };
					Device->CreateConstantBufferView(&CBVD, CDH);
					HandleCBV.emplace_back(GDH);
					CDH.ptr += IncSize;
					GDH.ptr += IncSize;
				}
#pragma endregion
			}
			{
				const D3D12_DESCRIPTOR_HEAP_DESC DHD = {.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV, .NumDescriptors = 1, .Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE, .NodeMask = 0 };
				VERIFY_SUCCEEDED(Device->CreateDescriptorHeap(&DHD, COM_PTR_UUIDOF_PUTVOID(HeapRTV)));

				auto CDH = HeapRTV->GetCPUDescriptorHandleForHeapStart();
				const auto IncSize = Device->GetDescriptorHandleIncrementSize(HeapRTV->GetDesc().Type);
				Device->CreateRenderTargetView(COM_PTR_GET(RenderTextures[0].Resource), &RenderTextures[0].RTV, CDH);
				HandleRTV.emplace_back(CDH);
				CDH.ptr += IncSize;
			}
			{
				const D3D12_DESCRIPTOR_HEAP_DESC DHD = {.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV, .NumDescriptors = 1, .Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE, .NodeMask = 0 };
				VERIFY_SUCCEEDED(Device->CreateDescriptorHeap(&DHD, COM_PTR_UUIDOF_PUTVOID(HeapDSV)));

				auto CDH = HeapDSV->GetCPUDescriptorHandleForHeapStart();
				const auto IncSize = Device->GetDescriptorHandleIncrementSize(HeapDSV->GetDesc().Type);
				Device->CreateDepthStencilView(COM_PTR_GET(DepthTextures.back().Resource), &DepthTextures.back().DSV, CDH);
				HandleDSV.emplace_back(CDH);
				CDH.ptr += IncSize;
			}
		}
#pragma endregion

#pragma region PASS1
		{
			auto& DescCBV = CbvSrvUavDescs.emplace_back();
			auto& HeapCBV = DescCBV.first;
			auto& HandleCBV = DescCBV.second;

			const D3D12_DESCRIPTOR_HEAP_DESC DHD = {.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, .NumDescriptors = 1, .Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE, .NodeMask = 0 };
			VERIFY_SUCCEEDED(Device->CreateDescriptorHeap(&DHD, COM_PTR_UUIDOF_PUTVOID(HeapCBV)));

			auto CDH = HeapCBV->GetCPUDescriptorHandleForHeapStart();
			auto GDH = HeapCBV->GetGPUDescriptorHandleForHeapStart();
			const auto IncSize = Device->GetDescriptorHandleIncrementSize(HeapCBV->GetDesc().Type);
			Device->CreateShaderResourceView(COM_PTR_GET(RenderTextures[0].Resource), &RenderTextures[0].SRV, CDH);
			HandleCBV.emplace_back(GDH);
			CDH.ptr += IncSize;
			GDH.ptr += IncSize;
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

	virtual void PopulateCommandList(const size_t i) override {
		const auto BCA = COM_PTR_GET(BundleCommandAllocators[0]);
#pragma region PASS0
		//!< メッシュ描画用
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
			const auto RT = COM_PTR_GET(RenderTextures[0].Resource);

#pragma region PASS0
			//!< メッシュ描画用
			{
				const auto& HandleRTV = RtvDescs[0].second;
				const auto& HandleDSV = DsvDescs[0].second;

				CL->SetGraphicsRootSignature(COM_PTR_GET(RootSignatures[0]));

				constexpr std::array<D3D12_RECT, 0> Rects = {};
				CL->ClearRenderTargetView(HandleRTV[0], DirectX::Colors::SkyBlue, static_cast<UINT>(size(Rects)), data(Rects));

				CL->ClearDepthStencilView(HandleDSV[0], D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, static_cast<UINT>(size(Rects)), data(Rects));

				const std::array RTCDHs = { HandleRTV[0]};
				CL->OMSetRenderTargets(static_cast<UINT>(size(RTCDHs)), data(RTCDHs), FALSE, &HandleDSV[0]);

				{
					const auto& Desc = CbvSrvUavDescs[0];
					const auto& Heap = Desc.first;
					const auto& Handle = Desc.second;

					const std::array DHs = { COM_PTR_GET(Heap) };
					CL->SetDescriptorHeaps(static_cast<UINT>(size(DHs)), data(DHs));
#pragma region FRAME_OBJECT
					CL->SetGraphicsRootDescriptorTable(0, Handle[i]);
#pragma endregion
				}

				const auto ViewTotal = GetQuiltSetting().GetViewTotal();
				const auto ViewportMax = GetViewportMax();
				const auto Repeat = ViewTotal / ViewportMax + (std::min)(ViewTotal % ViewportMax, 1);
				for (auto j = 0; j < Repeat; ++j) {
					const auto Start = j * ViewportMax;
					const auto Count = (std::min)(ViewTotal - j * ViewportMax, ViewportMax);
#pragma region ROOT_CONSTANT
					QuiltDraw.ViewIndexOffset = Start;
					CL->SetGraphicsRoot32BitConstants(1, static_cast<UINT>(sizeof(QuiltDraw)), &QuiltDraw, 0);
#pragma endregion
					CL->RSSetViewports(Count, &QuiltViewports[Start]);
					CL->RSSetScissorRects(Count, &QuiltScissorRects[Start]);
					CL->ExecuteBundle(BCL0);
				}
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
				CL->RSSetViewports(static_cast<UINT>(size(Viewports)), data(Viewports));
				CL->RSSetScissorRects(static_cast<UINT>(size(ScissorRects)), data(ScissorRects));

				CL->SetGraphicsRootSignature(COM_PTR_GET(RootSignatures[1]));

				auto SCCDH = SwapChainDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
				SCCDH.ptr += i * Device->GetDescriptorHandleIncrementSize(SwapChainDescriptorHeap->GetDesc().Type);

				const std::array RtvCDHs = { SCCDH };
				CL->OMSetRenderTargets(static_cast<UINT>(size(RtvCDHs)), data(RtvCDHs), FALSE, nullptr);

				const auto& Desc = CbvSrvUavDescs[1];
				const auto& Heap = Desc.first;
				const auto& Handle = Desc.second;

				const std::array DHs = { COM_PTR_GET(Heap) };
				CL->SetDescriptorHeaps(static_cast<UINT>(size(DHs)), data(DHs));
				CL->SetGraphicsRootDescriptorTable(0, Handle[0]);

#pragma region ROOT_CONSTANT
				CL->SetGraphicsRoot32BitConstants(1, static_cast<UINT>(sizeof(HoloDraw)), &HoloDraw, 0);
#pragma endregion

				CL->ExecuteBundle(BCL1);
			}
#pragma endregion

			//!< リソースバリア
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
		}
		VERIFY_SUCCEEDED(CL->Close());
	}

	virtual void CreateViewport(const FLOAT Width, const FLOAT Height, const FLOAT MinDepth = 0.0f, const FLOAT MaxDepth = 1.0f) override {
#pragma region PASS0
		{
			//!< キルトの構造
			//!< ------------> RightMost
			//!< ---------------------->
			//!< ---------------------->
			//!< ---------------------->
			//!< LeftMost ------------->
			const auto& QS = GetQuiltSetting();
			const auto W = QS.GetViewWidth(), H = QS.GetViewHeight();
			for (auto i = 0; i < QS.GetViewRow(); ++i) {
				for (auto j = 0; j < QS.GetViewColumn(); ++j) {
					const auto X = j * W, Y = QS.GetHeight() - (i + 1) * H;
					QuiltViewports.emplace_back(D3D12_VIEWPORT({ .TopLeftX = static_cast<FLOAT>(X), .TopLeftY = static_cast<FLOAT>(Y), .Width = static_cast<FLOAT>(W), .Height = static_cast<FLOAT>(H), .MinDepth = MinDepth, .MaxDepth = MaxDepth }));
					QuiltScissorRects.emplace_back(D3D12_RECT({ .left = X, .top = Y, .right = static_cast<LONG>(X + W), .bottom = static_cast<LONG>(Y + H) }));

					Logf("QuiltViewport[%d] = (%d, %d) %d x %d\n", i * QS.GetViewColumn() + j, static_cast<int>(QuiltViewports.back().TopLeftX), static_cast<int>(QuiltViewports.back().TopLeftY), static_cast<int>(QuiltViewports.back().Width), static_cast<int>(QuiltViewports.back().Height));
				}
			}
		}
#pragma endregion

#pragma region PASS1
		{
			Super::CreateViewport(Width, Height, MinDepth, MaxDepth);
		}
#pragma endregion
	}
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