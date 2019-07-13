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
				0, //!< b0 BaseShaderRegister ... ��) ": register(t3);" �� "3"
				0, //!< space0 RegisterSpace �ʏ��0 .... ��) ": register(t3,space5);" �� "5"
				D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND
			},
		};
	}

	void CreateDescriptorHeap_1CBV() {
		const D3D12_DESCRIPTOR_HEAP_DESC DHD = {
				D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
				1,
				D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE,
				0 //!< NodeMask ... �}���`GPU�̏ꍇ (Use with multi GPU)
		};
#ifdef USE_WINRT
		VERIFY_SUCCEEDED(Device->CreateDescriptorHeap(&DHD, __uuidof(ConstantBufferDescriptorHeap), ConstantBufferDescriptorHeap.put_void()));
#elif defined(USE_WRL)
		VERIFY_SUCCEEDED(Device->CreateDescriptorHeap(&DHD, IID_PPV_ARGS(ConstantBufferDescriptorHeap.GetAddressOf())));
#endif
	}

	template<typename T>
	void CreateDescriptorView_1CBV() {
		const auto Size256 = static_cast<UINT>(RoundUp(sizeof(T), 0xff)); //!< 256 byte align
		const D3D12_CONSTANT_BUFFER_VIEW_DESC ConstantBufferViewDesc = {
			ConstantBufferResource->GetGPUVirtualAddress(),
			Size256
		};
#ifdef USE_WINRT
		const auto CDH = GetCPUDescriptorHandle(ConstantBufferDescriptorHeap.get(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
#elif defined(USE_WRL)
		const auto CDH = GetCPUDescriptorHandle(ConstantBufferDescriptorHeap.Get(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
#endif
		Device->CreateConstantBufferView(&ConstantBufferViewDesc, CDH);

		LOG_OK();
	}

	/*
	@brief �P�̃V�F�[�_���\�[�X�r���[ (One shader resource view)
	*/
	void CreateRootParameters_1SRV(std::vector<D3D12_ROOT_PARAMETER>& RootParameters, const std::vector<D3D12_DESCRIPTOR_RANGE>& DescriptorRanges, const D3D12_SHADER_VISIBILITY ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL) const {
		RootParameters = {
			{
				D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE,
				{ static_cast<uint32_t>(DescriptorRanges.size()), DescriptorRanges.data() },
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

#ifdef USE_WINRT
		[&](const D3D12_DESCRIPTOR_HEAP_TYPE Type, const UINT Count, winrt::com_ptr<ID3D12DescriptorHeap>& DH){
			const D3D12_DESCRIPTOR_HEAP_DESC DescriptorHeapDesc = {
				Type,
				Count,
				D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE,
				0
			};
			VERIFY_SUCCEEDED(Device->CreateDescriptorHeap(&DescriptorHeapDesc, __uuidof(DH), DH.put_void()));
		}
#elif defined(USE_WRL)
		[&](const D3D12_DESCRIPTOR_HEAP_TYPE Type, const UINT Count, Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>& DH) {
			const D3D12_DESCRIPTOR_HEAP_DESC DescriptorHeapDesc = {
				Type,
				Count,
				D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE,
				0
			};
			VERIFY_SUCCEEDED(Device->CreateDescriptorHeap(&DescriptorHeapDesc, IID_PPV_ARGS(DH.GetAddressOf())));
		}
#endif
		(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 1, ImageDescriptorHeap);

#ifdef USE_WINRT
		[&](const D3D12_DESCRIPTOR_HEAP_TYPE Type, winrt::com_ptr<ID3D12Resource> Resource, winrt::com_ptr<ID3D12DescriptorHeap> DH) {
			const auto CDH = GetCPUDescriptorHandle(DH.get(), Type);
			Device->CreateShaderResourceView(Resource.get(), nullptr, CDH);
		}
#elif defined(USE_WRL)
		[&](const D3D12_DESCRIPTOR_HEAP_TYPE Type, Microsoft::WRL::ComPtr<ID3D12Resource> Resource, Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> DH) {
			const auto CDH = GetCPUDescriptorHandle(DH.Get(), Type);
			Device->CreateShaderResourceView(Resource.Get(), nullptr, CDH);
		} 
#endif
		(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, ImageResource, ImageDescriptorHeap);

		LOG_OK();
	}

	/*
	@brief �P�̃R���X�^���g�o�b�t�@�ƂP�̃V�F�[�_���\�[�X�r���[ (One constant buffer view and one shader resource view)
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
		CreateDescriptorHeap_1CBV();
		CreateDescriptorView_1CBV<T>();
		CreateDescriptorHeap_1SRV();

		LOG_OK();
	}
	
	/*
	@brief �P�̃A���I�[�_�[�h�A�N�Z�X�r���[ (One unordered access view)
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
#ifdef USE_WINRT
		[&](const D3D12_DESCRIPTOR_HEAP_TYPE Type, const UINT Count, winrt::com_ptr<ID3D12DescriptorHeap>& DH) {
			const D3D12_DESCRIPTOR_HEAP_DESC DescriptorHeapDesc = {
				Type,
				Count,
				D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE,
				0
			};
			VERIFY_SUCCEEDED(Device->CreateDescriptorHeap(&DescriptorHeapDesc, __uuidof(DH), DH.put_void()));
		}
#elif defined(USE_WRL)
		[&](const D3D12_DESCRIPTOR_HEAP_TYPE Type, const UINT Count, Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>& DH) {
			const D3D12_DESCRIPTOR_HEAP_DESC DescriptorHeapDesc = {
				Type,
				Count,
				D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE,
				0
			};
			VERIFY_SUCCEEDED(Device->CreateDescriptorHeap(&DescriptorHeapDesc, IID_PPV_ARGS(DH.GetAddressOf())));
		}
#endif
		(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 1, UnorderedAccessTextureDescriptorHeap);

#ifdef USE_WINRT
		[&](const D3D12_DESCRIPTOR_HEAP_TYPE Type, winrt::com_ptr<ID3D12Resource> Resource, winrt::com_ptr<ID3D12DescriptorHeap> DH) {
			const auto CDH = GetCPUDescriptorHandle(DH.get(), Type);
			D3D12_UNORDERED_ACCESS_VIEW_DESC UAVDesc = {
				DXGI_FORMAT_R8G8B8A8_UNORM,
				D3D12_UAV_DIMENSION_TEXTURE2D,
			};
			UAVDesc.Texture2D.MipSlice = 0;
			UAVDesc.Texture2D.PlaneSlice = 0;
			Device->CreateUnorderedAccessView(Resource.get(), nullptr, &UAVDesc, CDH);
		}
#elif defined(USE_WRL)
		[&](const D3D12_DESCRIPTOR_HEAP_TYPE Type, Microsoft::WRL::ComPtr<ID3D12Resource> Resource, Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> DH) {
			const auto CDH = GetCPUDescriptorHandle(DH.Get(), Type);
			D3D12_UNORDERED_ACCESS_VIEW_DESC UAVDesc = {
				DXGI_FORMAT_R8G8B8A8_UNORM,
				D3D12_UAV_DIMENSION_TEXTURE2D,
			};
			UAVDesc.Texture2D.MipSlice = 0;
			UAVDesc.Texture2D.PlaneSlice = 0;
			Device->CreateUnorderedAccessView(Resource.Get(), nullptr, &UAVDesc, CDH);
		}
#endif
		(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, UnorderedAccessTextureResource, UnorderedAccessTextureDescriptorHeap);

		LOG_OK();
	}


	//!< LinearWrap
	void CreateStaticSamplerDesc_LW(D3D12_STATIC_SAMPLER_DESC& StaticSamplerDesc, const D3D12_SHADER_VISIBILITY ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL, const FLOAT MaxLOD = (std::numeric_limits<FLOAT>::max)()) const;

	template<typename T> void CreateInputLayoutSlot(std::vector<D3D12_INPUT_ELEMENT_DESC>& InputElementDescs, const UINT InputSlot) const {}
	//!< �������Ńe���v���[�g���ꉻ���Ă��� (Template specialization here)
#include "DXInputLayout.inl"

	virtual D3D12_PRIMITIVE_TOPOLOGY_TYPE GetPrimitiveTopologyType() const override { return D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE; }

	virtual D3D_PRIMITIVE_TOPOLOGY GetPrimitiveTopology() const override { return D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP; }

#ifdef USE_WINRT
	void CreateShader_VsPs(std::vector<winrt::com_ptr<ID3DBlob>>& ShaderBlobs) const;
	void CreateShader_VsPsDsHsGs(std::vector<winrt::com_ptr<ID3DBlob>>& ShaderBlobs) const;
	void CreateShader_Cs(std::vector<winrt::com_ptr<ID3DBlob>>& ShaderBlobs) const; 
#elif defined(USE_WRL)
	void CreateShader_VsPs(std::vector<Microsoft::WRL::ComPtr<ID3DBlob>>& ShaderBlobs) const;
	void CreateShader_VsPsDsHsGs(std::vector<Microsoft::WRL::ComPtr<ID3DBlob>>& ShaderBlobs) const;
	void CreateShader_Cs(std::vector<Microsoft::WRL::ComPtr<ID3DBlob>>& ShaderBlobs) const;
#endif

	template<typename T> void CreatePipelineState_Vertex(winrt::com_ptr<ID3D12PipelineState>& PipelineState, ID3D12RootSignature* RS,
		const D3D12_SHADER_BYTECODE VS, const D3D12_SHADER_BYTECODE PS, const D3D12_SHADER_BYTECODE DS, const D3D12_SHADER_BYTECODE HS, const D3D12_SHADER_BYTECODE GS);
#include "DXPipeline.inl"
	void CreatePipelineState_VertexPositionColor(winrt::com_ptr<ID3D12PipelineState>& PipelineState, ID3D12RootSignature* RS,
		const D3D12_SHADER_BYTECODE VS, const D3D12_SHADER_BYTECODE PS, const D3D12_SHADER_BYTECODE DS, const D3D12_SHADER_BYTECODE HS, const D3D12_SHADER_BYTECODE GS);
	void CreatePipelineState_Tesselation(winrt::com_ptr<ID3D12PipelineState>& PipelineState, ID3D12RootSignature* RS,
		const D3D12_SHADER_BYTECODE VS, const D3D12_SHADER_BYTECODE PS, const D3D12_SHADER_BYTECODE DS, const D3D12_SHADER_BYTECODE HS, const D3D12_SHADER_BYTECODE GS);

	template<typename T>
	void CreateConstantBufferT(const T& Type) {
		const auto Size = RoundUp(sizeof(Type), 0xff); //!< 256�o�C�g�A���C��

		//!< #DX_TODO_PERF �{���̓o�b�t�@���Ƀ��������m�ۂ���̂ł͂Ȃ��A�\�ߑ傫�ȃ��������쐬���Ă����Ă��̈ꕔ�𕡐��̃o�b�t�@�֊��蓖�Ă�����悢
#ifdef USE_WINRT
		CreateUploadResource(ConstantBufferResource.put(), Size);
		CopyToUploadResource(ConstantBufferResource.get(), Size, &Type); 
#elif defined(USE_WRL)
		CreateUploadResource(ConstantBufferResource.GetAddressOf(), Size);
		CopyToUploadResource(ConstantBufferResource.Get(), Size, &Type); 
#endif
	
		LOG_OK();
	}
};
