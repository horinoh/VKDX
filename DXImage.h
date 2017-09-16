#pragma once

#include "DDSTextureLoader.h"

#include "DXExt.h"

class DXImage : public DXExt
{
private:
	using Super = DXExt;

protected:
	virtual void LoadImage(ID3D12Resource** Resource, const std::wstring& Path, const D3D12_RESOURCE_STATES ResourceState = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE) override;
	void LoadImage_DDS(ID3D12Resource** Resource, const std::wstring& Path, const D3D12_RESOURCE_STATES ResourceState);
};