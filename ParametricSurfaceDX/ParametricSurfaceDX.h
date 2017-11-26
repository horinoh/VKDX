#pragma once

#include "resource.h"

#pragma region Code
#include "../DXExt.h"

class ParametricSurfaceDX : public DXExt
{
private:
	using Super = DXExt;
public:
	ParametricSurfaceDX() : Super() {}
	virtual ~ParametricSurfaceDX() {}

protected:
	virtual void CreateIndirectBuffer() override { CreateIndirectBuffer_Vertices(0); }
	virtual void CreateShader(std::vector<Microsoft::WRL::ComPtr<ID3DBlob>>& ShaderBlobs, std::array<D3D12_SHADER_BYTECODE, 5>& ShaderBytecodes) const override {
		CreateShader_VsPsDsHsGs(ShaderBlobs, ShaderBytecodes);
	}
	virtual void CreateInputLayout(std::vector<D3D12_INPUT_ELEMENT_DESC>& InputElementDescs, const UINT InputSlot = 0) const override {
		CreateInputLayoutT<Vertex_Position>(InputElementDescs, InputSlot);
	}

	virtual D3D_PRIMITIVE_TOPOLOGY GetPrimitiveTopology() const override { return GetPrimitiveTopology_1ControlPointPatchlist(); }
	virtual D3D12_PRIMITIVE_TOPOLOGY_TYPE GetPrimitiveTopologyType() const override { return GetPrimitiveTopologyType_Patch(); }

	virtual void PopulateCommandList(const size_t i) override;
};
#pragma endregion