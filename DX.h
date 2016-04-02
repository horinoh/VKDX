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
	virtual void CreateDevice(HWND hWnd);
	virtual void CreateCommandQueue();
	virtual void CreateSwapChain(HWND hWnd, const UINT BufferCount = 2);
	virtual void CreateRootSignature();
	virtual void CreateInputLayout();
	virtual void CreateShader();
	virtual void CreatePipelineState();
	virtual void CreateCommandList();
	virtual void CreateVertexBuffer();
	virtual void CreateIndexBuffer();
	virtual void CreateFence();

	virtual void PopulateCommandList();
	virtual void WaitForFence();

protected:
	Microsoft::WRL::ComPtr<ID3D12Device> Device;
	Microsoft::WRL::ComPtr<ID3D12CommandQueue> CommandQueue;

	Microsoft::WRL::ComPtr<IDXGISwapChain3> SwapChain3;
	UINT BackBufferIndex;

	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> RenderTargetViewHeap;
	Microsoft::WRL::ComPtr<ID3D12Resource> RenderTargets[2];

	Microsoft::WRL::ComPtr<ID3D12RootSignature> RootSignature;

	std::vector<D3D12_INPUT_LAYOUT_DESC> InputLayoutDescs;

	Microsoft::WRL::ComPtr<ID3DBlob> BlobVS;
	std::vector<D3D12_SHADER_BYTECODE> ShaderBytecodesVSs;
	Microsoft::WRL::ComPtr<ID3DBlob> BlobPS;
	std::vector<D3D12_SHADER_BYTECODE> ShaderBytecodesPSs;
	
	Microsoft::WRL::ComPtr<ID3D12PipelineState> PipelineState;

	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> CommandAllocator;
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> CommandList;

	Microsoft::WRL::ComPtr<ID3D12Resource> VertexBuffer;
	D3D12_VERTEX_BUFFER_VIEW VertexBufferView;
	
	Microsoft::WRL::ComPtr<ID3D12Resource> IndexBuffer;
	D3D12_VERTEX_BUFFER_VIEW IndexBufferView;

	Microsoft::WRL::ComPtr<ID3D12Fence> Fence;
	UINT64 FenceValue;
	HANDLE FenceEvent;

	//UINT RTVDescriptorSize;
};