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
#ifdef USE_WINRT
	virtual void SerializeRootSignature(winrt::com_ptr<ID3DBlob>& RSBlob) override;
#elif defined(USE_WRL)
	virtual void SerializeRootSignature(Microsoft::WRL::ComPtr<ID3DBlob>& RSBlob) override;
#endif
	virtual void CreateShaderBlob() override { CreateShaderBlob_VsPs(); }
	virtual void CreatePipelineState() override { CreatePipelineState_VsPs(); }
	virtual void PopulateCommandList(const size_t i) override;
};
#pragma endregion