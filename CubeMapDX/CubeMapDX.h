#pragma once

#include "resource.h"

#pragma region Code
#include "../DXImage.h"

class CubeMapDX : public DXImage
{
private:
	using Super = DXImage;
public:
	CubeMapDX() : Super() {}
	virtual ~CubeMapDX() {}

protected:
	virtual void OnTimer(HWND hWnd, HINSTANCE hInstance) override {
		Super::OnTimer(hWnd, hInstance);

		const auto CamPos = DirectX::XMVector3Transform(DirectX::XMVectorSet(0.0f, 0.0f, 3.0f, 1.0f), DirectX::XMMatrixRotationY(DirectX::XMConvertToRadians(Degree)));
#ifdef USE_SKY_DOME
		const auto View = DirectX::XMMatrixLookAtRH(CamPos, DirectX::XMVectorSet(0.0f, 2.0f * DirectX::XMScalarSin(DirectX::XMConvertToRadians(Degree)), 0.0f, 1.0f), DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f));
		DirectX::XMStoreFloat4x4(&Tr.View, View);
		DirectX::XMStoreFloat4x4(&Tr.World, DirectX::XMMatrixTranslationFromVector(CamPos)); //!< カメラ位置に球を配置
#else
		const auto View = DirectX::XMMatrixLookAtRH(CamPos, DirectX::XMVectorSet(0.0f, 0.0f * DirectX::XMScalarSin(DirectX::XMConvertToRadians(Degree)), 0.0f, 1.0f), DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f));
		DirectX::XMStoreFloat4x4(&Tr.View, View);
#endif
		const auto WV = DirectX::XMMatrixMultiply(DirectX::XMLoadFloat4x4(&Tr.World), DirectX::XMLoadFloat4x4(&Tr.View));
		auto DetWV = DirectX::XMMatrixDeterminant(WV);
		DirectX::XMStoreFloat4(&Tr.LocalCameraPosition, DirectX::XMVector4Transform(DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f), DirectX::XMMatrixInverse(&DetWV, WV)));

		Degree += 1.0f;

#pragma region FRAME_OBJECT
		CopyToUploadResource(COM_PTR_GET(ConstantBuffers[GetCurrentBackBufferIndex()].Resource), RoundUp256(sizeof(Tr)), &Tr);
#pragma endregion
	}

	virtual void CreateGeometry() override {
		constexpr D3D12_DRAW_INDEXED_ARGUMENTS DIA = { .IndexCountPerInstance = 1, .InstanceCount = 1, .StartIndexLocation = 0, .BaseVertexLocation = 0, .StartInstanceLocation = 0 };
		IndirectBuffers.emplace_back().Create(COM_PTR_GET(Device), DIA).ExecuteCopyCommand(COM_PTR_GET(Device), COM_PTR_GET(CommandAllocators[0]), COM_PTR_GET(GraphicsCommandLists[0]), COM_PTR_GET(GraphicsCommandQueue), COM_PTR_GET(Fence), sizeof(DIA), &DIA);
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

#pragma region FRAME_OBJECT
		DXGI_SWAP_CHAIN_DESC1 SCD;
		SwapChain->GetDesc1(&SCD);
		for (UINT i = 0; i < SCD.BufferCount; ++i) {
			ConstantBuffers.emplace_back().Create(COM_PTR_GET(Device), sizeof(Tr));
		}
#pragma endregion
	}
	virtual void CreateTexture() override {
		std::wstring Path;
		if (FindDirectory("DDS", Path)) {
			const auto CA = COM_PTR_GET(CommandAllocators[0]);
			const auto GCL = COM_PTR_GET(GraphicsCommandLists[0]);
			//!< [0] キューブ(Cube) : PX, NX, PY, NY, PZ, NZ
			DDSTextures.emplace_back().Create(COM_PTR_GET(Device), Path + TEXT("\\CubeMap\\ninomaru_teien.dds")).ExecuteCopyCommand(COM_PTR_GET(Device), CA, GCL, COM_PTR_GET(GraphicsCommandQueue), COM_PTR_GET(Fence), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
			{
				//!< D3D12_SRV_DIMENSION_TEXTURECUBE にするため SRV を明示的に上書きしている (To use D3D12_SRV_DIMENSION_TEXTURECUBE, overwrite SRV explicitly)
				const auto RD = DDSTextures.back().Resource->GetDesc();
				DDSTextures.back().SRV = D3D12_SHADER_RESOURCE_VIEW_DESC({ .Format = RD.Format, .ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE, .Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING, .TextureCube = D3D12_TEXCUBE_SRV({.MostDetailedMip = 0, .MipLevels = RD.MipLevels, .ResourceMinLODClamp = 0.0f }), });
			}
			//!< [1] 法線(Normal)
			DDSTextures.emplace_back().Create(COM_PTR_GET(Device), Path + TEXT("\\Metal012_2K-JPG\\Metal012_2K_Normal.dds")).ExecuteCopyCommand(COM_PTR_GET(Device), CA, GCL, COM_PTR_GET(GraphicsCommandQueue), COM_PTR_GET(Fence), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
		}
#ifndef USE_SKY_DOME
		//!< [2] 深度(Depth)
		DepthTextures.emplace_back().Create(COM_PTR_GET(Device), static_cast<UINT64>(GetClientRectWidth()), static_cast<UINT>(GetClientRectHeight()), 1, D3D12_CLEAR_VALUE({ .Format = DXGI_FORMAT_D24_UNORM_S8_UINT, .DepthStencil = D3D12_DEPTH_STENCIL_VALUE({.Depth = 1.0f, .Stencil = 0 }) }));
#endif
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
#ifdef USE_SKY_DOME
		VERIFY_SUCCEEDED(D3DReadFileToBlob(data(ShaderPath + TEXT(".vs.cso")), COM_PTR_PUT(SBs.emplace_back())));
		VERIFY_SUCCEEDED(D3DReadFileToBlob(data(ShaderPath + TEXT("_sd.ps.cso")), COM_PTR_PUT(SBs.emplace_back())));
		VERIFY_SUCCEEDED(D3DReadFileToBlob(data(ShaderPath + TEXT("_sd.ds.cso")), COM_PTR_PUT(SBs.emplace_back())));
		VERIFY_SUCCEEDED(D3DReadFileToBlob(data(ShaderPath + TEXT("_sd.hs.cso")), COM_PTR_PUT(SBs.emplace_back())));
		VERIFY_SUCCEEDED(D3DReadFileToBlob(data(ShaderPath + TEXT("_sd.gs.cso")), COM_PTR_PUT(SBs.emplace_back())));
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
#ifndef USE_SKY_DOME
		CreatePipelineState_VsPsDsHsGs(D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH, RD, TRUE, SBCs);
#else
		CreatePipelineState_VsPsDsHsGs(D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH, RD, FALSE, SBCs);
#endif
	}
	virtual void CreateDescriptor() override {
		{
#pragma region FRAME_OBJECT
			DXGI_SWAP_CHAIN_DESC1 SCD;
			SwapChain->GetDesc1(&SCD);
			const D3D12_DESCRIPTOR_HEAP_DESC DHD = { .Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, .NumDescriptors = SCD.BufferCount + 2, .Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE, .NodeMask = 0 }; //!< CBV * N, SRV0, SRV1
#pragma endregion
			VERIFY_SUCCEEDED(Device->CreateDescriptorHeap(&DHD, COM_PTR_UUIDOF_PUTVOID(CbvSrvUavDescriptorHeaps.emplace_back())));
		}
#ifndef USE_SKY_DOME
		{
			const D3D12_DESCRIPTOR_HEAP_DESC DHD = { .Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV, .NumDescriptors = 1, .Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE, .NodeMask = 0 }; //!< DSV
			VERIFY_SUCCEEDED(Device->CreateDescriptorHeap(&DHD, COM_PTR_UUIDOF_PUTVOID(DsvDescriptorHeaps.emplace_back())));
		}
#endif

		{
			CbvSrvUavGPUHandles.emplace_back();
			auto CDH = CbvSrvUavDescriptorHeaps[0]->GetCPUDescriptorHandleForHeapStart();
			auto GDH = CbvSrvUavDescriptorHeaps[0]->GetGPUDescriptorHandleForHeapStart();
			const auto IncSize = Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
			//!< CBV
#pragma region FRAME_OBJECT
			DXGI_SWAP_CHAIN_DESC1 SCD;
			SwapChain->GetDesc1(&SCD);
			for (UINT i = 0; i < SCD.BufferCount; ++i) {
				const D3D12_CONSTANT_BUFFER_VIEW_DESC CBVD = { .BufferLocation = ConstantBuffers[i].Resource->GetGPUVirtualAddress(), .SizeInBytes = static_cast<UINT>(ConstantBuffers[i].Resource->GetDesc().Width) };
				Device->CreateConstantBufferView(&CBVD, CDH);
				CbvSrvUavGPUHandles.back().emplace_back(GDH);
				CDH.ptr += IncSize;
				GDH.ptr += IncSize;
			}
#pragma endregion
			//!< D3D12_SRV_DIMENSION_TEXTURECUBE を指定する必要がある為、(nullptr ではなく) 明示的にSHADER_RESOURCE_VIEW_DESCを使用すること
			//!< SRV0
			Device->CreateShaderResourceView(COM_PTR_GET(DDSTextures[0].Resource), &DDSTextures[0].SRV, CDH);
			CbvSrvUavGPUHandles.back().emplace_back(GDH);
			CDH.ptr += IncSize;
			GDH.ptr += IncSize;
			//!< SRV1
			Device->CreateShaderResourceView(COM_PTR_GET(DDSTextures[1].Resource), &DDSTextures[1].SRV, CDH);
			CbvSrvUavGPUHandles.back().emplace_back(GDH);
			CDH.ptr += IncSize;
			GDH.ptr += IncSize; 
		}
#ifndef USE_SKY_DOME
		{
			DsvCPUHandles.emplace_back();
			auto CDH = DsvDescriptorHeaps[0]->GetCPUDescriptorHandleForHeapStart();
			//!< DSV
			Device->CreateDepthStencilView(COM_PTR_GET(DepthTextures.back().Resource), &DepthTextures.back().DSV, CDH);
			DsvCPUHandles.back().emplace_back(CDH);
		}
#endif
	}

	virtual void PopulateCommandList(const size_t i) override {
		const auto PS = COM_PTR_GET(PipelineStates[0]);

		const auto BGCL = COM_PTR_GET(BundleGraphicsCommandLists[i]);
		const auto BCA = COM_PTR_GET(BundleCommandAllocators[0]);
		VERIFY_SUCCEEDED(BGCL->Reset(BCA, PS));
		{
			BGCL->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_1_CONTROL_POINT_PATCHLIST);
			BGCL->ExecuteIndirect(COM_PTR_GET(IndirectBuffers[0].CommandSignature), 1, COM_PTR_GET(IndirectBuffers[0].Resource), 0, nullptr, 0);
		}
		VERIFY_SUCCEEDED(BGCL->Close());

		const auto GCL = COM_PTR_GET(GraphicsCommandLists[i]);
		const auto CA = COM_PTR_GET(CommandAllocators[0]);
		VERIFY_SUCCEEDED(GCL->Reset(CA, PS));
		{
			GCL->SetGraphicsRootSignature(COM_PTR_GET(RootSignatures[0]));

			GCL->RSSetViewports(static_cast<UINT>(size(Viewports)), data(Viewports));
			GCL->RSSetScissorRects(static_cast<UINT>(size(ScissorRects)), data(ScissorRects));

			const auto SCR = COM_PTR_GET(SwapChainResources[i]);
			ResourceBarrier(GCL, SCR, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
			{
				constexpr std::array<D3D12_RECT, 0> Rects = {};
				GCL->ClearRenderTargetView(SwapChainCPUHandles[i], DirectX::Colors::SkyBlue, static_cast<UINT>(size(Rects)), data(Rects));
#ifndef USE_SKY_DOME
				GCL->ClearDepthStencilView(DsvCPUHandles.back()[0], D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, static_cast<UINT>(size(Rects)), data(Rects));
#endif
				const std::array CHs = { SwapChainCPUHandles[i] };
#ifndef USE_SKY_DOME
				GCL->OMSetRenderTargets(static_cast<UINT>(size(CHs)), data(CHs), FALSE, &DsvCPUHandles.back()[0]);
#else
				GCL->OMSetRenderTargets(static_cast<UINT>(size(CHs)), data(CHs), FALSE, nullptr);
#endif
				{
					const std::array DHs = { COM_PTR_GET(CbvSrvUavDescriptorHeaps[0]) };
					GCL->SetDescriptorHeaps(static_cast<UINT>(size(DHs)), data(DHs));

#pragma region FRAME_OBJECT
					//!< CBV
					GCL->SetGraphicsRootDescriptorTable(0, CbvSrvUavGPUHandles.back()[i]); 
#pragma endregion
					//!< SRV0, SRV1
					DXGI_SWAP_CHAIN_DESC1 SCD;
					SwapChain->GetDesc1(&SCD);
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
	};
	using Transform = struct Transform;
	FLOAT Degree = 0.0f;
	Transform Tr;
};
#pragma endregion