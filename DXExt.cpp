#include "stdafx.h"

#include "DXExt.h"

void DXExt::CreateIndirectBuffer_Draw(const UINT Count)
{
	const D3D12_DRAW_ARGUMENTS Source = { Count, 1, 0, 0 };
	const auto Stride = sizeof(Source);
	const auto Size = static_cast<UINT32>(Stride * 1);
#ifdef USE_WINRT
	const auto CA = CommandAllocators[0].get();
	const auto CL = GraphicsCommandLists[0].get(); 
	CreateIndirectBuffer(IndirectBufferResource.put(), Size, &Source, CA, CL);
#elif defined(USE_WRL)
	const auto CA = CommandAllocators[0].Get();
	const auto CL = GraphicsCommandLists[0].Get();
	CreateIndirectBuffer(IndirectBufferResource.GetAddressOf(), Size, &Source, CA, CL);
#endif

	const std::vector<D3D12_INDIRECT_ARGUMENT_DESC> IndArgDescs = {
		{ D3D12_INDIRECT_ARGUMENT_TYPE_DRAW },
	};
	const D3D12_COMMAND_SIGNATURE_DESC CmdSigDesc = {
		Stride,
		static_cast<const UINT>(IndArgDescs.size()), IndArgDescs.data(),
		0
	};
#ifdef USE_WINRT
	Device->CreateCommandSignature(&CmdSigDesc, RootSignature.get(), __uuidof(IndirectCommandSignature), IndirectCommandSignature.put_void());
#elif defined(USE_WRL)
	Device->CreateCommandSignature(&CmdSigDesc, RootSignature.Get(), IID_PPV_ARGS(IndirectCommandSignature.GetAddressOf()));
#endif
}

void DXExt::CreateIndirectBuffer_DrawIndexed(const UINT Count)
{
	const D3D12_DRAW_INDEXED_ARGUMENTS Source = { Count, 1, 0, 0, 0 };
	const auto Stride = sizeof(Source);
	const auto Size = static_cast<UINT32>(Stride * 1);

#ifdef USE_WINRT
	const auto CA = CommandAllocators[0].get();
	const auto CL = GraphicsCommandLists[0].get();
	CreateIndirectBuffer(IndirectBufferResource.put(), Size, &Source, CA, CL);
#elif defined(USE_WRL)
	const auto CA = CommandAllocators[0].Get();
	const auto CL = GraphicsCommandLists[0].Get();
	CreateIndirectBuffer(IndirectBufferResource.GetAddressOf(), Size, &Source, CA, CL);
#endif

	const std::vector<D3D12_INDIRECT_ARGUMENT_DESC> IndArgDescs = {
		{ D3D12_INDIRECT_ARGUMENT_TYPE_DRAW_INDEXED },
	};
	const D3D12_COMMAND_SIGNATURE_DESC CmdSigDesc = {
		Stride,
		static_cast<const UINT>(IndArgDescs.size()), IndArgDescs.data(),
		0
	};
#ifdef USE_WINRT
	Device->CreateCommandSignature(&CmdSigDesc, RootSignature.get(), __uuidof(IndirectCommandSignature), IndirectCommandSignature.put_void());
#elif defined(USE_WRL)
	Device->CreateCommandSignature(&CmdSigDesc, RootSignature.Get(), IID_PPV_ARGS(IndirectCommandSignature.GetAddressOf()));
#endif
}

void DXExt::CreateIndirectBuffer_Dispatch(const UINT X, const UINT Y, const UINT Z)
{
	const D3D12_DISPATCH_ARGUMENTS Source = { X, Y, Z };
	const auto Stride = sizeof(Source);
	const auto Size = static_cast<UINT32>(Stride * 1);
#ifdef USE_WINRT
	const auto CA = CommandAllocators[0].get();
	const auto CL = GraphicsCommandLists[0].get();
	CreateIndirectBuffer(IndirectBufferResource.put(), Size, &Source, CA, CL);
#elif defined(USE_WRL)
	const auto CA = CommandAllocators[0].Get();
	const auto CL = GraphicsCommandLists[0].Get();
	CreateIndirectBuffer(IndirectBufferResource.GetAddressOf(), Size, &Source, CA, CL);
#endif

	const std::vector<D3D12_INDIRECT_ARGUMENT_DESC> IndArgDescs = {
		{ D3D12_INDIRECT_ARGUMENT_TYPE_DISPATCH },
	};
	const D3D12_COMMAND_SIGNATURE_DESC CmdSigDesc = {
		Stride,
		static_cast<const UINT>(IndArgDescs.size()), IndArgDescs.data(),
		0
	};
#ifdef USE_WINRT
	Device->CreateCommandSignature(&CmdSigDesc, RootSignature.get(), __uuidof(IndirectCommandSignature), IndirectCommandSignature.put_void());
#elif defined(USE_WRL)
	Device->CreateCommandSignature(&CmdSigDesc, RootSignature.Get(), IID_PPV_ARGS(IndirectCommandSignature.GetAddressOf()));
#endif
}

//void DXExt::CreateStaticSamplerDesc_LW(D3D12_STATIC_SAMPLER_DESC& StaticSamplerDesc, const D3D12_SHADER_VISIBILITY ShaderVisibility, const FLOAT MaxLOD) const
//{
//	//!< �V�F�[�_���ł̋L�q�� SamplerState Sampler : register(s0, space0);
//	//!< DX�ɂ͐��K�����W�̐ݒ�͖����A�V�F�[�_�r�W�r���e�B�g���W�X�^�̐ݒ肪����
//	StaticSamplerDesc = {
//			D3D12_FILTER_MIN_MAG_MIP_LINEAR, // min, mag, mip
//			D3D12_TEXTURE_ADDRESS_MODE_WRAP, D3D12_TEXTURE_ADDRESS_MODE_WRAP, D3D12_TEXTURE_ADDRESS_MODE_WRAP, // u, v, w
//			0.0f, // lod bias
//			0, // anisotropy
//			D3D12_COMPARISON_FUNC_NEVER, // compare
//			D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE, // border
//			0.0f, MaxLOD, // min, maxlod
//			0, 0, ShaderVisibility //!< UINT ShaderRegister, UINT RegisterSpace, D3D12_SHADER_VISIBILITY ShaderVisibility
//	};
//}

void DXExt::CreatePipelineState_Tesselation(winrt::com_ptr<ID3D12PipelineState>& PipelineState, ID3D12RootSignature* RS, const D3D12_SHADER_BYTECODE VS, const D3D12_SHADER_BYTECODE PS, const D3D12_SHADER_BYTECODE DS, const D3D12_SHADER_BYTECODE HS, const D3D12_SHADER_BYTECODE GS)
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
		D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH, //!< �g�|���W�� PATCH ���w��
		1, { DXGI_FORMAT_R8G8B8A8_UNORM }, DXGI_FORMAT_D32_FLOAT_S8X24_UINT,
		SD,
		0,
		CPS,
		D3D12_PIPELINE_STATE_FLAG_NONE
	};
	assert(GPSD.NumRenderTargets <= _countof(GPSD.RTVFormats) && "");

	//!< DX�ł́u�p�b�`�R���g���[���|�C���g�v�̎w���IASetPrimitiveTopology()�̈����Ƃ��ăR�}���h���X�g�֎w�肷��AVK�Ƃ͌��\�قȂ�̂Œ���
	//!< CommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_1_CONTROL_POINT_PATCHLIST);

#ifdef USE_WINRT
	VERIFY_SUCCEEDED(Device->CreateGraphicsPipelineState(&GPSD, __uuidof(PipelineState), PipelineState.put_void()));
#elif defined(USE_WRL)
	VERIFY_SUCCEEDED(Device->CreateGraphicsPipelineState(&GPSD, IID_PPV_ARGS(PipelineState.GetAddressOf())));
#endif

	LOG_OK();
}

void DXExt::CreateShaderBlob_VsPs()
{
	ShaderBlobs.resize(2);
	const auto ShaderPath = GetBasePath();
#ifdef USE_WINRT
	VERIFY_SUCCEEDED(D3DReadFileToBlob((ShaderPath + TEXT(".vs.cso")).data(), ShaderBlobs[0].put()));
	VERIFY_SUCCEEDED(D3DReadFileToBlob((ShaderPath + TEXT(".ps.cso")).data(), ShaderBlobs[1].put()));
#elif defined(USE_WRL)
	VERIFY_SUCCEEDED(D3DReadFileToBlob((ShaderPath + TEXT(".vs.cso")).data(), ShaderBlobs[0].GetAddressOf()));
	VERIFY_SUCCEEDED(D3DReadFileToBlob((ShaderPath + TEXT(".ps.cso")).data(), ShaderBlobs[1].GetAddressOf()));
#endif
}
void DXExt::CreateShaderBlob_VsPsDsHsGs()
{
	ShaderBlobs.resize(5);
	const auto ShaderPath = GetBasePath();
#ifdef USE_WINRT
	VERIFY_SUCCEEDED(D3DReadFileToBlob((ShaderPath + TEXT(".vs.cso")).data(), ShaderBlobs[0].put()));
	VERIFY_SUCCEEDED(D3DReadFileToBlob((ShaderPath + TEXT(".ps.cso")).data(), ShaderBlobs[1].put()));
	VERIFY_SUCCEEDED(D3DReadFileToBlob((ShaderPath + TEXT(".ds.cso")).data(), ShaderBlobs[2].put()));
	VERIFY_SUCCEEDED(D3DReadFileToBlob((ShaderPath + TEXT(".hs.cso")).data(), ShaderBlobs[3].put()));
	VERIFY_SUCCEEDED(D3DReadFileToBlob((ShaderPath + TEXT(".gs.cso")).data(), ShaderBlobs[4].put()));
#elif defined(USE_WRL)
	VERIFY_SUCCEEDED(D3DReadFileToBlob((ShaderPath + TEXT(".vs.cso")).data(), ShaderBlobs[0].GetAddressOf()));
	VERIFY_SUCCEEDED(D3DReadFileToBlob((ShaderPath + TEXT(".ps.cso")).data(), ShaderBlobs[1].GetAddressOf()));
	VERIFY_SUCCEEDED(D3DReadFileToBlob((ShaderPath + TEXT(".ds.cso")).data(), ShaderBlobs[2].GetAddressOf()));
	VERIFY_SUCCEEDED(D3DReadFileToBlob((ShaderPath + TEXT(".hs.cso")).data(), ShaderBlobs[3].GetAddressOf()));
	VERIFY_SUCCEEDED(D3DReadFileToBlob((ShaderPath + TEXT(".gs.cso")).data(), ShaderBlobs[4].GetAddressOf()));
#endif
}
void DXExt::CreateShaderBlob_Cs()
{
	ShaderBlobs.resize(1);
	const auto ShaderPath = GetBasePath();
#ifdef USE_WINRT
	VERIFY_SUCCEEDED(D3DReadFileToBlob((ShaderPath + TEXT(".cs.cso")).data(), ShaderBlobs[0].put()));
#elif defined(USE_WRL)
	VERIFY_SUCCEEDED(D3DReadFileToBlob((ShaderPath + TEXT(".cs.cso")).data(), ShaderBlobs[0].GetAddressOf()));
#endif
}

void DXExt::CreatePipelineState_VsPs()
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
				CreatePipelineState_Default(Pipe, RS, VS, PS, DS, HS, GS);
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
void DXExt::CreatePipelineState_VsPsDsHsGs_Tesselation()
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

		assert(ShaderBlobs.size() > 4 && "");
		const std::array<D3D12_SHADER_BYTECODE, 5> SBCs = { {
			{ ShaderBlobs[0]->GetBufferPointer(), ShaderBlobs[0]->GetBufferSize() },
			{ ShaderBlobs[1]->GetBufferPointer(), ShaderBlobs[1]->GetBufferSize() },
			{ ShaderBlobs[2]->GetBufferPointer(), ShaderBlobs[2]->GetBufferSize() },
			{ ShaderBlobs[3]->GetBufferPointer(), ShaderBlobs[3]->GetBufferSize() },
			{ ShaderBlobs[4]->GetBufferPointer(), ShaderBlobs[4]->GetBufferSize() },
		} };
		auto Thread = std::thread::thread([&](winrt::com_ptr<ID3D12PipelineState>& Pipe, ID3D12RootSignature* RS,
			const D3D12_SHADER_BYTECODE VS, const D3D12_SHADER_BYTECODE PS, const D3D12_SHADER_BYTECODE DS, const D3D12_SHADER_BYTECODE HS, const D3D12_SHADER_BYTECODE GS)
			{
				CreatePipelineState_Tesselation(Pipe, RS, VS, PS, DS, HS, GS);
			},
			std::ref(PipelineState), RootSignature.get(), SBCs[0], SBCs[1], SBCs[2], SBCs[3], SBCs[4]);

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