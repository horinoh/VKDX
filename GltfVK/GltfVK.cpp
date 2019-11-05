// GltfVK.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "GltfVK.h"

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
    LoadStringW(hInstance, IDC_GLTFVK, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_GLTFVK));

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
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_GLTFVK));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_GLTFVK);
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
			Inst = new GltfVK();
		}
		if (nullptr != Inst) {
			try {
				Inst->OnCreate(hWnd, hInst, szTitle);
			}
			catch (std::exception & e) {
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
void GltfVK::CreateVertexBuffer()
{
	Load("..\\..\\glTF-Sample-Models\\2.0\\Duck\\glTF-Binary\\Duck.glb");
	//Load("..\\..\\glTF-Sample-Models\\2.0\\Duck\\glTF-Embedded\\Duck.gltf"); // LoadText()
	//Load("..\\..\\glTF-Sample-Models\\2.0\\CesiumMan\\glTF-Binary\\CesiumMan.glb");
	VertexBuffers.resize(1);

	const std::array<Vertex_PositionColor, 3> Vertices = { {
		{ { 0.0f, 0.5f, 0.0f }, { 1.0f, 0.0f, 0.0f, 1.0f } }, //!< CT
		{ { -0.5f, -0.5f, 0.0f }, { 0.0f, 1.0f, 0.0f, 1.0f } }, //!< LB
		{ { 0.5f, -0.5f, 0.0f }, { 0.0f, 0.0f, 1.0f, 1.0f } }, //!< RB
	} };
	const auto Stride = sizeof(Vertices[0]);
	const auto Size = static_cast<VkDeviceSize>(Stride * Vertices.size());

	CreateBuffer_Vertex(GraphicsQueue, CommandBuffers[0], &VertexBuffers[0], Size, Vertices.data());

#ifdef _DEBUG
	MarkerSetObjectName(Device, VertexBuffers[0], "MyVertexBuffer");
#endif

	LOG_OK();
}
void GltfVK::CreateIndexBuffer()
{
	IndexBuffers.resize(1);

	const std::array<uint32_t, 3> Indices = { 0, 1, 2 };

	//!< vkCmdDrawIndexed() が引数に取るので覚えておく必要がある (Save this value because vkCmdDrawIndexed() will use it)
	IndexCount = static_cast<uint32_t>(Indices.size());
	const auto Stride = sizeof(Indices[0]);
	const auto Size = static_cast<VkDeviceSize>(Stride * IndexCount);

	CreateBuffer_Index(GraphicsQueue, CommandBuffers[0], &IndexBuffers[0], Size, Indices.data());

#ifdef _DEBUG
	MarkerSetObjectName(Device, IndexBuffers[0], "MyIndexBuffer");
#endif

	LOG_OK();
}

void GltfVK::PopulateCommandBuffer(const size_t i)
{
	const auto CB = CommandBuffers[i];
#ifdef USE_SECONDARY_COMMAND_BUFFER
	const auto SCB = SecondaryCommandBuffers[i];
#endif
	const auto FB = Framebuffers[i];
	const auto RP = RenderPasses[0];
	const auto VB = VertexBuffers[0];
	const auto IB = IndexBuffers[0];
	const auto IndirectB = IndirectBuffers[0];
	const auto PL = Pipelines[0];

#ifdef USE_SECONDARY_COMMAND_BUFFER
	const VkCommandBufferInheritanceInfo CBII = {
		VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO,
		nullptr,
		RP,
		0,
		FB,
		VK_FALSE,
		0,
		0,
	};
	const VkCommandBufferBeginInfo SCBBI = {
		VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		nullptr,
		VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT,
		&CBII
	};
	VERIFY_SUCCEEDED(vkBeginCommandBuffer(SCB, &SCBBI)); {
		vkCmdSetViewport(SCB, 0, static_cast<uint32_t>(Viewports.size()), Viewports.data());
		vkCmdSetScissor(SCB, 0, static_cast<uint32_t>(ScissorRects.size()), ScissorRects.data());
		vkCmdBindPipeline(SCB, VK_PIPELINE_BIND_POINT_GRAPHICS, PL);
		const std::array<VkBuffer, 1> VBs = { VB };
		const std::array<VkDeviceSize, 1> Offsets = { 0 };
		assert(VBs.size() == Offsets.size() && "");
		vkCmdBindVertexBuffers(SCB, 0, static_cast<uint32_t>(VBs.size()), VBs.data(), Offsets.data());
		vkCmdBindIndexBuffer(SCB, IB, 0, VK_INDEX_TYPE_UINT32);
		vkCmdDrawIndexedIndirect(SCB, IndirectB, 0, 1, 0);
	} VERIFY_SUCCEEDED(vkEndCommandBuffer(SCB));
#endif

	const VkCommandBufferBeginInfo CBBI = {
		VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		nullptr,
		0,
		nullptr
	};
	VERIFY_SUCCEEDED(vkBeginCommandBuffer(CB, &CBBI)); {
#ifdef _DEBUG
		//MarkerBegin(CB, glm::vec4(0.0f, 1.0f, 0.0f, 1.0f), "Command Begin");
		ScopedMarker(CB, glm::vec4(0.0f, 1.0f, 0.0f, 1.0f), "Command Begin");
		//MarkerInsert(CB, glm::vec4(1.0f, 0.0f, 0.0f, 1.0f), "Command");
#endif

#ifdef USE_RENDER_PASS_CLEAR
		const std::array<VkClearValue, 1> CVs = { Colors::SkyBlue };
#else
		const std::array<VkClearValue, 0> CVs = {};
		ClearColor(CB, SwapchainImages[i], Colors::SkyBlue);
#endif
		const VkRenderPassBeginInfo RPBI = {
			VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
			nullptr,
			RP,
			FB,
			ScissorRects[0],
			static_cast<uint32_t>(CVs.size()), CVs.data()
		};
#ifdef USE_SECONDARY_COMMAND_BUFFER
		//!< セカンダリコマンドバッファの呼び出し
		vkCmdBeginRenderPass(CB, &RPBI, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS); {
			const std::array<VkCommandBuffer, 1> SCBs = { SCB };
			vkCmdExecuteCommands(CB, static_cast<uint32_t>(SCBs.size()), SCBs.data());
		} vkCmdEndRenderPass(CB);
#else
		vkCmdBeginRenderPass(CB, &RPBI, VK_SUBPASS_CONTENTS_INLINE); {
			vkCmdSetViewport(CB, 0, static_cast<uint32_t>(Viewports.size()), Viewports.data());
			vkCmdSetScissor(CB, 0, static_cast<uint32_t>(ScissorRects.size()), ScissorRects.data());
#if 0
			//!< プッシュコンスタント PushConstants
			const uint32_t Offset = 64; //!< 4の倍数であること(ここではフラグメントシェーダ用は 64byte オフセットしている) Mulitiple of 4(For fragment shader offset 64 byte in this case)
			const std::array<float, 4> Color = { 0.0f, 0.7f, 0.4f, 0.1f };
			const auto Size = static_cast<uint32_t>(Color.size() * sizeof(Color[0])); //!< 4の倍数であること Mulitiple of 4
			vkCmdPushConstants(CommandBuffer, PipelineLayouts[0], VK_SHADER_STAGE_FRAGMENT_BIT, Offset, Size, Color.data());
#endif
			vkCmdBindPipeline(CB, VK_PIPELINE_BIND_POINT_GRAPHICS, PL);
			const std::array<VkBuffer, 1> VBs = { VB };
			const std::array<VkDeviceSize, 1> Offsets = { 0 };
			assert(VBs.size() == Offsets.size() && "");
			vkCmdBindVertexBuffers(CB, 0, static_cast<uint32_t>(VBs.size()), VBs.data(), Offsets.data());
			vkCmdBindIndexBuffer(CB, IB, 0, VK_INDEX_TYPE_UINT32);
			vkCmdDrawIndexedIndirect(CB, IndirectB, 0, 1, 0);
		} vkCmdEndRenderPass(CB);
#endif

#ifdef _DEBUG
		//MarkerEnd(CB);
#endif
	} VERIFY_SUCCEEDED(vkEndCommandBuffer(CB));
}
#pragma endregion //!< Code