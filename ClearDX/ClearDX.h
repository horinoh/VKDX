#pragma once

#include "resource.h"

#pragma region Code
#include "../DXExt.h"

class ClearDX : public DXExt
{
private:
	using Super = DXExt;
public:
	ClearDX() : Super() {}
	virtual ~ClearDX() {}

	virtual void PopulateCommandList(const size_t i) override {
		const auto CL = COM_PTR_GET(DirectCommandLists[i]);
		const auto CA = COM_PTR_GET(DirectCommandAllocators[0]);
		VERIFY_SUCCEEDED(CL->Reset(CA, nullptr)); {
			CL->RSSetViewports(static_cast<UINT>(size(Viewports)), data(Viewports));
			CL->RSSetScissorRects(static_cast<UINT>(size(ScissorRects)), data(ScissorRects));
			const auto SCR = COM_PTR_GET(SwapChainResources[i]);
			ResourceBarrier(CL, SCR, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
			{
				constexpr std::array<D3D12_RECT, 0> Rects = {};
				CL->ClearRenderTargetView(SwapChainCPUHandles[i], DirectX::Colors::SkyBlue, static_cast<UINT>(size(Rects)), data(Rects));
			} 
			ResourceBarrier(CL, SCR, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
		} VERIFY_SUCCEEDED(CL->Close());
	}
};
#pragma endregion