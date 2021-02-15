#pragma once

#include "resource.h"

#pragma region Code
#include "../DXExt.h"

class TriangleDX : public DXExt
{
private:
	using Super = DXExt;
public:
	TriangleDX() : Super() {}
	virtual ~TriangleDX() {}

protected:
	virtual void CreateGeometry() override;
	virtual void CreateRootSignature() override {
		COM_PTR<ID3DBlob> Blob;
#ifdef USE_HLSL_ROOTSIGNATRUE
		GetRootSignaturePartFromShader(Blob, data(GetBasePath() + TEXT(".rs.cso")));
#else
		DX::SerializeRootSignature(Blob, {
#ifdef USE_ROOT_CONSTANTS
			D3D12_ROOT_PARAMETER({
				.ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS, 
				.Constants = D3D12_ROOT_CONSTANTS({.ShaderRegister = 0, .RegisterSpace = 0, .Num32BitValues = static_cast<UINT>(size(Color)) }),
				.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL
			}),
#endif
			}, {}, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT
#ifdef USE_ROOT_CONSTANTS
			| SHADER_ROOT_ACCESS_PS
#else
			| SHADER_ROOT_ACCESS_DENY_ALL
#endif
		); 
#endif
		VERIFY_SUCCEEDED(Device->CreateRootSignature(0, Blob->GetBufferPointer(), Blob->GetBufferSize(), COM_PTR_UUIDOF_PUTVOID(RootSignatures.emplace_back())));
		LOG_OK();
	}
	virtual void CreateShaderBlob() override { 
		const auto ShaderPath = GetBasePath();
		VERIFY_SUCCEEDED(D3DReadFileToBlob(data(ShaderPath + TEXT(".vs.cso")), COM_PTR_PUT(ShaderBlobs.emplace_back())));
#ifdef USE_ROOT_CONSTANTS
		VERIFY_SUCCEEDED(D3DReadFileToBlob(data(ShaderPath + TEXT("_rc.ps.cso")), COM_PTR_PUT(ShaderBlobs.emplace_back())));
#else
		VERIFY_SUCCEEDED(D3DReadFileToBlob(data(ShaderPath + TEXT(".ps.cso")), COM_PTR_PUT(ShaderBlobs.emplace_back())));
#endif
	}
	virtual void CreatePipelineState() override {
		//!< スロット0にまとめて入れるインターリーブ、セマンティックス毎にスロットを分けると非インターリーブとなる
		//!< 詰まっている場合は offsetof() の代わりに D3D12_APPEND_ALIGNED_ELEMENT で良い (When directly after the previous one, we can use D3D12_APPEND_ALIGNED_ELEMENT)
		const std::vector IEDs = {
			D3D12_INPUT_ELEMENT_DESC({ .SemanticName = "POSITION", .SemanticIndex = 0, .Format = DXGI_FORMAT_R32G32B32_FLOAT, .InputSlot = 0, .AlignedByteOffset = /*offsetof(Vertex_PositionColor, Position)*/D3D12_APPEND_ALIGNED_ELEMENT, .InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, .InstanceDataStepRate = 0 }),
			D3D12_INPUT_ELEMENT_DESC({ .SemanticName = "COLOR", .SemanticIndex = 0, .Format = DXGI_FORMAT_R32G32B32A32_FLOAT, .InputSlot = 0, .AlignedByteOffset = /*offsetof(Vertex_PositionColor, Color)*/D3D12_APPEND_ALIGNED_ELEMENT, .InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, .InstanceDataStepRate = 0 }),
		};
		DXExt::CreatePipelineState_VsPs_Input(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE, FALSE, IEDs);
	}

	virtual void PopulateCommandList(const size_t i) override;

#ifdef USE_ROOT_CONSTANTS
	const std::array<FLOAT, 4> Color = { 1.0f, 0.0f, 0.0f, 1.0f };
#endif
};
#pragma endregion