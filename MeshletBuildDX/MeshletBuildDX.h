#pragma once

#include "resource.h"

#pragma region Code
#include "../FBX.h"
#include "../DXExt.h"
#include "../DXMesh.h"

class MeshletBuildDX : public DXExtDepth, public Fbx
{
private:
	using Super = DXExtDepth;
public:
	MeshletBuildDX() : Super() {}
	virtual ~MeshletBuildDX() {}

	DefaultStructuredBuffer VertexBuffer;
	DefaultStructuredBuffer VertexIndexBuffer;
	DefaultStructuredBuffer MeshletBuffer;
	DefaultStructuredBuffer TriangleBuffer;

#pragma region FBX
	std::vector<UINT32> Indices;
	std::vector<DirectX::XMFLOAT3> Vertices;
	std::vector<DirectX::XMFLOAT3> Normals;
	DirectX::XMFLOAT3 ToFloat3(const FbxVector4& rhs) { return DirectX::XMFLOAT3(static_cast<FLOAT>(rhs[0]), static_cast<FLOAT>(rhs[1]), static_cast<FLOAT>(rhs[2])); }
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
			}
		}
		const auto Bound = std::max(std::max(Max.x - Min.x, Max.y - Min.y), Max.z - Min.z) * 1.0f;
		std::transform(begin(Vertices), end(Vertices), begin(Vertices), [&](const DirectX::XMFLOAT3& rhs) { return DirectX::XMFLOAT3(rhs.x / Bound, (rhs.y - (Max.y - Min.y) * 0.5f) / Bound, (rhs.z - Min.z) / Bound); });

		FbxArray<FbxVector4> Nrms;
		Mesh->GetPolygonVertexNormals(Nrms);
		for (auto i = 0; i < Nrms.Size(); ++i) {
			Normals.emplace_back(ToFloat3(Nrms[i]));
		}

		FbxStringList UVSetNames;
		Mesh->GetUVSetNames(UVSetNames);
		for (auto i = 0; i < UVSetNames.GetCount(); ++i) {
			FbxArray<FbxVector2> UVs;
			Mesh->GetPolygonVertexUVs(UVSetNames.GetStringAt(i), UVs);
		}
	}
#pragma endregion

	virtual void CreateGeometry() override {
		if (HasMeshShaderSupport(COM_PTR_GET(Device))) {
			const auto CA = COM_PTR_GET(DirectCommandAllocators[0]);
			const auto GCL = COM_PTR_GET(DirectCommandLists[0]);
			const auto CQ = COM_PTR_GET(GraphicsCommandQueue);
			
			constexpr D3D12_DISPATCH_MESH_ARGUMENTS DMA = { .ThreadGroupCountX = 1, .ThreadGroupCountY = 1, .ThreadGroupCountZ = 1 };
			IndirectBuffers.emplace_back().Create(COM_PTR_GET(Device), DMA).ExecuteCopyCommand(COM_PTR_GET(Device), CA, GCL, CQ, COM_PTR_GET(GraphicsFence), sizeof(DMA), &DMA);

			std::wstring Path;
			if (FindDirectory("FBX", Path)) {
				Load(ToString(Path) + "//bunny4.FBX");
			}
			std::vector<DirectX::Meshlet> Meshlets;
			std::vector<uint8_t> VertexIndices;
			std::vector<DirectX::MeshletTriangle> Triangles; //!< uint32_t の 30bit を使用して i0, i1, i2 それぞれ 10bit
			VERIFY_SUCCEEDED(DirectX::ComputeMeshlets(data(Indices), size(Indices) / 3, data(Vertices), size(Vertices), nullptr, Meshlets, VertexIndices, Triangles,
				DirectX::MESHLET_DEFAULT_MAX_VERTS, DirectX::MESHLET_DEFAULT_MAX_PRIMS));

			//assert(size(Meshlets) <= 32 && "32 を超える場合は複数回に分けて描画する必要があるが、ここでは超えてはいけないこととする");
			Logf("Meshlets Count = %d\n", size(Meshlets));
			for (size_t i = 0; i < std::min<size_t>(size(Meshlets), 8); ++i) {
				Logf("\t[%d] VertCount = %d, PrimCount = %d\n", i, Meshlets[i].VertCount, Meshlets[i].PrimCount);
			}
			Log("\t...\n");

			VertexBuffer.Create(COM_PTR_GET(Device), TotalSizeOf(Vertices), sizeof(Vertices[0]))
				.ExecuteCopyCommand(COM_PTR_GET(Device), CA, GCL, CQ, COM_PTR_GET(GraphicsFence), TotalSizeOf(Vertices), data(Vertices), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
			VertexIndexBuffer.Create(COM_PTR_GET(Device), TotalSizeOf(VertexIndices), sizeof(VertexIndices[0]))
				.ExecuteCopyCommand(COM_PTR_GET(Device), CA, GCL, CQ, COM_PTR_GET(GraphicsFence), TotalSizeOf(VertexIndices), data(VertexIndices), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE); 
			MeshletBuffer.Create(COM_PTR_GET(Device), TotalSizeOf(Meshlets), sizeof(Meshlets[0]))
				.ExecuteCopyCommand(COM_PTR_GET(Device), CA, GCL, CQ, COM_PTR_GET(GraphicsFence), TotalSizeOf(Meshlets), data(Meshlets), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
			TriangleBuffer.Create(COM_PTR_GET(Device), TotalSizeOf(Triangles), sizeof(Triangles[0]))
				.ExecuteCopyCommand(COM_PTR_GET(Device), CA, GCL, CQ, COM_PTR_GET(GraphicsFence), TotalSizeOf(Triangles), data(Triangles), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
		}
	}
	virtual void CreateRootSignature() override {
		COM_PTR<ID3DBlob> Blob;
#ifdef USE_HLSL_ROOTSIGNATRUE
		GetRootSignaturePartFromShader(Blob, data(GetBasePath() + TEXT(".grs.cso")));
#else
		constexpr std::array DRs = {
			D3D12_DESCRIPTOR_RANGE({.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV, .NumDescriptors = 4, .BaseShaderRegister = 0, .RegisterSpace = 0, .OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND })
		};
		DX::SerializeRootSignature(Blob, {
			D3D12_ROOT_PARAMETER({
				.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE,
				.DescriptorTable = D3D12_ROOT_DESCRIPTOR_TABLE({.NumDescriptorRanges = static_cast<UINT>(size(DRs)), .pDescriptorRanges = data(DRs) }),
				.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL
			}),
		}, {}, SHADER_ROOT_ACCESS_MS);
#endif
		VERIFY_SUCCEEDED(Device->CreateRootSignature(0, Blob->GetBufferPointer(), Blob->GetBufferSize(), COM_PTR_UUIDOF_PUTVOID(RootSignatures.emplace_back())));
	}
	virtual void CreatePipelineState() override {
		if (HasMeshShaderSupport(COM_PTR_GET(Device))) {
			const auto ShaderPath = GetBasePath();
			std::vector<COM_PTR<ID3DBlob>> SBs;
			VERIFY_SUCCEEDED(D3DReadFileToBlob(data(ShaderPath + TEXT(".as.cso")), COM_PTR_PUT(SBs.emplace_back())));
			VERIFY_SUCCEEDED(D3DReadFileToBlob(data(ShaderPath + TEXT(".ms.cso")), COM_PTR_PUT(SBs.emplace_back())));
			VERIFY_SUCCEEDED(D3DReadFileToBlob(data(ShaderPath + TEXT(".ps.cso")), COM_PTR_PUT(SBs.emplace_back())));
			const std::array SBCs = {
				D3D12_SHADER_BYTECODE({.pShaderBytecode = SBs[0]->GetBufferPointer(), .BytecodeLength = SBs[0]->GetBufferSize() }),
				D3D12_SHADER_BYTECODE({.pShaderBytecode = SBs[1]->GetBufferPointer(), .BytecodeLength = SBs[1]->GetBufferSize() }),
				D3D12_SHADER_BYTECODE({.pShaderBytecode = SBs[2]->GetBufferPointer(), .BytecodeLength = SBs[2]->GetBufferSize() }),
			};
			CreatePipelineState_AsMsPs(TRUE, SBCs);
		}
	}
	virtual void CreateDescriptor() override {
		const D3D12_DESCRIPTOR_HEAP_DESC DHD = { .Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, .NumDescriptors = 4, .Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE, .NodeMask = 0 };
		VERIFY_SUCCEEDED(Device->CreateDescriptorHeap(&DHD, COM_PTR_UUIDOF_PUTVOID(CbvSrvUavDescriptorHeaps.emplace_back())));

		CbvSrvUavGPUHandles.emplace_back();
		auto CDH = CbvSrvUavDescriptorHeaps[0]->GetCPUDescriptorHandleForHeapStart();
		auto GDH = CbvSrvUavDescriptorHeaps[0]->GetGPUDescriptorHandleForHeapStart();
		const auto IncSize = Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		Device->CreateShaderResourceView(COM_PTR_GET(VertexBuffer.Resource), &VertexBuffer.SRV, CDH);
		CbvSrvUavGPUHandles.back().emplace_back(GDH);
		CDH.ptr += IncSize;
		GDH.ptr += IncSize;
		Device->CreateShaderResourceView(COM_PTR_GET(VertexIndexBuffer.Resource), &VertexIndexBuffer.SRV, CDH);
		CbvSrvUavGPUHandles.back().emplace_back(GDH);
		CDH.ptr += IncSize;
		GDH.ptr += IncSize; 
		Device->CreateShaderResourceView(COM_PTR_GET(MeshletBuffer.Resource), &MeshletBuffer.SRV, CDH);
		CbvSrvUavGPUHandles.back().emplace_back(GDH);
		CDH.ptr += IncSize;
		GDH.ptr += IncSize;
		Device->CreateShaderResourceView(COM_PTR_GET(TriangleBuffer.Resource), &TriangleBuffer.SRV, CDH);
		CbvSrvUavGPUHandles.back().emplace_back(GDH);

		Super::CreateDescriptor();
	}
	virtual void PopulateCommandList(const size_t i) override {
		const auto HasMS = HasMeshShaderSupport(COM_PTR_GET(Device));
		const auto PS = HasMS ? COM_PTR_GET(PipelineStates[0]) : nullptr;
		const auto GCL = COM_PTR_GET(DirectCommandLists[i]);
		const auto CA = COM_PTR_GET(DirectCommandAllocators[0]);

		VERIFY_SUCCEEDED(GCL->Reset(CA, PS));
		{
			GCL->SetGraphicsRootSignature(COM_PTR_GET(RootSignatures[0]));

			GCL->RSSetViewports(static_cast<UINT>(size(Viewports)), data(Viewports));
			GCL->RSSetScissorRects(static_cast<UINT>(size(ScissorRects)), data(ScissorRects));

			const auto SCR = COM_PTR_GET(SwapChainResources[i]);
			ResourceBarrier(GCL, SCR, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
			{
				if (HasMS) {
					constexpr std::array<D3D12_RECT, 0> Rects = {};
					GCL->ClearRenderTargetView(SwapChainCPUHandles[i], DirectX::Colors::SkyBlue, static_cast<UINT>(size(Rects)), data(Rects));
					GCL->ClearDepthStencilView(DsvCPUHandles.back()[0], D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, static_cast<UINT>(size(Rects)), data(Rects));

					const std::array CHs = { SwapChainCPUHandles[i] };
					GCL->OMSetRenderTargets(static_cast<UINT>(size(CHs)), data(CHs), FALSE, &DsvCPUHandles.back()[0]);

					{
						const std::array DHs = { COM_PTR_GET(CbvSrvUavDescriptorHeaps[0]) };
						GCL->SetDescriptorHeaps(static_cast<UINT>(size(DHs)), data(DHs));
						GCL->SetGraphicsRootDescriptorTable(0, CbvSrvUavGPUHandles.back()[0]);
					}

					GCL->ExecuteIndirect(COM_PTR_GET(IndirectBuffers[0].CommandSignature), 1, COM_PTR_GET(IndirectBuffers[0].Resource), 0, nullptr, 0);
				}
				else {
					constexpr std::array<D3D12_RECT, 0> Rects = {};
					GCL->ClearRenderTargetView(SwapChainCPUHandles[i], DirectX::Colors::SkyBlue, static_cast<UINT>(size(Rects)), data(Rects));
				}
			}
			ResourceBarrier(GCL, SCR, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
		}
		VERIFY_SUCCEEDED(GCL->Close());
	}
};
#pragma endregion