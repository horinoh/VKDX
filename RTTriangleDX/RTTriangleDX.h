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

		const auto GCL = COM_PTR_GET(DirectCommandLists[0]);
		COM_PTR<ID3D12GraphicsCommandList4> GCL4;
		VERIFY_SUCCEEDED(GCL->QueryInterface(COM_PTR_UUIDOF_PUTVOID(GCL4)));
		const auto CA = COM_PTR_GET(DirectCommandAllocators[0]);
		const auto GCQ = COM_PTR_GET(GraphicsCommandQueue);

#pragma region BLAS_INPUT
		//!< �o�[�e�b�N�X�o�b�t�@ (VertexBuffer) ... �ʏ�ƈقȂ� D3D12_HEAP_TYPE_UPLOAD �ō쐬
		constexpr std::array Vertices = { DirectX::XMFLOAT3({ 0.0f, 0.5f, 0.0f }), DirectX::XMFLOAT3({ -0.5f, -0.5f, 0.0f }), DirectX::XMFLOAT3({ 0.5f, -0.5f, 0.0f }), };
		UploadResource VB;
		VB.Create(COM_PTR_GET(Device), TotalSizeOf(Vertices), data(Vertices));

		//!< �C���f�b�N�X�o�b�t�@ (IndexBuffer) ... �ʏ�ƈقȂ� D3D12_HEAP_TYPE_UPLOAD �ō쐬
		constexpr std::array Indices = { UINT32(0), UINT32(1), UINT32(2) };
		UploadResource IB;
		IB.Create(COM_PTR_GET(Device), TotalSizeOf(Indices), data(Indices));

		//!< �W�I���g�� (Geometry)
		const std::array RGDs = {
			D3D12_RAYTRACING_GEOMETRY_DESC({
				.Type = D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES,
				.Flags = D3D12_RAYTRACING_GEOMETRY_FLAG_OPAQUE,
				//!< �����ł̓g���C�A���O��
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
		//!< �C���v�b�g (Input)
		const D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS BRASI_Blas = {
			.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL, //!< �{�g�����x��
#ifdef USE_BLAS_COMPACTION
			.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE | D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_ALLOW_COMPACTION,
#else
			.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE,
#endif
			.NumDescs = static_cast<UINT>(size(RGDs)),
			.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY,
			.pGeometryDescs = data(RGDs), //!< �W�I���g�����w��
		};
#pragma endregion

#pragma region BLAS
		//!< �T�C�Y�擾 (Get sizes)
		D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO RASPI_Blas;
		Device5->GetRaytracingAccelerationStructurePrebuildInfo(&BRASI_Blas, &RASPI_Blas); 

#ifdef USE_BLAS_COMPACTION
		//!< �R���p�N�V�����T�C�Y���\�[�X���쐬
		ASCompaction Compaction;
		Compaction.Create(COM_PTR_GET(Device));

		//!< (�R���p�N�V�����T�C�Y���擾�ł���悤��)�����w�肵�� (�ꎞ)BLAS ���쐬
		BLAS Tmp;
		Tmp.Create(COM_PTR_GET(Device), RASPI_Blas.ResultDataMaxSizeInBytes)
			.ExecuteBuildCommand(COM_PTR_GET(Device), RASPI_Blas.ScratchDataSizeInBytes, BRASI_Blas, GCL, CA, GCQ, COM_PTR_GET(GraphicsFence), &Compaction);

		//!< �R���p�N�V�����T�C�Y���擾
		const UINT64 CompactedSizeInBytes = Compaction.GetSize();
		std::cout << "BLAS Compaction = " << RASPI_Blas.ResultDataMaxSizeInBytes << " -> " << CompactedSizeInBytes << std::endl;

		//!< �R���p�N�V�����T�C�Y�� (���K)BLAS ���쐬���� (�R�s�[����̂Ńr���h�͂��Ȃ���)
		BLASs.emplace_back().Create(COM_PTR_GET(Device), CompactedSizeInBytes)
			//!< �ꎞBLAS -> ���KBLAS �R�s�[�R�}���h�𔭍s���� 
			.ExecuteCopyCommand(GCL, CA, GCQ, COM_PTR_GET(GraphicsFence), COM_PTR_GET(Tmp.Resource));
#else
		//!< AS�쐬 (Create AS)
		BLASs.emplace_back().Create(COM_PTR_GET(Device), RASPI_Blas.ResultDataMaxSizeInBytes);
#endif
#pragma endregion

#pragma region TLAS_INPUT
		//!< �C���X�^���X�o�b�t�@ (InstanceBuffer) ... D3D12_HEAP_TYPE_UPLOAD �ō쐬
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
		UploadResource InsBuf;
		InsBuf.Create(COM_PTR_GET(Device), sizeof(RIDs), data(RIDs));

		//!< �C���v�b�g (Input)
		const D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS BRASI_Tlas = {
			.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL, //!< �g�b�v���x��
			.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE,
			.NumDescs = static_cast<UINT>(size(RIDs)),
			.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY,
			.InstanceDescs = InsBuf.Resource->GetGPUVirtualAddress() //!< �C���X�^���X���w��
		};
#pragma endregion

#pragma region TLAS
		//!< �T�C�Y�擾 (Get sizes)
		D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO RASPI_Tlas;
		Device5->GetRaytracingAccelerationStructurePrebuildInfo(&BRASI_Tlas, &RASPI_Tlas);
		//!< AS�쐬 (Create AS)
		TLASs.emplace_back().Create(COM_PTR_GET(Device), RASPI_Tlas.ResultDataMaxSizeInBytes);
#pragma endregion

#pragma region SCRATCH
		ScratchBuffer Scratch;
#ifdef USE_BLAS_COMPACTION
		Scratch.Create(COM_PTR_GET(Device), RASPI_Tlas.ScratchDataSizeInBytes);
#else
		Scratch.Create(COM_PTR_GET(Device), std::max(RASPI_Blas.ScratchDataSizeInBytes, RASPI_Tlas.ScratchDataSizeInBytes));
#endif
#pragma endregion

		VERIFY_SUCCEEDED(GCL->Reset(CA, nullptr)); {
#ifndef USE_BLAS_COMPACTION
			BLASs.back().PopulateBuildCommand(BRASI_Blas, GCL, COM_PTR_GET(Scratch.Resource));
#endif
			//!< TLAS �̃r���h���ɂ� BLAS �̃r���h(�R�s�[)���������Ă���K�v������̂Ńo���A
			BLASs.back().PopulateBarrierCommand(GCL);

			TLASs.back().PopulateBuildCommand(BRASI_Tlas, GCL, COM_PTR_GET(Scratch.Resource));
		} VERIFY_SUCCEEDED(GCL->Close());
		DX::ExecuteAndWait(GCQ, static_cast<ID3D12CommandList*>(GCL), COM_PTR_GET(GraphicsFence));
	}
	virtual void CreatePipelineState() override {
		if (!HasRaytracingSupport(COM_PTR_GET(Device))) { return; }

#pragma region STATE_OBJECT
		//!< �O���[�o�����[�g�V�O�l�`�� (Global root signature)
		const D3D12_GLOBAL_ROOT_SIGNATURE GRS = { .pGlobalRootSignature = COM_PTR_GET(RootSignatures[0]) };

		//!< �����ł� (VK�ɍ��킹��) �V�F�[�_�[�t�@�C���𕪂��Ă���
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

		//!< �q�b�g�O���[�v AnyHit, ClosestHit, Intersection ��3����Ȃ�A�����ł� D3D12_HIT_GROUP_TYPE_TRIANGLES �Ȃ̂ŁAClosestHit �݂̂��g�p
		//!< �q�b�g�O���[�v���̃V�F�[�_�͓������[�J�����[�g�V�O�l�`�����g�p���� 
		constexpr D3D12_HIT_GROUP_DESC HGD = {
			.HitGroupExport = TEXT("HitGroup"),
			.Type = D3D12_HIT_GROUP_TYPE_TRIANGLES,
			.AnyHitShaderImport = nullptr, .ClosestHitShaderImport = TEXT("OnClosestHit"), .IntersectionShaderImport = nullptr
		};

		//!< �V�F�[�_���A�y�C���[�h��A�g���r���[�g�T�C�Y�̎w��
		constexpr D3D12_RAYTRACING_SHADER_CONFIG RSC = {
			.MaxPayloadSizeInBytes = sizeof(DirectX::XMFLOAT3), //!< Payload �̃T�C�Y �����ł� HLSL���� struct Payload { float3 Color; } ���g�p���邽�� XMFLOAT3
			.MaxAttributeSizeInBytes = sizeof(DirectX::XMFLOAT2) //!< �����ł� HLSL���� struct BuiltInTriangleIntersectionAttributes { float2 barycentrics; } ���g�p���邽�� XMFLOAT2
		};

		//!< ���C�g���[�V���O�ċA�Ăяo���\�Ȓi�� [0, 31]
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

		VERIFY_SUCCEEDED(Device5->CreateStateObject(&SOD, COM_PTR_UUIDOF_PUTVOID(StateObjects.emplace_back())));
#pragma endregion
	}
	virtual void CreateShaderTable() override {
		COM_PTR<ID3D12StateObjectProperties> SOP;
		VERIFY_SUCCEEDED(StateObjects.back()->QueryInterface(COM_PTR_UUIDOF_PUTVOID(SOP)));

		//!< ���R�[�h�T�C�Y = �V�F�[�_���ʎq�T�C�Y + ���[�J�����[�g�����T�C�Y(�����ł͖��g�p�Ȃ̂�0) ���A���C��(D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT)��������
		constexpr auto RgenStride = Cmn::RoundUp(D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES + 0, D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT);
		//!< �e�[�u���T�C�Y = ���R�[�h�� * ���R�[�h�T�C�Y
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
		//!< �C���_�C���N�g�o�b�t�@ (IndirectBuffer)
		//!< DX �ł� ShaderTable ���܂߂邽�߁A���̎��_�łȂ��ƍ��Ȃ�
		const auto DRD = D3D12_DISPATCH_RAYS_DESC({
			.RayGenerationShaderRecord = D3D12_GPU_VIRTUAL_ADDRESS_RANGE({.StartAddress = ShaderTables[0].Range.StartAddress, .SizeInBytes = ShaderTables[0].Range.SizeInBytes }),
			.MissShaderTable = ShaderTables[1].Range,
			.HitGroupTable = ShaderTables[2].Range,
			.CallableShaderTable = D3D12_GPU_VIRTUAL_ADDRESS_RANGE_AND_STRIDE({.StartAddress = D3D12_GPU_VIRTUAL_ADDRESS(0), .SizeInBytes = 0, .StrideInBytes = 0}),
			.Width = static_cast<UINT>(GetClientRectWidth()), .Height = static_cast<UINT>(GetClientRectHeight()), .Depth = 1
		});
		IndirectBuffers.emplace_back().Create(COM_PTR_GET(Device), DRD).ExecuteCopyCommand(COM_PTR_GET(Device), COM_PTR_GET(DirectCommandAllocators[0]), COM_PTR_GET(DirectCommandLists[0]), COM_PTR_GET(GraphicsCommandQueue), COM_PTR_GET(GraphicsFence), sizeof(DRD), &DRD);
#endif
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
			} PopulateEndRenderTargetCommand(GCL, RT, COM_PTR_GET(SwapChainResources[i]));
		} VERIFY_SUCCEEDED(GCL->Close());
	}
};
#pragma endregion