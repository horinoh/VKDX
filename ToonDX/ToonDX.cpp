// ToonDX.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "ToonDX.h"

#pragma region Code
DX* Inst = nullptr;
#pragma endregion

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Place code here.

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_TOONDX, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_TOONDX));

    MSG msg;

    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_TOONDX));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_TOONDX);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Store instance handle in our global variable

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Parse the menu selections:
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
#pragma region Code
	case WM_CREATE:
		if (nullptr == Inst) {
			Inst = new ToonDX();
		}
		if (nullptr != Inst) {
			try {
				Inst->OnCreate(hWnd, hInst, szTitle);
			}
			catch (std::exception& e) {
				std::cerr << e.what() << std::endl;
			}
		}
		break;
	case WM_TIMER:
		if (nullptr != Inst) {
			Inst->OnTimer(hWnd, hInst);
		}
		break;
	case WM_SIZE:
		if (nullptr != Inst) {
			Inst->OnSize(hWnd, hInst);
		}
		break;
	case WM_EXITSIZEMOVE:
		if (nullptr != Inst) {
			Inst->OnExitSizeMove(hWnd, hInst);
		}
		break;
	case WM_KEYDOWN:
		if (VK_ESCAPE == wParam) {
			SendMessage(hWnd, WM_DESTROY, 0, 0);
		}
		break;
#pragma endregion
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: Add any drawing code that uses hdc here...
#pragma region Code
			if (nullptr != Inst) {
				Inst->OnPaint(hWnd, hInst);
			}
#pragma endregion
            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
#pragma region Code
		if (nullptr != Inst) {
			Inst->OnDestroy(hWnd, hInst);
		}
		SAFE_DELETE(Inst);
#pragma endregion
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}

#pragma region Code
void ToonDX::PopulateCommandList(const size_t i)
{
#ifdef USE_WINRT
	const auto CL = GraphicsCommandLists[i].get();
	const auto CA = CommandAllocators[0].get();
#elif defined(USE_WRL)
	const auto CL = GraphicsCommandLists[i].Get();
	const auto CA = CommandAllocators[0].Get();
#endif

#ifdef USE_WINRT
	const auto SCR = SwapChainResources[i].get();
	const auto SCHandle = GetCPUDescriptorHandle(SwapChainDescriptorHeap.get(), D3D12_DESCRIPTOR_HEAP_TYPE_RTV, static_cast<UINT>(i));
#elif defined(USE_WRL)
	const auto SCR = SwapChainResources[i].Get();
	const auto SCHandle = GetCPUDescriptorHandle(SwapChainDescriptorHeap.Get(), D3D12_DESCRIPTOR_HEAP_TYPE_RTV, static_cast<UINT>(i));
#endif
	
#ifdef USE_WINRT
	VERIFY_SUCCEEDED(CL->Reset(CA, PipelineState.get()));
#elif defined(USE_WRL)
	VERIFY_SUCCEEDED(CL->Reset(CA, PipelineState.Get()));
#endif
	{
		CL->RSSetViewports(static_cast<UINT>(Viewports.size()), Viewports.data());
		CL->RSSetScissorRects(static_cast<UINT>(ScissorRects.size()), ScissorRects.data());

		ResourceBarrier(CL, SCR, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
		{
			ClearColor(CL, SCHandle, DirectX::Colors::SkyBlue);

			const std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> RTDescriptorHandles = { SCHandle };
			CL->OMSetRenderTargets(static_cast<UINT>(RTDescriptorHandles.size()), RTDescriptorHandles.data(), FALSE, nullptr);

#ifdef USE_WINRT
			CL->SetGraphicsRootSignature(RootSignature.get());
#elif defined(USE_WRL)
			CL->SetGraphicsRootSignature(RootSignature.Get());
#endif

			//!< コンスタントバッファ
			{
#ifdef USE_WINRT
				const std::vector<ID3D12DescriptorHeap*> DH = { ConstantBufferDescriptorHeap.get() };
#elif defined(USE_WRL)
				const std::vector<ID3D12DescriptorHeap*> DH = { ConstantBufferDescriptorHeap.Get() };
#endif
				CL->SetDescriptorHeaps(static_cast<UINT>(DH.size()), DH.data());

#ifdef USE_WINRT
				auto CBHandle(GetGPUDescriptorHandle(ConstantBufferDescriptorHeap.get(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));
#elif defined(USE_WRL)
				auto CBHandle(GetGPUDescriptorHandle(ConstantBufferDescriptorHeap.Get(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));
#endif
				CL->SetGraphicsRootDescriptorTable(0, CBHandle);
			}

			CL->IASetPrimitiveTopology(GetPrimitiveTopology());

#ifdef USE_WINRT
			CL->ExecuteIndirect(IndirectCommandSignature.get(), 1, IndirectBufferResource.get(), 0, nullptr, 0);
#elif defined(USE_WRL)
			CL->ExecuteIndirect(IndirectCommandSignature.Get(), 1, IndirectBufferResource.Get(), 0, nullptr, 0);
#endif
		}
		ResourceBarrier(CL, SCR, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
	}
	VERIFY_SUCCEEDED(CL->Close());
}
#pragma endregion

