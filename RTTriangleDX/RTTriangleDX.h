#pragma once

#include "resource.h"

#pragma region Code
#include "../DXRT.h"

class RTTriangleDX : public DXRT
{
private:
	using Super = DXRT;
public:
	RTTriangleDX() : Super() {}
	virtual ~RTTriangleDX() {}

	virtual void CreateGeometry() override {
		if (!HasRaytracingSupport(COM_PTR_GET(Device))) { return; }

		COM_PTR<ID3D12Device5> Device5;
		VERIFY_SUCCEEDED(Device->QueryInterface(COM_PTR_UUIDOF_PUTVOID(Device5)));

		const auto GCL = COM_PTR_GET(GraphicsCommandLists[0]);
		const auto CA = COM_PTR_GET(CommandAllocators[0]);
		const auto GCQ = COM_PTR_GET(GraphicsCommandQueue);

#pragma region BLAS_INPUT
		//!< バーテックスバッファ (VertexBuffer) ... 通常と異なり D3D12_HEAP_TYPE_UPLOAD で作成
		constexpr std::array Vertices = { DirectX::XMFLOAT3({ 0.0f, 0.5f, 0.0f }), DirectX::XMFLOAT3({ -0.5f, -0.5f, 0.0f }), DirectX::XMFLOAT3({ 0.5f, -0.5f, 0.0f }), };
		ResourceBase VertBuf;
		VertBuf.Create(COM_PTR_GET(Device), sizeof(Vertices), D3D12_HEAP_TYPE_UPLOAD, data(Vertices));

		//!< インデックスバッファ (IndexBuffer) ... 通常と異なり D3D12_HEAP_TYPE_UPLOAD で作成
		constexpr std::array Indices = { UINT32(0), UINT32(1), UINT32(2) };
		ResourceBase IndBuf;
		IndBuf.Create(COM_PTR_GET(Device), sizeof(Indices), D3D12_HEAP_TYPE_UPLOAD, data(Indices));

		//!< ジオメトリ (Geometry)
		const std::array RGDs = {
			D3D12_RAYTRACING_GEOMETRY_DESC({
				.Type = D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES,
				.Flags = D3D12_RAYTRACING_GEOMETRY_FLAG_OPAQUE,
				//!< ここではトライアングル
				.Triangles = D3D12_RAYTRACING_GEOMETRY_TRIANGLES_DESC({
					.Transform3x4 = 0,
					.IndexFormat = DXGI_FORMAT_R32_UINT,
					.VertexFormat = DXGI_FORMAT_R32G32B32_FLOAT,
					.IndexCount = static_cast<UINT>(size(Indices)),
					.VertexCount = static_cast<UINT>(size(Vertices)),
					.IndexBuffer = IndBuf.Resource->GetGPUVirtualAddress(),
					.VertexBuffer = D3D12_GPU_VIRTUAL_ADDRESS_AND_STRIDE({.StartAddress = VertBuf.Resource->GetGPUVirtualAddress(), .StrideInBytes = sizeof(Vertices[0]) }),
				})
			}),
		};
		//!< インプット (Input)
		const D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS BRASI_Blas = {
			.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL, //!< ボトムレベル
#ifdef USE_BLAS_COMPACTION
			.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE | D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_ALLOW_COMPACTION,
#else
			.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE,
#endif
			.NumDescs = static_cast<UINT>(size(RGDs)),
			.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY,
			.pGeometryDescs = data(RGDs), //!< ジオメトリを指定
		};
#pragma endregion

#pragma region BLAS_AND_SCRATCH
		ScratchBuffer Scratch_Blas;
		{
			//!< サイズ取得 (Get sizes)
			D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO RASPI;
			Device5->GetRaytracingAccelerationStructurePrebuildInfo(&BRASI_Blas, &RASPI);

#ifdef USE_BLAS_COMPACTION
			//!< コンパクションサイズリソースを作成
			ASCompaction Compaction;
			Compaction.Create(COM_PTR_GET(Device));

			//!< (コンパクションサイズを取得できるように)引数指定して (一時)BLAS を作成
			BLAS Tmp;
			Tmp.Create(COM_PTR_GET(Device), RASPI.ResultDataMaxSizeInBytes)
				.ExecuteBuildCommand(COM_PTR_GET(Device), RASPI.ScratchDataSizeInBytes, BRASI_Blas, GCL, CA, GCQ, COM_PTR_GET(Fence), &Compaction);

			//!< コンパクションサイズを取得
			const UINT64 CompactedSizeInBytes = Compaction.GetSize();
			std::cout << "BLAS Compaction = " << RASPI.ResultDataMaxSizeInBytes << " -> " << CompactedSizeInBytes << std::endl;

			//!< コンパクションサイズで (正規)BLAS を作成する (コピーするのでビルドはしないよ)
			BLASs.emplace_back().Create(COM_PTR_GET(Device), CompactedSizeInBytes)
				//!< 一時BLAS -> 正規BLAS コピーコマンドを発行する 
				.ExecuteCopyCommand(GCL, CA, GCQ, COM_PTR_GET(Fence), COM_PTR_GET(Tmp.Resource));

			//!< AS作成、ビルド (Create and build AS)
			//BLASs.emplace_back().Create(COM_PTR_GET(Device), RASPI.ResultDataMaxSizeInBytes).ExecuteBuildCommand(COM_PTR_GET(Device), RASPI.ScratchDataSizeInBytes, BRASI_Blas, GCL, CA, GCQ, COM_PTR_GET(Fence));
#else
			//!< AS、スクラッチ作成 (Create AS and scratch)
			BLASs.emplace_back().Create(COM_PTR_GET(Device), RASPI.ResultDataMaxSizeInBytes);
			Scratch_Blas.Create(COM_PTR_GET(Device), RASPI.ScratchDataSizeInBytes);
#endif
		}
#pragma endregion

#pragma region TLAS_INPUT
		//!< インスタンスバッファ (InstanceBuffer) ... D3D12_HEAP_TYPE_UPLOAD で作成
		const std::array RIDs = {
			D3D12_RAYTRACING_INSTANCE_DESC({
				.Transform = {{ 1.0f, 0.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f, 0.0f }, {0.0f, 0.0f, 1.0f, 0.0f}},
				.InstanceID = 0,
				.InstanceMask = 0xff,
				.InstanceContributionToHitGroupIndex = 0,
				.Flags = D3D12_RAYTRACING_INSTANCE_FLAG_TRIANGLE_FRONT_COUNTERCLOCKWISE,
				.AccelerationStructure = BLASs.back().Resource->GetGPUVirtualAddress()
			})
		};
		ResourceBase InsBuf;
		InsBuf.Create(COM_PTR_GET(Device), sizeof(RIDs), D3D12_HEAP_TYPE_UPLOAD, data(RIDs));

		//!< インプット (Input)
		const D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS BRASI_Tlas = {
			.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL, //!< トップレベル
			.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE,
			.NumDescs = static_cast<UINT>(size(RIDs)),
			.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY,
			.InstanceDescs = InsBuf.Resource->GetGPUVirtualAddress() //!< インスタンスを指定
		};
#pragma endregion

#pragma region TLAS_AND_SCRATCH
		ScratchBuffer Scratch_Tlas;
		{
			//!< サイズ取得 (Get sizes)
			D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO RASPI;
			Device5->GetRaytracingAccelerationStructurePrebuildInfo(&BRASI_Tlas, &RASPI);

#ifdef USE_BLAS_COMPACTION
			//!< AS作成、ビルド (Create and build AS)
			TLASs.emplace_back().Create(COM_PTR_GET(Device), RASPI.ResultDataMaxSizeInBytes).ExecuteBuildCommand(COM_PTR_GET(Device), RASPI.ScratchDataSizeInBytes, BRASI_Tlas, GCL, CA, GCQ, COM_PTR_GET(Fence));
#else
			//!< AS、スクラッチ作成 (Create AS and scratch)
			TLASs.emplace_back().Create(COM_PTR_GET(Device), RASPI.ResultDataMaxSizeInBytes);
			Scratch_Tlas.Create(COM_PTR_GET(Device), RASPI.ScratchDataSizeInBytes);
#endif
		}
#pragma endregion

#ifndef USE_BLAS_COMPACTION
		VERIFY_SUCCEEDED(GCL->Reset(CA, nullptr)); {
			BLASs.back().PopulateBuildCommand(BRASI_Blas, GCL, COM_PTR_GET(Scratch_Blas.Resource));
			//!< TLAS のビルド時には BLAS のビルドが完了している必要がある
			BLASs.back().PopulateBarrierCommand(GCL);
			TLASs.back().PopulateBuildCommand(BRASI_Tlas, GCL, COM_PTR_GET(Scratch_Tlas.Resource));
		} VERIFY_SUCCEEDED(GCL->Close());
		DX::ExecuteAndWait(GCQ, static_cast<ID3D12CommandList*>(GCL), COM_PTR_GET(Fence));
#endif
	}
	virtual void CreatePipelineState() override {
		if (!HasRaytracingSupport(COM_PTR_GET(Device))) { return; }

#pragma region STATE_OBJECT
		//!< グローバルルートシグネチャ (Global root signature)
		const D3D12_GLOBAL_ROOT_SIGNATURE GRS = { .pGlobalRootSignature = COM_PTR_GET(RootSignatures[0]) };

		//!< ここでは (VKに合わせて) シェーダーファイルを分けている
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

		//!< ヒットグループ AnyHit, ClosestHit, Intersection の3つからなる、ここでは D3D12_HIT_GROUP_TYPE_TRIANGLES なので、ClosestHit のみを使用
		//!< ヒットグループ内のシェーダは同じローカルルートシグネチャを使用する 
		constexpr D3D12_HIT_GROUP_DESC HGD = {
			.HitGroupExport = TEXT("HitGroup"),
			.Type = D3D12_HIT_GROUP_TYPE_TRIANGLES,
			.AnyHitShaderImport = nullptr, .ClosestHitShaderImport = TEXT("OnClosestHit"), .IntersectionShaderImport = nullptr
		};

		//!< シェーダ内、ペイロードやアトリビュートサイズの指定
		constexpr D3D12_RAYTRACING_SHADER_CONFIG RSC = {
			.MaxPayloadSizeInBytes = sizeof(DirectX::XMFLOAT3), //!< Payload のサイズ ここでは struct Payload { float3 Color; } を使用するため XMFLOAT3
			.MaxAttributeSizeInBytes = sizeof(DirectX::XMFLOAT2) //!< ここでは struct BuiltInTriangleIntersectionAttributes { float2 barycentrics; } を使用するため XMFLOAT2
		};

		//!< レイトレーシング再帰呼び出し可能な段数
		constexpr D3D12_RAYTRACING_PIPELINE_CONFIG RPC = { .MaxTraceRecursionDepth = 1 };

		constexpr std::array SSs = {
			D3D12_STATE_SUBOBJECT({.Type = D3D12_STATE_SUBOBJECT_TYPE_GLOBAL_ROOT_SIGNATURE, .pDesc = &GRS }),
			D3D12_STATE_SUBOBJECT({.Type = D3D12_STATE_SUBOBJECT_TYPE_DXIL_LIBRARY, .pDesc = &DLD_rgen }),
			D3D12_STATE_SUBOBJECT({.Type = D3D12_STATE_SUBOBJECT_TYPE_DXIL_LIBRARY, .pDesc = &DLD_miss }),
			D3D12_STATE_SUBOBJECT({.Type = D3D12_STATE_SUBOBJECT_TYPE_DXIL_LIBRARY, .pDesc = &DLD_rchit }),
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
	}
	virtual void CreateShaderTable() override {
		COM_PTR<ID3D12StateObjectProperties> SOP;
		VERIFY_SUCCEEDED(StateObjects.back()->QueryInterface(COM_PTR_UUIDOF_PUTVOID(SOP)));

		//!< レコードサイズ = シェーダ識別子サイズ + ローカルルート引数サイズ(ここでは未使用なので0) をアライン(D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT)したもの
		constexpr auto RgenStride = Cmn::RoundUp(D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES + 0, D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT);
		//!< テーブルサイズ = レコード数 * レコードサイズ
		constexpr auto RgenSize = 1 * RgenStride;

		constexpr auto MissStride = Cmn::RoundUp(D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES + 0, D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT);
		constexpr auto MissSize = 1 * MissStride;

		constexpr auto RchitStride = Cmn::RoundUp(D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES + 0, D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT);
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
			} ShaderTables.back().Unmap();
		}

#ifdef USE_INDIRECT
		//!< インダイレクトバッファ (IndirectBuffer)
		//!< DX では ShaderTable を含めるため、この時点でないと作れない
		const auto DRD = D3D12_DISPATCH_RAYS_DESC({
			.RayGenerationShaderRecord = D3D12_GPU_VIRTUAL_ADDRESS_RANGE({.StartAddress = ShaderTables[0].Range.StartAddress, .SizeInBytes = ShaderTables[0].Range.SizeInBytes }),
			.MissShaderTable = ShaderTables[1].Range,
			.HitGroupTable = ShaderTables[2].Range,
			.CallableShaderTable = D3D12_GPU_VIRTUAL_ADDRESS_RANGE_AND_STRIDE({.StartAddress = D3D12_GPU_VIRTUAL_ADDRESS(0), .SizeInBytes = 0, .StrideInBytes = 0}),
			.Width = static_cast<UINT>(GetClientRectWidth()), .Height = static_cast<UINT>(GetClientRectHeight()), .Depth = 1
		});
		IndirectBuffers.emplace_back().Create(COM_PTR_GET(Device), DRD).ExecuteCopyCommand(COM_PTR_GET(Device), COM_PTR_GET(CommandAllocators[0]), COM_PTR_GET(GraphicsCommandLists[0]), COM_PTR_GET(GraphicsCommandQueue), COM_PTR_GET(Fence), sizeof(DRD), &DRD);
#endif
	}
	virtual void PopulateCommandList(const size_t i) override {
		if (!HasRaytracingSupport(COM_PTR_GET(Device))) { return; }

		const auto GCL = GraphicsCommandLists[i];
		const auto CA = COM_PTR_GET(CommandAllocators[0]);
		VERIFY_SUCCEEDED(GCL->Reset(CA, nullptr)); {
			PopulateBeginRenderTargetCommand(i); {
				GCL->SetComputeRootSignature(COM_PTR_GET(RootSignatures[0]));

				const std::array DHs = { COM_PTR_GET(CbvSrvUavDescriptorHeaps[0]) };
				GCL->SetDescriptorHeaps(static_cast<UINT>(size(DHs)), data(DHs));
				//!< [0] TLAS
				GCL->SetComputeRootDescriptorTable(0, CbvSrvUavGPUHandles.back()[0]);
				//!< [1] UAV
				GCL->SetComputeRootDescriptorTable(1, CbvSrvUavGPUHandles.back()[1]);

				COM_PTR<ID3D12GraphicsCommandList4> GCL4;
				VERIFY_SUCCEEDED(GCL->QueryInterface(COM_PTR_UUIDOF_PUTVOID(GCL4)));
				GCL4->SetPipelineState1(COM_PTR_GET(StateObjects[0]));

#ifdef USE_INDIRECT
				GCL->ExecuteIndirect(COM_PTR_GET(IndirectBuffers[0].CommandSignature), 1, COM_PTR_GET(IndirectBuffers[0].Resource), 0, nullptr, 0);
#else
				const auto DRD = D3D12_DISPATCH_RAYS_DESC({
				  .RayGenerationShaderRecord = D3D12_GPU_VIRTUAL_ADDRESS_RANGE({.StartAddress = ShaderTables[0].Range.StartAddress, .SizeInBytes = ShaderTables[0].Range.SizeInBytes }),
				  .MissShaderTable = ShaderTables[1].Range,
				  .HitGroupTable = ShaderTables[2].Range,
				  .CallableShaderTable = D3D12_GPU_VIRTUAL_ADDRESS_RANGE_AND_STRIDE({.StartAddress = D3D12_GPU_VIRTUAL_ADDRESS(0), .SizeInBytes = 0, .StrideInBytes = 0}),
				  .Width = static_cast<UINT>(GetClientRectWidth()), .Height = static_cast<UINT>(GetClientRectHeight()), .Depth = 1
					});
				GCL4->DispatchRays(&DRD);
#endif
			} PopulateEndRenderTargetCommand(i);
		} VERIFY_SUCCEEDED(GCL->Close());
	}
};
#pragma endregion