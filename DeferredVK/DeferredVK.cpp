// DeferredVK.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "DeferredVK.h"

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
    LoadStringW(hInstance, IDC_DEFERREDVK, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_DEFERREDVK));

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
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_DEFERREDVK));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_DEFERREDVK);
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
			Inst = new DeferredVK();
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
#pragma region Code
			if (nullptr != Inst) {
				Inst->OnPaint(hWnd, hInst);
			}
#pragma endregion
            // TODO: Add any drawing code that uses hdc here...
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
void DeferredVK::PopulateCommandBuffer(const size_t i)
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
#pragma region FRAME_OBJECT
			const auto DS = DescriptorSets[/*i*/0];
#pragma endregion
			const auto PL = Pipelines[0];
			const auto& IDB = IndirectBuffers[0];

			vkCmdSetViewport(SCB0, 0, static_cast<uint32_t>(size(Viewports)), data(Viewports));
			vkCmdSetScissor(SCB0, 0, static_cast<uint32_t>(size(ScissorRects)), data(ScissorRects));

			vkCmdBindPipeline(SCB0, VK_PIPELINE_BIND_POINT_GRAPHICS, PL);
			
			assert(!empty(DescriptorSets) && "");
			assert(!empty(PipelineLayouts) && "");
			const std::array<VkDescriptorSet, 1> DSs = { DS };
			vkCmdBindDescriptorSets(SCB0, VK_PIPELINE_BIND_POINT_GRAPHICS, PipelineLayouts[0], 0, static_cast<uint32_t>(size(DSs)), data(DSs), 0, nullptr);

			vkCmdDrawIndirect(SCB0, IDB.Buffer, 0, 1, 0);
		} VERIFY_SUCCEEDED(vkEndCommandBuffer(SCB0));
	}

	//!< パス1 : セカンダリコマンドバッファ(レンダーテクスチャ描画用)
#pragma region FRAME_OBJECT
	const auto SCCount = static_cast<uint32_t>(size(SwapchainImages));
#pragma endregion
	const auto SCB1 = SecondaryCommandBuffers[i + SCCount]; //!< オフセットさせる(ここでは2つのセカンダリコマンドバッファがぞれぞれスワップチェインイメージ数だけある)
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
#pragma region FRAME_OBJECT
			const auto DS = DescriptorSets[i + SCCount];
#pragma endregion
			const auto PL = Pipelines[1];
			const auto& IDB = IndirectBuffers[1];
			vkCmdSetViewport(SCB1, 0, static_cast<uint32_t>(size(Viewports)), data(Viewports));
			vkCmdSetScissor(SCB1, 0, static_cast<uint32_t>(size(ScissorRects)), data(ScissorRects));

			vkCmdBindPipeline(SCB1, VK_PIPELINE_BIND_POINT_GRAPHICS, PL);

			assert(2 == size(DescriptorSets) && "");
			assert(2 == size(PipelineLayouts) && "");
			const std::array<VkDescriptorSet, 1> DSs = { DS };
			vkCmdBindDescriptorSets(SCB1, VK_PIPELINE_BIND_POINT_GRAPHICS, PipelineLayouts[1], 0, static_cast<uint32_t>(size(DSs)), data(DSs), 0, nullptr);

			vkCmdDrawIndirect(SCB1, IDB.Buffer, 0, 1, 0);
		} VERIFY_SUCCEEDED(vkEndCommandBuffer(SCB1));
	}

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
			std::array<VkClearValue, 4 + 1> CVs = { 
				//!< レンダーターゲット : カラー(RenderTarget : Color)
				Colors::SkyBlue, 
#pragma region MRT
				//!< レンダーターゲット : 法線(RenderTarget : Normal)
				{ 0.5f, 0.5f, 1.0f, 1.0f },
				//!< レンダーターゲット : 深度(RenderTarget : Depth)
				Colors::Red,
				//!< レンダーターゲット : 未定
				Colors::SkyBlue,
#pragma endregion
			};
			CVs[4].depthStencil = { 1.0f, 0 };
			const VkRenderPassBeginInfo RPBI = {
				VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
				nullptr,
				RP0,
				FB0,
				RenderArea,
				static_cast<uint32_t>(size(CVs)), data(CVs)
			};
			vkCmdBeginRenderPass(CB, &RPBI, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS); {
				const std::array<VkCommandBuffer, 1> SCBs = { SCB0 };
				vkCmdExecuteCommands(CB, static_cast<uint32_t>(size(SCBs)), data(SCBs));
			} vkCmdEndRenderPass(CB);
		}

		//!< リソースバリア : VK_ACCESS_COLOR_ATTACHMENT_READ_BIT -> VK_ACCESS_SHADER_READ_BIT -> SubpassDependencyで代用可能かも？
		{
			const std::array<VkImageMemoryBarrier, 4> IMBs = { {
				//!< レンダーターゲット : カラー(RenderTarget : Color)
				{
					VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
					nullptr,
					VK_ACCESS_COLOR_ATTACHMENT_READ_BIT, VK_ACCESS_SHADER_READ_BIT,
					VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
					VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED,
					Images[0].Image,
					{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 }
				},
#pragma region MRT
				//!< レンダーターゲット : 法線(RenderTarget : Normal)
				{
					VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
					nullptr,
					VK_ACCESS_COLOR_ATTACHMENT_READ_BIT, VK_ACCESS_SHADER_READ_BIT,
					VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
					VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED,
					Images[1].Image,
					{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 }
				},
				//!< レンダーターゲット : 深度(RenderTarget : Depth)
				{
					VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
					nullptr,
					VK_ACCESS_COLOR_ATTACHMENT_READ_BIT, VK_ACCESS_SHADER_READ_BIT,
					VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
					VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED,
					Images[2].Image,
					{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 }
				},
				//!< レンダーターゲット : 未定
				{
					VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
					nullptr,
					VK_ACCESS_COLOR_ATTACHMENT_READ_BIT, VK_ACCESS_SHADER_READ_BIT,
					VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
					VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED,
					Images[3].Image,
					{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 }
				},
#pragma endregion
			} };
			vkCmdPipelineBarrier(CB,
				VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
				VK_DEPENDENCY_BY_REGION_BIT,
				0, nullptr,
				0, nullptr,
				static_cast<uint32_t>(size(IMBs)), data(IMBs));
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
				static_cast<uint32_t>(size(CVs)), data(CVs)
			};
			vkCmdBeginRenderPass(CB, &RPBI, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS); {
				const std::array<VkCommandBuffer, 1> SCBs = { SCB1 };
				vkCmdExecuteCommands(CB, static_cast<uint32_t>(size(SCBs)), data(SCBs));
			} vkCmdEndRenderPass(CB);
		}

	} VERIFY_SUCCEEDED(vkEndCommandBuffer(CB));
}
#pragma endregion