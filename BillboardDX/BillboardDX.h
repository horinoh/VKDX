
#pragma once

#include "resource.h"

#pragma region Code
#include "../DXExt.h"

class BillboardDX : public DXExt
{
private:
	using Super = DXExt;
public:
	BillboardDX() : Super() {}
	virtual ~BillboardDX() {}

protected:
	virtual void CreateDepthStencil() override {
		//CreateDepthStencilOfClientRect(DXGI_FORMAT_D32_FLOAT_S8X24_UINT);
	}

	virtual void CreateIndirectBuffer() override { CreateIndirectBuffer_DrawIndexed(1); }

	virtual void CreateShader(std::vector<Microsoft::WRL::ComPtr<ID3DBlob>>& ShaderBlobs) const override {
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
	
	virtual void CreateRootParameters(std::vector<D3D12_ROOT_PARAMETER>& RootParameters, const std::vector<D3D12_DESCRIPTOR_RANGE>& DescriptorRanges) const override {
		CreateRootParameters_1CBV(RootParameters, DescriptorRanges, D3D12_SHADER_VISIBILITY_GEOMETRY);
	}
	virtual void CreateDescriptorRanges(std::vector<D3D12_DESCRIPTOR_RANGE>& DescriptorRanges) const override {
		CreateDescriptorRanges_1CBV(DescriptorRanges);
	}
	virtual void CreateDescriptorHeap() override {
		CreateDescriptorHeap_1CBV<Transform>();
	}
	virtual void UpdateDescriptorHeap() override {
		//static FLOAT Angle = 0.0f;
		//DirectX::XMMATRIX World = DirectX::XMMatrixRotationX(Angle);
		//D3D12_RANGE Range = { offsetof(Transform, World), offsetof(Transform, World) + sizeof(World) };
		//BYTE* Data;
		//VERIFY_SUCCEEDED(ConstantBufferResource->Map(0, &Range, reinterpret_cast<void**>(&Data))); {
		//	memcpy(Data, reinterpret_cast<const void*>(&World), sizeof(World));
		//} ConstantBufferResource->Unmap(0, nullptr);
		//Angle += 1.0f;
	}

	virtual void CreateConstantBuffer() override {
		const auto Fov = 0.16f * DirectX::XM_PI;
		const auto Aspect = GetAspectRatioOfClientRect();
		const auto ZFar = 100.0f;
		const auto ZNear = ZFar * 0.0001f;
		const auto CamPos = DirectX::XMVectorSet(0.0f, 0.0f, 6.0f, 1.0f);
		const auto CamTag = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
		const auto CamUp = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
		Super::CreateConstantBuffer<Transform>({
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