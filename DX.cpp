#include "stdafx.h"

#include "DX.h"

//!< d3d12.lib が無いと言われる場合は、プロジェクトを右クリック - Retarget SDK Version で Windows10 にする
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "dxgi.lib")

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
	CreateCommandList();

	CreateSwapChain(hWnd);

	CreateDepthStencil();

	CreateShader();

	CreateRootSignature();

	CreateInputLayout();
	CreateViewport();
	CreatePipelineState();

	CreateVertexBuffer();
	CreateIndexBuffer();
	CreateConstantBuffer();

	// -----------------------


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
	if (false) {
		//!< WarpDevice
		ComPtr<IDXGIAdapter> Adapter;
		VERIFY_SUCCEEDED(Factory4->EnumWarpAdapter(IID_PPV_ARGS(&Adapter)));
		VERIFY_SUCCEEDED(D3D12CreateDevice(Adapter.Get(), D3D_FEATURE_LEVEL_12_1, IID_PPV_ARGS(&Device)));
	}
	else {
		ComPtr<IDXGIAdapter1> Adapter;
		//!< discrete GPU が(DXGI_ADAPTER_FLAG_SOFTWARE以外で)最後に列挙されるのでこうしている
		UINT AdapterIndex = 0xffffffff;
		for (UINT i = 0; DXGI_ERROR_NOT_FOUND != Factory4->EnumAdapters1(i, &Adapter); ++i) {
			DXGI_ADAPTER_DESC1 AdapterDesc1;
			Adapter->GetDesc1(&AdapterDesc1);
			if (!(DXGI_ADAPTER_FLAG_SOFTWARE & AdapterDesc1.Flags)) {
				AdapterIndex = i;
			}
		}
		assert(0xffffffff != AdapterIndex && "");
#ifdef _DEBUG
		std::cout << Yellow << "\t" << "Adapters" << White << std::endl;
		for (UINT i = 0; DXGI_ERROR_NOT_FOUND != Factory4->EnumAdapters1(i, &Adapter); ++i) {
			DXGI_ADAPTER_DESC1 AdapterDesc1;
			Adapter->GetDesc1(&AdapterDesc1);
			if (AdapterIndex == i) { std::cout << Red; }
			std::wcout << "\t" << "\t" << AdapterDesc1.Description << std::endl;
			std::cout << White;
		}
#endif

		Factory4->EnumAdapters1(AdapterIndex, &Adapter);
		if (SUCCEEDED(D3D12CreateDevice(Adapter.Get(), D3D_FEATURE_LEVEL_12_1, IID_PPV_ARGS(&Device)))) {
#ifdef _DEBUG
			std::cout << "CreateDevice" << COUT_OK << std::endl << std::endl;
#endif
		}
	}
}
void DX::CreateCommandQueue()
{
	const D3D12_COMMAND_QUEUE_DESC CommandQueueDesc = {
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		0,
		D3D12_COMMAND_QUEUE_FLAG_NONE,
		0
	};
	VERIFY_SUCCEEDED(Device->CreateCommandQueue(&CommandQueueDesc, IID_PPV_ARGS(&CommandQueue)));

#ifdef _DEBUG
	std::cout << "CreateCommandQueue" << COUT_OK << std::endl << std::endl;
#endif
}

void DX::CreateCommandList()
{
	VERIFY_SUCCEEDED(Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&CommandAllocator)));
	VERIFY_SUCCEEDED(Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, CommandAllocator.Get(), PipelineState.Get(), IID_PPV_ARGS(&CommandList)));
	VERIFY_SUCCEEDED(CommandList->Close());

#ifdef _DEBUG
	std::cout << "CreateCommandList" << COUT_OK << std::endl << std::endl;
#endif
}

void DX::CreateSwapChain(HWND hWnd, const UINT BufferCount)
{
	using namespace Microsoft::WRL;

	ComPtr<IDXGIFactory4> Factory4;
	VERIFY_SUCCEEDED(CreateDXGIFactory1(IID_PPV_ARGS(&Factory4)));

#pragma region SwapChain
	const DXGI_SWAP_CHAIN_DESC1 SwapChainDesc1 = {
		static_cast<UINT>(GetClientRectWidth()), static_cast<UINT>(GetClientRectHeight()),
		DXGI_FORMAT_R8G8B8A8_UNORM,
		FALSE,
		{ 1, 0 },
		DXGI_USAGE_RENDER_TARGET_OUTPUT,
		BufferCount,
		DXGI_SCALING_STRETCH,
		DXGI_SWAP_EFFECT_FLIP_DISCARD,
		DXGI_ALPHA_MODE_UNSPECIFIED,
		0
	};

	ComPtr<IDXGISwapChain1> SwapChain1;
	VERIFY_SUCCEEDED(Factory4->CreateSwapChainForHwnd(CommandQueue.Get(), hWnd, &SwapChainDesc1, nullptr, nullptr, &SwapChain1));
	VERIFY_SUCCEEDED(Factory4->MakeWindowAssociation(hWnd, DXGI_MWA_NO_ALT_ENTER));
	VERIFY_SUCCEEDED(SwapChain1.As(&SwapChain3));
	CurrentBackBufferIndex = SwapChain3->GetCurrentBackBufferIndex();
#ifdef _DEBUG
	std::cout << "\t" << "SwapChain3" << std::endl;
	std::cout << "\t" << "CurrentBackBufferIndex = " << CurrentBackBufferIndex << std::endl;
#endif
#pragma endregion

#pragma region SwapChainView
	const D3D12_DESCRIPTOR_HEAP_DESC DescripterHeapDesc = {
		D3D12_DESCRIPTOR_HEAP_TYPE_RTV,
		BufferCount,
		D3D12_DESCRIPTOR_HEAP_FLAG_NONE,
		0
	};
	VERIFY_SUCCEEDED(Device->CreateDescriptorHeap(&DescripterHeapDesc, IID_PPV_ARGS(&RenderTargetViewHeap)));

	auto RenderTargetViewHandle(RenderTargetViewHeap->GetCPUDescriptorHandleForHeapStart());
	const auto DescriptorSize = Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	for (UINT i = 0; i < BufferCount; ++i) {
		VERIFY_SUCCEEDED(SwapChain3->GetBuffer(i, IID_PPV_ARGS(&RenderTargets[i])));
		Device->CreateRenderTargetView(RenderTargets[i].Get(), nullptr, RenderTargetViewHandle);
		RenderTargetViewHandle.ptr += DescriptorSize;
#ifdef _DEBUG
		std::cout << "\t" << "\t" << "RenderTarget" << std::endl;
#endif
	}
#ifdef _DEBUG
	std::cout << "\t" << "RenderTargetView" << std::endl;
#endif
#pragma endregion

#ifdef _DEBUG
	std::cout << "CreateSwapChain" << COUT_OK << std::endl << std::endl;
#endif
}

void DX::CreateDepthStencil()
{
#pragma region DepthStencil
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

#ifdef _DEBUG
	std::cout << "\t" << "DepthStencil" << std::endl;
#endif
#pragma endregion

#pragma region DepthStencilView
	const D3D12_DEPTH_STENCIL_VIEW_DESC DepthStencilViewDesc = {
		DXGI_FORMAT_D32_FLOAT,
		D3D12_DSV_DIMENSION_TEXTURE2D,
		D3D12_DSV_FLAG_NONE,
		{ 0 }
	};

	const D3D12_DESCRIPTOR_HEAP_DESC DescriptorHeapDesc = {
		D3D12_DESCRIPTOR_HEAP_TYPE_DSV,
		1,
		D3D12_DESCRIPTOR_HEAP_FLAG_NONE,
		0
	};
	VERIFY_SUCCEEDED(Device->CreateDescriptorHeap(&DescriptorHeapDesc, IID_PPV_ARGS(&DepthStencilViewHeap)));

	auto DepthStencilViewHandle(DepthStencilViewHeap->GetCPUDescriptorHandleForHeapStart());
	const auto DescriptorSize = Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
	DepthStencilViewHandle.ptr += 0 * DescriptorSize;
	Device->CreateDepthStencilView(DepthStencil.Get(), &DepthStencilViewDesc, DepthStencilViewHandle);

#ifdef _DEBUG
	std::cout << "\t" << "DepthStencilView" << std::endl;
#endif
#pragma endregion

#ifdef _DEBUG
	std::cout << "CreateDepthStencil" << COUT_OK << std::endl << std::endl;
#endif
}

/**
@note VisualStudio に HLSL ファイルを追加すれば、コンパイルされて *.cso ファイルが作成される ( 出力先は x64/Debug/, x64/Release/ など)
*/
void DX::CreateShader()
{
	using namespace Microsoft::WRL;
	D3DReadFileToBlob(SHADER_PATH L"VS.cso", &BlobVS);
	ShaderBytecodesVSs.push_back({ BlobVS->GetBufferPointer(), BlobVS->GetBufferSize() });

	D3DReadFileToBlob(SHADER_PATH L"PS.cso", &BlobPS);
	ShaderBytecodesPSs.push_back({ BlobPS->GetBufferPointer(), BlobPS->GetBufferSize() });

#ifdef _DEBUG
	std::cout << "CreateShader" << COUT_OK << std::endl << std::endl;
#endif
}

void DX::CreateRootSignature()
{
	using namespace Microsoft::WRL;

	const D3D12_ROOT_SIGNATURE_DESC RootSignatureDesc = {
		0, nullptr,
		0, nullptr,
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT
	};

	ComPtr<ID3DBlob> Blob;
	ComPtr<ID3DBlob> ErrorBlob;
	VERIFY_SUCCEEDED(D3D12SerializeRootSignature(&RootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &Blob, &ErrorBlob));
	VERIFY_SUCCEEDED(Device->CreateRootSignature(0, Blob->GetBufferPointer(), Blob->GetBufferSize(), IID_PPV_ARGS(&RootSignature)));

#ifdef _DEBUG
	std::cout << "CreateRootSignature" << COUT_OK << std::endl << std::endl;
#endif
}

void DX::CreateInputLayout()
{
	InputElementDescs = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};
	InputLayoutDesc = { 
		InputElementDescs.data(), static_cast<UINT>(InputElementDescs.size()) 
	};

#ifdef _DEBUG
	std::cout << "CreateInputLayout" << COUT_OK << std::endl << std::endl;
#endif
}
void DX::CreateViewport()
{
	const auto Width = GetClientRectWidth();
	const auto Height = GetClientRectHeight();

	Viewports.push_back({ 0.0f, 0.0f, static_cast<FLOAT>(Width), static_cast<FLOAT>(Height), 0.0f, 1.0f });
	ScissorRects.push_back({ 0, 0, Width, Height });

#ifdef _DEBUG
	std::cout << "CreateViewport" << COUT_OK << std::endl << std::endl;
#endif
}
void DX::CreatePipelineState()
{
	const D3D12_SHADER_BYTECODE DefaultShaderBytecode = { nullptr, 0 };
	const D3D12_RENDER_TARGET_BLEND_DESC DefaultRenderTargetBlendDesc = {
		FALSE, FALSE,
		D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
		D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
		D3D12_LOGIC_OP_NOOP,
		D3D12_COLOR_WRITE_ENABLE_ALL,
	};
	const D3D12_BLEND_DESC BlendDesc = {
		FALSE,
		FALSE,
		{ DefaultRenderTargetBlendDesc/*, ... x8*/ }
	};
	const D3D12_RASTERIZER_DESC RasterizerDesc = {
		D3D12_FILL_MODE_SOLID,
		D3D12_CULL_MODE_BACK, FALSE,
		D3D12_DEFAULT_DEPTH_BIAS, D3D12_DEFAULT_DEPTH_BIAS_CLAMP, D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS,
		TRUE,
		FALSE,
		FALSE,
		0,
		D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF
	};
	const D3D12_DEPTH_STENCILOP_DESC DepthStencilOpDesc = {
		D3D12_STENCIL_OP_KEEP,
		D3D12_STENCIL_OP_KEEP,
		D3D12_STENCIL_OP_KEEP,
		D3D12_COMPARISON_FUNC_NEVER
	};
	const D3D12_DEPTH_STENCIL_DESC DepthStencilDesc = {
		FALSE,
		D3D12_DEPTH_WRITE_MASK_ZERO,
		D3D12_COMPARISON_FUNC_NEVER,
		FALSE,
		0,
		0,
		DepthStencilOpDesc,
		DepthStencilOpDesc
	};

	assert(nullptr != RootSignature);
	assert(!ShaderBytecodesVSs.empty());
	assert(!ShaderBytecodesPSs.empty());
	const D3D12_GRAPHICS_PIPELINE_STATE_DESC GraphicsPipelineStateDesc = {
		RootSignature.Get(),
		ShaderBytecodesVSs[0], ShaderBytecodesPSs[0], DefaultShaderBytecode, DefaultShaderBytecode, DefaultShaderBytecode,
		{
			nullptr, 0,
			nullptr, 0,
			0
		},
		BlendDesc,
		UINT_MAX,
		RasterizerDesc,
		DepthStencilDesc,
		InputLayoutDesc,
		D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED,
		D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE,
		1, { DXGI_FORMAT_R8G8B8A8_UNORM/*, ... x8*/ },
		DXGI_FORMAT_D32_FLOAT,
		{ 1, 0 },
		0,
		{ nullptr, 0 },
		D3D12_PIPELINE_STATE_FLAG_NONE
	};
	VERIFY_SUCCEEDED(Device->CreateGraphicsPipelineState(&GraphicsPipelineStateDesc, IID_PPV_ARGS(&PipelineState)));

#ifdef _DEBUG
	std::cout << "CreateGraphicsPipelineState" << COUT_OK << std::endl << std::endl;
#endif
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

	const D3D12_RESOURCE_DESC ResourceDesc = { 
		D3D12_RESOURCE_DIMENSION_BUFFER, 
		0, 
		Size, 1, 
		1, 
		1, 
		DXGI_FORMAT_UNKNOWN, 
		{ 1, 0 },
		D3D12_TEXTURE_LAYOUT_ROW_MAJOR,
		D3D12_RESOURCE_FLAG_NONE 
	};
	const D3D12_HEAP_PROPERTIES HeapProperties = { 
		D3D12_HEAP_TYPE_UPLOAD, 
		D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
		D3D12_MEMORY_POOL_UNKNOWN, 
		1,
		1
	};
	VERIFY_SUCCEEDED(Device->CreateCommittedResource(&HeapProperties,
		D3D12_HEAP_FLAG_NONE, 
		&ResourceDesc, 
		D3D12_RESOURCE_STATE_GENERIC_READ, 
		nullptr,
		IID_PPV_ARGS(&VertexBuffer)));

	UINT8* Data;
	const D3D12_RANGE Range = { 0, 0 };
	VERIFY_SUCCEEDED(VertexBuffer->Map(0, &Range, reinterpret_cast<void **>(&Data))); {
		memcpy(Data, Vertices.data(), Size);
	} VertexBuffer->Unmap(0, nullptr);

	VertexBufferView = {
		VertexBuffer->GetGPUVirtualAddress(), 
		Size,
		Stride
	};

#ifdef _DEBUG
	std::cout << "CreateVertexBuffer" << COUT_OK << std::endl << std::endl;
#endif
}
void DX::CreateIndexBuffer()
{
	const std::vector<UINT32> Indices = { 0, 1, 2 };

	const auto Size = sizeof(Indices);
	//!< DrawInstanced() が引数に取るので覚えておく必要がある
	IndexCount = static_cast<UINT32>(Indices.size());

	const D3D12_RESOURCE_DESC ResourceDesc = { 
		D3D12_RESOURCE_DIMENSION_BUFFER, 
		0,
		Size, 1,
		1,
		1, 
		DXGI_FORMAT_UNKNOWN, 
		{ 1, 0 },
		D3D12_TEXTURE_LAYOUT_ROW_MAJOR, 
		D3D12_RESOURCE_FLAG_NONE 
	};
	const D3D12_HEAP_PROPERTIES HeapProperties = { 
		D3D12_HEAP_TYPE_UPLOAD, 
		D3D12_CPU_PAGE_PROPERTY_UNKNOWN, 
		D3D12_MEMORY_POOL_UNKNOWN, 
		1, 
		1
	};
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

	IndexBufferView = { 
		IndexBuffer->GetGPUVirtualAddress(), 
		Size, 
		DXGI_FORMAT_R32_UINT 
	};

#ifdef _DEBUG
	std::cout << "CreateIndexBuffer" << COUT_OK << std::endl << std::endl;
#endif
}

void DX::CreateConstantBuffer()
{
	const auto Size = 1024 * 64;

	const D3D12_RESOURCE_DESC ResourceDesc = {
		D3D12_RESOURCE_DIMENSION_BUFFER,
		0,
		Size, 1,
		1,
		1,
		DXGI_FORMAT_UNKNOWN,
		{ 1, 0 },
		D3D12_TEXTURE_LAYOUT_ROW_MAJOR,
		D3D12_RESOURCE_FLAG_NONE
	};
	const D3D12_HEAP_PROPERTIES HeapProperties = {
		D3D12_HEAP_TYPE_UPLOAD,
		D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
		D3D12_MEMORY_POOL_UNKNOWN,
		1,
		1
	};
	VERIFY_SUCCEEDED(Device->CreateCommittedResource(&HeapProperties,
		D3D12_HEAP_FLAG_NONE,
		&ResourceDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&ConstantBuffer)));

	const D3D12_DESCRIPTOR_HEAP_DESC DescriptorHeapDesc = {
		D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
		1,
		D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE,
		0
	};
	VERIFY_SUCCEEDED(Device->CreateDescriptorHeap(&DescriptorHeapDesc, IID_PPV_ARGS(&ConstantBufferViewHeap)));
	const D3D12_CONSTANT_BUFFER_VIEW_DESC ConstantBufferViewDesc = {
		ConstantBuffer->GetGPUVirtualAddress(),
		(sizeof(ConstantBuffer) + 255) & ~255 //!< コンスタントバッファは 256 byte アラインでないとならない
	};
	Device->CreateConstantBufferView(&ConstantBufferViewDesc, ConstantBufferViewHeap->GetCPUDescriptorHandleForHeapStart());

#ifdef _DEBUG
	std::cout << "CreateConstantBuffer" << COUT_OK << std::endl << std::endl;
#endif
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

	auto RenderTargetViewHandle(RenderTargetViewHeap->GetCPUDescriptorHandleForHeapStart());
	{
		const auto DescriptorSize = Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		RenderTargetViewHandle.ptr += CurrentBackBufferIndex * DescriptorSize;
	}

	auto DepthStencilViewHandle(DepthStencilViewHeap->GetCPUDescriptorHandleForHeapStart());
	{
		const auto DescriptorSize = Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
		DepthStencilViewHandle.ptr += 0 * DescriptorSize;
	}
	CommandList->OMSetRenderTargets(1, &RenderTargetViewHandle, FALSE, nullptr);

	const float ClearColor[] = { 0.5f, 0.5f, 1.0f, 1.0f };
	CommandList->ClearRenderTargetView(RenderTargetViewHandle, ClearColor, 0, nullptr);
	CommandList->ClearDepthStencilView(DepthStencilViewHandle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

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

