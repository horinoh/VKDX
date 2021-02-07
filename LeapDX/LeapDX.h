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
		if (!empty(ImageResources)) {
			const auto Desc = ImageResources[0]->GetDesc();
			const auto Layers = Desc.DepthOrArraySize;
			const auto LayerSize = size(ImageData[0]);
			const auto PitchSize = Desc.Width * ImageProperties[0].bpp;
			const auto AlignedPitchSize = static_cast<UINT>(RoundUp(PitchSize, D3D12_TEXTURE_DATA_PITCH_ALIGNMENT));
			const auto AlignedLayerSize = Desc.Height * AlignedPitchSize;
			const std::array AlignedTop = { RoundUp(0, D3D12_TEXTURE_DATA_PLACEMENT_ALIGNMENT), RoundUp(AlignedLayerSize, D3D12_TEXTURE_DATA_PLACEMENT_ALIGNMENT) };
			const size_t AlignedTotalSize = AlignedTop.back() + AlignedLayerSize;

			const auto CA = COM_PTR_GET(CommandAllocators[0]);
			const auto CL = COM_PTR_GET(GraphicsCommandLists[0]);

			std::vector<std::byte> AlignedData(AlignedTotalSize, std::byte());
			for (UINT32 i = 0; i < Layers; ++i) {
				for (UINT j = 0; j < Desc.Height; ++j) {
					std::copy(&ImageData[i][j * PitchSize], &ImageData[i][(j + 1) * PitchSize - 1], &AlignedData[AlignedTop[i] + j * AlignedPitchSize]);
				}
			}

			COM_PTR<ID3D12Resource> UploadResource;
			CreateBufferResource(COM_PTR_PUT(UploadResource), size(AlignedData), D3D12_HEAP_TYPE_UPLOAD);
			CopyToUploadResource(COM_PTR_GET(UploadResource), size(AlignedData), data(AlignedData));

			std::vector<D3D12_PLACED_SUBRESOURCE_FOOTPRINT> PSFs;
			for (UINT32 i = 0; i < Layers; ++i) {
				PSFs.emplace_back(D3D12_PLACED_SUBRESOURCE_FOOTPRINT({
					.Offset = AlignedTop[i],
					.Footprint = D3D12_SUBRESOURCE_FOOTPRINT({.Format = Desc.Format, .Width = static_cast<UINT>(Desc.Width), .Height = Desc.Height, .Depth = 1, .RowPitch = AlignedPitchSize })
					}));
			}
			VERIFY_SUCCEEDED(CL->Reset(CA, nullptr)); {
				PopulateCommandList_CopyTextureRegion(CL, COM_PTR_GET(UploadResource), COM_PTR_GET(ImageResources[0]), PSFs, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
			} VERIFY_SUCCEEDED(CL->Close());

			ExecuteAndWait(COM_PTR_GET(CommandQueue), static_cast<ID3D12CommandList*>(CL));
		}
	}
	virtual void UpdateDistortionImage() override {
		if (!empty(ImageResources)) {
			const auto Desc = ImageResources[1]->GetDesc();
			const auto Layers = Desc.DepthOrArraySize;
			constexpr auto LayerSize = sizeof(DistortionMatrices[0]);
			constexpr auto PitchSize = sizeof(DistortionMatrices[0].matrix[0]);
			const auto AlignedPitchSize = static_cast<UINT>(RoundUp(PitchSize, D3D12_TEXTURE_DATA_PITCH_ALIGNMENT));
			const auto AlignedLayerSize = Desc.Height * AlignedPitchSize;
			const std::array AlignedTop = { RoundUp(0, D3D12_TEXTURE_DATA_PLACEMENT_ALIGNMENT), RoundUp(AlignedLayerSize, D3D12_TEXTURE_DATA_PLACEMENT_ALIGNMENT) };
			const size_t AlignedTotalSize = AlignedTop.back() + AlignedLayerSize;

			const auto CA = COM_PTR_GET(CommandAllocators[0]);
			const auto CL = COM_PTR_GET(GraphicsCommandLists[0]);

			std::vector<std::byte> AlignedData(AlignedTotalSize, std::byte());
			for (UINT32 i = 0; i < Layers; ++i) {
				*reinterpret_cast<LEAP_DISTORTION_MATRIX*>(&AlignedData[AlignedTop[i]]) = DistortionMatrices[i];
			}			

			COM_PTR<ID3D12Resource> UploadResource;
			CreateBufferResource(COM_PTR_PUT(UploadResource), size(AlignedData), D3D12_HEAP_TYPE_UPLOAD);
			CopyToUploadResource(COM_PTR_GET(UploadResource), size(AlignedData), data(AlignedData));

			std::vector<D3D12_PLACED_SUBRESOURCE_FOOTPRINT> PSFs;
			for (UINT32 i = 0; i < Layers; ++i) {
				PSFs.emplace_back(D3D12_PLACED_SUBRESOURCE_FOOTPRINT({
					.Offset = AlignedTop[i],
					.Footprint = D3D12_SUBRESOURCE_FOOTPRINT({.Format = Desc.Format, .Width = static_cast<UINT>(Desc.Width), .Height = Desc.Height, .Depth = 1, .RowPitch = static_cast<UINT>(AlignedPitchSize) })
					}));
			}
			VERIFY_SUCCEEDED(CL->Reset(CA, nullptr)); {
				PopulateCommandList_CopyTextureRegion(CL, COM_PTR_GET(UploadResource), COM_PTR_GET(ImageResources[1]), PSFs, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
			} VERIFY_SUCCEEDED(CL->Close());

			ExecuteAndWait(COM_PTR_GET(CommandQueue), static_cast<ID3D12CommandList*>(CL));
		}
	}
#endif

	virtual void CreateGeometry() override { CreateIndirectBuffer_Draw(4, 1); }
#pragma region CB
	virtual void CreateConstantBuffer() override {
		DXGI_SWAP_CHAIN_DESC1 SCD;
		SwapChain->GetDesc1(&SCD);
		for (UINT i = 0; i < SCD.BufferCount; ++i) {
			ConstantBuffers.emplace_back(ConstantBuffer());
			CreateBufferResource(COM_PTR_PUT(ConstantBuffers.back().Resource), RoundUp256(sizeof(Tracking)), D3D12_HEAP_TYPE_UPLOAD);
		}
	}
#pragma endregion
	virtual void CreateTexture() override {
#ifdef USE_LEAP
		//!< Leap イメージ
		{
			const auto Layers = static_cast<uint32_t>(size(ImageData));

			ImageResources.emplace_back(COM_PTR<ID3D12Resource>());
			const auto RD = D3D12_RESOURCE_DESC({
				.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D,
				.Alignment = 0,
				.Width = ImageProperties[0].width, .Height = ImageProperties[0].height, .DepthOrArraySize = static_cast<UINT16>(Layers), .MipLevels = 1,
				.Format = DXGI_FORMAT_R8_UNORM,
				.SampleDesc = DXGI_SAMPLE_DESC({.Count = 1, .Quality = 0 }),
				.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN,
				.Flags = D3D12_RESOURCE_FLAG_NONE
				});
			const D3D12_HEAP_PROPERTIES HP = {
				.Type = D3D12_HEAP_TYPE_DEFAULT,
				.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
				.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN,
				.CreationNodeMask = 0, .VisibleNodeMask = 0
			};
			VERIFY_SUCCEEDED(Device->CreateCommittedResource(&HP, D3D12_HEAP_FLAG_NONE, &RD, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, COM_PTR_UUIDOF_PUTVOID(ImageResources.back())));

			UpdateLeapImage();

			ShaderResourceViewDescs.emplace_back(D3D12_SHADER_RESOURCE_VIEW_DESC({
				.Format = ImageResources.back()->GetDesc().Format,
				.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY,
				.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING,
				.Texture2DArray = D3D12_TEX2D_ARRAY_SRV({.MostDetailedMip = 0, .MipLevels = ImageResources.back()->GetDesc().MipLevels, .FirstArraySlice = 0, .ArraySize = Layers, .PlaneSlice = 0, .ResourceMinLODClamp = 0.0f })
				}));
		}
		//!< ディストーションマップ
		{
			const auto Layers = static_cast<UINT32>(size(DistortionMatrices));
			
			ImageResources.emplace_back(COM_PTR<ID3D12Resource>());
			const auto RD = D3D12_RESOURCE_DESC({
				.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D,
				.Alignment = 0,
				.Width = LEAP_DISTORTION_MATRIX_N, .Height = LEAP_DISTORTION_MATRIX_N, .DepthOrArraySize = static_cast<UINT16>(Layers), .MipLevels = 1,
				.Format = DXGI_FORMAT_R32G32_FLOAT,
				.SampleDesc = DXGI_SAMPLE_DESC({.Count = 1, .Quality = 0 }),
				.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN,
				.Flags = D3D12_RESOURCE_FLAG_NONE
				});
			const D3D12_HEAP_PROPERTIES HP = {
				.Type = D3D12_HEAP_TYPE_DEFAULT,
				.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
				.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN,
				.CreationNodeMask = 0, .VisibleNodeMask = 0
			};
			VERIFY_SUCCEEDED(Device->CreateCommittedResource(&HP, D3D12_HEAP_FLAG_NONE, &RD, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, COM_PTR_UUIDOF_PUTVOID(ImageResources.back())));

			UpdateDistortionImage();

			ShaderResourceViewDescs.emplace_back(D3D12_SHADER_RESOURCE_VIEW_DESC({
				.Format = ImageResources.back()->GetDesc().Format,
				.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY,
				.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING,
				.Texture2DArray = D3D12_TEX2D_ARRAY_SRV({.MostDetailedMip = 0, .MipLevels = ImageResources.back()->GetDesc().MipLevels, .FirstArraySlice = 0, .ArraySize = Layers, .PlaneSlice = 0, .ResourceMinLODClamp = 0.0f })
				}));
		}
#else
		//!< ABRG
		CreateTextureArray1x1({ 0xff0000ff, 0xff00ff00 });
		CreateTextureArray1x1({ 0xffff0000, 0xff00ffff });
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
		const std::array DRs = {
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
		const std::array DRs_Cbv = {
			D3D12_DESCRIPTOR_RANGE({
				.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV,
				.NumDescriptors = 1,
				.BaseShaderRegister = 0, .RegisterSpace = 0,
				.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND
				})
		};
#pragma endregion
		assert(!empty(StaticSamplerDescs) && "");
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
			},
			D3D12_ROOT_SIGNATURE_FLAG_NONE | SHADER_ROOT_ACCESS_PS);
#endif
		RootSignatures.emplace_back(COM_PTR<ID3D12RootSignature>());
		VERIFY_SUCCEEDED(Device->CreateRootSignature(0, Blob->GetBufferPointer(), Blob->GetBufferSize(), COM_PTR_UUIDOF_PUTVOID(RootSignatures.back())));
		LOG_OK();
	}

	virtual void CreateShaderBlobs() override { CreateShaderBlob_VsPs(); }

	virtual void CreatePipelineState() override { CreatePipelineState_VsPs(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE, FALSE); }

	virtual void CreateDescriptorHeap() override {
		{
#pragma region CB
			DXGI_SWAP_CHAIN_DESC1 SCD;
			SwapChain->GetDesc1(&SCD);
#pragma endregion
			CbvSrvUavDescriptorHeaps.emplace_back(COM_PTR<ID3D12DescriptorHeap>());
			const D3D12_DESCRIPTOR_HEAP_DESC DHD = { 
				.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
#pragma region SECOND_TEXTURE, CB
				.NumDescriptors = 2 + SCD.BufferCount,
#pragma endregion
				.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE, .NodeMask = 0 };
			VERIFY_SUCCEEDED(Device->CreateDescriptorHeap(&DHD, COM_PTR_UUIDOF_PUTVOID(CbvSrvUavDescriptorHeaps.back())));
		}
	}
	virtual void CreateDescriptorView() override {
		assert(!empty(ImageResources) && "");
		const auto& DH = CbvSrvUavDescriptorHeaps[0];
		auto CDH = DH->GetCPUDescriptorHandleForHeapStart();
		Device->CreateShaderResourceView(COM_PTR_GET(ImageResources[0]), &ShaderResourceViewDescs[0], CDH); CDH.ptr += Device->GetDescriptorHandleIncrementSize(DH->GetDesc().Type);
#pragma region SECOND_TEXTURE
		Device->CreateShaderResourceView(COM_PTR_GET(ImageResources[1]), &ShaderResourceViewDescs[1], CDH); CDH.ptr += Device->GetDescriptorHandleIncrementSize(DH->GetDesc().Type); 
#pragma endregion
#pragma region CB
		DXGI_SWAP_CHAIN_DESC1 SCD;
		SwapChain->GetDesc1(&SCD);
		for (UINT i = 0; i < SCD.BufferCount; ++i) {
			const D3D12_CONSTANT_BUFFER_VIEW_DESC CBVD = { .BufferLocation = COM_PTR_GET(ConstantBuffers[i].Resource)->GetGPUVirtualAddress(), .SizeInBytes = static_cast<UINT>(ConstantBuffers[i].Resource->GetDesc().Width) };
			Device->CreateConstantBufferView(&CBVD, CDH); CDH.ptr += Device->GetDescriptorHandleIncrementSize(DH->GetDesc().Type);
		}
#pragma endregion
	}

	virtual void PopulateCommandList(const size_t i) override;

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