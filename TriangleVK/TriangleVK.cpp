// TriangleVK.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "TriangleVK.h"

#pragma region Code
VK* Inst = nullptr;
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
    LoadStringW(hInstance, IDC_TRIANGLEVK, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_TRIANGLEVK));

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
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_TRIANGLEVK));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_TRIANGLEVK);
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
			Inst = new TriangleVK();
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
void TriangleVK::CreateVertexBuffer()
{
	const std::vector<Vertex> Vertices = {
		{ { 0.0f, 0.5f, 0.0f }, { 1.0f, 0.0f, 0.0f, 1.0f } },
		{ { 0.5f, -0.5f, 0.0f }, { 0.0f, 1.0f, 0.0f, 1.0f } },
		{ { -0.5f, -0.5f, 0.0f }, { 0.0f, 0.0f, 1.0f, 1.0f } },
	};
	const auto Stride = sizeof(Vertices[0]);
	const auto Size = static_cast<VkDeviceSize>(Stride * Vertices.size());
	
	[&](VkBuffer* Buffer, VkDeviceMemory* DeviceMemory, const VkDeviceSize Size, const void* Data, const VkCommandBuffer CB) {
		VkBuffer StagingBuffer = VK_NULL_HANDLE;
		VkDeviceMemory StagingDeviceMemory = VK_NULL_HANDLE;
		{
			//!< ホストビジブルのバッファとメモリを作成、データをコピー Create host visible buffer and memory, and copy data
			CreateBuffer(&StagingBuffer, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, Size);
			CreateHostVisibleMemory(&StagingDeviceMemory, StagingBuffer);
			CopyToHostVisibleMemory(StagingDeviceMemory, Size, Data);
			BindDeviceMemory(StagingBuffer, StagingDeviceMemory);

			//!< デバイスローカルのバッファとメモリを作成 Create device local buffer and memory
			CreateBuffer(Buffer, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, Size);
			CreateDeviceLocalMemory(DeviceMemory, *Buffer);
			BindDeviceMemory(*Buffer, *DeviceMemory);

			//!< ホストビジブルからデバイスローカルへのコピーコマンドを発行 Submit copy command host visible to device local
			SubmitCopyBuffer(CB, StagingBuffer, *Buffer, VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT, VK_PIPELINE_STAGE_VERTEX_INPUT_BIT, Size);

		}
		if (VK_NULL_HANDLE != StagingDeviceMemory) {
			vkFreeMemory(Device, StagingDeviceMemory, GetAllocationCallbacks());
		}
		if (VK_NULL_HANDLE != StagingBuffer) {
			vkDestroyBuffer(Device, StagingBuffer, GetAllocationCallbacks());
		}
	}(&VertexBuffer, &VertexDeviceMemory, Size, Vertices.data(), CommandBuffers[0]);

	//!< ビューは必要ない No need view

#ifdef _DEBUG
	MarkerSetObjectName(Device, VertexBuffer, "MyVertexBuffer");
#endif

#ifdef _DEBUG
	std::cout << "CreateVertexBuffer" << COUT_OK << std::endl << std::endl;
#endif
}
void TriangleVK::CreateIndexBuffer()
{
	const std::vector<uint32_t> Indices = { 0, 1, 2 };

	//!< vkCmdDrawIndexed() が引数に取るので覚えておく必要がある Save this value because vkCmdDrawIndexed() will use it
	IndexCount = static_cast<uint32_t>(Indices.size());
	const auto Stride = sizeof(Indices[0]);
	const auto Size = static_cast<VkDeviceSize>(Stride * IndexCount);
	
	[&](VkBuffer* Buffer, VkDeviceMemory* DeviceMemory, const VkDeviceSize Size, const void* Data, const VkCommandBuffer CB) {
		VkBuffer StagingBuffer = VK_NULL_HANDLE;
		VkDeviceMemory StagingDeviceMemory = VK_NULL_HANDLE;
		{
			//!< ホストビジブルのバッファとメモリを作成、データをコピー Create host visible buffer and memory, and copy data
			CreateBuffer(&StagingBuffer, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, Size);
			CreateHostVisibleMemory(&StagingDeviceMemory, StagingBuffer);
			CopyToHostVisibleMemory(StagingDeviceMemory, Size, Data);
			BindDeviceMemory(StagingBuffer, StagingDeviceMemory);

			//!< デバイスローカルのバッファとメモリを作成 Create device local buffer and memory
			CreateBuffer(Buffer, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, Size);
			CreateDeviceLocalMemory(DeviceMemory, *Buffer);
			BindDeviceMemory(*Buffer, *DeviceMemory);

			//!< ホストビジブルからデバイスローカルへのコピーコマンドを発行 Submit copy command host visible to device local
			SubmitCopyBuffer(CB, StagingBuffer, *Buffer, VK_ACCESS_INDEX_READ_BIT, VK_PIPELINE_STAGE_VERTEX_INPUT_BIT, Size);
		}
		if (VK_NULL_HANDLE != StagingDeviceMemory) {
			vkFreeMemory(Device, StagingDeviceMemory, GetAllocationCallbacks());
		}
		if (VK_NULL_HANDLE != StagingBuffer) {
			vkDestroyBuffer(Device, StagingBuffer, GetAllocationCallbacks());
		}
	}(&IndexBuffer, &IndexDeviceMemory, Size, Indices.data(), CommandBuffers[0]);

	//!< ビューは必要ない No need view

#ifdef _DEBUG
	MarkerSetObjectName(Device, IndexBuffer, "MyIndexBuffer");
#endif

#ifdef _DEBUG
	std::cout << "CreateIndexBuffer" << COUT_OK << std::endl << std::endl;
#endif
}
void TriangleVK::PopulateCommandBuffer(const VkCommandBuffer CommandBuffer, const VkFramebuffer Framebuffer, const VkImage Image, const VkClearColorValue& Color)
{
	const VkCommandBufferBeginInfo BeginInfo = {
		VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		nullptr,
		0,
		nullptr
	};
	VERIFY_SUCCEEDED(vkBeginCommandBuffer(CommandBuffer, &BeginInfo)); {
#ifdef _DEBUG
		MarkerBegin(CommandBuffer, "Command Begin, End", glm::vec4(0,1,0,1));
#endif

		vkCmdSetViewport(CommandBuffer, 0, static_cast<uint32_t>(Viewports.size()), Viewports.data());
		vkCmdSetScissor(CommandBuffer, 0, static_cast<uint32_t>(ScissorRects.size()), ScissorRects.data());

		//!< クリア
		ClearColor(CommandBuffer, Image, Colors::SkyBlue);
		//ClearDepthStencil(CommandBuffer, DepthStencilImage, ClearDepthStencilValue);

		const VkRenderPassBeginInfo RenderPassBeginInfo = {
			VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
			nullptr,
			RenderPass,
			Framebuffer,
			ScissorRects[0],
			0, nullptr
		};
		vkCmdBeginRenderPass(CommandBuffer, &RenderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE); {
#if 0
			//!< ユニフォームバッファ
			if (!DescriptorSets.empty()) {
				vkCmdBindDescriptorSets(CommandBuffer, 
					VK_PIPELINE_BIND_POINT_GRAPHICS,
					PipelineLayout, 
					0, static_cast<uint32_t>(DescriptorSets.size()), DescriptorSets.data(), 
					0, nullptr);
			}
#endif
			vkCmdBindPipeline(CommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, Pipeline);

			const VkDeviceSize Offsets[] = { 0 };
			vkCmdBindVertexBuffers(CommandBuffer, 0, 1, &VertexBuffer, Offsets);
			vkCmdBindIndexBuffer(CommandBuffer, IndexBuffer, 0, VK_INDEX_TYPE_UINT32);

			//!< 描画
#ifdef USE_DRAW_INDIRECT
			vkCmdDrawIndexedIndirect(CommandBuffer, IndirectBuffer, 0, 1, 0);
#else
			vkCmdDrawIndexed(CommandBuffer, IndexCount, 1, 0, 0, 0);
#endif
		} vkCmdEndRenderPass(CommandBuffer);
#ifdef _DEBUG
		MarkerEnd(CommandBuffer);
#endif
	} VERIFY_SUCCEEDED(vkEndCommandBuffer(CommandBuffer));
}
#pragma endregion //!< Code