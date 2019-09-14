#pragma once

//!< テンプレート特殊化
//!< template specialization

template<> void CreatePipelineState_Vertex<Vertex_PositionColor>(/*winrt::com_ptr*/COM_PTR<ID3D12PipelineState>& PipelineState, ID3D12RootSignature* RS, const D3D12_SHADER_BYTECODE VS, const D3D12_SHADER_BYTECODE PS, const D3D12_SHADER_BYTECODE DS, const D3D12_SHADER_BYTECODE HS, const D3D12_SHADER_BYTECODE GS)
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

	//!< インプットエレメントを指定
	const std::array<D3D12_INPUT_ELEMENT_DESC, 2> IEDs = { {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(Vertex_PositionColor, Position), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, offsetof(Vertex_PositionColor, Color), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
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
		D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE, //!< トポロジに TRIANGLE を指定
		1, { DXGI_FORMAT_R8G8B8A8_UNORM }, DXGI_FORMAT_D32_FLOAT_S8X24_UINT,
		SD,
		0,
		CPS,
		D3D12_PIPELINE_STATE_FLAG_NONE
	};
	assert(GPSD.NumRenderTargets <= _countof(GPSD.RTVFormats) && "");

#ifdef USE_WINRT
	VERIFY_SUCCEEDED(Device->CreateGraphicsPipelineState(&GPSD, __uuidof(PipelineState), PipelineState.put_void()));
#elif defined(USE_WRL)
	VERIFY_SUCCEEDED(Device->CreateGraphicsPipelineState(&GPSD, IID_PPV_ARGS(PipelineState.GetAddressOf())));
#endif

	LOG_OK();
}

template<typename T> void CreatePipelineState_VsPs_Vertex()
{
	const auto PCOPath = GetBasePath() + TEXT(".pco");
	DeleteFile(PCOPath.data());

#ifdef USE_WINRT
	winrt::com_ptr<ID3D12Device1> Device1;
	VERIFY_SUCCEEDED(Device->QueryInterface(__uuidof(Device1), Device1.put_void()));

	winrt::com_ptr<ID3D12PipelineLibrary> PL;
	winrt::com_ptr<ID3DBlob> Blob;
	if (SUCCEEDED(D3DReadFileToBlob(PCOPath.c_str(), Blob.put())) && Blob->GetBufferSize()) {
		VERIFY_SUCCEEDED(Device1->CreatePipelineLibrary(Blob->GetBufferPointer(), Blob->GetBufferSize(), __uuidof(PL), PL.put_void()));

		winrt::com_ptr<ID3D12PipelineState> PS;
		const D3D12_GRAPHICS_PIPELINE_STATE_DESC GPSD = {};
		VERIFY_SUCCEEDED(PL->LoadGraphicsPipeline(TEXT("0"), &GPSD, __uuidof(PS), PS.put_void()));
	}
	else {
		VERIFY_SUCCEEDED(Device1->CreatePipelineLibrary(nullptr, 0, __uuidof(PipelineLibrary), PL.put_void()));

		assert(ShaderBlobs.size() > 1 && "");

		const std::array<D3D12_SHADER_BYTECODE, 2> SBCs = { {
			{ ShaderBlobs[0]->GetBufferPointer(), ShaderBlobs[0]->GetBufferSize() },
			{ ShaderBlobs[1]->GetBufferPointer(), ShaderBlobs[1]->GetBufferSize() },
		} };
		auto Thread = std::thread::thread([&](winrt::com_ptr<ID3D12PipelineState>& Pipe, ID3D12RootSignature* RS,
			const D3D12_SHADER_BYTECODE VS, const D3D12_SHADER_BYTECODE PS, const D3D12_SHADER_BYTECODE DS, const D3D12_SHADER_BYTECODE HS, const D3D12_SHADER_BYTECODE GS)
			{
				CreatePipelineState_Vertex<T>(Pipe, RS, VS, PS, DS, HS, GS);
			},
			std::ref(PipelineState), RootSignature.get(), SBCs[0], SBCs[1], NullShaderBC, NullShaderBC, NullShaderBC);

		Thread.join();

		VERIFY_SUCCEEDED(PL->StorePipeline(TEXT("0"), PipelineState.get()));

		const auto Size = PL->GetSerializedSize();
		if (Size) {
			winrt::com_ptr<ID3DBlob> Blob;
			VERIFY_SUCCEEDED(D3DCreateBlob(Size, Blob.put()));
			PL->Serialize(Blob->GetBufferPointer(), Size);
			VERIFY_SUCCEEDED(D3DWriteBlobToFile(Blob.get(), PCOPath.c_str(), TRUE));
		}
	}
#elif defined(USE_WRL)
	//!< #DX_TODO
#endif
}
