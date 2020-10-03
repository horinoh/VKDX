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

	PV.View = CurrentMatrix.back();
#if 1
	PV.View.r[3].m128_f32[0] = 0.0f;
	PV.View.r[3].m128_f32[1] = 0.0f;
#endif

	switch (Cam.type) {
		using enum fx::gltf::Camera::Type;
	case None: break;
	case Orthographic:
		PV.Projection = DirectX::XMMatrixOrthographicRH(Cam.orthographic.xmag, Cam.orthographic.ymag, Cam.orthographic.znear, Cam.orthographic.zfar);
		break;
	case Perspective:
		PV.Projection = DirectX::XMMatrixPerspectiveFovRH(Cam.perspective.yfov, Cam.perspective.aspectRatio, Cam.perspective.znear, Cam.perspective.zfar);
		break;
	}

#ifdef DEBUG_STDOUT
	std::cout << "View =" << std::endl;
	std::cout << PV.View;
	std::cout << "Projection =" << std::endl;
	std::cout << PV.Projection;
	//!< VKとは順次が逆
	std::cout << "View * Projection = " << PV.View * PV.Projection;
#endif

#if 1
	CopyToUploadResource(COM_PTR_GET(ConstantBuffers[0].Resource), RoundUp256(sizeof(PV)), &PV);
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
	ShaderBlobs.emplace_back(COM_PTR<ID3DBlob>());
	VERIFY_SUCCEEDED(D3DReadFileToBlob(data(ShaderPath + TEXT(".vs.cso")), COM_PTR_PUT(ShaderBlobs.back())));
	const auto VS = D3D12_SHADER_BYTECODE({ ShaderBlobs.back()->GetBufferPointer(), ShaderBlobs.back()->GetBufferSize() });
	ShaderBlobs.emplace_back(COM_PTR<ID3DBlob>());
	VERIFY_SUCCEEDED(D3DReadFileToBlob(data(ShaderPath + TEXT(".ps.cso")), COM_PTR_PUT(ShaderBlobs.back())));
	const auto PS = D3D12_SHADER_BYTECODE({ ShaderBlobs.back()->GetBufferPointer(), ShaderBlobs.back()->GetBufferSize() });

	//!< セマンティックとインデックスのリストを作る (Create semantic and index list)
	for (const auto& i : Prim.attributes) {
		std::string Name, Index;
		if (DecomposeSemantic(i.first, Name, Index)) {
			SemanticAndIndices.emplace_back(std::pair<std::string, UINT>({ Name.c_str(), std::stoi(Index) }));
		}
		else {
			SemanticAndIndices.emplace_back(std::pair<std::string, UINT>({ i.first.c_str(), 0 }));
		}
	}
	auto MorphIndex = 1;
	for (const auto& i : Prim.targets) {
		for (const auto& j : i) {
			std::string Name, Index;
			if (DecomposeSemantic(j.first, Name, Index)) {
				SemanticAndIndices.emplace_back(std::pair<std::string, UINT>({ Name.c_str(), std::stoi(Index) }));
			}
			else {
				SemanticAndIndices.emplace_back(std::pair<std::string, UINT>({ j.first.c_str(), MorphIndex }));
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
		IEDs.emplace_back(D3D12_INPUT_ELEMENT_DESC({ .SemanticName = Sem.first.c_str(), .SemanticIndex = Sem.second, .Format = ToDXFormat(Acc), .InputSlot = InputSlot, .AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT, .InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, .InstanceDataStepRate = 0 }));
		++InputSlot;
	}
	//!< モーフターゲット (Morph target)
	for (const auto& i : Prim.targets) {
		for (const auto& j : i) {
			const auto& Sem = SemanticAndIndices[InputSlot];
			const auto& Acc = Doc.accessors[j.second];
			IEDs.emplace_back(D3D12_INPUT_ELEMENT_DESC({ .SemanticName = Sem.first.c_str(), .SemanticIndex = Sem.second, .Format = ToDXFormat(Acc), .InputSlot = InputSlot, .AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT, .InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, .InstanceDataStepRate = 0 }));
			++InputSlot;
		}
	}
	const std::vector RTVs = { DXGI_FORMAT_R8G8B8A8_UNORM };

	const auto RS = COM_PTR_GET(RootSignatures[0]);
	PipelineStates.emplace_back(COM_PTR<ID3D12PipelineState>());
	const D3D12_RASTERIZER_DESC RD = {
		D3D12_FILL_MODE_SOLID,
		D3D12_CULL_MODE_BACK, TRUE,
		D3D12_DEFAULT_DEPTH_BIAS, D3D12_DEFAULT_DEPTH_BIAS_CLAMP, D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS,
		TRUE,
		FALSE, FALSE, 0,
		D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF
	};
	const D3D12_DEPTH_STENCILOP_DESC DSOD = { D3D12_STENCIL_OP_KEEP, D3D12_STENCIL_OP_KEEP, D3D12_STENCIL_OP_KEEP, D3D12_COMPARISON_FUNC_ALWAYS };
	const D3D12_DEPTH_STENCIL_DESC DSD = {
		TRUE, D3D12_DEPTH_WRITE_MASK_ALL, D3D12_COMPARISON_FUNC_LESS,
		FALSE, D3D12_DEFAULT_STENCIL_READ_MASK, D3D12_DEFAULT_STENCIL_WRITE_MASK,
		DSOD, DSOD
	};
	DX::CreatePipelineState(std::ref(PipelineStates.back()), COM_PTR_GET(Device), RS, ToDXPrimitiveTopologyType(Prim.mode), RD, DSD, VS, PS, NullShaderBC, NullShaderBC, NullShaderBC, IEDs, RTVs);

	DXGI_SWAP_CHAIN_DESC1 SCD;
	SwapChain->GetDesc1(&SCD);
	for (UINT i = 0; i < SCD.BufferCount; ++i) {
		BundleGraphicsCommandLists.push_back(COM_PTR<ID3D12GraphicsCommandList>());
		VERIFY_SUCCEEDED(Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_BUNDLE, COM_PTR_GET(BundleCommandAllocators[0]), nullptr, COM_PTR_UUIDOF_PUTVOID(BundleGraphicsCommandLists.back())));
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
		const auto BCL = COM_PTR_GET(BundleGraphicsCommandLists[size(BundleGraphicsCommandLists) - Count + i]);
		VERIFY_SUCCEEDED(BCL->Reset(BCA, PST));
		{
			//const std::array<D3D12_VERTEX_BUFFER_VIEW, 1> VBVs = { VertexBuffers.back().View };
			std::vector<D3D12_VERTEX_BUFFER_VIEW> VBVs; for (auto j : VertexBuffers) { VBVs.push_back(j.View); }
			const auto IBV = IndexBuffers.back().View;
			const auto IDBCS = COM_PTR_GET(IndirectBuffers.back().CommandSignature);
			const auto IDBR = COM_PTR_GET(IndirectBuffers.back().Resource);

#if 0
			BCL->SetGraphicsRootSignature(RS);
			//BCL->SetGraphicsRoot32BitConstants(0, static_cast<UINT>(sizeof(CurrentMatrix.back()) / 4), &CurrentMatrix.back(), 0);

			const auto& DH = CbvSrvUavDescriptorHeaps[0];

			const std::array DHs = { COM_PTR_GET(DH) };
			BCL->SetDescriptorHeaps(static_cast<UINT>(size(DHs)), data(DHs));

			auto GDH = DH->GetGPUDescriptorHandleForHeapStart();
			BCL->SetGraphicsRootDescriptorTable(0, GDH); GDH.ptr += Device->GetDescriptorHandleIncrementSize(DH->GetDesc().Type);
#endif
			BCL->IASetPrimitiveTopology(ToDXPrimitiveTopology(Prim.mode));
			BCL->IASetVertexBuffers(0, static_cast<UINT>(size(VBVs)), data(VBVs));
			BCL->IASetIndexBuffer(&IBV);
			BCL->ExecuteIndirect(IDBCS, 1, IDBR, 0, nullptr, 0);
		}
		VERIFY_SUCCEEDED(BCL->Close());
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

			if ("indices" == Identifier) {
				IndexBuffers.emplace_back(IndexBuffer());

				CreateAndCopyToDefaultResource(IndexBuffers.back().Resource, COM_PTR_GET(CommandAllocators[0]), COM_PTR_GET(GraphicsCommandLists[0]), Size, Data);
				IndexBuffers.back().View = { IndexBuffers.back().Resource->GetGPUVirtualAddress(), Size, ToDXFormat(Acc.componentType) };

				CreateIndirectBuffer_DrawIndexed(Acc.count, 1);
			}
			else if ("attributes" == Identifier || "targets" == Identifier) {
				VertexBuffers.emplace_back(VertexBuffer());

				CreateAndCopyToDefaultResource(VertexBuffers.back().Resource, COM_PTR_GET(CommandAllocators[0]), COM_PTR_GET(GraphicsCommandLists[0]), Size, Data);
				VertexBuffers.back().View = { VertexBuffers.back().Resource->GetGPUVirtualAddress(), Size, Stride };
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
	std::vector<ID3D12GraphicsCommandList*> BCLs;
	for (auto j = 0; j < PrimCount; ++j) {
		BCLs.emplace_back(COM_PTR_GET(BundleGraphicsCommandLists[j * SCD.BufferCount + i]));
	}

	const auto CA = COM_PTR_GET(CommandAllocators[0]);
	const auto CL = COM_PTR_GET(GraphicsCommandLists[i]);
	VERIFY_SUCCEEDED(CL->Reset(CA, nullptr));
	{
		const auto RS = COM_PTR_GET(RootSignatures[0]);
		const auto SCR = COM_PTR_GET(SwapChainResources[i]);
		
		CL->SetGraphicsRootSignature(RS);

		CL->RSSetViewports(static_cast<UINT>(size(Viewports)), data(Viewports));
		CL->RSSetScissorRects(static_cast<UINT>(size(ScissorRects)), data(ScissorRects));

		//CL->SetGraphicsRoot32BitConstants(0, static_cast<UINT>(sizeof(ViewProjection) / 4), &ViewProjection, 0);

		ResourceBarrier(CL, SCR, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
		{
			auto SCDH = SwapChainDescriptorHeap->GetCPUDescriptorHandleForHeapStart(); SCDH.ptr += i * Device->GetDescriptorHandleIncrementSize(SwapChainDescriptorHeap->GetDesc().Type);
			const auto DDH = DsvDescriptorHeaps[0]->GetCPUDescriptorHandleForHeapStart();

			const std::array<D3D12_RECT, 0> Rects = {};
			CL->ClearRenderTargetView(SCDH, DirectX::Colors::SkyBlue, static_cast<UINT>(size(Rects)), data(Rects));
			CL->ClearDepthStencilView(DDH, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, static_cast<UINT>(size(Rects)), data(Rects));

			const std::array RTDHs = { SCDH };
			CL->OMSetRenderTargets(static_cast<UINT>(size(RTDHs)), data(RTDHs), FALSE, &DDH);

			{
				const auto& DH = CbvSrvUavDescriptorHeaps[0];

				const std::array DHs = { COM_PTR_GET(DH) };
				CL->SetDescriptorHeaps(static_cast<UINT>(size(DHs)), data(DHs));

				auto GDH = DH->GetGPUDescriptorHandleForHeapStart();
				CL->SetGraphicsRootDescriptorTable(0, GDH); GDH.ptr += Device->GetDescriptorHandleIncrementSize(DH->GetDesc().Type);
			}

			for (auto j : BCLs) { CL->ExecuteBundle(j); }
		}
		ResourceBarrier(CL, SCR, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
	}
	VERIFY_SUCCEEDED(CL->Close());
}
#pragma endregion