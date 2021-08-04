// RayTracingVK.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "RayTracingVK.h"

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
    LoadStringW(hInstance, IDC_RAYTRACINGVK, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_RAYTRACINGVK));

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
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_RAYTRACINGVK));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_RAYTRACINGVK);
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
			Inst = new RayTracingVK();
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

#pragma region RAYTRACING
void RayTracingVK::PopulateCommandBuffer([[maybe_unused]]const size_t i)
{
	if (!HasRayTracingSupport(GetCurrentPhysicalDevice())) { return; }

	const auto CB = CommandBuffers[i];
	constexpr VkCommandBufferBeginInfo CBBI = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		.pNext = nullptr,
		.flags = 0,
		.pInheritanceInfo = nullptr
	};
	VERIFY_SUCCEEDED(vkBeginCommandBuffer(CB, &CBBI)); {
		vkCmdBindPipeline(CB, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, Pipelines[0]);

		const std::array DSs = { DescriptorSets[0] };
		constexpr std::array<uint32_t, 0> DynamicOffsets = {};
		vkCmdBindDescriptorSets(CB, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, PipelineLayouts[0], 0, static_cast<uint32_t>(size(DSs)), data(DSs), static_cast<uint32_t>(size(DynamicOffsets)), data(DynamicOffsets));
        
		VkPhysicalDeviceRayTracingPipelinePropertiesKHR PDRTPP = { .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_PROPERTIES_KHR, .pNext = nullptr };
		VkPhysicalDeviceProperties2 PDP2 = { .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2, .pNext = &PDRTPP, };
		vkGetPhysicalDeviceProperties2(GetCurrentPhysicalDevice(), &PDP2);

		const auto AlignedSize = RoundUp(PDRTPP.shaderGroupHandleSize, PDRTPP.shaderGroupHandleAlignment);
		const auto RayGen = VkStridedDeviceAddressRegionKHR({ .deviceAddress = GetDeviceAddress(Device, ShaderBindingTables[0].Buffer), .stride = AlignedSize, .size = AlignedSize });
		const auto Miss = VkStridedDeviceAddressRegionKHR({ .deviceAddress = GetDeviceAddress(Device, ShaderBindingTables[1].Buffer), .stride = AlignedSize, .size = AlignedSize });
		const auto Hit = VkStridedDeviceAddressRegionKHR({ .deviceAddress = GetDeviceAddress(Device, ShaderBindingTables[2].Buffer), .stride = AlignedSize, .size = AlignedSize });
		const auto Callable = VkStridedDeviceAddressRegionKHR({ .deviceAddress = 0, .stride = 0, .size = 0 });

        //vkCmdTraceRaysKHR(CB, &RayGen, &Miss, &Hit, &Callable, GetClientRectWidth(), GetClientRectHeight(), 1);
        vkCmdTraceRaysIndirectKHR(CB, &RayGen, &Miss, &Hit, &Callable, GetDeviceAddress(Device, IndirectBuffers[0].Buffer));
        
		constexpr auto ISR = VkImageSubresourceRange({ .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .baseMipLevel = 0, .levelCount = 1, .baseArrayLayer = 0, .layerCount = 1 });
        {
            const std::array IMBs = {
                //!< PRESENT_SRC_KHR -> TRANSFER_DST_OPTIMAL
                VkImageMemoryBarrier({
                    .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
                    .pNext = nullptr,
                    .srcAccessMask = 0, .dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
                    .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED, .newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                    .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED, .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                    .image = SwapchainImages[i],
                    .subresourceRange = ISR
                }),
                //!< GENERAL -> TRANSFER_SRC_OPTIMAL
                VkImageMemoryBarrier({
                    .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
                    .pNext = nullptr,
                    .srcAccessMask = 0, .dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT,
                    .oldLayout = VK_IMAGE_LAYOUT_GENERAL, .newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                    .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED, .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                    .image = StorageTextures[0].Image,
                    .subresourceRange = ISR
                })
            };
			vkCmdPipelineBarrier(CB, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, nullptr, 0, nullptr, static_cast<uint32_t>(size(IMBs)), data(IMBs));
        }

        const std::array ICs = {
            VkImageCopy({
                .srcSubresource = VkImageSubresourceLayers({.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .mipLevel = 0, .baseArrayLayer = 0, .layerCount = 1 }),
                .srcOffset = VkOffset3D({.x = 0, .y = 0, .z = 0}),
                .dstSubresource = VkImageSubresourceLayers({.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .mipLevel = 0, .baseArrayLayer = 0, .layerCount = 1 }),
                .dstOffset = VkOffset3D({.x = 0, .y = 0, .z = 0}),
                .extent = VkExtent3D({.width = static_cast<uint32_t>(GetClientRectWidth()), .height = static_cast<uint32_t>(GetClientRectHeight()), .depth = 1 }),
            }),
        };
		vkCmdCopyImage(CB, StorageTextures[0].Image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, SwapchainImages[i], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, static_cast<uint32_t>(size(ICs)), data(ICs));

        {
			const std::array IMBs = {	
				//!< TRANSFER_DST_OPTIMAL -> PRESENT_SRC_KHR, 
		        VkImageMemoryBarrier({
			        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
			        .pNext = nullptr,
			        .srcAccessMask = 0, .dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
					.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, .newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
					.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED, .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
			        .image = SwapchainImages[i],
			        .subresourceRange = ISR
		        }),
                //!< TRANSFER_SRC_OPTIMAL -> GENERAL
		        VkImageMemoryBarrier({
			        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
			        .pNext = nullptr,
			        .srcAccessMask = 0, .dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT,
			        .oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, .newLayout = VK_IMAGE_LAYOUT_GENERAL,
			        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED, .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
			        .image = StorageTextures[0].Image,
			        .subresourceRange = ISR
		        })
			};
			vkCmdPipelineBarrier(CB, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, nullptr, 0, nullptr, static_cast<uint32_t>(size(IMBs)), data(IMBs)); 
        }

	} VERIFY_SUCCEEDED(vkEndCommandBuffer(CB));
}
#pragma endregion
