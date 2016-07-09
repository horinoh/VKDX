#pragma once

#include <wrl.h>

#include <d3d12.h>
#include <d3dcompiler.h>
#include <DXGI1_4.h>

#include <DirectXMath.h>
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

class DX : public Win
{
private:
	using Super = Win;
public:
	DX();
	virtual ~DX();

	virtual void OnCreate(HWND hWnd, HINSTANCE hInstance) override;
	virtual void OnSize(HWND hWnd, HINSTANCE hInstance) override;
	virtual void OnTimer(HWND hWnd, HINSTANCE hInstance) override;
	virtual void OnPaint(HWND hWnd, HINSTANCE hInstance) override;
	virtual void OnDestroy(HWND hWnd, HINSTANCE hInstance) override;

	static std::string GetHRESULTString(const HRESULT Result);
	static std::wstring GetHRESULTStringW(const HRESULT Result);
	static std::string GetFormatString(const DXGI_FORMAT Format);

protected:
	virtual void CreateDevice(HWND hWnd, const DXGI_FORMAT ColorFormat);
	virtual void EnumAdapter(IDXGIFactory4* Factory);
	virtual void EnumOutput(IDXGIAdapter* Adapter);
	virtual void GetDisplayModeList(IDXGIOutput* Output, const DXGI_FORMAT Format);
	virtual void CheckFeatureLevel();
	virtual void CheckMultiSample(const DXGI_FORMAT Format);

	virtual void CreateCommandQueue();

	virtual void CreateCommandAllocator();
	virtual void CreateCommandList(ID3D12CommandAllocator* CommandAllocator);

	virtual void CreateFence();

	virtual void CreateSwapChain(HWND hWnd, const DXGI_FORMAT ColorFormat, const UINT BufferCount = 2);
	virtual void ResizeSwapChain();

	virtual void CreateDepthStencil();
	virtual void ResizeDepthStencil(const DXGI_FORMAT DepthFormat = DXGI_FORMAT_D32_FLOAT_S8X24_UINT);

	virtual void CreateShader();
	
	virtual void CreateRootSignature();

	virtual void CreateInputLayout();
	
	virtual void CreateViewport();

	virtual void CreatePipelineState() { CreateGraphicsPipelineState(); }
	virtual void CreateGraphicsPipelineState();
	virtual void CreateComputePipelineState();

	virtual void CreateBuffer(ID3D12CommandAllocator* CommandAllocator, ID3D12GraphicsCommandList* CommandList, ID3D12Resource** Resource, const void* Source, const size_t Size);
	virtual void CreateVertexBuffer(ID3D12CommandAllocator* CommandAllocator, ID3D12GraphicsCommandList* CommandList);
	virtual void CreateIndexBuffer(ID3D12CommandAllocator* CommandAllocator, ID3D12GraphicsCommandList* CommandList);
	virtual void CreateConstantBuffer();
	virtual void CreateUnorderedAccessTexture();

	virtual void Clear(ID3D12GraphicsCommandList* GraphicsCommandList) {}
	virtual void PopulateCommandList(ID3D12GraphicsCommandList* GraphicsCommandList);

	virtual void BarrierTransition(ID3D12GraphicsCommandList* CommandList, ID3D12Resource* Resource, const D3D12_RESOURCE_STATES Before, const D3D12_RESOURCE_STATES After);

	virtual void Draw();
	virtual void ExecuteCommandList(ID3D12GraphicsCommandList* GraphicsCommandList);
	virtual void Present();
	virtual void WaitForFence();

protected:
	Microsoft::WRL::ComPtr<ID3D12Device> Device;
	Microsoft::WRL::ComPtr<ID3D12CommandQueue> CommandQueue;

	std::vector<Microsoft::WRL::ComPtr<ID3D12CommandAllocator>> CommandAllocators;
	std::vector<Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>> GraphicsCommandLists;

	Microsoft::WRL::ComPtr<IDXGISwapChain3> SwapChain;
	//Microsoft::WRL::ComPtr<IDXGISwapChain> SwapChain;
	UINT CurrentBackBufferIndex;
	std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> SwapChainResources;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> SwapChainDescriptorHeap;

	Microsoft::WRL::ComPtr<ID3D12Resource> DepthStencilResource;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> DepthStencilDescriptorHeap;

	std::vector<Microsoft::WRL::ComPtr<ID3DBlob>> ShaderBlobs;

	Microsoft::WRL::ComPtr<ID3D12RootSignature> RootSignature;

	std::vector<D3D12_INPUT_ELEMENT_DESC> InputElementDescs;
	D3D12_INPUT_LAYOUT_DESC InputLayoutDesc;

	std::vector<D3D12_VIEWPORT> Viewports;
	std::vector<D3D12_RECT> ScissorRects;

	Microsoft::WRL::ComPtr<ID3D12PipelineState> PipelineState;

	Microsoft::WRL::ComPtr<ID3D12Resource> VertexBufferResource;
	D3D12_VERTEX_BUFFER_VIEW VertexBufferView;

	Microsoft::WRL::ComPtr<ID3D12Resource> IndexBufferResource;
	D3D12_INDEX_BUFFER_VIEW IndexBufferView;
	UINT IndexCount = 0;

	Microsoft::WRL::ComPtr<ID3D12Resource> ConstantBufferResource;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> ConstantBufferDescriptorHeap;

	Microsoft::WRL::ComPtr<ID3D12Resource> UnorderedAccessTextureResource;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> UnorderedAccessTextureDescriptorHeap;

	Microsoft::WRL::ComPtr<ID3D12Fence> Fence;
	UINT64 FenceValue;
};

class DXExt : public DX
{
private:
	using Super = DX;
public:
	virtual void CreateShader_VsPs();
	virtual void CreateShader_VsPsDsHsGs();
	virtual void CreateShader_Cs();

	virtual void CreateInputLayout_Position();
	virtual void CreateInputLayout_PositionColor();

	virtual void CreateGraphicsPipelineState_VsPs();
	virtual void CreateGraphicsPipelineState_VsPsDsHsGs();

	virtual void Clear(ID3D12GraphicsCommandList* GraphicsCommandList) override { Clear_Color(GraphicsCommandList); }
	virtual void Clear_Color(ID3D12GraphicsCommandList* GraphicsCommandList);
	virtual void Clear_Depth(ID3D12GraphicsCommandList* GraphicsCommandList);
};