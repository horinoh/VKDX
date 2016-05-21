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
	virtual void CreateVertexBuffer() override;
	virtual void CreateIndexBuffer() override;

private:
	using Vertex = std::tuple<DirectX::XMFLOAT3, DirectX::XMFLOAT4>;
};
#pragma endregion