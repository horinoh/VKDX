// BillboardDX.cpp : Defines the entry point for the application.
//

#include "BillboardDX.h"

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
    LoadStringW(hInstance, IDC_BILLBOARDDX, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_BILLBOARDDX));

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
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_BILLBOARDDX));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_BILLBOARDDX);
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
			Inst = new BillboardDX();
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
            /*HDC hdc =*/BeginPaint(hWnd, &ps);
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
void BillboardDX::PopulateCommandList(const size_t i)
{
	const auto CL = COM_PTR_GET(GraphicsCommandLists[i]);
	const auto CA = COM_PTR_GET(CommandAllocators[0]);
#ifdef USE_BUNDLE
	const auto BCL = COM_PTR_GET(BundleGraphicsCommandLists[i]);
	const auto BCA = COM_PTR_GET(BundleCommandAllocators[0]); 
#endif

	const auto IBR = COM_PTR_GET(IndirectBufferResources[0]);

	const auto SCR = COM_PTR_GET(SwapChainResources[i]);
	const auto SCH = GetCPUDescriptorHandle(COM_PTR_GET(SwapChainDescriptorHeap), static_cast<UINT>(i)); 
#ifdef USE_DEPTH_STENCIL
	const auto  DSH = GetCPUDescriptorHandle(COM_PTR_GET(DepthStencilDescriptorHeap), 0);
#endif

	const auto PS = COM_PTR_GET(PipelineStates[0]);

	const auto RS = COM_PTR_GET(RootSignatures[0]);
	
	const auto ICS = COM_PTR_GET(IndirectCommandSignatures[0]);

#ifdef USE_BUNDLE
	VERIFY_SUCCEEDED(BCL->Reset(BCA, PS));
	{
		BCL->SetGraphicsRootSignature(RS);
		{
			const std::array<ID3D12DescriptorHeap*, 1> DHs = { COM_PTR_GET(ConstantBufferDescriptorHeap) };
			BCL->SetDescriptorHeaps(static_cast<UINT>(DHs.size()), DHs.data());
			BCL->SetGraphicsRootDescriptorTable(0, GetGPUDescriptorHandle(COM_PTR_GET(ConstantBufferDescriptorHeap), 0));
		}
		BCL->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_1_CONTROL_POINT_PATCHLIST);
		BCL->ExecuteIndirect(ICS, 1, IBR, 0, nullptr, 0);
	}
	VERIFY_SUCCEEDED(BCL->Close());
#endif

	VERIFY_SUCCEEDED(CL->Reset(CA, PS));
	{
		CL->RSSetViewports(static_cast<UINT>(Viewports.size()), Viewports.data());
		CL->RSSetScissorRects(static_cast<UINT>(ScissorRects.size()), ScissorRects.data());

		ResourceBarrier(CL, SCR, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
		{
			const std::array<D3D12_RECT, 0> Rs = {};
			CL->ClearRenderTargetView(SCH, DirectX::Colors::SkyBlue, static_cast<UINT>(Rs.size()), Rs.data());
#ifdef USE_DEPTH_STENCIL
			CL->ClearDepthStencilView(DSH, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
#endif

			const std::array<D3D12_CPU_DESCRIPTOR_HANDLE, 1> RTDHs = { SCH };
#ifdef USE_DEPTH_STENCIL
			CL->OMSetRenderTargets(static_cast<UINT>(RTDHs.size()), RTDHs.data(), FALSE, &DSH);
#else
			CL->OMSetRenderTargets(static_cast<UINT>(RTDHs.size()), RTDHs.data(), FALSE, nullptr);
#endif

#ifdef USE_BUNDLE
			CL->ExecuteBundle(BCL);
#else
			CL->SetGraphicsRootSignature(RS);
			//!< コンスタントバッファ
			{
				const std::array<ID3D12DescriptorHeap*, 1> DHs = { COM_PTR_GET(ConstantBufferDescriptorHeap) };
				CL->SetDescriptorHeaps(static_cast<UINT>(DHs.size()), DHs.data());

				CL->SetGraphicsRootDescriptorTable(0, GetGPUDescriptorHandle(COM_PTR_GET(ConstantBufferDescriptorHeap), 0));
			}
			CL->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_1_CONTROL_POINT_PATCHLIST);
			CL->ExecuteIndirect(ICS, 1, IBR, 0, nullptr, 0);
#endif
		}
		ResourceBarrier(CL, SCR, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
	}
	VERIFY_SUCCEEDED(CL->Close());
}
#pragma endregion

