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
	virtual void OnTimer(HWND hWnd, HINSTANCE hInstance) override {
		Super::OnTimer(hWnd, hInstance);
		
		DirectX::XMStoreFloat4x4(&Tr.World, DirectX::XMMatrixRotationX(DirectX::XMConvertToRadians(Degree)));
		Degree += 1.0f;

#pragma region FRAME_OBJECT
		CopyToUploadResource(COM_PTR_GET(ConstantBuffers[GetCurrentBackBufferIndex()].Resource), RoundUp256(sizeof(Tr)), &Tr);
#pragma endregion
	}
	virtual void CreateIndirectBuffer() override { CreateIndirectBuffer_DrawIndexed(1, 1); }
	virtual void CreateConstantBuffer() override {
		//constexpr auto Fov = 0.16f * DirectX::XM_PI;
		constexpr auto Fov = 0.16f * std::numbers::pi_v<float>;
		const auto Aspect = GetAspectRatioOfClientRect();
		constexpr auto ZFar = 100.0f;
		constexpr auto ZNear = ZFar * 0.0001f;
		const auto CamPos = DirectX::XMVectorSet(0.0f, 0.0f, 3.0f, 1.0f);
		const auto CamTag = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
		const auto CamUp = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
		const auto Projection = DirectX::XMMatrixPerspectiveFovRH(Fov, Aspect, ZNear, ZFar);
		const auto View = DirectX::XMMatrixLookAtRH(CamPos, CamTag, CamUp);
		const auto World = DirectX::XMMatrixIdentity();
		DirectX::XMStoreFloat4x4(&Tr.Projection, Projection);
		DirectX::XMStoreFloat4x4(&Tr.View, View);
		DirectX::XMStoreFloat4x4(&Tr.World, World);
#pragma region FRAME_OBJECT
		DXGI_SWAP_CHAIN_DESC1 SCD;
		SwapChain->GetDesc1(&SCD);
		for (UINT i = 0; i < SCD.BufferCount; ++i) {
			ConstantBuffers.emplace_back(ConstantBuffer());
			CreateBufferResource(COM_PTR_PUT(ConstantBuffers.back().Resource), RoundUp256(sizeof(Tr)), D3D12_HEAP_TYPE_UPLOAD);
		}
#pragma endregion
	}

#ifdef USE_DEPTH
	//!< 深度テクスチャ
	virtual void CreateTexture() override {
		ImageResources.emplace_back(COM_PTR<ID3D12Resource>());
		const D3D12_HEAP_PROPERTIES HeapProperties = {
			.Type = D3D12_HEAP_TYPE_DEFAULT,
			.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
			.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN,
			.CreationNodeMask = 0, //!< マルチGPUの場合に使用(1つしか使わない場合は0で良い)
			.VisibleNodeMask = 0  //!< マルチGPUの場合に使用(1つしか使わない場合は0で良い)
		};
		const D3D12_RESOURCE_DESC RD = {
			.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D,
			.Alignment = 0,
			.Width = static_cast<UINT64>(GetClientRectWidth()), .Height = static_cast<UINT>(GetClientRectHeight()),
			.DepthOrArraySize = 1,
			.MipLevels = 1,
			.Format = DXGI_FORMAT_D24_UNORM_S8_UINT,
			.SampleDesc = {.Count = 1, .Quality = 0 }, //!< レンダーターゲットのものと一致すること,
			.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN,
			.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL
		};
		//!< 一致するクリア値なら最適化されるのでよく使うクリア値を指定しておく
		const D3D12_CLEAR_VALUE CV = { .Format = RD.Format, .DepthStencil = { .Depth = 1.0f, .Stencil = 0 } };
		VERIFY_SUCCEEDED(Device->CreateCommittedResource(&HeapProperties, D3D12_HEAP_FLAG_NONE, &RD, D3D12_RESOURCE_STATE_DEPTH_WRITE, &CV, COM_PTR_UUIDOF_PUTVOID(ImageResources.back())));

		//!< ビュー
		DepthStencilViewDescs.emplace_back(D3D12_DEPTH_STENCIL_VIEW_DESC({ 
			.Format = ImageResources.back()->GetDesc().Format, 
			.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D, 
			.Flags = D3D12_DSV_FLAG_NONE, 
			.Texture2D = { .MipSlice = 0 }
		}));
	}
#endif

	virtual void CreateRootSignature() override {
		COM_PTR<ID3DBlob> Blob;
#ifdef USE_HLSL_ROOTSIGNATRUE
		GetRootSignaturePartFromShader(Blob, data(GetBasePath() + TEXT(".rs.cso")));
#else
		const std::array DRs = {
			D3D12_DESCRIPTOR_RANGE({ .RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV, .NumDescriptors = 1, .BaseShaderRegister = 0, .RegisterSpace = 0, .OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND })
		};
		DX::SerializeRootSignature(Blob, {
				D3D12_ROOT_PARAMETER({.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE, .DescriptorTable = D3D12_ROOT_DESCRIPTOR_TABLE({.NumDescriptorRanges = static_cast<UINT>(size(DRs)), .pDescriptorRanges = data(DRs) }), .ShaderVisibility = D3D12_SHADER_VISIBILITY_GEOMETRY }),
			}, {}, D3D12_ROOT_SIGNATURE_FLAG_NONE | SHADER_ROOT_ACCESS_GS);
#endif
		RootSignatures.emplace_back(COM_PTR<ID3D12RootSignature>());
		VERIFY_SUCCEEDED(Device->CreateRootSignature(0, Blob->GetBufferPointer(), Blob->GetBufferSize(), COM_PTR_UUIDOF_PUTVOID(RootSignatures.back())));
		LOG_OK();
	}
	virtual void CreateShaderBlobs() override {
#ifdef USE_SCREENSPACE_WIREFRAME
		const auto ShaderPath = GetBasePath();
		ShaderBlobs.emplace_back(COM_PTR<ID3DBlob>());
		VERIFY_SUCCEEDED(D3DReadFileToBlob(data(ShaderPath + TEXT(".vs.cso")), COM_PTR_PUT(ShaderBlobs.back())));
		ShaderBlobs.emplace_back(COM_PTR<ID3DBlob>());
		VERIFY_SUCCEEDED(D3DReadFileToBlob(data(ShaderPath + TEXT("_wf.ps.cso")), COM_PTR_PUT(ShaderBlobs.back())));
		ShaderBlobs.emplace_back(COM_PTR<ID3DBlob>());
		VERIFY_SUCCEEDED(D3DReadFileToBlob(data(ShaderPath + TEXT(".ds.cso")), COM_PTR_PUT(ShaderBlobs.back())));
		ShaderBlobs.emplace_back(COM_PTR<ID3DBlob>());
		VERIFY_SUCCEEDED(D3DReadFileToBlob(data(ShaderPath + TEXT(".hs.cso")), COM_PTR_PUT(ShaderBlobs.back())));
		ShaderBlobs.emplace_back(COM_PTR<ID3DBlob>());
		VERIFY_SUCCEEDED(D3DReadFileToBlob(data(ShaderPath + TEXT("_wf.gs.cso")), COM_PTR_PUT(ShaderBlobs.back())));
#else
		CreateShaderBlob_VsPsDsHsGs();
#endif
	}
	virtual void CreatePipelineStates() override {
#ifdef USE_DEPTH
		CreatePipelineState_VsPsDsHsGs(D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH, TRUE);
#else
		CreatePipelineState_VsPsDsHsGs(D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH, FALSE);
#endif
	}

	virtual void CreateDescriptorHeap() override {
		{
			DXGI_SWAP_CHAIN_DESC1 SCD;
			SwapChain->GetDesc1(&SCD);
#pragma region FRAME_OBJECT
			const D3D12_DESCRIPTOR_HEAP_DESC DHD = { .Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, .NumDescriptors = SCD.BufferCount, .Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE, .NodeMask = 0 }; //!< CBV * N
#pragma endregion
			CbvSrvUavDescriptorHeaps.emplace_back(COM_PTR<ID3D12DescriptorHeap>());
			VERIFY_SUCCEEDED(Device->CreateDescriptorHeap(&DHD, COM_PTR_UUIDOF_PUTVOID(CbvSrvUavDescriptorHeaps.back())));
		}
#ifdef USE_DEPTH
		//!< 深度デスクリプタヒープ
		{
			const D3D12_DESCRIPTOR_HEAP_DESC DHD = { .Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV, .NumDescriptors = 1, .Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE, .NodeMask = 0 };
			DsvDescriptorHeaps.emplace_back(COM_PTR<ID3D12DescriptorHeap>());
			VERIFY_SUCCEEDED(Device->CreateDescriptorHeap(&DHD, COM_PTR_UUIDOF_PUTVOID(DsvDescriptorHeaps.back())));
		}
#endif
	}
	virtual void CreateDescriptorView() override {
		{
			DXGI_SWAP_CHAIN_DESC1 SCD;
			SwapChain->GetDesc1(&SCD);

			const auto& DH = CbvSrvUavDescriptorHeaps[0];
			auto CDH = DH->GetCPUDescriptorHandleForHeapStart();
#pragma region FRAME_OBJECT
			for (UINT i = 0; i < SCD.BufferCount; ++i) {
				const D3D12_CONSTANT_BUFFER_VIEW_DESC CBVD = { .BufferLocation = COM_PTR_GET(ConstantBuffers[i].Resource)->GetGPUVirtualAddress(), .SizeInBytes = static_cast<UINT>(ConstantBuffers[i].Resource->GetDesc().Width) };
				Device->CreateConstantBufferView(&CBVD, CDH); CDH.ptr += Device->GetDescriptorHandleIncrementSize(DH->GetDesc().Type);
			}
#pragma endregion
		}
#ifdef USE_DEPTH
		//!< 深度ビュー
		{
			assert(!empty(ImageResources) && "");
			const auto& DH = DsvDescriptorHeaps[0];
			auto CDH = DH->GetCPUDescriptorHandleForHeapStart();
#if 1
			Device->CreateDepthStencilView(COM_PTR_GET(ImageResources[0]), &DepthStencilViewDescs[0], CDH); CDH.ptr += Device->GetDescriptorHandleIncrementSize(DH->GetDesc().Type);
#else
			//!< リソースと同じフォーマットとディメンションで最初のミップマップとスライスをターゲットするような場合にはnullptrを指定できる
			Device->CreateDepthStencilView(COM_PTR_GET(ImageResources[0]), nullptr, CDH); CDH.ptr += Device->GetDescriptorHandleIncrementSize(DH->GetDesc().Type);
#endif
		}
#endif
	}

	virtual void PopulateCommandList(const size_t i) override;

private:
	FLOAT Degree = 0.0f;

	struct Transform
	{
		DirectX::XMFLOAT4X4 Projection;
		DirectX::XMFLOAT4X4 View;
		DirectX::XMFLOAT4X4 World;
	};
	using Transform = struct Transform;
	Transform Tr;
};
#pragma endregion