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
//#define USE_HLSL_ROOTSIGNATRUE

//!< バンドル : VKのセカンダリコマンドバッファ相当
#define USE_BUNDLE
//!< ルートコンスタント : VKのプッシュコンスタント相当
//#define USE_ROOT_CONSTANTS

#include <initguid.h>
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

	static std::array<float, 3> Lerp(const std::array<float, 3>& lhs, const std::array<float, 3>& rhs, const float t) {
		const auto l = DirectX::XMFLOAT3(lhs.data());
		const auto r = DirectX::XMFLOAT3(rhs.data());
		const auto v = DirectX::XMVectorLerp(DirectX::XMLoadFloat3(&l), DirectX::XMLoadFloat3(&r), t);
		return { v.m128_f32[0], v.m128_f32[1], v.m128_f32[2] };
	}
	static std::array<float, 4> Lerp(const std::array<float, 4>& lhs, const std::array<float, 4>& rhs, const float t) {
		const auto l = DirectX::XMFLOAT4(lhs.data());
		const auto r = DirectX::XMFLOAT4(rhs.data());
		const auto v = DirectX::XMVectorLerp(DirectX::XMLoadFloat4(&l), DirectX::XMLoadFloat4(&r), t);
		return { v.m128_f32[0], v.m128_f32[1], v.m128_f32[2], v.m128_f32[3] };
	}

protected:
	virtual void CopyToUploadResource(ID3D12Resource* Resource, const size_t Size, const void* Source, const D3D12_RANGE* Range = nullptr);
	virtual void CopyToUploadResource(ID3D12Resource* Resource, const std::vector<D3D12_PLACED_SUBRESOURCE_FOOTPRINT>& PlacedSubresourceFootprints, const std::vector<UINT>& NumRows, const std::vector<UINT64>& RowSizes, const std::vector<D3D12_SUBRESOURCE_DATA>& SubresourceData);

	virtual void ExecuteCopyBuffer(ID3D12Resource* DstResource, ID3D12CommandAllocator* CommandAllocator, ID3D12GraphicsCommandList* CommandList, const size_t Size, ID3D12Resource* SrcResource);
	virtual void ExecuteCopyTexture(ID3D12Resource* DstResource, ID3D12CommandAllocator* CommandAllocator, ID3D12GraphicsCommandList* CommandList, const std::vector<D3D12_PLACED_SUBRESOURCE_FOOTPRINT>& PlacedSubresourceFootprints, const D3D12_RESOURCE_STATES ResourceState, ID3D12Resource* SrcResource);

	virtual void CreateUploadResource(ID3D12Resource** Resource, const size_t Size);
	virtual void CreateDefaultResource(ID3D12Resource** Resource, const size_t Size);

	virtual void ResourceBarrier(ID3D12GraphicsCommandList* CL, ID3D12Resource* Resource, const D3D12_RESOURCE_STATES Before, const D3D12_RESOURCE_STATES After);
	virtual void PopulateCopyTextureCommand(ID3D12GraphicsCommandList* CL, ID3D12Resource* Src, ID3D12Resource* Dst, const std::vector<D3D12_PLACED_SUBRESOURCE_FOOTPRINT>& PSF, const D3D12_RESOURCE_STATES RS);
	virtual void PopulateCopyBufferCommand(ID3D12GraphicsCommandList* CL, ID3D12Resource* Src, ID3D12Resource* Dst, const std::vector<D3D12_PLACED_SUBRESOURCE_FOOTPRINT>& PSF, const D3D12_RESOURCE_STATES RS);
	virtual void PopulateCopyBufferCommand(ID3D12GraphicsCommandList* CL, ID3D12Resource* Src, ID3D12Resource* Dst, const UINT64 Size, const D3D12_RESOURCE_STATES RS);

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
	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandle(ID3D12DescriptorHeap* DH, const D3D12_DESCRIPTOR_HEAP_TYPE Type, const UINT Index) const {
		auto CDH = DH->GetCPUDescriptorHandleForHeapStart();
		CDH.ptr += static_cast<SIZE_T>(Index)* Device->GetDescriptorHandleIncrementSize(Type);
		return CDH;
	}
	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptorHandle(ID3D12DescriptorHeap* DH, const D3D12_DESCRIPTOR_HEAP_TYPE Type, const UINT Index) const {
		auto GDH = DH->GetGPUDescriptorHandleForHeapStart();
		GDH.ptr += static_cast<SIZE_T>(Index)* Device->GetDescriptorHandleIncrementSize(Type);
		return GDH;
	}
	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandle(ID3D12DescriptorHeap* DH, const UINT Index) const { return GetCPUDescriptorHandle(DH, DH->GetDesc().Type, Index); }
	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptorHandle(ID3D12DescriptorHeap* DH, const UINT Index) const { return GetGPUDescriptorHandle(DH, DH->GetDesc().Type, Index); }

	//!< DescriptorHandleIndices で管理する版
	D3D12_CPU_DESCRIPTOR_HANDLE GetCurrentCPUDescriptorHandle(ID3D12DescriptorHeap* DH, const D3D12_DESCRIPTOR_HEAP_TYPE Type) const { return GetCPUDescriptorHandle(DH, Type, DescriptorHandleIndices[Type]); }
	D3D12_GPU_DESCRIPTOR_HANDLE GetCurrentGPUDescriptorHandle(ID3D12DescriptorHeap* DH, const D3D12_DESCRIPTOR_HEAP_TYPE Type) const { return GetGPUDescriptorHandle(DH, Type, DescriptorHandleIndices[Type]); }
	D3D12_CPU_DESCRIPTOR_HANDLE GetCurrentCPUDescriptorHandle(ID3D12DescriptorHeap* DH) const { return DX::GetCPUDescriptorHandle(DH, DH->GetDesc().Type); }
	D3D12_CPU_DESCRIPTOR_HANDLE GetCurrentGPUDescriptorHandle(ID3D12DescriptorHeap* DH) const { return DX::GetCPUDescriptorHandle(DH, DH->GetDesc().Type); }
	void GetCurrentAndPushDescriptorHandles(D3D12_CPU_DESCRIPTOR_HANDLE& CDH, D3D12_GPU_DESCRIPTOR_HANDLE& GDH, ID3D12DescriptorHeap* DH) {
		const auto Type = DH->GetDesc().Type;
		CDH = GetCurrentCPUDescriptorHandle(DH, Type);
		GDH = GetCurrentGPUDescriptorHandle(DH, Type);
		PushDescriptorHandle(Type);
	}
	void PushDescriptorHandle(const D3D12_DESCRIPTOR_HEAP_TYPE Type) { DescriptorHandleIndices[Type]++; }
	void PopDescriptorHandle(const D3D12_DESCRIPTOR_HEAP_TYPE Type) { assert(0 < DescriptorHandleIndices[Type] && ""); DescriptorHandleIndices[Type]--; }
	void PushDescriptorHandle(ID3D12DescriptorHeap* DH) { PushDescriptorHandle(DH->GetDesc().Type); }
	void PopDescriptorHandle(ID3D12DescriptorHeap* DH) { PopDescriptorHandle(DH->GetDesc().Type); }

	virtual void CreateCommandQueue();

	virtual void CreateFence();

	virtual void CreateCommandAllocator();
	virtual UINT AddCommandList();
	virtual UINT AddBundleCommandList();
	virtual void CreateCommandList() { AddCommandList(); LOG_OK(); }
	virtual void CreateBundleCommandList() {}

	virtual void CreateSwapchain(HWND hWnd, const DXGI_FORMAT ColorFormat);
	virtual void CreateSwapChain(HWND hWnd, const DXGI_FORMAT ColorFormat, const UINT Width, const UINT Height);
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
	UINT AcquireNextBackBufferIndex() const {
		return 0xffffffff == CurrentBackBufferIndex ? SwapChain->GetCurrentBackBufferIndex() : (CurrentBackBufferIndex + 1) % static_cast<const UINT>(SwapChainResources.size());
	}

	virtual void CreateRenderTarget() {}
	virtual void CreateRenderTarget(const DXGI_FORMAT Format, const UINT Width, const UINT Height);

	virtual void CreateDepthStencil() {}
	virtual void CreateDepthStencil(const DXGI_FORMAT DepthFormat, const UINT Width, const UINT Height);
	virtual void CreateDepthStencilResource(const DXGI_FORMAT DepthFormat, const UINT Width, const UINT Height);
	virtual void ResizeDepthStencil(const DXGI_FORMAT DepthFormat, const UINT Width, const UINT Height);

	virtual void LoadImage(ID3D12Resource** /*Resource*/, const std::wstring& /*Path*/, const D3D12_RESOURCE_STATES /*ResourceState*/) { assert(false && "Not implemanted"); }
	virtual void LoadImage(ID3D12Resource** Resource, const std::string& Path, const D3D12_RESOURCE_STATES ResourceState = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE) { LoadImage(Resource/*, DescriptorHeap*/, ToWString(Path), ResourceState); }

	virtual void LoadScene() {}

	virtual void CreateAndCopyToUploadResource(COM_PTR<ID3D12Resource>& Res, const size_t Size, const void* Source);
	virtual void CreateAndCopyToDefaultResource(COM_PTR<ID3D12Resource>& Res, ID3D12CommandAllocator* CA, ID3D12GraphicsCommandList* CL, const size_t Size, const void* Source);
	virtual void CreateVertexBuffer() {}
	virtual void CreateIndexBuffer() {}
	virtual void CreateIndirectBuffer() {}
	virtual void CreateConstantBuffer() {}

	virtual void CreateUnorderedAccessTexture();

	virtual void CreateViewport(const FLOAT Width, const FLOAT Height, const FLOAT MinDepth = 0.0f, const FLOAT MaxDepth = 1.0f);
	virtual void CreateViewportTopFront(const FLOAT Width, const FLOAT Height) { CreateViewport(Width, Height, 0.0f, 0.0f); }

	virtual void SerializeRootSignature(COM_PTR<ID3DBlob>& Blob, const std::initializer_list<D3D12_ROOT_PARAMETER> il_RPs, const std::initializer_list<D3D12_STATIC_SAMPLER_DESC> il_SSDs, const D3D12_ROOT_SIGNATURE_FLAGS Flags);
	virtual void GetRootSignaturePartFromShader(COM_PTR<ID3DBlob>& Blob, LPCWSTR Path);
	virtual void CreateRootSignature(COM_PTR<ID3D12RootSignature>& RS, COM_PTR<ID3DBlob> Blob) {
		VERIFY_SUCCEEDED(Device->CreateRootSignature(0, Blob->GetBufferPointer(), Blob->GetBufferSize(), COM_PTR_UUIDOF_PUTVOID(RS)));
	}

	virtual void CreateRootSignature();

	virtual void CreateDescriptorHeap() {}
	virtual void CreateDescriptorView() {}

	virtual void CreateShader(std::vector<COM_PTR<ID3DBlob>>& ShaderBlobs) const;
	virtual void CreateShaderBlob() {}

#include "DXPipelineLibrary.inl"
	virtual void CreatePipelineState() {}
	static void CreatePipelineState(COM_PTR<ID3D12PipelineState>& PST, ID3D12Device* Device, ID3D12RootSignature* RS,
		const D3D12_PRIMITIVE_TOPOLOGY_TYPE Topology,
		const D3D12_SHADER_BYTECODE VS, const D3D12_SHADER_BYTECODE PS, const D3D12_SHADER_BYTECODE DS, const D3D12_SHADER_BYTECODE HS, const D3D12_SHADER_BYTECODE GS,
		const std::vector<D3D12_INPUT_ELEMENT_DESC>& IEDs, 
		const PipelineLibrarySerializer* PLS = nullptr, LPCWSTR Name = nullptr);
	//virtual void CreatePipelineState_Compute();

	virtual void CreateTexture() {}
	virtual void CreateStaticSampler() {}
	virtual void CreateSampler() {}

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
	std::vector<COM_PTR<ID3D12CommandAllocator>> BundleCommandAllocators;
	std::vector<COM_PTR<ID3D12GraphicsCommandList>> BundleGraphicsCommandLists;
	//std::vector<COM_PTR<ID3D12CommandList>> CommandLists;
	
	COM_PTR<IDXGISwapChain4> SwapChain;
	COM_PTR<ID3D12DescriptorHeap> SwapChainDescriptorHeap;
	std::vector<COM_PTR<ID3D12Resource>> SwapChainResources;
	UINT CurrentBackBufferIndex = 0xffffffff;

	COM_PTR<ID3D12Resource> RenderTargetResource;
	COM_PTR<ID3D12DescriptorHeap> RenderTargetDescriptorHeap;
	COM_PTR<ID3D12DescriptorHeap> ShaderResourceDescriptorHeap;

	COM_PTR<ID3D12Resource> DepthStencilResource;
	COM_PTR<ID3D12DescriptorHeap> DepthStencilDescriptorHeap;

	COM_PTR<ID3D12Resource> ImageResource;
	COM_PTR<ID3D12DescriptorHeap> ImageDescriptorHeap;

	std::vector<COM_PTR<ID3D12RootSignature>> RootSignatures;

	COM_PTR<ID3D12PipelineLibrary> PipelineLibrary;
	std::vector<COM_PTR<ID3D12PipelineState>> PipelineStates;

	std::vector<COM_PTR<ID3D12Resource>> VertexBufferResources;
	std::vector<D3D12_VERTEX_BUFFER_VIEW> VertexBufferViews;

	std::vector <COM_PTR<ID3D12Resource>> IndexBufferResources;
	std::vector<D3D12_INDEX_BUFFER_VIEW> IndexBufferViews;

	std::vector<COM_PTR<ID3D12Resource>> IndirectBufferResources;
	std::vector<COM_PTR<ID3D12CommandSignature>> IndirectCommandSignatures;
	//COM_PTR<ID3D12CommandSignature> IndirectCommandSignature;

	//!< 現状1つのみ、配列にする #DX_TODO
	//COM_PTR<ID3D12Resource> ConstantBufferResource;
	std::vector<COM_PTR<ID3D12Resource>> ConstantBuffers;
	COM_PTR<ID3D12DescriptorHeap> ConstantBufferDescriptorHeap;

	COM_PTR<ID3D12Resource> UnorderedAccessTextureResource;
	COM_PTR<ID3D12DescriptorHeap> UnorderedAccessTextureDescriptorHeap; 
	
	std::vector<D3D12_STATIC_SAMPLER_DESC> StaticSamplerDescs;
	std::vector<COM_PTR<ID3D12DescriptorHeap>> SamplerDescriptorHeaps;

	std::vector<D3D12_VIEWPORT> Viewports;
	std::vector<D3D12_RECT> ScissorRects;

	std::vector<COM_PTR<ID3DBlob>> ShaderBlobs;

	std::array<UINT, D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES> DescriptorHandleIndices{};

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

#ifdef DEBUG_STDOUT
static std::ostream& operator<<(std::ostream& rhs, const DirectX::XMVECTOR& Value) { for (auto i = 0; i < 4; ++i) { rhs << Value.m128_f32[i] << ", "; } rhs << std::endl; return rhs; }
static std::ostream& operator<<(std::ostream& rhs, const DirectX::XMMATRIX& Value) { for (auto i = 0; i < 4; ++i) { rhs << Value.r[i]; } rhs << std::endl; return rhs; }
#endif