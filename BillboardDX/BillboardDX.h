
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
	virtual void OnTimer(HWND hWnd, HINSTANCE hInstance) override {
		Super::OnTimer(hWnd, hInstance);

		//Tr.World = DirectX::XMMatrixRotationX(DirectX::XMConvertToRadians(Degree));
		DirectX::XMStoreFloat4x4(&Tr.World, DirectX::XMMatrixRotationX(DirectX::XMConvertToRadians(Degree)));
		Degree += 1.0f;
		
#if 0
		CopyToUploadResource(COM_PTR_GET(ConstantBuffers[0].Resource), RoundUp256(sizeof(Tr)), &Tr);
#else
		const D3D12_RANGE Range = { offsetof(Transform, World), offsetof(Transform, World) + sizeof(Tr.World) };
		CopyToUploadResource(COM_PTR_GET(ConstantBuffers[0].Resource), RoundUp256(sizeof(Tr)), &Tr, &Range);
#endif
	}
	virtual void CreateTexture() override {
		{
			ImageResources.push_back(COM_PTR<ID3D12Resource>());
			const D3D12_HEAP_PROPERTIES HeapProperties = {
				D3D12_HEAP_TYPE_DEFAULT,
				D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
				D3D12_MEMORY_POOL_UNKNOWN,
				0,
				0
			};
			const DXGI_SAMPLE_DESC SD = { 1, 0 };
			const D3D12_RESOURCE_DESC RD = {
				D3D12_RESOURCE_DIMENSION_TEXTURE2D,
				0,
				static_cast<UINT64>(GetClientRectWidth()), static_cast<UINT>(GetClientRectHeight()),
				1,
				1,
				DXGI_FORMAT_D24_UNORM_S8_UINT,
				SD,
				D3D12_TEXTURE_LAYOUT_UNKNOWN,
				D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL
			};
			const D3D12_CLEAR_VALUE CV = { RD.Format, { 1.0f, 0 } };
			VERIFY_SUCCEEDED(Device->CreateCommittedResource(&HeapProperties, D3D12_HEAP_FLAG_NONE, &RD, D3D12_RESOURCE_STATE_DEPTH_WRITE, &CV, COM_PTR_UUIDOF_PUTVOID(ImageResources.back())));
		}
	}
	virtual void CreateIndirectBuffer() override { CreateIndirectBuffer_DrawIndexed(1, 1); }
	virtual void CreateRootSignature() override {
		COM_PTR<ID3DBlob> Blob;

#ifdef USE_HLSL_ROOTSIGNATRUE
		GetRootSignaturePartFromShader(Blob, (GetBasePath() + TEXT(".rs.cso")).data());
#else
		const std::array<D3D12_DESCRIPTOR_RANGE, 1> DRs = {
			{ D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND }
		};
		DX::SerializeRootSignature(Blob, {
				{ D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE, { static_cast<UINT>(DRs.size()), DRs.data() }, D3D12_SHADER_VISIBILITY_GEOMETRY }
			}, {}, D3D12_ROOT_SIGNATURE_FLAG_NONE
			| D3D12_ROOT_SIGNATURE_FLAG_DENY_VERTEX_SHADER_ROOT_ACCESS
			| D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS
			| D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS
			//| D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS
			| D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS);
#endif
		RootSignatures.resize(1);
		VERIFY_SUCCEEDED(Device->CreateRootSignature(0, Blob->GetBufferPointer(), Blob->GetBufferSize(), COM_PTR_UUIDOF_PUTVOID(RootSignatures[0])));
		LOG_OK();
	}

	virtual void CreateDescriptorHeap() override {
		{
			CbvSrvUavDescriptorHeaps.resize(1);
			const D3D12_DESCRIPTOR_HEAP_DESC DHD = { D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 1, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE, 0 };
			VERIFY_SUCCEEDED(Device->CreateDescriptorHeap(&DHD, COM_PTR_UUIDOF_PUTVOID(CbvSrvUavDescriptorHeaps[0])));
		}
		{
			DsvDescriptorHeaps.resize(1);
			const D3D12_DESCRIPTOR_HEAP_DESC DHD = { D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 1, D3D12_DESCRIPTOR_HEAP_FLAG_NONE, 0 };
			VERIFY_SUCCEEDED(Device->CreateDescriptorHeap(&DHD, COM_PTR_UUIDOF_PUTVOID(DsvDescriptorHeaps[0])));
		}
	}
	virtual void CreateDescriptorView() override {
		{
			const auto& DH = CbvSrvUavDescriptorHeaps[0];
			auto CDH = DH->GetCPUDescriptorHandleForHeapStart();

			Device->CreateConstantBufferView(&ConstantBuffers[0].ViewDesc, CDH); CDH.ptr += Device->GetDescriptorHandleIncrementSize(DH->GetDesc().Type);
		}
		{
			assert(!ImageResources.empty() && "");
			const auto& DH = DsvDescriptorHeaps[0];
			auto CDH = DH->GetCPUDescriptorHandleForHeapStart();

			Device->CreateDepthStencilView(COM_PTR_GET(ImageResources[0]), nullptr, CDH); CDH.ptr += Device->GetDescriptorHandleIncrementSize(DH->GetDesc().Type);
		}
	}
	
	virtual void CreateConstantBuffer() override {
		const auto Fov = 0.16f * DirectX::XM_PI;
		const auto Aspect = GetAspectRatioOfClientRect();
		const auto ZFar = 100.0f;
		const auto ZNear = ZFar * 0.0001f;
		const auto CamPos = DirectX::XMVectorSet(0.0f, 0.0f, 6.0f, 1.0f);
		const auto CamTag = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
		const auto CamUp = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
		const auto Projection = DirectX::XMMatrixPerspectiveFovRH(Fov, Aspect, ZNear, ZFar);
		const auto View = DirectX::XMMatrixLookAtRH(CamPos, CamTag, CamUp);
		const auto World = DirectX::XMMatrixIdentity();
		DirectX::XMStoreFloat4x4(&Tr.Projection, Projection);
		DirectX::XMStoreFloat4x4(&Tr.View, View);
		DirectX::XMStoreFloat4x4(&Tr.World, World);

		ConstantBuffers.push_back(ConstantBuffer());
		CreateUploadResource(COM_PTR_PUT(ConstantBuffers.back().Resource), RoundUp256(sizeof(Tr))); //!< コンスタントバッファの場合、サイズは256バイトアラインにすること
		ConstantBuffers.back().ViewDesc = { COM_PTR_GET(ConstantBuffers.back().Resource)->GetGPUVirtualAddress(), static_cast<UINT>(ConstantBuffers.back().Resource->GetDesc().Width) };
		//ConstantBuffers.back().CreateViewDesc();
	}

	virtual void CreateShaderBlobs() override { CreateShaderBlob_VsPsDsHsGs(); }
	virtual void CreatePipelineStates() override { CreatePipelineState_VsPsDsHsGs(D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH, TRUE); }
	virtual void PopulateCommandList(const size_t i) override;

private: 
	struct Transform
	{
		DirectX::XMFLOAT4X4 Projection;
		DirectX::XMFLOAT4X4 View;
		DirectX::XMFLOAT4X4 World;
	};
	using Transform = struct Transform;

	FLOAT Degree = 0.0f;
	Transform Tr;
};
#pragma endregion