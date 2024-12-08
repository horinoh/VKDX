#pragma once

#include "resource.h"

#pragma region Code
#include "../DXExt.h"

class TriangleDX : public DXExt
{
private:
	using Super = DXExt;
public:
	TriangleDX() : Super() {}
	virtual ~TriangleDX() {}

protected:
	virtual void CreateGeometry() override {
		const auto CA = COM_PTR_GET(DirectCommandAllocators[0]);
		const auto DCL = COM_PTR_GET(DirectCommandLists[0]);
		const auto GCQ = COM_PTR_GET(GraphicsCommandQueue);

#if true
		constexpr std::array Vertices = {
			Vertex_PositionColor({.Position = { 0.0f, 0.5f, 0.0f }, .Color = { 1.0f, 0.0f, 0.0f, 1.0f } }), //!< CT
			Vertex_PositionColor({.Position = { -0.5f, -0.5f, 0.0f }, .Color = { 0.0f, 1.0f, 0.0f, 1.0f } }), //!< LB
			Vertex_PositionColor({.Position = { 0.5f, -0.5f, 0.0f }, .Color = { 0.0f, 0.0f, 1.0f, 1.0f } }), //!< RB
		};
#else
		//!< ピクセル指定
		constexpr FLOAT W = 1280.0f, H = 720.0f;
		constexpr std::array Vertices = {
			Vertex_PositionColor({.Position = { W * 0.5f, 100.0f, 0.0f }, .Color = { 1.0f, 0.0f, 0.0f, 1.0f } }), //!< CT
			Vertex_PositionColor({.Position = { W * 0.5f - 200.0f, H - 100.0f, 0.0f }, .Color = { 0.0f, 1.0f, 0.0f, 1.0f } }), //!< LB
			Vertex_PositionColor({.Position = { W * 0.5f + 200.0f, H - 100.0f, 0.0f }, .Color = { 0.0f, 0.0f, 1.0f, 1.0f } }), //!< RB
		};
#endif
		constexpr std::array Indices = { UINT32(0), UINT32(1), UINT32(2) };

		VertexBuffers.emplace_back().Create(COM_PTR_GET(Device), TotalSizeOf(Vertices), sizeof(Vertices[0]));
		UploadResource Upload_Vertex;
		Upload_Vertex.Create(COM_PTR_GET(Device), TotalSizeOf(Vertices), std::data(Vertices));
		SetName(COM_PTR_GET(VertexBuffers.back().Resource), TEXT("MyVertexBuffer"));

		IndexBuffers.emplace_back().Create(COM_PTR_GET(Device), TotalSizeOf(Indices), DXGI_FORMAT_R32_UINT);
		UploadResource Upload_Index;
		Upload_Index.Create(COM_PTR_GET(Device), TotalSizeOf(Indices), std::data(Indices));
		SetName(COM_PTR_GET(IndexBuffers.back().Resource), TEXT("MyIndexBuffer"));

		constexpr D3D12_DRAW_INDEXED_ARGUMENTS DIA = { .IndexCountPerInstance = static_cast<UINT32>(size(Indices)), .InstanceCount = 1, .StartIndexLocation = 0, .BaseVertexLocation = 0, .StartInstanceLocation = 0 };
		IndirectBuffers.emplace_back().Create(COM_PTR_GET(Device), DIA);
		UploadResource Upload_Indirect;
		Upload_Indirect.Create(COM_PTR_GET(Device), sizeof(DIA), &DIA);
		SetName(COM_PTR_GET(IndirectBuffers.back().Resource), TEXT("MyIndirectBuffer"));

		VERIFY_SUCCEEDED(DCL->Reset(CA, nullptr)); {
			VertexBuffers.back().PopulateCopyCommand(DCL, TotalSizeOf(Vertices), COM_PTR_GET(Upload_Vertex.Resource));
			IndexBuffers.back().PopulateCopyCommand(DCL, TotalSizeOf(Indices), COM_PTR_GET(Upload_Index.Resource));
			IndirectBuffers.back().PopulateCopyCommand(DCL, sizeof(DIA), COM_PTR_GET(Upload_Indirect.Resource));
		} VERIFY_SUCCEEDED(DCL->Close());
		DX::ExecuteAndWait(GCQ, DCL, COM_PTR_GET(GraphicsFence));

		LOG_OK();
	}
	virtual void CreateRootSignature() override {
		COM_PTR<ID3DBlob> Blob;
#ifdef USE_HLSL_ROOTSIGNATRUE
		GetRootSignaturePartFromShader(Blob, data(GetBasePath() + TEXT(".rs.cso")));
#else
#ifdef USE_ROOT_CONSTANTS
		DX::SerializeRootSignature(Blob, {
			D3D12_ROOT_PARAMETER({
				.ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS,
				.Constants = D3D12_ROOT_CONSTANTS({.ShaderRegister = 0, .RegisterSpace = 0, .Num32BitValues = static_cast<UINT>(size(Color)) }),
				.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL
			}),
		}, {}, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT | SHADER_ROOT_ACCESS_PS);
#else
		DX::SerializeRootSignature(Blob, {}, {}, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT | SHADER_ROOT_ACCESS_DENY_ALL);
#endif
#endif
		VERIFY_SUCCEEDED(Device->CreateRootSignature(0, Blob->GetBufferPointer(), Blob->GetBufferSize(), COM_PTR_UUIDOF_PUTVOID(RootSignatures.emplace_back())));
		LOG_OK();
	}
	virtual void CreatePipelineState() override {
		PipelineStates.emplace_back();

		std::vector<COM_PTR<ID3DBlob>> SBs;
#ifdef USE_ROOT_CONSTANTS
		VERIFY_SUCCEEDED(D3DReadFileToBlob(data(GetFilePath(".vs.cso").wstring()), COM_PTR_PUT(SBs.emplace_back())));
		VERIFY_SUCCEEDED(D3DReadFileToBlob(data(GetFilePath("_rc.ps.cso").wstring()), COM_PTR_PUT(SBs.emplace_back())));
#else
		VERIFY_SUCCEEDED(D3DReadFileToBlob(data(GetFilePath(".vs.cso").wstring()), COM_PTR_PUT(SBs.emplace_back())));
		{
#ifdef USE_SHADER_REFLECTION
			ProcessShaderReflection(COM_PTR_GET(SBs.back()));
#endif
#ifdef USE_SHADER_BLOB_PART
			SetBlobPart(SBs.back()); GetBlobPart(COM_PTR_GET(SBs.back())); StripShader(SBs.back());
#endif
		}
		VERIFY_SUCCEEDED(D3DReadFileToBlob(data(GetFilePath(".ps.cso").wstring()), COM_PTR_PUT(SBs.emplace_back())));
#endif
		const std::array SBCs = {
			D3D12_SHADER_BYTECODE({.pShaderBytecode = SBs[0]->GetBufferPointer(), .BytecodeLength = SBs[0]->GetBufferSize() }),
			D3D12_SHADER_BYTECODE({.pShaderBytecode = SBs[1]->GetBufferPointer(), .BytecodeLength = SBs[1]->GetBufferSize() }),
		};
		//!< スロット0にまとめて入れるインターリーブ、セマンティックス毎にスロットを分けると非インターリーブとなる
		//!< 詰まっている場合は offsetof() の代わりに D3D12_APPEND_ALIGNED_ELEMENT で良い (When directly after the previous one, we can use D3D12_APPEND_ALIGNED_ELEMENT)
		const std::vector IEDs = {
			D3D12_INPUT_ELEMENT_DESC({.SemanticName = "POSITION", .SemanticIndex = 0, .Format = DXGI_FORMAT_R32G32B32_FLOAT, .InputSlot = 0, .AlignedByteOffset = /*offsetof(Vertex_PositionColor, Position)*/D3D12_APPEND_ALIGNED_ELEMENT, .InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, .InstanceDataStepRate = 0 }),
			D3D12_INPUT_ELEMENT_DESC({.SemanticName = "COLOR", .SemanticIndex = 0, .Format = DXGI_FORMAT_R32G32B32A32_FLOAT, .InputSlot = 0, .AlignedByteOffset = /*offsetof(Vertex_PositionColor, Color)*/D3D12_APPEND_ALIGNED_ELEMENT, .InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, .InstanceDataStepRate = 0 }),
		};

		constexpr D3D12_RASTERIZER_DESC RD = {
			.FillMode = D3D12_FILL_MODE_SOLID,
			.CullMode = D3D12_CULL_MODE_BACK, .FrontCounterClockwise = TRUE,
			.DepthBias = D3D12_DEFAULT_DEPTH_BIAS, .DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP, .SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS,
			.DepthClipEnable = TRUE,
			.MultisampleEnable = FALSE, .AntialiasedLineEnable = FALSE, .ForcedSampleCount = 0,
			.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF
		};
		DXExt::CreatePipelineState_VsPs_Input(PipelineStates[0], COM_PTR_GET(RootSignatures[0]), D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE, RD, FALSE, IEDs, SBCs);

		for (auto& i : Threads) { i.join(); }
		Threads.clear();
	}
	virtual void PopulateBundleCommandList(const size_t i) override {
		const auto PS = COM_PTR_GET(PipelineStates[0]);
		const auto BCL = COM_PTR_GET(BundleCommandLists[i]);
		const auto BCA = COM_PTR_GET(BundleCommandAllocators[0]);

		VERIFY_SUCCEEDED(BCL->Reset(BCA, PS));
		{
			BCL->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

			const std::array VBVs = { VertexBuffers[0].View };
			BCL->IASetVertexBuffers(0, static_cast<UINT>(size(VBVs)), data(VBVs));
			BCL->IASetIndexBuffer(&IndexBuffers[0].View);

			BCL->ExecuteIndirect(COM_PTR_GET(IndirectBuffers[0].CommandSignature), 1, COM_PTR_GET(IndirectBuffers[0].Resource), 0, nullptr, 0);
		}
		VERIFY_SUCCEEDED(BCL->Close());
	}
	virtual void PopulateCommandList(const size_t i) override {
		const auto DCL = COM_PTR_GET(DirectCommandLists[i]);
		const auto DCA = COM_PTR_GET(DirectCommandAllocators[0]);
		const auto BCL = COM_PTR_GET(BundleCommandLists[i]);

		VERIFY_SUCCEEDED(DCL->Reset(DCA, nullptr));
		{
#ifdef USE_PIX
			PIXScopedEvent(DCL, PIX_COLOR(0, 255, 0), TEXT("Command Begin"));
#endif
			DCL->SetGraphicsRootSignature(COM_PTR_GET(RootSignatures[0]));
#ifdef USE_ROOT_CONSTANTS
			DCL->SetGraphicsRoot32BitConstants(0, static_cast<UINT>(size(Color)), data(Color), 0);
#endif

			DCL->RSSetViewports(static_cast<UINT>(size(Viewports)), data(Viewports));
			DCL->RSSetScissorRects(static_cast<UINT>(size(ScissorRects)), data(ScissorRects));

			const auto& RAH = SwapChain.ResourceAndHandles[i];

			const auto SCR = COM_PTR_GET(RAH.first);
			ResourceBarrier(DCL, SCR, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
			{
				constexpr std::array<D3D12_RECT, 0> Rects = {};
				DCL->ClearRenderTargetView(RAH.second, DirectX::Colors::SkyBlue, static_cast<UINT>(size(Rects)), data(Rects));

				const std::array CHs = { RAH.second };
				DCL->OMSetRenderTargets(static_cast<UINT>(std::size(CHs)), std::data(CHs), FALSE, nullptr);

				DCL->ExecuteBundle(BCL);
			}
			ResourceBarrier(DCL, SCR, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
		}
		VERIFY_SUCCEEDED(DCL->Close());
	}

#ifdef USE_ROOT_CONSTANTS
	const std::array<FLOAT, 4> Color = { 1.0f, 0.0f, 0.0f, 1.0f };
#endif
};
#pragma endregion