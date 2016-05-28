#pragma once

#include <wrl.h>

#include <d3d12.h>
#include <d3dcompiler.h>
#include <DXGI1_4.h>

#include <DirectXMath.h>
#include <DirectXPackedVector.h>
#include <DirectXColors.h>

//!< https://github.com/Microsoft/DirectX-Graphics-Samples/blob/master/Samples/Desktop/D3D12HelloWorld/src/HelloTriangle/d3dx12.h 
#include "d3dx12.h"

#include "Win.h"

#ifndef VERIFY_SUCCEEDED
//#define VERIFY_SUCCEEDED(hr) VERIFY(SUCCEEDED(hr))
#define VERIFY_SUCCEEDED(hr) if(FAILED(hr)) { throw std::runtime_error("VERIFY_SUCCEEDED failed"); }
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

protected:
	virtual void CreateDevice(HWND hWnd, const DXGI_FORMAT ColorFormat);
	virtual void EnumAdapter(IDXGIFactory4* Factory);
	virtual void EnumOutput(IDXGIAdapter* Adapter);
	virtual void GetDisplayModeList(IDXGIOutput* Output, const DXGI_FORMAT Format);
	virtual void CheckFeature();

	virtual void CreateCommandQueue();

	virtual void CreateCommandAllocator();
	virtual void CreateCommandList(ID3D12CommandAllocator* CommandAllocator);

	virtual void CreateFence();

	virtual void CreateSwapChain(HWND hWnd, const DXGI_FORMAT ColorFormat, const UINT BufferCount = 2);
	virtual void ResizeSwapChain();

	virtual void CreateDepthStencil();
	virtual void ResizeDepthStencil(const DXGI_FORMAT DepthFormat = DXGI_FORMAT_D32_FLOAT_S8X24_UINT);

	virtual void CreateShader();
	virtual void CreateShader_VSPS();
	virtual void CreateRootSignature();

	virtual void CreateInputLayout();
	virtual void CreateInputLayout_PositionColor();
	virtual void CreateViewport();
	virtual void CreateGraphicsPipelineState();

	virtual void CreateVertexBuffer(ID3D12CommandAllocator* CommandAllocator, ID3D12GraphicsCommandList* CommandList);
	virtual void CreateIndexBuffer(ID3D12CommandAllocator* CommandAllocator, ID3D12GraphicsCommandList* CommandList);
	virtual void CreateConstantBuffer();

	virtual void Clear(ID3D12GraphicsCommandList* GraphicsCommandList);
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

	std::vector<Microsoft::WRL::ComPtr<ID3DBlob>> BlobVSs;
	std::vector<Microsoft::WRL::ComPtr<ID3DBlob>> BlobPSs;

	Microsoft::WRL::ComPtr<ID3D12RootSignature> RootSignature;

	std::vector<D3D12_INPUT_ELEMENT_DESC> InputElementDescs;
	D3D12_INPUT_LAYOUT_DESC InputLayoutDesc;

	std::vector<D3D12_VIEWPORT> Viewports;
	std::vector<D3D12_RECT> ScissorRects;

	Microsoft::WRL::ComPtr<ID3D12PipelineState> PipelineState;

	Microsoft::WRL::ComPtr<ID3D12Resource> VertexBufferResource;
	Microsoft::WRL::ComPtr<ID3D12Resource> VertexBufferUploadResource;
	D3D12_VERTEX_BUFFER_VIEW VertexBufferView;
	//Microsoft::WRL::ComPtr<ID3DBlob> VertexBufferBlob; //!< CPU 側にもコピーを持たせる場合? 多分いらない?

	Microsoft::WRL::ComPtr<ID3D12Resource> IndexBufferResource;
	Microsoft::WRL::ComPtr<ID3D12Resource> IndexBufferUploadResource;
	D3D12_INDEX_BUFFER_VIEW IndexBufferView;
	UINT IndexCount = 0;
	//Microsoft::WRL::ComPtr<ID3DBlob> IndexBufferBlob; //!< CPU 側にもコピーを持たせる場合? 多分いらない?

#pragma region ConstantBuffer
	Microsoft::WRL::ComPtr<ID3D12Resource> ConstantBufferResource;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> ConstantBufferDescriptorHeap;
#pragma endregion

	Microsoft::WRL::ComPtr<ID3D12Fence> Fence;
	UINT64 FenceValue;
};