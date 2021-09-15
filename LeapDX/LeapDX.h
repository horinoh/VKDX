#pragma once

#include "resource.h"

#pragma region Code
#include "../DXExt.h"
#include "../Leap.h"

class LeapDX : public DXExt, public Leap
{
private:
	using Super = DXExt;
public:
	LeapDX() : Super(), Leap() {}
	virtual ~LeapDX() {}

protected:
	virtual void OnTimer(HWND hWnd, HINSTANCE hInstance) override {
		Super::OnTimer(hWnd, hInstance);
#ifdef USE_LEAP
		InterpolatedTrackingEvent();
#else
		for (auto i = 0; i < 5; ++i) {
			for (auto j = 0; j < 4; ++j) {
				DirectX::XMStoreFloat4(&Tracking.Hands[0][i][j], DirectX::XMVectorSet(i * 0.2f + 0.1f, 0.5f, (j + 1) * 0.2f - 0.5f, 1.0f));
			}
		}
		for (auto i = 0; i < 5; ++i) {
			for (auto j = 0; j < 4; ++j) {
				DirectX::XMStoreFloat4(&Tracking.Hands[1][i][j], DirectX::XMVectorSet(- i * 0.2f - 0.1f, 0.5f, (j + 1) * 0.2f - 0.5f, 1.0f));
			}
		}
#endif
#pragma region CB
		CopyToUploadResource(COM_PTR_GET(ConstantBuffers[GetCurrentBackBufferIndex()].Resource), RoundUp256(sizeof(Tracking)), &Tracking);
#pragma endregion
	}

	virtual void CreateGeometry() override {
		constexpr D3D12_DRAW_ARGUMENTS DA = { .VertexCountPerInstance = 4, .InstanceCount = 1, .StartVertexLocation = 0, .StartInstanceLocation = 0 };
		IndirectBuffers.emplace_back().Create(COM_PTR_GET(Device), DA).ExecuteCopyCommand(COM_PTR_GET(Device), COM_PTR_GET(CommandAllocators[0]), COM_PTR_GET(GraphicsCommandLists[0]), COM_PTR_GET(GraphicsCommandQueue), COM_PTR_GET(Fence), sizeof(DA), &DA);
	}
#pragma region CB
	virtual void CreateConstantBuffer() override {
		DXGI_SWAP_CHAIN_DESC1 SCD;
		SwapChain->GetDesc1(&SCD);
		for (UINT i = 0; i < SCD.BufferCount; ++i) {
			ConstantBuffers.emplace_back().Create(COM_PTR_GET(Device), sizeof(Tracking));
		}
	}
#pragma endregion
	virtual void CreateTexture() override {
#ifdef USE_LEAP
		//!< Leap イメージ
		{
			Textures.emplace_back().Create(COM_PTR_GET(Device), 1, 1, static_cast<UINT16>(size(ImageData)), DXGI_FORMAT_R8_UNORM);
			UpdateLeapImage();
		}
		//!< ディストーションマップ
		{
			Textures.emplace_back().Create(COM_PTR_GET(Device), 1, 1, static_cast<UINT16>(size(DistortionMatrices)), DXGI_FORMAT_R32G32_FLOAT);
			UpdateDistortionImage();
		}
#else
		CreateTextureArray1x1({ 0xff0000ff, 0xff00ff00 }, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
		CreateTextureArray1x1({ 0xffff0000, 0xff00ffff }, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
#endif
	}
	virtual void CreateStaticSampler() override {
		//!< https://developer.leapmotion.com/documentation/v4/images.html
		//!< フィルタを LINEAR、ラップモードを CLAMP
		StaticSamplerDescs.emplace_back(D3D12_STATIC_SAMPLER_DESC({
			.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR,
			.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP, .AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP, .AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
			.MipLODBias = 0.0f,
			.MaxAnisotropy = 0,
			.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER,
			.BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE,
			.MinLOD = 0.0f, .MaxLOD = 1.0f,
			.ShaderRegister = 0, .RegisterSpace = 0, .ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL
		}));
	}
	virtual void CreateRootSignature() override {
		COM_PTR<ID3DBlob> Blob;
#ifdef USE_HLSL_ROOTSIGNATRUE 
		GetRootSignaturePartFromShader(Blob, data(GetBasePath() + TEXT(".rs.cso")));
#else
		constexpr std::array DRs = {
			D3D12_DESCRIPTOR_RANGE({
				.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
#pragma region SECOND_TEXTURE
				.NumDescriptors = 2,
#pragma endregion
				.BaseShaderRegister = 0, .RegisterSpace = 0,
				.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND
			})
		};
#pragma region CB
		constexpr std::array DRs_Cbv = {
			D3D12_DESCRIPTOR_RANGE({.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV, .NumDescriptors = 1, .BaseShaderRegister = 0, .RegisterSpace = 0, .OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND })
		};
#pragma endregion
		DX::SerializeRootSignature(Blob, {
			D3D12_ROOT_PARAMETER({
				.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE,
				.DescriptorTable = D3D12_ROOT_DESCRIPTOR_TABLE({.NumDescriptorRanges = static_cast<uint32_t>(size(DRs)), .pDescriptorRanges = data(DRs) }),
				.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL }),
#pragma region CB
			D3D12_ROOT_PARAMETER({
				.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE,
				.DescriptorTable = D3D12_ROOT_DESCRIPTOR_TABLE({.NumDescriptorRanges = static_cast<UINT>(size(DRs_Cbv)), .pDescriptorRanges = data(DRs_Cbv) }),
				.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL }),
#pragma endregion
			}, {
				StaticSamplerDescs[0],
			}, SHADER_ROOT_ACCESS_PS);
#endif
		VERIFY_SUCCEEDED(Device->CreateRootSignature(0, Blob->GetBufferPointer(), Blob->GetBufferSize(), COM_PTR_UUIDOF_PUTVOID(RootSignatures.emplace_back())));
		LOG_OK();
	}
	virtual void CreatePipelineState() override {
		const auto ShaderPath = GetBasePath();
		std::vector<COM_PTR<ID3DBlob>> SBs = {};
		VERIFY_SUCCEEDED(D3DReadFileToBlob(data(ShaderPath + TEXT(".vs.cso")), COM_PTR_PUT(SBs.emplace_back())));
		VERIFY_SUCCEEDED(D3DReadFileToBlob(data(ShaderPath + TEXT(".ps.cso")), COM_PTR_PUT(SBs.emplace_back())));
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
		CreatePipelineState_VsPs(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE, RD, FALSE, SBCs); 
	}

	virtual void CreateDescriptor() override {
		{
#pragma region CB
			DXGI_SWAP_CHAIN_DESC1 SCD;
			SwapChain->GetDesc1(&SCD);
#pragma endregion
			const D3D12_DESCRIPTOR_HEAP_DESC DHD = {
				.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
#pragma region SECOND_TEXTURE, CB
				.NumDescriptors = 2 + SCD.BufferCount,
#pragma endregion
				.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE, .NodeMask = 0 
			};
			VERIFY_SUCCEEDED(Device->CreateDescriptorHeap(&DHD, COM_PTR_UUIDOF_PUTVOID(CbvSrvUavDescriptorHeaps.emplace_back())));
		}

		const auto& DH = CbvSrvUavDescriptorHeaps[0];
		auto CDH = DH->GetCPUDescriptorHandleForHeapStart();
		Device->CreateShaderResourceView(COM_PTR_GET(Textures[0].Resource), &Textures[0].SRV, CDH); CDH.ptr += Device->GetDescriptorHandleIncrementSize(DH->GetDesc().Type);
#pragma region SECOND_TEXTURE
		Device->CreateShaderResourceView(COM_PTR_GET(Textures[0].Resource), &Textures[1].SRV, CDH); CDH.ptr += Device->GetDescriptorHandleIncrementSize(DH->GetDesc().Type);
#pragma endregion
#pragma region CB
		DXGI_SWAP_CHAIN_DESC1 SCD;
		SwapChain->GetDesc1(&SCD);
		for (UINT i = 0; i < SCD.BufferCount; ++i) {
			const D3D12_CONSTANT_BUFFER_VIEW_DESC CBVD = {.BufferLocation = ConstantBuffers[i].Resource->GetGPUVirtualAddress(), .SizeInBytes = static_cast<UINT>(ConstantBuffers[i].Resource->GetDesc().Width) };
			Device->CreateConstantBufferView(&CBVD, CDH); CDH.ptr += Device->GetDescriptorHandleIncrementSize(DH->GetDesc().Type);
		}
#pragma endregion
	}

	virtual void PopulateCommandList(const size_t i) override {
		const auto PS = COM_PTR_GET(PipelineStates[0]);

		const auto BGCL = COM_PTR_GET(BundleGraphicsCommandLists[i]);
		const auto BCA = COM_PTR_GET(BundleCommandAllocators[0]);
		VERIFY_SUCCEEDED(BGCL->Reset(BCA, PS));
		{
			BGCL->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
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
				auto SCCDH = SwapChainDescriptorHeap->GetCPUDescriptorHandleForHeapStart(); SCCDH.ptr += i * Device->GetDescriptorHandleIncrementSize(SwapChainDescriptorHeap->GetDesc().Type);
				const std::array RTCDHs = { SCCDH };
				GCL->OMSetRenderTargets(static_cast<UINT>(size(RTCDHs)), data(RTCDHs), FALSE, nullptr);

				assert(!empty(CbvSrvUavDescriptorHeaps) && "");
				const std::array DHs = { COM_PTR_GET(CbvSrvUavDescriptorHeaps[0]) };
				GCL->SetDescriptorHeaps(static_cast<UINT>(size(DHs)), data(DHs));

				{
					const auto& DH = CbvSrvUavDescriptorHeaps[0];
					auto GDH = DH->GetGPUDescriptorHandleForHeapStart();
					GCL->SetGraphicsRootDescriptorTable(0, GDH); GDH.ptr += Device->GetDescriptorHandleIncrementSize(DH->GetDesc().Type) * 2;

#pragma region CB
					DXGI_SWAP_CHAIN_DESC1 SCD;
					SwapChain->GetDesc1(&SCD);
					GDH.ptr += Device->GetDescriptorHandleIncrementSize(DH->GetDesc().Type) * i;
					GCL->SetGraphicsRootDescriptorTable(1, GDH);
#pragma endregion
				}

				GCL->ExecuteBundle(BGCL);
			}
			ResourceBarrier(GCL, SCR, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
		}
		VERIFY_SUCCEEDED(GCL->Close());
	}

#ifdef USE_LEAP
	virtual void OnHand(const LEAP_HAND& Hand) override {
		Leap::OnHand(Hand);

		const auto Index = eLeapHandType_Right == Hand.type ? 0 : 1;
		for (auto i = 0; i < _countof(Hand.digits); ++i) {
			const auto& Digit = Hand.digits[i];
			for (auto j = 0; j < _countof(Digit.bones); ++j) {
				const auto& Bone = Digit.bones[j];
				const auto x = std::clamp(Bone.next_joint.x, -100.0f, 100.0f) / 100.0f;
				const auto y = std::clamp(Bone.next_joint.y, 0.0f, 300.0f) / 300.0f;
				const auto z = std::clamp(Bone.next_joint.z, -100.0f, 100.0f) / 100.0f;
				DirectX::XMStoreFloat4(&Tracking.Hands[Index][i][j], DirectX::XMVectorSet(x, y, z, 1.0f));
			}
		}
	}
	virtual void UpdateLeapImage() override {
		if (!empty(Textures)) {
			const auto CA = COM_PTR_GET(CommandAllocators[0]);
			const auto GCL = COM_PTR_GET(GraphicsCommandLists[0]);
			{
				const auto RD = Textures[0].Resource->GetDesc();
				const auto Layers = RD.DepthOrArraySize;
				const auto LayerSize = size(ImageData[0]);
				const auto PitchSize = RD.Width * ImageProperties[0].bpp;
				const auto AlignedPitchSize = static_cast<UINT>(RoundUp(PitchSize, D3D12_TEXTURE_DATA_PITCH_ALIGNMENT));
				const auto AlignedLayerSize = RD.Height * AlignedPitchSize;
				const std::array AlignedTop = { RoundUp(0, D3D12_TEXTURE_DATA_PLACEMENT_ALIGNMENT), RoundUp(AlignedLayerSize, D3D12_TEXTURE_DATA_PLACEMENT_ALIGNMENT) };

				ResourceBase Upload;
				{
					//!< アラインされたサイズを計算 (Calculate aligned size)
					const size_t AlignedSize = AlignedTop.back() + AlignedLayerSize;
					//!< アラインされたメモリへコピー (Copy to aligned memory)
					std::vector<std::byte> AlignedData(AlignedSize, std::byte());
					for (UINT32 i = 0; i < Layers; ++i) {
						for (UINT j = 0; j < RD.Height; ++j) {
							std::copy(&ImageData[i][j * PitchSize], &ImageData[i][(j + 1) * PitchSize - 1], &AlignedData[AlignedTop[i] + j * AlignedPitchSize]);
						}
					}
					Upload.Create(COM_PTR_GET(Device), size(AlignedData), D3D12_HEAP_TYPE_UPLOAD, data(AlignedData));
				}

				std::vector<D3D12_PLACED_SUBRESOURCE_FOOTPRINT> PSFs;
				for (UINT32 i = 0; i < Layers; ++i) {
					PSFs.emplace_back(D3D12_PLACED_SUBRESOURCE_FOOTPRINT({
						.Offset = AlignedTop[i],
						.Footprint = D3D12_SUBRESOURCE_FOOTPRINT({.Format = RD.Format, .Width = static_cast<UINT>(RD.Width), .Height = RD.Height, .Depth = 1, .RowPitch = AlignedPitchSize })
					}));
				}
				VERIFY_SUCCEEDED(GCL->Reset(CA, nullptr)); {
					PopulateCopyTextureRegionCommand(GCL, COM_PTR_GET(Upload.Resource), COM_PTR_GET(Textures[0].Resource), PSFs, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
				} VERIFY_SUCCEEDED(GCL->Close());
				DX::ExecuteAndWait(COM_PTR_GET(GraphicsCommandQueue), GCL, COM_PTR_GET(Fence));
			}
		}
	}
	virtual void UpdateDistortionImage() override {
		if (size(Textures) > 1) {
			const auto CA = COM_PTR_GET(CommandAllocators[0]);
			const auto GCL = COM_PTR_GET(GraphicsCommandLists[0]);
			{
				const auto RD = Textures[1].Resource->GetDesc();
				const auto Layers = RD.DepthOrArraySize;
				constexpr auto LayerSize = sizeof(DistortionMatrices[0]);
				constexpr auto PitchSize = sizeof(DistortionMatrices[0].matrix[0]);
				const auto AlignedPitchSize = static_cast<UINT>(RoundUp(PitchSize, D3D12_TEXTURE_DATA_PITCH_ALIGNMENT));
				const auto AlignedLayerSize = RD.Height * AlignedPitchSize;
				const std::array AlignedTop = { RoundUp(0, D3D12_TEXTURE_DATA_PLACEMENT_ALIGNMENT), RoundUp(AlignedLayerSize, D3D12_TEXTURE_DATA_PLACEMENT_ALIGNMENT) };

				ResourceBase Upload;
				{
					//!< アラインされたサイズを計算 (Calculate aligned size)
					const size_t AlignedSize = AlignedTop.back() + AlignedLayerSize;
					//!< アラインされたメモリへコピー (Copy to aligned memory)
					std::vector<std::byte> AlignedData(AlignedSize, std::byte());
					for (UINT32 i = 0; i < Layers; ++i) {
						*reinterpret_cast<LEAP_DISTORTION_MATRIX*>(&AlignedData[AlignedTop[i]]) = DistortionMatrices[i];
					}
					Upload.Create(COM_PTR_GET(Device), size(AlignedData), D3D12_HEAP_TYPE_UPLOAD, data(AlignedData));
				}

				std::vector<D3D12_PLACED_SUBRESOURCE_FOOTPRINT> PSFs;
				for (UINT32 i = 0; i < Layers; ++i) {
					PSFs.emplace_back(D3D12_PLACED_SUBRESOURCE_FOOTPRINT({
						.Offset = AlignedTop[i],
						.Footprint = D3D12_SUBRESOURCE_FOOTPRINT({.Format = RD.Format, .Width = static_cast<UINT>(RD.Width), .Height = RD.Height, .Depth = 1, .RowPitch = AlignedPitchSize })
					}));
				}
				VERIFY_SUCCEEDED(GCL->Reset(CA, nullptr)); {
					PopulateCopyTextureRegionCommand(GCL, COM_PTR_GET(Upload.Resource), COM_PTR_GET(Textures[1].Resource), PSFs, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
				} VERIFY_SUCCEEDED(GCL->Close());
				DX::ExecuteAndWait(COM_PTR_GET(GraphicsCommandQueue), GCL, COM_PTR_GET(Fence));
			}
		}
	}
#endif

private:
		struct HandTracking
		{
			//!< 16バイトアライン境界をまたいではいけないので XMFLOAT3 ではなく、XMFLOAT4 を使用する
#ifdef USE_LEAP
			std::array<std::array<std::array<DirectX::XMFLOAT4, _countof(LEAP_DIGIT::bones)>, _countof(LEAP_HAND::digits)>, 2> Hands;
#else
			std::array<std::array<std::array<DirectX::XMFLOAT4, 4>, 5>, 2> Hands;
#endif
		};
		using HandTracking = struct HandTracking;
		HandTracking Tracking;
};
#pragma endregion