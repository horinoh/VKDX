// RenderTargetVK.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "RenderTargetVK.h"

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
    LoadStringW(hInstance, IDC_RENDERTARGETVK, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_RENDERTARGETVK));

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
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_RENDERTARGETVK));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_RENDERTARGETVK);
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
			Inst = new RenderTargetVK();
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
void RenderTargetVK::PopulateCommandBuffer(const size_t i)
{
	//!< パス0
	const auto RP0 = RenderPasses[0];
	const auto FB0 = Framebuffers[0];

	//!< パス1 
	const auto RP1 = RenderPasses[1];
	const auto FB1 = Framebuffers[i + 1];

	//!< パス0 : セカンダリコマンドバッファ(メッシュ描画用)
	const auto SCB0 = SecondaryCommandBuffers[i];
	{
		const VkCommandBufferInheritanceInfo CBII = {
			VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO,
			nullptr,
			RP0,
			0,
			FB0,
			VK_FALSE,
			0,
			0,
		};
		const VkCommandBufferBeginInfo CBBI = {
			VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
			nullptr,
			VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT,
			&CBII
		};
		VERIFY_SUCCEEDED(vkBeginCommandBuffer(SCB0, &CBBI)); {
			const auto PL = Pipelines[0];
			const auto IDB = IndirectBuffers[0];
			vkCmdSetViewport(SCB0, 0, static_cast<uint32_t>(Viewports.size()), Viewports.data());
			vkCmdSetScissor(SCB0, 0, static_cast<uint32_t>(ScissorRects.size()), ScissorRects.data());
			vkCmdBindPipeline(SCB0, VK_PIPELINE_BIND_POINT_GRAPHICS, PL);
			vkCmdDrawIndirect(SCB0, IDB.Buffer, 0, 1, 0);
		} VERIFY_SUCCEEDED(vkEndCommandBuffer(SCB0));
	}

	//!< パス1 : セカンダリコマンドバッファ(レンダーテクスチャ描画用)
	const auto SCB1 = SecondaryCommandBuffers[i + SecondaryCommandBuffers.size() / 2]; //!< オフセットさせる(ここでは2つのセカンダリコマンドバッファがぞれぞれスワップチェインイメージ数だけある)
	{
		const VkCommandBufferInheritanceInfo CBII = {
			VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO,
			nullptr,
			RP1,
			0,
			FB1,
			VK_FALSE,
			0,
			0,
		};
		const VkCommandBufferBeginInfo CBBI = {
			VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
			nullptr,
			VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT,
			&CBII
		};
		VERIFY_SUCCEEDED(vkBeginCommandBuffer(SCB1, &CBBI)); {
			const auto PL = Pipelines[1];
			const auto IDB = IndirectBuffers[1];
			vkCmdSetViewport(SCB1, 0, static_cast<uint32_t>(Viewports.size()), Viewports.data());
			vkCmdSetScissor(SCB1, 0, static_cast<uint32_t>(ScissorRects.size()), ScissorRects.data());
			vkCmdBindPipeline(SCB1, VK_PIPELINE_BIND_POINT_GRAPHICS, PL);

			assert(!DescriptorSets.empty() && "");
			const auto PLL = PipelineLayouts[1];
			vkCmdBindDescriptorSets(SCB1, VK_PIPELINE_BIND_POINT_GRAPHICS, PLL, 0, static_cast<uint32_t>(DescriptorSets.size()), DescriptorSets.data(), 0, nullptr);

			vkCmdDrawIndirect(SCB1, IDB.Buffer, 0, 1, 0);
		} VERIFY_SUCCEEDED(vkEndCommandBuffer(SCB1));
	}

#ifdef USE_SUBPASS
	//vkCmdNextSubpass() を使用する
#endif

	const auto CB = CommandBuffers[i];
	const VkCommandBufferBeginInfo CBBI = {
		VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		nullptr,
		0,
		nullptr
	};
	VERIFY_SUCCEEDED(vkBeginCommandBuffer(CB, &CBBI)); {
		const VkRect2D RenderArea = { { 0, 0 }, SurfaceExtent2D };

		//!< パス0 : レンダーパス(メッシュ描画用)
		{
#ifdef USE_DEPTH
			std::array<VkClearValue, 2> CVs = { Colors::SkyBlue };
			CVs[1].depthStencil = { 1.0f, 0 };
#else
			const std::array<VkClearValue, 1> CVs = { Colors::SkyBlue };
#endif
			const VkRenderPassBeginInfo RPBI = {
				VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
				nullptr,
				RP0,
				FB0,
				RenderArea,
				static_cast<uint32_t>(CVs.size()), CVs.data()
			};
			vkCmdBeginRenderPass(CB, &RPBI, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS); {
				const std::array<VkCommandBuffer, 1> SCBs = { SCB0 };
				vkCmdExecuteCommands(CB, static_cast<uint32_t>(SCBs.size()), SCBs.data());
			} vkCmdEndRenderPass(CB);
		}

		//!< リソースバリア : VK_ACCESS_COLOR_ATTACHMENT_READ_BIT -> VK_ACCESS_SHADER_READ_BIT
		{
			const std::array<VkImageMemoryBarrier, 1> IMBs = {
				{
					VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
					nullptr,
					VK_ACCESS_COLOR_ATTACHMENT_READ_BIT, VK_ACCESS_SHADER_READ_BIT,
					VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
					VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED,
					Images[0].Image,
					{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 }
				},
			};
			vkCmdPipelineBarrier(CB,
				VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
				VK_DEPENDENCY_BY_REGION_BIT,
				0, nullptr,
				0, nullptr,
				static_cast<uint32_t>(IMBs.size()), IMBs.data());
		}
		
		//!< パス1 : レンダーパス(レンダーテクスチャ描画用)
		{
			const std::array<VkClearValue, 0> CVs = {};
			const VkRenderPassBeginInfo RPBI = {
				VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
				nullptr,
				RP1,
				FB1,
				RenderArea,
				static_cast<uint32_t>(CVs.size()), CVs.data()
			};
			vkCmdBeginRenderPass(CB, &RPBI, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS); {
				const std::array<VkCommandBuffer, 1> SCBs = { SCB1 };
				vkCmdExecuteCommands(CB, static_cast<uint32_t>(SCBs.size()), SCBs.data());
			} vkCmdEndRenderPass(CB);
		}

	} VERIFY_SUCCEEDED(vkEndCommandBuffer(CB));
}
#pragma endregion