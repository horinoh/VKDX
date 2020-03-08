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


	static void CreatePipelineState_G(COM_PTR<ID3D12PipelineState>&, ID3D12RootSignature*,
		const D3D12_PRIMITIVE_TOPOLOGY_TYPE,
		const D3D12_SHADER_BYTECODE, const D3D12_SHADER_BYTECODE, const D3D12_SHADER_BYTECODE, const D3D12_SHADER_BYTECODE, const D3D12_SHADER_BYTECODE) {}
	void CreatePipelineState_L(COM_PTR<ID3D12PipelineState>&, ID3D12RootSignature*,
		const D3D12_PRIMITIVE_TOPOLOGY_TYPE,
		const D3D12_SHADER_BYTECODE, const D3D12_SHADER_BYTECODE, const D3D12_SHADER_BYTECODE, const D3D12_SHADER_BYTECODE, const D3D12_SHADER_BYTECODE) {}
	void CreatePipelineState(const D3D12_PRIMITIVE_TOPOLOGY_TYPE Topology, const COM_PTR<ID3DBlob> Vs, const COM_PTR<ID3DBlob> Ps, COM_PTR<ID3DBlob> Ds, COM_PTR<ID3DBlob> Hs, COM_PTR<ID3DBlob> Gs);
	void CreatePipelineState_VsPs(const D3D12_PRIMITIVE_TOPOLOGY_TYPE Topology) { CreatePipelineState(Topology, ShaderBlobs[0], ShaderBlobs[1], nullptr, nullptr, nullptr); }
	void CreatePipelineState_VsPsDsHsGs(const D3D12_PRIMITIVE_TOPOLOGY_TYPE Topology) { CreatePipelineState(Topology, ShaderBlobs[0], ShaderBlobs[1], ShaderBlobs[2], ShaderBlobs[3], ShaderBlobs[4]); }
	void CreatePipelineState_Cs(COM_PTR<ID3D12PipelineState>& /*CS*/) { assert(0 && "TODO"); }
};
