#pragma once

#include "DX.h"

class DXExt : public DX
{
private:
	using Super = DX;
public:
	using Vertex_PositionColor = struct Vertex_PositionColor { DirectX::XMFLOAT3 Position; DirectX::XMFLOAT4 Color; };
	using Instance_OffsetXY = struct Instance_OffsetXY { DirectX::XMFLOAT2 Offset; };

	void CreateIndirectBuffer_Draw(const UINT IndexCount, const UINT InstanceCount);
	void CreateIndirectBuffer_DrawIndexed(const UINT IndexCount, const UINT InstanceCount);
	void CreateIndirectBuffer_Dispatch(const UINT X, const UINT Y, const UINT Z);

	void CreateShaderBlob_VsPs();
	void CreateShaderBlob_VsPsDsHsGs();
	void CreateShaderBlob_Cs();

	void CreatePipelineState_VsPs();
	void CreatePipelineState_VsPsDsHsGs_Tesselation();
	void CreatePipelineState_Cs(COM_PTR<ID3D12PipelineState>& /*PS*/) { assert(0 && "TODO"); }

	template<typename T>
	void CreateConstantBufferT(const T& Type) {
		ConstantBuffers.push_back(COM_PTR<ID3D12Resource>());

		const auto Size = RoundUp(sizeof(Type), 0xff); //!< 256バイトアライン
		//!< #DX_TODO_PERF 本来はバッファ毎にメモリを確保するのではなく、予め大きなメモリを作成しておいてその一部を複数のバッファへ割り当てる方がよい
		CreateUploadResource(COM_PTR_PUT(ConstantBuffers.back()), Size);
		CopyToUploadResource(COM_PTR_GET(ConstantBuffers.back()), Size, &Type); 
		LOG_OK();
	}
};
