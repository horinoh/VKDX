#pragma once

#include "resource.h"

#pragma region Code
#include "../DXRT.h"

class RTCallableDX : public DXRT
{
private:
	using Super = DXRT;
public:
	RTCallableDX() : Super() {}
	virtual ~RTCallableDX() {}

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
			.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL, //!< �{�g�����x��
			.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE,
			.NumDescs = static_cast<UINT>(size(RGDs)), //!< [HLSL] GeometryIndex() ([GLSL] gl_GeometryIndexEXT ����)
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
		//!< InstanceID								: 0==�s���͗l, 1==�c��, 2==���� (�����ł̓C���X�^���X���� CallableShader �̏o�������Ɏg�p)
		//!< InstanceContributionToHitGroupIndex	: 0==��, 1==�� (�����ł̓C���X�^���X���� HitShader �̏o�������Ɏg�p)
		const std::array RIDs = {
			#pragma region INSTANCES
			//!< �s����(����)
			D3D12_RAYTRACING_INSTANCE_DESC({
				.Transform = {
					{ 1.0f, 0.0f, 0.0f,  0.0f },
					{ 0.0f, 1.0f, 0.0f,  0.0f },
					{ 0.0f, 0.0f, 1.0f,  0.0f }
				},
				.InstanceID = 0, //!< [HLSL] InstanceID() ([GLSL] gl_InstanceCustomIndexEXT ����) (�����ł̓C���X�^���X���� CallableShader �̏o�������Ɏg�p)
				.InstanceMask = 0xff,
				.InstanceContributionToHitGroupIndex = 0, //!< �q�b�g�C���f�b�N�X (�����ł̓C���X�^���X���� HitShader �̏o�������Ɏg�p)
				.Flags = D3D12_RAYTRACING_INSTANCE_FLAG_TRIANGLE_FRONT_COUNTERCLOCKWISE,
				.AccelerationStructure = BLASs.back().Resource->GetGPUVirtualAddress()
			}),
			//!< �c��(��)
			D3D12_RAYTRACING_INSTANCE_DESC({
				.Transform = {
					{ 1.0f, 0.0f, 0.0f, -1.0f },
					{ 0.0f, 1.0f, 0.0f,  0.0f },
					{ 0.0f, 0.0f, 1.0f,  0.0f }
				},
				.InstanceID = 1, 
				.InstanceMask = 0xff,
				.InstanceContributionToHitGroupIndex = 0,
				.Flags = D3D12_RAYTRACING_INSTANCE_FLAG_TRIANGLE_FRONT_COUNTERCLOCKWISE,
				.AccelerationStructure = BLASs.back().Resource->GetGPUVirtualAddress()
			}),
			//!< ����(�E)
			D3D12_RAYTRACING_INSTANCE_DESC({
				.Transform = {
					{ 1.0f, 0.0f, 0.0f, 1.0f },
					{ 0.0f, 1.0f, 0.0f, 0.0f },
					{ 0.0f, 0.0f, 1.0f, 0.0f }
				},
				.InstanceID = 2,
				.InstanceMask = 0xff,
				.InstanceContributionToHitGroupIndex = 1,
				.Flags = D3D12_RAYTRACING_INSTANCE_FLAG_TRIANGLE_FRONT_COUNTERCLOCKWISE,
				.AccelerationStructure = BLASs.back().Resource->GetGPUVirtualAddress()
			})
			#pragma endregion
		};
		UploadResource InsBuf;
		InsBuf.Create(COM_PTR_GET(Device), sizeof(RIDs), data(RIDs));

		const D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS BRASI_Tlas = {
			.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL, //!< �g�b�v���x��
			.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE,
			.NumDescs = static_cast<UINT>(size(RIDs)),
			.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY,
			.InstanceDescs = InsBuf.Resource->GetGPUVirtualAddress() //!< �C���X�^���X���w��
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

		//!< �����ł� (VK�ɍ��킹��) �V�F�[�_�[�t�@�C���𕪂��Ă���
		COM_PTR<ID3DBlob> SB_Gen;
		VERIFY_SUCCEEDED(D3DReadFileToBlob(data(GetFilePath(".rgen.cso").wstring()), COM_PTR_PUT(SB_Gen)));
		COM_PTR<ID3DBlob> SB_Miss;
		VERIFY_SUCCEEDED(D3DReadFileToBlob(data(GetFilePath(".miss.cso").wstring()), COM_PTR_PUT(SB_Miss)));

		COM_PTR<ID3DBlob> SB_Hit0;
		VERIFY_SUCCEEDED(D3DReadFileToBlob(data(GetFilePath(".rchit.cso").wstring()), COM_PTR_PUT(SB_Hit0)));
		COM_PTR<ID3DBlob> SB_Hit1;
		VERIFY_SUCCEEDED(D3DReadFileToBlob(data(GetFilePath("_1.rchit.cso").wstring()), COM_PTR_PUT(SB_Hit1)));

		COM_PTR<ID3DBlob> SB_Call0;
		VERIFY_SUCCEEDED(D3DReadFileToBlob(data(GetFilePath(".rcall.cso").wstring()), COM_PTR_PUT(SB_Call0)));
		COM_PTR<ID3DBlob> SB_Call1;
		VERIFY_SUCCEEDED(D3DReadFileToBlob(data(GetFilePath("_1.rcall.cso").wstring()), COM_PTR_PUT(SB_Call1)));
		COM_PTR<ID3DBlob> SB_Call2;
		VERIFY_SUCCEEDED(D3DReadFileToBlob(data(GetFilePath("_2.rcall.cso").wstring()), COM_PTR_PUT(SB_Call2)));

		std::array EDs_Gen = { D3D12_EXPORT_DESC({.Name = TEXT("OnRayGeneration"), .ExportToRename = nullptr, .Flags = D3D12_EXPORT_FLAG_NONE }), };
		std::array EDs_Miss = { D3D12_EXPORT_DESC({.Name = TEXT("OnMiss"), .ExportToRename = nullptr, .Flags = D3D12_EXPORT_FLAG_NONE }), };

		std::array EDs_Hit0 = { D3D12_EXPORT_DESC({.Name = TEXT("OnClosestHit_0"), .ExportToRename = nullptr, .Flags = D3D12_EXPORT_FLAG_NONE }), };
		std::array EDs_Hit1 = { D3D12_EXPORT_DESC({.Name = TEXT("OnClosestHit_1"), .ExportToRename = nullptr, .Flags = D3D12_EXPORT_FLAG_NONE }), };

		std::array EDs_Call0 = { D3D12_EXPORT_DESC({.Name = TEXT("OnCallable_0"), .ExportToRename = nullptr, .Flags = D3D12_EXPORT_FLAG_NONE }), };
		std::array EDs_Call1 = { D3D12_EXPORT_DESC({.Name = TEXT("OnCallable_1"), .ExportToRename = nullptr, .Flags = D3D12_EXPORT_FLAG_NONE }), };
		std::array EDs_Call2 = { D3D12_EXPORT_DESC({.Name = TEXT("OnCallable_2"), .ExportToRename = nullptr, .Flags = D3D12_EXPORT_FLAG_NONE }), };

		const auto DLD_Gen = D3D12_DXIL_LIBRARY_DESC({
			.DXILLibrary = D3D12_SHADER_BYTECODE({.pShaderBytecode = SB_Gen->GetBufferPointer(), .BytecodeLength = SB_Gen->GetBufferSize() }),
			.NumExports = static_cast<UINT>(size(EDs_Gen)), .pExports = data(EDs_Gen)
			});
		const auto DLD_Miss = D3D12_DXIL_LIBRARY_DESC({
			.DXILLibrary = D3D12_SHADER_BYTECODE({.pShaderBytecode = SB_Miss->GetBufferPointer(), .BytecodeLength = SB_Miss->GetBufferSize() }),
			.NumExports = static_cast<UINT>(size(EDs_Miss)), .pExports = data(EDs_Miss)
			});

		const auto DLD_Hit0 = D3D12_DXIL_LIBRARY_DESC({
			.DXILLibrary = D3D12_SHADER_BYTECODE({.pShaderBytecode = SB_Hit0->GetBufferPointer(), .BytecodeLength = SB_Hit0->GetBufferSize() }),
			.NumExports = static_cast<UINT>(size(EDs_Hit0)), .pExports = data(EDs_Hit0)
			});
		const auto DLD_Hit1 = D3D12_DXIL_LIBRARY_DESC({
			.DXILLibrary = D3D12_SHADER_BYTECODE({.pShaderBytecode = SB_Hit1->GetBufferPointer(), .BytecodeLength = SB_Hit1->GetBufferSize() }),
			.NumExports = static_cast<UINT>(size(EDs_Hit1)), .pExports = data(EDs_Hit1)
			});

		const auto DLD_Call0 = D3D12_DXIL_LIBRARY_DESC({
			.DXILLibrary = D3D12_SHADER_BYTECODE({.pShaderBytecode = SB_Call0->GetBufferPointer(), .BytecodeLength = SB_Call0->GetBufferSize() }),
			.NumExports = static_cast<UINT>(size(EDs_Call0)), .pExports = data(EDs_Call0)
			});
		const auto DLD_Call1 = D3D12_DXIL_LIBRARY_DESC({
			.DXILLibrary = D3D12_SHADER_BYTECODE({.pShaderBytecode = SB_Call1->GetBufferPointer(), .BytecodeLength = SB_Call1->GetBufferSize() }),
			.NumExports = static_cast<UINT>(size(EDs_Call1)), .pExports = data(EDs_Call1)
			});
		const auto DLD_Call2 = D3D12_DXIL_LIBRARY_DESC({
			.DXILLibrary = D3D12_SHADER_BYTECODE({.pShaderBytecode = SB_Call2->GetBufferPointer(), .BytecodeLength = SB_Call2->GetBufferSize() }),
			.NumExports = static_cast<UINT>(size(EDs_Call2)), .pExports = data(EDs_Call2)
			});
#pragma endregion

		constexpr D3D12_HIT_GROUP_DESC HGD0 = {
			.HitGroupExport = TEXT("HitGroup_0"),
			.Type = D3D12_HIT_GROUP_TYPE_TRIANGLES,
			.AnyHitShaderImport = nullptr, .ClosestHitShaderImport = TEXT("OnClosestHit_0"), .IntersectionShaderImport = nullptr
		};
		constexpr D3D12_HIT_GROUP_DESC HGD1 = {
			.HitGroupExport = TEXT("HitGroup_1"),
			.Type = D3D12_HIT_GROUP_TYPE_TRIANGLES,
			.AnyHitShaderImport = nullptr, .ClosestHitShaderImport = TEXT("OnClosestHit_1"), .IntersectionShaderImport = nullptr
		};

		constexpr D3D12_RAYTRACING_SHADER_CONFIG RSC = {
			.MaxPayloadSizeInBytes = sizeof(DirectX::XMFLOAT3) + sizeof(int), //!< �����ł� struct PAYLOAD { float3 Color; int Recursive; } ���g�p���邽��
			.MaxAttributeSizeInBytes = sizeof(DirectX::XMFLOAT2) //!< �����ł� struct BuiltInTriangleIntersectionAttributes { float2 barycentrics; } ���g�p���邽��
		};

		constexpr D3D12_RAYTRACING_PIPELINE_CONFIG RPC = { .MaxTraceRecursionDepth = 1 };

		constexpr std::array SSs = {
			D3D12_STATE_SUBOBJECT({.Type = D3D12_STATE_SUBOBJECT_TYPE_GLOBAL_ROOT_SIGNATURE, .pDesc = &GRS }),
			D3D12_STATE_SUBOBJECT({.Type = D3D12_STATE_SUBOBJECT_TYPE_DXIL_LIBRARY, .pDesc = &DLD_Gen }),
			D3D12_STATE_SUBOBJECT({.Type = D3D12_STATE_SUBOBJECT_TYPE_DXIL_LIBRARY, .pDesc = &DLD_Miss }),

			D3D12_STATE_SUBOBJECT({.Type = D3D12_STATE_SUBOBJECT_TYPE_DXIL_LIBRARY, .pDesc = &DLD_Hit0 }),
			D3D12_STATE_SUBOBJECT({.Type = D3D12_STATE_SUBOBJECT_TYPE_DXIL_LIBRARY, .pDesc = &DLD_Hit1 }),

			D3D12_STATE_SUBOBJECT({.Type = D3D12_STATE_SUBOBJECT_TYPE_DXIL_LIBRARY, .pDesc = &DLD_Call0 }),
			D3D12_STATE_SUBOBJECT({.Type = D3D12_STATE_SUBOBJECT_TYPE_DXIL_LIBRARY, .pDesc = &DLD_Call1 }),
			D3D12_STATE_SUBOBJECT({.Type = D3D12_STATE_SUBOBJECT_TYPE_DXIL_LIBRARY, .pDesc = &DLD_Call2 }),

			D3D12_STATE_SUBOBJECT({.Type = D3D12_STATE_SUBOBJECT_TYPE_HIT_GROUP, .pDesc = &HGD0 }),
			D3D12_STATE_SUBOBJECT({.Type = D3D12_STATE_SUBOBJECT_TYPE_HIT_GROUP, .pDesc = &HGD1 }),
			D3D12_STATE_SUBOBJECT({.Type = D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_SHADER_CONFIG, .pDesc = &RSC }),
			D3D12_STATE_SUBOBJECT({.Type = D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_PIPELINE_CONFIG, .pDesc = &RPC }),
		};
		constexpr D3D12_STATE_OBJECT_DESC SOD = {
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
			//!< �e�O���[�v�̃n���h���̌� (Gen��1�Œ�)
			constexpr auto MissCount = 1;
			constexpr auto HitCount = 2;
			constexpr auto CallCount = 3;
			//!< �V�F�[�_���R�[�h�T�C�Y
			constexpr auto GenRecordSize = 0;
			constexpr auto MissRecordSize = 0;
			constexpr auto HitRecordSize = 0;
			constexpr auto CallRecordSize = 0;
			//!< �X�g���C�h
			constexpr auto GenStride = Cmn::RoundUp(D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES + GenRecordSize, D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT);
			constexpr auto MissStride = Cmn::RoundUp(D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES + MissRecordSize, D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT);
			constexpr auto HitStride = Cmn::RoundUp(D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES + HitRecordSize, D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT);
			constexpr auto CallStride = Cmn::RoundUp(D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES + CallRecordSize, D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT);
			//!< �T�C�Y
			constexpr auto GenSize = Cmn::RoundUp(GenStride, D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT);
			constexpr auto MissSize = Cmn::RoundUp(MissCount * MissStride, D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT);
			constexpr auto HitSize = Cmn::RoundUp(HitCount * HitStride, D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT);
			constexpr auto CallSize = Cmn::RoundUp(CallCount * CallStride, D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT);
			//!< �����W
			ST.AddressRange = D3D12_GPU_VIRTUAL_ADDRESS_RANGE({ .SizeInBytes = GenSize });
			ST.AddressRangeAndStrides[0] = D3D12_GPU_VIRTUAL_ADDRESS_RANGE_AND_STRIDE({ .SizeInBytes = MissSize, .StrideInBytes = MissStride });
			ST.AddressRangeAndStrides[1] = D3D12_GPU_VIRTUAL_ADDRESS_RANGE_AND_STRIDE({ .SizeInBytes = HitSize, .StrideInBytes = HitStride });
			ST.AddressRangeAndStrides[2] = D3D12_GPU_VIRTUAL_ADDRESS_RANGE_AND_STRIDE({ .SizeInBytes = CallSize, .StrideInBytes = CallStride });

			ST.Create(COM_PTR_GET(Device), GenSize + MissSize + HitSize + CallSize);
			auto MapData = ST.Map(); {
				auto Data = reinterpret_cast<std::byte*>(MapData);

				//!< �O���[�v (Gen)
				{
					const auto& Range = ST.AddressRange; {
						std::memcpy(Data, SOP->GetShaderIdentifier(TEXT("OnRayGeneration")), D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES);
						Data += Range.SizeInBytes;
					}
				}

				//!< �O���[�v (Miss)
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

				//!< �O���[�v (Hit)
				{
					//const auto Count = HitCount;
					const auto& Range = ST.AddressRangeAndStrides[1]; {
						auto p = Data;
						for (auto i = 0; i < HitCount; ++i, p += Range.StrideInBytes) {
							std::memcpy(p, SOP->GetShaderIdentifier(data(std::wstring(TEXT("HitGroup_")) + std::to_wstring(i))), D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES); //!< �q�b�g�O���[�v�쐬���Ɏw�肵���q�b�g�O���[�v�����g�p
						}
						Data += Range.SizeInBytes;
					}
				}

				//!< �O���[�v (Call)
				{
					//const auto Count = CallCount;
					const auto& Range = ST.AddressRangeAndStrides[2]; {
						auto p = Data;
						for (auto i = 0; i < CallCount; ++i, p += Range.StrideInBytes) {
							std::memcpy(p, SOP->GetShaderIdentifier(data(std::wstring(TEXT("OnCallable_")) + std::to_wstring(i))), D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES);
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
				const std::array DHs = { COM_PTR_GET(CbvSrvUavDescriptorHeaps[0]) };
				CL->SetDescriptorHeaps(static_cast<UINT>(size(DHs)), data(DHs)); 
				
				CL->SetComputeRootSignature(COM_PTR_GET(RootSignatures[0]));
				//!< [0] TLAS
				CL->SetComputeRootDescriptorTable(0, CbvSrvUavGPUHandles.back()[0]);
				//!< [1] UAV
				CL->SetComputeRootDescriptorTable(1, CbvSrvUavGPUHandles.back()[1]);

				TO_CL4(CL, CL4);
				CL4->SetPipelineState1(COM_PTR_GET(StateObjects[0]));

				CL->ExecuteIndirect(COM_PTR_GET(IndirectBuffers[0].CommandSignature), 1, COM_PTR_GET(IndirectBuffers[0].Resource), 0, nullptr, 0);

			} PopulateEndRenderTargetCommand(CL, RT, COM_PTR_GET(SwapChainResources[i]));
		} VERIFY_SUCCEEDED(CL->Close());
	}
};
#pragma endregion