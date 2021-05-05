#pragma once

#include "DX.h"

class DXExt : public DX
{
private:
	using Super = DX;
public:
	using Vertex_PositionColor = struct Vertex_PositionColor { DirectX::XMFLOAT3 Position; DirectX::XMFLOAT4 Color; };
	using Instance_OffsetXY = struct Instance_OffsetXY { DirectX::XMFLOAT2 Offset; };

	void CreatePipelineState_VsPs(const D3D12_PRIMITIVE_TOPOLOGY_TYPE PTT, const BOOL DepthEnable, const std::array<D3D12_SHADER_BYTECODE, 2>& SBCs) { CreatePipelineState_VsPs_Input(PTT, DepthEnable, {}, SBCs); }
	void CreatePipelineState_VsPs_Input(const D3D12_PRIMITIVE_TOPOLOGY_TYPE PTT, const BOOL DepthEnable, const std::vector<D3D12_INPUT_ELEMENT_DESC>& IEDs, const std::array<D3D12_SHADER_BYTECODE, 2>& SBCs);
	void CreatePipelineState_VsPsDsHsGs(const D3D12_PRIMITIVE_TOPOLOGY_TYPE PTT, const BOOL DepthEnable, const std::array<D3D12_SHADER_BYTECODE, 5>& SBCs) { CreatePipelineState_VsPsDsHsGs_Input(PTT, DepthEnable, {}, SBCs); }
	void CreatePipelineState_VsPsDsHsGs_Input(const D3D12_PRIMITIVE_TOPOLOGY_TYPE PTT, const BOOL DepthEnable, const std::vector<D3D12_INPUT_ELEMENT_DESC>& IEDs, const std::array<D3D12_SHADER_BYTECODE, 5>& SBCs);
	//void CreatePipelineState_VsGsPs();
	//void CreatePipelineState_VsPsDsHs();

	void CreatePipelineState_MsPs(const D3D12_PRIMITIVE_TOPOLOGY_TYPE PTT, const BOOL DepthEnable, const std::array<D3D12_SHADER_BYTECODE, 2>& SBCs);
	void CreatePipelineState_AsMsPs([[maybe_unused]] const D3D12_PRIMITIVE_TOPOLOGY_TYPE PTT, [[maybe_unused]] const BOOL DepthEnable, [[maybe_unused]] const std::array<D3D12_SHADER_BYTECODE, 3>& SBCs) {}
};
