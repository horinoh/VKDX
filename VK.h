#pragma once

#define VK_USE_PLATFORM_WIN32_KHR

#include "vulkan/vulkan.h"

#include "Win.h"

class VK : public Win
{
public:
	VK();
	virtual ~VK();
};