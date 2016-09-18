#pragma once

#include "DX.h"

class DXExt : public DX
{
private:
	using Super = DX;
public:
	using Vertex_Position = struct Vertex_Position { DirectX::XMFLOAT3 Position; };
	using Vertex_PositionColor = struct Vertex_PositionColor { DirectX::XMFLOAT3 Position; DirectX::XMFLOAT4 Color; };

	virtual void CreateRootSignature_1ConstantBuffer(const D3D12_SHADER_VISIBILITY ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL);

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

	virtual void CreateShader_VsPs(std::vector<Microsoft::WRL::ComPtr<ID3DBlob>>& ShaderBlobs, std::array<D3D12_SHADER_BYTECODE, 5>& ShaderBytecodes) const;
	virtual void CreateShader_VsPsDsHsGs(std::vector<Microsoft::WRL::ComPtr<ID3DBlob>>& ShaderBlobs, std::array<D3D12_SHADER_BYTECODE, 5>& ShaderBytecodes) const;

	template<typename T>
	void CreateConstantBuffer(const T& Type) {
		const auto Size = RoundUpTo256(sizeof(T));
		CreateUploadBuffer(ConstantBufferResource.GetAddressOf(), Size, &Type);
		CreateConstantBufferDescriptorHeap(static_cast<UINT>(Size));
#ifdef _DEBUG
		std::cout << "CreateConstantBuffer" << COUT_OK << std::endl << std::endl;
#endif
	}
};
