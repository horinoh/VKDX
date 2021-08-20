#pragma once

#include "DX.h"

class DXExt : public DX
{
private:
	using Super = DX;
public:
	using Vertex_Position = struct Vertex_Position { DirectX::XMFLOAT3 Position; };
	using Vertex_PositionColor = struct Vertex_PositionColor { DirectX::XMFLOAT3 Position; DirectX::XMFLOAT4 Color; };
	using Vertex_PositionNormal = struct Vertex_PositionNormal { DirectX::XMFLOAT3 Position; DirectX::XMFLOAT3 Normal; };
	using Instance_OffsetXY = struct Instance_OffsetXY { DirectX::XMFLOAT2 Offset; };

	void CreatePipelineState_VsPs(const D3D12_PRIMITIVE_TOPOLOGY_TYPE PTT, const D3D12_RASTERIZER_DESC& RD, const BOOL DepthEnable, const std::array<D3D12_SHADER_BYTECODE, 2>& SBCs) { CreatePipelineState_VsPs_Input(PTT, RD, DepthEnable, {}, SBCs); }
	void CreatePipelineState_VsPs_Input(const D3D12_PRIMITIVE_TOPOLOGY_TYPE PTT, const D3D12_RASTERIZER_DESC& RD, const BOOL DepthEnable, const std::vector<D3D12_INPUT_ELEMENT_DESC>& IEDs, const std::array<D3D12_SHADER_BYTECODE, 2>& SBCs);
	void CreatePipelineState_VsPsDsHsGs(const D3D12_PRIMITIVE_TOPOLOGY_TYPE PTT, const D3D12_RASTERIZER_DESC& RD, const BOOL DepthEnable, const std::array<D3D12_SHADER_BYTECODE, 5>& SBCs) { CreatePipelineState_VsPsDsHsGs_Input(PTT, RD, DepthEnable, {}, SBCs); }
	void CreatePipelineState_VsPsDsHsGs_Input(const D3D12_PRIMITIVE_TOPOLOGY_TYPE PTT, const D3D12_RASTERIZER_DESC& RD, const BOOL DepthEnable, const std::vector<D3D12_INPUT_ELEMENT_DESC>& IEDs, const std::array<D3D12_SHADER_BYTECODE, 5>& SBCs);
	//void CreatePipelineState_VsGsPs();
	//void CreatePipelineState_VsPsDsHs();

	void CreatePipelineState_MsPs(const BOOL DepthEnable, const std::array<D3D12_SHADER_BYTECODE, 2>& SBCs) { CreatePipelineState_AsMsPs(DepthEnable, { NullSBC, SBCs[0], SBCs[1] }); }
	void CreatePipelineState_AsMsPs(const BOOL DepthEnable, const std::array<D3D12_SHADER_BYTECODE, 3>& SBCs);
};
