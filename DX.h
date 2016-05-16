#pragma once

#include <wrl.h>

#include <d3d12.h>
#include <d3dcompiler.h>
#include <DXGI1_4.h>

#include <DirectXMath.h>
#include <DirectXPackedVector.h>

#include "Win.h"

#ifndef VERIFY_SUCCEEDED
#define VERIFY_SUCCEEDED(hr) VERIFY(SUCCEEDED(hr))
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
	virtual void CreateCommandQueue();

	virtual void CreateCommandList();

	virtual void CreateSwapChain(HWND hWnd, const DXGI_FORMAT ColorFormat, const UINT BufferCount = 2);
	virtual void CreateSwapChainView(const UINT BufferCount);

	virtual void CreateDepthStencil(const DXGI_FORMAT TyplessDepthFormat, const DXGI_FORMAT TypedDepthFormat);
	virtual void CreateDepthStencil(const DXGI_FORMAT TypedDepthFormat) { CreateDepthStencil(TypedDepthFormat, TypedDepthFormat); }
	virtual void CreateDepthStencilView(const DXGI_FORMAT TypedDepthFormat);
	virtual void CreateDepthStencilView();

	virtual void CreateShader();
	virtual void CreateRootSignature();

	virtual void CreateInputLayout();
	virtual void CreateViewport();
	virtual void CreatePipelineState();

	virtual void CreateVertexBuffer();
	virtual void CreateIndexBuffer();
	virtual void CreateConstantBuffer();

	virtual void CreateFence();

	virtual void Clear();
	virtual void BarrierRender();
	virtual void BarrierPresent();
	virtual void PopulateCommandList();

	virtual void Draw();
	virtual void ExecuteCommandList();
	virtual void Present();
	virtual void WaitForFence();

protected:
#pragma  region Device
	Microsoft::WRL::ComPtr<ID3D12Device> Device;
	Microsoft::WRL::ComPtr<ID3D12CommandQueue> CommandQueue;
#pragma endregion

#pragma region CommandList
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> CommandAllocator;
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> CommandList;
#pragma endregion

#pragma region SwapChain
	Microsoft::WRL::ComPtr<IDXGISwapChain3> SwapChain;
	//Microsoft::WRL::ComPtr<IDXGISwapChain> SwapChain;
	UINT CurrentBackBufferIndex;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> RenderTargetViewHeap;
	std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> RenderTargets;
#pragma endregion

#pragma region DepthStencil
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> DepthStencilViewHeap;
	Microsoft::WRL::ComPtr<ID3D12Resource> DepthStencil;
#pragma endregion

#pragma region Shader
	std::vector<Microsoft::WRL::ComPtr<ID3DBlob>> BlobVSs;
	std::vector<Microsoft::WRL::ComPtr<ID3DBlob>> BlobPSs;
#pragma endregion
#pragma region RootSignature
	Microsoft::WRL::ComPtr<ID3D12RootSignature> RootSignature;
#pragma endregion

#pragma region InputLayout
private:
	using Vertex = std::tuple<DirectX::XMFLOAT3, DirectX::XMFLOAT4>;
protected:
	std::vector<D3D12_INPUT_ELEMENT_DESC> InputElementDescs;
	D3D12_INPUT_LAYOUT_DESC InputLayoutDesc;
#pragma endregion

#pragma region Viewport
	std::vector<D3D12_VIEWPORT> Viewports;
	std::vector<D3D12_RECT> ScissorRects;
#pragma endregion

#pragma region PipelineState
	Microsoft::WRL::ComPtr<ID3D12PipelineState> PipelineState;
#pragma endregion

#pragma region VertexBuffer
	Microsoft::WRL::ComPtr<ID3D12Resource> VertexBuffer;
	D3D12_VERTEX_BUFFER_VIEW VertexBufferView;
#pragma endregion

#pragma region IndexBuffer
	Microsoft::WRL::ComPtr<ID3D12Resource> IndexBuffer;
	D3D12_INDEX_BUFFER_VIEW IndexBufferView;
	UINT IndexCount = 0;
#pragma endregion

#pragma region ConstantBuffer
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> ConstantBufferViewHeap;
	Microsoft::WRL::ComPtr<ID3D12Resource> ConstantBuffer;
#pragma endregion

	Microsoft::WRL::ComPtr<ID3D12Fence> Fence;
	UINT64 FenceValue;

	//UINT RTVDescriptorSize;
};