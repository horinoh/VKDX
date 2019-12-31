#pragma once

#include "DX.h"

class DXExt : public DX
{
private:
	using Super = DX;
public:
	//using Vertex_Position = struct Vertex_Position { DirectX::XMFLOAT3 Position; };
	using Vertex_PositionColor = struct Vertex_PositionColor { DirectX::XMFLOAT3 Position; DirectX::XMFLOAT4 Color; };
	//using Vertex_PositionNormalTexcoord = struct Vertex_PositionNormalTexcoord { DirectX::XMFLOAT3 Position; DirectX::XMFLOAT3 Normal; DirectX::XMFLOAT2 Texcoord; };
	using Instance_OffsetXY = struct Instance_OffsetXY { DirectX::XMFLOAT2 Offset; };

	void CreateIndirectBuffer_Draw(const UINT IndexCount, const UINT InstanceCount);
	void CreateIndirectBuffer_DrawIndexed(const UINT IndexCount, const UINT InstanceCount);
	void CreateIndirectBuffer_Dispatch(const UINT X, const UINT Y, const UINT Z);

	void CreateShaderBlob_VsPs();
	void CreateShaderBlob_VsPsDsHsGs();
	void CreateShaderBlob_Cs();

	template<typename T> void CreatePipelineState_Vertex(COM_PTR<ID3D12PipelineState>& PipelineState, ID3D12RootSignature* RS,
		const D3D12_SHADER_BYTECODE VS, const D3D12_SHADER_BYTECODE PS, const D3D12_SHADER_BYTECODE DS, const D3D12_SHADER_BYTECODE HS, const D3D12_SHADER_BYTECODE GS,
		ID3D12PipelineLibrary* PL = nullptr, LPCWSTR Name = nullptr, const bool IsLoad = false);
	template<typename T, typename U> void CreatePipelineState_Vertex_Instance(COM_PTR<ID3D12PipelineState>& PipelineState, ID3D12RootSignature* RS,
		const D3D12_SHADER_BYTECODE VS, const D3D12_SHADER_BYTECODE PS, const D3D12_SHADER_BYTECODE DS, const D3D12_SHADER_BYTECODE HS, const D3D12_SHADER_BYTECODE GS,
		ID3D12PipelineLibrary* PL = nullptr, LPCWSTR Name = nullptr, const bool IsLoad = false);

	void CreatePipelineState_VsPs();
	void CreatePipelineState_VsPsDsHsGs_Tesselation();
	void CreatePipelineState_Cs(COM_PTR<ID3D12PipelineState>& /*PS*/) { assert(0 && "TODO"); }
	//!< �������Ńe���v���[�g���ꉻ���Ă��� (Template specialization here)
#include "DXPipeline.inl"

	template<typename T>
	void CreateConstantBufferT(const T& Type) {
		ConstantBuffers.push_back(COM_PTR<ID3D12Resource>());

		const auto Size = RoundUp(sizeof(Type), 0xff); //!< 256�o�C�g�A���C��
		//!< #DX_TODO_PERF �{���̓o�b�t�@���Ƀ��������m�ۂ���̂ł͂Ȃ��A�\�ߑ傫�ȃ��������쐬���Ă����Ă��̈ꕔ�𕡐��̃o�b�t�@�֊��蓖�Ă�����悢
		CreateUploadResource(COM_PTR_PUT(ConstantBuffers.back()), Size);
		CopyToUploadResource(COM_PTR_GET(ConstantBuffers.back()), Size, &Type); 
		LOG_OK();
	}
};
