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

	void CreatePipelineState_VsPs(const D3D12_PRIMITIVE_TOPOLOGY_TYPE Topology) { CreatePipelineState_VsPs_Input(Topology, {}); }
	void CreatePipelineState_VsPs_Input(const D3D12_PRIMITIVE_TOPOLOGY_TYPE Topology, const std::vector<D3D12_INPUT_ELEMENT_DESC>& IEDs);
	void CreatePipelineState_VsPsDsHsGs(const D3D12_PRIMITIVE_TOPOLOGY_TYPE Topology) { CreatePipelineState_VsPsDsHsGs_Input(Topology, {}); }
	void CreatePipelineState_VsPsDsHsGs_Input(const D3D12_PRIMITIVE_TOPOLOGY_TYPE Topology, const std::vector<D3D12_INPUT_ELEMENT_DESC>& IEDs);
	void CreatePipelineState_Cs(COM_PTR<ID3D12PipelineState>& /*CS*/) { assert(0 && "TODO"); }
};
