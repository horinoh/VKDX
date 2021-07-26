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
void RayTracingDX::CreateGeometry()
{
	if (!HasRaytracingSupport(COM_PTR_GET(Device))) { return; }

	COM_PTR<ID3D12Device5> Device5;
	VERIFY_SUCCEEDED(Device->QueryInterface(COM_PTR_UUIDOF_PUTVOID(Device5)));

	const auto GCL = COM_PTR_GET(GraphicsCommandLists[0]);
    const auto CA = COM_PTR_GET(CommandAllocators[0]);
    const auto GCQ = COM_PTR_GET(GraphicsCommandQueue);

#pragma region BLAS
    {
        //!< バーテックスバッファ (VertexBuffer) ... ByteAddressBufferとして利用できるようにする
        constexpr std::array Vertices = { DirectX::XMFLOAT3({ 0.0f, 0.5f, 0.0f }), DirectX::XMFLOAT3({ -0.5f, -0.5f, 0.0f }), DirectX::XMFLOAT3({ 0.5f, -0.5f, 0.0f }), };
		ResourceBase VB;
        VB.Create(COM_PTR_GET(Device), sizeof(Vertices), D3D12_HEAP_TYPE_UPLOAD, data(Vertices));
#if 1
        const auto VBSRV = D3D12_SHADER_RESOURCE_VIEW_DESC({
            .Format = DXGI_FORMAT_R32_TYPELESS,
            .ViewDimension = D3D12_SRV_DIMENSION_BUFFER,
            .Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING,
            .Buffer = D3D12_BUFFER_SRV({
                .FirstElement = 0,.NumElements = size(Vertices), .StructureByteStride = 0,.Flags = D3D12_BUFFER_SRV_FLAG_RAW,
            }),
        });
#endif

        //!< インデックスバッファ (IndexBuffer) ... ByteAddressBufferとして利用できるようにする
        constexpr std::array Indices = { UINT32(0), UINT32(1), UINT32(2) };
        ResourceBase IB;
		IB.Create(COM_PTR_GET(Device), sizeof(Indices), D3D12_HEAP_TYPE_UPLOAD, data(Indices));
#if 1
		const auto IBSRV = D3D12_SHADER_RESOURCE_VIEW_DESC({
		   .Format = DXGI_FORMAT_R32_TYPELESS,
		   .ViewDimension = D3D12_SRV_DIMENSION_BUFFER,
		   .Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING,
		   .Buffer = D3D12_BUFFER_SRV({
			   .FirstElement = 0,.NumElements = size(Indices), .StructureByteStride = 0,.Flags = D3D12_BUFFER_SRV_FLAG_RAW,
		    }),
		});
#endif
  
#pragma region AS
		{
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
						.IndexBuffer = IB.Resource->GetGPUVirtualAddress(),
						.VertexBuffer = VB.Resource->GetGPUVirtualAddress(),
                    })
                }),
            };
            //!< インプット (Input)
            const D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS BRASI = {
                .Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL, //!< ボトムレベル
                .Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE/*D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_NONE*/,
                .NumDescs = static_cast<UINT>(size(RGDs)),
                .DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY,
                .pGeometryDescs = data(RGDs), //!< ジオメトリ(トライアングル)
            };

            //!< サイズ取得 (Get sizes)
            D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO RASPI;
            Device5->GetRaytracingAccelerationStructurePrebuildInfo(&BRASI, &RASPI);

			//!< AS作成、ビルド (Create and build AS)
            BLASs.emplace_back().Create(COM_PTR_GET(Device), RASPI.ResultDataMaxSizeInBytes).ExecuteBuildCommand(COM_PTR_GET(Device), RASPI.ScratchDataSizeInBytes, BRASI, GCL, CA, GCQ, COM_PTR_GET(Fence));
        }
#pragma endregion
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
                .Flags = D3D12_RAYTRACING_INSTANCE_FLAG_TRIANGLE_CULL_DISABLE/*D3D12_RAYTRACING_INSTANCE_FLAG_NONE*/,
				.AccelerationStructure = BLASs.back().Resource->GetGPUVirtualAddress()
            })
        };
        ResourceBase IB;
		IB.Create(COM_PTR_GET(Device), sizeof(RIDs), D3D12_HEAP_TYPE_UPLOAD, data(RIDs));
#if 1
		const auto IBSRV = D3D12_SHADER_RESOURCE_VIEW_DESC({
		   .Format = DXGI_FORMAT_R32_TYPELESS,
		   .ViewDimension = D3D12_SRV_DIMENSION_BUFFER,
		   .Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING,
		   .Buffer = D3D12_BUFFER_SRV({
			   .FirstElement = 0,.NumElements = 1, .StructureByteStride = 0,.Flags = D3D12_BUFFER_SRV_FLAG_RAW,
			}),
		});
#endif

#pragma region AS
        {
            //!< インプット (Input)
            const D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS BRASI = {
                .Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL, //!< トップレベル
                .Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE/*D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_NONE*/,
                .NumDescs = static_cast<UINT>(size(RIDs)),
                .DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY,
				.InstanceDescs = IB.Resource->GetGPUVirtualAddress() //!< インスタンス
            };

            //!< サイズ取得 (Get sizes)
            D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO RASPI;
            Device5->GetRaytracingAccelerationStructurePrebuildInfo(&BRASI, &RASPI);

			//!< AS作成、ビルド (Create and build AS)
            TLASs.emplace_back().Create(COM_PTR_GET(Device), RASPI.ResultDataMaxSizeInBytes).ExecuteBuildCommand(COM_PTR_GET(Device), RASPI.ScratchDataSizeInBytes, BRASI, GCL, CA, GCQ, COM_PTR_GET(Fence));
        }
#pragma endregion
    }
#pragma endregion
}
void RayTracingDX::CreatePipelineState()
{
	if (!HasRaytracingSupport(COM_PTR_GET(Device))) { return; }

#pragma region STATE_OBJECT
	//!< グローバルルートシグネチャ (Global root signature)
    const D3D12_GLOBAL_ROOT_SIGNATURE GRS = { .pGlobalRootSignature = COM_PTR_GET(RootSignatures[0]) };

	//!< ローカルルートシグネチャ (Local root signature)
	//const D3D12_LOCAL_ROOT_SIGNATURE LRS = { .pLocalRootSignature = COM_PTR_GET(RootSignatures[1]) };

    //!< ライブラリ
	const auto ShaderPath = GetBasePath();
	COM_PTR<ID3DBlob> SB;
	VERIFY_SUCCEEDED(D3DReadFileToBlob(data(ShaderPath + TEXT(".rts.cso")), COM_PTR_PUT(SB))); 
	std::array EDs = {
	   D3D12_EXPORT_DESC({.Name = TEXT("OnRayGeneration"), .ExportToRename = nullptr, .Flags = D3D12_EXPORT_FLAG_NONE }),
	   D3D12_EXPORT_DESC({.Name = TEXT("OnClosestHit"), .ExportToRename = nullptr, .Flags = D3D12_EXPORT_FLAG_NONE }),
	   D3D12_EXPORT_DESC({.Name = TEXT("OnMiss"), .ExportToRename = nullptr, .Flags = D3D12_EXPORT_FLAG_NONE }),
	};
    const D3D12_DXIL_LIBRARY_DESC DLD = {
        .DXILLibrary = D3D12_SHADER_BYTECODE({.pShaderBytecode = SB->GetBufferPointer(), .BytecodeLength = SB->GetBufferSize() }),
        .NumExports = static_cast<UINT>(size(EDs)), .pExports = data(EDs) 
    };

    //!< ヒットグループ AnyHit, ClosestHit, Intersection の3つからなる、ここでは D3D12_HIT_GROUP_TYPE_TRIANGLES なので、ClosestHit のみを使用
    //!< ヒットグループ内のシェーダは同じローカルルートシグネチャを使用する 
    constexpr D3D12_HIT_GROUP_DESC HGD = {
        .HitGroupExport = TEXT("MyHitGroup"),
        .Type = D3D12_HIT_GROUP_TYPE_TRIANGLES,
	    .AnyHitShaderImport = nullptr, .ClosestHitShaderImport = TEXT("OnClosestHit"), .IntersectionShaderImport = nullptr
    };

    //!< シェーダ内、ペイロードやアトリビュートサイズの指定
    constexpr D3D12_RAYTRACING_SHADER_CONFIG RSC = {
		.MaxPayloadSizeInBytes = sizeof(DirectX::XMFLOAT4), //!< Payload のサイズ ここでは struct Payload { float3 Color; } を使用するため XMFLOAT3
	    .MaxAttributeSizeInBytes = sizeof(DirectX::XMFLOAT2) //!< ここでは struct BuiltInTriangleIntersectionAttributes { float2 barycentrics; } を使用するため XMFLOAT2
    };

    //!< レイトレーシング再帰呼び出し可能な段数
    constexpr D3D12_RAYTRACING_PIPELINE_CONFIG RPC = { .MaxTraceRecursionDepth = 1 };

    constexpr std::array SSs = {
		D3D12_STATE_SUBOBJECT({.Type = D3D12_STATE_SUBOBJECT_TYPE_GLOBAL_ROOT_SIGNATURE, .pDesc = &GRS }),
		//D3D12_STATE_SUBOBJECT({.Type = D3D12_STATE_SUBOBJECT_TYPE_LOCAL_ROOT_SIGNATURE, .pDesc = &LRS }),
		//D3D12_STATE_SUBOBJECT({.Type = D3D12_STATE_SUBOBJECT_TYPE_SUBOBJECT_TO_EXPORTS_ASSOCIATION, .pDesc = &STEA }),
		D3D12_STATE_SUBOBJECT({.Type = D3D12_STATE_SUBOBJECT_TYPE_DXIL_LIBRARY, .pDesc = &DLD }),
		D3D12_STATE_SUBOBJECT({.Type = D3D12_STATE_SUBOBJECT_TYPE_HIT_GROUP, .pDesc = &HGD }),
		D3D12_STATE_SUBOBJECT({.Type = D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_SHADER_CONFIG, .pDesc = &RSC }),
		D3D12_STATE_SUBOBJECT({.Type = D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_PIPELINE_CONFIG, .pDesc = &RPC }),
    };
    constexpr D3D12_STATE_OBJECT_DESC SOD = {
        .Type = D3D12_STATE_OBJECT_TYPE_RAYTRACING_PIPELINE,
	    .NumSubobjects = static_cast<UINT>(size(SSs)), .pSubobjects = data(SSs)
    };
    COM_PTR<ID3D12Device5> Device5;
    VERIFY_SUCCEEDED(Device->QueryInterface(COM_PTR_UUIDOF_PUTVOID(Device5)));
    VERIFY_SUCCEEDED(Device5->CreateStateObject(&SOD, COM_PTR_UUIDOF_PUTVOID(StateObjects.emplace_back())));
#pragma endregion

#pragma region SHADER_TABLE
	COM_PTR<ID3D12StateObjectProperties> SOP;
	VERIFY_SUCCEEDED(StateObjects.back()->QueryInterface(COM_PTR_UUIDOF_PUTVOID(SOP)));

	const auto RaygenAlignedSize = Cmn::RoundUp(1 * Cmn::RoundUp(D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES, D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT), D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT);
	const auto MissAlignedSize = Cmn::RoundUp(1 * Cmn::RoundUp(D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES, D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT), D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT);
	const auto HitAlienedSize = Cmn::RoundUp(1 * Cmn::RoundUp(D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES, D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT), D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT);
	const auto TotalSize = RaygenAlignedSize + MissAlignedSize + HitAlienedSize;

	//!< #DX_TODO 1つのバッファにまとめる?
	ShaderTables.emplace_back().Create(COM_PTR_GET(Device), RaygenAlignedSize, D3D12_HEAP_TYPE_UPLOAD, SOP->GetShaderIdentifier(TEXT("OnRayGeneration")));
	ShaderTables.emplace_back().Create(COM_PTR_GET(Device), MissAlignedSize, D3D12_HEAP_TYPE_UPLOAD, SOP->GetShaderIdentifier(TEXT("OnMiss")));
	ShaderTables.emplace_back().Create(COM_PTR_GET(Device), HitAlienedSize, D3D12_HEAP_TYPE_UPLOAD, SOP->GetShaderIdentifier(TEXT("MyHitGroup")));
#pragma endregion
}
void RayTracingDX::PopulateCommandList(const size_t i)
{
	if (!HasRaytracingSupport(COM_PTR_GET(Device))) { return; }

    const auto GCL = GraphicsCommandLists[i];

    GCL->SetComputeRootSignature(COM_PTR_GET(RootSignatures[0]));
    {
        const auto& DH = CbvSrvUavDescriptorHeaps[0];
		auto GDH = DH->GetGPUDescriptorHandleForHeapStart();
        GCL->SetComputeRootDescriptorTable(0, GDH); //!< TLAS
		GDH.ptr += Device->GetDescriptorHandleIncrementSize(DH->GetDesc().Type);
        GCL->SetComputeRootDescriptorTable(1, GDH); //!< UAV
    }

    const auto UAV = COM_PTR_GET(UnorderedAccessTextures[0].Resource);
	ResourceBarrier(COM_PTR_GET(GCL), UAV, D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

    //!< バリア
    
    COM_PTR<ID3D12GraphicsCommandList4> GCL4;
	VERIFY_SUCCEEDED(GCL->QueryInterface(COM_PTR_UUIDOF_PUTVOID(GCL4)));
	GCL4->SetPipelineState1(COM_PTR_GET(StateObjects[0]));
	const auto DRD = D3D12_DISPATCH_RAYS_DESC({
	  .RayGenerationShaderRecord = D3D12_GPU_VIRTUAL_ADDRESS_RANGE({.StartAddress = ShaderTables[0].Resource->GetGPUVirtualAddress(), .SizeInBytes = 1 * Cmn::RoundUp(D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES, D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT) }),
	  .MissShaderTable = D3D12_GPU_VIRTUAL_ADDRESS_RANGE_AND_STRIDE({.StartAddress = ShaderTables[1].Resource->GetGPUVirtualAddress(), .SizeInBytes = 1 * Cmn::RoundUp(D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES, D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT), .StrideInBytes = 0}),
	  .HitGroupTable = D3D12_GPU_VIRTUAL_ADDRESS_RANGE_AND_STRIDE({.StartAddress = ShaderTables[2].Resource->GetGPUVirtualAddress(), .SizeInBytes = 1 * Cmn::RoundUp(D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES, D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT), .StrideInBytes = 0}),
	  .CallableShaderTable = D3D12_GPU_VIRTUAL_ADDRESS_RANGE_AND_STRIDE({.StartAddress = D3D12_GPU_VIRTUAL_ADDRESS(0), .SizeInBytes = 0, .StrideInBytes = 0}),
	  .Width = 1280,.Height = 720, .Depth = 1
	});
    GCL4->DispatchRays(&DRD);

	const auto SCR = COM_PTR_GET(SwapChainResources[i]);

	ResourceBarrier(COM_PTR_GET(GCL), UAV, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_SOURCE);
	ResourceBarrier(COM_PTR_GET(GCL), SCR, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_COPY_DEST);

    //!< #DX_TODO コピー
    
	ResourceBarrier(COM_PTR_GET(GCL), SCR, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PRESENT);

    VERIFY_SUCCEEDED(GCL->Close());
}
#pragma endregion
