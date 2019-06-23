#pragma once

#include "resource.h"

#pragma region Code
#include "../DXExt.h"

class ToonDX : public DXExt
{
private:
	using Super = DXExt;
public:
	ToonDX() : Super() {}
	virtual ~ToonDX() {}

protected:
	virtual void CreateIndirectBuffer() override { CreateIndirectBuffer_DrawIndexed(1); }

#ifdef USE_WINRT
	virtual void CreateShader(std::vector<winrt::com_ptr<ID3DBlob>>& ShaderBlobs) const override {
#elif defined(USE_WRL)
	virtual void CreateShader(std::vector<Microsoft::WRL::ComPtr<ID3DBlob>> & ShaderBlobs) const override {
#endif
		CreateShader_VsPsDsHsGs(ShaderBlobs);
		Super::CreateShader(ShaderBlobs);
	}

	virtual D3D12_PRIMITIVE_TOPOLOGY_TYPE GetPrimitiveTopologyType() const override {
		return D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH;
	}
	virtual D3D_PRIMITIVE_TOPOLOGY GetPrimitiveTopology() const override {
		const UINT PatchControlPoint = 1;
		return static_cast<D3D_PRIMITIVE_TOPOLOGY>(static_cast<UINT>(D3D_PRIMITIVE_TOPOLOGY_1_CONTROL_POINT_PATCHLIST) + (PatchControlPoint - 1));
	}

#ifdef USE_WINRT
	virtual void SerializeRootSignature(winrt::com_ptr<ID3DBlob>& RSBlob) override;
#elif defined(USE_WRL)
	virtual void SerializeRootSignature(Microsoft::WRL::ComPtr<ID3DBlob>& RSBlob) override;
#endif
	virtual void CreateRootParameters(std::vector<D3D12_ROOT_PARAMETER>& RootParameters, const std::vector<D3D12_DESCRIPTOR_RANGE>& DescriptorRanges) const override {
		CreateRootParameters_1CBV(RootParameters, DescriptorRanges, D3D12_SHADER_VISIBILITY_GEOMETRY);
	}
	virtual void CreateDescriptorRanges(std::vector<D3D12_DESCRIPTOR_RANGE>& DescriptorRanges) const override {
		CreateDescriptorRanges_1CBV(DescriptorRanges);
	}

	virtual void CreateDescriptorHeap() override {
		CreateDescriptorHeap_1CBV();
		CreateDescriptorView_1CBV<Transform>();
	}

	virtual void CreateConstantBuffer() override {
		const auto Fov = 0.16f * DirectX::XM_PI;
		const auto Aspect = GetAspectRatioOfClientRect();
		const auto ZFar = 100.0f;
		const auto ZNear = ZFar * 0.0001f;
		const auto CamPos = DirectX::XMVectorSet(0.0f, 0.0f, 3.0f, 1.0f);
		const auto CamTag = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
		const auto CamUp = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
		Super::CreateConstantBufferT<Transform>({
			DirectX::XMMatrixPerspectiveFovRH(Fov, Aspect, ZNear, ZFar),
			DirectX::XMMatrixLookAtRH(CamPos, CamTag, CamUp),
			DirectX::XMMatrixIdentity()
		});
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