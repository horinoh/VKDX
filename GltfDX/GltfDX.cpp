// GltfDX.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "GltfDX.h"

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
    LoadStringW(hInstance, IDC_GLTFDX, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_GLTFDX));

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
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_GLTFDX));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_GLTFDX);
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
			Inst = new GltfDX();
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
void GltfDX::LoadScene()
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
void GltfDX::Process(const fx::gltf::Primitive& Prim)
{
	Gltf::Process(Prim);

	std::vector<std::pair<std::string, UINT>> SemanticIndices;
	for (const auto& i : Prim.attributes) {
		std::string Name, Index;
		if (DecomposeSemantic(i.first, Name, Index)) {
			SemanticIndices.push_back({ Name.c_str(), std::stoi(Index) });
		}
		else {
			SemanticIndices.push_back({ i.first.c_str(), 0 });
		}
	}

	std::vector<D3D12_INPUT_ELEMENT_DESC> IEDs;
	UINT InputSlot = 0;
	for (const auto& i : Prim.attributes) {
		const auto& Sem = SemanticIndices[InputSlot];
		const auto& Acc = Document.accessors[i.second];
		IEDs.push_back({ Sem.first.c_str(), Sem.second, ToDXFormat(Acc), InputSlot, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
		++InputSlot;
	}

	CreateShaderBlob_VsPs();
#if 0
	CreatePipelineState_VsPs_Vertex<Vertex_PositionNormalTexcoord>();
#else
	const auto RS = COM_PTR_GET(RootSignatures[0]);

	PipelineStates.push_back(COM_PTR<ID3D12PipelineState>());
	DX::CreatePipelineState(std::ref(PipelineStates[0]), RS, { ShaderBlobs[0]->GetBufferPointer(), ShaderBlobs[0]->GetBufferSize() }, { ShaderBlobs[1]->GetBufferPointer(), ShaderBlobs[1]->GetBufferSize() }, NullShaderBC, NullShaderBC, NullShaderBC, IEDs, ToDXPrimitiveTopologyType(Prim.mode));
#endif

	const auto BCA = COM_PTR_GET(BundleCommandAllocators[0]);
	const auto PS = COM_PTR_GET(PipelineStates[0]);

	const auto& VBVs = VertexBufferViews;
	const auto & IBV = IndexBufferViews[0];
	const auto IBR = COM_PTR_GET(IndirectBufferResources[0]);
	const auto ICS = COM_PTR_GET(IndirectCommandSignatures[0]);

	for (auto i = 0; i < BundleGraphicsCommandLists.size(); ++i) {
		const auto BCL = COM_PTR_GET(BundleGraphicsCommandLists[i]);
		const auto SCH = GetCPUDescriptorHandle(COM_PTR_GET(SwapChainDescriptorHeap), D3D12_DESCRIPTOR_HEAP_TYPE_RTV, static_cast<UINT>(i));

		VERIFY_SUCCEEDED(BCL->Reset(BCA, PS));
		{
			BCL->SetGraphicsRootSignature(RS);
			BCL->IASetPrimitiveTopology(ToDXPrimitiveTopology(Prim.mode));
			BCL->IASetVertexBuffers(0, static_cast<UINT>(VBVs.size()), VBVs.data());
			BCL->IASetIndexBuffer(&IBV);
			BCL->ExecuteIndirect(ICS, 1, IBR, 0, nullptr, 0);
		}
		VERIFY_SUCCEEDED(BCL->Close());
	}
}
void GltfDX::Process(const fx::gltf::Accessor& Acc)
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
				IndexBufferResources.push_back(COM_PTR<ID3D12Resource>());

				CreateBuffer(COM_PTR_PUT(IndexBufferResources.back()), Size, Data, COM_PTR_GET(CommandAllocators[0]), COM_PTR_GET(GraphicsCommandLists[0]));
				IndexBufferViews.push_back({ IndexBufferResources.back()->GetGPUVirtualAddress(), Size, ToDXFormat(Acc.componentType) });

				CreateIndirectBuffer_DrawIndexed(Acc.count, 1);
			}
			else if (fx::gltf::BufferView::TargetType::ArrayBuffer == BufV.target) {
				VertexBufferResources.push_back(COM_PTR<ID3D12Resource>());

				CreateBuffer(COM_PTR_PUT(VertexBufferResources.back()), Size, Data, COM_PTR_GET(CommandAllocators[0]), COM_PTR_GET(GraphicsCommandLists[0]));
				VertexBufferViews.push_back({ VertexBufferResources.back()->GetGPUVirtualAddress(), Size, Stride });
			}
		}
	}
}

void GltfDX::PopulateCommandList(const size_t i)
{
	const auto CA = COM_PTR_GET(CommandAllocators[0]);
	const auto CL = COM_PTR_GET(GraphicsCommandLists[i]);
	const auto BCL = COM_PTR_GET(BundleGraphicsCommandLists[i]);
	const auto SCR = COM_PTR_GET(SwapChainResources[i]);
	const auto SCH = GetCPUDescriptorHandle(COM_PTR_GET(SwapChainDescriptorHeap), D3D12_DESCRIPTOR_HEAP_TYPE_RTV, static_cast<UINT>(i));

	VERIFY_SUCCEEDED(CL->Reset(CA, nullptr));
	{
		CL->RSSetViewports(static_cast<UINT>(Viewports.size()), Viewports.data());
		CL->RSSetScissorRects(static_cast<UINT>(ScissorRects.size()), ScissorRects.data());

		ResourceBarrier(CL, SCR, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
		{
			const std::array<D3D12_RECT, 0> Rs = {};
			CL->ClearRenderTargetView(SCH, DirectX::Colors::SkyBlue, static_cast<UINT>(Rs.size()), Rs.data());

			const std::array<D3D12_CPU_DESCRIPTOR_HANDLE, 1> RTDHs = { SCH };
			CL->OMSetRenderTargets(static_cast<UINT>(RTDHs.size()), RTDHs.data(), FALSE, nullptr);

			CL->ExecuteBundle(BCL);
		}
		ResourceBarrier(CL, SCR, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
	}
	VERIFY_SUCCEEDED(CL->Close());
}
#pragma endregion