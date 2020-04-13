#pragma once

#include "resource.h"

#pragma region Code
#include "../DXExt.h"

class InstancingDX : public DXExt
{
private:
	using Super = DXExt;
public:
	InstancingDX() : Super() {}
	virtual ~InstancingDX() {}

protected:
	virtual void CreateVertexBuffer() override;
	virtual void CreateIndexBuffer() override;
	virtual void CreateIndirectBuffer() override { CreateIndirectBuffer_DrawIndexed(IndexCount, InstanceCount); }
	virtual void CreateRootSignature() override {
		COM_PTR<ID3DBlob> Blob;
#ifdef USE_HLSL_ROOTSIGNATRUE
		GetRootSignaturePartFromShader(Blob, (GetBasePath() + TEXT(".rs.cso")).data());
#else
		DX::SerializeRootSignature(Blob, {}, {}, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
#endif
		RootSignatures.resize(1);
		DX::CreateRootSignature(RootSignatures[0], Blob);
		LOG_OK();
	}
	virtual void CreateShaderBlobs() override { CreateShaderBlob_VsPs(); }
	virtual void CreatePipelineStates() override {
		const std::vector<D3D12_INPUT_ELEMENT_DESC> IEDs = { {
			//!< Per Vertex
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(Vertex_PositionColor, Position), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, offsetof(Vertex_PositionColor, Color), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			//!< Per Instance
			{ "OFFSET", 0, DXGI_FORMAT_R32G32_FLOAT, 1, offsetof(Instance_OffsetXY, Offset), D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 },
		} };
		CreatePipelineState_VsPs_Input(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE, IEDs);
	}
	virtual void PopulateCommandList(const size_t i) override;

	UINT IndexCount = 0;
	UINT InstanceCount = 0;
};
#pragma endregion