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
void GltfDX::PreProcess()
{
	const auto Fov = 0.16f * DirectX::XM_PI;
	const auto Aspect = GetAspectRatioOfClientRect();
	const auto ZFar = 100.0f;
	const auto ZNear = ZFar * 0.0001f;
	PV.Projection = DirectX::XMMatrixPerspectiveFovRH(Fov, Aspect, ZNear, ZFar);

	const auto CamPos = DirectX::XMVectorSet(0.0f, 0.0f, 6.0f, 1.0f);
	const auto CamTag = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
	const auto CamUp = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	PV.View = DirectX::XMMatrixLookAtRH(CamPos, CamTag, CamUp);

#if 1
	auto CBSize = RoundUp(sizeof(PV), 0xff);

	//!< デスクリプタヒープ
	const D3D12_DESCRIPTOR_HEAP_DESC DHD = { D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 1, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE, 0 };
	VERIFY_SUCCEEDED(Device->CreateDescriptorHeap(&DHD, COM_PTR_UUIDOF_PUTVOID(ConstantBufferDescriptorHeap)));

	//!< コンスタントバッファ
	ConstantBuffers.push_back(COM_PTR<ID3D12Resource>());
	CreateUploadResource(COM_PTR_PUT(ConstantBuffers[0]), CBSize);

	//!< ビュー
	const D3D12_CONSTANT_BUFFER_VIEW_DESC CBVD = { COM_PTR_GET(ConstantBuffers[0])->GetGPUVirtualAddress(), static_cast<UINT>(CBSize) };
	Device->CreateConstantBufferView(&CBVD, GetCPUDescriptorHandle(COM_PTR_GET(ConstantBufferDescriptorHeap), 0));
#endif
}
void GltfDX::PostProcess()
{
}
void GltfDX::Process(const fx::gltf::Node& Nd, const uint32_t i)
{
	auto& Mtx = CurrentMatrix.back();

	if (fx::gltf::defaults::IdentityMatrix != Nd.matrix) {
		const auto Local = DirectX::XMFLOAT4X4(Nd.matrix.data());
		Mtx *= DirectX::XMLoadFloat4x4(&Local);
	}
	else {
		if (fx::gltf::defaults::NullVec3 != Nd.translation) {
			const auto Local = DirectX::XMFLOAT3(Nd.translation.data());
			Mtx *= DirectX::XMMatrixTranslationFromVector(DirectX::XMLoadFloat3(&Local));
		}
		if (fx::gltf::defaults::IdentityRotation != Nd.rotation) {
			const auto Local = DirectX::XMFLOAT4(Nd.rotation.data());
			Mtx *= DirectX::XMMatrixRotationQuaternion(DirectX::XMLoadFloat4(&Local));
		}
		if (fx::gltf::defaults::IdentityVec3 != Nd.scale) {
			const auto Local = DirectX::XMFLOAT3(Nd.scale.data());
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

#if 1
	switch (Cam.type) {
	case fx::gltf::Camera::Type::None: break;
	case fx::gltf::Camera::Type::Orthographic:
		PV.Projection = DirectX::XMMatrixOrthographicRH(Cam.orthographic.xmag, Cam.orthographic.ymag, Cam.orthographic.znear, Cam.orthographic.zfar);
		break;
	case fx::gltf::Camera::Type::Perspective:
		PV.Projection = DirectX::XMMatrixPerspectiveFovRH(Cam.perspective.yfov, Cam.perspective.aspectRatio, Cam.perspective.znear, Cam.perspective.zfar);
		break;
	}
#endif

#ifdef DEBUG_STDOUT
	std::cout << "View =" << std::endl;
	std::cout << PV.View;
	std::cout << "Projection =" << std::endl;
	std::cout << PV.Projection;
	//!< VKとは順次が逆
	std::cout << "View * Projection = " << PV.View * PV.Projection;
#endif

#if 1
	CopyToUploadResource(COM_PTR_GET(ConstantBuffers[0]), RoundUp(sizeof(PV), 0xff), &PV);
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
	const auto ShaderPath = GetBasePath() + TEXT("_") + std::wstring(SemnticInitial.begin(), SemnticInitial.end());
	ShaderBlobs.push_back(COM_PTR<ID3DBlob>());
	VERIFY_SUCCEEDED(D3DReadFileToBlob((ShaderPath + TEXT(".vs.cso")).data(), COM_PTR_PUT(ShaderBlobs.back())));
	const auto VS = ShaderBlobs.back();
	ShaderBlobs.push_back(COM_PTR<ID3DBlob>());
	VERIFY_SUCCEEDED(D3DReadFileToBlob((ShaderPath + TEXT(".ps.cso")).data(), COM_PTR_PUT(ShaderBlobs.back())));
	const auto PS = ShaderBlobs.back();

	//!< セマンティックとインデックスのリストを作る (Create semantic and index list)
	for (const auto& i : Prim.attributes) {
		std::string Name, Index;
		if (DecomposeSemantic(i.first, Name, Index)) {
			SemanticAndIndices.push_back({ Name.c_str(), std::stoi(Index) });
		}
		else {
			SemanticAndIndices.push_back({ i.first.c_str(), 0 });
		}
	}
	auto MorphIndex = 1;
	for (const auto& i : Prim.targets) {
		for (const auto& j : i) {
			std::string Name, Index;
			if (DecomposeSemantic(j.first, Name, Index)) {
				SemanticAndIndices.push_back({ Name.c_str(), std::stoi(Index) });
			}
			else {
				SemanticAndIndices.push_back({ j.first.c_str(), MorphIndex });
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
		IEDs.push_back({ Sem.first.c_str(), Sem.second, ToDXFormat(Acc), InputSlot, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
		++InputSlot;
	}
	//!< モーフターゲット (Morph target)
	for (const auto& i : Prim.targets) {
		for (const auto& j : i) {
			const auto& Sem = SemanticAndIndices[InputSlot];
			const auto& Acc = Doc.accessors[j.second];
			IEDs.push_back({ Sem.first.c_str(), Sem.second, ToDXFormat(Acc), InputSlot, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
			++InputSlot;
		}
	}

	const auto RS = COM_PTR_GET(RootSignatures[0]);
	PipelineStates.push_back(COM_PTR<ID3D12PipelineState>());
	DX::CreatePipelineState(std::ref(PipelineStates.back()), RS, { VS->GetBufferPointer(), VS->GetBufferSize() }, { PS->GetBufferPointer(), PS->GetBufferSize() }, NullShaderBC, NullShaderBC, NullShaderBC, IEDs, ToDXPrimitiveTopologyType(Prim.mode));

	const auto Count = AddBundleCommandList();
	const auto BCA = COM_PTR_GET(BundleCommandAllocators.back());
	const auto PST = COM_PTR_GET(PipelineStates.back());

	const auto& VBVs = VertexBufferViews;
	const auto& IBV = IndexBufferViews.back();
	const auto IBR = COM_PTR_GET(IndirectBufferResources.back());
	const auto ICS = COM_PTR_GET(IndirectCommandSignatures.back());

#ifdef DEBUG_STDOUT
	std::cout << "World =" << std::endl;
	std::cout << CurrentMatrix.back();
#endif

	for (auto i = 0; i < static_cast<int>(Count); ++i) {
		const auto BCL = COM_PTR_GET(BundleGraphicsCommandLists[BundleGraphicsCommandLists.size() - Count + i]);
		const auto SCH = GetCPUDescriptorHandle(COM_PTR_GET(SwapChainDescriptorHeap), static_cast<UINT>(i));

		VERIFY_SUCCEEDED(BCL->Reset(BCA, PST));
		{
#if 1
			BCL->SetGraphicsRootSignature(RS);
			//BCL->SetGraphicsRoot32BitConstants(0, static_cast<UINT>(sizeof(CurrentMatrix.back()) / 4), &CurrentMatrix.back(), 0);
			const std::array<ID3D12DescriptorHeap*, 1> DHs = { COM_PTR_GET(ConstantBufferDescriptorHeap) };
			BCL->SetDescriptorHeaps(static_cast<UINT>(DHs.size()), DHs.data());
			BCL->SetGraphicsRootDescriptorTable(0, GetGPUDescriptorHandle(COM_PTR_GET(ConstantBufferDescriptorHeap), 0));
#endif
			BCL->IASetPrimitiveTopology(ToDXPrimitiveTopology(Prim.mode));
			BCL->IASetVertexBuffers(0, static_cast<UINT>(VBVs.size()), VBVs.data());
			BCL->IASetIndexBuffer(&IBV);
			BCL->ExecuteIndirect(ICS, 1, IBR, 0, nullptr, 0);
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
			case fx::gltf::BufferView::TargetType::None: break;
			case fx::gltf::BufferView::TargetType::ArrayBuffer: break;
			case fx::gltf::BufferView::TargetType::ElementArrayBuffer: break;
			}

			if ("indices" == Identifier) {
				IndexBufferResources.push_back(COM_PTR<ID3D12Resource>());

				CreateAndCopyToDefaultResource(IndexBufferResources.back(), Size, Data, COM_PTR_GET(CommandAllocators[0]), COM_PTR_GET(GraphicsCommandLists[0]));
				IndexBufferViews.push_back({ IndexBufferResources.back()->GetGPUVirtualAddress(), Size, ToDXFormat(Acc.componentType) });

				CreateIndirectBuffer_DrawIndexed(Acc.count, 1);
			}
			else if ("attributes" == Identifier || "targets" == Identifier) {
				VertexBufferResources.push_back(COM_PTR<ID3D12Resource>());

				CreateAndCopyToDefaultResource(VertexBufferResources.back(), Size, Data, COM_PTR_GET(CommandAllocators[0]), COM_PTR_GET(GraphicsCommandLists[0]));
				VertexBufferViews.push_back({ VertexBufferResources.back()->GetGPUVirtualAddress(), Size, Stride });
			}
			else if ("inverseBindMatrices" == Identifier) {
				InverseBindMatrices.reserve(Acc.count);
				for (uint32_t i = 0; i < Acc.count; ++i) {
					InverseBindMatrices.push_back(reinterpret_cast<const DirectX::XMMATRIX*>(Data + Stride * i));
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
void GltfDX::Process(const fx::gltf::Mesh& Msh)
{
	Gltf::Process(Msh);

	MorphWeights = Msh.weights;
}
void GltfDX::Process(const fx::gltf::Skin& Skn)
{
	Gltf::Process(Skn);

	JointMatrices.reserve(Skn.joints.size());
	for (uint32_t i = 0; i < Skn.joints.size(); ++i) {
		const auto& IBM = *InverseBindMatrices[i];

		auto Wld = DirectX::XMMatrixIdentity();
		const auto& Nd = GetDocument().nodes[Skn.joints[i]];
		if (fx::gltf::defaults::NullVec3 != Nd.translation) {
			const auto Local = DirectX::XMFLOAT3(Nd.translation.data());
			Wld *= DirectX::XMMatrixTranslationFromVector(DirectX::XMLoadFloat3(&Local));
		}
		if (fx::gltf::defaults::IdentityRotation != Nd.rotation) {
			const auto Local = DirectX::XMFLOAT4(Nd.rotation.data());
			Wld *= DirectX::XMMatrixRotationQuaternion(DirectX::XMLoadFloat4(&Local));
		}
		if (fx::gltf::defaults::IdentityVec3 != Nd.scale) {
			const auto Local = DirectX::XMFLOAT3(Nd.scale.data());
			Wld *= DirectX::XMMatrixScalingFromVector(DirectX::XMLoadFloat3(&Local));
		}
		
		JointMatrices.push_back(Wld * IBM);
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
void GltfDX::OnTimer(HWND hWnd, HINSTANCE hInstance)
{
	Super::OnTimer(hWnd, hInstance);

	CurrentFrame += 0.1f; //static_cast<float>(Elapse) / 1000.0f;

	const auto bLoop = true;
	//const auto bLoop = false;
	UpdateAnimation(CurrentFrame, bLoop);
}

void GltfDX::UpdateAnimTranslation(const std::array<float, 3>& Value, const uint32_t NodeIndex)
{
	if (-1 != NodeIndex) {
		const auto Local = DirectX::XMFLOAT3(Value.data());
		NodeMatrices[NodeIndex] * DirectX::XMMatrixTranslationFromVector(DirectX::XMLoadFloat3(&Local));
	}
}
void GltfDX::UpdateAnimScale(const std::array<float, 3>& Value, const uint32_t NodeIndex)
{
	if (-1 != NodeIndex) {
		const auto Local = DirectX::XMFLOAT3(Value.data());
		NodeMatrices[NodeIndex] *= DirectX::XMMatrixScalingFromVector(DirectX::XMLoadFloat3(&Local));
	}
}
void GltfDX::UpdateAnimRotation(const std::array<float, 4>& Value, const uint32_t NodeIndex)
{
	if (-1 != NodeIndex) {
		const auto Local = DirectX::XMFLOAT4(Value.data());
		NodeMatrices[NodeIndex] * DirectX::XMMatrixRotationQuaternion(DirectX::XMLoadFloat4(&Local));
	}
}
void GltfDX::UpdateAnimWeights(const float* /*Data*/, const uint32_t /*PrevIndex*/, const uint32_t /*NextIndex*/, const float /*t*/)
{

}

void GltfDX::PopulateCommandList(const size_t i)
{
	const auto CA = COM_PTR_GET(CommandAllocators[0]);
	const auto CL = COM_PTR_GET(GraphicsCommandLists[i]);
	const auto SCR = COM_PTR_GET(SwapChainResources[i]);
	const auto SCH = GetCPUDescriptorHandle(COM_PTR_GET(SwapChainDescriptorHeap), static_cast<UINT>(i));
	const auto RS = COM_PTR_GET(RootSignatures[0]);

	DXGI_SWAP_CHAIN_DESC1 SCD;
	SwapChain->GetDesc1(&SCD);
	const auto PrimCount = BundleGraphicsCommandLists.size() / SCD.BufferCount;
	std::vector<ID3D12GraphicsCommandList*> BCLs;
	for (auto j = 0; j < PrimCount; ++j) {
		BCLs.push_back(COM_PTR_GET(BundleGraphicsCommandLists[j * SCD.BufferCount + i]));
	}

	VERIFY_SUCCEEDED(CL->Reset(CA, nullptr));
	{
		CL->RSSetViewports(static_cast<UINT>(Viewports.size()), Viewports.data());
		CL->RSSetScissorRects(static_cast<UINT>(ScissorRects.size()), ScissorRects.data());

		CL->SetGraphicsRootSignature(RS);

		//CL->SetGraphicsRoot32BitConstants(0, static_cast<UINT>(sizeof(ViewProjection) / 4), &ViewProjection, 0);
		//const std::array<ID3D12DescriptorHeap*, 1> DHs = { COM_PTR_GET(ConstantBufferDescriptorHeap) };
		//CL->SetDescriptorHeaps(static_cast<UINT>(DHs.size()), DHs.data());
		//CL->SetGraphicsRootDescriptorTable(0, GetGPUDescriptorHandle(COM_PTR_GET(ConstantBufferDescriptorHeap), 0));

		ResourceBarrier(CL, SCR, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
		{
			const std::array<D3D12_RECT, 0> Rs = {};
			CL->ClearRenderTargetView(SCH, DirectX::Colors::SkyBlue, static_cast<UINT>(Rs.size()), Rs.data());

			const std::array<D3D12_CPU_DESCRIPTOR_HANDLE, 1> RTDHs = { SCH };
			CL->OMSetRenderTargets(static_cast<UINT>(RTDHs.size()), RTDHs.data(), FALSE, nullptr);

			for (auto j : BCLs) { 
				CL->ExecuteBundle(j);
			}
		}
		ResourceBarrier(CL, SCR, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
	}
	VERIFY_SUCCEEDED(CL->Close());
}
#pragma endregion