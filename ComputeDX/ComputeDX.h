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
	virtual void CreateCommandList() override {
		VERIFY_SUCCEEDED(Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_COMPUTE, COM_PTR_UUIDOF_PUTVOID(CommandAllocators.emplace_back())));

		VERIFY_SUCCEEDED(Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_COMPUTE, COM_PTR_GET(CommandAllocators.back()), nullptr, COM_PTR_UUIDOF_PUTVOID(GraphicsCommandLists.back())));
		VERIFY_SUCCEEDED(GraphicsCommandLists.back()->Close());
	}
	virtual void CreateGeometry() override { 
		constexpr D3D12_DISPATCH_ARGUMENTS DA = { .ThreadGroupCountX = 32, .ThreadGroupCountY = 1, .ThreadGroupCountZ = 1 };
		IndirectBuffers.emplace_back().Create(COM_PTR_GET(Device), COM_PTR_GET(CommandAllocators[0]), COM_PTR_GET(GraphicsCommandLists[0]), COM_PTR_GET(GraphicsCommandQueue), COM_PTR_GET(Fence), DA);
	}
	virtual void CreateTexture() override {
		constexpr std::array Data = { 0.0f, 0.0f, 0.0f };

		COM_PTR<ID3D12Resource> Resource;
		//constexpr auto HP = D3D12_HEAP_PROPERTIES({.Type = D3D12_HEAP_TYPE_CUSTOM, .CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_WRITE_BACK , .MemoryPoolPreference  = D3D12_MEMORY_POOL_L0 });
		DX::CreateBufferResource(COM_PTR_PUT(Resource), COM_PTR_GET(Device), RoundUp256(sizeof(Data)), D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

		UnorderedAccessViewDescs.emplace_back(D3D12_UNORDERED_ACCESS_VIEW_DESC({
			.Format = DXGI_FORMAT_UNKNOWN,
			.ViewDimension = D3D12_UAV_DIMENSION_BUFFER,
			.Buffer = D3D12_BUFFER_UAV({.FirstElement = 0, .NumElements = static_cast<UINT>(size(Data)), .StructureByteStride = sizeof(Data[0]), .CounterOffsetInBytes = 0, .Flags = D3D12_BUFFER_UAV_FLAG_NONE})
		}));
	}
	virtual void CreateRootSignature() override {
		COM_PTR<ID3DBlob> Blob;
#ifdef USE_HLSL_ROOTSIGNATRUE
		GetRootSignaturePartFromShader(Blob, data(GetBasePath() + TEXT(".rs.cso")));
#else
		constexpr std::array DRs = {
			D3D12_DESCRIPTOR_RANGE({ D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND })
		};
		DX::SerializeRootSignature(Blob, {
			D3D12_ROOT_PARAMETER({
				.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE,
				.DescriptorTable = D3D12_ROOT_DESCRIPTOR_TABLE({ static_cast<UINT>(size(DRs)), data(DRs) }),
				.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL
			})
			}, {}, D3D12_ROOT_SIGNATURE_FLAG_NONE);
#endif
		VERIFY_SUCCEEDED(Device->CreateRootSignature(0, Blob->GetBufferPointer(), Blob->GetBufferSize(), COM_PTR_UUIDOF_PUTVOID(RootSignatures.emplace_back())));
		LOG_OK();
	}
	virtual void CreatePipelineState() override {
		const auto ShaderPath = GetBasePath();
		COM_PTR<ID3DBlob> SB;
		VERIFY_SUCCEEDED(D3DReadFileToBlob(data(ShaderPath + TEXT(".cs.cso")), COM_PTR_PUT(SB)));
		const D3D12_COMPUTE_PIPELINE_STATE_DESC CPSD = {
			.pRootSignature = COM_PTR_GET(RootSignatures[0]),
			.CS = D3D12_SHADER_BYTECODE({.pShaderBytecode = SB->GetBufferPointer(), .BytecodeLength = SB->GetBufferSize() }),
			.NodeMask = 0,
			.CachedPSO = D3D12_CACHED_PIPELINE_STATE({.pCachedBlob = nullptr, .CachedBlobSizeInBytes = 0}),
			.Flags = D3D12_PIPELINE_STATE_FLAG_NONE
		};
		VERIFY_SUCCEEDED(Device->CreateComputePipelineState(&CPSD, COM_PTR_UUIDOF_PUTVOID(PipelineStates.emplace_back())));
	}
	virtual void CreateDescriptorHeap() override {
		constexpr D3D12_DESCRIPTOR_HEAP_DESC DHD = { D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 1, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE, 0 };
		VERIFY_SUCCEEDED(Device->CreateDescriptorHeap(&DHD, COM_PTR_UUIDOF_PUTVOID(CbvSrvUavDescriptorHeaps.emplace_back())));		
	}
	virtual void CreateDescriptorView() override {
		const auto& DH = CbvSrvUavDescriptorHeaps[0];
		auto CDH = DH->GetCPUDescriptorHandleForHeapStart();
		//Device->CreateShaderResourceView(COM_PTR_GET(ImageResources[0]), &ShaderResourceViewDescs[0], CDH); CDH.ptr += Device->GetDescriptorHandleIncrementSize(DH->GetDesc().Type);
		//Device->CreateUnorderedAccessView(COM_PTR_GET(), nullptr, &UnorderedAccessViewDescs[0], CDH); CDH.ptr += Device->GetDescriptorHandleIncrementSize(DH->GetDesc().Type);
	}
	
	virtual void PopulateCommandList(const size_t i) override;

	virtual void Draw() override { Dispatch(); }

private:
};
#pragma endregion
