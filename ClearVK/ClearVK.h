#pragma once

#include "resource.h"

#pragma region Code
#include "../VK.h"

class ClearVK : public VK
{
private:
	using Super = VK;
public:
	ClearVK() : VK() {}
	virtual ~ClearVK() {}
};
#pragma endregion