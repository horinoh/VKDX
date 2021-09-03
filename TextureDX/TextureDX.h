#pragma once

#include "resource.h"

#pragma region Code
#include "../DXImage.h"

class TextureDX : public DXImage
{
private:
	using Super = DXImage;
public:
	TextureDX() : Super() {}
	virtual ~TextureDX() {}

protected:
	virtual void CreateGeometry() override {
		constexpr D3D12_DRAW_ARGUMENTS DA = { .VertexCountPerInstance = 4, .InstanceCount = 1, .StartVertexLocation = 0, .StartInstanceLocation = 0 };
		IndirectBuffers.emplace_back().Create(COM_PTR_GET(Device), DA).ExecuteCopyCommand(COM_PTR_GET(Device), COM_PTR_GET(CommandAllocators[0]), COM_PTR_GET(GraphicsCommandLists[0]), COM_PTR_GET(GraphicsCommandQueue), COM_PTR_GET(Fence), sizeof(DA), &DA);
	}
	virtual void CreateTexture() override {
		std::wstring Path;
		if (FindDirectory("DDS", Path)) {
			DDSTextures.emplace_back().Create(COM_PTR_GET(Device), Path + TEXT("\\PavingStones050_2K-JPG\\PavingStones050_2K_Color.dds")).ExecuteCopyCommand(COM_PTR_GET(Device), COM_PTR_GET(CommandAllocators[0]), COM_PTR_GET(GraphicsCommandLists[0]), COM_PTR_GET(GraphicsCommandQueue), COM_PTR_GET(Fence), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
		}
	}
#ifdef USE_STATIC_SAMPLER
	virtual void CreateStaticSampler() override {
		StaticSamplerDescs.emplace_back(D3D12_STATIC_SAMPLER_DESC({
			.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR, //!< ここではスタティックサンプラは LINEAR、非スタティックサンプラは　POINT にしている
			.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP, .AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP, .AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP,
			.MipLODBias = 0.0f,
			.MaxAnisotropy = 0,
			.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER,
			.BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE,
			.MinLOD = 0.0f, .MaxLOD = 1.0f,
			.ShaderRegister = 0, .RegisterSpace = 0, .ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL //!< register(t0, space0)
		}));
	}
#endif
	virtual void CreateRootSignature() override {
		COM_PTR<ID3DBlob> Blob;
#ifdef USE_HLSL_ROOTSIGNATRUE 
#ifdef USE_STATIC_SAMPLER
		GetRootSignaturePartFromShader(Blob, data(GetBasePath() + TEXT(".rs.cso")));
#else
		GetRootSignaturePartFromShader(Blob, data(GetBasePath() + TEXT("_s.rs.cso")));
#endif
#else
		constexpr std::array DRs_Srv = {
			D3D12_DESCRIPTOR_RANGE({.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV, .NumDescriptors = 1, .BaseShaderRegister = 0, .RegisterSpace = 0, .OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND })
		};
#ifndef USE_STATIC_SAMPLER
		constexpr std::array DRs_Smp = {
			D3D12_DESCRIPTOR_RANGE({.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, .NumDescriptors = 1, .BaseShaderRegister = 0, .RegisterSpace = 0, .OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND }) 
		};
#endif
		DX::SerializeRootSignature(Blob, {
			//!< SRV
			D3D12_ROOT_PARAMETER({
				.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE,
				.DescriptorTable = D3D12_ROOT_DESCRIPTOR_TABLE({.NumDescriptorRanges = static_cast<uint32_t>(size(DRs_Srv)), .pDescriptorRanges = data(DRs_Srv) }),
				.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL
			}),
#ifndef USE_STATIC_SAMPLER
			//!< Sampler
			D3D12_ROOT_PARAMETER({
				.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE,
				.DescriptorTable = D3D12_ROOT_DESCRIPTOR_TABLE({.NumDescriptorRanges = static_cast<uint32_t>(size(DRs_Smp)), .pDescriptorRanges = data(DRs_Smp) }),
				.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL
			})
#endif
		}, {
#ifdef USE_STATIC_SAMPLER
			StaticSamplerDescs[0],
#endif
		}, SHADER_ROOT_ACCESS_PS);
#endif
		VERIFY_SUCCEEDED(Device->CreateRootSignature(0, Blob->GetBufferPointer(), Blob->GetBufferSize(), COM_PTR_UUIDOF_PUTVOID(RootSignatures.emplace_back())));
		LOG_OK();
	}
	virtual void CreatePipelineState() override { 
		const auto ShaderPath = GetBasePath();
		std::vector<COM_PTR<ID3DBlob>> SBs = {};
		VERIFY_SUCCEEDED(D3DReadFileToBlob(data(ShaderPath + TEXT(".vs.cso")), COM_PTR_PUT(SBs.emplace_back())));
		VERIFY_SUCCEEDED(D3DReadFileToBlob(data(ShaderPath + TEXT(".ps.cso")), COM_PTR_PUT(SBs.emplace_back())));
		const std::array SBCs = {
			D3D12_SHADER_BYTECODE({.pShaderBytecode = SBs[0]->GetBufferPointer(), .BytecodeLength = SBs[0]->GetBufferSize() }),
			D3D12_SHADER_BYTECODE({.pShaderBytecode = SBs[1]->GetBufferPointer(), .BytecodeLength = SBs[1]->GetBufferSize() }),
		};

		constexpr D3D12_RASTERIZER_DESC RD = {
			.FillMode = D3D12_FILL_MODE_SOLID,
			.CullMode = D3D12_CULL_MODE_BACK, .FrontCounterClockwise = TRUE,
			.DepthBias = D3D12_DEFAULT_DEPTH_BIAS, .DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP, .SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS,
			.DepthClipEnable = TRUE,
			.MultisampleEnable = FALSE, .AntialiasedLineEnable = FALSE, .ForcedSampleCount = 0,
			.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF
		};
		CreatePipelineState_VsPs(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE, RD, FALSE, SBCs); 
	}
	virtual void CreateDescriptor() override {
		const D3D12_DESCRIPTOR_HEAP_DESC DHD = { .Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, .NumDescriptors = 1, .Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE, .NodeMask = 0 }; //!< SRV
		VERIFY_SUCCEEDED(Device->CreateDescriptorHeap(&DHD, COM_PTR_UUIDOF_PUTVOID(CbvSrvUavDescriptorHeaps.emplace_back())));
#ifndef USE_STATIC_SAMPLER
		const D3D12_DESCRIPTOR_HEAP_DESC DHD_Sampler = { .Type = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER, .NumDescriptors = 1, .Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE, .NodeMask = 0 }; //!< Sampler
		VERIFY_SUCCEEDED(Device->CreateDescriptorHeap(&DHD_Sampler, COM_PTR_UUIDOF_PUTVOID(SamplerDescriptorHeaps.emplace_back())));
#endif

		{
			CbvSrvUavGPUHandles.emplace_back();

			auto CDH = CbvSrvUavDescriptorHeaps[0]->GetCPUDescriptorHandleForHeapStart();
			auto GDH = CbvSrvUavDescriptorHeaps[0]->GetGPUDescriptorHandleForHeapStart();
			Device->CreateShaderResourceView(COM_PTR_GET(DDSTextures[0].Resource), &DDSTextures[0].SRV, CDH);
			//!< リソースと同じフォーマットとディメンションで最初のミップマップとスライスをターゲットするような場合には D3D12_SHADER_RESOURCE_VIEW_DESC* に nullptrを指定できる
			//Device->CreateShaderResourceView(COM_PTR_GET(DDSTextures[0].Resource), nullptr, CDH); 
			CbvSrvUavGPUHandles.back().emplace_back(GDH);
		}

#ifndef USE_STATIC_SAMPLER
		{
			SamplerGPUHandles.emplace_back();
			auto CDH = SamplerDescriptorHeaps[0]->GetCPUDescriptorHandleForHeapStart();
			auto GDH = SamplerDescriptorHeaps[0]->GetGPUDescriptorHandleForHeapStart();
			constexpr D3D12_SAMPLER_DESC SD = {
				.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT, //!< ここではスタティックサンプラは LINEAR、非スタティックサンプラは　POINT にしている
				.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP, .AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP, .AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP,
				.MipLODBias = 0.0f,
				.MaxAnisotropy = 0,
				.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER,
				.BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE,
				.MinLOD = 0.0f, .MaxLOD = 1.0f,
			};
			Device->CreateSampler(&SD, CDH);
			SamplerGPUHandles.back().emplace_back(GDH);
		}
#endif
	}

	virtual void PopulateCommandList(const size_t i) override {
		const auto PS = COM_PTR_GET(PipelineStates[0]);

		const auto BGCL = COM_PTR_GET(BundleGraphicsCommandLists[i]);
		const auto BCA = COM_PTR_GET(BundleCommandAllocators[0]);
		VERIFY_SUCCEEDED(BGCL->Reset(BCA, PS));
		{
			BGCL->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
			BGCL->ExecuteIndirect(COM_PTR_GET(IndirectBuffers[0].CommandSignature), 1, COM_PTR_GET(IndirectBuffers[0].Resource), 0, nullptr, 0);
		}
		VERIFY_SUCCEEDED(BGCL->Close());

		const auto GCL = COM_PTR_GET(GraphicsCommandLists[i]);
		const auto CA = COM_PTR_GET(CommandAllocators[0]);
		VERIFY_SUCCEEDED(GCL->Reset(CA, PS));
		{
			GCL->SetGraphicsRootSignature(COM_PTR_GET(RootSignatures[0]));

			GCL->RSSetViewports(static_cast<UINT>(size(Viewports)), data(Viewports));
			GCL->RSSetScissorRects(static_cast<UINT>(size(ScissorRects)), data(ScissorRects));

			const auto SCR = COM_PTR_GET(SwapChainResources[i]);
			ResourceBarrier(GCL, SCR, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
			{
				const std::array CHs = { SwapChainCPUHandles[i] };
				GCL->OMSetRenderTargets(static_cast<UINT>(size(CHs)), data(CHs), FALSE, nullptr);

#ifdef USE_STATIC_SAMPLER
				const std::array DHs = { COM_PTR_GET(CbvSrvUavDescriptorHeaps[0]) };
				GCL->SetDescriptorHeaps(static_cast<UINT>(size(DHs)), data(DHs));
				GCL->SetGraphicsRootDescriptorTable(0, CbvSrvUavGPUHandles.back()[0]);//!< SRV
#else
				const std::array DHs = { COM_PTR_GET(CbvSrvUavDescriptorHeaps[0]), COM_PTR_GET(SamplerDescriptorHeaps[0]) };
				GCL->SetDescriptorHeaps(static_cast<UINT>(size(DHs)), data(DHs));
				GCL->SetGraphicsRootDescriptorTable(0, CbvSrvUavGPUHandles.back()[0]);//!< SRV
				GCL->SetGraphicsRootDescriptorTable(1, SamplerGPUHandles.back()[0]); //!< Sampler
#endif
				GCL->ExecuteBundle(BGCL);
			}
			ResourceBarrier(GCL, SCR, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
		}
		VERIFY_SUCCEEDED(GCL->Close());
	}
};
#pragma endregion