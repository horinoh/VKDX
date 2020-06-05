#include "DX.h"

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "dxcompiler")
#pragma comment(lib, "dxgi.lib")

void DX::OnCreate(HWND hWnd, HINSTANCE hInstance, LPCWSTR Title)
{
	PERFORMANCE_COUNTER();

	Super::OnCreate(hWnd, hInstance, Title);

#ifdef USE_HDR
	const auto ColorFormat = DXGI_FORMAT_R16G16B16A16_FLOAT;
	//const auto ColorFormat = DXGI_FORMAT_R10G10B10A2_UNORM; //!< シェーダでガンマ補正が必要
#else
	const auto ColorFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
#endif

	//!< デバイス
	CreateDevice(hWnd);
	CheckMultiSample(ColorFormat);
	CreateCommandQueue();

	CreateFence();

	CreateSwapchain(hWnd, ColorFormat);

	CreateCommandAllocator();
	CreateCommandList();
	//InitializeSwapchainImage(COM_PTR_GET(CommandAllocators[0]), &DirectX::Colors::Red);

	CreateVertexBuffer();
	CreateIndexBuffer();
	CreateIndirectBuffer();

	//!< スタティックサンプラはこの時点(CreateRootSignature()より前)で必要
	CreateStaticSampler();

	//!< ルートシグネチャ (パイプライントレイアウト相当)
	CreateRootSignature();
	CreateShaderBlobs();
	//!< パイプライン
	CreatePipelineStates();

	CreateTexture();

	//!< コンスタントバッファ (ユニフォームバッファ相当)
	CreateConstantBuffer();

	//!< デスクリプタヒープ (デスクリプタプール相当)
	CreateDescriptorHeap();
	//!< デスクリプタビュー (デスクリプタセット更新相当) ... この時点でリソース、デスクリプタヒープ等が必要
	CreateDescriptorView();

	CreateSampler();

	SetTimer(hWnd, NULL, Elapse, nullptr);

	//!< ウインドウサイズ変更時に作り直すもの
	OnExitSizeMove(hWnd, hInstance);
}

/**
リサイズするのもの
	BackBuffer ... SwapChain->ResizeBuffers() を使用
作り直すもの
	DepthStencilBuffer, RenderTargetView, DepthStencilView
*/
void DX::OnExitSizeMove(HWND hWnd, HINSTANCE hInstance)
{
	PERFORMANCE_COUNTER();

	Super::OnExitSizeMove(hWnd, hInstance);

	//!< コマンドリストの完了を待つ
	WaitForFence();

	const auto W = GetClientRectWidth(), H = GetClientRectHeight();

	ResizeSwapChain(W, H);

	//const auto CommandList = GraphicsCommandLists[0].Get();
	//const auto CommandAllocator = CommandAllocators[0].Get();

	//VERIFY_SUCCEEDED(CommandList->Reset(CommandAllocator, nullptr));
	//{		
	//	ResizeSwapChain(Rect);
	//	ResizeDepthStencil(Rect);
	//}
	//VERIFY_SUCCEEDED(CommandList->Close());

	//ExecuteCommandListAndWaitForFence(CommandList);

	CreateViewport(static_cast<const FLOAT>(W), static_cast<const FLOAT>(H));

	//!< ビューポートサイズが決定してから
	LoadScene();
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
	const DXGI_SAMPLE_DESC SD = { 1, 0 };
	const D3D12_RESOURCE_DESC RD = {
		D3D12_RESOURCE_DIMENSION_BUFFER,
		0,
		Size, 1, //!< Widthに「バッファサイズ」を指定して、Heightは1にしておく
		1, 1,
		DXGI_FORMAT_UNKNOWN,
		SD,
		D3D12_TEXTURE_LAYOUT_ROW_MAJOR, //!< ROW_MAJORにすること
		D3D12_RESOURCE_FLAG_NONE
	};
	const D3D12_HEAP_PROPERTIES HP = {
		D3D12_HEAP_TYPE_UPLOAD, //!< UPLOAD にすること (Must be UPLOAD)
		D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
		D3D12_MEMORY_POOL_UNKNOWN,
		0,// CreationNodeMask ... マルチGPUの場合に使用(1つしか使わない場合は0で良い)
		0 // VisibleNodeMask ... マルチGPUの場合に使用(1つしか使わない場合は0で良い)
	};
	VERIFY_SUCCEEDED(Device->CreateCommittedResource(&HP,
		D3D12_HEAP_FLAG_NONE,
		&RD,
		D3D12_RESOURCE_STATE_GENERIC_READ, //!< GENERIC_READ にすること (Must be GENERIC_READ)
		nullptr,
		IID_PPV_ARGS(Resource)
	));
}
void DX::CopyToUploadResource(ID3D12Resource* Resource, const size_t Size, const void* Source, const D3D12_RANGE* Range)
{
	if (nullptr != Resource && Size && nullptr != Source) {
		BYTE* Data;
		VERIFY_SUCCEEDED(Resource->Map(0, Range, reinterpret_cast<void**>(&Data))); {
			memcpy(Data, Source, Size);
		} Resource->Unmap(0, nullptr);
	}
}
void DX::CopyToUploadResource(ID3D12Resource* Resource, const std::vector<D3D12_PLACED_SUBRESOURCE_FOOTPRINT>& PSF, const std::vector<UINT>& NumRows, const std::vector<UINT64>& RowSizes, const std::vector<D3D12_SUBRESOURCE_DATA>& SubresourceData)
{
	if (nullptr != Resource) {
		//assert(SubresourceData.size() == PSF.size() == NumRows.size() == RowSizes.size() && "Invalid size");
		const auto SubresourceCount = static_cast<const UINT>(SubresourceData.size());

		BYTE* Data;
		VERIFY_SUCCEEDED(Resource->Map(0, nullptr, reinterpret_cast<void**>(&Data))); {
			for (auto It = PSF.cbegin(); It != PSF.cend(); ++It) {
				const auto Index = std::distance(PSF.cbegin(), It);
				const auto& SD = SubresourceData[Index];
				const auto RowCount = NumRows[Index];
				const auto RowSize = RowSizes[Index];
				const D3D12_MEMCPY_DEST MemcpyDest = {
					Data + It->Offset,
					It->Footprint.RowPitch,
					static_cast<SIZE_T>(It->Footprint.RowPitch) * RowCount
				};
				for (UINT i = 0; i < It->Footprint.Depth; ++i) {
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

void DX::ExecuteCopyBuffer(ID3D12Resource* DstResource, ID3D12CommandAllocator* CA, ID3D12GraphicsCommandList* CL, const size_t Size, ID3D12Resource* SrcResource)
{
	VERIFY_SUCCEEDED(CL->Reset(CA, nullptr)); {
		PopulateCopyBufferCommand(CL, SrcResource, DstResource, Size, D3D12_RESOURCE_STATE_GENERIC_READ);
	} VERIFY_SUCCEEDED(CL->Close());

	const std::vector<ID3D12CommandList*> CLs = { CL };
	CommandQueue->ExecuteCommandLists(static_cast<UINT>(CLs.size()), CLs.data());
	WaitForFence();
}

void DX::ExecuteCopyTexture(ID3D12Resource* DstResource, ID3D12CommandAllocator* CA, ID3D12GraphicsCommandList* CL ,const std::vector<D3D12_PLACED_SUBRESOURCE_FOOTPRINT>& PSF, const D3D12_RESOURCE_STATES ResourceState, ID3D12Resource* SrcResource)
{
	VERIFY_SUCCEEDED(CL->Reset(CA, nullptr)); {
		if (D3D12_RESOURCE_DIMENSION_BUFFER == DstResource->GetDesc().Dimension) {
			PopulateCopyBufferCommand(CL, SrcResource, DstResource, PSF, ResourceState);
		}
		else {
			PopulateCopyTextureCommand(CL, SrcResource, DstResource, PSF, ResourceState);
		}
	} VERIFY_SUCCEEDED(CL->Close());

	const std::vector<ID3D12CommandList*> CLs = { CL };
	CommandQueue->ExecuteCommandLists(static_cast<UINT>(CLs.size()), CLs.data());
	WaitForFence();
}

void DX::CreateDefaultResource(ID3D12Resource** Resource, const size_t Size)
{
	const DXGI_SAMPLE_DESC SD = { 1, 0 };
	const D3D12_RESOURCE_DESC RD = {
		D3D12_RESOURCE_DIMENSION_BUFFER,
		0,
		Size, 1, //!< Widthに「バッファサイズ」を指定して、Heightは1にしておく
		1, 1,
		DXGI_FORMAT_UNKNOWN,
		SD,
		D3D12_TEXTURE_LAYOUT_ROW_MAJOR, //!< ROW_MAJORにすること
		D3D12_RESOURCE_FLAG_NONE
	};
	const D3D12_HEAP_PROPERTIES HP = {
		D3D12_HEAP_TYPE_DEFAULT, //!< DEFAULT にすること (Must be DEFAULT)
		D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
		D3D12_MEMORY_POOL_UNKNOWN,
		0,// CreationNodeMask ... マルチGPUの場合に使用(1つしか使わない場合は0で良い)
		0 // VisibleNodeMask ... マルチGPUの場合に使用(1つしか使わない場合は0で良い)
	};
	VERIFY_SUCCEEDED(Device->CreateCommittedResource(&HP,
		D3D12_HEAP_FLAG_NONE,
		&RD,
		D3D12_RESOURCE_STATE_COMMON, //!< COMMON にすること (Must be COMMON)
		nullptr,
		IID_PPV_ARGS(Resource)
	));
}
void DX::ResourceBarrier(ID3D12GraphicsCommandList* CL, ID3D12Resource* Resource, const D3D12_RESOURCE_STATES Before, const D3D12_RESOURCE_STATES After)
{
	const D3D12_RESOURCE_TRANSITION_BARRIER RTB = {
		Resource,
		D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES,
		Before,
		After
	};
	const std::array<D3D12_RESOURCE_BARRIER, 1> RBs = {
		{
			D3D12_RESOURCE_BARRIER_TYPE_TRANSITION,
			D3D12_RESOURCE_BARRIER_FLAG_NONE,
			RTB
		}
	};
	CL->ResourceBarrier(static_cast<UINT>(RBs.size()), RBs.data());
}
void DX::PopulateCopyTextureCommand(ID3D12GraphicsCommandList* CL, ID3D12Resource* Src, ID3D12Resource* Dst, const std::vector<D3D12_PLACED_SUBRESOURCE_FOOTPRINT>& PSF, const D3D12_RESOURCE_STATES RS)
{
	//!< (LoadDDSTextureFromFile()で作成される)Dstのステートは既にD3D12_RESOURCE_STATE_COPY_DESTで作成されている (Dst(created from LoadDDSTextureFromFile())'s state is already D3D12_RESOURCE_STATE_COPY_DEST)
	//ResourceBarrier(CommandList, Dst, D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST); {
	{
		for (auto It = PSF.cbegin(); It != PSF.cend(); ++It) {
			const D3D12_TEXTURE_COPY_LOCATION TCL_Dst = {
				Dst,
				D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX,
				static_cast<const UINT>(std::distance(PSF.cbegin(), It))
			};
			const D3D12_TEXTURE_COPY_LOCATION TCL_Src = {
				Src,
				D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT,
				*It
			};
			//const D3D12_BOX Box = {
			//	static_cast<UINT>(It->Offset),	//!< LEFT
			//	0,								//!< TOP
			//	0,								//!< FRONT
			//	static_cast<UINT>(It->Offset) + It->Footprint.Width, //!< RIGHT
			//	1,								//!< BOTTOM
			//	1								//!< BACK
			//};
			CL->CopyTextureRegion(&TCL_Dst, 0, 0, 0, &TCL_Src, nullptr);
		}
	} ResourceBarrier(CL, Dst, D3D12_RESOURCE_STATE_COPY_DEST, RS);
}
void DX::PopulateCopyBufferCommand(ID3D12GraphicsCommandList* CL, ID3D12Resource* Src, ID3D12Resource* Dst, const std::vector<D3D12_PLACED_SUBRESOURCE_FOOTPRINT>& PSF, const D3D12_RESOURCE_STATES RS)
{
	ResourceBarrier(CL, Dst, D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST); {
		for (auto It = PSF.cbegin(); It != PSF.cend(); ++It) {
			CL->CopyBufferRegion(Dst, 0, Src, It->Offset, It->Footprint.Width);
		}
	} ResourceBarrier(CL, Dst, D3D12_RESOURCE_STATE_COPY_DEST, RS);
}
void DX::PopulateCopyBufferCommand(ID3D12GraphicsCommandList* CL, ID3D12Resource* Src, ID3D12Resource* Dst, const UINT64 Size, const D3D12_RESOURCE_STATES RS)
{
	ResourceBarrier(CL, Dst, D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST); {
		CL->CopyBufferRegion(Dst, 0, Src, 0, Size);
	} ResourceBarrier(CL, Dst, D3D12_RESOURCE_STATE_COPY_DEST, RS);
}

void DX::CreateDevice(HWND /*hWnd*/)
{
#if defined(_DEBUG) || defined(USE_PIX)
	if (SUCCEEDED(DXGIGetDebugInterface1(0, COM_PTR_UUIDOF_PUTVOID(GraphicsAnalysis)))) {
		//!< グラフィックス診断は Alt + F5 で起動した場合のみ成功する (Enabled only if executed with Alt + F5)
		Log("Graphics Analysis is enabled\n");
		//!< GraphicsAnalysis->BeginCapture(), GraphicsAnalysis->EndCapture() でキャプチャ開始、終了する
	}
#endif

#ifdef _DEBUG
	COM_PTR<ID3D12Debug> Debug;
	VERIFY_SUCCEEDED(D3D12GetDebugInterface(COM_PTR_UUIDOF_PUTVOID(Debug)));
	Debug->EnableDebugLayer();

	//!< GPUバリデーション ... CPUバリデーションが効かなくなるので採用しない
#if 0 
	COM_PTR<ID3D12Debug1> Debug1;
	VERIFY_SUCCEEDED(Debug->QueryInterface(COM_PTR_UUIDOF_PUTVOID(Debug1)));
	Debug1->SetEnableGPUBasedValidation(true);
#endif
#endif

	//!< WARP アダプタを作成するのに IDXGIFactory4(のEnumWarpAdapter) が必要
#ifdef _DEBUG
	VERIFY_SUCCEEDED(CreateDXGIFactory2(DXGI_CREATE_FACTORY_DEBUG, COM_PTR_UUIDOF_PUTVOID(Factory)));
#else
	VERIFY_SUCCEEDED(CreateDXGIFactory1(COM_PTR_UUIDOF_PUTVOID(Factory)));
#endif

#ifdef _DEBUG
	//!< アダプター(GPU)の列挙
	//!<	-> 各々のアダプターに接続されているアウトプット(ディスプレイ)の列挙
	EnumAdapter(COM_PTR_GET(Factory));
#endif

#ifdef USE_WARP
	//!< WARP : Win7以下だと D3D_FEATURE_LEVEL_10_1 まで、Win8以上だと D3D_FEATURE_LEVEL_11_1 までサポート
	VERIFY_SUCCEEDED(Factory->EnumWarpAdapter(COM_PTR_UUIDOF_PUTVOID(Adapter)));
#else
	//!< アダプター(GPU)の選択 : (ここでは)最大メモリのアダプターを選択することにする
	UINT Index = UINT_MAX;
	SIZE_T VM = 0;
	for (UINT i = 0; DXGI_ERROR_NOT_FOUND != Factory->EnumAdapters(i, COM_PTR_PUT(Adapter)); ++i) {
		DXGI_ADAPTER_DESC AD;
		VERIFY_SUCCEEDED(Adapter->GetDesc(&AD));
		if (AD.DedicatedVideoMemory > VM) {
			VM = AD.DedicatedSystemMemory;
			Index = i;
		}
		COM_PTR_RESET(Adapter);
	}
	assert(UINT_MAX != Index);
	VERIFY_SUCCEEDED(Factory->EnumAdapters(Index, COM_PTR_PUT(Adapter)));
#endif
	Log("[ Selected Aadapter ]\n");
	LogAdapter(COM_PTR_GET(Adapter));

	//!< アウトプット(ディスプレイ)の選択 : (ここでは)全てのアダプター、アウトプットを列挙して、最初に見つかったアウトプットを選択することにする
	COM_PTR<IDXGIAdapter> Ad;
	for (UINT i = 0; DXGI_ERROR_NOT_FOUND != Factory->EnumAdapters(i, COM_PTR_PUT(Ad)); ++i) {
		for (UINT j = 0; DXGI_ERROR_NOT_FOUND != Ad->EnumOutputs(j, COM_PTR_PUT(Output)); ++j) {
			if (nullptr != Output) {
				break;
			}
			COM_PTR_RESET(Output);
		}
		COM_PTR_RESET(Ad);
		if (nullptr != Output) {
			break;
		}
	}
	Log("\t\t[ Selected Output ]\n");
	LogOutput(COM_PTR_GET(Output));

	//!< フォーマット指定で、選択したアウトプットのディスプレイモードを列挙
	GetDisplayModeList(COM_PTR_GET(Output), DXGI_FORMAT_R8G8B8A8_UNORM);

#if 0
	const std::vector<UUID> Experimental = { D3D12ExperimentalShaderModels, /*D3D12RaytracingPrototype*/ };
	VERIFY_SUCCEEDED(D3D12EnableExperimentalFeatures(static_cast<UINT>(Experimental.size()), Experimental.data(), nullptr, nullptr));
#endif

	//!< 高いフィーチャーレベル優先でデバイスを作成
	for (const auto i : FeatureLevels) {
		if (SUCCEEDED(D3D12CreateDevice(COM_PTR_GET(Adapter), i, COM_PTR_UUIDOF_PUTVOID(Device)))) {
			break;
		}
	}
	CheckFeatureLevel(COM_PTR_GET(Device));

#if 0
	COM_PTR<ID3D12InfoQueue> InfoQueue;
	COM_PTR_AS(Device, InfoQueue);
	//VERIFY_SUCCEEDED(InfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, TRUE));
	VERIFY_SUCCEEDED(InfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, TRUE));
	
	std::array<D3D12_MESSAGE_CATEGORY, 0> AMCs = {};
	std::array<D3D12_MESSAGE_SEVERITY, 0> AMSs = {};
	std::array<D3D12_MESSAGE_ID, 0> AMIs = {};
	std::array<D3D12_MESSAGE_CATEGORY, 0> DMCs = {};
	std::array<D3D12_MESSAGE_SEVERITY, 0> DMSs = { /*D3D12_MESSAGE_SEVERITY_INFO,*/ };
	std::array<D3D12_MESSAGE_ID, 0> DMIs = { /*D3D12_MESSAGE_ID_CLEARRENDERTARGETVIEW_MISMATCHINGCLEARVALUE,*/ };
	D3D12_INFO_QUEUE_FILTER IQF = {
		//!< Allow List
		{
			static_cast<UINT>(AMCs.size()), AMCs.data(),
			static_cast<UINT>(AMSs.size()), AMSs.data(),
			static_cast<UINT>(AMIs.size()), AMIs.data(),
		},
		//!< Deny List
		{
			static_cast<UINT>(DMCs.size()), DMCs.data(),
			static_cast<UINT>(DMSs.size()), DMSs.data(),
			static_cast<UINT>(DMIs.size()), DMIs.data(),
		},
	};
	VERIFY_SUCCEEDED(InfoQueue->PushStorageFilter(&IQF));
#endif

	LOG_OK();
}

//!< アダプタ(GPU)の列挙
void DX::LogAdapter(IDXGIAdapter* Ad)
{
	DXGI_ADAPTER_DESC AdapterDesc;
	VERIFY_SUCCEEDED(Ad->GetDesc(&AdapterDesc));
	Logf(TEXT("\t%s\n"), AdapterDesc.Description);
	Logf(TEXT("\t\tDedicatedVideoMemory = %lld\n"), AdapterDesc.DedicatedVideoMemory);
	Logf(TEXT("\t\tDedicatedSystemMemory = %lld\n"), AdapterDesc.DedicatedSystemMemory);
	Logf(TEXT("\t\tSharedSystemMemory = %lld\n"), AdapterDesc.SharedSystemMemory);
}
void DX::EnumAdapter(IDXGIFactory4* Fact)
{
	Log("[ Aadapters ]\n");
	COM_PTR<IDXGIAdapter> Ad;
	for (UINT i = 0; DXGI_ERROR_NOT_FOUND != Fact->EnumAdapters(i, COM_PTR_PUT(Ad)); ++i) {
		LogAdapter(COM_PTR_GET(Ad));
		EnumOutput(COM_PTR_GET(Ad));
		COM_PTR_RESET(Ad);
	}
}

//!< アダプター(GPU)に接続されている、アウトプット(ディスプレイ)の列挙
void DX::LogOutput(IDXGIOutput* Outp)
{
#ifdef USE_HDR
	COM_PTR<IDXGIOutput6> Output6;
	COM_PTR_AS(Outp, Output6);

	DXGI_OUTPUT_DESC1 OutputDesc;
	VERIFY_SUCCEEDED(Output6->GetDesc1(&OutputDesc));
	//!< Need to enable "Play HDR game and apps" in windows settings 
	assert(DXGI_COLOR_SPACE_RGB_FULL_G2084_NONE_P2020 == OutputDesc.ColorSpace && "HDR not supported");
#else
	DXGI_OUTPUT_DESC OutputDesc;
	VERIFY_SUCCEEDED(Outp->GetDesc(&OutputDesc));
#endif

	const auto Width = OutputDesc.DesktopCoordinates.right - OutputDesc.DesktopCoordinates.left;
	const auto Height = OutputDesc.DesktopCoordinates.bottom - OutputDesc.DesktopCoordinates.top;
	Logf(TEXT("\t\t\t%s\n"), OutputDesc.DeviceName);
	Logf(TEXT("\t\t\t%d x %d\n"), Width, Height);
	switch (OutputDesc.Rotation)
	{
	default: break;
	case DXGI_MODE_ROTATION_UNSPECIFIED: Log("\t\t\tROTATION_UNSPECIFIED\n"); break;
	case DXGI_MODE_ROTATION_IDENTITY: Log("\t\t\tROTATION_IDENTITY\n"); break;
	case DXGI_MODE_ROTATION_ROTATE90: Log("\t\t\tROTATE90\n"); break;
	case DXGI_MODE_ROTATION_ROTATE180: Log("\t\t\tROTATE180\n"); break;
	case DXGI_MODE_ROTATION_ROTATE270: Log("\t\t\tROTATE270\n"); break;
	}
}
void DX::EnumOutput(IDXGIAdapter* Ad)
{
	Log("\t\t[ Outputs ]\n");
	COM_PTR<IDXGIOutput> Outp;
	for (UINT i = 0; DXGI_ERROR_NOT_FOUND != Ad->EnumOutputs(i, COM_PTR_PUT(Outp)); ++i) {
		LogOutput(COM_PTR_GET(Outp));
		COM_PTR_RESET(Outp);
	}
}

//!< アウトプット(ディスプレイ)の描画モードの列挙
void DX::GetDisplayModeList(IDXGIOutput* Outp, const DXGI_FORMAT Format)
{
	UINT Count = 0;
	VERIFY_SUCCEEDED(Outp->GetDisplayModeList(Format, 0, &Count, nullptr));
	if (Count) {
		Logf("\t\t\t[ DisplayModes ] : %s\n", GetFormatString(Format).c_str());

		std::vector<DXGI_MODE_DESC> MDs(Count);
		VERIFY_SUCCEEDED(Outp->GetDisplayModeList(Format, 0, &Count, MDs.data()));
		for (const auto& i : MDs) {
			Logf("\t\t\t\t%d x %d @ %d, ", i.Width, i.Height, i.RefreshRate.Numerator / i.RefreshRate.Denominator);

#define SCANLINE_ORDERING_ENTRY(slo) case DXGI_MODE_SCANLINE_ORDER_##slo: Logf("SCANLINE_ORDER_%s, ", #slo); break;
			switch (i.ScanlineOrdering) {
			default: assert(0 && "Unknown ScanlineOrdering"); break;
				SCANLINE_ORDERING_ENTRY(UNSPECIFIED)
				SCANLINE_ORDERING_ENTRY(PROGRESSIVE)
				SCANLINE_ORDERING_ENTRY(UPPER_FIELD_FIRST)
				SCANLINE_ORDERING_ENTRY(LOWER_FIELD_FIRST)
			}
#undef SCANLINE_ORDERING_ENTRY

#define SCALING_ENTRY(s) case DXGI_MODE_SCALING_##s: Logf("SCALING_%s", #s); break;
			switch (i.Scaling) {
			default: assert(0 && "Unknown Scaling"); break;
				SCALING_ENTRY(UNSPECIFIED)
				SCALING_ENTRY(CENTERED)
				SCALING_ENTRY(STRETCHED)
			}
#undef SCALING_ENTRY

			Log("\n");
		}
	}
}

void DX::CheckFeatureLevel(ID3D12Device* Dev)
{
	D3D12_FEATURE_DATA_FEATURE_LEVELS DataFeatureLevels = {
		static_cast<UINT>(FeatureLevels.size()), FeatureLevels.data()
	};
	//!< NumFeatureLevels, pFeatureLevelsRequested は CheckFeatureSupport() への入力、MaxSupportedFeatureLevel には出力が返る
	VERIFY_SUCCEEDED(Dev->CheckFeatureSupport(D3D12_FEATURE_FEATURE_LEVELS, reinterpret_cast<void*>(&DataFeatureLevels), sizeof(DataFeatureLevels)));

	Log("MaxSupportedFeatureLevel\n");
#define D3D_FEATURE_LEVEL_ENTRY(fl) case D3D_FEATURE_LEVEL_##fl: Logf("\tD3D_FEATURE_LEVEL_%s\n", #fl); break;
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
}

void DX::CheckMultiSample(const DXGI_FORMAT Format)
{
	Logf("MultiSample for %s\n", GetFormatString(Format).c_str());

	SampleDescs.clear();
	for (UINT i = 1; i < D3D12_MAX_MULTISAMPLE_SAMPLE_COUNT; ++i) {
		//!< Format, SampleCount, Flags は CheckFeatureSupport() への入力、NumQualityLevels には CheckFeatureSupport() からの出力が返る
		D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS DataMultiSampleQaualityLevels = {
			Format,
			i,
			D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE,
			0
		};
		VERIFY_SUCCEEDED(Device->CheckFeatureSupport(D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS, reinterpret_cast<void*>(&DataMultiSampleQaualityLevels), sizeof(DataMultiSampleQaualityLevels)));
		//!< 0 == NumQualityLevels の場合はサポートされていないということ
		if (DataMultiSampleQaualityLevels.NumQualityLevels) {
			const DXGI_SAMPLE_DESC SampleDesc = {
				DataMultiSampleQaualityLevels.SampleCount,
				DataMultiSampleQaualityLevels.NumQualityLevels - 1
			}; 
			SampleDescs.push_back(SampleDesc);

			Logf("\tCount = %d, Quality = %d\n", SampleDesc.Count, SampleDesc.Quality);
		}
	}
}

//D3D12_CPU_DESCRIPTOR_HANDLE DX::GetCPUDescriptorHandle(ID3D12DescriptorHeap* DH, const UINT Index) const
//{
//	auto CDH(DH->GetCPUDescriptorHandleForHeapStart());
//	CDH.ptr += static_cast<SIZE_T>(Index) * Device->GetDescriptorHandleIncrementSize(DH->GetDesc().Type);
//	return CDH;
//}
//D3D12_GPU_DESCRIPTOR_HANDLE DX::GetGPUDescriptorHandle(ID3D12DescriptorHeap* DH, const UINT Index) const
//{
//	auto GDH(DH->GetGPUDescriptorHandleForHeapStart());
//	GDH.ptr += static_cast<SIZE_T>(Index) * Device->GetDescriptorHandleIncrementSize(DH->GetDesc().Type);
//	return GDH;
//}
/**
@brief マルチスレッドで「同じ」キューへサブミットできる
@note Vulkan ではマルチスレッドで「異なる」キューへのみサブミットできるので注意
*/
void DX::CreateCommandQueue()
{
	const D3D12_COMMAND_QUEUE_DESC CommandQueueDesc = {
		D3D12_COMMAND_LIST_TYPE_DIRECT, //!< D3D12_COMMAND_LIST_TYPE_COMPUTE, D3D12_COMMAND_LIST_TYPE_COPY (D3D12_COMMAND_LIST_TYPE_BUNDLEが直接キューイングされることは無いと思う)
		D3D12_COMMAND_QUEUE_PRIORITY_NORMAL,
		D3D12_COMMAND_QUEUE_FLAG_NONE,
		0 // NodeMask ... マルチGPUの場合に使用(1つしか使わない場合は0で良い)
	};
	VERIFY_SUCCEEDED(Device->CreateCommandQueue(&CommandQueueDesc, COM_PTR_UUIDOF_PUTVOID(CommandQueue)));

	LOG_OK();
}

/**
@brief CPU と GPU の同期用
*/
void DX::CreateFence()
{
	VERIFY_SUCCEEDED(Device->CreateFence(0, D3D12_FENCE_FLAG_NONE, COM_PTR_UUIDOF_PUTVOID(Fence)));

	LOG_OK();
}

//!< コマンド実行(CL->ExecuteCommandList())後、GPUがコマンドアロケータの参照を終えるまで、アロケータのリセット(CA->Reset())してはいけない、アロケータが覚えているのでコマンドのリセット(CL->Reset())はしても良い
//!< (ここでは)ダイレクト用1つ、バンドル用1つのコマンドアロケータ作成をデフォルト実装とする
void DX::CreateCommandAllocator()
{
	CommandAllocators.resize(1);
	VERIFY_SUCCEEDED(Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, COM_PTR_UUIDOF_PUTVOID(CommandAllocators[0])));

	BundleCommandAllocators.resize(1);
	VERIFY_SUCCEEDED(Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_BUNDLE, COM_PTR_UUIDOF_PUTVOID(BundleCommandAllocators[0])));

	LOG_OK();
}

//!< (ここでは)ダイレクト用1つ、バンドル用1つのコマンドリスト作成をデフォルト実装とする
void DX::CreateCommandList()
{
	DXGI_SWAP_CHAIN_DESC1 SCD;
	SwapChain->GetDesc1(&SCD);
	for (UINT i = 0; i < SCD.BufferCount; ++i) {
		GraphicsCommandLists.push_back(COM_PTR<ID3D12GraphicsCommandList>());
		//!< 描画コマンドを発行するコマンドリストにはパイプラインステートの指定が必要だが、後からでも指定(CL->Reset(CA, COM_PTR_GET(PS)))できるので、ここでは使用しない(nullptr)
		VERIFY_SUCCEEDED(Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, COM_PTR_GET(CommandAllocators[0]), nullptr, COM_PTR_UUIDOF_PUTVOID(GraphicsCommandLists.back())));
		VERIFY_SUCCEEDED(GraphicsCommandLists.back()->Close());

		BundleGraphicsCommandLists.push_back(COM_PTR<ID3D12GraphicsCommandList>());
		VERIFY_SUCCEEDED(Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_BUNDLE, COM_PTR_GET(BundleCommandAllocators[0]), nullptr, COM_PTR_UUIDOF_PUTVOID(BundleGraphicsCommandLists.back())));
		VERIFY_SUCCEEDED(BundleGraphicsCommandLists.back()->Close());
	}
}

void DX::CreateSwapchain(HWND hWnd, const DXGI_FORMAT ColorFormat)
{
	CreateSwapChain(hWnd, ColorFormat, GetClientRectWidth(), GetClientRectHeight());

	//!< ビューを作成 Create view
	CreateSwapChainResource();
}
void DX::CreateSwapChain(HWND hWnd, const DXGI_FORMAT ColorFormat, const UINT Width, const UINT Height)
{
	const UINT BufferCount = 3;

	//!< 最適なフルスクリーンのパフォーマンスを得るには、IDXGIOutput->GetDisplayModeList() で取得する(ディスプレイのサポートする)DXGI_MODE_DESC でないとダメなので注意  #DX_TODO
	const DXGI_RATIONAL Rational = { 60, 1 };
	const DXGI_MODE_DESC ModeDesc = {
		Width, Height,
		Rational,
		ColorFormat,
		DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED,
		DXGI_MODE_SCALING_UNSPECIFIED
	};
	const auto& SampleDesc = SampleDescs[0];

	DXGI_SWAP_CHAIN_DESC SCD = {
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
	COM_PTR_RESET(SwapChain);
	COM_PTR<IDXGISwapChain> NewSwapChain;
	VERIFY_SUCCEEDED(Factory->CreateSwapChain(COM_PTR_GET(CommandQueue), &SCD, COM_PTR_PUT(NewSwapChain)));
	COM_PTR_AS(NewSwapChain, SwapChain);

	//!< 起動時にフルスクリーンにする場合
	//DXGI_SWAP_CHAIN_DESC1 SCD = {
	//	Width, Height,
	//	ColorFormat,
	//	FALSE,
	//	SampleDesc,
	//	DXGI_USAGE_BACK_BUFFER/*DXGI_USAGE_RENDER_TARGET_OUTPUT*/, 
	//	BufferCount,
	//	DXGI_SCALING_STRETCH/*DXGI_SCALING_NONE*/,
	//	DXGI_SWAP_EFFECT_FLIP_DISCARD,
	//	DXGI_ALPHA_MODE_UNSPECIFIED,
	//	DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH,
	//};
	//DXGI_SWAP_CHAIN_FULLSCREEN_DESC SCFD = {
	//	Rational,
	//	DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED,
	//	DXGI_MODE_SCALING_UNSPECIFIED,
	//	FALSE,
	//};
	//COM_PTR_RESET(SwapChain);
	//COM_PTR<IDXGISwapChain1> NewSwapChain;
	//VERIFY_SUCCEEDED(Factory->CreateSwapChainForHwnd(COM_PTR_GET(CommandQueue), hWnd, &SCD, &SCFD, COM_PTR_GET(Output), COM_PTR_PUT(NewSwapChain)));

	//!< DXGI によるフルスクリーン化(Alt + Enter)を抑制する場合
	//Factory->MakeWindowAssociation(hWnd, DXGI_MWA_NO_ALT_ENTER);

	//!< 任意のタイミングでフルスクリーン化する場合、以下をコールする
	//VERIFY_SUCCEEDED(SwapChain->SetFullscreenState(TRUE, nullptr));

#ifdef USE_HDR
	switch (ColorFormat)
	{
	default: 
		SwapChain->SetColorSpace1(DXGI_COLOR_SPACE_RGB_FULL_G22_NONE_P709);
		break;
	case DXGI_FORMAT_R16G16B16A16_FLOAT:
		SwapChain->SetColorSpace1(DXGI_COLOR_SPACE_RGB_FULL_G10_NONE_P709);
		break;
	case DXGI_FORMAT_R10G10B10A2_UNORM:
		SwapChain->SetColorSpace1(DXGI_COLOR_SPACE_RGB_FULL_G2084_NONE_P2020);
		break;
	}
#endif

#ifdef USE_HDR
	switch (ColorFormat)
	{
	default:
	{
		DXGI_HDR_METADATA_HDR10 Metadata = {
			{ static_cast<UINT16>(0.64f * 50000.0f), static_cast<UINT16>(0.33f * 50000.0f) },
			{ static_cast<UINT16>(0.3f * 50000.0f), static_cast<UINT16>(0.6f * 50000.0f) },
			{ static_cast<UINT16>(0.15f * 50000.0f), static_cast<UINT16>(0.06f * 50000.0f) },
			{ static_cast<UINT16>(0.3127f * 50000.0f), static_cast<UINT16>(0.329f * 50000.0f) },
			static_cast<UINT>(1000.0f * 10000.0f),
			static_cast<UINT>(0.001f * 10000.0f),
			static_cast<UINT16>(2000.0f),
			static_cast<UINT16>(500.0f)
		};
		SwapChain->SetHDRMetaData(DXGI_HDR_METADATA_TYPE_HDR10, sizeof(Metadata), &Metadata);
	}
	break;
	case DXGI_FORMAT_R16G16B16A16_FLOAT:
	{
		DXGI_HDR_METADATA_HDR10 Metadata = {
			{ static_cast<UINT16>(0.708f * 50000.0f), static_cast<UINT16>(0.292f * 50000.0f) },
			{ static_cast<UINT16>(0.17f * 50000.0f), static_cast<UINT16>(0.797f * 50000.0f) },
			{ static_cast<UINT16>(0.131f * 50000.0f), static_cast<UINT16>(0.046f * 50000.0f) },
			{ static_cast<UINT16>(0.3127f * 50000.0f), static_cast<UINT16>(0.329f * 50000.0f) },
			static_cast<UINT>(1000.0f * 10000.0f),
			static_cast<UINT>(0.001f * 10000.0f),
			static_cast<UINT16>(2000.0f),
			static_cast<UINT16>(500.0f)
		};
		SwapChain->SetHDRMetaData(DXGI_HDR_METADATA_TYPE_HDR10, sizeof(Metadata), &Metadata);
	}
	break;
	}
#endif

	const D3D12_DESCRIPTOR_HEAP_DESC DHD = {
		D3D12_DESCRIPTOR_HEAP_TYPE_RTV,
		SCD.BufferCount,
		D3D12_DESCRIPTOR_HEAP_FLAG_NONE,
		0 // NodeMask ... マルチGPUの場合に使用(1つしか使わない場合は0で良い)
	};
	VERIFY_SUCCEEDED(Device->CreateDescriptorHeap(&DHD, COM_PTR_UUIDOF_PUTVOID(SwapChainDescriptorHeap)));

	LOG_OK();
}
void DX::CreateSwapChainResource()
{
	DXGI_SWAP_CHAIN_DESC1 SwapChainDesc;
	SwapChain->GetDesc1(&SwapChainDesc);

#ifdef USE_GAMMA_CORRECTION
	D3D12_RENDER_TARGET_VIEW_DESC RTVD = {
		DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, //!< ガンマ補正あり
		D3D12_RTV_DIMENSION_TEXTURE2D,
	};
	RTVD.Texture2D.MipSlice = 0;
	RTVD.Texture2D.PlaneSlice = 0;
#endif
	auto CDH = SwapChainDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	SwapChainResources.resize(SwapChainDesc.BufferCount);
	for (auto i = 0; i < SwapChainResources.size(); ++i) {
		//!< スワップチェインのバッファリソースを SwapChainResources へ取得
		VERIFY_SUCCEEDED(SwapChain->GetBuffer(i, COM_PTR_UUIDOF_PUTVOID(SwapChainResources[i])));
		//!< デスクリプタ(ビュー)の作成。リソース上でのオフセットを指定して作成している、結果が変数等に返るわけではない
		//!< (リソースがタイプドフォーマットなら D3D12_RENDER_TARGET_VIEW_DESC* へ nullptr 指定可能)
#ifdef USE_GAMMA_CORRECTION
		Device->CreateRenderTargetView(COM_PTR_GET(SwapChainResources[i]), &RTVD, CDH);
#else
		Device->CreateRenderTargetView(COM_PTR_GET(SwapChainResources[i]), nullptr, CDH);
#endif
		CDH.ptr += Device->GetDescriptorHandleIncrementSize(SwapChainDescriptorHeap->GetDesc().Type);
	}

	LOG_OK();
}

void DX::InitializeSwapchainImage(ID3D12CommandAllocator* CommandAllocator, const DirectX::XMVECTORF32* Color)
{
	assert(nullptr != Color && "");
	auto CDH = SwapChainDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	for (auto i = 0; i < SwapChainResources.size(); ++i) {
		const auto CL = COM_PTR_GET(GraphicsCommandLists[i]);
		VERIFY_SUCCEEDED(CL->Reset(CommandAllocator, nullptr));
		{
			const auto SCR = COM_PTR_GET(SwapChainResources[i]);
			ResourceBarrier(CL, SCR, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET); {
				CL->ClearRenderTargetView(CDH, *Color, 0, nullptr);
			} ResourceBarrier(CL, SCR, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
		}
		VERIFY_SUCCEEDED(CL->Close());
		CDH.ptr += Device->GetDescriptorHandleIncrementSize(SwapChainDescriptorHeap->GetDesc().Type);
	}

	//!< #DX_TODO : 0 番目しかクリアしていない
	const std::vector<ID3D12CommandList*> CommandLists = { COM_PTR_GET(GraphicsCommandLists[0]) };
	CommandQueue->ExecuteCommandLists(static_cast<UINT>(CommandLists.size()), CommandLists.data());

	WaitForFence();

	LOG_OK();
}

void DX::ResizeSwapChain(const UINT Width, const UINT Height)
{
	ResetSwapChainResource();

	DXGI_SWAP_CHAIN_DESC1 SCD;
	SwapChain->GetDesc1(&SCD);
	VERIFY_SUCCEEDED(SwapChain->ResizeBuffers(SCD.BufferCount, Width, Height, SCD.Format, SCD.Flags));
	Log("\tResizeBuffers\n");

	CreateSwapChainResource();

	LOG_OK();
}

#if 0
void DX::CreateRenderTarget(const DXGI_FORMAT Format, const UINT Width, const UINT Height)
{
	ImageResources.resize(1);

	const D3D12_HEAP_PROPERTIES HP = {
		D3D12_HEAP_TYPE_DEFAULT, 
		D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
		D3D12_MEMORY_POOL_UNKNOWN,
		0,// CreationNodeMask ... マルチGPUの場合に使用(1つしか使わない場合は0で良い)
		0 // VisibleNodeMask ... マルチGPUの場合に使用(1つしか使わない場合は0で良い)
	};
	const DXGI_SAMPLE_DESC SD = { 1, 0 };
	const D3D12_RESOURCE_DESC RD = {
		D3D12_RESOURCE_DIMENSION_TEXTURE2D,
		0,
		Width, Height,
		1,
		1,
		Format,
		SD,
		D3D12_TEXTURE_LAYOUT_UNKNOWN,
		D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET
	};
	const D3D12_CLEAR_VALUE CV = { Format, { DirectX::Colors::SkyBlue.f[0], DirectX::Colors::SkyBlue.f[1], DirectX::Colors::SkyBlue.f[2], DirectX::Colors::SkyBlue.f[3] } };
	VERIFY_SUCCEEDED(Device->CreateCommittedResource(&HP, D3D12_HEAP_FLAG_NONE, &RD, D3D12_RESOURCE_STATE_RENDER_TARGET, &CV, COM_PTR_UUIDOF_PUTVOID(/*RenderTargetResource*/ImageResources[0])));

	LOG_OK();
}
#endif

void DX::ResizeDepthStencil(const DXGI_FORMAT /*DepthFormat*/, const UINT /*Width*/, const UINT /*Height*/)
{
	//COM_PTR_RESET(DepthStencilResource);
	//CreateDepthStencilResource(DepthFormat, Width, Height);

	LOG_OK();
}

void DX::CreateAndCopyToUploadResource(COM_PTR<ID3D12Resource>& Res, const size_t Size, const void* Source)
{
	CreateUploadResource(COM_PTR_PUT(Res), Size);
	CopyToUploadResource(COM_PTR_GET(Res), Size, Source);
}
void DX::CreateAndCopyToDefaultResource(COM_PTR<ID3D12Resource>& Res, ID3D12CommandAllocator* CA, ID3D12GraphicsCommandList* CL, const size_t Size, const void* Source)
{
	//!< アップロード用のリソースを作成 (Create resource for upload)
	COM_PTR<ID3D12Resource> UploadRes;
	CreateAndCopyToUploadResource(UploadRes, Size, Source);

	//!< デフォルトのリソースを作成 (Create default resource)
	CreateDefaultResource(COM_PTR_PUT(Res), Size);

	//!< アップロードリソースからデフォルトリソースへのコピーコマンドを発行 (Execute copy command upload resource to default resource)
	ExecuteCopyBuffer(COM_PTR_GET(Res), CA, CL, Size, COM_PTR_GET(UploadRes));
}

void DX::CreateViewport(const FLOAT Width, const FLOAT Height, const FLOAT MinDepth, const FLOAT MaxDepth)
{
	//!< DirectX、OpenGLはBLが原点(Vulkan はTLが原点)
	Viewports = {
		{ 0.0f, 0.0f, Width, Height, MinDepth, MaxDepth },
	};
	//!< left, top, right, bottomで指定 (offset, extentで指定のVKとは異なるので注意)
	ScissorRects = {
		{ 0, 0, static_cast<LONG>(Width), static_cast<LONG>(Height) },
	};
	LOG_OK();
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
		0,// CreationNodeMask ... マルチGPUの場合に使用(1つしか使わない場合は0で良い)
		0 // VisibleNodeMask ... マルチGPUの場合に使用(1つしか使わない場合は0で良い)
	};
	VERIFY_SUCCEEDED(Device->CreateCommittedResource(&HeapProperties, D3D12_HEAP_FLAG_NONE, &ResourceDesc, D3D12_RESOURCE_STATE_COMMON, nullptr, COM_PTR_UUIDOF_PUTVOID(UnorderedAccessTextureResource)));

	LOG_OK();
}

//!< ルートシグネチャをシリアライズしてブロブを作る
void DX::SerializeRootSignature(COM_PTR<ID3DBlob>& Blob, const std::initializer_list<D3D12_ROOT_PARAMETER> il_RPs, const std::initializer_list<D3D12_STATIC_SAMPLER_DESC> il_SSDs, const D3D12_ROOT_SIGNATURE_FLAGS Flags)
{
	//!< RangeType ... D3D12_DESCRIPTOR_RANGE_TYPE_[SRV, UAV, CBV, SAMPLER]
	//!< NumDescriptors
	//!< BaseShaderRegister ... register(b0)なら 0、register(t3) なら 3
	//!< RegisterSpace ... 通常は 0 でよい register(t3, space5) なら 5
	//!< OffsetInDescriptorsFromTableStart ... 通常は D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND でよい
	const std::vector<D3D12_ROOT_PARAMETER> RPs(il_RPs.begin(), il_RPs.end());
	const std::vector<D3D12_STATIC_SAMPLER_DESC> SSDs(il_SSDs.begin(), il_SSDs.end());
	const D3D12_ROOT_SIGNATURE_DESC RSD = {
			static_cast<UINT>(RPs.size()), RPs.data(),
			static_cast<UINT>(SSDs.size()), SSDs.data(),
			Flags
	};
	COM_PTR<ID3DBlob> ErrorBlob;
	VERIFY_SUCCEEDED(D3D12SerializeRootSignature(&RSD, D3D_ROOT_SIGNATURE_VERSION_1_0, COM_PTR_PUT(Blob), COM_PTR_PUT(ErrorBlob)));

	//!< #DX_TODO RootSignature1.1を使用する場合は、CheckFeatureSupport()で利用可能かチェックした上でD3D12_ROOT_SIGNATURE_DESC1を使用し、D3D_ROOT_SIGNATURE_VERSION_1_1指定で作成すること
#if 0
	D3D12_FEATURE_DATA_ROOT_SIGNATURE FDRS;
	VERIFY_SUCCEEDED(Device->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, reinterpret_cast<void*>(&FDRS), sizeof(FDRS))); //!< CheckFeatureSupport()でちゃんと取れない… #DX_TODO
	if (FDRS.HighestVersion >= D3D_ROOT_SIGNATURE_VERSION_1_1) {
		//!< D3D12_ROOT_SIGNATURE_DESC1 : D3D12_ROOT_PARAMETER が D3D12_ROOT_PARAMETER1 に変更されている
		//!<	D3D12_ROOT_PARAMETER1 : D3D12_ROOT_DESCRIPTOR が D3D12_ROOT_DESCRIPTOR1 に変更されている
		//!<		D3D12_ROOT_DESCRIPTOR1 : D3D12_ROOT_DESCRIPTOR_FLAGS Flags メンバが増えている
		const std::vector<D3D12_ROOT_PARAMETER> RP1s = {};
		const D3D12_ROOT_SIGNATURE_DESC1 RSD1 = {
			static_cast<UINT>(RP1s.size()), RP1s.data(),
			static_cast<UINT>(SSDs.size()), SSDs.data(),
			Flags
		};
		VERIFY_SUCCEEDED(D3D12SerializeRootSignature(&RSD1, D3D_ROOT_SIGNATURE_VERSION_1_1, COM_PTR_PUT(Blob), COM_PTR_PUT(ErrorBlob)));
	}
#endif

	LOG_OK();
}

//!< シェーダからルートシグネチャパートを取り出しブロブを作る
void DX::GetRootSignaturePartFromShader(COM_PTR<ID3DBlob>& Blob, LPCWSTR Path)
{
	COM_PTR<ID3DBlob> ShaderBlob;
	VERIFY_SUCCEEDED(D3DReadFileToBlob(Path, COM_PTR_PUT(ShaderBlob)));
	VERIFY_SUCCEEDED(D3DGetBlobPart(ShaderBlob->GetBufferPointer(), ShaderBlob->GetBufferSize(), D3D_BLOB_ROOT_SIGNATURE, 0, COM_PTR_PUT(Blob)));
	LOG_OK();
}

/**
@brief シェーダとのバインディング (VK::CreateDescriptorSetLayout() 相当)
*/
void DX::CreateRootSignature()
{
	COM_PTR<ID3DBlob> Blob;

#ifdef USE_HLSL_ROOTSIGNATRUE
	GetRootSignaturePartFromShader(Blob, (GetBasePath() + TEXT(".rs.cso")).data());
#else
	SerializeRootSignature(Blob, {}, {}, D3D12_ROOT_SIGNATURE_FLAG_NONE
		| D3D12_ROOT_SIGNATURE_FLAG_DENY_VERTEX_SHADER_ROOT_ACCESS
		| D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS
		| D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS
		| D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS
		| D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS);
#endif

	RootSignatures.resize(1);
	VERIFY_SUCCEEDED(Device->CreateRootSignature(0, //!< マルチGPUの場合に使用(1つしか使わない場合は0で良い)
		Blob->GetBufferPointer(), Blob->GetBufferSize(), COM_PTR_UUIDOF_PUTVOID(RootSignatures[0])));

	LOG_OK();
}

void DX::CreatePipelineState(COM_PTR<ID3D12PipelineState>& PST, ID3D12Device* Device, ID3D12RootSignature* RS,
	const D3D12_PRIMITIVE_TOPOLOGY_TYPE Topology,
	const D3D12_RASTERIZER_DESC& RD,
	const D3D12_DEPTH_STENCIL_DESC& DSD,
	const D3D12_SHADER_BYTECODE VS, const D3D12_SHADER_BYTECODE PS, const D3D12_SHADER_BYTECODE DS, const D3D12_SHADER_BYTECODE HS, const D3D12_SHADER_BYTECODE GS,
	const std::vector<D3D12_INPUT_ELEMENT_DESC>& IEDs,
	const std::vector<DXGI_FORMAT>& RtvFormats,
	const PipelineLibrarySerializer* PLS, LPCWSTR Name)
{
	PERFORMANCE_COUNTER();

	assert((VS.pShaderBytecode != nullptr && VS.BytecodeLength) && "");

	//!< ストリームアウトプット (StreamOutput)
	const D3D12_STREAM_OUTPUT_DESC SOD = {
		nullptr, 0,
		nullptr, 0,
		0
	};

	//!< ブレンド (Blend)
	//!< 例) 
	//!< ブレンド	: Src * A + Dst * (1 - A)	= Src:D3D12_BLEND_SRC_ALPHA, Dst:D3D12_BLEND_INV_SRC_ALPHA, Op:D3D12_BLEND_OP_ADD
	//!< 加算		: Src * 1 + Dst * 1			= Src:D3D12_BLEND_ONE, Dst:D3D12_BLEND_ONE, Op:D3D12_BLEND_OP_ADD
	//!< 乗算		: Src * 0 + Dst * Src		= Src:D3D12_BLEND_ZERO, Dst:D3D12_BLEND_SRC_COLOR, Op:D3D12_BLEND_OP_ADD
	const D3D12_RENDER_TARGET_BLEND_DESC RTBD = {
		FALSE, FALSE, //!< ブレンド有効かどうか、論理演算有効かどうか (同時にTRUEにはできない)
		D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD, //!< ブレンド Src(新規), Dst(既存), Op
		D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD, //!< アルファ Src(新規), Dst(既存), Op
		D3D12_LOGIC_OP_NOOP, //!< 論理演算
		D3D12_COLOR_WRITE_ENABLE_ALL, //!< 書き込み時のマスク値
	};
	const D3D12_BLEND_DESC BD = {
		TRUE, //!< マルチサンプルを考慮したアルファテスト(AlphaToCoverageEnable)、アルファが0の箇所には無駄に書き込まない
		FALSE, //!< マルチレンダーターゲットにそれぞれ別のブレンドステートを割り当てる(IndependentBlendEnable)
		{ RTBD, } //!< TRUE==IndependentBlendEnableの場合、レンダーターゲットの分だけ用意すること #DX_TODO
	};

	//!< インプットレイアウト (InputLayout)
	const D3D12_INPUT_LAYOUT_DESC ILD = {
		IEDs.data(), static_cast<UINT>(IEDs.size())
	};

	//!< サンプル (Sample)
	const DXGI_SAMPLE_DESC SD = { 1, 0 };

	//!< キャッシュドパイプラインステート (CachedPipelineState)
	//!< (VK の VkGraphicsPipelineCreateInfo.basePipelineHandle, basePipelineIndex 相当?)
#if 0
	COM_PTR<ID3DBlob> PipelineBlob;
	VERIFY_SUCCEEDED(BasePipelineState->GetCachedBlob(COM_PTR_PUT(PipelineBlob)));
	const D3D12_CACHED_PIPELINE_STATE CPS = { PipelineBlob->GetBufferPointer(), PipelineBlob->GetBufferSize() };
#else
	const D3D12_CACHED_PIPELINE_STATE CPS = { nullptr, 0 };
#endif

	//!< DXでは「パッチコントロールポイント」個数の指定はIASetPrimitiveTopology()の引数として「コマンドリスト作成時」に指定する、VKとは結構異なるので注意
	//!< CL->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_1_CONTROL_POINT_PATCHLIST);

	D3D12_GRAPHICS_PIPELINE_STATE_DESC GPSD = {
		RS,
		VS, PS, DS, HS, GS,
		SOD,
		BD,
		D3D12_DEFAULT_SAMPLE_MASK,
		RD,
		DSD,
		ILD,
		D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED,
		Topology,
		static_cast<UINT>(RtvFormats.size()), {}, DSD.DepthEnable ? DXGI_FORMAT_D24_UNORM_S8_UINT : DXGI_FORMAT_UNKNOWN,
		SD,
		0,
		CPS,
		D3D12_PIPELINE_STATE_FLAG_NONE //!< D3D12_PIPELINE_STATE_FLAG_TOOL_DEBUG は Warp デバイスのみ
	};
	assert(GPSD.NumRenderTargets <= _countof(GPSD.RTVFormats) && "");
	std::copy(RtvFormats.begin(), RtvFormats.end(), GPSD.RTVFormats);
	assert((0 == GPSD.DS.BytecodeLength || 0 == GPSD.HS.BytecodeLength || GPSD.PrimitiveTopologyType == D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH) && "");

	if (nullptr != PLS && PLS->IsLoadSucceeded()) {
		VERIFY_SUCCEEDED(PLS->GetPipelineLibrary()->LoadGraphicsPipeline(Name, &GPSD, COM_PTR_UUIDOF_PUTVOID(PST)));
	}
	else {
		VERIFY_SUCCEEDED(Device->CreateGraphicsPipelineState(&GPSD, COM_PTR_UUIDOF_PUTVOID(PST)));
		if (nullptr != PLS) {
			VERIFY_SUCCEEDED(PLS->GetPipelineLibrary()->StorePipeline(Name, COM_PTR_GET(PST)));
		}
	}
	
	LOG_OK();
}

#if 0
void DX::CreatePipelineState_Compute()
{
	PERFORMANCE_COUNTER();

	assert(nullptr != RootSignature && "");

	//!< シェーダ
	std::vector<COM_PTR<ID3DBlob>> ShaderBlobs;
	CreateShader(ShaderBlobs);
	assert(!ShaderBlobs.empty() && "");

	const D3D12_CACHED_PIPELINE_STATE CPS = { nullptr, 0 };
	const D3D12_COMPUTE_PIPELINE_STATE_DESC CPSD = {
		COM_PTR_GET(RootSignature),
		D3D12_SHADER_BYTECODE({ ShaderBlobs[0]->GetBufferPointer(), ShaderBlobs[0]->GetBufferSize() }),
		0, // NodeMask ... マルチGPUの場合に使用(1つしか使わない場合は0で良い)
		CPS,
		D3D12_PIPELINE_STATE_FLAG_NONE
	};
	VERIFY_SUCCEEDED(Device->CreateComputePipelineState(&CPSD, COM_PTR_UUIDOF_PUTVOID(PipelineState)));

	LOG_OK();
}
#endif


void DX::PopulateCommandList(const size_t i)
{
	//!< GPUが参照している間は、「コマンドアロケータ」のリセットはできない
	//VERIFY_SUCCEEDED(CA->Reset());

	//!< コマンド実行後に、「コマンドリスト」はリセットして再利用が可能 (GPUは「コマンドアロケータ」を参照している)
	//!< 「コマンドリスト」作成時に「パイプライン」を指定していなくても、Reset()の引数に「パイプライン」を指定すれば良い
	const auto CL = COM_PTR_GET(GraphicsCommandLists[i]);
	const auto CA = COM_PTR_GET(CommandAllocators[0]);
	VERIFY_SUCCEEDED(CL->Reset(CA, nullptr));
	{
		const auto SCR = COM_PTR_GET(SwapChainResources[i]);

		CL->RSSetViewports(static_cast<UINT>(Viewports.size()), Viewports.data());
		CL->RSSetScissorRects(static_cast<UINT>(ScissorRects.size()), ScissorRects.data());

		ResourceBarrier(CL, SCR, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET); {
			auto CDH = SwapChainDescriptorHeap->GetCPUDescriptorHandleForHeapStart(); CDH.ptr += i * Device->GetDescriptorHandleIncrementSize(SwapChainDescriptorHeap->GetDesc().Type);
			const std::array<D3D12_RECT, 0> Rs = {};
			CL->ClearRenderTargetView(CDH, DirectX::Colors::SkyBlue, static_cast<UINT>(Rs.size()), Rs.data());
		} ResourceBarrier(CL, SCR, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
	}
	VERIFY_SUCCEEDED(CL->Close());
}

void DX::Draw()
{
#ifdef _DEBUG
	//PerformanceCounter PC(__func__);
#endif

	WaitForFence();

	const std::vector<ID3D12CommandList*> CLs = { COM_PTR_GET(GraphicsCommandLists[SwapChain->GetCurrentBackBufferIndex()]) };
	CommandQueue->ExecuteCommandLists(static_cast<UINT>(CLs.size()), CLs.data());
	
	Present();
}
void DX::Dispatch()
{
	//!< #DX_TODO Dispatch実装
	DEBUG_BREAK();
}
void DX::Present()
{
	//!< 垂直同期を待つ : 1
	VERIFY_SUCCEEDED(SwapChain->Present(1, 0));
}
void DX::WaitForFence()
{
	//!< CPU 側のフェンス値をインクリメント
	++FenceValue;

	//!< コマンドキューに Fencevalue を引数に Signal を追加する (GPU が到達すれば GetCompletedValue() が FenceValue になり、CPUに追いついたことになる)
	VERIFY_SUCCEEDED(CommandQueue->Signal(COM_PTR_GET(Fence), FenceValue));
	if (Fence->GetCompletedValue() < FenceValue) {
		auto hEvent = CreateEventEx(nullptr, nullptr, 0, EVENT_ALL_ACCESS);
		if (nullptr != hEvent) {
			//!< GetCompletedValue() が FenceValue になったらイベントが発行される
			VERIFY_SUCCEEDED(Fence->SetEventOnCompletion(FenceValue, hEvent));

			//!< イベント発行まで待つ
			WaitForSingleObject(hEvent, INFINITE);
			CloseHandle(hEvent);
		}
	}
}
