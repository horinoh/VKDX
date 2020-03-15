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
	const auto Fov = 0.16f * glm::pi<float>();
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
	assert(!DescriptorSetLayouts.empty() && "");
	const std::array<VkDescriptorSetLayout, 1> DSLs = { DescriptorSetLayouts[0] };
	assert(!DescriptorPools.empty() && "");
	const VkDescriptorSetAllocateInfo DSAI = {
		VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
		nullptr,
		DescriptorPools[0],
		static_cast<uint32_t>(DSLs.size()), DSLs.data()
	};
	DescriptorSets.resize(1);
	VERIFY_SUCCEEDED(vkAllocateDescriptorSets(Device, &DSAI, &DescriptorSets[0]));

	//!< ユニフォームバッファ
	UniformBuffers.resize(1);
	CreateBuffer(&UniformBuffers[0], VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, sizeof(UBSize));
	SuballocateBufferMemory(HeapIndex, Offset, UniformBuffers[0], VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

	//!< アップデートテンプレート
	const std::array<VkDescriptorUpdateTemplateEntry, 1> DUTEs = {
		{
			0, 0,
			_countof(DescriptorUpdateInfo::DBI), VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			offsetof(DescriptorUpdateInfo, DBI), sizeof(DescriptorUpdateInfo)
		}
	};
	assert(!DescriptorSetLayouts.empty() && "");
	const VkDescriptorUpdateTemplateCreateInfo DUTCI = {
		VK_STRUCTURE_TYPE_DESCRIPTOR_UPDATE_TEMPLATE_CREATE_INFO,
		nullptr,
		0,
		static_cast<uint32_t>(DUTEs.size()), DUTEs.data(),
		VK_DESCRIPTOR_UPDATE_TEMPLATE_TYPE_DESCRIPTOR_SET,
		DescriptorSetLayouts[0],
		VK_PIPELINE_BIND_POINT_GRAPHICS, VK_NULL_HANDLE, 0
	};
	DescriptorUpdateTemplates.resize(1);
	VERIFY_SUCCEEDED(vkCreateDescriptorUpdateTemplate(Device, &DUTCI, GetAllocationCallbacks(), &DescriptorUpdateTemplates[0]));

	//!< アップデート
	const DescriptorUpdateInfo DUI = {
		{ UniformBuffers[0], Offset, VK_WHOLE_SIZE },
	};
	assert(!DescriptorSets.empty() && "");
	assert(!DescriptorUpdateTemplates.empty() && "");
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
		Mtx = glm::make_mat4(Nd.matrix.data()) * Mtx;
	}
	else {
		if (fx::gltf::defaults::NullVec3 != Nd.translation) {
			Mtx = glm::translate(Mtx, glm::make_vec3(Nd.translation.data()));
		}
		if (fx::gltf::defaults::IdentityRotation != Nd.rotation) {
			Mtx = glm::mat4_cast(glm::make_quat(Nd.rotation.data())) * Mtx;
		}
		if (fx::gltf::defaults::IdentityVec3 != Nd.scale) {
			Mtx = glm::scale(Mtx, glm::make_vec3(Nd.scale.data()));
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
	case fx::gltf::Camera::Type::None: break;
	case fx::gltf::Camera::Type::Orthographic:
		PV.Projection = glm::orthoRH(0.0f, Cam.orthographic.xmag, 0.0f, Cam.orthographic.ymag, Cam.orthographic.znear, Cam.orthographic.zfar);
		break;
	case fx::gltf::Camera::Type::Perspective:
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
	CopyToHostVisibleDeviceMemory(DeviceMemories[HeapIndex], sizeof(PV), &PV, Offset);
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
	const auto ShaderPath = GetBasePath() + TEXT("_") + std::wstring(SemanticInitial.begin(), SemanticInitial.end());
	ShaderModules.push_back(VKExt::CreateShaderModules((ShaderPath + TEXT(".vert.spv")).data()));
	const auto VS = ShaderModules.back();
	ShaderModules.push_back(VKExt::CreateShaderModules((ShaderPath + TEXT(".frag.spv")).data()));
	const auto FS = ShaderModules.back();

	//!< アトリビュート (Attributes)
	std::vector<VkVertexInputBindingDescription> VIBDs;
	std::vector<VkVertexInputAttributeDescription> VIADs;
	uint32_t Binding = 0;
	uint32_t Location = 0;
	const auto& Doc = GetDocument();
	for (const auto& i : Prim.attributes) {
		const auto& Acc = Doc.accessors[i.second];
		VIBDs.push_back({ Binding, GetTypeSize(Acc),  VK_VERTEX_INPUT_RATE_VERTEX });
		VIADs.push_back({ Location, Binding, ToVKFormat(Acc), 0 });
		++Binding;
		++Location;
	}
	//!< モーフターゲット (Morph target)
	for (const auto& i : Prim.targets) {
		for (const auto& j : i) {
			const auto& Acc = Doc.accessors[j.second];
			VIBDs.push_back({ Binding, GetTypeSize(Acc),  VK_VERTEX_INPUT_RATE_VERTEX });
			VIADs.push_back({ Location, Binding, ToVKFormat(Acc), 0 });
			++Binding;
			++Location;
		}
	}

	const auto RP = RenderPasses[0];
	const auto PLL = PipelineLayouts[0];
	Pipelines.push_back(VkPipeline());
	VK::CreatePipeline(Pipelines.back(), Device, PLL, RP, ToVKPrimitiveTopology(Prim.mode), 0, VS, FS, NullShaderModule, NullShaderModule, NullShaderModule, VIBDs, VIADs);

#ifdef DEBUG_STDOUT
	std::cout << "World =" << std::endl;
	std::cout << CurrentMatrix.back();
#endif

	const auto Count = AddSecondaryCommandBuffer();
	const auto& VBs = VertexBuffers;
	const auto IB = IndexBuffers.back();
	const auto IndB = IndirectBuffers.back();
	const auto PL = Pipelines.back();

	const std::vector<VkDeviceSize> Offsets(VBs.size(), 0);
	for (auto i = 0; i < static_cast<int>(Count); ++i) {
		const auto SCB = SecondaryCommandBuffers[SecondaryCommandBuffers.size() - Count + i];
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

#if 0
			//vkCmdPushConstants(SCB, PLL, VK_SHADER_STAGE_VERTEX_BIT, 0, static_cast<uint32_t>(sizeof(CurrentMatrix.back())), &CurrentMatrix.back());
			const std::array<VkDescriptorSet, 1> DSs = { DescriptorSets.back() };
			vkCmdBindDescriptorSets(SCB, VK_PIPELINE_BIND_POINT_GRAPHICS, PLL, 0, static_cast<uint32_t>(DSs.size()), DSs.data(), 0, nullptr);
#endif

			vkCmdBindPipeline(SCB, VK_PIPELINE_BIND_POINT_GRAPHICS, PL);
			vkCmdBindVertexBuffers(SCB, 0, static_cast<uint32_t>(VBs.size()), VBs.data(), Offsets.data());
			vkCmdBindIndexBuffer(SCB, IB, 0, ToVKIndexType(GetDocument().accessors[Prim.indices].componentType));
			vkCmdDrawIndexedIndirect(SCB, IndB, 0, 1, 0);
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
			case fx::gltf::BufferView::TargetType::None: break;
			case fx::gltf::BufferView::TargetType::ArrayBuffer: break;
			case fx::gltf::BufferView::TargetType::ElementArrayBuffer: break;
			}

			if ("indices" == Identifier) {
				IndexBuffers.push_back(VkBuffer());
				CreateBuffer_Index(&IndexBuffers.back(), GraphicsQueue, CommandBuffers[0], Size, Data);

				CreateIndirectBuffer_DrawIndexed(Acc.count, 1);
			}
			else if ("attributes" == Identifier || "targets" == Identifier) {
				VertexBuffers.push_back(VkBuffer());
				CreateBuffer_Vertex(&VertexBuffers.back(), GraphicsQueue, CommandBuffers[0] , Size, Data);
			}
			else if ("inverseBindMatrices" == Identifier) {
				InverseBindMatrices.reserve(Acc.count);
				for (uint32_t i = 0; i < Acc.count; ++i) {
					InverseBindMatrices.push_back(reinterpret_cast<const glm::mat4*>(Data + Stride * i));
				}
#ifdef DEBUG_STDOUT
				if (InverseBindMatrices.size()) {
					std::cout << "InverseBindMatrices[" << InverseBindMatrices.size() << "]" << std::endl;
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

	JointMatrices.reserve(Skn.joints.size());
	for (uint32_t i = 0; i < Skn.joints.size(); ++i) {
		const auto& IBM = *InverseBindMatrices[i];

		const auto& Nd = GetDocument().nodes[Skn.joints[i]];
		auto Wld = glm::identity<glm::mat4>();
		if (fx::gltf::defaults::NullVec3 != Nd.translation) {
			glm::translate(Wld, glm::make_vec3(Nd.translation.data()));
		}
		if (fx::gltf::defaults::IdentityRotation != Nd.rotation) {
			Wld *= glm::mat4_cast(glm::make_quat(Nd.rotation.data()));
		}
		if (fx::gltf::defaults::IdentityVec3 != Nd.scale) {
			glm::scale(Wld, glm::make_vec3(Nd.scale.data()));
		}
		JointMatrices.push_back(IBM * Wld);
	}
#ifdef DEBUG_STDOUT
	if (JointMatrices.size()) {
		std::cout << "JointMatrices[" << JointMatrices.size() << "]" << std::endl;
		for (auto i : JointMatrices) {
			std::cout << i;
		}
	}
#endif
}
void GltfVK::OnTimer(HWND hWnd, HINSTANCE hInstance)
{
	Super::OnTimer(hWnd, hInstance);

	if (GetDocument().animations.empty()) { return; }

	CurrentFrame += 0.1f; //static_cast<float>(Elapse) / 1000.0f;

	AnimNodeMatrices.assign(NodeMatrices.begin(), NodeMatrices.end());

	const auto bLoop = true;
	//const auto bLoop = false;
	UpdateAnimation(CurrentFrame, bLoop);

#ifdef DEBUG_STDOUT
	if (AnimNodeMatrices.size()) {
		std::cout << "AnimNodeMatrices[" << AnimNodeMatrices.size() << "]" << std::endl;
		for (auto i : AnimNodeMatrices) {
			std::cout << i;
		}
	}
#endif
}

void GltfVK::UpdateAnimTranslation(const std::array<float, 3>& Value, const uint32_t NodeIndex)
{
	if (-1 != NodeIndex) {
		//AnimNodeMatrices[NodeIndex] = glm::translate(AnimNodeMatrices[NodeIndex], glm::make_vec3(Value.data())); //!< ↓と同じ結果にならない
		AnimNodeMatrices[NodeIndex] = glm::translate(glm::mat4(1.0f), glm::make_vec3(Value.data())) * AnimNodeMatrices[NodeIndex];
	}
}
void GltfVK::UpdateAnimScale(const std::array<float, 3>& Value, const uint32_t NodeIndex)
{
	if (-1 != NodeIndex) {
		AnimNodeMatrices[NodeIndex] = glm::scale(AnimNodeMatrices[NodeIndex], glm::make_vec3(Value.data()));
	}
}
void GltfVK::UpdateAnimRotation(const std::array<float, 4>& Value, const uint32_t NodeIndex)
{
	if (-1 != NodeIndex) {
		AnimNodeMatrices[NodeIndex] = glm::mat4_cast(glm::make_quat(Value.data())) * AnimNodeMatrices[NodeIndex];
	}
}
void GltfVK::UpdateAnimWeights(const float* /*Data*/, const uint32_t /*PrevIndex*/, const uint32_t /*NextIndex*/, const float /*t*/)
{
	//Lerp(Data[PrevIndex], Data[NextIndex], t);
}

void GltfVK::PopulateCommandBuffer(const size_t i)
{
	const auto RP = RenderPasses[0];

	const auto CB = CommandBuffers[i];
	const auto FB = Framebuffers[i];

	const auto SCICount = SwapchainImages.size();
	const auto PrimCount = SecondaryCommandBuffers.size() / SwapchainImages.size();
	std::vector<VkCommandBuffer> SCBs;
	for (auto j = 0; j < PrimCount; ++j) {
		SCBs.push_back(SecondaryCommandBuffers[j * SCICount + i]);
	}

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

		//const auto PLL = PipelineLayouts[0];
		//vkCmdPushConstants(CB, PLL, VK_SHADER_STAGE_VERTEX_BIT, 0, static_cast<uint32_t>(sizeof(ViewProjection)), &ViewProjection);
		//const std::array<VkDescriptorSet, 1> DSs = { DescriptorSets.back() };
		//vkCmdBindDescriptorSets(CB, VK_PIPELINE_BIND_POINT_GRAPHICS, PLL, 0, static_cast<uint32_t>(DSs.size()), DSs.data(), 0, nullptr);

		vkCmdBeginRenderPass(CB, &RPBI, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS); {
			vkCmdExecuteCommands(CB, static_cast<uint32_t>(SCBs.size()), SCBs.data());
		} vkCmdEndRenderPass(CB);
	} VERIFY_SUCCEEDED(vkEndCommandBuffer(CB));
}
#pragma endregion //!< Code