#pragma once

#include <wrl.h>
#include <d3d12.h>
#include <DXGI1_4.h>
#include <DirectXMath.h>
#include <d3dcompiler.h>

#include "Win.h"

#define VERIFY_SUCCEEDED(hr) VERIFY(SUCCEEDED(hr))
#ifdef _DEBUG
#define SHADER_PATH L"..\\x64\\Debug\\"
#else
#define SHADER_PATH L"..\\x64\\Release\\"
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
	virtual void CreateDevice(HWND hWnd);
	virtual void CreateCommandQueue();

	virtual void CreateCommandList();

	virtual void CreateSwapChain(HWND hWnd, const UINT BufferCount = 2);
	virtual void CreateDepthStencil();

	virtual void CreateShader();
	virtual void CreateRootSignature();

	virtual void CreateInputLayout();
	virtual void CreateViewport();
	virtual void CreatePipelineState();

	virtual void CreateVertexBuffer();
	virtual void CreateIndexBuffer();
	virtual void CreateConstantBuffer();

	virtual void CreateFence();

	virtual void PopulateCommandList();
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
	Microsoft::WRL::ComPtr<IDXGISwapChain3> SwapChain3;
	UINT CurrentBackBufferIndex;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> RenderTargetViewHeap;
	Microsoft::WRL::ComPtr<ID3D12Resource> RenderTargets[2];
#pragma endregion

#pragma region DepthStencil
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> DepthStencilViewHeap;
	Microsoft::WRL::ComPtr<ID3D12Resource> DepthStencil;
#pragma endregion

#pragma region Shader
	Microsoft::WRL::ComPtr<ID3DBlob> BlobVS;
	std::vector<D3D12_SHADER_BYTECODE> ShaderBytecodesVSs;
	Microsoft::WRL::ComPtr<ID3DBlob> BlobPS;
	std::vector<D3D12_SHADER_BYTECODE> ShaderBytecodesPSs;
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
	HANDLE FenceEvent;

	//UINT RTVDescriptorSize;
};