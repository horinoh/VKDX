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
#ifdef USE_GBUFFER_VISUALIZE
	virtual void CreateViewport(const FLOAT Width, const FLOAT Height, const FLOAT MinDepth = 0.0f, const FLOAT MaxDepth = 1.0f) override {
		D3D12_FEATURE_DATA_D3D12_OPTIONS3 FDO3;
		VERIFY_SUCCEEDED(Device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS3, reinterpret_cast<void*>(&FDO3), sizeof(FDO3)));
		assert(D3D12_VIEW_INSTANCING_TIER_1 < FDO3.ViewInstancingTier && "");

		const auto W = Width * 0.5f, H = Height * 0.5f;
		Viewports = {
			//!< �S��ʗp
			{ 0.0f, 0.0f, Width, Height, MinDepth, MaxDepth },
			//!< ������ʗp
			{ 0.0f, 0.0f, W, H, MinDepth, MaxDepth },
			{ W, 0.0f, W, H, MinDepth, MaxDepth },
			{ 0.0f, H, W, H, MinDepth, MaxDepth },
			{ W, H, W, H, MinDepth, MaxDepth },
		};
		//!< left, top, right, bottom�Ŏw�� (offset, extent�Ŏw���VK�Ƃ͈قȂ�̂Œ���)
		ScissorRects = {
			//!< �S��ʗp
			{ 0, 0, static_cast<LONG>(Width), static_cast<LONG>(Height) },
			//!< ������ʗp
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
		//!< �p�X1 : �o���h���R�}���h���X�g
		DXGI_SWAP_CHAIN_DESC1 SCD;
		SwapChain->GetDesc1(&SCD);
		for (UINT i = 0; i < SCD.BufferCount; ++i) {
			BundleGraphicsCommandLists.push_back(COM_PTR<ID3D12GraphicsCommandList>());
			VERIFY_SUCCEEDED(Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_BUNDLE, COM_PTR_GET(BundleCommandAllocators[0]), nullptr, COM_PTR_UUIDOF_PUTVOID(BundleGraphicsCommandLists.back())));
			VERIFY_SUCCEEDED(BundleGraphicsCommandLists.back()->Close());
		}
	}

	virtual void CreateIndirectBuffer() override {
		//!< �p�X0 : �C���_�C���N�g�o�b�t�@(���b�V���`��p)
		CreateIndirectBuffer_DrawIndexed(1, 1);
		//!< �p�X1 : �C���_�C���N�g�o�b�t�@(�t���X�N���[���`��p)
		CreateIndirectBuffer_Draw(4, 1);
	}
	virtual void CreateStaticSampler() override {
		//!< �p�X1 : �X�^�e�B�b�N�T���v��
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
		RootSignatures.resize(2);
		//!< �p�X0 : ���[�g�V�O�l�`��
		{
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
			VERIFY_SUCCEEDED(Device->CreateRootSignature(0, Blob->GetBufferPointer(), Blob->GetBufferSize(), COM_PTR_UUIDOF_PUTVOID(RootSignatures[0])));
		}
		//!< �p�X1 : ���[�g�V�O�l�`��
		{
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
			VERIFY_SUCCEEDED(Device->CreateRootSignature(0, Blob->GetBufferPointer(), Blob->GetBufferSize(), COM_PTR_UUIDOF_PUTVOID(RootSignatures[1])));
		}
		LOG_OK();
	}
	virtual void CreateTexture()
	{
		const D3D12_HEAP_PROPERTIES HP = {
			D3D12_HEAP_TYPE_DEFAULT,
			D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
			D3D12_MEMORY_POOL_UNKNOWN,
			0,// CreationNodeMask ... �}���`GPU�̏ꍇ�Ɏg�p(1�����g��Ȃ��ꍇ��0�ŗǂ�)
			0 // VisibleNodeMask ... �}���`GPU�̏ꍇ�Ɏg�p(1�����g��Ȃ��ꍇ��0�ŗǂ�)
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
				*static_cast<const FLOAT*>(DirectX::Colors::SkyBlue),
			};
			VERIFY_SUCCEEDED(Device->CreateCommittedResource(&HP, D3D12_HEAP_FLAG_NONE, &RD, D3D12_RESOURCE_STATE_RENDER_TARGET, &CV, COM_PTR_UUIDOF_PUTVOID(ImageResources.back())));
		}
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
			D3D12_CLEAR_VALUE CV = { RD.Format, };
			CV.DepthStencil = { 1.0f, 0 };
			VERIFY_SUCCEEDED(Device->CreateCommittedResource(&HP, D3D12_HEAP_FLAG_NONE, &RD, D3D12_RESOURCE_STATE_DEPTH_WRITE, &CV, COM_PTR_UUIDOF_PUTVOID(ImageResources.back())));
		}
	}

	virtual void CreateDescriptorHeap() override {
		//!< �p�X0 : �����_�[�^�[�Q�b�g
		{
			{
				RtvDescriptorHeaps.resize(1);
				const D3D12_DESCRIPTOR_HEAP_DESC DHD = { D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 1, D3D12_DESCRIPTOR_HEAP_FLAG_NONE, 0 };
				VERIFY_SUCCEEDED(Device->CreateDescriptorHeap(&DHD, COM_PTR_UUIDOF_PUTVOID(RtvDescriptorHeaps[0])));
			}
			{
				DsvDescriptorHeaps.resize(1);
				const D3D12_DESCRIPTOR_HEAP_DESC DHD = { D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 1, D3D12_DESCRIPTOR_HEAP_FLAG_NONE, 0 };
				VERIFY_SUCCEEDED(Device->CreateDescriptorHeap(&DHD, COM_PTR_UUIDOF_PUTVOID(DsvDescriptorHeaps[0])));
			}
		}
		//!< �p�X1 : �V�F�[�_���\�[�X
		{
			CbvSrvUavDescriptorHeaps.resize(1);
			const D3D12_DESCRIPTOR_HEAP_DESC DHD = { D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 2, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE, 0 };
			VERIFY_SUCCEEDED(Device->CreateDescriptorHeap(&DHD, COM_PTR_UUIDOF_PUTVOID(CbvSrvUavDescriptorHeaps[0])));
		}
	}
	virtual void CreateDescriptorView() override {
		//!< �p�X0 : �����_�[�^�[�Q�b�g�r���[
		{
			{
				const auto& DH = RtvDescriptorHeaps[0];
				auto CDH = DH->GetCPUDescriptorHandleForHeapStart();
				Device->CreateRenderTargetView(COM_PTR_GET(ImageResources[0]), nullptr, CDH); CDH.ptr += Device->GetDescriptorHandleIncrementSize(DH->GetDesc().Type);
			}
			{
				const auto& DH = DsvDescriptorHeaps[0];
				auto CDH = DH->GetCPUDescriptorHandleForHeapStart();
				Device->CreateDepthStencilView(COM_PTR_GET(ImageResources[1]), nullptr, CDH); CDH.ptr += Device->GetDescriptorHandleIncrementSize(DH->GetDesc().Type);
			}
		}
		//!< �p�X1 : �V�F�[�_���\�[�X�r���[
		{
			const auto& DH = CbvSrvUavDescriptorHeaps[0];
			auto CDH = DH->GetCPUDescriptorHandleForHeapStart();
			{
				Device->CreateShaderResourceView(COM_PTR_GET(ImageResources[0]), nullptr, CDH); CDH.ptr += Device->GetDescriptorHandleIncrementSize(DH->GetDesc().Type);
			}
			{
				const auto RD = ImageResources[1]->GetDesc(); assert(D3D12_RESOURCE_DIMENSION_TEXTURE2D == RD.Dimension);
				D3D12_SHADER_RESOURCE_VIEW_DESC SRVD = {
					DXGI_FORMAT_R24_UNORM_X8_TYPELESS, //!< DXGI_FORMAT_D24_UNORM_S8_UINT==RD.Format �̏ꍇ (��{�I�� D?? -> R?? �̂悤�ɓǂݑւ������̂ɂ���Ηǂ�����)
					D3D12_SRV_DIMENSION_TEXTURE2D,
					D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING,
				};
				SRVD.Texture2D = { 0, RD.MipLevels, 0, 0.0f };
				Device->CreateShaderResourceView(COM_PTR_GET(ImageResources[1]), &SRVD, CDH); CDH.ptr += Device->GetDescriptorHandleIncrementSize(DH->GetDesc().Type);
			}
		}
	}

	virtual void CreateShaderBlobs() override {
#ifdef USE_GBUFFER_VISUALIZE
		ShaderBlobs.resize(5 + 3);
#else
		ShaderBlobs.resize(5 + 2);
#endif
		const auto ShaderPath = GetBasePath();
		//!< �p�X0 : �V�F�[�_�u���u
		VERIFY_SUCCEEDED(D3DReadFileToBlob((ShaderPath + TEXT(".vs.cso")).data(), COM_PTR_PUT(ShaderBlobs[0])));
		VERIFY_SUCCEEDED(D3DReadFileToBlob((ShaderPath + TEXT(".ps.cso")).data(), COM_PTR_PUT(ShaderBlobs[1])));
		VERIFY_SUCCEEDED(D3DReadFileToBlob((ShaderPath + TEXT(".ds.cso")).data(), COM_PTR_PUT(ShaderBlobs[2])));
		VERIFY_SUCCEEDED(D3DReadFileToBlob((ShaderPath + TEXT(".hs.cso")).data(), COM_PTR_PUT(ShaderBlobs[3])));
		VERIFY_SUCCEEDED(D3DReadFileToBlob((ShaderPath + TEXT(".gs.cso")).data(), COM_PTR_PUT(ShaderBlobs[4])));
		//!< �p�X1 : �V�F�[�_�u���u
		VERIFY_SUCCEEDED(D3DReadFileToBlob((ShaderPath + TEXT("_1.vs.cso")).data(), COM_PTR_PUT(ShaderBlobs[5])));
#ifdef USE_GBUFFER_VISUALIZE
		VERIFY_SUCCEEDED(D3DReadFileToBlob((ShaderPath + TEXT("_gb_1.gs.cso")).data(), COM_PTR_PUT(ShaderBlobs[7])));
		VERIFY_SUCCEEDED(D3DReadFileToBlob((ShaderPath + TEXT("_gb_1.ps.cso")).data(), COM_PTR_PUT(ShaderBlobs[6])));
#else
		VERIFY_SUCCEEDED(D3DReadFileToBlob((ShaderPath + TEXT("_1.ps.cso")).data(), COM_PTR_PUT(ShaderBlobs[6])));
#endif
	}
	virtual void CreatePipelineStates() override {
		PipelineStates.resize(2);
		std::vector<std::thread> Threads;
		const D3D12_DEPTH_STENCILOP_DESC DSOD = { D3D12_STENCIL_OP_KEEP, D3D12_STENCIL_OP_KEEP, D3D12_STENCIL_OP_KEEP, D3D12_COMPARISON_FUNC_ALWAYS };
		const D3D12_DEPTH_STENCIL_DESC DSD = {
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
#ifdef USE_PIPELINE_SERIALIZE
		PipelineLibrarySerializer PLS(COM_PTR_GET(Device), GetBasePath() + TEXT(".plo"));
		//!< �p�X0 : �p�C�v���C���X�e�[�g
		Threads.push_back(std::thread::thread(DX::CreatePipelineState, std::ref(PipelineStates[0]), COM_PTR_GET(Device), COM_PTR_GET(RootSignatures[0]), D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH, DSD, SBCs_0[0], SBCs_0[1], SBCs_0[2], SBCs_0[3], SBCs_0[4], IEDs, &PLS, TEXT("0")));
		//!< �p�X1 : �p�C�v���C���X�e�[�g
#ifdef USE_GBUFFER_VISUALIZE
		Threads.push_back(std::thread::thread(DX::CreatePipelineState, std::ref(PipelineStates[1]), COM_PTR_GET(Device), COM_PTR_GET(RootSignatures[1]), D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE, DSD, SBCs_1[0], SBCs_1[1], NullShaderBC, NullShaderBC, SBCs_1[2], IEDs, &PLS, TEXT("1")));
#else
		Threads.push_back(std::thread::thread(DX::CreatePipelineState, std::ref(PipelineStates[1]), COM_PTR_GET(Device), COM_PTR_GET(RootSignatures[1]), D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE, DSD, SBCs_1[0], SBCs_1[1], NullShaderBC, NullShaderBC, NullShaderBC, IEDs, &PLS, TEXT("1")));
#endif
#else
		Threads.push_back(std::thread::thread(DX::CreatePipelineState, std::ref(PipelineStates[0]), COM_PTR_GET(Device), COM_PTR_GET(RootSignatures[0]), D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH, DSD, SBCs_0[0], SBCs_0[1], SBCs_0[2], SBCs_0[3], SBCs_0[4], IEDs, nullptr, nullptr));
#ifdef USE_GBUFFER_VISUALIZE
		Threads.push_back(std::thread::thread(DX::CreatePipelineState, std::ref(PipelineStates[1]), COM_PTR_GET(Device), COM_PTR_GET(RootSignatures[1]), D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE, DSD, SBCs_1[0], SBCs_1[1], NullShaderBC, NullShaderBC, SBCs_1[2], IEDs, nullptr, nullptr));
#else
		Threads.push_back(std::thread::thread(DX::CreatePipelineState, std::ref(PipelineStates[1]), COM_PTR_GET(Device), COM_PTR_GET(RootSignatures[1]), D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE, DSD, SBCs_1[0], SBCs_1[1], NullShaderBC, NullShaderBC, NullShaderBC, IEDs, nullptr, nullptr));
#endif
#endif	
		for (auto& i : Threads) { i.join(); }
	}
	virtual void PopulateCommandList(const size_t i) override;
};
#pragma endregion