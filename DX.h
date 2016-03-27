#pragma once

#include <wrl.h>
#include <d3d12.h>
#include <DXGI1_4.h>

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
	virtual void CreateRootSignature();
	virtual void CreatePipelineState();
	virtual void CreateCommandList();

protected:
	Microsoft::WRL::ComPtr<ID3D12Device> Device;
	Microsoft::WRL::ComPtr<ID3D12CommandQueue> CommandQueue;

	Microsoft::WRL::ComPtr<IDXGISwapChain3> SwapChain3;
	UINT BackBufferIndex;

	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> RenderTargetViewHeap;
	Microsoft::WRL::ComPtr<ID3D12Resource> RenderTargets[2];

	Microsoft::WRL::ComPtr<ID3D12RootSignature> RootSignature;

	Microsoft::WRL::ComPtr<ID3D12PipelineState> PipelineState;

	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> CommandAllocator;
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> CommandList;

	//UINT RTVDescriptorSize;
};