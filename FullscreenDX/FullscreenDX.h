#pragma once

#include "resource.h"

#pragma region Code
#include "../DXExt.h"

class FullscreenDX : public DXExt
{
private:
	using Super = DXExt;
public:
	FullscreenDX() : Super() {}
	virtual ~FullscreenDX() {}

protected:
#ifdef USE_INDIRECT
	virtual void CreateGeometry() override { 
		constexpr D3D12_DRAW_ARGUMENTS DA = { .VertexCountPerInstance = 4, .InstanceCount = 1, .StartVertexLocation = 0, .StartInstanceLocation = 0 };
		IndirectBuffers.emplace_back().Create(COM_PTR_GET(Device), DA).ExecuteCopyCommand(COM_PTR_GET(Device), COM_PTR_GET(CommandAllocators[0]), COM_PTR_GET(GraphicsCommandLists[0]), COM_PTR_GET(GraphicsCommandQueue), COM_PTR_GET(Fence), sizeof(DA), &DA);
	}
#endif
	virtual void CreatePipelineState() override {
		const auto ShaderPath = GetBasePath();
		std::vector<COM_PTR<ID3DBlob>> SBs;
#ifdef USE_DISTANCE_FUNCTION
		VERIFY_SUCCEEDED(D3DReadFileToBlob(data(ShaderPath + TEXT(".vs.cso")), COM_PTR_PUT(SBs.emplace_back())));
		VERIFY_SUCCEEDED(D3DReadFileToBlob(data(ShaderPath + TEXT("_df.ps.cso")), COM_PTR_PUT(SBs.emplace_back())));
#else
		VERIFY_SUCCEEDED(D3DReadFileToBlob(data(ShaderPath + TEXT(".vs.cso")), COM_PTR_PUT(SBs.emplace_back())));
		VERIFY_SUCCEEDED(D3DReadFileToBlob(data(ShaderPath + TEXT(".ps.cso")), COM_PTR_PUT(SBs.emplace_back())));
#endif
		const std::array SBCs = {
			D3D12_SHADER_BYTECODE({.pShaderBytecode = SBs[0]->GetBufferPointer(), .BytecodeLength = SBs[0]->GetBufferSize() }),
			D3D12_SHADER_BYTECODE({.pShaderBytecode = SBs[1]->GetBufferPointer(), .BytecodeLength = SBs[1]->GetBufferSize() }),
		};	

		constexpr D3D12_RASTERIZER_DESC RD = {
			.FillMode = D3D12_FILL_MODE_SOLID,
			.CullMode = D3D12_CULL_MODE_BACK, .FrontCounterClockwise = TRUE,
			.DepthBias = D3D12_DEFAULT_DEPTH_BIAS, .DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP, .SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS,
			.DepthClipEnable = TRUE,
			.MultisampleEnable = FALSE, .AntialiasedLineEnable = FALSE, .ForcedSampleCount = 0,
			.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF
		};
		CreatePipelineState_VsPs(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE, RD, FALSE, SBCs); 
	}

	virtual void PopulateCommandList(const size_t i) override;
};
#pragma endregion