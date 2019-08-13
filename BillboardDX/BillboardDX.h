
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
		//CreateDepthStencil(DXGI_FORMAT_D32_FLOAT_S8X24_UINT, Rect);
	}

	virtual void CreateIndirectBuffer() override { CreateIndirectBuffer_DrawIndexed(1); }

	virtual void CreateRootSignature() override {
#ifdef USE_WINRT
		winrt::com_ptr<ID3DBlob> Blob;
#elif defined(USE_WRL)
		Microsoft::WRL::ComPtr<ID3DBlob> Blob;
#endif
#ifdef ROOTSIGNATRUE_FROM_SHADER
		GetRootSignaturePartFromShader(Blob);
#else
		const std::array<D3D12_DESCRIPTOR_RANGE, 1> DRs = {
			{ D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND }
		};
		DX::SerializeRootSignature(Blob, {
				{ D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE, { static_cast<UINT>(DRs.size()), DRs.data() }, D3D12_SHADER_VISIBILITY_GEOMETRY }
			}, {}, D3D12_ROOT_SIGNATURE_FLAG_NONE);
#endif
		DX::CreateRootSignature(RootSignature, Blob);
		LOG_OK();
	}
	virtual void CreateConstantBuffer() override {
		const auto Fov = 0.16f * DirectX::XM_PI;
		const auto Aspect = GetAspectRatioOfClientRect();
		const auto ZFar = 100.0f;
		const auto ZNear = ZFar * 0.0001f;
		const auto CamPos = DirectX::XMVectorSet(0.0f, 0.0f, 6.0f, 1.0f);
		const auto CamTag = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
		const auto CamUp = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
		Super::CreateConstantBufferT<Transform>({
			DirectX::XMMatrixPerspectiveFovRH(Fov, Aspect, ZNear, ZFar),
			DirectX::XMMatrixLookAtRH(CamPos, CamTag, CamUp),
			DirectX::XMMatrixIdentity()
		});
	}

	virtual void CreateDescriptorHeap() override;
	virtual void CreateDescriptorView() override;
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
	virtual void CreateShaderBlob() override { CreateShaderBlob_VsPsDsHsGs(); }
	virtual void CreatePipelineState() override { CreatePipelineState_VsPsDsHsGs_Tesselation(); }

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