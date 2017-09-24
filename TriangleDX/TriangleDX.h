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
	//virtual void CreateInputLayout() override { CreateInputLayout_PositionColor(); }
	virtual void CreateVertexBuffer() override;
	virtual void CreateIndexBuffer() override;
#ifdef USE_DRAW_INDIRECT
	virtual void CreateIndirectBuffer() override { CreateIndirectBuffer_Indexed(); }
#endif

	virtual void CreateShader(std::vector<Microsoft::WRL::ComPtr<ID3DBlob>>& ShaderBlobs, std::array<D3D12_SHADER_BYTECODE, 5>& ShaderBytecodes) const override {
		CreateShader_VsPs(ShaderBlobs, ShaderBytecodes);
	}
	virtual void CreateInputLayout(std::vector<D3D12_INPUT_ELEMENT_DESC>& InputElementDescs, const UINT InputSlot = 0) const override {
		CreateInputLayoutT<Vertex_PositionColor>(InputElementDescs, InputSlot);
	}

#if 0
	virtual void CreateRootSignature() override { Super::CreateRootSignature_1CBV(D3D12_SHADER_VISIBILITY_PIXEL); }
	virtual void CreateConstantBuffer() override { Super::CreateConstantBuffer<DirectX::XMFLOAT4>(DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f)); }
#endif

	//virtual void PopulateCommandList(ID3D12GraphicsCommandList* CommandList, ID3D12Resource* SwapChainResource, const D3D12_CPU_DESCRIPTOR_HANDLE& DescriptorHandle) override;
	virtual void PopulateCommandList(ID3D12GraphicsCommandList* CommandList, ID3D12Resource* SwapChainResource, const D3D12_CPU_DESCRIPTOR_HANDLE& DescriptorHandle, const DirectX::XMVECTORF32& Color) override;

private:
	using Vertex = struct Vertex { DirectX::XMFLOAT3 Positon; DirectX::XMFLOAT4 Color; };
};
#pragma endregion