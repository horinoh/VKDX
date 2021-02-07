// InstancingVK.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "InstancingVK.h"

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
    LoadStringW(hInstance, IDC_INSTANCINGVK, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_INSTANCINGVK));

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
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_INSTANCINGVK));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_INSTANCINGVK);
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
			Inst = new InstancingVK();
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
void InstancingVK::CreateGeometry()
{
	const auto& CB = CommandBuffers[0];

	{
		VertexBuffers.push_back(VertexBuffer());
		const std::array Vertices = {
			Vertex_PositionColor({.Position = { 0.0f, 0.5f, 0.0f }, .Color = { 1.0f, 0.0f, 0.0f, 1.0f } }),
			Vertex_PositionColor({.Position = { -0.5f, -0.5f, 0.0f }, .Color = { 0.0f, 1.0f, 0.0f, 1.0f } }),
			Vertex_PositionColor({.Position = { 0.5f, -0.5f, 0.0f }, .Color = { 0.0f, 0.0f, 1.0f, 1.0f } }),
		};
		//CreateAndCopyToBuffer(&VertexBuffers.back().Buffer, &VertexBuffers.back().DeviceMemory, GraphicsQueue, CB, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT, VK_PIPELINE_STAGE_VERTEX_INPUT_BIT, sizeof(Vertices), data(Vertices));
		VertexBuffers.back().Create(Device, GetCurrentPhysicalDeviceMemoryProperties(), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, sizeof(Vertices), data(Vertices), CB, VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT, VK_PIPELINE_STAGE_VERTEX_INPUT_BIT, GraphicsQueue);
	}
	{
		VertexBuffers.push_back(VertexBuffer());
		const std::array Instances = {
			Instance_OffsetXY({ { -0.5f, -0.5f } }),
			Instance_OffsetXY({ { -0.25f, -0.25f } }),
			Instance_OffsetXY({ { 0.0f, 0.0f } }),
			Instance_OffsetXY({ { 0.25f, 0.25f } }),
			Instance_OffsetXY({ { 0.5f, 0.5f } }),
		};
		//CreateAndCopyToBuffer(&VertexBuffers.back().Buffer, &VertexBuffers.back().DeviceMemory, GraphicsQueue, CB, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT, VK_PIPELINE_STAGE_VERTEX_INPUT_BIT, sizeof(Instances), data(Instances));
		VertexBuffers.back().Create(Device, GetCurrentPhysicalDeviceMemoryProperties(), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, sizeof(Instances), data(Instances), CB, VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT, VK_PIPELINE_STAGE_VERTEX_INPUT_BIT, GraphicsQueue);

		IndexBuffers.push_back(IndexBuffer());
		const std::array<uint32_t, 3> Indices = { 0, 1, 2 };
		//CreateAndCopyToBuffer(&IndexBuffers.back().Buffer, &IndexBuffers.back().DeviceMemory, GraphicsQueue, CB, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_ACCESS_INDEX_READ_BIT, VK_PIPELINE_STAGE_VERTEX_INPUT_BIT, sizeof(Indices), data(Indices));
		IndexBuffers.back().Create(Device, GetCurrentPhysicalDeviceMemoryProperties(), VK_BUFFER_USAGE_INDEX_BUFFER_BIT, sizeof(Indices), data(Indices), CB, VK_ACCESS_INDEX_READ_BIT, VK_PIPELINE_STAGE_VERTEX_INPUT_BIT, GraphicsQueue);

		{
			IndirectBuffers.push_back(IndirectBuffer());
			constexpr VkDrawIndexedIndirectCommand DIIC = { .indexCount = static_cast<uint32_t>(size(Indices)), .instanceCount = static_cast<uint32_t>(size(Instances)), .firstIndex = 0, .vertexOffset = 0, .firstInstance = 0 };
			//CreateAndCopyToBuffer(&IndirectBuffers.back().Buffer, &IndirectBuffers.back().DeviceMemory, GraphicsQueue, CB, VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, VK_ACCESS_INDIRECT_COMMAND_READ_BIT, VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT, static_cast<VkDeviceSize>(sizeof(DIIC)), &DIIC);
			IndirectBuffers.back().Create(Device, GetCurrentPhysicalDeviceMemoryProperties(), VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, sizeof(DIIC), &DIIC, CB, VK_ACCESS_INDIRECT_COMMAND_READ_BIT, VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT, GraphicsQueue);
		}
	}
	LOG_OK();
}
void InstancingVK::PopulateCommandBuffer(const size_t i)
{
	const auto RP = RenderPasses[0];
	const auto FB = Framebuffers[i];

	const auto SCB = SecondaryCommandBuffers[i];
	const VkCommandBufferInheritanceInfo CBII = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO,
		.pNext = nullptr,
		.renderPass = RP,
		.subpass = 0,
		.framebuffer = FB,
		.occlusionQueryEnable = VK_FALSE, .queryFlags = 0,
		.pipelineStatistics = 0,
	};
	const VkCommandBufferBeginInfo SCBBI = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		.pNext = nullptr,
		.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT,
		.pInheritanceInfo = &CBII
	};
	VERIFY_SUCCEEDED(vkBeginCommandBuffer(SCB, &SCBBI)); {
		const auto PL = Pipelines[0];
		const auto& VB0 = VertexBuffers[0];
		const auto& VB1 = VertexBuffers[1];
		const auto& IB = IndexBuffers[0];
		const auto& IDB = IndirectBuffers[0];

		vkCmdSetViewport(SCB, 0, static_cast<uint32_t>(size(Viewports)), data(Viewports));
		vkCmdSetScissor(SCB, 0, static_cast<uint32_t>(size(ScissorRects)), data(ScissorRects));
		vkCmdBindPipeline(SCB, VK_PIPELINE_BIND_POINT_GRAPHICS, PL);
		const std::array VBs = { VB0.Buffer, VB1.Buffer };
		const std::array<VkDeviceSize, 2> Offsets = { 0, 0 };
		assert(size(VBs) == size(Offsets) && "");
		vkCmdBindVertexBuffers(SCB, 0, static_cast<uint32_t>(size(VBs)), data(VBs), data(Offsets));
		vkCmdBindIndexBuffer(SCB, IB.Buffer, 0, VK_INDEX_TYPE_UINT32);
		vkCmdDrawIndexedIndirect(SCB, IDB.Buffer, 0, 1, 0);
	} VERIFY_SUCCEEDED(vkEndCommandBuffer(SCB));

	const auto CB = CommandBuffers[i];
	const VkCommandBufferBeginInfo CBBI = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		.pNext = nullptr,
		.flags = 0,
		.pInheritanceInfo = nullptr
	};
	VERIFY_SUCCEEDED(vkBeginCommandBuffer(CB, &CBBI)); {
		const std::array CVs = { VkClearValue({.color = Colors::SkyBlue }) };
		const VkRenderPassBeginInfo RPBI = {
			.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
			.pNext = nullptr,
			.renderPass = RP,
			.framebuffer = FB,
			.renderArea = VkRect2D({.offset = VkOffset2D({.x = 0, .y = 0 }), .extent = SurfaceExtent2D }),
			.clearValueCount = static_cast<uint32_t>(size(CVs)), .pClearValues = data(CVs)
		};
		vkCmdBeginRenderPass(CB, &RPBI, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS); {
			const std::array SCBs = { SCB };
			vkCmdExecuteCommands(CB, static_cast<uint32_t>(size(SCBs)), data(SCBs));
		} vkCmdEndRenderPass(CB);
	} VERIFY_SUCCEEDED(vkEndCommandBuffer(CB));
}
#pragma endregion //!< Code