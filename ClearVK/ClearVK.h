#pragma once

#include "resource.h"

#pragma region Code
#include "../VKExt.h"

class ClearVK : public VKExt
{
private:
	using Super = VKExt;
public:
	ClearVK() : Super() {}
	virtual ~ClearVK() {}
};
#pragma endregion