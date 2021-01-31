#pragma once

#include "resource.h"

#pragma region Code
#include "../VKExt.h"

class RayTracingVK : public VKExt
{
private:
	using Super = VKExt;
public:
	RayTracingVK() : Super() {}
	virtual ~RayTracingVK() {}
};
#pragma endregion
