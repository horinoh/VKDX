#include "DX.h"

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "dxgi.lib")

#ifdef USE_DXC
//!< DXC (dxcompiler.dll が無いと怒られる場合は C:\Program Files (x86)\Windows Kits\10\bin\10.0.18362.0\x64 とかに存在するので、環境変数 Path に通しておく必要がある)
#pragma comment(lib, "dxcompiler")
#endif

const std::array FeatureLevels = {
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

	CreateCommandList();

	//InitializeSwapchainImage(COM_PTR_GET(CommandAllocators[0]), &DirectX::Colors::Red);

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

	//!< デスクリプタヒープ
	CreateDescriptorHeap();
	//!< サンプラ ... デスクリプタが必要
	CreateSampler();

	//!< デスクリプタビュー ... この時点でリソース、デスクリプタヒープ等が必要
	CreateDescriptorView();

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
	for (auto i = 0; i < size(GraphicsCommandLists); ++i) {
		PopulateCommandList(i);
	}
}

void DX::OnDestroy(HWND hWnd, HINSTANCE hInstance)
{
	Super::OnDestroy(hWnd, hInstance);

	//!< GPUが完了するまでここで待機 (Wait GPU)
	WaitForFence();
}

const char* DX::GetFormatChar(const DXGI_FORMAT Format)
{
#define DXGI_FORMAT_ENTRY(df) case DXGI_FORMAT_##df: return #df;
	switch (Format)
	{
	default: assert(0 && "Unknown DXGI_FORMAT"); return "";
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

void DX::CreateBufferResource(ID3D12Resource** Resource, const size_t Size, const D3D12_HEAP_TYPE HT)
{
	const D3D12_RESOURCE_DESC RD = {
		.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER,
		.Alignment = 0,
		.Width = Size, .Height = 1, //!< Widthに「バッファサイズ」を指定して、Heightは1にしておく (Set buffer size to Width, set Height to 1)
		.DepthOrArraySize = 1, .MipLevels = 1,
		.Format = DXGI_FORMAT_UNKNOWN,
		.SampleDesc = DXGI_SAMPLE_DESC({ .Count = 1, .Quality = 0 }),
		.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR, //!< Widthに「バッファサイズ」を指定しているので ROW_MAJOR
		.Flags = D3D12_RESOURCE_FLAG_NONE
	};
	const D3D12_HEAP_PROPERTIES HP = {
		.Type = HT,
		.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
		.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN,
		.CreationNodeMask = 0, .VisibleNodeMask = 0 //!< マルチGPUの場合に使用(1つしか使わない場合は0で良い)
	};
	VERIFY_SUCCEEDED(Device->CreateCommittedResource(&HP, D3D12_HEAP_FLAG_NONE, &RD, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(Resource)));
}

void DX::CreateTextureResource(ID3D12Resource** Resource, const DXGI_FORMAT Format, const UINT64 Width, const UINT Height, const UINT16 DepthOrArraySize, const UINT16 MipLevels)
{
	const D3D12_RESOURCE_DESC RD = {
		.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D, //!< TEXTURE3Dが使いたい場合は要考慮 #DX_TODO
		.Alignment = 0,
		.Width = Width, .Height = Height,
		.DepthOrArraySize = DepthOrArraySize, .MipLevels = MipLevels,
		.Format = Format,
		.SampleDesc = DXGI_SAMPLE_DESC({.Count = 1, .Quality = 0 }),
		.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN,
		.Flags = D3D12_RESOURCE_FLAG_NONE
	};
	const D3D12_HEAP_PROPERTIES HP = {
		.Type = D3D12_HEAP_TYPE_DEFAULT,
		.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
		.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN,
		.CreationNodeMask = 0, .VisibleNodeMask = 0 //!< マルチGPUの場合に使用(1つしか使わない場合は0で良い)
	};
	VERIFY_SUCCEEDED(Device->CreateCommittedResource(&HP,
		D3D12_HEAP_FLAG_ALLOW_ONLY_NON_RT_DS_TEXTURES,
		&RD,
		D3D12_RESOURCE_STATE_GENERIC_READ, //!< GENERIC_READ にすること (Must be GENERIC_READ)
		nullptr,
		IID_PPV_ARGS(Resource)
	));
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
	assert(!(RD.Flags & (D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET | D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL)) && "pOptimizedClearValue を使用しない");
	VERIFY_SUCCEEDED(Device->CreateCommittedResource(&HP, D3D12_HEAP_FLAG_NONE, &RD, RS, nullptr, IID_PPV_ARGS(Resource)));
}
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
	assert(RD.Flags & (D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET | D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL) && "RENDER_TARGET, DEPTH_STENCIL の場合 pOptimizedClearValue を使用する");
	VERIFY_SUCCEEDED(Device->CreateCommittedResource(&HP, D3D12_HEAP_FLAG_NONE, &RD, RS, &CV, IID_PPV_ARGS(Resource)));
}

void DX::CopyToUploadResource(ID3D12Resource* Resource, const size_t Size, const void* Source, const D3D12_RANGE* Range)
{
	if (nullptr != Resource && Size && nullptr != Source) [[likely]] {
		BYTE* Data;
		VERIFY_SUCCEEDED(Resource->Map(0, Range, reinterpret_cast<void**>(&Data))); {
			memcpy(Data, Source, Size);
		} Resource->Unmap(0, nullptr);
	}
}
void DX::CopyToUploadResource(ID3D12Resource* Resource, const std::vector<D3D12_PLACED_SUBRESOURCE_FOOTPRINT>& PSF, const std::vector<UINT>& NumRows, const std::vector<UINT64>& RowSizes, const std::vector<D3D12_SUBRESOURCE_DATA>& SubresourceData)
{
	if (nullptr != Resource) [[likely]] {
		BYTE* Data;
		VERIFY_SUCCEEDED(Resource->Map(0, nullptr, reinterpret_cast<void**>(&Data))); {
			for (auto i = 0; i < size(PSF); ++i) {
				const auto& SD = SubresourceData[i];
				const auto RowCount = NumRows[i];
				const auto RowSize = RowSizes[i];
				const D3D12_MEMCPY_DEST MemcpyDest = {
					.pData = Data + PSF[i].Offset,
					.RowPitch = PSF[i].Footprint.RowPitch,
					.SlicePitch = static_cast<SIZE_T>(PSF[i].Footprint.RowPitch) * RowCount
				};
				for (UINT j = 0; j < PSF[i].Footprint.Depth; ++j) {
					auto Dst = reinterpret_cast<BYTE*>(MemcpyDest.pData) + MemcpyDest.SlicePitch * j;
					const auto Src = reinterpret_cast<const BYTE*>(SD.pData) + SD.SlicePitch * j;
					for (UINT k = 0; k < RowCount; ++k) {
						memcpy(Dst + MemcpyDest.RowPitch * k, Src + SD.RowPitch * k, RowSize);
					}
				}
			}
		} Resource->Unmap(0, nullptr);
	}
}

void DX::CreateBufferResourceAndExecuteCopyCommand(ID3D12Resource** Resource, ID3D12Device* Device, const size_t Size, ID3D12GraphicsCommandList* GCL, ID3D12CommandAllocator* CA, ID3D12CommandQueue* CQ, ID3D12Fence* Fence, const void* Source)
{
	CreateBufferResource(Resource, Device, Size, D3D12_RESOURCE_FLAG_NONE, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_GENERIC_READ);

	COM_PTR<ID3D12Resource> Upload;
	CreateBufferResource(COM_PTR_PUT(Upload), Device, Size, D3D12_RESOURCE_FLAG_NONE, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_GENERIC_READ, Source);
	VERIFY_SUCCEEDED(GCL->Reset(CA, nullptr)); {
		PopulateCommandList_CopyBufferRegion(GCL, COM_PTR_GET(Upload), *Resource, Size, D3D12_RESOURCE_STATE_GENERIC_READ);
	} VERIFY_SUCCEEDED(GCL->Close());

	ExecuteAndWait(CQ, GCL, Fence);
}

void DX::PopulateCommandList_CopyBufferRegion(ID3D12GraphicsCommandList* GCL, ID3D12Resource* Src, ID3D12Resource* Dst, const UINT64 Size, const D3D12_RESOURCE_STATES RS)
{
	{
		const std::array RBs = {
			D3D12_RESOURCE_BARRIER({
				.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION,
				.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE,
				.Transition = D3D12_RESOURCE_TRANSITION_BARRIER({
					.pResource = Dst,
					.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES,
					.StateBefore = D3D12_RESOURCE_STATE_GENERIC_READ, .StateAfter = D3D12_RESOURCE_STATE_COPY_DEST
				})
			})
		};
		GCL->ResourceBarrier(static_cast<UINT>(size(RBs)), data(RBs));
	}
	GCL->CopyBufferRegion(Dst, 0, Src, 0, Size);
	{
		const std::array RBs = {
			D3D12_RESOURCE_BARRIER({
				.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION,
				.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE,
				.Transition = D3D12_RESOURCE_TRANSITION_BARRIER({
					.pResource = Dst,
					.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES,
					.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST, .StateAfter = RS
				})
			})
		};
		GCL->ResourceBarrier(static_cast<UINT>(size(RBs)), data(RBs));
	}
}
void DX::PopulateCommandList_CopyBufferRegion(ID3D12GraphicsCommandList* GCL, ID3D12Resource* Src, ID3D12Resource* Dst, const std::vector<D3D12_PLACED_SUBRESOURCE_FOOTPRINT>& PSF, const D3D12_RESOURCE_STATES RS)
{
	{
		const std::array RBs = {
			D3D12_RESOURCE_BARRIER({
				.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION,
				.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE,
				.Transition = D3D12_RESOURCE_TRANSITION_BARRIER({
					.pResource = Dst,
					.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES,
					.StateBefore = D3D12_RESOURCE_STATE_GENERIC_READ, .StateAfter = D3D12_RESOURCE_STATE_COPY_DEST
				})
			})
		};
		GCL->ResourceBarrier(static_cast<UINT>(size(RBs)), data(RBs));
	}
	for (auto i : PSF) {
		GCL->CopyBufferRegion(Dst, 0, Src, i.Offset, i.Footprint.Width);
	}
	{
		const std::array RBs = {
			D3D12_RESOURCE_BARRIER({
				.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION,
				.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE,
				.Transition = D3D12_RESOURCE_TRANSITION_BARRIER({
					.pResource = Dst,
					.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES,
					.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST, .StateAfter = RS
				})
			})
		};
		GCL->ResourceBarrier(static_cast<UINT>(size(RBs)), data(RBs));
	}
}
void DX::PopulateCommandList_CopyTextureRegion(ID3D12GraphicsCommandList* GCL, ID3D12Resource* Src, ID3D12Resource* Dst, const std::vector<D3D12_PLACED_SUBRESOURCE_FOOTPRINT>& PSF, const D3D12_RESOURCE_STATES RS)
{
	//!< LoadDDSTextureFromFile() で作成されるリソースのステートは既に D3D12_RESOURCE_STATE_COPY_DESTで 作成されている (Resource created by LoadDDSTextureFromFile()'s state is already D3D12_RESOURCE_STATE_COPY_DEST)
	for (UINT i = 0; i < size(PSF); ++i) {
		const D3D12_TEXTURE_COPY_LOCATION TCL_Dst = {
			.pResource = Dst,
			.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX,
			.SubresourceIndex = i
		};
		const D3D12_TEXTURE_COPY_LOCATION TCL_Src = {
			.pResource = Src,
			.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT,
			.PlacedFootprint = PSF[i]
		};
		//const D3D12_BOX Box = { .left = static_cast<UINT>(PSF[i].Offset), .top = 0, .front = 0, .right = static_cast<UINT>(PSF[i].Offset) + PSF[i].Footprint.Width, .bottom = 1, .back = 1, };
		GCL->CopyTextureRegion(&TCL_Dst, 0, 0, 0, &TCL_Src, nullptr);
	}
	{
		const std::array RBs = {
			D3D12_RESOURCE_BARRIER({
				.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION,
				.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE,
				.Transition = D3D12_RESOURCE_TRANSITION_BARRIER({
					.pResource = Dst,
					.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES,
					.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST, .StateAfter = RS
				})
			})
		};
		GCL->ResourceBarrier(static_cast<UINT>(size(RBs)), data(RBs));
	}
}

void DX::ExecuteAndWait(ID3D12CommandQueue* CQ, ID3D12CommandList* CL, ID3D12Fence* Fence)
{
	const std::array CLs = { CL };
	CQ->ExecuteCommandLists(static_cast<UINT>(size(CLs)), data(CLs));

	WaitForFence(CQ, Fence);
	//WaitForFence();
}

void DX::CreateDevice([[maybe_unused]] HWND hWnd)
{
#if defined(_DEBUG) || defined(USE_PIX)
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

	//!< GPUバリデーション ... CPUバリデーションが効かなくなるので採用しない…
	//COM_PTR<ID3D12Debug1> Debug1;
	//VERIFY_SUCCEEDED(Debug->QueryInterface(COM_PTR_UUIDOF_PUTVOID(Debug1)));
	//Debug1->SetEnableGPUBasedValidation(true);
#endif

#ifdef _DEBUG
	//!< CreateDXGIFactory2()では引数にフラグを取ることができるので DXGI_CREATE_FACTORY_DEBUG フラグを指定 (CreateDXGIFactory2() can specify flag argument, here use DXGI_CREATE_FACTORY_DEBUG)
	VERIFY_SUCCEEDED(CreateDXGIFactory2(DXGI_CREATE_FACTORY_DEBUG, COM_PTR_UUIDOF_PUTVOID(Factory)));
#else
	VERIFY_SUCCEEDED(CreateDXGIFactory1(COM_PTR_UUIDOF_PUTVOID(Factory)));
#endif

#ifdef _DEBUG
	//!< アダプター(GPU)の列挙 (Enumerate adapter(GPU))
	EnumAdapter(COM_PTR_GET(Factory));
#endif

#ifdef USE_WARP
	//!< ソフトウエアラスタライザ (Software rasterizer)
	VERIFY_SUCCEEDED(Factory->EnumWarpAdapter(COM_PTR_UUIDOF_PUTVOID(Adapter)));
#else
	//!< アダプター(GPU)の選択、ここでは最大メモリのアダプターを選択することにする (Select Adapter(GPU), here select max memory size adapter)
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
	assert(nullptr != Adapter && "");
	Log("[ Selected Aadapter ]\n");
	LogAdapter(COM_PTR_GET(Adapter));

	//!< アウトプット(ディスプレイ)の選択、ここでは全てのアダプター、アウトプットを列挙して、最初に見つかったアウトプットを選択することにする
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
	assert(nullptr != Output && "");
	Log("\t\t[ Selected Output ]\n");
	LogOutput(COM_PTR_GET(Output));
	
	//!< (フォーマット指定で)選択したアウトプットのディスプレイモードを列挙
	GetDisplayModeList(COM_PTR_GET(Output), DXGI_FORMAT_R8G8B8A8_UNORM);

	//!< 実験的機能、OS が Developer Mode でないとダメみたい
	const std::array Experimental = { D3D12ExperimentalShaderModels, /*D3D12RaytracingPrototype*/ };
	if (FAILED(D3D12EnableExperimentalFeatures(static_cast<UINT>(size(Experimental)), data(Experimental), nullptr, nullptr))) {
		Warning("\tD3D12EnableExperimentalFeatures() failed\n");
	}

	//!< 高フィーチャーレベル優先でデバイスを作成 (Create device with higher feature level possible)
	for (const auto i : FeatureLevels) {
		if (SUCCEEDED(D3D12CreateDevice(COM_PTR_GET(Adapter), i, COM_PTR_UUIDOF_PUTVOID(Device)))) {
			break;
		}
	}
	CheckFeatureLevel(COM_PTR_GET(Device));

	//!< ID3D12InfoQueue 関連はデバイス作成後 (ID3D12InfoQueue after device creation)
	{
		COM_PTR<ID3D12InfoQueue> InfoQueue;
		VERIFY_SUCCEEDED(Device->QueryInterface(COM_PTR_UUIDOF_PUTVOID(InfoQueue)));
		if (nullptr != InfoQueue) {
			//!< エラー等でブレークする設定 (Break with error)
#ifdef _DEBUG
			VERIFY_SUCCEEDED(InfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, TRUE));
			VERIFY_SUCCEEDED(InfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, TRUE));
#endif

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
	if (Count) [[likely]] {
		Logf("\t\t\t[ DisplayModes ] : %s\n", GetFormatChar(Format));

		std::vector<DXGI_MODE_DESC> MDs(Count);
		VERIFY_SUCCEEDED(Outp->GetDisplayModeList(Format, 0, &Count, data(MDs)));
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
	//!< NumFeatureLevels, pFeatureLevelsRequested は CheckFeatureSupport() への入力、MaxSupportedFeatureLevel は CheckFeatureSupport() からの出力となる (NumFeatureLevels, pFeatureLevelsRequested is input to CheckFeatureSupport(), MaxSupportedFeatureLevel is output from CheckFeatureSupport())
	D3D12_FEATURE_DATA_FEATURE_LEVELS FDFL = {
		.NumFeatureLevels = static_cast<UINT>(size(FeatureLevels)), .pFeatureLevelsRequested = data(FeatureLevels) //!< 入力 (Input)
		//.MaxSupportedFeatureLevel //!< 出力 (Output)
	};
	VERIFY_SUCCEEDED(Dev->CheckFeatureSupport(D3D12_FEATURE_FEATURE_LEVELS, reinterpret_cast<void*>(&FDFL), sizeof(FDFL)));

	Log("MaxSupportedFeatureLevel\n");
#define D3D_FEATURE_LEVEL_ENTRY(fl) case D3D_FEATURE_LEVEL_##fl: Logf("\tD3D_FEATURE_LEVEL_%s\n", #fl); break;
	switch (FDFL.MaxSupportedFeatureLevel) {
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
	Logf("MultiSample for %s\n", GetFormatChar(Format));

	SampleDescs.clear();
	for (UINT i = 1; i < D3D12_MAX_MULTISAMPLE_SAMPLE_COUNT; ++i) {
		//!< Format, SampleCount, Flags は CheckFeatureSupport() への入力、NumQualityLevels は CheckFeatureSupport() からの出力となる
		D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS DataMultiSampleQaualityLevels = {
			.Format = Format, //!< 入力 (Input)
			.SampleCount = i, //!< 入力 (Input)
			.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE, //!< 入力 (Input)
			.NumQualityLevels = 0 //!< 出力 (Output)
		};
		VERIFY_SUCCEEDED(Device->CheckFeatureSupport(D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS, reinterpret_cast<void*>(&DataMultiSampleQaualityLevels), sizeof(DataMultiSampleQaualityLevels)));
		//!< 0 == NumQualityLevels の場合はサポートされていないということ
		if (DataMultiSampleQaualityLevels.NumQualityLevels) [[likely]] {
			const DXGI_SAMPLE_DESC SampleDesc = {
				.Count = DataMultiSampleQaualityLevels.SampleCount,
				.Quality = DataMultiSampleQaualityLevels.NumQualityLevels - 1
			}; 
			SampleDescs.emplace_back(SampleDesc);

			Logf("\tCount = %d, Quality = %d\n", SampleDesc.Count, SampleDesc.Quality);
		}
	}
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
			.NodeMask = 0 //!< マルチGPUの場合に使用(1つしか使わない場合は0で良い)
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

//!< CPU と GPU の同期用
void DX::CreateFence()
{
	VERIFY_SUCCEEDED(Device->CreateFence(0, D3D12_FENCE_FLAG_NONE, COM_PTR_UUIDOF_PUTVOID(Fence)));

	LOG_OK();
}

//!< ここではデフォルト実装として、ダイレクト、バンドル共にスワップチェイン数分用意することとする
void DX::CreateCommandList()
{
	//!< コマンド実行(CL->ExecuteCommandList())後、GPUがコマンドアロケータの参照を終えるまで、アロケータのリセット(CA->Reset())してはいけない、アロケータが覚えているのでコマンドのリセット(CL->Reset())はしても良い
	//!< (ここでは)ダイレクト用1つ、バンドル用1つのコマンドアロケータ作成をデフォルト実装とする
	VERIFY_SUCCEEDED(Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, COM_PTR_UUIDOF_PUTVOID(CommandAllocators.emplace_back())));
	VERIFY_SUCCEEDED(Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_BUNDLE, COM_PTR_UUIDOF_PUTVOID(BundleCommandAllocators.emplace_back())));

	DXGI_SWAP_CHAIN_DESC1 SCD;
	SwapChain->GetDesc1(&SCD);
	for (UINT i = 0; i < SCD.BufferCount; ++i) {
		//!< 描画コマンドを発行するコマンドリストにはパイプラインステートの指定が必要だが、後からでも指定(CL->Reset(CA, COM_PTR_GET(PS)))できるので、ここではnullptrを指定
		VERIFY_SUCCEEDED(Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, COM_PTR_GET(CommandAllocators[0]), nullptr, COM_PTR_UUIDOF_PUTVOID(GraphicsCommandLists.emplace_back())));
		VERIFY_SUCCEEDED(GraphicsCommandLists.back()->Close());

		VERIFY_SUCCEEDED(Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_BUNDLE, COM_PTR_GET(BundleCommandAllocators[0]), nullptr, COM_PTR_UUIDOF_PUTVOID(BundleGraphicsCommandLists.emplace_back())));
		VERIFY_SUCCEEDED(BundleGraphicsCommandLists.back()->Close());
	}
}

void DX::CreateSwapchain(HWND hWnd, const DXGI_FORMAT ColorFormat)
{
	CreateSwapChain(hWnd, ColorFormat, GetClientRectWidth(), GetClientRectHeight());

	//!< リソースを取得、ビューを作成 (Get resource, create view)
	GetSwapChainResource();
}
void DX::CreateSwapChain(HWND hWnd, const DXGI_FORMAT ColorFormat, const UINT Width, const UINT Height)
{
	const UINT BufferCount = 3;

	//!< 最適なフルスクリーンのパフォーマンスを得るには、IDXGIOutput->GetDisplayModeList() で取得する(ディスプレイのサポートする)DXGI_MODE_DESC でないとダメなので注意  #DX_TODO
	const DXGI_MODE_DESC MD = {
		.Width = Width, .Height = Height,
		.RefreshRate = DXGI_RATIONAL({.Numerator = 60, .Denominator = 1 }),
		.Format = ColorFormat,
		.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED,
		.Scaling = DXGI_MODE_SCALING_UNSPECIFIED
	};
	const auto& SampleDesc = SampleDescs[0];
	DXGI_SWAP_CHAIN_DESC SCD = {
		.BufferDesc = MD,
		.SampleDesc = SampleDesc,
		.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
		.BufferCount = BufferCount,
		.OutputWindow = hWnd,
		.Windowed = TRUE,
		.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD,
		.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH //!< フルスクリーンにした時、最適なディスプレイモードが選択されるのを許可
	};
	//!< セッティングを変更してスワップチェインを再作成できるように、既存のを開放している
	COM_PTR_RESET(SwapChain);
	COM_PTR<IDXGISwapChain> NewSwapChain;
	VERIFY_SUCCEEDED(Factory->CreateSwapChain(COM_PTR_GET(GraphicsCommandQueue), &SCD, COM_PTR_PUT(NewSwapChain)));
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

	const D3D12_DESCRIPTOR_HEAP_DESC DHD = {
		.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV,
		.NumDescriptors = SCD.BufferCount,
		.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE,
		.NodeMask = 0 //!< マルチGPUの場合に使用(1つしか使わない場合は0で良い)
	};
	VERIFY_SUCCEEDED(Device->CreateDescriptorHeap(&DHD, COM_PTR_UUIDOF_PUTVOID(SwapChainDescriptorHeap)));

	LOG_OK();
}
void DX::GetSwapChainResource()
{
	DXGI_SWAP_CHAIN_DESC1 SCD;
	SwapChain->GetDesc1(&SCD);

#ifdef USE_GAMMA_CORRECTION
	const D3D12_RENDER_TARGET_VIEW_DESC RTVD = {
		.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, //!< ガンマ補正あり
		.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D,
		.Texture2D = D3D12_TEX2D_RTV({ .MipSlice = 0, .PlaneSlice = 0 })
	};
#endif

	auto CDH = SwapChainDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	for (UINT i = 0; i < SCD.BufferCount; ++i) {
		SwapChainResources.emplace_back(COM_PTR<ID3D12Resource>());
		//!< スワップチェインのバッファリソースを SwapChainResources へ取得
		VERIFY_SUCCEEDED(SwapChain->GetBuffer(i, COM_PTR_UUIDOF_PUTVOID(SwapChainResources.back())));

		//!< デスクリプタ(ビュー)の作成。リソース上でのオフセットを指定して作成している、結果が変数等に返るわけではない
		//!< (リソースがタイプドフォーマットなら D3D12_RENDER_TARGET_VIEW_DESC* へ nullptr 指定可能)
#ifdef USE_GAMMA_CORRECTION
		Device->CreateRenderTargetView(COM_PTR_GET(SwapChainResources.back()), &RTVD, CDH);
#else
		Device->CreateRenderTargetView(COM_PTR_GET(SwapChainResources.back()), nullptr, CDH);
#endif
		CDH.ptr += Device->GetDescriptorHandleIncrementSize(SwapChainDescriptorHeap->GetDesc().Type);
	}

	LOG_OK();
}

#if 0
void DX::InitializeSwapchainImage(ID3D12CommandAllocator* CommandAllocator, const DirectX::XMVECTORF32* Color)
{
	assert(nullptr != Color && "");

	DXGI_SWAP_CHAIN_DESC1 SCD;
	SwapChain->GetDesc1(&SCD);

	auto CDH = SwapChainDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	for (UINT i = 0; i < SCD.BufferCount; ++i) {
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

	const std::array CLs = { static_cast<ID3D12CommandList*>(COM_PTR_GET(GraphicsCommandLists[0])) };
	CommandQueue->ExecuteCommandLists(static_cast<UINT>(size(CLs)), data(CLs));

	WaitForFence();

	LOG_OK();
}
#endif

void DX::ResizeSwapChain(const UINT Width, const UINT Height)
{
	ResetSwapChainResource();

	DXGI_SWAP_CHAIN_DESC1 SCD;
	SwapChain->GetDesc1(&SCD);
	VERIFY_SUCCEEDED(SwapChain->ResizeBuffers(SCD.BufferCount, Width, Height, SCD.Format, SCD.Flags));
	Log("\tResizeBuffers\n");

	//!< リソースを取得、ビューを作成 (Get resource, create view)
	GetSwapChainResource();

	LOG_OK();
}

void DX::ResizeDepthStencil(const DXGI_FORMAT /*DepthFormat*/, const UINT /*Width*/, const UINT /*Height*/)
{
	//COM_PTR_RESET(DepthStencilResource);
	//CreateDepthStencilResource(DepthFormat, Width, Height);

	LOG_OK();
}

#pragma region RAYTRACING
void DX::BuildAccelerationStructure(ID3D12Device* Device, const UINT64 SBSize, const D3D12_GPU_VIRTUAL_ADDRESS GVA, const D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS& BRASI, ID3D12GraphicsCommandList* GCL, ID3D12CommandAllocator* CA, ID3D12CommandQueue* CQ, ID3D12Fence* Fence)
{
	ScratchBuffer SB;
	SB.Create(Device, SBSize);
	COM_PTR<ID3D12GraphicsCommandList4> GCL4;
	VERIFY_SUCCEEDED(GCL->QueryInterface(COM_PTR_UUIDOF_PUTVOID(GCL4)));
	VERIFY_SUCCEEDED(GCL4->Reset(CA, nullptr)); {
		const D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC BRASD = {
		.DestAccelerationStructureData = GVA,
		.Inputs = BRASI,
		.SourceAccelerationStructureData = 0,
		.ScratchAccelerationStructureData = COM_PTR_GET(SB.Resource)->GetGPUVirtualAddress()
		};
		GCL4->BuildRaytracingAccelerationStructure(&BRASD, 0, nullptr);
	} VERIFY_SUCCEEDED(GCL4->Close());
	ExecuteAndWait(CQ, static_cast<ID3D12CommandList*>(GCL), Fence);
}
#pragma endregion

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

template<> void DX::SerializeRootSignature(COM_PTR<ID3DBlob>& Blob, const std::vector<D3D12_ROOT_PARAMETER>& RPs, const std::vector<D3D12_STATIC_SAMPLER_DESC>& SSDs, const D3D12_ROOT_SIGNATURE_FLAGS Flags)
{
	D3D12_FEATURE_DATA_ROOT_SIGNATURE FDRS = { .HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0 };
	VERIFY_SUCCEEDED(Device->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, reinterpret_cast<void*>(&FDRS), sizeof(FDRS)));
	assert(FDRS.HighestVersion >= D3D_ROOT_SIGNATURE_VERSION_1_0 && "");

#if 1
	const D3D12_VERSIONED_ROOT_SIGNATURE_DESC VRSD = {
		.Version = D3D_ROOT_SIGNATURE_VERSION_1_0,
		.Desc_1_0 = D3D12_ROOT_SIGNATURE_DESC({
			.NumParameters = static_cast<UINT>(size(RPs)), .pParameters = data(RPs),
			.NumStaticSamplers = static_cast<UINT>(size(SSDs)), .pStaticSamplers = data(SSDs),
			.Flags = Flags
		}),
	};
	COM_PTR<ID3DBlob> ErrorBlob;
	VERIFY_SUCCEEDED(D3D12SerializeVersionedRootSignature(&VRSD, COM_PTR_PUT(Blob), COM_PTR_PUT(ErrorBlob)));
#else
	const D3D12_ROOT_SIGNATURE_DESC RSD = {
		.NumParameters = static_cast<UINT>(size(RPs)), .pParameters = data(RPs),
		.NumStaticSamplers = static_cast<UINT>(size(SSDs)), .pStaticSamplers = data(SSDs),
		.Flags = Flags
	};
	COM_PTR<ID3DBlob> ErrorBlob;
	VERIFY_SUCCEEDED(D3D12SerializeRootSignature(&VRSD.Desc_1_0, D3D_ROOT_SIGNATURE_VERSION_1_0, COM_PTR_PUT(Blob), COM_PTR_PUT(ErrorBlob)));
#endif
	LOG_OK();
}

//!< [ D3D12_ROOT_SIGNATURE_DESC と D3D12_ROOT_SIGNATURE_DESC1 の違い ]
//!< D3D12_ROOT_SIGNATURE_DESC1 では D3D12_ROOT_PARAMETER -> D3D12_ROOT_PARAMETER1 に変更されている
//!< D3D12_ROOT_PARAMETER1 では D3D12_ROOT_DESCRIPTOR -> D3D12_ROOT_DESCRIPTOR1 に変更されている
//!< D3D12_ROOT_DESCRIPTOR1 では D3D12_ROOT_DESCRIPTOR_FLAGS Flags メンバが増えている
template<> void DX::SerializeRootSignature(COM_PTR<ID3DBlob>& Blob, const std::vector<D3D12_ROOT_PARAMETER1>& RPs, const std::vector<D3D12_STATIC_SAMPLER_DESC>& SSDs, const D3D12_ROOT_SIGNATURE_FLAGS Flags)
{
	D3D12_FEATURE_DATA_ROOT_SIGNATURE FDRS = { .HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1 };
	VERIFY_SUCCEEDED(Device->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, reinterpret_cast<void*>(&FDRS), sizeof(FDRS)));
	assert(FDRS.HighestVersion >= D3D_ROOT_SIGNATURE_VERSION_1_1 && "");
	
	const D3D12_VERSIONED_ROOT_SIGNATURE_DESC VRSD = {
		.Version = D3D_ROOT_SIGNATURE_VERSION_1_1,
		.Desc_1_1 = D3D12_ROOT_SIGNATURE_DESC1({
			.NumParameters = static_cast<UINT>(size(RPs)), .pParameters = data(RPs),
			.NumStaticSamplers = static_cast<UINT>(size(SSDs)), .pStaticSamplers = data(SSDs),
			.Flags = Flags
		}),
	};
	COM_PTR<ID3DBlob> ErrorBlob;
	VERIFY_SUCCEEDED(D3D12SerializeVersionedRootSignature(&VRSD, COM_PTR_PUT(Blob), COM_PTR_PUT(ErrorBlob)));
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
	GetRootSignaturePartFromShader(Blob, data(GetBasePath() + TEXT(".rs.cso")));
#else
	SerializeRootSignature(Blob, {}, {}, D3D12_ROOT_SIGNATURE_FLAG_NONE | SHADER_ROOT_ACCESS_DENY_ALL);
#endif

	RootSignatures.emplace_back(COM_PTR<ID3D12RootSignature>());
	VERIFY_SUCCEEDED(Device->CreateRootSignature(0, //!< マルチGPUの場合に使用(1つしか使わない場合は0で良い)
		Blob->GetBufferPointer(), Blob->GetBufferSize(), COM_PTR_UUIDOF_PUTVOID(RootSignatures.back())));

	LOG_OK();
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

	D3D12_SHADER_DESC SD;
	VERIFY_SUCCEEDED(SR->GetDesc(&SD));

	if (0 < SD.ConstantBuffers) { Log("\tConstantBuffers\n"); }
	for (UINT i = 0; i < SD.ConstantBuffers; ++i) {
		const auto CB = SR->GetConstantBufferByIndex(i);
		D3D12_SHADER_BUFFER_DESC SBD;
		VERIFY_SUCCEEDED(CB->GetDesc(&SBD));
		Logf("\t\t%s\n", SBD.Name);
		for (UINT j = 0; j < SBD.Variables; ++j) {
			const auto SRV = CB->GetVariableByIndex(j);
			D3D12_SHADER_VARIABLE_DESC SVD;
			VERIFY_SUCCEEDED(SRV->GetDesc(&SVD));
			D3D12_SHADER_TYPE_DESC STD;
			VERIFY_SUCCEEDED(SRV->GetType()->GetDesc(&STD));
			Logf("\t\t\t%s\t%s\n", SVD.Name, STD.Name);
		}
	}

	if (0 < SD.BoundResources) { Log("\tBoundResources\n"); }
	for (UINT i = 0; i < SD.BoundResources; ++i) {
		D3D12_SHADER_INPUT_BIND_DESC SIBD;
		VERIFY_SUCCEEDED(SR->GetResourceBindingDesc(i, &SIBD));
		Logf("\t\t%s\n", SIBD.Name);

		Log("\t\t\tInput = ");
#define SHADER_INPUT_TYPE_ENTRY(X) case D3D_SIT_##X: Log(#X); break;
		switch (SIBD.Type) {
			SHADER_INPUT_TYPE_ENTRY(CBUFFER)
			SHADER_INPUT_TYPE_ENTRY(TBUFFER)
			SHADER_INPUT_TYPE_ENTRY(TEXTURE)
			SHADER_INPUT_TYPE_ENTRY(SAMPLER)
			SHADER_INPUT_TYPE_ENTRY(UAV_RWTYPED)
			SHADER_INPUT_TYPE_ENTRY(STRUCTURED)
			SHADER_INPUT_TYPE_ENTRY(UAV_RWSTRUCTURED)
			SHADER_INPUT_TYPE_ENTRY(BYTEADDRESS)
			SHADER_INPUT_TYPE_ENTRY(UAV_RWBYTEADDRESS)
			SHADER_INPUT_TYPE_ENTRY(UAV_APPEND_STRUCTURED)
			SHADER_INPUT_TYPE_ENTRY(UAV_CONSUME_STRUCTURED)
			SHADER_INPUT_TYPE_ENTRY(UAV_RWSTRUCTURED_WITH_COUNTER)
			default: break;
		}
#undef SIT_ENTRY
	 	0 < SIBD.BindCount ? Logf("(%d-%d)", SIBD.BindPoint, SIBD.BindCount + SIBD.BindCount) : Logf("(%d)", SIBD.BindPoint);
		Log("\n");

		//!< ReturnType, Dimension はテクスチャの場合のみ
		if (D3D_SIT_TEXTURE == SIBD.Type) {
			Log("\t\t\tReturn = ");
#define RETURN_TYPE_ENTRY(X) case D3D_RETURN_TYPE_##X: Log(#X); break;
			switch (SIBD.ReturnType)
			{
				RETURN_TYPE_ENTRY(UNORM)
				RETURN_TYPE_ENTRY(SNORM)
				RETURN_TYPE_ENTRY(SINT)
				RETURN_TYPE_ENTRY(UINT)
				RETURN_TYPE_ENTRY(FLOAT)
				RETURN_TYPE_ENTRY(MIXED)
				RETURN_TYPE_ENTRY(DOUBLE)
				RETURN_TYPE_ENTRY(CONTINUED)
				default: break;
			}
#undef TYPE_ENTRY
			Log("\n");

			Log("\t\t\tDimension = ");
#define DIMENSION_ENTRY(X) case D3D_SRV_DIMENSION_##X: Log(#X); break;
			switch (SIBD.Dimension)
			{
				DIMENSION_ENTRY(UNKNOWN)
				DIMENSION_ENTRY(BUFFER)
				DIMENSION_ENTRY(TEXTURE1D)
				DIMENSION_ENTRY(TEXTURE1DARRAY)
				DIMENSION_ENTRY(TEXTURE2D)
				DIMENSION_ENTRY(TEXTURE2DARRAY)
				DIMENSION_ENTRY(TEXTURE2DMS)
				DIMENSION_ENTRY(TEXTURE2DMSARRAY)
				DIMENSION_ENTRY(TEXTURE3D)
				DIMENSION_ENTRY(TEXTURECUBE)
				DIMENSION_ENTRY(TEXTURECUBEARRAY)
				DIMENSION_ENTRY(BUFFEREX)
				default: break;
			}
#undef DIMENSION_ENTRY
			Log("\n");
		}
	}

#define COMPONENT_TYPE_ENTRY(X) case D3D_REGISTER_COMPONENT_##X: Log(#X); break;
#define LOG_SIGNATURE_PARAMETER_DESC(X)	Logf("\t\t%s%d\t", X.SemanticName, X.SemanticIndex);\
	switch (X.ComponentType)\
	{\
		COMPONENT_TYPE_ENTRY(UNKNOWN)\
		COMPONENT_TYPE_ENTRY(UINT32)\
		COMPONENT_TYPE_ENTRY(SINT32)\
		COMPONENT_TYPE_ENTRY(FLOAT32)\
		default: break;\
	}\
	Logf("(%s%s%s%s)\n", (X.Mask & 1) ? "X" : "-", (X.Mask & 2) ? "Y" : "-", (X.Mask & 4) ? "Z" : "-", (X.Mask & 8) ? "W" : "-");

	if (0 < SD.InputParameters) { Logf("\tInputParameters\n"); }
	for (UINT i = 0; i < SD.InputParameters; ++i) {
		D3D12_SIGNATURE_PARAMETER_DESC SPD;
		VERIFY_SUCCEEDED(SR->GetInputParameterDesc(i, &SPD));
		LOG_SIGNATURE_PARAMETER_DESC(SPD);
	}
	if (0 < SD.OutputParameters) { Logf("\tOutputParameters\n"); }
	for (UINT i = 0; i < SD.OutputParameters; ++i) {
		D3D12_SIGNATURE_PARAMETER_DESC SPD;
		VERIFY_SUCCEEDED(SR->GetOutputParameterDesc(i, &SPD));
		LOG_SIGNATURE_PARAMETER_DESC(SPD);
	}
#undef LOG_SIGNATURE_PARAMETER_DESC
#undef COMPONENT_TYPE_ENTRY
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
		if (SUCCEEDED(D3DSetBlobPart(Blob->GetBufferPointer(), Blob->GetBufferSize(), D3D_BLOB_DEBUG_NAME, 0, data(Buf), size(Buf), COM_PTR_PUT(NewBlob)))) {
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

void DX::CreatePipelineState_(COM_PTR<ID3D12PipelineState>& PST, ID3D12Device* Device, ID3D12RootSignature* RS,
	const D3D12_PRIMITIVE_TOPOLOGY_TYPE Topology,
	const std::vector<D3D12_RENDER_TARGET_BLEND_DESC>& RTBDs,
	const D3D12_RASTERIZER_DESC& RD,
	const D3D12_DEPTH_STENCIL_DESC& DSD,
	const D3D12_SHADER_BYTECODE VS, const D3D12_SHADER_BYTECODE PS, const D3D12_SHADER_BYTECODE DS, const D3D12_SHADER_BYTECODE HS, const D3D12_SHADER_BYTECODE GS,
	const std::vector<D3D12_INPUT_ELEMENT_DESC>& IEDs,
	const std::vector<DXGI_FORMAT>& RtvFormats,
	const PipelineLibrarySerializer* PLS, LPCWSTR Name)
{
	PERFORMANCE_COUNTER();

	assert((VS.pShaderBytecode != nullptr && VS.BytecodeLength) && "");

	//!< キャッシュドパイプラインステート (CachedPipelineState)
	//!< (VK の VkGraphicsPipelineCreateInfo.basePipelineHandle, basePipelineIndex 相当?)
	COM_PTR<ID3DBlob> CachedBlob;
	if (nullptr != PST) {
		VERIFY_SUCCEEDED(PST->GetCachedBlob(COM_PTR_PUT(CachedBlob)));
	}

	//!< DXでは「パッチコントロールポイント」個数の指定はIASetPrimitiveTopology()の引数として「コマンドリスト作成時」に指定する、VKとは結構異なるので注意
	//!< CL->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_1_CONTROL_POINT_PATCHLIST);	

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
		.InputLayout = D3D12_INPUT_LAYOUT_DESC({ .pInputElementDescs = data(IEDs), .NumElements = static_cast<UINT>(size(IEDs)) }),
		.IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED,
		.PrimitiveTopologyType = Topology,
		.NumRenderTargets = static_cast<UINT>(size(RtvFormats)), .RTVFormats = {}, 
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
	assert(size(RTBDs) <= _countof(GPSD.BlendState.RenderTarget) && "");
	std::ranges::copy(RTBDs, GPSD.BlendState.RenderTarget);
	//!< TRUE == IndependentBlendEnable の場合はレンダーターゲットの分だけ用意すること (If TRUE == IndependentBlendEnable, need NumRenderTarget elements)
	assert((false == GPSD.BlendState.IndependentBlendEnable || size(RTBDs) == GPSD.NumRenderTargets) && "");
	assert(GPSD.NumRenderTargets <= _countof(GPSD.RTVFormats) && "");
	std::ranges::copy(RtvFormats, GPSD.RTVFormats);

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

void DX::CreateTexture1x1(const UINT32 Color, const D3D12_RESOURCE_STATES RS)
{
	ImageResources.emplace_back(COM_PTR<ID3D12Resource>());
	constexpr auto RD = D3D12_RESOURCE_DESC({
		.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D,
		.Alignment = 0,
		.Width = 1, .Height = 1, 
		.DepthOrArraySize = 1,
		.MipLevels = 1,
		.Format = DXGI_FORMAT_R8G8B8A8_UNORM,
		.SampleDesc = DXGI_SAMPLE_DESC({.Count = 1, .Quality = 0 }),
		.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN,
		.Flags = D3D12_RESOURCE_FLAG_NONE
		});
	const D3D12_HEAP_PROPERTIES HP = {
		.Type = D3D12_HEAP_TYPE_DEFAULT,
		.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
		.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN,
		.CreationNodeMask = 0, .VisibleNodeMask = 0
	};
	//!< D3D12_RESOURCE_STATE_COPY_DEST にすること
	VERIFY_SUCCEEDED(Device->CreateCommittedResource(&HP, D3D12_HEAP_FLAG_NONE, &RD, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, COM_PTR_UUIDOF_PUTVOID(ImageResources.back()))); 

	//!< ビューの作成 (Create view)
	ShaderResourceViewDescs.emplace_back(D3D12_SHADER_RESOURCE_VIEW_DESC({
		.Format = ImageResources.back()->GetDesc().Format,
		.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D,
		.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING,
		.Texture2D = D3D12_TEX2D_SRV({.MostDetailedMip = 0, .MipLevels = ImageResources.back()->GetDesc().MipLevels, .PlaneSlice = 0, .ResourceMinLODClamp = 0.0f })
		}));

	{
		constexpr auto PitchSize = 1 * static_cast<UINT32>(sizeof(Color));
		constexpr auto LayerSize = 1 * PitchSize;

		const auto CA = COM_PTR_GET(CommandAllocators[0]);
		const auto CL = COM_PTR_GET(GraphicsCommandLists[0]);

		//!< アップロード用バッファ (Buffer for upload)
		COM_PTR<ID3D12Resource> UploadResource;
		CreateBufferResource(COM_PTR_PUT(UploadResource), LayerSize, D3D12_HEAP_TYPE_UPLOAD);
		CopyToUploadResource(COM_PTR_GET(UploadResource), LayerSize, &Color);

		//!< バッファテクスチャ間転送コマンド (Buffer to image copy command)
		const std::vector PSFs = {
			D3D12_PLACED_SUBRESOURCE_FOOTPRINT({
				.Offset = 0, 
				.Footprint = D3D12_SUBRESOURCE_FOOTPRINT({.Format = RD.Format, .Width = static_cast<UINT>(RD.Width), .Height = RD.Height, .Depth = 1, .RowPitch = static_cast<UINT>(RoundUp(PitchSize, D3D12_TEXTURE_DATA_PITCH_ALIGNMENT)) })
				}),
		};
		VERIFY_SUCCEEDED(CL->Reset(CA, nullptr)); {
			PopulateCommandList_CopyTextureRegion(CL, COM_PTR_GET(UploadResource), COM_PTR_GET(ImageResources.back()), PSFs, RS);
		} VERIFY_SUCCEEDED(CL->Close());

		//!< コマンドの実行 (Execute command)
		ExecuteAndWait(COM_PTR_GET(GraphicsCommandQueue), static_cast<ID3D12CommandList*>(CL), COM_PTR_GET(Fence));
	}
}

//!< ABRG
void DX::CreateTextureArray1x1(const std::vector<UINT32>& Colors, const D3D12_RESOURCE_STATES RS)
{
#pragma region TEX_ARRAY
	const auto Layers = static_cast<UINT32>(size(Colors));
#pragma endregion

	const auto RD = D3D12_RESOURCE_DESC({
		.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D,
		.Alignment = 0,
		.Width = 1, .Height = 1, 
#pragma region TEX_ARRAY
		.DepthOrArraySize = static_cast<UINT16>(Layers),
#pragma endregion
		.MipLevels = 1,
		.Format = DXGI_FORMAT_R8G8B8A8_UNORM,
		.SampleDesc = DXGI_SAMPLE_DESC({.Count = 1, .Quality = 0 }),
		.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN,
		.Flags = D3D12_RESOURCE_FLAG_NONE
		});
	const D3D12_HEAP_PROPERTIES HP = {
		.Type = D3D12_HEAP_TYPE_DEFAULT,
		.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
		.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN,
		.CreationNodeMask = 0, .VisibleNodeMask = 0
	};
	VERIFY_SUCCEEDED(Device->CreateCommittedResource(&HP, D3D12_HEAP_FLAG_NONE, &RD, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, COM_PTR_UUIDOF_PUTVOID(ImageResources.emplace_back())));

	ShaderResourceViewDescs.emplace_back(D3D12_SHADER_RESOURCE_VIEW_DESC({
		.Format = ImageResources.back()->GetDesc().Format,
#pragma region TEX_ARRAY
		.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY,
#pragma endregion
		.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING,
		.Texture2DArray = D3D12_TEX2D_ARRAY_SRV({.MostDetailedMip = 0, .MipLevels = ImageResources.back()->GetDesc().MipLevels, .FirstArraySlice = 0, .ArraySize = Layers, .PlaneSlice = 0, .ResourceMinLODClamp = 0.0f })
	}));

	//Textures.emplace_back().Create(COM_PTR_GET(Device), 1, 1, static_cast<UINT16>(Layers), DXGI_FORMAT_R8G8B8A8_UNORM);

	{
		constexpr auto PitchSize = 1 * static_cast<UINT32>(sizeof(Colors[0]));
		constexpr auto LayerSize = 1 * PitchSize;

		const auto CA = COM_PTR_GET(CommandAllocators[0]);
		const auto CL = COM_PTR_GET(GraphicsCommandLists[0]);

		//!< アラインされたサイズを計算 (Calculate aligned size)
		size_t AlignedSize = 0;
		for (UINT32 i = 0; i < Layers; ++i) {
			AlignedSize = RoundUp(i * LayerSize, D3D12_TEXTURE_DATA_PLACEMENT_ALIGNMENT);
			AlignedSize += LayerSize;
		}
		//!< Colors をアラインされたメモリへコピー (Copy Colors to aligned memory)
		std::vector<std::byte> AlignedData(AlignedSize, std::byte());
		for (UINT32 i = 0; i < Layers; ++i) {
			*reinterpret_cast<UINT32*>(&AlignedData[RoundUp(i * LayerSize, D3D12_TEXTURE_DATA_PLACEMENT_ALIGNMENT)]) = Colors[i];
		}
		//!< (アラインされた)アップロード用バッファを作成 (Aligned buffer for upload)
		COM_PTR<ID3D12Resource> UploadResource;
		CreateBufferResource(COM_PTR_PUT(UploadResource), size(AlignedData), D3D12_HEAP_TYPE_UPLOAD);
		CopyToUploadResource(COM_PTR_GET(UploadResource), size(AlignedData), data(AlignedData));

#pragma region TEX_ARRAY
		std::vector<D3D12_PLACED_SUBRESOURCE_FOOTPRINT> PSFs;
		for (UINT32 i = 0; i < Layers; ++i) {
			PSFs.emplace_back(D3D12_PLACED_SUBRESOURCE_FOOTPRINT({ 
				.Offset = RoundUp(i * LayerSize, D3D12_TEXTURE_DATA_PLACEMENT_ALIGNMENT),
				.Footprint = D3D12_SUBRESOURCE_FOOTPRINT({.Format = RD.Format, .Width = static_cast<UINT>(RD.Width), .Height = RD.Height, .Depth = 1, .RowPitch = static_cast<UINT>(RoundUp(PitchSize, D3D12_TEXTURE_DATA_PITCH_ALIGNMENT)) }) 
				}));
		}
#pragma endregion

		VERIFY_SUCCEEDED(CL->Reset(CA, nullptr)); {
			PopulateCommandList_CopyTextureRegion(CL, COM_PTR_GET(UploadResource), COM_PTR_GET(ImageResources.back()), PSFs, RS);
		} VERIFY_SUCCEEDED(CL->Close());

		ExecuteAndWait(COM_PTR_GET(GraphicsCommandQueue), static_cast<ID3D12CommandList*>(CL), COM_PTR_GET(Fence));
	}
}

#if 0
void DX::CreatePipelineState_Compute()
{
	PERFORMANCE_COUNTER();

	assert(nullptr != RootSignature && "");

	//!< シェーダ
	std::vector<COM_PTR<ID3DBlob>> ShaderBlobs;
	CreateShader(ShaderBlobs);
	assert(!empty(ShaderBlobs) && "");

	const D3D12_CACHED_PIPELINE_STATE CPS = { nullptr, 0 };
	const D3D12_COMPUTE_PIPELINE_STATE_DESC CPSD = {
		COM_PTR_GET(RootSignature),
		D3D12_SHADER_BYTECODE({ ShaderBlobs[0]->GetBufferPointer(), ShaderBlobs[0]->GetBufferSize() }),
		0, //!< マルチGPUの場合に使用(1つしか使わない場合は0で良い)
		CPS,
		D3D12_PIPELINE_STATE_FLAG_NONE
	};
	VERIFY_SUCCEEDED(Device->CreateComputePipelineState(&CPSD, COM_PTR_UUIDOF_PUTVOID(PipelineState)));

	LOG_OK();
}
#endif

#if 0
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

		CL->RSSetViewports(static_cast<UINT>(size(Viewports)), data(Viewports));
		CL->RSSetScissorRects(static_cast<UINT>(size(ScissorRects)), data(ScissorRects));

		ResourceBarrier(CL, SCR, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET); {
			auto CDH = SwapChainDescriptorHeap->GetCPUDescriptorHandleForHeapStart(); CDH.ptr += i * Device->GetDescriptorHandleIncrementSize(SwapChainDescriptorHeap->GetDesc().Type);
			constexpr std::array<D3D12_RECT, 0> Rects = {};
			CL->ClearRenderTargetView(CDH, DirectX::Colors::SkyBlue, static_cast<UINT>(size(Rects)), data(Rects));
		} ResourceBarrier(CL, SCR, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
	}
	VERIFY_SUCCEEDED(CL->Close());
}
#endif

void DX::Draw()
{
	WaitForFence();

	DrawFrame(GetCurrentBackBufferIndex());

	Submit();
	
	Present();
}
void DX::Dispatch()
{
	//!< #DX_TODO Dispatch実装
	DEBUG_BREAK();
}
void DX::WaitForFence()
{
	//!< CPU 側のフェンス値をインクリメント
	++FenceValue;

	//!< コマンドキューに Fencevalue を引数に Signal を追加する (GPU が到達すれば GetCompletedValue() が FenceValue になり、CPUに追いついたことになる)
	VERIFY_SUCCEEDED(GraphicsCommandQueue->Signal(COM_PTR_GET(Fence), FenceValue));
	if (Fence->GetCompletedValue() < FenceValue) {
		auto hEvent = CreateEventEx(nullptr, nullptr, 0, EVENT_ALL_ACCESS);
		if (nullptr != hEvent) [[likely]] {
			//!< GetCompletedValue() が FenceValue になったらイベントが発行される
			VERIFY_SUCCEEDED(Fence->SetEventOnCompletion(FenceValue, hEvent));

			//!< イベント発行まで待つ
			WaitForSingleObject(hEvent, INFINITE);
			CloseHandle(hEvent);
		}
	}
}

void DX::WaitForFence(ID3D12CommandQueue* CQ, ID3D12Fence* Fence)
{
	auto Value = Fence->GetCompletedValue();
	++Value;
	//!< GPUが到達すれば Value になる
	VERIFY_SUCCEEDED(CQ->Signal(Fence, Value));
	if (Fence->GetCompletedValue() < Value) {
		auto hEvent = CreateEventEx(nullptr, nullptr, 0, EVENT_ALL_ACCESS);
		if (nullptr != hEvent) [[likely]] {
			//!< GetCompletedValue() が FenceValue になったらイベントが発行されるようにする
			VERIFY_SUCCEEDED(Fence->SetEventOnCompletion(Value, hEvent));
			//!< イベント発行まで待つ
			WaitForSingleObject(hEvent, INFINITE);
			CloseHandle(hEvent);
		}
	}
}

void DX::Submit()
{
	const std::array CLs = { static_cast<ID3D12CommandList*>(COM_PTR_GET(GraphicsCommandLists[GetCurrentBackBufferIndex()])) };
	GraphicsCommandQueue->ExecuteCommandLists(static_cast<UINT>(size(CLs)), data(CLs));
}
void DX::Present()
{
	//!< 垂直同期を待つ : 1
	VERIFY_SUCCEEDED(SwapChain->Present(1, 0));
}

