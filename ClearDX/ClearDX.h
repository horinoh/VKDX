#pragma once

#include "resource.h"

#pragma region Code
#include "../DX.h"

class ClearDX : public DX
{
private:
	using Super = DX;
public:
	ClearDX() : DX() {}
	virtual ~ClearDX() {}
};
#pragma endregion