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
		auto Max = DirectX::XMFLOAT3((std::numeric_limits<float>::min)(), (std::numeric_limits<float>::min)(), (std::numeric_limits<float>::min)());
		auto Min = DirectX::XMFLOAT3((std::numeric_limits<float>::max)(), (std::numeric_limits<float>::max)(), (std::numeric_limits<float>::max)());
		std::cout << "PolygonCount = " << Mesh->GetPolygonCount() << std::endl;
		for (auto i = 0; i < Mesh->GetPolygonCount(); ++i) {
			for (auto j = 0; j < Mesh->GetPolygonSize(i); ++j) {
				Indices.emplace_back(i * Mesh->GetPolygonSize(i) + j);

				Vs.emplace_back(ToFloat3(Mesh->GetControlPoints()[Mesh->GetPolygonVertex(i, j)]));
				Max.x = std::max(Max.x, Vs.back().x);
				Max.y = std::max(Max.y, Vs.back().y);
				Max.z = std::max(Max.z, Vs.back().z);
				Min.x = std::min(Min.x, Vs.back().x);
				Min.y = std::min(Min.y, Vs.back().y);
				Min.z = std::min(Min.z, Vs.back().z);
			}
		}
		const auto Bound = std::max(std::max(Max.x - Min.x, Max.y - Min.y), Max.z - Min.z) * 1.0f;
		std::transform(begin(Vs), end(Vs), begin(Vs), [&](const DirectX::XMFLOAT3& rhs) { return DirectX::XMFLOAT3(rhs.x / Bound, (rhs.y - (Max.y - Min.y) * 0.5f) / Bound, (rhs.z - Min.z) / Bound); });

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

		std::wstring Path;
		if (FindDirectory("FBX", Path)) {
			//Load(ToString(Path) + "//bunny.FBX");
			Load(ToString(Path) + "//dragon.FBX");
		}

		COM_PTR<ID3D12Device5> Device5;
		VERIFY_SUCCEEDED(Device->QueryInterface(COM_PTR_UUIDOF_PUTVOID(Device5)));

		const auto GCL = COM_PTR_GET(DirectCommandLists[0]);
		const auto CA = COM_PTR_GET(DirectCommandAllocators[0]);
		const auto GCQ = COM_PTR_GET(GraphicsCommandQueue);

#pragma region BLAS_INPUT
		VertexBuffer.Create(COM_PTR_GET(Device), TotalSizeOf(Vertices), sizeof(Vertices[0])).ExecuteCopyCommand(COM_PTR_GET(Device), CA, GCL, GCQ, COM_PTR_GET(GraphicsFence), TotalSizeOf(Vertices), data(Vertices));
		IndexBuffer.Create(COM_PTR_GET(Device), TotalSizeOf(Indices), sizeof(Indices[0])).ExecuteCopyCommand(COM_PTR_GET(Device), CA, GCL, GCQ, COM_PTR_GET(GraphicsFence), TotalSizeOf(Indices), data(Indices));
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

#pragma region BLAS_AND_SCRATCH
		ScratchBuffer Scratch_Blas;
		{
			D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO RASPI;
			Device5->GetRaytracingAccelerationStructurePrebuildInfo(&BRASI_Blas, &RASPI);

			BLASs.emplace_back().Create(COM_PTR_GET(Device), RASPI.ResultDataMaxSizeInBytes);
			Scratch_Blas.Create(COM_PTR_GET(Device), RASPI.ScratchDataSizeInBytes);
		}
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
				.InstanceID = 0, 
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
		ResourceBase InsBuf;
		InsBuf.Create(COM_PTR_GET(Device), sizeof(RIDs), D3D12_HEAP_TYPE_UPLOAD, data(RIDs));
		
		const D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS BRASI_Tlas = {
			.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL,
			.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE,
			.NumDescs = static_cast<UINT>(size(RIDs)),
			.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY,
			.InstanceDescs = InsBuf.Resource->GetGPUVirtualAddress()
		};
#pragma endregion

#pragma region TLAS_AND_SCRATCH
		ScratchBuffer Scratch_Tlas;
		{
			D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO RASPI;
			Device5->GetRaytracingAccelerationStructurePrebuildInfo(&BRASI_Tlas, &RASPI);

			TLASs.emplace_back().Create(COM_PTR_GET(Device), RASPI.ResultDataMaxSizeInBytes);
			Scratch_Tlas.Create(COM_PTR_GET(Device), RASPI.ScratchDataSizeInBytes);
		}
#pragma endregion
	
		VERIFY_SUCCEEDED(GCL->Reset(CA, nullptr)); {
			BLASs.back().PopulateBuildCommand(BRASI_Blas, GCL, COM_PTR_GET(Scratch_Blas.Resource));
			BLASs.back().PopulateBarrierCommand(GCL);
			TLASs.back().PopulateBuildCommand(BRASI_Tlas, GCL, COM_PTR_GET(Scratch_Tlas.Resource));
		} VERIFY_SUCCEEDED(GCL->Close());
		DX::ExecuteAndWait(GCQ, static_cast<ID3D12CommandList*>(GCL), COM_PTR_GET(GraphicsFence));
	}
	virtual void CreateTexture() override {
		Super::CreateTexture();

		std::wstring Path;
		if (FindDirectory("DDS", Path)) {
			const auto CA = COM_PTR_GET(DirectCommandAllocators[0]);
			const auto GCL = COM_PTR_GET(DirectCommandLists[0]);
			DDSTextures.emplace_back().Create(COM_PTR_GET(Device), Path + TEXT("\\CubeMap\\ninomaru_teien.dds"))
				.ExecuteCopyCommand(COM_PTR_GET(Device), CA, GCL, COM_PTR_GET(GraphicsCommandQueue), COM_PTR_GET(GraphicsFence), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

			const auto RD = DDSTextures.back().Resource->GetDesc();
			DDSTextures.back().SRV = D3D12_SHADER_RESOURCE_VIEW_DESC({ .Format = RD.Format, .ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE, .Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING, .TextureCube = D3D12_TEXCUBE_SRV({.MostDetailedMip = 0, .MipLevels = RD.MipLevels, .ResourceMinLODClamp = 0.0f }), });
		}
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
		StaticSamplerDescs.emplace_back(D3D12_STATIC_SAMPLER_DESC({
			.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR,
			.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP, .AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP, .AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP,
			.MipLODBias = 0.0f,
			.MaxAnisotropy = 0,
			.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER,
			.BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE,
			.MinLOD = 0.0f, .MaxLOD = 1.0f,
			.ShaderRegister = 0, .RegisterSpace = 0, .ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL //!< register(s0, space0)
		}));
	}
	virtual void CreateRootSignature() override {
		if (!HasRaytracingSupport(COM_PTR_GET(Device))) { return; }

		//!< (グローバル)ルートシグネチャ (Global root signature) ... ここでは RegisterSpace = 0 とする
		{
			COM_PTR<ID3DBlob> Blob;
#ifdef USE_HLSL_ROOTSIGNATRUE
			GetRootSignaturePartFromShader(Blob, data(GetBasePath() + TEXT(".grs.cso")));
#else
			constexpr std::array DRs_Srv = {
				D3D12_DESCRIPTOR_RANGE({.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV, .NumDescriptors = 2, .BaseShaderRegister = 0, .RegisterSpace = 0, .OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND })
			};
			constexpr std::array DRs_Uav = {
				D3D12_DESCRIPTOR_RANGE({.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV, .NumDescriptors = 1, .BaseShaderRegister = 0, .RegisterSpace = 0, .OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND })
			};
			DX::SerializeRootSignature(Blob, {
				//!< SRV0, SRV1 (TLAS, CubeMap) ... register(t0, space0), register(t1, space0)
				D3D12_ROOT_PARAMETER({
					.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE, 
					.DescriptorTable = D3D12_ROOT_DESCRIPTOR_TABLE({.NumDescriptorRanges = static_cast<UINT>(size(DRs_Srv)), .pDescriptorRanges = data(DRs_Srv) }),
					.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL 
				}),
				//!< UAV0 (RenderTarget) ... register(u0, space0)
				D3D12_ROOT_PARAMETER({
					.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE, 
					.DescriptorTable = D3D12_ROOT_DESCRIPTOR_TABLE({.NumDescriptorRanges = static_cast<UINT>(size(DRs_Uav)), .pDescriptorRanges = data(DRs_Uav) }),
					.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL
				}),
				//!< CBV0 (CB) ... register(b0, space0)
				//!< SetComputeRootConstantBufferView() を使用する為、D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE ではなくて D3D12_ROOT_PARAMETER_TYPE_CBV を使用している
				D3D12_ROOT_PARAMETER({
					.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV,
					.Descriptor = D3D12_ROOT_DESCRIPTOR({ .ShaderRegister = 0, .RegisterSpace = 0 }),
					.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL
				}),
			}, {
				StaticSamplerDescs[0],
			}, D3D12_ROOT_SIGNATURE_FLAG_NONE);
#endif
			VERIFY_SUCCEEDED(Device->CreateRootSignature(0, Blob->GetBufferPointer(), Blob->GetBufferSize(), COM_PTR_UUIDOF_PUTVOID(RootSignatures.emplace_back())));
		}

#pragma region SHADER_RECORD
		//!< ローカルルートシグネチャ (Local root signature) ... ここでは RegisterSpace = 1 とする
		{
			COM_PTR<ID3DBlob> Blob;
#ifdef USE_HLSL_ROOTSIGNATRUE
			GetRootSignaturePartFromShader(Blob, data(GetBasePath() + TEXT(".lrs.cso")));
#else
			DX::SerializeRootSignature(Blob, {
				//!< SRV0 (VB) ... register(t0, space1)
				D3D12_ROOT_PARAMETER({
					.ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV,
					.Descriptor = D3D12_ROOT_DESCRIPTOR({.ShaderRegister = 0, .RegisterSpace = 1 }),
					.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL
				}),
				//!< SRV1 (IB) ... register(t1, space1)
				D3D12_ROOT_PARAMETER({
					.ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV,
					.Descriptor = D3D12_ROOT_DESCRIPTOR({.ShaderRegister = 1, .RegisterSpace = 1 }),
					.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL
				}),
				}, {}, D3D12_ROOT_SIGNATURE_FLAG_LOCAL_ROOT_SIGNATURE);
#endif
			VERIFY_SUCCEEDED(Device->CreateRootSignature(0, Blob->GetBufferPointer(), Blob->GetBufferSize(), COM_PTR_UUIDOF_PUTVOID(RootSignatures.emplace_back())));
		}
#pragma endregion
	}
	virtual void CreatePipelineState() override {
		if (!HasRaytracingSupport(COM_PTR_GET(Device))) { return; }

#pragma region STATE_OBJECT
		const D3D12_GLOBAL_ROOT_SIGNATURE GRS = { .pGlobalRootSignature = COM_PTR_GET(RootSignatures[0]) };

		const auto ShaderPath = GetBasePath();
		COM_PTR<ID3DBlob> SB_rgen;
		VERIFY_SUCCEEDED(D3DReadFileToBlob(data(ShaderPath + TEXT(".rgen.cso")), COM_PTR_PUT(SB_rgen)));
		COM_PTR<ID3DBlob> SB_miss;
		VERIFY_SUCCEEDED(D3DReadFileToBlob(data(ShaderPath + TEXT(".miss.cso")), COM_PTR_PUT(SB_miss)));
		COM_PTR<ID3DBlob> SB_rchit;
		VERIFY_SUCCEEDED(D3DReadFileToBlob(data(ShaderPath + TEXT(".rchit.cso")), COM_PTR_PUT(SB_rchit)));

		std::array EDs_rgen = { D3D12_EXPORT_DESC({.Name = TEXT("OnRayGeneration"), .ExportToRename = nullptr, .Flags = D3D12_EXPORT_FLAG_NONE }), };
		std::array EDs_miss = { D3D12_EXPORT_DESC({.Name = TEXT("OnMiss"), .ExportToRename = nullptr, .Flags = D3D12_EXPORT_FLAG_NONE }), };
		std::array EDs_rchit = { D3D12_EXPORT_DESC({.Name = TEXT("OnClosestHit"), .ExportToRename = nullptr, .Flags = D3D12_EXPORT_FLAG_NONE }), };

		const auto DLD_rgen = D3D12_DXIL_LIBRARY_DESC({
			.DXILLibrary = D3D12_SHADER_BYTECODE({.pShaderBytecode = SB_rgen->GetBufferPointer(), .BytecodeLength = SB_rgen->GetBufferSize() }),
			.NumExports = static_cast<UINT>(size(EDs_rgen)), .pExports = data(EDs_rgen)
		});
		const auto DLD_miss = D3D12_DXIL_LIBRARY_DESC({
			.DXILLibrary = D3D12_SHADER_BYTECODE({.pShaderBytecode = SB_miss->GetBufferPointer(), .BytecodeLength = SB_miss->GetBufferSize() }),
			.NumExports = static_cast<UINT>(size(EDs_miss)), .pExports = data(EDs_miss)
		});
		const auto DLD_rchit = D3D12_DXIL_LIBRARY_DESC({
			.DXILLibrary = D3D12_SHADER_BYTECODE({.pShaderBytecode = SB_rchit->GetBufferPointer(), .BytecodeLength = SB_rchit->GetBufferSize() }),
			.NumExports = static_cast<UINT>(size(EDs_rchit)), .pExports = data(EDs_rchit)
		});

		constexpr D3D12_HIT_GROUP_DESC HGD = {
			.HitGroupExport = TEXT("HitGroup"),
			.Type = D3D12_HIT_GROUP_TYPE_TRIANGLES,
			.AnyHitShaderImport = nullptr, .ClosestHitShaderImport = TEXT("OnClosestHit"), .IntersectionShaderImport = nullptr
		};

		constexpr D3D12_RAYTRACING_SHADER_CONFIG RSC = {
			.MaxPayloadSizeInBytes = static_cast<UINT>(sizeof(DirectX::XMFLOAT3)), //!< Payload のサイズ
			.MaxAttributeSizeInBytes = sizeof(DirectX::XMFLOAT2) //!< ここでは struct BuiltInTriangleIntersectionAttributes { float2 barycentrics; } を使用するため XMFLOAT2
		};

		constexpr D3D12_RAYTRACING_PIPELINE_CONFIG RPC = { .MaxTraceRecursionDepth = 31 }; //!< 最大再帰呼び出し回数を指定

		std::vector SSs = {
			D3D12_STATE_SUBOBJECT({.Type = D3D12_STATE_SUBOBJECT_TYPE_GLOBAL_ROOT_SIGNATURE, .pDesc = &GRS }),
			D3D12_STATE_SUBOBJECT({.Type = D3D12_STATE_SUBOBJECT_TYPE_DXIL_LIBRARY, .pDesc = &DLD_rgen }),
			D3D12_STATE_SUBOBJECT({.Type = D3D12_STATE_SUBOBJECT_TYPE_DXIL_LIBRARY, .pDesc = &DLD_miss }),
			D3D12_STATE_SUBOBJECT({.Type = D3D12_STATE_SUBOBJECT_TYPE_DXIL_LIBRARY, .pDesc = &DLD_rchit }),
			D3D12_STATE_SUBOBJECT({.Type = D3D12_STATE_SUBOBJECT_TYPE_HIT_GROUP, .pDesc = &HGD }),
			D3D12_STATE_SUBOBJECT({.Type = D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_SHADER_CONFIG, .pDesc = &RSC }),
			D3D12_STATE_SUBOBJECT({.Type = D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_PIPELINE_CONFIG, .pDesc = &RPC }),
		};
#pragma region SHADER_RECORD
		{
			const D3D12_LOCAL_ROOT_SIGNATURE LRS = { .pLocalRootSignature = COM_PTR_GET(RootSignatures[1]) };
			SSs.emplace_back(D3D12_STATE_SUBOBJECT({ .Type = D3D12_STATE_SUBOBJECT_TYPE_LOCAL_ROOT_SIGNATURE, .pDesc = &LRS }));
			//!< ローカルルートシグネチャとヒットグループの関連付け
			std::array Exports = { TEXT("HitGroup") };
			const D3D12_SUBOBJECT_TO_EXPORTS_ASSOCIATION STEA = { .pSubobjectToAssociate = &SSs.back(), .NumExports = static_cast<UINT>(size(Exports)), .pExports = data(Exports) };
			SSs.emplace_back(D3D12_STATE_SUBOBJECT({ .Type = D3D12_STATE_SUBOBJECT_TYPE_SUBOBJECT_TO_EXPORTS_ASSOCIATION, .pDesc = &STEA }));
		}
#pragma endregion
		const D3D12_STATE_OBJECT_DESC SOD = {
			.Type = D3D12_STATE_OBJECT_TYPE_RAYTRACING_PIPELINE,
			.NumSubobjects = static_cast<UINT>(size(SSs)), .pSubobjects = data(SSs)
		};
		
		COM_PTR<ID3D12Device5> Device5;
		VERIFY_SUCCEEDED(Device->QueryInterface(COM_PTR_UUIDOF_PUTVOID(Device5)));
		VERIFY_SUCCEEDED(Device5->CreateStateObject(&SOD, COM_PTR_UUIDOF_PUTVOID(StateObjects.emplace_back())));
#pragma endregion
	}
	virtual void CreateDescriptor() override {
		DXGI_SWAP_CHAIN_DESC1 SCD;
		SwapChain->GetDesc1(&SCD);
#pragma region SHADER_RECORD
		const D3D12_DESCRIPTOR_HEAP_DESC DHD = { .Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, .NumDescriptors = 3 + 2, .Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE, .NodeMask = 0};
#pragma endregion
		VERIFY_SUCCEEDED(Device->CreateDescriptorHeap(&DHD, COM_PTR_UUIDOF_PUTVOID(CbvSrvUavDescriptorHeaps.emplace_back())));

		CbvSrvUavGPUHandles.emplace_back();
		auto CDH = CbvSrvUavDescriptorHeaps[0]->GetCPUDescriptorHandleForHeapStart();
		auto GDH = CbvSrvUavDescriptorHeaps[0]->GetGPUDescriptorHandleForHeapStart();
		const auto IncSize = Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		//!< [0] SRV (TLAS)
		Device->CreateShaderResourceView(nullptr, &TLASs[0].SRV, CDH); 
		CbvSrvUavGPUHandles.back().emplace_back(GDH);
		CDH.ptr += IncSize;
		GDH.ptr += IncSize;
		//!< [1] SRV (CubeMap)
		Device->CreateShaderResourceView(COM_PTR_GET(DDSTextures[0].Resource), &DDSTextures[0].SRV, CDH); 
		CbvSrvUavGPUHandles.back().emplace_back(GDH);
		CDH.ptr += IncSize;
		GDH.ptr += IncSize;		
		//!< [2] UAV (RenderTarget)
		Device->CreateUnorderedAccessView(COM_PTR_GET(UnorderedAccessTextures[0].Resource), nullptr, &UnorderedAccessTextures[0].UAV, CDH); 
		CbvSrvUavGPUHandles.back().emplace_back(GDH); 
		CDH.ptr += IncSize;
		GDH.ptr += IncSize; 
		
#pragma region SHADER_RECORD
		//!< [3] SRV (VB)
		Device->CreateShaderResourceView(COM_PTR_GET(VertexBuffer.Resource), &VertexBuffer.SRV, CDH);
		CbvSrvUavGPUHandles.back().emplace_back(GDH);
		CDH.ptr += IncSize;
		GDH.ptr += IncSize;
		//!< [4] SRV (IB)
		Device->CreateShaderResourceView(COM_PTR_GET(IndexBuffer.Resource), &IndexBuffer.SRV, CDH);
		CbvSrvUavGPUHandles.back().emplace_back(GDH);
#pragma endregion

		//!< この時点で削除してしまって良い？
		//COM_PTR_RESET(VertexBuffer.Resource);
		//COM_PTR_RESET(IndexBuffer.Resource);
	}
	virtual void CreateShaderTable() override {
		COM_PTR<ID3D12StateObjectProperties> SOP;
		VERIFY_SUCCEEDED(StateObjects.back()->QueryInterface(COM_PTR_UUIDOF_PUTVOID(SOP)));

		constexpr auto RgenStride = Cmn::RoundUp(D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES + 0, D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT);
		constexpr auto RgenSize = 1 * RgenStride;

		constexpr auto MissStride = Cmn::RoundUp(D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES + 0, D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT);
		constexpr auto MissSize = 1 * MissStride;

#pragma region SHADER_RECORD
		constexpr auto GPUDescSize = sizeof(D3D12_GPU_DESCRIPTOR_HANDLE);
		constexpr auto RchitStride = Cmn::RoundUp(D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES + GPUDescSize * 2, D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT);
#pragma endregion
		constexpr auto RchitSize = 1 * RchitStride;

		ShaderTables.emplace_back().Create(COM_PTR_GET(Device), RgenSize, RgenStride); {
			auto Data = ShaderTables.back().Map(); {
				std::memcpy(Data, SOP->GetShaderIdentifier(TEXT("OnRayGeneration")), D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES);
			} ShaderTables.back().Unmap();
		}
		ShaderTables.emplace_back().Create(COM_PTR_GET(Device), MissSize, MissStride); {
			auto Data = ShaderTables.back().Map(); {
				std::memcpy(Data, SOP->GetShaderIdentifier(TEXT("OnMiss")), D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES);
			} ShaderTables.back().Unmap();
		}
		ShaderTables.emplace_back().Create(COM_PTR_GET(Device), RchitSize, RchitStride); {
			auto Data = ShaderTables.back().Map(); {
				std::memcpy(Data, SOP->GetShaderIdentifier(TEXT("HitGroup")), D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES);
				//!< GPUハンドルを書き込む
#pragma region SHADER_RECORD
				//!< [3] SRV (VB)
				std::memcpy(Data + D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES, &CbvSrvUavGPUHandles.back()[3], GPUDescSize);
				//!< [4] SRV (IB)
				std::memcpy(Data + D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES + GPUDescSize, &CbvSrvUavGPUHandles.back()[4], GPUDescSize);
#pragma endregion
			} ShaderTables.back().Unmap();
		}

		const auto DRD = D3D12_DISPATCH_RAYS_DESC({
			.RayGenerationShaderRecord = D3D12_GPU_VIRTUAL_ADDRESS_RANGE({.StartAddress = ShaderTables[0].Range.StartAddress, .SizeInBytes = ShaderTables[0].Range.SizeInBytes }),
			.MissShaderTable = ShaderTables[1].Range,
			.HitGroupTable = ShaderTables[2].Range,
			.CallableShaderTable = D3D12_GPU_VIRTUAL_ADDRESS_RANGE_AND_STRIDE({.StartAddress = 0, .SizeInBytes = 0, .StrideInBytes = 0 }),
			.Width = static_cast<UINT>(GetClientRectWidth()), .Height = static_cast<UINT>(GetClientRectHeight()), .Depth = 1
		});
		IndirectBuffers.emplace_back().Create(COM_PTR_GET(Device), DRD).ExecuteCopyCommand(COM_PTR_GET(Device), COM_PTR_GET(DirectCommandAllocators[0]), COM_PTR_GET(DirectCommandLists[0]), COM_PTR_GET(GraphicsCommandQueue), COM_PTR_GET(GraphicsFence), sizeof(DRD), &DRD);
	}

	virtual void PopulateCommandList(const size_t i) override {
		if (!HasRaytracingSupport(COM_PTR_GET(Device))) { return; }

		const auto GCL = COM_PTR_GET(DirectCommandLists[i]);
		const auto CA = COM_PTR_GET(DirectCommandAllocators[0]);
		const auto RT = COM_PTR_GET(UnorderedAccessTextures[0].Resource);
		VERIFY_SUCCEEDED(GCL->Reset(CA, nullptr)); {
			PopulateBeginRenderTargetCommand(GCL, RT); {

				GCL->SetComputeRootSignature(COM_PTR_GET(RootSignatures[0]));

				const std::array DHs = { COM_PTR_GET(CbvSrvUavDescriptorHeaps[0]) };
				GCL->SetDescriptorHeaps(static_cast<UINT>(size(DHs)), data(DHs));
				//!< [0] SRV(TLAS), [1] SRV(CubeMap)
				GCL->SetComputeRootDescriptorTable(0, CbvSrvUavGPUHandles.back()[0]);
				//!< [2] UAV
				GCL->SetComputeRootDescriptorTable(1, CbvSrvUavGPUHandles.back()[2]);
				
				//!< CBV (デスクリプタヒープとは別扱い)
				GCL->SetComputeRootConstantBufferView(2, ConstantBuffers[i].Resource->GetGPUVirtualAddress());

				COM_PTR<ID3D12GraphicsCommandList4> GCL4;
				VERIFY_SUCCEEDED(GCL->QueryInterface(COM_PTR_UUIDOF_PUTVOID(GCL4)));
				GCL4->SetPipelineState1(COM_PTR_GET(StateObjects[0]));

				GCL->ExecuteIndirect(COM_PTR_GET(IndirectBuffers[0].CommandSignature), 1, COM_PTR_GET(IndirectBuffers[0].Resource), 0, nullptr, 0);

			} PopulateEndRenderTargetCommand(GCL, RT, COM_PTR_GET(SwapChainResources[i]));
		} VERIFY_SUCCEEDED(GCL->Close());
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