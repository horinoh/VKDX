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

	virtual void CreatePipelineState() override { Super::CreatePipelineState_Compute(); }
	virtual void CreateIndirectBuffer() override { CreateIndirectBuffer_Dispatch(32, 1, 1); }

	//!< #DX_TODO 出力テクスチャ用のUAVを用意しないとならない
	virtual void CreateRootParameters(std::vector<D3D12_ROOT_PARAMETER>& RootParameters, const std::vector<D3D12_DESCRIPTOR_RANGE>& DescriptorRanges) const override {
		CreateRootParameters_1UAV(RootParameters, DescriptorRanges, D3D12_SHADER_VISIBILITY_PIXEL/*#DX_TODO COMPUTE無い*/);
	}
	virtual void CreateDescriptorRanges(std::vector<D3D12_DESCRIPTOR_RANGE>& DescriptorRanges) const override {
		CreateDescriptorRanges_1UAV(DescriptorRanges);
	}
	virtual void CreateDescriptorHeap() override {
		CreateDescriptorHeap_1UAV();
	}

	virtual void CreateShader(std::vector<Microsoft::WRL::ComPtr<ID3DBlob>>& ShaderBlobs) const override {
		CreateShader_Cs(ShaderBlobs);
		Super::CreateShader(ShaderBlobs);
	}

	virtual void CreateTexture() override {
		//!< #DX_TODO
		//CreateDefaultResource();
	}

	virtual void PopulateCommandList(const size_t i) override;

	virtual void Draw() override { Dispatch(); }

private:
};
#pragma endregion
