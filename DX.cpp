#include "stdafx.h"

#include "DX.h"

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "dxgi.lib")

void DX::OnCreate(HWND hWnd, HINSTANCE hInstance, LPCWSTR Title)
{
#ifdef _DEBUG
	PerformanceCounter PC("OnCreate : ");
#endif

	Super::OnCreate(hWnd, hInstance, Title);

	const auto ColorFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
	CreateDevice(hWnd);
	CheckMultiSample(ColorFormat);
	CreateCommandQueue();

	CreateFence();

	CreateSwapchain(hWnd, ColorFormat);

	CreateDepthStencil();
	
	CreateVertexBuffer();
	CreateIndexBuffer();
	CreateIndirectBuffer();

	CreateTexture();

	CreateRootSignature();
	{
		CreateConstantBuffer();
	}
	CreateDescriptorHeap();
	UpdateDescriptorHeap();

	CreatePipelineState();

	//CreateUnorderedAccessTexture();
}
void DX::OnSize(HWND hWnd, HINSTANCE hInstance)
{
#ifdef _DEBUG
	PerformanceCounter PC("OnSize : ");
#endif

	Super::OnSize(hWnd, hInstance);

	WaitForFence();

	//const auto CommandList = GraphicsCommandLists[0].Get();
	//const auto CommandAllocator = CommandAllocators[0].Get();

	//VERIFY_SUCCEEDED(CommandList->Reset(CommandAllocator, nullptr));
	//{		
	//	ResizeSwapChainToClientRect();
	//	ResizeDepthStencilToClientRect();
	//}
	//VERIFY_SUCCEEDED(CommandList->Close());

	//ExecuteCommandListAndWaitForFence(CommandList);

	CreateViewport(static_cast<FLOAT>(GetClientRectWidth()), static_cast<FLOAT>(GetClientRectHeight()));

	for (auto i = 0; i < GraphicsCommandLists.size(); ++i) {
		PopulateCommandList(i);
	}
}
void DX::OnDestroy(HWND hWnd, HINSTANCE hInstance)
{
	Super::OnDestroy(hWnd, hInstance);

	//!< GPUが完了するまでここで待機 
	//!< Wait GPU
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

void DX::CreateUploadResource(ID3D12Resource** Resource, const size_t Size)
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
		D3D12_HEAP_TYPE_UPLOAD, //!< UPLOAD にすること Must be UPLOAD
		D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
		D3D12_MEMORY_POOL_UNKNOWN,
		1,
		1
	};
	VERIFY_SUCCEEDED(Device->CreateCommittedResource(&HeapProperties,
		D3D12_HEAP_FLAG_NONE,
		&ResourceDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ, //!< GENERIC_READ にすること Must be GENERIC_READ
		nullptr,
		IID_PPV_ARGS(Resource)));
}
void DX::CopyToUploadResource(ID3D12Resource* Resource, const size_t Size, const void* Source)
{
	if (nullptr != Resource && Size && nullptr != Source) {
		BYTE* Data;
		VERIFY_SUCCEEDED(Resource->Map(0, static_cast<const D3D12_RANGE*>(nullptr), reinterpret_cast<void**>(&Data))); {
			memcpy(Data, Source, Size);
		} Resource->Unmap(0, nullptr);
	}
}
void DX::CopyToUploadResource(ID3D12Resource* Resource, const std::vector<D3D12_SUBRESOURCE_DATA>& SubresourceData, const std::vector<D3D12_PLACED_SUBRESOURCE_FOOTPRINT>& PlacedSubresourceFootprints, const std::vector<UINT>& NumRows, const std::vector<UINT64>& RowSizes)
{
	if (nullptr != Resource) {
		assert(SubresourceData.size() == PlacedSubresourceFootprints.size() == NumRows.size() == RowSizes.size() && "Invalid size");
		const auto SubresourceCount = static_cast<const UINT>(SubresourceData.size());

		BYTE* Data;
		VERIFY_SUCCEEDED(Resource->Map(0, nullptr, reinterpret_cast<void**>(&Data))); {
			for (auto It = PlacedSubresourceFootprints.cbegin(); It != PlacedSubresourceFootprints.cend(); ++It) {
				const auto Index = std::distance(PlacedSubresourceFootprints.cbegin(), It);

				const auto& PSF = *It;
				const auto& SD = SubresourceData[Index];
				const auto RowCount = NumRows[Index];
				const auto RowSize = RowSizes[Index];

				const D3D12_MEMCPY_DEST MemcpyDest = {
					Data + PSF.Offset,
					PSF.Footprint.RowPitch,
					PSF.Footprint.RowPitch * RowCount
				};
				for (UINT i = 0; i < PSF.Footprint.Depth; ++i) {
					auto Dst = reinterpret_cast<BYTE*>(MemcpyDest.pData) + MemcpyDest.SlicePitch * i;
					const auto Src = reinterpret_cast<const BYTE*>(SD.pData) + SD.SlicePitch * i;
					for (UINT j = 0; j < RowCount; ++j) {
						memcpy(Dst + MemcpyDest.RowPitch * j, Src + SD.RowPitch * j, RowSize);
					}
				}
			}
		} Resource->Unmap(0, nullptr);
	}
}

void DX::ExecuteCopyBuffer(ID3D12CommandAllocator* CommandAllocator, ID3D12GraphicsCommandList* CommandList, ID3D12Resource* SrcResource, ID3D12Resource* DstResource, const size_t Size)
{
	VERIFY_SUCCEEDED(CommandList->Reset(CommandAllocator, nullptr)); {
		PopulateCopyBufferCommand(CommandList, SrcResource, DstResource, Size, D3D12_RESOURCE_STATE_GENERIC_READ);
	} VERIFY_SUCCEEDED(CommandList->Close());

	const std::vector<ID3D12CommandList*> CommandLists = { CommandList };
	CommandQueue->ExecuteCommandLists(static_cast<UINT>(CommandLists.size()), CommandLists.data());
	WaitForFence();
}

void DX::ExecuteCopyTexture(ID3D12CommandAllocator* CommandAllocator, ID3D12GraphicsCommandList* CommandList, ID3D12Resource* SrcResource, ID3D12Resource* DstResource, const std::vector<D3D12_PLACED_SUBRESOURCE_FOOTPRINT>& PlacedSubresourceFootprints, const D3D12_RESOURCE_STATES ResourceState)
{
	VERIFY_SUCCEEDED(CommandList->Reset(CommandAllocator, nullptr)); {
		if (D3D12_RESOURCE_DIMENSION_BUFFER == DstResource->GetDesc().Dimension) {
			PopulateCopyBufferCommand(CommandList, SrcResource, DstResource, PlacedSubresourceFootprints, ResourceState);
		}
		else {
			PopulateCopyTextureCommand(CommandList, SrcResource, DstResource, PlacedSubresourceFootprints, ResourceState);
		}
	} VERIFY_SUCCEEDED(CommandList->Close());

	const std::vector<ID3D12CommandList*> CommandLists = { CommandList };
	CommandQueue->ExecuteCommandLists(static_cast<UINT>(CommandLists.size()), CommandLists.data());
	WaitForFence();
}

void DX::CreateDefaultResource(ID3D12Resource** Resource, const size_t Size)
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
		D3D12_HEAP_TYPE_DEFAULT, //!< DEFAULT にすること Must be DEFAULT
		D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
		D3D12_MEMORY_POOL_UNKNOWN,
		1,
		1
	};
	VERIFY_SUCCEEDED(Device->CreateCommittedResource(&HeapProperties,
		D3D12_HEAP_FLAG_NONE,
		&ResourceDesc,
		D3D12_RESOURCE_STATE_COMMON, //!< COMMON にすること Must be COMMON
		nullptr,
		IID_PPV_ARGS(Resource)));
}
void DX::ResourceBarrier(ID3D12GraphicsCommandList* CL, ID3D12Resource* Resource, const D3D12_RESOURCE_STATES Before, const D3D12_RESOURCE_STATES After)
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
	CL->ResourceBarrier(static_cast<UINT>(ResourceBarrier.size()), ResourceBarrier.data());
}
void DX::PopulateCopyTextureCommand(ID3D12GraphicsCommandList* CommandList, ID3D12Resource* Src, ID3D12Resource* Dst, const std::vector<D3D12_PLACED_SUBRESOURCE_FOOTPRINT>& PlacedSubresourceFootprints, const D3D12_RESOURCE_STATES ResourceState)
{
	//!< Dst(LoadDDSTextureFromFile()で作成される)のステートは既にD3D12_RESOURCE_STATE_COPY_DESTで作成されている
	//!< Dst(created from LoadDDSTextureFromFile())'s state is already D3D12_RESOURCE_STATE_COPY_DEST
	//ResourceBarrier(CommandList, Dst, D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST); {
	{
		for (auto It = PlacedSubresourceFootprints.cbegin(); It != PlacedSubresourceFootprints.cend(); ++It) {
			const D3D12_TEXTURE_COPY_LOCATION TextureCopyLocation_Dst = {
				Dst,
				D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX,
				static_cast<const UINT>(std::distance(PlacedSubresourceFootprints.cbegin(), It))
			};
			const D3D12_TEXTURE_COPY_LOCATION TextureCopyLocation_Src = {
				Src,
				D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT,
				*It
			};
			CommandList->CopyTextureRegion(&TextureCopyLocation_Dst, 0, 0, 0, &TextureCopyLocation_Src, nullptr);
		}
	} ResourceBarrier(CommandList, Dst, D3D12_RESOURCE_STATE_COPY_DEST, ResourceState);
}
void DX::PopulateCopyBufferCommand(ID3D12GraphicsCommandList* CommandList, ID3D12Resource* Src, ID3D12Resource* Dst, const std::vector<D3D12_PLACED_SUBRESOURCE_FOOTPRINT>& PlacedSubresourceFootprints, const D3D12_RESOURCE_STATES ResourceState)
{
	ResourceBarrier(CommandList, Dst, D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST); {
		for (auto It = PlacedSubresourceFootprints.cbegin(); It != PlacedSubresourceFootprints.cend(); ++It) {
			//!< 色々なサンプルを見るとことごとく D3D12_BOX を作っているがどれも使ってはいない
			//const D3D12_BOX Box = {
			//	static_cast<UINT>(It->Offset),
			//	0,
			//	0,
			//	static_cast<UINT>(It->Offset) + It->Footprint.Width,
			//	1,
			//	1
			//};
			CommandList->CopyBufferRegion(Dst, 0, Src, It->Offset, It->Footprint.Width);
		}
	} ResourceBarrier(CommandList, Dst, D3D12_RESOURCE_STATE_COPY_DEST, ResourceState);
}
void DX::PopulateCopyBufferCommand(ID3D12GraphicsCommandList* CommandList, ID3D12Resource* Src, ID3D12Resource* Dst, const UINT64 Size, const D3D12_RESOURCE_STATES ResourceState)
{
	ResourceBarrier(CommandList, Dst, D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST); {
		CommandList->CopyBufferRegion(Dst, 0, Src, 0, Size);
	} ResourceBarrier(CommandList, Dst, D3D12_RESOURCE_STATE_COPY_DEST, ResourceState);
}

#ifdef _DEBUG
void DX::SetMarker(ID3D12GraphicsCommandList* CommandList, LPCWSTR Name, const UINT Color)
{
	PIXSetMarker(CommandList, Color, Name);
}

void DX::BeginEvent(ID3D12GraphicsCommandList* CommandList, LPCWSTR Name)
{
#if 0
	const auto Size = static_cast<UINT>((wcslen(Name) + 1) * sizeof(Name[0]));
	CommandList->BeginEvent(0, Name, Size);
#else
	PIXBeginEvent(CommandList, PIX_COLOR(255, 0, 0), Name);
#endif
}
void DX::EndEvent(ID3D12GraphicsCommandList* CommandList)
{
#if 0
	CommandList->EndEvent();
#else
	PIXEndEvent();
#endif
}
#endif //!< _DEBUG


void DX::CreateDevice(HWND hWnd)
{
	using namespace Microsoft::WRL;

#ifdef _DEBUG
	ComPtr<ID3D12Debug> Debug;
	VERIFY_SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(Debug.GetAddressOf())));
	Debug->EnableDebugLayer();
	
	//!< GPU-Based Validation
#if 0
	ComPtr<ID3D12Debug1> Debug1;
	VERIFY_SUCCEEDED(Debug->QueryInterface(IID_PPV_ARGS(Debug1.GetAddressOf())));
	Debug1->SetEnableGPUBasedValidation(true);
#endif
#endif //!< _DEBUG

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
	auto FeatureLevel = D3D_FEATURE_LEVEL_9_1;
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

D3D12_CPU_DESCRIPTOR_HANDLE DX::GetCPUDescriptorHandle(ID3D12DescriptorHeap* DescriptorHeap, const D3D12_DESCRIPTOR_HEAP_TYPE Type, const UINT Index /*= 0*/) const
{
	auto DescriptorHandle(DescriptorHeap->GetCPUDescriptorHandleForHeapStart());
	DescriptorHandle.ptr += Index * Device->GetDescriptorHandleIncrementSize(Type);
	return DescriptorHandle;
}
D3D12_GPU_DESCRIPTOR_HANDLE DX::GetGPUDescriptorHandle(ID3D12DescriptorHeap* DescriptorHeap, const D3D12_DESCRIPTOR_HEAP_TYPE Type, const UINT Index /*= 0*/) const
{
	auto DescriptorHandle(DescriptorHeap->GetGPUDescriptorHandleForHeapStart());
	DescriptorHandle.ptr += Index * Device->GetDescriptorHandleIncrementSize(Type);
	return DescriptorHandle;
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
@brief CPU と GPU の同期用
*/
void DX::CreateFence()
{
	VERIFY_SUCCEEDED(Device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(Fence.GetAddressOf())));

#ifdef DEBUG_STDOUT
	std::cout << "CreateFence" << COUT_OK << std::endl << std::endl;
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
void DX::CreateCommandList(ID3D12CommandAllocator* CommandAllocator, const size_t Count, const D3D12_COMMAND_LIST_TYPE CommandListType)
{
	for (auto i = 0; i < Count; ++i) {
		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> GraphicsCommandList;
		VERIFY_SUCCEEDED(Device->CreateCommandList(0, CommandListType, CommandAllocator, nullptr, IID_PPV_ARGS(GraphicsCommandList.GetAddressOf())));

		GraphicsCommandLists.push_back(GraphicsCommandList);

		//!< Close() しておく
		VERIFY_SUCCEEDED(GraphicsCommandLists.back()->Close());
	}

#ifdef DEBUG_STDOUT
	std::cout << "CreateCommandList" << COUT_OK << std::endl << std::endl;
#endif
}

void DX::CreateSwapchain(HWND hWnd, const DXGI_FORMAT ColorFormat)
{
	CreateSwapChainOfClientRect(hWnd, ColorFormat);

	//!< ビューを作成 Create view
	CreateSwapChainResource();

	//!< スワップチェインの枚数が決まったので、ここでコマンドリストを作成
	//!< イメージの初期化(する場合)でコマンドリストが必要
	//!< Count of swapchain is fixed, create commandlist here
	//!< In case initialize image, we need command list
	CreateCommandAllocator();
	DXGI_SWAP_CHAIN_DESC1 SwapChainDesc;
	SwapChain->GetDesc1(&SwapChainDesc);
	CreateCommandList(CommandAllocators[0].Get(), SwapChainDesc.BufferCount);
#if 1
	//!< イメージの初期化 Initialize images #TODO
	//InitializeSwapchainImage(CommandAllocators[0].Get());
	InitializeSwapchainImage(CommandAllocators[0].Get(), &DirectX::Colors::Red);
#endif
}
void DX::CreateSwapChain(HWND hWnd, const DXGI_FORMAT ColorFormat, const UINT Width, const UINT Height)
{
	using namespace Microsoft::WRL;

	const UINT BufferCount = 3;

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

	{
		const auto Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		const auto Count = SwapChainDesc.BufferCount;

		[&](const D3D12_DESCRIPTOR_HEAP_TYPE Type, const UINT Count, ID3D12DescriptorHeap** DescriptorHeap) {
			const D3D12_DESCRIPTOR_HEAP_DESC DescriptorHeapDesc = {
				Type,
				Count,
				D3D12_DESCRIPTOR_HEAP_FLAG_NONE,
				0 // NodeMask ... マルチGPUの場合
			};
			VERIFY_SUCCEEDED(Device->CreateDescriptorHeap(&DescriptorHeapDesc, IID_PPV_ARGS(DescriptorHeap)));
		}(Type, Count, SwapChainDescriptorHeap.GetAddressOf());
	}

#ifdef DEBUG_STDOUT
	std::cout << "CreateSwapChain" << COUT_OK << std::endl << std::endl;
#endif
}
void DX::CreateSwapChainResource()
{
	DXGI_SWAP_CHAIN_DESC1 SwapChainDesc;
	SwapChain->GetDesc1(&SwapChainDesc);

	SwapChainResources.resize(SwapChainDesc.BufferCount);
	//for (auto It = SwapChainResources.begin(); It != SwapChainResources.end(); ++It) {
	//	const auto Index = static_cast<UINT>(std::distance(SwapChainResources.begin(), It));
	//	//!< スワップチェインのバッファリソースを SwapChainResources へ取得
	//	VERIFY_SUCCEEDED(SwapChain->GetBuffer(Index, IID_PPV_ARGS(It->GetAddressOf())));

	//	//!< デスクリプタ(ビュー)の作成。リソース上でのオフセットを指定して作成している、結果が変数に返るわけではない
	//	//!< (リソースがタイプドフォーマットなら D3D12_RENDER_TARGET_VIEW_DESC* へ nullptr 指定可能)
	//	Device->CreateRenderTargetView(It->Get(), nullptr, GetCPUDescriptorHandle(SwapChainDescriptorHeap.Get(), D3D12_DESCRIPTOR_HEAP_TYPE_RTV, Index));
	//}
	for (auto i = 0; i < SwapChainResources.size(); ++i) {
		//!< スワップチェインのバッファリソースを SwapChainResources へ取得
		VERIFY_SUCCEEDED(SwapChain->GetBuffer(i, IID_PPV_ARGS(SwapChainResources[i].GetAddressOf())));

		//!< デスクリプタ(ビュー)の作成。リソース上でのオフセットを指定して作成している、結果が変数に返るわけではない
		//!< (リソースがタイプドフォーマットなら D3D12_RENDER_TARGET_VIEW_DESC* へ nullptr 指定可能)
		const auto SCR = SwapChainResources[i].Get();
		const auto CDH = GetCPUDescriptorHandle(SwapChainDescriptorHeap.Get(), D3D12_DESCRIPTOR_HEAP_TYPE_RTV, i);
		Device->CreateRenderTargetView(SCR, nullptr, CDH);
	}

#ifdef DEBUG_STDOUT
	std::cout << "CreateSwapChainResource" << COUT_OK << std::endl << std::endl;
#endif
}

/**
@note Vulkanと違って、スワップチェインイメージ毎に、別のコマンドリストを使用しないとダメっぽい
*/
void DX::InitializeSwapchainImage(ID3D12CommandAllocator* CommandAllocator, const DirectX::XMVECTORF32* Color)
{
	for (auto i = 0; i < SwapChainResources.size(); ++i) {
		const auto CL = GraphicsCommandLists[i].Get();

		VERIFY_SUCCEEDED(CL->Reset(CommandAllocator, nullptr));
		{
			const auto SCR = SwapChainResources[i].Get();
			const auto CDH = GetCPUDescriptorHandle(SwapChainDescriptorHeap.Get(), D3D12_DESCRIPTOR_HEAP_TYPE_RTV, i);

			ResourceBarrier(CL, SCR, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET); {
				if (nullptr != Color) {
					CL->ClearRenderTargetView(CDH, *Color, 0, nullptr);
				}
			} ResourceBarrier(CL, SCR, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
		}
		VERIFY_SUCCEEDED(CL->Close());
	}

	//!< #DX_TODO : 0 番目しかクリアしていない
#if 1
	const std::vector<ID3D12CommandList*> CommandLists = { GraphicsCommandLists[0].Get() };
#else
	std::vector<ID3D12CommandList*> CommandLists;
	CommandLists.reserve(GraphicsCommandLists.size());
	for (auto i : GraphicsCommandLists) { CommandLists.push_back(i.Get()); }
#endif
	CommandQueue->ExecuteCommandLists(static_cast<UINT>(CommandLists.size()), CommandLists.data());

	WaitForFence();

#ifdef DEBUG_STDOUT
	std::cout << "InitializeSwapchainImage" << COUT_OK << std::endl << std::endl;
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

	CreateSwapChainResource();

#ifdef DEBUG_STDOUT
	std::cout << "ResizeSwapChain" << COUT_OK << std::endl << std::endl;
#endif
}

void DX::CreateDepthStencil(const UINT Width, const UINT Height, const DXGI_FORMAT DepthFormat)
{
	const auto Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	const auto Count = 1;
	[&](const D3D12_DESCRIPTOR_HEAP_TYPE Type, const UINT Count, ID3D12DescriptorHeap** DescriptorHeap) {
		const D3D12_DESCRIPTOR_HEAP_DESC DescriptorHeapDesc = {
			Type,
			Count,
			D3D12_DESCRIPTOR_HEAP_FLAG_NONE,
			0
		};
		VERIFY_SUCCEEDED(Device->CreateDescriptorHeap(&DescriptorHeapDesc, IID_PPV_ARGS(DescriptorHeap)));
	}(Type, Count, DepthStencilDescriptorHeap.GetAddressOf());

	CreateDepthStencilResource(Width, Height, DepthFormat);

#ifdef DEBUG_STDOUT
	std::cout << "CreateDepthStencil" << COUT_OK << std::endl << std::endl;
#endif
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

	const auto CDH = GetCPUDescriptorHandle(DepthStencilDescriptorHeap.Get(), D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
	//!< デスクリプタ(ビュー)の作成。リソース上でのオフセットを指定して作成している、結果が変数に返るわけではない
	//!< (リソースがタイプドフォーマットなら D3D12_DEPTH_STENCIL_VIEW_DESC* へ nullptr 指定可能)
	Device->CreateDepthStencilView(DepthStencilResource.Get(), nullptr, CDH);

#ifdef DEBUG_STDOUT
	std::cout << "\t" << "DepthStencilView" << std::endl;
#endif

	//!< リソースの状態を初期 → デプス書き込みへ変更
	auto CL = GraphicsCommandLists[0];
	ResourceBarrier(CL.Get(), DepthStencilResource.Get(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_DEPTH_WRITE);

#ifdef DEBUG_STDOUT
	std::cout << "CreateDepthStencilResource" << COUT_OK << std::endl << std::endl;
#endif
}
void DX::ResizeDepthStencil(const UINT Width, const UINT Height, const DXGI_FORMAT DepthFormat)
{
	DepthStencilResource.Reset();

	CreateDepthStencilResource(Width, Height, DepthFormat);

#ifdef DEBUG_STDOUT
	std::cout << "ResizeDepthStencil" << COUT_OK << std::endl << std::endl;
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

//void DX::CreateRootSignature(ID3D12RootSignature** RootSignature) const
//{
//	using namespace Microsoft::WRL;
//
//	const std::vector<D3D12_DESCRIPTOR_RANGE> DescriptorRanges = {
//	};
//	const D3D12_ROOT_DESCRIPTOR_TABLE DescriptorTable = {
//		static_cast<UINT>(DescriptorRanges.size()), DescriptorRanges.data()
//	};
//	const std::vector<D3D12_ROOT_PARAMETER> RootParameters = {
//	};
//	const std::vector<D3D12_STATIC_SAMPLER_DESC> StaticSamplerDescs = {
//	};
//	const D3D12_ROOT_SIGNATURE_DESC RootSignatureDesc = {
//		static_cast<UINT>(RootParameters.size()), RootParameters.data(),
//		static_cast<UINT>(StaticSamplerDescs.size()), StaticSamplerDescs.data(),
//		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT
//	};
//	ComPtr<ID3DBlob> Blob;
//	ComPtr<ID3DBlob> ErrorBlob;
//	VERIFY_SUCCEEDED(D3D12SerializeRootSignature(&RootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, Blob.GetAddressOf(), ErrorBlob.GetAddressOf()));
//	VERIFY_SUCCEEDED(Device->CreateRootSignature(0, Blob->GetBufferPointer(), Blob->GetBufferSize(), IID_PPV_ARGS(RootSignature.GetAddressOf())));
//}

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

	//!< デスクリプタ(ビュー)の作成。リソース上でのオフセットを指定して作成している、結果が変数に返るわけではない
	UINT Index = 0;
	{
		/*const*/D3D12_SHADER_RESOURCE_VIEW_DESC SRVDesc = {
			Format,
			D3D12_SRV_DIMENSION_TEXTURE2D,
			D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING,
		};
		SRVDesc.Texture2D = {
			0, 1, 0, 0.0f
		};
		const auto CDH = GetCPUDescriptorHandle(UnorderedAccessTextureDescriptorHeap.Get(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, Index++);
		Device->CreateShaderResourceView(UnorderedAccessTextureResource.Get(), &SRVDesc, CDH);
	}

	{
		/*const*/D3D12_UNORDERED_ACCESS_VIEW_DESC UAVDesc = {
			Format,
			D3D12_UAV_DIMENSION_TEXTURE2D,
		};
		UAVDesc.Texture2D = {
			0, 0
		};
		//!< デスクリプタ(ビュー)の作成。リソース上でのオフセットを指定して作成している、結果が変数に返るわけではない
		const auto CDH = GetCPUDescriptorHandle(UnorderedAccessTextureDescriptorHeap.Get(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, Index++);
		Device->CreateUnorderedAccessView(UnorderedAccessTextureResource.Get(), nullptr, &UAVDesc, CDH);
	}

#ifdef DEBUG_STDOUT
	std::cout << "CreateUnorderedAccessBuffer" << COUT_OK << std::endl << std::endl;
#endif
}

/**
@brief シェーダとのバインディング (VK::CreateDescriptorSetLayout() 相当)
*/
void DX::CreateRootSignature()
{
	std::vector<D3D12_DESCRIPTOR_RANGE> DescriptorRanges = {
		/**
		D3D12_DESCRIPTOR_RANGE_TYPE RangeType; ... D3D12_DESCRIPTOR_RANGE_TYPE_[SRV, UAV, CBV, SAMPLER]
		UINT NumDescriptors;
		UINT BaseShaderRegister; ... register(b0) なら 0、register(t3) なら 3
		UINT RegisterSpace; ... 通常は 0 でよい register(t3, space5) なら 5
		UINT OffsetInDescriptorsFromTableStart; ... 通常は D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND でよい
		*/
	};
	CreateDescriptorRanges(DescriptorRanges);

	std::vector<D3D12_ROOT_PARAMETER> RootParameters = {
		/**
		D3D12_ROOT_PARAMETER_TYPE ParameterType; ... D3D12_ROOT_PARAMETER_TYPE_[DESCRIPTOR_TABLE, 32BIT_CONSTANTS, CBV, SRV, UAV]
		union
		{
			D3D12_ROOT_DESCRIPTOR_TABLE DescriptorTable
			{
				UINT NumDescriptorRanges;
				const D3D12_DESCRIPTOR_RANGE *pDescriptorRanges;
			};
			D3D12_ROOT_CONSTANTS Constants
			{
				UINT ShaderRegister;
				UINT RegisterSpace;
				UINT Num32BitValues;
			};
			D3D12_ROOT_DESCRIPTOR Descriptor
			{
				UINT ShaderRegister;
				UINT RegisterSpace;
			};
		};
		D3D12_SHADER_VISIBILITY ShaderVisibility; ... D3D12_SHADER_VISIBILITY_[ALL, VERTEX, HULL, DOMAIN, GEOMETRY, PIXEL]
		*/
	};
	CreateRootParameters(RootParameters, DescriptorRanges);

	const D3D12_ROOT_SIGNATURE_DESC RootSignatureDesc = {
		static_cast<UINT>(RootParameters.size()), RootParameters.data(),
		static_cast<UINT>(StaticSamplerDescs.size()), StaticSamplerDescs.data(),
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT
	};
	Microsoft::WRL::ComPtr<ID3DBlob> Blob;
	Microsoft::WRL::ComPtr<ID3DBlob> ErrorBlob;
	VERIFY_SUCCEEDED(D3D12SerializeRootSignature(&RootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, Blob.GetAddressOf(), ErrorBlob.GetAddressOf()));
	VERIFY_SUCCEEDED(Device->CreateRootSignature(0, Blob->GetBufferPointer(), Blob->GetBufferSize(), IID_PPV_ARGS(RootSignature.GetAddressOf())));

#ifdef DEBUG_STDOUT
	std::cout << "CreateRootSignature" << COUT_OK << std::endl << std::endl;
#endif
}

BOOL DX::LoadPipelineLibrary(const std::wstring& Path)
{
	Microsoft::WRL::ComPtr<ID3D12Device1> Device1;
	VERIFY_SUCCEEDED(Device->QueryInterface(IID_PPV_ARGS(Device1.GetAddressOf())));

	Microsoft::WRL::ComPtr<ID3DBlob> Blob;
	if (SUCCEEDED(D3DReadFileToBlob(Path.data(), Blob.GetAddressOf())) && Blob->GetBufferSize()) {
		VERIFY_SUCCEEDED(Device1->CreatePipelineLibrary(Blob->GetBufferPointer(), Blob->GetBufferSize(), IID_PPV_ARGS(PipelineLibrary.GetAddressOf())));
		return TRUE;
	}
	else {
		VERIFY_SUCCEEDED(Device1->CreatePipelineLibrary(nullptr, 0, IID_PPV_ARGS(PipelineLibrary.GetAddressOf())));
		return FALSE;
	}
}
void DX::StorePipelineLibrary(const std::wstring& Path) const
{
	const auto Size = PipelineLibrary->GetSerializedSize();
	if (Size) {
		Microsoft::WRL::ComPtr<ID3DBlob> Blob;
		VERIFY_SUCCEEDED(D3DCreateBlob(Size, Blob.GetAddressOf()));
		PipelineLibrary->Serialize(Blob->GetBufferPointer(), Size);
		VERIFY_SUCCEEDED(D3DWriteBlobToFile(Blob.Get(), Path.data(), TRUE));
	}
}

void DX::CreatePipelineState()
{
	CreatePipelineState_Graphics();
}
void DX::CreatePipelineState_Graphics()
{
#ifdef _DEBUG
	PerformanceCounter PC("CreatePipelineState_Graphics : ");
#endif

	assert(nullptr != RootSignature);
#if 0
	Microsoft::WRL::ComPtr<ID3D12RootSignature> RootSignature;
	CreateRootSignature(RootSignature.GetAddressOf());
#endif

	//!< シェーダ
	std::vector<Microsoft::WRL::ComPtr<ID3DBlob>> ShaderBlobs;
	std::vector<D3D12_SHADER_BYTECODE> ShaderBytecodes;
	CreateShader(ShaderBlobs, ShaderBytecodes);

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
		D3D12_CULL_MODE_BACK, TRUE/*CCW*/,
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

	//!< インプットレイアウト
	std::vector<D3D12_INPUT_ELEMENT_DESC> InputElementDescs;
	CreateInputLayout(InputElementDescs);
	const D3D12_INPUT_LAYOUT_DESC InputLayoutDesc = {
		InputElementDescs.data(), static_cast<UINT>(InputElementDescs.size())
	};

	const DXGI_SAMPLE_DESC SampleDesc = { 1, 0 };
	const D3D12_CACHED_PIPELINE_STATE CachedPipelineState = { nullptr, 0 };
	const D3D12_GRAPHICS_PIPELINE_STATE_DESC GraphicsPipelineStateDesc = {
		RootSignature.Get(),
		ShaderBytecodes[0], ShaderBytecodes[1], ShaderBytecodes[2], ShaderBytecodes[3], ShaderBytecodes[4],
		StreamOutputDesc,
		BlendDesc,
		UINT_MAX,
		RasterizerDesc,
		DepthStencilDesc,
		InputLayoutDesc,
		D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED,
		GetPrimitiveTopologyType(),
		1, { DXGI_FORMAT_R8G8B8A8_UNORM/*, ... x8*/ },
		DXGI_FORMAT_D32_FLOAT_S8X24_UINT,
		SampleDesc,
		0,
		CachedPipelineState,
		D3D12_PIPELINE_STATE_FLAG_NONE //!< D3D12_PIPELINE_STATE_FLAG_TOOL_DEBUG ... は Warp デバイスのみ
	};

	//!< パイプラインキャッシュオブジェクト Pipeline Cache Object
	const auto PCOPath = GetBasePath() + TEXT(".pco"); //!< シリアライズファイルパス Searialize file path
	const auto PCOName = GetTitleW() + TEXT(".pco"); //!< ライブラリデータベースへ登録する名前 Name to store in library
	DeleteFile(PCOPath.data());
	if(LoadPipelineLibrary(PCOPath)) {
		//!< ライブラリからパイプラインステートを取得する (重複を防ぐためにライブラリデータベースとDescが比較される)
		VERIFY_SUCCEEDED(PipelineLibrary->LoadGraphicsPipeline(PCOName.data(), &GraphicsPipelineStateDesc, IID_PPV_ARGS(PipelineState.GetAddressOf())));
	}
	else {
		VERIFY_SUCCEEDED(Device->CreateGraphicsPipelineState(&GraphicsPipelineStateDesc, IID_PPV_ARGS(PipelineState.GetAddressOf())));
		//!< 名前指定でライブラリデータベースへ追加する
		VERIFY_SUCCEEDED(PipelineLibrary->StorePipeline(PCOName.data(), PipelineState.Get()));

		StorePipelineLibrary(PCOPath);
	}

#ifdef _DEBUG
	std::cout << "CreatePipelineState_Graphics" << COUT_OK << std::endl << std::endl;
#endif
}

void DX::CreatePipelineState_Compute()
{
#ifdef _DEBUG
	PerformanceCounter PC("CreatePipelineState_Compute : ");
#endif

	assert(nullptr != RootSignature);

	//!< シェーダ
	std::vector<Microsoft::WRL::ComPtr<ID3DBlob>> ShaderBlobs;
	std::vector<D3D12_SHADER_BYTECODE> ShaderBytecodes;
	CreateShader(ShaderBlobs, ShaderBytecodes);

	const D3D12_CACHED_PIPELINE_STATE CachedPipelineState = { nullptr, 0 };
	const D3D12_COMPUTE_PIPELINE_STATE_DESC ComputePipelineStateDesc = {
		RootSignature.Get(),
		ShaderBytecodes[0],
		0, // NodeMask ... マルチGPUの場合
		CachedPipelineState,
		D3D12_PIPELINE_STATE_FLAG_NONE
	};
	VERIFY_SUCCEEDED(Device->CreateComputePipelineState(&ComputePipelineStateDesc, IID_PPV_ARGS(PipelineState.GetAddressOf())));

#ifdef DEBUG_STDOUT
	std::cout << "CreatePipelineState_Compute" << COUT_OK << std::endl << std::endl;
#endif
}

void DX::ClearColor(ID3D12GraphicsCommandList* CommandList, const D3D12_CPU_DESCRIPTOR_HANDLE& DescriptorHandle, const DirectX::XMVECTORF32& Color)
{
	CommandList->ClearRenderTargetView(DescriptorHandle, Color, 0, nullptr);
}

void DX::ClearDepthStencil(ID3D12GraphicsCommandList* CommandList, const D3D12_CPU_DESCRIPTOR_HANDLE& DescriptorHandle, const FLOAT Depth, const UINT8 Stencil)
{
	CommandList->ClearDepthStencilView(DescriptorHandle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, Depth, Stencil, 0, nullptr);
}

void DX::PopulateCommandList(const size_t i)
{
	const auto CL = GraphicsCommandLists[i].Get();
	const auto SCR = SwapChainResources[i].Get();
	const auto SCHandle = GetCPUDescriptorHandle(SwapChainDescriptorHeap.Get(), D3D12_DESCRIPTOR_HEAP_TYPE_RTV, static_cast<UINT>(i));

	const auto CA = CommandAllocators[0].Get();
	//!< GPU が参照している間は、コマンドアロケータの Reset() はできない
	//VERIFY_SUCCEEDED(CA->Reset());

	//!< CommandQueue->ExecuteCommandLists() 後に CommandList->Reset() でリセットして再利用が可能 (コマンドキューはコマンドリストではなく、コマンドアロケータを参照している)
	//!< CommandList 作成時に PipelineState を指定していなくても、ここで指定すれば OK
	VERIFY_SUCCEEDED(CL->Reset(CA, PipelineState.Get()));
	{
		//!< ビューポート、シザー
		CL->RSSetViewports(static_cast<UINT>(Viewports.size()), Viewports.data());
		CL->RSSetScissorRects(static_cast<UINT>(ScissorRects.size()), ScissorRects.data());

		//!< バリア
		ResourceBarrier(CL, SCR, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET); {
#if 1
			//!< クリア
			ClearColor(CL, SCHandle, DirectX::Colors::SkyBlue);
#endif
		} ResourceBarrier(CL, SCR, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
	}
	VERIFY_SUCCEEDED(CL->Close());
}

void DX::Draw()
{
	WaitForFence();

	CurrentBackBufferIndex = AcquireNextBackBufferIndex();

	const std::vector<ID3D12CommandList*> CommandLists = { 
		GraphicsCommandLists[CurrentBackBufferIndex].Get() 
	};
	CommandQueue->ExecuteCommandLists(static_cast<UINT>(CommandLists.size()), CommandLists.data());
	
	Present();
}
void DX::Dispatch()
{
	//!< #DX_TODO
}
void DX::Present()
{
	VERIFY_SUCCEEDED(SwapChain->Present(1, 0));
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
