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
		PopulateCommandList_Clear(i, DirectX::Colors::SkyBlue);
	}
};
#pragma endregion