//#include "framework.h"
//#include "stdafx.h"

#include "DX.h"

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "dxgi.lib")

void DX::OnCreate(HWND hWnd, HINSTANCE hInstance, LPCWSTR Title)
{
	PERFORMANCE_COUNTER();

	Super::OnCreate(hWnd, hInstance, Title);

#ifdef USE_HDR
	const auto ColorFormat = DXGI_FORMAT_R16G16B16A16_FLOAT;
	//const auto ColorFormat = DXGI_FORMAT_R10G10B10A2_UNORM; //!< �V�F�[�_�ŃK���}�␳���K�v
#else
	const auto ColorFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
#endif

	//!< �f�o�C�X
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
	
	CreateRenderTarget();

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

	//!< �R�}���h���X�g�̊�����҂�
	{
		const UINT64 InitValue = 0;
		const UINT64 ExpectValue = 1;
		//!< ��p�̃t�F���X��V�K�ɍ쐬
		COM_PTR<ID3D12Fence> TmpFence;
		VERIFY_SUCCEEDED(Device->CreateFence(InitValue, D3D12_FENCE_FLAG_NONE, COM_PTR_UUIDOF_PUTVOID(TmpFence)));
		VERIFY_SUCCEEDED(CommandQueue->Signal(COM_PTR_GET(TmpFence), ExpectValue));
		if (TmpFence->GetCompletedValue() != ExpectValue) {
			auto hEvent = CreateEventEx(nullptr, nullptr, 0, EVENT_ALL_ACCESS);
			if (nullptr != hEvent) {
				VERIFY_SUCCEEDED(TmpFence->SetEventOnCompletion(ExpectValue, hEvent));
				WaitForSingleObject(hEvent, INFINITE);
				CloseHandle(hEvent);
			}
		}
	}

	ResizeSwapChain(Rect);

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
					static_cast<SIZE_T>(PSF.Footprint.RowPitch) * RowCount
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

void DX::CreateDevice(HWND /*hWnd*/)
{
#if defined(_DEBUG) || defined(USE_PIX)
	if (SUCCEEDED(DXGIGetDebugInterface1(0, COM_PTR_UUIDOF_PUTVOID(GraphicsAnalysis)))) {
		//!< �O���t�B�b�N�X�f�f�� Alt + F5 �ŋN�������ꍇ�̂ݐ������� (Enabled only if executed with Alt + F5)
		Log("Graphics Analysis is enabled\n");
		//!< GraphicsAnalysis->BeginCapture(), GraphicsAnalysis->EndCapture() �ŃL���v�`���J�n�A�I������
	}
#endif

#ifdef _DEBUG
	COM_PTR<ID3D12Debug> Debug;
	VERIFY_SUCCEEDED(D3D12GetDebugInterface(COM_PTR_UUIDOF_PUTVOID(Debug)));
	Debug->EnableDebugLayer();

	//!< GPU-Based Validation
	COM_PTR<ID3D12Debug1> Debug1;
	VERIFY_SUCCEEDED(Debug->QueryInterface(COM_PTR_UUIDOF_PUTVOID(Debug1)));
	Debug1->SetEnableGPUBasedValidation(true);
#endif

	//!< WARP �A�_�v�^���쐬����̂� IDXGIFactory4(��EnumWarpAdapter) ���K�v
	VERIFY_SUCCEEDED(CreateDXGIFactory1(COM_PTR_UUIDOF_PUTVOID(Factory)));

#ifdef _DEBUG
	EnumAdapter(COM_PTR_GET(Factory));
#endif

#ifdef USE_WARP
	//!< WARP : Win7�ȉ����� D3D_FEATURE_LEVEL_10_1 �܂ŁAWin8�ȏゾ�� D3D_FEATURE_LEVEL_11_1 �܂ŃT�|�[�g
	VERIFY_SUCCEEDED(Factory->EnumWarpAdapter(COM_PTR_UUIDOF_PUTVOID(Adapter)));
#else
	//!< VideoMemory�̍ł��傫�ȃA�_�v�^�[(GPU)��I������
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

	//!< �ŏ��Ɍ��������A�E�g�v�b�g(Monitor)��I������
	COM_PTR<IDXGIAdapter> A;
	for (UINT i = 0; DXGI_ERROR_NOT_FOUND != Factory->EnumAdapters(i, COM_PTR_PUT(A)); ++i) {
		for (UINT j = 0; DXGI_ERROR_NOT_FOUND != A->EnumOutputs(j, COM_PTR_PUT(Output)); ++j) {
			if (nullptr != Output) {
				break;
			}
			COM_PTR_RESET(Output);
		}
		COM_PTR_RESET(A);
		if (nullptr != Output) {
			break;
		}
	}

	//!< �I�������A�E�g�v�b�g�̃f�B�X�v���C���[�h���
	GetDisplayModeList(COM_PTR_GET(Output), DXGI_FORMAT_R8G8B8A8_UNORM);

#if 0
	const std::vector<UUID> Experimental = { D3D12ExperimentalShaderModels, /*D3D12RaytracingPrototype*/ };
	VERIFY_SUCCEEDED(D3D12EnableExperimentalFeatures(static_cast<UINT>(Experimental.size()), Experimental.data(), nullptr, nullptr));
#endif

	//!< �����t�B�[�`���[���x���D��Ńf�o�C�X���쐬
	for (const auto i : FeatureLevels) {
		if (SUCCEEDED(D3D12CreateDevice(COM_PTR_GET(Adapter), i, COM_PTR_UUIDOF_PUTVOID(Device)))) {
			break;
		}
	}
	CheckFeatureLevel(COM_PTR_GET(Device));

	LOG_OK();
}

//!< �A�_�v�^(GPU)�̗�
void DX::EnumAdapter(IDXGIFactory4* Fact)
{
	COM_PTR<IDXGIAdapter> Ad;
	for (UINT i = 0; DXGI_ERROR_NOT_FOUND != Fact->EnumAdapters(i, COM_PTR_PUT(Ad)); ++i) {
		DXGI_ADAPTER_DESC AdapterDesc;
		VERIFY_SUCCEEDED(Ad->GetDesc(&AdapterDesc));
		if (0 == i) { Log("[ Aadapters ]\n"); }
		Logf(TEXT("\t%s\n"), AdapterDesc.Description);
		Logf(TEXT("\t\tDedicatedVideoMemory = %lld\n"), AdapterDesc.DedicatedVideoMemory);
		Logf(TEXT("\t\tDedicatedSystemMemory = %lld\n"), AdapterDesc.DedicatedSystemMemory);
		Logf(TEXT("\t\tSharedSystemMemory = %lld\n"), AdapterDesc.SharedSystemMemory);

		EnumOutput(COM_PTR_GET(Ad));
		COM_PTR_RESET(Ad);
	}
}

//!< �A�_�v�^�[(GPU)�ɐڑ�����Ă���A�A�E�g�v�b�g(�f�B�X�v���C)�̗�
void DX::EnumOutput(IDXGIAdapter* Ad)
{
	COM_PTR<IDXGIOutput> Outp;
	for (UINT i = 0; DXGI_ERROR_NOT_FOUND != Ad->EnumOutputs(i, COM_PTR_PUT(Outp)); ++i) {
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

		COM_PTR_RESET(Outp);
	}
}

//!< �A�E�g�v�b�g(�f�B�X�v���C)�̕`�惂�[�h�̗�
void DX::GetDisplayModeList(IDXGIOutput* Outp, const DXGI_FORMAT Format)
{
	UINT Count = 0;
	VERIFY_SUCCEEDED(Outp->GetDisplayModeList(Format, 0, &Count, nullptr));
	if (Count) {
		Log("[ DisplayModes ]\n");

		std::vector<DXGI_MODE_DESC> MDs(Count);
		VERIFY_SUCCEEDED(Outp->GetDisplayModeList(Format, 0, &Count, MDs.data()));
		for (const auto& i : MDs) {
			Logf("\t%d x %d @ %d, ", i.Width, i.Height, i.RefreshRate.Numerator / i.RefreshRate.Denominator);

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
	//!< NumFeatureLevels, pFeatureLevelsRequested �� CheckFeatureSupport() �ւ̓��́AMaxSupportedFeatureLevel �ɂ͏o�͂��Ԃ�
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

D3D12_CPU_DESCRIPTOR_HANDLE DX::GetCPUDescriptorHandle(ID3D12DescriptorHeap* DescriptorHeap, const D3D12_DESCRIPTOR_HEAP_TYPE Type, const UINT Index) const
{
	auto DescriptorHandle(DescriptorHeap->GetCPUDescriptorHandleForHeapStart());
	DescriptorHandle.ptr += static_cast<SIZE_T>(Index) * Device->GetDescriptorHandleIncrementSize(Type);
	return DescriptorHandle;
}
D3D12_GPU_DESCRIPTOR_HANDLE DX::GetGPUDescriptorHandle(ID3D12DescriptorHeap* DescriptorHeap, const D3D12_DESCRIPTOR_HEAP_TYPE Type, const UINT Index) const
{
	auto DescriptorHandle(DescriptorHeap->GetGPUDescriptorHandleForHeapStart());
	DescriptorHandle.ptr += static_cast<SIZE_T>(Index) * Device->GetDescriptorHandleIncrementSize(Type);
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
	VERIFY_SUCCEEDED(Device->CreateCommandQueue(&CommandQueueDesc, COM_PTR_UUIDOF_PUTVOID(CommandQueue)));

	LOG_OK();
}

/**
@brief CPU �� GPU �̓����p
*/
void DX::CreateFence()
{
	VERIFY_SUCCEEDED(Device->CreateFence(0, D3D12_FENCE_FLAG_NONE, COM_PTR_UUIDOF_PUTVOID(Fence)));

	LOG_OK();
}

//!< �R�}���h���s(CL->ExecuteCommandList())��AGPU���R�}���h�A���P�[�^�̎Q�Ƃ��I����܂ŁA�A���P�[�^�̃��Z�b�g(CA->Reset())���Ă͂����Ȃ��A�A���P�[�^���o���Ă���̂ŃR�}���h�̃��Z�b�g(CL->Reset())�͂��Ă��ǂ�
void DX::CreateCommandAllocator()
{
	CommandAllocators.resize(1);

	CreateCommandAllocator(CommandAllocators[0], D3D12_COMMAND_LIST_TYPE_DIRECT);

	LOG_OK();
}

void DX::CreateCommandList(COM_PTR<ID3D12GraphicsCommandList>& CL, ID3D12CommandAllocator* CA, const D3D12_COMMAND_LIST_TYPE CLT)
{
	//!< �`��R�}���h�𔭍s����R�}���h���X�g�ɂ̓p�C�v���C���X�e�[�g�̎w�肪�K�v�����A�ォ��w��(CL->Reset(CA, PS.get()))���ł��� (�����ł�nullptr�ō쐬���邱�Ƃɂ���)
	VERIFY_SUCCEEDED(Device->CreateCommandList(0, CLT, CA, nullptr, COM_PTR_UUIDOF_PUTVOID(CL)));
	VERIFY_SUCCEEDED(CL->Close());
}
void DX::CreateCommandList()
{
	DXGI_SWAP_CHAIN_DESC1 SCD;
	SwapChain->GetDesc1(&SCD);

	GraphicsCommandLists.resize(SCD.BufferCount);
	for (UINT i = 0; i < SCD.BufferCount; ++i) {
		CreateCommandList(GraphicsCommandLists[i], COM_PTR_GET(CommandAllocators[0]), D3D12_COMMAND_LIST_TYPE_DIRECT);
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

	DXGI_SWAP_CHAIN_DESC SCD = {
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
	COM_PTR_RESET(SwapChain);
	COM_PTR<IDXGISwapChain> NewSwapChain;
	VERIFY_SUCCEEDED(Factory->CreateSwapChain(COM_PTR_GET(CommandQueue), &SCD, COM_PTR_PUT(NewSwapChain)));
	COM_PTR_AS(NewSwapChain, SwapChain);

	//!< �N�����Ƀt���X�N���[���ɂ���ꍇ
	//DXGI_SWAP_CHAIN_DESC1 SCD = {
	//	Width, Height,
	//	ColorFormat,
	//	FALSE,
	//	SampleDesc,
	//	DXGI_USAGE_RENDER_TARGET_OUTPUT,
	//	BufferCount,
	//	DXGI_SCALING_NONE,
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

	//!< DXGI �ɂ��t���X�N���[����(Alt + Enter)��}������ꍇ
	//Factory->MakeWindowAssociation(hWnd, DXGI_MWA_NO_ALT_ENTER);

	//!< �C�ӂ̃^�C�~���O�Ńt���X�N���[��������ꍇ�A�ȉ����R�[������
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
			0 // NodeMask ... �}���`GPU�̏ꍇ
	};
	VERIFY_SUCCEEDED(Device->CreateDescriptorHeap(&DHD, COM_PTR_UUIDOF_PUTVOID(SwapChainDescriptorHeap)));

	LOG_OK();
}
void DX::CreateSwapChainResource()
{
	DXGI_SWAP_CHAIN_DESC1 SwapChainDesc;
	SwapChain->GetDesc1(&SwapChainDesc);

	SwapChainResources.resize(SwapChainDesc.BufferCount);
	for (auto i = 0; i < SwapChainResources.size(); ++i) {
		//!< �X���b�v�`�F�C���̃o�b�t�@���\�[�X�� SwapChainResources �֎擾
		VERIFY_SUCCEEDED(SwapChain->GetBuffer(i, COM_PTR_UUIDOF_PUTVOID(SwapChainResources[i])));
		//!< �f�X�N���v�^(�r���[)�̍쐬�B���\�[�X��ł̃I�t�Z�b�g���w�肵�č쐬���Ă���A���ʂ��ϐ��ɕԂ�킯�ł͂Ȃ�
		//!< (���\�[�X���^�C�v�h�t�H�[�}�b�g�Ȃ� D3D12_RENDER_TARGET_VIEW_DESC* �� nullptr �w��\)
		const auto SCR = COM_PTR_GET(SwapChainResources[i]);
		const auto CDH = GetCPUDescriptorHandle(COM_PTR_GET(SwapChainDescriptorHeap), D3D12_DESCRIPTOR_HEAP_TYPE_RTV, i);
		Device->CreateRenderTargetView(SCR, nullptr, CDH);
	}

	LOG_OK();
}

void DX::InitializeSwapchainImage(ID3D12CommandAllocator* CommandAllocator, const DirectX::XMVECTORF32* Color)
{
	for (auto i = 0; i < SwapChainResources.size(); ++i) {
		const auto CL = COM_PTR_GET(GraphicsCommandLists[i]);
		VERIFY_SUCCEEDED(CL->Reset(CommandAllocator, nullptr));
		{
			const auto SCR = COM_PTR_GET(SwapChainResources[i]);
			const auto CDH = GetCPUDescriptorHandle(COM_PTR_GET(SwapChainDescriptorHeap), D3D12_DESCRIPTOR_HEAP_TYPE_RTV, i);
			ResourceBarrier(CL, SCR, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET); {
				if (nullptr != Color) {
					CL->ClearRenderTargetView(CDH, *Color, 0, nullptr);
				}
			} ResourceBarrier(CL, SCR, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
		}
		VERIFY_SUCCEEDED(CL->Close());
	}

	//!< #DX_TODO : 0 �Ԗڂ����N���A���Ă��Ȃ�
	const std::vector<ID3D12CommandList*> CommandLists = { COM_PTR_GET(GraphicsCommandLists[0]) };
	CommandQueue->ExecuteCommandLists(static_cast<UINT>(CommandLists.size()), CommandLists.data());

	WaitForFence();

	LOG_OK();
}

void DX::InitializeSwapChain()
{
	//!< �C���[�W�̏����� (Initialize images)
	InitializeSwapchainImage(COM_PTR_GET(CommandAllocators[0]), &DirectX::Colors::Red); 
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

void DX::CreateRenderTarget(const DXGI_FORMAT Format, const UINT Width, const UINT Height)
{
	const D3D12_HEAP_PROPERTIES HP = {
		D3D12_HEAP_TYPE_DEFAULT, 
		D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
		D3D12_MEMORY_POOL_UNKNOWN,
		1,
		1
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
	D3D12_CLEAR_VALUE CV = {
		Format,
		{
			{ DirectX::Colors::SkyBlue[0], DirectX::Colors::SkyBlue[1], DirectX::Colors::SkyBlue[2], DirectX::Colors::SkyBlue[3] }
		},
	};
	VERIFY_SUCCEEDED(Device->CreateCommittedResource(&HP, D3D12_HEAP_FLAG_NONE, &RD, D3D12_RESOURCE_STATE_COMMON, &CV, COM_PTR_UUIDOF_PUTVOID(RenderTargetResource)));

	//!< �����_�[�^�[�Q�b�g�r���[
	{
		const D3D12_DESCRIPTOR_HEAP_DESC DHD = {
			D3D12_DESCRIPTOR_HEAP_TYPE_RTV,
			1,
			D3D12_DESCRIPTOR_HEAP_FLAG_NONE,
			0
		};
		VERIFY_SUCCEEDED(Device->CreateDescriptorHeap(&DHD, COM_PTR_UUIDOF_PUTVOID(RenderTargetDescriptorHeap)));
		Device->CreateRenderTargetView(COM_PTR_GET(RenderTargetResource), nullptr, GetCPUDescriptorHandle(COM_PTR_GET(RenderTargetDescriptorHeap), D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 0));
	}

	//!< �V�F�[�_���\�[�X�r���[
	{
		const D3D12_DESCRIPTOR_HEAP_DESC DHD = {
			D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
			1,
			D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE,
			0
		};
		VERIFY_SUCCEEDED(Device->CreateDescriptorHeap(&DHD, COM_PTR_UUIDOF_PUTVOID(ShaderResourceDescriptorHeap)));

		D3D12_SHADER_RESOURCE_VIEW_DESC SRVD = {
			Format,
			D3D12_SRV_DIMENSION_TEXTURE2D,
			D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING,
		};
		SRVD.Texture2D.MostDetailedMip = 0;
		SRVD.Texture2D.MipLevels = 1;
		SRVD.Texture2D.PlaneSlice = 0;
		SRVD.Texture2D.ResourceMinLODClamp = 0.0f;
		Device->CreateShaderResourceView(COM_PTR_GET(RenderTargetResource), &SRVD, GetCPUDescriptorHandle(COM_PTR_GET(ShaderResourceDescriptorHeap), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 0));
	}

	LOG_OK();
}

void DX::CreateDepthStencil(const DXGI_FORMAT DepthFormat, const UINT Width, const UINT Height)
{
	const D3D12_DESCRIPTOR_HEAP_DESC DHD = {
		D3D12_DESCRIPTOR_HEAP_TYPE_DSV,
		1,
		D3D12_DESCRIPTOR_HEAP_FLAG_NONE,
		0
	};
	VERIFY_SUCCEEDED(Device->CreateDescriptorHeap(&DHD, COM_PTR_UUIDOF_PUTVOID(DepthStencilDescriptorHeap)));

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
	COM_PTR_RESET(DepthStencilResource);
	VERIFY_SUCCEEDED(Device->CreateCommittedResource(&HeapProperties, D3D12_HEAP_FLAG_NONE, &ResourceDesc, D3D12_RESOURCE_STATE_COMMON/*COMMON �ɂ��邱��*/, &ClearValue, COM_PTR_UUIDOF_PUTVOID(DepthStencilResource)));
	const auto CDH = GetCPUDescriptorHandle(COM_PTR_GET(DepthStencilDescriptorHeap), D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 0);
	//!< �f�X�N���v�^(�r���[)�̍쐬�B���\�[�X��ł̃I�t�Z�b�g���w�肵�č쐬���Ă���A���ʂ��ϐ��ɕԂ�킯�ł͂Ȃ�
	//!< (���\�[�X���^�C�v�h�t�H�[�}�b�g�Ȃ� D3D12_DEPTH_STENCIL_VIEW_DESC* �� nullptr �w��\)
	Device->CreateDepthStencilView(COM_PTR_GET(DepthStencilResource), nullptr, CDH); 

	//!< ���\�[�X�̏�Ԃ����� �� �f�v�X�������݂֕ύX
	auto CL = GraphicsCommandLists[0];
	ResourceBarrier(COM_PTR_GET(CL), COM_PTR_GET(DepthStencilResource), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_DEPTH_WRITE);

	LOG_OK();
}
void DX::ResizeDepthStencil(const DXGI_FORMAT DepthFormat, const UINT Width, const UINT Height)
{
	COM_PTR_RESET(DepthStencilResource);

	CreateDepthStencilResource(DepthFormat, Width, Height);

	LOG_OK();
}

void DX::CreateBuffer(ID3D12Resource** Res, const UINT32 Size, const void* Source, ID3D12CommandAllocator* CA, ID3D12GraphicsCommandList* CL)
{
	COM_PTR<ID3D12Resource> UploadRes;
	CreateUploadResource(COM_PTR_PUT(UploadRes), Size);
	CopyToUploadResource(COM_PTR_GET(UploadRes), Size, Source);

	//!< �f�t�H���g�̃��\�[�X���쐬 Create default resource
	CreateDefaultResource(Res, Size);

	//!< �A�b�v���[�h���\�[�X����f�t�H���g���\�[�X�ւ̃R�s�[�R�}���h�𔭍s Execute copy command upload resource to default resource
	ExecuteCopyBuffer(CA, CL, COM_PTR_GET(UploadRes), *Res, Size);
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
	VERIFY_SUCCEEDED(Device->CreateCommittedResource(&HeapProperties, D3D12_HEAP_FLAG_NONE, &ResourceDesc, D3D12_RESOURCE_STATE_COMMON, nullptr, COM_PTR_UUIDOF_PUTVOID(UnorderedAccessTextureResource)));
//
	const auto Format = DXGI_FORMAT_R8G8B8A8_UNORM;

	const D3D12_DESCRIPTOR_HEAP_DESC DescritporHeapDesc = {
		D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
		2, //!< SRV, UAV �� 2 ��
		D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE,
		0 // NodeMask ... �}���`GPU�̏ꍇ
	};
	VERIFY_SUCCEEDED(Device->CreateDescriptorHeap(&DescritporHeapDesc, COM_PTR_UUIDOF_PUTVOID(UnorderedAccessTextureDescriptorHeap)));

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
		const auto CDH = GetCPUDescriptorHandle(COM_PTR_GET(UnorderedAccessTextureDescriptorHeap), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, Index++);
		Device->CreateShaderResourceView(COM_PTR_GET(UnorderedAccessTextureResource), &SRVDesc, CDH); 
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
		const auto CDH = GetCPUDescriptorHandle(COM_PTR_GET(UnorderedAccessTextureDescriptorHeap), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, Index++);
		Device->CreateUnorderedAccessView(COM_PTR_GET(UnorderedAccessTextureResource), nullptr, &UAVDesc, CDH);
	}

	LOG_OK();
}

//!< ���[�g�V�O�l�`�����V���A���C�Y���ău���u�����
void DX::SerializeRootSignature(COM_PTR<ID3DBlob>& Blob, const std::initializer_list<D3D12_ROOT_PARAMETER> il_RPs, const std::initializer_list<D3D12_STATIC_SAMPLER_DESC> il_SSDs, const D3D12_ROOT_SIGNATURE_FLAGS /*Flags*/)
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

	COM_PTR<ID3DBlob> ErrorBlob;
	VERIFY_SUCCEEDED(D3D12SerializeRootSignature(&RSD, D3D_ROOT_SIGNATURE_VERSION_1, COM_PTR_PUT(Blob), COM_PTR_PUT(ErrorBlob)));

	LOG_OK();
}

//!< �V�F�[�_���烋�[�g�V�O�l�`���p�[�g�����o���u���u�����
void DX::GetRootSignaturePartFromShader(COM_PTR<ID3DBlob>& Blob, LPCWSTR Path)
{
	COM_PTR<ID3DBlob> ShaderBlob;
	VERIFY_SUCCEEDED(D3DReadFileToBlob(Path, COM_PTR_PUT(ShaderBlob)));
	VERIFY_SUCCEEDED(D3DGetBlobPart(ShaderBlob->GetBufferPointer(), ShaderBlob->GetBufferSize(), D3D_BLOB_ROOT_SIGNATURE, 0, COM_PTR_PUT(Blob)));
	LOG_OK();
}

/**
@brief �V�F�[�_�Ƃ̃o�C���f�B���O (VK::CreateDescriptorSetLayout() ����)
*/
void DX::CreateRootSignature()
{
	COM_PTR<ID3DBlob> Blob;

#ifdef USE_HLSL_ROOTSIGNATRUE
	GetRootSignaturePartFromShader(Blob, (GetBasePath() + TEXT(".rs.cso")).data());
#else
	SerializeRootSignature(Blob, {}, {}, D3D12_ROOT_SIGNATURE_FLAG_NONE);
#endif

	CreateRootSignature(RootSignature, Blob);

	LOG_OK();
}

void DX::CreateShader(std::vector<COM_PTR<ID3DBlob>>& Blobs) const
{
	for (auto i : Blobs) {
		//!< PDB�p�[�g�A�����ꍇ������̂� HRESULT �� VERIFY ���Ȃ�
		COM_PTR<ID3DBlob> PDBPart;
		const auto HR = D3DGetBlobPart(i->GetBufferPointer(), i->GetBufferSize(), D3D_BLOB_PDB, 0, COM_PTR_PUT(PDBPart));

#if 0
		//!< �C�ӂ�(�u�f�o�b�O���v)�f�[�^
		const char DebugName[] = "DebugName";

		//!< 4�o�C�g�A���C�����ꂽ�X�g���[�W
		const auto Size = RoundUp(_countof(DebugName), 0x3);
		auto Data = new BYTE [Size];
		memcpy(Data, DebugName, _countof(DebugName));

		//!< �u�f�o�b�O���v�̕t�����u���u
		COM_PTR<ID3DBlob> WithDebugNamePart;
		if (SUCCEEDED(D3DSetBlobPart(i->GetBufferPointer(), i->GetBufferSize(), D3D_BLOB_DEBUG_NAME, 0, Data, Size, COM_PTR_PUT(WithDebugNamePart)))) {
			//!<�u�f�o�b�O���v�p�[�g���擾
			COM_PTR<ID3DBlob> DebugNamePart;
			if (SUCCEEDED(D3DGetBlobPart(WithDebugNamePart->GetBufferPointer(), WithDebugNamePart->GetBufferSize(), D3D_BLOB_DEBUG_NAME, 0, COM_PTR_PUT(DebugNamePart)))) {
				std::cout << reinterpret_cast<const char*>(DebugNamePart->GetBufferPointer()) << std::endl;
			}
		}

		delete[] Data;
#endif
	}

	//!< �f�o�b�O���A���[�g�V�O�l�`������菜��
#ifndef _DEBUG
	for (auto i : Blobs) {
		if (nullptr != i) {
			VERIFY_SUCCEEDED(D3DStripShader(i->GetBufferPointer(), i->GetBufferSize(), D3DCOMPILER_STRIP_DEBUG_INFO, COM_PTR_PUT(i)));
			VERIFY_SUCCEEDED(D3DStripShader(i->GetBufferPointer(), i->GetBufferSize(), D3DCOMPILER_STRIP_ROOT_SIGNATURE, COM_PTR_PUT(i)));
		}
	}
#endif
}

void DX::CreatePipelineState_Default(COM_PTR<ID3D12PipelineState>& PST, ID3D12RootSignature* RS, 
	const D3D12_SHADER_BYTECODE VS, const D3D12_SHADER_BYTECODE PS, const D3D12_SHADER_BYTECODE DS, const D3D12_SHADER_BYTECODE HS, const D3D12_SHADER_BYTECODE GS,
	ID3D12PipelineLibrary* PL, LPCWSTR Name, bool IsLoad)
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
	COM_PTR<ID3DBlob> PipelineBlob;
	VERIFY_SUCCEEDED(BasePipelineState->GetCachedBlob(COM_PTR_PUT(PipelineBlob)));
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

	if (IsLoad) {
		if (nullptr != PL && nullptr != Name) {
			VERIFY_SUCCEEDED(PL->LoadGraphicsPipeline(Name, &GPSD, COM_PTR_UUIDOF_PUTVOID(PST)));
		}
	}
	else {
		VERIFY_SUCCEEDED(Device->CreateGraphicsPipelineState(&GPSD, COM_PTR_UUIDOF_PUTVOID(PST)));
		if (nullptr != PL && nullptr != Name) {
			VERIFY_SUCCEEDED(PL->StorePipeline(Name, COM_PTR_GET(PST)));
		}
	}

	LOG_OK();
}
#if 0
void DX::CreatePipelineState_Compute()
{
	PERFORMANCE_COUNTER();

	assert(nullptr != RootSignature && "");

	//!< �V�F�[�_
	std::vector<COM_PTR<ID3DBlob>> ShaderBlobs;
	CreateShader(ShaderBlobs);
	assert(!ShaderBlobs.empty() && "");

	const D3D12_CACHED_PIPELINE_STATE CPS = { nullptr, 0 };
	const D3D12_COMPUTE_PIPELINE_STATE_DESC CPSD = {
		COM_PTR_GET(RootSignature),
		D3D12_SHADER_BYTECODE({ ShaderBlobs[0]->GetBufferPointer(), ShaderBlobs[0]->GetBufferSize() }),
		0, // NodeMask ... �}���`GPU�̏ꍇ
		CPS,
		D3D12_PIPELINE_STATE_FLAG_NONE
	};
	VERIFY_SUCCEEDED(Device->CreateComputePipelineState(&CPSD, COM_PTR_UUIDOF_PUTVOID(PipelineState)));

	LOG_OK();
}
#endif


void DX::PopulateCommandList(const size_t i)
{
	const auto CL = COM_PTR_GET(GraphicsCommandLists[i]);
	const auto CA = COM_PTR_GET(CommandAllocators[0]);
	const auto SCR = COM_PTR_GET(SwapChainResources[i]);
	const auto SCH = GetCPUDescriptorHandle(COM_PTR_GET(SwapChainDescriptorHeap), D3D12_DESCRIPTOR_HEAP_TYPE_RTV, static_cast<UINT>(i));
	const auto PS = COM_PTR_GET(PipelineStates[0]);

	//!< GPU ���Q�Ƃ��Ă���Ԃ́A�R�}���h�A���P�[�^�� Reset() �͂ł��Ȃ�
	//VERIFY_SUCCEEDED(CA->Reset());

	//!< CommandQueue->ExecuteCommandLists() ��� CommandList->Reset() �Ń��Z�b�g���čė��p���\ (�R�}���h�L���[�̓R�}���h���X�g�ł͂Ȃ��A�R�}���h�A���P�[�^���Q�Ƃ��Ă���)
	//!< CommandList �쐬���� PipelineState ���w�肵�Ă��Ȃ��Ă��A�����Ŏw�肷��� OK
	VERIFY_SUCCEEDED(CL->Reset(CA, PS));
	{
		//!< �r���[�|�[�g�A�V�U�[
		CL->RSSetViewports(static_cast<UINT>(Viewports.size()), Viewports.data());
		CL->RSSetScissorRects(static_cast<UINT>(ScissorRects.size()), ScissorRects.data());

		//!< �o���A
		ResourceBarrier(CL, SCR, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET); {
			
		const std::array<D3D12_RECT, 0> Rs = {};
		CL->ClearRenderTargetView(SCH, DirectX::Colors::SkyBlue, static_cast<UINT>(Rs.size()), Rs.data());
		//CL->ClearDepthStencilView(DSH, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, static_cast<UINT>(Rs.size()), Rs.data());

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

	const std::vector<ID3D12CommandList*> CLs = { COM_PTR_GET(GraphicsCommandLists[CurrentBackBufferIndex]) };
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
	VERIFY_SUCCEEDED(CommandQueue->Signal(COM_PTR_GET(Fence), FenceValue));
	if (Fence->GetCompletedValue() < FenceValue) {
		auto hEvent = CreateEventEx(nullptr, nullptr, 0, EVENT_ALL_ACCESS);
		if (nullptr != hEvent) {
			//!< GetCompletedValue() �� FenceValue �ɂȂ�����C�x���g�����s�����
			VERIFY_SUCCEEDED(Fence->SetEventOnCompletion(FenceValue, hEvent));

			//!< �C�x���g���s�܂ő҂�
			WaitForSingleObject(hEvent, INFINITE);
			CloseHandle(hEvent);
		}
	}
}
