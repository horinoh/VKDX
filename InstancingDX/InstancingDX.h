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
		DX::SerializeRootSignature(Blob, {}, {}, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT
			| D3D12_ROOT_SIGNATURE_FLAG_DENY_VERTEX_SHADER_ROOT_ACCESS
			| D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS
			| D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS
			| D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS
			| D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS);
#endif
		RootSignatures.resize(1);
		VERIFY_SUCCEEDED(Device->CreateRootSignature(0, Blob->GetBufferPointer(), Blob->GetBufferSize(), COM_PTR_UUIDOF_PUTVOID(RootSignatures[0])));
		LOG_OK();
	}
	virtual void CreateShaderBlobs() override { CreateShaderBlob_VsPs(); }
	virtual void CreatePipelineStates() override {
		const std::vector<D3D12_INPUT_ELEMENT_DESC> IEDs = { {
			//!< Per Vertex
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, /*offsetof(Vertex_PositionColor, Position)*/D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, /*offsetof(Vertex_PositionColor, Color)*/D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			//!< Per Instance
			{ "OFFSET", 0, DXGI_FORMAT_R32G32_FLOAT, 1, /*offsetof(Instance_OffsetXY, Offset)*/D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 },
		} };
		CreatePipelineState_VsPs_Input(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE, FALSE, IEDs);
	}
	virtual void PopulateCommandList(const size_t i) override;

	UINT IndexCount = 0;
	UINT InstanceCount = 0;
};
#pragma endregion