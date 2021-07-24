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
void RayTracingVK::CreateGeometry()
{
	if (!HasRayTracingSupport(GetCurrentPhysicalDevice())) { return; }

	const auto PDMP = GetCurrentPhysicalDeviceMemoryProperties();
    const auto& CB = CommandBuffers[0];

#pragma region BLAS
    {
        //!< バーテックスバッファ (VertexBuffer)
        constexpr std::array Vertices = { glm::vec3({ 0.0f, 0.5f, 0.0f }), glm::vec3({ -0.5f, -0.5f, 0.0f }), glm::vec3({ 0.5f, -0.5f, 0.0f }), };
        Scoped<BufferMemory> VB(Device);
        VB.Create(Device, PDMP, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, sizeof(Vertices), VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, data(Vertices));

        //!< インデックスバッファ (IndexBuffer)
        constexpr std::array Indices = { uint32_t(0), uint32_t(1), uint32_t(2) };
        Scoped<BufferMemory> IB(Device);
        IB.Create(Device, PDMP, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, sizeof(Indices), VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, data(Indices));

#pragma region AS
        {
            //!< ジオメトリ (Geometry)
            const std::vector ASGs = {
                VkAccelerationStructureGeometryKHR({
                    .sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR,
                    .pNext = nullptr,
                    .geometryType = VK_GEOMETRY_TYPE_TRIANGLES_KHR,
                    .geometry = VkAccelerationStructureGeometryDataKHR({
                        .triangles = VkAccelerationStructureGeometryTrianglesDataKHR({
                            .sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR,
                            .pNext = nullptr,
                            .vertexFormat = VK_FORMAT_R32G32B32_SFLOAT,
                            .vertexData = VkDeviceOrHostAddressConstKHR({.deviceAddress = GetDeviceAddress(Device, VB.Buffer)}), .vertexStride = sizeof(Vertices[0]), .maxVertex = static_cast<uint32_t>(size(Vertices)),
                            .indexType = VK_INDEX_TYPE_UINT32, .indexData = VkDeviceOrHostAddressConstKHR({.deviceAddress = GetDeviceAddress(Device, IB.Buffer)}),
                            .transformData = VkDeviceOrHostAddressConstKHR(),
                        }),
                    }),
                    .flags = VK_GEOMETRY_OPAQUE_BIT_KHR
                }),
            };

            //!< サイズ取得 (Get sizes)
            const VkAccelerationStructureBuildGeometryInfoKHR ASBGI = {
                .sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR,
                .pNext = nullptr,
                .type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR,
                .flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR,
                .mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR,
                .srcAccelerationStructure = VK_NULL_HANDLE, .dstAccelerationStructure = VK_NULL_HANDLE,
                .geometryCount = static_cast<uint32_t>(size(ASGs)),.pGeometries = data(ASGs), .ppGeometries = nullptr,
                .scratchData = VkDeviceOrHostAddressKHR()
            };
            constexpr uint32_t MaxPrimitiveCounts = 1;
            VkAccelerationStructureBuildSizesInfoKHR ASBSI;
            vkGetAccelerationStructureBuildSizesKHR(Device, VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR, &ASBGI, &MaxPrimitiveCounts, &ASBSI);

			//!< AS作成、ビルド (Create and build AS)
            BLASs.emplace_back().Create(Device, PDMP, ASBGI.type, ASBSI.accelerationStructureSize).SubmitBuildCommand(Device, PDMP, ASBGI.type, ASBSI.buildScratchSize, ASGs, GraphicsQueue, CB);
        }
#pragma endregion
    }
#pragma endregion

#pragma region TLAS
	{
		//!< インスタンスバッファ (InstanceBuffer)
		const VkAccelerationStructureInstanceKHR ASI = {
			.transform = VkTransformMatrixKHR({1.0f, 0.0f, 0.0f, 0.0f,
												0.0f, 1.0f, 0.0f, 0.0f,
												0.0f, 0.0f, 1.0f, 0.0f}),
			.instanceCustomIndex = 0,
			.mask = 0xff,
			.instanceShaderBindingTableRecordOffset = 0,
			.flags = VK_GEOMETRY_INSTANCE_TRIANGLE_FACING_CULL_DISABLE_BIT_KHR,
			.accelerationStructureReference = GetDeviceAddress(Device, BLASs.back().Buffer)
		};
        Scoped<BufferMemory> IB(Device);
        IB.Create(Device, PDMP, VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, sizeof(ASI), VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &ASI);

#pragma region AS
        {
            //!< ジオメトリ (Geometry)
            const std::vector ASGs = {
                VkAccelerationStructureGeometryKHR({
                    .sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR,
                    .pNext = nullptr,
                    .geometryType = VK_GEOMETRY_TYPE_INSTANCES_KHR,
                    .geometry = VkAccelerationStructureGeometryDataKHR({
                        .instances = VkAccelerationStructureGeometryInstancesDataKHR({
                            .sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR,
                            .pNext = nullptr,
                            .arrayOfPointers = VK_FALSE,
                            .data = VkDeviceOrHostAddressConstKHR({.deviceAddress = GetDeviceAddress(Device, IB.Buffer)})
                        })
                    }),
                    .flags = VK_GEOMETRY_OPAQUE_BIT_KHR
                }),
            };

            //!< サイズ取得 (Get sizes)
            const VkAccelerationStructureBuildGeometryInfoKHR ASBGI = {
                .sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR,
                .pNext = nullptr,
                .type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR,
                .flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR,
                .mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR,
                .srcAccelerationStructure = VK_NULL_HANDLE, .dstAccelerationStructure = VK_NULL_HANDLE,
                .geometryCount = static_cast<uint32_t>(size(ASGs)),.pGeometries = data(ASGs), .ppGeometries = nullptr,
                .scratchData = VkDeviceOrHostAddressKHR()
            };
            constexpr uint32_t MaxPrimitiveCounts = 1;
            VkAccelerationStructureBuildSizesInfoKHR ASBSI;
            vkGetAccelerationStructureBuildSizesKHR(Device, VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR, &ASBGI, &MaxPrimitiveCounts, &ASBSI);

			//!< AS作成、ビルド (Create and build AS)
            TLASs.emplace_back().Create(Device, PDMP, ASBGI.type, ASBSI.accelerationStructureSize).SubmitBuildCommand(Device, PDMP, ASBGI.type, ASBSI.buildScratchSize, ASGs, GraphicsQueue, CB);
        }
#pragma endregion
    }
#pragma endregion
}
void RayTracingVK::CreateTexture()
{
	if (!HasRayTracingSupport(GetCurrentPhysicalDevice())) { return; }
}
void RayTracingVK::CreatePipelineLayout()
{
	if (!HasRayTracingSupport(GetCurrentPhysicalDevice())) { return; }

	CreateDescriptorSetLayout(DescriptorSetLayouts.emplace_back(), 0, {
		//VkDescriptorSetLayoutBinding({.binding = 0, .descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, .descriptorCount = 1, .stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR, .pImmutableSamplers = nullptr }),
		//VkDescriptorSetLayoutBinding({.binding = 1, .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, .descriptorCount = 1, .stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR, .pImmutableSamplers = nullptr }),
    });
	VK::CreatePipelineLayout(PipelineLayouts.emplace_back(), DescriptorSetLayouts, {});
}
void RayTracingVK::CreatePipeline()
{
	if (!HasRayTracingSupport(GetCurrentPhysicalDevice())) { return; }

	const auto ShaderPath = GetBasePath();
    const std::array SMs = {
        VK::CreateShaderModule(data(ShaderPath + TEXT(".rgeb.spv"))),
        VK::CreateShaderModule(data(ShaderPath + TEXT(".rchit.spv"))),
        VK::CreateShaderModule(data(ShaderPath + TEXT(".rmiss.spv"))),
    };

#pragma region PIPELINE
    const auto PLL = PipelineLayouts.back();
    
	//!< 0:RayGen, 1:ClosestHit, 2:Miss, 
    const std::array PSSCIs = {
		VkPipelineShaderStageCreateInfo({.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, .pNext = nullptr, .flags = 0, .stage = VK_SHADER_STAGE_RAYGEN_BIT_KHR, .module = SMs[0], .pName = "main", .pSpecializationInfo = nullptr }),
		VkPipelineShaderStageCreateInfo({.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, .pNext = nullptr, .flags = 0, .stage = VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR, .module = SMs[1], .pName = "main", .pSpecializationInfo = nullptr }),
		VkPipelineShaderStageCreateInfo({.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, .pNext = nullptr, .flags = 0, .stage = VK_SHADER_STAGE_MISS_BIT_KHR, .module = SMs[2], .pName = "main", .pSpecializationInfo = nullptr }),
    };
	const std::array RTSGCIs = {
		VkRayTracingShaderGroupCreateInfoKHR({.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR, .pNext = nullptr, .type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR, .generalShader = 0, .closestHitShader = VK_SHADER_UNUSED_KHR, .anyHitShader = VK_SHADER_UNUSED_KHR, .intersectionShader = VK_SHADER_UNUSED_KHR, .pShaderGroupCaptureReplayHandle = nullptr }),
		VkRayTracingShaderGroupCreateInfoKHR({.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR, .pNext = nullptr, .type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR, .generalShader = VK_SHADER_UNUSED_KHR, .closestHitShader = 1, .anyHitShader = VK_SHADER_UNUSED_KHR, .intersectionShader = VK_SHADER_UNUSED_KHR, .pShaderGroupCaptureReplayHandle = nullptr }),
		VkRayTracingShaderGroupCreateInfoKHR({.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR, .pNext = nullptr, .type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR, .generalShader = 2, .closestHitShader = VK_SHADER_UNUSED_KHR, .anyHitShader = VK_SHADER_UNUSED_KHR, .intersectionShader = VK_SHADER_UNUSED_KHR, .pShaderGroupCaptureReplayHandle = nullptr }),
	};

	constexpr std::array DSs = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
	constexpr VkPipelineDynamicStateCreateInfo PDSCI = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
		.dynamicStateCount = static_cast<uint32_t>(size(DSs)), .pDynamicStates = data(DSs)
	};

	const std::array RTPCIs = {
		VkRayTracingPipelineCreateInfoKHR({
            .sType = VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_KHR,
            .pNext = nullptr,
            .flags = 0,
            .stageCount = static_cast<uint32_t>(size(PSSCIs)), .pStages = data(PSSCIs),
            .groupCount = static_cast<uint32_t>(size(RTSGCIs)), .pGroups = data(RTSGCIs),
            .maxPipelineRayRecursionDepth = 1,
            .pLibraryInfo = nullptr,
            .pLibraryInterface = nullptr,
            .pDynamicState = &PDSCI,
            .layout = PLL,
            .basePipelineHandle = VK_NULL_HANDLE, .basePipelineIndex = -1
	    }),
	};
	VkPipeline PL;
	VERIFY_SUCCEEDED(vkCreateRayTracingPipelinesKHR(Device, VK_NULL_HANDLE, VK_NULL_HANDLE, static_cast<uint32_t>(size(RTPCIs)), data(RTPCIs), GetAllocationCallbacks(), &PL));
#pragma endregion

#pragma region BINDING_TABLE
	VkPhysicalDeviceRayTracingPipelinePropertiesKHR PDRTPP = { .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_PROPERTIES_KHR, .pNext = nullptr };
	VkPhysicalDeviceProperties2 PDP2 = { .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2, .pNext = &PDRTPP, };
	vkGetPhysicalDeviceProperties2(GetCurrentPhysicalDevice(), &PDP2);

	const auto AlignedSize = RoundUp(PDRTPP.shaderGroupHandleSize, PDRTPP.shaderGroupHandleAlignment);
    constexpr auto Count = size(PSSCIs);
	std::vector<std::byte> Data(AlignedSize * Count);
	VERIFY_SUCCEEDED(vkGetRayTracingShaderGroupHandlesKHR(Device, Pipelines.back(), 0, Count, size(Data), data(Data)));

	const auto PDMP = GetCurrentPhysicalDeviceMemoryProperties();
	constexpr auto BUF = VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR | VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
	constexpr auto MPF = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	ShaderBindingTables.emplace_back().Create(Device, PDMP, BUF, PDRTPP.shaderGroupHandleSize, MPF, data(Data) + 0 * AlignedSize); //!< 0:RayGen
	ShaderBindingTables.emplace_back().Create(Device, PDMP, BUF, PDRTPP.shaderGroupHandleSize, MPF, data(Data) + 1 * AlignedSize); //!< 1:Miss
	ShaderBindingTables.emplace_back().Create(Device, PDMP, BUF, PDRTPP.shaderGroupHandleSize, MPF, data(Data) + 2 * AlignedSize); //!< 2:ClosestHit
#pragma endregion
}
void RayTracingVK::PopulateCommandBuffer([[maybe_unused]]const size_t i)
{
}
#pragma endregion
