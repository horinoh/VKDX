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

#ifdef USE_WINRT
	virtual void CreateShader(std::vector<winrt::com_ptr<ID3DBlob>>& ShaderBlobs) const override {
#elif defined(USE_WRL)
	virtual void CreateShader(std::vector<Microsoft::WRL::ComPtr<ID3DBlob>>& ShaderBlobs) const override {
#endif
		CreateShader_VsPs(ShaderBlobs);
		Super::CreateShader(ShaderBlobs);
	}
	
#ifdef USE_WINRT
	virtual void SerializeRootSignature(winrt::com_ptr<ID3DBlob>& RSBlob) override;
#elif defined(USE_WRL)
	virtual void SerializeRootSignature(Microsoft::WRL::ComPtr<ID3DBlob>& RSBlob) override;
#endif

	virtual void CreateInputLayout(std::vector<D3D12_INPUT_ELEMENT_DESC>& InputElementDescs) const override {
		CreateInputLayoutSlot<Vertex_PositionColor>(InputElementDescs, 0);
	}

	virtual void PopulateCommandList(const size_t i) override;
};
#pragma endregion