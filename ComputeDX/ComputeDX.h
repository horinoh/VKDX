#pragma once

#include "resource.h"

#pragma region Code
#include "../DXExt.h"

class ComputeDX : public DXExt
{
private:
	using Super = DXExt;
public:
	ComputeDX() : Super() {}
	virtual ~ComputeDX() {}

protected:
	virtual void CreateGeometry() override { 
		constexpr D3D12_DISPATCH_ARGUMENTS DA = { .ThreadGroupCountX = 32, .ThreadGroupCountY = 1, .ThreadGroupCountZ = 1 };
		IndirectBuffers.emplace_back().Create(COM_PTR_GET(Device), DA).ExecuteCopyCommand(COM_PTR_GET(Device), COM_PTR_GET(DirectCommandAllocators[0]), COM_PTR_GET(DirectCommandLists[0]), COM_PTR_GET(GraphicsCommandQueue), COM_PTR_GET(GraphicsFence), sizeof(DA), &DA);
	}
	virtual void CreateTexture() override {
		DXGI_SWAP_CHAIN_DESC1 SCD;
		SwapChain.DxSwapChain->GetDesc1(&SCD);
		UnorderedAccessTextures.emplace_back().Create(COM_PTR_GET(Device), GetClientRectWidth(), GetClientRectHeight(), 1, SCD.Format);
	}
	virtual void CreateRootSignature() override {
		COM_PTR<ID3DBlob> Blob;
#ifdef USE_HLSL_ROOTSIGNATRUE
		GetRootSignaturePartFromShader(Blob, data(GetBasePath() + TEXT(".rs.cso")));
#else
		constexpr std::array DRs = {
			//!< register(u0, space0)
			D3D12_DESCRIPTOR_RANGE1({.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV, .NumDescriptors = 1, .BaseShaderRegister = 0, .RegisterSpace = 0, .Flags = D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE,.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND })
		};
		DX::SerializeRootSignature(Blob, {
			//!< UAV0
			D3D12_ROOT_PARAMETER1({
				.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE,
				.DescriptorTable = D3D12_ROOT_DESCRIPTOR_TABLE1({.NumDescriptorRanges = static_cast<UINT>(size(DRs)), .pDescriptorRanges = data(DRs) }),
				.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL
			}),
		}, {}, D3D12_ROOT_SIGNATURE_FLAG_NONE);
#endif
		VERIFY_SUCCEEDED(Device->CreateRootSignature(0, Blob->GetBufferPointer(), Blob->GetBufferSize(), COM_PTR_UUIDOF_PUTVOID(RootSignatures.emplace_back())));
		LOG_OK();
	}
	virtual void CreatePipelineState() override {
		COM_PTR<ID3DBlob> SB;
		VERIFY_SUCCEEDED(D3DReadFileToBlob(data(GetFilePath(".cs.cso").wstring()), COM_PTR_PUT(SB)));
		const D3D12_COMPUTE_PIPELINE_STATE_DESC CPSD = {
			.pRootSignature = COM_PTR_GET(RootSignatures[0]),
			.CS = D3D12_SHADER_BYTECODE({.pShaderBytecode = SB->GetBufferPointer(), .BytecodeLength = SB->GetBufferSize() }),
			.NodeMask = 0,
			.CachedPSO = D3D12_CACHED_PIPELINE_STATE({.pCachedBlob = nullptr, .CachedBlobSizeInBytes = 0}),
			.Flags = D3D12_PIPELINE_STATE_FLAG_NONE
		};
		VERIFY_SUCCEEDED(Device->CreateComputePipelineState(&CPSD, COM_PTR_UUIDOF_PUTVOID(PipelineStates.emplace_back())));
	}
	virtual void CreateDescriptor() override {
		auto& Desc = CbvSrvUavDescs.emplace_back();
		auto& Heap = Desc.first;
		auto& Handle = Desc.second;

		const D3D12_DESCRIPTOR_HEAP_DESC DHD = { .Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, .NumDescriptors = 1, .Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE, .NodeMask = 0 };
		VERIFY_SUCCEEDED(Device->CreateDescriptorHeap(&DHD, COM_PTR_UUIDOF_PUTVOID(Heap)));

		auto CDH = Heap->GetCPUDescriptorHandleForHeapStart();
		auto GDH = Heap->GetGPUDescriptorHandleForHeapStart();
		const auto IncSize = Device->GetDescriptorHandleIncrementSize(Heap->GetDesc().Type);
		
		Device->CreateUnorderedAccessView(COM_PTR_GET(UnorderedAccessTextures[0].Resource), nullptr, &UnorderedAccessTextures[0].UAV, CDH);
		Handle.emplace_back(GDH);
		CDH.ptr += IncSize;
		GDH.ptr += IncSize;
	}
	
	virtual void PopulateCommandList(const size_t i) override {
		const auto PS = COM_PTR_GET(PipelineStates[0]);

		//!< [Pass0] コンピュート
		{
			const auto& Desc = CbvSrvUavDescs[0];
			const auto& Heap = Desc.first;
			const auto& Handle = Desc.second;

			const auto CCL = COM_PTR_GET(ComputeCommandLists[i]);
			const auto CCA = COM_PTR_GET(ComputeCommandAllocators[0]);
			VERIFY_SUCCEEDED(CCL->Reset(CCA, PS)); {
				CCL->SetComputeRootSignature(COM_PTR_GET(RootSignatures[0]));

				const std::array DHs = { COM_PTR_GET(Heap) };
				CCL->SetDescriptorHeaps(static_cast<UINT>(size(DHs)), data(DHs));
				CCL->SetComputeRootDescriptorTable(0, Handle[0]);

				CCL->ExecuteIndirect(COM_PTR_GET(IndirectBuffers[0].CommandSignature), 1, COM_PTR_GET(IndirectBuffers[0].Resource), 0, nullptr, 0);
			} VERIFY_SUCCEEDED(CCL->Close());
		}
		//!< [Pass1] 結果表示
		{
			const auto DCL = COM_PTR_GET(DirectCommandLists[i]);
			const auto DCA = COM_PTR_GET(DirectCommandAllocators[0]);
			const auto RT = COM_PTR_GET(UnorderedAccessTextures[0].Resource);
			VERIFY_SUCCEEDED(DCL->Reset(DCA, PS)); {
				PopulateBeginRenderTargetCommand(DCL, RT); {
				} PopulateEndRenderTargetCommand(DCL, RT, COM_PTR_GET(SwapChain.ResourceAndHandles[i].first));
			} VERIFY_SUCCEEDED(DCL->Close());
		}
	}
};
#pragma endregion
