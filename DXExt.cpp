#include "stdafx.h"

#include "DX.h"

void DXExt::CreateShader_VsPs()
{
	ShaderBlobs.resize(2);
	D3DReadFileToBlob(SHADER_PATH L"VS.cso", ShaderBlobs[0].GetAddressOf());
	D3DReadFileToBlob(SHADER_PATH L"PS.cso", ShaderBlobs[1].GetAddressOf());

#ifdef _DEBUG
	std::cout << "CreateShader" << COUT_OK << std::endl << std::endl;
#endif
}
void DXExt::CreateShader_VsPsDsHsGs()
{
	ShaderBlobs.resize(5);
	D3DReadFileToBlob(SHADER_PATH L"VS.cso", ShaderBlobs[0].GetAddressOf());
	D3DReadFileToBlob(SHADER_PATH L"PS.cso", ShaderBlobs[1].GetAddressOf());
	D3DReadFileToBlob(SHADER_PATH L"DS.cso", ShaderBlobs[2].GetAddressOf());
	D3DReadFileToBlob(SHADER_PATH L"HS.cso", ShaderBlobs[3].GetAddressOf());
	D3DReadFileToBlob(SHADER_PATH L"GS.cso", ShaderBlobs[4].GetAddressOf());

#ifdef _DEBUG
	std::cout << "CreateShader" << COUT_OK << std::endl << std::endl;
#endif
}
void DXExt::CreateShader_Cs()
{
	ShaderBlobs.resize(1);
	D3DReadFileToBlob(SHADER_PATH L"CS.cso", ShaderBlobs[0].GetAddressOf());

#ifdef _DEBUG
	std::cout << "CreateShader" << COUT_OK << std::endl << std::endl;
#endif
}

void DXExt::CreateInputLayout_Position()
{
	InputElementDescs = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	};
	InputLayoutDesc = {
		InputElementDescs.data(), static_cast<UINT>(InputElementDescs.size())
	};

#ifdef _DEBUG
	std::cout << "CreateInputLayout" << COUT_OK << std::endl << std::endl;
#endif
}
void DXExt::CreateInputLayout_PositionColor()
{
	InputElementDescs = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, sizeof(DirectX::XMFLOAT3), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};
	InputLayoutDesc = {
		InputElementDescs.data(), static_cast<UINT>(InputElementDescs.size())
	};

#ifdef _DEBUG
	std::cout << "CreateInputLayout" << COUT_OK << std::endl << std::endl;
#endif
}

void DXExt::CreateGraphicsPipelineState_VsPs()
{
	assert(nullptr != RootSignature);
	assert(1 < ShaderBlobs.size());

	const D3D12_SHADER_BYTECODE ShaderBytecodesVS = { ShaderBlobs[0]->GetBufferPointer(), ShaderBlobs[0]->GetBufferSize() };
	const D3D12_SHADER_BYTECODE ShaderBytecodesPS = { ShaderBlobs[1]->GetBufferPointer(), ShaderBlobs[1]->GetBufferSize() };
	const D3D12_SHADER_BYTECODE DefaultShaderBytecode = { nullptr, 0 };

	const D3D12_STREAM_OUTPUT_DESC StreamOutputDesc = {
		nullptr, 0,
		nullptr, 0,
		0
	};

	const D3D12_RENDER_TARGET_BLEND_DESC DefaultRenderTargetBlendDesc = {
		FALSE, FALSE,
		D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
		D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
		D3D12_LOGIC_OP_NOOP,
		D3D12_COLOR_WRITE_ENABLE_ALL,
	};
	const D3D12_BLEND_DESC BlendDesc = {
		FALSE,
		FALSE,
		{ DefaultRenderTargetBlendDesc/*, ... x8*/ }
	};

	const D3D12_RASTERIZER_DESC RasterizerDesc = {
		D3D12_FILL_MODE_SOLID,
		D3D12_CULL_MODE_BACK, FALSE,
		D3D12_DEFAULT_DEPTH_BIAS, D3D12_DEFAULT_DEPTH_BIAS_CLAMP, D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS,
		TRUE,
		FALSE,
		FALSE,
		0,
		D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF
	};

	const D3D12_DEPTH_STENCILOP_DESC DepthStencilOpDesc = {
		D3D12_STENCIL_OP_KEEP,
		D3D12_STENCIL_OP_KEEP,
		D3D12_STENCIL_OP_KEEP,
		D3D12_COMPARISON_FUNC_NEVER
	};

	const D3D12_DEPTH_STENCIL_DESC DepthStencilDesc = {
		FALSE,
		D3D12_DEPTH_WRITE_MASK_ZERO,
		D3D12_COMPARISON_FUNC_NEVER,
		FALSE,
		0,
		0,
		DepthStencilOpDesc,
		DepthStencilOpDesc
	};

	const DXGI_SAMPLE_DESC SampleDesc = { 1/*4*/, 0 };
	const D3D12_CACHED_PIPELINE_STATE CachedPipelineState = { nullptr, 0 };
	const D3D12_GRAPHICS_PIPELINE_STATE_DESC GraphicsPipelineStateDesc = {
		RootSignature.Get(),
		ShaderBytecodesVS, ShaderBytecodesPS, DefaultShaderBytecode, DefaultShaderBytecode, DefaultShaderBytecode,
		StreamOutputDesc,
		BlendDesc,
		UINT_MAX,
		RasterizerDesc,
		DepthStencilDesc,
		InputLayoutDesc,
		D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED,
		D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE,
		1,{ DXGI_FORMAT_R8G8B8A8_UNORM/*, ... x8*/ },
		DXGI_FORMAT_D32_FLOAT_S8X24_UINT,
		SampleDesc,
		0,
		CachedPipelineState,
		D3D12_PIPELINE_STATE_FLAG_NONE //!< D3D12_PIPELINE_STATE_FLAG_TOOL_DEBUG ... �� Warp �f�o�C�X�̂�
	};

	VERIFY_SUCCEEDED(Device->CreateGraphicsPipelineState(&GraphicsPipelineStateDesc, IID_PPV_ARGS(PipelineState.GetAddressOf())));

#ifdef _DEBUG
	std::cout << "CreateGraphicsPipelineState" << COUT_OK << std::endl << std::endl;
#endif
}
void DXExt::CreateGraphicsPipelineState_VsPsDsHsGs()
{
	assert(nullptr != RootSignature);
	assert(4 < ShaderBlobs.size());

	const D3D12_SHADER_BYTECODE ShaderBytecodesVS = { ShaderBlobs[0]->GetBufferPointer(), ShaderBlobs[0]->GetBufferSize() };
	const D3D12_SHADER_BYTECODE ShaderBytecodesPS = { ShaderBlobs[1]->GetBufferPointer(), ShaderBlobs[1]->GetBufferSize() };
	const D3D12_SHADER_BYTECODE ShaderBytecodesDS = { ShaderBlobs[2]->GetBufferPointer(), ShaderBlobs[2]->GetBufferSize() };
	const D3D12_SHADER_BYTECODE ShaderBytecodesHS = { ShaderBlobs[3]->GetBufferPointer(), ShaderBlobs[3]->GetBufferSize() };
	const D3D12_SHADER_BYTECODE ShaderBytecodesGS = { ShaderBlobs[4]->GetBufferPointer(), ShaderBlobs[4]->GetBufferSize() };

	const D3D12_STREAM_OUTPUT_DESC StreamOutputDesc = {
		nullptr, 0,
		nullptr, 0,
		0
	};

	const D3D12_RENDER_TARGET_BLEND_DESC DefaultRenderTargetBlendDesc = {
		FALSE, FALSE,
		D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
		D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
		D3D12_LOGIC_OP_NOOP,
		D3D12_COLOR_WRITE_ENABLE_ALL,
	};
	const D3D12_BLEND_DESC BlendDesc = {
		FALSE,
		FALSE,
		{ DefaultRenderTargetBlendDesc }
	};

	const D3D12_RASTERIZER_DESC RasterizerDesc = {
		D3D12_FILL_MODE_SOLID,
		D3D12_CULL_MODE_BACK, FALSE,
		D3D12_DEFAULT_DEPTH_BIAS, D3D12_DEFAULT_DEPTH_BIAS_CLAMP, D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS,
		TRUE,
		FALSE,
		FALSE,
		0,
		D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF
	};

	const D3D12_DEPTH_STENCILOP_DESC DepthStencilOpDesc = {
		D3D12_STENCIL_OP_KEEP,
		D3D12_STENCIL_OP_KEEP,
		D3D12_STENCIL_OP_KEEP,
		D3D12_COMPARISON_FUNC_NEVER
	};

	const D3D12_DEPTH_STENCIL_DESC DepthStencilDesc = {
		FALSE,
		D3D12_DEPTH_WRITE_MASK_ZERO,
		D3D12_COMPARISON_FUNC_NEVER,
		FALSE,
		0,
		0,
		DepthStencilOpDesc,
		DepthStencilOpDesc
	};

	const DXGI_SAMPLE_DESC SampleDesc = { 1, 0 };
	const D3D12_CACHED_PIPELINE_STATE CachedPipelineState = { nullptr, 0 };
	const D3D12_GRAPHICS_PIPELINE_STATE_DESC GraphicsPipelineStateDesc = {
		RootSignature.Get(),
		ShaderBytecodesVS, ShaderBytecodesPS, ShaderBytecodesDS, ShaderBytecodesHS, ShaderBytecodesGS,
		StreamOutputDesc,
		BlendDesc,
		UINT_MAX,
		RasterizerDesc,
		DepthStencilDesc,
		InputLayoutDesc,
		D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED,
		D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH, //!< �g�|���W�^�C�v���p�b�`�ɂ��Ă���
		1,{ DXGI_FORMAT_R8G8B8A8_UNORM },
		DXGI_FORMAT_D32_FLOAT_S8X24_UINT,
		SampleDesc,
		0,
		CachedPipelineState,
		D3D12_PIPELINE_STATE_FLAG_NONE
	};

	VERIFY_SUCCEEDED(Device->CreateGraphicsPipelineState(&GraphicsPipelineStateDesc, IID_PPV_ARGS(PipelineState.GetAddressOf())));

#ifdef _DEBUG
	std::cout << "CreateGraphicsPipelineState" << COUT_OK << std::endl << std::endl;
#endif
}

void DXExt::Clear_Color(ID3D12GraphicsCommandList* GraphicsCommandList)
{
	auto CPUDescriptorHandle(SwapChainDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
	const auto IncrementSize = Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	CPUDescriptorHandle.ptr += CurrentBackBufferIndex * IncrementSize;
	GraphicsCommandList->ClearRenderTargetView(CPUDescriptorHandle, DirectX::Colors::SkyBlue, 0, nullptr);
}
void DXExt::Clear_Depth(ID3D12GraphicsCommandList* GraphicsCommandList)
{
	if (nullptr != DepthStencilDescriptorHeap) {
		auto CPUDescriptorHandle(DepthStencilDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
		const auto IncrementSize = Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
		CPUDescriptorHandle.ptr += 0 * IncrementSize;
		GraphicsCommandList->ClearDepthStencilView(CPUDescriptorHandle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);
	}
}