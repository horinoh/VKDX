#include "DX.h"

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "dxgi.lib")

#ifdef USE_DXC
//!< DXC (dxcompiler.dll �������Ɠ{����ꍇ�� C:\Program Files (x86)\Windows Kits\10\bin\10.0.18362.0\x64 �Ƃ��ɑ��݂���̂ŁA���ϐ� Path �ɒʂ��Ă����K�v������)
#pragma comment(lib, "dxcompiler")
#endif

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
	CreateCommandQueue();

	CreateFence();

	CreateSwapchain(hWnd, ColorFormat);

	CreateCommandList();

	//!< �W�I���g�� (�o�[�e�b�N�X�o�b�t�@�A�C���f�b�N�X�o�b�t�@�A�A�N�Z�����[�V�����X�g���N�`����)
	CreateGeometry();

	//!< �R���X�^���g�o�b�t�@ (���j�t�H�[���o�b�t�@����)
	CreateConstantBuffer();

	//!< �e�N�X�`��
	CreateTexture();

	//!< �X�^�e�B�b�N�T���v���͂��̎��_(CreateRootSignature()���O)�ŕK�v
	CreateStaticSampler();
	//!< ���[�g�V�O�l�`�� (�p�C�v���C���g���C�A�E�g����)
	CreateRootSignature();

	//!< �����_�[�p�X�����͑��݂��Ȃ�

	//!< �p�C�v���C��
	CreatePipelineState();

	//!< �t���[���o�b�t�@�����͑��݂��Ȃ�

	//!< �f�X�N���v�^
	CreateDescriptor();
	{
		//!< �f�X�N���v�^�q�[�v
		CreateDescriptorHeap();
		//!< �f�X�N���v�^�r���[ ... ���̎��_�Ń��\�[�X�A�f�X�N���v�^�q�[�v�����K�v
		CreateDescriptorView();
	}
	CreateShaderTable();

	SetTimer(hWnd, NULL, Elapse, nullptr);

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
	WaitForFence(COM_PTR_GET(GraphicsCommandQueue), COM_PTR_GET(Fence));

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

	//!< �r���[�|�[�g�T�C�Y�����肵�Ă���
	LoadScene();
	for (auto i = 0; i < size(GraphicsCommandLists); ++i) {
		PopulateCommandList(i);
	}
}
void DX::OnPreDestroy(HWND hWnd, HINSTANCE hInstance)
{
	Super::OnPreDestroy(hWnd, hInstance);

	//!< �t���X�N���[���̏ꍇ�͉���
	DXGI_SWAP_CHAIN_DESC1 SCD1;
	SwapChain->GetDesc1(&SCD1);
	if (SCD1.Flags & DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING) {
		VERIFY_SUCCEEDED(SwapChain->SetFullscreenState(FALSE, nullptr));
	}

	//!< GPU����������܂ł����őҋ@ (Wait GPU)
	WaitForFence(COM_PTR_GET(GraphicsCommandQueue), COM_PTR_GET(Fence));
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
		.Width = Size, .Height = 1, //!< Width�Ɂu�o�b�t�@�T�C�Y�v���w�肵�āAHeight��1�ɂ��Ă��� (Set buffer size to Width, set Height to 1)
		.DepthOrArraySize = 1, .MipLevels = 1,
		.Format = DXGI_FORMAT_UNKNOWN,
		.SampleDesc = DXGI_SAMPLE_DESC({.Count = 1, .Quality = 0 }),
		.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR, //!< Width�Ɂu�o�b�t�@�T�C�Y�v���w�肵�Ă���̂� ROW_MAJOR
		.Flags = RF
	};
	const D3D12_HEAP_PROPERTIES HP = {
		.Type = HT,
		.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
		.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN,
		.CreationNodeMask = 0, .VisibleNodeMask = 0 //!< �}���`GPU�̏ꍇ�Ɏg�p(1�����g��Ȃ��ꍇ��0�ŗǂ�)
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
		for (auto i = 0; i < size(PSFs); ++i) {
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
	assert(!(RD.Flags & (D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET | D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL)) && "�� RENDER_TARGET, DEPTH_STENCIL �̏ꍇ�ApOptimizedClearValue ���g�p���Ȃ�");
	VERIFY_SUCCEEDED(Device->CreateCommittedResource(&HP, D3D12_HEAP_FLAG_NONE, &RD, RS, nullptr, IID_PPV_ARGS(Resource)));
}
//!< �����_�[�e�N�X�`���̏ꍇ�� D3D12_CLEAR_VALUE ���w�肷�镔�����ʏ�e�N�X�`���ƈقȂ�
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
	assert(RD.Flags & (D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET | D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL) && "RENDER_TARGET, DEPTH_STENCIL �̏ꍇ�ApOptimizedClearValue ���g�p����");
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
void DX::PopulateCommandList_CopyTextureRegion(ID3D12GraphicsCommandList* GCL, ID3D12Resource* Src, ID3D12Resource* Dst, const std::vector<D3D12_PLACED_SUBRESOURCE_FOOTPRINT>& PSFs, const D3D12_RESOURCE_STATES RS)
{
	//!< LoadDDSTextureFromFile() ���g�p����� D3D12_RESOURCE_STATE_COPY_DEST �ō쐬����Ă���̂Ńo���A�̕K�v�͖��� (Resource created by LoadDDSTextureFromFile()'s state is already D3D12_RESOURCE_STATE_COPY_DEST)
	
	for (UINT i = 0; i < size(PSFs); ++i) {
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
}

void DX::CreateDevice([[maybe_unused]] HWND hWnd)
{
#if defined(_DEBUG) || defined(USE_PIX)
	if (SUCCEEDED(DXGIGetDebugInterface1(0, COM_PTR_UUIDOF_PUTVOID(GraphicsAnalysis)))) {
		//!< �O���t�B�b�N�X�f�f�� Alt + F5 �ŋN�������ꍇ�̂ݐ������� (Enabled only if executed with Alt + F5)
		Log("Graphics Analysis is enabled\n");
		//!< �v���O��������L���v�`������ꍇ�� GraphicsAnalysis->BeginCapture(), GraphicsAnalysis->EndCapture() ���g�p����
	}
#endif

#ifdef _DEBUG
	COM_PTR<ID3D12Debug> Debug;
	VERIFY_SUCCEEDED(D3D12GetDebugInterface(COM_PTR_UUIDOF_PUTVOID(Debug)));
	Debug->EnableDebugLayer();

	//!< GPU�o���f�[�V���� ... CPU�o���f�[�V�����������Ȃ��Ȃ�̂ō̗p���Ȃ��c
	//COM_PTR<ID3D12Debug1> Debug1;
	//VERIFY_SUCCEEDED(Debug->QueryInterface(COM_PTR_UUIDOF_PUTVOID(Debug1)));
	//Debug1->SetEnableGPUBasedValidation(true);

	//!< CreateDXGIFactory2()�ł͈����Ƀt���O����邱�Ƃ��ł���̂� DXGI_CREATE_FACTORY_DEBUG �t���O���w�� (CreateDXGIFactory2() can specify flag argument, here use DXGI_CREATE_FACTORY_DEBUG)
	VERIFY_SUCCEEDED(CreateDXGIFactory2(DXGI_CREATE_FACTORY_DEBUG, COM_PTR_UUIDOF_PUTVOID(Factory)));
#else
	VERIFY_SUCCEEDED(CreateDXGIFactory1(COM_PTR_UUIDOF_PUTVOID(Factory)));
#endif

#ifdef DEBUG_STDOUT
	//!< �A�_�v�^�[(GPU)�̗� (Enumerate adapter(GPU))
	std::cout << COM_PTR_GET(Factory);
#endif

#ifdef USE_WARP
	//!< �\�t�g�E�G�A���X�^���C�U (Software rasterizer)
	VERIFY_SUCCEEDED(Factory->EnumWarpAdapter(COM_PTR_UUIDOF_PUTVOID(Adapter)));
#else
	std::vector<DXGI_ADAPTER_DESC> ADs;
	for (UINT i = 0; DXGI_ERROR_NOT_FOUND != Factory->EnumAdapters(i, COM_PTR_PUT(Adapter)); ++i) {
		VERIFY_SUCCEEDED(Adapter->GetDesc(&ADs.emplace_back()));
		COM_PTR_RESET(Adapter);
	}
	//!< �A�_�v�^�[(GPU)�̑I���A�����ł͍ő僁������I�����邱�Ƃɂ��� (Here, select max memory size adapter(GPU))
	const auto Index = static_cast<UINT>(std::distance(begin(ADs), std::ranges::max_element(ADs, [](const DXGI_ADAPTER_DESC& lhs, const DXGI_ADAPTER_DESC& rhs) { return lhs.DedicatedSystemMemory > rhs.DedicatedSystemMemory; })));
	VERIFY_SUCCEEDED(Factory->EnumAdapters(Index, COM_PTR_PUT(Adapter)));
#endif
	assert(nullptr != Adapter && "");
	Log("[ Selected Adapter ]\n");
#ifdef DEBUG_STDOUT
	std::cout << COM_PTR_GET(Adapter);
#endif

	//!< �����ł͍ŏ��Ɍ��������A�E�g�v�b�g(Display)��I�����邱�Ƃɂ��� (Here, select first found output(Display))
	{
		COM_PTR<IDXGIAdapter> DA;
		for (UINT i = 0; DXGI_ERROR_NOT_FOUND != Factory->EnumAdapters(i, COM_PTR_PUT(DA)); ++i) {
			for (UINT j = 0; DXGI_ERROR_NOT_FOUND != DA->EnumOutputs(j, COM_PTR_PUT(Output)); ++j) {
				if (nullptr != Output) { break; }
				COM_PTR_RESET(Output);
			}
			COM_PTR_RESET(DA);
			if (nullptr != Output) { break; }
		}
		assert(nullptr != Output && "");
	}
	Log("\t\t[ Selected Output ]\n");
#ifdef DEBUG_STDOUT
	std::cout << COM_PTR_GET(Output);
#endif

	//!< (�t�H�[�}�b�g�w���)�I�������A�E�g�v�b�g�̃f�B�X�v���C���[�h���
	GetDisplayModeList(COM_PTR_GET(Output), DXGI_FORMAT_R8G8B8A8_UNORM);

#if 1
	//!< �����I�@�\��L��������
	const std::array Experimental = { D3D12ExperimentalShaderModels, /*D3D12RaytracingPrototype*/ };
	if (FAILED(D3D12EnableExperimentalFeatures(static_cast<UINT>(size(Experimental)), data(Experimental), nullptr, nullptr))) {
		Warning("\tD3D12EnableExperimentalFeatures() failed\n");
	}
#endif

	//!< ���t�B�[�`���[���x���D��Ńf�o�C�X���쐬 (Create device with higher feature level possible)
	{
		constexpr std::array FeatureLevels = {
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
				//!< NumFeatureLevels, pFeatureLevelsRequested �͓��́AMaxSupportedFeatureLevel �͏o�͂ƂȂ� (NumFeatureLevels, pFeatureLevelsRequested is input, MaxSupportedFeatureLevel is output)
				D3D12_FEATURE_DATA_FEATURE_LEVELS FDFL = { .NumFeatureLevels = static_cast<UINT>(size(FeatureLevels)), .pFeatureLevelsRequested = data(FeatureLevels) };
				VERIFY_SUCCEEDED(Device->CheckFeatureSupport(D3D12_FEATURE_FEATURE_LEVELS, reinterpret_cast<void*>(&FDFL), sizeof(FDFL)));
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
				break;
			}
		}
	}

	//!< �f�o�C�X�쐬��
#ifdef _DEBUG
	{
		COM_PTR<ID3D12InfoQueue> IQ;
		VERIFY_SUCCEEDED(Device->QueryInterface(COM_PTR_UUIDOF_PUTVOID(IQ)));
		if (nullptr != IQ) {
			//!< �G���[���Ńu���[�N����ݒ� (Break with error)
			VERIFY_SUCCEEDED(IQ->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, TRUE));
			VERIFY_SUCCEEDED(IQ->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, TRUE));

			//!< ���[�j���O�����t�B���^�����O�������ꍇ���Ɏg�p
			//!< �K�؂ɐݒ肵�Ȃ��Əo�͂����邳���̂ł����ł͖����ɂ��Ă���
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
		VERIFY_SUCCEEDED(Outp->GetDisplayModeList(Format, 0, &Count, data(MDs)));
		for (const auto& i : MDs) {
			std::cout << i;
		}
	}
#endif
}

/**
@brief �}���`�X���b�h�Łu�����v�L���[�փT�u�~�b�g�ł���
@note Vulkan �ł̓}���`�X���b�h�Łu�قȂ�v�L���[�ւ̂݃T�u�~�b�g�ł���̂Œ���
*/
void DX::CreateCommandQueue()
{
	{
		constexpr D3D12_COMMAND_QUEUE_DESC CQD = {
			.Type = D3D12_COMMAND_LIST_TYPE_DIRECT,
			.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL,
			.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE,
			.NodeMask = 0 //!< �}���`GPU�̏ꍇ�Ɏg�p(1�����g��Ȃ��ꍇ��0�ŗǂ�)
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

//!< CPU �� GPU �̓����p
void DX::CreateFence()
{
	VERIFY_SUCCEEDED(Device->CreateFence(0, D3D12_FENCE_FLAG_NONE, COM_PTR_UUIDOF_PUTVOID(Fence)));
	LOG_OK();
}

void DX::CreateSwapchain(HWND hWnd, const DXGI_FORMAT ColorFormat)
{
	CreateSwapChain(hWnd, ColorFormat, GetClientRectWidth(), GetClientRectHeight());

	//!< ���\�[�X���擾�A�r���[���쐬 (Get resource, create view)
	GetSwapChainResource();
}
void DX::CreateSwapChain(HWND hWnd, const DXGI_FORMAT ColorFormat, const UINT Width, const UINT Height)
{
	const UINT BufferCount = 3;

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
	//!< �N�����Ƀt���X�N���[���ɂ���ꍇ
	const DXGI_SWAP_CHAIN_DESC1 SCD1 = {
		.Width = Width, .Height = Height,
		.Format = ColorFormat,
		.Stereo = FALSE,
		.SampleDesc = SDs[0],
		.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
		.BufferCount = BufferCount,
		.Scaling = DXGI_SCALING_STRETCH/*DXGI_SCALING_NONE*/,
		.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD,
		.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED,
		.Flags = 0/*DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING*/,
	};
	//const DXGI_SWAP_CHAIN_FULLSCREEN_DESC SCFD = {
	//	.RefreshRate = DXGI_RATIONAL({.Numerator = 60, .Denominator = 1 }),
	//	.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED,
	//	.Scaling = DXGI_MODE_SCALING_UNSPECIFIED,
	//	.Windowed = FALSE,
	//};
	COM_PTR_RESET(SwapChain);
	COM_PTR<IDXGISwapChain1> NewSwapChain;
	VERIFY_SUCCEEDED(Factory->CreateSwapChainForHwnd(COM_PTR_GET(GraphicsCommandQueue), hWnd, &SCD1, /*&SCFD*/nullptr, /*COM_PTR_GET(Output)*/nullptr, COM_PTR_PUT(NewSwapChain)));

	//!< DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING ���́AAlt + Enter �ɂ��t���X�N���[���ؑւ���}���ASwapChain->SetFullscreenState() ���g�p����
	if (SCD1.Flags & DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING) {
		Factory->MakeWindowAssociation(hWnd, DXGI_MWA_NO_ALT_ENTER);
	}

	//!< �t���X�N���[���ؑւ�(�g�O��)�̗�
	if(false){
		DXGI_SWAP_CHAIN_DESC1 SCD1;
		SwapChain->GetDesc1(&SCD1);
		if (!(SCD1.Flags & DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING)) {
			BOOL IsFullScreen;
			VERIFY_SUCCEEDED(SwapChain->SetFullscreenState(&IsFullScreen));
			VERIFY_SUCCEEDED(SwapChain->SetFullscreenState(!IsFullScreen, nullptr));
		}
	}
#else
	DXGI_SWAP_CHAIN_DESC SCD1 = {
		//!< �œK�ȃt���X�N���[���̃p�t�H�[�}���X�𓾂�ɂ́AIDXGIOutput->GetDisplayModeList() �Ŏ擾����(�f�B�X�v���C�̃T�|�[�g����)DXGI_MODE_DESC �łȂ��ƃ_���Ȃ̂Œ���  #DX_TODO
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
		.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD,
		.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH //!< �t���X�N���[���ɂ������A�œK�ȃf�B�X�v���C���[�h���I�������̂�����
	};
	//!< �Z�b�e�B���O��ύX���ăX���b�v�`�F�C�����č쐬�ł���悤�ɁA�����̂��J�����Ă���
	COM_PTR_RESET(SwapChain);
	COM_PTR<IDXGISwapChain> NewSwapChain;
	VERIFY_SUCCEEDED(Factory->CreateSwapChain(COM_PTR_GET(GraphicsCommandQueue), &SCD1, COM_PTR_PUT(NewSwapChain)));
	COM_PTR_AS(NewSwapChain, SwapChain);
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

	const D3D12_DESCRIPTOR_HEAP_DESC DHD = {
		.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV,
		.NumDescriptors = SCD1.BufferCount,
		.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE,
		.NodeMask = 0 //!< �}���`GPU�̏ꍇ�Ɏg�p(1�����g��Ȃ��ꍇ��0�ŗǂ�)
	};
	VERIFY_SUCCEEDED(Device->CreateDescriptorHeap(&DHD, COM_PTR_UUIDOF_PUTVOID(SwapChainDescriptorHeap)));

	LOG_OK();
}
void DX::GetSwapChainResource()
{
	DXGI_SWAP_CHAIN_DESC1 SCD;
	SwapChain->GetDesc1(&SCD);

	auto CDH = SwapChainDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	for (UINT i = 0; i < SCD.BufferCount; ++i) {
		//!< �X���b�v�`�F�C���̃o�b�t�@���\�[�X�� SwapChainResources �֎擾
		VERIFY_SUCCEEDED(SwapChain->GetBuffer(i, COM_PTR_UUIDOF_PUTVOID(SwapChainResources.emplace_back())));

		//!< �f�X�N���v�^(�r���[)�̍쐬
#ifdef USE_GAMMA_CORRECTION
		constexpr D3D12_RENDER_TARGET_VIEW_DESC RTVD = {
			.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, //!< �K���}�␳����
			.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D,
			.Texture2D = D3D12_TEX2D_RTV({.MipSlice = 0, .PlaneSlice = 0 })
		};
		Device->CreateRenderTargetView(COM_PTR_GET(SwapChainResources.back()), &RTVD, CDH);
#else
		//!< �^�C�v�h�t�H�[�}�b�g�Ȃ� D3D12_RENDER_TARGET_VIEW_DESC* �� nullptr �w��\
		Device->CreateRenderTargetView(COM_PTR_GET(SwapChainResources.back()), nullptr, CDH);
#endif
		CDH.ptr += Device->GetDescriptorHandleIncrementSize(SwapChainDescriptorHeap->GetDesc().Type);
	}

	LOG_OK();
}

void DX::ResizeSwapChain(const UINT Width, const UINT Height)
{
	for (auto& i : SwapChainResources) { COM_PTR_RESET(i); }
	SwapChainResources.clear();

	DXGI_SWAP_CHAIN_DESC1 SCD;
	SwapChain->GetDesc1(&SCD);
	VERIFY_SUCCEEDED(SwapChain->ResizeBuffers(SCD.BufferCount, Width, Height, SCD.Format, SCD.Flags));
	Log("\tResizeBuffers\n");

	//!< ���\�[�X���擾�A�r���[���쐬 (Get resource, create view)
	GetSwapChainResource();

	LOG_OK();
}

void DX::ResizeDepthStencil([[maybe_unused]] const DXGI_FORMAT DepthFormat, [[maybe_unused]] const UINT Width, [[maybe_unused]] const UINT Height)
{
	//COM_PTR_RESET(DepthStencilResource);
	//CreateDepthStencilResource(DepthFormat, Width, Height);

	LOG_OK();
}

//!< �����ł̓f�t�H���g�����Ƃ��āA�_�C���N�g�A�o���h�����ɃX���b�v�`�F�C�������p�ӂ��邱�ƂƂ���
void DX::CreateCommandList()
{
	//!< �R�}���h���s(GCL->ExecuteCommandList())��AGPU���R�}���h�A���P�[�^�̎Q�Ƃ��I����܂ŁA�A���P�[�^�̃��Z�b�g(CA->Reset())���Ă͂����Ȃ��A�A���P�[�^���o���Ă���̂ŃR�}���h�̃��Z�b�g(GCL->Reset())�͂��Ă��ǂ�
	//!< (�����ł�)�_�C���N�g�p1�A�o���h���p1�̃R�}���h�A���P�[�^�쐬���f�t�H���g�����Ƃ���
	VERIFY_SUCCEEDED(Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, COM_PTR_UUIDOF_PUTVOID(CommandAllocators.emplace_back())));
	VERIFY_SUCCEEDED(Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_BUNDLE, COM_PTR_UUIDOF_PUTVOID(BundleCommandAllocators.emplace_back())));

	DXGI_SWAP_CHAIN_DESC1 SCD;
	SwapChain->GetDesc1(&SCD);
	for (UINT i = 0; i < SCD.BufferCount; ++i) {
		//!< �p�C�v���C���X�e�[�g�͌ォ��ł��w��ł��� GCL->Reset(CA, COM_PTR_GET(PS)) �̂ŁA�����ł� nullptr ���w��
		VERIFY_SUCCEEDED(Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, COM_PTR_GET(CommandAllocators[0]), nullptr, COM_PTR_UUIDOF_PUTVOID(GraphicsCommandLists.emplace_back())));
		VERIFY_SUCCEEDED(GraphicsCommandLists.back()->Close());

		VERIFY_SUCCEEDED(Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_BUNDLE, COM_PTR_GET(BundleCommandAllocators[0]), nullptr, COM_PTR_UUIDOF_PUTVOID(BundleGraphicsCommandLists.emplace_back())));
		VERIFY_SUCCEEDED(BundleGraphicsCommandLists.back()->Close());
	}
}

void DX::CreateViewport(const FLOAT Width, const FLOAT Height, const FLOAT MinDepth, const FLOAT MaxDepth)
{
	//!< DirectX�AOpenGL�́u�����v�����_ (Vulkan �̓f�t�H���g�Łu����v�����_)
	Viewports = {
		D3D12_VIEWPORT({ .TopLeftX = 0.0f, .TopLeftY = 0.0f, .Width = Width, .Height = Height, .MinDepth = MinDepth, .MaxDepth = MaxDepth }),
	};
	//!< left, top, right, bottom�Ŏw�� (offset, extent�Ŏw���VK�Ƃ͈قȂ�̂Œ���)
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
		D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS //!< ALLOW_UNORDERED_ACCESS �ɂ���
	};
	const D3D12_HEAP_PROPERTIES HeapProperties = {
		D3D12_HEAP_TYPE_DEFAULT,
		D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
		D3D12_MEMORY_POOL_UNKNOWN,
		0, 0 //!< �}���`GPU�̏ꍇ�Ɏg�p(1�����g��Ȃ��ꍇ��0�ŗǂ�)
	};
	VERIFY_SUCCEEDED(Device->CreateCommittedResource(&HeapProperties, D3D12_HEAP_FLAG_NONE, &ResourceDesc, D3D12_RESOURCE_STATE_COMMON, nullptr, COM_PTR_UUIDOF_PUTVOID(UnorderedAccessTextureResource)));

	LOG_OK();
}
#endif

//!< D3D_ROOT_SIGNATURE_VERSION_1_0 ���g�p����ꍇ
template<> void DX::SerializeRootSignature(COM_PTR<ID3DBlob>& Blob, const std::vector<D3D12_ROOT_PARAMETER>& RPs, const std::vector<D3D12_STATIC_SAMPLER_DESC>& SSDs, const D3D12_ROOT_SIGNATURE_FLAGS Flags)
{
	D3D12_FEATURE_DATA_ROOT_SIGNATURE FDRS = { .HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0 };
	VERIFY_SUCCEEDED(Device->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, reinterpret_cast<void*>(&FDRS), sizeof(FDRS)));
	assert(FDRS.HighestVersion >= D3D_ROOT_SIGNATURE_VERSION_1_0 && "");

	COM_PTR<ID3DBlob> ErrorBlob;
	const D3D12_ROOT_SIGNATURE_DESC RSD = {
		.NumParameters = static_cast<UINT>(size(RPs)), .pParameters = data(RPs),
		.NumStaticSamplers = static_cast<UINT>(size(SSDs)), .pStaticSamplers = data(SSDs),
		.Flags = Flags
	}; 
#if 1
	//!< �f�X�N���v�^�Ƀo�[�W�������܂߂����
	const D3D12_VERSIONED_ROOT_SIGNATURE_DESC VRSD = { .Version = D3D_ROOT_SIGNATURE_VERSION_1_0, .Desc_1_0 = RSD, };
	VERIFY_SUCCEEDED(D3D12SerializeVersionedRootSignature(&VRSD, COM_PTR_PUT(Blob), COM_PTR_PUT(ErrorBlob)));
#else
	//!< �����Ƀo�[�W�������w�肷�����
	VERIFY_SUCCEEDED(D3D12SerializeRootSignature(&RSD, D3D_ROOT_SIGNATURE_VERSION_1_0, COM_PTR_PUT(Blob), COM_PTR_PUT(ErrorBlob)));
#endif
	LOG_OK();
}

//!< D3D_ROOT_SIGNATURE_VERSION_1_1 ���g�p����ꍇ
//!< [ D3D12_ROOT_SIGNATURE_DESC �� D3D12_ROOT_SIGNATURE_DESC1 �̈Ⴂ ]
//!< D3D12_ROOT_SIGNATURE_DESC1 �ł� D3D12_ROOT_PARAMETER -> D3D12_ROOT_PARAMETER1 �ɕύX����Ă���
//!< D3D12_ROOT_PARAMETER1 �ł� D3D12_ROOT_DESCRIPTOR -> D3D12_ROOT_DESCRIPTOR1 �ɕύX����Ă���
//!< D3D12_ROOT_DESCRIPTOR1 �ł� D3D12_ROOT_DESCRIPTOR_FLAGS Flags �����o�������Ă���
template<> void DX::SerializeRootSignature(COM_PTR<ID3DBlob>& Blob, const std::vector<D3D12_ROOT_PARAMETER1>& RPs, const std::vector<D3D12_STATIC_SAMPLER_DESC>& SSDs, const D3D12_ROOT_SIGNATURE_FLAGS Flags)
{
	D3D12_FEATURE_DATA_ROOT_SIGNATURE FDRS = { .HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1 };
	VERIFY_SUCCEEDED(Device->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, reinterpret_cast<void*>(&FDRS), sizeof(FDRS)));
	assert(FDRS.HighestVersion >= D3D_ROOT_SIGNATURE_VERSION_1_1 && "");
	
	COM_PTR<ID3DBlob> ErrorBlob;
	//!< �f�X�N���v�^�Ƀo�[�W�������܂߂�����̂�
	const D3D12_VERSIONED_ROOT_SIGNATURE_DESC VRSD = {
		.Version = D3D_ROOT_SIGNATURE_VERSION_1_1,
		.Desc_1_1 = D3D12_ROOT_SIGNATURE_DESC1({
			.NumParameters = static_cast<UINT>(size(RPs)), .pParameters = data(RPs),
			.NumStaticSamplers = static_cast<UINT>(size(SSDs)), .pStaticSamplers = data(SSDs),
			.Flags = Flags
		}),
	};
	VERIFY_SUCCEEDED(D3D12SerializeVersionedRootSignature(&VRSD, COM_PTR_PUT(Blob), COM_PTR_PUT(ErrorBlob)));
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
	GetRootSignaturePartFromShader(Blob, data(GetBasePath() + TEXT(".rs.cso")));
#else
	SerializeRootSignature(Blob, {}, {}, D3D12_ROOT_SIGNATURE_FLAG_NONE | SHADER_ROOT_ACCESS_DENY_ALL);
#endif
	VERIFY_SUCCEEDED(Device->CreateRootSignature(0/* �}���`GPU�̏ꍇ�Ɏg�p(1�����g��Ȃ��ꍇ��0�ŗǂ�)*/, Blob->GetBufferPointer(), Blob->GetBufferSize(), COM_PTR_UUIDOF_PUTVOID(RootSignatures.emplace_back())));
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

#ifdef DEBUG_STDOUT
	std::cout << COM_PTR_GET(SR);
#endif
}

void DX::SetBlobPart(COM_PTR<ID3DBlob>& Blob)
{
	//!< D3D_BLOB_DEBUG_NAME
	{
		//!< �t���閼�O
		std::string_view Name = "Debug Name";

		//!< 4 �o�C�g�A���C�����ꂽ�o�b�t�@
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

void DX::CreatePipelineState_(COM_PTR<ID3D12PipelineState>& PST,
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
	PERFORMANCE_COUNTER();

	assert((VS.pShaderBytecode != nullptr && VS.BytecodeLength) && "");

	//!< �L���b�V���h�p�C�v���C���X�e�[�g (CachedPipelineState)
	//!< (VK �� VkGraphicsPipelineCreateInfo.basePipelineHandle, basePipelineIndex ����?)
	COM_PTR<ID3DBlob> CachedBlob;
	if (nullptr != PST) {
		VERIFY_SUCCEEDED(PST->GetCachedBlob(COM_PTR_PUT(CachedBlob)));
	}

	//!< DX�ł́u�p�b�`�R���g���[���|�C���g�v���̎w���IASetPrimitiveTopology()�̈����Ƃ��āu�R�}���h���X�g�쐬���v�Ɏw�肷��AVK�Ƃ͌��\�قȂ�̂Œ���
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
			.AlphaToCoverageEnable = TRUE,		//!< �}���`�T���v�����l�������A���t�@�e�X�g(AlphaToCoverageEnable)�A�A���t�@��0�̉ӏ��ɂ͖��ʂɏ������܂Ȃ�
			.IndependentBlendEnable = FALSE,	//!< �}���`�����_�[�^�[�Q�b�g�ɂ��ꂼ��ʂ̃u�����h�X�e�[�g�����蓖�Ă�(IndependentBlendEnable)
			.RenderTarget = {} 
			}),
		.SampleMask = D3D12_DEFAULT_SAMPLE_MASK,
	 	.RasterizerState = RD,
		.DepthStencilState = DSD,
		.InputLayout = D3D12_INPUT_LAYOUT_DESC({ .pInputElementDescs = data(IEDs), .NumElements = static_cast<UINT>(size(IEDs)) }),
		.IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED,
		.PrimitiveTopologyType = PTT,
		.NumRenderTargets = static_cast<UINT>(size(RTVFormats)), .RTVFormats = {},
		.DSVFormat = DSD.DepthEnable ? DXGI_FORMAT_D24_UNORM_S8_UINT : DXGI_FORMAT_UNKNOWN,
		.SampleDesc = DXGI_SAMPLE_DESC({.Count = 1, .Quality = 0 }),
		.NodeMask = 0, //!< �}���`GPU�̏ꍇ�Ɏg�p(1�����g��Ȃ��ꍇ��0�ŗǂ�)
		.CachedPSO = D3D12_CACHED_PIPELINE_STATE({.pCachedBlob = nullptr != CachedBlob ? CachedBlob->GetBufferPointer() : nullptr, .CachedBlobSizeInBytes = nullptr != CachedBlob ? CachedBlob->GetBufferSize() : 0 }),
#if defined(_DEBUG) && defined(USE_WARP)
		//!< �p�C�v���C�����f�o�b�O�p�t����񂠂�ŃR���p�C�������AWARP���̂ݎg�p�\
		.Flags = D3D12_PIPELINE_STATE_FLAG_TOOL_DEBUG 
#else
		.Flags = D3D12_PIPELINE_STATE_FLAG_NONE
#endif
	};

	//!< �����_�[�^�[�Q�b�g���������K�v�Ȃ���
	assert(size(RTBDs) <= _countof(GPSD.BlendState.RenderTarget) && "");
	std::ranges::copy(RTBDs, GPSD.BlendState.RenderTarget);
	//!< TRUE == IndependentBlendEnable �̏ꍇ�̓����_�[�^�[�Q�b�g�̕������p�ӂ��邱�� (If TRUE == IndependentBlendEnable, need NumRenderTarget elements)
	assert((false == GPSD.BlendState.IndependentBlendEnable || size(RTBDs) == GPSD.NumRenderTargets) && "");
	assert(GPSD.NumRenderTargets <= _countof(GPSD.RTVFormats) && "");
	std::ranges::copy(RTVFormats, GPSD.RTVFormats);

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

void DX::CreatePipelineState__(COM_PTR<ID3D12PipelineState>& PST, 
	ID3D12Device* Device, 
	ID3D12RootSignature* RS,
	const std::vector<D3D12_RENDER_TARGET_BLEND_DESC>& RTBDs, 
	const D3D12_RASTERIZER_DESC& RD,
	const D3D12_DEPTH_STENCIL_DESC1& DSD, 
	const D3D12_SHADER_BYTECODE AS, const D3D12_SHADER_BYTECODE MS, const D3D12_SHADER_BYTECODE PS, 
	const std::vector<DXGI_FORMAT>& RTVFormats, 
	const PipelineLibrarySerializer* PLS, LPCWSTR Name)
{
	PERFORMANCE_COUNTER();

	//!< �L���b�V���h�p�C�v���C���X�e�[�g (CachedPipelineState)
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
		.RTVFormats = D3D12_RT_FORMAT_ARRAY({.RTFormats = {}, .NumRenderTargets = static_cast<UINT>(size(RTVFormats)) }),
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
	assert(size(RTBDs) <= _countof(PMSS.BlendState.Value.RenderTarget) && "");
	std::ranges::copy(RTBDs, PMSS.BlendState.Value.RenderTarget);
	assert((false == PMSS.BlendState.Value.IndependentBlendEnable || size(RTBDs) == PMSS.RTVFormats.Value.NumRenderTargets) && "");
	assert(PMSS.RTVFormats.Value.NumRenderTargets <= _countof(PMSS.RTVFormats.Value.RTFormats) && "");
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
	Textures.emplace_back().Create(COM_PTR_GET(Device), 1, 1, static_cast<UINT16>(size(Colors)), DXGI_FORMAT_R8G8B8A8_UNORM);

	const auto CA = COM_PTR_GET(CommandAllocators[0]);
	const auto GCL = COM_PTR_GET(GraphicsCommandLists[0]);
	{
		constexpr auto PitchSize = 1 * static_cast<UINT32>(sizeof(Colors[0]));
		constexpr auto LayerSize = 1 * PitchSize;
		ResourceBase Upload;
		{
			//!< �A���C�����ꂽ�T�C�Y���v�Z (Calculate aligned size)
			size_t AlignedSize = 0;
			for (UINT32 i = 0; i < size(Colors); ++i) {
				AlignedSize = RoundUp(i * LayerSize, D3D12_TEXTURE_DATA_PLACEMENT_ALIGNMENT);
				AlignedSize += LayerSize;
			}
			//!< Colors ���A���C�����ꂽ�������փR�s�[ (Copy Colors to aligned memory)
			std::vector<std::byte> AlignedData(AlignedSize, std::byte());
			for (UINT32 i = 0; i < size(Colors); ++i) {
				*reinterpret_cast<UINT32*>(&AlignedData[RoundUp(i * LayerSize, D3D12_TEXTURE_DATA_PLACEMENT_ALIGNMENT)]) = Colors[i];
			}
			Upload.Create(COM_PTR_GET(Device), size(AlignedData), D3D12_HEAP_TYPE_UPLOAD, data(AlignedData));
		}

		const auto RD = Textures.back().Resource->GetDesc();
		std::vector<D3D12_PLACED_SUBRESOURCE_FOOTPRINT> PSFs;
		for (UINT32 i = 0; i < size(Colors); ++i) {
			PSFs.emplace_back(D3D12_PLACED_SUBRESOURCE_FOOTPRINT({
				.Offset = RoundUp(i * LayerSize, D3D12_TEXTURE_DATA_PLACEMENT_ALIGNMENT),
				.Footprint = D3D12_SUBRESOURCE_FOOTPRINT({.Format = RD.Format, .Width = static_cast<UINT>(RD.Width), .Height = RD.Height, .Depth = 1, .RowPitch = static_cast<UINT>(RoundUp(PitchSize, D3D12_TEXTURE_DATA_PITCH_ALIGNMENT)) })
			}));
		}
		VERIFY_SUCCEEDED(GCL->Reset(CA, nullptr)); {
			PopulateCommandList_CopyTextureRegion(GCL, COM_PTR_GET(Upload.Resource), COM_PTR_GET(Textures.back().Resource), PSFs, RS);
		} VERIFY_SUCCEEDED(GCL->Close());
		DX::ExecuteAndWait(COM_PTR_GET(GraphicsCommandQueue), GCL, COM_PTR_GET(Fence));
	}
}

void DX::Draw()
{
	WaitForFence(COM_PTR_GET(GraphicsCommandQueue), COM_PTR_GET(Fence));

	DrawFrame(GetCurrentBackBufferIndex());

	Submit();
	
	Present();
}
void DX::Dispatch()
{
	//!< #DX_TODO Dispatch����
	DEBUG_BREAK();
}

void DX::WaitForFence(ID3D12CommandQueue* CQ, ID3D12Fence* Fence)
{
	auto Value = Fence->GetCompletedValue();
	++Value;
	//!< GPU�����B����� Value �ɂȂ�
	VERIFY_SUCCEEDED(CQ->Signal(Fence, Value));
	if (Fence->GetCompletedValue() < Value) {
		auto hEvent = CreateEventEx(nullptr, nullptr, 0, EVENT_ALL_ACCESS);
		if (nullptr != hEvent) [[likely]] {
			//!< GetCompletedValue() �� FenceValue �ɂȂ�����C�x���g�����s�����悤�ɂ���
			VERIFY_SUCCEEDED(Fence->SetEventOnCompletion(Value, hEvent));
			//!< �C�x���g���s�܂ő҂�
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
	//!< ����������҂� : 1
	VERIFY_SUCCEEDED(SwapChain->Present(1, 0));
}

