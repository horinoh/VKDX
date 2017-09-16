#pragma once

#include "DX.h"

class DXExt : public DX
{
private:
	using Super = DX;
public:
	using Vertex_Position = struct Vertex_Position { DirectX::XMFLOAT3 Position; };
	using Vertex_PositionColor = struct Vertex_PositionColor { DirectX::XMFLOAT3 Position; DirectX::XMFLOAT4 Color; };
	
	void CreateIndirectBuffer_4Vertices();
	void CreateIndirectBuffer_Indexed();

	void CreateRootParameters_DT(std::vector<D3D12_ROOT_PARAMETER>& RootParameters, const std::vector<D3D12_DESCRIPTOR_RANGE>& DescriptorRanges, const D3D12_SHADER_VISIBILITY ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL) const {
		RootParameters.push_back({ D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE, { static_cast<UINT>(DescriptorRanges.size()), DescriptorRanges.data() }, ShaderVisibility });
	}

	//!< �P�̃R���X�^���g�o�b�t�@
	void CreateDescriptorRanges_1CBV(std::vector<D3D12_DESCRIPTOR_RANGE>& DescriptorRanges) const {
		DescriptorRanges.push_back({ D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND});
	}
	template<typename T>
	void CreateDescriptorHeap_1CBV(const T& Type) {
		const auto Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		const auto Count = 1;
		const auto Size = RoundUpTo256(sizeof(T));

		[&](const D3D12_DESCRIPTOR_HEAP_TYPE Type, const UINT Count, ID3D12DescriptorHeap** DescriptorHeap) {
			const D3D12_DESCRIPTOR_HEAP_DESC DescriptorHeapDesc = {
				Type,
				Count,
				D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE,
				0 // NodeMask ... �}���`GPU�̏ꍇ Use with multi GPU
			};
			VERIFY_SUCCEEDED(Device->CreateDescriptorHeap(&DescriptorHeapDesc, IID_PPV_ARGS(DescriptorHeap)));
		}(Type, Count, ConstantBufferDescriptorHeap.GetAddressOf());

		[&](const D3D12_DESCRIPTOR_HEAP_TYPE Type, ID3D12Resource* Resource, ID3D12DescriptorHeap* DescriptorHeap, const UINT Size) {
			//!< 256�A���C������Ă��邱�� Must be 256 aligned
			const D3D12_CONSTANT_BUFFER_VIEW_DESC ConstantBufferViewDesc = {
				Resource->GetGPUVirtualAddress(),
				Size
			};
			//!< �f�X�N���v�^(�r���[)�̍쐬�B���\�[�X��ł̃I�t�Z�b�g���w�肵�č쐬���Ă���A���ʂ��ϐ��ɕԂ�킯�ł͂Ȃ�
			const auto CDH = GetCPUDescriptorHandle(DescriptorHeap, Type);
			Device->CreateConstantBufferView(&ConstantBufferViewDesc, CDH);
		}(Type, ConstantBufferResource.Get(), ConstantBufferDescriptorHeap.Get(), Size);
#ifdef DEBUG_STDOUT
		std::cout << "CreateDescriptorHeap" << COUT_OK << std::endl << std::endl;
#endif
	}

	//!< �P�̃V�F�[�_���\�[�X�r���[
	void CreateDescriptorRanges_1SRV(std::vector<D3D12_DESCRIPTOR_RANGE>& DescriptorRanges) const { 
		DescriptorRanges.push_back({ D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND }); 
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

		CreateUploadResource(ConstantBufferResource.GetAddressOf(), Size);
		CopyToUploadResource(ConstantBufferResource.Get(), Size, &Type);

#ifdef _DEBUG
		std::cout << "CreateConstantBuffer" << COUT_OK << std::endl << std::endl;
#endif
	}
};
