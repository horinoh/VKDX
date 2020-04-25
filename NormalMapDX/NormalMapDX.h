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

		//Tr.World = DirectX::XMMatrixRotationX(DirectX::XMConvertToRadians(Degree));

		const auto WV = DirectX::XMMatrixMultiply(Tr.World, Tr.View);
		auto DetWV = DirectX::XMMatrixDeterminant(WV);
		Tr.LocalCameraPosition = DirectX::XMVector4Transform(DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f), DirectX::XMMatrixInverse(&DetWV, WV));

		const auto LightPos = DirectX::XMVector4Transform(DirectX::XMVectorSet(10.0f, 0.0f, 0.0f, 0.0f), DirectX::XMMatrixRotationY(DirectX::XMConvertToRadians(Degree)));
		//const auto LightPos = DirectX::XMVector4Transform(DirectX::XMVectorSet(0.0f, 10.0f, 0.0f, 0.0f), DirectX::XMMatrixRotationX(DirectX::XMConvertToRadians(Degree)));
		auto DetWorld = DirectX::XMMatrixDeterminant(Tr.World);
		Tr.LocalLightDirection = DirectX::XMVector3Normalize(DirectX::XMVector4Transform(LightPos, DirectX::XMMatrixInverse(&DetWorld, Tr.World)));

		Degree += 1.0f;

		CopyToUploadResource(COM_PTR_GET(ConstantBufferResources[0]), RoundUp256(sizeof(Tr)), &Tr);
	}

	virtual void CreateDepthStencil() override { CreateDepthStencilResource(DXGI_FORMAT_D24_UNORM_S8_UINT, GetClientRectWidth(), GetClientRectHeight()); }
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
			0, 0, D3D12_SHADER_VISIBILITY_PIXEL
		});
	}
	virtual void CreateRootSignature() override {
		COM_PTR<ID3DBlob> Blob;
#ifdef USE_HLSL_ROOTSIGNATRUE
		GetRootSignaturePartFromShader(Blob, (GetBasePath() + TEXT(".rs.cso")).data());
#else
		const std::array<D3D12_DESCRIPTOR_RANGE, 1> DRs_Cbv = {
			{ D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND } //!< CBV
		};
		const std::array<D3D12_DESCRIPTOR_RANGE, 1> DRs_Srv = {
			{ D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 2, 0, 0, D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND } //!< SRV, SRV
		};
		assert(!StaticSamplerDescs.empty() && "");
		DX::SerializeRootSignature(Blob, {
				{ D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE, { static_cast<UINT>(DRs_Cbv.size()), DRs_Cbv.data() }, D3D12_SHADER_VISIBILITY_GEOMETRY },
				{ D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE, { static_cast<UINT>(DRs_Srv.size()), DRs_Srv.data() }, D3D12_SHADER_VISIBILITY_PIXEL },
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

		const auto WV = DirectX::XMMatrixMultiply(World, View);
		//const auto VW = DirectX::XMMatrixMultiply(View, World);
		auto DetWV = DirectX::XMMatrixDeterminant(WV);
		const auto LocalCameraPosition = DirectX::XMVector4Transform(DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f), DirectX::XMMatrixInverse(&DetWV, WV));

		const auto LightPos = DirectX::XMVectorSet(0.0f, 10.0f, 0.0f, 0.0f);
		auto DetWorld = DirectX::XMMatrixDeterminant(World);
		const auto LocalLightDirection = DirectX::XMVector3Normalize(DirectX::XMVector4Transform(LightPos, DirectX::XMMatrixInverse(&DetWorld, World)));

		Tr = Transform({ Projection, View, World, LocalCameraPosition, LocalLightDirection });

		ConstantBufferResources.push_back(COM_PTR<ID3D12Resource>());
		CreateUploadResource(COM_PTR_PUT(ConstantBufferResources.back()), RoundUp256(sizeof(Tr)));
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
		//LoadImage(COM_PTR_PUT(ImageResources[0]), TEXT("NormalMap.dds"), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
#endif
	}

	virtual void CreateDescriptorHeap() override {
		{
			CbvSrvUavDescriptorHeaps.resize(1);
			const D3D12_DESCRIPTOR_HEAP_DESC DHD = { D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 3, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE, 0 }; //!< CBV, SRV, SRV
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
			assert(!ConstantBufferResources.empty() && "");
			assert(ConstantBufferResources[0]->GetDesc().Width == RoundUp256(sizeof(Transform)) && "");
			const D3D12_CONSTANT_BUFFER_VIEW_DESC CBVD = { COM_PTR_GET(ConstantBufferResources[0])->GetGPUVirtualAddress(), static_cast<UINT>(ConstantBufferResources[0]->GetDesc().Width) };
			Device->CreateConstantBufferView(&CBVD, CDH); CDH.ptr += Device->GetDescriptorHandleIncrementSize(DH->GetDesc().Type); //!< CBV
			assert(!ImageResources.empty() && "");
			Device->CreateShaderResourceView(COM_PTR_GET(ImageResources[0]), nullptr, CDH); CDH.ptr += Device->GetDescriptorHandleIncrementSize(DH->GetDesc().Type); //!< SRV
			Device->CreateShaderResourceView(COM_PTR_GET(ImageResources[1]), nullptr, CDH); CDH.ptr += Device->GetDescriptorHandleIncrementSize(DH->GetDesc().Type); //!< SRV
		}
		{
			assert(!CbvSrvUavDescriptorHeaps.empty() && "");
			const auto& DH = DsvDescriptorHeaps[0];
			auto CDH = DH->GetCPUDescriptorHandleForHeapStart();
			Device->CreateDepthStencilView(COM_PTR_GET(DepthStencilResource), nullptr, CDH); CDH.ptr += Device->GetDescriptorHandleIncrementSize(DH->GetDesc().Type);
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
		DirectX::XMMATRIX Projection;
		DirectX::XMMATRIX View;
		DirectX::XMMATRIX World;
		DirectX::XMVECTOR LocalCameraPosition;
		DirectX::XMVECTOR LocalLightDirection;
	};
	using Transform = struct Transform;
	FLOAT Degree = 0.0f;
	Transform Tr;
};
#pragma endregion