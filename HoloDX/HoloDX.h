#pragma once

#include "resource.h"

#pragma region Code
#include "../DXExt.h"
#include "../Holo.h"

class HoloDX : public DXExt, public Holo
{
private:
	using Super = DXExt;
public:
	HoloDX() : Super(), Holo() {
		const auto& QS = GetQuiltSetting();
		QuiltWidth = static_cast<UINT64>(QS.GetWidth());
		QuiltHeight = static_cast<UINT>(QS.GetHeight());
	}
	virtual ~HoloDX() {}

protected:
	virtual void OnTimer(HWND hWnd, HINSTANCE hInstance) override { 
		Super::OnTimer(hWnd, hInstance); 
#pragma region FRAME_OBJECT
		//CopyToUploadResource(COM_PTR_GET(ConstantBuffers[GetCurrentBackBufferIndex()].Resource), RoundUp256(sizeof(Tr)), &Tr);
#pragma endregion
	}

	virtual void CreateCommandList() override {
		Super::CreateCommandList();
		//!< Pass1 : �o���h���R�}���h���X�g
		DXGI_SWAP_CHAIN_DESC1 SCD;
		SwapChain->GetDesc1(&SCD);
		for (UINT i = 0; i < SCD.BufferCount; ++i) {
			BundleGraphicsCommandLists.push_back(COM_PTR<ID3D12GraphicsCommandList>());
			VERIFY_SUCCEEDED(Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_BUNDLE, COM_PTR_GET(BundleCommandAllocators[0]), nullptr, COM_PTR_UUIDOF_PUTVOID(BundleGraphicsCommandLists.back())));
			VERIFY_SUCCEEDED(BundleGraphicsCommandLists.back()->Close());
		}
	}

	virtual void CreateIndirectBuffer() override {
		//!< Pass0 : ���b�V���`��p
		CreateIndirectBuffer_DrawIndexed(1, 1);
		//!< Pass1 : �t���X�N���[���`��p
		CreateIndirectBuffer_Draw(4, 1);
	}
	virtual void CreateConstantBuffer() override {
		constexpr auto Fov = DirectX::XMConvertToRadians(14.0f);
		const auto Aspect = GetRatio(GetDeviceIndex());
		constexpr auto ZFar = 100.0f;
		constexpr auto ZNear = 0.1f;

		const auto CamPos = DirectX::XMVectorSet(0.0f, 0.0f, 7.0f, 1.0f);
		const auto CamTag = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
		const auto CamUp = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

		const auto Projection = DirectX::XMMatrixPerspectiveFovRH(Fov, Aspect, ZNear, ZFar);
		const auto View = DirectX::XMMatrixLookAtRH(CamPos, CamTag, CamUp);
		const auto World = DirectX::XMMatrixIdentity();

		DirectX::XMStoreFloat4x4(&Tr.World, World);
		DirectX::XMStoreFloat4x4(&Tr.View, View);
		DirectX::XMStoreFloat4x4(&Tr.Projection, Projection);

		Tr.Aspect = Aspect;
		Tr.ViewCone = DirectX::XMConvertToRadians(GetViewCone(GetDeviceIndex()));
		Tr.ViewTotal = GetQuiltSetting().GetViewTotal();

		//const auto QS = GetQuiltSetting();
		//constexpr auto CameraSize = 5.0f;
		//const auto CameraDistance = -CameraSize / tan(Fov * 0.5f);
		//const auto ViewCone = DirectX::XMConvertToRadians(GetViewCone(GetDeviceIndex()));
		//for (auto i = 0; i < QS.GetViewTotal(); ++i) {
		//	const auto OffsetRadian = (static_cast<float>(i) / (QS.GetViewTotal() - 1) - 0.5f) * ViewCone;
		//	const auto OffsetX = CameraDistance * tan(OffsetRadian);

		//	DirectX::XMStoreFloat4x4(&Tr.View, View * DirectX::XMMatrixTranslationFromVector(DirectX::XMVector4Transform(DirectX::XMVectorSet(OffsetX, 0.0f, CameraDistance, 1.0f), View)));

		//	DirectX::XMStoreFloat4x4(&Tr.Projection, Projection);
		//	Tr.Projection.m[2][0] += OffsetX / (CameraSize * Aspect);
		//}

#pragma region FRAME_OBJECT
		DXGI_SWAP_CHAIN_DESC1 SCD;
		SwapChain->GetDesc1(&SCD);
		for (UINT i = 0; i < SCD.BufferCount; ++i) {
			ConstantBuffers.emplace_back(ConstantBuffer());
			CreateResource(COM_PTR_PUT(ConstantBuffers.back().Resource), RoundUp256(sizeof(Tr)), D3D12_HEAP_TYPE_UPLOAD);
		}
#pragma endregion
	}
	virtual void CreateStaticSampler() override {
		//!< Pass1 
		StaticSamplerDescs.emplace_back(D3D12_STATIC_SAMPLER_DESC({
			.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR,
			.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP, .AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP, .AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP,
			.MipLODBias = 0.0f,
			.MaxAnisotropy = 0,
			.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER,
			.BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE,
			.MinLOD = 0.0f, .MaxLOD = 1.0f,
			.ShaderRegister = 0, .RegisterSpace = 0, .ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL
			}));
	}
	virtual void CreateRootSignature() override {
		//!< Pass0 
		{
			RootSignatures.emplace_back(COM_PTR<ID3D12RootSignature>());
			COM_PTR<ID3DBlob> Blob;
#ifdef USE_HLSL_ROOTSIGNATRUE
			GetRootSignaturePartFromShader(Blob, data(GetBasePath() + TEXT(".rs.cso")));
#else
			const std::array DRs = {
				D3D12_DESCRIPTOR_RANGE({.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV, .NumDescriptors = 1, .BaseShaderRegister = 0, .RegisterSpace = 0, .OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND })
			};
			SerializeRootSignature(Blob, {
					D3D12_ROOT_PARAMETER({.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE, .DescriptorTable = D3D12_ROOT_DESCRIPTOR_TABLE({.NumDescriptorRanges = static_cast<UINT>(size(DRs)), .pDescriptorRanges = data(DRs) }), .ShaderVisibility = D3D12_SHADER_VISIBILITY_GEOMETRY }),
				}, {}, D3D12_ROOT_SIGNATURE_FLAG_NONE
				| D3D12_ROOT_SIGNATURE_FLAG_DENY_VERTEX_SHADER_ROOT_ACCESS
				| D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS
				| D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS
				| D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS);
#endif
			VERIFY_SUCCEEDED(Device->CreateRootSignature(0, Blob->GetBufferPointer(), Blob->GetBufferSize(), COM_PTR_UUIDOF_PUTVOID(RootSignatures.back())));
		}
		//!< Pass1 
		{
			RootSignatures.emplace_back(COM_PTR<ID3D12RootSignature>());
			COM_PTR<ID3DBlob> Blob;
#ifdef USE_HLSL_ROOTSIGNATRUE 
			GetRootSignaturePartFromShader(Blob, data(GetBasePath() + TEXT("_1.rs.cso")));
#else
			const std::array DRs_Srv = {
				D3D12_DESCRIPTOR_RANGE({.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV, .NumDescriptors = 1, .BaseShaderRegister = 0, .RegisterSpace = 0, .OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND })
			};
			assert(!empty(StaticSamplerDescs) && "");
			DX::SerializeRootSignature(Blob, {
					D3D12_ROOT_PARAMETER({.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE, .DescriptorTable = D3D12_ROOT_DESCRIPTOR_TABLE({.NumDescriptorRanges = static_cast<uint32_t>(size(DRs_Srv)), .pDescriptorRanges = data(DRs_Srv) }), .ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL })
				}, {
					StaticSamplerDescs[0],
				}, D3D12_ROOT_SIGNATURE_FLAG_NONE
				| D3D12_ROOT_SIGNATURE_FLAG_DENY_VERTEX_SHADER_ROOT_ACCESS
				| D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS
				| D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS
				| D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS
			);
#endif
			VERIFY_SUCCEEDED(Device->CreateRootSignature(0, Blob->GetBufferPointer(), Blob->GetBufferSize(), COM_PTR_UUIDOF_PUTVOID(RootSignatures.back())));
		}
		LOG_OK();
	}
	virtual void CreateTexture()
	{
		const D3D12_HEAP_PROPERTIES HP = {
			.Type = D3D12_HEAP_TYPE_DEFAULT,
			.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
			.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN,
			.CreationNodeMask = 0,
			.VisibleNodeMask = 0
		};
		const DXGI_SAMPLE_DESC SD = { .Count = 1, .Quality = 0 };

		{
			ImageResources.emplace_back(COM_PTR<ID3D12Resource>());
			const D3D12_RESOURCE_DESC RD = {
				.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D,
				.Alignment = 0,
				.Width = QuiltWidth, .Height = QuiltHeight,
				.DepthOrArraySize = 1,
				.MipLevels = 1,
				.Format = DXGI_FORMAT_R8G8B8A8_UNORM,
				.SampleDesc = SD,
				.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN,
				.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET
			};
			const D3D12_CLEAR_VALUE CV = {
				.Format = RD.Format,
				.Color = { DirectX::Colors::SkyBlue.f[0], DirectX::Colors::SkyBlue.f[1], DirectX::Colors::SkyBlue.f[2], DirectX::Colors::SkyBlue.f[3] },
			};
			VERIFY_SUCCEEDED(Device->CreateCommittedResource(&HP, D3D12_HEAP_FLAG_NONE, &RD, D3D12_RESOURCE_STATE_RENDER_TARGET, &CV, COM_PTR_UUIDOF_PUTVOID(ImageResources.back())));

			RenderTargetViewDescs.emplace_back(D3D12_RENDER_TARGET_VIEW_DESC({
				.Format = ImageResources.back()->GetDesc().Format,
				.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D,
				.Texture2D = D3D12_TEX2D_RTV({.MipSlice = 0, .PlaneSlice = 0 })
				}));

			ShaderResourceViewDescs.emplace_back(D3D12_SHADER_RESOURCE_VIEW_DESC({
				.Format = ImageResources.back()->GetDesc().Format,
				.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D,
				.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING,
				.Texture2D = D3D12_TEX2D_SRV({.MostDetailedMip = 0, .MipLevels = ImageResources.back()->GetDesc().MipLevels, .PlaneSlice = 0, .ResourceMinLODClamp = 0.0f })
				}));
		}
		{
			ImageResources.emplace_back(COM_PTR<ID3D12Resource>());
			const D3D12_RESOURCE_DESC RD = {
				.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D,
				.Alignment = 0,
				.Width = QuiltWidth, .Height = QuiltHeight,
				.DepthOrArraySize = 1,
				.MipLevels = 1,
				.Format = DXGI_FORMAT_D24_UNORM_S8_UINT,
				.SampleDesc = SD,
				.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN,
				.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL
			};
			const D3D12_CLEAR_VALUE CV = { .Format = RD.Format, .DepthStencil = D3D12_DEPTH_STENCIL_VALUE({.Depth = 1.0f, .Stencil = 0 }) };
			VERIFY_SUCCEEDED(Device->CreateCommittedResource(&HP, D3D12_HEAP_FLAG_NONE, &RD, D3D12_RESOURCE_STATE_DEPTH_WRITE, &CV, COM_PTR_UUIDOF_PUTVOID(ImageResources.back())));

			DepthStencilViewDescs.emplace_back(D3D12_DEPTH_STENCIL_VIEW_DESC({
				.Format = ImageResources.back()->GetDesc().Format,
				.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D,
				.Flags = D3D12_DSV_FLAG_NONE,
				.Texture2D = D3D12_TEX2D_DSV({.MipSlice = 0 })
				}));
		}
	}

	virtual void CreateDescriptorHeap() override {
		//!< Pass0 
		{
			{
				DXGI_SWAP_CHAIN_DESC1 SCD;
				SwapChain->GetDesc1(&SCD);
#pragma region FRAME_OBJECT
				const D3D12_DESCRIPTOR_HEAP_DESC DHD = { .Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, .NumDescriptors = SCD.BufferCount, .Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE, .NodeMask = 0 }; //!< CBV * N
#pragma endregion
				CbvSrvUavDescriptorHeaps.emplace_back(COM_PTR<ID3D12DescriptorHeap>());
				VERIFY_SUCCEEDED(Device->CreateDescriptorHeap(&DHD, COM_PTR_UUIDOF_PUTVOID(CbvSrvUavDescriptorHeaps.back())));
			}
			{
				RtvDescriptorHeaps.emplace_back(COM_PTR<ID3D12DescriptorHeap>());
				const D3D12_DESCRIPTOR_HEAP_DESC DHD = { .Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV, .NumDescriptors = 1, .Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE, .NodeMask = 0 };
				VERIFY_SUCCEEDED(Device->CreateDescriptorHeap(&DHD, COM_PTR_UUIDOF_PUTVOID(RtvDescriptorHeaps.back())));
			}
			{
				DsvDescriptorHeaps.emplace_back(COM_PTR<ID3D12DescriptorHeap>());
				const D3D12_DESCRIPTOR_HEAP_DESC DHD = { .Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV, .NumDescriptors = 1, .Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE, .NodeMask = 0 };
				VERIFY_SUCCEEDED(Device->CreateDescriptorHeap(&DHD, COM_PTR_UUIDOF_PUTVOID(DsvDescriptorHeaps.back())));
			}
		}
		//!< Pass1
		{
			CbvSrvUavDescriptorHeaps.emplace_back(COM_PTR<ID3D12DescriptorHeap>());
			const D3D12_DESCRIPTOR_HEAP_DESC DHD = { .Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, .NumDescriptors = 1, .Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE, .NodeMask = 0 };
			VERIFY_SUCCEEDED(Device->CreateDescriptorHeap(&DHD, COM_PTR_UUIDOF_PUTVOID(CbvSrvUavDescriptorHeaps.back())));
		}
	}
	virtual void CreateDescriptorView() override {
		assert(2 == size(ImageResources) && "");
		//!< Pass0 
		{
			{
				DXGI_SWAP_CHAIN_DESC1 SCD;
				SwapChain->GetDesc1(&SCD);
				const auto& DH = CbvSrvUavDescriptorHeaps[0];
				auto CDH = DH->GetCPUDescriptorHandleForHeapStart();
#pragma region FRAME_OBJECT
				for (UINT i = 0; i < SCD.BufferCount; ++i) {
					const D3D12_CONSTANT_BUFFER_VIEW_DESC CBVD = { .BufferLocation = COM_PTR_GET(ConstantBuffers[i].Resource)->GetGPUVirtualAddress(), .SizeInBytes = static_cast<UINT>(ConstantBuffers[i].Resource->GetDesc().Width) };
					Device->CreateConstantBufferView(&CBVD, CDH); CDH.ptr += Device->GetDescriptorHandleIncrementSize(DH->GetDesc().Type);
				}
#pragma endregion
			}
			{
				const auto& DH = RtvDescriptorHeaps[0];
				auto CDH = DH->GetCPUDescriptorHandleForHeapStart();
				Device->CreateRenderTargetView(COM_PTR_GET(ImageResources[0]), &RenderTargetViewDescs[0], CDH); CDH.ptr += Device->GetDescriptorHandleIncrementSize(DH->GetDesc().Type);
			}
			{
				const auto& DH = DsvDescriptorHeaps[0];
				auto CDH = DH->GetCPUDescriptorHandleForHeapStart();
				Device->CreateDepthStencilView(COM_PTR_GET(ImageResources[1]), &DepthStencilViewDescs[0], CDH); CDH.ptr += Device->GetDescriptorHandleIncrementSize(DH->GetDesc().Type);
			}
		}
		//!< Pass1
		{
			const auto& DH = CbvSrvUavDescriptorHeaps[1];
			auto CDH = DH->GetCPUDescriptorHandleForHeapStart();
			{
				Device->CreateShaderResourceView(COM_PTR_GET(ImageResources[0]), &ShaderResourceViewDescs[0], CDH); CDH.ptr += Device->GetDescriptorHandleIncrementSize(DH->GetDesc().Type);
			}
		}

#pragma region FRAME_OBJECT
		DXGI_SWAP_CHAIN_DESC1 SCD;
		SwapChain->GetDesc1(&SCD);
		for (UINT i = 0; i < SCD.BufferCount; ++i) {
			CopyToUploadResource(COM_PTR_GET(ConstantBuffers[i].Resource), RoundUp256(sizeof(Tr)), &Tr);
		}
#pragma endregion
	}

	virtual void CreateShaderBlobs() override {
		const auto ShaderPath = GetBasePath();
		//!< Pass0 
		ShaderBlobs.emplace_back(COM_PTR<ID3DBlob>());
		VERIFY_SUCCEEDED(D3DReadFileToBlob(data(ShaderPath + TEXT(".vs.cso")), COM_PTR_PUT(ShaderBlobs.back())));
		ShaderBlobs.emplace_back(COM_PTR<ID3DBlob>());
		VERIFY_SUCCEEDED(D3DReadFileToBlob(data(ShaderPath + TEXT(".ps.cso")), COM_PTR_PUT(ShaderBlobs.back())));
		ShaderBlobs.emplace_back(COM_PTR<ID3DBlob>());
		VERIFY_SUCCEEDED(D3DReadFileToBlob(data(ShaderPath + TEXT(".ds.cso")), COM_PTR_PUT(ShaderBlobs.back())));
		ShaderBlobs.emplace_back(COM_PTR<ID3DBlob>());
		VERIFY_SUCCEEDED(D3DReadFileToBlob(data(ShaderPath + TEXT(".hs.cso")), COM_PTR_PUT(ShaderBlobs.back())));
		ShaderBlobs.emplace_back(COM_PTR<ID3DBlob>());
		VERIFY_SUCCEEDED(D3DReadFileToBlob(data(ShaderPath + TEXT(".gs.cso")), COM_PTR_PUT(ShaderBlobs.back())));
		//!< Pass1 
		ShaderBlobs.emplace_back(COM_PTR<ID3DBlob>());
		VERIFY_SUCCEEDED(D3DReadFileToBlob(data(ShaderPath + TEXT("_1.vs.cso")), COM_PTR_PUT(ShaderBlobs.back())));
		ShaderBlobs.emplace_back(COM_PTR<ID3DBlob>());
		VERIFY_SUCCEEDED(D3DReadFileToBlob(data(ShaderPath + TEXT("_1.ps.cso")), COM_PTR_PUT(ShaderBlobs.back())));
	}
	virtual void CreatePipelineStates() override {
		PipelineStates.resize(2);
		std::vector<std::thread> Threads;
		const std::vector RTBDs = {
			D3D12_RENDER_TARGET_BLEND_DESC({
				.BlendEnable = FALSE, .LogicOpEnable = FALSE,
				.SrcBlend = D3D12_BLEND_ONE, .DestBlend = D3D12_BLEND_ZERO, .BlendOp = D3D12_BLEND_OP_ADD,
				.SrcBlendAlpha = D3D12_BLEND_ONE, .DestBlendAlpha = D3D12_BLEND_ZERO, .BlendOpAlpha = D3D12_BLEND_OP_ADD,
				.LogicOp = D3D12_LOGIC_OP_NOOP,
				.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL,
			}),
		};
		const D3D12_RASTERIZER_DESC RD = {
			.FillMode = D3D12_FILL_MODE_SOLID,
			.CullMode = D3D12_CULL_MODE_BACK, .FrontCounterClockwise = TRUE,
			.DepthBias = D3D12_DEFAULT_DEPTH_BIAS, .DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP, .SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS,
			.DepthClipEnable = TRUE,
			.MultisampleEnable = FALSE, .AntialiasedLineEnable = FALSE, .ForcedSampleCount = 0,
			.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF
		};
		const D3D12_DEPTH_STENCILOP_DESC DSOD = { .StencilFailOp = D3D12_STENCIL_OP_KEEP, .StencilDepthFailOp = D3D12_STENCIL_OP_KEEP, .StencilPassOp = D3D12_STENCIL_OP_KEEP, .StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS };
		const D3D12_DEPTH_STENCIL_DESC DSD_0 = {
			.DepthEnable = TRUE, .DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL, .DepthFunc = D3D12_COMPARISON_FUNC_LESS,
			.StencilEnable = FALSE, .StencilReadMask = D3D12_DEFAULT_STENCIL_READ_MASK, .StencilWriteMask = D3D12_DEFAULT_STENCIL_WRITE_MASK,
			.FrontFace = DSOD, .BackFace = DSOD
		};
		const D3D12_DEPTH_STENCIL_DESC DSD_1 = {
			.DepthEnable = FALSE, .DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL, .DepthFunc = D3D12_COMPARISON_FUNC_LESS,
			.StencilEnable = FALSE, .StencilReadMask = D3D12_DEFAULT_STENCIL_READ_MASK, .StencilWriteMask = D3D12_DEFAULT_STENCIL_WRITE_MASK,
			.FrontFace = DSOD, .BackFace = DSOD
		};
		const std::array SBCs_0 = {
			D3D12_SHADER_BYTECODE({.pShaderBytecode = ShaderBlobs[0]->GetBufferPointer(), .BytecodeLength = ShaderBlobs[0]->GetBufferSize() }),
			D3D12_SHADER_BYTECODE({.pShaderBytecode = ShaderBlobs[1]->GetBufferPointer(), .BytecodeLength = ShaderBlobs[1]->GetBufferSize() }),
			D3D12_SHADER_BYTECODE({.pShaderBytecode = ShaderBlobs[2]->GetBufferPointer(), .BytecodeLength = ShaderBlobs[2]->GetBufferSize() }),
			D3D12_SHADER_BYTECODE({.pShaderBytecode = ShaderBlobs[3]->GetBufferPointer(), .BytecodeLength = ShaderBlobs[3]->GetBufferSize() }),
			D3D12_SHADER_BYTECODE({.pShaderBytecode = ShaderBlobs[4]->GetBufferPointer(), .BytecodeLength = ShaderBlobs[4]->GetBufferSize() }),
		};
		const std::array SBCs_1 = {
			D3D12_SHADER_BYTECODE({.pShaderBytecode = ShaderBlobs[5]->GetBufferPointer(), .BytecodeLength = ShaderBlobs[5]->GetBufferSize() }),
			D3D12_SHADER_BYTECODE({.pShaderBytecode = ShaderBlobs[6]->GetBufferPointer(), .BytecodeLength = ShaderBlobs[6]->GetBufferSize() }),
		};
		const std::vector<D3D12_INPUT_ELEMENT_DESC> IEDs = {};
		const std::vector RTVs = { DXGI_FORMAT_R8G8B8A8_UNORM };
#ifdef USE_PIPELINE_SERIALIZE
		PipelineLibrarySerializer PLS(COM_PTR_GET(Device), GetBasePath() + TEXT(".plo"));
		//!< Pass0 
		Threads.push_back(std::thread::thread(DX::CreatePipelineState, std::ref(PipelineStates[0]), COM_PTR_GET(Device), COM_PTR_GET(RootSignatures[0]), D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH, RTBDs, RD, DSD_0, SBCs_0[0], SBCs_0[1], SBCs_0[2], SBCs_0[3], SBCs_0[4], IEDs, RTVs, &PLS, TEXT("0")));
		//!< Pass1 
		Threads.push_back(std::thread::thread(DX::CreatePipelineState, std::ref(PipelineStates[1]), COM_PTR_GET(Device), COM_PTR_GET(RootSignatures[1]), D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE, RTBDs, RD, DSD_1, SBCs_1[0], SBCs_1[1], NullShaderBC, NullShaderBC, NullShaderBC, IEDs, RTVs, &PLS, TEXT("1")));
#else
		Threads.push_back(std::thread::thread(DX::CreatePipelineState, std::ref(PipelineStates[0]), COM_PTR_GET(Device), COM_PTR_GET(RootSignatures[0]), D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH, RTBDs, RD, DSD_0, SBCs_0[0], SBCs_0[1], SBCs_0[2], SBCs_0[3], SBCs_0[4], IEDs, RTVs, nullptr, nullptr));
		Threads.push_back(std::thread::thread(DX::CreatePipelineState, std::ref(PipelineStates[1]), COM_PTR_GET(Device), COM_PTR_GET(RootSignatures[1]), D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE, RTBDs, RD, DSD_1, SBCs_1[0], SBCs_1[1], , NullShaderBC, NullShaderBC, NullShaderBC, IEDs, RTVs, nullptr, nullptr));
#endif	
		for (auto& i : Threads) { i.join(); }
	}

	virtual void CreateViewport(const FLOAT Width, const FLOAT Height, const FLOAT MinDepth = 0.0f, const FLOAT MaxDepth = 1.0f) override;

	virtual void PopulateCommandList(const size_t i) override;

	virtual int GetViewportMax() const override { return D3D12_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE; }

protected:
	struct Transform
	{
		DirectX::XMFLOAT4X4 Projection;
		DirectX::XMFLOAT4X4 View;
		DirectX::XMFLOAT4X4 World;
		float Aspect;
		float ViewCone;
		int ViewTotal;
	};
	using Transform = struct Transform;
	Transform Tr; 
	
	UINT64 QuiltWidth;
	UINT QuiltHeight;
	std::vector<D3D12_VIEWPORT> QuiltViewports;
	std::vector<D3D12_RECT> QuiltScissorRects;
};
#pragma endregion