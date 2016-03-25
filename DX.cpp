#include "stdafx.h"

#include "DX.h"

//!< d3d12.lib が無いと言われる場合は、プロジェクトを右クリック - Retarget SDK Version で Windows10 にする
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")

DX::DX()
{

}
DX::~DX()
{

}

void DX::OnInitialize(HWND hWnd, HINSTANCE hInstance)
{
	CreateDevice();
}

void DX::OnCreate(HWND hWnd)
{
}
void DX::OnSize(HWND hWnd)
{
}
void DX::OnTimer(HWND hWnd)
{
}
void DX::OnPaint(HWND hWnd)
{
}
void DX::OnDestroy(HWND hWnd)
{
}

void DX::CreateDevice()
{
	using namespace Microsoft::WRL;

#ifdef _DEBUG
	ComPtr<ID3D12Debug> Debug;
	VERIFY_SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&Debug)));
	Debug->EnableDebugLayer();
#endif

	ComPtr<IDXGIFactory4> Factory;
	VERIFY_SUCCEEDED(CreateDXGIFactory1(IID_PPV_ARGS(&Factory))); {
		if (false/*WarpDevice*/) {
			ComPtr<IDXGIAdapter> Adapter;
			VERIFY_SUCCEEDED(Factory->EnumWarpAdapter(IID_PPV_ARGS(&Adapter)));
			VERIFY_SUCCEEDED(D3D12CreateDevice(Adapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&Device)));
		}
		else {
			ComPtr<IDXGIAdapter1> Adapter;
			for (UINT i = 0; DXGI_ERROR_NOT_FOUND != Factory->EnumAdapters1(i, &Adapter); ++i) {
				DXGI_ADAPTER_DESC1 AdapterDesc1;
				Adapter->GetDesc1(&AdapterDesc1);
				if (AdapterDesc1.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) {
					continue;
				}
				if (SUCCEEDED(D3D12CreateDevice(Adapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&Device)))) {
					break;
				}
			}
		}
	}
}
