#pragma once

#include "DDSTextureLoader.h"

#include "DXExt.h"

class DXImage : public DXExt
{
private:
	using Super = DXExt;

protected:
	virtual void LoadImageResource(const std::wstring& Path) override { LoadImageResource_DDS(Path); }

	void LoadImageResource_DDS(const std::wstring& Path);
};