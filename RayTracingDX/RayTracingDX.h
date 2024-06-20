#pragma once

#include "resource.h"

#pragma region Code
#include "../FBX.h"
#include "../DXImage.h"

class RayTracingDX : public DXImageRT, public Fbx
{
private:
	using Super = DXImageRT;
public:
	RayTracingDX() : Super() {}
	virtual ~RayTracingDX() {}

	DefaultStructuredBuffer VertexBuffer;
	DefaultStructuredBuffer IndexBuffer;

#pragma region FBX
	DirectX::XMFLOAT3 ToFloat3(const FbxVector4& rhs) { return DirectX::XMFLOAT3(static_cast<FLOAT>(rhs[0]), static_cast<FLOAT>(rhs[1]), static_cast<FLOAT>(rhs[2])); }
	std::vector<UINT32> Indices;
	std::vector<Vertex_PositionNormal> Vertices;
	virtual void Process(FbxMesh* Mesh) override {
		Fbx::Process(Mesh);

		std::vector<DirectX::XMFLOAT3> Vs;
		std::vector<DirectX::XMFLOAT3> Ns;
		std::cout << "PolygonCount = " << Mesh->GetPolygonCount() << std::endl;
		for (auto i = 0; i < Mesh->GetPolygonCount(); ++i) {
			for (auto j = 0; j < Mesh->GetPolygonSize(i); ++j) {
				Indices.emplace_back(i * Mesh->GetPolygonSize(i) + j);

				Vs.emplace_back(ToFloat3(Mesh->GetControlPoints()[Mesh->GetPolygonVertex(i, j)]));
			}
		}
		AdjustScale(Vs, 1.0f);

		FbxArray<FbxVector4> PVNs;
		Mesh->GetPolygonVertexNormals(PVNs);
		for (auto i = 0; i < PVNs.Size(); ++i) {
			Ns.emplace_back(ToFloat3(PVNs[i]));
		}

		for (auto i = 0; i < size(Vs); ++i) {
			Vertices.emplace_back(Vertex_PositionNormal({ Vs[i], Ns[i] }));
		}
	}
#pragma endregion

	virtual void CreateGeometry() override {
		if (!HasRaytracingSupport(COM_PTR_GET(Device))) { return; }

		//Load(FBX_PATH / "bunny.FBX");
		Load(FBX_PATH / "dragon.FBX");

		const auto CL = COM_PTR_GET(DirectCommandLists[0]);
		const auto CA = COM_PTR_GET(DirectCommandAllocators[0]);
		const auto GCQ = COM_PTR_GET(GraphicsCommandQueue);

#pragma region BLAS_INPUT
		VertexBuffer.Create(COM_PTR_GET(Device), TotalSizeOf(Vertices), sizeof(Vertices[0])).ExecuteCopyCommand(COM_PTR_GET(Device), CA, CL, GCQ, COM_PTR_GET(GraphicsFence), TotalSizeOf(Vertices), data(Vertices));
		IndexBuffer.Create(COM_PTR_GET(Device), TotalSizeOf(Indices), sizeof(Indices[0])).ExecuteCopyCommand(COM_PTR_GET(Device), CA, CL, GCQ, COM_PTR_GET(GraphicsFence), TotalSizeOf(Indices), data(Indices));
		const std::array RGDs = {
			D3D12_RAYTRACING_GEOMETRY_DESC({
				.Type = D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES,
				.Flags = D3D12_RAYTRACING_GEOMETRY_FLAG_OPAQUE,
				.Triangles = D3D12_RAYTRACING_GEOMETRY_TRIANGLES_DESC({
					.Transform3x4 = 0,
					.IndexFormat = DXGI_FORMAT_R32_UINT,
					.VertexFormat = DXGI_FORMAT_R32G32B32_FLOAT,
					.IndexCount = static_cast<UINT>(size(Indices)),
					.VertexCount = static_cast<UINT>(size(Vertices)),
					.IndexBuffer = IndexBuffer.Resource->GetGPUVirtualAddress(),
					.VertexBuffer = D3D12_GPU_VIRTUAL_ADDRESS_AND_STRIDE({.StartAddress = VertexBuffer.Resource->GetGPUVirtualAddress(), .StrideInBytes = sizeof(Vertices[0]) }),
				})
			}),
		};
		const D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS BRASI_Blas = {
			.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL,
			.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE,
			.NumDescs = static_cast<UINT>(size(RGDs)), 
			.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY,
			.pGeometryDescs = data(RGDs), 
		};
#pragma endregion

#pragma region BLAS
		D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO RASPI_Blas;
		Device5->GetRaytracingAccelerationStructurePrebuildInfo(&BRASI_Blas, &RASPI_Blas);
		BLASs.emplace_back().Create(COM_PTR_GET(Device), RASPI_Blas.ResultDataMaxSizeInBytes);
#pragma endregion
		
#pragma region TLAS_INPUT
		const std::array RIDs = {
			#pragma region INSTANCES
			D3D12_RAYTRACING_INSTANCE_DESC({
				.Transform = {
					{ 1.0f, 0.0f, 0.0f, -1.0f }, 
					{ 0.0f, 1.0f, 0.0f,  0.0f },
					{ 0.0f, 0.0f, 1.0f,  0.0f }
				},
				.InstanceID = 0, //!< [HLSL] InstanceID() ([GLSL] gl_InstanceCustomIndexEXT 相当) (ここではインスタンス毎の出し分けに使用)
				.InstanceMask = 0xff,
				.InstanceContributionToHitGroupIndex = 0, 
				.Flags = D3D12_RAYTRACING_INSTANCE_FLAG_TRIANGLE_FRONT_COUNTERCLOCKWISE,
				.AccelerationStructure = BLASs.back().Resource->GetGPUVirtualAddress()
			}),
			D3D12_RAYTRACING_INSTANCE_DESC({
				.Transform = {
					{ 1.0f, 0.0f, 0.0f, 0.0f },
					{ 0.0f, 1.0f, 0.0f, 0.0f },
					{ 0.0f, 0.0f, 1.0f, 0.0f }
				},
				.InstanceID = 1,
				.InstanceMask = 0xff,
				.InstanceContributionToHitGroupIndex = 0,
				.Flags = D3D12_RAYTRACING_INSTANCE_FLAG_TRIANGLE_FRONT_COUNTERCLOCKWISE,
				.AccelerationStructure = BLASs.back().Resource->GetGPUVirtualAddress()
			}),
			D3D12_RAYTRACING_INSTANCE_DESC({
				.Transform = {
					{ 1.0f, 0.0f, 0.0f, 1.0f },
					{ 0.0f, 1.0f, 0.0f, 0.0f },
					{ 0.0f, 0.0f, 1.0f, 0.0f }
				},
				.InstanceID = 2, 
				.InstanceMask = 0xff,
				.InstanceContributionToHitGroupIndex = 0,
				.Flags = D3D12_RAYTRACING_INSTANCE_FLAG_TRIANGLE_FRONT_COUNTERCLOCKWISE,
				.AccelerationStructure = BLASs.back().Resource->GetGPUVirtualAddress()
			})
			#pragma endregion
		};
		UploadResource InsBuf;
		InsBuf.Create(COM_PTR_GET(Device), sizeof(RIDs), data(RIDs));
		
		const D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS BRASI_Tlas = {
			.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL,
			.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE,
			.NumDescs = static_cast<UINT>(size(RIDs)),
			.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY,
			.InstanceDescs = InsBuf.Resource->GetGPUVirtualAddress()
		};
#pragma endregion

#pragma region TLAS
		D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO RASPI_Tlas;
		Device5->GetRaytracingAccelerationStructurePrebuildInfo(&BRASI_Tlas, &RASPI_Tlas);
		TLASs.emplace_back().Create(COM_PTR_GET(Device), RASPI_Tlas.ResultDataMaxSizeInBytes);
#pragma endregion
	
#pragma region SCRATCH
		ScratchBuffer Scratch;
		Scratch.Create(COM_PTR_GET(Device), std::max(RASPI_Blas.ScratchDataSizeInBytes, RASPI_Tlas.ScratchDataSizeInBytes));
#pragma endregion

		VERIFY_SUCCEEDED(CL->Reset(CA, nullptr)); {
			BLASs.back().PopulateBuildCommand(BRASI_Blas, CL, COM_PTR_GET(Scratch.Resource));
			BLASs.back().PopulateBarrierCommand(CL);
			TLASs.back().PopulateBuildCommand(BRASI_Tlas, CL, COM_PTR_GET(Scratch.Resource));
		} VERIFY_SUCCEEDED(CL->Close());
		DX::ExecuteAndWait(GCQ, static_cast<ID3D12CommandList*>(CL), COM_PTR_GET(GraphicsFence));
	}
	virtual void CreateTexture() override {
		Super::CreateTexture();

		const auto CA = COM_PTR_GET(DirectCommandAllocators[0]);
		const auto CL = COM_PTR_GET(DirectCommandLists[0]);
		XTKTextures.emplace_back().Create(COM_PTR_GET(Device), DDS_PATH / "CubeMap" / "ninomaru_teien.dds")
			.ExecuteCopyCommand(COM_PTR_GET(Device), CA, CL, COM_PTR_GET(GraphicsCommandQueue), COM_PTR_GET(GraphicsFence), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

		const auto RD = XTKTextures.back().Resource->GetDesc();
		XTKTextures.back().SRV = D3D12_SHADER_RESOURCE_VIEW_DESC({ .Format = RD.Format, .ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE, .Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING, .TextureCube = D3D12_TEXCUBE_SRV({.MostDetailedMip = 0, .MipLevels = RD.MipLevels, .ResourceMinLODClamp = 0.0f }), });
	}
	virtual void CreateConstantBuffer() override {
		constexpr auto Fov = 0.16f * std::numbers::pi_v<float>;
		const auto Aspect = GetAspectRatioOfClientRect();
		constexpr auto ZFar = 100.0f;
		constexpr auto ZNear = ZFar * 0.0001f;
		const auto CamPos = DirectX::XMVectorSet(0.0f, 0.0f, 3.0f, 1.0f);
		const auto CamTag = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
		const auto CamUp = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
		const auto Projection = DirectX::XMMatrixPerspectiveFovRH(Fov, Aspect, ZNear, ZFar);
		const auto View = DirectX::XMMatrixLookAtRH(CamPos, CamTag, CamUp);
		const auto InvProjection = DirectX::XMMatrixInverse(nullptr, Projection);
		const auto InvView = DirectX::XMMatrixInverse(nullptr, View);
		DirectX::XMStoreFloat4x4(&Tr.Projection, Projection);
		DirectX::XMStoreFloat4x4(&Tr.View, View);
		DirectX::XMStoreFloat4x4(&Tr.InvProjection, InvProjection);
		DirectX::XMStoreFloat4x4(&Tr.InvView, InvView);
		DXGI_SWAP_CHAIN_DESC1 SCD;
		SwapChain->GetDesc1(&SCD);
		for (UINT i = 0; i < SCD.BufferCount; ++i) {
			ConstantBuffers.emplace_back().Create(COM_PTR_GET(Device), sizeof(Tr), &Tr);
		}
	}
	virtual void CreateStaticSampler() override {
		CreateStaticSampler_LW(0, 0, D3D12_SHADER_VISIBILITY_ALL);
	}
	virtual void CreateRootSignature() override {
		if (!HasRaytracingSupport(COM_PTR_GET(Device))) { return; }

		//!< ここではグローバルルートシグネチャに RegisterSpace = 0、ローカルルートシグネチャに RegisterSpace = 1 で運用する (レジスタ割り当てを楽にする為)

		//!< グローバルルートシグネチャ (Global root signature) 
		{
			COM_PTR<ID3DBlob> Blob;
#ifdef USE_HLSL_ROOTSIGNATRUE
			GetRootSignaturePartFromShader(Blob, data(GetBasePath() + TEXT(".grs.cso")));
#else
			constexpr std::array DRs_Srv = {
				D3D12_DESCRIPTOR_RANGE1({.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV, .NumDescriptors = 2, .BaseShaderRegister = 0, .RegisterSpace = 0, .Flags = D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE,.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND })
			};
			constexpr std::array DRs_Uav = {
				D3D12_DESCRIPTOR_RANGE1({.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV, .NumDescriptors = 1, .BaseShaderRegister = 0, .RegisterSpace = 0, .Flags = D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE,.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND })
			};
			DX::SerializeRootSignature(Blob, {
				//!< SRV0, SRV1 (TLAS, CubeMap) ... register(t0, space0), register(t1, space0)
				D3D12_ROOT_PARAMETER1({
					.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE, 
					.DescriptorTable = D3D12_ROOT_DESCRIPTOR_TABLE1({.NumDescriptorRanges = static_cast<UINT>(size(DRs_Srv)), .pDescriptorRanges = data(DRs_Srv) }),
					.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL 
				}),
				//!< UAV0 (RenderTarget) ... register(u0, space0)
				D3D12_ROOT_PARAMETER1({
					.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE, 
					.DescriptorTable = D3D12_ROOT_DESCRIPTOR_TABLE1({.NumDescriptorRanges = static_cast<UINT>(size(DRs_Uav)), .pDescriptorRanges = data(DRs_Uav) }),
					.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL
				}),
				//!< CBV0 (CB) ... register(b0, space0)
				//!< バックバッファ分のシェーダテーブルを作成しないで済むように SetComputeRootConstantBufferView() を使用する
				//!< SetComputeRootConstantBufferView() を使用する為、D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE ではなくて D3D12_ROOT_PARAMETER_TYPE_CBV を使用
				D3D12_ROOT_PARAMETER1({
					.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV,
					.Descriptor = D3D12_ROOT_DESCRIPTOR1({ .ShaderRegister = 0, .RegisterSpace = 0, .Flags = D3D12_ROOT_DESCRIPTOR_FLAG_NONE}),
					.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL
				}),
			}, {
				StaticSamplerDescs[0],
			}, D3D12_ROOT_SIGNATURE_FLAG_NONE); //!< グローバルルートシグネチャでは D3D12_ROOT_SIGNATURE_FLAG_NONE を指定して作成
#endif
			VERIFY_SUCCEEDED(Device->CreateRootSignature(0, Blob->GetBufferPointer(), Blob->GetBufferSize(), COM_PTR_UUIDOF_PUTVOID(RootSignatures.emplace_back())));
		}

#pragma region SHADER_RECORD
		//!< ローカルルートシグネチャ (Local root signature) ... D3D12_ROOT_SIGNATURE_FLAG_LOCAL_ROOT_SIGNATURE を指定して作成
		{
			COM_PTR<ID3DBlob> Blob;
#ifdef USE_HLSL_ROOTSIGNATRUE
			GetRootSignaturePartFromShader(Blob, data(GetBasePath() + TEXT(".lrs.cso")));
#else
			DX::SerializeRootSignature(Blob, {
				//!< SRV0 (VB) ... register(t0, space1)
				D3D12_ROOT_PARAMETER1({
					.ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV,
					.Descriptor = D3D12_ROOT_DESCRIPTOR1({.ShaderRegister = 0, .RegisterSpace = 1, .Flags = D3D12_ROOT_DESCRIPTOR_FLAG_NONE }),
					.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL
				}),
				//!< SRV1 (IB) ... register(t1, space1)
				D3D12_ROOT_PARAMETER1({
					.ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV,
					.Descriptor = D3D12_ROOT_DESCRIPTOR1({.ShaderRegister = 1, .RegisterSpace = 1, .Flags = D3D12_ROOT_DESCRIPTOR_FLAG_NONE }),
					.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL
				}),
				}, {}, D3D12_ROOT_SIGNATURE_FLAG_LOCAL_ROOT_SIGNATURE); //!< ローカルルートシグネチャでは D3D12_ROOT_SIGNATURE_FLAG_LOCAL_ROOT_SIGNATURE を指定して作成
#endif
			VERIFY_SUCCEEDED(Device->CreateRootSignature(0, Blob->GetBufferPointer(), Blob->GetBufferSize(), COM_PTR_UUIDOF_PUTVOID(RootSignatures.emplace_back())));
		}
#pragma endregion
	}
	virtual void CreatePipelineState() override {
		if (!HasRaytracingSupport(COM_PTR_GET(Device))) { return; }

#pragma region STATE_OBJECT
		const D3D12_GLOBAL_ROOT_SIGNATURE GRS = { .pGlobalRootSignature = COM_PTR_GET(RootSignatures[0]) };

		COM_PTR<ID3DBlob> SB_Gen;
		VERIFY_SUCCEEDED(D3DReadFileToBlob(data(GetFilePath(".rgen.cso").wstring()), COM_PTR_PUT(SB_Gen)));
		COM_PTR<ID3DBlob> SB_Miss;
		VERIFY_SUCCEEDED(D3DReadFileToBlob(data(GetFilePath(".miss.cso").wstring()), COM_PTR_PUT(SB_Miss)));
		COM_PTR<ID3DBlob> SB_Hit;
		VERIFY_SUCCEEDED(D3DReadFileToBlob(data(GetFilePath(".rchit.cso").wstring()), COM_PTR_PUT(SB_Hit)));

		std::array EDs_Gen = { D3D12_EXPORT_DESC({.Name = TEXT("OnRayGeneration"), .ExportToRename = nullptr, .Flags = D3D12_EXPORT_FLAG_NONE }), };
		std::array EDs_Miss = { D3D12_EXPORT_DESC({.Name = TEXT("OnMiss"), .ExportToRename = nullptr, .Flags = D3D12_EXPORT_FLAG_NONE }), };
		std::array EDs_Hit = { D3D12_EXPORT_DESC({.Name = TEXT("OnClosestHit"), .ExportToRename = nullptr, .Flags = D3D12_EXPORT_FLAG_NONE }), };

		const auto DLD_Gen = D3D12_DXIL_LIBRARY_DESC({
			.DXILLibrary = D3D12_SHADER_BYTECODE({.pShaderBytecode = SB_Gen->GetBufferPointer(), .BytecodeLength = SB_Gen->GetBufferSize() }),
			.NumExports = static_cast<UINT>(size(EDs_Gen)), .pExports = data(EDs_Gen)
		});
		const auto DLD_Miss = D3D12_DXIL_LIBRARY_DESC({
			.DXILLibrary = D3D12_SHADER_BYTECODE({.pShaderBytecode = SB_Miss->GetBufferPointer(), .BytecodeLength = SB_Miss->GetBufferSize() }),
			.NumExports = static_cast<UINT>(size(EDs_Miss)), .pExports = data(EDs_Miss)
		});
		const auto DLD_Hit = D3D12_DXIL_LIBRARY_DESC({
			.DXILLibrary = D3D12_SHADER_BYTECODE({.pShaderBytecode = SB_Hit->GetBufferPointer(), .BytecodeLength = SB_Hit->GetBufferSize() }),
			.NumExports = static_cast<UINT>(size(EDs_Hit)), .pExports = data(EDs_Hit)
		});

		constexpr D3D12_HIT_GROUP_DESC HGD = {
			.HitGroupExport = TEXT("HitGroup"),
			.Type = D3D12_HIT_GROUP_TYPE_TRIANGLES,
			.AnyHitShaderImport = nullptr, .ClosestHitShaderImport = TEXT("OnClosestHit"), .IntersectionShaderImport = nullptr
		};

		constexpr D3D12_RAYTRACING_SHADER_CONFIG RSC = {
			.MaxPayloadSizeInBytes = static_cast<UINT>(sizeof(DirectX::XMFLOAT3) + sizeof(int)), //!< ここでは struct PAYLOAD { float3 Color; int Recursive; } を使用するため
			.MaxAttributeSizeInBytes = sizeof(DirectX::XMFLOAT2) //!< ここでは struct BuiltInTriangleIntersectionAttributes { float2 barycentrics; } を使用するため
		};

		//!< レイトレーシング再帰呼び出し可能な段数 ([0, 31]でなければならない) 
		constexpr D3D12_RAYTRACING_PIPELINE_CONFIG RPC = { .MaxTraceRecursionDepth = 31 }; //!< ここでは最大再帰呼び出し回数を指定

		std::vector SSs = {
			D3D12_STATE_SUBOBJECT({.Type = D3D12_STATE_SUBOBJECT_TYPE_GLOBAL_ROOT_SIGNATURE, .pDesc = &GRS }),
			D3D12_STATE_SUBOBJECT({.Type = D3D12_STATE_SUBOBJECT_TYPE_DXIL_LIBRARY, .pDesc = &DLD_Gen }),
			D3D12_STATE_SUBOBJECT({.Type = D3D12_STATE_SUBOBJECT_TYPE_DXIL_LIBRARY, .pDesc = &DLD_Miss }),
			D3D12_STATE_SUBOBJECT({.Type = D3D12_STATE_SUBOBJECT_TYPE_DXIL_LIBRARY, .pDesc = &DLD_Hit }),
			D3D12_STATE_SUBOBJECT({.Type = D3D12_STATE_SUBOBJECT_TYPE_HIT_GROUP, .pDesc = &HGD }),
			D3D12_STATE_SUBOBJECT({.Type = D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_SHADER_CONFIG, .pDesc = &RSC }),
			D3D12_STATE_SUBOBJECT({.Type = D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_PIPELINE_CONFIG, .pDesc = &RPC }),
		};

#pragma region SHADER_RECORD
		SSs.reserve(size(SSs) + 2); //!< 以下で直前要素のポインタを指定するため、内部データの引っ越しが起こらないようにしておく
		{
			//!< ローカルルートシグネチャ
			const D3D12_LOCAL_ROOT_SIGNATURE LRS = { .pLocalRootSignature = COM_PTR_GET(RootSignatures[1]) };
			SSs.emplace_back(D3D12_STATE_SUBOBJECT({ .Type = D3D12_STATE_SUBOBJECT_TYPE_LOCAL_ROOT_SIGNATURE, .pDesc = &LRS }));

			//!< ローカルルートシグネチャ(直前に追加したD3D12_STATE_SUBOBJECT)とヒットグループの関連付け
			std::array Exports = { TEXT("HitGroup") };
			const D3D12_SUBOBJECT_TO_EXPORTS_ASSOCIATION STEA = { .pSubobjectToAssociate = &SSs.back()/*直前のD3D12_STATE_SUBOBJECT*/, .NumExports = static_cast<UINT>(size(Exports)), .pExports = data(Exports)};
			SSs.emplace_back(D3D12_STATE_SUBOBJECT({ .Type = D3D12_STATE_SUBOBJECT_TYPE_SUBOBJECT_TO_EXPORTS_ASSOCIATION, .pDesc = &STEA }));
		}
#pragma endregion

		const D3D12_STATE_OBJECT_DESC SOD = {
			.Type = D3D12_STATE_OBJECT_TYPE_RAYTRACING_PIPELINE,
			.NumSubobjects = static_cast<UINT>(size(SSs)), .pSubobjects = data(SSs)
		};
		
		VERIFY_SUCCEEDED(Device5->CreateStateObject(&SOD, COM_PTR_UUIDOF_PUTVOID(StateObjects.emplace_back())));
#pragma endregion
	}
	virtual void CreateDescriptor() override {
		auto& Desc = CbvSrvUavDescs.emplace_back();
		auto& Heap = Desc.first;
		auto& Handle = Desc.second;

		DXGI_SWAP_CHAIN_DESC1 SCD;
		SwapChain->GetDesc1(&SCD);
#pragma region SHADER_RECORD
		const D3D12_DESCRIPTOR_HEAP_DESC DHD = { .Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, .NumDescriptors = 3 + 2, .Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE, .NodeMask = 0};
#pragma endregion
		VERIFY_SUCCEEDED(Device->CreateDescriptorHeap(&DHD, COM_PTR_UUIDOF_PUTVOID(Heap)));

		auto CDH = Heap->GetCPUDescriptorHandleForHeapStart();
		auto GDH = Heap->GetGPUDescriptorHandleForHeapStart();
		const auto IncSize = Device->GetDescriptorHandleIncrementSize(Heap->GetDesc().Type);
		//!< [0] SRV (TLAS)
		Device->CreateShaderResourceView(nullptr, &TLASs[0].SRV, CDH); 
		Handle.emplace_back(GDH);
		CDH.ptr += IncSize;
		GDH.ptr += IncSize;
		//!< [1] SRV (CubeMap)
		Device->CreateShaderResourceView(COM_PTR_GET(XTKTextures[0].Resource), &XTKTextures[0].SRV, CDH); 
		Handle.emplace_back(GDH);
		CDH.ptr += IncSize;
		GDH.ptr += IncSize;		
		//!< [2] UAV (RenderTarget)
		Device->CreateUnorderedAccessView(COM_PTR_GET(UnorderedAccessTextures[0].Resource), nullptr, &UnorderedAccessTextures[0].UAV, CDH); 
		Handle.emplace_back(GDH); 
		CDH.ptr += IncSize;
		GDH.ptr += IncSize; 
		
#pragma region SHADER_RECORD
		//!< [3] SRV (VB)
		Device->CreateShaderResourceView(COM_PTR_GET(VertexBuffer.Resource), &VertexBuffer.SRV, CDH);
		Handle.emplace_back(GDH);
		CDH.ptr += IncSize;
		GDH.ptr += IncSize;
		//!< [4] SRV (IB)
		Device->CreateShaderResourceView(COM_PTR_GET(IndexBuffer.Resource), &IndexBuffer.SRV, CDH);
		Handle.emplace_back(GDH);
#pragma endregion

		//!< この時点で削除してしまって良い？
		//COM_PTR_RESET(VertexBuffer.Resource);
		//COM_PTR_RESET(IndexBuffer.Resource);
	}
	virtual void CreateShaderTable() override {
		COM_PTR<ID3D12StateObjectProperties> SOP;
		VERIFY_SUCCEEDED(StateObjects.back()->QueryInterface(COM_PTR_UUIDOF_PUTVOID(SOP)));
		auto& ST = ShaderTables.emplace_back(); {
			//!< 各グループのハンドルの個数 (Genは1固定)
			constexpr auto MissCount = 1;
			constexpr auto HitCount = 1;
			//!< シェーダレコードサイズ
			constexpr auto GenRecordSize = 0;
			constexpr auto MissRecordSize = 0;
#pragma region SHADER_RECORD
			//!< D3D12_GPU_VIRTUAL_ADDRESS		: 8 Byte アライン
			//!< D3D12_GPU_DESCRIPTOR_HANDLE	: 8 Byte アライン
			//!< uint32_t						: 4 Byte アライン
			//!<	8 Byte アラインに 4 Byte アラインのものを挟む場合等ではパディングが必要になるので注意
			//!<	8 8 4 4
			//!<	8 4 (4) 8 ... (4)はパディング
			constexpr auto HitRecordSize = Cmn::RoundUp(
				Cmn::RoundUp(
					sizeof(D3D12_GPU_DESCRIPTOR_HANDLE), 
					sizeof(D3D12_GPU_DESCRIPTOR_HANDLE)) + sizeof(D3D12_GPU_DESCRIPTOR_HANDLE),
				sizeof(D3D12_GPU_DESCRIPTOR_HANDLE));
#pragma endregion
			//!< ストライド
			constexpr auto GenStride = Cmn::RoundUp(D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES + GenRecordSize, D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT);
			constexpr auto MissStride = Cmn::RoundUp(D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES + MissRecordSize, D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT);
			constexpr auto HitStride = Cmn::RoundUp(D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES + HitRecordSize, D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT);
			//!< サイズ
			constexpr auto GenSize = Cmn::RoundUp(GenStride, D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT);
			constexpr auto MissSize = Cmn::RoundUp(MissCount * MissStride, D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT);
			constexpr auto HitSize = Cmn::RoundUp(HitCount * HitStride, D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT);
			//!< レンジ
			ST.AddressRange = D3D12_GPU_VIRTUAL_ADDRESS_RANGE({ .SizeInBytes = GenSize });
			ST.AddressRangeAndStrides[0] = D3D12_GPU_VIRTUAL_ADDRESS_RANGE_AND_STRIDE({ .SizeInBytes = MissSize, .StrideInBytes = MissStride });
			ST.AddressRangeAndStrides[1] = D3D12_GPU_VIRTUAL_ADDRESS_RANGE_AND_STRIDE({ .SizeInBytes = HitSize, .StrideInBytes = HitStride });

			ST.Create(COM_PTR_GET(Device), GenSize + MissSize + HitSize);
			auto MapData = ST.Map(); {
				auto Data = reinterpret_cast<std::byte*>(MapData);

				//!< グループ (Gen)
				{
					const auto& Range = ST.AddressRange; {
						std::memcpy(Data, SOP->GetShaderIdentifier(TEXT("OnRayGeneration")), D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES);
						Data += Range.SizeInBytes;
					}
				}

				//!< グループ (Miss)
				{
					const auto Count = MissCount;
					const auto& Range = ST.AddressRangeAndStrides[0]; {
						auto p = Data;
						for (auto i = 0; i < Count; ++i, p += Range.StrideInBytes) {
							std::memcpy(p, SOP->GetShaderIdentifier(TEXT("OnMiss")), D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES);
						}
						Data += Range.SizeInBytes;
					}
				}

				//!< グループ (Hit)
				{
					const auto& Desc = CbvSrvUavDescs[0];
					const auto& Handle = Desc.second;

					const auto Count = HitCount;
					const auto& Range = ST.AddressRangeAndStrides[1]; {
#pragma region SHADER_RECORD
						const auto& VB = Handle[3];
						const auto& IB = Handle[4];
#pragma endregion
						auto p = Data;
						for (auto i = 0; i < Count; ++i, p += Range.StrideInBytes) {
							std::memcpy(p, SOP->GetShaderIdentifier(TEXT("HitGroup")), D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES); //!< ヒットグループ作成時に指定したヒットグループ名を使用
							size_t Offset = Cmn::RoundUp(D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES, D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES);

#pragma region SHADER_RECORD
							//!< [3] SRV (VB)
							std::memcpy(p + Offset, &VB, sizeof(VB));
							Offset = Cmn::RoundUp(Offset + sizeof(VB), sizeof(VB));
							//!< [4] SRV (IB)
							std::memcpy(p + Offset, &IB, sizeof(IB));
							Offset = Cmn::RoundUp(Offset + sizeof(IB), sizeof(IB));
#pragma endregion
						}
						Data += Range.SizeInBytes;
					}
				}
			} ST.Unmap();

			const auto DRD = D3D12_DISPATCH_RAYS_DESC({
				.RayGenerationShaderRecord = ST.AddressRange,
				.MissShaderTable = ST.AddressRangeAndStrides[0],
				.HitGroupTable = ST.AddressRangeAndStrides[1],
				.CallableShaderTable = ST.AddressRangeAndStrides[2],
				.Width = static_cast<UINT>(GetClientRectWidth()), .Height = static_cast<UINT>(GetClientRectHeight()), .Depth = 1
				});
			IndirectBuffers.emplace_back().Create(COM_PTR_GET(Device), DRD).ExecuteCopyCommand(COM_PTR_GET(Device), COM_PTR_GET(DirectCommandAllocators[0]), COM_PTR_GET(DirectCommandLists[0]), COM_PTR_GET(GraphicsCommandQueue), COM_PTR_GET(GraphicsFence), sizeof(DRD), &DRD);
		}
	}

	virtual void PopulateCommandList(const size_t i) override {
		if (!HasRaytracingSupport(COM_PTR_GET(Device))) { return; }

		const auto CL = COM_PTR_GET(DirectCommandLists[i]);
		const auto CA = COM_PTR_GET(DirectCommandAllocators[0]);
		const auto RT = COM_PTR_GET(UnorderedAccessTextures[0].Resource);
		VERIFY_SUCCEEDED(CL->Reset(CA, nullptr)); {
			PopulateBeginRenderTargetCommand(CL, RT); {
				const auto& Desc = CbvSrvUavDescs[0];
				const auto& Heap = Desc.first;
				const auto& Handle = Desc.second;

				const std::array DHs = { COM_PTR_GET(Heap) };
				CL->SetDescriptorHeaps(static_cast<UINT>(size(DHs)), data(DHs));

				CL->SetComputeRootSignature(COM_PTR_GET(RootSignatures[0]));
				//!< [0] SRV(TLAS), [1] SRV(CubeMap)
				CL->SetComputeRootDescriptorTable(0, Handle[0]);
				//!< [2] UAV
				CL->SetComputeRootDescriptorTable(1, Handle[2]);
				//!< CBV (デスクリプタヒープとは別扱い) バックバッファ分のシェーダテーブルを作成しないで済む為に SetComputeRootConstantBufferView() を使用 
				CL->SetComputeRootConstantBufferView(2, ConstantBuffers[i].Resource->GetGPUVirtualAddress());

				TO_CL4(CL, CL4);
				CL4->SetPipelineState1(COM_PTR_GET(StateObjects[0]));

				CL->ExecuteIndirect(COM_PTR_GET(IndirectBuffers[0].CommandSignature), 1, COM_PTR_GET(IndirectBuffers[0].Resource), 0, nullptr, 0);

			} PopulateEndRenderTargetCommand(CL, RT, COM_PTR_GET(SwapChainBackBuffers[i].Resource));
		} VERIFY_SUCCEEDED(CL->Close());
	}
	
private:
	struct Transform
	{
		DirectX::XMFLOAT4X4 Projection;
		DirectX::XMFLOAT4X4 View;
		DirectX::XMFLOAT4X4 InvProjection;
		DirectX::XMFLOAT4X4 InvView;
	};
	using Transform = struct Transform;
	Transform Tr;
};
#pragma endregion