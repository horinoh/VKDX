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
	virtual void CreateIndirectBuffer() override { CreateIndirectBuffer_DrawIndexed(1, 1); } //!< メッシュ描画用
	//virtual void CreateIndirectBuffer() override { CreateIndirectBuffer_Draw(4, 1); } //!< フルスクリーン描画用 #DX_TODO
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
	virtual void CreateRootSignature() override {
		RootSignatures.resize(2);

		{
			COM_PTR<ID3DBlob> Blob;
#ifdef USE_HLSL_ROOTSIGNATRUE
			GetRootSignaturePartFromShader(Blob, (GetBasePath() + TEXT(".rs.cso")).data());
#else
			SerializeRootSignature(Blob, {}, {}, D3D12_ROOT_SIGNATURE_FLAG_NONE
				| D3D12_ROOT_SIGNATURE_FLAG_DENY_VERTEX_SHADER_ROOT_ACCESS
				| D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS
				| D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS
				| D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS
				| D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS);
#endif
			VERIFY_SUCCEEDED(Device->CreateRootSignature(0, Blob->GetBufferPointer(), Blob->GetBufferSize(), COM_PTR_UUIDOF_PUTVOID(RootSignatures[0])));
		}

		{
			COM_PTR<ID3DBlob> Blob;
#ifdef USE_HLSL_ROOTSIGNATRUE 
			GetRootSignaturePartFromShader(Blob, (GetBasePath() + TEXT("_1.rs.cso")).data());
#else
			const std::array<D3D12_DESCRIPTOR_RANGE, 1> DRs_Srv = {
				{ D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND }
			};
			assert(!StaticSamplerDescs.empty() && "");
			DX::SerializeRootSignature(Blob, {
					{ D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE, { static_cast<uint32_t>(DRs_Srv.size()), DRs_Srv.data() }, D3D12_SHADER_VISIBILITY_PIXEL }
				}, {
					StaticSamplerDescs[0],
				}, D3D12_ROOT_SIGNATURE_FLAG_NONE
				| D3D12_ROOT_SIGNATURE_FLAG_DENY_VERTEX_SHADER_ROOT_ACCESS
				| D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS
				| D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS
				| D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS
				//| D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS
			);
#endif
			VERIFY_SUCCEEDED(Device->CreateRootSignature(0, Blob->GetBufferPointer(), Blob->GetBufferSize(), COM_PTR_UUIDOF_PUTVOID(RootSignatures[1])));
		}
		LOG_OK();
	}
	virtual void CreateTexture() 
	{
		//!< #DX_TODO
	}

	virtual void CreateDescriptorHeap() override {
		{
			RtvDescriptorHeaps.resize(1);
			const D3D12_DESCRIPTOR_HEAP_DESC DHD = { D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 1, D3D12_DESCRIPTOR_HEAP_FLAG_NONE, 0 };
			VERIFY_SUCCEEDED(Device->CreateDescriptorHeap(&DHD, COM_PTR_UUIDOF_PUTVOID(RtvDescriptorHeaps[0])));
		}
		{
			CbvSrvUavDescriptorHeaps.resize(1);
			const D3D12_DESCRIPTOR_HEAP_DESC DHD = { D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 1, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE, 0 };
			VERIFY_SUCCEEDED(Device->CreateDescriptorHeap(&DHD, COM_PTR_UUIDOF_PUTVOID(CbvSrvUavDescriptorHeaps[0])));
		}
	}
	virtual void CreateDescriptorView() override {
		//{
		//	const auto& DH = RtvDescriptorHeaps[0];
		//	auto CDH = DH->GetCPUDescriptorHandleForHeapStart();
		//	const D3D12_DESCRIPTOR_HEAP_DESC DHD = {
		//		D3D12_DESCRIPTOR_HEAP_TYPE_RTV,
		//		1,
		//		D3D12_DESCRIPTOR_HEAP_FLAG_NONE,
		//		0
		//	};
		//	VERIFY_SUCCEEDED(Device->CreateDescriptorHeap(&DHD, COM_PTR_UUIDOF_PUTVOID(RtvDescriptorHeaps[0])));
		//	Device->CreateRenderTargetView(COM_PTR_GET(RenderTargetResource), nullptr, CDH); CDH.ptr += Device->GetDescriptorHandleIncrementSize(DH->GetDesc().Type);
		//}
		{
			const auto& DH = CbvSrvUavDescriptorHeaps[0];
			auto CDH = DH->GetCPUDescriptorHandleForHeapStart();
			D3D12_SHADER_RESOURCE_VIEW_DESC SRVD = {
				DXGI_FORMAT_R8G8B8A8_UNORM,
				D3D12_SRV_DIMENSION_TEXTURE2D,
				D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING,
			};
			SRVD.Texture2D.MostDetailedMip = 0;
			SRVD.Texture2D.MipLevels = 1;
			SRVD.Texture2D.PlaneSlice = 0;
			SRVD.Texture2D.ResourceMinLODClamp = 0.0f;
			Device->CreateShaderResourceView(COM_PTR_GET(RenderTargetResource), &SRVD, CDH); CDH.ptr += Device->GetDescriptorHandleIncrementSize(DH->GetDesc().Type);
		}
	}
	
	virtual void CreateShaderBlobs() override {
		ShaderBlobs.resize(5 + 2);
		const auto ShaderPath = GetBasePath();
		VERIFY_SUCCEEDED(D3DReadFileToBlob((ShaderPath + TEXT(".vs.cso")).data(), COM_PTR_PUT(ShaderBlobs[0])));
		VERIFY_SUCCEEDED(D3DReadFileToBlob((ShaderPath + TEXT(".ps.cso")).data(), COM_PTR_PUT(ShaderBlobs[1])));
		VERIFY_SUCCEEDED(D3DReadFileToBlob((ShaderPath + TEXT(".ds.cso")).data(), COM_PTR_PUT(ShaderBlobs[2])));
		VERIFY_SUCCEEDED(D3DReadFileToBlob((ShaderPath + TEXT(".hs.cso")).data(), COM_PTR_PUT(ShaderBlobs[3])));
		VERIFY_SUCCEEDED(D3DReadFileToBlob((ShaderPath + TEXT(".gs.cso")).data(), COM_PTR_PUT(ShaderBlobs[4])));

		VERIFY_SUCCEEDED(D3DReadFileToBlob((ShaderPath + TEXT("_1.vs.cso")).data(), COM_PTR_PUT(ShaderBlobs[5])));
		VERIFY_SUCCEEDED(D3DReadFileToBlob((ShaderPath + TEXT("_1.ps.cso")).data(), COM_PTR_PUT(ShaderBlobs[6])));
	}
	virtual void CreatePipelineStates() override {
		PipelineStates.resize(2);
		std::vector<std::thread> Threads;
		const D3D12_DEPTH_STENCILOP_DESC DSOD = { D3D12_STENCIL_OP_KEEP, D3D12_STENCIL_OP_KEEP, D3D12_STENCIL_OP_KEEP, D3D12_COMPARISON_FUNC_ALWAYS };
		const D3D12_DEPTH_STENCIL_DESC DSD = {
			FALSE, D3D12_DEPTH_WRITE_MASK_ALL, D3D12_COMPARISON_FUNC_LESS,
			FALSE, D3D12_DEFAULT_STENCIL_READ_MASK, D3D12_DEFAULT_STENCIL_WRITE_MASK,
			DSOD, DSOD
		};
		const std::vector<D3D12_INPUT_ELEMENT_DESC> IEDs = {};
#ifdef USE_PIPELINE_SERIALIZE
		PipelineLibrarySerializer PLS(COM_PTR_GET(Device), GetBasePath() + TEXT(".plo"));
		Threads.push_back(std::thread::thread(DX::CreatePipelineState, std::ref(PipelineStates[0]), COM_PTR_GET(Device), COM_PTR_GET(RootSignatures[0]), D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH, DSD, ToShaderBC(ShaderBlobs[0]), ToShaderBC(ShaderBlobs[1]), ToShaderBC(ShaderBlobs[2]), ToShaderBC(ShaderBlobs[3]), ToShaderBC(ShaderBlobs[4]), IEDs, &PLS, TEXT("0")));
		Threads.push_back(std::thread::thread(DX::CreatePipelineState, std::ref(PipelineStates[1]), COM_PTR_GET(Device), COM_PTR_GET(RootSignatures[1]), D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE, DSD, ToShaderBC(ShaderBlobs[5]), ToShaderBC(ShaderBlobs[6]), NullShaderBC, NullShaderBC, NullShaderBC, IEDs, &PLS, TEXT("1")));
#else
		Threads.push_back(std::thread::thread(DX::CreatePipelineState, std::ref(PipelineStates[0]), COM_PTR_GET(Device), COM_PTR_GET(RootSignatures[0]), D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH, DSD, ToShaderBC(ShaderBlobs[0]), ToShaderBC(ShaderBlobs[1]), ToShaderBC(ShaderBlobs[2]), ToShaderBC(ShaderBlobs[3]), ToShaderBC(ShaderBlobs[4]), IEDs, nullptr, nullptr));
		Threads.push_back(std::thread::thread(DX::CreatePipelineState, std::ref(PipelineStates[1]), COM_PTR_GET(Device), COM_PTR_GET(RootSignatures[1]), D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE, DSD, ToShaderBC(ShaderBlobs[5]), ToShaderBC(ShaderBlobs[6]), NullShaderBC, NullShaderBC, NullShaderBC, IEDs, nullptr, nullptr));
#endif	
		for (auto& i : Threads) { i.join(); }
	}
	virtual void PopulateCommandList(const size_t i) override;
};
#pragma endregion