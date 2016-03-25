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

	virtual void OnInitialize(HWND hWnd, HINSTANCE hInstance) override;

	virtual void OnCreate(HWND hWnd) override;
	virtual void OnSize(HWND hWnd) override;
	virtual void OnTimer(HWND hWnd) override;
	virtual void OnPaint(HWND hWnd) override;
	virtual void OnDestroy(HWND hWnd) override;

protected:
	virtual void CreateDevice();

protected:
	Microsoft::WRL::ComPtr<ID3D12Device> Device;
};