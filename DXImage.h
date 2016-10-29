#pragma once

#include "DDSTextureLoader.h"

#include "DXExt.h"

class DXImage : public DXExt
{
private:
	using Super = DXExt;

protected:
	virtual void LoadImage(ID3D12Resource** Resource, ID3D12DescriptorHeap* DescriptorHeap, const std::wstring& Path) override { LoadImage_DDS(Resource, DescriptorHeap, Path); }
	void LoadImage_DDS(ID3D12Resource** Resource, ID3D12DescriptorHeap* DescriptorHeap, const std::wstring& Path);
};