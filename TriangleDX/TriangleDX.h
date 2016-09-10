#pragma once

#include "resource.h"

#pragma region Code
#include "../DXExt.h"

class TriangleDX : public DXExt
{
private:
	using Super = DXExt;
public:
	TriangleDX() : DXExt() {}
	virtual ~TriangleDX() {}

protected:
	virtual void CreateInputLayout() override { CreateInputLayout_PositionColor(); }
	virtual void CreateVertexBuffer(ID3D12CommandAllocator* CommandAllocator, ID3D12GraphicsCommandList* CommandList) override;
	virtual void CreateIndexBuffer(ID3D12CommandAllocator* CommandAllocator, ID3D12GraphicsCommandList* CommandList) override;
	virtual void CreateGraphicsPipelineState() override { CreateGraphicsPipelineState_VsPs(); }

#if 0
	virtual void CreateRootSignature() override { Super::CreateRootSignature_1ConstantBuffer(D3D12_SHADER_VISIBILITY_PIXEL); }
	virtual void CreateConstantBuffer() override { Super::CreateConstantBuffer<DirectX::XMFLOAT4>(DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f)); }
#endif

	virtual void PopulateCommandList(ID3D12GraphicsCommandList* CommandList, ID3D12CommandAllocator* CommandAllocator) override;

private:
	using Vertex = struct Vertex { DirectX::XMFLOAT3 Positon; DirectX::XMFLOAT4 Color; };
};
#pragma endregion