#pragma once

#include "resource.h"

#pragma region Code
#include "../VK.h"

class ClearVK : public VKExt
{
private:
	using Super = VKExt;
public:
	ClearVK() : VKExt() {}
	virtual ~ClearVK() {}
};
#pragma endregion