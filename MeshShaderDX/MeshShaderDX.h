#pragma once

#include "resource.h"

#pragma region Code
#include "../DXExt.h"

class MeshShaderDX : public DXExt
{
private:
	using Super = DXExt;
public:
	MeshShaderDX() : Super() {}
	virtual ~MeshShaderDX() {}
};
#pragma endregion