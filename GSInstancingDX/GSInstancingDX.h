#pragma once

#include "resource.h"

#pragma region Code
#include "../DXExt.h"

class GSInstancingDX : public DXExt
{
private:
	using Super = DXExt;
public:
	GSInstancingDX() : Super() {}
	virtual ~GSInstancingDX() {}

protected:
	virtual void CreateGeometry() override { 
		constexpr D3D12_DRAW_INDEXED_ARGUMENTS DIA = { .IndexCountPerInstance = 1, .InstanceCount = 1, .StartIndexLocation = 0, .BaseVertexLocation = 0, .StartInstanceLocation = 0 };
		IndirectBuffers.emplace_back().Create(COM_PTR_GET(Device), DIA);
		IndirectBuffers.back().ExecuteCopyCommand(COM_PTR_GET(Device), COM_PTR_GET(CommandAllocators[0]), COM_PTR_GET(GraphicsCommandLists[0]), COM_PTR_GET(GraphicsCommandQueue), COM_PTR_GET(Fence), sizeof(DIA), &DIA);
	}
	virtual void CreatePipelineState() override { 
		const auto ShaderPath = GetBasePath();
		std::vector<COM_PTR<ID3DBlob>> SBs;
		VERIFY_SUCCEEDED(D3DReadFileToBlob(data(ShaderPath + TEXT(".vs.cso")), COM_PTR_PUT(SBs.emplace_back())));
		VERIFY_SUCCEEDED(D3DReadFileToBlob(data(ShaderPath + TEXT(".ps.cso")), COM_PTR_PUT(SBs.emplace_back())));
		VERIFY_SUCCEEDED(D3DReadFileToBlob(data(ShaderPath + TEXT(".ds.cso")), COM_PTR_PUT(SBs.emplace_back())));
		VERIFY_SUCCEEDED(D3DReadFileToBlob(data(ShaderPath + TEXT(".hs.cso")), COM_PTR_PUT(SBs.emplace_back())));
		VERIFY_SUCCEEDED(D3DReadFileToBlob(data(ShaderPath + TEXT(".gs.cso")), COM_PTR_PUT(SBs.emplace_back())));
		const std::array SBCs = {
			D3D12_SHADER_BYTECODE({.pShaderBytecode = SBs[0]->GetBufferPointer(), .BytecodeLength = SBs[0]->GetBufferSize() }),
			D3D12_SHADER_BYTECODE({.pShaderBytecode = SBs[1]->GetBufferPointer(), .BytecodeLength = SBs[1]->GetBufferSize() }),
			D3D12_SHADER_BYTECODE({.pShaderBytecode = SBs[2]->GetBufferPointer(), .BytecodeLength = SBs[2]->GetBufferSize() }),
			D3D12_SHADER_BYTECODE({.pShaderBytecode = SBs[3]->GetBufferPointer(), .BytecodeLength = SBs[3]->GetBufferSize() }),
			D3D12_SHADER_BYTECODE({.pShaderBytecode = SBs[4]->GetBufferPointer(), .BytecodeLength = SBs[4]->GetBufferSize() }),
		};
		CreatePipelineState_VsPsDsHsGs(D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH, FALSE, SBCs);
	}

	virtual void PopulateCommandList(const size_t i) override;

	virtual void CreateViewport(const FLOAT Width, const FLOAT Height, const FLOAT MinDepth = 0.0f, const FLOAT MaxDepth = 1.0f) override {
		D3D12_FEATURE_DATA_D3D12_OPTIONS3 FDO3;
		VERIFY_SUCCEEDED(Device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS3, reinterpret_cast<void*>(&FDO3), sizeof(FDO3)));
		assert(D3D12_VIEW_INSTANCING_TIER_1 < FDO3.ViewInstancingTier && "");

		const auto W = Width * 0.5f, H = Height * 0.5f;
		Viewports = {
			D3D12_VIEWPORT({.TopLeftX = 0.0f, .TopLeftY = 0.0f, .Width = W, .Height = H, .MinDepth = MinDepth, .MaxDepth = MaxDepth }),
			D3D12_VIEWPORT({.TopLeftX = W, .TopLeftY = 0.0f, .Width = W, .Height = H, .MinDepth = MinDepth, .MaxDepth = MaxDepth }),
			D3D12_VIEWPORT({.TopLeftX = 0.0f, .TopLeftY = H, .Width = W, .Height = H, .MinDepth = MinDepth, .MaxDepth = MaxDepth }),
			D3D12_VIEWPORT({.TopLeftX = W, .TopLeftY = H, .Width = W, .Height = H, .MinDepth = MinDepth, .MaxDepth = MaxDepth }),
		};
		//!< left, top, right, bottom‚ÅŽw’è (offset, extent‚ÅŽw’è‚ÌVK‚Æ‚ÍˆÙ‚È‚é‚Ì‚Å’ˆÓ)
		ScissorRects = {
			D3D12_RECT({.left = 0, .top = 0, .right = static_cast<LONG>(W), .bottom = static_cast<LONG>(H) }),
			D3D12_RECT({.left = static_cast<LONG>(W), .top = 0, .right = static_cast<LONG>(Width), .bottom = static_cast<LONG>(H) }),
			D3D12_RECT({.left = 0, .top = static_cast<LONG>(H), .right = static_cast<LONG>(W), .bottom = static_cast<LONG>(Height) }),
			D3D12_RECT({.left = static_cast<LONG>(W), .top = static_cast<LONG>(H), .right = static_cast<LONG>(Width), .bottom = static_cast<LONG>(Height) }),
		};
		LOG_OK();
	}
};
#pragma endregion