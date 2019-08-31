#include "stdafx.h"

#include "DX.h"

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "dxgi.lib")

void DX::OnCreate(HWND hWnd, HINSTANCE hInstance, LPCWSTR Title)
{
	PERFORMANCE_COUNTER();

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
	CreateCommandAllocator();
	CreateCommandList();
	InitializeSwapChain();

	//!< �f�v�X
	CreateDepthStencil();
	
	//!< ���_
	CreateVertexBuffer();
	CreateIndexBuffer();
	CreateIndirectBuffer();

	CreateTexture();
	CreateStaticSampler();

	//!< ���[�g�V�O�l�`�� (�p�C�v���C���g���C�A�E�g����)
	CreateRootSignature();
	CreateShaderBlob();
	//!< �p�C�v���C��
	CreatePipelineState();

	//!< �R���X�^���g�o�b�t�@���\�[�X�̍쐬 (���j�t�H�[���o�b�t�@����)
	CreateConstantBuffer();
	//!< �f�X�N���v�^�q�[�v (�f�X�N���v�^�v�[������)
	CreateDescriptorHeap();
	//!< �f�X�N���v�^�r���[ (�f�X�N���v�^�Z�b�g����)
	CreateDescriptorView();
	UpdateDescriptorHeap();

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
	PERFORMANCE_COUNTER();

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

#ifdef _DEBUG
	CheckFeatureLevel();
#endif

	LOG_OK();
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
D3D12_GPU_DESCRIPTOR_HANDLE DX::GetGPUDescriptorHandle(ID3D12DescriptorHeap* DescriptorHeap, const D3D12_DESCRIPTOR_HEAP_TYPE Type, const UINT Index) const
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

	LOG_OK();
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

	LOG_OK();
}

//!< �R�}���h���s(CL->ExecuteCommandList())��AGPU���R�}���h�A���P�[�^�̎Q�Ƃ��I����܂ŁA�A���P�[�^�̃��Z�b�g(CA->Reset())���Ă͂����Ȃ��A�A���P�[�^���o���Ă���̂ŃR�}���h�̃��Z�b�g(CL->Reset())�͂��Ă��ǂ�
void DX::CreateCommandAllocator()
{
	CommandAllocators.resize(1);

	CreateCommandAllocator(CommandAllocators[0], D3D12_COMMAND_LIST_TYPE_DIRECT);

	LOG_OK();
}

#ifdef USE_WINRT
void DX::CreateCommandList(winrt::com_ptr<ID3D12GraphicsCommandList>& CL, ID3D12CommandAllocator* CA, const D3D12_COMMAND_LIST_TYPE CLT)
{
	//!< �`��R�}���h�𔭍s����R�}���h���X�g�ɂ̓p�C�v���C���X�e�[�g�̎w�肪�K�v�����A�ォ��w��(CL->Reset(CA, PS.get()))���ł��� (�����ł�nullptr�ō쐬���邱�Ƃɂ���)
	VERIFY_SUCCEEDED(Device->CreateCommandList(0, CLT, CA, nullptr, __uuidof(CL), CL.put_void()));
	VERIFY_SUCCEEDED(CL->Close());
}
#elif defined(USE_WRL)
void DX::CreateCommandList(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>& CL, ID3D12CommandAllocator* CA, const D3D12_COMMAND_LIST_TYPE CLT)
{
	VERIFY_SUCCEEDED(Device->CreateCommandList(0, CLT, CA, nullptr, IID_PPV_ARGS(CL.GetAddressOf())));
	VERIFY_SUCCEEDED(CL->Close());
}
#endif
void DX::CreateCommandList()
{
	DXGI_SWAP_CHAIN_DESC1 SCD;
	SwapChain->GetDesc1(&SCD);

	GraphicsCommandLists.resize(SCD.BufferCount);
	for (UINT i = 0; i < SCD.BufferCount; ++i) {
#ifdef USE_WINRT
		CreateCommandList(GraphicsCommandLists[i], CommandAllocators[0].get(), D3D12_COMMAND_LIST_TYPE_DIRECT);
#elif defined(USE_WRL)
		CreateCommandList(GraphicsCommandLists[i], CommandAllocators[0].Get(), D3D12_COMMAND_LIST_TYPE_DIRECT);
#endif
	}

	LOG_OK();
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

	LOG_OK();
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

	LOG_OK();
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

	LOG_OK();
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

	LOG_OK();
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

	LOG_OK();
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
	const auto CDH = GetCPUDescriptorHandle(DepthStencilDescriptorHeap.get(), D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 0);
	//!< �f�X�N���v�^(�r���[)�̍쐬�B���\�[�X��ł̃I�t�Z�b�g���w�肵�č쐬���Ă���A���ʂ��ϐ��ɕԂ�킯�ł͂Ȃ�
	//!< (���\�[�X���^�C�v�h�t�H�[�}�b�g�Ȃ� D3D12_DEPTH_STENCIL_VIEW_DESC* �� nullptr �w��\)
	Device->CreateDepthStencilView(DepthStencilResource.get(), nullptr, CDH); 
#elif defined(USE_WRL)
	const auto CDH = GetCPUDescriptorHandle(DepthStencilDescriptorHeap.Get(), D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 0);
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

	LOG_OK();
}
void DX::ResizeDepthStencil(const DXGI_FORMAT DepthFormat, const UINT Width, const UINT Height)
{
#ifdef USE_WINRT
	DepthStencilResource = nullptr;
#elif defined(USE_WRL)
	DepthStencilResource.Reset();
#endif

	CreateDepthStencilResource(DepthFormat, Width, Height);

	LOG_OK();
}

void DX::CreateBuffer(ID3D12Resource** Res, const UINT32 Size, const void* Source, ID3D12CommandAllocator* CA, ID3D12GraphicsCommandList* CL)
{
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
	CreateDefaultResource(Res, Size);

	//!< �A�b�v���[�h���\�[�X����f�t�H���g���\�[�X�ւ̃R�s�[�R�}���h�𔭍s Execute copy command upload resource to default resource
#ifdef USE_WINRT
	ExecuteCopyBuffer(CA, CL, UploadRes.get(), *Res, Size);
#elif defined(USE_WRL)
	ExecuteCopyBuffer(CA, CL, UploadRes.Get(), *Res, Size);
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

	LOG_OK();
}

/**
std::vector<ID3D12DescriptorHeap*> DescriptorHeaps = { ConstantBufferDescriptorHeap.Get() };
GraphicsCommandList->SetDescriptorHeaps(static_cast<UINT>(DescriptorHeaps.size()), DescriptorHeaps.data());

auto CVDescriptorHandle(ConstantBufferDescriptorHeap->GetGPUDescriptorHandleForHeapStart());
const auto CVIncrementSize = Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
CVDescriptorHandle.ptr += 0 * CVIncrementSize;
GraphicsCommandList->SetGraphicsRootDescriptorTable(0, CVDescriptorHandle);
*/

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

	LOG_OK();
}

//!< ���[�g�V�O�l�`�����V���A���C�Y���ău���u�����
void DX::SerializeRootSignature(
#ifdef USE_WINRT 
	winrt::com_ptr<ID3DBlob>& Blob,
#elif defined(USE_WRL) 
	Microsoft::WRL::ComPtr<ID3DBlob>& Blob,
#endif 
	const std::initializer_list<D3D12_ROOT_PARAMETER> il_RPs, const std::initializer_list<D3D12_STATIC_SAMPLER_DESC> il_SSDs, const D3D12_ROOT_SIGNATURE_FLAGS Flags)
{
	//!< RangeType ... D3D12_DESCRIPTOR_RANGE_TYPE_[SRV, UAV, CBV, SAMPLER]
	//!< NumDescriptors
	//!< BaseShaderRegister ... register(b0)�Ȃ� 0�Aregister(t3) �Ȃ� 3
	//!< RegisterSpace ... �ʏ�� 0 �ł悢 register(t3, space5) �Ȃ� 5
	//!< OffsetInDescriptorsFromTableStart ... �ʏ�� D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND �ł悢
	const std::vector<D3D12_ROOT_PARAMETER> RPs(il_RPs.begin(), il_RPs.end());
	const std::vector<D3D12_STATIC_SAMPLER_DESC> SSDs(il_SSDs.begin(), il_SSDs.end());

	const D3D12_ROOT_SIGNATURE_DESC RSD = {
			static_cast<UINT>(RPs.size()), RPs.data(),
			static_cast<UINT>(SSDs.size()), SSDs.data(),
			D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT
	};

#ifdef USE_WINRT
	winrt::com_ptr<ID3DBlob> ErrorBlob;
	VERIFY_SUCCEEDED(D3D12SerializeRootSignature(&RSD, D3D_ROOT_SIGNATURE_VERSION_1, Blob.put(), ErrorBlob.put()));
#elif defined(USE_WRL)
	Microsoft::WRL::ComPtr<ID3DBlob> ErrorBlob;
	VERIFY_SUCCEEDED(D3D12SerializeRootSignature(&RSD, D3D_ROOT_SIGNATURE_VERSION_1, Blob.GetAddressOf(), ErrorBlob.GetAddressOf()));
#endif

	LOG_OK();
}

//!< �V�F�[�_���烋�[�g�V�O�l�`���p�[�g�����o���u���u�����
#ifdef USE_WINRT
void DX::GetRootSignaturePartFromShader(winrt::com_ptr<ID3DBlob>& Blob, LPCWSTR Path)
{
	winrt::com_ptr<ID3DBlob> ShaderBlob;
	VERIFY_SUCCEEDED(D3DReadFileToBlob(Path, ShaderBlob.put()));
	VERIFY_SUCCEEDED(D3DGetBlobPart(ShaderBlob->GetBufferPointer(), ShaderBlob->GetBufferSize(), D3D_BLOB_ROOT_SIGNATURE, 0, Blob.put()));
	LOG_OK();
}
#elif defined(USE_WRL)
void DX::GetRootSignaturePartFromShader(Microsoft::WRL::ComPtr<ID3DBlob>& Blob, LPCWSTR Path)
{
	Microsoft::WRL::ComPtr<ID3DBlob> ShaderBlob;
	VERIFY_SUCCEEDED(D3DReadFileToBlob(Path, ShaderBlob.GetAddressOf()));
	VERIFY_SUCCEEDED(D3DGetBlobPart(ShaderBlob->GetBufferPointer(), ShaderBlob->GetBufferSize(), D3D_BLOB_ROOT_SIGNATURE, 0, Blob.GetAddressOf()));
	LOG_OK();
}
#endif

/**
@brief �V�F�[�_�Ƃ̃o�C���f�B���O (VK::CreateDescriptorSetLayout() ����)
*/
void DX::CreateRootSignature()
{
#ifdef USE_WINRT
	winrt::com_ptr<ID3DBlob> Blob;
#elif defined(USE_WRL)
	Microsoft::WRL::ComPtr<ID3DBlob> Blob;
#endif

#ifdef USE_HLSL_ROOTSIGNATRUE
	GetRootSignaturePartFromShader(Blob, (GetBasePath() + TEXT(".rs.cso")).data());
#else
	SerializeRootSignature(Blob, {}, {}, D3D12_ROOT_SIGNATURE_FLAG_NONE);
#endif

	CreateRootSignature(RootSignature, Blob);

	LOG_OK();
}

#ifdef USE_WINRT
void DX::CreateShader(std::vector<winrt::com_ptr<ID3DBlob>>& ShaderBlobs) const
#elif defined(USE_WRL)
void DX::CreateShader(std::vector<Microsoft::WRL::ComPtr<ID3DBlob>>& ShaderBlobs) const
#endif
{
	for (auto i : ShaderBlobs) {
		//!< PDB�p�[�g�A�����ꍇ������̂� HRESULT �� VERIFY ���Ȃ�
#ifdef USE_WINRT
		winrt::com_ptr<ID3DBlob> PDBPart;
		const auto HR = D3DGetBlobPart(i->GetBufferPointer(), i->GetBufferSize(), D3D_BLOB_PDB, 0, PDBPart.put());
#elif defined(USE_WRL)
		Microsoft::WRL::ComPtr<ID3DBlob> PDBPart;
		const auto HR = D3DGetBlobPart(i->GetBufferPointer(), i->GetBufferSize(), D3D_BLOB_PDB, 0, PDBPart.GetAddressOf());
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

void DX::CreatePipelineState()
{
	const auto PCOPath = GetBasePath() + TEXT(".pco");
	//DeleteFile(PCOPath.data());
#ifdef USE_WINRT
	winrt::com_ptr<ID3D12Device1> Device1;
	VERIFY_SUCCEEDED(Device->QueryInterface(__uuidof(Device1), Device1.put_void()));

	//!< �p�C�v���C�����C�u�������t�@�C������ǂݍ��ށA�ǂݍ��߂Ȃ��ꍇ�͐V���ɍ쐬����
	winrt::com_ptr<ID3D12PipelineLibrary> PL;
	winrt::com_ptr<ID3DBlob> Blob;
	if (SUCCEEDED(D3DReadFileToBlob(PCOPath.c_str(), Blob.put())) && Blob->GetBufferSize()) {
		VERIFY_SUCCEEDED(Device1->CreatePipelineLibrary(Blob->GetBufferPointer(), Blob->GetBufferSize(), __uuidof(PL), PL.put_void()));

		//!< ���C�u��������p�C�v���C���X�e�[�g��ǂݍ���
		winrt::com_ptr<ID3D12PipelineState> PS0, PS1;
		const D3D12_GRAPHICS_PIPELINE_STATE_DESC GPSD = {};
		VERIFY_SUCCEEDED(PL->LoadGraphicsPipeline(TEXT("0"), &GPSD, __uuidof(PS0), PS0.put_void()));
		VERIFY_SUCCEEDED(PL->LoadGraphicsPipeline(TEXT("1"), &GPSD, __uuidof(PS1), PS1.put_void()));
	}
	else {
		VERIFY_SUCCEEDED(Device1->CreatePipelineLibrary(nullptr, 0, __uuidof(PipelineLibrary), PL.put_void()));
		
		//!< �����ł� �p�C�v���C���X�e�[�g PS0, PS1 ���쐬�����Ɖ���
		winrt::com_ptr<ID3D12PipelineState> PS0, PS1;

		//!< �p�C�v���C���X�e�[�g �� ���C�u���� �֓o�^
		VERIFY_SUCCEEDED(PL->StorePipeline(TEXT("0"), PS0.get()));
		VERIFY_SUCCEEDED(PL->StorePipeline(TEXT("1"), PS1.get()));

		//!< ���C�u�������t�@�C���֏�������
		const auto Size = PL->GetSerializedSize();
		if (Size) {
			winrt::com_ptr<ID3DBlob> Blob;
			VERIFY_SUCCEEDED(D3DCreateBlob(Size, Blob.put()));
			PL->Serialize(Blob->GetBufferPointer(), Size);
			VERIFY_SUCCEEDED(D3DWriteBlobToFile(Blob.get(), PCOPath.c_str(), TRUE));
		}
	}
#elif defined(USE_WRL)
	Microsoft::WRL::ComPtr<ID3D12Device1> Device1;
	VERIFY_SUCCEEDED(Device->QueryInterface(IID_PPV_ARGS(Device1.GetAddressOf())));

	Microsoft::WRL::ComPtr<ID3D12PipelineLibrary> PL;
	Microsoft::WRL::ComPtr<ID3DBlob> Blob;
	if (SUCCEEDED(D3DReadFileToBlob(PCOPath.c_str(), Blob.GetAddressOf())) && Blob->GetBufferSize()) {
		VERIFY_SUCCEEDED(Device1->CreatePipelineLibrary(Blob->GetBufferPointer(), Blob->GetBufferSize(), IID_PPV_ARGS(PL.GetAddressOf())));

		Microsoft::WRL::ComPtr<ID3D12PipelineState> PS0, PS1;
		const D3D12_GRAPHICS_PIPELINE_STATE_DESC GPSD = {};
		VERIFY_SUCCEEDED(PL->LoadGraphicsPipeline(TEXT("0"), &GPSD, IID_PPV_ARGS(PS0.GetAddressOf())));
		VERIFY_SUCCEEDED(PL->LoadGraphicsPipeline(TEXT("1"), &GPSD, IID_PPV_ARGS(PS1.GetAddressOf())));
	}
	else {
		VERIFY_SUCCEEDED(Device1->CreatePipelineLibrary(nullptr, 0, IID_PPV_ARGS(PL.GetAddressOf())));

		Microsoft::WRL::ComPtr<ID3D12PipelineState> PS0, PS1;

		VERIFY_SUCCEEDED(PL->StorePipeline(TEXT("0"), PS0.Get()));
		VERIFY_SUCCEEDED(PL->StorePipeline(TEXT("1"), PS1.Get()));

		const auto Size = PL->GetSerializedSize();
		if (Size) {
			Microsoft::WRL::ComPtr<ID3DBlob> Blob;
			VERIFY_SUCCEEDED(D3DCreateBlob(Size, Blob.GetAddressOf()));
			PL->Serialize(Blob->GetBufferPointer(), Size);
			VERIFY_SUCCEEDED(D3DWriteBlobToFile(Blob.Get(), PCOPath.c_str(), TRUE));
		}
	}
#endif

	const auto ShaderPath = GetBasePath();
#ifdef USE_WINRT
	ShaderBlobs.resize(5);
	VERIFY_SUCCEEDED(D3DReadFileToBlob((ShaderPath + TEXT(".vs.cso")).data(), ShaderBlobs[0].put()));
	VERIFY_SUCCEEDED(D3DReadFileToBlob((ShaderPath + TEXT(".ps.cso")).data(), ShaderBlobs[1].put()));
	VERIFY_SUCCEEDED(D3DReadFileToBlob((ShaderPath + TEXT(".ds.cso")).data(), ShaderBlobs[2].put()));
	VERIFY_SUCCEEDED(D3DReadFileToBlob((ShaderPath + TEXT(".hs.cso")).data(), ShaderBlobs[3].put()));
	VERIFY_SUCCEEDED(D3DReadFileToBlob((ShaderPath + TEXT(".gs.cso")).data(), ShaderBlobs[4].put()));
#elif defined(USE_WRL)
	VERIFY_SUCCEEDED(D3DReadFileToBlob((ShaderPath + TEXT(".vs.cso")).data(), ShaderBlobs[0].GetAddressOf()));
	VERIFY_SUCCEEDED(D3DReadFileToBlob((ShaderPath + TEXT(".ps.cso")).data(), ShaderBlobs[1].GetAddressOf()));
#endif
	const std::array<D3D12_SHADER_BYTECODE, 5> SBCs = { {
		{ ShaderBlobs[0]->GetBufferPointer(), ShaderBlobs[0]->GetBufferSize() },
		{ ShaderBlobs[1]->GetBufferPointer(), ShaderBlobs[1]->GetBufferSize() },
		{ ShaderBlobs[2]->GetBufferPointer(), ShaderBlobs[2]->GetBufferSize() },
		{ ShaderBlobs[3]->GetBufferPointer(), ShaderBlobs[3]->GetBufferSize() },
		{ ShaderBlobs[4]->GetBufferPointer(), ShaderBlobs[4]->GetBufferSize() },
	} };
	auto Thread = std::thread::thread([&](winrt::com_ptr<ID3D12PipelineState>& Pipe, ID3D12RootSignature* RS,
		const D3D12_SHADER_BYTECODE VS, const D3D12_SHADER_BYTECODE PS, const D3D12_SHADER_BYTECODE DS, const D3D12_SHADER_BYTECODE HS, const D3D12_SHADER_BYTECODE GS)
		{ CreatePipelineState_Default(Pipe, RS, VS, PS, DS, HS, GS); },
#ifdef USE_WINRT
		std::ref(PipelineState), RootSignature.get(), SBCs[0], NullShaderBC, NullShaderBC, NullShaderBC, NullShaderBC);
#elif defined(USE_WRL)
		std::ref(PipelineState), RootSignature.Get(), NullShaderBC, NullShaderBC, NullShaderBC, NullShaderBC, NullShaderBC);
#endif

	Thread.join();
}
void DX::CreatePipelineState_Default(winrt::com_ptr<ID3D12PipelineState>& PipelineState, ID3D12RootSignature* RS, 
	const D3D12_SHADER_BYTECODE VS, const D3D12_SHADER_BYTECODE PS, const D3D12_SHADER_BYTECODE DS, const D3D12_SHADER_BYTECODE HS, const D3D12_SHADER_BYTECODE GS)
{
	PERFORMANCE_COUNTER();

	assert((VS.pShaderBytecode != nullptr && VS.BytecodeLength) && "");

	//!< �X�g���[���A�E�g�v�b�g (StreamOutput)
	const D3D12_STREAM_OUTPUT_DESC SOD = {
		nullptr, 0,
		nullptr, 0,
		0
	};

	//!< �u�����h (Blend)
	const D3D12_RENDER_TARGET_BLEND_DESC RTBD = {
		FALSE, FALSE,
		D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
		D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
		D3D12_LOGIC_OP_NOOP,
		D3D12_COLOR_WRITE_ENABLE_ALL,
	};
	const D3D12_BLEND_DESC BD = {
		FALSE,
		FALSE,
		{ RTBD/*, ... x8*/ }
	};

	//!< ���X�^���C�U (Rasterizer)
	const D3D12_RASTERIZER_DESC RD = {
		D3D12_FILL_MODE_SOLID,
		D3D12_CULL_MODE_BACK, TRUE,
		D3D12_DEFAULT_DEPTH_BIAS, D3D12_DEFAULT_DEPTH_BIAS_CLAMP, D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS,
		TRUE,
		FALSE,
		FALSE,
		0,
		D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF
	};

	//!< �f�v�X�X�e���V�� (DepthStencil)
	const D3D12_DEPTH_STENCILOP_DESC DSOD = {
		D3D12_STENCIL_OP_KEEP,
		D3D12_STENCIL_OP_KEEP,
		D3D12_STENCIL_OP_KEEP,
		D3D12_COMPARISON_FUNC_NEVER
	};
	const D3D12_DEPTH_STENCIL_DESC DSD = {
		FALSE,
		D3D12_DEPTH_WRITE_MASK_ZERO,
		D3D12_COMPARISON_FUNC_NEVER,
		FALSE,
		0,
		0,
		DSOD,
		DSOD
	};

	//!< �C���v�b�g���C�A�E�g (InputLayout)
	const std::array<D3D12_INPUT_ELEMENT_DESC, 0> IEDs = {};
	const D3D12_INPUT_LAYOUT_DESC ILD = {
		IEDs.data(), static_cast<UINT>(IEDs.size())
	};

	//!< �T���v�� (Sample)
	const DXGI_SAMPLE_DESC SD = { 1, 0 };

	//!< �L���b�V���h�p�C�v���C���X�e�[�g (CachedPipelineState)
	//!< (VK �� VkGraphicsPipelineCreateInfo.basePipelineHandle, basePipelineIndex ����?)
#if 0
#ifdef USE_WINRT
	winrt::com_ptr<ID3DBlob> PipelineBlob;
	VERIFY_SUCCEEDED(BasePipelineState->GetCachedBlob(PipelineBlob.put()));
#elif defined(USE_WRL)
	Microsoft::WRL::ComPtr<ID3DBlob> PipelineBlob;
	VERIFY_SUCCEEDED(BasePipelineState->GetCachedBlob(PipelineBlob.GetAddressOf()));
#endif
	const D3D12_CACHED_PIPELINE_STATE CPS = { PipelineBlob->GetBufferPointer(), PipelineBlob->GetBufferSize() };
#else
	const D3D12_CACHED_PIPELINE_STATE CPS = { nullptr, 0 };
#endif

	const D3D12_GRAPHICS_PIPELINE_STATE_DESC GPSD = {
		RS,
		VS, PS, DS, HS, GS,
		SOD,
		BD,
		UINT_MAX,
		RD,
		DSD,
		ILD,
		D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED,
		D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE,
		1, { DXGI_FORMAT_R8G8B8A8_UNORM }, DXGI_FORMAT_D32_FLOAT_S8X24_UINT,
		SD,
		0,
		CPS,
		D3D12_PIPELINE_STATE_FLAG_NONE //!< D3D12_PIPELINE_STATE_FLAG_TOOL_DEBUG ... �� Warp �f�o�C�X�̂�
	};
	assert(GPSD.NumRenderTargets <= _countof(GPSD.RTVFormats) && "");
	assert((0 == GPSD.DS.BytecodeLength || 0 == GPSD.HS.BytecodeLength || GPSD.PrimitiveTopologyType == D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH) && "");

#ifdef USE_WINRT
	VERIFY_SUCCEEDED(Device->CreateGraphicsPipelineState(&GPSD, __uuidof(PipelineState), PipelineState.put_void()));
#elif defined(USE_WRL)
	VERIFY_SUCCEEDED(Device->CreateGraphicsPipelineState(&GPSD, IID_PPV_ARGS(PipelineState.GetAddressOf())));
#endif

	LOG_OK();
}
#if 0
void DX::CreatePipelineState_Compute()
{
	PERFORMANCE_COUNTER();

	assert(nullptr != RootSignature && "");

	//!< �V�F�[�_
#ifdef USE_WINRT
	std::vector<winrt::com_ptr<ID3DBlob>> ShaderBlobs;
#elif defined(USE_WRL)
	std::vector<Microsoft::WRL::ComPtr<ID3DBlob>> ShaderBlobs;
#endif
	CreateShader(ShaderBlobs);
	assert(!ShaderBlobs.empty() && "");

	const D3D12_CACHED_PIPELINE_STATE CPS = { nullptr, 0 };
	const D3D12_COMPUTE_PIPELINE_STATE_DESC CPSD = {
#ifdef USE_WINRT
		RootSignature.get(),
#elif defined(USE_WRL)
		RootSignature.Get(),
#endif
		D3D12_SHADER_BYTECODE({ ShaderBlobs[0]->GetBufferPointer(), ShaderBlobs[0]->GetBufferSize() }),
		0, // NodeMask ... �}���`GPU�̏ꍇ
		CPS,
		D3D12_PIPELINE_STATE_FLAG_NONE
	};
#ifdef USE_WINRT
	VERIFY_SUCCEEDED(Device->CreateComputePipelineState(&CPSD, __uuidof(PipelineState), PipelineState.put_void()));
#elif defined(USE_WRL)
	VERIFY_SUCCEEDED(Device->CreateComputePipelineState(&CPSD, IID_PPV_ARGS(PipelineState.GetAddressOf())));
#endif

	LOG_OK();
}
#endif

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
#ifdef _DEBUG
	//PerformanceCounter PC(__func__);
#endif

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
