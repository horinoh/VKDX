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
	virtual void CreateShaderBlobs() override { CreateShaderBlob_VsPs(); }
	virtual void CreatePipelineStates() override {
		PipelineStates.resize(1);
		std::vector<std::thread> Threads;
		const auto RS = COM_PTR_GET(RootSignatures[0]);
		//!< ‹l‚Ü‚Á‚Ä‚¢‚éê‡‚Í offsetof() ‚Ì‘ã‚í‚è‚É D3D12_APPEND_ALIGNED_ELEMENT ‚Å—Ç‚¢ (When directly after the previous one, we can use D3D12_APPEND_ALIGNED_ELEMENT)
		const std::vector<D3D12_INPUT_ELEMENT_DESC> IEDs = { {
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(Vertex_PositionColor, Position)/*D3D12_APPEND_ALIGNED_ELEMENT*/, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, offsetof(Vertex_PositionColor, Color)/*D3D12_APPEND_ALIGNED_ELEMENT*/, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		} };
#ifdef USE_PIPELINE_SERIALIZE
		const auto PCOPath = GetBasePath() + TEXT(".plo");
		PipelineLibrarySerializer PLS(COM_PTR_GET(Device), PCOPath.c_str());
		Threads.push_back(std::thread::thread(DX::CreatePipelineState, std::ref(PipelineStates[0]), COM_PTR_GET(Device), RS, D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE, ToShaderBC(ShaderBlobs[0]), ToShaderBC(ShaderBlobs[1]), NullShaderBC, NullShaderBC, NullShaderBC, IEDs, &PLS, TEXT("0")));
#else
		Threads.push_back(std::thread::thread(DX::CreatePipelineState, std::ref(PipelineStates[0]), COM_PTR_GET(Device), RS, D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE, ToShaderBC(ShaderBlobs[0]), ToShaderBC(ShaderBlobs[1]), NullShaderBC, NullShaderBC, NullShaderBC, IEDs, nullptr, nullptr));
#endif
		for (auto& i : Threads) { i.join(); }
	}
	virtual void PopulateCommandList(const size_t i) override;

	UINT IndexCount = 0;
#ifdef USE_ROOT_CONSTANTS
	const std::array<FLOAT, 4> Color = { 1.0f, 0.0f, 0.0f, 1.0f };
#endif
};
#pragma endregion