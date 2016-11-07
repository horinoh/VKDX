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
	virtual void CreateVertexBuffer(ID3D12CommandAllocator* CommandAllocator, ID3D12GraphicsCommandList* CommandList) override;
	virtual void CreateIndexBuffer(ID3D12CommandAllocator* CommandAllocator, ID3D12GraphicsCommandList* CommandList) override;
#ifdef USE_DRAW_INDIRECT
	virtual void CreateIndirectBuffer(ID3D12CommandAllocator* CommandAllocator, ID3D12GraphicsCommandList* CommandList) override { CreateIndirectBuffer_IndexedIndirect(CommandAllocator, CommandList); }
#endif

	virtual void CreateShader(std::vector<Microsoft::WRL::ComPtr<ID3DBlob>>& ShaderBlobs, std::array<D3D12_SHADER_BYTECODE, 5>& ShaderBytecodes) const override {
		CreateShader_VsPs(ShaderBlobs, ShaderBytecodes);
	}
	virtual void CreateInputLayout(std::vector<D3D12_INPUT_ELEMENT_DESC>& InputElementDescs, const UINT InputSlot = 0) const override {
		CreateInputLayoutT<Vertex_PositionColor>(InputElementDescs, InputSlot);
	}

	virtual void CreatePipelineState() override { CreateGraphicsPipelineState(); }

#if 0
	virtual void CreateRootSignature() override { Super::CreateRootSignature_1ConstantBuffer(D3D12_SHADER_VISIBILITY_PIXEL); }
	virtual void CreateConstantBuffer() override { Super::CreateConstantBuffer<DirectX::XMFLOAT4>(DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f)); }
#endif

	virtual void PopulateCommandList(ID3D12GraphicsCommandList* CommandList, ID3D12CommandAllocator* CommandAllocator) override;

private:
	using Vertex = struct Vertex { DirectX::XMFLOAT3 Positon; DirectX::XMFLOAT4 Color; };
};
#pragma endregion