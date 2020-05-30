#pragma once

#include "resource.h"

#pragma region Code
#include "../DXExt.h"

class RenderTargetDX : public DXExt
{
private:
	using Super = DXExt;
public:
	RenderTargetDX() : Super() {}
	virtual ~RenderTargetDX() {}

protected:
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
			SerializeRootSignature(Blob, {}, {}, D3D12_ROOT_SIGNATURE_FLAG_NONE
				| D3D12_ROOT_SIGNATURE_FLAG_DENY_VERTEX_SHADER_ROOT_ACCESS
				| D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS
				| D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS
				| D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS
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
			const std::array<D3D12_DESCRIPTOR_RANGE, 1> DRs_Srv = {
				{ D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND }
			};
			assert(!StaticSamplerDescs.empty() && "");
			DX::SerializeRootSignature(Blob, {
					{ D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE, { static_cast<uint32_t>(DRs_Srv.size()), DRs_Srv.data() }, D3D12_SHADER_VISIBILITY_PIXEL }
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
			const D3D12_CLEAR_VALUE CV = {
				RD.Format,
				{ DirectX::Colors::SkyBlue.f[0], DirectX::Colors::SkyBlue.f[1], DirectX::Colors::SkyBlue.f[2], DirectX::Colors::SkyBlue.f[3] },
			};
			VERIFY_SUCCEEDED(Device->CreateCommittedResource(&HP, D3D12_HEAP_FLAG_NONE, &RD, D3D12_RESOURCE_STATE_RENDER_TARGET, &CV, COM_PTR_UUIDOF_PUTVOID(ImageResources.back())));
		}

#ifdef USE_DEPTH
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
#endif
	}

	virtual void CreateDescriptorHeap() override {
		//!< パス0 : レンダーターゲット
		{
			{
				RtvDescriptorHeaps.push_back(COM_PTR<ID3D12DescriptorHeap>());
				const D3D12_DESCRIPTOR_HEAP_DESC DHD = { D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 1, D3D12_DESCRIPTOR_HEAP_FLAG_NONE, 0 };
				VERIFY_SUCCEEDED(Device->CreateDescriptorHeap(&DHD, COM_PTR_UUIDOF_PUTVOID(RtvDescriptorHeaps.back())));
			}
#ifdef USE_DEPTH
			{
				DsvDescriptorHeaps.push_back(COM_PTR<ID3D12DescriptorHeap>());
				const D3D12_DESCRIPTOR_HEAP_DESC DHD = { D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 1, D3D12_DESCRIPTOR_HEAP_FLAG_NONE, 0 };
				VERIFY_SUCCEEDED(Device->CreateDescriptorHeap(&DHD, COM_PTR_UUIDOF_PUTVOID(DsvDescriptorHeaps.back())));
			}
#endif
		}
		//!< パス1 : シェーダリソース
		{
			CbvSrvUavDescriptorHeaps.push_back(COM_PTR<ID3D12DescriptorHeap>());
			const D3D12_DESCRIPTOR_HEAP_DESC DHD = { D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 1, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE, 0 };
			VERIFY_SUCCEEDED(Device->CreateDescriptorHeap(&DHD, COM_PTR_UUIDOF_PUTVOID(CbvSrvUavDescriptorHeaps.back())));
		}
	}
	virtual void CreateDescriptorView() override {
#ifdef USE_DEPTH
		assert(2 == ImageResources.size() && "");
#else
		assert(!ImageResources.empty() && "");
#endif
		//!< パス0 : レンダーターゲットビュー
		{
			{
				const auto& DH = RtvDescriptorHeaps[0];
				auto CDH = DH->GetCPUDescriptorHandleForHeapStart();
#if 0
				const auto RD = ImageResources[0]->GetDesc(); assert(RD.MipLevels > 0 && ""); assert(D3D12_RESOURCE_DIMENSION_TEXTURE2D == RD.Dimension);
				D3D12_RENDER_TARGET_VIEW_DESC RTVD = {
					RD.Format,
					D3D12_RTV_DIMENSION_TEXTURE2D,
				};
				RTVD.Texture2D = { 0, 0 };
				Device->CreateRenderTargetView(COM_PTR_GET(ImageResources[0]), &RTVD, CDH); CDH.ptr += Device->GetDescriptorHandleIncrementSize(DH->GetDesc().Type);
#else
				Device->CreateRenderTargetView(COM_PTR_GET(ImageResources[0]), nullptr, CDH); CDH.ptr += Device->GetDescriptorHandleIncrementSize(DH->GetDesc().Type);
#endif
			}
#ifdef USE_DEPTH
			{
				const auto& DH = DsvDescriptorHeaps[0];
				auto CDH = DH->GetCPUDescriptorHandleForHeapStart();
#if 0
				const auto RD = ImageResources[1]->GetDesc(); assert(RD.MipLevels > 0 && ""); assert(D3D12_RESOURCE_DIMENSION_TEXTURE2D == RD.Dimension);
				D3D12_DEPTH_STENCIL_VIEW_DESC DSVD = {
					RD.Format,
					D3D12_DSV_DIMENSION_TEXTURE2D,
					D3D12_DSV_FLAG_NONE
				};
				DSVD.Texture2D = { 0 };
				Device->CreateDepthStencilView(COM_PTR_GET(ImageResources[1]), &DSVD, CDH); CDH.ptr += Device->GetDescriptorHandleIncrementSize(DH->GetDesc().Type);
#else
				Device->CreateDepthStencilView(COM_PTR_GET(ImageResources[1]), nullptr, CDH); CDH.ptr += Device->GetDescriptorHandleIncrementSize(DH->GetDesc().Type);
#endif
			}
#endif
		}
		//!< パス1 : シェーダリソースビュー
		{
			const auto& DH = CbvSrvUavDescriptorHeaps[0];
			auto CDH = DH->GetCPUDescriptorHandleForHeapStart();
			{
#if 0
				const auto RD = ImageResources[0]->GetDesc(); assert(D3D12_RESOURCE_DIMENSION_TEXTURE2D == RD.Dimension);
				D3D12_SHADER_RESOURCE_VIEW_DESC SRVD = {
					RD.Format,
					D3D12_SRV_DIMENSION_TEXTURE2D,
					D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING,
				};
				SRVD.Texture2D = { 0, RD.MipLevels, 0, 0.0f };
				Device->CreateShaderResourceView(COM_PTR_GET(ImageResources[0]), &SRVD, CDH); CDH.ptr += Device->GetDescriptorHandleIncrementSize(DH->GetDesc().Type);
#else
				Device->CreateShaderResourceView(COM_PTR_GET(ImageResources[0]), nullptr, CDH); CDH.ptr += Device->GetDescriptorHandleIncrementSize(DH->GetDesc().Type);
#endif
			}
		}
	}
	
	virtual void CreateShaderBlobs() override {
		ShaderBlobs.resize(5 + 2);
		const auto ShaderPath = GetBasePath();
		//!< パス0 : シェーダブロブ
		VERIFY_SUCCEEDED(D3DReadFileToBlob((ShaderPath + TEXT(".vs.cso")).data(), COM_PTR_PUT(ShaderBlobs[0])));
		VERIFY_SUCCEEDED(D3DReadFileToBlob((ShaderPath + TEXT(".ps.cso")).data(), COM_PTR_PUT(ShaderBlobs[1])));
		VERIFY_SUCCEEDED(D3DReadFileToBlob((ShaderPath + TEXT(".ds.cso")).data(), COM_PTR_PUT(ShaderBlobs[2])));
		VERIFY_SUCCEEDED(D3DReadFileToBlob((ShaderPath + TEXT(".hs.cso")).data(), COM_PTR_PUT(ShaderBlobs[3])));
		VERIFY_SUCCEEDED(D3DReadFileToBlob((ShaderPath + TEXT(".gs.cso")).data(), COM_PTR_PUT(ShaderBlobs[4])));
		//!< パス1 : シェーダブロブ
		VERIFY_SUCCEEDED(D3DReadFileToBlob((ShaderPath + TEXT("_1.vs.cso")).data(), COM_PTR_PUT(ShaderBlobs[5])));
		VERIFY_SUCCEEDED(D3DReadFileToBlob((ShaderPath + TEXT("_1.ps.cso")).data(), COM_PTR_PUT(ShaderBlobs[6])));
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
#ifdef USE_DEPTH
			TRUE, D3D12_DEPTH_WRITE_MASK_ALL, D3D12_COMPARISON_FUNC_LESS,
#else
			FALSE, D3D12_DEPTH_WRITE_MASK_ALL, D3D12_COMPARISON_FUNC_LESS,
#endif
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
		const std::array<D3D12_SHADER_BYTECODE, 2> SBCs_1 = {
			D3D12_SHADER_BYTECODE({ ShaderBlobs[5]->GetBufferPointer(), ShaderBlobs[5]->GetBufferSize() }),
			D3D12_SHADER_BYTECODE({ ShaderBlobs[6]->GetBufferPointer(), ShaderBlobs[6]->GetBufferSize() }),
		};
		const std::vector<D3D12_INPUT_ELEMENT_DESC> IEDs = {};
		const std::vector<DXGI_FORMAT> RTVs = { DXGI_FORMAT_R8G8B8A8_UNORM };
#ifdef USE_PIPELINE_SERIALIZE
		PipelineLibrarySerializer PLS(COM_PTR_GET(Device), GetBasePath() + TEXT(".plo"));
		//!< パス0 : パイプラインステート
		Threads.push_back(std::thread::thread(DX::CreatePipelineState, std::ref(PipelineStates[0]), COM_PTR_GET(Device), COM_PTR_GET(RootSignatures[0]), D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH, RD, DSD_0, SBCs_0[0], SBCs_0[1], SBCs_0[2], SBCs_0[3], SBCs_0[4], IEDs, RTVs, &PLS, TEXT("0")));
		//!< パス1 : パイプラインステート
		Threads.push_back(std::thread::thread(DX::CreatePipelineState, std::ref(PipelineStates[1]), COM_PTR_GET(Device), COM_PTR_GET(RootSignatures[1]), D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE, RD, DSD_1, SBCs_1[0], SBCs_1[1], NullShaderBC, NullShaderBC, NullShaderBC, IEDs, RTVs, &PLS, TEXT("1")));
#else
		Threads.push_back(std::thread::thread(DX::CreatePipelineState, std::ref(PipelineStates[0]), COM_PTR_GET(Device), COM_PTR_GET(RootSignatures[0]), D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH, RD, DSD_0, SBCs_0[0], SBCs_0[1], SBCs_0[2], SBCs_0[3], SBCs_0[4], IEDs, RTVs, nullptr, nullptr));
		Threads.push_back(std::thread::thread(DX::CreatePipelineState, std::ref(PipelineStates[1]), COM_PTR_GET(Device), COM_PTR_GET(RootSignatures[1]), D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE, RD, DSD_1, SBCs_1[0], SBCs_1[1],, NullShaderBC, NullShaderBC, NullShaderBC, IEDs, RTVs, nullptr, nullptr));
#endif	
		for (auto& i : Threads) { i.join(); }
	}
	virtual void PopulateCommandList(const size_t i) override;
};
#pragma endregion