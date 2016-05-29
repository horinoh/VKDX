#pragma once

#include "resource.h"

#pragma region Code
#include "../VK.h"

class ParametricSurfaceVK : public VK
{
private:
	using Super = VK;
public:
	ParametricSurfaceVK() : VK() {}
	virtual ~ParametricSurfaceVK() {}

protected:
	virtual void CreateShader() override;
	//virtual void CreateVertexInput() override;

private:
};
#pragma endregion