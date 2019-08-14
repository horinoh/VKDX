// TriangleDX.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "TriangleDX.h"

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
	LoadStringW(hInstance, IDC_TRIANGLEDX, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance (hInstance, nCmdShow))
	{
		return FALSE;
	}

	HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_TRIANGLEDX));

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
	wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_TRIANGLEDX));
	wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_TRIANGLEDX);
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
			Inst = new TriangleDX();
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
void TriangleDX::CreateVertexBuffer()
{
	VertexBufferResources.resize(1);

	const std::vector<Vertex_PositionColor> Vertices = {
		{ { 0.0f, 0.5f, 0.0f }, { 1.0f, 0.0f, 0.0f, 1.0f } }, //!< CT
		{ { -0.5f, -0.5f, 0.0f }, { 0.0f, 1.0f, 0.0f, 1.0f } }, //!< LB
		{ { 0.5f, -0.5f, 0.0f }, { 0.0f, 0.0f, 1.0f, 1.0f } }, //!< RB
	};
	const auto Stride = sizeof(Vertices[0]);
	const auto Size = static_cast<UINT32>(Stride * Vertices.size());

#ifdef USE_WINRT
	CreateBuffer(VertexBufferResources[0].put(), Size, Vertices.data(), CommandAllocators[0].get(), GraphicsCommandLists[0].get());
#elif defined(USE_WRL)
	CreateBuffer(VertexBufferResources[0].GetAddressOf(), Size, Vertices.data(), CommandAllocators[0].Get(), GraphicsCommandLists[0].Get());
#endif

	//!< DX�ł̓r���[���K�v Need view
	VertexBufferViews.push_back({ VertexBufferResources[0]->GetGPUVirtualAddress(), Size, Stride });

#ifdef _DEBUG
#ifdef USE_WINRT
	SetName(VertexBufferResources[0].get(), TEXT("MyVertexBuffer"));
#elif defined(USE_WRL)
	SetName(VertexBufferResources[0].Get(), TEXT("MyVertexBuffer"));
#endif
#endif

#ifdef DEBUG_STDOUT
	std::cout << "CreateVertexBuffer" << COUT_OK << std::endl << std::endl;
#endif
}
void TriangleDX::CreateIndexBuffer()
{
	IndexBufferResources.resize(1);

	const std::vector<UINT32> Indices = { 0, 1, 2 };
	//!< DrawInstanced() �������Ɏ��̂Ŋo���Ă����K�v������ Save this value because DrawInstanced() will use it
	IndexCount = static_cast<UINT32>(Indices.size());
	const auto Stride = sizeof(Indices[0]);
	const auto Size = static_cast<UINT32>(Stride * IndexCount);

#ifdef USE_WINRT
	CreateBuffer(IndexBufferResources[0].put(), Size, Indices.data(), CommandAllocators[0].get(), GraphicsCommandLists[0].get());
#elif defined(USE_WRL)
	CreateBuffer(IndexBufferResources[0].GetAddressOf(), Size, Indices.data(), CommandAllocators[0].Get(), GraphicsCommandLists[0].Get());
#endif

	//!< DX�ł̓r���[���K�v Need view
	IndexBufferView = { IndexBufferResources[0]->GetGPUVirtualAddress(), Size, DXGI_FORMAT_R32_UINT };

#ifdef _DEBUG
#ifdef USE_WINRT
	SetName(IndexBufferResources[0].get(), TEXT("MyIndexBuffer"));
#elif defined(USE_WRL)
	SetName(IndexBufferResources[0].Get(), TEXT("MyIndexBuffer"));
#endif
#endif

#ifdef DEBUG_STDOUT
	std::cout << "CreateIndexBuffer" << COUT_OK << std::endl << std::endl;
#endif
}
void TriangleDX::PopulateCommandList(const size_t i)
{
#ifdef USE_WINRT
	const auto CL = GraphicsCommandLists[i].get();
	const auto CA = CommandAllocators[0].get();
	const auto IBR = IndirectBufferResources[0].get();
#elif defined(USE_WRL)
	const auto CL = GraphicsCommandLists[i].Get();
	const auto CA = CommandAllocators[0].Get();
	const auto IBR = IndirectBufferResources[0].Get();
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
#if defined(_DEBUG) || defined(USE_PIX)
		//PIXBeginEvent(CL, PIX_COLOR(0, 255, 0), TEXT("Command Begin"));
		PIXScopedEvent(CL, PIX_COLOR(0, 255, 0), TEXT("Command Begin"));

		//PIXSetMarker(CL, PIX_COLOR(255, 0, 0), TEXT("Command"));
#endif

		//!< �r���[�|�[�g�A�V�U�[
		CL->RSSetViewports(static_cast<UINT>(Viewports.size()), Viewports.data());
		CL->RSSetScissorRects(static_cast<UINT>(ScissorRects.size()), ScissorRects.data());

		//!< �o���A
		ResourceBarrier(CL, SCR, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
		{
			//!< �N���A
			ClearColor(CL, SCHandle, DirectX::Colors::SkyBlue);

			//!< �����_�[�^�[�Q�b�g
			const std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> RTDescriptorHandles = { SCHandle };
			CL->OMSetRenderTargets(static_cast<UINT>(RTDescriptorHandles.size()), RTDescriptorHandles.data(), FALSE, nullptr);

			//!< ���[�g�V�O�j�`��
#ifdef USE_WINRT
			CL->SetGraphicsRootSignature(RootSignature.get());
#elif defined(USE_WRL)
			CL->SetGraphicsRootSignature(RootSignature.Get());
#endif

			//!< �C���v�b�g�A�Z���u���̃v���~�e�B�u�^�C�v
			CL->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

			//!< �o�[�e�b�N�X�o�b�t�@�A�C���f�b�N�X�o�b�t�@
			if (!VertexBufferViews.empty()) {
				CL->IASetVertexBuffers(0, static_cast<UINT>(VertexBufferViews.size()), VertexBufferViews.data());
				CL->IASetIndexBuffer(&IndexBufferView);
			}

			//!< �`��
#ifdef USE_WINRT
			CL->ExecuteIndirect(IndirectCommandSignature.get(), 1, IBR, 0, nullptr, 0);
#elif defined(USE_WRL)
			CL->ExecuteIndirect(IndirectCommandSignature.Get(), 1, IBR, 0, nullptr, 0);
#endif
		}
		ResourceBarrier(CL, SCR, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);

#if defined(_DEBUG) || defined(USE_PIX)
		//PIXEndEvent(CL);
#endif
	}
	VERIFY_SUCCEEDED(CL->Close());
}
#pragma endregion