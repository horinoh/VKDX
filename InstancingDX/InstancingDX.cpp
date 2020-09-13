// InstancingDX.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "InstancingDX.h"

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
    LoadStringW(hInstance, IDC_INSTANCINGDX, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_INSTANCINGDX));

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
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_INSTANCINGDX));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_INSTANCINGDX);
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
			Inst = new InstancingDX();
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
void InstancingDX::CreateVertexBuffer()
{
	VertexBuffers.push_back(VertexBuffer());
	{
		const std::array Vertices = { 
			Vertex_PositionColor({ .Position = { 0.0f, 0.5f, 0.0f }, .Color = { 1.0f, 0.0f, 0.0f, 1.0f } }),
			Vertex_PositionColor({ .Position = { -0.5f, -0.5f, 0.0f }, .Color = { 0.0f, 1.0f, 0.0f, 1.0f } }),
			Vertex_PositionColor({ .Position = { 0.5f, -0.5f, 0.0f }, .Color = { 0.0f, 0.0f, 1.0f, 1.0f } }),
		};
		const auto Stride = sizeof(Vertices[0]);
		const auto Size = static_cast<UINT32>(Stride * size(Vertices));
		CreateAndCopyToDefaultResource(VertexBuffers.back().Resource, COM_PTR_GET(CommandAllocators[0]), COM_PTR_GET(GraphicsCommandLists[0]), Size, data(Vertices));
		VertexBuffers.back().View = { VertexBuffers.back().Resource->GetGPUVirtualAddress(), Size, Stride };
	}

	VertexBuffers.push_back(VertexBuffer());
	{
		const std::array Instances = { 
			Instance_OffsetXY({ { -0.5f, -0.5f } }),
			Instance_OffsetXY({ { -0.25f, -0.25f } }),
			Instance_OffsetXY({ { 0.0f, 0.0f } }),
			Instance_OffsetXY({ { 0.25f, 0.25f } }),
			Instance_OffsetXY({ { 0.5f, 0.5f } }),
		};
		InstanceCount = static_cast<UINT>(size(Instances));
		const auto Stride = sizeof(Instances[0]);
		const auto Size = static_cast<UINT32>(Stride * InstanceCount);
		CreateAndCopyToDefaultResource(VertexBuffers.back().Resource, COM_PTR_GET(CommandAllocators[0]), COM_PTR_GET(GraphicsCommandLists[0]), Size, data(Instances));
		VertexBuffers.back().View = { VertexBuffers.back().Resource->GetGPUVirtualAddress(), Size, Stride };
	}

	LOG_OK();
}
void InstancingDX::CreateIndexBuffer()
{
	IndexBuffers.push_back(IndexBuffer());

	const std::array<UINT32, 3> Indices = { 0, 1, 2 };
	IndexCount = static_cast<UINT32>(size(Indices));
	const auto Stride = sizeof(Indices[0]);
	const auto Size = static_cast<UINT32>(Stride * IndexCount);
	CreateAndCopyToDefaultResource(IndexBuffers.back().Resource, COM_PTR_GET(CommandAllocators[0]), COM_PTR_GET(GraphicsCommandLists[0]), Size, data(Indices));
	IndexBuffers.back().View = { IndexBuffers.back().Resource->GetGPUVirtualAddress(), Size, DXGI_FORMAT_R32_UINT };

	LOG_OK();
}

void InstancingDX::CreateIndirectBuffer()
{
	IndirectBuffers.push_back(IndirectBuffer());

	const D3D12_DRAW_INDEXED_ARGUMENTS Source = { .IndexCountPerInstance = IndexCount, .InstanceCount = InstanceCount,.StartIndexLocation =  0, .BaseVertexLocation = 0, .StartInstanceLocation = 0 };
	const auto Stride = sizeof(Source);
	const auto Size = static_cast<UINT32>(Stride * 1);
	CreateAndCopyToDefaultResource(IndirectBuffers.back().Resource, COM_PTR_GET(CommandAllocators[0]), COM_PTR_GET(GraphicsCommandLists[0]), Size, &Source);

	const std::array IADs = {
		D3D12_INDIRECT_ARGUMENT_DESC({ .Type = D3D12_INDIRECT_ARGUMENT_TYPE_DRAW_INDEXED }),
	};
	const D3D12_COMMAND_SIGNATURE_DESC CSD = {
		.ByteStride = Stride,
		.NumArgumentDescs = static_cast<const UINT>(size(IADs)), .pArgumentDescs = data(IADs),
		.NodeMask = 0
	};
	Device->CreateCommandSignature(&CSD, nullptr, COM_PTR_UUIDOF_PUTVOID(IndirectBuffers.back().CommandSignature));
}

void InstancingDX::PopulateCommandList(const size_t i)
{
	const auto PS = COM_PTR_GET(PipelineStates[0]);

	const auto BCL = COM_PTR_GET(BundleGraphicsCommandLists[i]);
	const auto BCA = COM_PTR_GET(BundleCommandAllocators[0]);
	VERIFY_SUCCEEDED(BCL->Reset(BCA, PS));
	{
		const auto VBV0 = VertexBuffers[0].View;
		const auto VBV1 = VertexBuffers[1].View;
		const auto IBV = IndexBuffers[0].View;
		const auto ICS = COM_PTR_GET(IndirectBuffers[0].CommandSignature);
		const auto IBR = COM_PTR_GET(IndirectBuffers[0].Resource);

		BCL->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
		const std::array VBVs = { VBV0, VBV1 };
		BCL->IASetVertexBuffers(0, static_cast<UINT>(size(VBVs)), data(VBVs));
		BCL->IASetIndexBuffer(&IBV);
		BCL->ExecuteIndirect(ICS, 1, IBR, 0, nullptr, 0);
	}
	VERIFY_SUCCEEDED(BCL->Close());

	const auto CL = COM_PTR_GET(GraphicsCommandLists[i]);
	const auto CA = COM_PTR_GET(CommandAllocators[0]);
	VERIFY_SUCCEEDED(CL->Reset(CA, PS));
	{
		const auto RS = COM_PTR_GET(RootSignatures[0]);
		const auto SCR = COM_PTR_GET(SwapChainResources[i]);

		CL->SetGraphicsRootSignature(RS);

		CL->RSSetViewports(static_cast<UINT>(size(Viewports)), data(Viewports));
		CL->RSSetScissorRects(static_cast<UINT>(size(ScissorRects)), data(ScissorRects));

		ResourceBarrier(CL, SCR, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
		{
			auto CDH = SwapChainDescriptorHeap->GetCPUDescriptorHandleForHeapStart(); CDH.ptr += i * Device->GetDescriptorHandleIncrementSize(SwapChainDescriptorHeap->GetDesc().Type);

			const std::array<D3D12_RECT, 0> Rs = {};
			CL->ClearRenderTargetView(CDH, DirectX::Colors::SkyBlue, static_cast<UINT>(Rs.size()), Rs.data());

			const std::array RTDHs = { CDH };
			CL->OMSetRenderTargets(static_cast<UINT>(size(RTDHs)), data(RTDHs), FALSE, nullptr);

			CL->ExecuteBundle(BCL);
		}
		ResourceBarrier(CL, SCR, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
	}
	VERIFY_SUCCEEDED(CL->Close());
}
#pragma endregion