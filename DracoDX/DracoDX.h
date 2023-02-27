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
		//Load(DRC_PATH / "bunny.drc");
		//Load(DRC_PATH / "dragon.drc");
		Load(DRC_PATH / "dragon4.drc");
		//Load(DRC_SAMPLE_PATH / "car.drc");

#ifdef USE_CONVEXHULL
		std::vector<Vec3> VerticesVec3; 
		VerticesVec3.reserve(size(Vertices));
		for (auto& i : Vertices) { VerticesVec3.emplace_back(Vec3({ i.x, i.y, i.z })); }

		std::vector<Vec3> HullVertices;
		std::vector<TriangleIndices> HullIndices;
		BuildConvexHull(VerticesVec3, HullVertices, HullIndices);
		{
			Vertices.clear();
			Indices.clear();
			for (auto& i : HullVertices) {
				Vertices.emplace_back(DirectX::XMFLOAT3(i.x(), i.y(), i.z()));
			}
			const auto IndexBase = static_cast<UINT32>(size(Indices));
			for (auto i : HullIndices) {
				Indices.emplace_back(IndexBase + static_cast<UINT32>(std::get<1>(i)));
				Indices.emplace_back(IndexBase + static_cast<UINT32>(std::get<0>(i)));
				Indices.emplace_back(IndexBase + static_cast<UINT32>(std::get<2>(i)));
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

		VERIFY_SUCCEEDED(CL->Reset(CA, nullptr)); {
			VertexBuffers[0].PopulateCopyCommand(CL, TotalSizeOf(Vertices), COM_PTR_GET(Upload_Vertex.Resource));
			VertexBuffers[1].PopulateCopyCommand(CL, TotalSizeOf(Normals), COM_PTR_GET(Upload_Normal.Resource));
			IndexBuffers.back().PopulateCopyCommand(CL, TotalSizeOf(Indices), COM_PTR_GET(Upload_Index.Resource));
			IndirectBuffers.back().PopulateCopyCommand(CL, sizeof(DIA), COM_PTR_GET(Upload_Indirect.Resource));
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
		DXExt::CreatePipelineState_VsPs_Input(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE, RD, TRUE, IEDs, SBCs);
	}
	virtual void CreateDescriptor() override {
		const D3D12_DESCRIPTOR_HEAP_DESC DHD = { .Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV, .NumDescriptors = 1, .Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE, .NodeMask = 0 };
		VERIFY_SUCCEEDED(Device->CreateDescriptorHeap(&DHD, COM_PTR_UUIDOF_PUTVOID(DsvDescriptorHeaps.emplace_back())));

		Super::CreateDescriptor();
	}
	virtual void PopulateCommandList(const size_t i) override {
		const auto PS = COM_PTR_GET(PipelineStates[0]);

#pragma region BUNDLE_COMMAND_LIST
		const auto BCL = COM_PTR_GET(BundleCommandLists[i]);
		const auto BCA = COM_PTR_GET(BundleCommandAllocators[0]);
		VERIFY_SUCCEEDED(BCL->Reset(BCA, PS));
		{
			BCL->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

			const std::array VBVs = { VertexBuffers[0].View, VertexBuffers[1].View };
			BCL->IASetVertexBuffers(0, static_cast<UINT>(size(VBVs)), data(VBVs));
			BCL->IASetIndexBuffer(&IndexBuffers[0].View);

			BCL->ExecuteIndirect(COM_PTR_GET(IndirectBuffers[0].CommandSignature), 1, COM_PTR_GET(IndirectBuffers[0].Resource), 0, nullptr, 0);
		}
		VERIFY_SUCCEEDED(BCL->Close());
#pragma endregion

		const auto CL = COM_PTR_GET(DirectCommandLists[i]);
		const auto CA = COM_PTR_GET(DirectCommandAllocators[0]);
		VERIFY_SUCCEEDED(CL->Reset(CA, PS));
		{
			CL->SetGraphicsRootSignature(COM_PTR_GET(RootSignatures[0]));

			CL->RSSetViewports(static_cast<UINT>(size(Viewports)), data(Viewports));
			CL->RSSetScissorRects(static_cast<UINT>(size(ScissorRects)), data(ScissorRects));

			const auto SCR = COM_PTR_GET(SwapChainResources[i]);
			ResourceBarrier(CL, SCR, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
			{
				constexpr std::array<D3D12_RECT, 0> Rects = {};
				CL->ClearRenderTargetView(SwapChainCPUHandles[i], DirectX::Colors::SkyBlue, static_cast<UINT>(size(Rects)), data(Rects));
				CL->ClearDepthStencilView(DsvCPUHandles.back()[0], D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, static_cast<UINT>(size(Rects)), data(Rects));

				const std::array CHs = { SwapChainCPUHandles[i] };
				CL->OMSetRenderTargets(static_cast<UINT>(size(CHs)), data(CHs), FALSE, &DsvCPUHandles.back()[0]);

				CL->ExecuteBundle(BCL);
			}
			ResourceBarrier(CL, SCR, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
		}
		VERIFY_SUCCEEDED(CL->Close());
	}
};
#pragma endregion