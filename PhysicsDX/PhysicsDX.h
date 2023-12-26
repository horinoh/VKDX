#pragma once

#include "resource.h"

#pragma region Code
#include "../DXExt.h"
#include "../Phys.h"

class PhysicsDX : public DXExtDepth
{
private:
	using Super = DXExtDepth;
public:
	PhysicsDX() : Super() {}
	virtual ~PhysicsDX() {}

protected:
	static constexpr UINT InstanceCount = 100;

	virtual void OnCreate(HWND hWnd, HINSTANCE hInstance, LPCWSTR Title) override {
		Super::OnCreate(hWnd, hInstance, Title);

		Scn = new Scene();
		Scn->Init();
	}
	virtual void OnTimer(HWND hWnd, HINSTANCE hInstance) override {
		Super::OnTimer(hWnd, hInstance);

		if (IsUpdate()) {
			if (nullptr != Scn) {
				Scn->Update(DeltaSec);
			}
		}
	}
	virtual void OnDestroy(HWND hWnd, HINSTANCE hInstance) override {
		Super::OnDestroy(hWnd, hInstance);

		if (nullptr != Scn) {
			delete Scn;
		}
	}

	virtual void DrawFrame(const UINT i) override {
		if (nullptr != Scn) {
			for (auto i = 0; i < size(Scn->RigidBodies); ++i) {
				if (i < _countof(Tr.World)) {
					const auto Rb = Scn->RigidBodies[i];

					const auto Pos = DirectX::XMLoadFloat4(reinterpret_cast<const DirectX::XMFLOAT4*>(static_cast<const float*>(Rb->Position)));
					const auto Rot = DirectX::XMLoadFloat4(reinterpret_cast<const DirectX::XMFLOAT4*>(static_cast<const float*>(Rb->Rotation)));
					const auto Scl = static_cast<ShapeSphere*>(Rb->Shape)->Radius;

					//!< SRT
					DirectX::XMStoreFloat4x4(&Tr.World[i], DirectX::XMMatrixScaling(Scl, Scl, Scl) * DirectX::XMMatrixRotationQuaternion(Rot) * DirectX::XMMatrixTranslationFromVector(Pos));
				}
			}
		}

		CopyToUploadResource(COM_PTR_GET(ConstantBuffers[i].Resource), RoundUp256(sizeof(Tr)), &Tr);
	}

	virtual void CreateGeometry() override {
		constexpr D3D12_DRAW_INDEXED_ARGUMENTS DIA = { .IndexCountPerInstance = 1, .InstanceCount = InstanceCount, .StartIndexLocation = 0, .BaseVertexLocation = 0, .StartInstanceLocation = 0 };
		IndirectBuffers.emplace_back().Create(COM_PTR_GET(Device), DIA).ExecuteCopyCommand(COM_PTR_GET(Device), COM_PTR_GET(DirectCommandAllocators[0]), COM_PTR_GET(DirectCommandLists[0]), COM_PTR_GET(GraphicsCommandQueue), COM_PTR_GET(GraphicsFence), sizeof(DIA), &DIA);
	}
	virtual void CreateConstantBuffer() override {
		constexpr auto Fov = 0.16f * std::numbers::pi_v<float>;
		const auto Aspect = GetAspectRatioOfClientRect();
		constexpr auto ZFar = 100.0f;
		constexpr auto ZNear = ZFar * 0.0001f;
		const auto CamPos = DirectX::XMVectorSet(0.0f, 15.0f, 30.0f, 1.0f);
		const auto CamTag = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
		const auto CamUp = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
		const auto Projection = DirectX::XMMatrixPerspectiveFovRH(Fov, Aspect, ZNear, ZFar);
		const auto View = DirectX::XMMatrixLookAtRH(CamPos, CamTag, CamUp);
		DirectX::XMStoreFloat4x4(&Tr.Projection, Projection);
		DirectX::XMStoreFloat4x4(&Tr.View, View);
		//!< Œ©‚¦‚È‚¢‚Æ‚±‚ë‚É”ò‚Î‚µ‚Ä‚¨‚­
		for (auto& i : Tr.World) {
			DirectX::XMStoreFloat4x4(&i, DirectX::XMMatrixTranslation(0.0f, -1000.0f, 0.0f));
		}

		DXGI_SWAP_CHAIN_DESC1 SCD;
		SwapChain->GetDesc1(&SCD);
		for (UINT i = 0; i < SCD.BufferCount; ++i) {
			ConstantBuffers.emplace_back().Create(COM_PTR_GET(Device), sizeof(Tr));
		}
	}
	virtual void CreateRootSignature() override {
		COM_PTR<ID3DBlob> Blob;
#ifdef USE_HLSL_ROOTSIGNATRUE
		GetRootSignaturePartFromShader(Blob, data(GetBasePath() + TEXT(".rs.cso")));
#else
		constexpr std::array DRs = {
			D3D12_DESCRIPTOR_RANGE({.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV, .NumDescriptors = 1, .BaseShaderRegister = 0, .RegisterSpace = 0, .OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND })
		};
		DX::SerializeRootSignature(Blob, {
			D3D12_ROOT_PARAMETER({
				.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE,
				.DescriptorTable = D3D12_ROOT_DESCRIPTOR_TABLE({.NumDescriptorRanges = static_cast<UINT>(size(DRs)), .pDescriptorRanges = data(DRs) }),
				.ShaderVisibility = D3D12_SHADER_VISIBILITY_GEOMETRY
			}),
			}, {}, SHADER_ROOT_ACCESS_GS);
#endif
		VERIFY_SUCCEEDED(Device->CreateRootSignature(0, Blob->GetBufferPointer(), Blob->GetBufferSize(), COM_PTR_UUIDOF_PUTVOID(RootSignatures.emplace_back())));
		LOG_OK();
	}
	virtual void CreatePipelineState() override {
		std::vector<COM_PTR<ID3DBlob>> SBs;
		VERIFY_SUCCEEDED(D3DReadFileToBlob(data(GetFilePath(".vs.cso").wstring()), COM_PTR_PUT(SBs.emplace_back())));
		VERIFY_SUCCEEDED(D3DReadFileToBlob(data(GetFilePath(".ps.cso").wstring()), COM_PTR_PUT(SBs.emplace_back())));
		VERIFY_SUCCEEDED(D3DReadFileToBlob(data(GetFilePath(".ds.cso").wstring()), COM_PTR_PUT(SBs.emplace_back())));
		VERIFY_SUCCEEDED(D3DReadFileToBlob(data(GetFilePath(".hs.cso").wstring()), COM_PTR_PUT(SBs.emplace_back())));
		VERIFY_SUCCEEDED(D3DReadFileToBlob(data(GetFilePath(".gs.cso").wstring()), COM_PTR_PUT(SBs.emplace_back())));

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
		auto& Desc = CbvSrvUavDescs.emplace_back();
		auto& Heap = Desc.first;
		auto& Handle = Desc.second;

		{
			DXGI_SWAP_CHAIN_DESC1 SCD;
			SwapChain->GetDesc1(&SCD);
			//!< CBV * N
			const D3D12_DESCRIPTOR_HEAP_DESC DHD = { .Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, .NumDescriptors = SCD.BufferCount, .Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE, .NodeMask = 0 };
			VERIFY_SUCCEEDED(Device->CreateDescriptorHeap(&DHD, COM_PTR_UUIDOF_PUTVOID(Heap)));
		}
		{
			auto CDH = Heap->GetCPUDescriptorHandleForHeapStart();
			auto GDH = Heap->GetGPUDescriptorHandleForHeapStart();
			const auto IncSize = Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
			DXGI_SWAP_CHAIN_DESC1 SCD;
			SwapChain->GetDesc1(&SCD);
			for (UINT i = 0; i < SCD.BufferCount; ++i) {
				const D3D12_CONSTANT_BUFFER_VIEW_DESC CBVD = { .BufferLocation = ConstantBuffers[i].Resource->GetGPUVirtualAddress(), .SizeInBytes = static_cast<UINT>(ConstantBuffers[i].Resource->GetDesc().Width) };
				//!< CBV
				Device->CreateConstantBufferView(&CBVD, CDH);
				Handle.emplace_back(GDH);
				CDH.ptr += IncSize;
				GDH.ptr += IncSize;
			}
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

		const auto DCL = COM_PTR_GET(DirectCommandLists[i]);
		const auto DCA = COM_PTR_GET(DirectCommandAllocators[0]);
		VERIFY_SUCCEEDED(DCL->Reset(DCA, PS));
		{
			DCL->SetGraphicsRootSignature(COM_PTR_GET(RootSignatures[0]));

			DCL->RSSetViewports(static_cast<UINT>(size(Viewports)), data(Viewports));
			DCL->RSSetScissorRects(static_cast<UINT>(size(ScissorRects)), data(ScissorRects));

			const auto SCR = COM_PTR_GET(SwapChainResources[i]);
			ResourceBarrier(DCL, SCR, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
			{
				const auto& HandleDSV = DsvDescs[0].second;

				const auto& DescCBV = CbvSrvUavDescs[0];
				const auto& HeapCBV = DescCBV.first;
				const auto& HandleCBV = DescCBV.second;

				constexpr std::array<D3D12_RECT, 0> Rects = {};
				DCL->ClearRenderTargetView(SwapChainCPUHandles[i], DirectX::Colors::SkyBlue, static_cast<UINT>(size(Rects)), data(Rects));
				DCL->ClearDepthStencilView(HandleDSV[0], D3D12_CLEAR_FLAG_DEPTH/*| D3D12_CLEAR_FLAG_STENCIL*/, 1.0f, 0, static_cast<UINT>(size(Rects)), data(Rects));
				const std::array CHs = { SwapChainCPUHandles[i] };
				DCL->OMSetRenderTargets(static_cast<UINT>(size(CHs)), data(CHs), FALSE, &HandleDSV[0]);
				{
					const std::array DHs = { COM_PTR_GET(HeapCBV) };
					DCL->SetDescriptorHeaps(static_cast<UINT>(size(DHs)), data(DHs));
					DCL->SetGraphicsRootDescriptorTable(0, HandleCBV[i]);
				}
				DCL->ExecuteBundle(BCL);
			}
			ResourceBarrier(DCL, SCR, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
		}
		VERIFY_SUCCEEDED(DCL->Close());
	}

private:
	struct Transform
	{
		DirectX::XMFLOAT4X4 Projection;
		DirectX::XMFLOAT4X4 View;
		DirectX::XMFLOAT4X4 World[InstanceCount];
	};
	using Transform = struct Transform;
	Transform Tr;

	Scene* Scn = nullptr;
};
#pragma endregion