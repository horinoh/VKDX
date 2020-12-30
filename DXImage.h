#pragma once

#include "DDSTextureLoader.h"

#include "DXExt.h"

class DXImage : public DXExt
{
private:
	using Super = DXExt;

protected:
	virtual void LoadImage(ID3D12Resource** Resource, const D3D12_RESOURCE_STATES RS, std::wstring_view Path) {
		assert(std::filesystem::exists(Path) && "");
		assert(Path.ends_with(TEXT(".dds")) && "");
		LoadImage_DDS(Resource, RS, Path);
	}
	void LoadImage_DDS(ID3D12Resource** Resource, const D3D12_RESOURCE_STATES RS, std::wstring_view Path);
};