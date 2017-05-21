#pragma once

#include <wrl.h>

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

#ifndef BREAK_ON_FAILED
#define BREAK_ON_FAILED(vr) if(FAILED(vr)) { DEBUG_BREAK(); }
#endif
#ifndef THROW_ON_FAILED
#define THROW_ON_FAILED(hr) if(FAILED(hr)) { throw std::runtime_error("VERIFY_SUCCEEDED failed : " + DX::GetHRESULTString(hr)); }
#endif
#ifndef MESSAGEBOX_ON_FAILED
#define MESSAGEBOX_ON_FAILED(hr) if(FAILED(hr)) { Win::ShowMessageBoxW(nullptr, DX::GetHRESULTStringW(hr)); }
#endif
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

class DX : public Win
{
private:
	using Super = Win;

public:
	virtual void OnCreate(HWND hWnd, HINSTANCE hInstance, LPCWSTR Title) override;
	virtual void OnSize(HWND hWnd, HINSTANCE hInstance) override;
	//virtual void OnTimer(HWND hWnd, HINSTANCE hInstance) override { Super::OnTimer(hWnd, hInstance); }
	virtual void OnPaint(HWND hWnd, HINSTANCE hInstance) override { Super::OnPaint(hWnd, hInstance); Draw(); }
	virtual void OnDestroy(HWND hWnd, HINSTANCE hInstance) override;

	static std::string GetHRESULTString(const HRESULT Result);
	static std::wstring GetHRESULTStringW(const HRESULT Result);
	static std::string GetFormatString(const DXGI_FORMAT Format);

protected:
	virtual void CreateDefaultResource(ID3D12CommandAllocator* CommandAllocator, ID3D12GraphicsCommandList* CommandList, ID3D12Resource** Resource, const size_t Size, const void* Source);
	virtual void CreateUploadResource(ID3D12Resource** Resource, const size_t Size, const void* Source);
	virtual void CreateUploadResource(ID3D12Resource** Resource, const std::vector<D3D12_SUBRESOURCE_DATA>& SubresourceData, const UINT64 TotalSize, const std::vector<D3D12_PLACED_SUBRESOURCE_FOOTPRINT>& PlacedSubresourceFootprints, const std::vector<UINT>& NumRows, const std::vector<UINT64>& RowSizes);
	virtual void ResourceBarrier(ID3D12GraphicsCommandList* CommandList, ID3D12Resource* Resource, const D3D12_RESOURCE_STATES Before, const D3D12_RESOURCE_STATES After);
	virtual void PopulateCopyTextureCommand(ID3D12GraphicsCommandList* CommandList, ID3D12Resource* Src, ID3D12Resource* Dst, const std::vector<D3D12_PLACED_SUBRESOURCE_FOOTPRINT>& PlacedSubresourceFootprints, const D3D12_RESOURCE_STATES ResourceState);
	virtual void PopulateCopyBufferCommand(ID3D12GraphicsCommandList* CommandList, ID3D12Resource* Src, ID3D12Resource* Dst, const std::vector<D3D12_PLACED_SUBRESOURCE_FOOTPRINT>& PlacedSubresourceFootprints, const D3D12_RESOURCE_STATES ResourceState);
	virtual void PopulateCopyBufferCommand(ID3D12GraphicsCommandList* CommandList, ID3D12Resource* Src, ID3D12Resource* Dst, const UINT64 Size, const D3D12_RESOURCE_STATES ResourceState);
	
#ifdef _DEBUG
	//!< Insert() 相当は無い？
	static void BeginEvent(ID3D12GraphicsCommandList* CommandList, LPCWSTR Name);
	static void BeginEvent(ID3D12GraphicsCommandList* CommandList, const std::wstring& Name) { BeginEvent(CommandList, Name.c_str()); }
	static void BeginEvent(ID3D12GraphicsCommandList* CommandList, const std::string& Name) { BeginEvent(CommandList, std::wstring(Name.begin(), Name.end())); }
	static void EndEvent(ID3D12GraphicsCommandList* CommandList);
	static void SetName(ID3D12DeviceChild* Resource, LPCWSTR Name) { Resource->SetName(Name); }
	static void SetName(ID3D12DeviceChild* Resource, const std::wstring& Name) { SetName(Resource, Name.c_str()); }
	static void SetName(ID3D12DeviceChild* Resource, const std::string& Name) { SetName(Resource, std::wstring(Name.begin(), Name.end())); }
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
	virtual void CreateCommandList(ID3D12CommandAllocator* CommandAllocator, const size_t Count, const D3D12_COMMAND_LIST_TYPE CommandListType = D3D12_COMMAND_LIST_TYPE_DIRECT);

	virtual void CreateSwapchain(HWND hWnd, const DXGI_FORMAT ColorFormat);
	virtual void CreateSwapChain(HWND hWnd, const DXGI_FORMAT ColorFormat, const UINT Width, const UINT Height);
	virtual void CreateSwapChainOfClientRect(HWND hWnd, const DXGI_FORMAT ColorFormat) {
		CreateSwapChain(hWnd, ColorFormat, static_cast<UINT>(GetClientRectWidth()), static_cast<UINT>(GetClientRectHeight()));
	}
	virtual void CreateSwapChainResource();
	virtual void InitializeSwapchainImage(ID3D12CommandAllocator* CommandAllocator, const DirectX::XMVECTORF32* Color = nullptr);
	virtual void ResetSwapChainResource() { for (auto& i : SwapChainResources) { i.Reset(); } }
	virtual void ResizeSwapChain(const UINT Width, const UINT Height);
	virtual void ResizeSwapChainToClientRect() { 
		ResizeSwapChain(static_cast<const UINT>(GetClientRectWidth()), static_cast<const UINT>(GetClientRectHeight())); 
	}
	UINT AcquireNextBackBufferIndex() const {
		return 0xffffffff == CurrentBackBufferIndex ? SwapChain->GetCurrentBackBufferIndex() : (CurrentBackBufferIndex + 1) % static_cast<const UINT>(SwapChainResources.size());
	}

	virtual void CreateDepthStencil();
	virtual void CreateDepthStencilDescriptorHeap();
	virtual void ResetDepthStencilResource();
	virtual void CreateDepthStencilResource(const UINT Width, const UINT Height, const DXGI_FORMAT DepthFormat = DXGI_FORMAT_D32_FLOAT_S8X24_UINT);
	virtual void ResizeDepthStencil(const UINT Width, const UINT Height, const DXGI_FORMAT DepthFormat = DXGI_FORMAT_D32_FLOAT_S8X24_UINT);
	virtual void ResizeDepthStencilToClientRect(const DXGI_FORMAT DepthFormat = DXGI_FORMAT_D32_FLOAT_S8X24_UINT) {
		ResizeDepthStencil(static_cast<UINT>(GetClientRectWidth()), static_cast<UINT>(GetClientRectHeight()), DepthFormat);
	}

	virtual void CreateImageDescriptorHeap(ID3D12DescriptorHeap** DescriptorHeap);
	virtual void LoadImage(ID3D12Resource** Resource, ID3D12DescriptorHeap** DescriptorHeap, const std::wstring& Path, const D3D12_RESOURCE_STATES ResourceState = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE) { CreateImageDescriptorHeap(DescriptorHeap); }
	virtual void LoadImage(ID3D12Resource** Resource, ID3D12DescriptorHeap** DescriptorHeap, const std::string& Path, const D3D12_RESOURCE_STATES ResourceState = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE) { LoadImage(Resource, DescriptorHeap, std::wstring(Path.begin(), Path.end()), ResourceState); }
	
	virtual void CreateVertexBuffer() {}
	virtual void CreateIndexBuffer() {}
	virtual void CreateIndirectBuffer() {}
	virtual void CreateConstantBuffer();
	virtual void CreateConstantBufferDescriptorHeap(const UINT Size);
	virtual void CreateUnorderedAccessTexture();

	virtual void CreateViewport(const FLOAT Width, const FLOAT Height, const FLOAT MinDepth = 0.0f, const FLOAT MaxDepth = 1.0f);
	virtual void CreateViewportTopFront(const FLOAT Width, const FLOAT Height) { CreateViewport(Width, Height, 0.0f, 0.0f); }
	
	virtual void CreateDescriptorRanges(std::vector<D3D12_DESCRIPTOR_RANGE>& DescriptorRanges) const {}
	virtual void CreateRootParameters(std::vector<D3D12_ROOT_PARAMETER>& RootParameters, const std::vector<D3D12_DESCRIPTOR_RANGE>& DescriptorRanges) const {}
	virtual void CreateStaticSamplerDescs(std::vector<D3D12_STATIC_SAMPLER_DESC>& StaticSamplerDescs) const {}
	virtual void CreateRootSignature();

	virtual void CreateShader(std::vector<Microsoft::WRL::ComPtr<ID3DBlob>>& ShaderBlobs, std::array<D3D12_SHADER_BYTECODE, 5>& ShaderBytecodes) const {}
	virtual void CreateInputLayout(std::vector<D3D12_INPUT_ELEMENT_DESC>& InputElementDescs, const UINT InputSlot = 0) const {}
	virtual void CreatePipelineState() {}
	virtual void CreateGraphicsPipelineState();
	virtual void CreateComputePipelineState();

	virtual void CreateTexture() {}
	virtual void CreateSampler(const D3D12_SHADER_VISIBILITY ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL, const FLOAT MaxLOD = (std::numeric_limits<FLOAT>::max)()) {}

	virtual void ClearColor(ID3D12GraphicsCommandList* CommandList, const D3D12_CPU_DESCRIPTOR_HANDLE& DescriptorHandle, const DirectX::XMVECTORF32& Color);
	virtual void PopulateCommandList(ID3D12GraphicsCommandList* CommandList, ID3D12Resource* SwapChainResource, const D3D12_CPU_DESCRIPTOR_HANDLE& DescriptorHandle);
	virtual void PopulateCommandList(ID3D12GraphicsCommandList* CommandList, ID3D12Resource* SwapChainResource, const D3D12_CPU_DESCRIPTOR_HANDLE& DescriptorHandle, const DirectX::XMVECTORF32& Color);

	virtual void Draw();
	virtual void Present();
	virtual void WaitForFence();

protected:
	Microsoft::WRL::ComPtr<ID3D12Device> Device;
	std::vector<DXGI_SAMPLE_DESC> SampleDescs;
	Microsoft::WRL::ComPtr<ID3D12CommandQueue> CommandQueue;

	Microsoft::WRL::ComPtr<ID3D12Fence> Fence;
	UINT64 FenceValue = 0;

	std::vector<Microsoft::WRL::ComPtr<ID3D12CommandAllocator>> CommandAllocators;
	std::vector<Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>> GraphicsCommandLists;

	Microsoft::WRL::ComPtr<IDXGISwapChain3> SwapChain;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> SwapChainDescriptorHeap;
	UINT CurrentBackBufferIndex = 0xffffffff;
	std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> SwapChainResources;

	Microsoft::WRL::ComPtr<ID3D12Resource> DepthStencilResource;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> DepthStencilDescriptorHeap;

	Microsoft::WRL::ComPtr<ID3D12Resource> ImageResource;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> ImageDescriptorHeap;
	D3D12_STATIC_SAMPLER_DESC StaticSamplerDesc;

	Microsoft::WRL::ComPtr<ID3D12RootSignature> RootSignature;

	Microsoft::WRL::ComPtr<ID3D12PipelineState> PipelineState;

	Microsoft::WRL::ComPtr<ID3D12Resource> VertexBufferResource;
	std::vector<D3D12_VERTEX_BUFFER_VIEW> VertexBufferViews;

	Microsoft::WRL::ComPtr<ID3D12Resource> IndexBufferResource;
	D3D12_INDEX_BUFFER_VIEW IndexBufferView;
	UINT IndexCount = 0;

	Microsoft::WRL::ComPtr<ID3D12Resource> IndirectBufferResource;
	Microsoft::WRL::ComPtr<ID3D12CommandSignature> IndirectCommandSignature;

	//!< 現状1つのみ、配列にする #DX_TODO
	Microsoft::WRL::ComPtr<ID3D12Resource> ConstantBufferResource;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> ConstantBufferDescriptorHeap;

	Microsoft::WRL::ComPtr<ID3D12Resource> UnorderedAccessTextureResource;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> UnorderedAccessTextureDescriptorHeap;

	std::vector<D3D12_VIEWPORT> Viewports;
	std::vector<D3D12_RECT> ScissorRects;

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
};
