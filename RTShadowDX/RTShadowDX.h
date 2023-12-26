#pragma once

#include "resource.h"

#pragma region Code
#include "../DXRT.h"

class RTShadowDX : public DXRT
{
private:
	using Super = DXRT;
public:
	RTShadowDX() : Super() {}
	virtual ~RTShadowDX() {}

	virtual void CreateGeometry() override {
		if (!HasRaytracingSupport(COM_PTR_GET(Device))) { return; }

		const auto CL = COM_PTR_GET(DirectCommandLists[0]);
		const auto CA = COM_PTR_GET(DirectCommandAllocators[0]);
		const auto GCQ = COM_PTR_GET(GraphicsCommandQueue);

#pragma region BLAS_INPUT
		constexpr std::array Vertices = { DirectX::XMFLOAT3({ 0.0f, 0.5f, 0.0f }), DirectX::XMFLOAT3({ -0.5f, -0.5f, 0.0f }), DirectX::XMFLOAT3({ 0.5f, -0.5f, 0.0f }), };
		UploadResource VB;
		VB.Create(COM_PTR_GET(Device), TotalSizeOf(Vertices), data(Vertices));

		constexpr std::array Indices = { UINT32(0), UINT32(1), UINT32(2) };
		UploadResource IB;
		IB.Create(COM_PTR_GET(Device), TotalSizeOf(Indices), data(Indices));

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
					.IndexBuffer = IB.Resource->GetGPUVirtualAddress(),
					.VertexBuffer = D3D12_GPU_VIRTUAL_ADDRESS_AND_STRIDE({.StartAddress = VB.Resource->GetGPUVirtualAddress(), .StrideInBytes = sizeof(Vertices[0]) }),
				})
			}),
		};
		const D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS BRASI_Blas = {
			.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL, //!< ボトムレベル
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
		DirectX::XMFLOAT3X4 Tri, Floor;
		DirectX::XMStoreFloat3x4(&Tri, DirectX::XMMatrixScaling(0.25f, 0.25f, 1.0f) * DirectX::XMMatrixRotationRollPitchYaw(DirectX::XMConvertToRadians(45), 0.0f, 0.0f) * DirectX::XMMatrixTranslation(0.0f, 0.75f, 0.0f));
		DirectX::XMStoreFloat3x4(&Floor, DirectX::XMMatrixScaling(2.0f, 2.0f, 1.0f) * DirectX::XMMatrixRotationRollPitchYaw(DirectX::XMConvertToRadians(80), 0.0f, 0.0f) * DirectX::XMMatrixTranslation(0.0f, -0.75f, 0.0f));
		const std::array RIDs = {
			D3D12_RAYTRACING_INSTANCE_DESC({
				.Transform = {	{ Tri.m[0][0], Tri.m[0][1], Tri.m[0][2], Tri.m[0][3] },
								{ Tri.m[1][0], Tri.m[1][1], Tri.m[1][2], Tri.m[1][3] },
								{ Tri.m[2][0], Tri.m[2][1], Tri.m[2][2], Tri.m[2][3] }, },
				.InstanceID = 0,
				.InstanceMask = 0xff,
				.InstanceContributionToHitGroupIndex = 0,
				.Flags = D3D12_RAYTRACING_INSTANCE_FLAG_TRIANGLE_FRONT_COUNTERCLOCKWISE,
				.AccelerationStructure = BLASs.back().Resource->GetGPUVirtualAddress()
			}),
			D3D12_RAYTRACING_INSTANCE_DESC({
				.Transform = {	{ Floor.m[0][0], Floor.m[0][1], Floor.m[0][2], Floor.m[0][3] },
								{ Floor.m[1][0], Floor.m[1][1], Floor.m[1][2], Floor.m[1][3] },
								{ Floor.m[2][0], Floor.m[2][1], Floor.m[2][2], Floor.m[2][3] }, },
				.InstanceID = 0,
				.InstanceMask = 0xff,
				.InstanceContributionToHitGroupIndex = 0,
				.Flags = D3D12_RAYTRACING_INSTANCE_FLAG_TRIANGLE_FRONT_COUNTERCLOCKWISE,
				.AccelerationStructure = BLASs.back().Resource->GetGPUVirtualAddress()
			})
		};
		UploadResource InsBuf;
		InsBuf.Create(COM_PTR_GET(Device), sizeof(RIDs), data(RIDs));

		const D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS BRASI_Tlas = {
			.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL, //!< トップレベル
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
	virtual void CreatePipelineState() override {
		if (!HasRaytracingSupport(COM_PTR_GET(Device))) { return; }

#pragma region STATE_OBJECT
		const D3D12_GLOBAL_ROOT_SIGNATURE GRS = { .pGlobalRootSignature = COM_PTR_GET(RootSignatures[0]) };

		COM_PTR<ID3DBlob> SB_Gen;
		VERIFY_SUCCEEDED(D3DReadFileToBlob(data(GetFilePath(".rgen.cso").wstring()), COM_PTR_PUT(SB_Gen)));
		COM_PTR<ID3DBlob> SB_Miss;
		VERIFY_SUCCEEDED(D3DReadFileToBlob(data(GetFilePath(".miss.cso").wstring()), COM_PTR_PUT(SB_Miss)));
		COM_PTR<ID3DBlob> SB_Miss1;
		VERIFY_SUCCEEDED(D3DReadFileToBlob(data(GetFilePath("_1.miss.cso").wstring()), COM_PTR_PUT(SB_Miss1)));
		COM_PTR<ID3DBlob> SB_Hit;
		VERIFY_SUCCEEDED(D3DReadFileToBlob(data(GetFilePath(".rchit.cso").wstring()), COM_PTR_PUT(SB_Hit)));

		std::array EDs_Gen = { D3D12_EXPORT_DESC({.Name = TEXT("OnRayGeneration"), .ExportToRename = nullptr, .Flags = D3D12_EXPORT_FLAG_NONE }), };
		std::array EDs_Miss = { D3D12_EXPORT_DESC({.Name = TEXT("OnMiss"), .ExportToRename = nullptr, .Flags = D3D12_EXPORT_FLAG_NONE }), };
		std::array EDs_Miss1 = { D3D12_EXPORT_DESC({.Name = TEXT("OnMiss_1"), .ExportToRename = nullptr, .Flags = D3D12_EXPORT_FLAG_NONE }), };
		std::array EDs_Hit = { D3D12_EXPORT_DESC({.Name = TEXT("OnClosestHit"), .ExportToRename = nullptr, .Flags = D3D12_EXPORT_FLAG_NONE }), };
		const auto DLD_Gen = D3D12_DXIL_LIBRARY_DESC({
			.DXILLibrary = D3D12_SHADER_BYTECODE({.pShaderBytecode = SB_Gen->GetBufferPointer(), .BytecodeLength = SB_Gen->GetBufferSize() }),
			.NumExports = static_cast<UINT>(size(EDs_Gen)), .pExports = data(EDs_Gen)
			});
		const auto DLD_Miss = D3D12_DXIL_LIBRARY_DESC({
			.DXILLibrary = D3D12_SHADER_BYTECODE({.pShaderBytecode = SB_Miss->GetBufferPointer(), .BytecodeLength = SB_Miss->GetBufferSize() }),
			.NumExports = static_cast<UINT>(size(EDs_Miss)), .pExports = data(EDs_Miss)
			});
		const auto DLD_Miss1 = D3D12_DXIL_LIBRARY_DESC({
			.DXILLibrary = D3D12_SHADER_BYTECODE({.pShaderBytecode = SB_Miss1->GetBufferPointer(), .BytecodeLength = SB_Miss1->GetBufferSize() }),
			.NumExports = static_cast<UINT>(size(EDs_Miss1)), .pExports = data(EDs_Miss1)
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
			.MaxPayloadSizeInBytes = sizeof(DirectX::XMFLOAT3) + sizeof(int), 
			.MaxAttributeSizeInBytes = sizeof(DirectX::XMFLOAT2)
		};

		constexpr D3D12_RAYTRACING_PIPELINE_CONFIG RPC = { .MaxTraceRecursionDepth = 2 }; //!< 2 にする

		const std::array SSs = {
			D3D12_STATE_SUBOBJECT({.Type = D3D12_STATE_SUBOBJECT_TYPE_GLOBAL_ROOT_SIGNATURE, .pDesc = &GRS }),
			D3D12_STATE_SUBOBJECT({.Type = D3D12_STATE_SUBOBJECT_TYPE_DXIL_LIBRARY, .pDesc = &DLD_Gen }),
			D3D12_STATE_SUBOBJECT({.Type = D3D12_STATE_SUBOBJECT_TYPE_DXIL_LIBRARY, .pDesc = &DLD_Miss }),
			D3D12_STATE_SUBOBJECT({.Type = D3D12_STATE_SUBOBJECT_TYPE_DXIL_LIBRARY, .pDesc = &DLD_Miss1 }),
			D3D12_STATE_SUBOBJECT({.Type = D3D12_STATE_SUBOBJECT_TYPE_DXIL_LIBRARY, .pDesc = &DLD_Hit }),
			D3D12_STATE_SUBOBJECT({.Type = D3D12_STATE_SUBOBJECT_TYPE_HIT_GROUP, .pDesc = &HGD }),
			D3D12_STATE_SUBOBJECT({.Type = D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_SHADER_CONFIG, .pDesc = &RSC }),
			D3D12_STATE_SUBOBJECT({.Type = D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_PIPELINE_CONFIG, .pDesc = &RPC }),
		};
		const D3D12_STATE_OBJECT_DESC SOD = {
			.Type = D3D12_STATE_OBJECT_TYPE_RAYTRACING_PIPELINE,
			.NumSubobjects = static_cast<UINT>(size(SSs)), .pSubobjects = data(SSs)
		};

		VERIFY_SUCCEEDED(Device5->CreateStateObject(&SOD, COM_PTR_UUIDOF_PUTVOID(StateObjects.emplace_back())));
#pragma endregion
	}
	virtual void CreateShaderTable() override {
		COM_PTR<ID3D12StateObjectProperties> SOP;
		VERIFY_SUCCEEDED(StateObjects.back()->QueryInterface(COM_PTR_UUIDOF_PUTVOID(SOP)));
		auto& ST = ShaderTables.emplace_back(); {
			constexpr auto MissCount = 1;
			constexpr auto HitCount = 2;

			constexpr auto GenRecordSize = 0;
			constexpr auto MissRecordSize = 0;
			constexpr auto HitRecordSize = 0;

			constexpr auto GenStride = Cmn::RoundUp(D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES + GenRecordSize, D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT);
			constexpr auto MissStride = Cmn::RoundUp(D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES + MissRecordSize, D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT);
			constexpr auto HitStride = Cmn::RoundUp(D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES + HitRecordSize, D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT);

			constexpr auto GenSize = Cmn::RoundUp(GenStride, D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT);
			constexpr auto MissSize = Cmn::RoundUp(MissCount * MissStride, D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT);
			constexpr auto HitSize = Cmn::RoundUp(HitCount * HitStride, D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT);

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
					const auto Count = HitCount;
					const auto& Range = ST.AddressRangeAndStrides[1]; {
						auto p = Data;
						for (auto i = 0; i < Count; ++i, p += Range.StrideInBytes) {
							std::memcpy(p, SOP->GetShaderIdentifier(TEXT("HitGroup")), D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES); //!< ヒットグループ作成時に指定したヒットグループ名を使用
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
				//!< [0] TLAS
				CL->SetComputeRootDescriptorTable(0, Handle[0]);
				//!< [1] UAV
				CL->SetComputeRootDescriptorTable(1, Handle[1]);

				TO_CL4(CL, CL4);
				CL4->SetPipelineState1(COM_PTR_GET(StateObjects[0]));

				CL->ExecuteIndirect(COM_PTR_GET(IndirectBuffers[0].CommandSignature), 1, COM_PTR_GET(IndirectBuffers[0].Resource), 0, nullptr, 0);
			} PopulateEndRenderTargetCommand(CL, RT, COM_PTR_GET(SwapChainResources[i]));
		} VERIFY_SUCCEEDED(CL->Close());
	}
};
#pragma endregion