#include "stdafx.h"

#include "DXExt.h"

void DXExt::CreateIndirectBuffer_4Vertices()
{
	const D3D12_DRAW_ARGUMENTS DrawArguments = { 4, 1, 0, 0 };
	const auto Stride = sizeof(DrawArguments);
	const auto Size = static_cast<UINT32>(Stride * 1);

	[&](ID3D12Resource** Resource, const UINT32 Size, const void* Data, ID3D12CommandAllocator* CA, ID3D12GraphicsCommandList* CL) {
		//!< �A�b�v���[�h�p�̃��\�[�X���쐬�A�f�[�^���R�s�[ Create upload resource, and copy data
		Microsoft::WRL::ComPtr<ID3D12Resource> UploadResource;
		CreateUploadResource(UploadResource.GetAddressOf(), Size);
		CopyToUploadResource(UploadResource.Get(), Size, Data);

		//!< �f�t�H���g�̃��\�[�X���쐬 Create default resource
		CreateDefaultResource(Resource, Size);

		//!< �A�b�v���[�h���\�[�X����f�t�H���g���\�[�X�ւ̃R�s�[�R�}���h�𔭍s Execute copy command upload resource to default resource
		ExecuteCopyBuffer(CA, CL, UploadResource.Get(), *Resource, Size);
	}(IndirectBufferResource.GetAddressOf(), Size, &DrawArguments, CommandAllocators[0].Get(), GraphicsCommandLists[0].Get());

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
void DXExt::CreateIndirectBuffer_Indexed()
{
	const D3D12_DRAW_INDEXED_ARGUMENTS DrawIndexedArguments = { IndexCount, 1, 0, 0, 0 };
	const auto Stride = sizeof(DrawIndexedArguments);
	const auto Size = static_cast<UINT32>(Stride * 1);

	[&](ID3D12Resource** Resource, const UINT32 Size, const void* Data, ID3D12CommandAllocator* CA, ID3D12GraphicsCommandList* CL) {
		//!< �A�b�v���[�h�p�̃��\�[�X���쐬�A�f�[�^���R�s�[ Create upload resource, and copy data
		Microsoft::WRL::ComPtr<ID3D12Resource> UploadResource;
		CreateUploadResource(UploadResource.GetAddressOf(), Size);
		CopyToUploadResource(UploadResource.Get(), Size, Data);

		//!< �f�t�H���g�̃��\�[�X���쐬 Create default resource
		CreateDefaultResource(Resource, Size);

		//!< �A�b�v���[�h���\�[�X����f�t�H���g���\�[�X�ւ̃R�s�[�R�}���h�𔭍s Execute copy command upload resource to default resource
		ExecuteCopyBuffer(CA, CL, UploadResource.Get(), *Resource, Size);
	}(IndirectBufferResource.GetAddressOf(), Size, &DrawIndexedArguments, CommandAllocators[0].Get(), GraphicsCommandLists[0].Get());

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

void DXExt::CreateSampler_LinearWrap(const D3D12_SHADER_VISIBILITY ShaderVisibility, const FLOAT MaxLOD)
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

void DXExt::CreateShader_VsPs(std::vector<Microsoft::WRL::ComPtr<ID3DBlob>>& ShaderBlobs, std::array<D3D12_SHADER_BYTECODE, 5>& ShaderBytecodes) const
{
	const auto ShaderPath = GetBasePath();
	ShaderBlobs.resize(2);
	D3DReadFileToBlob((ShaderPath + L".vs.cso").data(), ShaderBlobs[0].GetAddressOf());
	D3DReadFileToBlob((ShaderPath + L".ps.cso").data(), ShaderBlobs[1].GetAddressOf());
	const D3D12_SHADER_BYTECODE DefaultShaderBytecode = { nullptr, 0 };
	ShaderBytecodes = {
		D3D12_SHADER_BYTECODE({ ShaderBlobs[0]->GetBufferPointer(), ShaderBlobs[0]->GetBufferSize() }),
		D3D12_SHADER_BYTECODE({ ShaderBlobs[1]->GetBufferPointer(), ShaderBlobs[1]->GetBufferSize() }),
		DefaultShaderBytecode,
		DefaultShaderBytecode,
		DefaultShaderBytecode,
	};
}
void DXExt::CreateShader_VsPsDsHsGs(std::vector<Microsoft::WRL::ComPtr<ID3DBlob>>& ShaderBlobs, std::array<D3D12_SHADER_BYTECODE, 5>& ShaderBytecodes) const
{
	const auto ShaderPath = GetBasePath();
	ShaderBlobs.resize(5);
	D3DReadFileToBlob((ShaderPath + L".vs.cso").data(), ShaderBlobs[0].GetAddressOf());
	D3DReadFileToBlob((ShaderPath + L".ps.cso").data(), ShaderBlobs[1].GetAddressOf());
	D3DReadFileToBlob((ShaderPath + L".ds.cso").data(), ShaderBlobs[2].GetAddressOf());
	D3DReadFileToBlob((ShaderPath + L".hs.cso").data(), ShaderBlobs[3].GetAddressOf());
	D3DReadFileToBlob((ShaderPath + L".gs.cso").data(), ShaderBlobs[4].GetAddressOf());	ShaderBytecodes = {
		D3D12_SHADER_BYTECODE({ ShaderBlobs[0]->GetBufferPointer(), ShaderBlobs[0]->GetBufferSize() }),
		D3D12_SHADER_BYTECODE({ ShaderBlobs[1]->GetBufferPointer(), ShaderBlobs[1]->GetBufferSize() }),
		D3D12_SHADER_BYTECODE({ ShaderBlobs[2]->GetBufferPointer(), ShaderBlobs[2]->GetBufferSize() }),
		D3D12_SHADER_BYTECODE({ ShaderBlobs[3]->GetBufferPointer(), ShaderBlobs[3]->GetBufferSize() }),
		D3D12_SHADER_BYTECODE({ ShaderBlobs[4]->GetBufferPointer(), ShaderBlobs[4]->GetBufferSize() }),
	};
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