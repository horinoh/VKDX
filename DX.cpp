#include "stdafx.h"

#include "DX.h"

//!< d3d12.lib が無いと言われる場合は、プロジェクトを右クリック - Retarget SDK Version で Windows10 にする
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")

DX::DX()
{
}
DX::~DX()
{
}

void DX::OnCreate(HWND hWnd, HINSTANCE hInstance)
{
	Super::OnCreate(hWnd, hInstance);

	CreateDevice(hWnd);
	CreateCommandQueue();

	CreateSwapChain(hWnd);

	CreateDepthStencil();

	CreateRootSignature();
	CreateInputLayout();
	CreateShader();
	CreateViewport();
	CreatePipelineState();
	CreateCommandList();

	CreateVertexBuffer();
	CreateIndexBuffer();
	CreateConstantBuffer();

	CreateFence();

	PopulateCommandList();
}
void DX::OnSize(HWND hWnd, HINSTANCE hInstance)
{
	Super::OnSize(hWnd, hInstance);
}
void DX::OnTimer(HWND hWnd, HINSTANCE hInstance)
{
	Super::OnTimer(hWnd, hInstance);
}
void DX::OnPaint(HWND hWnd, HINSTANCE hInstance)
{
	Super::OnPaint(hWnd, hInstance);

	//PopulateCommandList();

	ExecuteCommandList();

	Present();

	WaitForFence();
}
void DX::OnDestroy(HWND hWnd, HINSTANCE hInstance)
{
	Super::OnDestroy(hWnd, hInstance);

	WaitForFence();
	CloseHandle(FenceEvent);
}

void DX::CreateDevice(HWND hWnd)
{
	using namespace Microsoft::WRL;

#ifdef _DEBUG
	ComPtr<ID3D12Debug> Debug;
	VERIFY_SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&Debug)));
	Debug->EnableDebugLayer();
#endif

	ComPtr<IDXGIFactory4> Factory4;
	VERIFY_SUCCEEDED(CreateDXGIFactory1(IID_PPV_ARGS(&Factory4)));
	if (false/*WarpDevice*/) { // todo : WarpDevice は今のところやらない
		ComPtr<IDXGIAdapter> Adapter;
		VERIFY_SUCCEEDED(Factory4->EnumWarpAdapter(IID_PPV_ARGS(&Adapter)));
		VERIFY_SUCCEEDED(D3D12CreateDevice(Adapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&Device)));
	}
	else {
		ComPtr<IDXGIAdapter1> Adapter;
		for (UINT i = 0; DXGI_ERROR_NOT_FOUND != Factory4->EnumAdapters1(i, &Adapter); ++i) {
			DXGI_ADAPTER_DESC1 AdapterDesc1;
			Adapter->GetDesc1(&AdapterDesc1);
			if (AdapterDesc1.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) {
				continue;
			}
			if (SUCCEEDED(D3D12CreateDevice(Adapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&Device)))) {
				break;
			}
		}
	}
}
void DX::CreateCommandQueue()
{
	D3D12_COMMAND_QUEUE_DESC CommandQueueDesc = {};
	CommandQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	CommandQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	VERIFY_SUCCEEDED(Device->CreateCommandQueue(&CommandQueueDesc, IID_PPV_ARGS(&CommandQueue)));
}

void DX::CreateSwapChain(HWND hWnd, const UINT BufferCount)
{
	using namespace Microsoft::WRL;

	ComPtr<IDXGIFactory4> Factory4;
	VERIFY_SUCCEEDED(CreateDXGIFactory1(IID_PPV_ARGS(&Factory4)));

#pragma region SwapChain
	DXGI_SWAP_CHAIN_DESC1 SwapChainDesc1 = {};
	SwapChainDesc1.BufferCount = BufferCount;
	SwapChainDesc1.Width = GetClientRectWidth();
	SwapChainDesc1.Height = GetClientRectHeight();
	SwapChainDesc1.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	SwapChainDesc1.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	SwapChainDesc1.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	SwapChainDesc1.SampleDesc.Count = 1;

	ComPtr<IDXGISwapChain1> SwapChain1;
	VERIFY_SUCCEEDED(Factory4->CreateSwapChainForHwnd(CommandQueue.Get(), hWnd, &SwapChainDesc1, nullptr, nullptr, &SwapChain1));
	VERIFY_SUCCEEDED(Factory4->MakeWindowAssociation(hWnd, DXGI_MWA_NO_ALT_ENTER));
	VERIFY_SUCCEEDED(SwapChain1.As(&SwapChain3));
	CurrentBackBufferIndex = SwapChain3->GetCurrentBackBufferIndex();
#pragma endregion

	//!< デスクリプタヒープ(ビュー)を作成
#pragma region SwapChainView
	D3D12_DESCRIPTOR_HEAP_DESC DescripterHeapDesc = {};
	DescripterHeapDesc.NumDescriptors = BufferCount;
	DescripterHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	DescripterHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	VERIFY_SUCCEEDED(Device->CreateDescriptorHeap(&DescripterHeapDesc, IID_PPV_ARGS(&RenderTargetViewHeap)));

	D3D12_CPU_DESCRIPTOR_HANDLE RenderTargetViewHandle(RenderTargetViewHeap->GetCPUDescriptorHandleForHeapStart());
	const auto DescriptorSize = Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	for (UINT i = 0; i < BufferCount; ++i) {
		VERIFY_SUCCEEDED(SwapChain3->GetBuffer(i, IID_PPV_ARGS(&RenderTargets[i])));
		Device->CreateRenderTargetView(RenderTargets[i].Get(), nullptr, RenderTargetViewHandle);
		RenderTargetViewHandle.ptr += DescriptorSize;
	}
#pragma endregion
}

void DX::CreateDepthStencil()
{
	const D3D12_HEAP_PROPERTIES HeapProperties = {
		D3D12_HEAP_TYPE_DEFAULT,
		D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
		D3D12_MEMORY_POOL_UNKNOWN,
		1,
		1
	};
	const D3D12_RESOURCE_DESC ResourceDesc = {
		D3D12_RESOURCE_DIMENSION_TEXTURE2D,
		0,
		static_cast<UINT64>(GetClientRectWidth()), static_cast<UINT>(GetClientRectHeight()),
		1,
		1,
		DXGI_FORMAT_R32_TYPELESS,
		{ 1, 0 },
		D3D12_TEXTURE_LAYOUT_UNKNOWN,
		D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL
	};
	const D3D12_CLEAR_VALUE ClearValue = {
		DXGI_FORMAT_D32_FLOAT,
		{ 1.0f, 0 }
	};
	VERIFY_SUCCEEDED(Device->CreateCommittedResource(&HeapProperties, D3D12_HEAP_FLAG_NONE, &ResourceDesc, D3D12_RESOURCE_STATE_DEPTH_WRITE, &ClearValue, IID_PPV_ARGS(&DepthStencil)));

#pragma region DepthStencilView
	const D3D12_DEPTH_STENCIL_VIEW_DESC DepthStencilViewDesc = {
		DXGI_FORMAT_D32_FLOAT,
		D3D12_DSV_DIMENSION_TEXTURE2D,
		D3D12_DSV_FLAG_NONE,
		{ 0 }
	};
	const auto Size = Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
	// TODO
	//D3D12_CPU_DESCRIPTOR_HANDLE depthHandle(pDsvHeap->GetCPUDescriptorHandleForHeapStart(), 1 + frameResourceIndex, Size);
	//D3D12_CPU_DESCRIPTOR_HANDLE DepthStencilViewHandle;
	//Device->CreateDepthStencilView(DepthStencil.Get(), &DepthStencilViewDesc, DepthStencilViewHandle);
#pragma endregion
}

void DX::CreateRootSignature()
{
	using namespace Microsoft::WRL;

	D3D12_ROOT_SIGNATURE_DESC RootSignatureDesc;
	RootSignatureDesc.NumParameters = 0;
	RootSignatureDesc.pParameters = nullptr;
	RootSignatureDesc.NumStaticSamplers = 0;
	RootSignatureDesc.pStaticSamplers = nullptr;
	RootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	ComPtr<ID3DBlob> Blob;
	ComPtr<ID3DBlob> ErrorBlob;
	VERIFY_SUCCEEDED(D3D12SerializeRootSignature(&RootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &Blob, &ErrorBlob));
	VERIFY_SUCCEEDED(Device->CreateRootSignature(0, Blob->GetBufferPointer(), Blob->GetBufferSize(), IID_PPV_ARGS(&RootSignature)));
}

/**
@note VisualStudio に HLSL ファイルを追加すれば、コンパイルされて *.cso ファイルが作成される ( 出力先は x64/Debug/, x64/Release/ など)
*/
void DX::CreateShader()
{
	using namespace Microsoft::WRL;

	D3DReadFileToBlob(L"XXXVS.cso", &BlobVS);// todo
	ShaderBytecodesVSs.push_back({ BlobVS->GetBufferPointer(), BlobVS->GetBufferSize() });
	
	D3DReadFileToBlob(L"XXXPS.cso", &BlobPS);// todo
	ShaderBytecodesPSs.push_back({ BlobPS->GetBufferPointer(), BlobPS->GetBufferSize() });
}

void DX::CreateViewport()
{
	const auto Width = GetClientRectWidth();
	const auto Height = GetClientRectHeight();

	Viewports.push_back({ 0.0f, 0.0f, static_cast<FLOAT>(Width), static_cast<FLOAT>(Height), 0.0f, 1.0f });
	ScissorRects.push_back({ 0, 0, Width, Height });
}

void DX::CreatePipelineState()
{
	//!< 要 RootSignature
	D3D12_GRAPHICS_PIPELINE_STATE_DESC GraphicsPipelineStateDesc = {};
	GraphicsPipelineStateDesc.pRootSignature = RootSignature.Get();

	if (!InputLayoutDescs.empty()) {
		GraphicsPipelineStateDesc.InputLayout = InputLayoutDescs[0];
	}

	if (!ShaderBytecodesVSs.empty()) {
		GraphicsPipelineStateDesc.VS = ShaderBytecodesVSs[0];
	}
	if (!ShaderBytecodesPSs.empty()) {
		GraphicsPipelineStateDesc.PS = ShaderBytecodesPSs[0];
	}

	D3D12_RASTERIZER_DESC RasterizerDesc = {}; {
		RasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;
		RasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
		RasterizerDesc.FrontCounterClockwise = FALSE;
		RasterizerDesc.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
		RasterizerDesc.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
		RasterizerDesc.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
		RasterizerDesc.DepthClipEnable = TRUE;
		RasterizerDesc.MultisampleEnable = FALSE;
		RasterizerDesc.AntialiasedLineEnable = FALSE;
		RasterizerDesc.ForcedSampleCount = 0;
		RasterizerDesc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;
	} GraphicsPipelineStateDesc.RasterizerState = RasterizerDesc;

	D3D12_BLEND_DESC BlendDesc = {}; {
		BlendDesc.AlphaToCoverageEnable = FALSE;
		BlendDesc.IndependentBlendEnable = FALSE;
		const D3D12_RENDER_TARGET_BLEND_DESC RenderTargetBlendDesc = {
			FALSE, FALSE,
			D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
			D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
			D3D12_LOGIC_OP_NOOP,
			D3D12_COLOR_WRITE_ENABLE_ALL,
		};
		for (auto& i : BlendDesc.RenderTarget) {
			i = RenderTargetBlendDesc;
		}
	} GraphicsPipelineStateDesc.BlendState = BlendDesc;

	GraphicsPipelineStateDesc.DepthStencilState.DepthEnable = FALSE;
	GraphicsPipelineStateDesc.DepthStencilState.StencilEnable = FALSE;
	GraphicsPipelineStateDesc.SampleMask = UINT_MAX;
	GraphicsPipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	GraphicsPipelineStateDesc.NumRenderTargets = 1;
	GraphicsPipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	GraphicsPipelineStateDesc.SampleDesc.Count = 1;
	VERIFY_SUCCEEDED(Device->CreateGraphicsPipelineState(&GraphicsPipelineStateDesc, IID_PPV_ARGS(&PipelineState)));
}

void DX::CreateCommandList()
{
	VERIFY_SUCCEEDED(Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&CommandAllocator)));
	VERIFY_SUCCEEDED(Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, CommandAllocator.Get(), PipelineState.Get(), IID_PPV_ARGS(&CommandList)));
	VERIFY_SUCCEEDED(CommandList->Close());
}

void DX::CreateInputLayout()
{
	D3D12_INPUT_ELEMENT_DESC InputElementDescs[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};
	InputLayoutDescs.push_back({ InputElementDescs, _countof(InputElementDescs) });
}
void DX::CreateVertexBuffer()
{
	const std::vector<Vertex> Vertices = {
		{ Vertex({  0.0f,   0.25f, 0.0f }, { 0.0f, 0.0f, 0.0f, 1.0f }) },
		{ Vertex({  0.25f, -0.25f, 0.0f }, { 0.0f, 0.0f, 0.0f, 1.0f }) },
		{ Vertex({ -0.25f, -0.25f, 0.0f }, { 0.0f, 0.0f, 0.0f, 1.0f }) },
	};

	const auto Size = sizeof(Vertices);
	const auto Stride = sizeof(Vertices[0]);

	const D3D12_RESOURCE_DESC ResourceDesc = { D3D12_RESOURCE_DIMENSION_BUFFER, 0, Size, 1, 1, 1, DXGI_FORMAT_UNKNOWN, 1, 0, D3D12_TEXTURE_LAYOUT_ROW_MAJOR, D3D12_RESOURCE_FLAG_NONE };
	const D3D12_HEAP_PROPERTIES HeapProperties = { D3D12_HEAP_TYPE_UPLOAD, D3D12_CPU_PAGE_PROPERTY_UNKNOWN, D3D12_MEMORY_POOL_UNKNOWN, 1, 1 };
	VERIFY_SUCCEEDED(Device->CreateCommittedResource(&HeapProperties,
		D3D12_HEAP_FLAG_NONE, 
		&ResourceDesc, 
		D3D12_RESOURCE_STATE_GENERIC_READ, 
		nullptr,
		IID_PPV_ARGS(&VertexBuffer)));

	UINT8* Data;
	D3D12_RANGE Range = { 0, 0 };
	VERIFY_SUCCEEDED(VertexBuffer->Map(0, &Range, reinterpret_cast<void **>(&Data))); {
		memcpy(Data, Vertices.data(), Size);
	} VertexBuffer->Unmap(0, nullptr);

	VertexBufferView = { VertexBuffer->GetGPUVirtualAddress(), Size, Stride };
}

void DX::CreateIndexBuffer()
{
	const std::vector<UINT32> Indices = { 0, 1, 2 };

	const auto Size = sizeof(Indices);
	//!< DrawInstanced() が引数に取るので覚えておく必要がある
	IndexCount = static_cast<UINT32>(Indices.size());

	const D3D12_RESOURCE_DESC ResourceDesc = { D3D12_RESOURCE_DIMENSION_BUFFER, 0, Size, 1, 1, 1, DXGI_FORMAT_UNKNOWN, 1, 0, D3D12_TEXTURE_LAYOUT_ROW_MAJOR, D3D12_RESOURCE_FLAG_NONE };
	const D3D12_HEAP_PROPERTIES HeapProperties = { D3D12_HEAP_TYPE_UPLOAD, D3D12_CPU_PAGE_PROPERTY_UNKNOWN, D3D12_MEMORY_POOL_UNKNOWN, 1, 1 };
	VERIFY_SUCCEEDED(Device->CreateCommittedResource(&HeapProperties,
		D3D12_HEAP_FLAG_NONE,
		&ResourceDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&IndexBuffer)));

	UINT8* Data;
	D3D12_RANGE Range = { 0, 0 };
	VERIFY_SUCCEEDED(IndexBuffer->Map(0, &Range, reinterpret_cast<void **>(&Data))); {
		memcpy(Data, Indices.data(), Size);
	} IndexBuffer->Unmap(0, nullptr);

	IndexBufferView = { IndexBuffer->GetGPUVirtualAddress(), Size, DXGI_FORMAT_R32_UINT };
}

void DX::CreateConstantBuffer()
{
	const auto Size = 1024 * 64;

	const D3D12_HEAP_PROPERTIES HeapProperties = { D3D12_HEAP_TYPE_UPLOAD, D3D12_CPU_PAGE_PROPERTY_UNKNOWN, D3D12_MEMORY_POOL_UNKNOWN, 1, 1 };
	const D3D12_RESOURCE_DESC ResourceDesc = { D3D12_RESOURCE_DIMENSION_BUFFER, 0, Size, 1, 1, 1, DXGI_FORMAT_UNKNOWN, 1, 0, D3D12_TEXTURE_LAYOUT_ROW_MAJOR, D3D12_RESOURCE_FLAG_NONE };
	VERIFY_SUCCEEDED(Device->CreateCommittedResource(&HeapProperties, 
		D3D12_HEAP_FLAG_NONE, 
		&ResourceDesc, 
		D3D12_RESOURCE_STATE_GENERIC_READ, 
		nullptr, 
		IID_PPV_ARGS(&ConstantBuffer)));

	D3D12_CONSTANT_BUFFER_VIEW_DESC ConstantBufferViewDesc = {};
	ConstantBufferViewDesc.BufferLocation = ConstantBuffer->GetGPUVirtualAddress();
	ConstantBufferViewDesc.SizeInBytes = (sizeof(ConstantBuffer) + 255) & ~255;	//!< コンスタントバッファは 256 byte アラインでないとならない
	{
		D3D12_DESCRIPTOR_HEAP_DESC DescriptorHeapDesc = { D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 1, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE, 0 };
		VERIFY_SUCCEEDED(Device->CreateDescriptorHeap(&DescriptorHeapDesc, IID_PPV_ARGS(&ConstantBufferViewHeap)));
	}
	Device->CreateConstantBufferView(&ConstantBufferViewDesc, ConstantBufferViewHeap->GetCPUDescriptorHandleForHeapStart());
}

void DX::CreateFence()
{
	//!< 初期値を 0 として作成
	VERIFY_SUCCEEDED(Device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&Fence)));
	//!< 次回に備えて 1 にしておく
	FenceValue = 1;

	FenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	assert(nullptr != FenceEvent);

	WaitForFence();
}

void DX::PopulateCommandList()
{
	VERIFY_SUCCEEDED(CommandAllocator->Reset());
	VERIFY_SUCCEEDED(CommandList->Reset(CommandAllocator.Get(), PipelineState.Get()));

	CommandList->SetGraphicsRootSignature(RootSignature.Get());

	CommandList->RSSetViewports(static_cast<UINT>(Viewports.size()), Viewports.data());
	CommandList->RSSetScissorRects(static_cast<UINT>(ScissorRects.size()), ScissorRects.data());

	{
		using namespace DirectX;
		const std::vector<XMMATRIX> WVP = { XMMatrixIdentity(), XMMatrixIdentity(), XMMatrixIdentity() };

		UINT8* Data;
		D3D12_RANGE Range = { 0, 0 };
		VERIFY_SUCCEEDED(ConstantBuffer->Map(0, &Range, reinterpret_cast<void**>(&Data))); {
			memcpy(Data, &WVP, sizeof(WVP));
		} ConstantBuffer->Unmap(0, nullptr); //!< サンプルには アプリが終了するまで Unmap しない、リソースはマップされたままでOKと書いてあるが...よく分からない
	}

	CommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	CommandList->IASetVertexBuffers(0, 1, &VertexBufferView);
	CommandList->IASetIndexBuffer(&IndexBufferView);
	CommandList->DrawInstanced(IndexCount, 1, 0, 0);

	VERIFY_SUCCEEDED(CommandList->Close());
}

void DX::ExecuteCommandList()
{
	ID3D12CommandList* CommandLists[] = { CommandList.Get() };
	CommandQueue->ExecuteCommandLists(_countof(CommandLists), CommandLists);
}

void DX::Present()
{
	VERIFY_SUCCEEDED(SwapChain3->Present(1, 0));
}

void DX::WaitForFence()
{
	const auto Value = FenceValue;
	//!< コマンドキューの実行が終わった時に Fence を Value にする (初回なら Fence の値 0 が 1 になる)
	VERIFY_SUCCEEDED(CommandQueue->Signal(Fence.Get(), Value));
	++FenceValue;

	//!< この短い間に Fence の値が Value になっていないことを確認してから「待ち」に入る
	if (Fence->GetCompletedValue() < Value)	{
		//!< Fence の値が Value になったら FenceEvent 発行 (初回なら Fence の値 0 が 1 になったら)
		VERIFY_SUCCEEDED(Fence->SetEventOnCompletion(Value, FenceEvent));
		//!< ↑ FenceEvent が発行されるまで待つ
		WaitForSingleObject(FenceEvent, INFINITE);
	}

	CurrentBackBufferIndex = SwapChain3->GetCurrentBackBufferIndex();
}

