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

#include "Win.h"

#ifndef THROW_ON_FAILED
#define THROW_ON_FAILED(hr) if(FAILED(hr)) { throw std::runtime_error("VERIFY_SUCCEEDED failed : " + DX::GetHRESULTString(hr)); }
#endif
#ifndef MESSAGEBOX_ON_FAILED
#define MESSAGEBOX_ON_FAILED(hr) if(FAILED(hr)) { Win::ShowMessageBoxW(nullptr, DX::GetHRESULTStringW(hr)); }
#endif
#ifndef VERIFY_SUCCEEDED
#define VERIFY_SUCCEEDED(hr) THROW_ON_FAILED(hr)
//#define VERIFY_SUCCEEDED(hr) MESSAGEBOX_ON_FAILED(hr)
#endif

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
	virtual void OnTimer(HWND hWnd, HINSTANCE hInstance) override;
	virtual void OnPaint(HWND hWnd, HINSTANCE hInstance) override;
	virtual void OnDestroy(HWND hWnd, HINSTANCE hInstance) override;

	static std::string GetHRESULTString(const HRESULT Result);
	static std::wstring GetHRESULTStringW(const HRESULT Result);
	static std::string GetFormatString(const DXGI_FORMAT Format);

protected:
	virtual void CreateDevice(HWND hWnd);
	virtual HRESULT CreateMaxFeatureLevelDevice(IDXGIAdapter* Adapter);
	virtual void EnumAdapter(IDXGIFactory4* Factory);
	virtual void EnumOutput(IDXGIAdapter* Adapter);
	virtual void GetDisplayModeList(IDXGIOutput* Output, const DXGI_FORMAT Format);
	virtual void CheckFeatureLevel();
	virtual void CheckMultiSample(const DXGI_FORMAT Format);

	virtual void CreateCommandQueue();

	virtual void CreateCommandAllocator(const D3D12_COMMAND_LIST_TYPE CommandListType = D3D12_COMMAND_LIST_TYPE_DIRECT);
	virtual void CreateCommandList(ID3D12CommandAllocator* CommandAllocator, const D3D12_COMMAND_LIST_TYPE CommandListType = D3D12_COMMAND_LIST_TYPE_DIRECT);

	virtual void CreateFence();

	virtual void CreateSwapChain(HWND hWnd, const DXGI_FORMAT ColorFormat, const UINT Width, const UINT Height, const UINT BufferCount = 2);
	virtual void CreateSwapChainOfClientRect(HWND hWnd, const DXGI_FORMAT ColorFormat, const UINT BufferCount = 2) {
		CreateSwapChain(hWnd, ColorFormat, static_cast<UINT>(GetClientRectWidth()), static_cast<UINT>(GetClientRectHeight()), BufferCount);
	}
	virtual void CreateSwapChainDescriptorHeap();
	virtual void ResetSwapChainResource();
	virtual void CreateSwapChainResource();
	virtual void ResizeSwapChain(const UINT Width, const UINT Height);
	virtual void ResizeSwapChainToClientRect() { 
		ResizeSwapChain(static_cast<UINT>(GetClientRectWidth()), static_cast<UINT>(GetClientRectHeight())); 
	}

	virtual void CreateDepthStencilDescriptorHeap();
	virtual void ResetDepthStencilResource();
	virtual void CreateDepthStencilResource(const UINT Width, const UINT Height, const DXGI_FORMAT DepthFormat = DXGI_FORMAT_D32_FLOAT_S8X24_UINT);
	virtual void ResizeDepthStencil(const UINT Width, const UINT Height, const DXGI_FORMAT DepthFormat = DXGI_FORMAT_D32_FLOAT_S8X24_UINT);
	virtual void ResizeDepthStencilToClientRect(const DXGI_FORMAT DepthFormat = DXGI_FORMAT_D32_FLOAT_S8X24_UINT) {
		ResizeDepthStencil(static_cast<UINT>(GetClientRectWidth()), static_cast<UINT>(GetClientRectHeight()), DepthFormat);
	}

	virtual void CreateInputLayout();
	
	virtual void CreateDefaultBuffer(ID3D12CommandAllocator* CommandAllocator, ID3D12GraphicsCommandList* CommandList, ID3D12Resource** Resource, const size_t Size, const void* Source);
	virtual void CreateUploadBuffer(ID3D12Resource** Resource, const size_t Size, const void* Source);
	virtual void CreateVertexBuffer(ID3D12CommandAllocator* CommandAllocator, ID3D12GraphicsCommandList* CommandList);
	virtual void CreateIndexBuffer(ID3D12CommandAllocator* CommandAllocator, ID3D12GraphicsCommandList* CommandList);
	virtual void CreateConstantBuffer();
	virtual void CreateConstantBufferDescriptorHeap(const UINT Size);
	virtual void CreateUnorderedAccessTexture();

	virtual void CreateViewport(const FLOAT Width, const FLOAT Height, const FLOAT MinDepth = 0.0f, const FLOAT MaxDepth = 1.0f);
	virtual void CreateViewportTopFront(const FLOAT Width, const FLOAT Height) { CreateViewport(Width, Height, 0.0f, 0.0f); }
	
	virtual void CreateRootSignature();

	virtual void CreatePipelineState() { CreateGraphicsPipelineState(); }
	virtual void CreateGraphicsPipelineState();
	virtual void CreateComputePipelineState();

	virtual void PopulateCommandList(ID3D12GraphicsCommandList* CommandList, ID3D12CommandAllocator* CommandAllocator);

	virtual void BarrierTransition(ID3D12GraphicsCommandList* CommandList, ID3D12Resource* Resource, const D3D12_RESOURCE_STATES Before, const D3D12_RESOURCE_STATES After);

	virtual void Draw();
	virtual void Present();
	virtual void WaitForFence();

protected:
	Microsoft::WRL::ComPtr<ID3D12Device> Device;
	std::vector<DXGI_SAMPLE_DESC> SampleDescs;
	Microsoft::WRL::ComPtr<ID3D12CommandQueue> CommandQueue;

	std::vector<Microsoft::WRL::ComPtr<ID3D12CommandAllocator>> CommandAllocators;
	std::vector<Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>> GraphicsCommandLists;

	Microsoft::WRL::ComPtr<ID3D12Fence> Fence;
	UINT64 FenceValue;

	Microsoft::WRL::ComPtr<IDXGISwapChain3> SwapChain;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> SwapChainDescriptorHeap;
	UINT CurrentBackBufferIndex;
	std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> SwapChainResources;

	Microsoft::WRL::ComPtr<ID3D12Resource> DepthStencilResource;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> DepthStencilDescriptorHeap;

	Microsoft::WRL::ComPtr<ID3D12RootSignature> RootSignature;

	std::vector<D3D12_INPUT_ELEMENT_DESC> InputElementDescs;
	D3D12_INPUT_LAYOUT_DESC InputLayoutDesc;

	Microsoft::WRL::ComPtr<ID3D12PipelineState> PipelineState;

	Microsoft::WRL::ComPtr<ID3D12Resource> VertexBufferResource;
	std::vector<D3D12_VERTEX_BUFFER_VIEW> VertexBufferViews;

	Microsoft::WRL::ComPtr<ID3D12Resource> IndexBufferResource;
	D3D12_INDEX_BUFFER_VIEW IndexBufferView;
	UINT IndexCount = 0;

	//!< #TODO 現状1つのみ、配列にする
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
