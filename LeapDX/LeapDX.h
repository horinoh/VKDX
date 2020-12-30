#pragma once

#include "resource.h"

#pragma region Code
#include "../DXExt.h"
#include "../Leap.h"

class LeapDX : public DXExt, public Leap
{
private:
	using Super = DXExt;
public:
	LeapDX() : Super() {}
	virtual ~LeapDX() {}

protected:
	virtual void OnTimer(HWND hWnd, HINSTANCE hInstance) override {
		Super::OnTimer(hWnd, hInstance);
#ifdef USE_LEAP
		InterpolatedTrackingEvent();
#endif
	}
#ifdef USE_LEAP
	virtual void OnTrackingEvent(const LEAP_TRACKING_EVENT* TE) override {
		Leap::OnTrackingEvent(TE);
	}
	virtual void OnImageEvent(const LEAP_IMAGE_EVENT* IE) override {
		Leap::OnImageEvent(IE);
	}
#endif

	virtual void CreateIndirectBuffer() override { CreateIndirectBuffer_Draw(4, 1); }
	virtual void CreateTexture() override {
#ifdef USE_LEAP
		constexpr auto Layers = 1;// _countof(LEAP_IMAGE_EVENT::image); // TODO

		{
			ImageResources.emplace_back(COM_PTR<ID3D12Resource>());
		}
		{
			ImageResources.emplace_back(COM_PTR<ID3D12Resource>());

			{
				COM_PTR<ID3D12Resource> Resource;
				{
					constexpr auto Size = sizeof(DistortionMatrices[0]);
					CreateBufferResource(COM_PTR_PUT(Resource), Size, D3D12_HEAP_TYPE_UPLOAD);
					//CopyToUploadResource(COM_PTR_GET(Resource), PSF, NumRows, RowBytes, Subresource);
				}
			}
		}
#else
		CreateTexture1x1(0xff0000ff); //!< ABGR
#endif
	}
	virtual void CreateStaticSampler() override {
		//!< https://developer.leapmotion.com/documentation/v4/images.html
		//!< フィルタを LINEAR、ラップモードを CLAMP
		StaticSamplerDescs.emplace_back(D3D12_STATIC_SAMPLER_DESC({
			.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR,
			.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP, .AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP, .AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
			.MipLODBias = 0.0f,
			.MaxAnisotropy = 0,
			.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER,
			.BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE,
			.MinLOD = 0.0f, .MaxLOD = 1.0f,
			.ShaderRegister = 0, .RegisterSpace = 0, .ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL //!< register(t0, space0)
			}));
	}

	virtual void CreateRootSignature() override {
		COM_PTR<ID3DBlob> Blob;
#ifdef USE_HLSL_ROOTSIGNATRUE 
		GetRootSignaturePartFromShader(Blob, data(GetBasePath() + TEXT(".rs.cso")));
#else
		const std::array DRs = {
			D3D12_DESCRIPTOR_RANGE({
				.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
				.NumDescriptors = 1,
				.BaseShaderRegister = 0, .RegisterSpace = 0,
				.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND
				})
		};
		assert(!empty(StaticSamplerDescs) && "");
		DX::SerializeRootSignature(Blob, {
			D3D12_ROOT_PARAMETER({
				.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE,
				.DescriptorTable = D3D12_ROOT_DESCRIPTOR_TABLE({.NumDescriptorRanges = static_cast<uint32_t>(size(DRs)), .pDescriptorRanges = data(DRs) }),
				.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL
					}),
			}, {
				StaticSamplerDescs[0],
			},
			D3D12_ROOT_SIGNATURE_FLAG_NONE
			| D3D12_ROOT_SIGNATURE_FLAG_DENY_VERTEX_SHADER_ROOT_ACCESS
			| D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS
			| D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS
			| D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS
		);
#endif
		RootSignatures.emplace_back(COM_PTR<ID3D12RootSignature>());
		VERIFY_SUCCEEDED(Device->CreateRootSignature(0, Blob->GetBufferPointer(), Blob->GetBufferSize(), COM_PTR_UUIDOF_PUTVOID(RootSignatures.back())));
		LOG_OK();
	}

	virtual void CreateShaderBlobs() override { CreateShaderBlob_VsPs(); }

	virtual void CreatePipelineStates() override { CreatePipelineState_VsPs(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE, FALSE); }

	virtual void CreateDescriptorHeap() override {
		{
			CbvSrvUavDescriptorHeaps.emplace_back(COM_PTR<ID3D12DescriptorHeap>());
			const D3D12_DESCRIPTOR_HEAP_DESC DHD = { .Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, .NumDescriptors = 1, .Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE, .NodeMask = 0 };
			VERIFY_SUCCEEDED(Device->CreateDescriptorHeap(&DHD, COM_PTR_UUIDOF_PUTVOID(CbvSrvUavDescriptorHeaps.back())));
		}
	}
	virtual void CreateDescriptorView() override {
		assert(!empty(ImageResources) && "");
		const auto& DH = CbvSrvUavDescriptorHeaps[0];
		auto CDH = DH->GetCPUDescriptorHandleForHeapStart();
		Device->CreateShaderResourceView(COM_PTR_GET(ImageResources[0]), &ShaderResourceViewDescs[0], CDH); CDH.ptr += Device->GetDescriptorHandleIncrementSize(DH->GetDesc().Type);
	}

	virtual void PopulateCommandList(const size_t i) override;
};
#pragma endregion