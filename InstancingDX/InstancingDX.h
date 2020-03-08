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
#ifdef USE_BUNDLE
	virtual void CreateBundleCommandList() override { AddBundleCommandList(); }
#endif
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
	virtual void CreateShaderBlob() override { CreateShaderBlob_VsPs(); }
	virtual void CreatePipelineState() override {
		PipelineStates.resize(1);

#ifdef USE_PIPELINE_SERIALIZE
		const auto PCOPath = GetBasePath() + TEXT(".plo");
		PipelineLibrarySerializer PLS(COM_PTR_GET(Device), PCOPath.c_str());
#endif
		const auto RS = COM_PTR_GET(RootSignatures[0]);
		const D3D12_SHADER_BYTECODE SBC_VS = { ShaderBlobs[0]->GetBufferPointer(), ShaderBlobs[0]->GetBufferSize() };
		const D3D12_SHADER_BYTECODE SBC_PS = { ShaderBlobs[1]->GetBufferPointer(), ShaderBlobs[1]->GetBufferSize() };
		const std::vector<D3D12_INPUT_ELEMENT_DESC> IEDs = { {
			//!< Per Vertex
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(Vertex_PositionColor, Position), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, offsetof(Vertex_PositionColor, Color), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			//!< Per Instance
			{ "OFFSET", 0, DXGI_FORMAT_R32G32_FLOAT, 1, offsetof(Instance_OffsetXY, Offset), D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 },
		} };

		std::vector<std::thread> Threads;
		Threads.push_back(std::thread::thread([&](COM_PTR<ID3D12PipelineState>& PST, ID3D12RootSignature* RS, const D3D12_SHADER_BYTECODE VS, const D3D12_SHADER_BYTECODE PS)
			{
#ifdef USE_PIPELINE_SERIALIZE
				DX::CreatePipelineState(PST, COM_PTR_GET(Device), RS, D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE, VS, PS, NullShaderBC, NullShaderBC, NullShaderBC, IEDs, &PLS, TEXT("0"));
#else
				DX::CreatePipelineState(PST, COM_PTR_GET(Device), RS, D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE, VS, PS, NullShaderBC, NullShaderBC, NullShaderBC, IEDs);
#endif
			},
			std::ref(PipelineStates[0]), RS, SBC_VS, SBC_PS));

		for (auto& i : Threads) { i.join(); }
	}
	virtual void PopulateCommandList(const size_t i) override;

	UINT IndexCount = 0;
	UINT InstanceCount = 0;
};
#pragma endregion