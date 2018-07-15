#pragma once

#include "resource.h"

#pragma region Code
#include "../DXExt.h"

#define USE_DRAW_INDIRECT

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
#ifdef USE_DRAW_INDIRECT
	virtual void CreateIndirectBuffer() override { CreateIndirectBuffer_Indexed(IndexCount); }
#endif

	virtual void CreateShader(std::vector<Microsoft::WRL::ComPtr<ID3DBlob>>& ShaderBlobs) const override {
		CreateShader_VsPs(ShaderBlobs);
		Super::CreateShader(ShaderBlobs);
	}
	virtual void CreateInputLayout(std::vector<D3D12_INPUT_ELEMENT_DESC>& InputElementDescs, const UINT InputSlot = 0) const override {
		CreateInputLayoutT<Vertex_PositionColor>(InputElementDescs, InputSlot);
	}

	virtual void PopulateCommandList(const size_t i) override;
};
#pragma endregion