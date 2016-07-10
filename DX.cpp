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

	CreateCommandAllocator();
	CreateCommandList(CommandAllocators[0].Get());

	CreateFence();

	CreateSwapChain(hWnd, ColorFormat);

	CreateDepthStencil();

	CreateShader();

	CreateRootSignature();

	CreateInputLayout();

	CreateViewport();
	CreatePipelineState();

	CreateVertexBuffer(CommandAllocators[0].Get(), GraphicsCommandLists[0].Get());
	CreateIndexBuffer(CommandAllocators[0].Get(), GraphicsCommandLists[0].Get());
	//CreateConstantBuffer();
	//CreateUnorderedAccessTexture();

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

	const auto CommandList = GraphicsCommandLists[0].Get();

	VERIFY_SUCCEEDED(CommandList->Reset(CommandAllocators[0].Get(), nullptr));
	{		
		ResizeSwapChain();
		ResizeDepthStencil();
	}
	VERIFY_SUCCEEDED(CommandList->Close());

	ExecuteCommandList(CommandList);
	
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
std::string DX::GetHRESULTString(const HRESULT Result)
{
	const auto WResultString = GetHRESULTStringW(Result);
#if 1
	//!< ���{��Ή�
	char Temporary[256];
	size_t NumOfCharConverted;
	wcstombs_s(&NumOfCharConverted, Temporary, WResultString.c_str(), _countof(Temporary));
	return std::string(Temporary);
#else
	return std::string(WResultString.begin(), WResultString.end());
#endif
}
std::wstring DX::GetHRESULTStringW(const HRESULT Result)
{
	//!< 16�i�̕�����\�L
	//std::stringstream ss;
	//ss << "0x" << std::hex << Result << std::dec;
	//ss.str();

	return std::wstring(_com_error(Result).ErrorMessage());
}

std::string DX::GetFormatString(const DXGI_FORMAT Format)
{
#define DXGI_FORMAT_ENTRY(df) case DXGI_FORMAT_##df: return #df;
	switch (Format)
	{
	default: assert(0 && "Unknown DXGI_FORMAT"); return "";
#include "DXFormat.h"
	}
#undef DXGI_FORMAT_CASE
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

	//!< DedicatedVideoMemory �̂���Ō�̃A�_�v�^(GPU)�C���f�b�N�X��Ԃ�
	auto GetLastIndexOfHardwareAdapter = [&]() {
		UINT Index = UINT_MAX;
		ComPtr<IDXGIAdapter> Adapter;
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
	ComPtr<IDXGIAdapter> Adapter;
	Factory->EnumAdapters(GetLastIndexOfHardwareAdapter(), Adapter.ReleaseAndGetAddressOf());
#ifdef _DEBUG
	DXGI_ADAPTER_DESC AdapterDesc;
	VERIFY_SUCCEEDED(Adapter->GetDesc(&AdapterDesc));
	std::wcout << "\t[ " << Yellow << AdapterDesc.Description << White << " ]" << std::endl;
#endif

	if (FAILED(D3D12CreateDevice(Adapter.Get(), D3D_FEATURE_LEVEL_12_1, IID_PPV_ARGS(Device.GetAddressOf())))) {
#ifdef _DEBUG
		std::cout << "\t" << Red << "Cannot create device, trying to create WarpDevice ..." << White << std::endl;
#endif
		VERIFY_SUCCEEDED(Factory->EnumWarpAdapter(IID_PPV_ARGS(Adapter.GetAddressOf())));
		VERIFY_SUCCEEDED(D3D12CreateDevice(Adapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(Device.GetAddressOf())));
	}

#ifdef _DEBUG
	CheckFeatureLevel();
	CheckMultiSample(DXGI_FORMAT_R8G8B8A8_UNORM);
#endif

#ifdef _DEBUG
	std::cout << "CreateDevice" << COUT_OK << std::endl << std::endl;
#endif
}

//!< �A�_�v�^(GPU)�̗�
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

//!< �A�E�g�v�b�g(�f�B�X�v���C)�̗�
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

//!< �`�惂�[�h�̗�
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
void DX::CheckFeatureLevel()
{
	const std::vector<D3D_FEATURE_LEVEL> FeatureLevels = {
		D3D_FEATURE_LEVEL_12_1,
		D3D_FEATURE_LEVEL_12_0,
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
		D3D_FEATURE_LEVEL_9_3,
		D3D_FEATURE_LEVEL_9_2,
		D3D_FEATURE_LEVEL_9_1,
	};
	D3D12_FEATURE_DATA_FEATURE_LEVELS DataFeatureLevels = {
		static_cast<UINT>(FeatureLevels.size()), FeatureLevels.data()
	};
	VERIFY_SUCCEEDED(Device->CheckFeatureSupport(D3D12_FEATURE_FEATURE_LEVELS, reinterpret_cast<void*>(&DataFeatureLevels), sizeof(DataFeatureLevels)));
#ifdef _DEBUG
	std::cout << "\t" << "\t" << "MaxSupportedFeatureLevel = ";
#define D3D_FEATURE_LEVEL_ENTRY(fl) case D3D_FEATURE_LEVEL_##fl: std::cout << "D3D_FEATURE_LEVEL_" #fl << std::endl; break;
	switch (DataFeatureLevels.MaxSupportedFeatureLevel) {
	default: assert(0 && "Unknown FeatureLevel"); break;
	D3D_FEATURE_LEVEL_ENTRY(12_1)
	D3D_FEATURE_LEVEL_ENTRY(12_0)
	D3D_FEATURE_LEVEL_ENTRY(11_1)
	D3D_FEATURE_LEVEL_ENTRY(11_0)
	D3D_FEATURE_LEVEL_ENTRY(10_1)
	D3D_FEATURE_LEVEL_ENTRY(10_0)
	D3D_FEATURE_LEVEL_ENTRY(9_3)
	D3D_FEATURE_LEVEL_ENTRY(9_2)
	D3D_FEATURE_LEVEL_ENTRY(9_1)
	}
#undef D3D_FEATURE_LEVEL_ENTRY
#endif
}

void DX::CheckMultiSample(const DXGI_FORMAT Format)
{
#ifdef _DEBUG
	std::cout << "\t" << "\t" << "MultiSample" << std::endl;
	std::cout << "\t" << "\t" << "\t" << "Format = " << GetFormatString(Format) << std::endl;
#endif
	for (UINT i = 1; i < D3D12_MAX_MULTISAMPLE_SAMPLE_COUNT; ++i) {
		D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS DataMultiSampleQaualityLevels = {
			Format,
			i,
			D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE,
		};
		VERIFY_SUCCEEDED(Device->CheckFeatureSupport(D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS, reinterpret_cast<void*>(&DataMultiSampleQaualityLevels), sizeof(DataMultiSampleQaualityLevels)));
#ifdef _DEBUG
		std::cout << "\t" << "\t" << "\t" << "Count, QualityLevels = " << DataMultiSampleQaualityLevels.SampleCount << ", " << DataMultiSampleQaualityLevels.NumQualityLevels << std::endl;
#endif
	}
}

void DX::CreateCommandQueue()
{
	const D3D12_COMMAND_QUEUE_DESC CommandQueueDesc = {
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		0,
		D3D12_COMMAND_QUEUE_FLAG_NONE,
		0 // NodeMask ... �}���`GPU�̏ꍇ
	};
	VERIFY_SUCCEEDED(Device->CreateCommandQueue(&CommandQueueDesc, IID_PPV_ARGS(CommandQueue.GetAddressOf())));

#ifdef _DEBUG
	std::cout << "CreateCommandQueue" << COUT_OK << std::endl << std::endl;
#endif
}

/**
@note �R�}���h�A���P�[�^�ɕ����̃R�}���h���X�g���쐬�ł��邪�A�R�}���h���X�g�͓����ɂ͋L�^�ł��Ȃ� (Close()���Ȃ��ƃ_��)
@note CommandList->ExecuteCommandList() �� GPU �� CommandAllocator �̎Q�Ƃ��I����܂ŁACommandAllocator->Reset() ���Ă͂����Ȃ�
*/
void DX::CreateCommandAllocator(const D3D12_COMMAND_LIST_TYPE CommandListType)
{
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> CommandAllocator;
	VERIFY_SUCCEEDED(Device->CreateCommandAllocator(CommandListType, IID_PPV_ARGS(CommandAllocator.GetAddressOf())));

	CommandAllocators.push_back(CommandAllocator);

#ifdef _DEBUG
	std::cout << "\t" << "CommandAllocator" << std::endl;
#endif
}

/**
@note CommandList->ExecuteCommandList() ��� CommandList->Reset() �����Ă��ǂ��B(CommandAllocator ���o���Ă���̂ŁACommandQueue �ɂ͉e�����Ȃ�)
�`��R�}���h�𔭍s����R�}���h���X�g�� PipelineState �̎w�肪�K�v
�ォ��CommandList->Reset(Allocator, PipelineState) �̈����ł��w��ł���
�`��R�}���h�𔭍s���Ȃ��R�}���h���X�g(�������p�r��)��A�o���h���� nullptr �w��ŗǂ�
�����ł� PipelineState == nullptr �ō쐬���Ă��܂��Ă���
*/
void DX::CreateCommandList(ID3D12CommandAllocator* CommandAllocator, const D3D12_COMMAND_LIST_TYPE CommandListType)
{
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> GraphicsCommandList;
	VERIFY_SUCCEEDED(Device->CreateCommandList(0, CommandListType, CommandAllocator, nullptr, IID_PPV_ARGS(GraphicsCommandList.GetAddressOf())));

	GraphicsCommandLists.push_back(GraphicsCommandList);

	//!< Close() ���Ă���
	VERIFY_SUCCEEDED(GraphicsCommandLists.back()->Close());

#ifdef _DEBUG
	std::cout << "CreateCommandList" << COUT_OK << std::endl << std::endl;
#endif
}

/**
@brief CPU �� GPU �̓���
*/
void DX::CreateFence()
{
	VERIFY_SUCCEEDED(Device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(Fence.GetAddressOf())));

#ifdef _DEBUG
	std::cout << "CreateFence" << COUT_OK << std::endl << std::endl;
#endif
	//WaitForFence();
}

/**
@brief DXGISwapChain �� ID3D12DescriptorHeap �̍쐬
*/
void DX::CreateSwapChain(HWND hWnd, const DXGI_FORMAT ColorFormat, const UINT BufferCount)
{
	using namespace Microsoft::WRL;

	ComPtr<IDXGIFactory4> Factory;
	VERIFY_SUCCEEDED(CreateDXGIFactory1(IID_PPV_ARGS(Factory.GetAddressOf())));

	//!< TODO ModeDesc �� DisplayMode �̗񋓂���E���Ă���悤�ɂ���H
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
		0 // NodeMask ... �}���`GPU�̏ꍇ
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
@brief DXGISwapChain �̃��T�C�Y�ASwapChainResource ���Ď擾���āA
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
		//!< �~�b�v���x��0�̂��ׂẴ��\�[�X�ɃA�N�Z�X����ꍇ�͏ȗ�(nullptr)�\
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
	//!< ClearValue ���w�肵�Ȃ��ꍇ�� nullptr �ɂł���
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
	//!< DepthFormat ����̓I�ȃt�H�[�}�b�g�ł����(TYPELESS �łȂ����) DepthStencilViewDesc �� nullptr ���w��ł���
	Device->CreateDepthStencilView(DepthStencilResource.Get(), 
		nullptr,//&DepthStencilViewDesc
		CpuDescriptorHandle);
	//CpuDescriptorHandle.ptr += IncrementSize;
#ifdef _DEBUG
	std::cout << "\t" << "DepthStencilView" << std::endl;
#endif

	auto CommandList = GraphicsCommandLists[0];

	BarrierTransition(CommandList.Get(), DepthStencilResource.Get(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_DEPTH_WRITE);
#endif

#ifdef _DEBUG
	std::cout << "ResizeDepthStencil" << COUT_OK << std::endl << std::endl;
#endif
}

/**
@note VisualStudio �� HLSL �t�@�C����ǉ�����΁A�R���p�C������� *.cso �t�@�C�����쐬����� ( �o�͐�� x64/Debug/, x64/Release/ �Ȃ�)
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
	VERIFY_SUCCEEDED(D3D12SerializeRootSignature(&RootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, Blob.GetAddressOf(), ErrorBlob.GetAddressOf()));
	VERIFY_SUCCEEDED(Device->CreateRootSignature(0, Blob->GetBufferPointer(), Blob->GetBufferSize(), IID_PPV_ARGS(RootSignature.GetAddressOf())));

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
void DX::CreateGraphicsPipelineState()
{
#ifdef _DEBUG
	std::cout << "CreateGraphicsPipelineState" << COUT_OK << std::endl << std::endl;
#endif
}

void DX::CreateComputePipelineState()
{
	assert(nullptr != RootSignature);
	assert(!ShaderBlobs.empty());

	const D3D12_SHADER_BYTECODE ShaderBytecodesCS = { ShaderBlobs[0]->GetBufferPointer(), ShaderBlobs[0]->GetBufferSize() };
	const D3D12_CACHED_PIPELINE_STATE CachedPipelineState = { nullptr, 0 };
	const D3D12_COMPUTE_PIPELINE_STATE_DESC ComputePipelineStateDesc = {
		RootSignature.Get(),
		ShaderBytecodesCS,
		0, // NodeMask ... �}���`GPU�̏ꍇ
		CachedPipelineState,
		D3D12_PIPELINE_STATE_FLAG_NONE
	};
	VERIFY_SUCCEEDED(Device->CreateComputePipelineState(&ComputePipelineStateDesc, IID_PPV_ARGS(PipelineState.GetAddressOf())));

#ifdef _DEBUG
	std::cout << "CreateComputePipelineState" << COUT_OK << std::endl << std::endl;
#endif
}

void DX::CreateBuffer(ID3D12CommandAllocator* CommandAllocator, ID3D12GraphicsCommandList* CommandList, ID3D12Resource** Resource, const void* Source, const size_t Size)
{
	//!< ���\�[�X�f�X�N���v�^(���p)
	const DXGI_SAMPLE_DESC SampleDesc = { 1, 0 };
	const D3D12_RESOURCE_DESC ResourceDesc = {
		D3D12_RESOURCE_DIMENSION_BUFFER,
		0,
		Size, 1,
		1,
		1,
		DXGI_FORMAT_UNKNOWN,
		SampleDesc,
		D3D12_TEXTURE_LAYOUT_ROW_MAJOR,
		D3D12_RESOURCE_FLAG_NONE
	};

	//!< �A�b�v���[�h�p�̃��\�[�X���쐬(Map() ���ăR�s�[)
#pragma region Upload
	Microsoft::WRL::ComPtr<ID3D12Resource> UploadResource;
	const D3D12_HEAP_PROPERTIES HeapProperties_Upload = {
		D3D12_HEAP_TYPE_UPLOAD, //!< UPLOAD �ɂ��邱��
		D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
		D3D12_MEMORY_POOL_UNKNOWN,
		1,
		1
	};
	VERIFY_SUCCEEDED(Device->CreateCommittedResource(&HeapProperties_Upload,
		D3D12_HEAP_FLAG_NONE,
		&ResourceDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ, //!< UPLOAD �ł� GENERIC_READ �ɂ��邱��
		nullptr,
		IID_PPV_ARGS(UploadResource.GetAddressOf())));
	BYTE* Data;
	VERIFY_SUCCEEDED(UploadResource->Map(0, nullptr, reinterpret_cast<void**>(&Data))); {
		memcpy(Data, Source, Size);
	} UploadResource->Unmap(0, nullptr);
#pragma endregion

	//!< ���\�[�X���쐬
	const D3D12_HEAP_PROPERTIES HeapProperties = {
		D3D12_HEAP_TYPE_DEFAULT, //!< DEFAULT �ɂ��邱��
		D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
		D3D12_MEMORY_POOL_UNKNOWN,
		1,
		1
	};
	VERIFY_SUCCEEDED(Device->CreateCommittedResource(&HeapProperties,
		D3D12_HEAP_FLAG_NONE,
		&ResourceDesc,
		D3D12_RESOURCE_STATE_COMMON, //!< COMMON �ɂ��邱��
		nullptr,
		//IID_PPV_ARGS(Resource->GetAddressOf())));
		IID_PPV_ARGS(Resource)));

	//!< �R�s�[����R�}���h���X�g�𔭍s����
	VERIFY_SUCCEEDED(CommandList->Reset(CommandAllocator, nullptr)); {
		BarrierTransition(CommandList, *Resource, D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST); {
			CommandList->CopyBufferRegion(*Resource, 0, UploadResource.Get(), 0, Size);
		} BarrierTransition(CommandList, *Resource, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ);
	} VERIFY_SUCCEEDED(CommandList->Close());

	ExecuteCommandList(CommandList);

	WaitForFence();
}

void DX::CreateVertexBuffer(ID3D12CommandAllocator* CommandAllocator, ID3D12GraphicsCommandList* CommandList)
{
	//!< CPU ���ɂ��R�s�[����������A�����K�v�Ȃ�?
	//Microsoft::WRL::ComPtr<ID3DBlob> VertexBufferBlob;
	//VERIFY_SUCCEEDED(D3DCreateBlob(Size, VertexBufferBlob.GetAddressOf()));
	//CopyMemory(VertexBufferBlob->GetBufferPointer(), Vertices.data(), Size);

#ifdef _DEBUG
	std::cout << "CreateVertexBuffer" << COUT_OK << std::endl << std::endl;
#endif
}
void DX::CreateIndexBuffer(ID3D12CommandAllocator* CommandAllocator, ID3D12GraphicsCommandList* CommandList)
{
	//!< CPU ���ɂ��R�s�[����������A�����K�v�Ȃ�?
	//Microsoft::WRL::ComPtr<ID3DBlob> IndexBufferBlob;
	//VERIFY_SUCCEEDED(D3DCreateBlob(Size, IndexBufferBlob.GetAddressOf()));
	//CopyMemory(IndexBufferBlob->GetBufferPointer(), Indices.data(), Size);

#ifdef _DEBUG
	std::cout << "CreateIndexBuffer" << COUT_OK << std::endl << std::endl;
#endif
}
void DX::CreateConstantBuffer()
{
	const DirectX::XMFLOAT4X4 WVP;
	const auto Size = sizeof(WVP);
	//!< �R���X�^���g�o�b�t�@�T�C�Y�� 256kb �A���C��
	const auto CBSize = (Size + 255) & ~255;

	const DXGI_SAMPLE_DESC SampleDesc = { 1, 0 };
	const D3D12_RESOURCE_DESC ResourceDesc = {
		D3D12_RESOURCE_DIMENSION_BUFFER,
		0,
		CBSize, 1,
		1,
		1,
		DXGI_FORMAT_UNKNOWN,
		SampleDesc,
		D3D12_TEXTURE_LAYOUT_ROW_MAJOR,
		D3D12_RESOURCE_FLAG_NONE
	};
	const D3D12_HEAP_PROPERTIES HeapProperties = {
		D3D12_HEAP_TYPE_UPLOAD, //!< �R���X�^���g�o�b�t�@�� CPU ����X�V����̂� UPLOAD �ɂ���
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
		IID_PPV_ARGS(ConstantBufferResource.GetAddressOf())));

	const D3D12_DESCRIPTOR_HEAP_DESC DescriptorHeapDesc = {
		D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
		1,
		D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE,
		0 // NodeMask ... �}���`GPU�̏ꍇ
	};
	VERIFY_SUCCEEDED(Device->CreateDescriptorHeap(&DescriptorHeapDesc, IID_PPV_ARGS(ConstantBufferDescriptorHeap.GetAddressOf())));

	const D3D12_CONSTANT_BUFFER_VIEW_DESC ConstantBufferViewDesc = {
		ConstantBufferResource->GetGPUVirtualAddress() + 0 * CBSize,
		CBSize
	};
	Device->CreateConstantBufferView(&ConstantBufferViewDesc, ConstantBufferDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

	//!< TODO �R���X�^���g�o�b�t�@�̍X�V
	//BYTE* Data;
	//VERIFY_SUCCEEDED(ConstantBufferResource->Map(0, nullptr, reinterpret_cast<void**>(&Data))); {
	//	memcpy(Data, nullptr/*TODO*/, CBSize);
	//} ConstantBufferResource->Unmap(0, nullptr);

#ifdef _DEBUG
	std::cout << "CreateConstantBuffer" << COUT_OK << std::endl << std::endl;
#endif
}

void DX::CreateUnorderedAccessTexture()
{
	const UINT64 Width = 256;
	const UINT Height = 256;

	const DXGI_SAMPLE_DESC SampleDesc = { 1, 0 };
	const D3D12_RESOURCE_DESC ResourceDesc = {
		D3D12_RESOURCE_DIMENSION_TEXTURE2D,
		0,
		Width, Height, 1, 1,
		DXGI_FORMAT_R8G8B8A8_UNORM,
		SampleDesc,
		D3D12_TEXTURE_LAYOUT_UNKNOWN,
		D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS //!< ALLOW_UNORDERED_ACCESS �ɂ���
	};
	const D3D12_HEAP_PROPERTIES HeapProperties = {
		D3D12_HEAP_TYPE_DEFAULT,
		D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
		D3D12_MEMORY_POOL_UNKNOWN,
		1,
		1
	};
	VERIFY_SUCCEEDED(Device->CreateCommittedResource(&HeapProperties,
		D3D12_HEAP_FLAG_NONE,
		&ResourceDesc,
		D3D12_RESOURCE_STATE_COMMON,
		nullptr,
		IID_PPV_ARGS(UnorderedAccessTextureResource.GetAddressOf())));

	const auto Format = DXGI_FORMAT_R8G8B8A8_UNORM;

	const D3D12_DESCRIPTOR_HEAP_DESC DescritporHeapDesc = {
		D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
		2, //!< SRV, UAV �� 2 ��
		D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE,
		0 // NodeMask ... �}���`GPU�̏ꍇ
	};
	VERIFY_SUCCEEDED(Device->CreateDescriptorHeap(&DescritporHeapDesc, IID_PPV_ARGS(UnorderedAccessTextureDescriptorHeap.GetAddressOf())));

	auto CPUDescriptorHandle(UnorderedAccessTextureDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
	const auto IncrementSize = Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	CPUDescriptorHandle.ptr += 0 * IncrementSize;
	/*const*/D3D12_SHADER_RESOURCE_VIEW_DESC SRVDesc = {
		Format,
		D3D12_SRV_DIMENSION_TEXTURE2D,
		D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING,
	};
	SRVDesc.Texture2D.MostDetailedMip = 0;
	SRVDesc.Texture1D.MipLevels = 1;
	Device->CreateShaderResourceView(UnorderedAccessTextureResource.Get(), &SRVDesc, CPUDescriptorHandle);

	CPUDescriptorHandle.ptr += 1 * IncrementSize;
	/*const*/D3D12_UNORDERED_ACCESS_VIEW_DESC UAVDesc = {
		Format,
		D3D12_UAV_DIMENSION_TEXTURE2D,
	};
	UAVDesc.Texture2D.MipSlice = 0;
	Device->CreateUnorderedAccessView(UnorderedAccessTextureResource.Get(), nullptr, &UAVDesc, CPUDescriptorHandle);

#ifdef _DEBUG
	std::cout << "CreateUnorderedAccessBuffer" << COUT_OK << std::endl << std::endl;
#endif
}

void DX::PopulateCommandList(ID3D12GraphicsCommandList* GraphicsCommandList)
{
	Clear(GraphicsCommandList);
}

void DX::BarrierTransition(ID3D12GraphicsCommandList* CommandList, ID3D12Resource* Resource, const D3D12_RESOURCE_STATES Before, const D3D12_RESOURCE_STATES After)
{
	const std::vector<D3D12_RESOURCE_BARRIER> ResourceBarrier = {
		{
			D3D12_RESOURCE_BARRIER_TYPE_TRANSITION,
			D3D12_RESOURCE_BARRIER_FLAG_NONE,
			{
				Resource,
				D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES,
				Before,
				After
			}
		}
	};
	CommandList->ResourceBarrier(static_cast<UINT>(ResourceBarrier.size()), ResourceBarrier.data());
}

void DX::Draw()
{
	if (CommandAllocators.empty() || GraphicsCommandLists.empty()) { return; }

	const auto CommandAllocator = CommandAllocators[0].Get();
	const auto CommandList = GraphicsCommandLists[0].Get();

	//!< GPU ���Q�Ƃ��Ă���Ԃ� CommandAllocator->Reset() �ł��Ȃ�
	VERIFY_SUCCEEDED(CommandAllocator->Reset());
	
	//!< CommandQueue->ExecuteCommandLists() ��� CommandList->Reset() �Ń��Z�b�g���čė��p���\ (�R�}���h�L���[�̓R�}���h���X�g�ł͂Ȃ��A�R�}���h�A���P�[�^���Q�Ƃ��Ă���)
	//!< CommandList �쐬���� PipelineState ���w�肵�Ă��Ȃ��Ă��A�����Ŏw�肷��� OK
	VERIFY_SUCCEEDED(CommandList->Reset(CommandAllocator, PipelineState.Get()));
	{
		CommandList->RSSetViewports(static_cast<UINT>(Viewports.size()), Viewports.data());
		CommandList->RSSetScissorRects(static_cast<UINT>(ScissorRects.size()), ScissorRects.data());

		BarrierTransition(CommandList, SwapChainResources[CurrentBackBufferIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
		{
			PopulateCommandList(CommandList);
		}
		BarrierTransition(CommandList, SwapChainResources[CurrentBackBufferIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
	}
	VERIFY_SUCCEEDED(CommandList->Close());

	ExecuteCommandList(CommandList);

	Present();

	WaitForFence();
}
void DX::ExecuteCommandList(ID3D12GraphicsCommandList* GraphicsCommandList)
{
	std::vector<ID3D12CommandList*> CommandLists = { GraphicsCommandList };
	CommandQueue->ExecuteCommandLists(static_cast<UINT>(CommandLists.size()), CommandLists.data());
}
void DX::Present()
{
	VERIFY_SUCCEEDED(SwapChain->Present(1, 0));

	CurrentBackBufferIndex = ++CurrentBackBufferIndex % static_cast<UINT>(SwapChainResources.size());
}
void DX::WaitForFence()
{
	//!< CPU ���̃t�F���X�l���C���N�������g
	++FenceValue;

	//!< GPU �R�}���h�� Signal() �܂œ��B����� GetCompletedValue() �� FenceValue �ɂȂ�ACPU�ɒǂ��������ƂɂȂ�
	VERIFY_SUCCEEDED(CommandQueue->Signal(Fence.Get(), FenceValue));
	if (Fence->GetCompletedValue() < FenceValue) {
		auto hEvent = CreateEventEx(nullptr, nullptr, 0, EVENT_ALL_ACCESS);
		//!< GetCompletedValue() �� FenceValue �ɂȂ�����C�x���g�����s�����
		VERIFY_SUCCEEDED(Fence->SetEventOnCompletion(FenceValue, hEvent));

		//!< �C�x���g���s�܂ő҂�
		WaitForSingleObject(hEvent, INFINITE);
		CloseHandle(hEvent);
	}
}

