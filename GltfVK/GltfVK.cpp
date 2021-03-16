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

#pragma region Code

void GltfVK::LoadScene()
{
	Unload();

	const std::string BasePath = "..\\fx-gltf\\test\\data\\";

	//!< PN(POS, NRM)
	//Load(BasePath + "glTF-Sample-Models\\2.0\\Box\\glTF-Binary\\Box.glb"); //!< Scale = 1.0f

	//!< PT(POS, TEX0)
	//!< KHR_texture_transform 拡張
	//Load(BasePath + "glTF-Sample-Models\\2.0\\TextureTransformTest\\glTF\\TextureTransformTest.gltf"); 

	//!< PNT(POS, NRM, TEX0)
	Load(BasePath + "glTF-Sample-Models\\2.0\\Duck\\glTF-Binary\\Duck.glb"); //!< Scale = 0.005f
	//Load(BasePath + "glTF-Sample-Models\\2.0\\DamagedHelmet\\glTF-Binary\\DamagedHelmet.glb"); //!< Scale = 0.5f
	//Load(BasePath + "glTF-Sample-Models\\2.0\\BoxTextured\\glTF-Binary\\BoxTextured.glb"); //!< Scale = 1.0f

	//!< TPN(TAN, POS, NRM)
	//!< モーフターゲット
	//Load(BasePath + "glTF-Sample-Models\\2.0\\AnimatedMorphCube\\glTF-Binary\\AnimatedMorphCube.glb");
	//Load(BasePath + "glTF-Sample-Models\\2.0\\AnimatedMorphSphere\\glTF-Binary\\AnimatedMorphSphere.glb");

	//!< CPNT(COL0, POS, NRM. TEX0)
	//Load(BasePath + "glTF-Sample-Models\\2.0\\BoxVertexColors\\glTF-Binary\\BoxVertexColors.glb"); //!< Scale = 1.0f

	//!< TPNT(TAN, POS, NRM, TEX0)
	//Load(BasePath + "glTF-Sample-Models\\2.0\\SciFiHelmet\\glTF\\SciFiHelmet.gltf"); //!< Scale = 0.5f
	//Load(BasePath + "glTF-Sample-Models\\2.0\\Suzanne\\glTF\\Suzanne.gltf"); //!< Scale = 0.5f
	 
	//!< JPNW(JNT0, POS, NRM, WGT0)
	//Load(BasePath + "glTF-Sample-Models\\2.0\\RiggedSimple\\glTF-Binary\\RiggedSimple.glb"); //!< Scale = 0.2f
	//Load(BasePath + "glTF-Sample-Models\\2.0\\RiggedFigure\\glTF-Binary\\RiggedFigure.glb"); //!< Scale = 0.5f

	//!< JPNTW(JNT0, POS, NRM, TEX0, WGT0)
	//Load(BasePath + "glTF-Sample-Models\\2.0\\CesiumMan\\glTF-Binary\\CesiumMan.glb"); //!< Scale = 0.5f
	//Load(BasePath + "glTF-Sample-Models\\2.0\\Monster\\glTF-Binary\\Monster.glb"); //!< Scale = 0.02f
}
void GltfVK::PreProcess()
{
	const auto Fov = 0.16f * std::numbers::pi_v<float>;
	const auto Aspect = GetAspectRatioOfClientRect();
	const auto ZFar = 100.0f;
	const auto ZNear = ZFar * 0.0001f;
	PV.Projection = glm::perspective(Fov, Aspect, ZNear, ZFar);

	const auto CamPos = glm::vec3(0.0f, 0.0f, 6.0f);
	const auto CamTag = glm::vec3(0.0f);
	const auto CamUp = glm::vec3(0.0f, 1.0f, 0.0f);
	PV.View = glm::lookAt(CamPos, CamTag, CamUp);

#if 0
	auto UBSize = sizeof(PV);

	//!< デスクリプタプール
	DescriptorPools.resize(1);
	VKExt::CreateDescriptorPool(DescriptorPools[0], 0, { { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1 } });

	//!< デスクリプタセット
	assert(!empty(DescriptorSetLayouts) && "");
	const std::array DSLs = { DescriptorSetLayouts[0] };
	assert(!empty(DescriptorPools) && "");
	const VkDescriptorSetAllocateInfo DSAI = {
		VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
		nullptr,
		DescriptorPools[0],
		static_cast<uint32_t>(size(DSLs)), data(DSLs)
	};
	DescriptorSets.resize(1);
	VERIFY_SUCCEEDED(vkAllocateDescriptorSets(Device, &DSAI, &DescriptorSets[0]));

	//!< ユニフォームバッファ
	//UniformBuffers.oush_back(UniformBuffer());
	//CreateBuffer(&UniformBuffers.back().Buffer, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, sizeof(UBSize));
	//AllocateDeviceMemory(&UniformBuffers.back().DeviceMemory, UniformBuffers.back().Buffer, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
	//VERIFY_SUCCEEDED(vkBindBufferMemory(Device, UniformBuffers.back().Buffer, UniformBuffers.back().DeviceMemory, 0));
	UniformBuffers.emplace_back().Create(Device, GetCurrentPhysicalDeviceMemoryProperties(), sizeof(UBSize));

	//!< アップデートテンプレート
	const std::array DUTEs = {
		VkDescriptorUpdateTemplateEntry({
			0, 0,
			_countof(DescriptorUpdateInfo::DBI), VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			offsetof(DescriptorUpdateInfo, DBI), sizeof(DescriptorUpdateInfo)
		})
	};
	assert(!empty(DescriptorSetLayouts) && "");
	const VkDescriptorUpdateTemplateCreateInfo DUTCI = {
		VK_STRUCTURE_TYPE_DESCRIPTOR_UPDATE_TEMPLATE_CREATE_INFO,
		nullptr,
		0,
		static_cast<uint32_t>(size(DUTEs)), data(DUTEs),
		VK_DESCRIPTOR_UPDATE_TEMPLATE_TYPE_DESCRIPTOR_SET,
		DescriptorSetLayouts[0],
		VK_PIPELINE_BIND_POINT_GRAPHICS, 
		VK_NULL_HANDLE, 0
	};
	DescriptorUpdateTemplates.emplace_back(VkDescriptorUpdateTemplate());
	VERIFY_SUCCEEDED(vkCreateDescriptorUpdateTemplate(Device, &DUTCI, GetAllocationCallbacks(), &DescriptorUpdateTemplates.back()));

	//!< アップデート
	const DescriptorUpdateInfo DUI = {
		{ UniformBuffers[0].Buffer, 0, VK_WHOLE_SIZE },
	};
	assert(!empty(DescriptorSets) && "");
	assert(!empty(DescriptorUpdateTemplates) && "");
	vkUpdateDescriptorSetWithTemplate(Device, DescriptorSets[0], DescriptorUpdateTemplates[0], &DUI);
#endif
}
void GltfVK::PostProcess()
{
}
void GltfVK::Process(const fx::gltf::Node& Nd, const uint32_t i)
{
	auto& Mtx = CurrentMatrix.back();

	if (fx::gltf::defaults::IdentityMatrix != Nd.matrix) {
		Mtx = glm::make_mat4(data(Nd.matrix)) * Mtx;
	}
	else {
		if (fx::gltf::defaults::NullVec3 != Nd.translation) {
			Mtx = glm::translate(Mtx, glm::make_vec3(data(Nd.translation)));
		}
		if (fx::gltf::defaults::IdentityRotation != Nd.rotation) {
			Mtx = glm::mat4_cast(glm::make_quat(data(Nd.rotation))) * Mtx;
		}
		if (fx::gltf::defaults::IdentityVec3 != Nd.scale) {
			Mtx = glm::scale(Mtx, glm::make_vec3(data(Nd.scale)));
		}
	}

	NodeMatrices[i] = Mtx;

	Gltf::Process(Nd, i);
}
void GltfVK::Process(const fx::gltf::Camera& Cam)
{
	Gltf::Process(Cam);

	PV.View = CurrentMatrix.back();
#if 1
	PV.View[3][0] = 0.0f;
	PV.View[3][1] = 0.0f;
#endif

	switch (Cam.type) {
		using enum fx::gltf::Camera::Type;
	case None: break;
	case Orthographic:
		PV.Projection = glm::orthoRH(0.0f, Cam.orthographic.xmag, 0.0f, Cam.orthographic.ymag, Cam.orthographic.znear, Cam.orthographic.zfar);
		break;
	case Perspective:
		PV.Projection = glm::perspective(Cam.perspective.yfov, Cam.perspective.aspectRatio, Cam.perspective.znear, Cam.perspective.zfar);
		break;
	}

#ifdef DEBUG_STDOUT
	std::cout << "View =" << std::endl;
	std::cout << PV.View;
	std::cout << "Projection =" << std::endl;
	std::cout << PV.Projection;
	//!< DXとは順次が逆
	std::cout << "Projection * View = " << PV.Projection * PV.View;
#endif

#if 0
	CopyToHostVisibleDeviceMemory(UniformBuffers[0].DeviceMemory, 0, sizeof(PV), &PV);
#endif
}
void GltfVK::Process(const fx::gltf::Primitive& Prim)
{
	Gltf::Process(Prim);

	//!< セマンティックの頭文字から読み込むシェーダを決定 (Select shader file by semantic initial)
	std::string SemanticInitial;
	for (const auto& i : Prim.attributes) {
		SemanticInitial += i.first.substr(0, 1);
	}
	const auto ShaderPath = GetBasePath() + TEXT("_") + std::wstring(cbegin(SemanticInitial), cend(SemanticInitial));
	ShaderModules.emplace_back(VK::CreateShaderModule(data(ShaderPath + TEXT(".vert.spv"))));
	const auto VS = VkPipelineShaderStageCreateInfo({ .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, .pNext = nullptr, .flags = 0, .stage = VK_SHADER_STAGE_VERTEX_BIT, .module = ShaderModules.back(), .pName = "main", .pSpecializationInfo = nullptr });
	ShaderModules.emplace_back(VK::CreateShaderModule(data(ShaderPath + TEXT(".frag.spv"))));
	const auto FS = VkPipelineShaderStageCreateInfo({ .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, .pNext = nullptr, .flags = 0, .stage = VK_SHADER_STAGE_FRAGMENT_BIT, .module = ShaderModules.back(), .pName = "main", .pSpecializationInfo = nullptr });

	//!< アトリビュート (Attributes)
	std::vector<VkVertexInputBindingDescription> VIBDs;
	std::vector<VkVertexInputAttributeDescription> VIADs;
	uint32_t Binding = 0;
	uint32_t Location = 0;
	const auto& Doc = GetDocument();
	for (const auto& i : Prim.attributes) {
		const auto& Acc = Doc.accessors[i.second];
		VIBDs.emplace_back(VkVertexInputBindingDescription({ .binding = Binding, .stride = GetTypeSize(Acc), .inputRate = VK_VERTEX_INPUT_RATE_VERTEX }));
		VIADs.emplace_back(VkVertexInputAttributeDescription({ .location = Location, .binding = Binding, .format = ToVKFormat(Acc), .offset = 0 }));
		++Binding;
		++Location;
	}
	//!< モーフターゲット (Morph target)
	for (const auto& i : Prim.targets) {
		for (const auto& j : i) {
			const auto& Acc = Doc.accessors[j.second];
			VIBDs.emplace_back(VkVertexInputBindingDescription({ .binding = Binding, .stride = GetTypeSize(Acc), .inputRate = VK_VERTEX_INPUT_RATE_VERTEX }));
			VIADs.emplace_back(VkVertexInputAttributeDescription({ .location = Location, .binding = Binding, .format = ToVKFormat(Acc), .offset = 0 }));
			++Binding;
			++Location;
		}
	}

	const auto RP = RenderPasses[0];
	const auto PLL = PipelineLayouts[0];
	Pipelines.emplace_back(VkPipeline());
	const VkPipelineRasterizationStateCreateInfo PRSCI = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
		.depthClampEnable = VK_FALSE,
		.rasterizerDiscardEnable = VK_FALSE,
		.polygonMode = VK_POLYGON_MODE_FILL,
		.cullMode = VK_CULL_MODE_BACK_BIT,
		.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
		.depthBiasEnable = VK_FALSE, .depthBiasConstantFactor = 0.0f, .depthBiasClamp = 0.0f, .depthBiasSlopeFactor = 0.0f,
		.lineWidth = 1.0f
	};
	const VkPipelineDepthStencilStateCreateInfo PDSSCI = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
		.depthTestEnable = VK_TRUE, .depthWriteEnable = VK_TRUE, .depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL,
		.depthBoundsTestEnable = VK_FALSE,
		.stencilTestEnable = VK_FALSE, 
		.front = VkStencilOpState({ .failOp = VK_STENCIL_OP_KEEP, .passOp = VK_STENCIL_OP_KEEP, .depthFailOp = VK_STENCIL_OP_KEEP, .compareOp = VK_COMPARE_OP_NEVER, .compareMask = 0, .writeMask = 0, .reference = 0 }),
		.back = VkStencilOpState({ .failOp = VK_STENCIL_OP_KEEP, .passOp = VK_STENCIL_OP_KEEP, .depthFailOp = VK_STENCIL_OP_KEEP, .compareOp = VK_COMPARE_OP_ALWAYS, .compareMask = 0, .writeMask = 0, .reference = 0 }),
		.minDepthBounds = 0.0f, .maxDepthBounds = 1.0f
	}; 
	const std::vector PCBASs = {
		VkPipelineColorBlendAttachmentState({
			.blendEnable = VK_FALSE,
			.srcColorBlendFactor = VK_BLEND_FACTOR_ONE, .dstColorBlendFactor = VK_BLEND_FACTOR_ONE, .colorBlendOp = VK_BLEND_OP_ADD,
			.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE, .dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE, .alphaBlendOp = VK_BLEND_OP_ADD,
			.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
		}),
	};
	VK::CreatePipeline_(Pipelines.back(), Device, PLL, RP, ToVKPrimitiveTopology(Prim.mode), 0, PRSCI, PDSSCI, &VS, &FS, nullptr, nullptr, nullptr, VIBDs, VIADs, PCBASs);

#ifdef DEBUG_STDOUT
	std::cout << "World =" << std::endl;
	std::cout << CurrentMatrix.back();
#endif

	const auto SCCount = static_cast<uint32_t>(size(SwapchainImages));
	assert(!empty(SecondaryCommandPools) && "");
	const auto PrevCount = size(SecondaryCommandBuffers);
	SecondaryCommandBuffers.resize(PrevCount + SCCount);
	const VkCommandBufferAllocateInfo CBAI = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		.pNext = nullptr,
		.commandPool = SecondaryCommandPools[0],
		.level = VK_COMMAND_BUFFER_LEVEL_SECONDARY,
		.commandBufferCount = SCCount
	};
	VERIFY_SUCCEEDED(vkAllocateCommandBuffers(Device, &CBAI, &SecondaryCommandBuffers[PrevCount]));

	for (auto i = 0; i < static_cast<int>(SCCount); ++i) {
		const auto FB = Framebuffers[i];
		const auto SCB = SecondaryCommandBuffers[size(SecondaryCommandBuffers) - SCCount + i];
		const VkCommandBufferInheritanceInfo CBII = {
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO,
			.pNext = nullptr,
			.renderPass = RP,
			.subpass = 0,
			.framebuffer = FB,
			.occlusionQueryEnable = VK_FALSE, .queryFlags = 0,
			.pipelineStatistics = 0,
		};
		const VkCommandBufferBeginInfo SCBBI = {
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
			.pNext = nullptr,
			.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT,
			.pInheritanceInfo = &CBII
		};

		VERIFY_SUCCEEDED(vkBeginCommandBuffer(SCB, &SCBBI)); {
			const auto PL = Pipelines.back();
			std::vector<VkBuffer> VBs; for (auto j : VertexBuffers) { VBs.emplace_back(j.Buffer); }
			const auto& IB = IndexBuffers.back();
			const auto& IDB = IndirectBuffers.back();

			vkCmdSetViewport(SCB, 0, static_cast<uint32_t>(size(Viewports)), data(Viewports));
			vkCmdSetScissor(SCB, 0, static_cast<uint32_t>(size(ScissorRects)), data(ScissorRects));

#if 0
			//vkCmdPushConstants(SCB, PLL, VK_SHADER_STAGE_VERTEX_BIT, 0, static_cast<uint32_t>(sizeof(CurrentMatrix.back())), &CurrentMatrix.back());
			const std::array<VkDescriptorSet, 1> DSs = { DescriptorSets.back() };
			constexpr std::array<uint32_t, 0> DynamicOffsets = {};
			vkCmdBindDescriptorSets(SCB, VK_PIPELINE_BIND_POINT_GRAPHICS, PLL, 0, static_cast<uint32_t>(size(DSs)), data(DSs), static_cast<uint32_t>(size(DynamicOffsets)), data(DynamicOffsets));
#endif

			vkCmdBindPipeline(SCB, VK_PIPELINE_BIND_POINT_GRAPHICS, PL);
			const std::vector<VkDeviceSize> Offsets(size(VBs), 0);
			vkCmdBindVertexBuffers(SCB, 0, static_cast<uint32_t>(size(VBs)), data(VBs), data(Offsets));
			vkCmdBindIndexBuffer(SCB, IB.Buffer, 0, ToVKIndexType(GetDocument().accessors[Prim.indices].componentType));
			vkCmdDrawIndexedIndirect(SCB, IDB.Buffer, 0, 1, 0);
		} VERIFY_SUCCEEDED(vkEndCommandBuffer(SCB));
	}
}

void GltfVK::Process(const std::string& Identifier, const fx::gltf::Accessor& Acc)
{
	Gltf::Process(Identifier, Acc);

	if (-1 != Acc.bufferView) {
		const auto& Doc = GetDocument();

		const auto& BufV = Doc.bufferViews[Acc.bufferView];

		if (-1 != BufV.buffer) {
			const auto& Buf = Doc.buffers[BufV.buffer];

			const auto Data = &Buf.data[BufV.byteOffset + Acc.byteOffset];
			const auto Stride = (0 == BufV.byteStride ? GetTypeSize(Acc) : BufV.byteStride);
			const auto Size = Acc.count * Stride;

			//!< BufferView.target はセットされてない事が多々あるので自前でIdentifierを用意…
			switch (BufV.target)
			{
				using enum fx::gltf::BufferView::TargetType;
			case None: break;
			case ArrayBuffer: break;
			case ElementArrayBuffer: break;
			}

			const auto PDMP = GetCurrentPhysicalDeviceMemoryProperties();
			const auto CB = CommandBuffers[0];
			if ("indices" == Identifier) {
				IndexBuffers.emplace_back().Create(Device, PDMP, Size, Data, CB, GraphicsQueue);

				const VkDrawIndexedIndirectCommand DIIC = { .indexCount = Acc.count, .instanceCount = 1, .firstIndex = 0, .vertexOffset = 0, .firstInstance = 0 };
				IndirectBuffers.emplace_back().Create(Device, GetCurrentPhysicalDeviceMemoryProperties(), DIIC, CommandBuffers[0], GraphicsQueue);
			}
			else if ("attributes" == Identifier || "targets" == Identifier) {
				VertexBuffers.emplace_back().Create(Device, PDMP, Size, Data, CB, GraphicsQueue);
			}
			else if ("inverseBindMatrices" == Identifier) {
				InverseBindMatrices.reserve(Acc.count);
				for (uint32_t i = 0; i < Acc.count; ++i) {
					InverseBindMatrices.emplace_back(reinterpret_cast<const glm::mat4*>(Data + Stride * i));
				}
#ifdef DEBUG_STDOUT
				if (size(InverseBindMatrices)) {
					std::cout << "InverseBindMatrices[" << size(InverseBindMatrices) << "]" << std::endl;
					for (auto i : InverseBindMatrices) {
						std::cout << *i;
					}
				}
#endif
			}
		}
	}
}
void GltfVK::Process(const fx::gltf::Mesh& Msh)
{
	Gltf::Process(Msh);

	MorphWeights = Msh.weights;
}
void GltfVK::Process(const fx::gltf::Skin& Skn)
{
	Gltf::Process(Skn);

	JointMatrices.reserve(size(Skn.joints));
	for (uint32_t i = 0; i < size(Skn.joints); ++i) {
		const auto& IBM = *InverseBindMatrices[i];

		const auto& Nd = GetDocument().nodes[Skn.joints[i]];
		auto Wld = glm::identity<glm::mat4>();
		if (fx::gltf::defaults::NullVec3 != Nd.translation) {
			glm::translate(Wld, glm::make_vec3(data(Nd.translation)));
		}
		if (fx::gltf::defaults::IdentityRotation != Nd.rotation) {
			Wld *= glm::mat4_cast(glm::make_quat(data(Nd.rotation)));
		}
		if (fx::gltf::defaults::IdentityVec3 != Nd.scale) {
			glm::scale(Wld, glm::make_vec3(data(Nd.scale)));
		}
		JointMatrices.emplace_back(IBM * Wld);
	}
#ifdef DEBUG_STDOUT
	if (size(JointMatrices)) {
		std::cout << "JointMatrices[" << size(JointMatrices) << "]" << std::endl;
		for (auto i : JointMatrices) {
			std::cout << i;
		}
	}
#endif
}
void GltfVK::OnTimer(HWND hWnd, HINSTANCE hInstance)
{
	Super::OnTimer(hWnd, hInstance);

	if (empty(GetDocument().animations)) { return; }

	const auto SlowFactor = 1.0f;
	CurrentFrame += static_cast<float>(Elapse) / 1000.0f * SlowFactor;

	AnimNodeMatrices.assign(cbegin(NodeMatrices), cend(NodeMatrices));

	UpdateAnimation(CurrentFrame, true);

#ifdef DEBUG_STDOUT
	if (size(AnimNodeMatrices)) {
		std::cout << "AnimNodeMatrices[" << size(AnimNodeMatrices) << "]" << std::endl;
		for (auto i : AnimNodeMatrices) {
			std::cout << i;
		}
	}
#endif
}

void GltfVK::UpdateAnimTranslation(const std::array<float, 3>& Value, const uint32_t NodeIndex)
{
	if (-1 != NodeIndex) {
		//AnimNodeMatrices[NodeIndex] = glm::translate(AnimNodeMatrices[NodeIndex], glm::make_vec3(data(Value))); //!< ↓と同じ結果にならない
		AnimNodeMatrices[NodeIndex] = glm::translate(glm::mat4(1.0f), glm::make_vec3(data(Value))) * AnimNodeMatrices[NodeIndex];
	}
}
void GltfVK::UpdateAnimScale(const std::array<float, 3>& Value, const uint32_t NodeIndex)
{
	if (-1 != NodeIndex) {
		AnimNodeMatrices[NodeIndex] = glm::scale(AnimNodeMatrices[NodeIndex], glm::make_vec3(data(Value)));
	}
}
void GltfVK::UpdateAnimRotation(const std::array<float, 4>& Value, const uint32_t NodeIndex)
{
	if (-1 != NodeIndex) {
		//!< クォータニオンは要正規化
		AnimNodeMatrices[NodeIndex] = glm::mat4_cast(glm::normalize(glm::make_quat(data(Value)))) * AnimNodeMatrices[NodeIndex];
	}
}
void GltfVK::UpdateAnimWeights([[maybe_unused]] const float* Data, [[maybe_unused]] const uint32_t PrevIndex, [[maybe_unused]] const uint32_t NextIndex, [[maybe_unused]] const float t)
{
	//Lerp(Data[PrevIndex], Data[NextIndex], t);
}

void GltfVK::PopulateCommandBuffer(const size_t i)
{
	const auto RP = RenderPasses[0];
	const auto FB = Framebuffers[i];

	const auto SCCount = size(SwapchainImages);
	const auto PrimCount = size(SecondaryCommandBuffers) / SCCount;
	std::vector<VkCommandBuffer> SCBs;
	for (auto j = 0; j < PrimCount; ++j) {
		SCBs.emplace_back(SecondaryCommandBuffers[j * SCCount + i]);
	}

	const auto CB = CommandBuffers[i];
	const VkCommandBufferBeginInfo CBBI = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		.pNext = nullptr,
		.flags = 0,
		.pInheritanceInfo = nullptr
	};
	VERIFY_SUCCEEDED(vkBeginCommandBuffer(CB, &CBBI)); {
		const std::array CVs = { VkClearValue({.color = Colors::SkyBlue }), VkClearValue({.depthStencil = {.depth = 1.0f, .stencil = 0 } }) };
		const VkRenderPassBeginInfo RPBI = {
			.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
			.pNext = nullptr,
			.renderPass = RP,
			.framebuffer = FB,
			.renderArea = VkRect2D({.offset = VkOffset2D({.x = 0, .y = 0 }), .extent = SurfaceExtent2D }),
			.clearValueCount = static_cast<uint32_t>(size(CVs)), .pClearValues = data(CVs)
		};

		//const auto PLL = PipelineLayouts[0];
		//vkCmdPushConstants(CB, PLL, VK_SHADER_STAGE_VERTEX_BIT, 0, static_cast<uint32_t>(sizeof(ViewProjection)), &ViewProjection);
		//const std::array<VkDescriptorSet, 1> DSs = { DescriptorSets.back() };
		//constexpr std::array<uint32_t, 0> DynamicOffsets = {};
		//vkCmdBindDescriptorSets(CB, VK_PIPELINE_BIND_POINT_GRAPHICS, PLL, 0, static_cast<uint32_t>(size(DSs)), data(DSs), static_cast<uint32_t>(size(DynamicOffsets)), data(DynamicOffsets));

		vkCmdBeginRenderPass(CB, &RPBI, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS); {
			vkCmdExecuteCommands(CB, static_cast<uint32_t>(size(SCBs)), data(SCBs));
		} vkCmdEndRenderPass(CB);
	} VERIFY_SUCCEEDED(vkEndCommandBuffer(CB));
}
#pragma endregion //!< Code