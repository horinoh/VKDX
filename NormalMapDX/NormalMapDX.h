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

#pragma region FRAME_OBJECT
		CopyToUploadResource(COM_PTR_GET(ConstantBuffers[GetCurrentBackBufferIndex()].Resource), RoundUp256(sizeof(Tr)), &Tr);
#pragma endregion
	}
	virtual void CreateIndirectBuffer() override { CreateIndirectBuffer_DrawIndexed(1, 1); }

	virtual void CreateStaticSampler() override {
		StaticSamplerDescs.emplace_back(D3D12_STATIC_SAMPLER_DESC({
			.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR,
			.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP, .AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP, .AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP,
			.MipLODBias = 0.0f,
			.MaxAnisotropy = 0,
			.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER,
			.BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE,
			.MinLOD = 0.0f, .MaxLOD = 1.0f,
			.ShaderRegister = 0, .RegisterSpace = 0, .ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL //!< register(s0, space0)
		}));
	}
	virtual void CreateRootSignature() override {
		COM_PTR<ID3DBlob> Blob;
#ifdef USE_HLSL_ROOTSIGNATRUE
		GetRootSignaturePartFromShader(Blob, data(GetBasePath() + TEXT(".rs.cso")));
#else
		const std::array DRs_Cbv = {
			D3D12_DESCRIPTOR_RANGE({ .RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV, .NumDescriptors = 1, .BaseShaderRegister = 0, .RegisterSpace = 0, .OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND }) //!< register(b0, space0)
		};
		const std::array DRs_Srv = {
			D3D12_DESCRIPTOR_RANGE({ .RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV, .NumDescriptors = 2, .BaseShaderRegister = 0, .RegisterSpace = 0, .OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND }) //!< register(t0, space0), register(t1, space0)
		};
		assert(!empty(StaticSamplerDescs) && "");
		DX::SerializeRootSignature(Blob, {
				D3D12_ROOT_PARAMETER({ .ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE, .DescriptorTable = D3D12_ROOT_DESCRIPTOR_TABLE({ .NumDescriptorRanges = static_cast<UINT>(size(DRs_Cbv)), .pDescriptorRanges = data(DRs_Cbv) }), .ShaderVisibility = D3D12_SHADER_VISIBILITY_GEOMETRY }), //!< CBV
				D3D12_ROOT_PARAMETER({ .ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE, .DescriptorTable = D3D12_ROOT_DESCRIPTOR_TABLE({ .NumDescriptorRanges = static_cast<UINT>(size(DRs_Srv)), .pDescriptorRanges = data(DRs_Srv) }), .ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL }), //!< SRV0, SRV1
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
		RootSignatures.emplace_back(COM_PTR<ID3D12RootSignature>());
		VERIFY_SUCCEEDED(Device->CreateRootSignature(0, Blob->GetBufferPointer(), Blob->GetBufferSize(), COM_PTR_UUIDOF_PUTVOID(RootSignatures.back())));
		LOG_OK();
	}

	virtual void CreateConstantBuffer() override {
		//const auto Fov = 0.16f * DirectX::XM_PI;
		const auto Fov = 0.16f * std::numbers::pi_v<float>;
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

#pragma region FRAME_OBJECT
		DXGI_SWAP_CHAIN_DESC1 SCD;
		SwapChain->GetDesc1(&SCD);
		for (UINT i = 0; i < SCD.BufferCount; ++i) {
			ConstantBuffers.emplace_back(ConstantBuffer());
			CreateBufferResource(COM_PTR_PUT(ConstantBuffers.back().Resource), RoundUp256(sizeof(Tr)), D3D12_HEAP_TYPE_UPLOAD);
		}
#pragma endregion
	}
	virtual void CreateTexture() override {
		std::wstring Path;
#ifdef USE_PARALLAX_MAP
		if (FindDirectory("DDS", Path)) {
			ImageResources.emplace_back(COM_PTR<ID3D12Resource>());
			LoadImage(COM_PTR_PUT(ImageResources.back()), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, Path + TEXT("\\Leather009_2K-JPG\\Leather009_2K_Normal.dds"));

			ShaderResourceViewDescs.emplace_back(D3D12_SHADER_RESOURCE_VIEW_DESC({ 
				.Format = ImageResources.back()->GetDesc().Format, 
				.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D, 
				.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING,
				.Texture2D = D3D12_TEX2D_SRV({ .MostDetailedMip = 0, .MipLevels = ImageResources.back()->GetDesc().MipLevels, .PlaneSlice = 0, .ResourceMinLODClamp = 0.0f })
				}));

			ImageResources.emplace_back(COM_PTR<ID3D12Resource>());
			LoadImage(COM_PTR_PUT(ImageResources.back()), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, Path + TEXT("\\Leather009_2K-JPG\\Leather009_2K_Displacement.dds"));

			ShaderResourceViewDescs.emplace_back(D3D12_SHADER_RESOURCE_VIEW_DESC({ 
				.Format = ImageResources.back()->GetDesc().Format, 
				.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D, 
				.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING,
				.Texture2D = D3D12_TEX2D_SRV ({ .MostDetailedMip = 0, .MipLevels = ImageResources.back()->GetDesc().MipLevels, .PlaneSlice = 0, .ResourceMinLODClamp = 0.0f })
				}));
		}
#else
		if (FindDirectory("DDS", Path)) {
			ImageResources.emplace_back(COM_PTR<ID3D12Resource>());
			LoadImage(COM_PTR_PUT(ImageResources.back()), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, Path + TEXT("\\Rocks007_2K-JPG\\Rocks007_2K_Normal.dds"));

			ShaderResourceViewDescs.emplace_back(D3D12_SHADER_RESOURCE_VIEW_DESC({ 
				.Format = ImageResources.back()->GetDesc().Format, 
				.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D, 
				.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING,
				.Texture2D = D3D12_TEX2D_SRV({ .MostDetailedMip = 0, .MipLevels = ImageResources.back()->GetDesc().MipLevels, .PlaneSlice = 0, .ResourceMinLODClamp = 0.0f })
				}));

			ImageResources.emplace_back(COM_PTR<ID3D12Resource>());
			LoadImage(COM_PTR_PUT(ImageResources.back()), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, Path + TEXT("\\Rocks007_2K-JPG\\Rocks007_2K_Color.dds"));

			ShaderResourceViewDescs.emplace_back(D3D12_SHADER_RESOURCE_VIEW_DESC({ 
				.Format = ImageResources.back()->GetDesc().Format, 
				.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D, 
				.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING,
				.Texture2D = D3D12_TEX2D_SRV({ .MostDetailedMip = 0, .MipLevels = ImageResources.back()->GetDesc().MipLevels, .PlaneSlice = 0, .ResourceMinLODClamp = 0.0f })
				}));
		}
#endif
		{
			ImageResources.emplace_back(COM_PTR<ID3D12Resource>());
			const D3D12_HEAP_PROPERTIES HeapProperties = {
				.Type = D3D12_HEAP_TYPE_DEFAULT,
				.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
				.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN,
				.CreationNodeMask = 0, .VisibleNodeMask = 0
			};
			const DXGI_SAMPLE_DESC SD = { .Count = 1, .Quality = 0 };
			const D3D12_RESOURCE_DESC RD = {
				.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D,
				.Alignment = 0,
				.Width = static_cast<UINT64>(GetClientRectWidth()), .Height = static_cast<UINT>(GetClientRectHeight()),
				.DepthOrArraySize = 1,
				.MipLevels = 1,
				.Format = DXGI_FORMAT_D24_UNORM_S8_UINT,
				.SampleDesc = SD,
				.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN,
				.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL
			};
			const D3D12_CLEAR_VALUE CV = { .Format = RD.Format, .DepthStencil = D3D12_DEPTH_STENCIL_VALUE({ .Depth = 1.0f, .Stencil = 0 }) };
			VERIFY_SUCCEEDED(Device->CreateCommittedResource(&HeapProperties, D3D12_HEAP_FLAG_NONE, &RD, D3D12_RESOURCE_STATE_DEPTH_WRITE, &CV, COM_PTR_UUIDOF_PUTVOID(ImageResources.back())));

			DepthStencilViewDescs.emplace_back(D3D12_DEPTH_STENCIL_VIEW_DESC({
				.Format = ImageResources.back()->GetDesc().Format,  
				.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D,  
				.Flags = D3D12_DSV_FLAG_NONE,
				.Texture2D = D3D12_TEX2D_DSV({ .MipSlice = 0 })
				}));
		}
	}

	virtual void CreateDescriptorHeap() override {
		{
			CbvSrvUavDescriptorHeaps.emplace_back(COM_PTR<ID3D12DescriptorHeap>());
#pragma region FRAME_OBJECT
			DXGI_SWAP_CHAIN_DESC1 SCD;
			SwapChain->GetDesc1(&SCD);
			const D3D12_DESCRIPTOR_HEAP_DESC DHD = { D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, SCD.BufferCount + 2, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE, 0 }; //!< CBV * N, SRV0, SRV1
#pragma endregion
			VERIFY_SUCCEEDED(Device->CreateDescriptorHeap(&DHD, COM_PTR_UUIDOF_PUTVOID(CbvSrvUavDescriptorHeaps.back())));
		}
		{
			DsvDescriptorHeaps.emplace_back(COM_PTR<ID3D12DescriptorHeap>());
			const D3D12_DESCRIPTOR_HEAP_DESC DHD = { D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 1, D3D12_DESCRIPTOR_HEAP_FLAG_NONE, 0 }; //!< DSV
			VERIFY_SUCCEEDED(Device->CreateDescriptorHeap(&DHD, COM_PTR_UUIDOF_PUTVOID(DsvDescriptorHeaps.back())));
		}
	}
	virtual void CreateDescriptorView() override {
		{
			const auto& DH = CbvSrvUavDescriptorHeaps[0];
			auto CDH = DH->GetCPUDescriptorHandleForHeapStart();

#pragma region FRAME_OBJECT
			DXGI_SWAP_CHAIN_DESC1 SCD;
			SwapChain->GetDesc1(&SCD);
			for (UINT i = 0; i < SCD.BufferCount; ++i) {
				const D3D12_CONSTANT_BUFFER_VIEW_DESC CBVD = { COM_PTR_GET(ConstantBuffers[i].Resource)->GetGPUVirtualAddress(), static_cast<UINT>(ConstantBuffers[i].Resource->GetDesc().Width) };
				Device->CreateConstantBufferView(&CBVD, CDH); CDH.ptr += Device->GetDescriptorHandleIncrementSize(DH->GetDesc().Type); //!< CBV
			}
#pragma endregion

			assert(2 <= size(ImageResources) && "");
#if 1
			Device->CreateShaderResourceView(COM_PTR_GET(ImageResources[0]), &ShaderResourceViewDescs[0], CDH); CDH.ptr += Device->GetDescriptorHandleIncrementSize(DH->GetDesc().Type); //!< SRV0
			Device->CreateShaderResourceView(COM_PTR_GET(ImageResources[1]), &ShaderResourceViewDescs[1], CDH); CDH.ptr += Device->GetDescriptorHandleIncrementSize(DH->GetDesc().Type); //!< SRV1
#else
			//!< リソースと同じフォーマットとディメンションで最初のミップマップとスライスをターゲットするような場合にはnullptrを指定できる
			Device->CreateShaderResourceView(COM_PTR_GET(ImageResources[0]), nullptr, CDH); CDH.ptr += Device->GetDescriptorHandleIncrementSize(DH->GetDesc().Type); //!< SRV0
			Device->CreateShaderResourceView(COM_PTR_GET(ImageResources[1]), nullptr, CDH); CDH.ptr += Device->GetDescriptorHandleIncrementSize(DH->GetDesc().Type); //!< SRV1
#endif
		}
		{
			assert(3 == size(ImageResources) && "");
			assert(!empty(CbvSrvUavDescriptorHeaps) && "");
			const auto& DH = DsvDescriptorHeaps[0];
			auto CDH = DH->GetCPUDescriptorHandleForHeapStart();
#if 1
			Device->CreateDepthStencilView(COM_PTR_GET(ImageResources[2]), &DepthStencilViewDescs[0], CDH); CDH.ptr += Device->GetDescriptorHandleIncrementSize(DH->GetDesc().Type); //!< DSV
#else
			//!< リソースと同じフォーマットとディメンションで最初のミップマップとスライスをターゲットするような場合にはnullptrを指定できる
			Device->CreateDepthStencilView(COM_PTR_GET(ImageResources[2]), nullptr, CDH); CDH.ptr += Device->GetDescriptorHandleIncrementSize(DH->GetDesc().Type); //!< DSV
#endif
		}
	}
	virtual void CreateShaderBlobs() override {
#ifdef USE_PARALLAX_MAP
		const auto ShaderPath = GetBasePath();
		ShaderBlobs.push_back(COM_PTR<ID3DBlob>());
		VERIFY_SUCCEEDED(D3DReadFileToBlob(data(ShaderPath + TEXT(".vs.cso")), COM_PTR_PUT(ShaderBlobs.back())));
		ShaderBlobs.push_back(COM_PTR<ID3DBlob>());
		VERIFY_SUCCEEDED(D3DReadFileToBlob(data(ShaderPath + TEXT("_pm.ps.cso")), COM_PTR_PUT(ShaderBlobs.back())));
		ShaderBlobs.push_back(COM_PTR<ID3DBlob>());
		VERIFY_SUCCEEDED(D3DReadFileToBlob(data(ShaderPath + TEXT(".ds.cso")), COM_PTR_PUT(ShaderBlobs.back())));
		ShaderBlobs.push_back(COM_PTR<ID3DBlob>());
		VERIFY_SUCCEEDED(D3DReadFileToBlob(data(ShaderPath + TEXT(".hs.cso")), COM_PTR_PUT(ShaderBlobs.back())));
		ShaderBlobs.push_back(COM_PTR<ID3DBlob>());
		VERIFY_SUCCEEDED(D3DReadFileToBlob(data(ShaderPath + TEXT(".gs.cso")), COM_PTR_PUT(ShaderBlobs.back()))); 
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