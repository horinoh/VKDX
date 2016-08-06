#pragma once

#include "resource.h"

#pragma region Code
#include "../DXExt.h"

class ClearDX : public DXExt
{
private:
	using Super = DXExt;
public:
	ClearDX() : DXExt() {}
	virtual ~ClearDX() {}
};
#pragma endregion