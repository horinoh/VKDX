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

void DXExt::CreateStaticSamplerDesc_LW(D3D12_STATIC_SAMPLER_DESC& StaticSamplerDesc, const D3D12_SHADER_VISIBILITY ShaderVisibility, const FLOAT MaxLOD) const
{
	//!< シェーダ内での記述例 SamplerState Sampler : register(s0, space0);
	//!< DXには正規化座標の設定は無く、シェーダビジビリティトレジスタの設定がある
	StaticSamplerDesc = {
			D3D12_FILTER_MIN_MAG_MIP_LINEAR, // min, mag, mip
			D3D12_TEXTURE_ADDRESS_MODE_WRAP, D3D12_TEXTURE_ADDRESS_MODE_WRAP, D3D12_TEXTURE_ADDRESS_MODE_WRAP, // u, v, w
			0.0f, // lod bias
			0, // anisotropy
			D3D12_COMPARISON_FUNC_NEVER, // compare
			D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE, // border
			0.0f, MaxLOD, // min, maxlod
			0, 0, ShaderVisibility //!< UINT ShaderRegister, UINT RegisterSpace, D3D12_SHADER_VISIBILITY ShaderVisibility
	};
}

#ifdef USE_WINRT
void DXExt::CreateShader_VsPs(std::vector<winrt::com_ptr<ID3DBlob>>& ShaderBlobs) const
{
	const auto ShaderPath = GetBasePath();
	ShaderBlobs.resize(2);
	VERIFY_SUCCEEDED(D3DReadFileToBlob((ShaderPath + TEXT(".vs.cso")).data(), ShaderBlobs[0].put()));
	VERIFY_SUCCEEDED(D3DReadFileToBlob((ShaderPath + TEXT(".ps.cso")).data(), ShaderBlobs[1].put()));
}
#elif defined(USE_WRL)
void DXExt::CreateShader_VsPs(std::vector<Microsoft::WRL::ComPtr<ID3DBlob>>& ShaderBlobs) const
{
	const auto ShaderPath = GetBasePath();
	ShaderBlobs.resize(2);
	VERIFY_SUCCEEDED(D3DReadFileToBlob((ShaderPath + TEXT(".vs.cso")).data(), ShaderBlobs[0].GetAddressOf()));
	VERIFY_SUCCEEDED(D3DReadFileToBlob((ShaderPath + TEXT(".ps.cso")).data(), ShaderBlobs[1].GetAddressOf()));
}
#endif

#ifdef USE_WINRT
void DXExt::CreateShader_VsPsDsHsGs(std::vector<winrt::com_ptr<ID3DBlob>>& ShaderBlobs) const
{
	const auto ShaderPath = GetBasePath();
	ShaderBlobs.resize(5);
	VERIFY_SUCCEEDED(D3DReadFileToBlob((ShaderPath + TEXT(".vs.cso")).data(), ShaderBlobs[0].put()));
	VERIFY_SUCCEEDED(D3DReadFileToBlob((ShaderPath + TEXT(".ps.cso")).data(), ShaderBlobs[1].put()));
	VERIFY_SUCCEEDED(D3DReadFileToBlob((ShaderPath + TEXT(".ds.cso")).data(), ShaderBlobs[2].put()));
	VERIFY_SUCCEEDED(D3DReadFileToBlob((ShaderPath + TEXT(".hs.cso")).data(), ShaderBlobs[3].put()));
	VERIFY_SUCCEEDED(D3DReadFileToBlob((ShaderPath + TEXT(".gs.cso")).data(), ShaderBlobs[4].put()));
}
#elif defined(USE_WRL)
void DXExt::CreateShader_VsPsDsHsGs(std::vector<Microsoft::WRL::ComPtr<ID3DBlob>>& ShaderBlobs) const
{
	const auto ShaderPath = GetBasePath();
	ShaderBlobs.resize(5);
	VERIFY_SUCCEEDED(D3DReadFileToBlob((ShaderPath + TEXT(".vs.cso")).data(), ShaderBlobs[0].GetAddressOf()));
	VERIFY_SUCCEEDED(D3DReadFileToBlob((ShaderPath + TEXT(".ps.cso")).data(), ShaderBlobs[1].GetAddressOf()));
	VERIFY_SUCCEEDED(D3DReadFileToBlob((ShaderPath + TEXT(".ds.cso")).data(), ShaderBlobs[2].GetAddressOf()));
	VERIFY_SUCCEEDED(D3DReadFileToBlob((ShaderPath + TEXT(".hs.cso")).data(), ShaderBlobs[3].GetAddressOf()));
	VERIFY_SUCCEEDED(D3DReadFileToBlob((ShaderPath + TEXT(".gs.cso")).data(), ShaderBlobs[4].GetAddressOf()));
}
#endif

#ifdef USE_WINRT
void DXExt::CreateShader_Cs(std::vector<winrt::com_ptr<ID3DBlob>>& ShaderBlobs) const
{
	const auto ShaderPath = GetBasePath();
	ShaderBlobs.resize(1);
	VERIFY_SUCCEEDED(D3DReadFileToBlob((ShaderPath + TEXT(".cs.cso")).data(), ShaderBlobs[0].put()));
}
#elif defined(USE_WRL)
void DXExt::CreateShader_Cs(std::vector<Microsoft::WRL::ComPtr<ID3DBlob>>& ShaderBlobs) const
{
	const auto ShaderPath = GetBasePath();
	ShaderBlobs.resize(1);
	VERIFY_SUCCEEDED(D3DReadFileToBlob((ShaderPath + TEXT(".cs.cso")).data(), ShaderBlobs[0].GetAddressOf()));
}
#endif

void DXExt::CreatePipelineState_VertexPositionColor(winrt::com_ptr<ID3D12PipelineState>& PipelineState, ID3D12RootSignature* RS, const D3D12_SHADER_BYTECODE VS, const D3D12_SHADER_BYTECODE PS, const D3D12_SHADER_BYTECODE DS, const D3D12_SHADER_BYTECODE HS, const D3D12_SHADER_BYTECODE GS)
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

#ifdef USE_WINRT
	VERIFY_SUCCEEDED(Device->CreateGraphicsPipelineState(&GPSD, __uuidof(PipelineState), PipelineState.put_void()));
#elif defined(USE_WRL)
	VERIFY_SUCCEEDED(Device->CreateGraphicsPipelineState(&GPSD, IID_PPV_ARGS(PipelineState.GetAddressOf())));
#endif

	LOG_OK();
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