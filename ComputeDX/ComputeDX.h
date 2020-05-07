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
	
	//!< #DX_TODO コマンドリストを作成

	virtual void CreateIndirectBuffer() override { CreateIndirectBuffer_Dispatch(32, 1, 1); }

	virtual void CreateRootSignature() override {
		COM_PTR<ID3DBlob> Blob;
#ifdef USE_HLSL_ROOTSIGNATRUE
		GetRootSignaturePartFromShader(Blob, (GetBasePath() + TEXT(".rs.cso")).data());
#else
		const std::array<D3D12_DESCRIPTOR_RANGE, 1> DRs = {
			{ D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND }
		};
		DX::SerializeRootSignature(Blob, {
				{ D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE, { static_cast<UINT>(DRs.size()), DRs.data() }, D3D12_SHADER_VISIBILITY_PIXEL }
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

		//const auto RD = ImageResources[0]->GetDesc(); RD.Format; RD.MipLevels;
		D3D12_SHADER_RESOURCE_VIEW_DESC SRVD = {
				DXGI_FORMAT_R8G8B8A8_UNORM,
				D3D12_SRV_DIMENSION_TEXTURE2D,
				D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING,
		};
		SRVD.Texture2D = { 0, 1, 0, 0.0f };
		Device->CreateShaderResourceView(COM_PTR_GET(UnorderedAccessTextureResource), &SRVD, CDH); CDH.ptr += Device->GetDescriptorHandleIncrementSize(DH->GetDesc().Type);

		D3D12_UNORDERED_ACCESS_VIEW_DESC UAVD = {
			DXGI_FORMAT_R8G8B8A8_UNORM,
			D3D12_UAV_DIMENSION_TEXTURE2D
		};
		UAVD.Texture2D.MipSlice = 0;
		UAVD.Texture2D.PlaneSlice = 0;
		Device->CreateUnorderedAccessView(COM_PTR_GET(UnorderedAccessTextureResource), nullptr, &UAVD, CDH); CDH.ptr += Device->GetDescriptorHandleIncrementSize(DH->GetDesc().Type);
	}
	//virtual void CreateShader(std::vector<COM_PTR<ID3DBlob>>& SBs) const override {
	//	//CreateShader_Cs(ShaderBlobs);
	//	Super::CreateShader(SBs);
	//}

	virtual void CreateTexture() override {
		//!< #DX_TODO
		//CreateDefaultResource();
	}

	virtual void CreateShaderBlobs() override { CreateShaderBlob_Cs(); }
	virtual void CreatePipelineStates() override { PipelineStates.resize(1); CreatePipelineState_Cs(PipelineStates[0]); }
	virtual void PopulateCommandList(const size_t i) override;

	virtual void Draw() override { Dispatch(); }

private:
};
#pragma endregion
