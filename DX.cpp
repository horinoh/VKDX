#include "stdafx.h"

#include "DX.h"

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "dxgi.lib")

DX::DX()
{
}
DX::~DX()
{
}

void DX::OnCreate(HWND hWnd, HINSTANCE hInstance)
{
#ifdef _DEBUG
	__int64 A;
	QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&A));
#endif

	Super::OnCreate(hWnd, hInstance);
	
	const auto ColorFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
	CreateDevice(hWnd, ColorFormat);
	CreateCommandQueue();
	CreateCommandList();

	CreateSwapChain(hWnd, ColorFormat);
	CreateDepthStencil();

	CreateShader();

	//CreateRootSignature();

	CreateInputLayout();
	//CreateViewport();
	//CreatePipelineState();

	CreateVertexBuffer();
	CreateIndexBuffer();
	//CreateConstantBuffer();

	CreateFence();

	OnSize(hWnd, hInstance);

#ifdef _DEBUG
	__int64 B;
	QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&B));
	std::cout << "OnCreate : " << (B - A) * SecondsPerCount << " sec" << std::endl << std::endl;
#endif
}
void DX::OnSize(HWND hWnd, HINSTANCE hInstance)
{
#ifdef _DEBUG
	__int64 A;
	QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&A));
#endif

	Super::OnSize(hWnd, hInstance);

	WaitForFence();

	const auto CommandList = GraphicsCommandLists.back();

	VERIFY_SUCCEEDED(CommandList->Reset(CommandAllocator.Get(), nullptr));
	{		
		ResizeSwapChain();
		ResizeDepthStencil();
	}
	VERIFY_SUCCEEDED(CommandList->Close());

	ExecuteCommandList();
	
	WaitForFence();

#ifdef _DEBUG
	__int64 B;
	QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&B));
	std::cout << "OnSize : " << (B - A) * SecondsPerCount << " sec" << std::endl << std::endl;
#endif

	CreateViewport();
}
void DX::OnTimer(HWND hWnd, HINSTANCE hInstance)
{
	Super::OnTimer(hWnd, hInstance);
}
void DX::OnPaint(HWND hWnd, HINSTANCE hInstance)
{
	Super::OnPaint(hWnd, hInstance);

	Draw();
}
void DX::OnDestroy(HWND hWnd, HINSTANCE hInstance)
{
	Super::OnDestroy(hWnd, hInstance);

	WaitForFence();
}

void DX::CreateDevice(HWND hWnd, const DXGI_FORMAT ColorFormat)
{
	using namespace Microsoft::WRL;

#ifdef _DEBUG
	ComPtr<ID3D12Debug> Debug;
	VERIFY_SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(Debug.GetAddressOf())));
	Debug->EnableDebugLayer();
#endif

	ComPtr<IDXGIFactory4> Factory;
	VERIFY_SUCCEEDED(CreateDXGIFactory1(IID_PPV_ARGS(Factory.GetAddressOf())));

#ifdef _DEBUG
	EnumAdapter(Factory.Get());
#endif

#pragma region SelectAdapter
	ComPtr<IDXGIAdapter> Adapter;
	auto GetLastIndexOfHardwareAdapter = [&]() {
		UINT Index = UINT_MAX;
		for (UINT i = 0; DXGI_ERROR_NOT_FOUND != Factory->EnumAdapters(i, Adapter.ReleaseAndGetAddressOf()); ++i) {
			DXGI_ADAPTER_DESC AdapterDesc;
			VERIFY_SUCCEEDED(Adapter->GetDesc(&AdapterDesc));
			if (AdapterDesc.DedicatedVideoMemory) {
				Index = i;
			}
		}
		assert(UINT_MAX != Index);
		return Index;
	};
	Factory->EnumAdapters(GetLastIndexOfHardwareAdapter(), Adapter.ReleaseAndGetAddressOf());
#ifdef _DEBUG
	DXGI_ADAPTER_DESC AdapterDesc;
	VERIFY_SUCCEEDED(Adapter->GetDesc(&AdapterDesc));
	//std::cout << Red; std::wcout << "\t" << AdapterDesc.Description; std::cout << White << " is selected" << std::endl;
#endif
#pragma endregion

	if (FAILED(D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(Device.GetAddressOf())))) {
	//if (FAILED(D3D12CreateDevice(Adapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(Device.GetAddressOf())))) {
#ifdef _DEBUG
		std::cout << "\t" << Red << "Cannot create device, trying to create WarpDevice ..." << White << std::endl;
#endif
		VERIFY_SUCCEEDED(Factory->EnumWarpAdapter(IID_PPV_ARGS(Adapter.GetAddressOf())));
		VERIFY_SUCCEEDED(D3D12CreateDevice(Adapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(Device.GetAddressOf())));
	}

#ifdef _DEBUG
	CheckFeature();
#endif

#ifdef _DEBUG
	std::cout << "CreateDevice" << COUT_OK << std::endl << std::endl;
#endif
}
void DX::EnumAdapter(IDXGIFactory4* Factory)
{
	using namespace Microsoft::WRL;
	
	ComPtr<IDXGIAdapter> Adapter;
	for (UINT i = 0; DXGI_ERROR_NOT_FOUND != Factory->EnumAdapters(i, Adapter.ReleaseAndGetAddressOf()); ++i) {
		DXGI_ADAPTER_DESC AdapterDesc;
		VERIFY_SUCCEEDED(Adapter->GetDesc(&AdapterDesc));
#ifdef _DEBUG
		std::wcout << "\t" << AdapterDesc.Description << std::endl;
		std::cout << "\t" << "\t" << "DedicatedVideoMemory = " << AdapterDesc.DedicatedVideoMemory << std::endl;
#endif

		EnumOutput(Adapter.Get());
	}
}
void DX::EnumOutput(IDXGIAdapter* Adapter)
{
	using namespace Microsoft::WRL;

	ComPtr<IDXGIOutput> Output;
	for (UINT i = 0; DXGI_ERROR_NOT_FOUND != Adapter->EnumOutputs(i, Output.ReleaseAndGetAddressOf()); ++i) {
		DXGI_OUTPUT_DESC OutputDesc;
		VERIFY_SUCCEEDED(Output->GetDesc(&OutputDesc));
#ifdef _DEBUG
		const auto Width = OutputDesc.DesktopCoordinates.right - OutputDesc.DesktopCoordinates.left;
		const auto Height = OutputDesc.DesktopCoordinates.bottom - OutputDesc.DesktopCoordinates.top;
		std::wcout << "\t" << "\t" << "\t" << OutputDesc.DeviceName << " = " << Width << " x " << Height << std::endl;
#endif

		GetDisplayModeList(Output.Get(), DXGI_FORMAT_R8G8B8A8_UNORM);
	}
}
void DX::GetDisplayModeList(IDXGIOutput* Output, const DXGI_FORMAT Format)
{
	UINT NumModes;
	VERIFY_SUCCEEDED(Output->GetDisplayModeList(Format, 0, &NumModes, nullptr));
	if (NumModes) {
		std::vector<DXGI_MODE_DESC> ModeDescs(NumModes);
		VERIFY_SUCCEEDED(Output->GetDisplayModeList(Format, 0, &NumModes, ModeDescs.data()));
#ifdef _DEBUG
		for (const auto& i : ModeDescs) {
			std::wcout << "\t" << "\t" << "\t" << "\t" << i.Width << " x " << i.Height << " @ " << i.RefreshRate.Numerator / i.RefreshRate.Denominator << std::endl;
		}
#endif
	}
}
void DX::CheckFeature()
{
	const std::vector<D3D_FEATURE_LEVEL> FeatureLevels = {
		D3D_FEATURE_LEVEL_12_0,
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_0,
		D3D_FEATURE_LEVEL_9_3,
	};
	D3D12_FEATURE_DATA_FEATURE_LEVELS DataFeatureLevels = {
		static_cast<UINT>(FeatureLevels.size()), FeatureLevels.data()
	};
	VERIFY_SUCCEEDED(Device->CheckFeatureSupport(D3D12_FEATURE_FEATURE_LEVELS, reinterpret_cast<void*>(&DataFeatureLevels), sizeof(DataFeatureLevels)));
#ifdef _DEBUG
	std::cout << "\t" << "\t" << "MaxSupportedFeatureLevel = ";
	switch (DataFeatureLevels.MaxSupportedFeatureLevel) {
	default: std::cout << std::hex << DataFeatureLevels.MaxSupportedFeatureLevel << std::dec << std::endl; break;
	case D3D_FEATURE_LEVEL_12_0: std::cout << "D3D_FEATURE_LEVEL_12_0" << std::endl; break;
	case D3D_FEATURE_LEVEL_11_0: std::cout << "D3D_FEATURE_LEVEL_11_0" << std::endl; break;
	case D3D_FEATURE_LEVEL_10_0: std::cout << "D3D_FEATURE_LEVEL_10_0" << std::endl; break;
	case D3D_FEATURE_LEVEL_9_3: std::cout << "D3D_FEATURE_LEVEL_9_3" << std::endl; break;
	}
#endif

	D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS DataMultiSampleQaualityLevels = {
		DXGI_FORMAT_R8G8B8A8_UNORM,
		D3D12_MAX_MULTISAMPLE_SAMPLE_COUNT,
		D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE
	};
	VERIFY_SUCCEEDED(Device->CheckFeatureSupport(D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS, reinterpret_cast<void*>(&DataMultiSampleQaualityLevels), sizeof(DataMultiSampleQaualityLevels)));
#ifdef _DEBUG
	std::cout << "\t" << "\t" << "MultiSample" << std::endl;
	std::cout << "\t" << "\t" << "\t" << "Count = " << DataMultiSampleQaualityLevels.SampleCount << std::endl;
	std::cout << "\t" << "\t" << "\t" << "QualityLevels = " << DataMultiSampleQaualityLevels.NumQualityLevels << std::endl;
#endif
}

void DX::CreateCommandQueue()
{
	const D3D12_COMMAND_QUEUE_DESC CommandQueueDesc = {
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		0,
		D3D12_COMMAND_QUEUE_FLAG_NONE,
		0
	};
	VERIFY_SUCCEEDED(Device->CreateCommandQueue(&CommandQueueDesc, IID_PPV_ARGS(CommandQueue.GetAddressOf())));

#ifdef _DEBUG
	std::cout << "CreateCommandQueue" << COUT_OK << std::endl << std::endl;
#endif
}

/**
@brief ID3D12CommandAllocator と ID3D12GraphicsCommandList を作成
描画コマンドを発行する場合は PipelineState の指定が必要
TODO CommandList をいくつ作ってどう運用するか？
*/
void DX::CreateCommandList(ID3D12PipelineState* PipelineState)
{
	VERIFY_SUCCEEDED(Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(CommandAllocator.GetAddressOf())));
#ifdef _DEBUG
	std::cout << "\t" << "CommandAllocator" << std::endl;
#endif

	GraphicsCommandLists.resize(GraphicsCommandLists.size() + 1);
	//!< 描画コマンドを発行する CommandList の場合は PipelineState の指定が必要
	VERIFY_SUCCEEDED(Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, CommandAllocator.Get(), PipelineState, IID_PPV_ARGS(GraphicsCommandLists.back().GetAddressOf())));
	VERIFY_SUCCEEDED(GraphicsCommandLists.back()->Close());
#ifdef _DEBUG
	std::cout << "CreateCommandList" << COUT_OK << std::endl << std::endl;
#endif
}

/**
@brief DXGISwapChain と ID3D12DescriptorHeap の作成
*/
void DX::CreateSwapChain(HWND hWnd, const DXGI_FORMAT ColorFormat, const UINT BufferCount)
{
	using namespace Microsoft::WRL;

	ComPtr<IDXGIFactory4> Factory;
	VERIFY_SUCCEEDED(CreateDXGIFactory1(IID_PPV_ARGS(Factory.GetAddressOf())));

	//!< TODO ModeDesc を DisplayMode の列挙から拾ってくるようにする？
	const DXGI_RATIONAL Rational = { 60, 1 };
	const DXGI_MODE_DESC ModeDesc = {
		static_cast<UINT>(GetClientRectWidth()), static_cast<UINT>(GetClientRectHeight()),
		Rational,
		ColorFormat,
		DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED,
		DXGI_MODE_SCALING_UNSPECIFIED
	}; 
	const DXGI_SAMPLE_DESC SampleDesc = { 1/*4*/, 0 };
	DXGI_SWAP_CHAIN_DESC SwapChainDesc = {
		ModeDesc,
		SampleDesc,
		DXGI_USAGE_RENDER_TARGET_OUTPUT,
		BufferCount,
		hWnd,
		TRUE,
		DXGI_SWAP_EFFECT_FLIP_DISCARD,
		DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH
	};
	Microsoft::WRL::ComPtr<IDXGISwapChain> SC;
	VERIFY_SUCCEEDED(Factory->CreateSwapChain(CommandQueue.Get(), &SwapChainDesc, SC.GetAddressOf()));
	SwapChain.Reset();
	VERIFY_SUCCEEDED(SC.As(&SwapChain));
#ifdef _DEBUG
	std::cout << "\t" << "SwapChain" << std::endl;
#endif

	const D3D12_DESCRIPTOR_HEAP_DESC DescripterHeapDesc = {
		D3D12_DESCRIPTOR_HEAP_TYPE_RTV,
		BufferCount,
		D3D12_DESCRIPTOR_HEAP_FLAG_NONE,
		0
	};
	VERIFY_SUCCEEDED(Device->CreateDescriptorHeap(&DescripterHeapDesc, IID_PPV_ARGS(SwapChainDescriptorHeap.GetAddressOf())));
#ifdef _DEBUG
	std::cout << "\t" << "SwapChainDescriptorHeap" << std::endl;
#endif

#ifdef _DEBUG
	std::cout << "CreateSwapChain" << COUT_OK << std::endl << std::endl;
#endif
}
/**
@brief DXGISwapChain のリサイズ、SwapChainResource を再取得して、
*/
void DX::ResizeSwapChain()
{
	DXGI_SWAP_CHAIN_DESC1 SwapChainDesc;
	SwapChain->GetDesc1(&SwapChainDesc);

	for (auto& i : SwapChainResources) { 
		i.Reset();
	}

	VERIFY_SUCCEEDED(SwapChain->ResizeBuffers(SwapChainDesc.BufferCount,
		static_cast<UINT>(GetClientRectWidth()), static_cast<UINT>(GetClientRectHeight()),
		SwapChainDesc.Format/*DXGI_FORMAT_R8G8B8A8_UNORM*/,
		SwapChainDesc.Flags/*DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH*/));
#ifdef _DEBUG
	std::cout << "\t" << "ResizeBuffers" << std::endl;
#endif

	CurrentBackBufferIndex = SwapChain->GetCurrentBackBufferIndex();
#ifdef _DEBUG
	std::cout << "\t" << "CurrentBackBufferIndex = " << CurrentBackBufferIndex << std::endl;
#endif

	auto CpuDescriptorHandle(SwapChainDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
	const auto IncrementSize = Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	SwapChainResources.resize(SwapChainDesc.BufferCount);
	for (UINT i = 0; i < SwapChainResources.size(); ++i) {
		VERIFY_SUCCEEDED(SwapChain->GetBuffer(i, IID_PPV_ARGS(SwapChainResources[i].GetAddressOf())));
		//!< ミップレベル0のすべてのリソースにアクセスする場合は省略(nullptr)可能
		//D3D12_RENDER_TARGET_VIEW_DESC RenderTargetViewDesc = {};
		Device->CreateRenderTargetView(SwapChainResources[i].Get(), nullptr/*&RenderTargetViewDesc*/, CpuDescriptorHandle);
		CpuDescriptorHandle.ptr += IncrementSize;
	}
#ifdef _DEBUG
	std::cout << "\t" << "RenderTargetView" << std::endl;
#endif

#ifdef _DEBUG
	std::cout << "ResizeSwapChain" << COUT_OK << std::endl << std::endl;
#endif
}

void DX::CreateDepthStencil()
{
#if 0
	const D3D12_DESCRIPTOR_HEAP_DESC DescriptorHeapDesc = {
		D3D12_DESCRIPTOR_HEAP_TYPE_DSV,
		1,
		D3D12_DESCRIPTOR_HEAP_FLAG_NONE,
		0
	};
	VERIFY_SUCCEEDED(Device->CreateDescriptorHeap(&DescriptorHeapDesc, IID_PPV_ARGS(DepthStencilDescriptorHeap.GetAddressOf())));
#ifdef _DEBUG
	std::cout << "\t" << "DepthStencilDescriptorHeap" << std::endl;
#endif
#endif

#ifdef _DEBUG
	std::cout << "CreateDepthStencil" << COUT_OK << std::endl << std::endl;
#endif
}
void DX::ResizeDepthStencil(const DXGI_FORMAT DepthFormat)
{
#if 0
	const D3D12_HEAP_PROPERTIES HeapProperties = {
		D3D12_HEAP_TYPE_DEFAULT,
		D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
		D3D12_MEMORY_POOL_UNKNOWN,
		1,
		1
	};
	const DXGI_SAMPLE_DESC SampleDesc = { 1/*4*/, 0 };
	const D3D12_RESOURCE_DESC ResourceDesc = {
		D3D12_RESOURCE_DIMENSION_TEXTURE2D,
		0,
		static_cast<UINT64>(GetClientRectWidth()), static_cast<UINT>(GetClientRectHeight()),
		1,
		1,
		DepthFormat,
		SampleDesc,
		D3D12_TEXTURE_LAYOUT_UNKNOWN,
		D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL
	};
	const D3D12_CLEAR_VALUE ClearValue = {
		DepthFormat,
		{ 1.0f, 0 }
	};
	//!< ClearValue を指定しない場合は nullptr にできる
	VERIFY_SUCCEEDED(Device->CreateCommittedResource(&HeapProperties,
		D3D12_HEAP_FLAG_NONE,
		&ResourceDesc,
		D3D12_RESOURCE_STATE_DEPTH_WRITE,
		nullptr, //&ClearValue, 
		IID_PPV_ARGS(DepthStencilResource.ReleaseAndGetAddressOf())));
#ifdef _DEBUG
	std::cout << "\t" << "CommittedResource" << std::endl;
#endif

	auto CpuDescriptorHandle(DepthStencilDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
	//const auto IncrementSize = Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

	const D3D12_DEPTH_STENCIL_VIEW_DESC DepthStencilViewDesc = {
		DepthFormat,
		D3D12_DSV_DIMENSION_TEXTURE2D,
		D3D12_DSV_FLAG_NONE,
		{ 0 }
	};
	//!< DepthFormat が具体的なフォーマットであれば(TYPELESS でなければ) DepthStencilViewDesc に nullptr を指定できる
	Device->CreateDepthStencilView(DepthStencilResource.Get(), 
		nullptr,//&DepthStencilViewDesc
		CpuDescriptorHandle);
	//CpuDescriptorHandle.ptr += IncrementSize;
#ifdef _DEBUG
	std::cout << "\t" << "DepthStencilView" << std::endl;
#endif

	BarrierDepthWrite();
#endif

#ifdef _DEBUG
	std::cout << "ResizeDepthStencil" << COUT_OK << std::endl << std::endl;
#endif
}

/**
@note VisualStudio に HLSL ファイルを追加すれば、コンパイルされて *.cso ファイルが作成される ( 出力先は x64/Debug/, x64/Release/ など)
*/
void DX::CreateShader()
{
#ifdef _DEBUG
	std::cout << "CreateShader" << COUT_OK << std::endl << std::endl;
#endif
}

void DX::CreateRootSignature()
{
	using namespace Microsoft::WRL;

	const D3D12_ROOT_SIGNATURE_DESC RootSignatureDesc = {
		0, nullptr,
		0, nullptr,
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT
	};

	ComPtr<ID3DBlob> Blob;
	ComPtr<ID3DBlob> ErrorBlob;
	VERIFY_SUCCEEDED(D3D12SerializeRootSignature(&RootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &Blob, &ErrorBlob));
	VERIFY_SUCCEEDED(Device->CreateRootSignature(0, Blob->GetBufferPointer(), Blob->GetBufferSize(), IID_PPV_ARGS(&RootSignature)));

#ifdef _DEBUG
	std::cout << "CreateRootSignature" << COUT_OK << std::endl << std::endl;
#endif
}

void DX::CreateInputLayout()
{
#ifdef _DEBUG
	std::cout << "CreateInputLayout" << COUT_OK << std::endl << std::endl;
#endif
}
void DX::CreateViewport()
{
	const auto Width = GetClientRectWidth();
	const auto Height = GetClientRectHeight();

	Viewports = {
		{ 
			0.0f, 0.0f, 
			static_cast<FLOAT>(Width), static_cast<FLOAT>(Height), 
			0.0f, 1.0f
		}
	};

	ScissorRects = {
		{ 
			0, 0, 
			Width, Height
		}
	};

#ifdef _DEBUG
	std::cout << "CreateViewport" << COUT_OK << std::endl << std::endl;
#endif
}
void DX::CreatePipelineState()
{
	//assert(nullptr != RootSignature);
	//assert(!BlobVSs.empty());
	//const D3D12_SHADER_BYTECODE ShaderBytecodesVS = { BlobVSs[0]->GetBufferPointer(), BlobVSs[0]->GetBufferSize() };
	//assert(!BlobPSs.empty());
	//const D3D12_SHADER_BYTECODE ShaderBytecodesPS = { BlobPSs[0]->GetBufferPointer(), BlobPSs[0]->GetBufferSize() };
	const D3D12_SHADER_BYTECODE DefaultShaderBytecode = { nullptr, 0 };

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

	const D3D12_GRAPHICS_PIPELINE_STATE_DESC GraphicsPipelineStateDesc = {
		nullptr,//RootSignature.Get(),
		//ShaderBytecodesVS, ShaderBytecodesPS, DefaultShaderBytecode, DefaultShaderBytecode, DefaultShaderBytecode,
		DefaultShaderBytecode, DefaultShaderBytecode, DefaultShaderBytecode, DefaultShaderBytecode, DefaultShaderBytecode,
		{
			nullptr, 0,
			nullptr, 0,
			0
		},
		BlendDesc,
		UINT_MAX,
		RasterizerDesc,
		DepthStencilDesc,
		InputLayoutDesc,
		D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED,
		D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE,
		1, { DXGI_FORMAT_R8G8B8A8_UNORM/*, ... x8*/ },
		DXGI_FORMAT_D32_FLOAT_S8X24_UINT,
		{ 1, 0 },
		0,
		{ nullptr, 0 },
		D3D12_PIPELINE_STATE_FLAG_NONE
	};
	VERIFY_SUCCEEDED(Device->CreateGraphicsPipelineState(&GraphicsPipelineStateDesc, IID_PPV_ARGS(&PipelineState)));

#ifdef _DEBUG
	std::cout << "CreateGraphicsPipelineState" << COUT_OK << std::endl << std::endl;
#endif
}

void DX::CreateVertexBuffer()
{
#ifdef _DEBUG
	std::cout << "CreateVertexBuffer" << COUT_OK << std::endl << std::endl;
#endif
}
void DX::CreateIndexBuffer()
{
#ifdef _DEBUG
	std::cout << "CreateIndexBuffer" << COUT_OK << std::endl << std::endl;
#endif
}
void DX::CreateConstantBuffer()
{
	const auto Size = 1024 * 64;

	const D3D12_RESOURCE_DESC ResourceDesc = {
		D3D12_RESOURCE_DIMENSION_BUFFER,
		0,
		Size, 1,
		1,
		1,
		DXGI_FORMAT_UNKNOWN,
		{ 1, 0 },
		D3D12_TEXTURE_LAYOUT_ROW_MAJOR,
		D3D12_RESOURCE_FLAG_NONE
	};
	const D3D12_HEAP_PROPERTIES HeapProperties = {
		D3D12_HEAP_TYPE_UPLOAD,
		D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
		D3D12_MEMORY_POOL_UNKNOWN,
		1,
		1
	};
	VERIFY_SUCCEEDED(Device->CreateCommittedResource(&HeapProperties,
		D3D12_HEAP_FLAG_NONE,
		&ResourceDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&ConstantBufferResource)));

	const D3D12_DESCRIPTOR_HEAP_DESC DescriptorHeapDesc = {
		D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
		1,
		D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE,
		0
	};
	VERIFY_SUCCEEDED(Device->CreateDescriptorHeap(&DescriptorHeapDesc, IID_PPV_ARGS(&ConstantBufferDescriptorHeap)));
	const D3D12_CONSTANT_BUFFER_VIEW_DESC ConstantBufferViewDesc = {
		ConstantBufferResource->GetGPUVirtualAddress(),
		(sizeof(ConstantBufferResource) + 255) & ~255 //!< コンスタントバッファは 256 byte アラインでないとならない
	};
	Device->CreateConstantBufferView(&ConstantBufferViewDesc, ConstantBufferDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

#ifdef _DEBUG
	std::cout << "CreateConstantBuffer" << COUT_OK << std::endl << std::endl;
#endif
}

void DX::CreateFence()
{
	VERIFY_SUCCEEDED(Device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(Fence.GetAddressOf())));
#ifdef _DEBUG
	std::cout << "CreateFence" << COUT_OK << std::endl << std::endl;
#endif

	//WaitForFence();
}

void DX::Clear()
{
	const auto CommandList = GraphicsCommandLists.back();

	auto CpuDescritptorHandle(SwapChainDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
	CpuDescritptorHandle.ptr += CurrentBackBufferIndex * Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	CommandList->ClearRenderTargetView(CpuDescritptorHandle, DirectX::Colors::SkyBlue, 0, nullptr);
	
	//auto DepthStencilViewHandle(DepthStencilDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
	//DepthStencilViewHandle.ptr += 0 * Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV); 
	//CommandList->ClearDepthStencilView(DepthStencilViewHandle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);
}

void DX::BarrierDepthWrite()
{
	const auto CommandList = GraphicsCommandLists.back();

	std::vector<D3D12_RESOURCE_BARRIER> ResourceBarrier = {
		{
			D3D12_RESOURCE_BARRIER_TYPE_TRANSITION,
			D3D12_RESOURCE_BARRIER_FLAG_NONE,
			{
				DepthStencilResource.Get(),
				D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES,
				D3D12_RESOURCE_STATE_COMMON,
				D3D12_RESOURCE_STATE_DEPTH_WRITE
			}
		}
	};
	CommandList->ResourceBarrier(static_cast<UINT>(ResourceBarrier.size()), ResourceBarrier.data());

#ifdef _DEBUG
	//std::cout << "\t" << "ResourceBarrier" << " : " << "To DepthWrite" << std::endl;
#endif
}
void DX::BarrierRenderTarget()
{
	const auto CommandList = GraphicsCommandLists.back();

	std::vector<D3D12_RESOURCE_BARRIER> ResourceBarrier = {
		{
			D3D12_RESOURCE_BARRIER_TYPE_TRANSITION,
			D3D12_RESOURCE_BARRIER_FLAG_NONE,
			{
				SwapChainResources[CurrentBackBufferIndex].Get(),
				D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES,
				D3D12_RESOURCE_STATE_PRESENT,
				D3D12_RESOURCE_STATE_RENDER_TARGET
			}
		}
	};
	CommandList->ResourceBarrier(static_cast<UINT>(ResourceBarrier.size()), ResourceBarrier.data());

#ifdef _DEBUG
	//std::cout << "\t" << "ResourceBarrier" << " : " << "To RenderTarget" << std::endl;
#endif
}
void DX::BarrierPresent()
{
	const auto CommandList = GraphicsCommandLists.back();

	std::vector<D3D12_RESOURCE_BARRIER> ResourceBarrier = {
		{
			D3D12_RESOURCE_BARRIER_TYPE_TRANSITION,
			D3D12_RESOURCE_BARRIER_FLAG_NONE,
			{
				SwapChainResources[CurrentBackBufferIndex].Get(),
				D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES,
				D3D12_RESOURCE_STATE_RENDER_TARGET,
				D3D12_RESOURCE_STATE_PRESENT
			}
		}
	};
	CommandList->ResourceBarrier(static_cast<UINT>(ResourceBarrier.size()), ResourceBarrier.data());

#ifdef _DEBUG
	//std::cout << "\t" << "ResourceBarrier" << " :  " << "To Present" << std::endl;
#endif
}
void DX::PopulateCommandList()
{
	Clear();
	
	//!< レンダーターゲット(フレームバッファ)
	//auto RenderTargetViewHandle(SwapChainDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
	//RenderTargetViewHandle.ptr += CurrentBackBufferIndex * Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	////auto DepthStencilViewHandle(DepthStencilDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
	////DepthStencilViewHandle.ptr += 0 * Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
	//CommandList->OMSetRenderTargets(1, &RenderTargetViewHandle, FALSE, nullptr/*&DepthStencilViewHandle*/);

//	//{
//	//	using namespace DirectX;
//	//	const std::vector<XMMATRIX> WVP = { XMMatrixIdentity(), XMMatrixIdentity(), XMMatrixIdentity() };

//	//	UINT8* Data;
//	//	D3D12_RANGE Range = { 0, 0 };
//	//	VERIFY_SUCCEEDED(ConstantBuffer->Map(0, &Range, reinterpret_cast<void**>(&Data))); {
//	//		memcpy(Data, &WVP, sizeof(WVP));
//	//	} ConstantBuffer->Unmap(0, nullptr); //!< サンプルには アプリが終了するまで Unmap しない、リソースはマップされたままでOKと書いてあるが...よく分からない
//	//}

//	//CommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//CommandList->RSSetViewports(static_cast<UINT>(Viewports.size()), Viewports.data());
	//CommandList->RSSetScissorRects(static_cast<UINT>(ScissorRects.size()), ScissorRects.data());

//	//CommandList->IASetVertexBuffers(0, 1, &VertexBufferView);
//	//CommandList->IASetIndexBuffer(&IndexBufferView);

//	//CommandList->DrawIndexedInstanced(IndexCount, 1, 0, 0, 0);
}

void DX::Draw()
{
	const auto CommandList = GraphicsCommandLists.back();

	//!< GPU が参照している間は CommandAllocator->Reset() できない
	VERIFY_SUCCEEDED(CommandAllocator->Reset());
	
	//!< CommandQueue->ExecuteCommandLists() 後に CommandList->Reset() でリセットして再利用が可能
	//!< コマンドキューはコマンドリストではなく、コマンドアロケータを参照している
	VERIFY_SUCCEEDED(CommandList->Reset(CommandAllocator.Get(), PipelineState.Get()));
	{
		BarrierRenderTarget();
		{
			PopulateCommandList();
		}
		BarrierPresent();
	}
	VERIFY_SUCCEEDED(CommandList->Close());

	ExecuteCommandList();

	Present();

	WaitForFence();
}
void DX::ExecuteCommandList()
{
	auto CommandList = GraphicsCommandLists.back();

	std::vector<ID3D12CommandList*> CommandLists = { CommandList.Get() };
	CommandQueue->ExecuteCommandLists(static_cast<UINT>(CommandLists.size()), CommandLists.data());
}
void DX::Present()
{
	VERIFY_SUCCEEDED(SwapChain->Present(1, 0));

#ifdef _DEBUG
	//std::cout << CurrentBackBufferIndex;
#endif
	CurrentBackBufferIndex = ++CurrentBackBufferIndex % static_cast<UINT>(SwapChainResources.size());
}
void DX::WaitForFence()
{
	//!< CPU 側のフェンス値をインクリメント
	++FenceValue;

	//!< GPU 側からフェンス値を設定 (GPU 側でコマンドがここまで到達すればフェンス値が CPU に追いつく事になる)
	VERIFY_SUCCEEDED(CommandQueue->Signal(Fence.Get(), FenceValue));
	if (Fence->GetCompletedValue() < FenceValue) {
		auto hEvent = CreateEventEx(nullptr, nullptr, 0, EVENT_ALL_ACCESS);
		//!< GPU 側のフェンス値が追いついた時にイベントが発行される
		VERIFY_SUCCEEDED(Fence->SetEventOnCompletion(FenceValue, hEvent));

		//!< イベント発行まで待つ
		WaitForSingleObject(hEvent, INFINITE);
		CloseHandle(hEvent);
	}

#ifdef _DEBUG
	//std::cout << "Fence = " << FenceValue << std::endl;
#endif
}

