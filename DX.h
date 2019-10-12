#pragma once

//!< 少なくとも USE_WINRT, USE_WRL のいずれかは定義すること、両方定義された場合は USE_WINRT が優先される (At least define USE_WINRT or USE_WRL, if both defined USE_WINRT will be used)
#define USE_WINRT
#define USE_WRL
#ifdef USE_WINRT
//!< Property - All Configurations, C/C++ - Language - C++ Language Standard - Select ISO C++17 Standard (Default is C++14)
#include <winrt/base.h>
#define COM_PTR winrt::com_ptr
#define COM_PTR_GET(_x) _x.get()
#define COM_PTR_PUT(_x) _x.put()
#define COM_PTR_PUTVOID(_x) _x.put_void()
#define COM_PTR_UUIDOF_PUTVOID(_x) __uuidof(_x), COM_PTR_PUTVOID(_x)
#define COM_PTR_RESET(_x) _x = nullptr
#define COM_PTR_AS(_x, _y) winrt::copy_to_abi(_x, *_y.put_void());
#elif defined(USE_WRL)
#include <wrl.h>
#define COM_PTR Microsoft::WRL::ComPtr
#define COM_PTR_GET(_x) _x.Get()
#define COM_PTR_PUT(_x) _x.GetAddressOf()
#define COM_PTR_PUTVOID(_x) _x.GetAddressOf()
#define COM_PTR_UUIDOF_PUTVOID(_x) IID_PPV_ARGS(COM_PTR_PUTVOID(_x))
#define COM_PTR_RESET(_x) _x.Reset()
#define COM_PTR_AS(_x, _y) VERIFY_SUCCEEDED(_x.As(&_y));
#endif

//#define USE_WARP

#define USE_STATIC_SAMPLER
//!< HLSLからルートシグネチャを作成する (Create root signature from HLSL)
//	#define USE_HLSL_ROOTSIGNATRUE

#include <d3d12.h>
#include <d3dcompiler.h>
#include <DXGI1_6.h>

#include <DirectXMath.h>
/**
@brief 32 bit カラー DirectX::PackedVector::XMCOLOR
@note ARGBレイアウト

XMCOLOR Color32;
XMVECTOR Color128;

@note 128 bit カラー → 32 bit カラー
DirectX::PackedVector::XMStoreColor(&Color32, Color128);

@note 32 bit カラー → 128 bit カラー
Color128 = DirectX::PackedVector::XMLoadColor(Color32);
*/
#include <DirectXPackedVector.h>
#include <DirectXColors.h>

#include <comdef.h>

//!< _DEBUG であれば何もしなくても PIX 使用可能、Release で PIX を使用したいような場合は USE_PIX を定義する (When want to use pix in Release build, define USE_PIX)
//!< ソリューション右クリック - ソリューションのNuGetパッケージの管理 - 参照タブ - WinPixEventRuntimeで検索 - プロジェクトを選択してインストールしておくこと
//#define USE_PIX
#include <pix3.h>
//!< プログラムからキャプチャを行いたい場合 (Capture in program code)
#if defined(_DEBUG) || defined(USE_PIX)
#include <DXProgrammableCapture.h>
#endif

#ifndef BREAK_ON_FAILED
#define BREAK_ON_FAILED(vr) if(FAILED(vr)) { Log(DX::GetHRESULTString(vr).c_str()); DEBUG_BREAK(); }
#endif
#ifndef THROW_ON_FAILED
#define THROW_ON_FAILED(hr) if(FAILED(hr)) { throw std::runtime_error("VERIFY_SUCCEEDED failed : " + DX::GetHRESULTString(hr)); }
#endif
#ifndef MESSAGEBOX_ON_FAILED
#define MESSAGEBOX_ON_FAILED(hr) if(FAILED(hr)) { Win::ShowMessageBoxW(nullptr, DX::GetHRESULTStringW(hr)); }
#endif

#include "Cmn.h"
#include "Win.h"

/**
リソースが作成された時 MakeResident() され、破棄された時 Evict() される。
アプリから明示的にこれを行いたい場合は以下のようにする
ID3D12Resource* Resource;
const std::vector<ID3D12Pageable*> Pageables = { Resource };
Device->MakeResident(static_cast<UINT>(Pageables.size()), Pageables.data());
Device->Evict(static_cast<UINT>(Pageables.size()), Pageables.data());
*/

/**
CommandList、CommandAllocator はスレッドセーフではないので各スレッド毎に持つ必要がある
CommandQueue はスレッドフリーで各スレッドから使用可能
*/

class DX : public Cmn, public Win
{
private:
	using Super = Win;

public:
	virtual void OnCreate(HWND hWnd, HINSTANCE hInstance, LPCWSTR Title) override;
	//virtual void OnSize(HWND hWnd, HINSTANCE hInstance) override {}
	virtual void OnExitSizeMove(HWND hWnd, HINSTANCE hInstance) override;
	virtual void OnTimer(HWND hWnd, HINSTANCE hInstance) override { 
		Super::OnTimer(hWnd, hInstance); 
	}
	virtual void OnPaint(HWND hWnd, HINSTANCE hInstance) override { 
		Super::OnPaint(hWnd, hInstance); 
		Draw();
	}
	virtual void OnDestroy(HWND hWnd, HINSTANCE hInstance) override;

	static std::string GetHRESULTString(const HRESULT Result) { return ToString(GetHRESULTWString(Result)); }
	static std::wstring GetHRESULTWString(const HRESULT Result) { return std::wstring(_com_error(Result).ErrorMessage()); }
	static std::string GetFormatString(const DXGI_FORMAT Format);

protected:
	virtual void CopyToUploadResource(ID3D12Resource* Resource, const size_t Size, const void* Source);
	virtual void CopyToUploadResource(ID3D12Resource* Resource, const std::vector<D3D12_SUBRESOURCE_DATA>& SubresourceData, const std::vector<D3D12_PLACED_SUBRESOURCE_FOOTPRINT>& PlacedSubresourceFootprints, const std::vector<UINT>& NumRows, const std::vector<UINT64>& RowSizes);

	virtual void ExecuteCopyBuffer(ID3D12CommandAllocator* CommandAllocator, ID3D12GraphicsCommandList* CommandList, ID3D12Resource* SrcResource, ID3D12Resource* DstResource, const size_t Size);
	virtual void ExecuteCopyTexture(ID3D12CommandAllocator* CommandAllocator, ID3D12GraphicsCommandList* CommandList, ID3D12Resource* SrcResource, ID3D12Resource* DstResource, const std::vector<D3D12_PLACED_SUBRESOURCE_FOOTPRINT>& PlacedSubresourceFootprints, const D3D12_RESOURCE_STATES ResourceState);

	virtual void CreateUploadResource(ID3D12Resource** Resource, const size_t Size);
	virtual void CreateDefaultResource(ID3D12Resource** Resource, const size_t Size);

	virtual void ResourceBarrier(ID3D12GraphicsCommandList* CL, ID3D12Resource* Resource, const D3D12_RESOURCE_STATES Before, const D3D12_RESOURCE_STATES After);
	virtual void PopulateCopyTextureCommand(ID3D12GraphicsCommandList* CommandList, ID3D12Resource* Src, ID3D12Resource* Dst, const std::vector<D3D12_PLACED_SUBRESOURCE_FOOTPRINT>& PlacedSubresourceFootprints, const D3D12_RESOURCE_STATES ResourceState);
	virtual void PopulateCopyBufferCommand(ID3D12GraphicsCommandList* CommandList, ID3D12Resource* Src, ID3D12Resource* Dst, const std::vector<D3D12_PLACED_SUBRESOURCE_FOOTPRINT>& PlacedSubresourceFootprints, const D3D12_RESOURCE_STATES ResourceState);
	virtual void PopulateCopyBufferCommand(ID3D12GraphicsCommandList* CommandList, ID3D12Resource* Src, ID3D12Resource* Dst, const UINT64 Size, const D3D12_RESOURCE_STATES ResourceState);

#if defined(_DEBUG) || defined(USE_PIX)
	//!< #DX_TODO PIX 関連
	//PIXReportCounter(PCWSTR, float);
	//PIXNotifyWakeFromFenceSignal(HANDLE);
	static void SetName(ID3D12DeviceChild * Resource, LPCWSTR Name) { Resource->SetName(Name); }
	static void SetName(ID3D12DeviceChild* Resource, const std::wstring& Name) { SetName(Resource, Name.c_str()); }
	static void SetName(ID3D12DeviceChild* Resource, const std::string& Name) { SetName(Resource, ToWString(Name)); }
#endif

	virtual void CreateDevice(HWND hWnd);
	virtual void EnumAdapter(IDXGIFactory4* Factory);
	virtual void EnumOutput(IDXGIAdapter* Adapter);
	virtual void GetDisplayModeList(IDXGIOutput* Output, const DXGI_FORMAT Format);
	virtual void CheckFeatureLevel(ID3D12Device* Device);
	virtual void CheckMultiSample(const DXGI_FORMAT Format);
	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandle(ID3D12DescriptorHeap* DescriptorHeap, const D3D12_DESCRIPTOR_HEAP_TYPE Type, const UINT Index) const;
	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptorHandle(ID3D12DescriptorHeap* DescriptorHeap, const D3D12_DESCRIPTOR_HEAP_TYPE Type, const UINT Index) const;

	virtual void CreateCommandQueue();

	virtual void CreateFence();

	virtual void CreateCommandAllocator(COM_PTR<ID3D12CommandAllocator>& CA, const D3D12_COMMAND_LIST_TYPE CLT) {
		VERIFY_SUCCEEDED(Device->CreateCommandAllocator(CLT, COM_PTR_UUIDOF_PUTVOID(CA)));
	}
	virtual void CreateCommandAllocator();

	virtual void CreateCommandList(COM_PTR<ID3D12GraphicsCommandList>& CL, ID3D12CommandAllocator* CA, const D3D12_COMMAND_LIST_TYPE CLT);
	virtual void CreateCommandList();

	virtual void CreateSwapchain(HWND hWnd, const DXGI_FORMAT ColorFormat);
	virtual void CreateSwapChain(HWND hWnd, const DXGI_FORMAT ColorFormat, const UINT Width, const UINT Height);
	virtual void CreateSwapChain(HWND hWnd, const DXGI_FORMAT ColorFormat, const RECT& Rct) { CreateSwapChain(hWnd, ColorFormat, static_cast<uint32_t>(Rct.right - Rct.left), static_cast<uint32_t>(Rct.bottom - Rct.top)); }
	virtual void CreateSwapChainResource();
	virtual void InitializeSwapchainImage(ID3D12CommandAllocator* CommandAllocator, const DirectX::XMVECTORF32* Color = nullptr);
	virtual void InitializeSwapChain();
	virtual void ResetSwapChainResource() {
		for (auto& i : SwapChainResources) {
			COM_PTR_RESET(i);
		}
		SwapChainResources.clear();
	}
	virtual void ResizeSwapChain(const UINT Width, const UINT Height);
	virtual void ResizeSwapChain(const RECT& Rct) { ResizeSwapChain(static_cast<uint32_t>(Rct.right - Rct.left), static_cast<uint32_t>(Rct.bottom - Rct.top)); }
	UINT AcquireNextBackBufferIndex() const {
		return 0xffffffff == CurrentBackBufferIndex ? SwapChain->GetCurrentBackBufferIndex() : (CurrentBackBufferIndex + 1) % static_cast<const UINT>(SwapChainResources.size());
	}

	virtual void CreateDepthStencil() {}
	virtual void CreateDepthStencil(const DXGI_FORMAT DepthFormat, const UINT Width, const UINT Height);
	virtual void CreateDepthStencil(const DXGI_FORMAT DepthFormat, const RECT& Rct) { CreateDepthStencil(DepthFormat, static_cast<uint32_t>(Rct.right - Rct.left), static_cast<uint32_t>(Rct.bottom - Rct.top)); }
	virtual void CreateDepthStencilResource(const DXGI_FORMAT DepthFormat, const UINT Width, const UINT Height);
	virtual void ResizeDepthStencil(const DXGI_FORMAT DepthFormat, const UINT Width, const UINT Height);
	virtual void ResizeDepthStencil(const DXGI_FORMAT DepthFormat, const RECT& Rct) { ResizeDepthStencil(DepthFormat, static_cast<uint32_t>(Rct.right - Rct.left), static_cast<uint32_t>(Rct.bottom - Rct.top)); }

	virtual void LoadImage(ID3D12Resource** /*Resource*/, const std::wstring& /*Path*/, const D3D12_RESOURCE_STATES /*ResourceState*/) { assert(false && "Not implemanted"); }
	virtual void LoadImage(ID3D12Resource** Resource, const std::string& Path, const D3D12_RESOURCE_STATES ResourceState = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE) { LoadImage(Resource/*, DescriptorHeap*/, ToWString(Path), ResourceState); }

	virtual void CreateBuffer(ID3D12Resource** Res, const UINT32 Size, const void* Source, ID3D12CommandAllocator* CA, ID3D12GraphicsCommandList* CL);
	virtual void CreateVertexBuffer() {}
	virtual void CreateIndexBuffer() {}
	virtual void CreateIndirectBuffer() {}
	virtual void CreateConstantBuffer() {}

	virtual void CreateUnorderedAccessTexture();

	virtual void CreateViewport(const FLOAT Width, const FLOAT Height, const FLOAT MinDepth = 0.0f, const FLOAT MaxDepth = 1.0f);
	virtual void CreateViewport(const RECT& Rct, const FLOAT MinDepth = 0.0f, const FLOAT MaxDepth = 1.0f) { CreateViewport(static_cast<FLOAT>(Rct.right - Rct.left), static_cast<FLOAT>(Rct.bottom - Rct.top), MinDepth, MaxDepth); }
	virtual void CreateViewportTopFront(const FLOAT Width, const FLOAT Height) { CreateViewport(Width, Height, 0.0f, 0.0f); }

	virtual void SerializeRootSignature(COM_PTR<ID3DBlob>& Blob, const std::initializer_list<D3D12_ROOT_PARAMETER> il_RPs, const std::initializer_list<D3D12_STATIC_SAMPLER_DESC> il_SSDs, const D3D12_ROOT_SIGNATURE_FLAGS Flags);
	virtual void GetRootSignaturePartFromShader(COM_PTR<ID3DBlob>& Blob, LPCWSTR Path);
	virtual void CreateRootSignature(COM_PTR<ID3D12RootSignature>& RS, COM_PTR<ID3DBlob> Blob) {
		VERIFY_SUCCEEDED(Device->CreateRootSignature(0, Blob->GetBufferPointer(), Blob->GetBufferSize(), COM_PTR_UUIDOF_PUTVOID(RS)));
	}

	virtual void CreateRootSignature();

	virtual void CreateDescriptorHeap(COM_PTR<ID3D12DescriptorHeap>& DH, const D3D12_DESCRIPTOR_HEAP_DESC DHD) {
		VERIFY_SUCCEEDED(Device->CreateDescriptorHeap(&DHD, COM_PTR_UUIDOF_PUTVOID(DH)));
	}
	virtual void CreateDescriptorHeap() {}

	virtual void CreateConstantBufferView(const COM_PTR<ID3D12Resource>& Res, const COM_PTR<ID3D12DescriptorHeap>& DH, const size_t Size) {
		const D3D12_CONSTANT_BUFFER_VIEW_DESC CBVD = {
			Res->GetGPUVirtualAddress(),
			static_cast<UINT>(RoundUp(Size, 0xff)) //!< 256 byte align
		};
		Device->CreateConstantBufferView(&CBVD, GetCPUDescriptorHandle(COM_PTR_GET(DH), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 0));
	}
	virtual void CreateShaderResourceView(const COM_PTR<ID3D12Resource>& Res, const COM_PTR<ID3D12DescriptorHeap>& DH) {
		Device->CreateShaderResourceView(COM_PTR_GET(Res), nullptr, GetCPUDescriptorHandle(COM_PTR_GET(DH), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 0));
	}
	virtual void CreateUnorderedAccessView(const COM_PTR<ID3D12Resource>& Res, const COM_PTR<ID3D12DescriptorHeap>& DH) {
		D3D12_UNORDERED_ACCESS_VIEW_DESC UAVD = {
			DXGI_FORMAT_R8G8B8A8_UNORM,
			D3D12_UAV_DIMENSION_TEXTURE2D,
		};
		UAVD.Texture2D.MipSlice = 0;
		UAVD.Texture2D.PlaneSlice = 0;
		Device->CreateUnorderedAccessView(COM_PTR_GET(Res), nullptr, &UAVD, GetCPUDescriptorHandle(COM_PTR_GET(DH), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 0));
	}
	virtual void CreateDescriptorView() {}

	virtual void CreateShader(std::vector<COM_PTR<ID3DBlob>>& ShaderBlobs) const;
	virtual void CreateShaderBlob() {}

	virtual void CreatePipelineState();
	void CreatePipelineState_Default(COM_PTR<ID3D12PipelineState>& PipelineState, ID3D12RootSignature* RS,
		const D3D12_SHADER_BYTECODE VS, const D3D12_SHADER_BYTECODE PS, const D3D12_SHADER_BYTECODE DS, const D3D12_SHADER_BYTECODE HS, const D3D12_SHADER_BYTECODE GS);
	//virtual void CreatePipelineState_Compute();

	virtual void CreateTexture() {}
	virtual void CreateStaticSampler() {}

	virtual void ClearColor(ID3D12GraphicsCommandList* CommandList, const D3D12_CPU_DESCRIPTOR_HANDLE& DescriptorHandle, const DirectX::XMVECTORF32& Color);
	virtual void ClearDepthStencil(ID3D12GraphicsCommandList* CommandList, const D3D12_CPU_DESCRIPTOR_HANDLE& DescriptorHandle, const FLOAT Depth = 1.0f, const UINT8 Stencil = 0);
	virtual void PopulateCommandList(const size_t i);

	virtual void Draw();
	virtual void Dispatch();
	virtual void Present();
	virtual void WaitForFence();

protected:
#if defined(_DEBUG) || defined(USE_PIX)
	COM_PTR<IDXGraphicsAnalysis> GraphicsAnalysis;
#endif

	COM_PTR<IDXGIFactory4> Factory;
	COM_PTR<IDXGIAdapter> Adapter;
	COM_PTR<IDXGIOutput> Output;

	COM_PTR<ID3D12Device> Device;
	std::vector<DXGI_SAMPLE_DESC> SampleDescs;
	COM_PTR<ID3D12CommandQueue> CommandQueue;
	COM_PTR<ID3D12Fence> Fence;
	UINT64 FenceValue = 0;

	std::vector<COM_PTR<ID3D12CommandAllocator>> CommandAllocators;
	std::vector<COM_PTR<ID3D12GraphicsCommandList>> GraphicsCommandLists;
	//std::vector<COM_PTR<ID3D12CommandList>> CommandLists;
	
	COM_PTR<IDXGISwapChain4> SwapChain;
	COM_PTR<ID3D12DescriptorHeap> SwapChainDescriptorHeap;
	std::vector<COM_PTR<ID3D12Resource>> SwapChainResources;
	UINT CurrentBackBufferIndex = 0xffffffff;

	COM_PTR<ID3D12Resource> DepthStencilResource;
	COM_PTR<ID3D12DescriptorHeap> DepthStencilDescriptorHeap;

	COM_PTR<ID3D12Resource> ImageResource;
	COM_PTR<ID3D12DescriptorHeap> ImageDescriptorHeap;

	COM_PTR<ID3D12RootSignature> RootSignature;

	COM_PTR<ID3D12PipelineLibrary> PipelineLibrary;
	COM_PTR<ID3D12PipelineState> PipelineState; 
	
	std::vector<COM_PTR<ID3D12Resource>> VertexBufferResources;
	std::vector<D3D12_VERTEX_BUFFER_VIEW> VertexBufferViews;

	std::vector <COM_PTR<ID3D12Resource>> IndexBufferResources;
	std::vector<D3D12_INDEX_BUFFER_VIEW> IndexBufferViews;

	std::vector<COM_PTR<ID3D12Resource>> IndirectBufferResources;
	COM_PTR<ID3D12CommandSignature> IndirectCommandSignature;

	//!< 現状1つのみ、配列にする #DX_TODO
	COM_PTR<ID3D12Resource> ConstantBufferResource;
	COM_PTR<ID3D12DescriptorHeap> ConstantBufferDescriptorHeap; 

	COM_PTR<ID3D12Resource> UnorderedAccessTextureResource;
	COM_PTR<ID3D12DescriptorHeap> UnorderedAccessTextureDescriptorHeap; 
	
	std::vector<D3D12_STATIC_SAMPLER_DESC> StaticSamplerDescs;
	std::vector<COM_PTR<ID3D12DescriptorHeap>> SamplerDescriptorHeaps;

	std::vector<D3D12_VIEWPORT> Viewports;
	std::vector<D3D12_RECT> ScissorRects;

	std::vector<COM_PTR<ID3DBlob>> ShaderBlobs;

protected:
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
	const D3D12_SHADER_BYTECODE NullShaderBC = { nullptr, 0 };
};
