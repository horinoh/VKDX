#pragma once

#include "resource.h"

#pragma region Code
#include "../FBX.h"
#include "../DXExt.h"

class FbxDX : public DXExt, public Fbx
{
private:
	using Super = DXExt;
public:
#pragma region FBX
	DirectX::XMFLOAT3 ToFloat3(const FbxVector4& rhs) { return DirectX::XMFLOAT3(static_cast<FLOAT>(rhs[0]), static_cast<FLOAT>(rhs[1]), static_cast<FLOAT>(rhs[2])); }
	std::vector<UINT32> Indices;
	std::vector<DirectX::XMFLOAT3> Vertices;
	std::vector<DirectX::XMFLOAT3> Normals;
	virtual void Process(FbxMesh* Mesh) override {
		Fbx::Process(Mesh);

		auto Max = DirectX::XMFLOAT3((std::numeric_limits<float>::min)(), (std::numeric_limits<float>::min)(), (std::numeric_limits<float>::min)());
		auto Min = DirectX::XMFLOAT3((std::numeric_limits<float>::max)(), (std::numeric_limits<float>::max)(), (std::numeric_limits<float>::max)());
		std::cout << "PolygonCount = " << Mesh->GetPolygonCount() << std::endl;
		for (auto i = 0; i < Mesh->GetPolygonCount(); ++i) {
			for (auto j = 0; j < Mesh->GetPolygonSize(i); ++j) {
				Indices.emplace_back(i * Mesh->GetPolygonSize(i) + j);

				Vertices.emplace_back(ToFloat3(Mesh->GetControlPoints()[Mesh->GetPolygonVertex(i, j)]));
				Max.x = std::max(Max.x, Vertices.back().x);
				Max.y = std::max(Max.y, Vertices.back().y);
				Max.z = std::max(Max.z, Vertices.back().z);
				Min.x = std::min(Min.x, Vertices.back().x);
				Min.y = std::min(Min.y, Vertices.back().y);
				Min.z = std::min(Min.z, Vertices.back().z);

				for (auto k = 0; k < Mesh->GetElementNormalCount(); ++k) {
					Normals.emplace_back(ToFloat3(Mesh->GetElementNormal(k)->GetDirectArray().GetAt(i)));
				}
			}
		}
		const auto Bound = std::max(std::max(Max.x - Min.x, Max.y - Min.y), Max.z - Min.z) * 1.0f;
		std::transform(begin(Vertices), end(Vertices), begin(Vertices), [&](const DirectX::XMFLOAT3& rhs) { return DirectX::XMFLOAT3(rhs.x / Bound, (rhs.y - (Max.y - Min.y) * 0.5f)/ Bound, (rhs.z - Min.z) / Bound); });

		//for (auto i : Vertices) { std::cout << i.x << ", " << i.y << ", " << i.z << std::endl; }
	}
#pragma endregion

	FbxDX() : Super() {}
	virtual ~FbxDX() {}
	virtual void CreateGeometry() override {
		std::wstring Path;
		if (FindDirectory("FBX", Path)) {
			Load(ToString(Path) + "//bunny.FBX");
			//Load(ToString(Path) + "//dragon.FBX");
		}
		//Load(GetEnv("FBX_SDK_PATH") + "\\samples\\ConvertScene\\box.fbx");
		//Load(GetEnv("FBX_SDK_PATH") + "\\samples\\ViewScene\\humanoid.fbx"); 

		const auto CA = COM_PTR_GET(CommandAllocators[0]);
		const auto GCL = COM_PTR_GET(GraphicsCommandLists[0]);
		const auto CQ = COM_PTR_GET(GraphicsCommandQueue);

		VertexBuffers.emplace_back().Create(COM_PTR_GET(Device), Sizeof(Vertices), sizeof(Vertices[0]));
		UploadResource Upload_Vertex;
		Upload_Vertex.Create(COM_PTR_GET(Device), Sizeof(Vertices), data(Vertices));

		VertexBuffers.emplace_back().Create(COM_PTR_GET(Device), Sizeof(Normals), sizeof(Normals[0]));
		UploadResource Upload_Normal;
		Upload_Normal.Create(COM_PTR_GET(Device), Sizeof(Normals), data(Normals));

		IndexBuffers.emplace_back().Create(COM_PTR_GET(Device), Sizeof(Indices), DXGI_FORMAT_R32_UINT);
		UploadResource Upload_Index;
		Upload_Index.Create(COM_PTR_GET(Device), Sizeof(Indices), data(Indices));

		const D3D12_DRAW_INDEXED_ARGUMENTS DIA = { .IndexCountPerInstance = static_cast<UINT32>(size(Indices)), .InstanceCount = 1, .StartIndexLocation = 0, .BaseVertexLocation = 0, .StartInstanceLocation = 0 };
		IndirectBuffers.emplace_back().Create(COM_PTR_GET(Device), DIA);
		UploadResource Upload_Indirect;
		Upload_Indirect.Create(COM_PTR_GET(Device), sizeof(DIA), &DIA);

		VERIFY_SUCCEEDED(GCL->Reset(CA, nullptr)); {
			VertexBuffers[0].PopulateCopyCommand(GCL, Sizeof(Vertices), COM_PTR_GET(Upload_Vertex.Resource));
			VertexBuffers[1].PopulateCopyCommand(GCL, Sizeof(Normals), COM_PTR_GET(Upload_Vertex.Resource));
			IndexBuffers.back().PopulateCopyCommand(GCL, Sizeof(Indices), COM_PTR_GET(Upload_Index.Resource));
			IndirectBuffers.back().PopulateCopyCommand(GCL, sizeof(DIA), COM_PTR_GET(Upload_Indirect.Resource));
		} VERIFY_SUCCEEDED(GCL->Close());
		DX::ExecuteAndWait(CQ, GCL, COM_PTR_GET(Fence));
	}
	virtual void CreateRootSignature() override {
		COM_PTR<ID3DBlob> Blob;
		DX::SerializeRootSignature(Blob, {}, {}, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT | SHADER_ROOT_ACCESS_DENY_ALL);
		VERIFY_SUCCEEDED(Device->CreateRootSignature(0, Blob->GetBufferPointer(), Blob->GetBufferSize(), COM_PTR_UUIDOF_PUTVOID(RootSignatures.emplace_back())));
		LOG_OK();
	}
	virtual void CreatePipelineState() override {
		const auto ShaderPath = GetBasePath();
		std::vector<COM_PTR<ID3DBlob>> SBs;
		VERIFY_SUCCEEDED(D3DReadFileToBlob(data(ShaderPath + TEXT(".vs.cso")), COM_PTR_PUT(SBs.emplace_back())));
		VERIFY_SUCCEEDED(D3DReadFileToBlob(data(ShaderPath + TEXT(".ps.cso")), COM_PTR_PUT(SBs.emplace_back())));
		const std::array SBCs = {
			D3D12_SHADER_BYTECODE({.pShaderBytecode = SBs[0]->GetBufferPointer(), .BytecodeLength = SBs[0]->GetBufferSize() }),
			D3D12_SHADER_BYTECODE({.pShaderBytecode = SBs[1]->GetBufferPointer(), .BytecodeLength = SBs[1]->GetBufferSize() }),
		};
		const std::vector IEDs = {
			D3D12_INPUT_ELEMENT_DESC({.SemanticName = "POSITION", .SemanticIndex = 0, .Format = DXGI_FORMAT_R32G32B32_FLOAT, .InputSlot = 0, .AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT, .InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, .InstanceDataStepRate = 0 }),
			D3D12_INPUT_ELEMENT_DESC({.SemanticName = "NORMAL", .SemanticIndex = 0, .Format = DXGI_FORMAT_R32G32B32_FLOAT, .InputSlot = 1, .AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT, .InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, .InstanceDataStepRate = 0 }),
		};

		constexpr D3D12_RASTERIZER_DESC RD = {
			.FillMode = D3D12_FILL_MODE_WIREFRAME,
			//.FillMode = D3D12_FILL_MODE_SOLID,
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
			BGCL->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

			const std::array VBVs = { VertexBuffers[0].View, VertexBuffers[1].View };
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
			GCL->SetGraphicsRootSignature(COM_PTR_GET(RootSignatures[0]));

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
};
#pragma endregion