#pragma once

#include "resource.h"

#pragma region Code
#include "../DXExt.h"

class AnimatedTextureDX : public DXExt
{
private:
	using Super = DXExt;
public:
	AnimatedTextureDX() : Super() {}
	virtual ~AnimatedTextureDX() {}

protected:
	static const uint32_t W = 1280, H = 720;
	std::array<uint32_t, W * H> TexPattern;
	//virtual void DrawFrame(const UINT i) override {
	//	if (0 == i) {
	//		std::random_device RndDev;
	//		std::ranges::generate(TexPattern, [&]() { return RndDev(); });

	//		constexpr auto Bpp = 4;
	//		constexpr auto Layers = 1;
	//		AnimatedTextures[0].UpdateUploadBuffer(W, H, Bpp, Layers, std::data(TexPattern));
	//	}
	//}
	//virtual void PopulateAnimatedTextureCommand(const size_t i) override {
	//	const auto DCL = COM_PTR_GET(DirectCommandLists[i]);

	//	constexpr auto Bpp = 4;
	//	AnimatedTextures[0].PopulateUploadToTextureCommand(DCL, Bpp);
	//}
	virtual void CreateGeometry() override {
		constexpr D3D12_DRAW_ARGUMENTS DA = { .VertexCountPerInstance = 4, .InstanceCount = 1, .StartVertexLocation = 0, .StartInstanceLocation = 0 };
		IndirectBuffers.emplace_back().Create(COM_PTR_GET(Device), DA).ExecuteCopyCommand(COM_PTR_GET(Device), COM_PTR_GET(DirectCommandAllocators[0]), COM_PTR_GET(DirectCommandLists[0]), COM_PTR_GET(GraphicsCommandQueue), COM_PTR_GET(GraphicsFence), sizeof(DA), &DA);
	}
	virtual void CreateTexture() override {
		constexpr auto Layers = 1;
		AnimatedTextures.emplace_back().Create(COM_PTR_GET(Device), W, H, Layers, DXGI_FORMAT_R8G8B8A8_UNORM, D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE);
	}
	virtual void CreateStaticSampler() override {
		CreateStaticSampler_LinearWrap(0, 0, D3D12_SHADER_VISIBILITY_PIXEL);
	}
	virtual void CreateRootSignature() override {
		COM_PTR<ID3DBlob> Blob;
#ifdef USE_HLSL_ROOTSIGNATRUE 
		GetRootSignaturePartFromShader(Blob, data(GetBasePath() + TEXT(".rs.cso")));
#else
		constexpr std::array DRs_Srv = {
			D3D12_DESCRIPTOR_RANGE1({.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV, .NumDescriptors = 1, .BaseShaderRegister = 0, .RegisterSpace = 0,.Flags = D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE, .OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND })
		};
		DX::SerializeRootSignature(Blob, {
			//!< SRV
			D3D12_ROOT_PARAMETER1({
				.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE,
				.DescriptorTable = D3D12_ROOT_DESCRIPTOR_TABLE1({.NumDescriptorRanges = static_cast<uint32_t>(size(DRs_Srv)), .pDescriptorRanges = data(DRs_Srv) }),
				.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL
			}),
			}, {
				StaticSamplerDescs[0],
			}, SHADER_ROOT_ACCESS_PS);
#endif
		VERIFY_SUCCEEDED(Device->CreateRootSignature(0, Blob->GetBufferPointer(), Blob->GetBufferSize(), COM_PTR_UUIDOF_PUTVOID(RootSignatures.emplace_back())));
		LOG_OK();
	}
	virtual void CreatePipelineState() override {
		PipelineStates.emplace_back();

		std::vector<COM_PTR<ID3DBlob>> SBs = {};
		VERIFY_SUCCEEDED(D3DReadFileToBlob(data(GetFilePath(".vs.cso").wstring()), COM_PTR_PUT(SBs.emplace_back())));
		VERIFY_SUCCEEDED(D3DReadFileToBlob(data(GetFilePath(".ps.cso").wstring()), COM_PTR_PUT(SBs.emplace_back())));
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
		CreatePipelineState_VsPs(PipelineStates[0], COM_PTR_GET(RootSignatures[0]), D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE, RD, FALSE, SBCs);

		for (auto& i : Threads) { i.join(); }
		Threads.clear();
	}
	virtual void CreateDescriptor() override {
		{
			auto& Desc = CbvSrvUavDescs.emplace_back();
			auto& Heap = Desc.first;
			auto& Handle = Desc.second;

			const D3D12_DESCRIPTOR_HEAP_DESC DHD = { .Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, .NumDescriptors = 1, .Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE, .NodeMask = 0 }; //!< SRV
			VERIFY_SUCCEEDED(Device->CreateDescriptorHeap(&DHD, COM_PTR_UUIDOF_PUTVOID(Heap)));

			auto CDH = Heap->GetCPUDescriptorHandleForHeapStart();
			const auto& Tex = AnimatedTextures[0];
			Device->CreateShaderResourceView(COM_PTR_GET(Tex.Resource), &Tex.SRV, CDH);

			auto GDH = Heap->GetGPUDescriptorHandleForHeapStart();
			Handle.emplace_back(GDH);
		}
	}

	virtual void PopulateBundleCommandList(const size_t i) override {
		const auto PS = COM_PTR_GET(PipelineStates[0]);
		const auto BCA = COM_PTR_GET(BundleCommandAllocators[0]);
		const auto BCL = COM_PTR_GET(BundleCommandLists[i]);

		VERIFY_SUCCEEDED(BCL->Reset(BCA, PS));
		{
			BCL->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
			BCL->ExecuteIndirect(COM_PTR_GET(IndirectBuffers[0].CommandSignature), 1, COM_PTR_GET(IndirectBuffers[0].Resource), 0, nullptr, 0);
		}
		VERIFY_SUCCEEDED(BCL->Close());
	}
	virtual void PopulateCommandList(const size_t i) override {
		const auto BCL = COM_PTR_GET(BundleCommandLists[i]);
		const auto CL = COM_PTR_GET(DirectCommandLists[i]);
		const auto CA = COM_PTR_GET(DirectCommandAllocators[0]);

		VERIFY_SUCCEEDED(CL->Reset(CA, nullptr));
		{
			CL->SetGraphicsRootSignature(COM_PTR_GET(RootSignatures[0]));

			CL->RSSetViewports(static_cast<UINT>(size(Viewports)), data(Viewports));
			CL->RSSetScissorRects(static_cast<UINT>(size(ScissorRects)), data(ScissorRects));

			const auto SCR = COM_PTR_GET(SwapChainBackBuffers[i].Resource);
			ResourceBarrier(CL, SCR, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
			{
				const std::array CHs = { SwapChainBackBuffers[i].Handle };
				CL->OMSetRenderTargets(static_cast<UINT>(size(CHs)), data(CHs), FALSE, nullptr);

				const auto& DescSRV = CbvSrvUavDescs[0];
				const auto& HeapSRV = DescSRV.first;
				const auto& HandleSRV = DescSRV.second;

				const std::array DHs = { COM_PTR_GET(HeapSRV) };
				CL->SetDescriptorHeaps(static_cast<UINT>(size(DHs)), data(DHs));
				CL->SetGraphicsRootDescriptorTable(0, HandleSRV[0]); //!< SRV

				CL->ExecuteBundle(BCL);
			}
			ResourceBarrier(CL, SCR, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
		}
		VERIFY_SUCCEEDED(CL->Close());
	}
};
#pragma endregion