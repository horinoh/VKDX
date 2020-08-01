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
		DirectX::XMStoreFloat4x4(&Tr.World, DirectX::XMMatrixTranslationFromVector(CamPos)); //!< カメラ位置に球を配置
#else
		const auto View = DirectX::XMMatrixLookAtRH(CamPos, DirectX::XMVectorSet(0.0f, 0.0f * DirectX::XMScalarSin(DirectX::XMConvertToRadians(Degree)), 0.0f, 1.0f), DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f));
		DirectX::XMStoreFloat4x4(&Tr.View, View);
#endif
		const auto WV = DirectX::XMMatrixMultiply(DirectX::XMLoadFloat4x4(&Tr.World), DirectX::XMLoadFloat4x4(&Tr.View));
		auto DetWV = DirectX::XMMatrixDeterminant(WV);
		DirectX::XMStoreFloat4(&Tr.LocalCameraPosition, DirectX::XMVector4Transform(DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f), DirectX::XMMatrixInverse(&DetWV, WV)));

		Degree += 1.0f;

#pragma region FRAME_OBJECT
		CopyToUploadResource(COM_PTR_GET(ConstantBuffers[SwapChain->GetCurrentBackBufferIndex()].Resource), RoundUp256(sizeof(Tr)), &Tr);
#pragma endregion
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
		RootSignatures.push_back(COM_PTR<ID3D12RootSignature>());
		VERIFY_SUCCEEDED(Device->CreateRootSignature(0, Blob->GetBufferPointer(), Blob->GetBufferSize(), COM_PTR_UUIDOF_PUTVOID(RootSignatures.back())));
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

#pragma region FRAME_OBJECT
		for (auto i = 0; i < SwapChainResources.size(); ++i) {
			ConstantBuffers.push_back(ConstantBuffer());
			CreateUploadResource(COM_PTR_PUT(ConstantBuffers.back().Resource), RoundUp256(sizeof(Tr)));
		}
#pragma endregion
	}
	virtual void CreateTexture() override {
		std::wstring Path;
		if (FindDirectory("DDS", Path)) {
			//!< [0] キューブ(Cube) : PX, NX, PY, NY, PZ, NZ
			ImageResources.push_back(COM_PTR<ID3D12Resource>());
			LoadImage(COM_PTR_PUT(ImageResources.back()), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, Path + TEXT("\\CubeMap\\DebugCube.dds"));

			//!< ビューでD3D12_SRV_DIMENSION_TEXTURECUBEを指定するため明示的なD3D12_SHADER_RESOURCE_VIEW_DESCの使用が必須
			ShaderResourceViewDescs.push_back({ ImageResources.back()->GetDesc().Format, D3D12_SRV_DIMENSION_TEXTURECUBE, D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING });
			ShaderResourceViewDescs.back().TextureCube =  { 0, ImageResources.back()->GetDesc().MipLevels, 0.0f };

			//!< [1] 法線(Normal)
			ImageResources.push_back(COM_PTR<ID3D12Resource>());
			LoadImage(COM_PTR_PUT(ImageResources.back()), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, Path + TEXT("\\Metal012_2K-JPG\\Metal012_2K_Normal.dds"));

			ShaderResourceViewDescs.push_back({ ImageResources.back()->GetDesc().Format, D3D12_SRV_DIMENSION_TEXTURE2D, D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING });
			ShaderResourceViewDescs.back().Texture2D = { 0, ImageResources.back()->GetDesc().MipLevels, 0, 0.0f };
		}
#if !defined(USE_SKY_DOME)
		//!< [2] 深度(Depth)
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

			DepthStencilViewDescs.push_back({ ImageResources.back()->GetDesc().Format,  D3D12_DSV_DIMENSION_TEXTURE2D,  D3D12_DSV_FLAG_NONE });
			DepthStencilViewDescs.back().Texture2D = { 0 };
		}
#endif
	}

	virtual void CreateDescriptorHeap() override {
		{
			CbvSrvUavDescriptorHeaps.push_back(COM_PTR<ID3D12DescriptorHeap>());
#pragma region FRAME_OBJECT
			const D3D12_DESCRIPTOR_HEAP_DESC DHD = { D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, static_cast<UINT>(SwapChainResources.size()) + 2, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE, 0 }; //!< CBV * N, SRV0, SRV1
#pragma endregion
			VERIFY_SUCCEEDED(Device->CreateDescriptorHeap(&DHD, COM_PTR_UUIDOF_PUTVOID(CbvSrvUavDescriptorHeaps.back())));
		}
#if !defined(USE_SKY_DOME)
		{
			DsvDescriptorHeaps.push_back(COM_PTR<ID3D12DescriptorHeap>());
			const D3D12_DESCRIPTOR_HEAP_DESC DHD = { D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 1, D3D12_DESCRIPTOR_HEAP_FLAG_NONE, 0 }; //!< DSV
			VERIFY_SUCCEEDED(Device->CreateDescriptorHeap(&DHD, COM_PTR_UUIDOF_PUTVOID(DsvDescriptorHeaps.back())));
		}
#endif
	}
	virtual void CreateDescriptorView() override {
		{
			const auto& DH = CbvSrvUavDescriptorHeaps[0];
			auto CDH = DH->GetCPUDescriptorHandleForHeapStart();
			
#pragma region FRAME_OBJECT
			for (auto i = 0; i < SwapChainResources.size(); ++i) {
				const D3D12_CONSTANT_BUFFER_VIEW_DESC CBVD = { COM_PTR_GET(ConstantBuffers[i].Resource)->GetGPUVirtualAddress(), static_cast<UINT>(ConstantBuffers[i].Resource->GetDesc().Width) };
				Device->CreateConstantBufferView(&CBVD, CDH); CDH.ptr += Device->GetDescriptorHandleIncrementSize(DH->GetDesc().Type); //!< CBV
			}
#pragma endregion

			assert(2 <= ImageResources.size() && "");
			//!< D3D12_SRV_DIMENSION_TEXTURECUBEを指定する必要がある為、(nullptrではなく)明示的にSHADER_RESOURCE_VIEW_DESCを使用すること
			Device->CreateShaderResourceView(COM_PTR_GET(ImageResources[0]), &ShaderResourceViewDescs[0], CDH); CDH.ptr += Device->GetDescriptorHandleIncrementSize(DH->GetDesc().Type); //!< SRV0

#if 1
			Device->CreateShaderResourceView(COM_PTR_GET(ImageResources[1]), &ShaderResourceViewDescs[1], CDH); CDH.ptr += Device->GetDescriptorHandleIncrementSize(DH->GetDesc().Type); //!< SRV1
#else
			//!< リソースと同じフォーマットとディメンションで最初のミップマップとスライスをターゲットするような場合にはnullptrを指定できる
			Device->CreateShaderResourceView(COM_PTR_GET(ImageResources[1]), nullptr, CDH); CDH.ptr += Device->GetDescriptorHandleIncrementSize(DH->GetDesc().Type); //!< SRV1
#endif
		}
#if !defined(USE_SKY_DOME)
		{
			assert(3 == ImageResources.size() && "");
			assert(!CbvSrvUavDescriptorHeaps.empty() && "");
			const auto& DH = DsvDescriptorHeaps[0];
			auto CDH = DH->GetCPUDescriptorHandleForHeapStart();
#if 1
			Device->CreateDepthStencilView(COM_PTR_GET(ImageResources[2]), &DepthStencilViewDescs[0], CDH); CDH.ptr += Device->GetDescriptorHandleIncrementSize(DH->GetDesc().Type); //!< DSV
#else
			//!< リソースと同じフォーマットとディメンションで最初のミップマップとスライスをターゲットするような場合にはnullptrを指定できる
			Device->CreateDepthStencilView(COM_PTR_GET(ImageResources[2]), nullptr, CDH); CDH.ptr += Device->GetDescriptorHandleIncrementSize(DH->GetDesc().Type); //!< DSV
#endif
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
#if !defined(USE_SKY_DOME)
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