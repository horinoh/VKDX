#pragma once

#include "DXExt.h"

class DXRT : public DXExt
{
private:
	using Super = DXExt;
protected:
	virtual void CreateTexture() override {
		if (!HasRaytracingSupport(COM_PTR_GET(Device))) { return; }
		DXGI_SWAP_CHAIN_DESC1 SCD;
		SwapChain->GetDesc1(&SCD);
		UnorderedAccessTextures.emplace_back().Create(COM_PTR_GET(Device), GetClientRectWidth(), GetClientRectHeight(), 1, SCD.Format);
	}
	virtual void CreateRootSignature() override {
		if (!HasRaytracingSupport(COM_PTR_GET(Device))) { return; }
		COM_PTR<ID3DBlob> Blob;
#ifdef USE_HLSL_ROOTSIGNATRUE
		GetRootSignaturePartFromShader(Blob, data(GetBasePath() + TEXT(".grs.cso")));
#else
		constexpr std::array DRs_Tlas = {
			//!< register(t0, space0)
			D3D12_DESCRIPTOR_RANGE({.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV, .NumDescriptors = 1, .BaseShaderRegister = 0, .RegisterSpace = 0, .OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND })
		};
		constexpr std::array DRs_Uav = {
			//!< register(u0, space0)
			D3D12_DESCRIPTOR_RANGE({.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV, .NumDescriptors = 1, .BaseShaderRegister = 0, .RegisterSpace = 0, .OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND })
		};
		DX::SerializeRootSignature(Blob, {
			//!< TLAS
			D3D12_ROOT_PARAMETER({
				.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE,
				.DescriptorTable = D3D12_ROOT_DESCRIPTOR_TABLE({.NumDescriptorRanges = static_cast<UINT>(size(DRs_Tlas)), .pDescriptorRanges = data(DRs_Tlas) }),
				.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL
			}),
			//!< UAV0
			D3D12_ROOT_PARAMETER({
				.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE,
				.DescriptorTable = D3D12_ROOT_DESCRIPTOR_TABLE({.NumDescriptorRanges = static_cast<UINT>(size(DRs_Uav)), .pDescriptorRanges = data(DRs_Uav) }),
				.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL
			}),
			}, {}, D3D12_ROOT_SIGNATURE_FLAG_NONE);
#endif
		VERIFY_SUCCEEDED(Device->CreateRootSignature(0, Blob->GetBufferPointer(), Blob->GetBufferSize(), COM_PTR_UUIDOF_PUTVOID(RootSignatures.emplace_back())));
	}
	virtual void CreateDescriptor() override {
		const D3D12_DESCRIPTOR_HEAP_DESC DHD = { .Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, .NumDescriptors = 2, .Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE, .NodeMask = 0 };
		VERIFY_SUCCEEDED(Device->CreateDescriptorHeap(&DHD, COM_PTR_UUIDOF_PUTVOID(CbvSrvUavDescriptorHeaps.emplace_back())));

		CbvSrvUavGPUHandles.emplace_back();
		auto CDH = CbvSrvUavDescriptorHeaps[0]->GetCPUDescriptorHandleForHeapStart();
		auto GDH = CbvSrvUavDescriptorHeaps[0]->GetGPUDescriptorHandleForHeapStart();
		const auto IncSize = Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		//!< [0] TLAS
		Device->CreateShaderResourceView(nullptr/* AS ÇÃèÍçá nullptr ÇéwíËÇ∑ÇÈ*/, &TLASs[0].SRV, CDH);
		CbvSrvUavGPUHandles.back().emplace_back(GDH);
		CDH.ptr += IncSize;
		GDH.ptr += IncSize;
		//!< [1] UAV
		Device->CreateUnorderedAccessView(COM_PTR_GET(UnorderedAccessTextures[0].Resource), nullptr, &UnorderedAccessTextures[0].UAV, CDH);
		CbvSrvUavGPUHandles.back().emplace_back(GDH);
	}

	virtual void PopulateBeginRenderTargetCommand(const size_t i) {
		const auto GCL = GraphicsCommandLists[i];
		const auto UAV = COM_PTR_GET(UnorderedAccessTextures[0].Resource);
		const std::array RBs = {
			//!< RenderTarget : COPY_SOURCE -> UNORDERED_ACCESS
			D3D12_RESOURCE_BARRIER({
				.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION,
				.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE,
				.Transition = D3D12_RESOURCE_TRANSITION_BARRIER({
					.pResource = UAV,
					.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES,
					.StateBefore = D3D12_RESOURCE_STATE_COPY_SOURCE, .StateAfter = D3D12_RESOURCE_STATE_UNORDERED_ACCESS
				})
			})
		};
		GCL->ResourceBarrier(static_cast<UINT>(size(RBs)), data(RBs));
	}
	virtual void PopulateEndRenderTargetCommand(const size_t i) {
		const auto GCL = GraphicsCommandLists[i];
		const auto UAV = COM_PTR_GET(UnorderedAccessTextures[0].Resource);
		const auto SCR = COM_PTR_GET(SwapChainResources[i]);
		{
			const std::array RBs = {
				//!< RenderTarget : UNORDERED_ACCESS -> COPY_SOURCE
				D3D12_RESOURCE_BARRIER({
					.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION,
					.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE,
					.Transition = D3D12_RESOURCE_TRANSITION_BARRIER({.pResource = UAV, .Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES, .StateBefore = D3D12_RESOURCE_STATE_UNORDERED_ACCESS, .StateAfter = D3D12_RESOURCE_STATE_COPY_SOURCE })
				 }),
				//!< SwapChain : PRESENT -> COPY_DEST
				D3D12_RESOURCE_BARRIER({
					.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION,
					.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE,
					.Transition = D3D12_RESOURCE_TRANSITION_BARRIER({.pResource = SCR, .Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES, .StateBefore = D3D12_RESOURCE_STATE_PRESENT, .StateAfter = D3D12_RESOURCE_STATE_COPY_DEST })
				})
			};
			GCL->ResourceBarrier(static_cast<UINT>(size(RBs)), data(RBs));
		}

		GCL->CopyResource(SCR, UAV);

		{
			const std::array RBs = {
				//!< SwapChain : COPY_DEST -> PRESENT
				D3D12_RESOURCE_BARRIER({
					.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION,
					.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE,
					.Transition = D3D12_RESOURCE_TRANSITION_BARRIER({
						.pResource = SCR,
						.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES,
						.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST, .StateAfter = D3D12_RESOURCE_STATE_PRESENT
					})
				}),
			};
			GCL->ResourceBarrier(static_cast<UINT>(size(RBs)), data(RBs));
		}
	}
};
