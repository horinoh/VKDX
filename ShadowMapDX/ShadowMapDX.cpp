// ShadowMapDX.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "ShadowMapDX.h"

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
    LoadStringW(hInstance, IDC_SHADOWMAPDX, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_SHADOWMAPDX));

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
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_SHADOWMAPDX));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_SHADOWMAPDX);
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
			Inst = new ShadowMapDX();
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
		if (nullptr != Inst) {
			Inst->OnKeyDown(hWnd, hInst, wParam);
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
			Inst->OnPreDestroy();
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
void ShadowMapDX::PopulateCommandList(const size_t i)
{
	const auto PS0 = COM_PTR_GET(PipelineStates[0]);
	const auto PS1 = COM_PTR_GET(PipelineStates[1]);

	const auto BCA = COM_PTR_GET(BundleCommandAllocators[0]);
	//!< パス0 : バンドルコマンドリスト(シャドウキャスタ描画用)
	const auto BGCL0 = COM_PTR_GET(BundleCommandLists[i]);
	VERIFY_SUCCEEDED(BGCL0->Reset(BCA, PS0));
	{
		const auto IDBCS = COM_PTR_GET(IndirectBuffers[0].CommandSignature);
		const auto IDBR = COM_PTR_GET(IndirectBuffers[0].Resource);
		BGCL0->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_1_CONTROL_POINT_PATCHLIST);
		BGCL0->ExecuteIndirect(IDBCS, 1, IDBR, 0, nullptr, 0);
	}
	VERIFY_SUCCEEDED(BGCL0->Close());

	//!< パス1 : バンドルコマンドリスト(レンダーテクスチャ描画用、シャドウレシーバ描画用)
	const auto BGCL1 = COM_PTR_GET(BundleCommandLists[i + size(BundleCommandLists) / 2]); //!< オフセットさせる(ここでは2つのバンドルコマンドリストがぞれぞれスワップチェインイメージ数だけある)
	VERIFY_SUCCEEDED(BGCL1->Reset(BCA, PS1));
	{
		const auto IDBCS = COM_PTR_GET(IndirectBuffers[1].CommandSignature);
		const auto IDBR = COM_PTR_GET(IndirectBuffers[1].Resource);
#ifdef USE_SHADOWMAP_VISUALIZE
		BGCL1->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
#else
		BGCL1->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_1_CONTROL_POINT_PATCHLIST);
#endif
		BGCL1->ExecuteIndirect(IDBCS, 1, IDBR, 0, nullptr, 0);
	}
	VERIFY_SUCCEEDED(BGCL1->Close());

	const auto GCL = COM_PTR_GET(DirectCommandLists[i]);
	const auto CA = COM_PTR_GET(DirectCommandAllocators[0]);
	VERIFY_SUCCEEDED(GCL->Reset(CA, PS1));
	{
		const auto SCR = COM_PTR_GET(SwapchainBackBuffers[i].Resource);
		const auto IR = COM_PTR_GET(DepthTextures[0].Resource);

		//!< パス0 : (シャドウキャスタ描画用)
		{
			const auto& HandleDSV = DsvDescs[0].second;

			const std::array VPs = { D3D12_VIEWPORT({ .TopLeftX = 0.0f, .TopLeftY = 0.0f, .Width = static_cast<FLOAT>(ShadowMapExtentW), .Height = static_cast<FLOAT>(ShadowMapExtentH), .MinDepth = 0.0f, .MaxDepth = 1.0f }) };
			const std::array SCs = { D3D12_RECT({ .left = 0, .top = 0, .right = static_cast<LONG>(ShadowMapExtentW), .bottom = static_cast<LONG>(ShadowMapExtentH) }) };
			GCL->RSSetViewports(static_cast<UINT>(size(VPs)), data(VPs));
			GCL->RSSetScissorRects(static_cast<UINT>(size(SCs)), data(SCs));

			GCL->SetGraphicsRootSignature(COM_PTR_GET(RootSignatures[0]));

			{
				constexpr std::array<D3D12_RECT, 0> Rects = {};
				GCL->ClearDepthStencilView(HandleDSV[0], D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, static_cast<UINT>(size(Rects)), data(Rects)); //!< DSV(0)
			}
			{
				const std::array<D3D12_CPU_DESCRIPTOR_HANDLE, 0> RtvDHs = {};
				GCL->OMSetRenderTargets(static_cast<UINT>(size(RtvDHs)), data(RtvDHs), FALSE, &HandleDSV[0]); //!< DSV(0)
			}
			{
				const auto& Desc = CbvSrvUavDescs[0];
				const auto& Heap = Desc.first;
				const auto& Handle = Desc.second;

				const std::array DHs = { COM_PTR_GET(Heap) };
				GCL->SetDescriptorHeaps(static_cast<UINT>(size(DHs)), data(DHs));

				GCL->SetGraphicsRootDescriptorTable(0, Handle[0]); //!< CBV(0)
			}
			GCL->ExecuteBundle(BGCL0);
		}

		//!< リソースバリア
		{
			const std::array RBs = { 
				//!< PRESENT -> RENDER_TARGET
				D3D12_RESOURCE_BARRIER({ 
					.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION, 
					.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE,
					.Transition = D3D12_RESOURCE_TRANSITION_BARRIER({.pResource = SCR, .Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES, .StateBefore = D3D12_RESOURCE_STATE_PRESENT, .StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET })
				}),
				//!< DEPTH_WRITE -> PIXEL_SHADER_RESOURCE
				D3D12_RESOURCE_BARRIER({ 
					.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION,
					.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE, 
					.Transition = D3D12_RESOURCE_TRANSITION_BARRIER({.pResource = IR, .Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES, .StateBefore = D3D12_RESOURCE_STATE_DEPTH_WRITE, .StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE })
				}),
			};
			GCL->ResourceBarrier(static_cast<UINT>(size(RBs)), data(RBs));
		}

		//!< パス1 : (レンダーテクスチャ描画用、シャドウレシーバ描画用)
		{
			GCL->RSSetViewports(static_cast<UINT>(size(Viewports)), data(Viewports));
			GCL->RSSetScissorRects(static_cast<UINT>(size(ScissorRects)), data(ScissorRects));

			GCL->SetGraphicsRootSignature(COM_PTR_GET(RootSignatures[1]));

			auto ScCDH = SwapChainDescriptorHeap->GetCPUDescriptorHandleForHeapStart(); ScCDH.ptr += i * Device->GetDescriptorHandleIncrementSize(SwapChainDescriptorHeap->GetDesc().Type);			
#ifndef USE_SHADOWMAP_VISUALIZE
			const auto& DsvDH = DsvDescriptorHeaps[0];
			auto DsvCDH = DsvDH->GetCPUDescriptorHandleForHeapStart();
			DsvCDH.ptr += Device->GetDescriptorHandleIncrementSize(DsvDH->GetDesc().Type); 
			{
				constexpr std::array<D3D12_RECT, 0> Rects = {};
				GCL->ClearRenderTargetView(ScCDH, DirectX::Colors::SkyBlue, static_cast<UINT>(size(Rects)), data(Rects));
				GCL->ClearDepthStencilView(DsvCDH, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, static_cast<UINT>(size(Rects)), data(Rects)); //!< DSV(1)
			}
#endif
			{
				const std::array RtvDHs = { ScCDH };
#ifdef USE_SHADOWMAP_VISUALIZE
				GCL->OMSetRenderTargets(static_cast<UINT>(size(RtvDHs)), data(RtvDHs), FALSE, nullptr);
#else
				GCL->OMSetRenderTargets(static_cast<UINT>(size(RtvDHs)), data(RtvDHs), FALSE, &DsvCDH); //!< DSV(1)
#endif
			}
			{
				const auto& Desc = CbvSrvUavDescs[0];
				const auto& Heap = Desc.first;
				const auto& Handle = Desc.second;

				const std::array DHs = { COM_PTR_GET(Heap) };
				GCL->SetDescriptorHeaps(static_cast<UINT>(size(DHs)), data(DHs));

				DXGI_SWAP_CHAIN_DESC1 SCD;
				SwapChain->GetDesc1(&SCD);

#pragma region FRAME_OBJECT
				GCL->SetGraphicsRootDescriptorTable(0, Handle[i]); //!< CBV
#pragma endregion
				GCL->SetGraphicsRootDescriptorTable(0, Handle[SCD.BufferCount]); //!< SRV(1)

#ifndef USE_SHADOWMAP_VISUALIZE
#pragma region FRAME_OBJECT
				GCL->SetGraphicsRootDescriptorTable(1, Handle[SCD.BufferCount + 1 + i]); //!< CBV(2)
#pragma endregion
#endif
			}
			GCL->ExecuteBundle(BGCL1);
		}

		//!< リソースバリア
		{
			const std::array RBs = { 
				//!< RENDER_TARGET -> PRESENT
				D3D12_RESOURCE_BARRIER({ 
					.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION,
					.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE, 
					.Transition = D3D12_RESOURCE_TRANSITION_BARRIER({.pResource = SCR, .Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES, .StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET, .StateAfter = D3D12_RESOURCE_STATE_PRESENT })
				}),
				//!< PIXEL_SHADER_RESOURCE -> DEPTH_WRITE
				D3D12_RESOURCE_BARRIER({ 
					.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION, 
					.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE,
					.Transition = D3D12_RESOURCE_TRANSITION_BARRIER({.pResource = IR, .Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES, .StateBefore = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, .StateAfter = D3D12_RESOURCE_STATE_DEPTH_WRITE })
				}),
			};
			GCL->ResourceBarrier(static_cast<UINT>(size(RBs)), data(RBs));
		}
	}
	VERIFY_SUCCEEDED(GCL->Close());
}
#pragma endregion