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
	virtual void CreateShader() override { CreateShader_VsPsTesTcsGs(); }
	virtual void CreateVertexInput() override { CreateVertexInput_Position(); }
	virtual void CreateGraphicsPipeline() override { CreateGraphicsPipeline_VsPsTesTcsGs(); }

private:
};
#pragma endregion