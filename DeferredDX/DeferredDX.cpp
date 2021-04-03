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
void DeferredDX::PopulateCommandList(const size_t i)
{
	const auto BCA = COM_PTR_GET(BundleCommandAllocators[0]);

#pragma region PASS0
	//!< メッシュ描画用
	const auto PS0 = COM_PTR_GET(PipelineStates[0]);
	const auto BGCL0 = COM_PTR_GET(BundleGraphicsCommandLists[i]);
	VERIFY_SUCCEEDED(BGCL0->Reset(BCA, PS0));
	{
		const auto IDBCS = COM_PTR_GET(IndirectBuffers[0].CommandSignature);
		const auto IDBR = COM_PTR_GET(IndirectBuffers[0].Resource);
	
		BGCL0->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_1_CONTROL_POINT_PATCHLIST);
		BGCL0->ExecuteIndirect(IDBCS, 1, IDBR, 0, nullptr, 0);
	}
	VERIFY_SUCCEEDED(BGCL0->Close());
#pragma endregion

#pragma region PASS1
	//!< レンダーテクスチャ描画用
	const auto PS1 = COM_PTR_GET(PipelineStates[1]);
	const auto BGCL1 = COM_PTR_GET(BundleGraphicsCommandLists[i + size(BundleGraphicsCommandLists) / 2]); //!< オフセットさせる(ここでは2つのバンドルコマンドリストがぞれぞれスワップチェインイメージ数だけある)
	VERIFY_SUCCEEDED(BGCL1->Reset(BCA, PS1));
	{
		const auto IDBCS = COM_PTR_GET(IndirectBuffers[1].CommandSignature);
		const auto IDBR = COM_PTR_GET(IndirectBuffers[1].Resource);
	
		BGCL1->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
		BGCL1->ExecuteIndirect(IDBCS, 1, IDBR, 0, nullptr, 0);
	}
	VERIFY_SUCCEEDED(BGCL1->Close());
#pragma endregion

	const auto GCL = COM_PTR_GET(GraphicsCommandLists[i]);
	const auto CA = COM_PTR_GET(CommandAllocators[0]);
	VERIFY_SUCCEEDED(GCL->Reset(CA, PS1));
	{
		const auto SCR = COM_PTR_GET(SwapChainResources[i]);
		const auto RT = COM_PTR_GET(RenderTextures[0].Resource);

		GCL->RSSetViewports(static_cast<UINT>(size(Viewports)), data(Viewports));
		GCL->RSSetScissorRects(static_cast<UINT>(size(ScissorRects)), data(ScissorRects));

#pragma region PASS0
		//!< メッシュ描画用
		{
			GCL->SetGraphicsRootSignature(COM_PTR_GET(RootSignatures[0]));

			const auto RtvDH = RtvDescriptorHeaps[0];
			auto RtvCDH = RtvDH->GetCPUDescriptorHandleForHeapStart();
			const auto DsvCDH = DsvDescriptorHeaps[0]->GetCPUDescriptorHandleForHeapStart();
			{
				constexpr std::array<D3D12_RECT, 0> Rects = {};
				GCL->ClearRenderTargetView(RtvCDH, DirectX::Colors::SkyBlue, static_cast<UINT>(size(Rects)), data(Rects)); RtvCDH.ptr += Device->GetDescriptorHandleIncrementSize(RtvDH->GetDesc().Type);
#pragma region MRT
				GCL->ClearRenderTargetView(RtvCDH, data(std::array<FLOAT, 4>({ 0.5f, 0.5f, 1.0f, 1.0f })), static_cast<UINT>(size(Rects)), data(Rects)); RtvCDH.ptr += Device->GetDescriptorHandleIncrementSize(RtvDH->GetDesc().Type);
				GCL->ClearRenderTargetView(RtvCDH, DirectX::Colors::Red, static_cast<UINT>(size(Rects)), data(Rects)); RtvCDH.ptr += Device->GetDescriptorHandleIncrementSize(RtvDH->GetDesc().Type);
				GCL->ClearRenderTargetView(RtvCDH, DirectX::Colors::SkyBlue, static_cast<UINT>(size(Rects)), data(Rects)); RtvCDH.ptr += Device->GetDescriptorHandleIncrementSize(RtvDH->GetDesc().Type);
#pragma endregion
				GCL->ClearDepthStencilView(DsvCDH, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, static_cast<UINT>(size(Rects)), data(Rects));
			}
			{
				RtvCDH = RtvDH->GetCPUDescriptorHandleForHeapStart();
#if 0
				const std::array RtvDHs = {
					RtvCDH,
					RtvCDH.ptr += Device->GetDescriptorHandleIncrementSize(RtvDH->GetDesc().Type),
					RtvCDH.ptr += Device->GetDescriptorHandleIncrementSize(RtvDH->GetDesc().Type),
					RtvCDH.ptr += Device->GetDescriptorHandleIncrementSize(RtvDH->GetDesc().Type),
				};
				GCL->OMSetRenderTargets(static_cast<UINT>(size(RtvDHs)), data(RtvDHs), FALSE, &DsvDH); //!< RTV, DSV
#else
				//!< 「連続している」場合は、「個数」と「先頭アドレス」を指定して「RTsSingleHandleToDescriptorRange==TRUE」で良い
				const std::array RtvDHs = { RtvCDH, };
				GCL->OMSetRenderTargets(4, data(RtvDHs), TRUE, &DsvCDH); //!< RTV, DSV
#endif
			}
			{
				const auto& DH = CbvSrvUavDescriptorHeaps[0];
				const std::array DHs = { COM_PTR_GET(DH) };
				GCL->SetDescriptorHeaps(static_cast<UINT>(size(DHs)), data(DHs));

				auto GDH = DH->GetGPUDescriptorHandleForHeapStart();
#pragma region FRAME_OBJECT
				GDH.ptr += Device->GetDescriptorHandleIncrementSize(DH->GetDesc().Type) * i;
				GCL->SetGraphicsRootDescriptorTable(0, GDH); //!< CBV
#pragma endregion
			}

			GCL->ExecuteBundle(BGCL0);
		}
#pragma endregion

		//!< リソースバリア
		{
			const std::array RBs = { 
				//!< スワップチェイン PRESENT -> RENDER_TARGET
				D3D12_RESOURCE_BARRIER({
					.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION,
					.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE, 
					.Transition = D3D12_RESOURCE_TRANSITION_BARRIER({.pResource = SCR, .Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES, .StateBefore = D3D12_RESOURCE_STATE_PRESENT, .StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET })
				}),
				//!< イメージ RENDER_TARGET -> PIXEL_SHADER_RESOURCE
				D3D12_RESOURCE_BARRIER({
					.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION,
					.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE,
					.Transition = D3D12_RESOURCE_TRANSITION_BARRIER({.pResource = RT, .Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES, .StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET, .StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE })
				}),
			};
			GCL->ResourceBarrier(static_cast<UINT>(size(RBs)), data(RBs));
		}

#pragma region PASS1
		//!< レンダーテクスチャ描画用
		{
			GCL->SetGraphicsRootSignature(COM_PTR_GET(RootSignatures[1]));
			{
				auto CDH = SwapChainDescriptorHeap->GetCPUDescriptorHandleForHeapStart(); CDH.ptr += i * Device->GetDescriptorHandleIncrementSize(SwapChainDescriptorHeap->GetDesc().Type);
				const std::array CDHs = { CDH };
				GCL->OMSetRenderTargets(static_cast<UINT>(size(CDHs)), data(CDHs), FALSE, nullptr); //!< RTV
			}
				
			{
				const auto& DH = CbvSrvUavDescriptorHeaps[1];
				const std::array DHs = { COM_PTR_GET(DH) };
				GCL->SetDescriptorHeaps(static_cast<UINT>(size(DHs)), data(DHs));
				auto GDH = DH->GetGPUDescriptorHandleForHeapStart();
				GCL->SetGraphicsRootDescriptorTable(0, GDH); //!< SRV
#pragma region FRAME_OBJECT
				//!< #DX_TODO
				//GDH.ptr += Device->GetDescriptorHandleIncrementSize(DH->GetDesc().Type) * 4;
				//GDH.ptr += Device->GetDescriptorHandleIncrementSize(DH->GetDesc().Type) * i;
				//GCL->SetGraphicsRootDescriptorTable(0, GDH); //!< CBV
#pragma endregion
			}

			GCL->ExecuteBundle(BGCL1);
		}
#pragma endregion

		//!< リソースバリア : D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE -> D3D12_RESOURCE_STATE_RENDER_TARGET
		{
			const std::array RBs = {
				//!< スワップチェイン RENDER_TARGET -> PRESENT
				D3D12_RESOURCE_BARRIER({
					.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION,
					.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE, 
					.Transition = D3D12_RESOURCE_TRANSITION_BARRIER({ .pResource = SCR, .Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES, .StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET, .StateAfter = D3D12_RESOURCE_STATE_PRESENT })
				}),
				//!< イメージ PIXEL_SHADER_RESOURCE -> RENDER_TARGET
				D3D12_RESOURCE_BARRIER({ 
					.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION, 
					.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE,
					.Transition = D3D12_RESOURCE_TRANSITION_BARRIER({ .pResource = RT, .Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES, .StateBefore = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, .StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET })
				}),
			};
			GCL->ResourceBarrier(static_cast<UINT>(size(RBs)), data(RBs));
		}
	}
	VERIFY_SUCCEEDED(GCL->Close());
}
#pragma endregion