//#include "framework.h
//#include "stdafx.h"

#include "DXExt.h"

void DXExt::CreateIndirectBuffer_Draw(const UINT IndexCount, const UINT InstanceCount)
{
	IndirectBufferResources.resize(1);
	const D3D12_DRAW_ARGUMENTS Source = { IndexCount, InstanceCount, 0, 0 };
	const auto Stride = sizeof(Source);
	const auto Size = static_cast<UINT32>(Stride * 1);
	CreateBuffer(COM_PTR_PUT(IndirectBufferResources[0]), Size, &Source, COM_PTR_GET(CommandAllocators[0]), COM_PTR_GET(GraphicsCommandLists[0]));

	IndirectCommandSignatures.resize(1);
	const std::array<D3D12_INDIRECT_ARGUMENT_DESC, 1> IADs = {
		{ D3D12_INDIRECT_ARGUMENT_TYPE_DRAW },
	};
	const D3D12_COMMAND_SIGNATURE_DESC CSD = {
		Stride,
		static_cast<const UINT>(IADs.size()), IADs.data(),
		0
	};
	Device->CreateCommandSignature(&CSD, /*COM_PTR_GET(RootSignatures[0])*/nullptr, COM_PTR_UUIDOF_PUTVOID(IndirectCommandSignatures[0]));
}

void DXExt::CreateIndirectBuffer_DrawIndexed(const UINT IndexCount, const UINT InstanceCount)
{
	IndirectBufferResources.resize(1);
	const D3D12_DRAW_INDEXED_ARGUMENTS Source = { IndexCount, InstanceCount, 0, 0, 0 };
	const auto Stride = sizeof(Source);
	const auto Size = static_cast<UINT32>(Stride * 1);
	CreateBuffer(COM_PTR_PUT(IndirectBufferResources[0]), Size, &Source, COM_PTR_GET(CommandAllocators[0]), COM_PTR_GET(GraphicsCommandLists[0]));

	IndirectCommandSignatures.resize(1);
	const std::array<D3D12_INDIRECT_ARGUMENT_DESC, 1> IADs = {
		{ D3D12_INDIRECT_ARGUMENT_TYPE_DRAW_INDEXED },
	};
	const D3D12_COMMAND_SIGNATURE_DESC CSD = {
		Stride,
		static_cast<const UINT>(IADs.size()), IADs.data(),
		0
	};
	//!< パイプラインのバインディングを更新するような場合はルートシグネチャが必要、Draw や Dispatch のみの場合はnullptrを指定できる
	Device->CreateCommandSignature(&CSD, /*COM_PTR_GET(RootSignatures[0])*/nullptr, COM_PTR_UUIDOF_PUTVOID(IndirectCommandSignatures[0]));
}

void DXExt::CreateIndirectBuffer_Dispatch(const UINT X, const UINT Y, const UINT Z)
{
	IndirectBufferResources.resize(1);
	const D3D12_DISPATCH_ARGUMENTS Source = { X, Y, Z };
	const auto Stride = sizeof(Source);
	const auto Size = static_cast<UINT32>(Stride * 1);
	CreateBuffer(COM_PTR_PUT(IndirectBufferResources[0]), Size, &Source, COM_PTR_GET(CommandAllocators[0]), COM_PTR_GET(GraphicsCommandLists[0]));

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

void DXExt::CreatePipelineState_Tesselation(COM_PTR<ID3D12PipelineState>& PST, ID3D12RootSignature* RS, 
	const D3D12_SHADER_BYTECODE VS, const D3D12_SHADER_BYTECODE PS, const D3D12_SHADER_BYTECODE DS, const D3D12_SHADER_BYTECODE HS, const D3D12_SHADER_BYTECODE GS,
	ID3D12PipelineLibrary* PL, LPCWSTR Name, const bool IsLoad)
{
	PERFORMANCE_COUNTER();

	assert((VS.pShaderBytecode != nullptr && VS.BytecodeLength) && "");

	const D3D12_STREAM_OUTPUT_DESC SOD = {
		nullptr, 0,
		nullptr, 0,
		0
	};

	const D3D12_RENDER_TARGET_BLEND_DESC RTBD = {
		FALSE, FALSE,
		D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
		D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
		D3D12_LOGIC_OP_NOOP,
		D3D12_COLOR_WRITE_ENABLE_ALL,
	};
	const D3D12_BLEND_DESC BD = {
		FALSE,
		FALSE,
		{ RTBD }
	};

	const D3D12_RASTERIZER_DESC RD = {
		D3D12_FILL_MODE_SOLID,
		D3D12_CULL_MODE_BACK, TRUE,
		D3D12_DEFAULT_DEPTH_BIAS, D3D12_DEFAULT_DEPTH_BIAS_CLAMP, D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS,
		TRUE,
		FALSE,
		FALSE,
		0,
		D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF
	};

	const D3D12_DEPTH_STENCILOP_DESC DSOD = {
		D3D12_STENCIL_OP_KEEP,
		D3D12_STENCIL_OP_KEEP,
		D3D12_STENCIL_OP_KEEP,
		D3D12_COMPARISON_FUNC_NEVER
	};
	const D3D12_DEPTH_STENCIL_DESC DSD = {
		FALSE,
		D3D12_DEPTH_WRITE_MASK_ZERO,
		D3D12_COMPARISON_FUNC_NEVER,
		FALSE,
		0,
		0,
		DSOD,
		DSOD
	};

	const std::array<D3D12_INPUT_ELEMENT_DESC, 0> IEDs = {};
	const D3D12_INPUT_LAYOUT_DESC ILD = {
		IEDs.data(), static_cast<UINT>(IEDs.size())
	};

	const DXGI_SAMPLE_DESC SD = { 1, 0 };

	const D3D12_CACHED_PIPELINE_STATE CPS = { nullptr, 0 };
	const D3D12_GRAPHICS_PIPELINE_STATE_DESC GPSD = {
		RS,
		VS, PS, DS, HS, GS,
		SOD,
		BD,
		UINT_MAX,
		RD,
		DSD,
		ILD,
		D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED,
		D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH, //!< トポロジに PATCH を指定
		1, { DXGI_FORMAT_R8G8B8A8_UNORM }, DXGI_FORMAT_D32_FLOAT_S8X24_UINT,
		SD,
		0,
		CPS,
		D3D12_PIPELINE_STATE_FLAG_NONE
	};
	assert(GPSD.NumRenderTargets <= _countof(GPSD.RTVFormats) && "");

	//!< DXでは「パッチコントロールポイント」の指定はIASetPrimitiveTopology()の引数としてコマンドリストへ指定する、VKとは結構異なるので注意
	//!< CommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_1_CONTROL_POINT_PATCHLIST);

	if (IsLoad) {
		//!< ドキュメントに何も書いてないがスレッドセーフであると信じたい
		if (nullptr != PL && nullptr != Name) {
			VERIFY_SUCCEEDED(PL->LoadGraphicsPipeline(Name, &GPSD, COM_PTR_UUIDOF_PUTVOID(PST)));
		}
	}
	else {
		VERIFY_SUCCEEDED(Device->CreateGraphicsPipelineState(&GPSD, COM_PTR_UUIDOF_PUTVOID(PST)));
		//!< ドキュメントに何も書いてないがスレッドセーフであると信じたい
		if (nullptr != PL && nullptr != Name) {
			VERIFY_SUCCEEDED(PL->StorePipeline(Name, COM_PTR_GET(PST)));
		}
	}

	LOG_OK();
}

void DXExt::CreateShaderBlob_VsPs()
{
	ShaderBlobs.resize(2);
	const auto ShaderPath = GetBasePath();
	VERIFY_SUCCEEDED(D3DReadFileToBlob((ShaderPath + TEXT(".vs.cso")).data(), COM_PTR_PUT(ShaderBlobs[0])));
	VERIFY_SUCCEEDED(D3DReadFileToBlob((ShaderPath + TEXT(".ps.cso")).data(), COM_PTR_PUT(ShaderBlobs[1])));
}
void DXExt::CreateShaderBlob_VsPsDsHsGs()
{
	ShaderBlobs.resize(5);
	const auto ShaderPath = GetBasePath();
	VERIFY_SUCCEEDED(D3DReadFileToBlob((ShaderPath + TEXT(".vs.cso")).data(), COM_PTR_PUT(ShaderBlobs[0])));
	VERIFY_SUCCEEDED(D3DReadFileToBlob((ShaderPath + TEXT(".ps.cso")).data(), COM_PTR_PUT(ShaderBlobs[1])));
	VERIFY_SUCCEEDED(D3DReadFileToBlob((ShaderPath + TEXT(".ds.cso")).data(), COM_PTR_PUT(ShaderBlobs[2])));
	VERIFY_SUCCEEDED(D3DReadFileToBlob((ShaderPath + TEXT(".hs.cso")).data(), COM_PTR_PUT(ShaderBlobs[3])));
	VERIFY_SUCCEEDED(D3DReadFileToBlob((ShaderPath + TEXT(".gs.cso")).data(), COM_PTR_PUT(ShaderBlobs[4])));
}
void DXExt::CreateShaderBlob_Cs()
{
	ShaderBlobs.resize(1);
	const auto ShaderPath = GetBasePath();
	VERIFY_SUCCEEDED(D3DReadFileToBlob((ShaderPath + TEXT(".cs.cso")).data(), COM_PTR_PUT(ShaderBlobs[0])));
}

void DXExt::CreatePipelineState_VsPs()
{
	PipelineStates.resize(1);

#ifdef USE_PIPELINE_SERIALIZE
	PipelineLibrarySerializer PLS(COM_PTR_GET(Device), GetBasePath() + TEXT(".plo"));
#endif

	std::vector<std::thread> Threads;

	{
		const std::array<D3D12_SHADER_BYTECODE, 2> SBCs = { {
			{ ShaderBlobs[0]->GetBufferPointer(), ShaderBlobs[0]->GetBufferSize() },
			{ ShaderBlobs[1]->GetBufferPointer(), ShaderBlobs[1]->GetBufferSize() },
		} };
		Threads.push_back(std::thread::thread([&](COM_PTR<ID3D12PipelineState>& PST, ID3D12RootSignature* RS,
			const D3D12_SHADER_BYTECODE VS, const D3D12_SHADER_BYTECODE PS, const D3D12_SHADER_BYTECODE DS, const D3D12_SHADER_BYTECODE HS, const D3D12_SHADER_BYTECODE GS)
			{
#ifdef USE_PIPELINE_SERIALIZE
				CreatePipelineState_Default(PST, RS, VS, PS, DS, HS, GS, PLS.GetPipelineLibrary(), TEXT("0"), PLS.IsLoadSucceeded());
#else
				CreatePipelineState_Default(PST, RS, VS, PS, DS, HS, GS);
#endif
			},
			std::ref(PipelineStates[0]), COM_PTR_GET(RootSignatures[0]), SBCs[0], SBCs[1], NullShaderBC, NullShaderBC, NullShaderBC));
	}

	for (auto& i : Threads) {
		i.join();
	}
}
void DXExt::CreatePipelineState_VsPsDsHsGs_Tesselation()
{
	PipelineStates.resize(1);

#ifdef USE_PIPELINE_SERIALIZE
	PipelineLibrarySerializer PLS(COM_PTR_GET(Device), GetBasePath() + TEXT(".plo"));
#endif

	std::vector<std::thread> Threads;

	{
		const std::array<D3D12_SHADER_BYTECODE, 5> SBCs = { {
			{ ShaderBlobs[0]->GetBufferPointer(), ShaderBlobs[0]->GetBufferSize() },
			{ ShaderBlobs[1]->GetBufferPointer(), ShaderBlobs[1]->GetBufferSize() },
			{ ShaderBlobs[2]->GetBufferPointer(), ShaderBlobs[2]->GetBufferSize() },
			{ ShaderBlobs[3]->GetBufferPointer(), ShaderBlobs[3]->GetBufferSize() },
			{ ShaderBlobs[4]->GetBufferPointer(), ShaderBlobs[4]->GetBufferSize() },
		} };

		Threads.push_back(std::thread::thread([&](COM_PTR<ID3D12PipelineState>& PST, ID3D12RootSignature* RS,
			const D3D12_SHADER_BYTECODE VS, const D3D12_SHADER_BYTECODE PS, const D3D12_SHADER_BYTECODE DS, const D3D12_SHADER_BYTECODE HS, const D3D12_SHADER_BYTECODE GS)
			{
#ifdef USE_PIPELINE_SERIALIZE
				CreatePipelineState_Tesselation(PST, RS, VS, PS, DS, HS, GS, PLS.GetPipelineLibrary(), TEXT("0"), PLS.IsLoadSucceeded());
#else
				CreatePipelineState_Tesselation(PST, RS, VS, PS, DS, HS, GS);
#endif
			},
			std::ref(PipelineStates[0]), COM_PTR_GET(RootSignatures[0]), SBCs[0], SBCs[1], SBCs[2], SBCs[3], SBCs[4]));
	}

	for (auto& i : Threads) {
		i.join();
	}
}
//void DXExt::Clear_Color(ID3D12GraphicsCommandList* GraphicsCommandList)
//{
//	auto CPUDescriptorHandle(SwapChainDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
//	const auto IncrementSize = Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
//	CPUDescriptorHandle.ptr += CurrentBackBufferIndex * IncrementSize;
//	GraphicsCommandList->ClearRenderTargetView(CPUDescriptorHandle, DirectX::Colors::SkyBlue, 0, nullptr);
//}
//void DXExt::Clear_Depth(ID3D12GraphicsCommandList* GraphicsCommandList)
//{
//	if (nullptr != DepthStencilDescriptorHeap) {
//		auto CPUDescriptorHandle(DepthStencilDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
//		const auto IncrementSize = Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
//		CPUDescriptorHandle.ptr += 0 * IncrementSize;
//		GraphicsCommandList->ClearDepthStencilView(CPUDescriptorHandle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);
//	}
//}