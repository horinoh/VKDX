#pragma once

#include "resource.h"

#include "../DX.h"

class TriangleDX : public DX
{
private:
	using Super = DX;
public:
	TriangleDX() : DX() {}
	virtual ~TriangleDX() {}
};