    // ShadowMapVK.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "ShadowMapVK.h"

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
    LoadStringW(hInstance, IDC_SHADOWMAPVK, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_SHADOWMAPVK));

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
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_SHADOWMAPVK));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_SHADOWMAPVK);
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
			Inst = new ShadowMapVK();
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
void ShadowMapVK::PopulateCommandBuffer(const size_t i)
{
	//!< パス0
	const auto RP0 = RenderPasses[0];
	const auto FB0 = Framebuffers[0];

	//!< パス1 
	const auto RP1 = RenderPasses[1];
	const auto FB1 = Framebuffers[i + 1];

	//!< パス0 : セカンダリコマンドバッファ(シャドウキャスタ描画用)
	const auto SCB0 = SecondaryCommandBuffers[i];
	{
		const VkCommandBufferInheritanceInfo CBII = {
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO,
			.pNext = nullptr,
			.renderPass = RP0,
			.subpass = 0,
			.framebuffer = FB0,
			.occlusionQueryEnable = VK_FALSE, .queryFlags = 0,
			.pipelineStatistics = 0,
		};
		const VkCommandBufferBeginInfo CBBI = {
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
			.pNext = nullptr,
			.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT,
			.pInheritanceInfo = &CBII
		};
		VERIFY_SUCCEEDED(vkBeginCommandBuffer(SCB0, &CBBI)); {
			const auto PL = Pipelines[0];
#pragma region FRAME_OBJECT
			const auto DS = DescriptorSets[i];
#pragma endregion
			const auto PLL = PipelineLayouts[0];
			const auto& IDB = IndirectBuffers[0];
#if 0
			vkCmdSetViewport(SCB0, 0, static_cast<uint32_t>(size(Viewports)), data(Viewports));
			vkCmdSetScissor(SCB0, 0, static_cast<uint32_t>(size(ScissorRects)), data(ScissorRects));
#else
			const std::array VPs = { VkViewport({ .x = 0.0f, .y = static_cast<float>(ShadowMapExtent.height), .width = static_cast<float>(ShadowMapExtent.width), .height = -static_cast<float>(ShadowMapExtent.height), .minDepth = 0.0f, .maxDepth = 1.0f  }) };
			const std::array SCs = { VkRect2D({ VkOffset2D({ .x = 0, .y = 0 }), VkExtent2D({ .width = ShadowMapExtent.width, .height = ShadowMapExtent.height }) }) };
			vkCmdSetViewport(SCB0, 0, static_cast<uint32_t>(size(VPs)), data(VPs));
			vkCmdSetScissor(SCB0, 0, static_cast<uint32_t>(size(SCs)), data(SCs));
#endif
			vkCmdBindPipeline(SCB0, VK_PIPELINE_BIND_POINT_GRAPHICS, PL);

			const std::array DSs = { DS };
			constexpr std::array<uint32_t, 0> DynamicOffsets = {};
			vkCmdBindDescriptorSets(SCB0, VK_PIPELINE_BIND_POINT_GRAPHICS, PLL, 0, static_cast<uint32_t>(size(DSs)), data(DSs), static_cast<uint32_t>(size(DynamicOffsets)), data(DynamicOffsets));

			vkCmdDrawIndirect(SCB0, IDB.Buffer, 0, 1, 0);
		} VERIFY_SUCCEEDED(vkEndCommandBuffer(SCB0));
	}

	//!< パス1 : セカンダリコマンドバッファ(レンダーテクスチャ描画用、シャドウレシーバ描画用)
#pragma region FRAME_OBJECT
	const auto SCCount = static_cast<uint32_t>(size(SwapchainBackBuffers));
#pragma endregion
	const auto SCB1 = SecondaryCommandBuffers[i + SCCount]; //!< オフセットさせる(ここでは2つのセカンダリコマンドバッファがぞれぞれスワップチェインイメージ数だけある)
	{
		const VkCommandBufferInheritanceInfo CBII = {
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO,
			.pNext = nullptr,
			.renderPass = RP1,
			.subpass = 0,
			.framebuffer = FB1,
			.occlusionQueryEnable = VK_FALSE, .queryFlags = 0,
			.pipelineStatistics = 0,
		};
		const VkCommandBufferBeginInfo CBBI = {
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
			.pNext = nullptr,
			.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT,
			.pInheritanceInfo = &CBII
		};
		VERIFY_SUCCEEDED(vkBeginCommandBuffer(SCB1, &CBBI)); {
			const auto PL = Pipelines[1];
#pragma region FRAME_OBJECT
			const auto DS = DescriptorSets[i + SCCount];
#pragma endregion
			const auto PLL = PipelineLayouts[1];
			const auto& IDB = IndirectBuffers[1];

			vkCmdSetViewport(SCB1, 0, static_cast<uint32_t>(size(Viewports)), data(Viewports));
			vkCmdSetScissor(SCB1, 0, static_cast<uint32_t>(size(ScissorRects)), data(ScissorRects));

			vkCmdBindPipeline(SCB1, VK_PIPELINE_BIND_POINT_GRAPHICS, PL);

			const std::array DSs = { DS };
			constexpr std::array<uint32_t, 0> DynamicOffsets = {};
			vkCmdBindDescriptorSets(SCB1, VK_PIPELINE_BIND_POINT_GRAPHICS, PLL, 0, static_cast<uint32_t>(size(DSs)), data(DSs), static_cast<uint32_t>(size(DynamicOffsets)), data(DynamicOffsets));

			vkCmdDrawIndirect(SCB1, IDB.Buffer, 0, 1, 0);
		} VERIFY_SUCCEEDED(vkEndCommandBuffer(SCB1));
	}

	const auto CB = CommandBuffers[i];
	const VkCommandBufferBeginInfo CBBI = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		.pNext = nullptr,
		.flags = 0,
		.pInheritanceInfo = nullptr
	};
	VERIFY_SUCCEEDED(vkBeginCommandBuffer(CB, &CBBI)); {
		//!< パス0 : レンダーパス (シャドウキャスタ描画用)
		{
			const std::array CVs = { VkClearValue({.color = Colors::SkyBlue }), VkClearValue({.depthStencil = {.depth = 1.0f, .stencil = 0 } }) };
			const VkRenderPassBeginInfo RPBI = {
				.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
				.pNext = nullptr,
				.renderPass = RP0,
				.framebuffer = FB0,
				.renderArea = VkRect2D({.offset = VkOffset2D({.x = 0, .y = 0 }), .extent = ShadowMapExtent }),
				.clearValueCount = static_cast<uint32_t>(size(CVs)), .pClearValues = data(CVs)
			};
			vkCmdBeginRenderPass(CB, &RPBI, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS); {
				//!< パイプライン作成時に指定しておくこと
				//!< VkPipelineRasterizationStateCreateInfo.depthBiasEnable, depthBiasConstantFactor, depthBiasClamp, depthBiasSlopeFactor 
				//!< また、pDynamicState に VK_DYNAMIC_STATE_DEPTH_BIAS を追加しておくと、ランライムに vkCmdSetDepthBias(CB, depthBiasConstantFactor, depthBiasClamp, depthBiasSlopeFactor) 変更できる  
				//vkCmdSetDepthBias(CB, 1.25f, 0.0f, 1.75f);

				const std::array SCBs = { SCB0 };
				vkCmdExecuteCommands(CB, static_cast<uint32_t>(size(SCBs)), data(SCBs));
			} vkCmdEndRenderPass(CB);
		}

		//!< リソースバリア
		{
			constexpr std::array<VkMemoryBarrier, 0> MBs = {};
			constexpr std::array<VkBufferMemoryBarrier, 0> BMBs = {};
			const std::array IMBs = { 
				VkImageMemoryBarrier({
					.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
					.pNext = nullptr,
					.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT, .dstAccessMask = VK_ACCESS_SHADER_READ_BIT,
					.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED, .newLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL,
					.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED, .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
					.image = DepthTextures[0].Image,
					.subresourceRange = VkImageSubresourceRange({ 
						.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT, 
						.baseMipLevel = 0, .levelCount = 1, 
						.baseArrayLayer = 0, .layerCount = 1 
					})
				}),
			};
			vkCmdPipelineBarrier(CB,
				VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
				VK_DEPENDENCY_BY_REGION_BIT,
				static_cast<uint32_t>(std::size(MBs)), std::data(MBs),
				static_cast<uint32_t>(std::size(BMBs)), std::data(BMBs),
				static_cast<uint32_t>(std::size(IMBs)), std::data(IMBs));
		}

		//!< パス1 : レンダーパス(レンダーテクスチャ描画用)
		{
#ifdef USE_SHADOWMAP_VISUALIZE
			const std::array<VkClearValue, 0> CVs = {};
#else
			const std::array CVs = { VkClearValue({.color = Colors::SkyBlue }), VkClearValue({.depthStencil = {.depth = 1.0f, .stencil = 0 } }) };
#endif
			const VkRenderPassBeginInfo RPBI = {
				.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
				.pNext = nullptr,
				.renderPass = RP1,
				.framebuffer = FB1,
				.renderArea = VkRect2D({.offset = VkOffset2D({.x = 0, .y = 0 }), .extent = SurfaceExtent2D }),
				.clearValueCount = static_cast<uint32_t>(size(CVs)), .pClearValues = data(CVs)
			};
			vkCmdBeginRenderPass(CB, &RPBI, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS); {
				const std::array SCBs = { SCB1 };
				vkCmdExecuteCommands(CB, static_cast<uint32_t>(size(SCBs)), data(SCBs));
			} vkCmdEndRenderPass(CB);
		}
	} VERIFY_SUCCEEDED(vkEndCommandBuffer(CB));
}
#pragma endregion
