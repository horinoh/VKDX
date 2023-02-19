#pragma once

#include "resource.h"

#pragma region Code
#include "../DXImage.h"

class NormalMapDX : public DXImageDepth
{
private:
	using Super = DXImageDepth;
public:
	NormalMapDX() : Super() {}
	virtual ~NormalMapDX() {}

protected:
	virtual void DrawFrame(const UINT i) override {
		const auto CameraPos = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
		const auto WVInv = DirectX::XMMatrixInverse(nullptr, DirectX::XMMatrixMultiply(DirectX::XMLoadFloat4x4(&Tr.World), DirectX::XMLoadFloat4x4(&Tr.View)));
		DirectX::XMStoreFloat4(&Tr.LocalCameraPosition, DirectX::XMVector4Transform(CameraPos, WVInv));

		const auto LightPos = DirectX::XMVector4Transform(DirectX::XMVectorSet(10.0f, 0.0f, 0.0f, 0.0f), DirectX::XMMatrixRotationY(DirectX::XMConvertToRadians(Degree)));
		const auto WInv = DirectX::XMMatrixInverse(nullptr, DirectX::XMLoadFloat4x4(&Tr.World));
		DirectX::XMStoreFloat4(&Tr.LocalLightDirection, DirectX::XMVector3Normalize(DirectX::XMVector4Transform(LightPos, WInv)));

		if (IsUpdate()) {
			Degree += 1.0f;
		}

#pragma region FRAME_OBJECT
		CopyToUploadResource(COM_PTR_GET(ConstantBuffers[i].Resource), RoundUp256(sizeof(Tr)), &Tr);
#pragma endregion
	}
	virtual void CreateGeometry() override { 
		constexpr D3D12_DRAW_INDEXED_ARGUMENTS DIA = {.IndexCountPerInstance = 1, .InstanceCount = 1, .StartIndexLocation = 0, .BaseVertexLocation = 0, .StartInstanceLocation = 0 };
		IndirectBuffers.emplace_back().Create(COM_PTR_GET(Device), DIA).ExecuteCopyCommand(COM_PTR_GET(Device), COM_PTR_GET(DirectCommandAllocators[0]), COM_PTR_GET(DirectCommandLists[0]), COM_PTR_GET(GraphicsCommandQueue), COM_PTR_GET(GraphicsFence), sizeof(DIA), &DIA);
	}
	virtual void CreateConstantBuffer() override {
		constexpr auto Fov = 0.16f * std::numbers::pi_v<float>;
		const auto Aspect = GetAspectRatioOfClientRect();
		constexpr auto ZFar = 100.0f;
		constexpr auto ZNear = ZFar * 0.0001f;
		const auto CamPos = DirectX::XMVectorSet(0.0f, 0.0f, 3.0f, 1.0f);
		const auto CamTag = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
		const auto CamUp = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
		const auto Projection = DirectX::XMMatrixPerspectiveFovRH(Fov, Aspect, ZNear, ZFar);
		const auto View = DirectX::XMMatrixLookAtRH(CamPos, CamTag, CamUp);
		const auto World = DirectX::XMMatrixIdentity();

		DirectX::XMStoreFloat4x4(&Tr.Projection, Projection);
		DirectX::XMStoreFloat4x4(&Tr.View, View);
		DirectX::XMStoreFloat4x4(&Tr.World, World);
		DirectX::XMStoreFloat4(&Tr.LocalCameraPosition, DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f));
		DirectX::XMStoreFloat4(&Tr.LocalLightDirection, DirectX::XMVectorSet(10.0f, 0.0f, 0.0f, 0.0f));

#pragma region FRAME_OBJECT
		DXGI_SWAP_CHAIN_DESC1 SCD;
		SwapChain->GetDesc1(&SCD);
		for (UINT i = 0; i < SCD.BufferCount; ++i) {
			ConstantBuffers.emplace_back().Create(COM_PTR_GET(Device), sizeof(Tr));
		}
#pragma endregion
	}
	virtual void CreateTexture() override {
		std::filesystem::path Path = std::filesystem::path(DDS_DIR);

		const auto CA = COM_PTR_GET(DirectCommandAllocators[0]);
		const auto GCL = COM_PTR_GET(DirectCommandLists[0]);
#ifdef USE_PARALLAX_MAP
		//!< [0] 法線(Normal)
		DDSTextures.emplace_back().Create(COM_PTR_GET(Device), Path / "Leather009_2K-JPG" / "Leather009_2K_Normal.dds").ExecuteCopyCommand(COM_PTR_GET(Device), CA, GCL, COM_PTR_GET(GraphicsCommandQueue), COM_PTR_GET(GraphicsFence), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
		//!< [1] ディスプレースメント(Displacement)
		DDSTextures.emplace_back().Create(COM_PTR_GET(Device), Path / "Leather009_2K-JPG" / "Leather009_2K_Displacement.dds").ExecuteCopyCommand(COM_PTR_GET(Device), CA, GCL, COM_PTR_GET(GraphicsCommandQueue), COM_PTR_GET(GraphicsFence), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
#else
		//!< [0] 法線(Normal)
		DDSTextures.emplace_back().Create(COM_PTR_GET(Device), Path / "Rocks007_2K-JPG" / "Rocks007_2K_Normal.dds").ExecuteCopyCommand(COM_PTR_GET(Device), CA, GCL, COM_PTR_GET(GraphicsCommandQueue), COM_PTR_GET(GraphicsFence), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
		//!< [1] カラー(Color)
		DDSTextures.emplace_back().Create(COM_PTR_GET(Device), Path / "Rocks007_2K-JPG" / "Rocks007_2K_Color.dds").ExecuteCopyCommand(COM_PTR_GET(Device), CA, GCL, COM_PTR_GET(GraphicsCommandQueue), COM_PTR_GET(GraphicsFence), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
#endif

		Super::CreateTexture();
	}
	virtual void CreateStaticSampler() override {
		StaticSamplerDescs.emplace_back(D3D12_STATIC_SAMPLER_DESC({
			.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR,
			.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP, .AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP, .AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP,
			.MipLODBias = 0.0f,
			.MaxAnisotropy = 0,
			.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER,
			.BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE,
			.MinLOD = 0.0f, .MaxLOD = 1.0f,
			.ShaderRegister = 0, .RegisterSpace = 0, .ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL //!< register(s0, space0)
		}));
	}
	virtual void CreateRootSignature() override {
		COM_PTR<ID3DBlob> Blob;
#ifdef USE_HLSL_ROOTSIGNATRUE
		GetRootSignaturePartFromShader(Blob, data(GetBasePath() + TEXT(".rs.cso")));
#else
		constexpr std::array DRs_Cbv = {
			//!< register(b0, space0)
			D3D12_DESCRIPTOR_RANGE({.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV, .NumDescriptors = 1, .BaseShaderRegister = 0, .RegisterSpace = 0, .OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND }) 
		};
		constexpr std::array DRs_Srv = {
			//!< register(t0, space0), register(t1, space0)
			D3D12_DESCRIPTOR_RANGE({.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV, .NumDescriptors = 2, .BaseShaderRegister = 0, .RegisterSpace = 0, .OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND }) 
		};
		assert(!empty(StaticSamplerDescs) && "");
		DX::SerializeRootSignature(Blob, {
			//!< CBV
			D3D12_ROOT_PARAMETER({ 
				.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE, 
				.DescriptorTable = D3D12_ROOT_DESCRIPTOR_TABLE({ .NumDescriptorRanges = static_cast<UINT>(size(DRs_Cbv)), .pDescriptorRanges = data(DRs_Cbv) }), 
				.ShaderVisibility = D3D12_SHADER_VISIBILITY_GEOMETRY 
			}), 
			//!< SRV0, SRV1
			D3D12_ROOT_PARAMETER({ 
				.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE, 
				.DescriptorTable = D3D12_ROOT_DESCRIPTOR_TABLE({ .NumDescriptorRanges = static_cast<UINT>(size(DRs_Srv)), .pDescriptorRanges = data(DRs_Srv) }),
				.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL 
			}),
		}, {
			StaticSamplerDescs[0],
		}, SHADER_ROOT_ACCESS_GS_PS);
#endif
		VERIFY_SUCCEEDED(Device->CreateRootSignature(0, Blob->GetBufferPointer(), Blob->GetBufferSize(), COM_PTR_UUIDOF_PUTVOID(RootSignatures.emplace_back())));
		LOG_OK();
	}
	virtual void CreatePipelineState() override {
		const auto ShaderPath = GetBasePath();
		std::vector<COM_PTR<ID3DBlob>> SBs;
#ifdef USE_PARALLAX_MAP
		VERIFY_SUCCEEDED(D3DReadFileToBlob(data(ShaderPath + TEXT(".vs.cso")), COM_PTR_PUT(SBs.emplace_back())));
		VERIFY_SUCCEEDED(D3DReadFileToBlob(data(ShaderPath + TEXT("_pm.ps.cso")), COM_PTR_PUT(SBs.emplace_back())));
		VERIFY_SUCCEEDED(D3DReadFileToBlob(data(ShaderPath + TEXT(".ds.cso")), COM_PTR_PUT(SBs.emplace_back())));
		VERIFY_SUCCEEDED(D3DReadFileToBlob(data(ShaderPath + TEXT(".hs.cso")), COM_PTR_PUT(SBs.emplace_back())));
		VERIFY_SUCCEEDED(D3DReadFileToBlob(data(ShaderPath + TEXT(".gs.cso")), COM_PTR_PUT(SBs.emplace_back())));
#else
		VERIFY_SUCCEEDED(D3DReadFileToBlob(data(ShaderPath + TEXT(".vs.cso")), COM_PTR_PUT(SBs.emplace_back())));
		VERIFY_SUCCEEDED(D3DReadFileToBlob(data(ShaderPath + TEXT(".ps.cso")), COM_PTR_PUT(SBs.emplace_back())));
		VERIFY_SUCCEEDED(D3DReadFileToBlob(data(ShaderPath + TEXT(".ds.cso")), COM_PTR_PUT(SBs.emplace_back())));
		VERIFY_SUCCEEDED(D3DReadFileToBlob(data(ShaderPath + TEXT(".hs.cso")), COM_PTR_PUT(SBs.emplace_back())));
		VERIFY_SUCCEEDED(D3DReadFileToBlob(data(ShaderPath + TEXT(".gs.cso")), COM_PTR_PUT(SBs.emplace_back())));
#endif
		const std::array SBCs = {
			D3D12_SHADER_BYTECODE({.pShaderBytecode = SBs[0]->GetBufferPointer(), .BytecodeLength = SBs[0]->GetBufferSize() }),
			D3D12_SHADER_BYTECODE({.pShaderBytecode = SBs[1]->GetBufferPointer(), .BytecodeLength = SBs[1]->GetBufferSize() }),
			D3D12_SHADER_BYTECODE({.pShaderBytecode = SBs[2]->GetBufferPointer(), .BytecodeLength = SBs[2]->GetBufferSize() }),
			D3D12_SHADER_BYTECODE({.pShaderBytecode = SBs[3]->GetBufferPointer(), .BytecodeLength = SBs[3]->GetBufferSize() }),
			D3D12_SHADER_BYTECODE({.pShaderBytecode = SBs[4]->GetBufferPointer(), .BytecodeLength = SBs[4]->GetBufferSize() }),
		};

		constexpr D3D12_RASTERIZER_DESC RD = {
			.FillMode = D3D12_FILL_MODE_SOLID,
			.CullMode = D3D12_CULL_MODE_BACK, .FrontCounterClockwise = TRUE,
			.DepthBias = D3D12_DEFAULT_DEPTH_BIAS, .DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP, .SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS,
			.DepthClipEnable = TRUE,
			.MultisampleEnable = FALSE, .AntialiasedLineEnable = FALSE, .ForcedSampleCount = 0,
			.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF
		};
		CreatePipelineState_VsPsDsHsGs(D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH, RD, TRUE, SBCs);
	}
	virtual void CreateDescriptor() override {
		{
#pragma region FRAME_OBJECT
			DXGI_SWAP_CHAIN_DESC1 SCD;
			SwapChain->GetDesc1(&SCD);
			const D3D12_DESCRIPTOR_HEAP_DESC DHD = { D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, SCD.BufferCount + 2, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE, 0 }; //!< CBV * N, SRV0, SRV1
#pragma endregion
			VERIFY_SUCCEEDED(Device->CreateDescriptorHeap(&DHD, COM_PTR_UUIDOF_PUTVOID(CbvSrvUavDescriptorHeaps.emplace_back())));
		}

		{
			CbvSrvUavGPUHandles.emplace_back();
			auto CDH = CbvSrvUavDescriptorHeaps[0]->GetCPUDescriptorHandleForHeapStart();
			auto GDH = CbvSrvUavDescriptorHeaps[0]->GetGPUDescriptorHandleForHeapStart();
			const auto IncSize = Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
#pragma region FRAME_OBJECT
			DXGI_SWAP_CHAIN_DESC1 SCD;
			SwapChain->GetDesc1(&SCD);
			for (UINT i = 0; i < SCD.BufferCount; ++i) {
				const D3D12_CONSTANT_BUFFER_VIEW_DESC CBVD = { ConstantBuffers[i].Resource->GetGPUVirtualAddress(), static_cast<UINT>(ConstantBuffers[i].Resource->GetDesc().Width) };
				//!< CBV
				Device->CreateConstantBufferView(&CBVD, CDH); 
				CbvSrvUavGPUHandles.back().emplace_back(GDH);
				CDH.ptr += IncSize;
				GDH.ptr += IncSize; 
			}
#pragma endregion
			//!< SRV0
			Device->CreateShaderResourceView(COM_PTR_GET(DDSTextures[0].Resource), &DDSTextures[0].SRV, CDH); 
			CbvSrvUavGPUHandles.back().emplace_back(GDH);
			CDH.ptr += IncSize;
			GDH.ptr += IncSize;
			//!< SRV1
			Device->CreateShaderResourceView(COM_PTR_GET(DDSTextures[1].Resource), &DDSTextures[1].SRV, CDH);
			CbvSrvUavGPUHandles.back().emplace_back(GDH);
		}

		Super::CreateDescriptor();
	}
	virtual void PopulateCommandList(const size_t i) override {
		const auto PS = COM_PTR_GET(PipelineStates[0]);

		const auto BGCL = COM_PTR_GET(BundleCommandLists[i]);
		const auto BCA = COM_PTR_GET(BundleCommandAllocators[0]);
		VERIFY_SUCCEEDED(BGCL->Reset(BCA, PS));
		{
			BGCL->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_1_CONTROL_POINT_PATCHLIST);
			BGCL->ExecuteIndirect(COM_PTR_GET(IndirectBuffers[0].CommandSignature), 1, COM_PTR_GET(IndirectBuffers[0].Resource), 0, nullptr, 0);
		}
		VERIFY_SUCCEEDED(BGCL->Close());

		const auto GCL = COM_PTR_GET(DirectCommandLists[i]);
		const auto CA = COM_PTR_GET(DirectCommandAllocators[0]);
		VERIFY_SUCCEEDED(GCL->Reset(CA, PS));
		{
			const auto RS = COM_PTR_GET(RootSignatures[0]);
			const auto SCR = COM_PTR_GET(SwapChainResources[i]);

			GCL->SetGraphicsRootSignature(RS);

			GCL->RSSetViewports(static_cast<UINT>(size(Viewports)), data(Viewports));
			GCL->RSSetScissorRects(static_cast<UINT>(size(ScissorRects)), data(ScissorRects));

			ResourceBarrier(GCL, SCR, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
			{
				constexpr std::array<D3D12_RECT, 0> Rects = {};
				GCL->ClearRenderTargetView(SwapChainCPUHandles[i], DirectX::Colors::SkyBlue, static_cast<UINT>(size(Rects)), data(Rects));
				GCL->ClearDepthStencilView(DsvCPUHandles.back()[0], D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, static_cast<UINT>(size(Rects)), data(Rects));

				const std::array CHs = { SwapChainCPUHandles[i] };
				GCL->OMSetRenderTargets(static_cast<UINT>(size(CHs)), data(CHs), FALSE, &DsvCPUHandles.back()[0]);

				{
					const std::array DHs = { COM_PTR_GET(CbvSrvUavDescriptorHeaps[0]) };
					GCL->SetDescriptorHeaps(static_cast<UINT>(size(DHs)), data(DHs));

					DXGI_SWAP_CHAIN_DESC1 SCD;
					SwapChain->GetDesc1(&SCD);
#pragma region FRAME_OBJECT
					//!< CBV
					GCL->SetGraphicsRootDescriptorTable(0, CbvSrvUavGPUHandles.back()[i]); 
#pragma endregion
					//!< SRV0, SRV1
					GCL->SetGraphicsRootDescriptorTable(1, CbvSrvUavGPUHandles.back()[SCD.BufferCount]);
				}

				GCL->ExecuteBundle(BGCL);
			}
			ResourceBarrier(GCL, SCR, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
		}
		VERIFY_SUCCEEDED(GCL->Close());
	}

private:
	struct Transform
	{
		DirectX::XMFLOAT4X4 Projection;
		DirectX::XMFLOAT4X4 View;
		DirectX::XMFLOAT4X4 World;
		DirectX::XMFLOAT4 LocalCameraPosition;
		DirectX::XMFLOAT4 LocalLightDirection;
	};
	using Transform = struct Transform;
	FLOAT Degree = 0.0f;
	Transform Tr;
};
#pragma endregion