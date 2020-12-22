#pragma once

#include "resource.h"

#pragma region Code
#include "../DXExt.h"
#include "../Leap.h"

class LeapDX : public DXExt, public Leap
{
private:
	using Super = DXExt;
public:
	LeapDX() : Super() {}
	virtual ~LeapDX() {}
};
#pragma endregion