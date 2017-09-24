#pragma once

#include "resource.h"

#pragma region Code
#include "../DXExt.h"

class ClearDX : public DXExt
{
private:
	using Super = DXExt;
public:
	ClearDX() : Super() {}
	virtual ~ClearDX() {}

protected:
	virtual void CreatePipelineState() override {}
};
#pragma endregion