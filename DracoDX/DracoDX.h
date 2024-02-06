#pragma once

#include "resource.h"

#pragma region Code
#include "../DRACO.h"
#include "../DXExt.h"

#ifdef USE_CONVEXHULL
#include "../Colli.h"
#endif

class DracoDX : public DXExtDepth, public Draco 
{
private:
	using Super = DXExtDepth;
public:
	std::vector<UINT32> Indices;
	std::vector<DirectX::XMFLOAT3> Vertices;
	std::vector<DirectX::XMFLOAT3> Normals;
#ifdef USE_CONVEXHULL
	std::vector<UINT32> IndicesCH;
	std::vector<DirectX::XMFLOAT3> VerticesCH; 
#endif

#pragma region DRACO
	virtual void Process(const draco::Mesh* Mesh) override {
		Draco::Process(Mesh);
		if (nullptr != Mesh) {
#pragma region POSITION
			const auto POSITION = Mesh->GetNamedAttribute(draco::GeometryAttribute::POSITION);
			if (nullptr != POSITION) {
				for (auto i = 0; i < POSITION->size(); ++i) {
					POSITION->ConvertValue(static_cast<draco::AttributeValueIndex>(i), &Vertices.emplace_back().x);
				}
				AdjustScale(Vertices, 1.0f);
			}
#pragma endregion

#pragma region NORMAL
			const auto NORMAL = Mesh->GetNamedAttribute(draco::GeometryAttribute::NORMAL);
			if (nullptr != NORMAL) {
				for (auto i = 0; i < NORMAL->size(); ++i) {
					NORMAL->ConvertValue(static_cast<draco::AttributeValueIndex>(i), &Normals.emplace_back().x);
				}
			}
#pragma endregion

#pragma region INDEX
			for (uint32_t i = 0; i < Mesh->num_faces(); ++i) {
				const auto Face = Mesh->face(static_cast<draco::FaceIndex>(i));
				for (auto j = 0; j < 3; ++j) {
					Indices.emplace_back(POSITION->mapped_index(Face[j]).value());
				}
			}
#pragma endregion
		}
	}
#pragma endregion

	DracoDX() : Super() {}
	virtual ~DracoDX() {}
	virtual void CreateGeometry() override {
#ifdef USE_CONVEXHULL
		Load(DRC_PATH / "bunny4.drc");
		//Load(DRC_PATH / "dragon4.drc");
#else
		Load(DRC_PATH / "bunny.drc");
		//Load(DRC_PATH / "dragon.drc");
		//Load(DRC_SAMPLE_PATH / "car.drc");
		//Load(DRC_SAMPLE_PATH / "bunny_gltf.drc");
#endif

#ifdef USE_CONVEXHULL
		std::vector<Vec3> VerticesVec3; 
		VerticesVec3.reserve(size(Vertices));
		for (auto& i : Vertices) { VerticesVec3.emplace_back(Vec3({ i.x, i.y, i.z })); }

		std::vector<Vec3> HullVertices;
		std::vector<TriangleIndices> HullIndices;
		BuildConvexHull(VerticesVec3, HullVertices, HullIndices);
		{
			for (auto& i : HullVertices) {
				VerticesCH.emplace_back(DirectX::XMFLOAT3(i.X(), i.Y(), i.Z()));
			}
			for (auto i : HullIndices) {
				IndicesCH.emplace_back(static_cast<UINT32>(std::get<1>(i)));
				IndicesCH.emplace_back(static_cast<UINT32>(std::get<0>(i)));
				IndicesCH.emplace_back(static_cast<UINT32>(std::get<2>(i)));
			}
		}
#endif

		const auto CA = COM_PTR_GET(DirectCommandAllocators[0]);
		const auto CL = COM_PTR_GET(DirectCommandLists[0]);
		const auto CQ = COM_PTR_GET(GraphicsCommandQueue);

		VertexBuffers.emplace_back().Create(COM_PTR_GET(Device), TotalSizeOf(Vertices), sizeof(Vertices[0]));
		UploadResource Upload_Vertex;
		Upload_Vertex.Create(COM_PTR_GET(Device), TotalSizeOf(Vertices), data(Vertices));

		VertexBuffers.emplace_back().Create(COM_PTR_GET(Device), TotalSizeOf(Normals), sizeof(Normals[0]));
		UploadResource Upload_Normal;
		Upload_Normal.Create(COM_PTR_GET(Device), TotalSizeOf(Normals), data(Normals));

		IndexBuffers.emplace_back().Create(COM_PTR_GET(Device), TotalSizeOf(Indices), DXGI_FORMAT_R32_UINT);
		UploadResource Upload_Index;
		Upload_Index.Create(COM_PTR_GET(Device), TotalSizeOf(Indices), data(Indices));

		const D3D12_DRAW_INDEXED_ARGUMENTS DIA = { .IndexCountPerInstance = static_cast<UINT32>(size(Indices)), .InstanceCount = 1, .StartIndexLocation = 0, .BaseVertexLocation = 0, .StartInstanceLocation = 0 };
		IndirectBuffers.emplace_back().Create(COM_PTR_GET(Device), DIA);
		UploadResource Upload_Indirect;
		Upload_Indirect.Create(COM_PTR_GET(Device), sizeof(DIA), &DIA);

#ifdef USE_CONVEXHULL
		VertexBuffers.emplace_back().Create(COM_PTR_GET(Device), TotalSizeOf(VerticesCH), sizeof(VerticesCH[0]));
		UploadResource Upload_VertexCH;
		Upload_VertexCH.Create(COM_PTR_GET(Device), TotalSizeOf(VerticesCH), data(VerticesCH));

		IndexBuffers.emplace_back().Create(COM_PTR_GET(Device), TotalSizeOf(IndicesCH), DXGI_FORMAT_R32_UINT);
		UploadResource Upload_IndexCH;
		Upload_IndexCH.Create(COM_PTR_GET(Device), TotalSizeOf(IndicesCH), data(IndicesCH));

		const D3D12_DRAW_INDEXED_ARGUMENTS DIA_CH = { .IndexCountPerInstance = static_cast<UINT32>(size(IndicesCH)), .InstanceCount = 1, .StartIndexLocation = 0, .BaseVertexLocation = 0, .StartInstanceLocation = 0 };
		IndirectBuffers.emplace_back().Create(COM_PTR_GET(Device), DIA_CH);
		UploadResource Upload_IndirectCH;
		Upload_IndirectCH.Create(COM_PTR_GET(Device), sizeof(DIA_CH), &DIA_CH);
#endif

		VERIFY_SUCCEEDED(CL->Reset(CA, nullptr)); {
			VertexBuffers[0].PopulateCopyCommand(CL, TotalSizeOf(Vertices), COM_PTR_GET(Upload_Vertex.Resource));
			VertexBuffers[1].PopulateCopyCommand(CL, TotalSizeOf(Normals), COM_PTR_GET(Upload_Normal.Resource));
			IndexBuffers[0].PopulateCopyCommand(CL, TotalSizeOf(Indices), COM_PTR_GET(Upload_Index.Resource));
			IndirectBuffers[0].PopulateCopyCommand(CL, sizeof(DIA), COM_PTR_GET(Upload_Indirect.Resource));
#ifdef USE_CONVEXHULL
			VertexBuffers[2].PopulateCopyCommand(CL, TotalSizeOf(VerticesCH), COM_PTR_GET(Upload_VertexCH.Resource));
			IndexBuffers[1].PopulateCopyCommand(CL, TotalSizeOf(IndicesCH), COM_PTR_GET(Upload_IndexCH.Resource));
			IndirectBuffers[1].PopulateCopyCommand(CL, sizeof(DIA_CH), COM_PTR_GET(Upload_IndirectCH.Resource));
#endif
		} VERIFY_SUCCEEDED(CL->Close());
		DX::ExecuteAndWait(CQ, CL, COM_PTR_GET(GraphicsFence));
	}
	virtual void CreateRootSignature() override {
		COM_PTR<ID3DBlob> Blob;
		DX::SerializeRootSignature(Blob, {}, {}, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT | SHADER_ROOT_ACCESS_DENY_ALL);
		VERIFY_SUCCEEDED(Device->CreateRootSignature(0, Blob->GetBufferPointer(), Blob->GetBufferSize(), COM_PTR_UUIDOF_PUTVOID(RootSignatures.emplace_back())));
		LOG_OK();
	}
	virtual void CreatePipelineState() override {
		PipelineStates.emplace_back();
#ifdef USE_CONVEXHULL
		PipelineStates.emplace_back();
#endif

		std::vector<COM_PTR<ID3DBlob>> SBs;
		VERIFY_SUCCEEDED(D3DReadFileToBlob(data(GetFilePath(".vs.cso").wstring()), COM_PTR_PUT(SBs.emplace_back())));
		VERIFY_SUCCEEDED(D3DReadFileToBlob(data(GetFilePath(".ps.cso").wstring()), COM_PTR_PUT(SBs.emplace_back())));
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
			.CullMode = D3D12_CULL_MODE_BACK, .FrontCounterClockwise = TRUE,
			.DepthBias = D3D12_DEFAULT_DEPTH_BIAS, .DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP, .SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS,
			.DepthClipEnable = TRUE,
			.MultisampleEnable = FALSE, .AntialiasedLineEnable = FALSE, .ForcedSampleCount = 0,
			.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF
		};
		DXExt::CreatePipelineState_VsPs_Input(PipelineStates[0], COM_PTR_GET(RootSignatures[0]), D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE, RD, TRUE, IEDs, SBCs);

#ifdef USE_CONVEXHULL
		std::vector<COM_PTR<ID3DBlob>> SBs_CH;
		VERIFY_SUCCEEDED(D3DReadFileToBlob(data(GetFilePath("_CH.vs.cso").wstring()), COM_PTR_PUT(SBs_CH.emplace_back())));
		VERIFY_SUCCEEDED(D3DReadFileToBlob(data(GetFilePath("_CH.ps.cso").wstring()), COM_PTR_PUT(SBs_CH.emplace_back())));
		const std::array SBCs_CH = {
			D3D12_SHADER_BYTECODE({.pShaderBytecode = SBs_CH[0]->GetBufferPointer(), .BytecodeLength = SBs_CH[0]->GetBufferSize() }),
			D3D12_SHADER_BYTECODE({.pShaderBytecode = SBs_CH[1]->GetBufferPointer(), .BytecodeLength = SBs_CH[1]->GetBufferSize() }),
		};
		const std::vector IEDs_CH = {
			D3D12_INPUT_ELEMENT_DESC({.SemanticName = "POSITION", .SemanticIndex = 0, .Format = DXGI_FORMAT_R32G32B32_FLOAT, .InputSlot = 0, .AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT, .InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, .InstanceDataStepRate = 0 }),
		};
		constexpr D3D12_RASTERIZER_DESC RD_CH = {
			.FillMode = D3D12_FILL_MODE_WIREFRAME,
			.CullMode = D3D12_CULL_MODE_BACK, .FrontCounterClockwise = TRUE,
			.DepthBias = D3D12_DEFAULT_DEPTH_BIAS, .DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP, .SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS,
			.DepthClipEnable = TRUE,
			.MultisampleEnable = FALSE, .AntialiasedLineEnable = FALSE, .ForcedSampleCount = 0,
			.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF
		};
		DXExt::CreatePipelineState_VsPs_Input(PipelineStates[1], COM_PTR_GET(RootSignatures[0]), D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE, RD_CH, TRUE, IEDs_CH, SBCs_CH);
#endif

		for (auto& i : Threads) { i.join(); }
		Threads.clear();
	}
	virtual void PopulateBundleCommandList(const size_t i) override {
		const auto PS = COM_PTR_GET(PipelineStates[0]);
		const auto BCL = COM_PTR_GET(BundleCommandLists[i]);
		const auto BCA = COM_PTR_GET(BundleCommandAllocators[0]);

		VERIFY_SUCCEEDED(BCL->Reset(BCA, PS));
		{
			BCL->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

			//BCL->SetPipelineState(PS);

			const std::array VBVs = { VertexBuffers[0].View, VertexBuffers[1].View };
			BCL->IASetVertexBuffers(0, static_cast<UINT>(size(VBVs)), data(VBVs));
			BCL->IASetIndexBuffer(&IndexBuffers[0].View);

			BCL->ExecuteIndirect(COM_PTR_GET(IndirectBuffers[0].CommandSignature), 1, COM_PTR_GET(IndirectBuffers[0].Resource), 0, nullptr, 0);

#ifdef USE_CONVEXHULL
			BCL->SetPipelineState(COM_PTR_GET(PipelineStates[1]));

			const std::array VBVs_CH = { VertexBuffers[2].View };
			BCL->IASetVertexBuffers(0, static_cast<UINT>(size(VBVs_CH)), data(VBVs_CH));
			BCL->IASetIndexBuffer(&IndexBuffers[1].View);

			BCL->ExecuteIndirect(COM_PTR_GET(IndirectBuffers[1].CommandSignature), 1, COM_PTR_GET(IndirectBuffers[1].Resource), 0, nullptr, 0);
#endif
		}
		VERIFY_SUCCEEDED(BCL->Close());
	}
	virtual void PopulateCommandList(const size_t i) override {
		const auto BCL = COM_PTR_GET(BundleCommandLists[i]);
		const auto DCL = COM_PTR_GET(DirectCommandLists[i]);
		const auto DCA = COM_PTR_GET(DirectCommandAllocators[0]);

		VERIFY_SUCCEEDED(DCL->Reset(DCA, nullptr));
		{
			DCL->SetGraphicsRootSignature(COM_PTR_GET(RootSignatures[0]));

			DCL->RSSetViewports(static_cast<UINT>(size(Viewports)), data(Viewports));
			DCL->RSSetScissorRects(static_cast<UINT>(size(ScissorRects)), data(ScissorRects));

			const auto SCR = COM_PTR_GET(SwapChainBackBuffers[i].Resource);
			ResourceBarrier(DCL, SCR, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
			{
				const auto& HandleDSV = DsvDescs[0].second;

				constexpr std::array<D3D12_RECT, 0> Rects = {};
				DCL->ClearRenderTargetView(SwapChainBackBuffers[i].Handle, DirectX::Colors::SkyBlue, static_cast<UINT>(size(Rects)), data(Rects));
				DCL->ClearDepthStencilView(HandleDSV[0], D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, static_cast<UINT>(size(Rects)), data(Rects));

				const std::array CHs = { SwapChainBackBuffers[i].Handle };
				DCL->OMSetRenderTargets(static_cast<UINT>(size(CHs)), data(CHs), FALSE, &HandleDSV[0]);

				DCL->ExecuteBundle(BCL);
			}
			ResourceBarrier(DCL, SCR, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
		}
		VERIFY_SUCCEEDED(DCL->Close());
	}
};
#pragma endregion