
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
#ifdef USE_HLSL_ROOTSIGNATRUE
		GetRootSignaturePartFromShader(Blob, (GetBasePath() + TEXT(".rs.cso")).data());
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
		Tr = Transform({ DirectX::XMMatrixPerspectiveFovRH(Fov, Aspect, ZNear, ZFar), DirectX::XMMatrixLookAtRH(CamPos, CamTag, CamUp), DirectX::XMMatrixIdentity() });
		Super::CreateConstantBufferT(Tr);
	}

	virtual void CreateDescriptorHeap() override {
		DX::CreateDescriptorHeap(ConstantBufferDescriptorHeap,
			{ D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 1, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE, 0 }
		);
	}
	virtual void CreateDescriptorView() override {
		DX::CreateConstantBufferView(ConstantBufferResource, ConstantBufferDescriptorHeap, sizeof(Transform));
	}
	virtual void UpdateDescriptorHeap() override {
		Tr.World = DirectX::XMMatrixRotationX(DirectX::XMConvertToRadians(Degree));
		Degree += 1.0f;		
#if 1
#ifdef USE_WINRT
		CopyToUploadResource(ConstantBufferResource.get(), RoundUp(sizeof(Tr), 0xff), &Tr);
#elif defined(USE_WRL)
		CopyToUploadResource(ConstantBufferResource.Get(), RoundUp(sizeof(Tr), 0xff), &Tr);
#endif
#else
		D3D12_RANGE Range = { offsetof(Transform, World), offsetof(Transform, World) + sizeof(Tr.World) };
		BYTE* Data;
		VERIFY_SUCCEEDED(ConstantBufferResource->Map(0, &Range, reinterpret_cast<void**>(&Data))); {
			memcpy(Data, reinterpret_cast<const void*>(&Tr.World), sizeof(Tr.World));
		} ConstantBufferResource->Unmap(0, nullptr);
#endif
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

	FLOAT Degree = 0.0f;
	Transform Tr;
};
#pragma endregion