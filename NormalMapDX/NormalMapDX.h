#pragma once

#include "resource.h"

#pragma region Code
#include "../DXImage.h"

class NormalMapDX : public DXImage
{
private:
	using Super = DXImage;
public:
	NormalMapDX() : Super() {}
	virtual ~NormalMapDX() {}

protected:
	virtual void OnTimer(HWND hWnd, HINSTANCE hInstance) override {
		//UpdateDescriptorHeap();
	}

	virtual void CreateIndirectBuffer() override { CreateIndirectBuffer_Indexed(1); }

	virtual void CreateShader(std::vector<Microsoft::WRL::ComPtr<ID3DBlob>>& ShaderBlobs, std::vector<D3D12_SHADER_BYTECODE>& ShaderBytecodes) const override {
		CreateShader_VsPsDsHsGs(ShaderBlobs, ShaderBytecodes);
	}

	virtual D3D12_PRIMITIVE_TOPOLOGY_TYPE GetPrimitiveTopologyType() const override {
		return D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH;
	}
	virtual D3D_PRIMITIVE_TOPOLOGY GetPrimitiveTopology() const override {
		const UINT PatchControlPoint = 1;
		return static_cast<D3D_PRIMITIVE_TOPOLOGY>(static_cast<UINT>(D3D_PRIMITIVE_TOPOLOGY_1_CONTROL_POINT_PATCHLIST) + (PatchControlPoint - 1));
	}

	virtual void CreateRootParameters(std::vector<D3D12_ROOT_PARAMETER>& RootParameters, const std::vector<D3D12_DESCRIPTOR_RANGE>& DescriptorRanges) const override {
		CreateRootParameters_1CBV(RootParameters, DescriptorRanges, D3D12_SHADER_VISIBILITY_GEOMETRY);
		//CreateRootParameters_1CBV_1SRV(RootParameters, DescriptorRanges, D3D12_SHADER_VISIBILITY_GEOMETRY, D3D12_SHADER_VISIBILITY_PIXEL);
	}
	virtual void CreateDescriptorRanges(std::vector<D3D12_DESCRIPTOR_RANGE>& DescriptorRanges) const override {
		CreateDescriptorRanges_1CBV(DescriptorRanges);
		//CreateDescriptorRanges_1CBV_1SRV(DescriptorRanges);
	}
	virtual void CreateDescriptorHeap() override {
		CreateDescriptorHeap_1CBV<Transform>();
		//CreateDescriptorHeap_1CBV_1SRV<Transform>();
	}
	virtual void UpdateDescriptorHeap() override {
	}

	virtual void CreateConstantBuffer() override {
		const auto Fov = 0.16f * DirectX::XM_PI;
		const auto Aspect = GetAspectRatioOfClientRect();
		const auto ZFar = 100.0f;
		const auto ZNear = ZFar * 0.0001f;
		const auto CamPos = DirectX::XMVectorSet(0.0f, 0.0f, 3.0f, 1.0f);
		const auto CamTag = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
		const auto CamUp = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
		Super::CreateConstantBuffer<Transform>({
			DirectX::XMMatrixPerspectiveFovRH(Fov, Aspect, ZNear, ZFar),
			DirectX::XMMatrixLookAtRH(CamPos, CamTag, CamUp),
			DirectX::XMMatrixIdentity()
		});
	}

	virtual void CreateTexture() override {
		LoadImage(ImageResource.GetAddressOf(), TEXT("NormalMap.dds"), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	}
	virtual void CreateStaticSamplerDesc(D3D12_STATIC_SAMPLER_DESC& StaticSamplerDesc, const D3D12_SHADER_VISIBILITY ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL, const FLOAT MaxLOD = (std::numeric_limits<FLOAT>::max)()) const override {
		CreateStaticSamplerDesc_LW(StaticSamplerDesc, ShaderVisibility, MaxLOD);
	}

	virtual void PopulateCommandList(const size_t i) override;

private:
	struct Transform
	{
		DirectX::XMMATRIX Projection;
		DirectX::XMMATRIX View;
		DirectX::XMMATRIX World;
	};
	using Transform = struct Transform;
};
#pragma endregion