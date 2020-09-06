#include "DXExt.h"

void DXExt::CreateIndirectBuffer_Draw(const UINT IndexCount, const UINT InstanceCount)
{
	IndirectBuffers.push_back(IndirectBuffer());
	const D3D12_DRAW_ARGUMENTS Source = { IndexCount, InstanceCount, 0, 0 };
	const auto Stride = sizeof(Source);
	const auto Size = static_cast<UINT32>(Stride * 1);
	CreateAndCopyToDefaultResource(IndirectBuffers.back().Resource, COM_PTR_GET(CommandAllocators[0]), COM_PTR_GET(GraphicsCommandLists[0]), Size, &Source);

	const std::array<D3D12_INDIRECT_ARGUMENT_DESC, 1> IADs = {
		{ D3D12_INDIRECT_ARGUMENT_TYPE_DRAW },
	};
	const D3D12_COMMAND_SIGNATURE_DESC CSD = {
		Stride,
		static_cast<const UINT>(IADs.size()), IADs.data(),
		0
	};
	//!< パイプラインのバインディングを更新するような場合はルートシグネチャが必要、Draw や Dispatch のみの場合はnullptrを指定できる
	Device->CreateCommandSignature(&CSD, /*COM_PTR_GET(RootSignatures[0])*/nullptr, COM_PTR_UUIDOF_PUTVOID(IndirectBuffers.back().CommandSignature));
}

void DXExt::CreateIndirectBuffer_DrawIndexed(const UINT IndexCount, const UINT InstanceCount)
{
	IndirectBuffers.push_back(IndirectBuffer());
	const D3D12_DRAW_INDEXED_ARGUMENTS Source = { IndexCount, InstanceCount, 0, 0, 0 };
	const auto Stride = sizeof(Source);
	const auto Size = static_cast<UINT32>(Stride * 1);
	CreateAndCopyToDefaultResource(IndirectBuffers.back().Resource, COM_PTR_GET(CommandAllocators[0]), COM_PTR_GET(GraphicsCommandLists[0]), Size, &Source);

	const std::array<D3D12_INDIRECT_ARGUMENT_DESC, 1> IADs = {
		{ D3D12_INDIRECT_ARGUMENT_TYPE_DRAW_INDEXED },
	};
	const D3D12_COMMAND_SIGNATURE_DESC CSD = {
		Stride,
		static_cast<const UINT>(IADs.size()), IADs.data(),
		0
	};
	Device->CreateCommandSignature(&CSD, nullptr, COM_PTR_UUIDOF_PUTVOID(IndirectBuffers.back().CommandSignature));
}

void DXExt::CreateIndirectBuffer_Dispatch(const UINT X, const UINT Y, const UINT Z)
{
	IndirectBuffers.push_back(IndirectBuffer());

	const D3D12_DISPATCH_ARGUMENTS Source = { X, Y, Z };
	const auto Stride = sizeof(Source);
	const auto Size = static_cast<UINT32>(Stride * 1);
	CreateAndCopyToDefaultResource(IndirectBuffers.back().Resource, COM_PTR_GET(CommandAllocators[0]), COM_PTR_GET(GraphicsCommandLists[0]), Size, &Source);

	const std::array<D3D12_INDIRECT_ARGUMENT_DESC, 1> IADs = {
		{ D3D12_INDIRECT_ARGUMENT_TYPE_DISPATCH },
	};
	const D3D12_COMMAND_SIGNATURE_DESC CSD = {
		Stride,
		static_cast<const UINT>(IADs.size()), IADs.data(),
		0
	};
	Device->CreateCommandSignature(&CSD, nullptr, COM_PTR_UUIDOF_PUTVOID(IndirectBuffers.back().CommandSignature));
}

#ifdef USE_SHADER_REFLECTION
void DXExt::PrintShaderReflection(ID3DBlob* Blob)
{
#if 0
	//!< FXC
	COM_PTR<ID3D12ShaderReflection> SR;
	VERIFY_SUCCEEDED(D3DReflect(Blob->GetBufferPointer(), Blob->GetBufferSize(), COM_PTR_UUIDOF_PUTVOID(SR)));
	D3D12_SHADER_DESC SD;
	VERIFY_SUCCEEDED(SR->GetDesc(&SD));
#else
	//!< DXC
	//!< (dxcompiler.dll が無いと怒られる場合は C:\Program Files (x86)\Windows Kits\10\bin\10.0.18362.0\x64 とかに存在するので、環境変数 Path に通しておく必要がある)
	COM_PTR<IDxcLibrary> DL;
	VERIFY_SUCCEEDED(DxcCreateInstance(CLSID_DxcLibrary, COM_PTR_UUIDOF_PUTVOID(DL)));

	COM_PTR<IDxcBlobEncoding> DBE;
	VERIFY_SUCCEEDED(DL->CreateBlobWithEncodingOnHeapCopy(Blob->GetBufferPointer(), static_cast<UINT32>(Blob->GetBufferSize()), CP_ACP, COM_PTR_PUT(DBE)));

	COM_PTR<IDxcContainerReflection> DCR;
	VERIFY_SUCCEEDED(DxcCreateInstance(CLSID_DxcContainerReflection, COM_PTR_UUIDOF_PUTVOID(DCR)));

	UINT Index = 0;
	VERIFY_SUCCEEDED(DCR->Load(COM_PTR_GET(DBE)));
	VERIFY_SUCCEEDED(DCR->FindFirstPartKind(DXIL_FOURCC('D', 'X', 'I', 'L'), &Index));

	COM_PTR<ID3D12ShaderReflection> SR;
	VERIFY_SUCCEEDED(DCR->GetPartReflection(Index, COM_PTR_UUIDOF_PUTVOID(SR)));

	D3D12_SHADER_DESC SD;
	VERIFY_SUCCEEDED(SR->GetDesc(&SD));
	
	if (0 < SD.ConstantBuffers) { std::cout << "\tConstantBuffers" << std::endl; }
	for (UINT i = 0; i < SD.ConstantBuffers; ++i) {
		const auto CB = SR->GetConstantBufferByIndex(i);
		D3D12_SHADER_BUFFER_DESC SBD;
		VERIFY_SUCCEEDED(CB->GetDesc(&SBD));
		std::cout << "\t\t" << SBD.Name << std::endl;
		for (UINT j = 0; j < SBD.Variables; ++j) {
			const auto SRV = CB->GetVariableByIndex(j);
			D3D12_SHADER_VARIABLE_DESC SVD;
			VERIFY_SUCCEEDED(SRV->GetDesc(&SVD));
			D3D12_SHADER_TYPE_DESC STD;
			VERIFY_SUCCEEDED(SRV->GetType()->GetDesc(&STD));
			std::cout << "\t\t\t" << SVD.Name << "\t" << STD.Name << std::endl;
		}
	}

	if (0 < SD.BoundResources) { std::cout << "\tBoundResources" << std::endl; }
	for (UINT i = 0; i < SD.BoundResources; ++i) {
		D3D12_SHADER_INPUT_BIND_DESC SIBD;
		VERIFY_SUCCEEDED(SR->GetResourceBindingDesc(i, &SIBD));
		std::cout << "\t\t" << SIBD.Name << "\t";
		switch (SIBD.Type) {
		case D3D_SIT_CBUFFER: std::cout << "CBUFFER"; break;
		case D3D_SIT_TBUFFER: break;
		case D3D_SIT_TEXTURE: std::cout << "TEXTURE"; break;
		case D3D_SIT_SAMPLER: std::cout << "SAMPLER"; break;
		case D3D_SIT_UAV_RWTYPED: break;
		case D3D_SIT_STRUCTURED: break;
		case D3D_SIT_UAV_RWSTRUCTURED: break;
		case D3D_SIT_BYTEADDRESS: break;
		case D3D_SIT_UAV_RWBYTEADDRESS: break;
		case D3D_SIT_UAV_APPEND_STRUCTURED: break;
		case D3D_SIT_UAV_CONSUME_STRUCTURED: break;
		case D3D_SIT_UAV_RWSTRUCTURED_WITH_COUNTER: break;
		default: break;
		}
		std::cout << "(" << SIBD.BindPoint;
		if (0 < SIBD.BindCount) { std::cout << "-" << SIBD.BindPoint + SIBD.BindCount; }
		std::cout << ")";

		if (D3D_SIT_TEXTURE == SIBD.Type) {
			std::cout << SIBD.ReturnType;
			std::cout << SIBD.Dimension;
		}
		std::cout << std::endl;
	}
	
	if (0 < SD.InputParameters) { std::cout << "\tInputParameters" << std::endl; }
	for (UINT i = 0; i < SD.InputParameters; ++i) {
		D3D12_SIGNATURE_PARAMETER_DESC SPD;
		VERIFY_SUCCEEDED(SR->GetInputParameterDesc(i, &SPD));
		std::cout << "\t\t" << SPD.SemanticName << SPD.SemanticIndex << "\t";
		switch (SPD.ComponentType)
		{
		default:
		case D3D_REGISTER_COMPONENT_UNKNOWN: std::cout << "UNKNOWN"; break;
		case D3D_REGISTER_COMPONENT_UINT32: std::cout << "UINT32"; break;
		case D3D_REGISTER_COMPONENT_SINT32: std::cout << "SINT32"; break;
		case D3D_REGISTER_COMPONENT_FLOAT32: std::cout << "FLOAT32"; break;
		}
		std::cout << "(" << ((SPD.Mask & 1) ? "x" : "-") << ((SPD.Mask & 2) ? "y" : "-") << ((SPD.Mask & 4) ? "z" : "-") << ((SPD.Mask & 8) ? "w" : "-") << ")" << std::endl;
	}

	if (0 < SD.OutputParameters) { std::cout << "\tOutputParameters" << std::endl; }
	for (UINT i = 0; i < SD.OutputParameters; ++i) {
		D3D12_SIGNATURE_PARAMETER_DESC SPD;
		VERIFY_SUCCEEDED(SR->GetOutputParameterDesc(i, &SPD));
		std::cout << "\t\t" << SPD.SemanticName << SPD.SemanticIndex << "\t";
		switch (SPD.ComponentType)
		{
		default:
		case D3D_REGISTER_COMPONENT_UNKNOWN: std::cout << "UNKNOWN"; break;
		case D3D_REGISTER_COMPONENT_UINT32: std::cout << "UINT32"; break;
		case D3D_REGISTER_COMPONENT_SINT32: std::cout << "SINT32"; break;
		case D3D_REGISTER_COMPONENT_FLOAT32: std::cout << "FLOAT32"; break;
		}
		std::cout << "(" << ((SPD.Mask & 1) ? "x" : "-") << ((SPD.Mask & 2) ? "y" : "-") << ((SPD.Mask & 4) ? "z" : "-") << ((SPD.Mask & 8) ? "w" : "-") << ")" << std::endl;
	}
#endif
}
#endif

void DXExt::CreateShaderBlob_VsPs()
{
	const auto ShaderPath = GetBasePath();
	ShaderBlobs.emplace_back(COM_PTR<ID3DBlob>());
	VERIFY_SUCCEEDED(D3DReadFileToBlob((ShaderPath + TEXT(".vs.cso")).data(), COM_PTR_PUT(ShaderBlobs.back())));
#ifdef USE_SHADER_REFLECTION
	std::wcout << (ShaderPath + TEXT(".vs.cso")) << std::endl;
	PrintShaderReflection(COM_PTR_GET(ShaderBlobs.back()));
#endif
	ShaderBlobs.emplace_back(COM_PTR<ID3DBlob>());
	VERIFY_SUCCEEDED(D3DReadFileToBlob((ShaderPath + TEXT(".ps.cso")).data(), COM_PTR_PUT(ShaderBlobs.back())));
#ifdef USE_SHADER_REFLECTION
	std::wcout << (ShaderPath + TEXT(".ps.cso")) << std::endl;
	PrintShaderReflection(COM_PTR_GET(ShaderBlobs.back()));
#endif

#if 0
	//!< PDBパート(無い場合もある)
	for (auto i : ShaderBlobs) {
		COM_PTR<ID3DBlob> PDBPart;
		const auto HR = D3DGetBlobPart(i->GetBufferPointer(), i->GetBufferSize(), D3D_BLOB_PDB, 0, COM_PTR_PUT(PDBPart));
	}

	//!< デバッグ名の付与、取得
	for (auto i : ShaderBlobs) {
		//!< 付ける「デバッグ名」(任意)
		const char DebugName[] = "DebugName";
		//!< 4バイトアラインされたストレージ
		const auto Size = RoundUp(_countof(DebugName), 4);
		auto Data = new BYTE[Size];
		memcpy(Data, DebugName, _countof(DebugName));
		//!<「デバッグ名」を付ける
		COM_PTR<ID3DBlob> BlobWithDebugNamePart;
		if (SUCCEEDED(D3DSetBlobPart(i->GetBufferPointer(), i->GetBufferSize(), D3D_BLOB_DEBUG_NAME, 0, Data, Size, COM_PTR_PUT(BlobWithDebugNamePart)))) {
			//!< 付けた「デバッグ名」パートを取得してみる
			COM_PTR<ID3DBlob> DebugNamePart;
			if (SUCCEEDED(D3DGetBlobPart(BlobWithDebugNamePart->GetBufferPointer(), BlobWithDebugNamePart->GetBufferSize(), D3D_BLOB_DEBUG_NAME, 0, COM_PTR_PUT(DebugNamePart)))) {
				std::cout << reinterpret_cast<const char*>(DebugNamePart->GetBufferPointer()) << std::endl;
			}
		}
		delete[] Data;
	}

	//!< 各種情報のストリップ
#ifndef _DEBUG
	for (auto i : ShaderBlobs) {
		//!< デバッグ情報
		VERIFY_SUCCEEDED(D3DStripShader(i->GetBufferPointer(), i->GetBufferSize(), D3DCOMPILER_STRIP_DEBUG_INFO, COM_PTR_PUT(i)));
		//!< ルートシグネチャ
		VERIFY_SUCCEEDED(D3DStripShader(i->GetBufferPointer(), i->GetBufferSize(), D3DCOMPILER_STRIP_ROOT_SIGNATURE, COM_PTR_PUT(i)));
	}
#endif
#endif
}
void DXExt::CreateShaderBlob_VsPsDsHsGs()
{
	const auto ShaderPath = GetBasePath();
	ShaderBlobs.emplace_back(COM_PTR<ID3DBlob>());
	VERIFY_SUCCEEDED(D3DReadFileToBlob((ShaderPath + TEXT(".vs.cso")).data(), COM_PTR_PUT(ShaderBlobs.back())));
#ifdef USE_SHADER_REFLECTION
	std::wcout << (ShaderPath + TEXT(".vs.cso")) << std::endl;
	PrintShaderReflection(COM_PTR_GET(ShaderBlobs.back()));
#endif
	ShaderBlobs.emplace_back(COM_PTR<ID3DBlob>());
	VERIFY_SUCCEEDED(D3DReadFileToBlob((ShaderPath + TEXT(".ps.cso")).data(), COM_PTR_PUT(ShaderBlobs.back())));
#ifdef USE_SHADER_REFLECTION
	std::wcout << (ShaderPath + TEXT(".ps.cso")) << std::endl;
	PrintShaderReflection(COM_PTR_GET(ShaderBlobs.back()));
#endif
	ShaderBlobs.emplace_back(COM_PTR<ID3DBlob>());
	VERIFY_SUCCEEDED(D3DReadFileToBlob((ShaderPath + TEXT(".ds.cso")).data(), COM_PTR_PUT(ShaderBlobs.back())));
#ifdef USE_SHADER_REFLECTION
	std::wcout << (ShaderPath + TEXT(".ds.cso")) << std::endl;
	PrintShaderReflection(COM_PTR_GET(ShaderBlobs.back()));
#endif
	ShaderBlobs.emplace_back(COM_PTR<ID3DBlob>());
	VERIFY_SUCCEEDED(D3DReadFileToBlob((ShaderPath + TEXT(".hs.cso")).data(), COM_PTR_PUT(ShaderBlobs.back())));
#ifdef USE_SHADER_REFLECTION
	std::wcout << (ShaderPath + TEXT(".hs.cso")) << std::endl;
	PrintShaderReflection(COM_PTR_GET(ShaderBlobs.back()));
#endif
	ShaderBlobs.emplace_back(COM_PTR<ID3DBlob>());
	VERIFY_SUCCEEDED(D3DReadFileToBlob((ShaderPath + TEXT(".gs.cso")).data(), COM_PTR_PUT(ShaderBlobs.back())));
#ifdef USE_SHADER_REFLECTION
	std::wcout << (ShaderPath + TEXT(".gs.cso")) << std::endl;
	PrintShaderReflection(COM_PTR_GET(ShaderBlobs.back()));
#endif
}
void DXExt::CreateShaderBlob_Cs()
{
	const auto ShaderPath = GetBasePath();
	ShaderBlobs.emplace_back(COM_PTR<ID3DBlob>());
	VERIFY_SUCCEEDED(D3DReadFileToBlob((ShaderPath + TEXT(".cs.cso")).data(), COM_PTR_PUT(ShaderBlobs.back())));
}

void DXExt::CreatePipelineState_VsPs_Input(const D3D12_PRIMITIVE_TOPOLOGY_TYPE Topology, const BOOL DepthEnable, const std::vector<D3D12_INPUT_ELEMENT_DESC>& IEDs)
{
	const D3D12_RASTERIZER_DESC RD = {
		D3D12_FILL_MODE_SOLID,
		D3D12_CULL_MODE_BACK, TRUE,
		D3D12_DEFAULT_DEPTH_BIAS, D3D12_DEFAULT_DEPTH_BIAS_CLAMP, D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS, 
		TRUE,
		FALSE, FALSE, 0,
		D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF
	};
	const D3D12_DEPTH_STENCILOP_DESC DSOD = {
		D3D12_STENCIL_OP_KEEP,			//!< ステンシルテスト失敗時
		D3D12_STENCIL_OP_KEEP,			//!< ステンシルテスト成功、デプステスト失敗時
		D3D12_STENCIL_OP_KEEP,			//!< ステンシルテスト成功、デプステスト成功時
		D3D12_COMPARISON_FUNC_ALWAYS	//!< 既存のステンシル値との比較方法
	};
	const auto StencilEnable = FALSE;
	const D3D12_DEPTH_STENCIL_DESC DSD = {
		//!< #DX_TODO (アルファブレンド等で)「テスト」は有効だが「ライト」は無効にする場合は D3D12_DEPTH_WRITE_MASK_ZERO にする
		DepthEnable, D3D12_DEPTH_WRITE_MASK_ALL, D3D12_COMPARISON_FUNC_LESS,
		StencilEnable, D3D12_DEFAULT_STENCIL_READ_MASK, D3D12_DEFAULT_STENCIL_WRITE_MASK,
		DSOD, DSOD //!< 法線がカメラに向いている場合と、向いていない場合
	};
	const std::array<D3D12_SHADER_BYTECODE, 5> SBCs = {
		D3D12_SHADER_BYTECODE({ ShaderBlobs[0]->GetBufferPointer(), ShaderBlobs[0]->GetBufferSize() }),
		D3D12_SHADER_BYTECODE({ ShaderBlobs[1]->GetBufferPointer(), ShaderBlobs[1]->GetBufferSize() }),
	};
	const std::vector<DXGI_FORMAT> RTVs = { DXGI_FORMAT_R8G8B8A8_UNORM };

	PipelineStates.resize(1);
	std::vector<std::thread> Threads;

	//!< メンバ関数をスレッドで使用したい場合、以下のようにthisを引数に取る形式を使用
	//std::thread::thread(&DXExt::Func, this, Arg0, Arg1,...);

#ifdef USE_PIPELINE_SERIALIZE
	PipelineLibrarySerializer PLS(COM_PTR_GET(Device), GetBasePath() + TEXT(".plo"));
	Threads.push_back(std::thread::thread(DX::CreatePipelineState, std::ref(PipelineStates[0]), COM_PTR_GET(Device), COM_PTR_GET(RootSignatures[0]), Topology, RD, DSD, SBCs[0], SBCs[1], NullShaderBC, NullShaderBC, NullShaderBC, IEDs, RTVs, &PLS, TEXT("0")));
#else
	Threads.push_back(std::thread::thread(DX::CreatePipelineState, std::ref(PipelineStates[0]), COM_PTR_GET(Device), COM_PTR_GET(RootSignatures[0]), Topology, RD, DSD, SBCs[0], SBCs[1], NullShaderBC, NullShaderBC, NullShaderBC, IEDs, RTVs, nullptr, nullptr));
#endif	
	for (auto& i : Threads) { i.join(); }
}

void DXExt::CreatePipelineState_VsPsDsHsGs_Input(const D3D12_PRIMITIVE_TOPOLOGY_TYPE Topology, const BOOL DepthEnable, const std::vector<D3D12_INPUT_ELEMENT_DESC>& IEDs)
{
	const D3D12_RASTERIZER_DESC RD = {
		D3D12_FILL_MODE_SOLID,
		D3D12_CULL_MODE_BACK, TRUE,
		D3D12_DEFAULT_DEPTH_BIAS, D3D12_DEFAULT_DEPTH_BIAS_CLAMP, D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS, 
		TRUE,
		FALSE, FALSE, 0,
		D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF
	};
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
	const std::vector<DXGI_FORMAT> RTVs = { DXGI_FORMAT_R8G8B8A8_UNORM };

	PipelineStates.resize(1);
	std::vector<std::thread> Threads;
#ifdef USE_PIPELINE_SERIALIZE
	PipelineLibrarySerializer PLS(COM_PTR_GET(Device), GetBasePath() + TEXT(".plo"));
	Threads.push_back(std::thread::thread(DX::CreatePipelineState, std::ref(PipelineStates[0]), COM_PTR_GET(Device), COM_PTR_GET(RootSignatures[0]), Topology, RD, DSD, SBCs[0], SBCs[1], SBCs[2], SBCs[3], SBCs[4], IEDs, RTVs, &PLS, TEXT("0")));
#else
	Threads.push_back(std::thread::thread(DX::CreatePipelineState, std::ref(PipelineStates[0]), COM_PTR_GET(Device), COM_PTR_GET(RootSignatures[0]), Topology, RD, DSD, SBCs[0], SBCs[1], SBCs[2], SBCs[3], SBCs[4], IEDs, RTVs, nullptr, nullptr));
#endif	
	for (auto& i : Threads) { i.join(); }
}

void DXExt::CreateRenderTexture()
{

}
