#pragma once

#include "resource.h"

#pragma region Code
#include "../DXExt.h"

class RenderTargetDX : public DXExt
{
private:
	using Super = DXExt;
public:
	RenderTargetDX() : Super() {}
	virtual ~RenderTargetDX() {}

protected:
#ifdef USE_BUNDLE
	virtual void CreateBundleCommandList() override { AddBundleCommandList(); }
#endif
	virtual void CreateIndirectBuffer() override { CreateIndirectBuffer_DrawIndexed(1, 1); }
	//virtual void CreateIndirectBuffer() override { CreateIndirectBuffer_Draw(4, 1); }
	virtual void CreateRootSignature() override {
		RootSignatures.resize(2);

		{
			COM_PTR<ID3DBlob> Blob;
#ifdef USE_HLSL_ROOTSIGNATRUE
			GetRootSignaturePartFromShader(Blob, (GetBasePath() + TEXT(".rs.cso")).data());
#else
			SerializeRootSignature(Blob, {}, {}, D3D12_ROOT_SIGNATURE_FLAG_NONE);
#endif
			DX::CreateRootSignature(RootSignatures[0], Blob);
		}

		{
			COM_PTR<ID3DBlob> Blob;
#ifdef USE_HLSL_ROOTSIGNATRUE 
			GetRootSignaturePartFromShader(Blob, (GetBasePath() + TEXT(".rs.cso")).data());
#else
			const std::array<D3D12_DESCRIPTOR_RANGE, 1> DRs_Srv = {
				{ D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND }
			};
			assert(!StaticSamplerDescs.empty() && "");
			DX::SerializeRootSignature(Blob, {
					{ D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE, { static_cast<uint32_t>(DRs_Srv.size()), DRs_Srv.data() }, D3D12_SHADER_VISIBILITY_PIXEL }
				}, {
					StaticSamplerDescs[0],
				}, D3D12_ROOT_SIGNATURE_FLAG_NONE);
#endif
			DX::CreateRootSignature(RootSignatures[1], Blob);
		}
		LOG_OK();
	}
	virtual void CreateStaticSampler() override {
		StaticSamplerDescs.push_back({
			D3D12_FILTER_MIN_MAG_MIP_LINEAR,
			D3D12_TEXTURE_ADDRESS_MODE_WRAP, D3D12_TEXTURE_ADDRESS_MODE_WRAP, D3D12_TEXTURE_ADDRESS_MODE_WRAP,
			0.0f,
			0,
			D3D12_COMPARISON_FUNC_NEVER,
			D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE,
			0.0f, 1.0f,
			0, 0, D3D12_SHADER_VISIBILITY_PIXEL
			});
	}
	virtual void CreateShaderBlob() override {
		ShaderBlobs.resize(5 + 2);
		const auto ShaderPath = GetBasePath();
		VERIFY_SUCCEEDED(D3DReadFileToBlob((ShaderPath + TEXT(".vs.cso")).data(), COM_PTR_PUT(ShaderBlobs[0])));
		VERIFY_SUCCEEDED(D3DReadFileToBlob((ShaderPath + TEXT(".ps.cso")).data(), COM_PTR_PUT(ShaderBlobs[1])));
		VERIFY_SUCCEEDED(D3DReadFileToBlob((ShaderPath + TEXT(".ds.cso")).data(), COM_PTR_PUT(ShaderBlobs[2])));
		VERIFY_SUCCEEDED(D3DReadFileToBlob((ShaderPath + TEXT(".hs.cso")).data(), COM_PTR_PUT(ShaderBlobs[3])));
		VERIFY_SUCCEEDED(D3DReadFileToBlob((ShaderPath + TEXT(".gs.cso")).data(), COM_PTR_PUT(ShaderBlobs[4])));

		VERIFY_SUCCEEDED(D3DReadFileToBlob((ShaderPath + TEXT("_1") + TEXT(".vs.cso")).data(), COM_PTR_PUT(ShaderBlobs[5]))); //!<
		VERIFY_SUCCEEDED(D3DReadFileToBlob((ShaderPath + TEXT("_1") + TEXT(".ps.cso")).data(), COM_PTR_PUT(ShaderBlobs[6]))); //!< 
	}
	virtual void CreatePipelineState() override {
		PipelineStates.resize(2); //!< 
#ifdef USE_PIPELINE_SERIALIZE
		PipelineLibrarySerializer PLS(COM_PTR_GET(Device), GetBasePath() + TEXT(".plo"));
#endif
		std::vector<std::thread> Threads;
		{
			const std::array<D3D12_SHADER_BYTECODE, 5> SBCs = { {
				{ ShaderBlobs[0]->GetBufferPointer(), ShaderBlobs[0]->GetBufferSize() },
				{ ShaderBlobs[1]->GetBufferPointer(), ShaderBlobs[1]->GetBufferSize() },
				{ ShaderBlobs[2]->GetBufferPointer(), ShaderBlobs[2]->GetBufferSize() },
				{ ShaderBlobs[3]->GetBufferPointer(), ShaderBlobs[3]->GetBufferSize() },
				{ ShaderBlobs[4]->GetBufferPointer(), ShaderBlobs[4]->GetBufferSize() },
			} };

			Threads.push_back(std::thread::thread([&](COM_PTR<ID3D12PipelineState>& PST, ID3D12RootSignature* RS,
				const D3D12_SHADER_BYTECODE VS, const D3D12_SHADER_BYTECODE PS, const D3D12_SHADER_BYTECODE DS, const D3D12_SHADER_BYTECODE HS, const D3D12_SHADER_BYTECODE GS)
				{
#ifdef USE_PIPELINE_SERIALIZE
					DX::CreatePipelineState(PST, COM_PTR_GET(Device), RS, D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH, VS, PS, DS, HS, GS, {}, &PLS, TEXT("0"));
#else
					DX::CreatePipelineState(PST, COM_PTR_GET(Device), RS, D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH, VS, PS, DS, HS, GS, {});
#endif
				},
				std::ref(PipelineStates[0]), COM_PTR_GET(RootSignatures[0]), SBCs[0], SBCs[1], SBCs[2], SBCs[3], SBCs[4]));
		}
		{
			const std::array<D3D12_SHADER_BYTECODE, 2> SBCs = { {
				{ ShaderBlobs[5]->GetBufferPointer(), ShaderBlobs[5]->GetBufferSize() }, //!< 
				{ ShaderBlobs[6]->GetBufferPointer(), ShaderBlobs[6]->GetBufferSize() }, //!< 
			} };
			Threads.push_back(std::thread::thread([&](COM_PTR<ID3D12PipelineState>& PST, ID3D12RootSignature* RS,
				const D3D12_SHADER_BYTECODE VS, const D3D12_SHADER_BYTECODE PS, const D3D12_SHADER_BYTECODE DS, const D3D12_SHADER_BYTECODE HS, const D3D12_SHADER_BYTECODE GS)
				{
#ifdef USE_PIPELINE_SERIALIZE
					DX::CreatePipelineState(PST, COM_PTR_GET(Device), RS, D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE, VS, PS, DS, HS, GS, {}, &PLS, TEXT("1"));
#else
					DX::CreatePipelineState(PST, COM_PTR_GET(Device), RS, D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE, VS, PS, DS, HS, GS, {});
#endif
				},
				std::ref(PipelineStates[1]), COM_PTR_GET(RootSignatures[1]), SBCs[0], SBCs[1], NullShaderBC, NullShaderBC, NullShaderBC)); //!< 
		}
		for (auto& i : Threads) {
			i.join();
		}
	}
	virtual void PopulateCommandList(const size_t i) override;
};
#pragma endregion