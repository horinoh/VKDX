#include "stdafx.h"

#include "DXExt.h"

void DXExt::CreateRootSignature_1ConstantBuffer(const D3D12_SHADER_VISIBILITY ShaderVisibility)
{
	using namespace Microsoft::WRL;

	const std::vector<D3D12_DESCRIPTOR_RANGE> DescriptorRanges = {
		{
			D3D12_DESCRIPTOR_RANGE_TYPE_CBV,
			1, //!< NumDescriptors
			0, //!< BaseShaderRegister ... HLSL ‚Å register(b0) ‚È‚ç 0Aregister(t3) ‚È‚ç 3 ‚Æ‚¢‚¤Š´‚¶
			0, //!< RegisterSpace ... ’Êí‚Í 0 ‚Å‚æ‚¢BHLSL ‚Å register(t3, space5) ‚È‚ç 5
			D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND //!< OffsetInDescriptorsFromTableStart ... ’Êí‚Í D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND ‚Å‚æ‚¢
		},
	};
	const D3D12_ROOT_DESCRIPTOR_TABLE DescriptorTable = {
		static_cast<UINT>(DescriptorRanges.size()), DescriptorRanges.data()
	};
	const std::vector<D3D12_ROOT_PARAMETER> RootParameters = {
		{
			D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE,
			DescriptorTable,
			ShaderVisibility
		},
	};
	const std::vector<D3D12_STATIC_SAMPLER_DESC> StaticSamplerDescs = {
	};
	const D3D12_ROOT_SIGNATURE_DESC RootSignatureDesc = {
		static_cast<UINT>(RootParameters.size()), RootParameters.data(),
		static_cast<UINT>(StaticSamplerDescs.size()), StaticSamplerDescs.data(),
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT
	};
	ComPtr<ID3DBlob> Blob;
	ComPtr<ID3DBlob> ErrorBlob;
	VERIFY_SUCCEEDED(D3D12SerializeRootSignature(&RootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, Blob.GetAddressOf(), ErrorBlob.GetAddressOf()));
	VERIFY_SUCCEEDED(Device->CreateRootSignature(0, Blob->GetBufferPointer(), Blob->GetBufferSize(), IID_PPV_ARGS(RootSignature.GetAddressOf())));

#ifdef _DEBUG
	std::cout << "CreateRootSignature" << COUT_OK << std::endl << std::endl;
#endif
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