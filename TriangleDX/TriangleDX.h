#pragma once

#include "resource.h"

#pragma region Code
#include "../DXExt.h"

class TriangleDX : public DXExt
{
private:
	using Super = DXExt;
public:
	TriangleDX() : Super() {}
	virtual ~TriangleDX() {}

protected:
#ifdef USE_BUNDLE
	virtual void CreateBundleCommandList() override { AddBundleCommandList(); }
#endif
	virtual void CreateVertexBuffer() override;
	virtual void CreateIndexBuffer() override;
	virtual void CreateIndirectBuffer() override { CreateIndirectBuffer_DrawIndexed(IndexCount, 1); }
	virtual void CreateRootSignature() override {
		COM_PTR<ID3DBlob> Blob;
#ifdef USE_HLSL_ROOTSIGNATRUE
		GetRootSignaturePartFromShader(Blob, (GetBasePath() + TEXT(".rs.cso")).data());
#else
#ifdef USE_ROOT_CONSTANTS
		D3D12_ROOT_PARAMETER RP;
		RP.ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
		RP.Constants = { 0, 0, static_cast<UINT>(Color.size()) };
		RP.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
		DX::SerializeRootSignature(Blob, { RP }, {}, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
#else
		DX::SerializeRootSignature(Blob, {}, {}, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
#endif
#endif
		RootSignatures.resize(1);
		DX::CreateRootSignature(RootSignatures[0], Blob);
		LOG_OK();
	}
	virtual void CreateShaderBlob() override { CreateShaderBlob_VsPs(); }
	virtual void CreatePipelineState() override { CreatePipelineState_VsPs_Vertex<Vertex_PositionColor>(); }
	virtual void PopulateCommandList(const size_t i) override;

	UINT IndexCount = 0;
#ifdef USE_ROOT_CONSTANTS
	const std::array<FLOAT, 4> Color = { 1.0f, 0.0f, 0.0f, 1.0f };
#endif
};
#pragma endregion