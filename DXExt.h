#pragma once

#include "DX.h"

class DXExt : public DX
{
private:
	using Super = DX;
public:
	using Vertex_Position = struct Vertex_Position { DirectX::XMFLOAT3 Position; };
	using Vertex_PositionColor = struct Vertex_PositionColor { DirectX::XMFLOAT3 Position; DirectX::XMFLOAT4 Color; };
	
	void CreateIndirectBuffer_Indirect4Vertices(ID3D12CommandAllocator* CommandAllocator, ID3D12GraphicsCommandList* CommandList);
	void CreateIndirectBuffer_IndexedIndirect(ID3D12CommandAllocator* CommandAllocator, ID3D12GraphicsCommandList* CommandList);

	void CreateDescriptorRanges_1CBV(std::vector<D3D12_DESCRIPTOR_RANGE>& DescriptorRanges) const {
		DescriptorRanges.push_back({ D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND});
	}
	void CreateRootParameters_1CBV(std::vector<D3D12_ROOT_PARAMETER>& RootParameters, const std::vector<D3D12_DESCRIPTOR_RANGE>& DescriptorRanges, const D3D12_SHADER_VISIBILITY ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL) const {
		const D3D12_ROOT_DESCRIPTOR_TABLE RootDescriptorTable = {
			static_cast<UINT>(DescriptorRanges.size()), DescriptorRanges.data()
		};
		RootParameters.push_back({ D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE, RootDescriptorTable, ShaderVisibility });
	}

	void CreateDescriptorRanges_1SRV(std::vector<D3D12_DESCRIPTOR_RANGE>& DescriptorRanges) const { 
		DescriptorRanges.push_back({ D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND }); 
	}
	void CreateRootParameters_1SRV(std::vector<D3D12_ROOT_PARAMETER>& RootParameters, const std::vector<D3D12_DESCRIPTOR_RANGE>& DescriptorRanges, const D3D12_SHADER_VISIBILITY ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL) const {
		const D3D12_ROOT_DESCRIPTOR_TABLE RootDescriptorTable = {
			static_cast<UINT>(DescriptorRanges.size()), DescriptorRanges.data()
		};
		RootParameters.push_back({ D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE, RootDescriptorTable, ShaderVisibility });
	}
	
	void CreateSampler_LinearWrap(const D3D12_SHADER_VISIBILITY ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL, const FLOAT MaxLOD = (std::numeric_limits<FLOAT>::max)());

	template<typename T>
	void CreateInputLayoutT(std::vector<D3D12_INPUT_ELEMENT_DESC>& InputElementDescs, const UINT InputSlot) const {}
	template<>
	void CreateInputLayoutT<Vertex_Position>(std::vector<D3D12_INPUT_ELEMENT_DESC>& InputElementDescs, const UINT InputSlot) const {
		InputElementDescs = {
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, InputSlot, offsetof(Vertex_Position, Position), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		};
	}
	template<>
	void CreateInputLayoutT<Vertex_PositionColor>(std::vector<D3D12_INPUT_ELEMENT_DESC>& InputElementDescs, const UINT InputSlot) const {
		InputElementDescs = {
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, InputSlot, offsetof(Vertex_PositionColor, Position), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, InputSlot, offsetof(Vertex_PositionColor, Color), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
		};
	}

	void CreateShader_VsPs(std::vector<Microsoft::WRL::ComPtr<ID3DBlob>>& ShaderBlobs, std::array<D3D12_SHADER_BYTECODE, 5>& ShaderBytecodes) const;
	void CreateShader_VsPsDsHsGs(std::vector<Microsoft::WRL::ComPtr<ID3DBlob>>& ShaderBlobs, std::array<D3D12_SHADER_BYTECODE, 5>& ShaderBytecodes) const;

	template<typename T>
	void CreateConstantBuffer(const T& Type) {
		const auto Size = RoundUpTo256(sizeof(T));
		CreateUploadResource(ConstantBufferResource.GetAddressOf(), Size, &Type);
		CreateConstantBufferDescriptorHeap(static_cast<UINT>(Size));
#ifdef _DEBUG
		std::cout << "CreateConstantBuffer" << COUT_OK << std::endl << std::endl;
#endif
	}
};
