#pragma once

#include "resource.h"

#pragma region Code
#include "../DXExt.h"

class TriangleDX : public DXExt
{
private:
	using Super = DXExt;
public:
	TriangleDX() : Super() {}
	virtual ~TriangleDX() {}

protected:
	virtual void CreateVertexBuffer() override;
	virtual void CreateIndexBuffer() override;
	virtual void CreateIndirectBuffer() override { CreateIndirectBuffer_DrawIndexed(IndexCount); }

	virtual void CreateShader(std::vector<Microsoft::WRL::ComPtr<ID3DBlob>>& ShaderBlobs) const override {
		CreateShader_VsPs(ShaderBlobs);
		Super::CreateShader(ShaderBlobs);
	}
	virtual void CreateInputLayout(std::vector<D3D12_INPUT_ELEMENT_DESC>& InputElementDescs) const override {
		CreateInputLayoutSlot<Vertex_PositionColor>(InputElementDescs, 0);
	}

	virtual void PopulateCommandList(const size_t i) override;
};
#pragma endregion