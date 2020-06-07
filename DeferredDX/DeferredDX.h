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

		CopyToUploadResource(COM_PTR_GET(ConstantBufferResources[0]), RoundUp256(sizeof(Tr)), &Tr);
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
			BundleGraphicsCommandLists.push_back(COM_PTR<ID3D12GraphicsCommandList>());
			VERIFY_SUCCEEDED(Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_BUNDLE, COM_PTR_GET(BundleCommandAllocators[0]), nullptr, COM_PTR_UUIDOF_PUTVOID(BundleGraphicsCommandLists.back())));
			VERIFY_SUCCEEDED(BundleGraphicsCommandLists.back()->Close());
		}
	}

	virtual void CreateIndirectBuffer() override {
		//!< パス0 : インダイレクトバッファ(メッシュ描画用)
		CreateIndirectBuffer_DrawIndexed(1, 1);
		//!< パス1 : インダイレクトバッファ(フルスクリーン描画用)
		CreateIndirectBuffer_Draw(4, 1);
	}
	virtual void CreateStaticSampler() override {
		//!< パス1 : スタティックサンプラ
		StaticSamplerDescs.push_back({
			D3D12_FILTER_MIN_MAG_MIP_LINEAR,
			D3D12_TEXTURE_ADDRESS_MODE_WRAP, D3D12_TEXTURE_ADDRESS_MODE_WRAP, D3D12_TEXTURE_ADDRESS_MODE_WRAP,
			0.0f,
			0,
			D3D12_COMPARISON_FUNC_NEVER,
			D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE,
			0.0f, 1.0f,
			0, 0, D3D12_SHADER_VISIBILITY_PIXEL
			});
	}
	virtual void CreateRootSignature() override {
		//!< パス0 : ルートシグネチャ
		{
			RootSignatures.push_back(COM_PTR<ID3D12RootSignature>());
			COM_PTR<ID3DBlob> Blob;
#ifdef USE_HLSL_ROOTSIGNATRUE
			GetRootSignaturePartFromShader(Blob, (GetBasePath() + TEXT(".rs.cso")).data());
#else
			const std::array<D3D12_DESCRIPTOR_RANGE, 1> DRs = { {
				{ D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND },
			} };
			SerializeRootSignature(Blob, {
					{ D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE, { static_cast<UINT>(DRs.size()), DRs.data() }, D3D12_SHADER_VISIBILITY_GEOMETRY }
				}, {}, D3D12_ROOT_SIGNATURE_FLAG_NONE
				| D3D12_ROOT_SIGNATURE_FLAG_DENY_VERTEX_SHADER_ROOT_ACCESS
				| D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS
				| D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS
				//| D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS
				| D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS);
#endif
			VERIFY_SUCCEEDED(Device->CreateRootSignature(0, Blob->GetBufferPointer(), Blob->GetBufferSize(), COM_PTR_UUIDOF_PUTVOID(RootSignatures.back())));
		}
		//!< パス1 : ルートシグネチャ
		{
			RootSignatures.push_back(COM_PTR<ID3D12RootSignature>());
			COM_PTR<ID3DBlob> Blob;
#ifdef USE_HLSL_ROOTSIGNATRUE 
			GetRootSignaturePartFromShader(Blob, (GetBasePath() + TEXT("_1.rs.cso")).data());
#else
			const std::array<D3D12_DESCRIPTOR_RANGE, 2> DRs = { {
#pragma region MRT
				//!< レンダーターゲット : カラー(RenderTarget : Color), 法線(RenderTarget : Normal), 深度(RenderTarget : Depth), 未定
				{ D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 4, 0, 0, D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND },
#pragma endregion
				{ D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND },
			} };
			assert(!StaticSamplerDescs.empty() && "");
			DX::SerializeRootSignature(Blob, {
					{ D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE, { static_cast<uint32_t>(DRs.size()), DRs.data() }, D3D12_SHADER_VISIBILITY_PIXEL },
				}, {
					StaticSamplerDescs[0],
				}, D3D12_ROOT_SIGNATURE_FLAG_NONE
				| D3D12_ROOT_SIGNATURE_FLAG_DENY_VERTEX_SHADER_ROOT_ACCESS
				| D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS
				| D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS
				| D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS
				//| D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS
			);
#endif
			VERIFY_SUCCEEDED(Device->CreateRootSignature(0, Blob->GetBufferPointer(), Blob->GetBufferSize(), COM_PTR_UUIDOF_PUTVOID(RootSignatures.back())));
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
			ImageResources.push_back(COM_PTR<ID3D12Resource>());
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
			VERIFY_SUCCEEDED(Device->CreateCommittedResource(&HP, D3D12_HEAP_FLAG_NONE, &RD, D3D12_RESOURCE_STATE_RENDER_TARGET, &CV, COM_PTR_UUIDOF_PUTVOID(ImageResources.back())));
		}
#pragma region MRT
		//!< レンダーターゲット : 法線(RenderTarget : Normal)
		{
			ImageResources.push_back(COM_PTR<ID3D12Resource>());
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
			VERIFY_SUCCEEDED(Device->CreateCommittedResource(&HP, D3D12_HEAP_FLAG_NONE, &RD, D3D12_RESOURCE_STATE_RENDER_TARGET, &CV, COM_PTR_UUIDOF_PUTVOID(ImageResources.back())));
		}
		//!< レンダーターゲット : 深度(RenderTarget : Depth)
		{
			ImageResources.push_back(COM_PTR<ID3D12Resource>());
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
			VERIFY_SUCCEEDED(Device->CreateCommittedResource(&HP, D3D12_HEAP_FLAG_NONE, &RD, D3D12_RESOURCE_STATE_RENDER_TARGET, &CV, COM_PTR_UUIDOF_PUTVOID(ImageResources.back())));
		}
		//!< レンダーターゲット : 未定
		{
			ImageResources.push_back(COM_PTR<ID3D12Resource>());
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
			VERIFY_SUCCEEDED(Device->CreateCommittedResource(&HP, D3D12_HEAP_FLAG_NONE, &RD, D3D12_RESOURCE_STATE_RENDER_TARGET, &CV, COM_PTR_UUIDOF_PUTVOID(ImageResources.back())));
		}
#pragma endregion
		{
			ImageResources.push_back(COM_PTR<ID3D12Resource>());
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
			VERIFY_SUCCEEDED(Device->CreateCommittedResource(&HP, D3D12_HEAP_FLAG_NONE, &RD, D3D12_RESOURCE_STATE_DEPTH_WRITE, &CV, COM_PTR_UUIDOF_PUTVOID(ImageResources.back())));
		}
	}

	virtual void CreateDescriptorHeap() override {
		//!< パス0 : レンダーターゲット
		{
			{
				CbvSrvUavDescriptorHeaps.push_back(COM_PTR<ID3D12DescriptorHeap>());
				const D3D12_DESCRIPTOR_HEAP_DESC DHD = { D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 1, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE, 0 }; //!< CBV
				VERIFY_SUCCEEDED(Device->CreateDescriptorHeap(&DHD, COM_PTR_UUIDOF_PUTVOID(CbvSrvUavDescriptorHeaps.back())));
			}
#pragma region MRT
			{
				//!< レンダーターゲット : カラー(RenderTarget : Color), 法線(RenderTarget : Normal), 深度(RenderTarget : Depth), 未定
				RtvDescriptorHeaps.push_back(COM_PTR<ID3D12DescriptorHeap>());
				const D3D12_DESCRIPTOR_HEAP_DESC DHD = { D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 4, D3D12_DESCRIPTOR_HEAP_FLAG_NONE, 0 }; //!< 4RTV
				VERIFY_SUCCEEDED(Device->CreateDescriptorHeap(&DHD, COM_PTR_UUIDOF_PUTVOID(RtvDescriptorHeaps.back())));
			}
#pragma endregion
			{
				DsvDescriptorHeaps.push_back(COM_PTR<ID3D12DescriptorHeap>());
				const D3D12_DESCRIPTOR_HEAP_DESC DHD = { D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 1, D3D12_DESCRIPTOR_HEAP_FLAG_NONE, 0 }; //!< DSV
				VERIFY_SUCCEEDED(Device->CreateDescriptorHeap(&DHD, COM_PTR_UUIDOF_PUTVOID(DsvDescriptorHeaps.back())));
			}
		}
		//!< パス1 : シェーダリソース
		{
#pragma region MRT
			{
				//!< レンダーターゲット : カラー(RenderTarget : Color), 法線(RenderTarget : Normal), 深度(RenderTarget : Depth), 未定
				CbvSrvUavDescriptorHeaps.push_back(COM_PTR<ID3D12DescriptorHeap>());
				const D3D12_DESCRIPTOR_HEAP_DESC DHD = { D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 4 + 1, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE, 0 }; //!< 4SRV + CBV
				VERIFY_SUCCEEDED(Device->CreateDescriptorHeap(&DHD, COM_PTR_UUIDOF_PUTVOID(CbvSrvUavDescriptorHeaps.back())));
			}
#pragma endregion
		}
	}
	virtual void CreateDescriptorView() override {
		//!< パス0 : レンダーターゲットビュー
		{
			{
				const auto& DH = CbvSrvUavDescriptorHeaps[0];
				auto CDH = DH->GetCPUDescriptorHandleForHeapStart();
				assert(!ConstantBufferResources.empty() && "");
				const auto CBR = ConstantBufferResources[0];
				assert(CBR->GetDesc().Width == RoundUp256(sizeof(Tr)) && "");
				const D3D12_CONSTANT_BUFFER_VIEW_DESC CBVD = { COM_PTR_GET(CBR)->GetGPUVirtualAddress(), static_cast<UINT>(CBR->GetDesc().Width) };
				Device->CreateConstantBufferView(&CBVD, CDH); CDH.ptr += Device->GetDescriptorHandleIncrementSize(DH->GetDesc().Type);
			}
			{
				const auto& DH = RtvDescriptorHeaps[0];
				auto CDH = DH->GetCPUDescriptorHandleForHeapStart();
				//!< レンダーターゲット : カラー(RenderTarget : Color)
				Device->CreateRenderTargetView(COM_PTR_GET(ImageResources[0]), nullptr, CDH); CDH.ptr += Device->GetDescriptorHandleIncrementSize(DH->GetDesc().Type);
#pragma region MRT
				//!< レンダーターゲット : 法線(RenderTarget : Normal)
				Device->CreateRenderTargetView(COM_PTR_GET(ImageResources[1]), nullptr, CDH); CDH.ptr += Device->GetDescriptorHandleIncrementSize(DH->GetDesc().Type);
				//!< レンダーターゲット : 深度(RenderTarget : Depth)
				Device->CreateRenderTargetView(COM_PTR_GET(ImageResources[2]), nullptr, CDH); CDH.ptr += Device->GetDescriptorHandleIncrementSize(DH->GetDesc().Type);
				//!< レンダーターゲット : 未定
				Device->CreateRenderTargetView(COM_PTR_GET(ImageResources[3]), nullptr, CDH); CDH.ptr += Device->GetDescriptorHandleIncrementSize(DH->GetDesc().Type);
#pragma endregion
			}
			{
				const auto& DH = DsvDescriptorHeaps[0];
				auto CDH = DH->GetCPUDescriptorHandleForHeapStart();
				Device->CreateDepthStencilView(COM_PTR_GET(ImageResources[4]), nullptr, CDH); CDH.ptr += Device->GetDescriptorHandleIncrementSize(DH->GetDesc().Type);
			}
		}
		//!< パス1 : シェーダリソースビュー
		{
			const auto& DH = CbvSrvUavDescriptorHeaps[1];
			auto CDH = DH->GetCPUDescriptorHandleForHeapStart();
			{
				//!< レンダーターゲット : カラー(RenderTarget : Color)
				Device->CreateShaderResourceView(COM_PTR_GET(ImageResources[0]), nullptr, CDH); CDH.ptr += Device->GetDescriptorHandleIncrementSize(DH->GetDesc().Type);
#pragma region MRT
				//!< レンダーターゲット : 法線(RenderTarget : Normal)
				Device->CreateShaderResourceView(COM_PTR_GET(ImageResources[1]), nullptr, CDH); CDH.ptr += Device->GetDescriptorHandleIncrementSize(DH->GetDesc().Type);
				//!< レンダーターゲット : 深度(RenderTarget : Depth)
				Device->CreateShaderResourceView(COM_PTR_GET(ImageResources[2]), nullptr, CDH); CDH.ptr += Device->GetDescriptorHandleIncrementSize(DH->GetDesc().Type);
				//!< レンダーターゲット : 未定
				Device->CreateShaderResourceView(COM_PTR_GET(ImageResources[3]), nullptr, CDH); CDH.ptr += Device->GetDescriptorHandleIncrementSize(DH->GetDesc().Type);
#pragma	endregion
			}
			{
				const auto CBR = ConstantBufferResources[0];
				assert(CBR->GetDesc().Width == RoundUp256(sizeof(Tr)) && "");
				const D3D12_CONSTANT_BUFFER_VIEW_DESC CBVD = { COM_PTR_GET(CBR)->GetGPUVirtualAddress(), static_cast<UINT>(CBR->GetDesc().Width) };
				Device->CreateConstantBufferView(&CBVD, CDH); CDH.ptr += Device->GetDescriptorHandleIncrementSize(DH->GetDesc().Type);
			}
		}
	}

	virtual void CreateConstantBuffer() override {
		const auto Fov = 0.16f * DirectX::XM_PI;
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

		{
			ConstantBufferResources.push_back(COM_PTR<ID3D12Resource>());
			CreateUploadResource(COM_PTR_PUT(ConstantBufferResources.back()), RoundUp256(sizeof(Tr)));
		}
	}

	virtual void CreateShaderBlobs() override {
#ifdef USE_GBUFFER_VISUALIZE
		ShaderBlobs.resize(5 + 3);
#else
		ShaderBlobs.resize(5 + 2);
#endif
		const auto ShaderPath = GetBasePath();
		//!< パス0 : シェーダブロブ
		VERIFY_SUCCEEDED(D3DReadFileToBlob((ShaderPath + TEXT(".vs.cso")).data(), COM_PTR_PUT(ShaderBlobs[0])));
		VERIFY_SUCCEEDED(D3DReadFileToBlob((ShaderPath + TEXT(".ps.cso")).data(), COM_PTR_PUT(ShaderBlobs[1])));
		VERIFY_SUCCEEDED(D3DReadFileToBlob((ShaderPath + TEXT(".ds.cso")).data(), COM_PTR_PUT(ShaderBlobs[2])));
		VERIFY_SUCCEEDED(D3DReadFileToBlob((ShaderPath + TEXT(".hs.cso")).data(), COM_PTR_PUT(ShaderBlobs[3])));
		VERIFY_SUCCEEDED(D3DReadFileToBlob((ShaderPath + TEXT(".gs.cso")).data(), COM_PTR_PUT(ShaderBlobs[4])));
		//!< パス1 : シェーダブロブ
		VERIFY_SUCCEEDED(D3DReadFileToBlob((ShaderPath + TEXT("_1.vs.cso")).data(), COM_PTR_PUT(ShaderBlobs[5])));
#ifdef USE_GBUFFER_VISUALIZE
		VERIFY_SUCCEEDED(D3DReadFileToBlob((ShaderPath + TEXT("_gb_1.ps.cso")).data(), COM_PTR_PUT(ShaderBlobs[6])));
		VERIFY_SUCCEEDED(D3DReadFileToBlob((ShaderPath + TEXT("_gb_1.gs.cso")).data(), COM_PTR_PUT(ShaderBlobs[7])));
#else
		VERIFY_SUCCEEDED(D3DReadFileToBlob((ShaderPath + TEXT("_1.ps.cso")).data(), COM_PTR_PUT(ShaderBlobs[6])));
#endif
	}
	virtual void CreatePipelineStates() override {
		PipelineStates.resize(2);
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
		Threads.push_back(std::thread::thread(DX::CreatePipelineState, std::ref(PipelineStates[0]), COM_PTR_GET(Device), COM_PTR_GET(RootSignatures[0]), D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH, RD, DSD_0, SBCs_0[0], SBCs_0[1], SBCs_0[2], SBCs_0[3], SBCs_0[4], IEDs, RTVs_0, &PLS, TEXT("0")));
		//!< パス1 : パイプラインステート
#ifdef USE_GBUFFER_VISUALIZE
		Threads.push_back(std::thread::thread(DX::CreatePipelineState, std::ref(PipelineStates[1]), COM_PTR_GET(Device), COM_PTR_GET(RootSignatures[1]), D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE, RD, DSD_1, SBCs_1[0], SBCs_1[1], NullShaderBC, NullShaderBC, SBCs_1[2], IEDs, RTVs_1, &PLS, TEXT("1")));
#else
		Threads.push_back(std::thread::thread(DX::CreatePipelineState, std::ref(PipelineStates[1]), COM_PTR_GET(Device), COM_PTR_GET(RootSignatures[1]), D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE, RD, DSD_1, SBCs_1[0], SBCs_1[1], NullShaderBC, NullShaderBC, NullShaderBC, IEDs, RTVs_1, &PLS, TEXT("1")));
#endif
#else
		Threads.push_back(std::thread::thread(DX::CreatePipelineState, std::ref(PipelineStates[0]), COM_PTR_GET(Device), COM_PTR_GET(RootSignatures[0]), D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH, RD, DSD_0, SBCs_0[0], SBCs_0[1], SBCs_0[2], SBCs_0[3], SBCs_0[4], IEDs, RTVs_0, nullptr, nullptr));
#ifdef USE_GBUFFER_VISUALIZE
		Threads.push_back(std::thread::thread(DX::CreatePipelineState, std::ref(PipelineStates[1]), COM_PTR_GET(Device), COM_PTR_GET(RootSignatures[1]), D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE, RD, DSD_1, SBCs_1[0], SBCs_1[1], NullShaderBC, NullShaderBC, SBCs_1[2], IEDs, RTVs_1, nullptr, nullptr));
#else
		Threads.push_back(std::thread::thread(DX::CreatePipelineState, std::ref(PipelineStates[1]), COM_PTR_GET(Device), COM_PTR_GET(RootSignatures[1]), D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE, RD, DSD_1, SBCs_1[0], SBCs_1[1], NullShaderBC, NullShaderBC, NullShaderBC, IEDs, RTVs_1, nullptr, nullptr));
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