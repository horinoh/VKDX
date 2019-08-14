#pragma once

#include "DX.h"

class DXExt : public DX
{
private:
	using Super = DX;
public:
	//using Vertex_Position = struct Vertex_Position { DirectX::XMFLOAT3 Position; };
	using Vertex_PositionColor = struct Vertex_PositionColor { DirectX::XMFLOAT3 Position; DirectX::XMFLOAT4 Color; };
	
	void CreateIndirectBuffer_Draw(const UINT Count);
	void CreateIndirectBuffer_DrawIndexed(const UINT Count);
	void CreateIndirectBuffer_Dispatch(const UINT X, const UINT Y, const UINT Z);

	//void CreateStaticSamplerDesc_LW(D3D12_STATIC_SAMPLER_DESC& StaticSamplerDesc, const D3D12_SHADER_VISIBILITY ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL, const FLOAT MaxLOD = (std::numeric_limits<FLOAT>::max)()) const;

	void CreateShaderBlob_VsPs();
	void CreateShaderBlob_VsPsDsHsGs();
	void CreateShaderBlob_Cs();

	template<typename T> void CreatePipelineState_Vertex(winrt::com_ptr<ID3D12PipelineState>& PipelineState, ID3D12RootSignature* RS,
		const D3D12_SHADER_BYTECODE VS, const D3D12_SHADER_BYTECODE PS, const D3D12_SHADER_BYTECODE DS, const D3D12_SHADER_BYTECODE HS, const D3D12_SHADER_BYTECODE GS);
	void CreatePipelineState_Tesselation(winrt::com_ptr<ID3D12PipelineState>& PipelineState, ID3D12RootSignature* RS,
		const D3D12_SHADER_BYTECODE VS, const D3D12_SHADER_BYTECODE PS, const D3D12_SHADER_BYTECODE DS, const D3D12_SHADER_BYTECODE HS, const D3D12_SHADER_BYTECODE GS);

	void CreatePipelineState_VsPs();
	void CreatePipelineState_VsPsDsHsGs_Tesselation();
	void CreatePipelineState_Cs() { assert(0 && "TODO"); }
	//!< �������Ńe���v���[�g���ꉻ���Ă��� (Template specialization here)
#include "DXPipeline.inl"

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
