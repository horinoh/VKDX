#pragma once

#define USE_WINRT
#define USE_WRL
#ifdef USE_WINRT
//!< プロジェクト右クリック - Property - All Configurations にする - C / C++ - Language - C++ Language Standard - ISO C++17 Standard を選択しておくこと (デフォルトではC++14)
#include <winrt/base.h>
#endif
#ifdef USE_WRL
#include <wrl.h>
#endif

#include <d3d12.h>
#include <d3dcompiler.h>
#include <DXGI1_4.h>

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

//!< シェーダブロブからルートシグネチャを作成する場合 (Create root signature from shader blob)
//#define ROOTSIGNATRUE_FROM_SHADER

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
	//virtual void OnTimer(HWND hWnd, HINSTANCE hInstance) override { Super::OnTimer(hWnd, hInstance); }
	virtual void OnPaint(HWND hWnd, HINSTANCE hInstance) override { Super::OnPaint(hWnd, hInstance); Draw(); }
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
	virtual HRESULT CreateMaxFeatureLevelDevice(IDXGIAdapter* Adapter);
	virtual void EnumAdapter(IDXGIFactory4* Factory);
	virtual void EnumOutput(IDXGIAdapter* Adapter);
	virtual void GetDisplayModeList(IDXGIOutput* Output, const DXGI_FORMAT Format);
	virtual void CheckFeatureLevel();
	virtual void CheckMultiSample(const DXGI_FORMAT Format);
	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandle(ID3D12DescriptorHeap* DescriptorHeap, const D3D12_DESCRIPTOR_HEAP_TYPE Type, const UINT Index = 0) const;
	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptorHandle(ID3D12DescriptorHeap* DescriptorHeap, const D3D12_DESCRIPTOR_HEAP_TYPE Type, const UINT Index = 0) const;

	virtual void CreateCommandQueue();

	virtual void CreateFence();

	virtual void CreateCommandAllocator(const D3D12_COMMAND_LIST_TYPE CommandListType = D3D12_COMMAND_LIST_TYPE_DIRECT);
	virtual void CreateCommandList(ID3D12CommandAllocator* CommandAllocator, const size_t Count, const D3D12_COMMAND_LIST_TYPE CommandListType);
	virtual void CreateCommandList();

	virtual void CreateSwapchain(HWND hWnd, const DXGI_FORMAT ColorFormat);
	virtual void CreateSwapChain(HWND hWnd, const DXGI_FORMAT ColorFormat, const UINT Width, const UINT Height);
	virtual void CreateSwapChain(HWND hWnd, const DXGI_FORMAT ColorFormat, const RECT& Rect) { CreateSwapChain(hWnd, ColorFormat, static_cast<uint32_t>(Rect.right - Rect.left), static_cast<uint32_t>(Rect.bottom - Rect.top)); }
	virtual void CreateSwapChainResource();
	virtual void InitializeSwapchainImage(ID3D12CommandAllocator* CommandAllocator, const DirectX::XMVECTORF32* Color = nullptr);
	virtual void InitializeSwapChain();
	virtual void ResetSwapChainResource() {
		for (auto& i : SwapChainResources) {
#ifdef USE_WINRT
			i = nullptr;
#elif defined(USE_WRL)
			i.Reset();
#endif
		}
	}
	virtual void ResizeSwapChain(const UINT Width, const UINT Height);
	virtual void ResizeSwapChain(const RECT& Rect) { ResizeSwapChain(static_cast<uint32_t>(Rect.right - Rect.left), static_cast<uint32_t>(Rect.bottom - Rect.top)); }
	UINT AcquireNextBackBufferIndex() const {
		return 0xffffffff == CurrentBackBufferIndex ? SwapChain->GetCurrentBackBufferIndex() : (CurrentBackBufferIndex + 1) % static_cast<const UINT>(SwapChainResources.size());
	}

	virtual void CreateDepthStencil() {}
	virtual void CreateDepthStencil(const DXGI_FORMAT DepthFormat, const UINT Width, const UINT Height);
	virtual void CreateDepthStencil(const DXGI_FORMAT DepthFormat, const RECT& Rect) { CreateDepthStencil(DepthFormat, static_cast<uint32_t>(Rect.right - Rect.left), static_cast<uint32_t>(Rect.bottom - Rect.top)); }
	virtual void CreateDepthStencilResource(const DXGI_FORMAT DepthFormat, const UINT Width, const UINT Height);
	virtual void ResizeDepthStencil(const DXGI_FORMAT DepthFormat, const UINT Width, const UINT Height);
	virtual void ResizeDepthStencil(const DXGI_FORMAT DepthFormat, const RECT& Rect) { ResizeDepthStencil(DepthFormat, static_cast<uint32_t>(Rect.right - Rect.left), static_cast<uint32_t>(Rect.bottom - Rect.top)); }

	virtual void LoadImage(ID3D12Resource** Resource/*, ID3D12DescriptorHeap** DescriptorHeap*/, const std::wstring& Path, const D3D12_RESOURCE_STATES ResourceState = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE) { assert(false && "Not implemanted"); }
	virtual void LoadImage(ID3D12Resource** Resource/*, ID3D12DescriptorHeap** DescriptorHeap*/, const std::string& Path, const D3D12_RESOURCE_STATES ResourceState = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE) { LoadImage(Resource/*, DescriptorHeap*/, ToWString(Path), ResourceState); }

	virtual void CreateVertexBuffer() {}
	virtual void CreateIndexBuffer() {}
	virtual void CreateIndirectBuffer(ID3D12Resource** Resource, const UINT32 Size, const void* Source, ID3D12CommandAllocator* CA, ID3D12GraphicsCommandList* CL);
	virtual void CreateIndirectBuffer() {}
	virtual void CreateConstantBuffer();

	virtual void CreateUnorderedAccessTexture();

	virtual void CreateViewport(const FLOAT Width, const FLOAT Height, const FLOAT MinDepth = 0.0f, const FLOAT MaxDepth = 1.0f);
	virtual void CreateViewport(const RECT& Rect, const FLOAT MinDepth = 0.0f, const FLOAT MaxDepth = 1.0f) { CreateViewport(static_cast<FLOAT>(Rect.right - Rect.left), static_cast<FLOAT>(Rect.bottom - Rect.top), MinDepth, MaxDepth); }
	virtual void CreateViewportTopFront(const FLOAT Width, const FLOAT Height) { CreateViewport(Width, Height, 0.0f, 0.0f); }

#ifdef USE_WINRT
	virtual void SerializeRootSignature(winrt::com_ptr<ID3DBlob>& RSBlob, const std::initializer_list<D3D12_ROOT_PARAMETER> il_RPs, const std::initializer_list<D3D12_STATIC_SAMPLER_DESC> il_SSDs, const D3D12_ROOT_SIGNATURE_FLAGS Flags);
	virtual void GetRootSignaturePartFromShader(winrt::com_ptr<ID3DBlob>& RSBlob);
	virtual void CreateRootSignature(winrt::com_ptr<ID3D12RootSignature>& RS, winrt::com_ptr<ID3DBlob> Blob);
#elif defined(USE_WRL)
	virtual void SerializeRootSignature(Microsoft::WRL::ComPtr<ID3DBlob>& RSBlob, const std::initializer_list<D3D12_ROOT_PARAMETER> il_RPs, const std::initializer_list<D3D12_STATIC_SAMPLER_DESC> il_SSDs, const D3D12_ROOT_SIGNATURE_FLAGS Flags);
	virtual void GetRootSignaturePartFromShader(Microsoft::WRL::ComPtr<ID3DBlob>& RSBlob);
	virtual void CreateRootSignature(Microsoft::WRL::ComPtr<ID3D12RootSignature>& RS, Microsoft::WRL::ComPtr<ID3DBlob> Blob);
#endif
	virtual void CreateRootSignature();

#ifdef USE_WINRT
	virtual void CreateDescriptorHeap(winrt::com_ptr<ID3D12DescriptorHeap>& DH, const D3D12_DESCRIPTOR_HEAP_DESC DHD) {
		VERIFY_SUCCEEDED(Device->CreateDescriptorHeap(&DHD, __uuidof(DH), DH.put_void()));
	}
#elif defined(USE_WRL)
	virtual void CreateDescriptorHeap(Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>& DH, const D3D12_DESCRIPTOR_HEAP_DESC DHD) {
		VERIFY_SUCCEEDED(Device->CreateDescriptorHeap(&DHD, IID_PPV_ARGS(DH.GetAddressOf())));
	}
#endif
	virtual void CreateDescriptorHeap() {}

#ifdef USE_WINRT
	virtual void CreateConstantBufferView(winrt::com_ptr<ID3D12Resource>& Res, winrt::com_ptr<ID3D12DescriptorHeap>& DH, const size_t Size) {
		const D3D12_CONSTANT_BUFFER_VIEW_DESC CBVD = {
			Res->GetGPUVirtualAddress(),
			static_cast<UINT>(RoundUp(Size, 0xff)) //!< 256 byte align
		};
		Device->CreateConstantBufferView(&CBVD, GetCPUDescriptorHandle(DH.get(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));
	}
	virtual void CreateShaderResourceView(winrt::com_ptr<ID3D12Resource>& Res, winrt::com_ptr<ID3D12DescriptorHeap>& DH) {
		Device->CreateShaderResourceView(Res.get(), nullptr, GetCPUDescriptorHandle(DH.get(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));
	}
	virtual void CreateUnorderedAccessView(winrt::com_ptr<ID3D12Resource>& Res, winrt::com_ptr<ID3D12DescriptorHeap>& DH) {
		D3D12_UNORDERED_ACCESS_VIEW_DESC UAVD = {
			DXGI_FORMAT_R8G8B8A8_UNORM,
			D3D12_UAV_DIMENSION_TEXTURE2D,
		};
		UAVD.Texture2D.MipSlice = 0;
		UAVD.Texture2D.PlaneSlice = 0;
		Device->CreateUnorderedAccessView(Res.get(), nullptr, &UAVD, GetCPUDescriptorHandle(DH.get(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));
	}
#elif defined(USE_WRL)
	virtual void CreateConstantBufferView(Microsoft::WRL::ComPtr<ID3D12Resource> Res, Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>& DH, const size_t Size) {
		const D3D12_CONSTANT_BUFFER_VIEW_DESC CBVD = {
			Res->GetGPUVirtualAddress(),
			static_cast<UINT>(RoundUp(Size, 0xff))
		};
		Device->CreateConstantBufferView(&CBVD, GetCPUDescriptorHandle(DH.Get(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));
	}
	virtual void CreateShaderResourceView(Microsoft::WRL::ComPtr<ID3D12Resource> Res, Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>& DH) {
		Device->CreateShaderResourceView(Resource.Get(), nullptr, GetCPUDescriptorHandle(DH.Get(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));
	}
	virtual void CreateUnorderedAccessView(winrt::com_ptr<ID3D12Resource>& Res, winrt::com_ptr<ID3D12DescriptorHeap>& DH) {
		D3D12_UNORDERED_ACCESS_VIEW_DESC UAVD = {
			DXGI_FORMAT_R8G8B8A8_UNORM,
			D3D12_UAV_DIMENSION_TEXTURE2D,
		};
		UAVD.Texture2D.MipSlice = 0;
		UAVD.Texture2D.PlaneSlice = 0;
		Device->CreateUnorderedAccessView(Res.Get(), nullptr, &UAVD, GetCPUDescriptorHandle(DH.Get(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));
	}
#endif
	virtual void CreateDescriptorView() {}

	virtual void UpdateDescriptorHeap() { /*CopyToUploadResource()等を行う*/ }

#ifdef USE_WINRT
	virtual void CreateShader(std::vector<winrt::com_ptr<ID3DBlob>>& ShaderBlobs) const;
#elif defined(USE_WRL)
	virtual void CreateShader(std::vector<Microsoft::WRL::ComPtr<ID3DBlob>>& ShaderBlobs) const;
	virtual void CreateShader(std::vector<Microsoft::WRL::ComPtr<ID3DBlob>>& ShaderBlobs) const;
#endif
	virtual void CreateShaderBlob() {}

	virtual void CreatePipelineState();
	void CreatePipelineState_Default(winrt::com_ptr<ID3D12PipelineState>& PipelineState, ID3D12RootSignature* RS,
		const D3D12_SHADER_BYTECODE VS, const D3D12_SHADER_BYTECODE PS, const D3D12_SHADER_BYTECODE DS, const D3D12_SHADER_BYTECODE HS, const D3D12_SHADER_BYTECODE GS);
	//virtual void CreatePipelineState_Compute();

	virtual void CreateTexture() {}
	virtual void CreateStaticSamplerDesc(D3D12_STATIC_SAMPLER_DESC& StaticSamplerDesc, const D3D12_SHADER_VISIBILITY ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL, const FLOAT MaxLOD = (std::numeric_limits<FLOAT>::max)()) const {}

	virtual void ClearColor(ID3D12GraphicsCommandList* CommandList, const D3D12_CPU_DESCRIPTOR_HANDLE& DescriptorHandle, const DirectX::XMVECTORF32& Color);
	virtual void ClearDepthStencil(ID3D12GraphicsCommandList* CommandList, const D3D12_CPU_DESCRIPTOR_HANDLE& DescriptorHandle, const FLOAT Depth = 1.0f, const UINT8 Stencil = 0);
	virtual void PopulateCommandList(const size_t i);

	virtual void Draw();
	virtual void Dispatch();
	virtual void Present();
	virtual void WaitForFence();

protected:
#if defined(_DEBUG) || defined(USE_PIX)
#ifdef USE_WINRT
	winrt::com_ptr<IDXGraphicsAnalysis> GraphicsAnalysis;
#elif defined(USE_WRL)
	Microsoft::WRL::ComPtr<IDXGraphicsAnalysis> GraphicsAnalysis;
#endif
#endif

#ifdef USE_WINRT
	winrt::com_ptr<ID3D12Device> Device;
#elif defined(USE_WRL)
	Microsoft::WRL::ComPtr<ID3D12Device> Device;
#endif
	std::vector<DXGI_SAMPLE_DESC> SampleDescs;
#ifdef USE_WINRT
	winrt::com_ptr<ID3D12CommandQueue> CommandQueue;
	winrt::com_ptr<ID3D12Fence> Fence;
#elif defined(USE_WRL)
	Microsoft::WRL::ComPtr<ID3D12CommandQueue> CommandQueue;
	Microsoft::WRL::ComPtr<ID3D12Fence> Fence;
#endif
	UINT64 FenceValue = 0;

#ifdef USE_WINRT
	std::vector<winrt::com_ptr<ID3D12CommandAllocator>> CommandAllocators;
	std::vector<winrt::com_ptr<ID3D12GraphicsCommandList>> GraphicsCommandLists;
	//std::vector<winrt::com_ptr<ID3D12CommandList>> CommandLists;
#elif defined(USE_WRL)
	std::vector<Microsoft::WRL::ComPtr<ID3D12CommandAllocator>> CommandAllocators;
	std::vector<Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>> GraphicsCommandLists;
	//std::vector<Microsoft::WRL::ComPtr<ID3D12CommandList>> CommandLists;
#endif
	
#ifdef USE_WINRT
	winrt::com_ptr<IDXGISwapChain3> SwapChain;
	winrt::com_ptr<ID3D12DescriptorHeap> SwapChainDescriptorHeap;
	std::vector<winrt::com_ptr<ID3D12Resource>> SwapChainResources;
#elif defined(USE_WRL)
	Microsoft::WRL::ComPtr<IDXGISwapChain3> SwapChain;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> SwapChainDescriptorHeap; 
	std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> SwapChainResources;
#endif
	UINT CurrentBackBufferIndex = 0xffffffff;

#ifdef USE_WINRT
	winrt::com_ptr<ID3D12Resource> DepthStencilResource;
	winrt::com_ptr<ID3D12DescriptorHeap> DepthStencilDescriptorHeap;
#elif defined(USE_WRL)
	Microsoft::WRL::ComPtr<ID3D12Resource> DepthStencilResource;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> DepthStencilDescriptorHeap; 
#endif

#ifdef USE_WINRT
	winrt::com_ptr<ID3D12Resource> ImageResource;
	winrt::com_ptr<ID3D12DescriptorHeap> ImageDescriptorHeap;
#elif defined(USE_WRL)
	Microsoft::WRL::ComPtr<ID3D12Resource> ImageResource;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> ImageDescriptorHeap;
#endif

#ifdef USE_WINRT
	winrt::com_ptr<ID3D12Resource> SamplerResource;
	winrt::com_ptr<ID3D12DescriptorHeap> SamplerDescriptorHeap;
#elif defined(USE_WRL)
	Microsoft::WRL::ComPtr<ID3D12Resource> SamplerResource;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> SamplerDescriptorHeap; 
#endif
	std::vector<D3D12_STATIC_SAMPLER_DESC> StaticSamplerDescs;

#ifdef USE_WINRT
	winrt::com_ptr<ID3D12RootSignature> RootSignature;
#elif defined(USE_WRL)
	Microsoft::WRL::ComPtr<ID3D12RootSignature> RootSignature;
#endif

#ifdef USE_WINRT
	winrt::com_ptr<ID3D12PipelineLibrary> PipelineLibrary;
	winrt::com_ptr<ID3D12PipelineState> PipelineState; 
#elif defined(USE_WRL)
	Microsoft::WRL::ComPtr<ID3D12PipelineLibrary> PipelineLibrary;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> PipelineState; 
#endif
	
#ifdef USE_WINRT
	winrt::com_ptr<ID3D12Resource> VertexBufferResource;
#elif defined(USE_WRL)
	Microsoft::WRL::ComPtr<ID3D12Resource> VertexBufferResource;
#endif
	std::vector<D3D12_VERTEX_BUFFER_VIEW> VertexBufferViews;

#ifdef USE_WINRT
	winrt::com_ptr<ID3D12Resource> IndexBufferResource;
#elif defined(USE_WRL)
	Microsoft::WRL::ComPtr<ID3D12Resource> IndexBufferResource;
#endif
	D3D12_INDEX_BUFFER_VIEW IndexBufferView;
	UINT IndexCount = 0;

#ifdef USE_WINRT
	winrt::com_ptr<ID3D12Resource> IndirectBufferResource;
	winrt::com_ptr<ID3D12CommandSignature> IndirectCommandSignature; 
#elif defined(USE_WRL)
	Microsoft::WRL::ComPtr<ID3D12Resource> IndirectBufferResource;
	Microsoft::WRL::ComPtr<ID3D12CommandSignature> IndirectCommandSignature; 
#endif

	//!< 現状1つのみ、配列にする #DX_TODO
#ifdef USE_WINRT
	winrt::com_ptr<ID3D12Resource> ConstantBufferResource;
	winrt::com_ptr<ID3D12DescriptorHeap> ConstantBufferDescriptorHeap; 
#elif defined(USE_WRL)
	Microsoft::WRL::ComPtr<ID3D12Resource> ConstantBufferResource;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> ConstantBufferDescriptorHeap; 
#endif

#ifdef USE_WINRT
	winrt::com_ptr<ID3D12Resource> UnorderedAccessTextureResource;
	winrt::com_ptr<ID3D12DescriptorHeap> UnorderedAccessTextureDescriptorHeap; 
#elif defined(USE_WRL)
	Microsoft::WRL::ComPtr<ID3D12Resource> UnorderedAccessTextureResource;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> UnorderedAccessTextureDescriptorHeap; 
#endif
	
	std::vector<D3D12_VIEWPORT> Viewports;
	std::vector<D3D12_RECT> ScissorRects;

#ifdef USE_WINRT
	std::vector<winrt::com_ptr<ID3DBlob>> ShaderBlobs;
#elif defined(USE_WRL)
	std::vector<Microsoft::WRL::ComPtr <<ID3DBlob>> ShaderBlobs;
#endif
	//std::vector<D3D12_SHADER_BYTECODE> ShaderByteCodes;

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
