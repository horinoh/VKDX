#pragma once

#include "resource.h"

#include "../VK.h"

class TriangleVK : public VK
{
private:
	using Super = VK;
public:
	TriangleVK() : VK() {}
	virtual ~TriangleVK() {}
};