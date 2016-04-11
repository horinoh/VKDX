#pragma once

#include <wrl.h>
#include <d3d12.h>
#include <DXGI1_4.h>
#include <DirectXMath.h>
#include <d3dcompiler.h>

#include "Win.h"

#define VERIFY_SUCCEEDED(hr) VERIFY(SUCCEEDED(hr))

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
#pragma region DeviceQueue
	virtual void CreateDevice(HWND hWnd);
	virtual void CreateCommandQueue();
#pragma endregion

#pragma region Swapchain
	virtual void CreateSwapChain(HWND hWnd, const UINT BufferCount = 2);
#pragma endregion

#pragma region PipelineLayout
	virtual void CreateRootSignature();
#pragma endregion

#pragma region Shader
	virtual void CreateShader();
#pragma endregion

#pragma region UniformBuffer
	virtual void CreateConstantBuffer();
#pragma endregion

#pragma region Viewport
	virtual void CreateViewport();
#pragma endregion

	virtual void CreatePipelineState();

	virtual void CreateCommandList();

#pragma region VertexInput
	virtual void CreateInputLayout();
#pragma endregion

#pragma region VertexBuffer
	virtual void CreateVertexBuffer();
	virtual void CreateIndexBuffer();
#pragma endregion

	virtual void CreateFence();

	virtual void PopulateCommandList();
	virtual void ExecuteCommandList();
	virtual void Present();
	virtual void WaitForFence();

protected:
#pragma  region DeviceQueue
	Microsoft::WRL::ComPtr<ID3D12Device> Device;
	Microsoft::WRL::ComPtr<ID3D12CommandQueue> CommandQueue;
#pragma endregion

#pragma region Swapchain
	Microsoft::WRL::ComPtr<IDXGISwapChain3> SwapChain3;
	UINT CurrentBackBufferIndex;
#pragma endregion

	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> RenderTargetViewHeap;
	Microsoft::WRL::ComPtr<ID3D12Resource> RenderTargets[2];

#pragma region PipelineLayout
	Microsoft::WRL::ComPtr<ID3D12RootSignature> RootSignature;
#pragma endregion

#pragma region Shader
	Microsoft::WRL::ComPtr<ID3DBlob> BlobVS;
	std::vector<D3D12_SHADER_BYTECODE> ShaderBytecodesVSs;
	Microsoft::WRL::ComPtr<ID3DBlob> BlobPS;
	std::vector<D3D12_SHADER_BYTECODE> ShaderBytecodesPSs;
#pragma endregion

#pragma region UniformBuffer
	//todo
#pragma endregion

	Microsoft::WRL::ComPtr<ID3D12PipelineState> PipelineState;

	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> CommandAllocator;
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> CommandList;

#pragma region Viewport
	std::vector<D3D12_VIEWPORT> Viewports;
	std::vector<D3D12_RECT> ScissorRects;
#pragma endregion

#pragma region VertexInput
	std::vector<D3D12_INPUT_LAYOUT_DESC> InputLayoutDescs;
#pragma endregion

#pragma region VertexBuffer
	Microsoft::WRL::ComPtr<ID3D12Resource> VertexBuffer;
	D3D12_VERTEX_BUFFER_VIEW VertexBufferView;	
	Microsoft::WRL::ComPtr<ID3D12Resource> IndexBuffer;
	D3D12_VERTEX_BUFFER_VIEW IndexBufferView;
#pragma endregion

	Microsoft::WRL::ComPtr<ID3D12Fence> Fence;
	UINT64 FenceValue;
	HANDLE FenceEvent;

	//UINT RTVDescriptorSize;
};