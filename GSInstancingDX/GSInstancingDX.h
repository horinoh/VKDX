#pragma once

#include "resource.h"

#pragma region Code
#include "../DXExt.h"

class GSInstancingDX : public DXExt
{
private:
	using Super = DXExt;
public:
	GSInstancingDX() : Super() {}
	virtual ~GSInstancingDX() {}

protected:
	virtual void CreateIndirectBuffer() override { CreateIndirectBuffer_DrawIndexed(1, 1); }
	virtual void CreateShaderBlobs() override { CreateShaderBlob_VsPsDsHsGs(); }
	virtual void CreatePipelineStates() override { CreatePipelineState_VsPsDsHsGs(D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH, FALSE); }

	virtual void CreateViewport(const FLOAT Width, const FLOAT Height, const FLOAT MinDepth = 0.0f, const FLOAT MaxDepth = 1.0f) override {
		D3D12_FEATURE_DATA_D3D12_OPTIONS3 FDO3;
		VERIFY_SUCCEEDED(Device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS3, reinterpret_cast<void*>(&FDO3), sizeof(FDO3)));
		assert(D3D12_VIEW_INSTANCING_TIER_1 < FDO3.ViewInstancingTier && "");

		const auto W = Width * 0.5f, H = Height * 0.5f;
		Viewports = {
			D3D12_VIEWPORT({.TopLeftX = 0.0f, .TopLeftY = 0.0f, .Width = W, .Height = H, .MinDepth = MinDepth, .MaxDepth = MaxDepth }),
			D3D12_VIEWPORT({.TopLeftX = W, .TopLeftY = 0.0f, .Width = W, .Height = H, .MinDepth = MinDepth, .MaxDepth = MaxDepth }),
			D3D12_VIEWPORT({.TopLeftX = 0.0f, .TopLeftY = H, .Width = W, .Height = H, .MinDepth = MinDepth, .MaxDepth = MaxDepth }),
			D3D12_VIEWPORT({.TopLeftX = W, .TopLeftY = H, .Width = W, .Height = H, .MinDepth = MinDepth, .MaxDepth = MaxDepth }),
		};
		//!< left, top, right, bottom‚ÅŽw’è (offset, extent‚ÅŽw’è‚ÌVK‚Æ‚ÍˆÙ‚È‚é‚Ì‚Å’ˆÓ)
		ScissorRects = {
			D3D12_RECT({.left = 0, .top = 0, .right = static_cast<LONG>(W), .bottom = static_cast<LONG>(H) }),
			D3D12_RECT({.left = static_cast<LONG>(W), .top = 0, .right = static_cast<LONG>(Width), .bottom = static_cast<LONG>(H) }),
			D3D12_RECT({.left = 0, .top = static_cast<LONG>(H), .right = static_cast<LONG>(W), .bottom = static_cast<LONG>(Height) }),
			D3D12_RECT({.left = static_cast<LONG>(W), .top = static_cast<LONG>(H), .right = static_cast<LONG>(Width), .bottom = static_cast<LONG>(Height) }),
		};
		LOG_OK();
	}

	virtual void PopulateCommandList(const size_t i) override;
};
#pragma endregion