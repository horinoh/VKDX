#pragma once

#include "resource.h"

#pragma region Code
#include "../DXExt.h"

//!< RayGeneration <- Shader Identifier 1
//!< Miss
//!< HitGroup <- Shader Identifier 2
//!<	ClosestHit
//!<	AnyHit
//!<	Intersection
//!< HitGroup...

//!< ShaderRecord
//!<	Shader Identifier -> 1
//!<	Local Root Argument
//!< ShaderRecord
//!<	Shader Identifier -> 2 
//!<	Local Root Argument
//!< ShaderRecord...  

class RayTracingDX : public DXExt
{
private:
	using Super = DXExt;
public:
	RayTracingDX() : Super() {}
	virtual ~RayTracingDX() {}

#pragma region RAYTRACING
	virtual void CreateGeometry() override {
		if (!HasRaytracingSupport(COM_PTR_GET(Device))) { return; }

		COM_PTR<ID3D12Device5> Device5;
		VERIFY_SUCCEEDED(Device->QueryInterface(COM_PTR_UUIDOF_PUTVOID(Device5)));

		const auto GCL = COM_PTR_GET(GraphicsCommandLists[0]);
		const auto CA = COM_PTR_GET(CommandAllocators[0]);
		const auto GCQ = COM_PTR_GET(GraphicsCommandQueue);

#pragma region BLAS
		{
			//!< バーテックスバッファ (VertexBuffer) ... 通常と異なり D3D12_HEAP_TYPE_UPLOAD で作成
			constexpr std::array Vertices = { DirectX::XMFLOAT3({ 0.0f, 0.5f, 0.0f }), DirectX::XMFLOAT3({ -0.5f, -0.5f, 0.0f }), DirectX::XMFLOAT3({ 0.5f, -0.5f, 0.0f }), };
			ResourceBase VB;
			VB.Create(COM_PTR_GET(Device), sizeof(Vertices), D3D12_HEAP_TYPE_UPLOAD, data(Vertices));

			//!< インデックスバッファ (IndexBuffer) ... 通常と異なり D3D12_HEAP_TYPE_UPLOAD で作成
			constexpr std::array Indices = { UINT32(0), UINT32(1), UINT32(2) };
			ResourceBase IB;
			IB.Create(COM_PTR_GET(Device), sizeof(Indices), D3D12_HEAP_TYPE_UPLOAD, data(Indices));

#pragma region AS
			{
				//!< ジオメトリ (Geometry)
				const std::array RGDs = {
					//!< ここではトライアングル
					D3D12_RAYTRACING_GEOMETRY_DESC({
						.Type = D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES,
						.Flags = D3D12_RAYTRACING_GEOMETRY_FLAG_OPAQUE,
						.Triangles = D3D12_RAYTRACING_GEOMETRY_TRIANGLES_DESC({
							.Transform3x4 = 0,
							.IndexFormat = DXGI_FORMAT_R32_UINT,
							.VertexFormat = DXGI_FORMAT_R32G32B32_FLOAT,
							.IndexCount = static_cast<UINT>(size(Indices)),
							.VertexCount = static_cast<UINT>(size(Vertices)),
							.IndexBuffer = IB.Resource->GetGPUVirtualAddress(),
							.VertexBuffer = VB.Resource->GetGPUVirtualAddress(),
						})
					}),
				};
				//!< ASインプット (ASInput)
				const D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS BRASI = {
					.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL, //!< ボトムレベル
					.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE,
					//.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_NONE,
					.NumDescs = static_cast<UINT>(size(RGDs)),
					.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY,
					.pGeometryDescs = data(RGDs), //!< ジオメトリを指定
				};
				//!< サイズ取得 (Get sizes)
				D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO RASPI;
				Device5->GetRaytracingAccelerationStructurePrebuildInfo(&BRASI, &RASPI);

				//!< AS作成、ビルド (Create and build AS)
				BLASs.emplace_back().Create(COM_PTR_GET(Device), RASPI.ResultDataMaxSizeInBytes).ExecuteBuildCommand(COM_PTR_GET(Device), RASPI.ScratchDataSizeInBytes, BRASI, GCL, CA, GCQ, COM_PTR_GET(Fence));
			}
#pragma endregion
		}
#pragma endregion

#pragma region TLAS
		{
			//!< インスタンスバッファ (InstanceBuffer) ... D3D12_HEAP_TYPE_UPLOAD で作成
			const std::array RIDs = {
				D3D12_RAYTRACING_INSTANCE_DESC({
					.Transform = {{ 1.0f, 0.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f, 0.0f }, {0.0f, 0.0f, 1.0f, 0.0f}},
					.InstanceID = 0,
					.InstanceMask = 0xff,
					.InstanceContributionToHitGroupIndex = 0,
					.Flags = D3D12_RAYTRACING_INSTANCE_FLAG_TRIANGLE_CULL_DISABLE,
					//.Flags = D3D12_RAYTRACING_INSTANCE_FLAG_TRIANGLE_FRONT_COUNTERCLOCKWISE,
					.AccelerationStructure = BLASs.back().Resource->GetGPUVirtualAddress()
				})
			};
			ResourceBase IB;
			IB.Create(COM_PTR_GET(Device), sizeof(RIDs), D3D12_HEAP_TYPE_UPLOAD, data(RIDs));

#pragma region AS
			{
				//!< ASインプット (ASInput)
				const D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS BRASI = {
					.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL, //!< トップレベル
					.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE,
					//.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_NONE,
					.NumDescs = static_cast<UINT>(size(RIDs)),
					.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY,
					.InstanceDescs = IB.Resource->GetGPUVirtualAddress() //!< インスタンスを指定
				};
				//!< サイズ取得 (Get sizes)
				D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO RASPI;
				Device5->GetRaytracingAccelerationStructurePrebuildInfo(&BRASI, &RASPI);

				//!< AS作成、ビルド (Create and build AS)
				TLASs.emplace_back().Create(COM_PTR_GET(Device), RASPI.ResultDataMaxSizeInBytes).ExecuteBuildCommand(COM_PTR_GET(Device), RASPI.ScratchDataSizeInBytes, BRASI, GCL, CA, GCQ, COM_PTR_GET(Fence));
			}
#pragma endregion
		}
#pragma endregion
	}
	virtual void CreateTexture() override {
		if (!HasRaytracingSupport(COM_PTR_GET(Device))) { return; }
		UnorderedAccessTextures.emplace_back().Create(COM_PTR_GET(Device), 1280, 720, 1, DXGI_FORMAT_R8G8B8A8_UNORM);
	}
	virtual void CreateRootSignature() override {
		if (!HasRaytracingSupport(COM_PTR_GET(Device))) { return; }

		//!< グローバルルートシグネチャ (Global root signature)
		{
			COM_PTR<ID3DBlob> Blob;
#ifdef USE_HLSL_ROOTSIGNATRUE
			GetRootSignaturePartFromShader(Blob, data(GetBasePath() + TEXT(".grs.cso")));
#else
			constexpr std::array DRs_Tlas = {
				//!< register(t0, space0)
				D3D12_DESCRIPTOR_RANGE({.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV, .NumDescriptors = 1, .BaseShaderRegister = 0, .RegisterSpace = 0, .OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND })
			};
			constexpr std::array DRs_Uav = {
				//!< register(u0, space0)
				D3D12_DESCRIPTOR_RANGE({.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV, .NumDescriptors = 1, .BaseShaderRegister = 0, .RegisterSpace = 0, .OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND })
			};
			DX::SerializeRootSignature(Blob, {
				//!< TLAS
				D3D12_ROOT_PARAMETER({
					.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE, 
					.DescriptorTable = D3D12_ROOT_DESCRIPTOR_TABLE({.NumDescriptorRanges = static_cast<UINT>(size(DRs_Tlas)), .pDescriptorRanges = data(DRs_Tlas) }),
					.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL 
				}),
				//!< UAV0
				D3D12_ROOT_PARAMETER({
					.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE, 
					.DescriptorTable = D3D12_ROOT_DESCRIPTOR_TABLE({.NumDescriptorRanges = static_cast<UINT>(size(DRs_Uav)), .pDescriptorRanges = data(DRs_Uav) }),
					.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL
				}),
				}, {}, D3D12_ROOT_SIGNATURE_FLAG_NONE);
#endif
			VERIFY_SUCCEEDED(Device->CreateRootSignature(0, Blob->GetBufferPointer(), Blob->GetBufferSize(), COM_PTR_UUIDOF_PUTVOID(RootSignatures.emplace_back())));
		}

#if 0
		//!< ローカルルートシグネチャ (Local root signature)
		{
			COM_PTR<ID3DBlob> Blob;
#ifdef USE_HLSL_ROOTSIGNATRUE
			GetRootSignaturePartFromShader(Blob, data(GetBasePath() + TEXT(".lrs.cso")));
#else
			DX::SerializeRootSignature(Blob, {}, {}, D3D12_ROOT_SIGNATURE_FLAG_LOCAL_ROOT_SIGNATURE);
#endif
			VERIFY_SUCCEEDED(Device->CreateRootSignature(0, Blob->GetBufferPointer(), Blob->GetBufferSize(), COM_PTR_UUIDOF_PUTVOID(RootSignatures.emplace_back())));
		}
#endif
	}
	virtual void CreatePipelineState() override {
		if (!HasRaytracingSupport(COM_PTR_GET(Device))) { return; }

#pragma region STATE_OBJECT
		//!< グローバルルートシグネチャ (Global root signature)
		const D3D12_GLOBAL_ROOT_SIGNATURE GRS = { .pGlobalRootSignature = COM_PTR_GET(RootSignatures[0]) };

		//!< ローカルルートシグネチャ (Local root signature)
		//const D3D12_LOCAL_ROOT_SIGNATURE LRS = { .pLocalRootSignature = COM_PTR_GET(RootSignatures[1]) };

		//!< ライブラリ
		const auto ShaderPath = GetBasePath();
		COM_PTR<ID3DBlob> ShaderBlob;
		VERIFY_SUCCEEDED(D3DReadFileToBlob(data(ShaderPath + TEXT(".rts.cso")), COM_PTR_PUT(ShaderBlob)));
		std::array EDs = {
		   D3D12_EXPORT_DESC({.Name = TEXT("OnRayGeneration"), .ExportToRename = nullptr, .Flags = D3D12_EXPORT_FLAG_NONE }),
		   D3D12_EXPORT_DESC({.Name = TEXT("OnClosestHit"), .ExportToRename = nullptr, .Flags = D3D12_EXPORT_FLAG_NONE }),
		   D3D12_EXPORT_DESC({.Name = TEXT("OnMiss"), .ExportToRename = nullptr, .Flags = D3D12_EXPORT_FLAG_NONE }),
		};
		const D3D12_DXIL_LIBRARY_DESC DLD = {
			.DXILLibrary = D3D12_SHADER_BYTECODE({.pShaderBytecode = ShaderBlob->GetBufferPointer(), .BytecodeLength = ShaderBlob->GetBufferSize() }),
			.NumExports = static_cast<UINT>(size(EDs)), .pExports = data(EDs)
		};

		//!< ヒットグループ AnyHit, ClosestHit, Intersection の3つからなる、ここでは D3D12_HIT_GROUP_TYPE_TRIANGLES なので、ClosestHit のみを使用
		//!< ヒットグループ内のシェーダは同じローカルルートシグネチャを使用する 
		constexpr D3D12_HIT_GROUP_DESC HGD = {
			.HitGroupExport = TEXT("MyHitGroup"),
			.Type = D3D12_HIT_GROUP_TYPE_TRIANGLES,
			.AnyHitShaderImport = nullptr, .ClosestHitShaderImport = TEXT("OnClosestHit"), .IntersectionShaderImport = nullptr
		};

		//!< シェーダ内、ペイロードやアトリビュートサイズの指定
		constexpr D3D12_RAYTRACING_SHADER_CONFIG RSC = {
			.MaxPayloadSizeInBytes = sizeof(DirectX::XMFLOAT4), //!< Payload のサイズ ここでは struct Payload { float3 Color; } を使用するため XMFLOAT3
			.MaxAttributeSizeInBytes = sizeof(DirectX::XMFLOAT2) //!< ここでは struct BuiltInTriangleIntersectionAttributes { float2 barycentrics; } を使用するため XMFLOAT2
		};

		//!< レイトレーシング再帰呼び出し可能な段数
		constexpr D3D12_RAYTRACING_PIPELINE_CONFIG RPC = { .MaxTraceRecursionDepth = 1 };

		constexpr std::array SSs = {
			D3D12_STATE_SUBOBJECT({.Type = D3D12_STATE_SUBOBJECT_TYPE_GLOBAL_ROOT_SIGNATURE, .pDesc = &GRS }),
			//D3D12_STATE_SUBOBJECT({.Type = D3D12_STATE_SUBOBJECT_TYPE_LOCAL_ROOT_SIGNATURE, .pDesc = &LRS }),
			//D3D12_STATE_SUBOBJECT({.Type = D3D12_STATE_SUBOBJECT_TYPE_SUBOBJECT_TO_EXPORTS_ASSOCIATION, .pDesc = &STEA }),
			D3D12_STATE_SUBOBJECT({.Type = D3D12_STATE_SUBOBJECT_TYPE_DXIL_LIBRARY, .pDesc = &DLD }),
			D3D12_STATE_SUBOBJECT({.Type = D3D12_STATE_SUBOBJECT_TYPE_HIT_GROUP, .pDesc = &HGD }),
			D3D12_STATE_SUBOBJECT({.Type = D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_SHADER_CONFIG, .pDesc = &RSC }),
			D3D12_STATE_SUBOBJECT({.Type = D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_PIPELINE_CONFIG, .pDesc = &RPC }),
		};
		constexpr D3D12_STATE_OBJECT_DESC SOD = {
			.Type = D3D12_STATE_OBJECT_TYPE_RAYTRACING_PIPELINE,
			.NumSubobjects = static_cast<UINT>(size(SSs)), .pSubobjects = data(SSs)
		};
		COM_PTR<ID3D12Device5> Device5;
		VERIFY_SUCCEEDED(Device->QueryInterface(COM_PTR_UUIDOF_PUTVOID(Device5)));
		VERIFY_SUCCEEDED(Device5->CreateStateObject(&SOD, COM_PTR_UUIDOF_PUTVOID(StateObjects.emplace_back())));
#pragma endregion

#pragma region SHADER_TABLE
		COM_PTR<ID3D12StateObjectProperties> SOP;
		VERIFY_SUCCEEDED(StateObjects.back()->QueryInterface(COM_PTR_UUIDOF_PUTVOID(SOP)));

		const auto RaygenAlignedSize = Cmn::RoundUp(1 * Cmn::RoundUp(D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES, D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT), D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT);
		const auto MissAlignedSize = Cmn::RoundUp(1 * Cmn::RoundUp(D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES, D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT), D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT);
		const auto HitAlienedSize = Cmn::RoundUp(1 * Cmn::RoundUp(D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES, D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT), D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT);
		const auto TotalSize = RaygenAlignedSize + MissAlignedSize + HitAlienedSize;

		//!< #DX_TODO 1つのバッファにまとめる?
		ShaderTables.emplace_back().Create(COM_PTR_GET(Device), RaygenAlignedSize, D3D12_HEAP_TYPE_UPLOAD, SOP->GetShaderIdentifier(TEXT("OnRayGeneration")));
		ShaderTables.emplace_back().Create(COM_PTR_GET(Device), MissAlignedSize, D3D12_HEAP_TYPE_UPLOAD, SOP->GetShaderIdentifier(TEXT("OnMiss")));
		ShaderTables.emplace_back().Create(COM_PTR_GET(Device), HitAlienedSize, D3D12_HEAP_TYPE_UPLOAD, SOP->GetShaderIdentifier(TEXT("MyHitGroup")));
#pragma endregion
	}
	virtual void CreateDescriptorHeap() override {
		const D3D12_DESCRIPTOR_HEAP_DESC DHD = { .Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, .NumDescriptors = 2, .Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE, .NodeMask = 0 };
		VERIFY_SUCCEEDED(Device->CreateDescriptorHeap(&DHD, COM_PTR_UUIDOF_PUTVOID(CbvSrvUavDescriptorHeaps.emplace_back())));
	}
	virtual void CreateDescriptorView() override {
		const auto DH = CbvSrvUavDescriptorHeaps[0];
		auto CDH = DH->GetCPUDescriptorHandleForHeapStart();
		Device->CreateShaderResourceView(COM_PTR_GET(TLASs[0].Resource), &TLASs[0].SRV, CDH); CDH.ptr += Device->GetDescriptorHandleIncrementSize(DH->GetDesc().Type);
		Device->CreateUnorderedAccessView(COM_PTR_GET(UnorderedAccessTextures[0].Resource), nullptr, &UnorderedAccessTextures[0].UAV, CDH); CDH.ptr += Device->GetDescriptorHandleIncrementSize(DH->GetDesc().Type);
	}
	virtual void PopulateCommandList(const size_t i) override;
#pragma endregion
};
#pragma endregion