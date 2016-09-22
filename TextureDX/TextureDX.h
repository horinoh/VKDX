#pragma once

#include "resource.h"

#pragma region Code
#include "../DXExt.h"

class TextureDX : public DXExt
{
private:
	using Super = DXExt;
public:
	TextureDX() : DXExt() {}
	virtual ~TextureDX() {}

protected:
	virtual void CreateShader(std::vector<Microsoft::WRL::ComPtr<ID3DBlob>>& ShaderBlobs, std::array<D3D12_SHADER_BYTECODE, 5>& ShaderBytecodes) const override {
		CreateShader_VsPs(ShaderBlobs, ShaderBytecodes);
	}
	virtual void CreatePipelineState() override { CreateGraphicsPipelineState(); }
	virtual void PopulateCommandList(ID3D12GraphicsCommandList* CommandList, ID3D12CommandAllocator* CommandAllocator) override;
};
#pragma endregion