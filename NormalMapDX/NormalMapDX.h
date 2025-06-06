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
	virtual void OnUpdate(const UINT i) override {
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
		SwapChain.DxSwapChain->GetDesc1(&SCD);
		for (UINT i = 0; i < SCD.BufferCount; ++i) {
			ConstantBuffers.emplace_back().Create(COM_PTR_GET(Device), sizeof(Tr));
		}
#pragma endregion
	}
	virtual void CreateTexture() override {
		const auto CA = COM_PTR_GET(DirectCommandAllocators[0]);
		const auto CL = COM_PTR_GET(DirectCommandLists[0]);
#ifdef USE_PARALLAX_MAP
		//!< [0] 法線(Normal)
		XTKTextures.emplace_back().Create(COM_PTR_GET(Device), DDS_PATH / "Leather009_2K-JPG" / "Leather009_2K_Normal.dds");
		//!< [1] ディスプレースメント(Displacement)
		XTKTextures.emplace_back().Create(COM_PTR_GET(Device), DDS_PATH / "Leather009_2K-JPG" / "Leather009_2K_Displacement.dds");
#else
		//!< [0] 法線(Normal)
		XTKTextures.emplace_back().Create(COM_PTR_GET(Device), DDS_PATH / "Rocks007_2K-JPG" / "Rocks007_2K_Normal.dds");
		//!< [1] カラー(Color)
		XTKTextures.emplace_back().Create(COM_PTR_GET(Device), DDS_PATH / "Rocks007_2K-JPG" / "Rocks007_2K_Color.dds");
#endif

		{
			std::vector<D3D12_PLACED_SUBRESOURCE_FOOTPRINT> PSFs0;
			COM_PTR<ID3D12Resource> Upload0;
			XTKTextures[0].CopyToUploadResource(COM_PTR_GET(Device), COM_PTR_PUT(Upload0), PSFs0);

			std::vector<D3D12_PLACED_SUBRESOURCE_FOOTPRINT> PSFs1;
			COM_PTR<ID3D12Resource> Upload1;
			XTKTextures[1].CopyToUploadResource(COM_PTR_GET(Device), COM_PTR_PUT(Upload1), PSFs1);

			VERIFY_SUCCEEDED(CL->Reset(CA, nullptr)); {
				XTKTextures[0].PopulateCopyCommand(CL, PSFs0, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, COM_PTR_GET(Upload0));
				XTKTextures[1].PopulateCopyCommand(CL, PSFs1, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, COM_PTR_GET(Upload1));
			} VERIFY_SUCCEEDED(CL->Close());
			DX::ExecuteAndWait(COM_PTR_GET(GraphicsCommandQueue), CL, COM_PTR_GET(GraphicsFence));
			XTKTextures[0].Release();
			XTKTextures[1].Release();
		}

		Super::CreateTexture();
	}
	virtual void CreateStaticSampler() override {
		CreateStaticSampler_LW(0, 0, D3D12_SHADER_VISIBILITY_PIXEL);
	}
	virtual void CreateRootSignature() override {
		COM_PTR<ID3DBlob> Blob;
#ifdef USE_HLSL_ROOTSIGNATRUE
		GetRootSignaturePartFromShader(Blob, GetFilePath(".rs.cso"));
#else
		constexpr std::array DRs_Cbv = {
			//!< register(b0, space0)
			D3D12_DESCRIPTOR_RANGE1({.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV, .NumDescriptors = 1, .BaseShaderRegister = 0, .RegisterSpace = 0, .Flags = D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE,.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND })
		};
		constexpr std::array DRs_Srv = {
			//!< register(t0, space0), register(t1, space0)
			D3D12_DESCRIPTOR_RANGE1({.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV, .NumDescriptors = 2, .BaseShaderRegister = 0, .RegisterSpace = 0,.Flags = D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE, .OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND })
		};
		assert(!empty(StaticSamplerDescs) && "");
		DX::SerializeRootSignature(Blob, {
			//!< CBV
			D3D12_ROOT_PARAMETER1({ 
				.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE, 
				.DescriptorTable = D3D12_ROOT_DESCRIPTOR_TABLE1({ .NumDescriptorRanges = static_cast<UINT>(size(DRs_Cbv)), .pDescriptorRanges = data(DRs_Cbv) }), 
				.ShaderVisibility = D3D12_SHADER_VISIBILITY_GEOMETRY 
			}), 
			//!< SRV0, SRV1
			D3D12_ROOT_PARAMETER1({ 
				.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE, 
				.DescriptorTable = D3D12_ROOT_DESCRIPTOR_TABLE1({ .NumDescriptorRanges = static_cast<UINT>(size(DRs_Srv)), .pDescriptorRanges = data(DRs_Srv) }),
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
		PipelineStates.emplace_back();

		std::vector<COM_PTR<ID3DBlob>> SBs;
#ifdef USE_PARALLAX_MAP
		VERIFY_SUCCEEDED(D3DReadFileToBlob(data(GetFilePath(".vs.cso").wstring()), COM_PTR_PUT(SBs.emplace_back())));
		VERIFY_SUCCEEDED(D3DReadFileToBlob(data(GetFilePath("_pm.ps.cso").wstring()), COM_PTR_PUT(SBs.emplace_back())));
		VERIFY_SUCCEEDED(D3DReadFileToBlob(data(GetFilePath(".ds.cso").wstring()), COM_PTR_PUT(SBs.emplace_back())));
		VERIFY_SUCCEEDED(D3DReadFileToBlob(data(GetFilePath(".hs.cso").wstring()), COM_PTR_PUT(SBs.emplace_back())));
		VERIFY_SUCCEEDED(D3DReadFileToBlob(data(GetFilePath(".gs.cso").wstring()), COM_PTR_PUT(SBs.emplace_back())));
#else
		VERIFY_SUCCEEDED(D3DReadFileToBlob(data(GetFilePath(".vs.cso").wstring()), COM_PTR_PUT(SBs.emplace_back())));
		VERIFY_SUCCEEDED(D3DReadFileToBlob(data(GetFilePath(".ps.cso").wstring()), COM_PTR_PUT(SBs.emplace_back())));
		VERIFY_SUCCEEDED(D3DReadFileToBlob(data(GetFilePath(".ds.cso").wstring()), COM_PTR_PUT(SBs.emplace_back())));
		VERIFY_SUCCEEDED(D3DReadFileToBlob(data(GetFilePath(".hs.cso").wstring()), COM_PTR_PUT(SBs.emplace_back())));
		VERIFY_SUCCEEDED(D3DReadFileToBlob(data(GetFilePath(".gs.cso").wstring()), COM_PTR_PUT(SBs.emplace_back())));
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
		CreatePipelineState_VsPsDsHsGs(PipelineStates[0], COM_PTR_GET(RootSignatures[0]), D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH, RD, TRUE, SBCs);

		for (auto& i : Threads) { i.join(); }
		Threads.clear();
	}
	virtual void CreateDescriptor() override {
		auto& Desc = CbvSrvUavDescs.emplace_back();
		auto& Heap = Desc.first;
		auto& Handle = Desc.second;

		{
#pragma region FRAME_OBJECT
			DXGI_SWAP_CHAIN_DESC1 SCD;
			SwapChain.DxSwapChain->GetDesc1(&SCD);
			const D3D12_DESCRIPTOR_HEAP_DESC DHD = { D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, SCD.BufferCount + 2, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE, 0 }; //!< CBV * N, SRV0, SRV1
#pragma endregion
			VERIFY_SUCCEEDED(Device->CreateDescriptorHeap(&DHD, COM_PTR_UUIDOF_PUTVOID(Heap)));
		}

		{
			auto CDH = Heap->GetCPUDescriptorHandleForHeapStart();
			auto GDH = Heap->GetGPUDescriptorHandleForHeapStart();
			const auto IncSize = Device->GetDescriptorHandleIncrementSize(Heap->GetDesc().Type);
#pragma region FRAME_OBJECT
			DXGI_SWAP_CHAIN_DESC1 SCD;
			SwapChain.DxSwapChain->GetDesc1(&SCD);
			for (UINT i = 0; i < SCD.BufferCount; ++i) {
				const D3D12_CONSTANT_BUFFER_VIEW_DESC CBVD = { ConstantBuffers[i].Resource->GetGPUVirtualAddress(), static_cast<UINT>(ConstantBuffers[i].Resource->GetDesc().Width) };
				//!< CBV
				Device->CreateConstantBufferView(&CBVD, CDH); 
				Handle.emplace_back(GDH);
				CDH.ptr += IncSize;
				GDH.ptr += IncSize; 
			}
#pragma endregion
			//!< SRV0
			Device->CreateShaderResourceView(COM_PTR_GET(XTKTextures[0].Resource), &XTKTextures[0].SRV, CDH); 
			Handle.emplace_back(GDH);
			CDH.ptr += IncSize;
			GDH.ptr += IncSize;
			//!< SRV1
			Device->CreateShaderResourceView(COM_PTR_GET(XTKTextures[1].Resource), &XTKTextures[1].SRV, CDH);
			Handle.emplace_back(GDH);
		}

		Super::CreateDescriptor();
	}
	virtual void PopulateCommandList(const size_t i) override {
		const auto PS = COM_PTR_GET(PipelineStates[0]);

		const auto BCL = COM_PTR_GET(BundleCommandLists[i]);
		const auto BCA = COM_PTR_GET(BundleCommandAllocators[0]);
		VERIFY_SUCCEEDED(BCL->Reset(BCA, PS));
		{
			BCL->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_1_CONTROL_POINT_PATCHLIST);
			BCL->ExecuteIndirect(COM_PTR_GET(IndirectBuffers[0].CommandSignature), 1, COM_PTR_GET(IndirectBuffers[0].Resource), 0, nullptr, 0);
		}
		VERIFY_SUCCEEDED(BCL->Close());

		const auto CL = COM_PTR_GET(DirectCommandLists[i]);
		const auto CA = COM_PTR_GET(DirectCommandAllocators[0]);
		VERIFY_SUCCEEDED(CL->Reset(CA, PS));
		{
			const auto RS = COM_PTR_GET(RootSignatures[0]);

			CL->SetGraphicsRootSignature(RS);

			CL->RSSetViewports(static_cast<UINT>(std::size(Viewports)), std::data(Viewports));
			CL->RSSetScissorRects(static_cast<UINT>(std::size(ScissorRects)), std::data(ScissorRects));

			const auto& RAH = SwapChain.ResourceAndHandles[i];

			ResourceBarrier(CL, COM_PTR_GET(RAH.first), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
			{
				const auto& HandleDSV = DsvDescs[0].second;

				constexpr std::array<D3D12_RECT, 0> Rects = {};
				CL->ClearRenderTargetView(RAH.second, DirectX::Colors::SkyBlue, static_cast<UINT>(std::size(Rects)), std::data(Rects));
				CL->ClearDepthStencilView(HandleDSV[0], D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, static_cast<UINT>(std::size(Rects)), std::data(Rects));

				const std::array CHs = { RAH.second };
				CL->OMSetRenderTargets(static_cast<UINT>(std::size(CHs)), std::data(CHs), FALSE, &HandleDSV[0]);

				{
					const auto& Desc = CbvSrvUavDescs[0];
					const auto& Heap = Desc.first;
					const auto& Handle = Desc.second;

					const std::array DHs = { COM_PTR_GET(Heap) };
					CL->SetDescriptorHeaps(static_cast<UINT>(std::size(DHs)), std::data(DHs));

					DXGI_SWAP_CHAIN_DESC1 SCD;
					SwapChain.DxSwapChain->GetDesc1(&SCD);
#pragma region FRAME_OBJECT
					//!< CBV
					CL->SetGraphicsRootDescriptorTable(0, Handle[i]); 
#pragma endregion
					//!< SRV0, SRV1
					CL->SetGraphicsRootDescriptorTable(1, Handle[SCD.BufferCount]);
				}

				CL->ExecuteBundle(BCL);
			}
			ResourceBarrier(CL, COM_PTR_GET(RAH.first), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
		}
		VERIFY_SUCCEEDED(CL->Close());
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