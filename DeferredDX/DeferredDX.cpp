// DeferredDX.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "DeferredDX.h"

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
    LoadStringW(hInstance, IDC_DEFERREDDX, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_DEFERREDDX));

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
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_DEFERREDDX));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_DEFERREDDX);
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
//  PURPOSE: Processes messages for the main window.
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
			Inst = new DeferredDX();
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
void DeferredDX::PopulateCommandList(const size_t i)
{
	const auto PS0 = COM_PTR_GET(PipelineStates[0]);
	const auto PS1 = COM_PTR_GET(PipelineStates[1]);

	const auto BCA = COM_PTR_GET(BundleCommandAllocators[0]);
	//!< パス0 : バンドルコマンドリスト(メッシュ描画用)
	const auto BCL0 = COM_PTR_GET(BundleGraphicsCommandLists[i]);
	VERIFY_SUCCEEDED(BCL0->Reset(BCA, PS0));
	{
		const auto ICS = COM_PTR_GET(IndirectCommandSignatures[0]);
		const auto IBR = COM_PTR_GET(IndirectBufferResources[0]);
		BCL0->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_1_CONTROL_POINT_PATCHLIST);
		BCL0->ExecuteIndirect(ICS, 1, IBR, 0, nullptr, 0);
	}
	VERIFY_SUCCEEDED(BCL0->Close());

	//!< パス1 : バンドルコマンドリスト(レンダーテクスチャ描画用)
	const auto BCL1 = COM_PTR_GET(BundleGraphicsCommandLists[i + BundleGraphicsCommandLists.size() / 2]); //!< オフセットさせる(ここでは2つのバンドルコマンドリストがぞれぞれスワップチェインイメージ数だけある)
	VERIFY_SUCCEEDED(BCL1->Reset(BCA, PS1));
	{
		const auto ICS = COM_PTR_GET(IndirectCommandSignatures[1]);
		const auto IBR = COM_PTR_GET(IndirectBufferResources[1]);
		BCL1->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
		BCL1->ExecuteIndirect(ICS, 1, IBR, 0, nullptr, 0);
	}
	VERIFY_SUCCEEDED(BCL1->Close());

	const auto CL = COM_PTR_GET(GraphicsCommandLists[i]);
	const auto CA = COM_PTR_GET(CommandAllocators[0]);
	VERIFY_SUCCEEDED(CL->Reset(CA, PS1));
	{
		const auto SCR = COM_PTR_GET(SwapChainResources[i]);
		const auto IR = COM_PTR_GET(ImageResources[0]);

		CL->RSSetViewports(static_cast<UINT>(Viewports.size()), Viewports.data());
		CL->RSSetScissorRects(static_cast<UINT>(ScissorRects.size()), ScissorRects.data());

		//!< パス0 : (メッシュ描画用)
		{
			CL->SetGraphicsRootSignature(COM_PTR_GET(RootSignatures[0]));

			const auto RtvDH = RtvDescriptorHeaps[0];
			auto RtvCDH = RtvDH->GetCPUDescriptorHandleForHeapStart();
			const std::array<D3D12_RECT, 0> Rects = {};
			CL->ClearRenderTargetView(RtvCDH, DirectX::Colors::SkyBlue, static_cast<UINT>(Rects.size()), Rects.data()); RtvCDH.ptr += Device->GetDescriptorHandleIncrementSize(RtvDH->GetDesc().Type);
#pragma region MRT
			CL->ClearRenderTargetView(RtvCDH, std::array<FLOAT, 4>({ 0.5f, 0.5f, 1.0f, 1.0f }).data(), static_cast<UINT>(Rects.size()), Rects.data()); RtvCDH.ptr += Device->GetDescriptorHandleIncrementSize(RtvDH->GetDesc().Type);
			CL->ClearRenderTargetView(RtvCDH, DirectX::Colors::White, static_cast<UINT>(Rects.size()), Rects.data()); RtvCDH.ptr += Device->GetDescriptorHandleIncrementSize(RtvDH->GetDesc().Type);
			CL->ClearRenderTargetView(RtvCDH, DirectX::Colors::SkyBlue, static_cast<UINT>(Rects.size()), Rects.data()); RtvCDH.ptr += Device->GetDescriptorHandleIncrementSize(RtvDH->GetDesc().Type);
#pragma endregion

			const auto DsvDH = DsvDescriptorHeaps[0]->GetCPUDescriptorHandleForHeapStart();
			CL->ClearDepthStencilView(DsvDH, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, static_cast<UINT>(Rects.size()), Rects.data());

			RtvCDH = RtvDH->GetCPUDescriptorHandleForHeapStart();
			const std::array<D3D12_CPU_DESCRIPTOR_HANDLE, 4> RtvDHs = {
				RtvCDH,
				RtvCDH.ptr += Device->GetDescriptorHandleIncrementSize(RtvDH->GetDesc().Type),
				RtvCDH.ptr += Device->GetDescriptorHandleIncrementSize(RtvDH->GetDesc().Type),
				RtvCDH.ptr += Device->GetDescriptorHandleIncrementSize(RtvDH->GetDesc().Type),
			};
			CL->OMSetRenderTargets(static_cast<UINT>(RtvDHs.size()), RtvDHs.data(), FALSE, &DsvDH);

			{
				const auto& DH = CbvSrvUavDescriptorHeaps[0];
				const std::array<ID3D12DescriptorHeap*, 1> DHs = { COM_PTR_GET(DH) };
				CL->SetDescriptorHeaps(static_cast<UINT>(DHs.size()), DHs.data());

				auto GDH = DH->GetGPUDescriptorHandleForHeapStart();
				CL->SetGraphicsRootDescriptorTable(0, GDH); GDH.ptr += Device->GetDescriptorHandleIncrementSize(DH->GetDesc().Type);
			}

			CL->ExecuteBundle(BCL0);
		}

		//!< リソースバリア : D3D12_RESOURCE_STATE_RENDER_TARGET -> D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE
		{
			const D3D12_RESOURCE_TRANSITION_BARRIER RTB_SCR = { SCR, D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET };
			const D3D12_RESOURCE_TRANSITION_BARRIER RTB_IR = { IR, D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE };
			const std::array<D3D12_RESOURCE_BARRIER, 2> RBs = { {
				{ D3D12_RESOURCE_BARRIER_TYPE_TRANSITION, D3D12_RESOURCE_BARRIER_FLAG_NONE, RTB_SCR },
				{ D3D12_RESOURCE_BARRIER_TYPE_TRANSITION, D3D12_RESOURCE_BARRIER_FLAG_NONE, RTB_IR },
			} };
			CL->ResourceBarrier(static_cast<UINT>(RBs.size()), RBs.data());
		}

		//!< パス1 : (レンダーテクスチャ描画用)
		{
			CL->SetGraphicsRootSignature(COM_PTR_GET(RootSignatures[1]));

			auto ScCDH = SwapChainDescriptorHeap->GetCPUDescriptorHandleForHeapStart(); ScCDH.ptr += i * Device->GetDescriptorHandleIncrementSize(SwapChainDescriptorHeap->GetDesc().Type);

			const std::array<D3D12_CPU_DESCRIPTOR_HANDLE, 1> RtvDHs = { ScCDH };
			CL->OMSetRenderTargets(static_cast<UINT>(RtvDHs.size()), RtvDHs.data(), FALSE, nullptr);

			const auto& SrvDH = CbvSrvUavDescriptorHeaps[1];
			const std::array<ID3D12DescriptorHeap*, 1> SrvDHs = { COM_PTR_GET(SrvDH) };
			CL->SetDescriptorHeaps(static_cast<UINT>(SrvDHs.size()), SrvDHs.data());
			auto SrvGDH = SrvDH->GetGPUDescriptorHandleForHeapStart();
			CL->SetGraphicsRootDescriptorTable(0, SrvGDH); SrvGDH.ptr += Device->GetDescriptorHandleIncrementSize(SrvDH->GetDesc().Type); //!< SRV

			CL->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
			CL->ExecuteBundle(BCL1);
		}

		//!< リソースバリア : D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE -> D3D12_RESOURCE_STATE_RENDER_TARGET
		{
			const D3D12_RESOURCE_TRANSITION_BARRIER RTB_SCR = { SCR, D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT };
			const D3D12_RESOURCE_TRANSITION_BARRIER RTB_IR = { IR, D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET };
			const std::array<D3D12_RESOURCE_BARRIER, 2> RBs = { {
				{ D3D12_RESOURCE_BARRIER_TYPE_TRANSITION, D3D12_RESOURCE_BARRIER_FLAG_NONE, RTB_SCR },
				{ D3D12_RESOURCE_BARRIER_TYPE_TRANSITION, D3D12_RESOURCE_BARRIER_FLAG_NONE, RTB_IR },
			} };
			CL->ResourceBarrier(static_cast<UINT>(RBs.size()), RBs.data());
		}
	}
	VERIFY_SUCCEEDED(CL->Close());
}
#pragma endregion