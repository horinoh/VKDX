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
	
	//!< #DX_TODO 出力テクスチャ用のUAVを用意しないとならない

	virtual void CreatePipelineState() override { Super::CreatePipelineState_Compute(); }

	virtual void CreateIndirectBuffer() override { CreateIndirectBuffer_Dispatch(32, 1, 1); }
	virtual void CreateShader(std::vector<Microsoft::WRL::ComPtr<ID3DBlob>>& ShaderBlobs, std::vector<D3D12_SHADER_BYTECODE>& ShaderBytecodes) const override {
		CreateShader_Cs(ShaderBlobs, ShaderBytecodes);
	}

	virtual void PopulateCommandList(ID3D12GraphicsCommandList* CommandList, ID3D12Resource* SwapChainResource, const D3D12_CPU_DESCRIPTOR_HANDLE& DescriptorHandle);

	virtual void Draw() override { Dispatch(); }

private:
};
#pragma endregion
