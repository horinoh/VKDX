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

void GltfVK::LoadScene()
{
	//!< POS, NRM
	//Load("..\\..\\glTF-Sample-Models\\2.0\\CesiumMilkTruck\\glTF-Binary\\CesiumMilkTruck.glb"); 
	//Load("..\\..\\glTF-Sample-Models\\2.0\\2CylinderEngine\\glTF-Binary\\2CylinderEngine.glb");
	
	//!< POS, NRM, TEX0
	Load("..\\..\\glTF-Sample-Models\\2.0\\Duck\\glTF-Binary\\Duck.glb");
	//Load("..\\..\\glTF-Sample-Models\\2.0\\Duck\\glTF-Embedded\\Duck.gltf");
	//Load("..\\..\\glTF-Sample-Models\\2.0\\BoxTextured\\glTF-Binary\\BoxTextured.glb");

	//!< POS, NRM, TEX0, COL0
	//Load("..\\..\\glTF-Sample-Models\\2.0\\BoxVertexColors\\glTF-Binary\\BoxVertexColors.glb");

	//!< POS, NRM, TAN, TEX0
	//Load("..\\..\\glTF-Sample-Models\\2.0\\SciFiHelmet\\glTF\\SciFiHelmet.gltf"); 
	//Load("..\\..\\glTF-Sample-Models\\2.0\\Suzanne\\glTF\\Suzanne.gltf");
	//Load("..\\..\\glTF-Sample-Models\\2.0\\WaterBottle\\glTF-Binary\\WaterBottle.glb"); 

	//!< POS, NRM, JNT0, WEG0
	//Load("..\\..\\glTF-Sample-Models\\2.0\\BrainStem\\glTF-Binary\\BrainStem.glb");

	//!< POS, NRM, TEX0, JNT0, WEG0
	//Load("..\\..\\glTF-Sample-Models\\2.0\\CesiumMan\\glTF-Binary\\CesiumMan.glb");
	//Load("..\\..\\glTF-Sample-Models\\2.0\\Monster\\glTF-Binary\\Monster.glb");
}
void GltfVK::Process(const fx::gltf::Primitive& Prim)
{
	Gltf::Process(Prim);

	std::vector<VkVertexInputBindingDescription> VIBDs;
	std::vector<VkVertexInputAttributeDescription> VIADs;
	uint32_t Binding = 0;
	uint32_t Location = 0;
	for (const auto& i : Prim.attributes) {
		const auto& Acc = Document.accessors[i.second];
		VIBDs.push_back({ Binding, GetTypeSize(Acc),  VK_VERTEX_INPUT_RATE_VERTEX });
		VIADs.push_back({ Location, Binding, ToVKFormat(Acc), 0 });
		++Binding;
		++Location;
	}

	CreateShaderModle_VsFs(); 

	const auto RP = RenderPasses[0];
	const auto PLL = PipelineLayouts[0];
	Pipelines.push_back(VkPipeline());
	CreatePipeline(Pipelines[0], PLL, RP, ShaderModules[0], ShaderModules[1], NullShaderModule, NullShaderModule, NullShaderModule, VIBDs, VIADs, ToVKPrimitiveTopology(Prim.mode));

	const auto& VBs = VertexBuffers;
	const auto IB = IndexBuffers[0];
	const auto IndB = IndirectBuffers[0];
	const auto PL = Pipelines[0];
	const std::vector<VkDeviceSize> Offsets(VBs.size(), 0);
	for (auto i = 0; i < SecondaryCommandBuffers.size(); ++i) {
		const auto SCB = SecondaryCommandBuffers[i];
		const auto FB = Framebuffers[i];
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
			vkCmdBindVertexBuffers(SCB, 0, static_cast<uint32_t>(VBs.size()), VBs.data(), Offsets.data());
			vkCmdBindIndexBuffer(SCB, IB, 0, ToVKIndexType(Document.accessors[Prim.indices].componentType));
			vkCmdDrawIndexedIndirect(SCB, IndB, 0, 1, 0);
		} VERIFY_SUCCEEDED(vkEndCommandBuffer(SCB));
	}
}
void GltfVK::Process(const fx::gltf::Accessor& Acc)
{
	Gltf::Process(Acc);

	if (-1 != Acc.bufferView) {
		const auto& BufV = Document.bufferViews[Acc.bufferView];

		if (-1 != BufV.buffer) {
			const auto& Buf = Document.buffers[BufV.buffer];

			const auto Data = &Buf.data[BufV.byteOffset + Acc.byteOffset];
			const auto Stride = BufV.byteStride;
			const auto TypeSize = GetTypeSize(Acc);
			const auto Size = Acc.count * (0 == Stride ? TypeSize : Stride);

			if (fx::gltf::BufferView::TargetType::ElementArrayBuffer == BufV.target) {
				IndexBuffers.push_back(VkBuffer());
				CreateBuffer_Index(GraphicsQueue, CommandBuffers[0], &IndexBuffers.back(), Size, Data);

				CreateIndirectBuffer_DrawIndexed(Acc.count, 1);
			}
			else if (fx::gltf::BufferView::TargetType::ArrayBuffer == BufV.target) {
				VertexBuffers.push_back(VkBuffer());
				CreateBuffer_Vertex(GraphicsQueue, CommandBuffers[0], &VertexBuffers.back(), Size, Data);
			}
		}
	}
}

void GltfVK::PopulateCommandBuffer(const size_t i)
{
	const auto RP = RenderPasses[0];

	const auto CB = CommandBuffers[i];
	const auto FB = Framebuffers[i];

	const auto SCB = SecondaryCommandBuffers[i];

	const VkCommandBufferBeginInfo CBBI = {
		VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		nullptr,
		0,
		nullptr
	};
	VERIFY_SUCCEEDED(vkBeginCommandBuffer(CB, &CBBI)); {
		const std::array<VkClearValue, 1> CVs = { Colors::SkyBlue };
		const VkRenderPassBeginInfo RPBI = {
			VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
			nullptr,
			RP,
			FB,
			ScissorRects[0],
			static_cast<uint32_t>(CVs.size()), CVs.data()
		};
		vkCmdBeginRenderPass(CB, &RPBI, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS); {
			const std::array<VkCommandBuffer, 1> SCBs = { SCB };
			vkCmdExecuteCommands(CB, static_cast<uint32_t>(SCBs.size()), SCBs.data());
		} vkCmdEndRenderPass(CB);
	} VERIFY_SUCCEEDED(vkEndCommandBuffer(CB));
}
#pragma endregion //!< Code