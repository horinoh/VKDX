#pragma once

#include "resource.h"

#pragma region Code
#include "../DX.h"

class TriangleDX : public DX
{
private:
	using Super = DX;
public:
	TriangleDX() : DX() {}
	virtual ~TriangleDX() {}

protected:
	virtual void CreateShader() override;
	virtual void CreateInputLayout() override;
	virtual void CreateVertexBuffer(ID3D12CommandAllocator* CommandAllocator, ID3D12GraphicsCommandList* CommandList) override;
	virtual void CreateIndexBuffer(ID3D12CommandAllocator* CommandAllocator, ID3D12GraphicsCommandList* CommandList) override;
	virtual void CreateGraphicsPipelineState() override;

	virtual void PopulateCommandList(ID3D12GraphicsCommandList* GraphicsCommandList) override;

private:
	using Vertex = std::tuple<DirectX::XMFLOAT3, DirectX::XMFLOAT4>;
};
#pragma endregion