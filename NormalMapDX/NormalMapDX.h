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
		Super::OnTimer(hWnd, hInstance);

		//DirectX::XMStoreFloat4x4(&Tr.World, DirectX::XMMatrixRotationX(DirectX::XMConvertToRadians(Degree)));

		const auto WV = DirectX::XMMatrixMultiply(DirectX::XMLoadFloat4x4(&Tr.World), DirectX::XMLoadFloat4x4(&Tr.View));
		auto DetWV = DirectX::XMMatrixDeterminant(WV);
		DirectX::XMStoreFloat4(&Tr.LocalCameraPosition, DirectX::XMVector4Transform(DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f), DirectX::XMMatrixInverse(&DetWV, WV)));

		const auto LightPos = DirectX::XMVector4Transform(DirectX::XMVectorSet(10.0f, 0.0f, 0.0f, 0.0f), DirectX::XMMatrixRotationY(DirectX::XMConvertToRadians(Degree)));
		//const auto LightPos = DirectX::XMVector4Transform(DirectX::XMVectorSet(0.0f, 10.0f, 0.0f, 0.0f), DirectX::XMMatrixRotationX(DirectX::XMConvertToRadians(Degree)));
		const auto World = DirectX::XMLoadFloat4x4(&Tr.World);
		auto DetWorld = DirectX::XMMatrixDeterminant(DirectX::XMLoadFloat4x4(&Tr.World));
		DirectX::XMStoreFloat4(&Tr.LocalLightDirection, DirectX::XMVector3Normalize(DirectX::XMVector4Transform(LightPos, DirectX::XMMatrixInverse(&DetWorld, World))));

		Degree += 1.0f;

		CopyToUploadResource(COM_PTR_GET(ConstantBuffers[0].Resource), RoundUp256(sizeof(Tr)), &Tr);
	}
	virtual void CreateIndirectBuffer() override { CreateIndirectBuffer_DrawIndexed(1, 1); }

	virtual void CreateStaticSampler() override {
		StaticSamplerDescs.push_back({
			D3D12_FILTER_MIN_MAG_MIP_LINEAR,
			D3D12_TEXTURE_ADDRESS_MODE_WRAP, D3D12_TEXTURE_ADDRESS_MODE_WRAP, D3D12_TEXTURE_ADDRESS_MODE_WRAP,
			0.0f,
			0,
			D3D12_COMPARISON_FUNC_NEVER,
			D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE,
			0.0f, 1.0f,
			0, 0, D3D12_SHADER_VISIBILITY_PIXEL //!< register(s0, space0)
		});
	}
	virtual void CreateRootSignature() override {
		COM_PTR<ID3DBlob> Blob;
#ifdef USE_HLSL_ROOTSIGNATRUE
		GetRootSignaturePartFromShader(Blob, (GetBasePath() + TEXT(".rs.cso")).data());
#else
		const std::array<D3D12_DESCRIPTOR_RANGE, 1> DRs_Cbv = {
			{ D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND } //!< register(b0, space0)
		};
		const std::array<D3D12_DESCRIPTOR_RANGE, 1> DRs_Srv = {
			{ D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 2, 0, 0, D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND } //!< register(t0, space0), register(t1, space0)
		};
		assert(!StaticSamplerDescs.empty() && "");
		DX::SerializeRootSignature(Blob, {
				{ D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE, { static_cast<UINT>(DRs_Cbv.size()), DRs_Cbv.data() }, D3D12_SHADER_VISIBILITY_GEOMETRY }, //!< CBV
				{ D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE, { static_cast<UINT>(DRs_Srv.size()), DRs_Srv.data() }, D3D12_SHADER_VISIBILITY_PIXEL }, //!< SRV0, SRV1
			}, {
				StaticSamplerDescs[0],
			}, D3D12_ROOT_SIGNATURE_FLAG_NONE 
			| D3D12_ROOT_SIGNATURE_FLAG_DENY_VERTEX_SHADER_ROOT_ACCESS
			| D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS
			| D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS
			//| D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS
			//| D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS
		);
#endif
		RootSignatures.resize(1);
		VERIFY_SUCCEEDED(Device->CreateRootSignature(0, Blob->GetBufferPointer(), Blob->GetBufferSize(), COM_PTR_UUIDOF_PUTVOID(RootSignatures[0])));
		LOG_OK();
	}

	virtual void CreateConstantBuffer() override {
		const auto Fov = 0.16f * DirectX::XM_PI;
		const auto Aspect = GetAspectRatioOfClientRect();
		const auto ZFar = 100.0f;
		const auto ZNear = ZFar * 0.0001f;
		const auto CamPos = DirectX::XMVectorSet(0.0f, 0.0f, 3.0f, 1.0f);
		const auto CamTag = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
		const auto CamUp = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
		const auto Projection = DirectX::XMMatrixPerspectiveFovRH(Fov, Aspect, ZNear, ZFar);
		const auto View = DirectX::XMMatrixLookAtRH(CamPos, CamTag, CamUp);
		const auto World = DirectX::XMMatrixIdentity();

		DirectX::XMStoreFloat4x4(&Tr.Projection, Projection);
		DirectX::XMStoreFloat4x4(&Tr.View, View);
		DirectX::XMStoreFloat4x4(&Tr.World, World);
		DirectX::XMStoreFloat4(&Tr.LocalCameraPosition, DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f));
		DirectX::XMStoreFloat4(&Tr.LocalLightDirection, DirectX::XMVectorSet(10.0f, 0.0f, 0.0f, 0.0f));

		ConstantBuffers.push_back(ConstantBuffer());
		CreateUploadResource(COM_PTR_PUT(ConstantBuffers.back().Resource), RoundUp256(sizeof(Tr)));
		ConstantBuffers.back().ViewDesc = { COM_PTR_GET(ConstantBuffers.back().Resource)->GetGPUVirtualAddress(), static_cast<UINT>(ConstantBuffers.back().Resource->GetDesc().Width) };
		//ConstantBuffers.back().CreateViewDesc();
	}
	virtual void CreateTexture() override {
#ifdef USE_PARALLAX_MAP
		ImageResources.resize(2);
		std::wstring Path;
		if (FindDirectory("DDS", Path)) {
			LoadImage(COM_PTR_PUT(ImageResources[0]), Path + TEXT("\\Leather009_2K-JPG\\Leather009_2K_Normal.dds"), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
			LoadImage(COM_PTR_PUT(ImageResources[1]), Path + TEXT("\\Leather009_2K-JPG\\Leather009_2K_Displacement.dds"), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
		}
#else
		ImageResources.resize(2);
		std::wstring Path;
		if (FindDirectory("DDS", Path)) {
			LoadImage(COM_PTR_PUT(ImageResources[0]), Path + TEXT("\\Rocks007_2K-JPG\\Rocks007_2K_Normal.dds"), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
			LoadImage(COM_PTR_PUT(ImageResources[1]), Path + TEXT("\\Rocks007_2K-JPG\\Rocks007_2K_Color.dds"), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

			//LoadImage(COM_PTR_PUT(ImageResources[0]), Path + TEXT("\\PavingStones050_2K-JPG\\PavingStones050_2K_Normal.dds"), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
			//LoadImage(COM_PTR_PUT(ImageResources[1]), Path + TEXT("\\PavingStones050_2K-JPG\\PavingStones050_2K_Color.dds"), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

			//LoadImage(COM_PTR_PUT(ImageResources[0]), Path + TEXT("\\Leather009_2K-JPG\\Leather009_2K_Normal.dds"), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
			//LoadImage(COM_PTR_PUT(ImageResources[1]), Path + TEXT("\\Leather009_2K-JPG\\Leather009_2K_Color.dds"), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
		}
#endif
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

	virtual void CreateDescriptorHeap() override {
		{
			CbvSrvUavDescriptorHeaps.resize(1);
			const D3D12_DESCRIPTOR_HEAP_DESC DHD = { D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 3, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE, 0 }; //!< CBV, SRV0, SRV1
			VERIFY_SUCCEEDED(Device->CreateDescriptorHeap(&DHD, COM_PTR_UUIDOF_PUTVOID(CbvSrvUavDescriptorHeaps[0])));
		}
		{
			DsvDescriptorHeaps.resize(1);
			const D3D12_DESCRIPTOR_HEAP_DESC DHD = { D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 1, D3D12_DESCRIPTOR_HEAP_FLAG_NONE, 0 }; //!< DSV
			VERIFY_SUCCEEDED(Device->CreateDescriptorHeap(&DHD, COM_PTR_UUIDOF_PUTVOID(DsvDescriptorHeaps[0])));
		}
	}
	virtual void CreateDescriptorView() override {
		{
			const auto& DH = CbvSrvUavDescriptorHeaps[0];
			auto CDH = DH->GetCPUDescriptorHandleForHeapStart();

			Device->CreateConstantBufferView(&ConstantBuffers[0].ViewDesc, CDH); CDH.ptr += Device->GetDescriptorHandleIncrementSize(DH->GetDesc().Type); //!< CBV

			assert(2 <= ImageResources.size() && "");
			Device->CreateShaderResourceView(COM_PTR_GET(ImageResources[0]), nullptr, CDH); CDH.ptr += Device->GetDescriptorHandleIncrementSize(DH->GetDesc().Type); //!< SRV0
			Device->CreateShaderResourceView(COM_PTR_GET(ImageResources[1]), nullptr, CDH); CDH.ptr += Device->GetDescriptorHandleIncrementSize(DH->GetDesc().Type); //!< SRV1
		}
		{
			assert(3 == ImageResources.size() && "");
			assert(!CbvSrvUavDescriptorHeaps.empty() && "");
			const auto& DH = DsvDescriptorHeaps[0];
			auto CDH = DH->GetCPUDescriptorHandleForHeapStart();
			Device->CreateDepthStencilView(COM_PTR_GET(ImageResources[2]), nullptr, CDH); CDH.ptr += Device->GetDescriptorHandleIncrementSize(DH->GetDesc().Type); //!< DSV
		}
	}
	virtual void CreateShaderBlobs() override {
#ifdef USE_PARALLAX_MAP
		const auto ShaderPath = GetBasePath();
		ShaderBlobs.push_back(COM_PTR<ID3DBlob>());
		VERIFY_SUCCEEDED(D3DReadFileToBlob((ShaderPath + TEXT(".vs.cso")).data(), COM_PTR_PUT(ShaderBlobs.back())));
		ShaderBlobs.push_back(COM_PTR<ID3DBlob>());
		VERIFY_SUCCEEDED(D3DReadFileToBlob((ShaderPath + TEXT("_pm.ps.cso")).data(), COM_PTR_PUT(ShaderBlobs.back())));
		ShaderBlobs.push_back(COM_PTR<ID3DBlob>());
		VERIFY_SUCCEEDED(D3DReadFileToBlob((ShaderPath + TEXT(".ds.cso")).data(), COM_PTR_PUT(ShaderBlobs.back())));
		ShaderBlobs.push_back(COM_PTR<ID3DBlob>());
		VERIFY_SUCCEEDED(D3DReadFileToBlob((ShaderPath + TEXT(".hs.cso")).data(), COM_PTR_PUT(ShaderBlobs.back())));
		ShaderBlobs.push_back(COM_PTR<ID3DBlob>());
		VERIFY_SUCCEEDED(D3DReadFileToBlob((ShaderPath + TEXT(".gs.cso")).data(), COM_PTR_PUT(ShaderBlobs.back()))); 
#else
		CreateShaderBlob_VsPsDsHsGs();
#endif
	}
	virtual void CreatePipelineStates() override { CreatePipelineState_VsPsDsHsGs(D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH, TRUE); }
	virtual void PopulateCommandList(const size_t i) override;

private:
	struct Transform
	{
		DirectX::XMFLOAT4X4 Projection;
		DirectX::XMFLOAT4X4 View;
		DirectX::XMFLOAT4X4 World;
		DirectX::XMFLOAT4 LocalCameraPosition;
		DirectX::XMFLOAT4 LocalLightDirection;
	};
	using Transform = struct Transform;
	FLOAT Degree = 0.0f;
	Transform Tr;
};
#pragma endregion