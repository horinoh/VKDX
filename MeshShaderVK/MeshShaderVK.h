#pragma once

#include "resource.h"

#pragma region Code
#include "../VKExt.h"

class MeshShaderVK : public VKExt
{
private:
	using Super = VKExt;
public:
	MeshShaderVK() : Super() {}
	virtual ~MeshShaderVK() {}

	virtual void CreatePipelineLayout() override;
	virtual void CreatePipeline() override;
};
#pragma endregion