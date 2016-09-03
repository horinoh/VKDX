#include "stdafx.h"

#include "DX.h"

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "dxgi.lib")

void DX::OnCreate(HWND hWnd, HINSTANCE hInstance)
{
#ifdef _DEBUG
	PerformanceCounter PC("OnCreate : ");
#endif

	Super::OnCreate(hWnd, hInstance);

	//!< デバイス、キュー
	const auto ColorFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
	CreateDevice(hWnd);
	CheckMultiSample(ColorFormat);
	CreateCommandQueue();

	//!< コマンドアロケータ、リスト
	CreateCommandAllocator();
	auto CommandAllocator = CommandAllocators[0].Get();
	CreateCommandList(CommandAllocator);

	CreateFence();

	//!< スワップチェイン
	CreateSwapChainOfClientRect(hWnd, ColorFormat);
	CreateSwapChainDescriptorHeap();
	//!< ResizeSwapChain() で SwapChainResources が作られる、明示的にしなくても OnSize() からコールされる
	//!< デプスステンシル
	CreateDepthStencilDescriptorHeap();
	//!< ResizeDepthStencil() で DepthStencilResource が作られる、明示的にしなくても OnSize() からコールされる

	//!< ルートシグニチャ
	CreateRootSignature();

	//!< インプットレイアウト
	CreateInputLayout();

	//!< パイプライン
	CreatePipelineState();

	//!< バーテックスバッファ、インデックスバッファ
	auto CommandList = GraphicsCommandLists[0].Get();
	CreateVertexBuffer(CommandAllocator, CommandList);
	CreateIndexBuffer(CommandAllocator, CommandList);
	WaitForFence();
	
	//!< コンスタントバッファ
	CreateConstantBuffer();

	//CreateUnorderedAccessTexture();
}
void DX::OnSize(HWND hWnd, HINSTANCE hInstance)
{
#ifdef _DEBUG
	PerformanceCounter PC("OnSize : ");
#endif

	Super::OnSize(hWnd, hInstance);

	WaitForFence();

	const auto CommandList = GraphicsCommandLists[0].Get();
	const auto CommandAllocator = CommandAllocators[0].Get();

	VERIFY_SUCCEEDED(CommandList->Reset(CommandAllocator, nullptr));
	{		
		ResizeSwapChainToClientRect();
		ResizeDepthStencilToClientRect();
	}
	VERIFY_SUCCEEDED(CommandList->Close());

	ExecuteCommandList(CommandList);
	
	WaitForFence();

	CreateViewport(static_cast<FLOAT>(GetClientRectWidth()), static_cast<FLOAT>(GetClientRectHeight()));
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

	//!< GPUの完了を待たなくてはならない
	WaitForFence();
}
std::string DX::GetHRESULTString(const HRESULT Result)
{
	const auto WResultString = GetHRESULTStringW(Result);
#if 1
	//!< 日本語対応
	char Temporary[BUFSIZ];
	size_t NumOfCharConverted;
	wcstombs_s(&NumOfCharConverted, Temporary, WResultString.c_str(), _countof(Temporary));
	return std::string(Temporary);
#else
	return std::string(WResultString.begin(), WResultString.end());
#endif
}
std::wstring DX::GetHRESULTStringW(const HRESULT Result)
{
	//!< 16進の文字列表記
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

void DX::CreateDevice(HWND hWnd)
{
	using namespace Microsoft::WRL;

#ifdef _DEBUG
	ComPtr<ID3D12Debug> Debug;
	VERIFY_SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(Debug.GetAddressOf())));
	Debug->EnableDebugLayer();
#endif

	//!< WARP アダプタを作成するのに IDXGIFactory4(のEnumWarpAdapter) が必要
	ComPtr<IDXGIFactory4> Factory;
	VERIFY_SUCCEEDED(CreateDXGIFactory1(IID_PPV_ARGS(Factory.GetAddressOf())));
#ifdef _DEBUG
	EnumAdapter(Factory.Get());
#endif

	//!< DedicatedVideoMemory のある最後のアダプタ(GPU)インデックスを返す
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
#ifdef DEBUG_STDOUT
	DXGI_ADAPTER_DESC AdapterDesc;
	VERIFY_SUCCEEDED(Adapter->GetDesc(&AdapterDesc));
	std::cout << Lightblue << "Adapter" << White << std::endl;
	std::wcout << "\t" << Yellow << AdapterDesc.Description << White << std::endl;
#endif
	
	if (FAILED(CreateMaxFeatureLevelDevice(Adapter.Get()))) {
#ifdef DEBUG_STDOUT
		std::cout << "\t" << Red << "Cannot create device, trying to create WarpDevice ..." << White << std::endl;
#endif
		//!< WARP : Win7以下だと D3D_FEATURE_LEVEL_10_1 まで、Win8以上だと D3D_FEATURE_LEVEL_11_1 までサポート
		VERIFY_SUCCEEDED(Factory->EnumWarpAdapter(IID_PPV_ARGS(Adapter.GetAddressOf())));
		VERIFY_SUCCEEDED(CreateMaxFeatureLevelDevice(Adapter.Get()));
	}

#ifdef DEBUG_STDOUT
	std::cout << "CreateDevice" << COUT_OK << std::endl << std::endl;
#endif

#ifdef _DEBUG
		CheckFeatureLevel();
#endif
}
HRESULT DX::CreateMaxFeatureLevelDevice(IDXGIAdapter* Adapter)
{
	D3D_FEATURE_LEVEL FeatureLevel = D3D_FEATURE_LEVEL_9_1;
	for (const auto i : FeatureLevels) {
		if (SUCCEEDED(D3D12CreateDevice(Adapter, i, _uuidof(ID3D12Device), nullptr))) {
			FeatureLevel = i;
			break;
		}
	}
	return D3D12CreateDevice(Adapter, FeatureLevel, IID_PPV_ARGS(Device.GetAddressOf()));
}

//!< アダプタ(GPU)の列挙
void DX::EnumAdapter(IDXGIFactory4* Factory)
{
	using namespace Microsoft::WRL;
#ifdef DEBUG_STDOUT
	std::cout << Lightblue << "ADAPTERS" << White << std::endl;
#endif
	ComPtr<IDXGIAdapter> Adapter;
	for (UINT i = 0; DXGI_ERROR_NOT_FOUND != Factory->EnumAdapters(i, Adapter.ReleaseAndGetAddressOf()); ++i) {
		DXGI_ADAPTER_DESC AdapterDesc;
		VERIFY_SUCCEEDED(Adapter->GetDesc(&AdapterDesc));
#ifdef DEBUG_STDOUT
		std::wcout << "\t" << AdapterDesc.Description << std::endl;
		std::cout << "\t" << "\t" << "DedicatedVideoMemory = " << AdapterDesc.DedicatedVideoMemory << std::endl;
#endif

		EnumOutput(Adapter.Get());
	}
}

//!< アウトプット(ディスプレイ)の列挙
void DX::EnumOutput(IDXGIAdapter* Adapter)
{
	using namespace Microsoft::WRL;

	ComPtr<IDXGIOutput> Output;
	for (UINT i = 0; DXGI_ERROR_NOT_FOUND != Adapter->EnumOutputs(i, Output.ReleaseAndGetAddressOf()); ++i) {
		DXGI_OUTPUT_DESC OutputDesc;
		VERIFY_SUCCEEDED(Output->GetDesc(&OutputDesc));
#ifdef DEBUG_STDOUT
		const auto Width = OutputDesc.DesktopCoordinates.right - OutputDesc.DesktopCoordinates.left;
		const auto Height = OutputDesc.DesktopCoordinates.bottom - OutputDesc.DesktopCoordinates.top;
		if (0 == i) {
			std::cout << Lightblue << "\t" << "OUTPUTS" << White << std::endl;
		}
		std::wcout << "\t" << "\t" << OutputDesc.DeviceName << " = " << Width << " x " << Height << std::endl;
#endif

		GetDisplayModeList(Output.Get(), DXGI_FORMAT_R8G8B8A8_UNORM);
	}
}

//!< 描画モードの列挙
void DX::GetDisplayModeList(IDXGIOutput* Output, const DXGI_FORMAT Format)
{
	UINT NumModes;
	VERIFY_SUCCEEDED(Output->GetDisplayModeList(Format, 0, &NumModes, nullptr));
	if (NumModes) {
#ifdef DEBUG_STDOUT
		std::cout << Lightblue << "\t" << "\t" << "MODES" << White << std::endl;
#endif
		std::vector<DXGI_MODE_DESC> ModeDescs(NumModes);
		VERIFY_SUCCEEDED(Output->GetDisplayModeList(Format, 0, &NumModes, ModeDescs.data()));
#ifdef DEBUG_STDOUT
		for (const auto& i : ModeDescs) {
			//!< #TODO : DXGI_MODE_DESC を覚えておいて選択できるようにする？
			std::wcout << "\t" << "\t" << "\t" << i.Width << " x " << i.Height << " @ " << i.RefreshRate.Numerator / i.RefreshRate.Denominator << std::endl;
			std::cout << "\t" << "\t" << "\t" << "..." << std::endl; break; //!< 省略
		}
#endif
	}
}

void DX::CheckFeatureLevel()
{
	D3D12_FEATURE_DATA_FEATURE_LEVELS DataFeatureLevels = {
		static_cast<UINT>(FeatureLevels.size()), FeatureLevels.data()
	};
	VERIFY_SUCCEEDED(Device->CheckFeatureSupport(D3D12_FEATURE_FEATURE_LEVELS, reinterpret_cast<void*>(&DataFeatureLevels), sizeof(DataFeatureLevels)));
#ifdef DEBUG_STDOUT
	std::cout << Lightblue << "MaxSupportedFeatureLevel" << White << std::endl;
#define D3D_FEATURE_LEVEL_ENTRY(fl) case D3D_FEATURE_LEVEL_##fl: std::cout << Yellow << "\t" << "D3D_FEATURE_LEVEL_" #fl << White << std::endl; break;
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
#ifdef DEBUG_STDOUT
	std::cout << Lightblue << "MultiSample" << White << std::endl;
	std::cout << "\t" << GetFormatString(Format) << std::endl;
#endif

	SampleDescs.clear();
	for (UINT i = 1; i < D3D12_MAX_MULTISAMPLE_SAMPLE_COUNT; ++i) {
		D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS DataMultiSampleQaualityLevels = {
			Format,
			i,
			D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE,
		};
		VERIFY_SUCCEEDED(Device->CheckFeatureSupport(D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS, reinterpret_cast<void*>(&DataMultiSampleQaualityLevels), sizeof(DataMultiSampleQaualityLevels)));
		//!< 1 > NumQualityLevels の場合はサポートされていない
		if (DataMultiSampleQaualityLevels.NumQualityLevels) {
			const DXGI_SAMPLE_DESC SampleDesc = {
				DataMultiSampleQaualityLevels.SampleCount,
				DataMultiSampleQaualityLevels.NumQualityLevels - 1
			}; 
			SampleDescs.push_back(SampleDesc);
#ifdef DEBUG_STDOUT
			std::cout << "\t" << "Count = " << SampleDesc.Count << ", ";
			std::cout << "Quality = ";
			for (UINT i = 0; i <= SampleDesc.Quality; ++i) {
				std::cout << (i ? ", " : "") << i;
			}
			std::cout << std::endl;
#endif
		}
	}
}

/**
@brief マルチスレッドで「同じ」キューへサブミットできる
@note Vulkan ではマルチスレッドで「異なる」キューへのみサブミットできるので注意
*/
void DX::CreateCommandQueue()
{
	const D3D12_COMMAND_QUEUE_DESC CommandQueueDesc = {
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		0,
		D3D12_COMMAND_QUEUE_FLAG_NONE,
		0 // NodeMask ... マルチGPUの場合
	};
	VERIFY_SUCCEEDED(Device->CreateCommandQueue(&CommandQueueDesc, IID_PPV_ARGS(CommandQueue.GetAddressOf())));

#ifdef DEBUG_STDOUT
	std::cout << "CreateCommandQueue" << COUT_OK << std::endl << std::endl;
#endif
}

/**
@note コマンドアロケータに対し複数のコマンドリストを作成できるが、コマンドリストは同時には記録できない (Close()しないとダメ)
@note CommandList->ExecuteCommandList() 後 GPU が CommandAllocator の参照を終えるまで、CommandAllocator->Reset() してはいけない
*/
void DX::CreateCommandAllocator(const D3D12_COMMAND_LIST_TYPE CommandListType)
{
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> CommandAllocator;
	VERIFY_SUCCEEDED(Device->CreateCommandAllocator(CommandListType, IID_PPV_ARGS(CommandAllocator.GetAddressOf())));

	CommandAllocators.push_back(CommandAllocator);

#ifdef DEBUG_STDOUT
	std::cout << "\t" << "CommandAllocator" << std::endl;
#endif
}

/**
@note CommandList->ExecuteCommandList() 後に CommandList->Reset() をしても良い。(CommandAllocator が覚えているので、CommandQueue には影響しない)
描画コマンドを発行するコマンドリストは PipelineState の指定が必要
後からCommandList->Reset(Allocator, PipelineState) の引数でも指定できる
描画コマンドを発行しないコマンドリスト(初期化用途等)や、バンドルは nullptr 指定で良い
ここでは PipelineState == nullptr で作成してしまっている
*/
void DX::CreateCommandList(ID3D12CommandAllocator* CommandAllocator, const D3D12_COMMAND_LIST_TYPE CommandListType)
{
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> GraphicsCommandList;
	VERIFY_SUCCEEDED(Device->CreateCommandList(0, CommandListType, CommandAllocator, nullptr, IID_PPV_ARGS(GraphicsCommandList.GetAddressOf())));

	GraphicsCommandLists.push_back(GraphicsCommandList);

	//!< Close() しておく
	VERIFY_SUCCEEDED(GraphicsCommandLists.back()->Close());

#ifdef DEBUG_STDOUT
	std::cout << "CreateCommandList" << COUT_OK << std::endl << std::endl;
#endif
}

/**
@brief CPU と GPU の同期用
*/
void DX::CreateFence()
{
	VERIFY_SUCCEEDED(Device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(Fence.GetAddressOf())));

#ifdef DEBUG_STDOUT
	std::cout << "CreateFence" << COUT_OK << std::endl << std::endl;
#endif
}

void DX::CreateSwapChain(HWND hWnd, const DXGI_FORMAT ColorFormat, const UINT Width, const UINT Height, const UINT BufferCount)
{
	using namespace Microsoft::WRL;

	ComPtr<IDXGIFactory4> Factory;
	VERIFY_SUCCEEDED(CreateDXGIFactory1(IID_PPV_ARGS(Factory.GetAddressOf())));

	//!< #TODO : GetDisplayModeList() で DXGI_MODE_DESC を覚えておく？
	const DXGI_RATIONAL Rational = { 60, 1 };
	const DXGI_MODE_DESC ModeDesc = {
		Width, Height,
		Rational,
		ColorFormat,
		DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED,
		DXGI_MODE_SCALING_UNSPECIFIED
	}; 
	const auto& SampleDesc = SampleDescs[0];
	DXGI_SWAP_CHAIN_DESC SwapChainDesc = {
		ModeDesc,
		SampleDesc,
		DXGI_USAGE_RENDER_TARGET_OUTPUT,
		BufferCount,
		hWnd,
		TRUE,
		DXGI_SWAP_EFFECT_FLIP_DISCARD,
		DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH //!< フルスクリーンにした時、最適なディスプレイモードが選択されるのを許可
	};
	//!< セッティングを変更してスワップチェインを再作成できるように、既存のを開放している
	SwapChain.Reset(); 
	ComPtr<IDXGISwapChain> NewSwapChain;
	VERIFY_SUCCEEDED(Factory->CreateSwapChain(CommandQueue.Get(), &SwapChainDesc, NewSwapChain.GetAddressOf()));
	VERIFY_SUCCEEDED(NewSwapChain.As(&SwapChain));

#ifdef DEBUG_STDOUT
	std::cout << "\t" << "SwapChain" << std::endl;
#endif

#ifdef DEBUG_STDOUT
	std::cout << "CreateSwapChain" << COUT_OK << std::endl << std::endl;
#endif
}
void DX::CreateSwapChainDescriptorHeap()
{
	DXGI_SWAP_CHAIN_DESC1 SwapChainDesc;
	SwapChain->GetDesc1(&SwapChainDesc);

	const D3D12_DESCRIPTOR_HEAP_DESC DescripterHeapDesc = {
		D3D12_DESCRIPTOR_HEAP_TYPE_RTV,
		SwapChainDesc.BufferCount,
		D3D12_DESCRIPTOR_HEAP_FLAG_NONE,
		0 // NodeMask ... マルチGPUの場合
	};
	VERIFY_SUCCEEDED(Device->CreateDescriptorHeap(&DescripterHeapDesc, IID_PPV_ARGS(SwapChainDescriptorHeap.GetAddressOf())));

#ifdef DEBUG_STDOUT
	std::cout << "\t" << "SwapChainDescriptorHeap" << std::endl;
#endif
}

void DX::ResetSwapChainResource()
{
	for (auto& i : SwapChainResources) {
		i.Reset();
	}
}
void DX::CreateSwapChainResource()
{
	DXGI_SWAP_CHAIN_DESC1 SwapChainDesc;
	SwapChain->GetDesc1(&SwapChainDesc);

	auto CpuDescriptorHandle(SwapChainDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
	const auto IncrementSize = Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	SwapChainResources.resize(SwapChainDesc.BufferCount);
	for (auto It = SwapChainResources.begin(); It != SwapChainResources.end(); ++It) {
		const auto Index = static_cast<UINT>(std::distance(SwapChainResources.begin(), It));
		//!< スワップチェインのバッファリソースを SwapChainResources へ取得
		VERIFY_SUCCEEDED(SwapChain->GetBuffer(Index, IID_PPV_ARGS(It->GetAddressOf())));

		//!< デスクリプタ(ビュー)の作成。リソース上でのオフセットを指定して作成している、結果が変数に返るわけではない
		//!< (リソースがタイプドフォーマットなら D3D12_RENDER_TARGET_VIEW_DESC* へ nullptr 指定可能)
		Device->CreateRenderTargetView(It->Get(), nullptr, CpuDescriptorHandle);
		CpuDescriptorHandle.ptr += IncrementSize;
	}

#ifdef DEBUG_STDOUT
	std::cout << "\t" << "SwapChainResource" << std::endl;
#endif
}
void DX::ResizeSwapChain(const UINT Width, const UINT Height)
{
	ResetSwapChainResource();

	DXGI_SWAP_CHAIN_DESC1 SwapChainDesc;
	SwapChain->GetDesc1(&SwapChainDesc);
	VERIFY_SUCCEEDED(SwapChain->ResizeBuffers(SwapChainDesc.BufferCount, Width, Height, SwapChainDesc.Format, SwapChainDesc.Flags));
#ifdef DEBUG_STDOUT
	std::cout << "\t" << "ResizeBuffers" << std::endl;
#endif

	CurrentBackBufferIndex = SwapChain->GetCurrentBackBufferIndex();
#ifdef DEBUG_STDOUT
	std::cout << "\t" << "CurrentBackBufferIndex = " << CurrentBackBufferIndex << std::endl;
#endif

	CreateSwapChainResource();

#ifdef DEBUG_STDOUT
	std::cout << "ResizeSwapChain" << COUT_OK << std::endl << std::endl;
#endif
}

void DX::CreateDepthStencilDescriptorHeap()
{
	const D3D12_DESCRIPTOR_HEAP_DESC DescriptorHeapDesc = {
		D3D12_DESCRIPTOR_HEAP_TYPE_DSV,
		1,
		D3D12_DESCRIPTOR_HEAP_FLAG_NONE,
		0
	};
	VERIFY_SUCCEEDED(Device->CreateDescriptorHeap(&DescriptorHeapDesc, IID_PPV_ARGS(DepthStencilDescriptorHeap.GetAddressOf())));
#ifdef DEBUG_STDOUT
	std::cout << "\t" << "DepthStencilDescriptorHeap" << std::endl;
#endif

#ifdef DEBUG_STDOUT
	std::cout << "CreateDepthStencil" << COUT_OK << std::endl << std::endl;
#endif
}

void DX::ResetDepthStencilResource()
{
	DepthStencilResource.Reset();
}
void DX::CreateDepthStencilResource(const UINT Width, const UINT Height, const DXGI_FORMAT DepthFormat)
{
	//!< リソースの作成
	const D3D12_HEAP_PROPERTIES HeapProperties = {
		D3D12_HEAP_TYPE_DEFAULT, //!< DEFAULT にすること
		D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
		D3D12_MEMORY_POOL_UNKNOWN,
		1,
		1
	};
	const auto& SampleDesc = SampleDescs[0]; //!< レンダーターゲットのものと一致すること
	const D3D12_RESOURCE_DESC ResourceDesc = {
		D3D12_RESOURCE_DIMENSION_TEXTURE2D,
		0,
		Width, Height,
		1,
		1,
		DepthFormat,
		SampleDesc,
		D3D12_TEXTURE_LAYOUT_UNKNOWN,
		D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL
	};
	//!< 一致するクリア値なら最適化されるのでよく使うクリア値を指定しておく
	const D3D12_CLEAR_VALUE ClearValue = {
		DepthFormat,
		{ 1.0f, 0 }
	};
	VERIFY_SUCCEEDED(Device->CreateCommittedResource(&HeapProperties,
		D3D12_HEAP_FLAG_NONE,
		&ResourceDesc,
		D3D12_RESOURCE_STATE_COMMON, //!< COMMON にすること
		&ClearValue,
		IID_PPV_ARGS(DepthStencilResource.ReleaseAndGetAddressOf())));

	auto CpuDescriptorHandle(DepthStencilDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
	const auto IncrementSize = Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV); //!< ここでは必要ないが一応

	//!< デスクリプタ(ビュー)の作成。リソース上でのオフセットを指定して作成している、結果が変数に返るわけではない
	//!< (リソースがタイプドフォーマットなら D3D12_DEPTH_STENCIL_VIEW_DESC* へ nullptr 指定可能)
	Device->CreateDepthStencilView(DepthStencilResource.Get(), nullptr, CpuDescriptorHandle);
	CpuDescriptorHandle.ptr += IncrementSize; //!< ここでは必要ないが一応

#ifdef DEBUG_STDOUT
	std::cout << "\t" << "DepthStencilView" << std::endl;
#endif

	//!< リソースの状態を初期 → デプスへ変更
	auto CommandList = GraphicsCommandLists[0];
	BarrierTransition(CommandList.Get(), DepthStencilResource.Get(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_DEPTH_WRITE);

#ifdef DEBUG_STDOUT
	std::cout << "CreateDepthStencilResource" << COUT_OK << std::endl << std::endl;
#endif
}
void DX::ResizeDepthStencil(const UINT Width, const UINT Height, const DXGI_FORMAT DepthFormat)
{
	ResetDepthStencilResource();
	CreateDepthStencilResource(Width, Height, DepthFormat);

#ifdef DEBUG_STDOUT
	std::cout << "ResizeDepthStencil" << COUT_OK << std::endl << std::endl;
#endif
}

//!< # TODO ここの実装は消す、Extへ持っていく
void DX::CreateRootSignature()
{
	using namespace Microsoft::WRL;

#if 0
	const std::vector<D3D12_DESCRIPTOR_RANGE> DescriptorRanges = {
		{
			D3D12_DESCRIPTOR_RANGE_TYPE_CBV,
			1, //!< NumDescriptors
			0, //!< BaseShaderRegister ... HLSL で register(b0) なら 0、register(t3) なら 3 という感じ
			0, //!< RegisterSpace ... 通常は 0 でよい。HLSL で register(t3, space5) なら 5
			D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND //!< OffsetInDescriptorsFromTableStart ... 通常は D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND でよい
		},
	};
	const D3D12_ROOT_DESCRIPTOR_TABLE DescriptorTable = {
		static_cast<UINT>(DescriptorRanges.size()), DescriptorRanges.data()
	};
	const std::vector<D3D12_ROOT_PARAMETER> RootParameters = {
		{
			D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE,
			DescriptorTable,
			D3D12_SHADER_VISIBILITY_ALL //!< #TODO 必要とするシェーダに限定すべき
		},
	};
	const std::vector<D3D12_STATIC_SAMPLER_DESC> StaticSamplerDescs = {
	};
	const D3D12_ROOT_SIGNATURE_DESC RootSignatureDesc = {
		static_cast<UINT>(RootParameters.size()), RootParameters.data(),
		static_cast<UINT>(StaticSamplerDescs.size()), StaticSamplerDescs.data(),
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT
	};
	ComPtr<ID3DBlob> Blob;
	ComPtr<ID3DBlob> ErrorBlob;
	VERIFY_SUCCEEDED(D3D12SerializeRootSignature(&RootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, Blob.GetAddressOf(), ErrorBlob.GetAddressOf()));
	VERIFY_SUCCEEDED(Device->CreateRootSignature(0, Blob->GetBufferPointer(), Blob->GetBufferSize(), IID_PPV_ARGS(RootSignature.GetAddressOf())));
#else
	const std::vector<D3D12_DESCRIPTOR_RANGE> DescriptorRanges = {
	};
	const D3D12_ROOT_DESCRIPTOR_TABLE DescriptorTable = {
		static_cast<UINT>(DescriptorRanges.size()), DescriptorRanges.data()
	};
	const std::vector<D3D12_ROOT_PARAMETER> RootParameters = {
	};
	const std::vector<D3D12_STATIC_SAMPLER_DESC> StaticSamplerDescs = {
	};
	const D3D12_ROOT_SIGNATURE_DESC RootSignatureDesc = {
		static_cast<UINT>(RootParameters.size()), RootParameters.data(),
		static_cast<UINT>(StaticSamplerDescs.size()), StaticSamplerDescs.data(),
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT
	};
	ComPtr<ID3DBlob> Blob;
	ComPtr<ID3DBlob> ErrorBlob;
	VERIFY_SUCCEEDED(D3D12SerializeRootSignature(&RootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, Blob.GetAddressOf(), ErrorBlob.GetAddressOf()));
	VERIFY_SUCCEEDED(Device->CreateRootSignature(0, Blob->GetBufferPointer(), Blob->GetBufferSize(), IID_PPV_ARGS(RootSignature.GetAddressOf())));
#endif

#ifdef DEBUG_STDOUT
	std::cout << "CreateRootSignature" << COUT_OK << std::endl << std::endl;
#endif
}

void DX::CreateInputLayout()
{
#ifdef DEBUG_STDOUT
	std::cout << "CreateInputLayout" << COUT_OK << std::endl << std::endl;
#endif
}

void DX::CreateViewport(const FLOAT Width, const FLOAT Height, const FLOAT MinDepth, const FLOAT MaxDepth)
{
	Viewports = {
		{ 
			0.0f, 0.0f, 
			Width, Height,
			MinDepth, MaxDepth
		}
	};

	ScissorRects = {
		{ 
			0, 0, 
			static_cast<LONG>(Width), static_cast<LONG>(Height)
		}
	};

#ifdef DEBUG_STDOUT
	std::cout << "CreateViewport" << COUT_OK << std::endl << std::endl;
#endif
}
void DX::CreateGraphicsPipelineState()
{
#ifdef DEBUG_STDOUT
	std::cout << "CreateGraphicsPipelineState" << COUT_OK << std::endl << std::endl;
#endif
}

void DX::CreateComputePipelineState()
{
	assert(nullptr != RootSignature);

	std::vector<Microsoft::WRL::ComPtr<ID3DBlob>> ShaderBlobs(1);
	D3DReadFileToBlob(SHADER_PATH L"CS.cso", ShaderBlobs[0].GetAddressOf());
	const D3D12_SHADER_BYTECODE ShaderBytecodesCS = { ShaderBlobs[0]->GetBufferPointer(), ShaderBlobs[0]->GetBufferSize() };

	const D3D12_CACHED_PIPELINE_STATE CachedPipelineState = { nullptr, 0 };
	const D3D12_COMPUTE_PIPELINE_STATE_DESC ComputePipelineStateDesc = {
		RootSignature.Get(),
		ShaderBytecodesCS,
		0, // NodeMask ... マルチGPUの場合
		CachedPipelineState,
		D3D12_PIPELINE_STATE_FLAG_NONE
	};
	VERIFY_SUCCEEDED(Device->CreateComputePipelineState(&ComputePipelineStateDesc, IID_PPV_ARGS(PipelineState.GetAddressOf())));

#ifdef DEBUG_STDOUT
	std::cout << "CreateComputePipelineState" << COUT_OK << std::endl << std::endl;
#endif
}

/**
@note 
アップロード用のリソースを D3D12_HEAP_TYPE_UPLOAD で作成してそこにデータをコピーする
目的のリソースは D3D12_HEAP_TYPE_DEFAULT で作成する (頻繁に更新しないリソースは D3D12_HEAP_TYPE_DEFAULT にしておきたい)
アップロードリソースから目的のリソースへのコピーコマンドを発行する
*/
void DX::CreateDefaultBuffer(ID3D12CommandAllocator* CommandAllocator, ID3D12GraphicsCommandList* CommandList, ID3D12Resource** Resource, const size_t Size, const void* Source)
{
	//!< リソースデスクリプタ(共用)
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

	//!< アップロード用のリソースを作成
	Microsoft::WRL::ComPtr<ID3D12Resource> UploadResource;
	const D3D12_HEAP_PROPERTIES UploadHeapProperties = {
		D3D12_HEAP_TYPE_UPLOAD, //!< UPLOAD にすること
		D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
		D3D12_MEMORY_POOL_UNKNOWN,
		1,
		1
	};
	VERIFY_SUCCEEDED(Device->CreateCommittedResource(&UploadHeapProperties,
		D3D12_HEAP_FLAG_NONE,
		&ResourceDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ, //!< UPLOAD では GENERIC_READ にすること
		nullptr,
		IID_PPV_ARGS(UploadResource.GetAddressOf())));
	BYTE* Data;
	VERIFY_SUCCEEDED(UploadResource->Map(0, nullptr, reinterpret_cast<void**>(&Data))); {
		memcpy(Data, Source, Size);
	} UploadResource->Unmap(0, nullptr);

	//!< 目的のリソースを作成
	const D3D12_HEAP_PROPERTIES HeapProperties = {
		D3D12_HEAP_TYPE_DEFAULT, //!< DEFAULT にすること
		D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
		D3D12_MEMORY_POOL_UNKNOWN,
		1,
		1
	};
	VERIFY_SUCCEEDED(Device->CreateCommittedResource(&HeapProperties,
		D3D12_HEAP_FLAG_NONE,
		&ResourceDesc,
		D3D12_RESOURCE_STATE_COMMON, //!< COMMON にすること
		nullptr,
		IID_PPV_ARGS(Resource)));

	//!< コピーコマンドを発行
	VERIFY_SUCCEEDED(CommandList->Reset(CommandAllocator, nullptr)); {
		BarrierTransition(CommandList, *Resource, D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST); {
			CommandList->CopyBufferRegion(*Resource, 0, UploadResource.Get(), 0, Size);
		} BarrierTransition(CommandList, *Resource, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ);
	} VERIFY_SUCCEEDED(CommandList->Close());
	ExecuteCommandList(CommandList);
	//WaitForFence();
}

/**
@note
アップロード用のリソースを D3D12_HEAP_TYPE_UPLOAD で作成する (頻繁に更新するリソースは D3D12_HEAP_TYPE_UPLOAD にしておきたい)
*/
void DX::CreateUploadBuffer(ID3D12Resource** Resource, const size_t Size, const void* Source)
{
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
	const D3D12_HEAP_PROPERTIES HeapProperties = {
		D3D12_HEAP_TYPE_UPLOAD, //!< UPLOAD にすること
		D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
		D3D12_MEMORY_POOL_UNKNOWN,
		1,
		1
	};
	VERIFY_SUCCEEDED(Device->CreateCommittedResource(&HeapProperties,
		D3D12_HEAP_FLAG_NONE,
		&ResourceDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ, //!< UPLOAD では GENERIC_READ にすること
		nullptr,
		IID_PPV_ARGS(Resource)));

	if (nullptr != Source) {
		BYTE* Data;
		VERIFY_SUCCEEDED((*Resource)->Map(0, nullptr, reinterpret_cast<void**>(&Data))); {
			memcpy(Data, Source, Size);
		} (*Resource)->Unmap(0, nullptr);
	}
}

void DX::CreateVertexBuffer(ID3D12CommandAllocator* CommandAllocator, ID3D12GraphicsCommandList* CommandList)
{
	//!< CPU 側にもコピーを持たせる、多分必要ない?
	//Microsoft::WRL::ComPtr<ID3DBlob> VertexBufferBlob;
	//VERIFY_SUCCEEDED(D3DCreateBlob(Size, VertexBufferBlob.GetAddressOf()));
	//CopyMemory(VertexBufferBlob->GetBufferPointer(), Vertices.data(), Size);

#ifdef DEBUG_STDOUT
	std::cout << "CreateVertexBuffer" << COUT_OK << std::endl << std::endl;
#endif
}
void DX::CreateIndexBuffer(ID3D12CommandAllocator* CommandAllocator, ID3D12GraphicsCommandList* CommandList)
{
	//!< CPU 側にもコピーを持たせる、多分必要ない?
	//Microsoft::WRL::ComPtr<ID3DBlob> IndexBufferBlob;
	//VERIFY_SUCCEEDED(D3DCreateBlob(Size, IndexBufferBlob.GetAddressOf()));
	//CopyMemory(IndexBufferBlob->GetBufferPointer(), Indices.data(), Size);

#ifdef DEBUG_STDOUT
	std::cout << "CreateIndexBuffer" << COUT_OK << std::endl << std::endl;
#endif
}

/**
std::vector<ID3D12DescriptorHeap*> DescriptorHeaps = { ConstantBufferDescriptorHeap.Get() };
GraphicsCommandList->SetDescriptorHeaps(static_cast<UINT>(DescriptorHeaps.size()), DescriptorHeaps.data());

auto CVDescriptorHandle(ConstantBufferDescriptorHeap->GetGPUDescriptorHandleForHeapStart());
const auto CVIncrementSize = Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
CVDescriptorHandle.ptr += 0 * CVIncrementSize;
GraphicsCommandList->SetGraphicsRootDescriptorTable(0, CVDescriptorHandle);
*/
void DX::CreateConstantBuffer()
{
#ifdef DEBUG_STDOUT
	std::cout << "CreateConstantBuffer" << COUT_OK << std::endl << std::endl;
#endif
}
void DX::CreateConstantBufferDescriptorHeap(const UINT Size)
{
	//!< デスクリプタヒープの作成
	const D3D12_DESCRIPTOR_HEAP_DESC DescriptorHeapDesc = {
		D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
		1,
		D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE,
		0 // NodeMask ... マルチGPUの場合
	};
	VERIFY_SUCCEEDED(Device->CreateDescriptorHeap(&DescriptorHeapDesc, IID_PPV_ARGS(ConstantBufferDescriptorHeap.GetAddressOf())));

	//!< ここではコンスタントバッファ全体を指定している
	//!< D3D12_CONSTANT_BUFFER_VIEW_DESC.BufferLocation, SizeInBytes は 256 の倍数で指定しなくはならない、
	const D3D12_CONSTANT_BUFFER_VIEW_DESC ConstantBufferViewDesc = {
		ConstantBufferResource->GetGPUVirtualAddress(),
		Size
	};
	//!< デスクリプタ(ビュー)の作成。リソース上でのオフセットを指定して作成している、結果が変数に返るわけではない
	Device->CreateConstantBufferView(&ConstantBufferViewDesc, ConstantBufferDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

#ifdef DEBUG_STDOUT
	std::cout << "CreateConstantBufferDescriptorHeap" << COUT_OK << std::endl << std::endl;
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
		D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS //!< ALLOW_UNORDERED_ACCESS にする
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
		2, //!< SRV, UAV の 2 つ
		D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE,
		0 // NodeMask ... マルチGPUの場合
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
	//!< デスクリプタ(ビュー)の作成。リソース上でのオフセットを指定して作成している、結果が変数に返るわけではない
	Device->CreateShaderResourceView(UnorderedAccessTextureResource.Get(), &SRVDesc, CPUDescriptorHandle);

	CPUDescriptorHandle.ptr += 1 * IncrementSize;
	/*const*/D3D12_UNORDERED_ACCESS_VIEW_DESC UAVDesc = {
		Format,
		D3D12_UAV_DIMENSION_TEXTURE2D,
	};
	UAVDesc.Texture2D.MipSlice = 0;
	//!< デスクリプタ(ビュー)の作成。リソース上でのオフセットを指定して作成している、結果が変数に返るわけではない
	Device->CreateUnorderedAccessView(UnorderedAccessTextureResource.Get(), nullptr, &UAVDesc, CPUDescriptorHandle);

#ifdef DEBUG_STDOUT
	std::cout << "CreateUnorderedAccessBuffer" << COUT_OK << std::endl << std::endl;
#endif
}

void DX::PopulateCommandList(ID3D12GraphicsCommandList* GraphicsCommandList)
{
	Clear(GraphicsCommandList);
}

void DX::BarrierTransition(ID3D12GraphicsCommandList* CommandList, ID3D12Resource* Resource, const D3D12_RESOURCE_STATES Before, const D3D12_RESOURCE_STATES After)
{
	const D3D12_RESOURCE_TRANSITION_BARRIER ResourceTransitionBarrier = {
		Resource,
		D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES,
		Before,
		After
	};
	const std::vector<D3D12_RESOURCE_BARRIER> ResourceBarrier = {
		{
			D3D12_RESOURCE_BARRIER_TYPE_TRANSITION,
			D3D12_RESOURCE_BARRIER_FLAG_NONE,
			ResourceTransitionBarrier
		}
	};
	CommandList->ResourceBarrier(static_cast<UINT>(ResourceBarrier.size()), ResourceBarrier.data());
}

void DX::Draw()
{
	if (CommandAllocators.empty() || GraphicsCommandLists.empty()) { return; }

	const auto CommandAllocator = CommandAllocators[0].Get();
	const auto CommandList = GraphicsCommandLists[0].Get();

	//!< GPU が参照している間は CommandAllocator->Reset() できない
	VERIFY_SUCCEEDED(CommandAllocator->Reset());
	
	//!< CommandQueue->ExecuteCommandLists() 後に CommandList->Reset() でリセットして再利用が可能 (コマンドキューはコマンドリストではなく、コマンドアロケータを参照している)
	//!< CommandList 作成時に PipelineState を指定していなくても、ここで指定すれば OK
	VERIFY_SUCCEEDED(CommandList->Reset(CommandAllocator, PipelineState.Get()));
	{
		CommandList->RSSetViewports(static_cast<UINT>(Viewports.size()), Viewports.data());
		CommandList->RSSetScissorRects(static_cast<UINT>(ScissorRects.size()), ScissorRects.data());

		auto Resource = SwapChainResources[CurrentBackBufferIndex].Get();
		BarrierTransition(CommandList, Resource, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
		{
			PopulateCommandList(CommandList);
		}
		BarrierTransition(CommandList, Resource, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
	}
	VERIFY_SUCCEEDED(CommandList->Close());

	ExecuteCommandList(CommandList);

	WaitForFence();

	Present();
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
	//!< CPU 側のフェンス値をインクリメント
	++FenceValue;

	//!< GPU コマンドが Signal() まで到達すれば GetCompletedValue() が FenceValue になり、CPUに追いついたことになる
	VERIFY_SUCCEEDED(CommandQueue->Signal(Fence.Get(), FenceValue));
	if (Fence->GetCompletedValue() < FenceValue) {
		auto hEvent = CreateEventEx(nullptr, nullptr, 0, EVENT_ALL_ACCESS);
		//!< GetCompletedValue() が FenceValue になったらイベントが発行される
		VERIFY_SUCCEEDED(Fence->SetEventOnCompletion(FenceValue, hEvent));

		//!< イベント発行まで待つ
		WaitForSingleObject(hEvent, INFINITE);
		CloseHandle(hEvent);
	}
}
