// RenderTargetDX.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "RenderTargetDX.h"

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
    LoadStringW(hInstance, IDC_RENDERTARGETDX, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_RENDERTARGETDX));

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
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_RENDERTARGETDX));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_RENDERTARGETDX);
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
			Inst = new RenderTargetDX();
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
			[[maybe_unused]] HDC hdc = BeginPaint(hWnd, &ps);
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
			Inst->OnPreDestroy(hWnd, hInst);
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
void RenderTargetDX::PopulateCommandList(const size_t i)
{
	const auto BCA = COM_PTR_GET(BundleCommandAllocators[0]);

#pragma region PASS0
	//!< バンドルコマンドリスト(メッシュ描画用
	const auto PS0 = COM_PTR_GET(PipelineStates[0]);
	const auto BCL0 = COM_PTR_GET(BundleGraphicsCommandLists[i]);
	VERIFY_SUCCEEDED(BCL0->Reset(BCA, PS0));
	{
		BCL0->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_1_CONTROL_POINT_PATCHLIST);
		BCL0->ExecuteIndirect(COM_PTR_GET(IndirectBuffers[0].CommandSignature), 1, COM_PTR_GET(IndirectBuffers[0].Resource), 0, nullptr, 0);
	}
	VERIFY_SUCCEEDED(BCL0->Close());
#pragma endregion

#pragma region PASS1
	//!< レンダーテクスチャ描画用
	const auto PS1 = COM_PTR_GET(PipelineStates[1]);
	DXGI_SWAP_CHAIN_DESC1 SCD;
	SwapChain->GetDesc1(&SCD);
	const auto BCL1 = COM_PTR_GET(BundleGraphicsCommandLists[i + SCD.BufferCount]); //!< オフセットさせる(ここでは2つのバンドルコマンドリストがぞれぞれスワップチェインイメージ数だけある)
	VERIFY_SUCCEEDED(BCL1->Reset(BCA, PS1));
	{
		BCL1->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
		BCL1->ExecuteIndirect(COM_PTR_GET(IndirectBuffers[1].CommandSignature), 1, COM_PTR_GET(IndirectBuffers[1].Resource), 0, nullptr, 0);
	}
	VERIFY_SUCCEEDED(BCL1->Close());
#pragma endregion

	const auto CL = COM_PTR_GET(GraphicsCommandLists[i]);
	const auto CA = COM_PTR_GET(CommandAllocators[0]);
	VERIFY_SUCCEEDED(CL->Reset(CA, PS1));
	{
		const auto SCR = COM_PTR_GET(SwapChainResources[i]);
		const auto RT = COM_PTR_GET(RenderTextures.back().Resource);

		CL->RSSetViewports(static_cast<UINT>(size(Viewports)), data(Viewports));
		CL->RSSetScissorRects(static_cast<UINT>(size(ScissorRects)), data(ScissorRects));

#pragma region PASS0
		//!< メッシュ描画用
		{
			CL->SetGraphicsRootSignature(COM_PTR_GET(RootSignatures[0]));

			const auto RtvDH = RtvDescriptorHeaps[0];
			auto RtvCDH = RtvDH->GetCPUDescriptorHandleForHeapStart(); 
			constexpr std::array<D3D12_RECT, 0> Rects = {};
			CL->ClearRenderTargetView(RtvCDH, DirectX::Colors::SkyBlue, static_cast<UINT>(size(Rects)), data(Rects)); //RtvCDH.ptr += Device->GetDescriptorHandleIncrementSize(RtvDH->GetDesc().Type);
#ifdef USE_DEPTH
			const auto DsvDH = DsvDescriptorHeaps[0]->GetCPUDescriptorHandleForHeapStart(); 
			CL->ClearDepthStencilView(DsvDH, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, static_cast<UINT>(size(Rects)), data(Rects));

			const std::array RtvCDHs = { RtvCDH };
			CL->OMSetRenderTargets(static_cast<UINT>(size(RtvCDHs)), data(RtvCDHs), FALSE, &DsvDH);
#else			
			const std::array RtvCDHs = { RtvCDH };
			CL->OMSetRenderTargets(static_cast<UINT>(size(RtvCDHs)), data(RtvCDHs), FALSE, nullptr);
#endif

			CL->ExecuteBundle(BCL0);
		}
#pragma endregion

		//!< リソースバリア
		{
			//!< PRESENT -> RENDER_TARGET
			const D3D12_RESOURCE_TRANSITION_BARRIER RTB_SCR = {.pResource = SCR, .Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES, .StateBefore = D3D12_RESOURCE_STATE_PRESENT, .StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET };
			//!< RENDER_TARGET -> PIXEL_SHADER_RESOURCE
			const D3D12_RESOURCE_TRANSITION_BARRIER RTB_IR = {.pResource = RT, .Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES, .StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET, .StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE };
			const std::array RBs = { 
				D3D12_RESOURCE_BARRIER({ .Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION, .Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE, .Transition = RTB_SCR }),
				D3D12_RESOURCE_BARRIER({ .Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION, .Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE, .Transition = RTB_IR }),
			};
			CL->ResourceBarrier(static_cast<UINT>(size(RBs)), data(RBs));
		}

#pragma region PASS1
		//!< レンダーテクスチャ描画用
		{
			CL->SetGraphicsRootSignature(COM_PTR_GET(RootSignatures[1]));

			auto ScCDH = SwapChainDescriptorHeap->GetCPUDescriptorHandleForHeapStart(); ScCDH.ptr += i * Device->GetDescriptorHandleIncrementSize(SwapChainDescriptorHeap->GetDesc().Type);

			const std::array RtvCDHs = { ScCDH };
			CL->OMSetRenderTargets(static_cast<UINT>(size(RtvCDHs)), data(RtvCDHs), FALSE, nullptr);

			const auto& SrvDH = CbvSrvUavDescriptorHeaps[0];
			const std::array SrvDHs = { COM_PTR_GET(SrvDH) };
			CL->SetDescriptorHeaps(static_cast<UINT>(size(SrvDHs)), data(SrvDHs));
			auto SrvGDH = SrvDH->GetGPUDescriptorHandleForHeapStart();
			CL->SetGraphicsRootDescriptorTable(0, SrvGDH); SrvGDH.ptr += Device->GetDescriptorHandleIncrementSize(SrvDH->GetDesc().Type); //!< SRV

			CL->ExecuteBundle(BCL1);
		}

		//!< リソースバリア : D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE -> D3D12_RESOURCE_STATE_RENDER_TARGET
		{
			//!< RENDER_TARGET -> PRESENT
			const D3D12_RESOURCE_TRANSITION_BARRIER RTB_SCR = {.pResource = SCR, .Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES, .StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET, .StateAfter = D3D12_RESOURCE_STATE_PRESENT };
			//!< PIXEL_SHADER_RESOURCE -> RENDER_TARGET
			const D3D12_RESOURCE_TRANSITION_BARRIER RTB_IR = {.pResource = RT, .Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES, .StateBefore = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, .StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET };
			const std::array RBs = { 
				D3D12_RESOURCE_BARRIER({ .Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION, .Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE, .Transition = RTB_SCR }),
				D3D12_RESOURCE_BARRIER({ .Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION, .Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE, .Transition = RTB_IR }),
			};
			CL->ResourceBarrier(static_cast<UINT>(size(RBs)), data(RBs));
		}
#pragma endregion
	}
	VERIFY_SUCCEEDED(CL->Close());
}
#pragma endregion
