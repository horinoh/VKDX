#include "stdafx.h"

#include "DXExt.h"

void DXExt::CreateIndirectBuffer_Vertices(const UINT Count)
{
	const D3D12_DRAW_ARGUMENTS Arguments = { 
		Count, 1, 0, 0 
	};
	const auto Stride = sizeof(Arguments);
	const auto Size = static_cast<UINT32>(Stride * 1);

	[&](ID3D12Resource** Resource, const UINT32 Size, const void* Data, ID3D12CommandAllocator* CA, ID3D12GraphicsCommandList* CL) {
		//!< アップロード用のリソースを作成、データをコピー Create upload resource, and copy data
		Microsoft::WRL::ComPtr<ID3D12Resource> UploadResource;
		CreateUploadResource(UploadResource.GetAddressOf(), Size);
		CopyToUploadResource(UploadResource.Get(), Size, Data);

		//!< デフォルトのリソースを作成 Create default resource
		CreateDefaultResource(Resource, Size);

		//!< アップロードリソースからデフォルトリソースへのコピーコマンドを発行 Execute copy command upload resource to default resource
		ExecuteCopyBuffer(CA, CL, UploadResource.Get(), *Resource, Size);
	}(IndirectBufferResource.GetAddressOf(), Size, &Arguments, CommandAllocators[0].Get(), GraphicsCommandLists[0].Get());

	const std::vector<D3D12_INDIRECT_ARGUMENT_DESC> IndirectArgumentDescs = {
		{ D3D12_INDIRECT_ARGUMENT_TYPE_DRAW },
	};
	const D3D12_COMMAND_SIGNATURE_DESC CommandSignatureDesc = {
		Stride,
		static_cast<const UINT>(IndirectArgumentDescs.size()), IndirectArgumentDescs.data(),
		0
	};
	Device->CreateCommandSignature(&CommandSignatureDesc, RootSignature.Get(), IID_PPV_ARGS(IndirectCommandSignature.GetAddressOf()));
}

void DXExt::CreateIndirectBuffer_Indexed(const UINT Count)
{
	const D3D12_DRAW_INDEXED_ARGUMENTS Arguments = { 
		Count, 1, 0, 0, 0 
	};
	const auto Stride = sizeof(Arguments);
	const auto Size = static_cast<UINT32>(Stride * 1);

	[&](ID3D12Resource** Resource, const UINT32 Size, const void* Data, ID3D12CommandAllocator* CA, ID3D12GraphicsCommandList* CL) {
		Microsoft::WRL::ComPtr<ID3D12Resource> UploadResource;
		CreateUploadResource(UploadResource.GetAddressOf(), Size);
		CopyToUploadResource(UploadResource.Get(), Size, Data);

		CreateDefaultResource(Resource, Size);

		ExecuteCopyBuffer(CA, CL, UploadResource.Get(), *Resource, Size);
	}(IndirectBufferResource.GetAddressOf(), Size, &Arguments, CommandAllocators[0].Get(), GraphicsCommandLists[0].Get());

	const std::vector<D3D12_INDIRECT_ARGUMENT_DESC> IndirectArgumentDescs = {
		{ D3D12_INDIRECT_ARGUMENT_TYPE_DRAW_INDEXED },
	};
	const D3D12_COMMAND_SIGNATURE_DESC CommandSignatureDesc = {
		Stride,
		static_cast<const UINT>(IndirectArgumentDescs.size()), IndirectArgumentDescs.data(),
		0
	};
	Device->CreateCommandSignature(&CommandSignatureDesc, RootSignature.Get(), IID_PPV_ARGS(IndirectCommandSignature.GetAddressOf()));
}

void DXExt::CreateIndirectBuffer_Dispatch(const UINT X, const UINT Y, const UINT Z)
{
	const D3D12_DISPATCH_ARGUMENTS Arguments = {
		X, Y, Z
	};
	const auto Stride = sizeof(Arguments);
	const auto Size = static_cast<UINT32>(Stride * 1);

	[&](ID3D12Resource** Resource, const UINT32 Size, const void* Data, ID3D12CommandAllocator* CA, ID3D12GraphicsCommandList* CL) {
		Microsoft::WRL::ComPtr<ID3D12Resource> UploadResource;
		CreateUploadResource(UploadResource.GetAddressOf(), Size);
		CopyToUploadResource(UploadResource.Get(), Size, Data);

		CreateDefaultResource(Resource, Size);

		ExecuteCopyBuffer(CA, CL, UploadResource.Get(), *Resource, Size);
	}(IndirectBufferResource.GetAddressOf(), Size, &Arguments, CommandAllocators[0].Get(), GraphicsCommandLists[0].Get()); 

	const std::vector<D3D12_INDIRECT_ARGUMENT_DESC> IndirectArgumentDescs = {
		{ D3D12_INDIRECT_ARGUMENT_TYPE_DISPATCH },
	};
	const D3D12_COMMAND_SIGNATURE_DESC CommandSignatureDesc = {
		Stride,
		static_cast<const UINT>(IndirectArgumentDescs.size()), IndirectArgumentDescs.data(),
		0
	};
	Device->CreateCommandSignature(&CommandSignatureDesc, RootSignature.Get(), IID_PPV_ARGS(IndirectCommandSignature.GetAddressOf()));
}

void DXExt::CreateStaticSamplerDesc_LW(D3D12_STATIC_SAMPLER_DESC& StaticSamplerDesc, const D3D12_SHADER_VISIBILITY ShaderVisibility, const FLOAT MaxLOD) const
{
	StaticSamplerDesc = {
		D3D12_FILTER_MIN_MAG_MIP_LINEAR,
			D3D12_TEXTURE_ADDRESS_MODE_WRAP, D3D12_TEXTURE_ADDRESS_MODE_WRAP, D3D12_TEXTURE_ADDRESS_MODE_WRAP,
			0.0f,
			0,
			D3D12_COMPARISON_FUNC_NEVER,
			D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE,
			0.0f, MaxLOD,
			0, 0, ShaderVisibility //!< UINT ShaderRegister, UINT RegisterSpace, D3D12_SHADER_VISIBILITY ShaderVisibility
	};
}

void DXExt::CreateShader_VsPs(std::vector<Microsoft::WRL::ComPtr<ID3DBlob>>& ShaderBlobs) const
{
	const auto ShaderPath = GetBasePath();
	ShaderBlobs.resize(2);
	D3DReadFileToBlob((ShaderPath + TEXT(".vs.cso")).data(), ShaderBlobs[0].GetAddressOf());
	D3DReadFileToBlob((ShaderPath + TEXT(".ps.cso")).data(), ShaderBlobs[1].GetAddressOf());
}
void DXExt::CreateShader_VsPsDsHsGs(std::vector<Microsoft::WRL::ComPtr<ID3DBlob>>& ShaderBlobs) const
{
	const auto ShaderPath = GetBasePath();
	ShaderBlobs.resize(5);
	D3DReadFileToBlob((ShaderPath + TEXT(".vs.cso")).data(), ShaderBlobs[0].GetAddressOf());
	D3DReadFileToBlob((ShaderPath + TEXT(".ps.cso")).data(), ShaderBlobs[1].GetAddressOf());
	D3DReadFileToBlob((ShaderPath + TEXT(".ds.cso")).data(), ShaderBlobs[2].GetAddressOf());
	D3DReadFileToBlob((ShaderPath + TEXT(".hs.cso")).data(), ShaderBlobs[3].GetAddressOf());
	D3DReadFileToBlob((ShaderPath + TEXT(".gs.cso")).data(), ShaderBlobs[4].GetAddressOf());	
}

void DXExt::CreateShader_Cs(std::vector<Microsoft::WRL::ComPtr<ID3DBlob>>& ShaderBlobs) const
{
	const auto ShaderPath = GetBasePath();
	ShaderBlobs.resize(1);
	D3DReadFileToBlob((ShaderPath + TEXT(".cs.cso")).data(), ShaderBlobs[0].GetAddressOf());
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