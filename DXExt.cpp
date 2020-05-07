#include "DXExt.h"

void DXExt::CreateIndirectBuffer_Draw(const UINT IndexCount, const UINT InstanceCount)
{
	IndirectBufferResources.push_back(COM_PTR<ID3D12Resource>());

	const D3D12_DRAW_ARGUMENTS Source = { IndexCount, InstanceCount, 0, 0 };
	const auto Stride = sizeof(Source);
	const auto Size = static_cast<UINT32>(Stride * 1);
	CreateAndCopyToDefaultResource(IndirectBufferResources.back(), COM_PTR_GET(CommandAllocators[0]), COM_PTR_GET(GraphicsCommandLists[0]), Size, &Source);

	IndirectCommandSignatures.push_back(COM_PTR<ID3D12CommandSignature>());
	const std::array<D3D12_INDIRECT_ARGUMENT_DESC, 1> IADs = {
		{ D3D12_INDIRECT_ARGUMENT_TYPE_DRAW },
	};
	const D3D12_COMMAND_SIGNATURE_DESC CSD = {
		Stride,
		static_cast<const UINT>(IADs.size()), IADs.data(),
		0
	};
	//!< �p�C�v���C���̃o�C���f�B���O���X�V����悤�ȏꍇ�̓��[�g�V�O�l�`�����K�v�ADraw �� Dispatch �݂̂̏ꍇ��nullptr���w��ł���
	Device->CreateCommandSignature(&CSD, /*COM_PTR_GET(RootSignatures[0])*/nullptr, COM_PTR_UUIDOF_PUTVOID(IndirectCommandSignatures.back()));
}

void DXExt::CreateIndirectBuffer_DrawIndexed(const UINT IndexCount, const UINT InstanceCount)
{
	IndirectBufferResources.push_back(COM_PTR<ID3D12Resource>());

	const D3D12_DRAW_INDEXED_ARGUMENTS Source = { IndexCount, InstanceCount, 0, 0, 0 };
	const auto Stride = sizeof(Source);
	const auto Size = static_cast<UINT32>(Stride * 1);
	CreateAndCopyToDefaultResource(IndirectBufferResources.back(), COM_PTR_GET(CommandAllocators[0]), COM_PTR_GET(GraphicsCommandLists[0]), Size, &Source);

	IndirectCommandSignatures.resize(1);
	const std::array<D3D12_INDIRECT_ARGUMENT_DESC, 1> IADs = {
		{ D3D12_INDIRECT_ARGUMENT_TYPE_DRAW_INDEXED },
	};
	const D3D12_COMMAND_SIGNATURE_DESC CSD = {
		Stride,
		static_cast<const UINT>(IADs.size()), IADs.data(),
		0
	};
	Device->CreateCommandSignature(&CSD, /*COM_PTR_GET(RootSignatures[0])*/nullptr, COM_PTR_UUIDOF_PUTVOID(IndirectCommandSignatures[0]));
}

void DXExt::CreateIndirectBuffer_Dispatch(const UINT X, const UINT Y, const UINT Z)
{
	IndirectBufferResources.push_back(COM_PTR<ID3D12Resource>());

	const D3D12_DISPATCH_ARGUMENTS Source = { X, Y, Z };
	const auto Stride = sizeof(Source);
	const auto Size = static_cast<UINT32>(Stride * 1);
	CreateAndCopyToDefaultResource(IndirectBufferResources.back(), COM_PTR_GET(CommandAllocators[0]), COM_PTR_GET(GraphicsCommandLists[0]), Size, &Source);

	IndirectCommandSignatures.resize(1);
	const std::array<D3D12_INDIRECT_ARGUMENT_DESC, 1> IADs = {
		{ D3D12_INDIRECT_ARGUMENT_TYPE_DISPATCH },
	};
	const D3D12_COMMAND_SIGNATURE_DESC CSD = {
		Stride,
		static_cast<const UINT>(IADs.size()), IADs.data(),
		0
	};
	Device->CreateCommandSignature(&CSD, /*COM_PTR_GET(RootSignatures[0])*/nullptr, COM_PTR_UUIDOF_PUTVOID(IndirectCommandSignatures[0]));
}

void DXExt::CreateShaderBlob_VsPs()
{
	const auto ShaderPath = GetBasePath();
	ShaderBlobs.push_back(COM_PTR<ID3DBlob>());
	VERIFY_SUCCEEDED(D3DReadFileToBlob((ShaderPath + TEXT(".vs.cso")).data(), COM_PTR_PUT(ShaderBlobs.back())));
	ShaderBlobs.push_back(COM_PTR<ID3DBlob>());
	VERIFY_SUCCEEDED(D3DReadFileToBlob((ShaderPath + TEXT(".ps.cso")).data(), COM_PTR_PUT(ShaderBlobs.back())));

#if 0
	//!< PDB�p�[�g(�����ꍇ������)
	for (auto i : ShaderBlobs) {
		COM_PTR<ID3DBlob> PDBPart;
		const auto HR = D3DGetBlobPart(i->GetBufferPointer(), i->GetBufferSize(), D3D_BLOB_PDB, 0, COM_PTR_PUT(PDBPart));
	}

	//!< �f�o�b�O���̕t�^�A�擾
	for (auto i : ShaderBlobs) {
		//!< �t����u�f�o�b�O���v(�C��)
		const char DebugName[] = "DebugName";
		//!< 4�o�C�g�A���C�����ꂽ�X�g���[�W
		const auto Size = RoundUp(_countof(DebugName), 4);
		auto Data = new BYTE[Size];
		memcpy(Data, DebugName, _countof(DebugName));
		//!<�u�f�o�b�O���v��t����
		COM_PTR<ID3DBlob> BlobWithDebugNamePart;
		if (SUCCEEDED(D3DSetBlobPart(i->GetBufferPointer(), i->GetBufferSize(), D3D_BLOB_DEBUG_NAME, 0, Data, Size, COM_PTR_PUT(BlobWithDebugNamePart)))) {
			//!< �t�����u�f�o�b�O���v�p�[�g���擾���Ă݂�
			COM_PTR<ID3DBlob> DebugNamePart;
			if (SUCCEEDED(D3DGetBlobPart(BlobWithDebugNamePart->GetBufferPointer(), BlobWithDebugNamePart->GetBufferSize(), D3D_BLOB_DEBUG_NAME, 0, COM_PTR_PUT(DebugNamePart)))) {
				std::cout << reinterpret_cast<const char*>(DebugNamePart->GetBufferPointer()) << std::endl;
			}
		}
		delete[] Data;
	}

	//!< �e����̃X�g���b�v
#ifndef _DEBUG
	for (auto i : ShaderBlobs) {
		//!< �f�o�b�O���
		VERIFY_SUCCEEDED(D3DStripShader(i->GetBufferPointer(), i->GetBufferSize(), D3DCOMPILER_STRIP_DEBUG_INFO, COM_PTR_PUT(i)));
		//!< ���[�g�V�O�l�`��
		VERIFY_SUCCEEDED(D3DStripShader(i->GetBufferPointer(), i->GetBufferSize(), D3DCOMPILER_STRIP_ROOT_SIGNATURE, COM_PTR_PUT(i)));
	}
#endif
#endif
}
void DXExt::CreateShaderBlob_VsPsDsHsGs()
{
	const auto ShaderPath = GetBasePath();
	ShaderBlobs.push_back(COM_PTR<ID3DBlob>());
	VERIFY_SUCCEEDED(D3DReadFileToBlob((ShaderPath + TEXT(".vs.cso")).data(), COM_PTR_PUT(ShaderBlobs.back())));
	ShaderBlobs.push_back(COM_PTR<ID3DBlob>());
	VERIFY_SUCCEEDED(D3DReadFileToBlob((ShaderPath + TEXT(".ps.cso")).data(), COM_PTR_PUT(ShaderBlobs.back())));
	ShaderBlobs.push_back(COM_PTR<ID3DBlob>());
	VERIFY_SUCCEEDED(D3DReadFileToBlob((ShaderPath + TEXT(".ds.cso")).data(), COM_PTR_PUT(ShaderBlobs.back())));
	ShaderBlobs.push_back(COM_PTR<ID3DBlob>());
	VERIFY_SUCCEEDED(D3DReadFileToBlob((ShaderPath + TEXT(".hs.cso")).data(), COM_PTR_PUT(ShaderBlobs.back())));
	ShaderBlobs.push_back(COM_PTR<ID3DBlob>());
	VERIFY_SUCCEEDED(D3DReadFileToBlob((ShaderPath + TEXT(".gs.cso")).data(), COM_PTR_PUT(ShaderBlobs.back())));
}
void DXExt::CreateShaderBlob_Cs()
{
	const auto ShaderPath = GetBasePath();
	ShaderBlobs.push_back(COM_PTR<ID3DBlob>());
	VERIFY_SUCCEEDED(D3DReadFileToBlob((ShaderPath + TEXT(".cs.cso")).data(), COM_PTR_PUT(ShaderBlobs.back())));
}

void DXExt::CreatePipelineState_VsPs_Input(const D3D12_PRIMITIVE_TOPOLOGY_TYPE Topology, const BOOL DepthEnable, const std::vector<D3D12_INPUT_ELEMENT_DESC>& IEDs)
{
	const D3D12_DEPTH_STENCILOP_DESC DSOD = {
		D3D12_STENCIL_OP_KEEP,			//!< �X�e���V���e�X�g���s��
		D3D12_STENCIL_OP_KEEP,			//!< �X�e���V���e�X�g�����A�f�v�X�e�X�g���s��
		D3D12_STENCIL_OP_KEEP,			//!< �X�e���V���e�X�g�����A�f�v�X�e�X�g������
		D3D12_COMPARISON_FUNC_ALWAYS	//!< �����̃X�e���V���l�Ƃ̔�r���@
	};
	const auto StencilEnable = FALSE;
	const D3D12_DEPTH_STENCIL_DESC DSD = {
		//!< #DX_TODO (�A���t�@�u�����h����)�u�e�X�g�v�͗L�������u���C�g�v�͖����ɂ���ꍇ�� D3D12_DEPTH_WRITE_MASK_ZERO �ɂ���
		DepthEnable, D3D12_DEPTH_WRITE_MASK_ALL, D3D12_COMPARISON_FUNC_LESS,
		StencilEnable, D3D12_DEFAULT_STENCIL_READ_MASK, D3D12_DEFAULT_STENCIL_WRITE_MASK,
		DSOD, DSOD //!< �@�����J�����Ɍ����Ă���ꍇ�ƁA�����Ă��Ȃ��ꍇ
	};
	const std::array<D3D12_SHADER_BYTECODE, 5> SBCs = {
		D3D12_SHADER_BYTECODE({ ShaderBlobs[0]->GetBufferPointer(), ShaderBlobs[0]->GetBufferSize() }),
		D3D12_SHADER_BYTECODE({ ShaderBlobs[1]->GetBufferPointer(), ShaderBlobs[1]->GetBufferSize() }),
	};

	PipelineStates.resize(1);
	std::vector<std::thread> Threads;

	//!< �����o�֐����X���b�h�Ŏg�p�������ꍇ�A�ȉ��̂悤��this�������Ɏ��`�����g�p
	//std::thread::thread(&DXExt::Func, this, Arg0, Arg1,...);

#ifdef USE_PIPELINE_SERIALIZE
	PipelineLibrarySerializer PLS(COM_PTR_GET(Device), GetBasePath() + TEXT(".plo"));
	Threads.push_back(std::thread::thread(DX::CreatePipelineState, std::ref(PipelineStates[0]), COM_PTR_GET(Device), COM_PTR_GET(RootSignatures[0]), Topology, DSD, SBCs[0], SBCs[1], NullShaderBC, NullShaderBC, NullShaderBC, IEDs, &PLS, TEXT("0")));
#else
	Threads.push_back(std::thread::thread(DX::CreatePipelineState, std::ref(PipelineStates[0]), COM_PTR_GET(Device), COM_PTR_GET(RootSignatures[0]), Topology, DSD, SBCs[0], SBCs[1], NullShaderBC, NullShaderBC, NullShaderBC, IEDs, nullptr, nullptr));
#endif	
	for (auto& i : Threads) { i.join(); }
}

void DXExt::CreatePipelineState_VsPsDsHsGs_Input(const D3D12_PRIMITIVE_TOPOLOGY_TYPE Topology, const BOOL DepthEnable, const std::vector<D3D12_INPUT_ELEMENT_DESC>& IEDs)
{
	const D3D12_DEPTH_STENCILOP_DESC DSOD = { D3D12_STENCIL_OP_KEEP, D3D12_STENCIL_OP_KEEP, D3D12_STENCIL_OP_KEEP, D3D12_COMPARISON_FUNC_ALWAYS };
	const D3D12_DEPTH_STENCIL_DESC DSD = {
		DepthEnable, D3D12_DEPTH_WRITE_MASK_ALL, D3D12_COMPARISON_FUNC_LESS,
		FALSE, D3D12_DEFAULT_STENCIL_READ_MASK, D3D12_DEFAULT_STENCIL_WRITE_MASK, 
		DSOD, DSOD 
	};
	const std::array<D3D12_SHADER_BYTECODE, 5> SBCs = {
			D3D12_SHADER_BYTECODE({ ShaderBlobs[0]->GetBufferPointer(), ShaderBlobs[0]->GetBufferSize() }),
			D3D12_SHADER_BYTECODE({ ShaderBlobs[1]->GetBufferPointer(), ShaderBlobs[1]->GetBufferSize() }),
			D3D12_SHADER_BYTECODE({ ShaderBlobs[2]->GetBufferPointer(), ShaderBlobs[2]->GetBufferSize() }),
			D3D12_SHADER_BYTECODE({ ShaderBlobs[3]->GetBufferPointer(), ShaderBlobs[3]->GetBufferSize() }),
			D3D12_SHADER_BYTECODE({ ShaderBlobs[4]->GetBufferPointer(), ShaderBlobs[4]->GetBufferSize() }),
	};

	PipelineStates.resize(1);
	std::vector<std::thread> Threads;
#ifdef USE_PIPELINE_SERIALIZE
	PipelineLibrarySerializer PLS(COM_PTR_GET(Device), GetBasePath() + TEXT(".plo"));
	Threads.push_back(std::thread::thread(DX::CreatePipelineState, std::ref(PipelineStates[0]), COM_PTR_GET(Device), COM_PTR_GET(RootSignatures[0]), Topology, DSD, SBCs[0], SBCs[1], SBCs[2], SBCs[3], SBCs[4], IEDs, &PLS, TEXT("0")));
#else
	Threads.push_back(std::thread::thread(DX::CreatePipelineState, std::ref(PipelineStates[0]), COM_PTR_GET(Device), COM_PTR_GET(RootSignatures[0]), Topology, DSD, SBCs[0], SBCs[1], SBCs[2], SBCs[3], SBCs[4], IEDs, nullptr, nullptr));
#endif	
	for (auto& i : Threads) { i.join(); }
}

void DXExt::CreateRenderTexture()
{

}
