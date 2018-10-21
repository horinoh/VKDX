#pragma once

#include "resource.h"

#pragma region Code
#include "../DXExt.h"

#define USE_DRAW_INDIRECT

class FullscreenDX : public DXExt
{
private:
	using Super = DXExt;
public:
	FullscreenDX() : Super() {}
	virtual ~FullscreenDX() {}

protected:
#ifdef USE_DRAW_INDIRECT
	virtual void CreateIndirectBuffer() override { CreateIndirectBuffer_Draw(4); }
#endif

	virtual void CreateShader(std::vector<Microsoft::WRL::ComPtr<ID3DBlob>>& ShaderBlobs) const override {
		CreateShader_VsPs(ShaderBlobs);
		Super::CreateShader(ShaderBlobs);
	}
	virtual void PopulateCommandList(const size_t i) override;
};
#pragma endregion