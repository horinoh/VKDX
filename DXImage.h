#pragma once

#include "DDSTextureLoader.h"

#include "DXExt.h"

class DXImage : public DXExt
{
private:
	using Super = DXExt;

protected:
	void LoadDDS(const std::wstring& Path);
	virtual void LoadImageResource(const std::wstring& Path) override { LoadDDS(Path); }
};