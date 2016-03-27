#include "stdafx.h"

#include "DX.h"

//!< d3d12.lib が無いと言われる場合は、プロジェクトを右クリック - Retarget SDK Version で Windows10 にする
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")

DX::DX()
{

}
DX::~DX()
{

}

void DX::OnCreate(HWND hWnd, HINSTANCE hInstance)
{
	CreateDevice(hWnd);
	CreateRootSignature();
	CreatePipelineState();
	CreateCommandList();
}
void DX::OnSize(HWND hWnd, HINSTANCE hInstance)
{
}
void DX::OnTimer(HWND hWnd, HINSTANCE hInstance)
{
}
void DX::OnPaint(HWND hWnd, HINSTANCE hInstance)
{
}
void DX::OnDestroy(HWND hWnd, HINSTANCE hInstance)
{
}

void DX::CreateDevice(HWND hWnd)
{
	using namespace Microsoft::WRL;

#ifdef _DEBUG
	ComPtr<ID3D12Debug> Debug;
	VERIFY_SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&Debug)));
	Debug->EnableDebugLayer();
#endif

#pragma region CreateDevice
	ComPtr<IDXGIFactory4> Factory4;
	VERIFY_SUCCEEDED(CreateDXGIFactory1(IID_PPV_ARGS(&Factory4))); {
		if (false/*WarpDevice*/) { // todo
			ComPtr<IDXGIAdapter> Adapter;
			VERIFY_SUCCEEDED(Factory4->EnumWarpAdapter(IID_PPV_ARGS(&Adapter)));
			VERIFY_SUCCEEDED(D3D12CreateDevice(Adapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&Device)));
		}
		else {
			ComPtr<IDXGIAdapter1> Adapter;
			for (UINT i = 0; DXGI_ERROR_NOT_FOUND != Factory4->EnumAdapters1(i, &Adapter); ++i) {
				DXGI_ADAPTER_DESC1 AdapterDesc1;
				Adapter->GetDesc1(&AdapterDesc1);
				if (AdapterDesc1.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) {
					continue;
				}
				if (SUCCEEDED(D3D12CreateDevice(Adapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&Device)))) {
					break;
				}
			}
		}
	}
#pragma endregion

#pragma region CreateCommandQueue
	{
		D3D12_COMMAND_QUEUE_DESC CommandQueueDesc = {};
		CommandQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
		CommandQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
		VERIFY_SUCCEEDED(Device->CreateCommandQueue(&CommandQueueDesc, IID_PPV_ARGS(&CommandQueue)));
	}
#pragma endregion

#pragma region CreateSwapChain
	{
		DXGI_SWAP_CHAIN_DESC1 SwapChainDesc1 = {};
		SwapChainDesc1.BufferCount = 2;
		SwapChainDesc1.Width = 1280; // todo
		SwapChainDesc1.Height = 720; // todo
		SwapChainDesc1.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		SwapChainDesc1.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		SwapChainDesc1.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		SwapChainDesc1.SampleDesc.Count = 1;

		ComPtr<IDXGISwapChain1> SwapChain1;
		VERIFY_SUCCEEDED(Factory4->CreateSwapChainForHwnd(CommandQueue.Get(), hWnd, &SwapChainDesc1, nullptr, nullptr, &SwapChain1));
		VERIFY_SUCCEEDED(Factory4->MakeWindowAssociation(hWnd, DXGI_MWA_NO_ALT_ENTER));
		VERIFY_SUCCEEDED(SwapChain1.As(&SwapChain3));
		BackBufferIndex = SwapChain3->GetCurrentBackBufferIndex();
	}
#pragma endregion

#pragma region CreateRTV
	{
		D3D12_DESCRIPTOR_HEAP_DESC DescripterHeapDesc = {};
		DescripterHeapDesc.NumDescriptors = 2;
		DescripterHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		DescripterHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		VERIFY_SUCCEEDED(Device->CreateDescriptorHeap(&DescripterHeapDesc, IID_PPV_ARGS(&RenderTargetViewHeap)));

		const auto DescriptorSize = Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		D3D12_CPU_DESCRIPTOR_HANDLE RenderTargetViewHandle(RenderTargetViewHeap->GetCPUDescriptorHandleForHeapStart());
		for (UINT i = 0; i < 2; ++i) {
			VERIFY_SUCCEEDED(SwapChain3->GetBuffer(i, IID_PPV_ARGS(&RenderTargets[i])));
			Device->CreateRenderTargetView(RenderTargets[i].Get(), nullptr, RenderTargetViewHandle);
			RenderTargetViewHandle.ptr += DescriptorSize;
		}
	}
#pragma endregion
}

void DX::CreateRootSignature()
{
	using namespace Microsoft::WRL;

	D3D12_ROOT_SIGNATURE_DESC RootSignatureDesc;
	RootSignatureDesc.NumParameters = 0;
	RootSignatureDesc.pParameters = nullptr;
	RootSignatureDesc.NumStaticSamplers = 0;
	RootSignatureDesc.pStaticSamplers = nullptr;
	RootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	ComPtr<ID3DBlob> Blob;
	ComPtr<ID3DBlob> ErrorBlob;
	VERIFY_SUCCEEDED(D3D12SerializeRootSignature(&RootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &Blob, &ErrorBlob));
	VERIFY_SUCCEEDED(Device->CreateRootSignature(0, Blob->GetBufferPointer(), Blob->GetBufferSize(), IID_PPV_ARGS(&RootSignature)));
}

void DX::CreatePipelineState()
{
	D3D12_GRAPHICS_PIPELINE_STATE_DESC GraphicsPipelineStateDesc = {};
	GraphicsPipelineStateDesc.pRootSignature = RootSignature.Get();

	D3D12_INPUT_ELEMENT_DESC InputElementDescs[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	}; 
	GraphicsPipelineStateDesc.InputLayout = { InputElementDescs, _countof(InputElementDescs) };
	
	D3D12_SHADER_BYTECODE ShaderBytecode_VS = {}; {
		//ShaderBytecode_VS.pShaderBytecode = BlobVS->GetBufferPointer(); // todo
		//ShaderBytecode_VS.BytecodeLength = BlobVS->GetBufferSize(); // todo
	} GraphicsPipelineStateDesc.VS = ShaderBytecode_VS; 
	D3D12_SHADER_BYTECODE ShaderBytecode_PS = {}; {
		//ShaderBytecode_PS.pShaderBytecode = BlobPS->GetBufferPointer(); // todo
		//ShaderBytecode_PS.BytecodeLength = BlobPS->GetBufferSize(); // todo
	} GraphicsPipelineStateDesc.PS = ShaderBytecode_PS;
	
	D3D12_RASTERIZER_DESC RasterizerDesc = {}; {
		RasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;
		RasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
		RasterizerDesc.FrontCounterClockwise = FALSE;
		RasterizerDesc.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
		RasterizerDesc.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
		RasterizerDesc.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
		RasterizerDesc.DepthClipEnable = TRUE;
		RasterizerDesc.MultisampleEnable = FALSE;
		RasterizerDesc.AntialiasedLineEnable = FALSE;
		RasterizerDesc.ForcedSampleCount = 0;
		RasterizerDesc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;
	} GraphicsPipelineStateDesc.RasterizerState = RasterizerDesc;

	D3D12_BLEND_DESC BlendDesc = {}; {
		BlendDesc.AlphaToCoverageEnable = FALSE;
		BlendDesc.IndependentBlendEnable = FALSE;
		const D3D12_RENDER_TARGET_BLEND_DESC RenderTargetBlendDesc = {
			FALSE, FALSE,
			D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
			D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
			D3D12_LOGIC_OP_NOOP,
			D3D12_COLOR_WRITE_ENABLE_ALL,
		};
		for (auto& i : BlendDesc.RenderTarget) {
			i = RenderTargetBlendDesc;
		}
	} GraphicsPipelineStateDesc.BlendState = BlendDesc;

	GraphicsPipelineStateDesc.DepthStencilState.DepthEnable = FALSE;
	GraphicsPipelineStateDesc.DepthStencilState.StencilEnable = FALSE;
	GraphicsPipelineStateDesc.SampleMask = UINT_MAX;
	GraphicsPipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	GraphicsPipelineStateDesc.NumRenderTargets = 1;
	GraphicsPipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	GraphicsPipelineStateDesc.SampleDesc.Count = 1;
	VERIFY_SUCCEEDED(Device->CreateGraphicsPipelineState(&GraphicsPipelineStateDesc, IID_PPV_ARGS(&PipelineState)));
}

void DX::CreateCommandList()
{
	VERIFY_SUCCEEDED(Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&CommandAllocator)));
	VERIFY_SUCCEEDED(Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, CommandAllocator.Get(), PipelineState.Get(), IID_PPV_ARGS(&CommandList)));
	VERIFY_SUCCEEDED(CommandList->Close());
}


