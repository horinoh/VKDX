#pragma once

#include "resource.h"

#pragma region Code
#include "../DXExt.h"

class ShadowMapDX : public DXExt
{
private:
	using Super = DXExt;
public:
	ShadowMapDX() : Super() {}
	virtual ~ShadowMapDX() {}

protected:
	virtual void OnTimer(HWND hWnd, HINSTANCE hInstance) override {
		Super::OnTimer(hWnd, hInstance);

		//DirectX::XMStoreFloat4x4(&Tr.World, DirectX::XMMatrixRotationX(DirectX::XMConvertToRadians(Degree)));
		//Degree += 1.0f;

		CopyToUploadResource(COM_PTR_GET(ConstantBufferResources[0]), RoundUp256(sizeof(Tr)), &Tr);
	}

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
		//!< パス0 : インダイレクトバッファ(シャドウキャスタ描画用)
		CreateIndirectBuffer_DrawIndexed(1, 1);
#ifdef USE_SHADOWMAP_VISUALIZE
		//!< パス1 : インダイレクトバッファ(シャドウマップ描画用)
		CreateIndirectBuffer_Draw(4, 1);
#else
		//!< パス1 : インダイレクトバッファ(シャドウレシーバ描画用)
		CreateIndirectBuffer_DrawIndexed(1, 2);
#endif
	}
	virtual void CreateStaticSampler() override {
		//!< パス1 : スタティックサンプラ
		//!< シェーダ内で SamplerComparisonState を使用する場合は、比較方法(D3D12_FILTER_COMPARISON_..., D3D12_COMPARISON_FUNC_...)を指定すること
		StaticSamplerDescs.push_back({
			D3D12_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR,
			D3D12_TEXTURE_ADDRESS_MODE_WRAP, D3D12_TEXTURE_ADDRESS_MODE_WRAP, D3D12_TEXTURE_ADDRESS_MODE_WRAP,
			0.0f,
			0,
			D3D12_COMPARISON_FUNC_LESS_EQUAL,
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
			assert(!StaticSamplerDescs.empty() && "");
#ifdef USE_SHADOWMAP_VISUALIZE
			const std::array<D3D12_DESCRIPTOR_RANGE, 1> DRs = { {
				{ D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND },
			} };
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
#else
			const std::array<D3D12_DESCRIPTOR_RANGE, 1> DRs_Srv = { { { D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND }, } };
			const std::array<D3D12_DESCRIPTOR_RANGE, 1> DRs_Cbv = { { { D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND }, } };
			DX::SerializeRootSignature(Blob, {
				{ D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE, { static_cast<uint32_t>(DRs_Srv.size()), DRs_Srv.data() }, D3D12_SHADER_VISIBILITY_PIXEL },
				{ D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE, { static_cast<uint32_t>(DRs_Cbv.size()), DRs_Cbv.data() }, D3D12_SHADER_VISIBILITY_GEOMETRY },
			}, {
				StaticSamplerDescs[0],
			}, D3D12_ROOT_SIGNATURE_FLAG_NONE
				| D3D12_ROOT_SIGNATURE_FLAG_DENY_VERTEX_SHADER_ROOT_ACCESS
				| D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS
				| D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS
				//| D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS
				//| D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS
			);
#endif //!< USE_SHADOWMAP_VISUALIZE
#endif //!< USE_HLSL_ROOTSIGNATRUE
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
		const D3D12_RESOURCE_DESC RD = {
			D3D12_RESOURCE_DIMENSION_TEXTURE2D,
			0,
			static_cast<UINT64>(GetClientRectWidth()), static_cast<UINT>(GetClientRectHeight()), // 2048, 2048,
			1,
			1,
			DXGI_FORMAT_D24_UNORM_S8_UINT,
			SD,
			D3D12_TEXTURE_LAYOUT_UNKNOWN,
			D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL
		};
		D3D12_CLEAR_VALUE CV = { RD.Format, { 1.0f, 0 } };
		//!< パス0
		ImageResources.push_back(COM_PTR<ID3D12Resource>());
		VERIFY_SUCCEEDED(Device->CreateCommittedResource(&HP, D3D12_HEAP_FLAG_NONE, &RD, D3D12_RESOURCE_STATE_DEPTH_WRITE, &CV, COM_PTR_UUIDOF_PUTVOID(ImageResources.back())));
		//!< パス1
#ifndef USE_SHADOWMAP_VISUALIZE
		ImageResources.push_back(COM_PTR<ID3D12Resource>());
		VERIFY_SUCCEEDED(Device->CreateCommittedResource(&HP, D3D12_HEAP_FLAG_NONE, &RD, D3D12_RESOURCE_STATE_DEPTH_WRITE, &CV, COM_PTR_UUIDOF_PUTVOID(ImageResources.back())));
#endif
	}

	virtual void CreateDescriptorHeap() override {
		{
			CbvSrvUavDescriptorHeaps.push_back(COM_PTR<ID3D12DescriptorHeap>());
			//!< パス0 + パス1
#ifdef USE_SHADOWMAP_VISUALIZE
			const D3D12_DESCRIPTOR_HEAP_DESC DHD = { D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 1 + 1, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE, 0 }; //!< CBV + SRV
#else
			const D3D12_DESCRIPTOR_HEAP_DESC DHD = { D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 1 + 2, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE, 0 }; //!< CBV + SRV, CBV
#endif
			VERIFY_SUCCEEDED(Device->CreateDescriptorHeap(&DHD, COM_PTR_UUIDOF_PUTVOID(CbvSrvUavDescriptorHeaps.back())));
		}
		{
			DsvDescriptorHeaps.push_back(COM_PTR<ID3D12DescriptorHeap>());
			//!< パス0 + パス1
#ifdef USE_SHADOWMAP_VISUALIZE
			const D3D12_DESCRIPTOR_HEAP_DESC DHD = { D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 1 + 0, D3D12_DESCRIPTOR_HEAP_FLAG_NONE, 0 }; //!< DSV + 
#else
			const D3D12_DESCRIPTOR_HEAP_DESC DHD = { D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 1 + 1, D3D12_DESCRIPTOR_HEAP_FLAG_NONE, 0 }; //!< DSV + DSV
#endif
			VERIFY_SUCCEEDED(Device->CreateDescriptorHeap(&DHD, COM_PTR_UUIDOF_PUTVOID(DsvDescriptorHeaps.back())));
		}
	}
	virtual void CreateDescriptorView() override {
		{
			const auto& DH = CbvSrvUavDescriptorHeaps[0];
			auto CDH = DH->GetCPUDescriptorHandleForHeapStart();

			assert(!ConstantBufferResources.empty() && "");
			const auto CBR = ConstantBufferResources[0];
			assert(CBR->GetDesc().Width == RoundUp256(sizeof(Tr)) && "");
			const D3D12_CONSTANT_BUFFER_VIEW_DESC CBVD = {
				COM_PTR_GET(CBR)->GetGPUVirtualAddress(),
				static_cast<UINT>(CBR->GetDesc().Width)
			};

			//!< パス0
			Device->CreateConstantBufferView(&CBVD, CDH); CDH.ptr += Device->GetDescriptorHandleIncrementSize(DH->GetDesc().Type); //!< CBV

			//!< パス1
			const auto RD = ImageResources[0]->GetDesc();
			assert(DXGI_FORMAT_D24_UNORM_S8_UINT == RD.Format && "");
			assert(D3D12_RESOURCE_DIMENSION_TEXTURE2D == RD.Dimension && "");
			D3D12_SHADER_RESOURCE_VIEW_DESC SRVD = {
				DXGI_FORMAT_R24_UNORM_X8_TYPELESS,
				D3D12_SRV_DIMENSION_TEXTURE2D,
				D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING,
			};
			SRVD.Texture2D = { 0, RD.MipLevels, 0, 0.0f };
			Device->CreateShaderResourceView(COM_PTR_GET(ImageResources[0]), &SRVD, CDH); CDH.ptr += Device->GetDescriptorHandleIncrementSize(DH->GetDesc().Type); //!< SRV

#ifndef USE_SHADOWMAP_VISUALIZE
			Device->CreateConstantBufferView(&CBVD, CDH); CDH.ptr += Device->GetDescriptorHandleIncrementSize(DH->GetDesc().Type); //!< CBV
#endif
		}
		{
			const auto& DH = DsvDescriptorHeaps[0];
			auto CDH = DH->GetCPUDescriptorHandleForHeapStart();

			//!< パス0
			Device->CreateDepthStencilView(COM_PTR_GET(ImageResources[0]), nullptr, CDH); CDH.ptr += Device->GetDescriptorHandleIncrementSize(DH->GetDesc().Type); //!< DSV

			//!< パス1
#ifndef USE_SHADOWMAP_VISUALIZE
			Device->CreateDepthStencilView(COM_PTR_GET(ImageResources[1]), nullptr, CDH); CDH.ptr += Device->GetDescriptorHandleIncrementSize(DH->GetDesc().Type); //!< DSV
#endif
		}
	}

	virtual void CreateConstantBuffer() override {
		{
			const auto Direction = DirectX::XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
			const auto Z = DirectX::XMVector3Normalize(DirectX::XMVectorNegate(Direction));
			const auto X = DirectX::XMVector3Normalize(DirectX::XMVector3Cross(Z, DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f)));
			const auto Y = DirectX::XMVector3Cross(X, Z);
			//!< シャドウキャスタのAABB
			const auto Center = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
			const auto Radius = DirectX::XMVectorSet(1.0f, 1.0f, 1.0f, 1.0f);
			const std::array<DirectX::XMVECTOR, 8> Points = {
				DirectX::XMVectorAdd(Center, DirectX::XMVectorSet( Radius.m128_f32[0],  Radius.m128_f32[1],  Radius.m128_f32[2], 0.0f)),
				DirectX::XMVectorAdd(Center, DirectX::XMVectorSet( Radius.m128_f32[0],  Radius.m128_f32[1], -Radius.m128_f32[2], 0.0f)),
				DirectX::XMVectorAdd(Center, DirectX::XMVectorSet( Radius.m128_f32[0], -Radius.m128_f32[1],  Radius.m128_f32[2], 0.0f)),
				DirectX::XMVectorAdd(Center, DirectX::XMVectorSet( Radius.m128_f32[0], -Radius.m128_f32[1], -Radius.m128_f32[2], 0.0f)),
				DirectX::XMVectorAdd(Center, DirectX::XMVectorSet(-Radius.m128_f32[0],  Radius.m128_f32[1],  Radius.m128_f32[2], 0.0f)),
				DirectX::XMVectorAdd(Center, DirectX::XMVectorSet(-Radius.m128_f32[0],  Radius.m128_f32[1], -Radius.m128_f32[2], 0.0f)),
				DirectX::XMVectorAdd(Center, DirectX::XMVectorSet(-Radius.m128_f32[0], -Radius.m128_f32[1],  Radius.m128_f32[2], 0.0f)),
				DirectX::XMVectorAdd(Center, DirectX::XMVectorSet(-Radius.m128_f32[0], -Radius.m128_f32[1], -Radius.m128_f32[2], 0.0f)),
			};

			auto Mn = (std::numeric_limits<FLOAT>::max)();
			auto Mx = (std::numeric_limits<FLOAT>::min)();
			for (auto i : Points) {
				const auto t = DirectX::XMVector3Dot(i, Z).m128_f32[0];
				Mn = std::min(Mn, t);
				Mx = std::max(Mx, t);
			}
			const auto ZRadius = (Mx - Mn) * 0.5f;

			Mn = (std::numeric_limits<FLOAT>::max)();
			Mx = (std::numeric_limits<FLOAT>::min)();
			for (auto i : Points) {
				const auto t = DirectX::XMVector3Dot(i, X).m128_f32[0];
				Mn = std::min(Mn, t);
				Mx = std::max(Mx, t);
			}
			const auto XRadius = (Mx - Mn) * 0.5f;

			Mn = (std::numeric_limits<FLOAT>::max)();
			Mx = (std::numeric_limits<FLOAT>::min)();
			for (auto i : Points) {
				const auto t = DirectX::XMVector3Dot(i, Y).m128_f32[0];
				Mn = std::min(Mn, t);
				Mx = std::max(Mx, t);
			}
			const auto YRadius = (Mx - Mn) * 0.5f;

			const auto Projection = DirectX::XMMatrixOrthographicRH(XRadius * 2.0f, YRadius * 2.0f, 1.0f, 1.0f + ZRadius * 2.0f);
			const auto View = DirectX::XMMatrixLookAtRH(DirectX::XMVectorSubtract(Center, DirectX::XMVectorScale(Z, (1.0f + ZRadius))), Center, Y);
			DirectX::XMStoreFloat4x4(&Tr.LightProjection, Projection);
			DirectX::XMStoreFloat4x4(&Tr.LightView, View);
#ifdef USE_SHADOWMAP_VISUALIZE
			DirectX::XMStoreFloat4x4(&Tr.Projection, Projection);
			DirectX::XMStoreFloat4x4(&Tr.View, View);
#endif
			const auto ViewProjection = DirectX::XMMatrixMultiply(View, Projection);
			auto Det = DirectX::XMMatrixDeterminant(ViewProjection);
		}
#ifndef USE_SHADOWMAP_VISUALIZE
		{
			const auto Fov = 0.16f * DirectX::XM_PI;
			const auto Aspect = GetAspectRatioOfClientRect();
			const auto ZFar = 4.0f;
			const auto ZNear = 2.0f;
			const auto CamPos = DirectX::XMVectorSet(0.0f, 0.0f, 3.0f, 1.0f);
			const auto CamTag = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
			const auto CamUp = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
			const auto Projection = DirectX::XMMatrixPerspectiveFovRH(Fov, Aspect, ZNear, ZFar);
			const auto View = DirectX::XMMatrixLookAtRH(CamPos, CamTag, CamUp);
			DirectX::XMStoreFloat4x4(&Tr.Projection, Projection);
			DirectX::XMStoreFloat4x4(&Tr.View, View);
			const auto ViewProjection = DirectX::XMMatrixMultiply(View, Projection);
			auto Det = DirectX::XMMatrixDeterminant(ViewProjection);
		}
#endif
		const auto World = DirectX::XMMatrixIdentity();
		DirectX::XMStoreFloat4x4(&Tr.World, World);

		{
			ConstantBufferResources.push_back(COM_PTR<ID3D12Resource>());
			CreateUploadResource(COM_PTR_PUT(ConstantBufferResources.back()), RoundUp256(sizeof(Tr)));
		}
	}

	virtual void CreateShaderBlobs() override {
		ShaderBlobs.resize(5 + 2);
		const auto ShaderPath = GetBasePath();
		//!< パス0 : シェーダブロブ
		VERIFY_SUCCEEDED(D3DReadFileToBlob((ShaderPath + TEXT(".vs.cso")).data(), COM_PTR_PUT(ShaderBlobs[0])));
		VERIFY_SUCCEEDED(D3DReadFileToBlob((ShaderPath + TEXT(".ds.cso")).data(), COM_PTR_PUT(ShaderBlobs[1])));
		VERIFY_SUCCEEDED(D3DReadFileToBlob((ShaderPath + TEXT(".hs.cso")).data(), COM_PTR_PUT(ShaderBlobs[2])));
		VERIFY_SUCCEEDED(D3DReadFileToBlob((ShaderPath + TEXT(".gs.cso")).data(), COM_PTR_PUT(ShaderBlobs[3])));
		//!< パス1 : シェーダブロブ
#ifdef USE_SHADOWMAP_VISUALIZE
		VERIFY_SUCCEEDED(D3DReadFileToBlob((ShaderPath + TEXT("_sm_1.vs.cso")).data(), COM_PTR_PUT(ShaderBlobs[4])));
		VERIFY_SUCCEEDED(D3DReadFileToBlob((ShaderPath + TEXT("_sm_1.ps.cso")).data(), COM_PTR_PUT(ShaderBlobs[5])));
#else
		VERIFY_SUCCEEDED(D3DReadFileToBlob((ShaderPath + TEXT("_1.ps.cso")).data(), COM_PTR_PUT(ShaderBlobs[4])));
		VERIFY_SUCCEEDED(D3DReadFileToBlob((ShaderPath + TEXT("_1.gs.cso")).data(), COM_PTR_PUT(ShaderBlobs[5])));
#endif
	}
	virtual void CreatePipelineStates() override {
		PipelineStates.resize(2);
		std::vector<std::thread> Threads;
		//!< RD_0 : デプスバイアス有り (With depth bias)
		const D3D12_RASTERIZER_DESC RD_0 = {
			D3D12_FILL_MODE_SOLID, 
			D3D12_CULL_MODE_BACK, TRUE,
			//!< シャドウキャスタの描画でデプスバイアスを有効にする
			//!< DepthBias, DepthBiasClamp, SlopeScaledDepthBias
			//!< r * DepthBias + m * SlopeScaledDepthBias
			//!< DepthBiasClamp : 非0.0fを指定の場合クランプが有効になる(絶対値がDepthBiasClamp以下になるようにクランプされる)
			1, 0.0f, 1.75f, 
			TRUE,
			FALSE, FALSE, 0,
			D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF
		};
		//!< RD_1 : デプスバイアス無し (No depth bias)
		const D3D12_RASTERIZER_DESC RD_1 = {
			D3D12_FILL_MODE_SOLID,
			D3D12_CULL_MODE_BACK, TRUE,
			D3D12_DEFAULT_DEPTH_BIAS, D3D12_DEFAULT_DEPTH_BIAS_CLAMP, D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS,
			TRUE,
			FALSE, FALSE, 0,
			D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF
		};
		const D3D12_DEPTH_STENCILOP_DESC DSOD = { D3D12_STENCIL_OP_KEEP, D3D12_STENCIL_OP_KEEP, D3D12_STENCIL_OP_KEEP, D3D12_COMPARISON_FUNC_ALWAYS };
		//!< DSD_0 : デプステスト有り (With depth test)
		const D3D12_DEPTH_STENCIL_DESC DSD_0 = {
			TRUE, D3D12_DEPTH_WRITE_MASK_ALL, D3D12_COMPARISON_FUNC_LESS,
			FALSE, D3D12_DEFAULT_STENCIL_READ_MASK, D3D12_DEFAULT_STENCIL_WRITE_MASK,
			DSOD, DSOD
		};
		//!< DSD_1 : デプステスト無し (No depth test)
		const D3D12_DEPTH_STENCIL_DESC DSD_1 = {
			FALSE, D3D12_DEPTH_WRITE_MASK_ALL, D3D12_COMPARISON_FUNC_LESS,
			FALSE, D3D12_DEFAULT_STENCIL_READ_MASK, D3D12_DEFAULT_STENCIL_WRITE_MASK,
			DSOD, DSOD
		};
		const std::array<D3D12_SHADER_BYTECODE, 4> SBCs_0 = {
			D3D12_SHADER_BYTECODE({ ShaderBlobs[0]->GetBufferPointer(), ShaderBlobs[0]->GetBufferSize() }),
			D3D12_SHADER_BYTECODE({ ShaderBlobs[1]->GetBufferPointer(), ShaderBlobs[1]->GetBufferSize() }),
			D3D12_SHADER_BYTECODE({ ShaderBlobs[2]->GetBufferPointer(), ShaderBlobs[2]->GetBufferSize() }),
			D3D12_SHADER_BYTECODE({ ShaderBlobs[3]->GetBufferPointer(), ShaderBlobs[3]->GetBufferSize() }),
		};
#ifdef USE_SHADOWMAP_VISUALIZE
		const std::array<D3D12_SHADER_BYTECODE, 2> SBCs_1 = {
			D3D12_SHADER_BYTECODE({ ShaderBlobs[4]->GetBufferPointer(), ShaderBlobs[4]->GetBufferSize() }),
			D3D12_SHADER_BYTECODE({ ShaderBlobs[5]->GetBufferPointer(), ShaderBlobs[5]->GetBufferSize() }),
		};
#else
		const std::array<D3D12_SHADER_BYTECODE, 5> SBCs_1 = {
			D3D12_SHADER_BYTECODE({ ShaderBlobs[0]->GetBufferPointer(), ShaderBlobs[0]->GetBufferSize() }),
			D3D12_SHADER_BYTECODE({ ShaderBlobs[4]->GetBufferPointer(), ShaderBlobs[4]->GetBufferSize() }), //!< 
			D3D12_SHADER_BYTECODE({ ShaderBlobs[1]->GetBufferPointer(), ShaderBlobs[1]->GetBufferSize() }),
			D3D12_SHADER_BYTECODE({ ShaderBlobs[2]->GetBufferPointer(), ShaderBlobs[2]->GetBufferSize() }),
			D3D12_SHADER_BYTECODE({ ShaderBlobs[5]->GetBufferPointer(), ShaderBlobs[5]->GetBufferSize() }), //!< 
		};
#endif
		const std::vector<D3D12_INPUT_ELEMENT_DESC> IEDs = {};
		const std::vector<DXGI_FORMAT> RTVs_0 = {};
		const std::vector<DXGI_FORMAT> RTVs_1 = { DXGI_FORMAT_R8G8B8A8_UNORM };
#ifdef USE_PIPELINE_SERIALIZE
		PipelineLibrarySerializer PLS(COM_PTR_GET(Device), GetBasePath() + TEXT(".plo"));
		//!< パス0 : パイプラインステート
		Threads.push_back(std::thread::thread(DX::CreatePipelineState, std::ref(PipelineStates[0]), COM_PTR_GET(Device), COM_PTR_GET(RootSignatures[0]), D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH, RD_0, DSD_0, SBCs_0[0], NullShaderBC, SBCs_0[1], SBCs_0[2], SBCs_0[3], IEDs, RTVs_0, &PLS, TEXT("0")));
		//!< パス1 : パイプラインステート
#ifdef USE_SHADOWMAP_VISUALIZE
		Threads.push_back(std::thread::thread(DX::CreatePipelineState, std::ref(PipelineStates[1]), COM_PTR_GET(Device), COM_PTR_GET(RootSignatures[1]), D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE, RD_1, DSD_1, SBCs_1[0], SBCs_1[1], NullShaderBC, NullShaderBC, NullShaderBC, IEDs, RTVs_1, &PLS, TEXT("1")));
#else
		Threads.push_back(std::thread::thread(DX::CreatePipelineState, std::ref(PipelineStates[1]), COM_PTR_GET(Device), COM_PTR_GET(RootSignatures[1]), D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH, RD_1, DSD_0, SBCs_1[0], SBCs_1[1], SBCs_1[2], SBCs_1[3], SBCs_1[4], IEDs, RTVs_1, &PLS, TEXT("1")));
#endif
#else
		Threads.push_back(std::thread::thread(DX::CreatePipelineState, std::ref(PipelineStates[0]), COM_PTR_GET(Device), COM_PTR_GET(RootSignatures[0]), D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH, RD_0, DSD_0, SBCs_0[0], NullShaderBC, SBCs_0[1], SBCs_0[2], SBCs_0[3], IEDs, RTVs_0, nullptr, nullptr));
#ifdef USE_SHADOWMAP_VISUALIZE
		Threads.push_back(std::thread::thread(DX::CreatePipelineState, std::ref(PipelineStates[1]), COM_PTR_GET(Device), COM_PTR_GET(RootSignatures[1]), D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE, RD_1, DSD_1, SBCs_1[0], SBCs_1[1], NullShaderBC, NullShaderBC, NullShaderBC, IEDs, RTVs_1, nullptr, nullptr));
#else
		Threads.push_back(std::thread::thread(DX::CreatePipelineState, std::ref(PipelineStates[1]), COM_PTR_GET(Device), COM_PTR_GET(RootSignatures[1]), D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH, RD_1, DSD_0, SBCs_1[0], SBCs_1[1], SBCs_1[2], SBCs_1[3], SBCs_1[4], IEDs, RTVs_1, nullptr, nullptr));
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
		DirectX::XMFLOAT4X4 LightProjection;
		DirectX::XMFLOAT4X4 LightView;
	};
	using Transform = struct Transform;
	Transform Tr;
};
#pragma endregion