#pragma once

#include "resource.h"

#pragma region Code
#include "../DXImage.h"

class TextureDX : public DXImage
{
private:
	using Super = DXImage;
public:
	TextureDX() : Super() {}
	virtual ~TextureDX() {}

protected:
	virtual void CreateShader(std::vector<Microsoft::WRL::ComPtr<ID3DBlob>>& ShaderBlobs, std::array<D3D12_SHADER_BYTECODE, 5>& ShaderBytecodes) const override {
		CreateShader_VsPs(ShaderBlobs, ShaderBytecodes);
	}
	virtual void CreatePipelineState() override { CreateGraphicsPipelineState(); LoadImage(L"UV.dds"); }
	virtual void PopulateCommandList(ID3D12GraphicsCommandList* CommandList, ID3D12CommandAllocator* CommandAllocator) override;
};
#pragma endregion