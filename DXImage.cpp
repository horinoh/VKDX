#include "stdafx.h"

#include "DXImage.h"

#pragma comment(lib, "DirectXTK12.lib")

void DXImage::LoadDDS(const std::wstring& Path)
{
	Microsoft::WRL::ComPtr<ID3D12Resource> Resource;
	std::unique_ptr<uint8_t[]> DDSData;
	std::vector<D3D12_SUBRESOURCE_DATA> SubresourceData;
	VERIFY_SUCCEEDED(DirectX::LoadDDSTextureFromFile(Device.Get(), Path.c_str(), Resource.GetAddressOf(), DDSData, SubresourceData));

#if 0
	const UINT64 uploadBufferSize = GetRequiredIntermediateSize(tex.Get(), 0,
		static_cast<UINT>(subresources.size()));

	// Create the GPU upload buffer.
	ComPtr<ID3D12Resource> uploadHeap;
	DX::ThrowIfFailed(
		device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize),
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(uploadHeap.GetAddressOf())));

	UpdateSubresources(commandList, tex.Get(), uploadHeap.Get(),
		0, 0, static_cast<UINT>(subresources.size()), subresources.data());
	commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(tex.Get(),
		D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));

	DX::ThrowIfFailed(commandList->Close());
	m_deviceResources->GetCommandQueue()->ExecuteCommandLists(1,
		CommandListCast(&commandList));

	// Wait until assets have been uploaded to the GPU.
	m_deviceResources->WaitForGpu();
#endif
}
