#pragma once

#include "resource.h"

#pragma region Code
#include "../DXExt.h"

class ComputeDX : public DXExt
{
private:
	using Super = DXExt;
public:
	ComputeDX() : Super() {}
	virtual ~ComputeDX() {}

protected:
	
	//!< #DX_TODO �R�}���h���X�g���쐬

	virtual void CreateGeometry() override { 
		constexpr D3D12_DISPATCH_ARGUMENTS DA = { .ThreadGroupCountX = 32, .ThreadGroupCountY = 1, .ThreadGroupCountZ = 1 };
		IndirectBuffers.emplace_back().Create(COM_PTR_GET(Device), COM_PTR_GET(CommandAllocators[0]), COM_PTR_GET(GraphicsCommandLists[0]), COM_PTR_GET(CommandQueue), COM_PTR_GET(Fence), DA);
	}

	virtual void CreateRootSignature() override {
		COM_PTR<ID3DBlob> Blob;
#ifdef USE_HLSL_ROOTSIGNATRUE
		GetRootSignaturePartFromShader(Blob, data(GetBasePath() + TEXT(".rs.cso")));
#else
		const std::array<D3D12_DESCRIPTOR_RANGE, 1> DRs = {
			{ D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND }
		};
		DX::SerializeRootSignature(Blob, {
				{ D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE, { static_cast<UINT>(size(DRs)), data(DRs) }, D3D12_SHADER_VISIBILITY_PIXEL }
			}, {}, D3D12_ROOT_SIGNATURE_FLAG_NONE);
#endif
		RootSignatures.resize(1);
		VERIFY_SUCCEEDED(Device->CreateRootSignature(0, Blob->GetBufferPointer(), Blob->GetBufferSize(), COM_PTR_UUIDOF_PUTVOID(RootSignatures[0])));
		LOG_OK();
	}
	virtual void CreateDescriptorHeap() override {
		CbvSrvUavDescriptorHeaps.resize(1);
		const D3D12_DESCRIPTOR_HEAP_DESC DHD = { D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 2, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE, 0 };
		VERIFY_SUCCEEDED(Device->CreateDescriptorHeap(&DHD, COM_PTR_UUIDOF_PUTVOID(CbvSrvUavDescriptorHeaps[0])));
	}
	virtual void CreateDescriptorView() override {
		const auto& DH = CbvSrvUavDescriptorHeaps[0];
		auto CDH = DH->GetCPUDescriptorHandleForHeapStart();

#if 0
		Device->CreateShaderResourceView(COM_PTR_GET(ImageResources[0]), &ShaderResourceViewDescs[0], CDH); CDH.ptr += Device->GetDescriptorHandleIncrementSize(DH->GetDesc().Type);
		Device->CreateUnorderedAccessView(COM_PTR_GET(ImageResources[0]), nullptr, &ShaderResourceViewDescs[1], CDH); CDH.ptr += Device->GetDescriptorHandleIncrementSize(DH->GetDesc().Type);
#else
		//!< ���\�[�X�Ɠ����t�H�[�}�b�g�ƃf�B�����V�����ōŏ��̃~�b�v�}�b�v�ƃX���C�X���^�[�Q�b�g����悤�ȏꍇ�ɂ�nullptr���w��ł���
		Device->CreateShaderResourceView(COM_PTR_GET(ImageResources[0]), nullptr, CDH); CDH.ptr += Device->GetDescriptorHandleIncrementSize(DH->GetDesc().Type);
		Device->CreateUnorderedAccessView(COM_PTR_GET(ImageResources[0]), nullptr, nullptr, CDH); CDH.ptr += Device->GetDescriptorHandleIncrementSize(DH->GetDesc().Type);
#endif
	}
	//virtual void CreateShader(std::vector<COM_PTR<ID3DBlob>>& SBs) const override {
	//	//CreateShader_Cs(ShaderBlobs);
	//	Super::CreateShader(SBs);
	//}

	virtual void CreateTexture() override {
		//!< #DX_TODO
		//CreateUnorderedAccessTexture();
		
		//ShaderResourceViewDescs.push_back({ ImageResources.back()->GetDesc().Format, D3D12_SRV_DIMENSION_TEXTURE2D, D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING });
		//ShaderResourceViewDescs.back().Texture2D = { 0, ImageResources.back()->GetDesc().MipLevels, 0, 0.0f };

		//UnorderedAccessViewDescs.push_back({ /*ImageResources.back()->GetDesc().Format*/DXGI_FORMAT_R8G8B8A8_UNORM, D3D12_UAV_DIMENSION_TEXTURE2D });
		//UnorderedAccessViewDescs.back().Texture2D = { 0, 0 };
	}
	virtual void CreatePipelineState() override { 
		const auto ShaderPath = GetBasePath();
		std::vector<COM_PTR<ID3DBlob>> SBs;
		VERIFY_SUCCEEDED(D3DReadFileToBlob(data(ShaderPath + TEXT(".cs.cso")), COM_PTR_PUT(SBs.emplace_back())));

		CreatePipelineState_Cs(); 
	}
	virtual void PopulateCommandList(const size_t i) override;

	virtual void Draw() override { Dispatch(); }

private:
};
#pragma endregion
