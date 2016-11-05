#pragma once

#include "resource.h"

#pragma region Code
#include "../DXImage.h"

class TextureDX : public DXImage
{
private:
	using Super = DXImage;
public:
	TextureDX() : Super() {}
	virtual ~TextureDX() {}

protected:
	virtual void CreateDescriptorRanges(std::vector<D3D12_DESCRIPTOR_RANGE>& DescriptorRanges) const override { 
		CreateDescriptorRanges_1SRV(DescriptorRanges); 
	}
	virtual void CreateRootParameters(std::vector<D3D12_ROOT_PARAMETER>& RootParameters, const std::vector<D3D12_DESCRIPTOR_RANGE>& DescriptorRanges) const override { 
		CreateRootParameters_1SRV(RootParameters, DescriptorRanges, D3D12_SHADER_VISIBILITY_PIXEL);
	}
	virtual void CreateStaticSamplerDescs(std::vector<D3D12_STATIC_SAMPLER_DESC>& StaticSamplerDescs) const override {
		//!< LoadImageResource_DDS() 内で作成したサンプラを使用する (DDS から取得した MaxLOD を使用、D3D12_FLOAT32_MAXでも良いのだが念のため)
		StaticSamplerDescs.push_back(StaticSamplerDesc);
	}

	virtual void CreateShader(std::vector<Microsoft::WRL::ComPtr<ID3DBlob>>& ShaderBlobs, std::array<D3D12_SHADER_BYTECODE, 5>& ShaderBytecodes) const override {
		CreateShader_VsPs(ShaderBlobs, ShaderBytecodes);
	}

	virtual void CreateTexture() override {
		LoadImage(ImageResource.GetAddressOf(), ImageDescriptorHeap.GetAddressOf(), L"UV.dds", D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	}
	virtual void CreateSampler(const D3D12_SHADER_VISIBILITY ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL, const FLOAT MaxLOD = (std::numeric_limits<FLOAT>::max)()) override {
		CreateSampler_LinearWrap(ShaderVisibility, MaxLOD);
	}

	virtual void CreatePipelineState() override { CreateGraphicsPipelineState(); }
	virtual void PopulateCommandList(ID3D12GraphicsCommandList* CommandList, ID3D12CommandAllocator* CommandAllocator) override;
};
#pragma endregion