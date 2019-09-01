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
	virtual void CreateIndirectBuffer() override { CreateIndirectBuffer_DrawIndexed(1); }

	virtual void CreateRootSignature() override {
#ifdef USE_WINRT
		winrt::com_ptr<ID3DBlob> Blob;
#elif defined(USE_WRL)
		Microsoft::WRL::ComPtr<ID3DBlob> Blob;
#endif
#ifdef USE_HLSL_ROOTSIGNATRUE
#ifdef USE_STATIC_SAMPLER
		GetRootSignaturePartFromShader(Blob, (GetBasePath() + TEXT(".rs.cso")).data());
#else
		GetRootSignaturePartFromShader(Blob, (GetBasePath() + TEXT("-S.rs.cso")).data());
#endif
#else
		const std::array<D3D12_DESCRIPTOR_RANGE, 1> DRs_Cbv = {
			{ D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND }
		};
		const std::array<D3D12_DESCRIPTOR_RANGE, 1> DRs_Srv = {
			{ D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND }
		};
#ifdef USE_STATIC_SAMPLER
		assert(!StaticSamplerDescs.empty() && "");
		DX::SerializeRootSignature(Blob, {
				{ D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE, { static_cast<UINT>(DRs_Cbv.size()), DRs_Cbv.data() }, D3D12_SHADER_VISIBILITY_GEOMETRY },
				{ D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE, { static_cast<UINT>(DRs_Srv.size()), DRs_Srv.data() }, D3D12_SHADER_VISIBILITY_PIXEL },
			}, {
				StaticSamplerDescs[0],
			}, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
#else
		const std::array<D3D12_DESCRIPTOR_RANGE, 1> DRs_Smp = {
			{ D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND }
		};
		DX::SerializeRootSignature(Blob, {
				{ D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE, { static_cast<UINT>(DRs_Cbv.size()), DRs_Cbv.data() }, D3D12_SHADER_VISIBILITY_GEOMETRY },
				{ D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE, { static_cast<UINT>(DRs_Srv.size()), DRs_Srv.data() }, D3D12_SHADER_VISIBILITY_PIXEL },
				{ D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE, { static_cast<UINT>(DRs_Smp.size()), DRs_Smp.data() }, D3D12_SHADER_VISIBILITY_PIXEL }
			}, {}, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
#endif
#endif
		DX::CreateRootSignature(RootSignature, Blob);
		LOG_OK();
	}


	virtual void CreateDescriptorHeap() override {
		DX::CreateDescriptorHeap(ConstantBufferDescriptorHeap,
			{ D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 1, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE, 0 }
		);
		DX::CreateDescriptorHeap(ImageDescriptorHeap,
			{ D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 1, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE, 0 }
		);

#ifndef USE_STATIC_SAMPLER
		SamplerDescriptorHeaps.resize(1);
		DX::CreateDescriptorHeap(SamplerDescriptorHeaps[0],
			{ D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER, 1, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE, 0 }
		);
#endif
		LOG_OK();
	}
	virtual void CreateDescriptorView() override {
		DX::CreateConstantBufferView(ConstantBufferResource, ConstantBufferDescriptorHeap, sizeof(Transform));
		DX::CreateShaderResourceView(ImageResource, ImageDescriptorHeap);

#ifndef USE_STATIC_SAMPLER
		const D3D12_SAMPLER_DESC SD = {
			D3D12_FILTER_MIN_MAG_MIP_POINT,
			D3D12_TEXTURE_ADDRESS_MODE_WRAP, D3D12_TEXTURE_ADDRESS_MODE_WRAP, D3D12_TEXTURE_ADDRESS_MODE_WRAP,
			0.0f,
			0,
			D3D12_COMPARISON_FUNC_NEVER,
			D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE,
			0.0f, 1.0f,
		};
		Device->CreateSampler(&SD, SamplerDescriptorHeaps[0]->GetCPUDescriptorHandleForHeapStart());
#endif
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
		Tr = Transform({ DirectX::XMMatrixPerspectiveFovRH(Fov, Aspect, ZNear, ZFar), DirectX::XMMatrixLookAtRH(CamPos, CamTag, CamUp), DirectX::XMMatrixIdentity() });
		Super::CreateConstantBufferT(Tr);
	}
	virtual void UpdateDescriptorHeap() override {
//		Tr.World = DirectX::XMMatrixRotationX(DirectX::XMConvertToRadians(Degree));
//		Degree += 1.0f;
//#ifdef USE_WINRT
//		CopyToUploadResource(ConstantBufferResource.get(), RoundUp(sizeof(Tr), 0xff), &Tr);
//#elif defined(USE_WRL)
//		CopyToUploadResource(ConstantBufferResource.Get(), RoundUp(sizeof(Tr), 0xff), &Tr);
//#endif
	}
	virtual void CreateTexture() override {
#ifdef USE_WINRT
		LoadImage(ImageResource.put(), TEXT("NormalMap.dds"), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
#elif defined(USE_WRL)
		LoadImage(ImageResource.GetAddressOf(), TEXT("NormalMap.dds"), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
#endif
	}
#ifdef USE_STATIC_SAMPLER
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
#endif
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