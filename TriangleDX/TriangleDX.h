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
#define COMMAND_COPY_TOGETHER

		const auto CA = COM_PTR_GET(CommandAllocators[0]);
		const auto GCL = COM_PTR_GET(GraphicsCommandLists[0]);
		const auto CQ = COM_PTR_GET(GraphicsCommandQueue);

#if 1
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
		constexpr std::array<UINT32, 3> Indices = { 0, 1, 2 };

#ifdef COMMAND_COPY_TOGETHER
		VertexBuffers.emplace_back().Create(COM_PTR_GET(Device), sizeof(Vertices), sizeof(Vertices[0]));
		UploadResource Upload_Vertex;
		Upload_Vertex.Create(COM_PTR_GET(Device), sizeof(Vertices), data(Vertices));
#else
		VertexBuffers.emplace_back().Create(COM_PTR_GET(Device), sizeof(Vertices), sizeof(Vertices[0])).ExecuteCopyCommand(COM_PTR_GET(Device), CA, GCL, CQ, COM_PTR_GET(Fence), sizeof(Vertices), data(Vertices));
#endif
		SetName(COM_PTR_GET(VertexBuffers.back().Resource), TEXT("MyVertexBuffer"));

#ifdef COMMAND_COPY_TOGETHER
		IndexBuffers.emplace_back().Create(COM_PTR_GET(Device), sizeof(Indices), DXGI_FORMAT_R32_UINT);
		UploadResource Upload_Index;
		Upload_Index.Create(COM_PTR_GET(Device), sizeof(Indices), data(Indices));
#else
		IndexBuffers.emplace_back().Create(COM_PTR_GET(Device), sizeof(Indices), DXGI_FORMAT_R32_UINT).ExecuteCopyCommand(COM_PTR_GET(Device), CA, GCL, CQ, COM_PTR_GET(Fence), sizeof(Indices), data(Indices));
#endif	
		SetName(COM_PTR_GET(IndexBuffers.back().Resource), TEXT("MyIndexBuffer"));

		constexpr D3D12_DRAW_INDEXED_ARGUMENTS DIA = { .IndexCountPerInstance = static_cast<UINT32>(size(Indices)), .InstanceCount = 1, .StartIndexLocation = 0, .BaseVertexLocation = 0, .StartInstanceLocation = 0 };
#ifdef COMMAND_COPY_TOGETHER
		IndirectBuffers.emplace_back().Create(COM_PTR_GET(Device), DIA);
		UploadResource Upload_Indirect;
		Upload_Indirect.Create(COM_PTR_GET(Device), sizeof(DIA), &DIA);
#else
		IndirectBuffers.emplace_back().Create(COM_PTR_GET(Device), DIA).ExecuteCopyCommand(COM_PTR_GET(Device), CA, GCL, CQ, COM_PTR_GET(Fence), sizeof(DIA), &DIA);
#endif	
		SetName(COM_PTR_GET(IndirectBuffers.back().Resource), TEXT("MyIndirectBuffer"));

#ifdef COMMAND_COPY_TOGETHER
		VERIFY_SUCCEEDED(GCL->Reset(CA, nullptr)); {
			VertexBuffers.back().PopulateCopyCommand(GCL, sizeof(Vertices), COM_PTR_GET(Upload_Vertex.Resource));
			IndexBuffers.back().PopulateCopyCommand(GCL, sizeof(Indices), COM_PTR_GET(Upload_Index.Resource));
			IndirectBuffers.back().PopulateCopyCommand(GCL, sizeof(DIA), COM_PTR_GET(Upload_Indirect.Resource));
		} VERIFY_SUCCEEDED(GCL->Close());
		DX::ExecuteAndWait(CQ, GCL, COM_PTR_GET(Fence));
#endif

		LOG_OK();
	}
	virtual void CreateRootSignature() override {
		COM_PTR<ID3DBlob> Blob;
#ifdef USE_HLSL_ROOTSIGNATRUE
		GetRootSignaturePartFromShader(Blob, data(GetBasePath() + TEXT(".rs.cso")));
#else
		DX::SerializeRootSignature(Blob, {
#ifdef USE_ROOT_CONSTANTS
			D3D12_ROOT_PARAMETER({
				.ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS, 
				.Constants = D3D12_ROOT_CONSTANTS({.ShaderRegister = 0, .RegisterSpace = 0, .Num32BitValues = static_cast<UINT>(size(Color)) }),
				.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL
			}),
#endif
			}, {}, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT
#ifdef USE_ROOT_CONSTANTS
			| SHADER_ROOT_ACCESS_PS
#else
			| SHADER_ROOT_ACCESS_DENY_ALL
#endif
		); 
#endif
		VERIFY_SUCCEEDED(Device->CreateRootSignature(0, Blob->GetBufferPointer(), Blob->GetBufferSize(), COM_PTR_UUIDOF_PUTVOID(RootSignatures.emplace_back())));
		LOG_OK();
	}
	virtual void CreatePipelineState() override {
		const auto ShaderPath = GetBasePath();
		std::vector<COM_PTR<ID3DBlob>> SBs;
#ifdef USE_ROOT_CONSTANTS
		VERIFY_SUCCEEDED(D3DReadFileToBlob(data(ShaderPath + TEXT(".vs.cso")), COM_PTR_PUT(SBs.emplace_back())));
		VERIFY_SUCCEEDED(D3DReadFileToBlob(data(ShaderPath + TEXT("_rc.ps.cso")), COM_PTR_PUT(SBs.emplace_back())));
#else
		VERIFY_SUCCEEDED(D3DReadFileToBlob(data(ShaderPath + TEXT(".vs.cso")), COM_PTR_PUT(SBs.emplace_back())));
		{
#ifdef USE_SHADER_REFLECTION
			ProcessShaderReflection(COM_PTR_GET(SBs.back()));
#endif
#ifdef USE_SHADER_BLOB_PART
			SetBlobPart(SBs.back()); GetBlobPart(COM_PTR_GET(SBs.back())); StripShader(SBs.back());
#endif
		}

		VERIFY_SUCCEEDED(D3DReadFileToBlob(data(ShaderPath + TEXT(".ps.cso")), COM_PTR_PUT(SBs.emplace_back())));
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
		DXExt::CreatePipelineState_VsPs_Input(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE, RD, FALSE, IEDs, SBCs);
	}

	virtual void PopulateCommandList(const size_t i) override {
		const auto PS = COM_PTR_GET(PipelineStates[0]);

#pragma region BUNDLE_COMMAND_LIST
		const auto BGCL = COM_PTR_GET(BundleGraphicsCommandLists[i]);
		const auto BCA = COM_PTR_GET(BundleCommandAllocators[0]);
		VERIFY_SUCCEEDED(BGCL->Reset(BCA, PS));
		{
			BGCL->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

			const std::array VBVs = { VertexBuffers[0].View };
			BGCL->IASetVertexBuffers(0, static_cast<UINT>(size(VBVs)), data(VBVs));
			BGCL->IASetIndexBuffer(&IndexBuffers[0].View);

			BGCL->ExecuteIndirect(COM_PTR_GET(IndirectBuffers[0].CommandSignature), 1, COM_PTR_GET(IndirectBuffers[0].Resource), 0, nullptr, 0);
		}
		VERIFY_SUCCEEDED(BGCL->Close());
#pragma endregion

		const auto GCL = COM_PTR_GET(GraphicsCommandLists[i]);
		const auto CA = COM_PTR_GET(CommandAllocators[0]);
		VERIFY_SUCCEEDED(GCL->Reset(CA, PS));
		{
#if defined(_DEBUG) || defined(USE_PIX)
			PIXScopedEvent(GCL, PIX_COLOR(0, 255, 0), TEXT("Command Begin"));
#endif
			GCL->SetGraphicsRootSignature(COM_PTR_GET(RootSignatures[0]));
#ifdef USE_ROOT_CONSTANTS
			GCL->SetGraphicsRoot32BitConstants(0, static_cast<UINT>(size(Color)), data(Color), 0);
#endif

			GCL->RSSetViewports(static_cast<UINT>(size(Viewports)), data(Viewports));
			GCL->RSSetScissorRects(static_cast<UINT>(size(ScissorRects)), data(ScissorRects));

			const auto SCR = COM_PTR_GET(SwapChainResources[i]);
			ResourceBarrier(GCL, SCR, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
			{
				auto SCCDH = SwapChainDescriptorHeap->GetCPUDescriptorHandleForHeapStart(); SCCDH.ptr += i * Device->GetDescriptorHandleIncrementSize(SwapChainDescriptorHeap->GetDesc().Type);

				constexpr std::array<D3D12_RECT, 0> Rects = {};
				GCL->ClearRenderTargetView(SCCDH, DirectX::Colors::SkyBlue, static_cast<UINT>(size(Rects)), data(Rects));

				const std::array RTCDHs = { SCCDH };
				GCL->OMSetRenderTargets(static_cast<UINT>(size(RTCDHs)), data(RTCDHs), FALSE, nullptr);

				GCL->ExecuteBundle(BGCL);
			}
			ResourceBarrier(GCL, SCR, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
		}
		VERIFY_SUCCEEDED(GCL->Close());
	}

#ifdef USE_ROOT_CONSTANTS
	const std::array<FLOAT, 4> Color = { 1.0f, 0.0f, 0.0f, 1.0f };
#endif
};
#pragma endregion