#pragma once

//!< テンプレート特殊化
//!< template specialization

template<> void CreatePipelineState_Vertex<Vertex_PositionColor>(COM_PTR<ID3D12PipelineState>& PST, ID3D12RootSignature* RS, 
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

	//!< 詰まっている場合は offsetof() の代わりに D3D12_APPEND_ALIGNED_ELEMENT で良い (When directly after the previous one, we can use D3D12_APPEND_ALIGNED_ELEMENT)
	const std::array<D3D12_INPUT_ELEMENT_DESC, 2> IEDs = { {
#if 0
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(Vertex_PositionColor, Position), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, offsetof(Vertex_PositionColor, Color), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
#else
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
#endif
	} };
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
		D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE, //!< TRIANGLE
		1, { DXGI_FORMAT_R8G8B8A8_UNORM }, DXGI_FORMAT_D32_FLOAT_S8X24_UINT,
		SD,
		0,
		CPS,
		D3D12_PIPELINE_STATE_FLAG_NONE
	};
	assert(GPSD.NumRenderTargets <= _countof(GPSD.RTVFormats) && "");

	if (IsLoad) {
		if (nullptr != PL && nullptr != Name) {
			VERIFY_SUCCEEDED(PL->LoadGraphicsPipeline(Name, &GPSD, COM_PTR_UUIDOF_PUTVOID(PST)));
		}
	}
	else {
		VERIFY_SUCCEEDED(Device->CreateGraphicsPipelineState(&GPSD, COM_PTR_UUIDOF_PUTVOID(PST)));
		if (nullptr != PL && nullptr != Name) {
			VERIFY_SUCCEEDED(PL->StorePipeline(Name, COM_PTR_GET(PST)));
		}
	}

	LOG_OK();
}

template<typename T> void CreatePipelineState_VsPs_Vertex()
{
	PipelineStates.resize(1);

#ifdef USE_PIPELINE_SERIALIZE
	const auto PCOPath = GetBasePath() + TEXT(".plo");
	PipelineLibrarySerializer PLS(COM_PTR_GET(Device), PCOPath.c_str());
#endif

	std::vector<std::thread> Threads;

	{
		const std::array<D3D12_SHADER_BYTECODE, 2> SBCs = { {
			{ ShaderBlobs[0]->GetBufferPointer(), ShaderBlobs[0]->GetBufferSize() },
			{ ShaderBlobs[1]->GetBufferPointer(), ShaderBlobs[1]->GetBufferSize() },
		} };
		Threads.push_back(std::thread::thread([&](COM_PTR<ID3D12PipelineState>& Pipe, ID3D12RootSignature* RS,
			const D3D12_SHADER_BYTECODE VS, const D3D12_SHADER_BYTECODE PS, const D3D12_SHADER_BYTECODE DS, const D3D12_SHADER_BYTECODE HS, const D3D12_SHADER_BYTECODE GS)
			{
#ifdef USE_PIPELINE_SERIALIZE
				CreatePipelineState_Vertex<T>(Pipe, RS, VS, PS, DS, HS, GS, PLS.GetPipelineLibrary(), TEXT("0"), PLS.IsLoadSucceeded());
#else
				CreatePipelineState_Vertex<T>(Pipe, RS, VS, PS, DS, HS, GS);
#endif
			},
			std::ref(PipelineStates[0]), COM_PTR_GET(RootSignatures[0]), SBCs[0], SBCs[1], NullShaderBC, NullShaderBC, NullShaderBC));
	}

	for (auto& i : Threads) {
		i.join();
	}
}
