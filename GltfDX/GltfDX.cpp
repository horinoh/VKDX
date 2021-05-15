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
void GltfDX::LoadScene()
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
	//Load(BasePath + "glTF-Sample-Models\\2.0\\DamagedHelmet\\glTF-Binary\\DamagedHelmet.glb"); //!< Scale = 0.5f (NG)
	//Load(BasePath + "glTF-Sample-Models\\2.0\\BoxTextured\\glTF-Binary\\BoxTextured.glb"); //!< Scale = 1.0f

	//!< TPN(TAN, POS, NRM)
	//!< モーフターゲット
	//Load(BasePath + "glTF-Sample-Models\\2.0\\AnimatedMorphCube\\glTF-Binary\\AnimatedMorphCube.glb");
	//Load(BasePath + "glTF-Sample-Models\\2.0\\AnimatedMorphSphere\\glTF-Binary\\AnimatedMorphSphere.glb");

	//!< CPNT(COL0, POS, NRM. TEX0)
	//Load(BasePath + "glTF-Sample-Models\\2.0\\BoxVertexColors\\glTF-Binary\\BoxVertexColors.glb"); //!< Scale = 1.0f (NG)

	//!< TPNT(TAN, POS, NRM, TEX0)
	//Load(BasePath + "glTF-Sample-Models\\2.0\\SciFiHelmet\\glTF\\SciFiHelmet.gltf"); //!< Scale = 0.5f (NG)
	//Load(BasePath + "glTF-Sample-Models\\2.0\\Suzanne\\glTF\\Suzanne.gltf"); //!< Scale = 0.5f (NG)

	//!< JPNW(JNT0, POS, NRM, WGT0)
	//Load(BasePath + "glTF-Sample-Models\\2.0\\RiggedSimple\\glTF-Binary\\RiggedSimple.glb"); //!< Scale=0.2f
	//Load(BasePath + "glTF-Sample-Models\\2.0\\RiggedFigure\\glTF-Binary\\RiggedFigure.glb"); //!< Scale = 0.5f

	//!< JPNTW(JNT0, POS, NRM, TEX0, WGT0)
	//Load(BasePath + "glTF-Sample-Models\\2.0\\CesiumMan\\glTF-Binary\\CesiumMan.glb"); //!< Scale = 0.5f
	//Load(BasePath + "glTF-Sample-Models\\2.0\\Monster\\glTF-Binary\\Monster.glb"); //!< Scale = 0.02f
}
void GltfDX::Process(const fx::gltf::Node& Nd, const uint32_t i)
{
	auto& Mtx = CurrentMatrix.back();

	if (fx::gltf::defaults::IdentityMatrix != Nd.matrix) {
		const auto Local = DirectX::XMFLOAT4X4(data(Nd.matrix));
		Mtx *= DirectX::XMLoadFloat4x4(&Local);
	}
	else {
		if (fx::gltf::defaults::NullVec3 != Nd.translation) {
			const auto Local = DirectX::XMFLOAT3(data(Nd.translation));
			Mtx *= DirectX::XMMatrixTranslationFromVector(DirectX::XMLoadFloat3(&Local));
		}
		if (fx::gltf::defaults::IdentityRotation != Nd.rotation) {
			const auto Local = DirectX::XMFLOAT4(data(Nd.rotation));
			Mtx *= DirectX::XMMatrixRotationQuaternion(DirectX::XMLoadFloat4(&Local));
		}
		if (fx::gltf::defaults::IdentityVec3 != Nd.scale) {
			const auto Local = DirectX::XMFLOAT3(data(Nd.scale));
			Mtx *= DirectX::XMMatrixScalingFromVector(DirectX::XMLoadFloat3(&Local));
		}
	}

	NodeMatrices[i] = Mtx;

	Gltf::Process(Nd, i);
}
void GltfDX::Process(const fx::gltf::Camera& Cam)
{
	Gltf::Process(Cam);

	Tr.View = CurrentMatrix.back();
#if 1
	Tr.View.r[3].m128_f32[0] = 0.0f;
	Tr.View.r[3].m128_f32[1] = 0.0f;
#endif

	switch (Cam.type) {
		using enum fx::gltf::Camera::Type;
	case None: break;
	case Orthographic:
		Tr.Projection = DirectX::XMMatrixOrthographicRH(Cam.orthographic.xmag, Cam.orthographic.ymag, Cam.orthographic.znear, Cam.orthographic.zfar);
		break;
	case Perspective:
		Tr.Projection = DirectX::XMMatrixPerspectiveFovRH(Cam.perspective.yfov, Cam.perspective.aspectRatio, Cam.perspective.znear, Cam.perspective.zfar);
		break;
	}

#ifdef DEBUG_STDOUT
	std::cout << "View =" << std::endl;
	std::cout << Tr.View;
	std::cout << "Projection =" << std::endl;
	std::cout << Tr.Projection;
	//!< VKとは順次が逆
	std::cout << "View * Projection = " << Tr.View * Tr.Projection;
#endif

#if 1
	CopyToUploadResource(COM_PTR_GET(ConstantBuffers[0].Resource), RoundUp256(sizeof(Tr)), &Tr);
#endif
}
void GltfDX::Process(const fx::gltf::Primitive& Prim)
{
	Gltf::Process(Prim);

	//!< セマンティックの頭文字から読み込むシェーダを決定 (Select shader file by semantic initial)
	std::vector<std::pair<std::string, UINT>> SemanticAndIndices;
	std::string SemnticInitial;
	for (const auto& i : Prim.attributes) {
		SemnticInitial += i.first.substr(0, 1);
	}
	const auto ShaderPath = GetBasePath() + TEXT("_") + std::wstring(cbegin(SemnticInitial), cend(SemnticInitial));
	std::vector<COM_PTR<ID3DBlob>> SBs = {};
	VERIFY_SUCCEEDED(D3DReadFileToBlob(data(ShaderPath + TEXT(".vs.cso")), COM_PTR_PUT(SBs.emplace_back())));
	VERIFY_SUCCEEDED(D3DReadFileToBlob(data(ShaderPath + TEXT(".ps.cso")), COM_PTR_PUT(SBs.emplace_back())));
	const auto VS = D3D12_SHADER_BYTECODE({ .pShaderBytecode = SBs[0]->GetBufferPointer(), .BytecodeLength = SBs[0]->GetBufferSize() });
	const auto PS = D3D12_SHADER_BYTECODE({ .pShaderBytecode = SBs[1]->GetBufferPointer(), .BytecodeLength = SBs[1]->GetBufferSize() });

	//!< セマンティックとインデックスのリストを作る (Create semantic and index list)
	for (const auto& i : Prim.attributes) {
		std::string Name, IndexStr;
		if (DecomposeSemantic(i.first, Name, IndexStr)) {
			SemanticAndIndices.emplace_back(std::pair<std::string, UINT>({ data(Name), std::stoi(IndexStr) }));
		}
		else {
			SemanticAndIndices.emplace_back(std::pair<std::string, UINT>({ data(i.first), 0 }));
		}
	}
	auto MorphIndex = 1;
	for (const auto& i : Prim.targets) {
		for (const auto& j : i) {
			std::string Name, IndexStr;
			if (DecomposeSemantic(j.first, Name, IndexStr)) {
				SemanticAndIndices.emplace_back(std::pair<std::string, UINT>({ data(Name), std::stoi(IndexStr) }));
			}
			else {
				SemanticAndIndices.emplace_back(std::pair<std::string, UINT>({ data(j.first), MorphIndex }));
			}
		}
		++MorphIndex;
	}

	const auto& Doc = GetDocument();
	//!< アトリビュート (Attributes)
	std::vector<D3D12_INPUT_ELEMENT_DESC> IEDs;
	UINT InputSlot = 0;
	for (const auto& i : Prim.attributes) {
		const auto& Sem = SemanticAndIndices[InputSlot];
		const auto& Acc = Doc.accessors[i.second];
		IEDs.emplace_back(D3D12_INPUT_ELEMENT_DESC({ .SemanticName = data(Sem.first), .SemanticIndex = Sem.second, .Format = ToDXFormat(Acc), .InputSlot = InputSlot, .AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT, .InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, .InstanceDataStepRate = 0 }));
		++InputSlot;
	}
	//!< モーフターゲット (Morph target)
	for (const auto& i : Prim.targets) {
		for (const auto& j : i) {
			const auto& Sem = SemanticAndIndices[InputSlot];
			const auto& Acc = Doc.accessors[j.second];
			IEDs.emplace_back(D3D12_INPUT_ELEMENT_DESC({ .SemanticName = data(Sem.first), .SemanticIndex = Sem.second, .Format = ToDXFormat(Acc), .InputSlot = InputSlot, .AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT, .InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, .InstanceDataStepRate = 0 }));
			++InputSlot;
		}
	}
	const std::vector RTVs = { DXGI_FORMAT_R8G8B8A8_UNORM };

	const auto RS = COM_PTR_GET(RootSignatures[0]);
	PipelineStates.emplace_back(COM_PTR<ID3D12PipelineState>());
	const std::vector RTBDs = {
		D3D12_RENDER_TARGET_BLEND_DESC({
			.BlendEnable = FALSE, .LogicOpEnable = FALSE,
			.SrcBlend = D3D12_BLEND_ONE, .DestBlend = D3D12_BLEND_ZERO, .BlendOp = D3D12_BLEND_OP_ADD,
			.SrcBlendAlpha = D3D12_BLEND_ONE, .DestBlendAlpha = D3D12_BLEND_ZERO, .BlendOpAlpha = D3D12_BLEND_OP_ADD,
			.LogicOp = D3D12_LOGIC_OP_NOOP,
			.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL,
		}),
	};
	const D3D12_RASTERIZER_DESC RD = {
		.FillMode = D3D12_FILL_MODE_SOLID,
		.CullMode = D3D12_CULL_MODE_BACK, .FrontCounterClockwise = TRUE,
		.DepthBias = D3D12_DEFAULT_DEPTH_BIAS, .DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP, .SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS,
		.DepthClipEnable = TRUE,
		.MultisampleEnable = FALSE, .AntialiasedLineEnable = FALSE, .ForcedSampleCount = 0,
		.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF
	};
	const D3D12_DEPTH_STENCILOP_DESC DSOD = { .StencilFailOp = D3D12_STENCIL_OP_KEEP, .StencilDepthFailOp = D3D12_STENCIL_OP_KEEP, .StencilPassOp = D3D12_STENCIL_OP_KEEP, .StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS };
	const D3D12_DEPTH_STENCIL_DESC DSD = {
		.DepthEnable = TRUE, .DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL, .DepthFunc = D3D12_COMPARISON_FUNC_LESS,
		.StencilEnable = FALSE, .StencilReadMask = D3D12_DEFAULT_STENCIL_READ_MASK, .StencilWriteMask = D3D12_DEFAULT_STENCIL_WRITE_MASK,
		.FrontFace = DSOD, .BackFace = DSOD
	};
	DX::CreatePipelineState_(std::ref(PipelineStates.back()), COM_PTR_GET(Device), RS, ToDXPrimitiveTopologyType(Prim.mode), RTBDs, RD, DSD, VS, PS, NullSBC, NullSBC, NullSBC, IEDs, RTVs);

	DXGI_SWAP_CHAIN_DESC1 SCD;
	SwapChain->GetDesc1(&SCD);
	for (UINT i = 0; i < SCD.BufferCount; ++i) {
		VERIFY_SUCCEEDED(Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_BUNDLE, COM_PTR_GET(BundleCommandAllocators[0]), nullptr, COM_PTR_UUIDOF_PUTVOID(BundleGraphicsCommandLists.emplace_back())));
		VERIFY_SUCCEEDED(BundleGraphicsCommandLists[i]->Close());
	}
	const auto Count = SCD.BufferCount;
	
	const auto PST = COM_PTR_GET(PipelineStates.back());

#ifdef DEBUG_STDOUT
	std::cout << "World =" << std::endl;
	std::cout << CurrentMatrix.back();
#endif

	const auto BCA = COM_PTR_GET(BundleCommandAllocators.back());
	for (auto i = 0; i < static_cast<int>(Count); ++i) {
		const auto BGCL = COM_PTR_GET(BundleGraphicsCommandLists[size(BundleGraphicsCommandLists) - Count + i]);
		VERIFY_SUCCEEDED(BGCL->Reset(BCA, PST));
		{
			//const std::array<D3D12_VERTEX_BUFFER_VIEW, 1> VBVs = { VertexBuffers.back().View };
			std::vector<D3D12_VERTEX_BUFFER_VIEW> VBVs; for (auto j : VertexBuffers) { VBVs.emplace_back(j.View); }
			const auto IBV = IndexBuffers.back().View;
			const auto IDBCS = COM_PTR_GET(IndirectBuffers.back().CommandSignature);
			const auto IDBR = COM_PTR_GET(IndirectBuffers.back().Resource);

#if 0
			BGCL->SetGraphicsRootSignature(RS);
			//BGCL->SetGraphicsRoot32BitConstants(0, static_cast<UINT>(sizeof(CurrentMatrix.back()) / 4), &CurrentMatrix.back(), 0);

			const auto& DH = CbvSrvUavDescriptorHeaps[0];

			const std::array DHs = { COM_PTR_GET(DH) };
			BGCL->SetDescriptorHeaps(static_cast<UINT>(size(DHs)), data(DHs));

			auto GDH = DH->GetGPUDescriptorHandleForHeapStart();
			BGCL->SetGraphicsRootDescriptorTable(0, GDH); GDH.ptr += Device->GetDescriptorHandleIncrementSize(DH->GetDesc().Type);
#endif
			BGCL->IASetPrimitiveTopology(ToDXPrimitiveTopology(Prim.mode));
			BGCL->IASetVertexBuffers(0, static_cast<UINT>(size(VBVs)), data(VBVs));
			BGCL->IASetIndexBuffer(&IBV);
			BGCL->ExecuteIndirect(IDBCS, 1, IDBR, 0, nullptr, 0);
		}
		VERIFY_SUCCEEDED(BGCL->Close());
	}
}

void GltfDX::Process(const std::string& Identifier, const fx::gltf::Accessor& Acc)
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

			const auto CA = COM_PTR_GET(CommandAllocators[0]);
			const auto GCL = COM_PTR_GET(GraphicsCommandLists[0]);
			if ("indices" == Identifier) {
				IndexBuffers.emplace_back().Create(COM_PTR_GET(Device), Size, ToDXFormat(Acc.componentType)).ExecuteCopyCommand(COM_PTR_GET(Device), CA, GCL, COM_PTR_GET(GraphicsCommandQueue), COM_PTR_GET(Fence), Size, Data);

				const D3D12_DRAW_INDEXED_ARGUMENTS DIA = { .IndexCountPerInstance = Acc.count, .InstanceCount = 1, .StartIndexLocation = 0, .BaseVertexLocation = 0, .StartInstanceLocation = 0 };
				IndirectBuffers.emplace_back().Create(COM_PTR_GET(Device), DIA).ExecuteCopyCommand(COM_PTR_GET(Device), CA, GCL, COM_PTR_GET(GraphicsCommandQueue), COM_PTR_GET(Fence), sizeof(DIA), &DIA);
			}
			else if ("attributes" == Identifier || "targets" == Identifier) {
				VertexBuffers.emplace_back().Create(COM_PTR_GET(Device), Size, Stride).ExecuteCopyCommand(COM_PTR_GET(Device), CA, GCL, COM_PTR_GET(GraphicsCommandQueue), COM_PTR_GET(Fence), Size, Data);
			}
			else if ("inverseBindMatrices" == Identifier) {
				InverseBindMatrices.reserve(Acc.count);
				for (uint32_t i = 0; i < Acc.count; ++i) {
					InverseBindMatrices.emplace_back(reinterpret_cast<const DirectX::XMMATRIX*>(Data + Stride * i));
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
void GltfDX::Process(const fx::gltf::Mesh& Msh)
{
	Gltf::Process(Msh);

	MorphWeights = Msh.weights;
}
void GltfDX::Process(const fx::gltf::Skin& Skn)
{
	Gltf::Process(Skn);

	JointMatrices.reserve(size(Skn.joints));
	for (uint32_t i = 0; i < size(Skn.joints); ++i) {
		const auto& IBM = *InverseBindMatrices[i];

		auto Wld = DirectX::XMMatrixIdentity();
		const auto& Nd = GetDocument().nodes[Skn.joints[i]];
		if (fx::gltf::defaults::NullVec3 != Nd.translation) {
			const auto Local = DirectX::XMFLOAT3(data(Nd.translation));
			Wld *= DirectX::XMMatrixTranslationFromVector(DirectX::XMLoadFloat3(&Local));
		}
		if (fx::gltf::defaults::IdentityRotation != Nd.rotation) {
			const auto Local = DirectX::XMFLOAT4(data(Nd.rotation));
			Wld *= DirectX::XMMatrixRotationQuaternion(DirectX::XMLoadFloat4(&Local));
		}
		if (fx::gltf::defaults::IdentityVec3 != Nd.scale) {
			const auto Local = DirectX::XMFLOAT3(data(Nd.scale));
			Wld *= DirectX::XMMatrixScalingFromVector(DirectX::XMLoadFloat3(&Local));
		}
		
		JointMatrices.emplace_back(Wld * IBM);
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
void GltfDX::OnTimer(HWND hWnd, HINSTANCE hInstance)
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

void GltfDX::UpdateAnimTranslation(const std::array<float, 3>& Value, const uint32_t NodeIndex)
{
	if (-1 != NodeIndex) {
		const auto Local = DirectX::XMFLOAT3(data(Value));
		AnimNodeMatrices[NodeIndex] *= DirectX::XMMatrixTranslationFromVector(DirectX::XMLoadFloat3(&Local));
	}
}
void GltfDX::UpdateAnimScale(const std::array<float, 3>& Value, const uint32_t NodeIndex)
{
	if (-1 != NodeIndex) {
		const auto Local = DirectX::XMFLOAT3(data(Value));
		AnimNodeMatrices[NodeIndex] *= DirectX::XMMatrixScalingFromVector(DirectX::XMLoadFloat3(&Local));
	}
}
void GltfDX::UpdateAnimRotation(const std::array<float, 4>& Value, const uint32_t NodeIndex)
{
	if (-1 != NodeIndex) {
		const auto Local = DirectX::XMFLOAT4(data(Value));
		//!< クォータニオンは要正規化
		AnimNodeMatrices[NodeIndex] *= DirectX::XMMatrixRotationQuaternion(DirectX::XMQuaternionNormalize(DirectX::XMLoadFloat4(&Local)));
	}
}
void GltfDX::UpdateAnimWeights([[maybe_unused]] const float* Data, [[maybe_unused]] const uint32_t PrevIndex, [[maybe_unused]] const uint32_t NextIndex, [[maybe_unused]] const float t)
{
}

void GltfDX::PopulateCommandList(const size_t i)
{
	DXGI_SWAP_CHAIN_DESC1 SCD;
	SwapChain->GetDesc1(&SCD);
	const auto PrimCount = size(BundleGraphicsCommandLists) / SCD.BufferCount;
	std::vector<ID3D12GraphicsCommandList*> BGCLs;
	for (auto j = 0; j < PrimCount; ++j) {
		BGCLs.emplace_back(COM_PTR_GET(BundleGraphicsCommandLists[j * SCD.BufferCount + i]));
	}

	const auto CA = COM_PTR_GET(CommandAllocators[0]);
	const auto GCL = COM_PTR_GET(GraphicsCommandLists[i]);
	VERIFY_SUCCEEDED(GCL->Reset(CA, nullptr));
	{
		const auto RS = COM_PTR_GET(RootSignatures[0]);
		const auto SCR = COM_PTR_GET(SwapChainResources[i]);
		
		GCL->SetGraphicsRootSignature(RS);

		GCL->RSSetViewports(static_cast<UINT>(size(Viewports)), data(Viewports));
		GCL->RSSetScissorRects(static_cast<UINT>(size(ScissorRects)), data(ScissorRects));

		//GCL->SetGraphicsRoot32BitConstants(0, static_cast<UINT>(sizeof(ViewProjection) / 4), &ViewProjection, 0);

		ResourceBarrier(GCL, SCR, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
		{
			auto SCDH = SwapChainDescriptorHeap->GetCPUDescriptorHandleForHeapStart(); SCDH.ptr += i * Device->GetDescriptorHandleIncrementSize(SwapChainDescriptorHeap->GetDesc().Type);
			const auto DDH = DsvDescriptorHeaps[0]->GetCPUDescriptorHandleForHeapStart();

			constexpr std::array<D3D12_RECT, 0> Rects = {};
			GCL->ClearRenderTargetView(SCDH, DirectX::Colors::SkyBlue, static_cast<UINT>(size(Rects)), data(Rects));
			GCL->ClearDepthStencilView(DDH, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, static_cast<UINT>(size(Rects)), data(Rects));

			const std::array RTDHs = { SCDH };
			GCL->OMSetRenderTargets(static_cast<UINT>(size(RTDHs)), data(RTDHs), FALSE, &DDH);

			{
				const auto& DH = CbvSrvUavDescriptorHeaps[0];

				const std::array DHs = { COM_PTR_GET(DH) };
				GCL->SetDescriptorHeaps(static_cast<UINT>(size(DHs)), data(DHs));

				auto GDH = DH->GetGPUDescriptorHandleForHeapStart();
				GCL->SetGraphicsRootDescriptorTable(0, GDH); GDH.ptr += Device->GetDescriptorHandleIncrementSize(DH->GetDesc().Type);
			}

			for (auto j : BGCLs) { GCL->ExecuteBundle(j); }
		}
		ResourceBarrier(GCL, SCR, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
	}
	VERIFY_SUCCEEDED(GCL->Close());
}
#pragma endregion