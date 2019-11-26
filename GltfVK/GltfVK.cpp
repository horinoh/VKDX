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
	//!< PN(POS, NRM)
	//Load("..\\..\\glTF-Sample-Models\\2.0\\Box\\glTF-Binary\\Box.glb"); //!< Scale = 1.0f

	//!< PT(POS, TEX0) ... KHR_texture_transform �g�����
	//Load("..\\..\\glTF-Sample-Models\\2.0\\TextureTransformTest\\glTF\\TextureTransformTest.gltf");

	//!< PNT(POS, NRM, TEX0)
	Load("..\\..\\glTF-Sample-Models\\2.0\\Duck\\glTF-Binary\\Duck.glb"); //!< Scale = 0.005f
	//Load("..\\..\\glTF-Sample-Models\\2.0\\DamagedHelmet\\glTF-Binary\\DamagedHelmet.glb"); //!< Scale = 0.5f
	//Load("..\\..\\glTF-Sample-Models\\2.0\\BoxTextured\\glTF-Binary\\BoxTextured.glb"); //!< Scale = 1.0f

	//!< TPN(TAN, POS, NRM)
	//Load("..\\..\\glTF-Sample-Models\\2.0\\AnimatedMorphCube\\glTF-Binary\\AnimatedMorphCube.glb"); //!< Scale = 50.0f
	//Load("..\\..\\glTF-Sample-Models\\2.0\\AnimatedMorphSphere\\glTF-Binary\\AnimatedMorphSphere.glb"); //!< Scale = 50.0f

	//!< CPNT(COL0, POS, NRM. TEX0)
	//Load("..\\..\\glTF-Sample-Models\\2.0\\BoxVertexColors\\glTF-Binary\\BoxVertexColors.glb"); //!< Scale = 1.0f

	//!< TPNT(TAN, POS, NRM, TEX0)
	//Load("..\\..\\glTF-Sample-Models\\2.0\\SciFiHelmet\\glTF\\SciFiHelmet.gltf"); //!< Scale = 0.5f
	//Load("..\\..\\glTF-Sample-Models\\2.0\\Suzanne\\glTF\\Suzanne.gltf"); //!< Scale = 0.5f
	 
	//!< JPNW(JNT0, POS, NRM, WGT0)
	//Load("..\\..\\glTF-Sample-Models\\2.0\\RiggedSimple\\glTF-Binary\\RiggedSimple.glb"); //!< Scale = 0.2f
	//Load("..\\..\\glTF-Sample-Models\\2.0\\RiggedFigure\\glTF-Binary\\RiggedFigure.glb"); //!< Scale = 0.5f

	//!< JPNTW(JNT0, POS, NRM, TEX0, WGT0)
	//Load("..\\..\\glTF-Sample-Models\\2.0\\CesiumMan\\glTF-Binary\\CesiumMan.glb"); //!< Scale = 0.5f
	//Load("..\\..\\glTF-Sample-Models\\2.0\\Monster\\glTF-Binary\\Monster.glb"); //!< Scale = 0.02f
}
void GltfVK::Process(const fx::gltf::Node& Nd)
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
			Mtx *= glm::mat4_cast(glm::make_quat(Nd.rotation.data()));
		}
		if (fx::gltf::defaults::IdentityVec3 != Nd.scale) {
			Mtx = glm::scale(Mtx, glm::make_vec3(Nd.scale.data()));
		}
	}

	Gltf::Process(Nd);
}
void GltfVK::Process(const fx::gltf::Camera& Cam)
{
	Gltf::Process(Cam);

	glm::mat4 View = CurrentMatrix.back();
	glm::mat4 Projection;
	switch (Cam.type) {
	case fx::gltf::Camera::Type::None: break;
	case fx::gltf::Camera::Type::Orthographic:
		Projection = glm::orthoRH(0.0f, Cam.orthographic.xmag, 0.0f, Cam.orthographic.ymag, Cam.orthographic.znear, Cam.orthographic.zfar);
		break;
	case fx::gltf::Camera::Type::Perspective:
		Projection = glm::perspective(Cam.perspective.yfov, Cam.perspective.aspectRatio, Cam.perspective.znear, Cam.perspective.zfar);
		break;
	}

#ifdef DEBUG_STDOUT
	std::cout << "View =" << std::endl;
	std::cout << View;
	std::cout << "Projection =" << std::endl;
	std::cout << Projection;
#endif
}
void GltfVK::Process(const fx::gltf::Primitive& Prim)
{
	Gltf::Process(Prim);

	std::string SemanticInitial;
	for (const auto& i : Prim.attributes) {
		SemanticInitial += i.first.substr(0, 1);
	}

	const auto ShaderPath = GetBasePath() + TEXT("_") + std::wstring(SemanticInitial.begin(), SemanticInitial.end());
	ShaderModules.push_back(VKExt::CreateShaderModule((ShaderPath + TEXT(".vert.spv")).data()));
	const auto VS = ShaderModules.back();
	ShaderModules.push_back(VKExt::CreateShaderModule((ShaderPath + TEXT(".frag.spv")).data()));
	const auto FS = ShaderModules.back();

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

	const auto RP = RenderPasses[0];
	const auto PLL = PipelineLayouts[0];
	Pipelines.push_back(VkPipeline());
	CreatePipeline(Pipelines.back(), PLL, RP, VS, FS, NullShaderModule, NullShaderModule, NullShaderModule, VIBDs, VIADs, ToVKPrimitiveTopology(Prim.mode));

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
			//vkCmdPushConstants(SCB, PLL, VK_SHADER_STAGE_VERTEX_BIT, 0, static_cast<uint32_t>(sizeof(CurrentMatrix.back())), &CurrentMatrix.back());
			vkCmdBindPipeline(SCB, VK_PIPELINE_BIND_POINT_GRAPHICS, PL);
			vkCmdBindVertexBuffers(SCB, 0, static_cast<uint32_t>(VBs.size()), VBs.data(), Offsets.data());
			vkCmdBindIndexBuffer(SCB, IB, 0, ToVKIndexType(Document.accessors[Prim.indices].componentType));
			vkCmdDrawIndexedIndirect(SCB, IndB, 0, 1, 0);
		} VERIFY_SUCCEEDED(vkEndCommandBuffer(SCB));
	}
}

void GltfVK::Process(const std::string& Identifier, const fx::gltf::Accessor& Acc)
{
	Gltf::Process(Identifier, Acc);

	if (-1 != Acc.bufferView) {
		const auto& BufV = Document.bufferViews[Acc.bufferView];

		if (-1 != BufV.buffer) {
			const auto& Buf = Document.buffers[BufV.buffer];

			const auto Data = &Buf.data[BufV.byteOffset + Acc.byteOffset];
			const auto Stride = (0 == BufV.byteStride ? GetTypeSize(Acc) : BufV.byteStride);
			const auto Size = Acc.count * Stride;

			//!< BufferView.target �̓Z�b�g����ĂȂ��������X����̂Ŏ��O�ł��c
			switch (BufV.target)
			{
			case fx::gltf::BufferView::TargetType::None: break;
			case fx::gltf::BufferView::TargetType::ArrayBuffer: break;
			case fx::gltf::BufferView::TargetType::ElementArrayBuffer: break;
			}

			if ("indices" == Identifier) {
				IndexBuffers.push_back(VkBuffer());
				CreateBuffer_Index(GraphicsQueue, CommandBuffers[0], &IndexBuffers.back(), Size, Data);

				CreateIndirectBuffer_DrawIndexed(Acc.count, 1);
			}
			else if ("attributes" == Identifier) {
				VertexBuffers.push_back(VkBuffer());
				CreateBuffer_Vertex(GraphicsQueue, CommandBuffers[0], &VertexBuffers.back(), Size, Data);
			}
			else if ("inverseBindMatrices" == Identifier) {
				InverseBindMatrices.reserve(Acc.count);
				for (uint32_t i = 0; i < Acc.count; ++i) {
					InverseBindMatrices.push_back(reinterpret_cast<const glm::mat4*>(Data + Stride * i));
#ifdef DEBUG_STDOUT
					std::cout << *InverseBindMatrices.back();
#endif
				}
			}
		}
	}
}

void GltfVK::Process(const fx::gltf::Skin& Skn)
{
	Gltf::Process(Skn);

	JointMatrices.reserve(Skn.joints.size());
	for (uint32_t i = 0; i < Skn.joints.size(); ++i) {
		const auto& IBM = *InverseBindMatrices[i];

		const auto& Nd = Document.nodes[Skn.joints[i]];
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
		JointMatrices.push_back(Wld * IBM);
	}
}

void GltfVK::Process(const fx::gltf::Material::Texture& Tex)
{
	Gltf::Process(Tex);

#ifdef DEBUG_STDOUT
	//!< KHR_texture_transform �g��
	const auto ItExtensions = Tex.extensionsAndExtras.find("extensions");
	if (ItExtensions != Tex.extensionsAndExtras.end()) {
		const auto ItTexTransform = ItExtensions->find("KHR_texture_transform");
		if (ItTexTransform != ItExtensions->end()) {
			const auto ItOffset = ItTexTransform->find("offset");
			if (ItOffset != ItTexTransform->end()) {
				if (ItOffset->is_array() && 2 == ItOffset->size()) {
					if (ItOffset->at(0).is_number_float()) {
						std::cout << ItOffset->at(0).get<float>() << ", " << ItOffset->at(1).get<float>() << std::endl;
					}
				}
			}
		}
	}

	const auto ItExtras = Tex.extensionsAndExtras.find("extras");
	if (ItExtras != Tex.extensionsAndExtras.end()) {}
#endif
}
void GltfVK::Process(const fx::gltf::Texture& Tex)
{
	Gltf::Process(Tex);

#ifdef DEBUG_STDOUT
	//!< MSFT_texture_dds �g��
	const auto ItExtensions = Tex.extensionsAndExtras.find("extensions");
	if (ItExtensions != Tex.extensionsAndExtras.end()) {
		const auto ItTexDDS = ItExtensions->find("MSFT_texture_dds");
		if (ItTexDDS != ItExtensions->end()) {
			const auto ItSrc = ItTexDDS->find("source");
			if (ItSrc != ItTexDDS->end()) {
				if (ItSrc->is_number_integer()) {
					std::cout << ItSrc->get<int32_t>() << std::endl;
				}
			}
		}
	}

	const auto ItExtras = Tex.extensionsAndExtras.find("extras");
	if (ItExtras != Tex.extensionsAndExtras.end()) {}
#endif
}

void GltfVK::OnTimer(HWND hWnd, HINSTANCE hInstance)
{
	Super::OnTimer(hWnd, hInstance);

	CurrentFrame += 0.1f; //static_cast<float>(Elapse) / 1000.0f;

	for (const auto& i : Document.animations) {
		for (const auto& j : i.channels) {
			if (-1 != j.sampler) {
				const auto& Smp = i.samplers[j.sampler];
				if (-1 != Smp.input && -1 != Smp.output) {
					const auto& InAcc = Document.accessors[Smp.input];
					if (InAcc.type == fx::gltf::Accessor::Type::Scalar && InAcc.componentType == fx::gltf::Accessor::ComponentType::Float) {
						const auto Keyframes = reinterpret_cast<const float*>(GetData(InAcc));
						const auto MaxFrame = Keyframes[InAcc.count - 1];
						CurrentFrame = std::min(CurrentFrame, MaxFrame);

						uint32_t PrevIndex = 0, NextIndex = 0;
						for (uint32_t k = 0; k < InAcc.count; ++k) {
							if (Keyframes[k] >= CurrentFrame) {
								NextIndex = k;
								PrevIndex = NextIndex - 1;
								break;
							}
						}
						const auto PrevFrame = Keyframes[PrevIndex];
						const auto NextFrame = Keyframes[NextIndex];
						const auto Delta = NextFrame - PrevFrame;
						std::cout << "Frame = " << PrevFrame << " < " << CurrentFrame << " < " << NextFrame << ", Max = " << MaxFrame << std::endl;

						const auto t = (CurrentFrame - PrevFrame) / Delta;
						std::cout << "t = " << t << std::endl;

#ifdef DEBUG_STDOUT
						const auto& OutAcc = Document.accessors[Smp.output];
#endif
						std::cout << "\t" << j.target.path << " = ";
						switch (Smp.interpolation)
						{
						case fx::gltf::Animation::Sampler::Type::Linear:
							if ("translation" == j.target.path || "scale" == j.target.path) {
#ifdef DEBUG_STDOUT
								const auto Data = reinterpret_cast<const glm::vec3*>(GetData(OutAcc));
								std::cout << glm::mix(Data[PrevIndex], Data[NextIndex], t);
#endif
							} else if("rotation" == j.target.path) {
#ifdef DEBUG_STDOUT
								const auto Data = reinterpret_cast<const glm::quat*>(GetData(OutAcc));
								std::cout << glm::slerp(Data[PrevIndex], Data[NextIndex], t);
#endif
							}
							break;
						case fx::gltf::Animation::Sampler::Type::Step:
							if ("translation" == j.target.path || "scale" == j.target.path) {
#ifdef DEBUG_STDOUT
								const auto Data = reinterpret_cast<const glm::vec3*>(GetData(OutAcc));
								std::cout << Data[PrevIndex];
#endif
							}
							else if ("rotation" == j.target.path) {
#ifdef DEBUG_STDOUT
								const auto Data = reinterpret_cast<const glm::quat*>(GetData(OutAcc));
								std::cout << Data[PrevIndex];
#endif
							}
							break;
						case fx::gltf::Animation::Sampler::Type::CubicSpline:
							//!< https://github.com/KhronosGroup/glTF/tree/master/specification/2.0#appendix-c-spline-interpolation
							//!< CubicSpline �̏ꍇ�A(InTangent, Value, OutTangent)��3�ŃZ�b�g�ɂȂ��Ă���̂�3�̔{���ɂȂ�
							if ("translation" == j.target.path || "scale" == j.target.path) {
#ifdef DEBUG_STDOUT
								const auto Data = reinterpret_cast<const glm::vec3*>(GetData(OutAcc));

								const auto PrevSet = PrevIndex * 3; //!< 0:PrevInTangent, 1:PrevValue, 2:PrevOutTangent
								const auto NextSet = NextIndex * 3; //!< 0:NextInTangent, 1:NextValue, 3:NextOutTangent
								
								const auto& PrevValue = Data[PrevSet + 1];
								const auto& PrevOutTan = Data[PrevSet + 2];
								const auto& NextInTan = Data[NextSet + 0];
								const auto& NextValue = Data[NextSet + 1];

								const auto t2 = t * t;
								const auto t3 = t2 * t;

								std::cout << (2.0f * t3 - 3.0f * t2 + 1.0f)* PrevValue + (t3 - 2.0f * t2 + t) * PrevOutTan + (-2.0f * t3 + 3.0f * t2) * NextValue + (t3 - t2) * NextInTan << std::endl;
#endif
							}
							else if ("rotation" == j.target.path) {
#ifdef DEBUG_STDOUT
								//const auto Data = reinterpret_cast<const glm::quat*>(GetData(OutAcc));
#endif
							}
							break;
						}						

						if (-1 != j.target.node) {
							auto& Nd = Document.nodes[j.target.node];
							Nd.translation;
							Nd.rotation;
							Nd.scale;
						}
					}
				}
			}
		}
	}
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
		//vkCmdPushConstants(CB, PLL, VK_SHADER_STAGE_VERTEX_BIT, 0, static_cast<uint32_t>(sizeof(ViewProjection)), &ViewProjection);
		vkCmdBeginRenderPass(CB, &RPBI, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS); {
			vkCmdExecuteCommands(CB, static_cast<uint32_t>(SCBs.size()), SCBs.data());
		} vkCmdEndRenderPass(CB);
	} VERIFY_SUCCEEDED(vkEndCommandBuffer(CB));
}
#pragma endregion //!< Code