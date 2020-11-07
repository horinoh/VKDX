#pragma once

#include "DDSTextureLoader.h"

#include "DXExt.h"

class DXImage : public DXExt
{
private:
	using Super = DXExt;

protected:
	virtual void LoadImage(ID3D12Resource** Resource, const D3D12_RESOURCE_STATES ResourceState, std::wstring_view Path) {
		assert(std::filesystem::exists(Path) && "");
		assert(Path.ends_with(TEXT(".dds")) && "");
		LoadImage_DDS(Resource, ResourceState, Path);
	}
	void LoadImage_DDS(ID3D12Resource** Resource, const D3D12_RESOURCE_STATES ResourceState, std::wstring_view Path);

	virtual void CreateColorImage(ID3D12Resource** Resource, const D3D12_RESOURCE_STATES RS, const UINT32 Color = 0xffffffff);
};