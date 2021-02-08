// RayTracingDX.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "RayTracingDX.h"

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
    LoadStringW(hInstance, IDC_RAYTRACINGDX, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_RAYTRACINGDX));

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
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_RAYTRACINGDX));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_RAYTRACINGDX);
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
			Inst = new RayTracingDX();
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

#ifdef USE_RAYTRACING
void RayTracingDX::CreateGeometry()
{
	COM_PTR<ID3D12Device5> Device5;
	VERIFY_SUCCEEDED(Device->QueryInterface(COM_PTR_UUIDOF_PUTVOID(Device5)));

    COM_PTR<ID3D12GraphicsCommandList4> GCL4;
	VERIFY_SUCCEEDED(COM_PTR_GET(GraphicsCommandLists[0])->QueryInterface(COM_PTR_UUIDOF_PUTVOID(GCL4)));

#pragma region BLAS
    {
        //!< バーテックスバッファ (VertexBuffer)
        constexpr std::array Vertices = { DirectX::XMFLOAT3({ 0.0f, 0.5f, 0.0f }), DirectX::XMFLOAT3({ -0.5f, -0.5f, 0.0f }), DirectX::XMFLOAT3({ 0.5f, -0.5f, 0.0f }), };
		Buffer VB;
        VB.Create(COM_PTR_GET(Device), sizeof(Vertices), D3D12_HEAP_TYPE_UPLOAD, data(Vertices));

        //!< インデックスバッファ (IndexBuffer)
        constexpr std::array Indices = { UINT32(0), UINT32(1), UINT32(2) };
        Buffer IB;
		IB.Create(COM_PTR_GET(Device), sizeof(Indices), D3D12_HEAP_TYPE_UPLOAD, data(Indices));

        //!< ジオメトリ (Geometry)
        const std::array RGDs = {
            D3D12_RAYTRACING_GEOMETRY_DESC({
                .Type = D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES,
                .Flags = D3D12_RAYTRACING_GEOMETRY_FLAG_OPAQUE,
                .Triangles = D3D12_RAYTRACING_GEOMETRY_TRIANGLES_DESC({
                    .Transform3x4 = 0,
                    .IndexFormat = DXGI_FORMAT_R32_UINT,
                    .VertexFormat = DXGI_FORMAT_R32G32B32_FLOAT,
                    .IndexCount = static_cast<UINT>(size(Indices)),
                    .VertexCount = static_cast<UINT>(size(Vertices)),
                    .IndexBuffer = COM_PTR_GET(IB.Resource)->GetGPUVirtualAddress(),
                    .VertexBuffer = COM_PTR_GET(VB.Resource)->GetGPUVirtualAddress(),
                })
            }),
        };
        //!< インプット (Input)
        const D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS BRASI = {
            .Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL,
            .Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE,
            .NumDescs = static_cast<UINT>(size(RGDs)),
            .DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY,
            .pGeometryDescs = data(RGDs),
        };

        //!< サイズ取得 (Get sizes)
        D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO RASPI;
		Device5->GetRaytracingAccelerationStructurePrebuildInfo(&BRASI, &RASPI);

		//!< AS作成 (Create AS)
		BLASs.emplace_back().Create(COM_PTR_GET(Device), RASPI.ResultDataMaxSizeInBytes);
        //!< ASビルド (Build AS)
        UAVBuffer SB;
		SB.Create(COM_PTR_GET(Device), RASPI.ScratchDataSizeInBytes);
        const D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC BRASD = {
            .DestAccelerationStructureData = COM_PTR_GET(BLASs.back().Resource)->GetGPUVirtualAddress(),
	        .Inputs = BRASI,
	        .SourceAccelerationStructureData = 0,
	        .ScratchAccelerationStructureData = COM_PTR_GET(SB.Resource)->GetGPUVirtualAddress()
        };
		GCL4->BuildRaytracingAccelerationStructure(&BRASD, 0, nullptr);
    }
#pragma endregion

#pragma region TLAS
    {
        //!< インスタンスバッファ (InstanceBuffer)
        const std::array RIDs = {
            D3D12_RAYTRACING_INSTANCE_DESC({
                .Transform = {{ 1.0f, 0.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f, 0.0f }, {0.0f, 0.0f, 1.0f, 0.0f}},
                .InstanceID = 0,
                .InstanceMask = 0xff,
                .InstanceContributionToHitGroupIndex = 0,
                .Flags = D3D12_RAYTRACING_INSTANCE_FLAG_TRIANGLE_CULL_DISABLE,
                .AccelerationStructure = COM_PTR_GET(BLASs.back().Resource)->GetGPUVirtualAddress()
            })
        };
        Buffer IB;
		IB.Create(COM_PTR_GET(Device), sizeof(RIDs), D3D12_HEAP_TYPE_UPLOAD, data(RIDs));

        //!< インプット (Input)
		const D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS BRASI = {
			.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL,
			.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE,
			.NumDescs = static_cast<UINT>(size(RIDs)),
			.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY,
            .InstanceDescs = COM_PTR_GET(IB.Resource)->GetGPUVirtualAddress()
		};

        //!< サイズ取得 (Get sizes)
		D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO RASPI;
		Device5->GetRaytracingAccelerationStructurePrebuildInfo(&BRASI, &RASPI);

		//!< AS作成 (Create AS)
		TLASs.emplace_back().Create(COM_PTR_GET(Device), RASPI.ResultDataMaxSizeInBytes);
		//!< ASビルド (Build AS)
	    UAVBuffer SB;
		SB.Create(COM_PTR_GET(Device), RASPI.ScratchDataSizeInBytes);
		const D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC BRASD = {
			.DestAccelerationStructureData = COM_PTR_GET(TLASs.back().Resource)->GetGPUVirtualAddress(),
			.Inputs = BRASI,
			.SourceAccelerationStructureData = 0,
			.ScratchAccelerationStructureData = COM_PTR_GET(SB.Resource)->GetGPUVirtualAddress()
		};
		GCL4->BuildRaytracingAccelerationStructure(&BRASD, 0, nullptr);
    }
#pragma endregion
}
void RayTracingDX::CreatePipelineState()
{

}
#endif
