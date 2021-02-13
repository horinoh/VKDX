#pragma once

#include "resource.h"

#pragma region Code
#include "../DXExt.h"

class DeferredDX : public DXExt
{
private:
	using Super = DXExt;
public:
	DeferredDX() : Super() {}
	virtual ~DeferredDX() {}

protected:
	virtual void OnTimer(HWND hWnd, HINSTANCE hInstance) override {
		Super::OnTimer(hWnd, hInstance);

		DirectX::XMStoreFloat4x4(&Tr.World, DirectX::XMMatrixRotationX(DirectX::XMConvertToRadians(Degree)));
		Degree += 1.0f;

#pragma region FRAME_OBJECT
		CopyToUploadResource(COM_PTR_GET(ConstantBuffers[GetCurrentBackBufferIndex()].Resource), RoundUp256(sizeof(Tr)), &Tr);
#pragma endregion
	}

#ifdef USE_GBUFFER_VISUALIZE
	virtual void CreateViewport(const FLOAT Width, const FLOAT Height, const FLOAT MinDepth = 0.0f, const FLOAT MaxDepth = 1.0f) override {
		D3D12_FEATURE_DATA_D3D12_OPTIONS3 FDO3;
		VERIFY_SUCCEEDED(Device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS3, reinterpret_cast<void*>(&FDO3), sizeof(FDO3)));
		assert(D3D12_VIEW_INSTANCING_TIER_1 < FDO3.ViewInstancingTier && "");

		const auto W = Width * 0.5f, H = Height * 0.5f;
		Viewports = {
			//!< 全画面用
			{ 0.0f, 0.0f, Width, Height, MinDepth, MaxDepth },
			//!< 分割画面用
			{ 0.0f, 0.0f, W, H, MinDepth, MaxDepth },
			{ W, 0.0f, W, H, MinDepth, MaxDepth },
			{ 0.0f, H, W, H, MinDepth, MaxDepth },
			{ W, H, W, H, MinDepth, MaxDepth },
		};
		//!< left, top, right, bottomで指定 (offset, extentで指定のVKとは異なるので注意)
		ScissorRects = {
			//!< 全画面用
			{ 0, 0, static_cast<LONG>(Width), static_cast<LONG>(Height) },
			//!< 分割画面用
			{ 0, 0, static_cast<LONG>(W), static_cast<LONG>(H) },
			{ static_cast<LONG>(W), 0, static_cast<LONG>(Width), static_cast<LONG>(H) },
			{ 0, static_cast<LONG>(H), static_cast<LONG>(W), static_cast<LONG>(Height) },
			{ static_cast<LONG>(W), static_cast<LONG>(H), static_cast<LONG>(Width), static_cast<LONG>(Height) },
		};
		LOG_OK();
	}
#endif
	virtual void CreateCommandList() override {
		Super::CreateCommandList();
		//!< パス1 : バンドルコマンドリスト
		DXGI_SWAP_CHAIN_DESC1 SCD;
		SwapChain->GetDesc1(&SCD);
		for (UINT i = 0; i < SCD.BufferCount; ++i) {
			VERIFY_SUCCEEDED(Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_BUNDLE, COM_PTR_GET(BundleCommandAllocators[0]), nullptr, COM_PTR_UUIDOF_PUTVOID(BundleGraphicsCommandLists.emplace_back())));
			VERIFY_SUCCEEDED(BundleGraphicsCommandLists.back()->Close());
		}
	}

	virtual void CreateGeometry() override {
		const auto CA = COM_PTR_GET(CommandAllocators[0]);
		const auto GCL = COM_PTR_GET(GraphicsCommandLists[0]);
		//!< Pass0 : インダイレクトバッファ(メッシュ描画用)
		{
			constexpr D3D12_DRAW_INDEXED_ARGUMENTS DIA = { .IndexCountPerInstance = 1, .InstanceCount = 1, .StartIndexLocation = 0, .BaseVertexLocation = 0, .StartInstanceLocation = 0 };
			IndirectBuffers.emplace_back().Create(COM_PTR_GET(Device), CA, GCL, COM_PTR_GET(CommandQueue), COM_PTR_GET(Fence), DIA);
		}
		//!< Pass1 : インダイレクトバッファ(フルスクリーン描画用)
		{
			constexpr D3D12_DRAW_ARGUMENTS DA = { .VertexCountPerInstance = 4, .InstanceCount = 1, .StartVertexLocation = 0, .StartInstanceLocation = 0 };
			IndirectBuffers.emplace_back().Create(COM_PTR_GET(Device), CA, GCL, COM_PTR_GET(CommandQueue), COM_PTR_GET(Fence), DA);
		}
	}
	virtual void CreateStaticSampler() override {
		//!< Pass1 : スタティックサンプラ
		StaticSamplerDescs.emplace_back(D3D12_STATIC_SAMPLER_DESC({
			.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR,
			.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP, .AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP, .AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP,
			.MipLODBias = 0.0f,
			.MaxAnisotropy = 0,
			.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER,
			.BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE,
			.MinLOD = 0.0f, .MaxLOD = 1.0f,
			.ShaderRegister = 0, .RegisterSpace = 0, .ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL
			}));
	}
	virtual void CreateRootSignature() override {
		//!< Pass0 : ルートシグネチャ
		{
			COM_PTR<ID3DBlob> Blob;
#ifdef USE_HLSL_ROOTSIGNATRUE
			GetRootSignaturePartFromShader(Blob, data(GetBasePath() + TEXT(".rs.cso")));
#else
			const std::array<D3D12_DESCRIPTOR_RANGE, 1> DRs = { {
				{ D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND },
			} };
			SerializeRootSignature(Blob, {
					{ D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE, { static_cast<UINT>(size(DRs)), data(DRs) }, D3D12_SHADER_VISIBILITY_GEOMETRY }
				}, {}, D3D12_ROOT_SIGNATURE_FLAG_NONE | SHADER_ROOT_ACCESS_GS);
#endif
			VERIFY_SUCCEEDED(Device->CreateRootSignature(0, Blob->GetBufferPointer(), Blob->GetBufferSize(), COM_PTR_UUIDOF_PUTVOID(RootSignatures.emplace_back())));
		}
		//!< Pass1 : ルートシグネチャ
		{
			COM_PTR<ID3DBlob> Blob;
#ifdef USE_HLSL_ROOTSIGNATRUE 
			GetRootSignaturePartFromShader(Blob, data(GetBasePath() + TEXT("_1.rs.cso")));
#else
			const std::array<D3D12_DESCRIPTOR_RANGE, 2> DRs = { {
#pragma region MRT
				//!< レンダーターゲット : カラー(RenderTarget : Color), 法線(RenderTarget : Normal), 深度(RenderTarget : Depth), 未定
				{ D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 4, 0, 0, D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND },
#pragma endregion
				{ D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND },
			} };
			assert(!empty(StaticSamplerDescs) && "");
			DX::SerializeRootSignature(Blob, {
					{ D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE, { static_cast<uint32_t>(size(DRs)), data(DRs) }, D3D12_SHADER_VISIBILITY_PIXEL },
				}, {
					StaticSamplerDescs[0],
				}, D3D12_ROOT_SIGNATURE_FLAG_NONE | SHADER_ROOT_ACCESS_PS);
#endif
			VERIFY_SUCCEEDED(Device->CreateRootSignature(0, Blob->GetBufferPointer(), Blob->GetBufferSize(), COM_PTR_UUIDOF_PUTVOID(RootSignatures.emplace_back())));
		}
		LOG_OK();
	}
	virtual void CreateTexture()
	{
		const D3D12_HEAP_PROPERTIES HP = {
			D3D12_HEAP_TYPE_DEFAULT,
			D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
			D3D12_MEMORY_POOL_UNKNOWN,
			0,// CreationNodeMask ... マルチGPUの場合に使用(1つしか使わない場合は0で良い)
			0 // VisibleNodeMask ... マルチGPUの場合に使用(1つしか使わない場合は0で良い)
		};
		const DXGI_SAMPLE_DESC SD = { 1, 0 };
		//!< レンダーターゲット : カラー(RenderTarget : Color)
		{
			const D3D12_RESOURCE_DESC RD = {
				D3D12_RESOURCE_DIMENSION_TEXTURE2D,
				0,
				static_cast<UINT64>(GetClientRectWidth()), static_cast<UINT>(GetClientRectHeight()),
				1,
				1,
				DXGI_FORMAT_R8G8B8A8_UNORM,
				SD,
				D3D12_TEXTURE_LAYOUT_UNKNOWN,
				D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET
			};
			const D3D12_CLEAR_VALUE CV = { RD.Format, { DirectX::Colors::SkyBlue.f[0],DirectX::Colors::SkyBlue.f[1],DirectX::Colors::SkyBlue.f[2],DirectX::Colors::SkyBlue.f[3] } };
			VERIFY_SUCCEEDED(Device->CreateCommittedResource(&HP, D3D12_HEAP_FLAG_NONE, &RD, D3D12_RESOURCE_STATE_RENDER_TARGET, &CV, COM_PTR_UUIDOF_PUTVOID(ImageResources.emplace_back())));

			RenderTargetViewDescs.emplace_back(D3D12_RENDER_TARGET_VIEW_DESC({ 
				.Format = ImageResources.back()->GetDesc().Format, 
				.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D,
				.Texture2D = D3D12_TEX2D_RTV({.MipSlice = 0, .PlaneSlice = 0}) 
			}));
			ShaderResourceViewDescs.emplace_back(D3D12_SHADER_RESOURCE_VIEW_DESC({
				.Format = ImageResources.back()->GetDesc().Format,
				.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D, 
				.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING,
				.Texture2D = D3D12_TEX2D_SRV({.MostDetailedMip = 0, .MipLevels = ImageResources.back()->GetDesc().MipLevels, .PlaneSlice = 0, .ResourceMinLODClamp = 0.0f})
			}));
		}
#pragma region MRT
		//!< レンダーターゲット : 法線(RenderTarget : Normal)
		{
			const D3D12_RESOURCE_DESC RD = {
				D3D12_RESOURCE_DIMENSION_TEXTURE2D,
				0,
				static_cast<UINT64>(GetClientRectWidth()), static_cast<UINT>(GetClientRectHeight()),
				1,
				1,
				DXGI_FORMAT_R10G10B10A2_UNORM,
				SD,
				D3D12_TEXTURE_LAYOUT_UNKNOWN,
				D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET
			};
			const D3D12_CLEAR_VALUE CV = { RD.Format, { 0.5f, 0.5f, 1.0f, 1.0f } };
			VERIFY_SUCCEEDED(Device->CreateCommittedResource(&HP, D3D12_HEAP_FLAG_NONE, &RD, D3D12_RESOURCE_STATE_RENDER_TARGET, &CV, COM_PTR_UUIDOF_PUTVOID(ImageResources.emplace_back())));

			RenderTargetViewDescs.emplace_back(D3D12_RENDER_TARGET_VIEW_DESC({ 
				.Format = ImageResources.back()->GetDesc().Format,
				.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D,
				.Texture2D = D3D12_TEX2D_RTV({.MipSlice = 0, .PlaneSlice = 0 })
			}));
			ShaderResourceViewDescs.emplace_back(D3D12_SHADER_RESOURCE_VIEW_DESC({ 
				.Format = ImageResources.back()->GetDesc().Format,
				.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D, 
				.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING,
				.Texture2D = D3D12_TEX2D_SRV({.MostDetailedMip = 0, .MipLevels = ImageResources.back()->GetDesc().MipLevels, .PlaneSlice = 0, .ResourceMinLODClamp = 0.0f })
			}));
		}
		//!< レンダーターゲット : 深度(RenderTarget : Depth)
		{
			const D3D12_RESOURCE_DESC RD = {
				D3D12_RESOURCE_DIMENSION_TEXTURE2D,
				0,
				static_cast<UINT64>(GetClientRectWidth()), static_cast<UINT>(GetClientRectHeight()),
				1,
				1,
				DXGI_FORMAT_R32_FLOAT,
				SD,
				D3D12_TEXTURE_LAYOUT_UNKNOWN,
				D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET
			};
			const D3D12_CLEAR_VALUE CV = { RD.Format, { DirectX::Colors::Red.f[0], DirectX::Colors::Red.f[1], DirectX::Colors::Red.f[2], DirectX::Colors::Red.f[3] } };
			VERIFY_SUCCEEDED(Device->CreateCommittedResource(&HP, D3D12_HEAP_FLAG_NONE, &RD, D3D12_RESOURCE_STATE_RENDER_TARGET, &CV, COM_PTR_UUIDOF_PUTVOID(ImageResources.emplace_back())));

			RenderTargetViewDescs.emplace_back(D3D12_RENDER_TARGET_VIEW_DESC({
				.Format = ImageResources.back()->GetDesc().Format, 
				.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D,
				.Texture2D = D3D12_TEX2D_RTV({.MipSlice = 0, .PlaneSlice = 0 })
			}));
			ShaderResourceViewDescs.emplace_back(D3D12_SHADER_RESOURCE_VIEW_DESC({ 
				.Format = ImageResources.back()->GetDesc().Format,
				.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D,
				.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING,
				.Texture2D = D3D12_TEX2D_SRV({.MostDetailedMip = 0, .MipLevels = ImageResources.back()->GetDesc().MipLevels, .PlaneSlice = 0, .ResourceMinLODClamp = 0.0f })
			}));
		}
		//!< レンダーターゲット : 未定
		{
			const D3D12_RESOURCE_DESC RD = {
				D3D12_RESOURCE_DIMENSION_TEXTURE2D,
				0,
				static_cast<UINT64>(GetClientRectWidth()), static_cast<UINT>(GetClientRectHeight()),
				1,
				1,
				DXGI_FORMAT_R8G8B8A8_UNORM,
				SD,
				D3D12_TEXTURE_LAYOUT_UNKNOWN,
				D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET
			};
			const D3D12_CLEAR_VALUE CV = { RD.Format, { DirectX::Colors::SkyBlue.f[0],  DirectX::Colors::SkyBlue.f[1], DirectX::Colors::SkyBlue.f[2], DirectX::Colors::SkyBlue.f[3] } };
			VERIFY_SUCCEEDED(Device->CreateCommittedResource(&HP, D3D12_HEAP_FLAG_NONE, &RD, D3D12_RESOURCE_STATE_RENDER_TARGET, &CV, COM_PTR_UUIDOF_PUTVOID(ImageResources.emplace_back())));

			RenderTargetViewDescs.emplace_back(D3D12_RENDER_TARGET_VIEW_DESC({
				.Format = ImageResources.back()->GetDesc().Format, 
				.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D,
				.Texture2D = D3D12_TEX2D_RTV({.MipSlice = 0, .PlaneSlice = 0 })
			}));
			ShaderResourceViewDescs.emplace_back(D3D12_SHADER_RESOURCE_VIEW_DESC({
				.Format = ImageResources.back()->GetDesc().Format, 
				.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D,
				.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING,
				.Texture2D = D3D12_TEX2D_SRV({.MostDetailedMip = 0, .MipLevels = ImageResources.back()->GetDesc().MipLevels, .PlaneSlice = 0, .ResourceMinLODClamp = 0.0f })
			}));
		}
#pragma endregion
		{
			const D3D12_RESOURCE_DESC RD = {
				D3D12_RESOURCE_DIMENSION_TEXTURE2D,
				0,
				static_cast<UINT64>(GetClientRectWidth()), static_cast<UINT>(GetClientRectHeight()),
				1,
				1,
				DXGI_FORMAT_D24_UNORM_S8_UINT,
				SD,
				D3D12_TEXTURE_LAYOUT_UNKNOWN,
				D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL
			};
			D3D12_CLEAR_VALUE CV = { RD.Format, { 1.0f, 0 } };
			VERIFY_SUCCEEDED(Device->CreateCommittedResource(&HP, D3D12_HEAP_FLAG_NONE, &RD, D3D12_RESOURCE_STATE_DEPTH_WRITE, &CV, COM_PTR_UUIDOF_PUTVOID(ImageResources.emplace_back())));

			DepthStencilViewDescs.emplace_back(D3D12_DEPTH_STENCIL_VIEW_DESC({ 
				.Format = ImageResources.back()->GetDesc().Format, 
				.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D,  
				.Flags = D3D12_DSV_FLAG_NONE,
				.Texture2D = D3D12_TEX2D_DSV({.MipSlice = 0 })
			}));
		}
	}

	virtual void CreateDescriptorHeap() override {
#pragma region FRAME_OBJECT
		DXGI_SWAP_CHAIN_DESC1 SCD;
		SwapChain->GetDesc1(&SCD);
#pragma endregion
		//!< Pass0 : レンダーターゲット
		{
			{
#pragma region FRAME_OBJECT
				const D3D12_DESCRIPTOR_HEAP_DESC DHD = { D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, SCD.BufferCount, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE, 0 }; //!< CBV * N
#pragma endregion
				VERIFY_SUCCEEDED(Device->CreateDescriptorHeap(&DHD, COM_PTR_UUIDOF_PUTVOID(CbvSrvUavDescriptorHeaps.emplace_back())));
			}
#pragma region MRT
			{
				//!< レンダーターゲット : カラー(RenderTarget : Color), 法線(RenderTarget : Normal), 深度(RenderTarget : Depth), 未定
				const D3D12_DESCRIPTOR_HEAP_DESC DHD = { D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 4, D3D12_DESCRIPTOR_HEAP_FLAG_NONE, 0 }; //!< 4RTV
				VERIFY_SUCCEEDED(Device->CreateDescriptorHeap(&DHD, COM_PTR_UUIDOF_PUTVOID(RtvDescriptorHeaps.emplace_back())));
			}
#pragma endregion
			{
				const D3D12_DESCRIPTOR_HEAP_DESC DHD = { D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 1, D3D12_DESCRIPTOR_HEAP_FLAG_NONE, 0 }; //!< DSV
				VERIFY_SUCCEEDED(Device->CreateDescriptorHeap(&DHD, COM_PTR_UUIDOF_PUTVOID(DsvDescriptorHeaps.emplace_back())));
			}
		}
		//!< Pass1 : シェーダリソース
		{
#pragma region MRT
			{
				//!< レンダーターゲット : カラー(RenderTarget : Color), 法線(RenderTarget : Normal), 深度(RenderTarget : Depth), 未定
				CbvSrvUavDescriptorHeaps.push_back(COM_PTR<ID3D12DescriptorHeap>());
#pragma region FRAME_OBJECT
				const D3D12_DESCRIPTOR_HEAP_DESC DHD = { D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 4 + SCD.BufferCount, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE, 0 }; //!< 4SRV + CBV * N
#pragma endregion
				VERIFY_SUCCEEDED(Device->CreateDescriptorHeap(&DHD, COM_PTR_UUIDOF_PUTVOID(CbvSrvUavDescriptorHeaps.back())));
			}
#pragma endregion
		}
	}
	virtual void CreateDescriptorView() override {
#pragma region FRAME_OBJECT
		DXGI_SWAP_CHAIN_DESC1 SCD;
		SwapChain->GetDesc1(&SCD);
#pragma endregion

		//!< Pass0 : レンダーターゲットビュー
		{
			{
				const auto& DH = CbvSrvUavDescriptorHeaps[0];
				auto CDH = DH->GetCPUDescriptorHandleForHeapStart();

#pragma region FRAME_OBJECT
				for (UINT i = 0; i < SCD.BufferCount; ++i) {
					const D3D12_CONSTANT_BUFFER_VIEW_DESC CBVD = { COM_PTR_GET(ConstantBuffers[i].Resource)->GetGPUVirtualAddress(), static_cast<UINT>(ConstantBuffers[i].Resource->GetDesc().Width) };
					Device->CreateConstantBufferView(&CBVD, CDH); CDH.ptr += Device->GetDescriptorHandleIncrementSize(DH->GetDesc().Type); //!< CBV
				}
#pragma endregion
			}
			{
				const auto& DH = RtvDescriptorHeaps[0];
				auto CDH = DH->GetCPUDescriptorHandleForHeapStart();

#if 1
				//!< レンダーターゲット : カラー(RenderTarget : Color)
				Device->CreateRenderTargetView(COM_PTR_GET(ImageResources[0]), &RenderTargetViewDescs[0], CDH); CDH.ptr += Device->GetDescriptorHandleIncrementSize(DH->GetDesc().Type);
#pragma region MRT
				//!< レンダーターゲット : 法線(RenderTarget : Normal)
				Device->CreateRenderTargetView(COM_PTR_GET(ImageResources[1]), &RenderTargetViewDescs[1], CDH); CDH.ptr += Device->GetDescriptorHandleIncrementSize(DH->GetDesc().Type);
				//!< レンダーターゲット : 深度(RenderTarget : Depth)
				Device->CreateRenderTargetView(COM_PTR_GET(ImageResources[2]), &RenderTargetViewDescs[2], CDH); CDH.ptr += Device->GetDescriptorHandleIncrementSize(DH->GetDesc().Type);
				//!< レンダーターゲット : 未定
				Device->CreateRenderTargetView(COM_PTR_GET(ImageResources[3]), &RenderTargetViewDescs[3], CDH); CDH.ptr += Device->GetDescriptorHandleIncrementSize(DH->GetDesc().Type);
#pragma endregion
#else
				//!< リソースのフォーマットやディメンジョンを引き継ぎ、ミップマップやスライスの最初の要素を使用するような場合は XXX_VIEW_DESC に nullptr を指定できる
				Device->CreateRenderTargetView(COM_PTR_GET(ImageResources[0]), nullptr, CDH); CDH.ptr += Device->GetDescriptorHandleIncrementSize(DH->GetDesc().Type);
				Device->CreateRenderTargetView(COM_PTR_GET(ImageResources[1]), nullptr, CDH); CDH.ptr += Device->GetDescriptorHandleIncrementSize(DH->GetDesc().Type);
				Device->CreateRenderTargetView(COM_PTR_GET(ImageResources[2]), nullptr, CDH); CDH.ptr += Device->GetDescriptorHandleIncrementSize(DH->GetDesc().Type);
				Device->CreateRenderTargetView(COM_PTR_GET(ImageResources[3]), nullptr, CDH); CDH.ptr += Device->GetDescriptorHandleIncrementSize(DH->GetDesc().Type);
#endif
			}
			{
				const auto& DH = DsvDescriptorHeaps[0];
				auto CDH = DH->GetCPUDescriptorHandleForHeapStart();

#if 1
				Device->CreateDepthStencilView(COM_PTR_GET(ImageResources[4]), &DepthStencilViewDescs[0], CDH); CDH.ptr += Device->GetDescriptorHandleIncrementSize(DH->GetDesc().Type);
#else
				//!< リソースのフォーマットやディメンジョンを引き継ぎ、ミップマップやスライスの最初の要素を使用するような場合は XXX_VIEW_DESC に nullptr を指定できる
				Device->CreateDepthStencilView(COM_PTR_GET(ImageResources[4]), nullptr, CDH); CDH.ptr += Device->GetDescriptorHandleIncrementSize(DH->GetDesc().Type);
#endif
			}
		}
		//!< Pass1 : シェーダリソースビュー
		{
			const auto& DH = CbvSrvUavDescriptorHeaps[1];
			auto CDH = DH->GetCPUDescriptorHandleForHeapStart();

			{
#if 1
				//!< レンダーターゲット : カラー(RenderTarget : Color)
				Device->CreateShaderResourceView(COM_PTR_GET(ImageResources[0]), &ShaderResourceViewDescs[0], CDH); CDH.ptr += Device->GetDescriptorHandleIncrementSize(DH->GetDesc().Type);
#pragma region MRT
				//!< レンダーターゲット : 法線(RenderTarget : Normal)
				Device->CreateShaderResourceView(COM_PTR_GET(ImageResources[1]), &ShaderResourceViewDescs[1], CDH); CDH.ptr += Device->GetDescriptorHandleIncrementSize(DH->GetDesc().Type);
				//!< レンダーターゲット : 深度(RenderTarget : Depth)
				Device->CreateShaderResourceView(COM_PTR_GET(ImageResources[2]), &ShaderResourceViewDescs[2], CDH); CDH.ptr += Device->GetDescriptorHandleIncrementSize(DH->GetDesc().Type);
				//!< レンダーターゲット : 未定
				Device->CreateShaderResourceView(COM_PTR_GET(ImageResources[3]), &ShaderResourceViewDescs[3], CDH); CDH.ptr += Device->GetDescriptorHandleIncrementSize(DH->GetDesc().Type);
#pragma	endregion
#else
				//!< リソースのフォーマットやディメンジョンを引き継ぎ、ミップマップやスライスの最初の要素を使用するような場合は XXX_VIEW_DESC に nullptr を指定できる
				Device->CreateShaderResourceView(COM_PTR_GET(ImageResources[0]), nullptr, CDH); CDH.ptr += Device->GetDescriptorHandleIncrementSize(DH->GetDesc().Type);
				Device->CreateShaderResourceView(COM_PTR_GET(ImageResources[1]), nullptr, CDH); CDH.ptr += Device->GetDescriptorHandleIncrementSize(DH->GetDesc().Type);
				Device->CreateShaderResourceView(COM_PTR_GET(ImageResources[2]), nullptr, CDH); CDH.ptr += Device->GetDescriptorHandleIncrementSize(DH->GetDesc().Type);
				Device->CreateShaderResourceView(COM_PTR_GET(ImageResources[3]), nullptr, CDH); CDH.ptr += Device->GetDescriptorHandleIncrementSize(DH->GetDesc().Type);
#endif
			}
			{
#pragma region FRAME_OBJECT
				for (UINT i = 0; i < SCD.BufferCount; ++i) {
					const D3D12_CONSTANT_BUFFER_VIEW_DESC CBVD = { COM_PTR_GET(ConstantBuffers[i].Resource)->GetGPUVirtualAddress(), static_cast<UINT>(ConstantBuffers[i].Resource->GetDesc().Width) };
					Device->CreateConstantBufferView(&CBVD, CDH); CDH.ptr += Device->GetDescriptorHandleIncrementSize(DH->GetDesc().Type); //!< CBV
				}
#pragma endregion
			}
		}
	}

	virtual void CreateConstantBuffer() override {
		//const auto Fov = 0.16f * DirectX::XM_PI;
		const auto Fov = 0.16f * std::numbers::pi_v<float>;
		const auto Aspect = GetAspectRatioOfClientRect();
		const auto ZFar = 4.0f;
		const auto ZNear = 2.0f;
		const auto CamPos = DirectX::XMVectorSet(0.0f, 0.0f, 3.0f, 1.0f);
		const auto CamTag = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
		const auto CamUp = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
		const auto Projection = DirectX::XMMatrixPerspectiveFovRH(Fov, Aspect, ZNear, ZFar);
		const auto View = DirectX::XMMatrixLookAtRH(CamPos, CamTag, CamUp);
		const auto World = DirectX::XMMatrixIdentity();

		const auto ViewProjection = DirectX::XMMatrixMultiply(View, Projection);
		auto Det = DirectX::XMMatrixDeterminant(ViewProjection);
		const auto InverseViewProjection = DirectX::XMMatrixInverse(&Det, ViewProjection);
		
		DirectX::XMStoreFloat4x4(&Tr.Projection, Projection);
		DirectX::XMStoreFloat4x4(&Tr.View, View);
		DirectX::XMStoreFloat4x4(&Tr.World, World);
		DirectX::XMStoreFloat4x4(&Tr.InverseViewProjection, InverseViewProjection);

#pragma region FRAME_OBJECT
		DXGI_SWAP_CHAIN_DESC1 SCD;
		SwapChain->GetDesc1(&SCD);
		for (UINT i = 0; i < SCD.BufferCount; ++i) {
			ConstantBuffers.emplace_back().Create(COM_PTR_GET(Device), sizeof(Tr));
		}
#pragma endregion
	}

	virtual void CreateShaderBlob() override {
		const auto ShaderPath = GetBasePath();
		//!< パス0 : シェーダブロブ
		VERIFY_SUCCEEDED(D3DReadFileToBlob(data(ShaderPath + TEXT(".vs.cso")), COM_PTR_PUT(ShaderBlobs.emplace_back())));
		VERIFY_SUCCEEDED(D3DReadFileToBlob(data(ShaderPath + TEXT(".ps.cso")), COM_PTR_PUT(ShaderBlobs.emplace_back())));
		VERIFY_SUCCEEDED(D3DReadFileToBlob(data(ShaderPath + TEXT(".ds.cso")), COM_PTR_PUT(ShaderBlobs.emplace_back())));
		VERIFY_SUCCEEDED(D3DReadFileToBlob(data(ShaderPath + TEXT(".hs.cso")), COM_PTR_PUT(ShaderBlobs.emplace_back())));
		VERIFY_SUCCEEDED(D3DReadFileToBlob(data(ShaderPath + TEXT(".gs.cso")), COM_PTR_PUT(ShaderBlobs.emplace_back())));
		//!< パス1 : シェーダブロブ
		VERIFY_SUCCEEDED(D3DReadFileToBlob(data(ShaderPath + TEXT("_1.vs.cso")), COM_PTR_PUT(ShaderBlobs.emplace_back())));
#ifdef USE_GBUFFER_VISUALIZE
		VERIFY_SUCCEEDED(D3DReadFileToBlob(data(ShaderPath + TEXT("_gb_1.ps.cso")), COM_PTR_PUT(ShaderBlobs.emplace_back())));
		VERIFY_SUCCEEDED(D3DReadFileToBlob(data(ShaderPath + TEXT("_gb_1.gs.cso")), COM_PTR_PUT(ShaderBlobs.emplace_back())));
#else
		VERIFY_SUCCEEDED(D3DReadFileToBlob(data(ShaderPath + TEXT("_1.ps.cso")), COM_PTR_PUT(ShaderBlobs.emplace_back())));
#endif
	}
	virtual void CreatePipelineState() override {
		PipelineStates.resize(2);
		const std::vector RTBDs = {
			D3D12_RENDER_TARGET_BLEND_DESC({
				.BlendEnable = FALSE, .LogicOpEnable = FALSE,
				.SrcBlend = D3D12_BLEND_ONE, .DestBlend = D3D12_BLEND_ZERO, .BlendOp = D3D12_BLEND_OP_ADD,
				.SrcBlendAlpha = D3D12_BLEND_ONE, .DestBlendAlpha = D3D12_BLEND_ZERO, .BlendOpAlpha = D3D12_BLEND_OP_ADD,
				.LogicOp = D3D12_LOGIC_OP_NOOP,
				.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL,
			}),
		};
		std::vector<std::thread> Threads;
		const D3D12_RASTERIZER_DESC RD = {
			D3D12_FILL_MODE_SOLID,
			D3D12_CULL_MODE_BACK, TRUE,
			D3D12_DEFAULT_DEPTH_BIAS, D3D12_DEFAULT_DEPTH_BIAS_CLAMP, D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS, 
			TRUE,
			FALSE, FALSE, 0,
			D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF
		};
		const D3D12_DEPTH_STENCILOP_DESC DSOD = { D3D12_STENCIL_OP_KEEP, D3D12_STENCIL_OP_KEEP, D3D12_STENCIL_OP_KEEP, D3D12_COMPARISON_FUNC_ALWAYS };
		const D3D12_DEPTH_STENCIL_DESC DSD_0 = {
			TRUE, D3D12_DEPTH_WRITE_MASK_ALL, D3D12_COMPARISON_FUNC_LESS,
			FALSE, D3D12_DEFAULT_STENCIL_READ_MASK, D3D12_DEFAULT_STENCIL_WRITE_MASK,
			DSOD, DSOD
		};
		const D3D12_DEPTH_STENCIL_DESC DSD_1 = {
			FALSE, D3D12_DEPTH_WRITE_MASK_ALL, D3D12_COMPARISON_FUNC_LESS,
			FALSE, D3D12_DEFAULT_STENCIL_READ_MASK, D3D12_DEFAULT_STENCIL_WRITE_MASK,
			DSOD, DSOD
		};
		const std::array<D3D12_SHADER_BYTECODE, 5> SBCs_0 = {
			D3D12_SHADER_BYTECODE({ ShaderBlobs[0]->GetBufferPointer(), ShaderBlobs[0]->GetBufferSize() }),
			D3D12_SHADER_BYTECODE({ ShaderBlobs[1]->GetBufferPointer(), ShaderBlobs[1]->GetBufferSize() }),
			D3D12_SHADER_BYTECODE({ ShaderBlobs[2]->GetBufferPointer(), ShaderBlobs[2]->GetBufferSize() }),
			D3D12_SHADER_BYTECODE({ ShaderBlobs[3]->GetBufferPointer(), ShaderBlobs[3]->GetBufferSize() }),
			D3D12_SHADER_BYTECODE({ ShaderBlobs[4]->GetBufferPointer(), ShaderBlobs[4]->GetBufferSize() }),
		};
#ifdef USE_GBUFFER_VISUALIZE
		const std::array<D3D12_SHADER_BYTECODE, 3> SBCs_1 = {
			D3D12_SHADER_BYTECODE({ ShaderBlobs[5]->GetBufferPointer(), ShaderBlobs[5]->GetBufferSize() }),
			D3D12_SHADER_BYTECODE({ ShaderBlobs[6]->GetBufferPointer(), ShaderBlobs[6]->GetBufferSize() }),
			D3D12_SHADER_BYTECODE({ ShaderBlobs[7]->GetBufferPointer(), ShaderBlobs[7]->GetBufferSize() }),
		};
#else
		const std::array<D3D12_SHADER_BYTECODE, 2> SBCs_1 = {
			D3D12_SHADER_BYTECODE({ ShaderBlobs[5]->GetBufferPointer(), ShaderBlobs[5]->GetBufferSize() }),
			D3D12_SHADER_BYTECODE({ ShaderBlobs[6]->GetBufferPointer(), ShaderBlobs[6]->GetBufferSize() }),
		};
#endif
		const std::vector<D3D12_INPUT_ELEMENT_DESC> IEDs = {};
		const std::vector<DXGI_FORMAT> RTVs_0 = { 
			DXGI_FORMAT_R8G8B8A8_UNORM, 
#pragma region MRT
			DXGI_FORMAT_R10G10B10A2_UNORM,
			DXGI_FORMAT_R32_FLOAT, 
			DXGI_FORMAT_R8G8B8A8_UNORM
#pragma endregion
		};
		const std::vector<DXGI_FORMAT> RTVs_1 = { DXGI_FORMAT_R8G8B8A8_UNORM };
#ifdef USE_PIPELINE_SERIALIZE
		PipelineLibrarySerializer PLS(COM_PTR_GET(Device), GetBasePath() + TEXT(".plo"));
		//!< パス0 : パイプラインステート
		Threads.emplace_back(std::thread::thread(DX::CreatePipelineState_, std::ref(PipelineStates[0]), COM_PTR_GET(Device), COM_PTR_GET(RootSignatures[0]), D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH, RTBDs, RD, DSD_0, SBCs_0[0], SBCs_0[1], SBCs_0[2], SBCs_0[3], SBCs_0[4], IEDs, RTVs_0, &PLS, TEXT("0")));
		//!< パス1 : パイプラインステート
#ifdef USE_GBUFFER_VISUALIZE
		Threads.emplace_back(std::thread::thread(DX::CreatePipelineState_, std::ref(PipelineStates[1]), COM_PTR_GET(Device), COM_PTR_GET(RootSignatures[1]), D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE, RTBDs, RD, DSD_1, SBCs_1[0], SBCs_1[1], NullShaderBC, NullShaderBC, SBCs_1[2], IEDs, RTVs_1, &PLS, TEXT("1")));
#else
		Threads.emplace_back(std::thread::thread(DX::CreatePipelineState_, std::ref(PipelineStates[1]), COM_PTR_GET(Device), COM_PTR_GET(RootSignatures[1]), D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE, RTBDs, RD, DSD_1, SBCs_1[0], SBCs_1[1], NullShaderBC, NullShaderBC, NullShaderBC, IEDs, RTVs_1, &PLS, TEXT("1")));
#endif
#else
		Threads.emplace_back(std::thread::thread(DX::CreatePipelineState_, std::ref(PipelineStates[0]), COM_PTR_GET(Device), COM_PTR_GET(RootSignatures[0]), D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH, RTBDs, RD, DSD_0, SBCs_0[0], SBCs_0[1], SBCs_0[2], SBCs_0[3], SBCs_0[4], IEDs, RTVs_0, nullptr, nullptr));
#ifdef USE_GBUFFER_VISUALIZE
		Threads.emplace_back(std::thread::thread(DX::CreatePipelineState_, std::ref(PipelineStates[1]), COM_PTR_GET(Device), COM_PTR_GET(RootSignatures[1]), D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE, RTBDs, RD, DSD_1, SBCs_1[0], SBCs_1[1], NullShaderBC, NullShaderBC, SBCs_1[2], IEDs, RTVs_1, nullptr, nullptr));
#else
		Threads.emplace_back(std::thread::thread(DX::CreatePipelineState_, std::ref(PipelineStates[1]), COM_PTR_GET(Device), COM_PTR_GET(RootSignatures[1]), D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE, RTBDs, RD, DSD_1, SBCs_1[0], SBCs_1[1], NullShaderBC, NullShaderBC, NullShaderBC, IEDs, RTVs_1, nullptr, nullptr));
#endif
#endif	
		for (auto& i : Threads) { i.join(); }
	}
	virtual void PopulateCommandList(const size_t i) override;

private:
	FLOAT Degree = 0.0f;
	struct Transform
	{
		DirectX::XMFLOAT4X4 Projection;
		DirectX::XMFLOAT4X4 View;
		DirectX::XMFLOAT4X4 World;
		DirectX::XMFLOAT4X4 InverseViewProjection;
	};
	using Transform = struct Transform;
	Transform Tr;
};
#pragma endregion