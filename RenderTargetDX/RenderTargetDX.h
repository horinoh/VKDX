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
		const auto BCA = COM_PTR_GET(BundleCommandAllocators[0]);
		DXGI_SWAP_CHAIN_DESC1 SCD;
		SwapChain.DxSwapChain->GetDesc1(&SCD);
		for (UINT i = 0; i < SCD.BufferCount; ++i) {
			VERIFY_SUCCEEDED(Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_BUNDLE, BCA, nullptr, COM_PTR_UUIDOF_PUTVOID(BundleCommandLists.emplace_back())));
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
		IndirectBuffers.emplace_back().Create(COM_PTR_GET(Device), DIA);
		UploadResource Upload0;
		Upload0.Create(COM_PTR_GET(Device), sizeof(DIA), &DIA);
#pragma endregion

#pragma region PASS1
		//!< フルスクリーン描画用
		constexpr D3D12_DRAW_ARGUMENTS DA = { .VertexCountPerInstance = 4, .InstanceCount = 1, .StartVertexLocation = 0, .StartInstanceLocation = 0 };
		IndirectBuffers.emplace_back().Create(COM_PTR_GET(Device), DA);
		UploadResource Upload1;
		Upload1.Create(COM_PTR_GET(Device), sizeof(DA), &DA);
#pragma endregion

		VERIFY_SUCCEEDED(CL->Reset(CA, nullptr)); {
			IndirectBuffers[0].PopulateCopyCommand(CL, sizeof(DIA), COM_PTR_GET(Upload0.Resource));
			IndirectBuffers[1].PopulateCopyCommand(CL, sizeof(DA), COM_PTR_GET(Upload1.Resource));
		} VERIFY_SUCCEEDED(CL->Close());
		DX::ExecuteAndWait(GCQ, CL, COM_PTR_GET(GraphicsFence));
	}
	virtual void CreateTexture() {
		CreateTexture_Render();
#ifdef USE_DEPTH
		Super::CreateTexture();
#endif
	}
	virtual void CreateStaticSampler() override {
#pragma region PASS1
		CreateStaticSampler_LW(0, 0, D3D12_SHADER_VISIBILITY_PIXEL);
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
				D3D12_DESCRIPTOR_RANGE1({.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV, .NumDescriptors = 1, .BaseShaderRegister = 0, .RegisterSpace = 0, .Flags = D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE,.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND })
			};
			DX::SerializeRootSignature(Blob, {
				D3D12_ROOT_PARAMETER1({
					.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE, 
					.DescriptorTable = D3D12_ROOT_DESCRIPTOR_TABLE1({.NumDescriptorRanges = static_cast<uint32_t>(size(DRs_Srv)), .pDescriptorRanges = data(DRs_Srv) }), 
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
		PipelineStates.emplace_back();
		PipelineStates.emplace_back();

#ifdef USE_PIPELINE_SERIALIZE
		PipelineLibrarySerializer PLS(COM_PTR_GET(Device), GetFilePath(".plo"));
#endif
		constexpr D3D12_RASTERIZER_DESC RD = {
			.FillMode = D3D12_FILL_MODE_SOLID,
			.CullMode = D3D12_CULL_MODE_BACK, .FrontCounterClockwise = TRUE,
			.DepthBias = D3D12_DEFAULT_DEPTH_BIAS, .DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP, .SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS,
			.DepthClipEnable = TRUE,
			.MultisampleEnable = FALSE, .AntialiasedLineEnable = FALSE, .ForcedSampleCount = 0,
			.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF
		};
#pragma region PASS0
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
		CreatePipelineState_VsPsDsHsGs(PipelineStates[0], COM_PTR_GET(RootSignatures[0]), D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH, RD, TRUE, SBCs0);
#pragma endregion

#pragma region PASS1
		std::vector<COM_PTR<ID3DBlob>> SBs1;
		VERIFY_SUCCEEDED(D3DReadFileToBlob(data(GetFilePath("_1.vs.cso").wstring()), COM_PTR_PUT(SBs1.emplace_back())));
		VERIFY_SUCCEEDED(D3DReadFileToBlob(data(GetFilePath("_1.ps.cso").wstring()), COM_PTR_PUT(SBs1.emplace_back())));
		const std::array SBCs1 = {
			D3D12_SHADER_BYTECODE({.pShaderBytecode = SBs1[0]->GetBufferPointer(), .BytecodeLength = SBs1[0]->GetBufferSize() }),
			D3D12_SHADER_BYTECODE({.pShaderBytecode = SBs1[1]->GetBufferPointer(), .BytecodeLength = SBs1[1]->GetBufferSize() }),
		};
		CreatePipelineState_VsPs(PipelineStates[1], COM_PTR_GET(RootSignatures[1]), D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE, RD, FALSE, SBCs1);
#pragma endregion
		
		for (auto& i : Threads) { i.join(); }
		Threads.clear();
	}
	virtual void CreateDescriptor() override {
		auto& DescCBV = CbvSrvUavDescs.emplace_back();
		auto& HeapCBV = DescCBV.first;
		auto& HandleCBV = DescCBV.second;

		auto& DescRTV = RtvDescs.emplace_back();
		auto& HeapRTV = DescRTV.first;
		auto& HandleRTV = DescRTV.second;

#pragma region PASS0
		{
			const D3D12_DESCRIPTOR_HEAP_DESC DHD = {.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV, .NumDescriptors = 1, .Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE, .NodeMask = 0 };
			VERIFY_SUCCEEDED(Device->CreateDescriptorHeap(&DHD, COM_PTR_UUIDOF_PUTVOID(HeapRTV)));
		}
#pragma endregion

#pragma region PASS1
		{
			const D3D12_DESCRIPTOR_HEAP_DESC DHD = {.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, .NumDescriptors = 1, .Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE, .NodeMask = 0 };
			VERIFY_SUCCEEDED(Device->CreateDescriptorHeap(&DHD, COM_PTR_UUIDOF_PUTVOID(HeapCBV)));
		}
#pragma endregion

#pragma region PASS0
		//!< RTV
		{
			auto CDH = HeapRTV->GetCPUDescriptorHandleForHeapStart();
			Device->CreateRenderTargetView(COM_PTR_GET(RenderTextures.back().Resource), &RenderTextures.back().RTV, CDH);
			HandleRTV.emplace_back(CDH);
		}
#ifdef USE_DEPTH
		//!< DSV
		Super::CreateDescriptor();
#endif
#pragma endregion

#pragma region PASS1
		//!< SRV
		{
			auto CDH = HeapCBV->GetCPUDescriptorHandleForHeapStart();
			auto GDH = HeapCBV->GetGPUDescriptorHandleForHeapStart();
			Device->CreateShaderResourceView(COM_PTR_GET(RenderTextures.back().Resource), &RenderTextures.back().SRV, CDH);
			HandleCBV.emplace_back(GDH);
		}
#pragma endregion
	}
	void PopulateBundleCommandList_Pass0(const size_t i) {
		const auto BCA = COM_PTR_GET(BundleCommandAllocators[0]);
		//!< バンドルコマンドリスト(メッシュ描画用)
		const auto PS = COM_PTR_GET(PipelineStates[0]);
		const auto BCL = COM_PTR_GET(BundleCommandLists[i]);
		VERIFY_SUCCEEDED(BCL->Reset(BCA, PS));
		{
			BCL->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_1_CONTROL_POINT_PATCHLIST);
			BCL->ExecuteIndirect(COM_PTR_GET(IndirectBuffers[0].CommandSignature), 1, COM_PTR_GET(IndirectBuffers[0].Resource), 0, nullptr, 0);
		}
		VERIFY_SUCCEEDED(BCL->Close());
	}
	void PopulateBundleCommandList_Pass1(const size_t i) {
		const auto BCA = COM_PTR_GET(BundleCommandAllocators[0]);
		//!< レンダーテクスチャ描画用
		const auto PS = COM_PTR_GET(PipelineStates[1]);
		DXGI_SWAP_CHAIN_DESC1 SCD;
		SwapChain.DxSwapChain->GetDesc1(&SCD);
		const auto BCL = COM_PTR_GET(BundleCommandLists[i + SCD.BufferCount]); //!< オフセットさせる(ここでは2つのバンドルコマンドリストがぞれぞれスワップチェインイメージ数だけある)
		VERIFY_SUCCEEDED(BCL->Reset(BCA, PS));
		{
			BCL->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
			BCL->ExecuteIndirect(COM_PTR_GET(IndirectBuffers[1].CommandSignature), 1, COM_PTR_GET(IndirectBuffers[1].Resource), 0, nullptr, 0);
		}
		VERIFY_SUCCEEDED(BCL->Close());
	}
	virtual void PopulateBundleCommandList(const size_t i) override {
#pragma region PASS0
		PopulateBundleCommandList_Pass0(i);
#pragma endregion
#pragma region PASS1
		PopulateBundleCommandList_Pass1(i);
#pragma endregion
	}
	virtual void PopulateCommandList(const size_t i) override {
		const auto BCL0 = COM_PTR_GET(BundleCommandLists[i]);
		DXGI_SWAP_CHAIN_DESC1 SCD;
		SwapChain.DxSwapChain->GetDesc1(&SCD);
		const auto BCL1 = COM_PTR_GET(BundleCommandLists[i + SCD.BufferCount]); 

		const auto DCL = COM_PTR_GET(DirectCommandLists[i]);
		const auto DCA = COM_PTR_GET(DirectCommandAllocators[0]);
		VERIFY_SUCCEEDED(DCL->Reset(DCA, nullptr));
		{
			const auto& RAH = SwapChain.ResourceAndHandles[i];

			const auto SCR = COM_PTR_GET(RAH.first);
			const auto RT = COM_PTR_GET(RenderTextures.back().Resource);

			DCL->RSSetViewports(static_cast<UINT>(std::size(Viewports)), std::data(Viewports));
			DCL->RSSetScissorRects(static_cast<UINT>(std::size(ScissorRects)), std::data(ScissorRects));

#pragma region PASS0
			//!< メッシュ描画用バンドルコマンドリストを発行
			{
				const auto& HandleRTV = RtvDescs[0].second;
				const auto& HandleDSV = DsvDescs[0].second;

				DCL->SetGraphicsRootSignature(COM_PTR_GET(RootSignatures[0]));

				constexpr std::array<D3D12_RECT, 0> Rects = {};
				DCL->ClearRenderTargetView(HandleRTV[0], DirectX::Colors::SkyBlue, static_cast<UINT>(std::size(Rects)), std::data(Rects));
#ifdef USE_DEPTH
				DCL->ClearDepthStencilView(HandleDSV[0], D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, static_cast<UINT>(std::size(Rects)), std::data(Rects));
#endif

				const std::array RTCHs = { HandleRTV[0] };
#ifdef USE_DEPTH
				DCL->OMSetRenderTargets(static_cast<UINT>(std::size(RTCHs)), std::data(RTCHs), FALSE, &HandleDSV[0]);
#else			
				DCL->OMSetRenderTargets(static_cast<UINT>(std::size(RTCHs)), std::data(RTCHs), FALSE, nullptr);
#endif

				DCL->ExecuteBundle(BCL0);
			}
#pragma endregion

			//!< リソースバリア
			//!<	スワップチェインをレンダーターゲットへ、レンダーテクスチャをシェーダリソースへ
			ResourceBarrier2(DCL,
				SCR, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET,
				RT, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

#pragma region PASS1
			//!< レンダーテクスチャ描画用バンドルコマンドリストを発行
			{
				const auto& Desc = CbvSrvUavDescs[0];
				const auto& Heap = Desc.first;
				const auto& Handle = Desc.second;

				DCL->SetGraphicsRootSignature(COM_PTR_GET(RootSignatures[1]));

				const std::array CHs = { RAH.second };
				DCL->OMSetRenderTargets(static_cast<UINT>(std::size(CHs)), std::data(CHs), FALSE, nullptr);

				const std::array DHs = { COM_PTR_GET(Heap) };
				DCL->SetDescriptorHeaps(static_cast<UINT>(std::size(DHs)), std::data(DHs));
				//!< SRV
				DCL->SetGraphicsRootDescriptorTable(0, Handle[0]); 

				DCL->ExecuteBundle(BCL1);
			}

			//!< リソースバリア
			//!<	スワップチェインをプレゼントへ、レンダーテクスチャをレンダーターゲットへ
			ResourceBarrier2(DCL,
				SCR, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT,
				RT, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET);
#pragma endregion
		}
		VERIFY_SUCCEEDED(DCL->Close());
	}
};
#pragma endregion