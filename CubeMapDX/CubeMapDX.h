#pragma once

#include "resource.h"

#pragma region Code
#include "../DXImage.h"

class CubeMapDX : public DXImage
{
private:
	using Super = DXImage;
public:
	CubeMapDX() : Super() {}
	virtual ~CubeMapDX() {}

protected:
	virtual void OnTimer(HWND hWnd, HINSTANCE hInstance) override {
		Super::OnTimer(hWnd, hInstance);

		const auto CamPos = DirectX::XMVector3Transform(DirectX::XMVectorSet(0.0f, 0.0f, 3.0f, 1.0f), DirectX::XMMatrixRotationY(DirectX::XMConvertToRadians(Degree)));
#ifdef USE_SKY_DOME
		const auto View = DirectX::XMMatrixLookAtRH(CamPos, DirectX::XMVectorSet(0.0f, 2.0f * DirectX::XMScalarSin(DirectX::XMConvertToRadians(Degree)), 0.0f, 1.0f), DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f));
		DirectX::XMStoreFloat4x4(&Tr.View, View);
		DirectX::XMStoreFloat4x4(&Tr.World, DirectX::XMMatrixTranslationFromVector(CamPos)); //!< �J�����ʒu�ɋ���z�u
#else
		const auto View = DirectX::XMMatrixLookAtRH(CamPos, DirectX::XMVectorSet(0.0f, 0.0f * DirectX::XMScalarSin(DirectX::XMConvertToRadians(Degree)), 0.0f, 1.0f), DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f));
		DirectX::XMStoreFloat4x4(&Tr.View, View);
#endif
		const auto WV = DirectX::XMMatrixMultiply(DirectX::XMLoadFloat4x4(&Tr.World), DirectX::XMLoadFloat4x4(&Tr.View));
		auto DetWV = DirectX::XMMatrixDeterminant(WV);
		DirectX::XMStoreFloat4(&Tr.LocalCameraPosition, DirectX::XMVector4Transform(DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f), DirectX::XMMatrixInverse(&DetWV, WV)));

		Degree += 1.0f;

		CopyToUploadResource(COM_PTR_GET(ConstantBufferResources[0]), RoundUp256(sizeof(Tr)), &Tr);
	}
#if !defined(USE_SKY_DOME) || defined(USE_DEPTH_STENCIL)
	virtual void CreateDepthStencil() override { CreateDepthStencilResource(DXGI_FORMAT_D24_UNORM_S8_UINT, GetClientRectWidth(), GetClientRectHeight()); }
#endif
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

		ConstantBufferResources.push_back(COM_PTR<ID3D12Resource>());
		CreateUploadResource(COM_PTR_PUT(ConstantBufferResources.back()), RoundUp256(sizeof(Tr)));
	}
	virtual void CreateTexture() override {
		ImageResources.resize(2);
		std::wstring Path;
		if (FindDirectory("DDS", Path)) {
			//!< �L���[�u�}�b�v : PX, NX, PY, NY, PZ, NZ
			LoadImage(COM_PTR_PUT(ImageResources[0]), Path + TEXT("\\CubeMap\\DebugCube.dds"), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
			//LoadImage(COM_PTR_PUT(ImageResources[0]), Path + TEXT("\\CubeMap\\DesertCube.dds"), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
			//LoadImage(COM_PTR_PUT(ImageResources[0]), Path + TEXT("\\CubeMap\\GrassCube.dds"), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
			//LoadImage(COM_PTR_PUT(ImageResources[0]), Path + TEXT("\\CubeMap\\SunsetCube.dds"), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
			
			//!< �@���}�b�v
			//LoadImage(COM_PTR_PUT(ImageResources[1]), Path + TEXT("\\Leather009_2K-JPG\\Leather009_2K_Normal.dds"), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
			LoadImage(COM_PTR_PUT(ImageResources[1]), Path + TEXT("\\Metal012_2K-JPG\\Metal012_2K_Normal.dds"), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
		}
	}

	virtual void CreateDescriptorHeap() override {
		{
			CbvSrvUavDescriptorHeaps.resize(1);
			const D3D12_DESCRIPTOR_HEAP_DESC DHD = { D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 3, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE, 0 }; //!< CBV, SRV0, SRV1
			VERIFY_SUCCEEDED(Device->CreateDescriptorHeap(&DHD, COM_PTR_UUIDOF_PUTVOID(CbvSrvUavDescriptorHeaps[0])));
		}
#if !defined(USE_SKY_DOME) || defined(USE_DEPTH_STENCIL)
		{
			DsvDescriptorHeaps.resize(1);
			const D3D12_DESCRIPTOR_HEAP_DESC DHD = { D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 1, D3D12_DESCRIPTOR_HEAP_FLAG_NONE, 0 }; //!< DSV
			VERIFY_SUCCEEDED(Device->CreateDescriptorHeap(&DHD, COM_PTR_UUIDOF_PUTVOID(DsvDescriptorHeaps[0])));
		}
#endif
	}
	virtual void CreateDescriptorView() override {
		{
			const auto& DH = CbvSrvUavDescriptorHeaps[0];
			auto CDH = DH->GetCPUDescriptorHandleForHeapStart();
			assert(!ConstantBufferResources.empty() && "");
			assert(ConstantBufferResources[0]->GetDesc().Width == RoundUp256(sizeof(Transform)) && "");
			const D3D12_CONSTANT_BUFFER_VIEW_DESC CBVD = { COM_PTR_GET(ConstantBufferResources[0])->GetGPUVirtualAddress(), static_cast<UINT>(ConstantBufferResources[0]->GetDesc().Width) };
			Device->CreateConstantBufferView(&CBVD, CDH); CDH.ptr += Device->GetDescriptorHandleIncrementSize(DH->GetDesc().Type); //!< CBV
			assert(2 == ImageResources.size() && "");
			//!< (�f�B�����V������D3D12_SRV_DIMENSION_TEXTURECUBE�̈�)�����I��SHADER_RESOURCE_VIEW_DESC���w�肷�邱�� (���\�[�X�Ɠ����t�H�[�}�b�g�ƃf�B�����V�����A�ŏ��̃~�b�v�}�b�v�ƃX���C�X���^�[�Q�b�g����ꍇ��nullptr���w��ł���)
			const auto RD = ImageResources[0]->GetDesc(); assert(D3D12_RESOURCE_DIMENSION_TEXTURE2D == RD.Dimension);
			D3D12_SHADER_RESOURCE_VIEW_DESC SRVD = {
				RD.Format,
				D3D12_SRV_DIMENSION_TEXTURECUBE,
				D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING,
			};
			SRVD.TextureCube = { 0, RD.MipLevels, 0.0f };
			Device->CreateShaderResourceView(COM_PTR_GET(ImageResources[0]), &SRVD, CDH); CDH.ptr += Device->GetDescriptorHandleIncrementSize(DH->GetDesc().Type); //!< SRV0
			Device->CreateShaderResourceView(COM_PTR_GET(ImageResources[1]), nullptr, CDH); CDH.ptr += Device->GetDescriptorHandleIncrementSize(DH->GetDesc().Type); //!< SRV1
		}
#if !defined(USE_SKY_DOME) || defined(USE_DEPTH_STENCIL)
		{
			assert(!CbvSrvUavDescriptorHeaps.empty() && "");
			const auto& DH = DsvDescriptorHeaps[0];
			auto CDH = DH->GetCPUDescriptorHandleForHeapStart();
			Device->CreateDepthStencilView(COM_PTR_GET(DepthStencilResource), nullptr, CDH); CDH.ptr += Device->GetDescriptorHandleIncrementSize(DH->GetDesc().Type); //!< DSV
		}
#endif
	}
	virtual void CreateShaderBlobs() override {
#ifdef USE_SKY_DOME
		const auto ShaderPath = GetBasePath();
		ShaderBlobs.push_back(COM_PTR<ID3DBlob>());
		VERIFY_SUCCEEDED(D3DReadFileToBlob((ShaderPath + TEXT(".vs.cso")).data(), COM_PTR_PUT(ShaderBlobs.back())));
		ShaderBlobs.push_back(COM_PTR<ID3DBlob>());
		VERIFY_SUCCEEDED(D3DReadFileToBlob((ShaderPath + TEXT("_sd.ps.cso")).data(), COM_PTR_PUT(ShaderBlobs.back())));
		ShaderBlobs.push_back(COM_PTR<ID3DBlob>());
		VERIFY_SUCCEEDED(D3DReadFileToBlob((ShaderPath + TEXT("_sd.ds.cso")).data(), COM_PTR_PUT(ShaderBlobs.back())));
		ShaderBlobs.push_back(COM_PTR<ID3DBlob>());
		VERIFY_SUCCEEDED(D3DReadFileToBlob((ShaderPath + TEXT("_sd.hs.cso")).data(), COM_PTR_PUT(ShaderBlobs.back())));
		ShaderBlobs.push_back(COM_PTR<ID3DBlob>());
		VERIFY_SUCCEEDED(D3DReadFileToBlob((ShaderPath + TEXT("_sd.gs.cso")).data(), COM_PTR_PUT(ShaderBlobs.back())));
#else
		CreateShaderBlob_VsPsDsHsGs();
#endif
	}
	virtual void CreatePipelineStates() override { 
#if !defined(USE_SKY_DOME) || defined(USE_DEPTH_STENCIL)
		CreatePipelineState_VsPsDsHsGs(D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH, TRUE);
#else
		CreatePipelineState_VsPsDsHsGs(D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH, FALSE);
#endif
	}
	virtual void PopulateCommandList(const size_t i) override;

private:
	struct Transform
	{
		DirectX::XMFLOAT4X4 Projection;
		DirectX::XMFLOAT4X4 View;
		DirectX::XMFLOAT4X4 World;
		DirectX::XMFLOAT4 LocalCameraPosition;
	};
	using Transform = struct Transform;
	FLOAT Degree = 0.0f;
	Transform Tr;
};
#pragma endregion