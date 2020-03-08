#include "DXExt.h"

void DXExt::CreateIndirectBuffer_Draw(const UINT IndexCount, const UINT InstanceCount)
{
	IndirectBufferResources.push_back(COM_PTR<ID3D12Resource>());

	const D3D12_DRAW_ARGUMENTS Source = { IndexCount, InstanceCount, 0, 0 };
	const auto Stride = sizeof(Source);
	const auto Size = static_cast<UINT32>(Stride * 1);
	CreateAndCopyToDefaultResource(IndirectBufferResources.back(), COM_PTR_GET(CommandAllocators[0]), COM_PTR_GET(GraphicsCommandLists[0]), Size, &Source);

	IndirectCommandSignatures.resize(1);
	const std::array<D3D12_INDIRECT_ARGUMENT_DESC, 1> IADs = {
		{ D3D12_INDIRECT_ARGUMENT_TYPE_DRAW },
	};
	const D3D12_COMMAND_SIGNATURE_DESC CSD = {
		Stride,
		static_cast<const UINT>(IADs.size()), IADs.data(),
		0
	};
	//!< パイプラインのバインディングを更新するような場合はルートシグネチャが必要、Draw や Dispatch のみの場合はnullptrを指定できる
	Device->CreateCommandSignature(&CSD, /*COM_PTR_GET(RootSignatures[0])*/nullptr, COM_PTR_UUIDOF_PUTVOID(IndirectCommandSignatures[0]));
}

void DXExt::CreateIndirectBuffer_DrawIndexed(const UINT IndexCount, const UINT InstanceCount)
{
	IndirectBufferResources.push_back(COM_PTR<ID3D12Resource>());

	const D3D12_DRAW_INDEXED_ARGUMENTS Source = { IndexCount, InstanceCount, 0, 0, 0 };
	const auto Stride = sizeof(Source);
	const auto Size = static_cast<UINT32>(Stride * 1);
	CreateAndCopyToDefaultResource(IndirectBufferResources.back(), COM_PTR_GET(CommandAllocators[0]), COM_PTR_GET(GraphicsCommandLists[0]), Size, &Source);

	IndirectCommandSignatures.resize(1);
	const std::array<D3D12_INDIRECT_ARGUMENT_DESC, 1> IADs = {
		{ D3D12_INDIRECT_ARGUMENT_TYPE_DRAW_INDEXED },
	};
	const D3D12_COMMAND_SIGNATURE_DESC CSD = {
		Stride,
		static_cast<const UINT>(IADs.size()), IADs.data(),
		0
	};
	Device->CreateCommandSignature(&CSD, /*COM_PTR_GET(RootSignatures[0])*/nullptr, COM_PTR_UUIDOF_PUTVOID(IndirectCommandSignatures[0]));
}

void DXExt::CreateIndirectBuffer_Dispatch(const UINT X, const UINT Y, const UINT Z)
{
	IndirectBufferResources.push_back(COM_PTR<ID3D12Resource>());

	const D3D12_DISPATCH_ARGUMENTS Source = { X, Y, Z };
	const auto Stride = sizeof(Source);
	const auto Size = static_cast<UINT32>(Stride * 1);
	CreateAndCopyToDefaultResource(IndirectBufferResources.back(), COM_PTR_GET(CommandAllocators[0]), COM_PTR_GET(GraphicsCommandLists[0]), Size, &Source);

	IndirectCommandSignatures.resize(1);
	const std::array<D3D12_INDIRECT_ARGUMENT_DESC, 1> IADs = {
		{ D3D12_INDIRECT_ARGUMENT_TYPE_DISPATCH },
	};
	const D3D12_COMMAND_SIGNATURE_DESC CSD = {
		Stride,
		static_cast<const UINT>(IADs.size()), IADs.data(),
		0
	};
	Device->CreateCommandSignature(&CSD, /*COM_PTR_GET(RootSignatures[0])*/nullptr, COM_PTR_UUIDOF_PUTVOID(IndirectCommandSignatures[0]));
}

void DXExt::CreateShaderBlob_VsPs()
{
	const auto ShaderPath = GetBasePath();
	ShaderBlobs.push_back(COM_PTR<ID3DBlob>());
	VERIFY_SUCCEEDED(D3DReadFileToBlob((ShaderPath + TEXT(".vs.cso")).data(), COM_PTR_PUT(ShaderBlobs.back())));
	ShaderBlobs.push_back(COM_PTR<ID3DBlob>());
	VERIFY_SUCCEEDED(D3DReadFileToBlob((ShaderPath + TEXT(".ps.cso")).data(), COM_PTR_PUT(ShaderBlobs.back())));
}
void DXExt::CreateShaderBlob_VsPsDsHsGs()
{
	const auto ShaderPath = GetBasePath();
	ShaderBlobs.push_back(COM_PTR<ID3DBlob>());
	VERIFY_SUCCEEDED(D3DReadFileToBlob((ShaderPath + TEXT(".vs.cso")).data(), COM_PTR_PUT(ShaderBlobs.back())));
	ShaderBlobs.push_back(COM_PTR<ID3DBlob>());
	VERIFY_SUCCEEDED(D3DReadFileToBlob((ShaderPath + TEXT(".ps.cso")).data(), COM_PTR_PUT(ShaderBlobs.back())));
	ShaderBlobs.push_back(COM_PTR<ID3DBlob>());
	VERIFY_SUCCEEDED(D3DReadFileToBlob((ShaderPath + TEXT(".ds.cso")).data(), COM_PTR_PUT(ShaderBlobs.back())));
	ShaderBlobs.push_back(COM_PTR<ID3DBlob>());
	VERIFY_SUCCEEDED(D3DReadFileToBlob((ShaderPath + TEXT(".hs.cso")).data(), COM_PTR_PUT(ShaderBlobs.back())));
	ShaderBlobs.push_back(COM_PTR<ID3DBlob>());
	VERIFY_SUCCEEDED(D3DReadFileToBlob((ShaderPath + TEXT(".gs.cso")).data(), COM_PTR_PUT(ShaderBlobs.back())));
}
void DXExt::CreateShaderBlob_Cs()
{
	const auto ShaderPath = GetBasePath();
	ShaderBlobs.push_back(COM_PTR<ID3DBlob>());
	VERIFY_SUCCEEDED(D3DReadFileToBlob((ShaderPath + TEXT(".cs.cso")).data(), COM_PTR_PUT(ShaderBlobs.back())));
}

void DXExt::CreatePipelineState(const D3D12_PRIMITIVE_TOPOLOGY_TYPE Topology, 
	const COM_PTR<ID3DBlob> Vs, const COM_PTR<ID3DBlob> Ps, COM_PTR<ID3DBlob> Ds, COM_PTR<ID3DBlob> Hs, COM_PTR<ID3DBlob> Gs)
{
	PipelineStates.resize(1);

#ifdef USE_PIPELINE_SERIALIZE
	PipelineLibrarySerializer PLS(COM_PTR_GET(Device), GetBasePath() + TEXT(".plo"));
#endif

	std::vector<std::thread> Threads;

	Threads.push_back(std::thread::thread([&](COM_PTR<ID3D12PipelineState>& PST, ID3D12RootSignature* RS,
		const D3D12_SHADER_BYTECODE VS, const D3D12_SHADER_BYTECODE PS, const D3D12_SHADER_BYTECODE DS, const D3D12_SHADER_BYTECODE HS, const D3D12_SHADER_BYTECODE GS)
		{
#ifdef USE_PIPELINE_SERIALIZE
			DX::CreatePipelineState(PST, COM_PTR_GET(Device), RS, Topology, VS, PS, DS, HS, GS, {}, &PLS, TEXT("0"));
#else
			DX::CreatePipelineState(PST, COM_PTR_GET(Device), RS, , Topology, VS, PS, DS, HS, GS, {});
#endif
		},
		std::ref(PipelineStates[0]), COM_PTR_GET(RootSignatures[0]), 
			D3D12_SHADER_BYTECODE({ Vs->GetBufferPointer(), Vs->GetBufferSize() }),
			D3D12_SHADER_BYTECODE({ Ps->GetBufferPointer(), Ps->GetBufferSize() }),
			D3D12_SHADER_BYTECODE({ nullptr == Ds ? nullptr : Ds->GetBufferPointer(), nullptr == Ds ? 0 : Ds->GetBufferSize() }),
			D3D12_SHADER_BYTECODE({ nullptr == Hs ? nullptr : Hs->GetBufferPointer(), nullptr == Hs ? 0 : Hs->GetBufferSize() }),
			D3D12_SHADER_BYTECODE({ nullptr == Gs ? nullptr : Gs->GetBufferPointer(), nullptr == Gs ? 0 : Gs->GetBufferSize() })));

	//std::thread g(DXExt::CreatePipelineState_G, std::ref(PipelineStates[0]), COM_PTR_GET(RootSignatures[0]), Topology,
	//	D3D12_SHADER_BYTECODE({ Vs->GetBufferPointer(), Vs->GetBufferSize() }),
	//	D3D12_SHADER_BYTECODE({ Ps->GetBufferPointer(), Ps->GetBufferSize() }),
	//	D3D12_SHADER_BYTECODE({ nullptr == Ds ? nullptr : Ds->GetBufferPointer(), nullptr == Ds ? 0 : Ds->GetBufferSize() }),
	//	D3D12_SHADER_BYTECODE({ nullptr == Hs ? nullptr : Hs->GetBufferPointer(), nullptr == Hs ? 0 : Hs->GetBufferSize() }),
	//	D3D12_SHADER_BYTECODE({ nullptr == Gs ? nullptr : Gs->GetBufferPointer(), nullptr == Gs ? 0 : Gs->GetBufferSize() }));
	//std::thread l(&DXExt::CreatePipelineState_L, this, std::ref(PipelineStates[0]), COM_PTR_GET(RootSignatures[0]), Topology,
	//	D3D12_SHADER_BYTECODE({ Vs->GetBufferPointer(), Vs->GetBufferSize() }),
	//	D3D12_SHADER_BYTECODE({ Ps->GetBufferPointer(), Ps->GetBufferSize() }),
	//	D3D12_SHADER_BYTECODE({ nullptr == Ds ? nullptr : Ds->GetBufferPointer(), nullptr == Ds ? 0 : Ds->GetBufferSize() }),
	//	D3D12_SHADER_BYTECODE({ nullptr == Hs ? nullptr : Hs->GetBufferPointer(), nullptr == Hs ? 0 : Hs->GetBufferSize() }),
	//	D3D12_SHADER_BYTECODE({ nullptr == Gs ? nullptr : Gs->GetBufferPointer(), nullptr == Gs ? 0 : Gs->GetBufferSize() }));

	for (auto& i : Threads) { i.join(); }
}

//void DXExt::Clear_Color(ID3D12GraphicsCommandList* GraphicsCommandList)
//{
//	auto CPUDescriptorHandle(SwapChainDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
//	const auto IncrementSize = Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
//	CPUDescriptorHandle.ptr += CurrentBackBufferIndex * IncrementSize;
//	GraphicsCommandList->ClearRenderTargetView(CPUDescriptorHandle, DirectX::Colors::SkyBlue, 0, nullptr);
//}
//void DXExt::Clear_Depth(ID3D12GraphicsCommandList* GraphicsCommandList)
//{
//	if (nullptr != DepthStencilDescriptorHeap) {
//		auto CPUDescriptorHandle(DepthStencilDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
//		const auto IncrementSize = Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
//		CPUDescriptorHandle.ptr += 0 * IncrementSize;
//		GraphicsCommandList->ClearDepthStencilView(CPUDescriptorHandle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);
//	}
//}