#include "stdafx.h"

#include "DXExt.h"

void DXExt::CreateIndirectBuffer_Draw(const UINT Count)
{
	const D3D12_DRAW_ARGUMENTS Source = { Count, 1, 0, 0 };
	const auto Stride = sizeof(Source);
	const auto Size = static_cast<UINT32>(Stride * 1);
#ifdef USE_WINRT
	const auto CA = CommandAllocators[0].get();
	const auto CL = GraphicsCommandLists[0].get(); 
#elif defined(USE_WRL)
	const auto CA = CommandAllocators[0].Get();
	const auto CL = GraphicsCommandLists[0].Get(); 
#endif
	
#ifdef USE_WINRT
	CreateIndirectBuffer(IndirectBufferResource.put(), Size, &Source, CA, CL);
#elif defined(USE_WRL)
	CreateIndirectBuffer(IndirectBufferResource.GetAddressOf(), Size, &Source, CA, CL);
#endif

	const std::vector<D3D12_INDIRECT_ARGUMENT_DESC> IndArgDescs = {
		{ D3D12_INDIRECT_ARGUMENT_TYPE_DRAW },
	};
	const D3D12_COMMAND_SIGNATURE_DESC CmdSigDesc = {
		Stride,
		static_cast<const UINT>(IndArgDescs.size()), IndArgDescs.data(),
		0
	};
#ifdef USE_WINRT
	Device->CreateCommandSignature(&CmdSigDesc, RootSignature.get(), __uuidof(IndirectCommandSignature), IndirectCommandSignature.put_void());
#elif defined(USE_WRL)
	Device->CreateCommandSignature(&CmdSigDesc, RootSignature.Get(), IID_PPV_ARGS(IndirectCommandSignature.GetAddressOf()));
#endif
}

void DXExt::CreateIndirectBuffer_DrawIndexed(const UINT Count)
{
	const D3D12_DRAW_INDEXED_ARGUMENTS Source = { Count, 1, 0, 0, 0 };
	const auto Stride = sizeof(Source);
	const auto Size = static_cast<UINT32>(Stride * 1);

#ifdef USE_WINRT
	const auto CA = CommandAllocators[0].get();
	const auto CL = GraphicsCommandLists[0].get();
#elif defined(USE_WRL)
	const auto CA = CommandAllocators[0].Get();
	const auto CL = GraphicsCommandLists[0].Get();
#endif

#ifdef USE_WINRT
	CreateIndirectBuffer(IndirectBufferResource.put(), Size, &Source, CA, CL);
#elif defined(USE_WRL)
	CreateIndirectBuffer(IndirectBufferResource.GetAddressOf(), Size, &Source, CA, CL);
#endif

	const std::vector<D3D12_INDIRECT_ARGUMENT_DESC> IndArgDescs = {
		{ D3D12_INDIRECT_ARGUMENT_TYPE_DRAW_INDEXED },
	};
	const D3D12_COMMAND_SIGNATURE_DESC CmdSigDesc = {
		Stride,
		static_cast<const UINT>(IndArgDescs.size()), IndArgDescs.data(),
		0
	};
#ifdef USE_WINRT
	Device->CreateCommandSignature(&CmdSigDesc, RootSignature.get(), __uuidof(IndirectCommandSignature), IndirectCommandSignature.put_void());
#elif defined(USE_WRL)
	Device->CreateCommandSignature(&CmdSigDesc, RootSignature.Get(), IID_PPV_ARGS(IndirectCommandSignature.GetAddressOf()));
#endif
}

void DXExt::CreateIndirectBuffer_Dispatch(const UINT X, const UINT Y, const UINT Z)
{
	const D3D12_DISPATCH_ARGUMENTS Source = { X, Y, Z };
	const auto Stride = sizeof(Source);
	const auto Size = static_cast<UINT32>(Stride * 1);
#ifdef USE_WINRT
	const auto CA = CommandAllocators[0].get();
	const auto CL = GraphicsCommandLists[0].get();
#elif defined(USE_WRL)
	const auto CA = CommandAllocators[0].Get();
	const auto CL = GraphicsCommandLists[0].Get();
#endif

#ifdef USE_WINRT
	CreateIndirectBuffer(IndirectBufferResource.put(), Size, &Source, CA, CL);
#elif defined(USE_WRL)
	CreateIndirectBuffer(IndirectBufferResource.GetAddressOf(), Size, &Source, CA, CL);
#endif

	const std::vector<D3D12_INDIRECT_ARGUMENT_DESC> IndArgDescs = {
		{ D3D12_INDIRECT_ARGUMENT_TYPE_DISPATCH },
	};
	const D3D12_COMMAND_SIGNATURE_DESC CmdSigDesc = {
		Stride,
		static_cast<const UINT>(IndArgDescs.size()), IndArgDescs.data(),
		0
	};
#ifdef USE_WINRT
	Device->CreateCommandSignature(&CmdSigDesc, RootSignature.get(), __uuidof(IndirectCommandSignature), IndirectCommandSignature.put_void());
#elif defined(USE_WRL)
	Device->CreateCommandSignature(&CmdSigDesc, RootSignature.Get(), IID_PPV_ARGS(IndirectCommandSignature.GetAddressOf()));
#endif
}

void DXExt::CreateStaticSamplerDesc_LW(D3D12_STATIC_SAMPLER_DESC& StaticSamplerDesc, const D3D12_SHADER_VISIBILITY ShaderVisibility, const FLOAT MaxLOD) const
{
	//!< シェーダ内での記述例 SamplerState Sampler : register(s0, space0);
	//!< DXには正規化座標の設定は無く、シェーダビジビリティトレジスタの設定がある
	StaticSamplerDesc = {
			D3D12_FILTER_MIN_MAG_MIP_LINEAR, // min, mag, mip
			D3D12_TEXTURE_ADDRESS_MODE_WRAP, D3D12_TEXTURE_ADDRESS_MODE_WRAP, D3D12_TEXTURE_ADDRESS_MODE_WRAP, // u, v, w
			0.0f, // lod bias
			0, // anisotropy
			D3D12_COMPARISON_FUNC_NEVER, // compare
			D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE, // border
			0.0f, MaxLOD, // min, maxlod
			0, 0, ShaderVisibility //!< UINT ShaderRegister, UINT RegisterSpace, D3D12_SHADER_VISIBILITY ShaderVisibility
	};
}

#ifdef USE_WINRT
void DXExt::CreateShader_VsPs(std::vector<winrt::com_ptr<ID3DBlob>>& ShaderBlobs) const
#elif defined(USE_WRL)
void DXExt::CreateShader_VsPs(std::vector<Microsoft::WRL::ComPtr<ID3DBlob>>& ShaderBlobs) const
#endif
{
	const auto ShaderPath = GetBasePath();
	ShaderBlobs.resize(2);
#ifdef USE_WINRT
	D3DReadFileToBlob((ShaderPath + TEXT(".vs.cso")).data(), ShaderBlobs[0].put());
	D3DReadFileToBlob((ShaderPath + TEXT(".ps.cso")).data(), ShaderBlobs[1].put()); 
#elif defined(USE_WRL)
	D3DReadFileToBlob((ShaderPath + TEXT(".vs.cso")).data(), ShaderBlobs[0].GetAddressOf());
	D3DReadFileToBlob((ShaderPath + TEXT(".ps.cso")).data(), ShaderBlobs[1].GetAddressOf()); 
#endif
}
#ifdef USE_WINRT
void DXExt::CreateShader_VsPsDsHsGs(std::vector<winrt::com_ptr<ID3DBlob>>& ShaderBlobs) const
#elif defined(USE_WRL)
void DXExt::CreateShader_VsPsDsHsGs(std::vector<Microsoft::WRL::ComPtr<ID3DBlob>>& ShaderBlobs) const
#endif
{
	const auto ShaderPath = GetBasePath();
	ShaderBlobs.resize(5);
#ifdef USE_WINRT
	D3DReadFileToBlob((ShaderPath + TEXT(".vs.cso")).data(), ShaderBlobs[0].put());
	D3DReadFileToBlob((ShaderPath + TEXT(".ps.cso")).data(), ShaderBlobs[1].put());
	D3DReadFileToBlob((ShaderPath + TEXT(".ds.cso")).data(), ShaderBlobs[2].put());
	D3DReadFileToBlob((ShaderPath + TEXT(".hs.cso")).data(), ShaderBlobs[3].put());
	D3DReadFileToBlob((ShaderPath + TEXT(".gs.cso")).data(), ShaderBlobs[4].put());	
#elif defined(USE_WRL)
	D3DReadFileToBlob((ShaderPath + TEXT(".vs.cso")).data(), ShaderBlobs[0].GetAddressOf());
	D3DReadFileToBlob((ShaderPath + TEXT(".ps.cso")).data(), ShaderBlobs[1].GetAddressOf());
	D3DReadFileToBlob((ShaderPath + TEXT(".ds.cso")).data(), ShaderBlobs[2].GetAddressOf());
	D3DReadFileToBlob((ShaderPath + TEXT(".hs.cso")).data(), ShaderBlobs[3].GetAddressOf());
	D3DReadFileToBlob((ShaderPath + TEXT(".gs.cso")).data(), ShaderBlobs[4].GetAddressOf());	
#endif
}
#ifdef USE_WINRT
void DXExt::CreateShader_Cs(std::vector<winrt::com_ptr<ID3DBlob>>& ShaderBlobs) const
#elif defined(USE_WRL)
void DXExt::CreateShader_Cs(std::vector<Microsoft::WRL::ComPtr<ID3DBlob>>& ShaderBlobs) const
#endif
{
	const auto ShaderPath = GetBasePath();
	ShaderBlobs.resize(1);
#ifdef USE_WINRT
	D3DReadFileToBlob((ShaderPath + TEXT(".cs.cso")).data(), ShaderBlobs[0].put());
#elif defined(USE_WRL)
	D3DReadFileToBlob((ShaderPath + TEXT(".cs.cso")).data(), ShaderBlobs[0].GetAddressOf());
#endif
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