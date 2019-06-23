// NormalMapVK.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "NormalMapVK.h"

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
    LoadStringW(hInstance, IDC_NORMALMAPVK, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_NORMALMAPVK));

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
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_NORMALMAPVK));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_NORMALMAPVK);
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
			Inst = new NormalMapVK();
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
void NormalMapVK::CreatePipelineLayout()
{
	//!< ImmutableSampler == STATIC_SAMPLER_DESC 相当？
	//const VkSamplerCreateInfo SCI = {
	//	VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
	//	nullptr,
	//	0,
	//	VK_FILTER_LINEAR, VK_FILTER_LINEAR, VK_SAMPLER_MIPMAP_MODE_LINEAR,
	//	VK_SAMPLER_ADDRESS_MODE_REPEAT, VK_SAMPLER_ADDRESS_MODE_REPEAT, VK_SAMPLER_ADDRESS_MODE_REPEAT,
	//	0.0f,
	//	VK_FALSE, 1.0f,
	//	VK_FALSE, VK_COMPARE_OP_NEVER,
	//	0.0f, 1.0f,
	//	VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE,
	//	VK_FALSE
	//};
	//VkSampler Sampler = VK_NULL_HANDLE;
	//VERIFY_SUCCEEDED(vkCreateSampler(Device, &SCI, GetAllocationCallbacks(), &Sampler));
	//const std::array<VkSampler, 1> ISs = {{ Sampler }};

	const std::array<VkDescriptorSetLayoutBinding, 2> DSLBs = { { //!< arrayの初期化は"{"が余計に必要
		{
			0,
			VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			1,
			VK_SHADER_STAGE_GEOMETRY_BIT,
			nullptr
		},
		{
			1,
			VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			1,//static_cast<uint32_t>(ISs.size()),
			VK_SHADER_STAGE_FRAGMENT_BIT,
			nullptr//ISs.data()
		}
	}};

	const VkDescriptorSetLayoutCreateInfo DSLCI = {
		VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
		nullptr,
		0,
		static_cast<uint32_t>(DSLBs.size()), DSLBs.data()
	};

	VkDescriptorSetLayout DSL = VK_NULL_HANDLE;
	VERIFY_SUCCEEDED(vkCreateDescriptorSetLayout(Device, &DSLCI, GetAllocationCallbacks(), &DSL));
	DescriptorSetLayouts.push_back(DSL);

	const std::array<VkPushConstantRange, 0> PCRs = {};

	const VkPipelineLayoutCreateInfo PLCI = {
		VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
		nullptr,
		0,
		static_cast<uint32_t>(DescriptorSetLayouts.size()), DescriptorSetLayouts.data(),
		static_cast<uint32_t>(PCRs.size()), PCRs.data()
	};
	VERIFY_SUCCEEDED(vkCreatePipelineLayout(Device, &PLCI, GetAllocationCallbacks(), &PipelineLayout));

	//vkDestroySampler(Device, Sampler, GetAllocationCallbacks());

	LogOK("CreatePipelineLayout");
}
void NormalMapVK::CreateDescriptorPool()
{
	const std::array<VkDescriptorPoolSize, 2> DPSs = { {
		{
			VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			1
		},
		{
			VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			1
		}
	}};
	const uint32_t MaxSets = [&]() {
		uint32_t Count = 0;
		for (const auto& i : DPSs) {
			Count = std::max(Count, i.descriptorCount);
		}
		return Count;
	}();
	const VkDescriptorPoolCreateInfo DPCI = {
		VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
		nullptr,
		0,
		MaxSets,
		static_cast<uint32_t>(DPSs.size()), DPSs.data()
	};
	VERIFY_SUCCEEDED(vkCreateDescriptorPool(Device, &DPCI, GetAllocationCallbacks(), &DescriptorPool));
	assert(VK_NULL_HANDLE != DescriptorPool && "Failed to create descriptor pool");

	LogOK("CreateDescriptorPool");
}
void NormalMapVK::UpdateDescriptorSet()
{
	const std::array<VkDescriptorBufferInfo, 1> DBIs = {
		{
			UniformBuffer,
			0,
			VK_WHOLE_SIZE
		}
	};
	const std::array<VkDescriptorImageInfo, 1> DIIs = {
		{
			Samplers[0],
			ImageView,
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
		}
	};
	const std::array<VkWriteDescriptorSet, 2> WDSs = { {
		{
			VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
			nullptr,
			DescriptorSets[0], 0, 0,
			static_cast<uint32_t>(DBIs.size()),
			VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			nullptr,
			DBIs.data(),
			nullptr
		},
		{
			VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
			nullptr,
			DescriptorSets[0], 1, 0,
			static_cast<uint32_t>(DIIs.size()),
			VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			DIIs.data(),
			nullptr,
			nullptr
		}
	}};

	const std::array<VkCopyDescriptorSet, 0> CDSs = {};

	vkUpdateDescriptorSets(Device,
		static_cast<uint32_t>(WDSs.size()), WDSs.data(),
		static_cast<uint32_t>(CDSs.size()), CDSs.data());

	LogOK("UpdateDescriptorSet");
}
void NormalMapVK::PopulateCommandBuffer(const size_t i)
{
	const auto CB = CommandPools[0].second[i];//CommandBuffers[i];
	//const auto SCB = SecondaryCommandBuffers[i];
	const auto FB = Framebuffers[i];
	const auto Image = SwapchainImages[i];

	const VkCommandBufferBeginInfo BeginInfo = {
		VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		nullptr,
		0,
		nullptr
	};
	VERIFY_SUCCEEDED(vkBeginCommandBuffer(CB, &BeginInfo)); {
		vkCmdSetViewport(CB, 0, static_cast<uint32_t>(Viewports.size()), Viewports.data());
		vkCmdSetScissor(CB, 0, static_cast<uint32_t>(ScissorRects.size()), ScissorRects.data());

		ClearColor(CB, Image, Colors::SkyBlue);

		const VkRenderPassBeginInfo RenderPassBeginInfo = {
			VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
			nullptr,
			RenderPass,
			FB,
			ScissorRects[0],
			0, nullptr
		};
		vkCmdBeginRenderPass(CB, &RenderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE); {
			
			if (!DescriptorSets.empty()) {
				vkCmdBindDescriptorSets(CB,
					VK_PIPELINE_BIND_POINT_GRAPHICS,
					PipelineLayout,
					0, static_cast<uint32_t>(DescriptorSets.size()), DescriptorSets.data(),
					0, nullptr);
			}

			vkCmdBindPipeline(CB, VK_PIPELINE_BIND_POINT_GRAPHICS, Pipeline);

			vkCmdDrawIndirect(CB, IndirectBuffer, 0, 1, 0);

		} vkCmdEndRenderPass(CB);
	} VERIFY_SUCCEEDED(vkEndCommandBuffer(CB));
}
#pragma endregion