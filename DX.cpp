#include "DX.h"

template<> const DirectX::XMFLOAT3 GetMin(const DirectX::XMFLOAT3& lhs, const DirectX::XMFLOAT3& rhs) { return DirectX::XMFLOAT3((std::min)(lhs.x, rhs.x), (std::min)(lhs.y, rhs.y), (std::min)(lhs.z, rhs.z)); }
template<> const DirectX::XMFLOAT3 GetMax(const DirectX::XMFLOAT3& lhs, const DirectX::XMFLOAT3& rhs) { return DirectX::XMFLOAT3((std::max)(lhs.x, rhs.x), (std::max)(lhs.y, rhs.y), (std::max)(lhs.z, rhs.z)); }
template<>
void AdjustScale(std::vector<DirectX::XMFLOAT3>& Vertices, const float Scale)
{
	auto Max = (std::numeric_limits<DirectX::XMFLOAT3>::min)();
	auto Min = (std::numeric_limits<DirectX::XMFLOAT3>::max)();
	for (const auto& i : Vertices) {
		Min = GetMin(Min, i);
		Max = GetMax(Max, i);
	}

	const auto Diff = DirectX::XMFLOAT3(Max.x - Min.x, Max.y - Min.y, Max.z - Min.z);
	const auto Bound = (std::max)((std::max)(Diff.x, Diff.y), Diff.z);
	const auto Coef = Scale / Bound;
	std::ranges::transform(Vertices, std::begin(Vertices), [&](const DirectX::XMFLOAT3& rhs) { return DirectX::XMFLOAT3(rhs.x * Coef, (rhs.y - Diff.y * 0.5f) * Coef, (rhs.z - Min.z) * Coef); });
}

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "dxgi.lib")

#ifdef USE_DXC
//!< DXC (dxcompiler.dll が無いと怒られる場合は C:\Program Files (x86)\Windows Kits\10\bin\10.0.18362.0\x64 とかに存在するので、環境変数 Path に通しておく必要がある)
#pragma comment(lib, "dxcompiler.lib")

//!< オンラインコンパイルしたシェーダを使用する場合は $(WindowsSdkDir)Redist\D3D\$(PlatformTarget)\dxil.dll へのパスを通しておく
//!<	ex) C:\Program Files (x86)\Windows Kits\10\Redist\D3D\x64
#endif

void DX::OnCreate(HWND hWnd, HINSTANCE hInstance, LPCWSTR Title)
{
	PERFORMANCE_COUNTER_FUNC();

	Super::OnCreate(hWnd, hInstance, Title);

#ifdef USE_HDR
	const auto ColorFormat = DXGI_FORMAT_R16G16B16A16_FLOAT;
	//const auto ColorFormat = DXGI_FORMAT_R10G10B10A2_UNORM; //!< シェーダでガンマ補正が必要
#else
	const auto ColorFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
#endif

	//!< デバイス
	CreateDevice(hWnd);
	CreateCommandQueue();

	CreateFence();

	CreateSwapchain(hWnd, ColorFormat);

	CreateCommandList();

	//!< ジオメトリ (バーテックスバッファ、インデックスバッファ、アクセラレーションストラクチャ等)
	CreateGeometry();

	//!< コンスタントバッファ (ユニフォームバッファ相当)
	CreateConstantBuffer();

	//!< テクスチャ
	CreateTexture();

	//!< スタティックサンプラはこの時点(CreateRootSignature()より前)で必要
	CreateStaticSampler();
	//!< ルートシグネチャ (パイプライントレイアウト相当)
	CreateRootSignature();

	//!< レンダーパス相当は存在しない

	//!< パイプライン
	CreatePipelineState();

	//!< フレームバッファ相当は存在しない

	//!< デスクリプタ
	CreateDescriptor();

	CreateShaderTable();

	CreateVideo();

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
	constexpr UINT_PTR IDT_TIMER1 = 1000; //!< 何でも良い
	KillTimer(hWnd, IDT_TIMER1); {
		OnPreDestroy();

		PERFORMANCE_COUNTER_FUNC();

		Super::OnExitSizeMove(hWnd, hInstance);

		//!< GPU の完了を待つ
		WaitForGPU();

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
		for (auto i = 0; i < std::size(DirectCommandLists); ++i) {
			PopulateBundleCommandList(i);
			PopulateCommandList(i);
		}
	} SetTimer(hWnd, IDT_TIMER1, DeltaMsec, nullptr);
}
void DX::OnPreDestroy()
{
	Super::OnPreDestroy();

	//!< フルスクリーンの場合は解除
	DXGI_SWAP_CHAIN_DESC1 SCD1;
	SwapChain.DxSwapChain->GetDesc1(&SCD1);
	if (SCD1.Flags & DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING) {
		VERIFY_SUCCEEDED(SwapChain.DxSwapChain->SetFullscreenState(FALSE, nullptr));
	}

	//!< GPUが完了するまでここで待機 (Wait GPU)
	WaitForGPU();
}

const char* DX::GetFormatChar(const DXGI_FORMAT Format)
{
#define DXGI_FORMAT_ENTRY(df) case DXGI_FORMAT_##df: return #df;
	switch (Format)
	{
	default: VERIFY(false); return "";
#include "DXFormat.h"
	}
#undef DXGI_FORMAT_CASE
}

void DX::CreateBufferResource(ID3D12Resource** Resource, ID3D12Device* Device, const size_t Size, const D3D12_RESOURCE_FLAGS RF, const D3D12_HEAP_TYPE HT, const D3D12_RESOURCE_STATES RS, const void* Source)
{
	const D3D12_RESOURCE_DESC RD = {
		.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER,
		.Alignment = 0,
		.Width = Size, .Height = 1, //!< Widthに「バッファサイズ」を指定して、Heightは1にしておく (Set buffer size to Width, set Height to 1)
		.DepthOrArraySize = 1, .MipLevels = 1,
		.Format = DXGI_FORMAT_UNKNOWN,
		.SampleDesc = DXGI_SAMPLE_DESC({.Count = 1, .Quality = 0 }),
		.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR, //!< Widthに「バッファサイズ」を指定しているので ROW_MAJOR
		.Flags = RF
	};
	const D3D12_HEAP_PROPERTIES HP = {
		.Type = HT,
		.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
		.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN,
		.CreationNodeMask = 0, .VisibleNodeMask = 0 //!< マルチGPUの場合に使用(1つしか使わない場合は0で良い)
	};
	VERIFY_SUCCEEDED(Device->CreateCommittedResource(&HP, D3D12_HEAP_FLAG_NONE, &RD, RS, nullptr, IID_PPV_ARGS(Resource)));

	if (nullptr != Source) {
		//const D3D12_RANGE Range = {.Begin = 0, .End = 0};
		BYTE * Data;
		VERIFY_SUCCEEDED((*Resource)->Map(0, /*&Range*/nullptr, reinterpret_cast<void**>(&Data))); {
			memcpy(Data, Source, Size);
		} (*Resource)->Unmap(0, nullptr);
	}
}
void DX::CreateBufferResource(ID3D12Resource** Resource, ID3D12Device* Device, const std::vector<D3D12_SUBRESOURCE_DATA>& SRDs, const std::vector<D3D12_PLACED_SUBRESOURCE_FOOTPRINT>& PSFs, const std::vector<UINT>& NumRows, const std::vector<UINT64>& RowSizeInBytes, const UINT64 TotalBytes)
{
	const D3D12_RESOURCE_DESC RD = {
		.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER,
		.Alignment = 0,
		.Width = TotalBytes, .Height = 1,
		.DepthOrArraySize = 1, .MipLevels = 1,
		.Format = DXGI_FORMAT_UNKNOWN,
		.SampleDesc = DXGI_SAMPLE_DESC({.Count = 1, .Quality = 0 }),
		.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR,
		.Flags = D3D12_RESOURCE_FLAG_NONE
	};
	constexpr D3D12_HEAP_PROPERTIES HP = {
		.Type = D3D12_HEAP_TYPE_UPLOAD,
		.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
		.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN,
		.CreationNodeMask = 0, .VisibleNodeMask = 0
	};
	VERIFY_SUCCEEDED(Device->CreateCommittedResource(&HP, D3D12_HEAP_FLAG_NONE, &RD, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(Resource)));

	BYTE* Data;
	VERIFY_SUCCEEDED((*Resource)->Map(0, nullptr, reinterpret_cast<void**>(&Data))); {
		for (auto i = 0; i < std::size(PSFs); ++i) {
			const auto NR = NumRows[i];
			const auto RSIB = RowSizeInBytes[i];
			const D3D12_MEMCPY_DEST MCD = {
				.pData = Data + PSFs[i].Offset,
				.RowPitch = PSFs[i].Footprint.RowPitch,
				.SlicePitch = static_cast<SIZE_T>(PSFs[i].Footprint.RowPitch) * NR
			};
			const auto& SRD = SRDs[i];
			for (UINT j = 0; j < PSFs[i].Footprint.Depth; ++j) {
				auto Dst = reinterpret_cast<BYTE*>(MCD.pData) + MCD.SlicePitch * j;
				const auto Src = reinterpret_cast<const BYTE*>(SRD.pData) + SRD.SlicePitch * j;
				for (UINT k = 0; k < NR; ++k) {
					memcpy(Dst + MCD.RowPitch * k, Src + SRD.RowPitch * k, RSIB);
				}
			}
		}
	} (*Resource)->Unmap(0, nullptr);
}
void DX::CreateTextureResource(ID3D12Resource** Resource, ID3D12Device* Device, const UINT64 Width, const UINT Height, const UINT16 DepthOrArraySize, const UINT16 MipLevels, const DXGI_FORMAT Format, const D3D12_RESOURCE_FLAGS RF, const D3D12_RESOURCE_STATES RS)
{
	constexpr D3D12_HEAP_PROPERTIES HP = {
		.Type = D3D12_HEAP_TYPE_DEFAULT,
		.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
		.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN,
		.CreationNodeMask = 0, .VisibleNodeMask = 0
	};
	const D3D12_RESOURCE_DESC RD = {
		.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D,
		.Alignment = 0,
		.Width = Width, .Height = Height,
		.DepthOrArraySize = DepthOrArraySize,
		.MipLevels = MipLevels,
		.Format = Format,
		.SampleDesc = DXGI_SAMPLE_DESC({.Count = 1, .Quality = 0 }),
		.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN,
		.Flags = RF
	};
	VERIFY(!(RD.Flags & (D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET | D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL)));
	VERIFY_SUCCEEDED(Device->CreateCommittedResource(&HP, D3D12_HEAP_FLAG_NONE, &RD, RS, nullptr, IID_PPV_ARGS(Resource)));
}
//!< レンダーテクスチャの場合は D3D12_CLEAR_VALUE を指定する部分が通常テクスチャと異なる
void DX::CreateRenderTextureResource(ID3D12Resource** Resource, ID3D12Device* Device, const UINT64 Width, const UINT Height, const UINT16 DepthOrArraySize, const UINT16 MipLevels, const D3D12_CLEAR_VALUE& CV, const D3D12_RESOURCE_FLAGS RF, const D3D12_RESOURCE_STATES RS)
{
	constexpr D3D12_HEAP_PROPERTIES HP = {
		.Type = D3D12_HEAP_TYPE_DEFAULT,
		.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
		.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN,
		.CreationNodeMask = 0, .VisibleNodeMask = 0
	};
	const D3D12_RESOURCE_DESC RD = {
		.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D,
		.Alignment = 0,
		.Width = Width, .Height = Height,
		.DepthOrArraySize = DepthOrArraySize,
		.MipLevels = MipLevels,
		.Format = CV.Format,
		.SampleDesc = DXGI_SAMPLE_DESC({.Count = 1, .Quality = 0 }),
		.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN,
		.Flags = RF
	};
	VERIFY(RD.Flags & (D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET | D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL));
	VERIFY_SUCCEEDED(Device->CreateCommittedResource(&HP, D3D12_HEAP_FLAG_NONE, &RD, RS, &CV, IID_PPV_ARGS(Resource)));
}

#ifdef USE_DXC
bool DX::CompileShader(LPCWSTR ShaderPath, LPCWSTR Target, ID3DBlob** Blob)
{
	const auto HDC = LoadLibrary(TEXT("dxcompiler.dll"));
	if (HDC) {
		const auto DxcCreateInst = reinterpret_cast<DxcCreateInstanceProc>(GetProcAddress(HDC, "DxcCreateInstance"));
		COM_PTR<IDxcLibrary> DL;
		VERIFY_SUCCEEDED(DxcCreateInst(CLSID_DxcLibrary, __uuidof(IDxcLibrary), reinterpret_cast<LPVOID*>(&DL)));
		COM_PTR<IDxcBlobEncoding> DBE;
		VERIFY_SUCCEEDED(DL->CreateBlobFromFile(ShaderPath, nullptr, COM_PTR_PUT(DBE)));
		COM_PTR<IDxcIncludeHandler> DIH;
		VERIFY_SUCCEEDED(DL->CreateIncludeHandler(COM_PTR_PUT(DIH)));
		COM_PTR<IDxcCompiler> DC;
		VERIFY_SUCCEEDED(DxcCreateInst(CLSID_DxcCompiler, __uuidof(IDxcCompiler), reinterpret_cast<LPVOID*>(&DC)));
		COM_PTR<IDxcOperationResult> DOR;
		VERIFY_SUCCEEDED(DC->Compile(COM_PTR_GET(DBE), nullptr, nullptr, Target, nullptr, 0, nullptr, 0, COM_PTR_GET(DIH), COM_PTR_PUT(DOR)));
		HRESULT HR; DOR->GetStatus(&HR); VERIFY_SUCCEEDED(HR);
		VERIFY_SUCCEEDED(DOR->GetResult(reinterpret_cast<IDxcBlob**>(Blob)));

		COM_PTR<IDxcBlobEncoding> DBEError;
		VERIFY_SUCCEEDED(DOR->GetErrorBuffer(COM_PTR_PUT(DBEError)));
		const auto Text = DBEError->GetBufferPointer();
		if (nullptr != Text) {
			//!< HasError
		}
		return true;
	}
	return false;
}
#endif

//void DX::PopulateCopyBufferRegionCommand(ID3D12GraphicsCommandList* GCL, ID3D12Resource* Src, ID3D12Resource* Dst, const std::vector<D3D12_PLACED_SUBRESOURCE_FOOTPRINT>& PSF, const D3D12_RESOURCE_STATES RS)
//{
//	{
//		const std::array RBs = {
//			D3D12_RESOURCE_BARRIER({
//				.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION,
//				.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE,
//				.Transition = D3D12_RESOURCE_TRANSITION_BARRIER({
//					.pResource = Dst,
//					.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES,
//					.StateBefore = D3D12_RESOURCE_STATE_GENERIC_READ, .StateAfter = D3D12_RESOURCE_STATE_COPY_DEST
//				})
//			})
//		};
//		GCL->ResourceBarrier(static_cast<UINT>(size(RBs)), data(RBs));
//	}
//	for (auto i : PSF) {
//		GCL->CopyBufferRegion(Dst, 0, Src, i.Offset, i.Footprint.Width);
//	}
//	{
//		const std::array RBs = {
//			D3D12_RESOURCE_BARRIER({
//				.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION,
//				.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE,
//				.Transition = D3D12_RESOURCE_TRANSITION_BARRIER({
//					.pResource = Dst,
//					.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES,
//					.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST, .StateAfter = RS
//				})
//			})
//		};
//		GCL->ResourceBarrier(static_cast<UINT>(size(RBs)), data(RBs));
//	}
//}
void DX::PopulateCopyTextureRegionCommand(ID3D12GraphicsCommandList* GCL, ID3D12Resource* Src, ID3D12Resource* Dst, const std::vector<D3D12_PLACED_SUBRESOURCE_FOOTPRINT>& PSFs, const D3D12_RESOURCE_STATES RS)
{
	//!< LoadDDSTextureFromFile() を使用すると D3D12_RESOURCE_STATE_COPY_DEST で作成されているのでバリアの必要は無い (Resource created by LoadDDSTextureFromFile()'s state is already D3D12_RESOURCE_STATE_COPY_DEST)
	
	for (UINT i = 0; i < std::size(PSFs); ++i) {
		const D3D12_TEXTURE_COPY_LOCATION TCL_Dst = {
			.pResource = Dst,
			.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX,
			.SubresourceIndex = i
		};
		const D3D12_TEXTURE_COPY_LOCATION TCL_Src = {
			.pResource = Src,
			.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT,
			.PlacedFootprint = PSFs[i]
		};
		//const D3D12_BOX Box = { .left = static_cast<UINT>(PSFs[i].Offset), .top = 0, .front = 0, .right = static_cast<UINT>(PSFs[i].Offset) + PSFs[i].Footprint.Width, .bottom = 1, .back = 1, };
		GCL->CopyTextureRegion(&TCL_Dst, 0, 0, 0, &TCL_Src, nullptr);
	}
	ResourceBarrier(GCL, Dst, D3D12_RESOURCE_STATE_COPY_DEST, RS);
}

void DX::ExecuteAndWait(ID3D12CommandQueue* CQ, ID3D12CommandList* CL, ID3D12Fence* Fence)
{
	const std::array CLs = { CL };
	CQ->ExecuteCommandLists(static_cast<UINT>(std::size(CLs)), std::data(CLs));

	WaitForFence(CQ, Fence);
}

void DX::CreateDevice([[maybe_unused]] HWND hWnd)
{
#ifdef USE_PIX
	if (SUCCEEDED(DXGIGetDebugInterface1(0, COM_PTR_UUIDOF_PUTVOID(GraphicsAnalysis)))) {
		//!< グラフィックス診断は Alt + F5 で起動した場合のみ成功する (Enabled only if executed with Alt + F5)
		Log("Graphics Analysis is enabled\n");
		//!< プログラムからキャプチャする場合は GraphicsAnalysis->BeginCapture(), GraphicsAnalysis->EndCapture() を使用する
	}
#endif

#ifdef _DEBUG
	COM_PTR<ID3D12Debug> Debug;
	VERIFY_SUCCEEDED(D3D12GetDebugInterface(COM_PTR_UUIDOF_PUTVOID(Debug)));
	Debug->EnableDebugLayer();

	//!< GPUバリデーション (GPU validation)
	COM_PTR<ID3D12Debug3> Debug3;
	COM_PTR_AS(Debug, Debug3)
	if (Debug3) {
		//Debug3->SetEnableGPUBasedValidation(TRUE); 
	}

	//!< CreateDXGIFactory2() では引数にフラグを取ることができる (CreateDXGIFactory2() can specify flag argument)
	VERIFY_SUCCEEDED(CreateDXGIFactory2(DXGI_CREATE_FACTORY_DEBUG, COM_PTR_UUIDOF_PUTVOID(Factory)));
#else
	//VERIFY_SUCCEEDED(CreateDXGIFactory1(COM_PTR_UUIDOF_PUTVOID(Factory)));
	VERIFY_SUCCEEDED(CreateDXGIFactory2(0, COM_PTR_UUIDOF_PUTVOID(Factory)));
#endif

#ifdef DEBUG_STDOUT
	//!< アダプター(GPU)の列挙 (Enumerate adapter(GPU))
	std::cout << COM_PTR_GET(Factory);
#endif

#ifdef USE_WARP
	//!< ソフトウエアラスタライザ (Software rasterizer)
	VERIFY_SUCCEEDED(Factory->EnumWarpAdapter(COM_PTR_UUIDOF_PUTVOID(Adapter)));
#else
	//!< アダプター (GPU) の列挙 (Enumerate adapters(GPU))
	std::vector<DXGI_ADAPTER_DESC1> ADs;
	for (UINT i = 0; DXGI_ERROR_NOT_FOUND != Factory->EnumAdapters1(i, COM_PTR_PUT(Adapter)); ++i) {
		VERIFY_SUCCEEDED(Adapter->GetDesc1(&ADs.emplace_back()));
		if (ADs.back().Flags & DXGI_ADAPTER_FLAG_SOFTWARE) {}
		COM_PTR_RESET(Adapter);
	}
	//!< アダプターの選択、ここでは最大メモリを選択することにする (Here, select max memory size adapter)
	const auto Index = static_cast<UINT>(std::distance(std::begin(ADs),
		std::ranges::max_element(ADs,
			[](const DXGI_ADAPTER_DESC1& lhs, const DXGI_ADAPTER_DESC1& rhs) {
				return lhs.DedicatedSystemMemory > rhs.DedicatedSystemMemory;
			})
	));
	VERIFY_SUCCEEDED(Factory->EnumAdapters1(Index, COM_PTR_PUT(Adapter)));
#endif
	VERIFY(nullptr != Adapter);
	Log("[ Selected Adapter ]\n");
#ifdef DEBUG_STDOUT
	std::cout << COM_PTR_GET(Adapter);
#endif

	//!< ここでは最初に見つかったアウトプット(Display)を選択することにする (Here, select first found output(Display))
	{
		COM_PTR<IDXGIAdapter1> DA;
		for (UINT i = 0; DXGI_ERROR_NOT_FOUND != Factory->EnumAdapters1(i, COM_PTR_PUT(DA)); ++i) {
			for (UINT j = 0; DXGI_ERROR_NOT_FOUND != DA->EnumOutputs(j, COM_PTR_PUT(Output)); ++j) {
				if (nullptr != Output) { break; }
				COM_PTR_RESET(Output);
			}
			COM_PTR_RESET(DA);
			if (nullptr != Output) { break; }
		}
		VERIFY(nullptr != Output);
	}
	Log("\t\t[ Selected Output ]\n");
#ifdef DEBUG_STDOUT
	std::cout << COM_PTR_GET(Output);
#endif

	//!< (フォーマット指定で)選択したアウトプットのディスプレイモードを列挙
	GetDisplayModeList(COM_PTR_GET(Output), DXGI_FORMAT_R8G8B8A8_UNORM);

#if false
	//!< 実験的機能を有効化する (Windows を Developer Mode にする必要がある)
	const std::array Experimental = { D3D12ExperimentalShaderModels, /*D3D12RaytracingPrototype*/ };
	if (FAILED(D3D12EnableExperimentalFeatures(static_cast<UINT>(size(Experimental)), data(Experimental), nullptr, nullptr))) {
		Warning("\tD3D12EnableExperimentalFeatures() failed\n");
	}
#endif

	//!< 高フィーチャーレベル優先でデバイスを作成 (Create device with higher feature level possible)
	{
		constexpr std::array FeatureLevels = {
			D3D_FEATURE_LEVEL_12_2,
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
		for (const auto i : FeatureLevels) {
			if (SUCCEEDED(D3D12CreateDevice(COM_PTR_GET(Adapter), i, COM_PTR_UUIDOF_PUTVOID(Device)))) {
				//!< NumFeatureLevels, pFeatureLevelsRequested は入力、MaxSupportedFeatureLevel は出力となる (NumFeatureLevels, pFeatureLevelsRequested is input, MaxSupportedFeatureLevel is output)
				D3D12_FEATURE_DATA_FEATURE_LEVELS FDFL = { .NumFeatureLevels = static_cast<UINT>(std::size(FeatureLevels)), .pFeatureLevelsRequested = std::data(FeatureLevels) };
				VERIFY_SUCCEEDED(Device->CheckFeatureSupport(D3D12_FEATURE_FEATURE_LEVELS, reinterpret_cast<void*>(&FDFL), sizeof(FDFL)));
				Log("MaxSupportedFeatureLevel\n");
#define D3D_FEATURE_LEVEL_ENTRY(fl) case D3D_FEATURE_LEVEL_##fl: Logf("\tD3D_FEATURE_LEVEL_%s\n", #fl); break;
				switch (FDFL.MaxSupportedFeatureLevel) {
				default: VERIFY(false); break;
					D3D_FEATURE_LEVEL_ENTRY(12_2)
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
				break;
			}
		}
	}

	//!< デバイス作成後
#ifdef _DEBUG
	{
		COM_PTR<ID3D12InfoQueue> IQ;
		VERIFY_SUCCEEDED(Device->QueryInterface(COM_PTR_UUIDOF_PUTVOID(IQ)));
		if (nullptr != IQ) {
			//!< エラー等でブレークする設定 (Break with error)
			VERIFY_SUCCEEDED(IQ->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, TRUE));
			VERIFY_SUCCEEDED(IQ->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, TRUE));

			//!< ワーニング等をフィルタリングしたい場合等に使用
			//!< 適切に設定しないと出力がうるさいのでここでは無効にしている
#if 0
			std::array<D3D12_MESSAGE_CATEGORY, 0> AllowMCs = {};
			std::array<D3D12_MESSAGE_SEVERITY, 0> AllowMSs = {};
			std::array<D3D12_MESSAGE_ID, 0> AllowMIs = {};
			std::array<D3D12_MESSAGE_CATEGORY, 0> DenyMCs = {};
			std::array<D3D12_MESSAGE_SEVERITY, 0> DenyMSs = { /*D3D12_MESSAGE_SEVERITY_INFO,*/ };
			std::array<D3D12_MESSAGE_ID, 0> DenyMIs = { /*D3D12_MESSAGE_ID_CLEARRENDERTARGETVIEW_MISMATCHINGCLEARVALUE, */ /*D3D12_MESSAGE_ID_INVALID_DESCRIPTOR_HANDLE,*/ };
			D3D12_INFO_QUEUE_FILTER IQF = {
				.AllowList = D3D12_INFO_QUEUE_FILTER_DESC({
					.NumCategories = static_cast<UINT>(size(AllowMCs)), .pCategoryList = data(AllowMCs),
					.NumSeverities = static_cast<UINT>(size(AllowMSs)), .pSeverityList = data(AllowMSs),
					.NumIDs = static_cast<UINT>(size(AllowMIs)), .pIDList = data(AllowMIs),
				}),
				.DenyList = D3D12_INFO_QUEUE_FILTER_DESC({
					.NumCategories = static_cast<UINT>(size(DenyMCs)), .pCategoryList = data(DenyMCs),
					.NumSeverities = static_cast<UINT>(size(DenyMSs)), .pSeverityList = data(DenyMSs),
					.NumIDs = static_cast<UINT>(size(DenyMIs)), .pIDList = data(DenyMIs),
				}),
			};
			VERIFY_SUCCEEDED(InfoQueue->PushStorageFilter(&IQF));
#endif
		}
	}
#endif

	LOG_OK();
}

void DX::GetDisplayModeList([[maybe_unused]] IDXGIOutput* Outp, [[maybe_unused]] const DXGI_FORMAT Format)
{
#ifdef DEBUG_STDOUT
	UINT Count = 0;
	VERIFY_SUCCEEDED(Outp->GetDisplayModeList(Format, 0, &Count, nullptr));
	if (Count) [[likely]] {
		Logf("\t\t\t[ DisplayModes ] : %s\n", GetFormatChar(Format));
		std::vector<DXGI_MODE_DESC> MDs(Count);
		VERIFY_SUCCEEDED(Outp->GetDisplayModeList(Format, 0, &Count, std::data(MDs)));
		for (const auto& i : MDs) {
			std::cout << i;
		}
	}
#endif
}

/**
@brief マルチスレッドで「同じ」キューへサブミットできる
@note Vulkan ではマルチスレッドで「異なる」キューへのみサブミットできるので注意
*/
void DX::CreateCommandQueue()
{
	{
		constexpr D3D12_COMMAND_QUEUE_DESC CQD = {
			.Type = D3D12_COMMAND_LIST_TYPE_DIRECT,
			.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL,
			.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE,
			.NodeMask = 0 //!< マルチGPUの場合に使用 (1つしか使わない場合は0で良い)
		};
		VERIFY_SUCCEEDED(Device->CreateCommandQueue(&CQD, COM_PTR_UUIDOF_PUTVOID(GraphicsCommandQueue)));
	}
	{
		constexpr D3D12_COMMAND_QUEUE_DESC CQD = {
			.Type = D3D12_COMMAND_LIST_TYPE_COMPUTE,
			.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL,
			.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE,
			.NodeMask = 0
		};
		VERIFY_SUCCEEDED(Device->CreateCommandQueue(&CQD, COM_PTR_UUIDOF_PUTVOID(ComputeCommandQueue)));
	}
	LOG_OK();
}

void DX::DestroySwapchain() 
{
	//!< #TODO
}
bool DX::ReCreateSwapchain() 
{
	//!< #TODO
	return false;
}
void DX::CreateSwapChain(HWND hWnd, const DXGI_FORMAT ColorFormat, const UINT Width, const UINT Height)
{
	constexpr UINT BufferCount = 3;

	std::vector<DXGI_SAMPLE_DESC> SDs;
	{
		for (UINT i = 1; i <= D3D12_MAX_MULTISAMPLE_SAMPLE_COUNT; ++i) {
			auto FDMSQL = D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS({
				.Format = ColorFormat,
				.SampleCount = i,
				.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE,
				.NumQualityLevels = 0
				});
			VERIFY_SUCCEEDED(Device->CheckFeatureSupport(D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS, reinterpret_cast<void*>(&FDMSQL), sizeof(FDMSQL)));
			if (FDMSQL.NumQualityLevels) {
				SDs.emplace_back(DXGI_SAMPLE_DESC({ .Count = FDMSQL.SampleCount, .Quality = FDMSQL.NumQualityLevels - 1 }));
			}
		}
	}

#ifdef USE_FULLSCREEN
	//!< 起動時にフルスクリーンにする場合
	const auto SCD = DXGI_SWAP_CHAIN_DESC1({
		.Width = Width, .Height = Height,
		.Format = ColorFormat,
		.Stereo = FALSE,
		.SampleDesc = SDs[0],
		.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
		.BufferCount = BufferCount,
		.Scaling = DXGI_SCALING_STRETCH,
		.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD,
		.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED,
		.Flags = 0/*DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING*/,
	});
	constexpr DXGI_SWAP_CHAIN_FULLSCREEN_DESC SCFD = {
		.RefreshRate = DXGI_RATIONAL({.Numerator = 60, .Denominator = 1 }),
		.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED,
		.Scaling = DXGI_MODE_SCALING_UNSPECIFIED,
		.Windowed = FALSE,
	};
	COM_PTR_RESET(SwapChain.DxSwapChain);
	COM_PTR<IDXGISwapChain1> NewSwapChain;
	VERIFY_SUCCEEDED(Factory->CreateSwapChainForHwnd(COM_PTR_GET(GraphicsCommandQueue), hWnd, &SCD, /*&SCFD*/nullptr, /*COM_PTR_GET(Output)*/nullptr, COM_PTR_PUT(NewSwapChain)));
	COM_PTR_AS(NewSwapChain, SwapChain.DxSwapChain);

	//!< Alt + Enter によるフルスクリーン切替えを抑制 (フルスクリーン変更対応が必要となり、手間がかかる)
	Factory->MakeWindowAssociation(hWnd, DXGI_MWA_NO_ALT_ENTER);

	//!< フルスクリーン切替え(トグル)の例
	if(false){
		DXGI_SWAP_CHAIN_DESC1 SCD1;
		SwapChain.DxSwapChain->GetDesc1(&SCD1);
		if (!(SCD.Flags & DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING)) {
			BOOL IsFullScreen;
			VERIFY_SUCCEEDED(SwapChain.DxSwapChain->GetFullscreenState(&IsFullScreen, nullptr));
			VERIFY_SUCCEEDED(SwapChain.DxSwapChain->SetFullscreenState(!IsFullScreen, nullptr));
		}
	}
#else
	auto SCD = DXGI_SWAP_CHAIN_DESC({
		//!< 最適なフルスクリーンのパフォーマンスを得るには、IDXGIOutput->GetDisplayModeList() で取得する(ディスプレイのサポートする)DXGI_MODE_DESC でないとダメなので注意  #DX_TODO
		.BufferDesc = DXGI_MODE_DESC({
			.Width = Width, .Height = Height,
			.RefreshRate = DXGI_RATIONAL({.Numerator = 60, .Denominator = 1 }),
			.Format = ColorFormat,
			.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED,
			.Scaling = DXGI_MODE_SCALING_UNSPECIFIED
		}),
		.SampleDesc = SDs[0],
		.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
		.BufferCount = BufferCount,
		.OutputWindow = hWnd,
		.Windowed = TRUE,
		.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD, //!< バックバッファ内容は破棄 (毎フレーム描画)
		.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH //!< フルスクリーンにした時、最適なディスプレイモードが選択されるのを許可
	});
	//!< セッティングを変更してスワップチェインを再作成できるように、既存のを開放している
	COM_PTR_RESET(SwapChain.DxSwapChain);
	COM_PTR<IDXGISwapChain> NewSwapChain;
	VERIFY_SUCCEEDED(Factory->CreateSwapChain(COM_PTR_GET(GraphicsCommandQueue), &SCD, COM_PTR_PUT(NewSwapChain)));
	COM_PTR_AS(NewSwapChain, SwapChain.DxSwapChain);
#endif

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
	switch (ColorFormat)
	{
	default:
	{
		DXGI_HDR_METADATA_HDR10 Metadata = {
			.RedPrimary = { static_cast<UINT16>(0.64f * 50000.0f), static_cast<UINT16>(0.33f * 50000.0f) },
			.GreenPrimary = { static_cast<UINT16>(0.3f * 50000.0f), static_cast<UINT16>(0.6f * 50000.0f) },
			.BluePrimary = { static_cast<UINT16>(0.15f * 50000.0f), static_cast<UINT16>(0.06f * 50000.0f) },
			.WhitePoint = { static_cast<UINT16>(0.3127f * 50000.0f), static_cast<UINT16>(0.329f * 50000.0f) },
			.MaxMasteringLuminance = static_cast<UINT>(1000.0f * 10000.0f),
			.MinMasteringLuminance = static_cast<UINT>(0.001f * 10000.0f),
			.MaxContentLightLevel = static_cast<UINT16>(2000.0f),
			.MaxFrameAverageLightLevel = static_cast<UINT16>(500.0f)
		};
		SwapChain->SetHDRMetaData(DXGI_HDR_METADATA_TYPE_HDR10, sizeof(Metadata), &Metadata);
	}
	break;
	case DXGI_FORMAT_R16G16B16A16_FLOAT:
	{
		DXGI_HDR_METADATA_HDR10 Metadata = {
			.RedPrimary = { static_cast<UINT16>(0.708f * 50000.0f), static_cast<UINT16>(0.292f * 50000.0f) },
			.GreenPrimary = { static_cast<UINT16>(0.17f * 50000.0f), static_cast<UINT16>(0.797f * 50000.0f) },
			.BluePrimary = { static_cast<UINT16>(0.131f * 50000.0f), static_cast<UINT16>(0.046f * 50000.0f) },
			.WhitePoint = { static_cast<UINT16>(0.3127f * 50000.0f), static_cast<UINT16>(0.329f * 50000.0f) },
			.MaxMasteringLuminance = static_cast<UINT>(1000.0f * 10000.0f),
			.MinMasteringLuminance = static_cast<UINT>(0.001f * 10000.0f),
			.MaxContentLightLevel = static_cast<UINT16>(2000.0f),
			.MaxFrameAverageLightLevel = static_cast<UINT16>(500.0f)
		};
		SwapChain->SetHDRMetaData(DXGI_HDR_METADATA_TYPE_HDR10, sizeof(Metadata), &Metadata);
	}
	break;
	}
#endif

	const auto DHD = D3D12_DESCRIPTOR_HEAP_DESC({
		.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV,
		.NumDescriptors = SCD.BufferCount,
		.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE,
		.NodeMask = 0 //!< マルチGPUの場合に使用(1つしか使わない場合は0で良い)
	});
	VERIFY_SUCCEEDED(Device->CreateDescriptorHeap(&DHD, COM_PTR_UUIDOF_PUTVOID(SwapChain.DescriptorHeap)));

	LOG_OK();
}
void DX::GetSwapChainResource()
{
	DXGI_SWAP_CHAIN_DESC1 SCD;
	SwapChain.DxSwapChain->GetDesc1(&SCD);

	auto CDH = SwapChain.DescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	const auto IncSize = Device->GetDescriptorHandleIncrementSize(SwapChain.DescriptorHeap->GetDesc().Type);
	for (UINT i = 0; i < SCD.BufferCount; ++i) {
		auto& RAHs = SwapChain.ResourceAndHandles.emplace_back();

		//!< スワップチェインのバッファリソースを取得
		VERIFY_SUCCEEDED(SwapChain.DxSwapChain->GetBuffer(i, COM_PTR_UUIDOF_PUTVOID(RAHs.first)));

		//!< デスクリプタ(ビュー)の作成
#ifdef USE_GAMMA_CORRECTION
		constexpr D3D12_RENDER_TARGET_VIEW_DESC RTVD = {
			.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, //!< ガンマ補正あり
			.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D,
			.Texture2D = D3D12_TEX2D_RTV({.MipSlice = 0, .PlaneSlice = 0 })
		};
		Device->CreateRenderTargetView(COM_PTR_GET(RAH.first), &RTVD, CDH);
#else
		//!< タイプドフォーマットなら D3D12_RENDER_TARGET_VIEW_DESC* へ nullptr 指定可能
		Device->CreateRenderTargetView(COM_PTR_GET(RAHs.first), nullptr, CDH);
#endif
		RAHs.second = CDH;
		CDH.ptr += IncSize;
	}

	LOG_OK();
}
void DX::CreateSwapchain(HWND hWnd, const DXGI_FORMAT ColorFormat)
{
	CreateSwapChain(hWnd, ColorFormat, GetClientRectWidth(), GetClientRectHeight());

	//!< リソースを取得、ビューを作成 (Get resource, create view)
	GetSwapChainResource();
}

void DX::ResizeSwapChain(const UINT Width, const UINT Height)
{
	for (auto& i : SwapChain.ResourceAndHandles) { 
		COM_PTR_RESET(i.first); 
	}
	SwapChain.ResourceAndHandles.clear();

	DXGI_SWAP_CHAIN_DESC1 SCD;
	SwapChain.DxSwapChain->GetDesc1(&SCD);
	VERIFY_SUCCEEDED(SwapChain.DxSwapChain->ResizeBuffers(SCD.BufferCount, Width, Height, SCD.Format, SCD.Flags));
	Log("\tResizeBuffers\n");

	//!< リソースを取得、ビューを作成 (Get resource, create view)
	GetSwapChainResource();

	LOG_OK();
}

void DX::ResizeDepthStencil([[maybe_unused]] const DXGI_FORMAT DepthFormat, [[maybe_unused]] const UINT Width, [[maybe_unused]] const UINT Height)
{
	//COM_PTR_RESET(DepthStencilResource);
	//CreateDepthStencilResource(DepthFormat, Width, Height);

	LOG_OK();
}

void DX::CreateDirectCommandList(const UINT Count)
{
	//!< コマンド実行 CommandList->ExecuteCommandList() 後、GPUがコマンドアロケータの参照を終えるまで、アロケータのリセット CommandAllocator->Reset() してはいけない
	//!< (アロケータが覚えているのでコマンドのリセット CommandList->Reset() は可能)
	VERIFY_SUCCEEDED(Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, COM_PTR_UUIDOF_PUTVOID(DirectCommandAllocators.emplace_back())));

	const auto DCA = COM_PTR_GET(DirectCommandAllocators[0]);
	for (UINT i = 0; i < Count; ++i) {
		//!< パイプラインステートは後からでも指定できる GCL->Reset(CA, COM_PTR_GET(PS)) ので、ここでは nullptr を指定
		VERIFY_SUCCEEDED(Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, DCA, nullptr, COM_PTR_UUIDOF_PUTVOID(DirectCommandLists.emplace_back())));
		VERIFY_SUCCEEDED(DirectCommandLists.back()->Close());
	}
}
void DX::CreateBundleCommandList(const UINT Count)
{
	VERIFY_SUCCEEDED(Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_BUNDLE, COM_PTR_UUIDOF_PUTVOID(BundleCommandAllocators.emplace_back())));

	const auto BCA = COM_PTR_GET(BundleCommandAllocators[0]);
	for (UINT i = 0; i < Count; ++i) {
		VERIFY_SUCCEEDED(Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_BUNDLE, BCA, nullptr, COM_PTR_UUIDOF_PUTVOID(BundleCommandLists.emplace_back())));
		VERIFY_SUCCEEDED(BundleCommandLists.back()->Close());
	}
}
//void DX::CreateBundleCommandList()
//{
//	VERIFY_SUCCEEDED(Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_BUNDLE, COM_PTR_UUIDOF_PUTVOID(BundleCommandAllocators.emplace_back())));
//
//	const auto BCA = COM_PTR_GET(BundleCommandAllocators[0]);
//	DXGI_SWAP_CHAIN_DESC1 SCD;
//	SwapChain->GetDesc1(&SCD);
//	for (UINT i = 0; i < SCD.BufferCount; ++i) {
//		VERIFY_SUCCEEDED(Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_BUNDLE, BCA, nullptr, COM_PTR_UUIDOF_PUTVOID(BundleCommandLists.emplace_back())));
//		VERIFY_SUCCEEDED(BundleCommandLists.back()->Close());
//	}
//}
void DX::CreateComputeCommandList()
{
	VERIFY_SUCCEEDED(Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_COMPUTE, COM_PTR_UUIDOF_PUTVOID(ComputeCommandAllocators.emplace_back())));

	DXGI_SWAP_CHAIN_DESC1 SCD;
	SwapChain.DxSwapChain->GetDesc1(&SCD);
	for (UINT i = 0; i < SCD.BufferCount; ++i) {
		VERIFY_SUCCEEDED(Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_COMPUTE, COM_PTR_GET(ComputeCommandAllocators[0]), nullptr, COM_PTR_UUIDOF_PUTVOID(ComputeCommandLists.emplace_back())));
		VERIFY_SUCCEEDED(ComputeCommandLists.back()->Close());
	}
}
//!< ここではデフォルト実装として、ダイレクト、バンドル共にスワップチェイン数分用意することとする
//void DX::CreateCommandList()
//{
//	//!< コマンド実行(GCL->ExecuteCommandList())後、GPUがコマンドアロケータの参照を終えるまで、アロケータのリセット(CA->Reset())してはいけない、アロケータが覚えているのでコマンドのリセット(GCL->Reset())はしても良い
//	//!< (ここでは)ダイレクト用1つ、バンドル用1つのコマンドアロケータ作成をデフォルト実装とする
//	VERIFY_SUCCEEDED(Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, COM_PTR_UUIDOF_PUTVOID(DirectCommandAllocators.emplace_back())));
//	VERIFY_SUCCEEDED(Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_BUNDLE, COM_PTR_UUIDOF_PUTVOID(BundleCommandAllocators.emplace_back())));
//
//	DXGI_SWAP_CHAIN_DESC1 SCD;
//	SwapChain->GetDesc1(&SCD);
//	for (UINT i = 0; i < SCD.BufferCount; ++i) {
//		//!< パイプラインステートは後からでも指定できる GCL->Reset(CA, COM_PTR_GET(PS)) ので、ここでは nullptr を指定
//		VERIFY_SUCCEEDED(Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, COM_PTR_GET(DirectCommandAllocators[0]), nullptr, COM_PTR_UUIDOF_PUTVOID(DirectCommandLists.emplace_back())));
//		VERIFY_SUCCEEDED(DirectCommandLists.back()->Close());
//
//		VERIFY_SUCCEEDED(Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_BUNDLE, COM_PTR_GET(BundleCommandAllocators[0]), nullptr, COM_PTR_UUIDOF_PUTVOID(BundleCommandLists.emplace_back())));
//		VERIFY_SUCCEEDED(BundleCommandLists.back()->Close());
//	}
//}

void DX::CreateViewport(const FLOAT Width, const FLOAT Height, const FLOAT MinDepth, const FLOAT MaxDepth)
{
	//!< DirectX、OpenGLは「左下」が原点 (Vulkan はデフォルトで「左上」が原点)
	Viewports = {
		D3D12_VIEWPORT({ .TopLeftX = 0.0f, .TopLeftY = 0.0f, .Width = Width, .Height = Height, .MinDepth = MinDepth, .MaxDepth = MaxDepth }),
	};
	//!< left, top, right, bottomで指定 (offset, extentで指定のVKとは異なるので注意)
	ScissorRects = {
		D3D12_RECT({ .left = 0, .top = 0, .right = static_cast<LONG>(Width), .bottom = static_cast<LONG>(Height) }),
	};
	LOG_OK();
}

#if 0
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
		0, 0 //!< マルチGPUの場合に使用(1つしか使わない場合は0で良い)
	};
	VERIFY_SUCCEEDED(Device->CreateCommittedResource(&HeapProperties, D3D12_HEAP_FLAG_NONE, &ResourceDesc, D3D12_RESOURCE_STATE_COMMON, nullptr, COM_PTR_UUIDOF_PUTVOID(UnorderedAccessTextureResource)));

	LOG_OK();
}
#endif

//!< D3D_ROOT_SIGNATURE_VERSION_1_0 を使用する場合
template<> void DX::SerializeRootSignature(COM_PTR<ID3DBlob>& Blob, const std::vector<D3D12_ROOT_PARAMETER>& RPs, const std::vector<D3D12_STATIC_SAMPLER_DESC>& SSDs, const D3D12_ROOT_SIGNATURE_FLAGS Flags)
{
	D3D12_FEATURE_DATA_ROOT_SIGNATURE FDRS = { .HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0 };
	VERIFY_SUCCEEDED(Device->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, reinterpret_cast<void*>(&FDRS), sizeof(FDRS)));
	VERIFY(FDRS.HighestVersion >= D3D_ROOT_SIGNATURE_VERSION_1_0);

	COM_PTR<ID3DBlob> ErrorBlob;
	const D3D12_ROOT_SIGNATURE_DESC RSD = {
		.NumParameters = static_cast<UINT>(std::size(RPs)), .pParameters = std::data(RPs),
		.NumStaticSamplers = static_cast<UINT>(std::size(SSDs)), .pStaticSamplers = std::data(SSDs),
		.Flags = Flags
	}; 
#if 1
	//!< デスクリプタにバージョンを含めるやり方
	const D3D12_VERSIONED_ROOT_SIGNATURE_DESC VRSD = { .Version = D3D_ROOT_SIGNATURE_VERSION_1_0, .Desc_1_0 = RSD, };
	VERIFY_SUCCEEDED(D3D12SerializeVersionedRootSignature(&VRSD, COM_PTR_PUT(Blob), COM_PTR_PUT(ErrorBlob)));
#else
	//!< 引数にバージョンを指定するやり方
	VERIFY_SUCCEEDED(D3D12SerializeRootSignature(&RSD, D3D_ROOT_SIGNATURE_VERSION_1_0, COM_PTR_PUT(Blob), COM_PTR_PUT(ErrorBlob)));
#endif
	LOG_OK();
}

//!< D3D_ROOT_SIGNATURE_VERSION_1_1 を使用する場合
//!< [ D3D12_ROOT_SIGNATURE_DESC と D3D12_ROOT_SIGNATURE_DESC1 の違い ]
//!< D3D12_ROOT_SIGNATURE_DESC1 では D3D12_ROOT_PARAMETER -> D3D12_ROOT_PARAMETER1 に変更されている
//!< D3D12_ROOT_PARAMETER1 では D3D12_ROOT_DESCRIPTOR -> D3D12_ROOT_DESCRIPTOR1 に変更されている
//!< D3D12_ROOT_DESCRIPTOR1 では D3D12_ROOT_DESCRIPTOR_FLAGS Flags メンバが増えている
template<> void DX::SerializeRootSignature(COM_PTR<ID3DBlob>& Blob, const std::vector<D3D12_ROOT_PARAMETER1>& RPs, const std::vector<D3D12_STATIC_SAMPLER_DESC>& SSDs, const D3D12_ROOT_SIGNATURE_FLAGS Flags)
{
	D3D12_FEATURE_DATA_ROOT_SIGNATURE FDRS = { .HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1 };
	VERIFY_SUCCEEDED(Device->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, reinterpret_cast<void*>(&FDRS), sizeof(FDRS)));
	VERIFY(FDRS.HighestVersion >= D3D_ROOT_SIGNATURE_VERSION_1_1);
	
	COM_PTR<ID3DBlob> ErrorBlob;
	const D3D12_ROOT_SIGNATURE_DESC1 RSD = {
		.NumParameters = static_cast<UINT>(std::size(RPs)), .pParameters = std::data(RPs),
		.NumStaticSamplers = static_cast<UINT>(std::size(SSDs)), .pStaticSamplers = std::data(SSDs),
		.Flags = Flags
	};
	const D3D12_VERSIONED_ROOT_SIGNATURE_DESC VRSD = { .Version = D3D_ROOT_SIGNATURE_VERSION_1_1, .Desc_1_1 = RSD, };
	VERIFY_SUCCEEDED(D3D12SerializeVersionedRootSignature(&VRSD, COM_PTR_PUT(Blob), COM_PTR_PUT(ErrorBlob)));
	LOG_OK();
}

//!< シェーダからルートシグネチャパートを取り出しブロブを作る
void DX::GetRootSignaturePartFromShader(COM_PTR<ID3DBlob>& Blob, const std::filesystem::path& Path)
{
	COM_PTR<ID3DBlob> ShaderBlob;
	VERIFY_SUCCEEDED(D3DReadFileToBlob(data(Path.wstring()), COM_PTR_PUT(ShaderBlob)));
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
	GetRootSignaturePartFromShader(Blob, data(GetBasePath() + TEXT(".rs.cso")));
#else
	SerializeRootSignature(Blob, {}, {}, D3D12_ROOT_SIGNATURE_FLAG_NONE | SHADER_ROOT_ACCESS_DENY_ALL);
#endif
	VERIFY_SUCCEEDED(Device->CreateRootSignature(0/* マルチGPUの場合に使用(1つしか使わない場合は0で良い)*/, Blob->GetBufferPointer(), Blob->GetBufferSize(), COM_PTR_UUIDOF_PUTVOID(RootSignatures.emplace_back())));
	LOG_OK();
}

void DX::CreateVideo()
{
	COM_PTR<ID3D12VideoDevice> VideoDevice;
	VERIFY_SUCCEEDED(Device->QueryInterface(COM_PTR_UUIDOF_PUTVOID(VideoDevice)));

	//!< デコードプロファイル数を取得
	D3D12_FEATURE_DATA_VIDEO_DECODE_PROFILE_COUNT FDVDPC = { .NodeIndex = 0 };
	VERIFY_SUCCEEDED(VideoDevice->CheckFeatureSupport(D3D12_FEATURE_VIDEO_DECODE_PROFILE_COUNT, reinterpret_cast<void*>(&FDVDPC), sizeof(FDVDPC)));
	if (FDVDPC.ProfileCount) {
		//!< デコードプロファイルを列挙
		std::vector<GUID> Profiles(FDVDPC.ProfileCount);
		auto FDVDP = D3D12_FEATURE_DATA_VIDEO_DECODE_PROFILES({
			.NodeIndex = 0,
			.ProfileCount = static_cast<UINT>(std::size(Profiles)),
			.pProfiles = data(Profiles)
		});
		VERIFY_SUCCEEDED(VideoDevice->CheckFeatureSupport(D3D12_FEATURE_VIDEO_DECODE_PROFILES, reinterpret_cast<void*>(&FDVDP), sizeof(FDVDP)));
		for (auto i : Profiles) {
			OutputDebugStringA(data(std::format("Prifile = {} {} {} {}\n", i.Data1, i.Data2, i.Data3, i.Data4[0])));
			const auto Configration = D3D12_VIDEO_DECODE_CONFIGURATION({
				.DecodeProfile = i,
				.BitstreamEncryption = D3D12_BITSTREAM_ENCRYPTION_TYPE_NONE,
				.InterlaceType = D3D12_VIDEO_FRAME_CODED_INTERLACE_TYPE_NONE
			});
			//!< デコードフォーマット数を取得
			D3D12_FEATURE_DATA_VIDEO_DECODE_FORMAT_COUNT FDVDFC = { .NodeIndex = 0, .Configuration = Configration };
			VERIFY_SUCCEEDED(VideoDevice->CheckFeatureSupport(D3D12_FEATURE_VIDEO_DECODE_FORMAT_COUNT, reinterpret_cast<void*>(&FDVDFC), sizeof(FDVDFC)));
			if (FDVDFC.FormatCount) {
				//!< デコードフォーマットを列挙
				std::vector<DXGI_FORMAT> Formats(FDVDFC.FormatCount);
				D3D12_FEATURE_DATA_VIDEO_DECODE_FORMATS FDVDF = {
					.NodeIndex = 0,
					.Configuration = Configration,
					.FormatCount = static_cast<UINT>(std::size(Formats)),
					.pOutputFormats = data(Formats),
				};
				VERIFY_SUCCEEDED(VideoDevice->CheckFeatureSupport(D3D12_FEATURE_VIDEO_DECODE_FORMATS, reinterpret_cast<void*>(&FDVDF), sizeof(FDVDF)));
				for (auto j : Formats) {
					OutputDebugStringA(data(std::format("\tFormat = {}\n", (int)j)));
					
					constexpr auto FrameRate30 = DXGI_RATIONAL({ .Numerator = 30, .Denominator = 1 });
					//!< デコードサポート情報を取得
					auto FDVDS = D3D12_FEATURE_DATA_VIDEO_DECODE_SUPPORT({
						.NodeIndex = 0,
						.Configuration = Configration,
						.Width = 1024, .Height = 1024,
						.DecodeFormat = j,
						.FrameRate = FrameRate30,
						.BitRate = 0,
					});
					VERIFY_SUCCEEDED(VideoDevice->CheckFeatureSupport(D3D12_FEATURE_VIDEO_DECODE_SUPPORT, reinterpret_cast<void*>(&FDVDS), sizeof(FDVDS)));
					if (FDVDS.SupportFlags & D3D12_VIDEO_DECODE_SUPPORT_FLAG_SUPPORTED) {
						if (FDVDS.ConfigurationFlags& D3D12_VIDEO_DECODE_CONFIGURATION_FLAG_HEIGHT_ALIGNMENT_MULTIPLE_32_REQUIRED) {}
						if (FDVDS.ConfigurationFlags& D3D12_VIDEO_DECODE_CONFIGURATION_FLAG_POST_PROCESSING_SUPPORTED) {}
						if (FDVDS.ConfigurationFlags& D3D12_VIDEO_DECODE_CONFIGURATION_FLAG_REFERENCE_ONLY_ALLOCATIONS_REQUIRED) {}
						if (FDVDS.ConfigurationFlags& D3D12_VIDEO_DECODE_CONFIGURATION_FLAG_ALLOW_RESOLUTION_CHANGE_ON_NON_KEY_FRAME) {}
						switch (FDVDS.DecodeTier) {
						case D3D12_VIDEO_DECODE_TIER_NOT_SUPPORTED:
							break;
						case D3D12_VIDEO_DECODE_TIER_1:
							break;
						case D3D12_VIDEO_DECODE_TIER_2:
							break;
						case D3D12_VIDEO_DECODE_TIER_3:
							break;
						}

#if 0
						//!< ビデオデコーダの作成
						const D3D12_VIDEO_DECODER_DESC VDD = {
							 .NodeMask = 0,
							.Configuration = Configration
						};
						VERIFY_SUCCEEDED(VideoDevice->CreateVideoDecoder(&VDD, COM_PTR_UUIDOF_PUTVOID(VideoDecoder)));

						//!< ビデオデコーダヒープの作成
						const D3D12_VIDEO_DECODER_HEAP_DESC VDHD = {
							.NodeMask = 0,
							.Configuration = Configration,
							.DecodeWidth = 1024, .DecodeHeight = 1024,
							.Format = j,
							.FrameRate = DXGI_RATIONAL({.Numerator = 0, .Denominator = 0 }),
							.BitRate = 0,
							.MaxDecodePictureBufferCount = 0
						};
						VERIFY_SUCCEEDED(VideoDevice->CreateVideoDecoderHeap(&VDHD, COM_PTR_UUIDOF_PUTVOID(VideoDecoderHeap)));
#endif

						D3D12_VIDEO_DECODE_FRAME_ARGUMENT VDFAs = {.Type= D3D12_VIDEO_DECODE_ARGUMENT_TYPE_PICTURE_PARAMETERS, .Size = 0, .pData = nullptr };

						D3D12_VIDEO_DECODE_REFERENCE_FRAMES VDRF = {
							.NumTexture2Ds=1,
						}; 
						D3D12_VIDEO_DECODE_COMPRESSED_BITSTREAM VDCB = { .pBuffer = nullptr, .Offset = 0, .Size = 0 };
						D3D12_VIDEO_DECODE_INPUT_STREAM_ARGUMENTS VDISA = {
							.NumFrameArguments = 1,
							.FrameArguments = VDFAs,
							.ReferenceFrames = VDRF,
							.CompressedBitstream = VDCB,
							.pHeap = COM_PTR_GET(VideoDecoderHeap)
						};
					}

					//!< プロセスサポート情報を取得
					auto FDVPS = D3D12_FEATURE_DATA_VIDEO_PROCESS_SUPPORT({
						.NodeIndex = 0,
						.InputSample = D3D12_VIDEO_SAMPLE({
							.Width = 1024, .Height = 1024,
							.Format = D3D12_VIDEO_FORMAT({.Format = j, .ColorSpace = DXGI_COLOR_SPACE_RGB_FULL_G22_NONE_P709 })
						}),
						.InputFieldType = D3D12_VIDEO_FIELD_TYPE_NONE,
						.InputStereoFormat = D3D12_VIDEO_FRAME_STEREO_FORMAT_NONE,
						.InputFrameRate = FrameRate30,
						.OutputFormat = D3D12_VIDEO_FORMAT({.Format = DXGI_FORMAT_R8G8B8A8_UNORM, .ColorSpace = DXGI_COLOR_SPACE_RGB_FULL_G22_NONE_P709 }),
						.OutputStereoFormat = D3D12_VIDEO_FRAME_STEREO_FORMAT_NONE,
						.OutputFrameRate = FrameRate30,
					});
					VERIFY_SUCCEEDED(VideoDevice->CheckFeatureSupport(D3D12_FEATURE_VIDEO_PROCESS_SUPPORT, reinterpret_cast<void*>(&FDVPS), sizeof(FDVPS)));
					if (FDVPS.SupportFlags & D3D12_VIDEO_PROCESS_SUPPORT_FLAG_SUPPORTED) {
						FDVPS.ScaleSupport.OutputSizeRange;
						switch (FDVPS.ScaleSupport.Flags)
						{
						case D3D12_VIDEO_SCALE_SUPPORT_FLAG_NONE:
						case D3D12_VIDEO_SCALE_SUPPORT_FLAG_POW2_ONLY:
						case D3D12_VIDEO_SCALE_SUPPORT_FLAG_EVEN_DIMENSIONS_ONLY:
							break;
						}

						if (FDVPS.FeatureSupport & D3D12_VIDEO_PROCESS_FEATURE_FLAG_ALPHA_FILL) {}
						if (FDVPS.FeatureSupport & D3D12_VIDEO_PROCESS_FEATURE_FLAG_LUMA_KEY) {}
						if (FDVPS.FeatureSupport & D3D12_VIDEO_PROCESS_FEATURE_FLAG_STEREO) {}
						if (FDVPS.FeatureSupport & D3D12_VIDEO_PROCESS_FEATURE_FLAG_ROTATION) {}
						if (FDVPS.FeatureSupport & D3D12_VIDEO_PROCESS_FEATURE_FLAG_FLIP) {}
						if (FDVPS.FeatureSupport & D3D12_VIDEO_PROCESS_FEATURE_FLAG_ALPHA_BLENDING) {}
						if (FDVPS.FeatureSupport & D3D12_VIDEO_PROCESS_FEATURE_FLAG_PIXEL_ASPECT_RATIO) {}

						if (FDVPS.DeinterlaceSupport & D3D12_VIDEO_PROCESS_DEINTERLACE_FLAG_BOB) {}
						if (FDVPS.DeinterlaceSupport & D3D12_VIDEO_PROCESS_DEINTERLACE_FLAG_CUSTOM) {}

						if (FDVPS.AutoProcessingSupport & D3D12_VIDEO_PROCESS_AUTO_PROCESSING_FLAG_DENOISE) {}
						if (FDVPS.AutoProcessingSupport & D3D12_VIDEO_PROCESS_AUTO_PROCESSING_FLAG_DERINGING) {}
						if (FDVPS.AutoProcessingSupport & D3D12_VIDEO_PROCESS_AUTO_PROCESSING_FLAG_EDGE_ENHANCEMENT) {}
						if (FDVPS.AutoProcessingSupport & D3D12_VIDEO_PROCESS_AUTO_PROCESSING_FLAG_COLOR_CORRECTION) {}
						if (FDVPS.AutoProcessingSupport & D3D12_VIDEO_PROCESS_AUTO_PROCESSING_FLAG_FLESH_TONE_MAPPING) {}
						if (FDVPS.AutoProcessingSupport & D3D12_VIDEO_PROCESS_AUTO_PROCESSING_FLAG_IMAGE_STABILIZATION) {}
						if (FDVPS.AutoProcessingSupport & D3D12_VIDEO_PROCESS_AUTO_PROCESSING_FLAG_SUPER_RESOLUTION) {}
						if (FDVPS.AutoProcessingSupport & D3D12_VIDEO_PROCESS_AUTO_PROCESSING_FLAG_ANAMORPHIC_SCALING) {}
						if (FDVPS.AutoProcessingSupport & D3D12_VIDEO_PROCESS_AUTO_PROCESSING_FLAG_CUSTOM) {}

						if (FDVPS.FilterSupport & D3D12_VIDEO_PROCESS_FILTER_FLAG_BRIGHTNESS) {}
						if (FDVPS.FilterSupport & D3D12_VIDEO_PROCESS_FILTER_FLAG_CONTRAST) {}
						if (FDVPS.FilterSupport & D3D12_VIDEO_PROCESS_FILTER_FLAG_HUE) {}
						if (FDVPS.FilterSupport & D3D12_VIDEO_PROCESS_FILTER_FLAG_SATURATION) {}
						if (FDVPS.FilterSupport & D3D12_VIDEO_PROCESS_FILTER_FLAG_NOISE_REDUCTION) {}
						if (FDVPS.FilterSupport & D3D12_VIDEO_PROCESS_FILTER_FLAG_EDGE_ENHANCEMENT) {}
						if (FDVPS.FilterSupport & D3D12_VIDEO_PROCESS_FILTER_FLAG_ANAMORPHIC_SCALING) {}
						if (FDVPS.FilterSupport & D3D12_VIDEO_PROCESS_FILTER_FLAG_STEREO_ADJUSTMENT) {}

						FDVPS.FilterRangeSupport[0].Minimum;
						FDVPS.FilterRangeSupport[0].Maximum;
						FDVPS.FilterRangeSupport[0].Default;
						FDVPS.FilterRangeSupport[0].Multiplier;
					}
				}
			}
		}
	}
}

void DX::ProcessShaderReflection(ID3DBlob* Blob)
{
#ifdef USE_DXC
	COM_PTR<IDxcLibrary> DL;
	VERIFY_SUCCEEDED(DxcCreateInstance(CLSID_DxcLibrary, COM_PTR_UUIDOF_PUTVOID(DL)));

	COM_PTR<IDxcBlobEncoding> DBE;
	VERIFY_SUCCEEDED(DL->CreateBlobWithEncodingOnHeapCopy(Blob->GetBufferPointer(), static_cast<UINT32>(Blob->GetBufferSize()), CP_ACP, COM_PTR_PUT(DBE)));

	COM_PTR<IDxcContainerReflection> DCR;
	VERIFY_SUCCEEDED(DxcCreateInstance(CLSID_DxcContainerReflection, COM_PTR_UUIDOF_PUTVOID(DCR)));

	UINT Index = 0;
	VERIFY_SUCCEEDED(DCR->Load(COM_PTR_GET(DBE)));
	VERIFY_SUCCEEDED(DCR->FindFirstPartKind(DXIL_FOURCC('D', 'X', 'I', 'L'), &Index));

	COM_PTR<ID3D12ShaderReflection> SR;
	VERIFY_SUCCEEDED(DCR->GetPartReflection(Index, COM_PTR_UUIDOF_PUTVOID(SR)));	
#else
	COM_PTR<ID3D12ShaderReflection> SR;
	VERIFY_SUCCEEDED(D3DReflect(Blob->GetBufferPointer(), Blob->GetBufferSize(), COM_PTR_UUIDOF_PUTVOID(SR)));
#endif

#ifdef DEBUG_STDOUT
	std::cout << COM_PTR_GET(SR);
#endif
}

void DX::SetBlobPart(COM_PTR<ID3DBlob>& Blob)
{
	//!< D3D_BLOB_DEBUG_NAME
	{
		//!< 付ける名前
		std::string_view Name = "Debug Name";

		//!< 4 バイトアラインされたバッファ
		std::vector<char> Buf(RoundUp(Name.length() + 1, 4));
		Buf[Name.length()] = '\0'; //!< std::ranges::fill(Buf, '\0');
		std::ranges::copy(Name, begin(Buf));

		COM_PTR<ID3DBlob> NewBlob;
		if (SUCCEEDED(D3DSetBlobPart(Blob->GetBufferPointer(), Blob->GetBufferSize(), D3D_BLOB_DEBUG_NAME, 0, std::data(Buf), std::size(Buf), COM_PTR_PUT(NewBlob)))) {
			COM_PTR_COPY(Blob, NewBlob);
		}
	}
}
void DX::GetBlobPart(ID3DBlob* Blob)
{
	//!< D3D_BLOB_DEBUG_INFO
	{
		COM_PTR<ID3DBlob> Part;
		if (SUCCEEDED(D3DGetBlobPart(Blob->GetBufferPointer(), Blob->GetBufferSize(), D3D_BLOB_DEBUG_INFO, 0, COM_PTR_PUT(Part)))) {
			Log("\tD3D_BLOB_DEBUG_INFO\n");
		}
	}
	//!< D3D_BLOB_PDB
	{
		COM_PTR<ID3DBlob> Part;
		if (SUCCEEDED(D3DGetBlobPart(Blob->GetBufferPointer(), Blob->GetBufferSize(), D3D_BLOB_PDB, 0, COM_PTR_PUT(Part)))) {
			Log("\tD3D_BLOB_PDB\n");
		}
	}
	//!< D3D_BLOB_PRIVATE_DATA
	{
		COM_PTR<ID3DBlob> Part;
		if (SUCCEEDED(D3DGetBlobPart(Blob->GetBufferPointer(), Blob->GetBufferSize(), D3D_BLOB_PRIVATE_DATA, 0, COM_PTR_PUT(Part)))) {
			Log("\tD3D_BLOB_PRIVATE_DATA\n");
		}
	}
	//!< D3D_BLOB_ROOT_SIGNATURE
	{
		COM_PTR<ID3DBlob> Part;
		if (SUCCEEDED(D3DGetBlobPart(Blob->GetBufferPointer(), Blob->GetBufferSize(), D3D_BLOB_ROOT_SIGNATURE, 0, COM_PTR_PUT(Part)))) {
			Log("\tD3D_BLOB_ROOT_SIGNATURE\n");
		}
	}
	//!< D3D_BLOB_DEBUG_NAME
	{
		COM_PTR<ID3DBlob> Part;
		if (SUCCEEDED(D3DGetBlobPart(Blob->GetBufferPointer(), Blob->GetBufferSize(), D3D_BLOB_DEBUG_NAME, 0, COM_PTR_PUT(Part)))) {
			Logf("\tD3D_BLOB_DEBUG_NAME = %s\n", reinterpret_cast<const char*>(Part->GetBufferPointer()));
		}
	}
}
void DX::StripShader(COM_PTR<ID3DBlob>& Blob)
{	 
	//!< D3DCOMPILER_STRIP_REFLECTION_DATA
	{
		COM_PTR<ID3DBlob> NewBlob;
		if (SUCCEEDED(D3DStripShader(Blob->GetBufferPointer(), Blob->GetBufferSize(), D3DCOMPILER_STRIP_REFLECTION_DATA, COM_PTR_PUT(NewBlob)))) {
			Log("\tD3DCOMPILER_STRIP_REFLECTION_DATA\n");
			COM_PTR_COPY(Blob, NewBlob);
		}
	}
	//!< D3DCOMPILER_STRIP_DEBUG_INFO
	{
		COM_PTR<ID3DBlob> NewBlob;
		if (SUCCEEDED(D3DStripShader(Blob->GetBufferPointer(), Blob->GetBufferSize(), D3DCOMPILER_STRIP_DEBUG_INFO, COM_PTR_PUT(NewBlob)))) {
			Log("\tD3DCOMPILER_STRIP_DEBUG_INFO\n");
			COM_PTR_COPY(Blob, NewBlob);
		}
	}
	//!< D3DCOMPILER_STRIP_TEST_BLOBS
	{
		COM_PTR<ID3DBlob> NewBlob;
		if (SUCCEEDED(D3DStripShader(Blob->GetBufferPointer(), Blob->GetBufferSize(), D3DCOMPILER_STRIP_TEST_BLOBS, COM_PTR_PUT(NewBlob)))) {
			Log("\tD3DCOMPILER_STRIP_TEST_BLOBS\n");
			COM_PTR_COPY(Blob, NewBlob);
		}
	}
	//!< D3DCOMPILER_STRIP_PRIVATE_DATA
	{
		COM_PTR<ID3DBlob> NewBlob;
		if (SUCCEEDED(D3DStripShader(Blob->GetBufferPointer(), Blob->GetBufferSize(), D3DCOMPILER_STRIP_PRIVATE_DATA, COM_PTR_PUT(NewBlob)))) {
			Log("\tD3DCOMPILER_STRIP_PRIVATE_DATA\n");
			COM_PTR_COPY(Blob, NewBlob);
		}
	}
	//!< D3DCOMPILER_STRIP_ROOT_SIGNATURE
	{
		COM_PTR<ID3DBlob> NewBlob;
		if (SUCCEEDED(D3DStripShader(Blob->GetBufferPointer(), Blob->GetBufferSize(), D3DCOMPILER_STRIP_ROOT_SIGNATURE, COM_PTR_PUT(NewBlob)))) {
			Log("\tD3DCOMPILER_STRIP_ROOT_SIGNATURE\n");
			COM_PTR_COPY(Blob, NewBlob);
		}
	}
}

void DX::CreatePipelineStateVsPsDsHsGs(COM_PTR<ID3D12PipelineState>& PST,
	ID3D12Device* Device, ID3D12RootSignature* RS,
	const D3D12_PRIMITIVE_TOPOLOGY_TYPE PTT,
	const std::vector<D3D12_RENDER_TARGET_BLEND_DESC>& RTBDs,
	const D3D12_RASTERIZER_DESC& RD,
	const D3D12_DEPTH_STENCIL_DESC& DSD,
	const D3D12_SHADER_BYTECODE VS, const D3D12_SHADER_BYTECODE PS, const D3D12_SHADER_BYTECODE DS, const D3D12_SHADER_BYTECODE HS, const D3D12_SHADER_BYTECODE GS,
	const std::vector<D3D12_INPUT_ELEMENT_DESC>& IEDs,
	const std::vector<DXGI_FORMAT>& RTVFormats,
	const PipelineLibrarySerializer* PLS, LPCWSTR Name)
{
	PERFORMANCE_COUNTER_FUNC();

	VERIFY((VS.pShaderBytecode != nullptr && VS.BytecodeLength));

	//!< キャッシュドパイプラインステート (CachedPipelineState)
	//!< (VK の VkGraphicsPipelineCreateInfo.basePipelineHandle, basePipelineIndex 相当?)
	COM_PTR<ID3DBlob> CachedBlob;
	if (nullptr != PST) {
		VERIFY_SUCCEEDED(PST->GetCachedBlob(COM_PTR_PUT(CachedBlob)));
	}

	//!< DXでは「パッチコントロールポイント」個数の指定はIASetPrimitiveTopology()の引数として「コマンドリスト作成時」に指定する、VKとは結構異なるので注意
	//!< GCL->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_1_CONTROL_POINT_PATCHLIST);	

	D3D12_GRAPHICS_PIPELINE_STATE_DESC GPSD = {
		.pRootSignature = RS,
		.VS = VS, .PS = PS, .DS = DS, .HS = HS, .GS = GS,
		.StreamOutput = D3D12_STREAM_OUTPUT_DESC({
			.pSODeclaration = nullptr, .NumEntries = 0,
			.pBufferStrides = nullptr, .NumStrides = 0,
			.RasterizedStream = 0 
			}),
		.BlendState = D3D12_BLEND_DESC({
			.AlphaToCoverageEnable = TRUE,		//!< マルチサンプルを考慮したアルファテスト(AlphaToCoverageEnable)、アルファが0の箇所には無駄に書き込まない
			.IndependentBlendEnable = FALSE,	//!< マルチレンダーターゲットにそれぞれ別のブレンドステートを割り当てる(IndependentBlendEnable)
			.RenderTarget = {} 
			}),
		.SampleMask = D3D12_DEFAULT_SAMPLE_MASK,
	 	.RasterizerState = RD,
		.DepthStencilState = DSD,
		.InputLayout = D3D12_INPUT_LAYOUT_DESC({ .pInputElementDescs = data(IEDs), .NumElements = static_cast<UINT>(std::size(IEDs)) }),
		.IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED,
		.PrimitiveTopologyType = PTT,
		.NumRenderTargets = static_cast<UINT>(std::size(RTVFormats)), .RTVFormats = {},
		.DSVFormat = DSD.DepthEnable ? DXGI_FORMAT_D24_UNORM_S8_UINT : DXGI_FORMAT_UNKNOWN,
		.SampleDesc = DXGI_SAMPLE_DESC({.Count = 1, .Quality = 0 }),
		.NodeMask = 0, //!< マルチGPUの場合に使用(1つしか使わない場合は0で良い)
		.CachedPSO = D3D12_CACHED_PIPELINE_STATE({.pCachedBlob = nullptr != CachedBlob ? CachedBlob->GetBufferPointer() : nullptr, .CachedBlobSizeInBytes = nullptr != CachedBlob ? CachedBlob->GetBufferSize() : 0 }),
#if defined(_DEBUG) && defined(USE_WARP)
		//!< パイプラインがデバッグ用付加情報ありでコンパイルされる、WARP時のみ使用可能
		.Flags = D3D12_PIPELINE_STATE_FLAG_TOOL_DEBUG 
#else
		.Flags = D3D12_PIPELINE_STATE_FLAG_NONE
#endif
	};

	//!< レンダーターゲット数分だけ必要なもの
	VERIFY(std::size(RTBDs) <= std::size(GPSD.BlendState.RenderTarget));
	std::ranges::copy(RTBDs, GPSD.BlendState.RenderTarget);
	//!< TRUE == IndependentBlendEnable の場合はレンダーターゲットの分だけ用意すること (If TRUE == IndependentBlendEnable, need NumRenderTarget elements)
	VERIFY((false == GPSD.BlendState.IndependentBlendEnable || std::size(RTBDs) == GPSD.NumRenderTargets));
	VERIFY(GPSD.NumRenderTargets <= std::size(GPSD.RTVFormats));
	std::ranges::copy(RTVFormats, GPSD.RTVFormats);

	VERIFY((0 == GPSD.DS.BytecodeLength || 0 == GPSD.HS.BytecodeLength || GPSD.PrimitiveTopologyType == D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH));

	if (nullptr != PLS && PLS->IsLoadSucceeded()) {
		VERIFY_SUCCEEDED(PLS->GetPipelineLibrary()->LoadGraphicsPipeline(Name, &GPSD, COM_PTR_UUIDOF_PUTVOID(PST)));
	}
	else {
		//!< コンパイルしたシェーダモデルバージョンによってコケる場合がある
		VERIFY_SUCCEEDED(Device->CreateGraphicsPipelineState(&GPSD, COM_PTR_UUIDOF_PUTVOID(PST)));
		if (nullptr != PLS) {
			VERIFY_SUCCEEDED(PLS->GetPipelineLibrary()->StorePipeline(Name, COM_PTR_GET(PST)));
		}
	}
	LOG_OK();
}

void DX::CreatePipelineStateAsMsPs(COM_PTR<ID3D12PipelineState>& PST,
	ID3D12Device* Device, 
	ID3D12RootSignature* RS,
	const std::vector<D3D12_RENDER_TARGET_BLEND_DESC>& RTBDs, 
	const D3D12_RASTERIZER_DESC& RD,
	const D3D12_DEPTH_STENCIL_DESC1& DSD, 
	const D3D12_SHADER_BYTECODE AS, const D3D12_SHADER_BYTECODE MS, const D3D12_SHADER_BYTECODE PS, 
	const std::vector<DXGI_FORMAT>& RTVFormats, 
	const PipelineLibrarySerializer* PLS, LPCWSTR Name)
{
	PERFORMANCE_COUNTER_FUNC();

	//!< キャッシュドパイプラインステート (CachedPipelineState)
	COM_PTR<ID3DBlob> CachedBlob;
	if (nullptr != PST) {
		VERIFY_SUCCEEDED(PST->GetCachedBlob(COM_PTR_PUT(CachedBlob)));
	}
	
	PIPELINE_MESH_STATE_STREAM PMSS = {
		.pRootSignature = RS,
		.AS = AS, .MS = MS, .PS = PS,
		.BlendState = D3D12_BLEND_DESC({.AlphaToCoverageEnable = TRUE, .IndependentBlendEnable = FALSE, .RenderTarget = {}}),
		.SampleMask = D3D12_DEFAULT_SAMPLE_MASK,
		.RasterizerState = RD,
		.DepthStencilState = DSD,
		.RTVFormats = D3D12_RT_FORMAT_ARRAY({.RTFormats = {}, .NumRenderTargets = static_cast<UINT>(std::size(RTVFormats)) }),
		.DSVFormat = DSD.DepthEnable ? DXGI_FORMAT_D24_UNORM_S8_UINT : DXGI_FORMAT_UNKNOWN,
		.SampleDesc = DXGI_SAMPLE_DESC({.Count = 1, .Quality = 0 }),
		.NodeMask = 0,
		.CachedPSO = D3D12_CACHED_PIPELINE_STATE({.pCachedBlob = nullptr != CachedBlob ? CachedBlob->GetBufferPointer() : nullptr, .CachedBlobSizeInBytes = nullptr != CachedBlob ? CachedBlob->GetBufferSize() : 0 }),
#if defined(_DEBUG) && defined(USE_WARP)
		.Flags = D3D12_PIPELINE_STATE_FLAG_TOOL_DEBUG,
#else
		.Flags = D3D12_PIPELINE_STATE_FLAG_NONE,
#endif
		.ViewInstancingDesc = D3D12_VIEW_INSTANCING_DESC({.ViewInstanceCount = 0, .pViewInstanceLocations = nullptr, .Flags = D3D12_VIEW_INSTANCING_FLAG_NONE }),
	};
	VERIFY(std::size(RTBDs) <= std::size(PMSS.BlendState.Value.RenderTarget));
	std::ranges::copy(RTBDs, PMSS.BlendState.Value.RenderTarget);
	VERIFY((false == PMSS.BlendState.Value.IndependentBlendEnable || std::size(RTBDs) == PMSS.RTVFormats.Value.NumRenderTargets));
	VERIFY(PMSS.RTVFormats.Value.NumRenderTargets <= std::size(PMSS.RTVFormats.Value.RTFormats));
	std::ranges::copy(RTVFormats, PMSS.RTVFormats.Value.RTFormats);
	
	const D3D12_PIPELINE_STATE_STREAM_DESC PSSD = { .SizeInBytes = sizeof(PMSS), .pPipelineStateSubobjectStream = reinterpret_cast<void*>(&PMSS) };

	if (nullptr != PLS && PLS->IsLoadSucceeded()) {
		COM_PTR<ID3D12PipelineLibrary1> PL1;
		VERIFY_SUCCEEDED(PLS->GetPipelineLibrary()->QueryInterface(COM_PTR_UUIDOF_PUTVOID(PL1)));
		VERIFY_SUCCEEDED(PL1->LoadPipeline(Name, &PSSD, COM_PTR_UUIDOF_PUTVOID(PST)));
	}
	else {
		COM_PTR<ID3D12Device2> Device2;
		VERIFY_SUCCEEDED(Device->QueryInterface(COM_PTR_UUIDOF_PUTVOID(Device2)));
		VERIFY_SUCCEEDED(Device2->CreatePipelineState(&PSSD, COM_PTR_UUIDOF_PUTVOID(PST)));
		if (nullptr != PLS) {
			VERIFY_SUCCEEDED(PLS->GetPipelineLibrary()->StorePipeline(Name, COM_PTR_GET(PST)));
		}
	}
	LOG_OK();
}

//!< UINT32 -> A8B8R8G8
void DX::CreateTextureArray1x1(const std::vector<UINT32>& Colors, const D3D12_RESOURCE_STATES RS)
{
	Textures.emplace_back().Create(COM_PTR_GET(Device), 1, 1, static_cast<UINT16>(std::size(Colors)), DXGI_FORMAT_R8G8B8A8_UNORM);

	const auto CA = COM_PTR_GET(DirectCommandAllocators[0]);
	const auto GCL = COM_PTR_GET(DirectCommandLists[0]);
	{
		constexpr auto PitchSize = 1 * static_cast<UINT32>(sizeof(Colors[0]));
		constexpr auto LayerSize = 1 * PitchSize;
		ResourceBase Upload;
		{
			//!< アラインされたサイズを計算 (Calculate aligned size)
			size_t AlignedSize = 0;
			for (UINT32 i = 0; i < std::size(Colors); ++i) {
				AlignedSize = RoundUp(i * LayerSize, D3D12_TEXTURE_DATA_PLACEMENT_ALIGNMENT);
				AlignedSize += LayerSize;
			}
			//!< Colors をアラインされたメモリへコピー (Copy Colors to aligned memory)
			std::vector AlignedData(AlignedSize, std::byte());
			for (UINT32 i = 0; i < std::size(Colors); ++i) {
				*reinterpret_cast<UINT32*>(&AlignedData[RoundUp(i * LayerSize, D3D12_TEXTURE_DATA_PLACEMENT_ALIGNMENT)]) = Colors[i];
			}
			Upload.Create(COM_PTR_GET(Device), std::size(AlignedData), D3D12_HEAP_TYPE_UPLOAD, std::data(AlignedData));
		}

		const auto RD = Textures.back().Resource->GetDesc();
		std::vector<D3D12_PLACED_SUBRESOURCE_FOOTPRINT> PSFs;
		for (UINT32 i = 0; i < std::size(Colors); ++i) {
			PSFs.emplace_back(D3D12_PLACED_SUBRESOURCE_FOOTPRINT({
				.Offset = RoundUp(i * LayerSize, D3D12_TEXTURE_DATA_PLACEMENT_ALIGNMENT),
				.Footprint = D3D12_SUBRESOURCE_FOOTPRINT({.Format = RD.Format, .Width = static_cast<UINT>(RD.Width), .Height = RD.Height, .Depth = 1, .RowPitch = static_cast<UINT>(RoundUp(PitchSize, D3D12_TEXTURE_DATA_PITCH_ALIGNMENT)) })
			}));
		}
		VERIFY_SUCCEEDED(GCL->Reset(CA, nullptr)); {
			PopulateCopyTextureRegionCommand(GCL, COM_PTR_GET(Upload.Resource), COM_PTR_GET(Textures.back().Resource), PSFs, RS);
		} VERIFY_SUCCEEDED(GCL->Close());
		DX::ExecuteAndWait(COM_PTR_GET(GraphicsCommandQueue), GCL, COM_PTR_GET(GraphicsFence));
	}
}
void DX::WaitForFence(ID3D12CommandQueue* CQ, ID3D12Fence* Fence)
{
	auto Value = Fence->GetCompletedValue();
	//!< インクリメントされた値
	++Value;

	//!< GPU が到達すれば、フェンスをシグナル状態にし、インクリメントされた値へ更新
	VERIFY_SUCCEEDED(CQ->Signal(Fence, Value));
	if (Fence->GetCompletedValue() < Value) {
		auto hEvent = CreateEventEx(nullptr, nullptr, 0, EVENT_ALL_ACCESS);
		if (nullptr != hEvent) [[likely]] {
			//!< CPU はインクリメントされた値になったらイベントが発行されるようにしておく
			VERIFY_SUCCEEDED(Fence->SetEventOnCompletion(Value, hEvent));
			//!< CPU はイベント発行まで待つ
			WaitForSingleObject(hEvent, INFINITE);
			CloseHandle(hEvent);
		}
	}
}
void DX::WaitForGPU()
{
	//!< 任意の場所で使うよう、既存のフェンスとは別に用意する
	COM_PTR< ID3D12Fence> Fence;
	VERIFY_SUCCEEDED(Device->CreateFence(0, D3D12_FENCE_FLAG_NONE, COM_PTR_UUIDOF_PUTVOID(Fence)));

	constexpr UINT64 Value = 1;
	VERIFY_SUCCEEDED(GraphicsCommandQueue->Signal(COM_PTR_GET(Fence), Value));

	auto hEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	Fence->SetEventOnCompletion(Value, hEvent);

	WaitForSingleObjectEx(hEvent, INFINITE, FALSE);
	CloseHandle(hEvent);
}

void DX::SubmitGraphics(const UINT i)
{
	const std::array CLs = { static_cast<ID3D12CommandList*>(COM_PTR_GET(DirectCommandLists[i])) };
	GraphicsCommandQueue->ExecuteCommandLists(static_cast<UINT>(std::size(CLs)), std::data(CLs));
}
void DX::SubmitCompute(const UINT i)
{
	const std::array CLs = { static_cast<ID3D12CommandList*>(COM_PTR_GET(ComputeCommandLists[i])) };
	ComputeCommandQueue->ExecuteCommandLists(static_cast<UINT>(std::size(CLs)), std::data(CLs));
}
void DX::Present()
{
	//constexpr UINT SyncInterval = 0; //!< 垂直同期を待たない	(ティアリング発生) (Not wait sync)
	constexpr UINT SyncInterval = 1; //!< 垂直同期を待つ (Wait sync)
	
	//!< Present() コールでバックバッファのインデックスが更新される
	VERIFY_SUCCEEDED(SwapChain.DxSwapChain->Present(SyncInterval, 0));
}
void DX::Draw()
{
	//!< CPU と GPU の同期 (Synchronization between cpu and gpu)
	WaitForFence(COM_PTR_GET(GraphicsCommandQueue), COM_PTR_GET(GraphicsFence));

	//!< コンスタントバッファの更新等
	OnUpdate(GetCurrentBackBufferIndex());

	SubmitGraphics(GetCurrentBackBufferIndex());
	
	Present();
}
void DX::Dispatch()
{
	WaitForFence(COM_PTR_GET(ComputeCommandQueue), COM_PTR_GET(ComputeFence));

	SubmitCompute(0);
}



