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

	//!< �f�o�C�X
	const auto ColorFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
	CreateDevice(hWnd);
	CheckMultiSample(ColorFormat);
	CreateCommandQueue();

	//!< ����
	CreateFence();

	//!< �X���b�v�`�F�C��
	CreateSwapchain(hWnd, ColorFormat);

	//!< �R�}���h
	CreateCommandList();
	InitializeSwapChain();

	//!< �f�v�X
	CreateDepthStencil();
	
	//!< ���_
	CreateVertexBuffer();
	CreateIndexBuffer();
	CreateIndirectBuffer();

	CreateTexture();

	//!< ���[�g�V�O�l�`��(�f�X�N���v�^�Z�b�g���C�A�E�g)
	CreateRootSignature();
	{
		//!< �R���X�^���g�o�b�t�@(���j�t�H�[���o�b�t�@)
		CreateConstantBuffer();
	}
	//!< �f�X�N���v�^
	CreateDescriptorHeap();
	UpdateDescriptorHeap();

	//!< �p�C�v���C��
	CreatePipelineState();

	//CreateUnorderedAccessTexture();

	SetTimer(hWnd, NULL, 1000 / 60, nullptr);

	//!< �E�C���h�E�T�C�Y�ύX���ɍ�蒼������
	OnExitSizeMove(hWnd, hInstance);
}

/**
���T�C�Y����̂���
	BackBuffer ... SwapChain->ResizeBuffers() ���g�p
��蒼������
	DepthStencilBuffer, RenderTargetView, DepthStencilView
*/
void DX::OnExitSizeMove(HWND hWnd, HINSTANCE hInstance)
{
#ifdef _DEBUG
	PerformanceCounter PC("OnExitSizeMove : ");
#endif

	Super::OnExitSizeMove(hWnd, hInstance);

	WaitForFence();

	//const auto CommandList = GraphicsCommandLists[0].Get();
	//const auto CommandAllocator = CommandAllocators[0].Get();

	//VERIFY_SUCCEEDED(CommandList->Reset(CommandAllocator, nullptr));
	//{		
	//	ResizeSwapChain(Rect);
	//	ResizeDepthStencil(Rect);
	//}
	//VERIFY_SUCCEEDED(CommandList->Close());

	//ExecuteCommandListAndWaitForFence(CommandList);

	CreateViewport(Rect);

	for (auto i = 0; i < GraphicsCommandLists.size(); ++i) {
		PopulateCommandList(i);
	}
}

void DX::OnDestroy(HWND hWnd, HINSTANCE hInstance)
{
	Super::OnDestroy(hWnd, hInstance);

	//!< GPU����������܂ł����őҋ@ 
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
		D3D12_HEAP_TYPE_UPLOAD, //!< UPLOAD �ɂ��邱�� Must be UPLOAD
		D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
		D3D12_MEMORY_POOL_UNKNOWN,
		1,
		1
	};
	VERIFY_SUCCEEDED(Device->CreateCommittedResource(&HeapProperties,
		D3D12_HEAP_FLAG_NONE,
		&ResourceDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ, //!< GENERIC_READ �ɂ��邱�� Must be GENERIC_READ
		nullptr,
//#ifdef USE_WINRT
//		__uuidof(Resource), Resource.put_void()
//#elif defined(USE_WRL)
//		IID_PPV_ARGS(Resource.GetAddressOf())
//#endif
		IID_PPV_ARGS(Resource)
	));
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
		D3D12_HEAP_TYPE_DEFAULT, //!< DEFAULT �ɂ��邱�� Must be DEFAULT
		D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
		D3D12_MEMORY_POOL_UNKNOWN,
		1,
		1
	};
	VERIFY_SUCCEEDED(Device->CreateCommittedResource(&HeapProperties,
		D3D12_HEAP_FLAG_NONE,
		&ResourceDesc,
		D3D12_RESOURCE_STATE_COMMON, //!< COMMON �ɂ��邱�� Must be COMMON
		nullptr,
//#ifdef USE_WINRT
//		__uuidof(Resource), Resource.put_void()
//#elif defined(USE_WRL)
//		IID_PPV_ARGS(Resource.GetAddressOf())
//#endif
		IID_PPV_ARGS(Resource)
	));
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
	//!< Dst(LoadDDSTextureFromFile()�ō쐬�����)�̃X�e�[�g�͊���D3D12_RESOURCE_STATE_COPY_DEST�ō쐬����Ă���
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
			//!< �F�X�ȃT���v��������Ƃ��Ƃ��Ƃ� D3D12_BOX ������Ă��邪�ǂ���g���Ă͂��Ȃ�
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

void DX::CreateDevice(HWND hWnd)
{
#ifdef USE_WINRT
#if defined(_DEBUG) || defined(USE_PIX)
	if (SUCCEEDED(DXGIGetDebugInterface1(0, __uuidof(GraphicsAnalysis), GraphicsAnalysis.put_void()))) {
		//!< �O���t�B�b�N�X�f�f�� Alt + F5 �ŋN�������ꍇ�̂ݐ������� (Enabled only if executed with Alt + F5)
		Log("Graphics Analysis is enabled\n");
		//!< GraphicsAnalysis->BeginCapture(), GraphicsAnalysis->EndCapture() �ŃL���v�`���J�n�A�I������
	}
#endif

#ifdef _DEBUG
	winrt::com_ptr<ID3D12Debug> Debug;
	VERIFY_SUCCEEDED(D3D12GetDebugInterface(__uuidof(Debug), Debug.put_void()));
	Debug->EnableDebugLayer();
#if 0
	//!< GPU-Based Validation
	Microsoft::WRL::ComPtr<ID3D12Debug1> Debug1;
	VERIFY_SUCCEEDED(Debug->QueryInterface(IID_PPV_ARGS(Debug1.GetAddressOf())));
	Debug1->SetEnableGPUBasedValidation(true);
#endif
#endif //!< _DEBUG

	//!< WARP �A�_�v�^���쐬����̂� IDXGIFactory4(��EnumWarpAdapter) ���K�v
	winrt::com_ptr<IDXGIFactory4> Factory;
	VERIFY_SUCCEEDED(CreateDXGIFactory1(__uuidof(Factory), Factory.put_void()));
#ifdef _DEBUG
	EnumAdapter(Factory.get());
#endif

	//!< DedicatedVideoMemory �̂���Ō�̃A�_�v�^(GPU)�C���f�b�N�X��Ԃ�
	auto GetLastIndexOfHardwareAdapter = [&]() {
		UINT Index = UINT_MAX;
		winrt::com_ptr<IDXGIAdapter> Adapter;
		for (UINT i = 0; DXGI_ERROR_NOT_FOUND != Factory->EnumAdapters(i, Adapter.put()); ++i) {
			DXGI_ADAPTER_DESC AdapterDesc;
			VERIFY_SUCCEEDED(Adapter->GetDesc(&AdapterDesc));
			if (AdapterDesc.DedicatedVideoMemory) {
				Index = i;
			}
			Adapter = nullptr;
		}
		assert(UINT_MAX != Index);
		return Index;
	};
	winrt::com_ptr<IDXGIAdapter> Adapter;
	Factory->EnumAdapters(GetLastIndexOfHardwareAdapter(), Adapter.put());

	DXGI_ADAPTER_DESC AdapterDesc;
	VERIFY_SUCCEEDED(Adapter->GetDesc(&AdapterDesc));
	Logf("\t%s\n", AdapterDesc.Description);

#if 0
	const std::vector<UUID> Experimental = { D3D12ExperimentalShaderModels, /*D3D12RaytracingPrototype*/ };
	VERIFY_SUCCEEDED(D3D12EnableExperimentalFeatures(static_cast<UINT>(Experimental.size()), Experimental.data(), nullptr, nullptr));
#endif

	if (FAILED(CreateMaxFeatureLevelDevice(Adapter.get()))) {
		Error(TEXT("Cannot create device, trying to create WarpDevice ...\n"));

		//!< WARP : Win7�ȉ����� D3D_FEATURE_LEVEL_10_1 �܂ŁAWin8�ȏゾ�� D3D_FEATURE_LEVEL_11_1 �܂ŃT�|�[�g
		VERIFY_SUCCEEDED(Factory->EnumWarpAdapter(__uuidof(Adapter), Adapter.put_void()));
		VERIFY_SUCCEEDED(CreateMaxFeatureLevelDevice(Adapter.get()));
	}

#elif defined(USE_WRL)

#if defined(_DEBUG) || defined(USE_PIX)
	if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(GraphicsAnalysis.GetAddressOf())))) {
		Log("Graphics Analysis is enabled\n");
	}
#endif

#ifdef _DEBUG
	Microsoft::WRL::ComPtr<ID3D12Debug> Debug;
	VERIFY_SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(Debug.GetAddressOf())));
	Debug->EnableDebugLayer();
#endif 

	Microsoft::WRL::ComPtr<IDXGIFactory4> Factory;
	VERIFY_SUCCEEDED(CreateDXGIFactory1(IID_PPV_ARGS(Factory.GetAddressOf())));
#ifdef _DEBUG
	EnumAdapter(Factory.Get());
#endif

	auto GetLastIndexOfHardwareAdapter = [&]() {
		UINT Index = UINT_MAX;
		Microsoft::WRL::ComPtr<IDXGIAdapter> Adapter;
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
	winrt::com_ptr<IDXGIAdapter> Adapter;
	Factory->EnumAdapters(GetLastIndexOfHardwareAdapter(), Adapter.put());

	DXGI_ADAPTER_DESC AdapterDesc;
	VERIFY_SUCCEEDED(Adapter->GetDesc(&AdapterDesc));
	Logf("\t%s\n", AdapterDesc.Description);

	if (FAILED(CreateMaxFeatureLevelDevice(Adapter.Get()))) {
		Error(TEXT("Cannot create device, trying to create WarpDevice ...\n"));
		VERIFY_SUCCEEDED(Factory->EnumWarpAdapter(IID_PPV_ARGS(Adapter.GetAddressOf())));
		VERIFY_SUCCEEDED(CreateMaxFeatureLevelDevice(Adapter.Get()));
	}

#endif //!< USE_WINRT

	LogOK("CreateDevice");
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
#ifdef USE_WINRT
	return D3D12CreateDevice(Adapter, FeatureLevel, __uuidof(Device), Device.put_void());
#elif defined(USE_WRL)
	return D3D12CreateDevice(Adapter, FeatureLevel, IID_PPV_ARGS(Device.GetAddressOf()));
#endif
}

//!< �A�_�v�^(GPU)�̗�
void DX::EnumAdapter(IDXGIFactory4* Factory)
{
#ifdef USE_WINRT
	winrt::com_ptr<IDXGIAdapter> Adapter;
	for (UINT i = 0; DXGI_ERROR_NOT_FOUND != Factory->EnumAdapters(i, Adapter.put()); ++i) {
		DXGI_ADAPTER_DESC AdapterDesc;
		VERIFY_SUCCEEDED(Adapter->GetDesc(&AdapterDesc));
		if (0 == i) { Log("[ Aadapters ]\n"); }
		Logf(TEXT("\t%s\n"), AdapterDesc.Description);
		Logf(TEXT("\t\tDedicatedVideoMemory = %lld\n"), AdapterDesc.DedicatedVideoMemory);
		Logf(TEXT("\t\tDedicatedSystemMemory = %lld\n"), AdapterDesc.DedicatedSystemMemory);
		Logf(TEXT("\t\tSharedSystemMemory = %lld\n"), AdapterDesc.SharedSystemMemory);

		EnumOutput(Adapter.get());

		Adapter = nullptr;
	}
#elif defined(USE_WRL)
	Microsoft::WRL::ComPtr<IDXGIAdapter> Adapter;
	for (UINT i = 0; DXGI_ERROR_NOT_FOUND != Factory->EnumAdapters(i, Adapter.ReleaseAndGetAddressOf()); ++i) {
		DXGI_ADAPTER_DESC AdapterDesc;
		VERIFY_SUCCEEDED(Adapter->GetDesc(&AdapterDesc));
		if (0 == i) { Log("[ Aadapters ]\n"); }
		Logf(TEXT("\t%s\n"), AdapterDesc.Description);
		Logf(TEXT("\t\tDedicatedVideoMemory = %lld\n"), AdapterDesc.DedicatedVideoMemory);
		Logf(TEXT("\t\tDedicatedSystemMemory = %lld\n"), AdapterDesc.DedicatedSystemMemory);
		Logf(TEXT("\t\tSharedSystemMemory = %lld\n"), AdapterDesc.SharedSystemMemory);

		EnumOutput(Adapter.Get());
	}
#endif
}

//!< �A�_�v�^�[(GPU)�ɐڑ�����Ă���A�A�E�g�v�b�g(�f�B�X�v���C)�̗�
void DX::EnumOutput(IDXGIAdapter* Adapter)
{
#ifdef USE_WINRT
	winrt::com_ptr<IDXGIOutput> Output;
	for (UINT i = 0; DXGI_ERROR_NOT_FOUND != Adapter->EnumOutputs(i, Output.put()); ++i) {
		DXGI_OUTPUT_DESC OutputDesc;
		VERIFY_SUCCEEDED(Output->GetDesc(&OutputDesc));

		const auto Width = OutputDesc.DesktopCoordinates.right - OutputDesc.DesktopCoordinates.left;
		const auto Height = OutputDesc.DesktopCoordinates.bottom - OutputDesc.DesktopCoordinates.top;
		if (0 == i) { Log("\t\t[ Output ]\n"); }
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

		GetDisplayModeList(Output.get(), DXGI_FORMAT_R8G8B8A8_UNORM);

		Output = nullptr;
	}

#elif defined(USE_WRL)
	Microsoft::WRL::ComPtr<IDXGIOutput> Output;
	for (UINT i = 0; DXGI_ERROR_NOT_FOUND != Adapter->EnumOutputs(i, Output.ReleaseAndGetAddressOf()); ++i) {
		DXGI_OUTPUT_DESC OutputDesc;
		VERIFY_SUCCEEDED(Output->GetDesc(&OutputDesc));

		const auto Width = OutputDesc.DesktopCoordinates.right - OutputDesc.DesktopCoordinates.left;
		const auto Height = OutputDesc.DesktopCoordinates.bottom - OutputDesc.DesktopCoordinates.top;
		if (0 == i) { Log("\t\t[ Output ]\n"); }
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

		GetDisplayModeList(Output.Get(), DXGI_FORMAT_R8G8B8A8_UNORM);
	}
#endif
}

//!< �A�E�g�v�b�g(�f�B�X�v���C)�̕`�惂�[�h�̗�
void DX::GetDisplayModeList(IDXGIOutput* Output, const DXGI_FORMAT Format)
{
	UINT NumModes;
	VERIFY_SUCCEEDED(Output->GetDisplayModeList(Format, 0, &NumModes, nullptr));
	if (NumModes) {
		Log("\t\t\t[ DisplayModes ]\n");

		std::vector<DXGI_MODE_DESC> ModeDescs(NumModes);
		VERIFY_SUCCEEDED(Output->GetDisplayModeList(Format, 0, &NumModes, ModeDescs.data()));
		for (const auto& i : ModeDescs) {
			Logf("\t\t\t\t%d x %d @ %d\n", i.Width, i.Height, i.RefreshRate.Numerator / i.RefreshRate.Denominator);
		}
	}
}

void DX::CheckFeatureLevel()
{
	D3D12_FEATURE_DATA_FEATURE_LEVELS DataFeatureLevels = {
		static_cast<UINT>(FeatureLevels.size()), FeatureLevels.data()
	};
	//!< NumFeatureLevels, pFeatureLevelsRequested �� CheckFeatureSupport() �ւ̓��́AMaxSupportedFeatureLevel �ɂ� CheckFeatureSupport() ����̏o�͂��Ԃ�
	VERIFY_SUCCEEDED(Device->CheckFeatureSupport(D3D12_FEATURE_FEATURE_LEVELS, reinterpret_cast<void*>(&DataFeatureLevels), sizeof(DataFeatureLevels)));

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
		//!< Format, SampleCount, Flags �� CheckFeatureSupport() �ւ̓��́ANumQualityLevels �ɂ� CheckFeatureSupport() ����̏o�͂��Ԃ�
		D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS DataMultiSampleQaualityLevels = {
			Format,
			i,
			D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE,
			0
		};
		VERIFY_SUCCEEDED(Device->CheckFeatureSupport(D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS, reinterpret_cast<void*>(&DataMultiSampleQaualityLevels), sizeof(DataMultiSampleQaualityLevels)));
		//!< 0 == NumQualityLevels �̏ꍇ�̓T�|�[�g����Ă��Ȃ��Ƃ�������
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
@brief �}���`�X���b�h�Łu�����v�L���[�փT�u�~�b�g�ł���
@note Vulkan �ł̓}���`�X���b�h�Łu�قȂ�v�L���[�ւ̂݃T�u�~�b�g�ł���̂Œ���
*/
void DX::CreateCommandQueue()
{
	const D3D12_COMMAND_QUEUE_DESC CommandQueueDesc = {
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		0,
		D3D12_COMMAND_QUEUE_FLAG_NONE,
		0 // NodeMask ... �}���`GPU�̏ꍇ
	};
#ifdef USE_WINRT
	VERIFY_SUCCEEDED(Device->CreateCommandQueue(&CommandQueueDesc, __uuidof(CommandQueue), CommandQueue.put_void()));
#elif defined(USE_WRL)
	VERIFY_SUCCEEDED(Device->CreateCommandQueue(&CommandQueueDesc, IID_PPV_ARGS(CommandQueue.GetAddressOf())));
#endif

	LogOK("CreateCommandQueue");
}

/**
@brief CPU �� GPU �̓����p
*/
void DX::CreateFence()
{
#ifdef USE_WINRT
	VERIFY_SUCCEEDED(Device->CreateFence(0, D3D12_FENCE_FLAG_NONE, __uuidof(Fence), Fence.put_void()));
#elif defined(USE_WRL)
	VERIFY_SUCCEEDED(Device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(Fence.GetAddressOf())));
#endif

	LogOK("CreateFence");
}

/**
@note �R�}���h�A���P�[�^�ɑ΂������̃R�}���h���X�g���쐬�ł��邪�A�R�}���h���X�g�͓����ɂ͋L�^�ł��Ȃ� (Close()���Ȃ��ƃ_��)
@note CommandList->ExecuteCommandList() �� GPU �� CommandAllocator �̎Q�Ƃ��I����܂ŁACommandAllocator->Reset() ���Ă͂����Ȃ�
*/
void DX::CreateCommandAllocator(const D3D12_COMMAND_LIST_TYPE CommandListType)
{
#ifdef USE_WINRT
	winrt::com_ptr<ID3D12CommandAllocator> CA;
	VERIFY_SUCCEEDED(Device->CreateCommandAllocator(CommandListType, __uuidof(CA), CA.put_void()));
#elif defined(USE_WRL)
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> CA;
	VERIFY_SUCCEEDED(Device->CreateCommandAllocator(CommandListType, IID_PPV_ARGS(CA.GetAddressOf())));
#endif
	CommandAllocators.push_back(CA);

	Log("\tCommandAllocator\n");
}

/**
@note CommandList->ExecuteCommandList() ��� CommandList->Reset() �����Ă��ǂ��B(CommandAllocator ���o���Ă���̂ŁACommandQueue �ɂ͉e�����Ȃ�)
�`��R�}���h�𔭍s����R�}���h���X�g�� PipelineState �̎w�肪�K�v
�ォ��CommandList->Reset(Allocator, PipelineState) �̈����ł��w��ł���
�`��R�}���h�𔭍s���Ȃ��R�}���h���X�g(�������p�r��)��A�o���h���� nullptr �w��ŗǂ�
�����ł� PipelineState == nullptr �ō쐬���Ă��܂��Ă���
*/
void DX::CreateCommandList(ID3D12CommandAllocator* CommandAllocator, const size_t Count, const D3D12_COMMAND_LIST_TYPE CommandListType)
{
	for (auto i = 0; i < Count; ++i) {
#ifdef USE_WINRT
		winrt::com_ptr<ID3D12GraphicsCommandList> CL;
		VERIFY_SUCCEEDED(Device->CreateCommandList(0, CommandListType, CommandAllocator, nullptr, __uuidof(CL), CL.put_void()));
#elif defined(USE_WRL)
		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> CL;
		VERIFY_SUCCEEDED(Device->CreateCommandList(0, CommandListType, CommandAllocator, nullptr, IID_PPV_ARGS(CL.GetAddressOf()))); 
#endif
		GraphicsCommandLists.push_back(CL);

		//!< Close() ���Ă���
		//const auto GCL = static_cast<ID3D12GraphicsCommandList*>(GraphicsCommandLists.back().Get());
		//if (nullptr != GCL) {
		//	VERIFY_SUCCEEDED(GCL->Close());
		//}
		VERIFY_SUCCEEDED(GraphicsCommandLists.back()->Close());
	}

	LogOK("CreateCommandList");
}

void DX::CreateCommandList()
{
	CreateCommandAllocator();
	DXGI_SWAP_CHAIN_DESC1 SwapChainDesc;
	SwapChain->GetDesc1(&SwapChainDesc);
	//!< �����0�Ԃ̃R�}���h�A���P�[�^���ߑł� #DX_TODO
#ifdef USE_WINRT
	CreateCommandList(CommandAllocators[0].get(), SwapChainDesc.BufferCount, D3D12_COMMAND_LIST_TYPE_DIRECT); 
#elif defined(USE_WRL)
	CreateCommandList(CommandAllocators[0].Get(), SwapChainDesc.BufferCount, D3D12_COMMAND_LIST_TYPE_DIRECT);
#endif
}

void DX::CreateSwapchain(HWND hWnd, const DXGI_FORMAT ColorFormat)
{
	CreateSwapChain(hWnd, ColorFormat, Rect);

	//!< �r���[���쐬 Create view
	CreateSwapChainResource();
}
void DX::CreateSwapChain(HWND hWnd, const DXGI_FORMAT ColorFormat, const UINT Width, const UINT Height)
{
	const UINT BufferCount = 3;
#ifdef USE_WINRT
	winrt::com_ptr<IDXGIFactory4> Factory;
	VERIFY_SUCCEEDED(CreateDXGIFactory1(__uuidof(Factory), Factory.put_void()));
#elif defined(USE_WRL)
	Microsoft::WRL::ComPtr<IDXGIFactory4> Factory;
	VERIFY_SUCCEEDED(CreateDXGIFactory1(IID_PPV_ARGS(Factory.GetAddressOf())));
#endif

	//!< �œK�ȃt���X�N���[���̃p�t�H�[�}���X�𓾂�ɂ́AIDXGIOutput->GetDisplayModeList() �Ŏ擾����(�f�B�X�v���C�̃T�|�[�g����)DXGI_MODE_DESC �łȂ��ƃ_���Ȃ̂Œ���  #DX_TODO
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
		DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH //!< �t���X�N���[���ɂ������A�œK�ȃf�B�X�v���C���[�h���I�������̂�����
	};
	//!< �Z�b�e�B���O��ύX���ăX���b�v�`�F�C�����č쐬�ł���悤�ɁA�����̂��J�����Ă���
#ifdef USE_WINRT
	SwapChain = nullptr;
	winrt::com_ptr<IDXGISwapChain> NewSwapChain;
	VERIFY_SUCCEEDED(Factory->CreateSwapChain(CommandQueue.get(), &SwapChainDesc, NewSwapChain.put()));
	winrt::copy_to_abi(NewSwapChain, *SwapChain.put_void());
#elif defined(USE_WRL)
	SwapChain.Reset();
	Microsoft::WRL::ComPtr<IDXGISwapChain> NewSwapChain;
	VERIFY_SUCCEEDED(Factory->CreateSwapChain(CommandQueue.Get(), &SwapChainDesc, NewSwapChain.GetAddressOf()));
	VERIFY_SUCCEEDED(NewSwapChain.As(&SwapChain));
#endif

#ifdef USE_WINRT
	[&](const D3D12_DESCRIPTOR_HEAP_TYPE Type, const UINT Count, winrt::com_ptr<ID3D12DescriptorHeap>& DH) {
		const D3D12_DESCRIPTOR_HEAP_DESC DescriptorHeapDesc = {
			Type,
			Count,
			D3D12_DESCRIPTOR_HEAP_FLAG_NONE,
			0 // NodeMask ... �}���`GPU�̏ꍇ
		};
		//VERIFY_SUCCEEDED(Device->CreateDescriptorHeap(&DescriptorHeapDesc, __uuidof(DH). DH.put_void()));
		VERIFY_SUCCEEDED(Device->CreateDescriptorHeap(&DescriptorHeapDesc, IID_PPV_ARGS(DH.put())));
	}
#elif defined(USE_WRL)
	[&](const D3D12_DESCRIPTOR_HEAP_TYPE Type, const UINT Count, Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>& DH) {
		const D3D12_DESCRIPTOR_HEAP_DESC DescriptorHeapDesc = {
			Type,
			Count,
			D3D12_DESCRIPTOR_HEAP_FLAG_NONE,
			0
		};
		VERIFY_SUCCEEDED(Device->CreateDescriptorHeap(&DescriptorHeapDesc, IID_PPV_ARGS(DH.GetAddressOf())));
	}
#endif
	(D3D12_DESCRIPTOR_HEAP_TYPE_RTV, SwapChainDesc.BufferCount, SwapChainDescriptorHeap);

	LogOK("CreateSwapChain");
}
void DX::CreateSwapChainResource()
{
	DXGI_SWAP_CHAIN_DESC1 SwapChainDesc;
	SwapChain->GetDesc1(&SwapChainDesc);

	SwapChainResources.resize(SwapChainDesc.BufferCount);
	//for (auto It = SwapChainResources.begin(); It != SwapChainResources.end(); ++It) {
	//	const auto Index = static_cast<UINT>(std::distance(SwapChainResources.begin(), It));
	//	//!< �X���b�v�`�F�C���̃o�b�t�@���\�[�X�� SwapChainResources �֎擾
	//	VERIFY_SUCCEEDED(SwapChain->GetBuffer(Index, __uuidof(It), It.put_void()));
	////VERIFY_SUCCEEDED(SwapChain->GetBuffer(Index, IID_PPV_ARGS(It->GetAddressOf())));

	//	//!< �f�X�N���v�^(�r���[)�̍쐬�B���\�[�X��ł̃I�t�Z�b�g���w�肵�č쐬���Ă���A���ʂ��ϐ��ɕԂ�킯�ł͂Ȃ�
	//	//!< (���\�[�X���^�C�v�h�t�H�[�}�b�g�Ȃ� D3D12_RENDER_TARGET_VIEW_DESC* �� nullptr �w��\)
	//	Device->CreateRenderTargetView(It->Get(), nullptr, GetCPUDescriptorHandle(SwapChainDescriptorHeap.Get(), D3D12_DESCRIPTOR_HEAP_TYPE_RTV, Index));
	//}
	for (auto i = 0; i < SwapChainResources.size(); ++i) {
#ifdef USE_WINRT
		//!< �X���b�v�`�F�C���̃o�b�t�@���\�[�X�� SwapChainResources �֎擾
		VERIFY_SUCCEEDED(SwapChain->GetBuffer(i, __uuidof(SwapChainResources[i]), SwapChainResources[i].put_void()));
		//!< �f�X�N���v�^(�r���[)�̍쐬�B���\�[�X��ł̃I�t�Z�b�g���w�肵�č쐬���Ă���A���ʂ��ϐ��ɕԂ�킯�ł͂Ȃ�
		//!< (���\�[�X���^�C�v�h�t�H�[�}�b�g�Ȃ� D3D12_RENDER_TARGET_VIEW_DESC* �� nullptr �w��\)
		const auto SCR = SwapChainResources[i].get();
		const auto CDH = GetCPUDescriptorHandle(SwapChainDescriptorHeap.get(), D3D12_DESCRIPTOR_HEAP_TYPE_RTV, i);
#elif defined(USE_WRL)
		VERIFY_SUCCEEDED(SwapChain->GetBuffer(i, IID_PPV_ARGS(SwapChainResources[i].GetAddressOf())));
		const auto SCR = SwapChainResources[i].Get();
		const auto CDH = GetCPUDescriptorHandle(SwapChainDescriptorHeap.Get(), D3D12_DESCRIPTOR_HEAP_TYPE_RTV, i); 
#endif		
		Device->CreateRenderTargetView(SCR, nullptr, CDH);
	}

	LogOK("CreateSwapChainResource");
}

/**
@note Vulkan�ƈ���āA�X���b�v�`�F�C���C���[�W���ɁA�ʂ̃R�}���h���X�g���g�p���Ȃ��ƃ_�����ۂ�
*/
void DX::InitializeSwapchainImage(ID3D12CommandAllocator* CommandAllocator, const DirectX::XMVECTORF32* Color)
{
#ifdef USE_WINRT
	for (auto i = 0; i < SwapChainResources.size(); ++i) {
		const auto CL = GraphicsCommandLists[i].get();
		VERIFY_SUCCEEDED(CL->Reset(CommandAllocator, nullptr));
		{
			const auto SCR = SwapChainResources[i].get();
			const auto CDH = GetCPUDescriptorHandle(SwapChainDescriptorHeap.get(), D3D12_DESCRIPTOR_HEAP_TYPE_RTV, i);
			ResourceBarrier(CL, SCR, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET); {
				if (nullptr != Color) {
					CL->ClearRenderTargetView(CDH, *Color, 0, nullptr);
				}
			} ResourceBarrier(CL, SCR, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
		}
		VERIFY_SUCCEEDED(CL->Close());
	}

	//!< #DX_TODO : 0 �Ԗڂ����N���A���Ă��Ȃ�
	const std::vector<ID3D12CommandList*> CommandLists = { GraphicsCommandLists[0].get() };
	CommandQueue->ExecuteCommandLists(static_cast<UINT>(CommandLists.size()), CommandLists.data());

#elif defined(USE_WRL)

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

	const std::vector<ID3D12CommandList*> CommandLists = { GraphicsCommandLists[0].Get() };
	CommandQueue->ExecuteCommandLists(static_cast<UINT>(CommandLists.size()), CommandLists.data());

#endif //!< USE_WINRT

	WaitForFence();

	LogOK("InitializeSwapchainImage");
}

void DX::InitializeSwapChain()
{
	//!< �C���[�W�̏����� Initialize images
#ifdef USE_WINRT
	//InitializeSwapchainImage(CommandAllocators[0].get());
	InitializeSwapchainImage(CommandAllocators[0].get(), &DirectX::Colors::Red); 
#elif defined(USE_WRL)
	InitializeSwapchainImage(CommandAllocators[0].Get(), &DirectX::Colors::Red); 
#endif
}

void DX::ResizeSwapChain(const UINT Width, const UINT Height)
{
	ResetSwapChainResource();

	DXGI_SWAP_CHAIN_DESC1 SwapChainDesc;
	SwapChain->GetDesc1(&SwapChainDesc);
	VERIFY_SUCCEEDED(SwapChain->ResizeBuffers(SwapChainDesc.BufferCount, Width, Height, SwapChainDesc.Format, SwapChainDesc.Flags));
	Log("\tResizeBuffers\n");

	CreateSwapChainResource();

	LogOK("ResizeSwapChain");
}

void DX::CreateDepthStencil(const DXGI_FORMAT DepthFormat, const UINT Width, const UINT Height)
{
	[&](const D3D12_DESCRIPTOR_HEAP_TYPE Type, const UINT Count/*, ID3D12DescriptorHeap** DescriptorHeap*/) {
		const D3D12_DESCRIPTOR_HEAP_DESC DescriptorHeapDesc = {
			Type,
			Count,
			D3D12_DESCRIPTOR_HEAP_FLAG_NONE,
			0
		};
#ifdef USE_WINRT
		VERIFY_SUCCEEDED(Device->CreateDescriptorHeap(&DescriptorHeapDesc, __uuidof(DepthStencilDescriptorHeap), DepthStencilDescriptorHeap.put_void()));
#elif defined(USE_WRL)
		VERIFY_SUCCEEDED(Device->CreateDescriptorHeap(&DescriptorHeapDesc, IID_PPV_ARGS(DepthStencilDescriptorHeap.GetAddressOf())));
#endif
	}(D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 1);

	CreateDepthStencilResource(DepthFormat, Width, Height);

	LogOK("CreateDepthStencil");
}

void DX::CreateDepthStencilResource(const DXGI_FORMAT DepthFormat, const UINT Width, const UINT Height)
{
	//!< ���\�[�X�̍쐬
	const D3D12_HEAP_PROPERTIES HeapProperties = {
		D3D12_HEAP_TYPE_DEFAULT, //!< DEFAULT �ɂ��邱��
		D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
		D3D12_MEMORY_POOL_UNKNOWN,
		1,
		1
	};
	const auto& SampleDesc = SampleDescs[0]; //!< �����_�[�^�[�Q�b�g�̂��̂ƈ�v���邱��
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
	//!< ��v����N���A�l�Ȃ�œK�������̂ł悭�g���N���A�l���w�肵�Ă���
	const D3D12_CLEAR_VALUE ClearValue = {
		DepthFormat,
		{ 1.0f, 0 }
	};
#ifdef USE_WINRT
	DepthStencilResource = nullptr;
	VERIFY_SUCCEEDED(Device->CreateCommittedResource(&HeapProperties, D3D12_HEAP_FLAG_NONE, &ResourceDesc, D3D12_RESOURCE_STATE_COMMON/*COMMON �ɂ��邱��*/, &ClearValue, __uuidof(DepthStencilResource), DepthStencilResource.put_void()));
#elif defined(USE_WRL)
	VERIFY_SUCCEEDED(Device->CreateCommittedResource(&HeapProperties, D3D12_HEAP_FLAG_NONE, &ResourceDesc, D3D12_RESOURCE_STATE_COMMON, &ClearValue, IID_PPV_ARGS(DepthStencilResource.ReleaseAndGetAddressOf()))); 
#endif

#ifdef USE_WINRT
	const auto CDH = GetCPUDescriptorHandle(DepthStencilDescriptorHeap.get(), D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
	//!< �f�X�N���v�^(�r���[)�̍쐬�B���\�[�X��ł̃I�t�Z�b�g���w�肵�č쐬���Ă���A���ʂ��ϐ��ɕԂ�킯�ł͂Ȃ�
	//!< (���\�[�X���^�C�v�h�t�H�[�}�b�g�Ȃ� D3D12_DEPTH_STENCIL_VIEW_DESC* �� nullptr �w��\)
	Device->CreateDepthStencilView(DepthStencilResource.get(), nullptr, CDH); 
#elif defined(USE_WRL)
	const auto CDH = GetCPUDescriptorHandle(DepthStencilDescriptorHeap.Get(), D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
	Device->CreateDepthStencilView(DepthStencilResource.Get(), nullptr, CDH); 
#endif

#ifdef DEBUG_STDOUT
	std::cout << "\t" << "DepthStencilView" << std::endl;
#endif

	//!< ���\�[�X�̏�Ԃ����� �� �f�v�X�������݂֕ύX
	auto CL = GraphicsCommandLists[0];
#ifdef USE_WINRT
	ResourceBarrier(CL.get(), DepthStencilResource.get(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_DEPTH_WRITE);
#elif defined(USE_WRL)
	ResourceBarrier(CL.Get(), DepthStencilResource.Get(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_DEPTH_WRITE);
#endif

	LogOK("CreateDepthStencilResource");
}
void DX::ResizeDepthStencil(const DXGI_FORMAT DepthFormat, const UINT Width, const UINT Height)
{
#ifdef USE_WINRT
	DepthStencilResource = nullptr;
#elif defined(USE_WRL)
	DepthStencilResource.Reset();
#endif

	CreateDepthStencilResource(DepthFormat, Width, Height);

	LogOK("ResizeDepthStencil");
}

void DX::CreateIndirectBuffer(ID3D12Resource** Resource, const UINT32 Size, const void* Source, ID3D12CommandAllocator* CA, ID3D12GraphicsCommandList* CL)
{
	//!< �A�b�v���[�h�p�̃��\�[�X���쐬�A�f�[�^���R�s�[ Create upload resource, and copy data
#ifdef USE_WINRT
	winrt::com_ptr<ID3D12Resource> UploadRes;
	CreateUploadResource(UploadRes.put(), Size);
	CopyToUploadResource(UploadRes.get(), Size, Source); 
#elif defined(USE_WRL)
	Microsoft::WRL::ComPtr<ID3D12Resource> UploadRes;
	CreateUploadResource(UploadRes.GetAddressOf(), Size);
	CopyToUploadResource(UploadRes.Get(), Size, Source); 
#endif
	

	//!< �f�t�H���g�̃��\�[�X���쐬 Create default resource
	CreateDefaultResource(Resource, Size);

	//!< �A�b�v���[�h���\�[�X����f�t�H���g���\�[�X�ւ̃R�s�[�R�}���h�𔭍s Execute copy command upload resource to default resource
#ifdef USE_WINRT
	ExecuteCopyBuffer(CA, CL, UploadRes.get(), *Resource, Size);
#elif defined(USE_WRL)
	ExecuteCopyBuffer(CA, CL, UploadRes.Get(), *Resource, Size);
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

	LogOK("CreateViewport");
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
//	VERIFY_SUCCEEDED(Device->CreateRootSignature(0, Blob->GetBufferPointer(), Blob->GetBufferSize(), __uuidof(RootSignature), RootSignature.put_void()));
////VERIFY_SUCCEEDED(Device->CreateRootSignature(0, Blob->GetBufferPointer(), Blob->GetBufferSize(), IID_PPV_ARGS(RootSignature.GetAddressOf())));
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
	LogOK("CreateConstantBuffer");
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
#ifdef USE_WINRT
	VERIFY_SUCCEEDED(Device->CreateCommittedResource(&HeapProperties, D3D12_HEAP_FLAG_NONE, &ResourceDesc, D3D12_RESOURCE_STATE_COMMON, nullptr, __uuidof(UnorderedAccessTextureResource), UnorderedAccessTextureResource.put_void()));
#elif defined(USE_WRL)
	VERIFY_SUCCEEDED(Device->CreateCommittedResource(&HeapProperties, D3D12_HEAP_FLAG_NONE, &ResourceDesc, D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(UnorderedAccessTextureResource.GetAddressOf())));
#endif

	const auto Format = DXGI_FORMAT_R8G8B8A8_UNORM;

	const D3D12_DESCRIPTOR_HEAP_DESC DescritporHeapDesc = {
		D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
		2, //!< SRV, UAV �� 2 ��
		D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE,
		0 // NodeMask ... �}���`GPU�̏ꍇ
	};
#ifdef USE_WINRT
	VERIFY_SUCCEEDED(Device->CreateDescriptorHeap(&DescritporHeapDesc, __uuidof(UnorderedAccessTextureDescriptorHeap), UnorderedAccessTextureDescriptorHeap.put_void()));
#elif defined(USE_WRL)
	VERIFY_SUCCEEDED(Device->CreateDescriptorHeap(&DescritporHeapDesc, IID_PPV_ARGS(UnorderedAccessTextureDescriptorHeap.GetAddressOf())));
#endif

	//!< �f�X�N���v�^(�r���[)�̍쐬�B���\�[�X��ł̃I�t�Z�b�g���w�肵�č쐬���Ă���A���ʂ��ϐ��ɕԂ�킯�ł͂Ȃ�
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
#ifdef USE_WINRT
		const auto CDH = GetCPUDescriptorHandle(UnorderedAccessTextureDescriptorHeap.get(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, Index++);
		Device->CreateShaderResourceView(UnorderedAccessTextureResource.get(), &SRVDesc, CDH); 
#elif defined(USE_WRL)
		const auto CDH = GetCPUDescriptorHandle(UnorderedAccessTextureDescriptorHeap.Get(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, Index++);
		Device->CreateShaderResourceView(UnorderedAccessTextureResource.Get(), &SRVDesc, CDH); 
#endif		
	}

	{
		/*const*/D3D12_UNORDERED_ACCESS_VIEW_DESC UAVDesc = {
			Format,
			D3D12_UAV_DIMENSION_TEXTURE2D,
		};
		UAVDesc.Texture2D = {
			0, 0
		};
		//!< �f�X�N���v�^(�r���[)�̍쐬�B���\�[�X��ł̃I�t�Z�b�g���w�肵�č쐬���Ă���A���ʂ��ϐ��ɕԂ�킯�ł͂Ȃ�
#ifdef USE_WINRT
		const auto CDH = GetCPUDescriptorHandle(UnorderedAccessTextureDescriptorHeap.get(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, Index++);
		Device->CreateUnorderedAccessView(UnorderedAccessTextureResource.get(), nullptr, &UAVDesc, CDH);
#elif defined(USE_WRL)
		const auto CDH = GetCPUDescriptorHandle(UnorderedAccessTextureDescriptorHeap.Get(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, Index++);
		Device->CreateUnorderedAccessView(UnorderedAccessTextureResource.Get(), nullptr, &UAVDesc, CDH);
#endif
		
	}

	LogOK("CreateUnorderedAccessBuffer");
}

//!< ���[�g�V�O�l�`�����V���A���C�Y���ău���u�����
#ifdef USE_WINRT
void DX::SerializeRootSignature(winrt::com_ptr<ID3DBlob>& RSBlob)
#elif defined(USE_WRL)
void DX::SerializeRootSignature(Microsoft::WRL::ComPtr<ID3DBlob>& RSBlob)
#endif
{
	std::vector<D3D12_DESCRIPTOR_RANGE> DescriptorRanges = {
		/**
		D3D12_DESCRIPTOR_RANGE_TYPE RangeType; ... D3D12_DESCRIPTOR_RANGE_TYPE_[SRV, UAV, CBV, SAMPLER]
		UINT NumDescriptors;
		UINT BaseShaderRegister; ... register(b0) �Ȃ� 0�Aregister(t3) �Ȃ� 3
		UINT RegisterSpace; ... �ʏ�� 0 �ł悢 register(t3, space5) �Ȃ� 5
		UINT OffsetInDescriptorsFromTableStart; ... �ʏ�� D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND �ł悢
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
			D3D12_DESCRIPTOR_RANGE
			{
				D3D12_DESCRIPTOR_RANGE_TYPE RangeType;
				UINT NumDescriptors;
				UINT BaseShaderRegister;
				UINT RegisterSpace;
				UINT OffsetInDescriptorsFromTableStart;
			}
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
		//!< �����̃��[�g�V�O�l�`���œ����T���v�����g���񂷂悤�ȏꍇ�̓X�^�C�e�B�b�N�T���v�����쐬���Ă����Ɨǂ�
		static_cast<UINT>(StaticSamplerDescs.size()), StaticSamplerDescs.data(),
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT
	};
#ifdef USE_WINRT
	winrt::com_ptr<ID3DBlob> ErrorBlob;
	VERIFY_SUCCEEDED(D3D12SerializeRootSignature(&RootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, RSBlob.put(), ErrorBlob.put()));
#elif defined(USE_WRL)
	Microsoft::WRL::ComPtr<ID3DBlob> ErrorBlob;
	VERIFY_SUCCEEDED(D3D12SerializeRootSignature(&RootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, RSBlob.GetAddressOf(), ErrorBlob.GetAddressOf()));
#endif
}

//!< �V�F�[�_���烋�[�g�V�O�l�`���p�[�g�����o���u���u�����
#ifdef USE_WINRT
void DX::GetRootSignaturePartFromShader(winrt::com_ptr<ID3DBlob>& RSBlob)
#elif defined(USE_WRL)
void DX::GetRootSignaturePartFromShader(Microsoft::WRL::ComPtr<ID3DBlob>& RSBlob)
#endif
{
#ifdef USE_WINRT
	winrt::com_ptr<ID3DBlob> ShaderBlob;
	D3DReadFileToBlob((GetBasePath() + TEXT(".rs.cso")).data(), ShaderBlob.put());
#elif defined(USE_WRL)
	Microsoft::WRL::ComPtr<ID3DBlob> ShaderBlob;
	D3DReadFileToBlob((GetBasePath() + TEXT(".rs.cso")).data(), ShaderBlob.GetAddressOf());
#endif
#ifdef USE_WINRT
	VERIFY_SUCCEEDED(D3DGetBlobPart(ShaderBlob->GetBufferPointer(), ShaderBlob->GetBufferSize(), D3D_BLOB_ROOT_SIGNATURE, 0, RSBlob.put()));
#elif defined(USE_WRL)
	VERIFY_SUCCEEDED(D3DGetBlobPart(ShaderBlob->GetBufferPointer(), ShaderBlob->GetBufferSize(), D3D_BLOB_ROOT_SIGNATURE, 0, RSBlob.GetAddressOf()));
#endif
}

/**
@brief �V�F�[�_�Ƃ̃o�C���f�B���O (VK::CreateDescriptorSetLayout() ����)
*/
void DX::CreateRootSignature()
{
#ifdef USE_WINRT
	winrt::com_ptr<ID3DBlob> RSBlob;
#elif defined(USE_WRL)
Microsoft::WRL::ComPtr<ID3DBlob> RSBlob;
#endif
	
#ifdef ROOTSIGNATRUE_FROM_SHADER
	GetRootSignaturePartFromShader(RSBlob);
#else
	SerializeRootSignature(RSBlob);
#endif

#ifdef USE_WINRT
	VERIFY_SUCCEEDED(Device->CreateRootSignature(0, RSBlob->GetBufferPointer(), RSBlob->GetBufferSize(), __uuidof(RootSignature), RootSignature.put_void()));
#elif defined(USE_WRL)
	VERIFY_SUCCEEDED(Device->CreateRootSignature(0, RSBlob->GetBufferPointer(), RSBlob->GetBufferSize(), IID_PPV_ARGS(RootSignature.GetAddressOf())));
#endif

	LogOK("CreateRootSignature");
}

#ifdef USE_WINRT
void DX::CreateShader(std::vector<winrt::com_ptr<ID3DBlob>>& ShaderBlobs) const
#elif defined(USE_WRL)
void DX::CreateShader(std::vector<Microsoft::WRL::ComPtr<ID3DBlob>>& ShaderBlobs) const
#endif
{
	for (auto i : ShaderBlobs) {
		//!< �uPDB�p�[�g�v���擾
#ifdef USE_WINRT
		winrt::com_ptr<ID3DBlob> PDBPart;
		VERIFY_SUCCEEDED(D3DGetBlobPart(i->GetBufferPointer(), i->GetBufferSize(), D3D_BLOB_PDB, 0, PDBPart.put())); 
#elif defined(USE_WRL)
		Microsoft::WRL::ComPtr<ID3DBlob> PDBPart;
		VERIFY_SUCCEEDED(D3DGetBlobPart(i->GetBufferPointer(), i->GetBufferSize(), D3D_BLOB_PDB, 0, PDBPart.GetAddressOf())); 
#endif

#if 0
		//!< �C�ӂ�(�u�f�o�b�O���v)�f�[�^
		const char DebugName[] = "DebugName";

		//!< 4�o�C�g�A���C�����ꂽ�X�g���[�W
		const auto Size = RoundUp(_countof(DebugName), 0x3);
		auto Data = new BYTE [Size];
		memcpy(Data, DebugName, _countof(DebugName));

		//!< �u�f�o�b�O���v�̕t�����u���u
		Microsoft::WRL::ComPtr<ID3DBlob> WithDebugNamePart;
		if (SUCCEEDED(D3DSetBlobPart(i->GetBufferPointer(), i->GetBufferSize(), D3D_BLOB_DEBUG_NAME, 0, Data, Size, WithDebugNamePart.GetAddressOf()))) {
			//!<�u�f�o�b�O���v�p�[�g���擾
			Microsoft::WRL::ComPtr<ID3DBlob> DebugNamePart;
			if (SUCCEEDED(D3DGetBlobPart(WithDebugNamePart->GetBufferPointer(), WithDebugNamePart->GetBufferSize(), D3D_BLOB_DEBUG_NAME, 0, DebugNamePart.GetAddressOf()))) {
				std::cout << reinterpret_cast<const char*>(DebugNamePart->GetBufferPointer()) << std::endl;
			}
		}

		delete[] Data;
#endif
	}

	//!< �f�o�b�O���A���[�g�V�O�l�`������菜��
#ifndef _DEBUG
	for (auto i : ShaderBlobs) {
		if (nullptr != i) {
#ifdef USE_WINRT
			VERIFY_SUCCEEDED(D3DStripShader(i->GetBufferPointer(), i->GetBufferSize(), D3DCOMPILER_STRIP_DEBUG_INFO, i.put()));
			VERIFY_SUCCEEDED(D3DStripShader(i->GetBufferPointer(), i->GetBufferSize(), D3DCOMPILER_STRIP_ROOT_SIGNATURE, i.put()));
#elif defined(USE_WRL)
			VERIFY_SUCCEEDED(D3DStripShader(i->GetBufferPointer(), i->GetBufferSize(), D3DCOMPILER_STRIP_DEBUG_INFO, i.GetAddressOf()));
			VERIFY_SUCCEEDED(D3DStripShader(i->GetBufferPointer(), i->GetBufferSize(), D3DCOMPILER_STRIP_ROOT_SIGNATURE, i.GetAddressOf()));
#endif
		}
	}
#endif
}

//!< @brief (�悭�g���p�^�[���Ƃ���)VS, PS, DS, HS, GS �̏��ŋl�߂Ă����A�������ɂ������ꍇ�̓I�[�o�[���C�h���Ď�������K�v����
#ifdef USE_WINRT
void DX::CreateShaderByteCode(const std::vector<winrt::com_ptr<ID3DBlob>>& ShaderBlobs, std::array<D3D12_SHADER_BYTECODE, 5>& ShaderBCs) const
#elif defined(USE_WRL)
void DX::CreateShaderByteCode(const std::vector<Microsoft::WRL::ComPtr<ID3DBlob>>& ShaderBlobs, std::array<D3D12_SHADER_BYTECODE, 5>& ShaderBCs) const
#endif
{
	for (auto i = 0; i < ShaderBlobs.size(); ++i) {
		ShaderBCs[i] = D3D12_SHADER_BYTECODE({ ShaderBlobs[i]->GetBufferPointer(), ShaderBlobs[i]->GetBufferSize() });
	}
}

BOOL DX::LoadPipelineLibrary(const std::wstring& Path)
{
#ifdef USE_WINRT
	winrt::com_ptr<ID3D12Device1> Device1;
	VERIFY_SUCCEEDED(Device->QueryInterface(__uuidof(Device1), Device1.put_void()));
#elif defined(USE_WRL)
	Microsoft::WRL::ComPtr<ID3D12Device1> Device1;
	VERIFY_SUCCEEDED(Device->QueryInterface(IID_PPV_ARGS(Device1.GetAddressOf())));
#endif

#ifdef USE_WINRT
	winrt::com_ptr<ID3DBlob> Blob;
	if (SUCCEEDED(D3DReadFileToBlob(Path.data(), Blob.put())) && Blob->GetBufferSize()) {
		VERIFY_SUCCEEDED(Device1->CreatePipelineLibrary(Blob->GetBufferPointer(), Blob->GetBufferSize(), __uuidof(PipelineLibrary), PipelineLibrary.put_void()));
		return TRUE;
	}
#elif defined(USE_WRL)
	Microsoft::WRL::ComPtr<ID3DBlob> Blob;
	if (SUCCEEDED(D3DReadFileToBlob(Path.data(), Blob.GetAddressOf())) && Blob->GetBufferSize()) {
		VERIFY_SUCCEEDED(Device1->CreatePipelineLibrary(Blob->GetBufferPointer(), Blob->GetBufferSize(), IID_PPV_ARGS(PipelineLibrary.GetAddressOf())));
		return TRUE;
	}
#endif
	else {
#ifdef USE_WINRT
		VERIFY_SUCCEEDED(Device1->CreatePipelineLibrary(nullptr, 0, __uuidof(PipelineLibrary), PipelineLibrary.put_void()));
#elif defined(USE_WRL)
		VERIFY_SUCCEEDED(Device1->CreatePipelineLibrary(nullptr, 0, IID_PPV_ARGS(PipelineLibrary.GetAddressOf())));
#endif
		return FALSE;
	}
}
void DX::StorePipelineLibrary(const std::wstring& Path) const
{
	const auto Size = PipelineLibrary->GetSerializedSize();
	if (Size) {
#ifdef USE_WINRT
		winrt::com_ptr<ID3DBlob> Blob;
		VERIFY_SUCCEEDED(D3DCreateBlob(Size, Blob.put()));
		PipelineLibrary->Serialize(Blob->GetBufferPointer(), Size);
		VERIFY_SUCCEEDED(D3DWriteBlobToFile(Blob.get(), Path.data(), TRUE)); 
#elif defined(USE_WRL)
		Microsoft::WRL::ComPtr<ID3DBlob> Blob;
		VERIFY_SUCCEEDED(D3DCreateBlob(Size, Blob.GetAddressOf()));
		PipelineLibrary->Serialize(Blob->GetBufferPointer(), Size);
		VERIFY_SUCCEEDED(D3DWriteBlobToFile(Blob.Get(), Path.data(), TRUE));
#endif
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

	//!< �V�F�[�_
#ifdef USE_WINRT
	std::vector<winrt::com_ptr<ID3DBlob>> ShaderBlobs;
#elif defined(USE_WRL)
	std::vector<Microsoft::WRL::ComPtr<ID3DBlob>> ShaderBlobs;
#endif
	CreateShader(ShaderBlobs);
	std::array<D3D12_SHADER_BYTECODE, 5> ShaderBCs{ NullShaderBC, NullShaderBC, NullShaderBC, NullShaderBC, NullShaderBC };
	CreateShaderByteCode(ShaderBlobs, ShaderBCs);

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

	//!< �C���v�b�g���C�A�E�g
	std::vector<D3D12_INPUT_ELEMENT_DESC> InputElementDescs;
	CreateInputLayout(InputElementDescs);
	const D3D12_INPUT_LAYOUT_DESC InputLayoutDesc = {
		InputElementDescs.data(), static_cast<UINT>(InputElementDescs.size())
	};

	const DXGI_SAMPLE_DESC SampleDesc = { 1, 0 };
	const D3D12_CACHED_PIPELINE_STATE CachedPipelineState = { nullptr, 0 };
	const D3D12_GRAPHICS_PIPELINE_STATE_DESC GraphicsPipelineStateDesc = {
#ifdef USE_WINRT
		RootSignature.get(),
#elif defined(USE_WRL)
		RootSignature.Get(),
#endif
		ShaderBCs[0], ShaderBCs[1], ShaderBCs[2], ShaderBCs[3], ShaderBCs[4],
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
		D3D12_PIPELINE_STATE_FLAG_NONE //!< D3D12_PIPELINE_STATE_FLAG_TOOL_DEBUG ... �� Warp �f�o�C�X�̂�
	};

	//!< �p�C�v���C���L���b�V���I�u�W�F�N�g Pipeline Cache Object
	const auto PCOPath = GetBasePath() + TEXT(".pco"); //!< �V���A���C�Y�t�@�C���p�X Searialize file path
	const auto PCOName = GetTitleW() + TEXT(".pco"); //!< ���C�u�����f�[�^�x�[�X�֓o�^���閼�O Name to store in library
	DeleteFile(PCOPath.data());
	if(LoadPipelineLibrary(PCOPath)) {
		//!< ���C�u��������p�C�v���C���X�e�[�g���擾���� (�d����h�����߂Ƀ��C�u�����f�[�^�x�[�X��Desc����r�����)
#ifdef USE_WINRT
		VERIFY_SUCCEEDED(PipelineLibrary->LoadGraphicsPipeline(PCOName.data(), &GraphicsPipelineStateDesc, __uuidof(PipelineState), PipelineState.put_void()));
#elif defined(USE_WRL)
		VERIFY_SUCCEEDED(PipelineLibrary->LoadGraphicsPipeline(PCOName.data(), &GraphicsPipelineStateDesc, IID_PPV_ARGS(PipelineState.GetAddressOf())));
#endif
	}
	else {
#ifdef USE_WINRT
		VERIFY_SUCCEEDED(Device->CreateGraphicsPipelineState(&GraphicsPipelineStateDesc, __uuidof(PipelineState), PipelineState.put_void()));
		//!< ���O�w��Ń��C�u�����f�[�^�x�[�X�֒ǉ�����
		VERIFY_SUCCEEDED(PipelineLibrary->StorePipeline(PCOName.data(), PipelineState.get()));
#elif defined(USE_WRL)
		VERIFY_SUCCEEDED(Device->CreateGraphicsPipelineState(&GraphicsPipelineStateDesc, IID_PPV_ARGS(PipelineState.GetAddressOf())));
		VERIFY_SUCCEEDED(PipelineLibrary->StorePipeline(PCOName.data(), PipelineState.Get()));
#endif

		StorePipelineLibrary(PCOPath);
	}

	LogOK("CreatePipelineState_Graphics");
}

void DX::CreatePipelineState_Compute()
{
#ifdef _DEBUG
	PerformanceCounter PC("CreatePipelineState_Compute : ");
#endif
	assert(nullptr != RootSignature && "");

	//!< �V�F�[�_
#ifdef USE_WINRT
	std::vector<winrt::com_ptr<ID3DBlob>> ShaderBlobs;
#elif defined(USE_WRL)
	std::vector<Microsoft::WRL::ComPtr<ID3DBlob>> ShaderBlobs;
#endif
	CreateShader(ShaderBlobs);
	assert(!ShaderBlobs.empty() && "");

	const D3D12_CACHED_PIPELINE_STATE CachedPipelineState = { nullptr, 0 };
	const D3D12_COMPUTE_PIPELINE_STATE_DESC ComputePipelineStateDesc = {
#ifdef USE_WINRT
		RootSignature.get(),
#elif defined(USE_WRL)
		RootSignature.Get(),
#endif
		D3D12_SHADER_BYTECODE({ ShaderBlobs[0]->GetBufferPointer(), ShaderBlobs[0]->GetBufferSize() }),
		0, // NodeMask ... �}���`GPU�̏ꍇ
		CachedPipelineState,
		D3D12_PIPELINE_STATE_FLAG_NONE
	};
#ifdef USE_WINRT
	VERIFY_SUCCEEDED(Device->CreateComputePipelineState(&ComputePipelineStateDesc, __uuidof(PipelineState), PipelineState.put_void()));
#elif defined(USE_WRL)
	VERIFY_SUCCEEDED(Device->CreateComputePipelineState(&ComputePipelineStateDesc, IID_PPV_ARGS(PipelineState.GetAddressOf())));
#endif

	LogOK("CreatePipelineState_Compute");
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
#ifdef USE_WINRT
	const auto CL = GraphicsCommandLists[i].get();
	const auto CA = CommandAllocators[0].get();
	const auto SCR = SwapChainResources[i].get();
	const auto SCHandle = GetCPUDescriptorHandle(SwapChainDescriptorHeap.get(), D3D12_DESCRIPTOR_HEAP_TYPE_RTV, static_cast<UINT>(i));
#elif defined(USE_WRL)
	const auto SCR = SwapChainResources[i].Get();
	const auto SCHandle = GetCPUDescriptorHandle(SwapChainDescriptorHeap.Get(), D3D12_DESCRIPTOR_HEAP_TYPE_RTV, static_cast<UINT>(i));
	const auto CL = GraphicsCommandLists[i].Get();
	const auto CA = CommandAllocators[0].Get();
#endif

	//!< GPU ���Q�Ƃ��Ă���Ԃ́A�R�}���h�A���P�[�^�� Reset() �͂ł��Ȃ�
	//VERIFY_SUCCEEDED(CA->Reset());

	//!< CommandQueue->ExecuteCommandLists() ��� CommandList->Reset() �Ń��Z�b�g���čė��p���\ (�R�}���h�L���[�̓R�}���h���X�g�ł͂Ȃ��A�R�}���h�A���P�[�^���Q�Ƃ��Ă���)
	//!< CommandList �쐬���� PipelineState ���w�肵�Ă��Ȃ��Ă��A�����Ŏw�肷��� OK
#ifdef USE_WINRT
	VERIFY_SUCCEEDED(CL->Reset(CA, PipelineState.get()));
#elif defined(USE_WRL)
	VERIFY_SUCCEEDED(CL->Reset(CA, PipelineState.Get()));
#endif
	{
		//!< �r���[�|�[�g�A�V�U�[
		CL->RSSetViewports(static_cast<UINT>(Viewports.size()), Viewports.data());
		CL->RSSetScissorRects(static_cast<UINT>(ScissorRects.size()), ScissorRects.data());

		//!< �o���A
		ResourceBarrier(CL, SCR, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET); {
#if 1
			//!< �N���A
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

#ifdef USE_WINRT
	const std::vector<ID3D12CommandList*> CLs = { GraphicsCommandLists[CurrentBackBufferIndex].get() };
#elif defined(USE_WRL)
	const std::vector<ID3D12CommandList*> CLs = { GraphicsCommandLists[CurrentBackBufferIndex].Get() };
#endif
	CommandQueue->ExecuteCommandLists(static_cast<UINT>(CLs.size()), CLs.data());
	
	Present();
}
void DX::Dispatch()
{
	//!< #DX_TODO Dispatch����
	DEBUG_BREAK();
}
void DX::Present()
{
	VERIFY_SUCCEEDED(SwapChain->Present(1, 0));
}
void DX::WaitForFence()
{
	//!< CPU ���̃t�F���X�l���C���N�������g
	++FenceValue;

	//!< GPU �R�}���h�� Signal() �܂œ��B����� GetCompletedValue() �� FenceValue �ɂȂ�ACPU�ɒǂ��������ƂɂȂ�
#ifdef USE_WINRT
	VERIFY_SUCCEEDED(CommandQueue->Signal(Fence.get(), FenceValue));
#elif defined(USE_WRL)
	VERIFY_SUCCEEDED(CommandQueue->Signal(Fence.Get(), FenceValue));
#endif
	if (Fence->GetCompletedValue() < FenceValue) {
		auto hEvent = CreateEventEx(nullptr, nullptr, 0, EVENT_ALL_ACCESS);
		//!< GetCompletedValue() �� FenceValue �ɂȂ�����C�x���g�����s�����
		VERIFY_SUCCEEDED(Fence->SetEventOnCompletion(FenceValue, hEvent));

		//!< �C�x���g���s�܂ő҂�
		WaitForSingleObject(hEvent, INFINITE);
		CloseHandle(hEvent);
	}
}
