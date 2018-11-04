#pragma once

#include "DX.h"

class DXExt : public DX
{
private:
	using Super = DX;
public:
	using Vertex_Position = struct Vertex_Position { DirectX::XMFLOAT3 Position; };
	using Vertex_PositionColor = struct Vertex_PositionColor { DirectX::XMFLOAT3 Position; DirectX::XMFLOAT4 Color; };
	
	void CreateIndirectBuffer_Draw(const UINT Count);
	void CreateIndirectBuffer_DrawIndexed(const UINT Count);
	void CreateIndirectBuffer_Dispatch(const UINT X, const UINT Y, const UINT Z);

	/*
	@brief �P�̃R���X�^���g�o�b�t�@ One constant buffer view
	*/
	void CreateRootParameters_1CBV(std::vector<D3D12_ROOT_PARAMETER>& RootParameters, const std::vector<D3D12_DESCRIPTOR_RANGE>& DescriptorRanges, const D3D12_SHADER_VISIBILITY ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL) const {
		RootParameters = {
			{
				D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE,
				{ 1, &DescriptorRanges[0] },
				ShaderVisibility
			},
		};
	}
	void CreateDescriptorRanges_1CBV(std::vector<D3D12_DESCRIPTOR_RANGE>& DescriptorRanges) const {
		DescriptorRanges = {
			{ 
				D3D12_DESCRIPTOR_RANGE_TYPE_CBV,
				1, //!< NumDescriptors �������̏ꍇ UINT_MAX ���w��A�������ɂł���͍̂Ō�̗v�f�̂�
				0, //!< b0 BaseShaderRegister �Ⴆ�� ": register(t3);" �� "3"
				0, //!< space0 RegisterSpace �ʏ��0�A�Ⴆ�� ": register(t3,space5);" �� "5"
				D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND
			},
		};
	}
	template<typename T>
	void CreateDescriptorHeap_1CBV() {
		const auto Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		const auto Count = 1;
		const auto Size = RoundUp(sizeof(T), 0xff); //!< 256�o�C�g�A���C��

		[&](const D3D12_DESCRIPTOR_HEAP_TYPE Type, const UINT Count, ID3D12DescriptorHeap** DescriptorHeap) {
			const D3D12_DESCRIPTOR_HEAP_DESC DescriptorHeapDesc = {
				Type,
				Count,
				D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE,
				0 //!< NodeMask ... �}���`GPU�̏ꍇ Use with multi GPU
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
		}(Type, ConstantBufferResource.Get(), ConstantBufferDescriptorHeap.Get(), static_cast<UINT>(Size));

#ifdef DEBUG_STDOUT
		std::cout << "CreateDescriptorHeap" << COUT_OK << std::endl << std::endl;
#endif
	}

	/*
	@brief �P�̃V�F�[�_���\�[�X�r���[ One shader resource view
	*/
	void CreateRootParameters_1SRV(std::vector<D3D12_ROOT_PARAMETER>& RootParameters, const std::vector<D3D12_DESCRIPTOR_RANGE>& DescriptorRanges, const D3D12_SHADER_VISIBILITY ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL) const {
		RootParameters = {
			{
				D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE,
				{ 1, &DescriptorRanges[0] },
				ShaderVisibility
			},
		};
	}
	void CreateDescriptorRanges_1SRV(std::vector<D3D12_DESCRIPTOR_RANGE>& DescriptorRanges) const {
		DescriptorRanges = {
			{ 
				D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 
				1, 
				0, //!< t0
				0, //!< space0
				D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND 
			},
		};
	}
	void CreateDescriptorHeap_1SRV() {
		const auto Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		const auto Count = 1;

		[&](const D3D12_DESCRIPTOR_HEAP_TYPE Type, const UINT Count, ID3D12DescriptorHeap** DescriptorHeap) {
			const D3D12_DESCRIPTOR_HEAP_DESC DescriptorHeapDesc = {
				Type,
				Count,
				D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE,
				0
			};
			VERIFY_SUCCEEDED(Device->CreateDescriptorHeap(&DescriptorHeapDesc, IID_PPV_ARGS(DescriptorHeap)));
		}(Type, Count, ImageDescriptorHeap.GetAddressOf());

		[&](const D3D12_DESCRIPTOR_HEAP_TYPE Type, ID3D12Resource* Resource, ID3D12DescriptorHeap* DescriptorHeap) {
			const auto CDH = GetCPUDescriptorHandle(DescriptorHeap, Type);
			Device->CreateShaderResourceView(Resource, nullptr, CDH);
		}(Type, ImageResource.Get(), ImageDescriptorHeap.Get());

#ifdef DEBUG_STDOUT
		std::cout << "CreateDescriptorHeap" << COUT_OK << std::endl << std::endl;
#endif
	}

	/*
	@brief �P�̃R���X�^���g�o�b�t�@�ƂP�̃V�F�[�_���\�[�X�r���[ One constant buffer view and one shader resource view
	*/
	void CreateRootParameters_1CBV_1SRV(std::vector<D3D12_ROOT_PARAMETER>& RootParameters, const std::vector<D3D12_DESCRIPTOR_RANGE>& DescriptorRanges, const D3D12_SHADER_VISIBILITY ShaderVisibility_CBV = D3D12_SHADER_VISIBILITY_ALL, const D3D12_SHADER_VISIBILITY ShaderVisibility_SRV = D3D12_SHADER_VISIBILITY_ALL) const {
		RootParameters = {
			{
				D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE,
				{ 1, &DescriptorRanges[0] },
				ShaderVisibility_CBV
			},
			{
				D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE,
				{ 1, &DescriptorRanges[1] },
				ShaderVisibility_SRV
			},
		};
	}
	void CreateDescriptorRanges_1CBV_1SRV(std::vector<D3D12_DESCRIPTOR_RANGE>& DescriptorRanges) const {
		DescriptorRanges = {
			{ 
				D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 
				1,
				0, //!< b0
				0, //!< space0
				D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND
			},
			{ 
				D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 
				1,
				0, //!< t0
				0, //!< space0
				D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND 
			},
		};
	}
	template<typename T>
	void CreateDescriptorHeap_1CBV_1SRV() {
		CreateDescriptorHeap_1CBV<T>();
		CreateDescriptorHeap_1SRV();
#ifdef DEBUG_STDOUT
		std::cout << "CreateDescriptorHeap" << COUT_OK << std::endl << std::endl;
#endif
	}
	
	/*
	@brief �P�̃A���I�[�_�[�h�A�N�Z�X�r���[ One unordered access view
	*/
	void CreateRootParameters_1UAV(std::vector<D3D12_ROOT_PARAMETER>& RootParameters, const std::vector<D3D12_DESCRIPTOR_RANGE>& DescriptorRanges, const D3D12_SHADER_VISIBILITY ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL) const {
		RootParameters = {
			{
				D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE,
				{ 1, &DescriptorRanges[0] },
				ShaderVisibility
			},
		};
	}
	void CreateDescriptorRanges_1UAV(std::vector<D3D12_DESCRIPTOR_RANGE>& DescriptorRanges) const {
		DescriptorRanges = {
			{
				D3D12_DESCRIPTOR_RANGE_TYPE_UAV,
				1,
				0, //!< u0
				0, //!< space0
				D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND
			},
		};
	}
	void CreateDescriptorHeap_1UAV() {
		const auto Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		const auto Count = 1;

		[&](const D3D12_DESCRIPTOR_HEAP_TYPE Type, const UINT Count, ID3D12DescriptorHeap** DescriptorHeap) {
			const D3D12_DESCRIPTOR_HEAP_DESC DescriptorHeapDesc = {
				Type,
				Count,
				D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE,
				0
			};
			VERIFY_SUCCEEDED(Device->CreateDescriptorHeap(&DescriptorHeapDesc, IID_PPV_ARGS(DescriptorHeap)));
		}(Type, Count, UnorderedAccessTextureDescriptorHeap.GetAddressOf());

		[&](const D3D12_DESCRIPTOR_HEAP_TYPE Type, ID3D12Resource* Resource, ID3D12DescriptorHeap* DescriptorHeap) {
			const auto CDH = GetCPUDescriptorHandle(DescriptorHeap, Type);
			D3D12_UNORDERED_ACCESS_VIEW_DESC UAVDesc = {
				DXGI_FORMAT_R8G8B8A8_UNORM,
				D3D12_UAV_DIMENSION_TEXTURE2D,
			};
			UAVDesc.Texture2D.MipSlice = 0;
			UAVDesc.Texture2D.PlaneSlice = 0;
			Device->CreateUnorderedAccessView(Resource, nullptr, &UAVDesc, CDH);
		}(Type, UnorderedAccessTextureResource.Get(), UnorderedAccessTextureDescriptorHeap.Get());

#ifdef DEBUG_STDOUT
		std::cout << "CreateDescriptorHeap" << COUT_OK << std::endl << std::endl;
#endif
	}


	//!< LinearWarp
	void CreateStaticSamplerDesc_LW(D3D12_STATIC_SAMPLER_DESC& StaticSamplerDesc, const D3D12_SHADER_VISIBILITY ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL, const FLOAT MaxLOD = (std::numeric_limits<FLOAT>::max)()) const;

	template<typename T> void CreateInputLayoutSlot(std::vector<D3D12_INPUT_ELEMENT_DESC>& InputElementDescs, const UINT InputSlot) const {}
	//!< �������Ńe���v���[�g���ꉻ���Ă��� Template specialization here
#include "DXInputLayout.inl"

	virtual D3D12_PRIMITIVE_TOPOLOGY_TYPE GetPrimitiveTopologyType() const override { return D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE; }

	virtual D3D_PRIMITIVE_TOPOLOGY GetPrimitiveTopology() const override { return D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP; }

	void CreateShader_VsPs(std::vector<Microsoft::WRL::ComPtr<ID3DBlob>>& ShaderBlobs) const;
	void CreateShader_VsPsDsHsGs(std::vector<Microsoft::WRL::ComPtr<ID3DBlob>>& ShaderBlobs) const;
	void CreateShader_Cs(std::vector<Microsoft::WRL::ComPtr<ID3DBlob>>& ShaderBlobs) const;

	template<typename T>
	void CreateConstantBuffer(const T& Type) {
		const auto Size = RoundUp(sizeof(T), 0xff); //!< 256�o�C�g�A���C��

		CreateUploadResource(ConstantBufferResource.GetAddressOf(), Size);
		CopyToUploadResource(ConstantBufferResource.Get(), Size, &Type);

#ifdef DEBUG_STDOUT
		std::cout << "CreateConstantBuffer" << COUT_OK << std::endl << std::endl;
#endif
	}
};
