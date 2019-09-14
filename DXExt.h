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

	void CreateShaderBlob_VsPs();
	void CreateShaderBlob_VsPsDsHsGs();
	void CreateShaderBlob_Cs();

	template<typename T> void CreatePipelineState_Vertex(/*winrt::com_ptr*/COM_PTR<ID3D12PipelineState>& PipelineState, ID3D12RootSignature* RS,
		const D3D12_SHADER_BYTECODE VS, const D3D12_SHADER_BYTECODE PS, const D3D12_SHADER_BYTECODE DS, const D3D12_SHADER_BYTECODE HS, const D3D12_SHADER_BYTECODE GS);
	void CreatePipelineState_Tesselation(/*winrt::com_ptr*/COM_PTR<ID3D12PipelineState>& PipelineState, ID3D12RootSignature* RS,
		const D3D12_SHADER_BYTECODE VS, const D3D12_SHADER_BYTECODE PS, const D3D12_SHADER_BYTECODE DS, const D3D12_SHADER_BYTECODE HS, const D3D12_SHADER_BYTECODE GS);

	void CreatePipelineState_VsPs();
	void CreatePipelineState_VsPsDsHsGs_Tesselation();
	void CreatePipelineState_Cs() { assert(0 && "TODO"); }
	//!< ↓ここでテンプレート特殊化している (Template specialization here)
#include "DXPipeline.inl"

	template<typename T>
	void CreateConstantBufferT(const T& Type) {
		const auto Size = RoundUp(sizeof(Type), 0xff); //!< 256バイトアライン
		//!< #DX_TODO_PERF 本来はバッファ毎にメモリを確保するのではなく、予め大きなメモリを作成しておいてその一部を複数のバッファへ割り当てる方がよい
		CreateUploadResource(COM_PTR_PUT(ConstantBufferResource), Size);
		CopyToUploadResource(COM_PTR_GET(ConstantBufferResource), Size, &Type); 
		LOG_OK();
	}
};
