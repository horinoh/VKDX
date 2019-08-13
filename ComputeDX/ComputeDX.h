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
#ifdef USE_WINRT
		winrt::com_ptr<ID3DBlob> Blob;
#elif defined(USE_WRL)
		Microsoft::WRL::ComPtr<ID3DBlob> Blob;
#endif
#ifdef ROOTSIGNATRUE_FROM_SHADER
		GetRootSignaturePartFromShader(Blob);
#else
		const std::array<D3D12_DESCRIPTOR_RANGE, 1> DRs = {
			{ D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND }
		};
		DX::SerializeRootSignature(Blob, {
				{ D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE, { static_cast<UINT>(DRs.size()), DRs.data() }, D3D12_SHADER_VISIBILITY_PIXEL }
			}, {}, D3D12_ROOT_SIGNATURE_FLAG_NONE);
#endif
		DX::CreateRootSignature(RootSignature, Blob);
		LOG_OK();
	}
	virtual void CreateDescriptorHeap() override {
		DX::CreateDescriptorHeap(UnorderedAccessTextureDescriptorHeap, 
			{ D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 1, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE, 0 }
		);
		LOG_OK();
	}
	virtual void CreateDescriptorView() override {
		DX::CreateUnorderedAccessView(UnorderedAccessTextureResource, UnorderedAccessTextureDescriptorHeap);
		LOG_OK();
	}
#ifdef USE_WINRT
	virtual void CreateShader(std::vector<winrt::com_ptr<ID3DBlob>>& ShaderBlobs) const override {
#elif defined(USE_WRL)
	virtual void CreateShader(std::vector<Microsoft::WRL::ComPtr<ID3DBlob>> & ShaderBlobs) const override {
#endif
		//CreateShader_Cs(ShaderBlobs);
		Super::CreateShader(ShaderBlobs);
	}

	virtual void CreateTexture() override {
		//!< #DX_TODO
		//CreateDefaultResource();
	}

	virtual void CreateShaderBlob() override { CreateShaderBlob_Cs(); }
	virtual void CreatePipelineState() override { CreatePipelineState_Cs(); }
	virtual void PopulateCommandList(const size_t i) override;

	virtual void Draw() override { Dispatch(); }

private:
};
#pragma endregion
