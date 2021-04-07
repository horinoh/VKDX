#pragma once

#include "resource.h"

#pragma region Code
#include "../DXExt.h"

class MeshShaderDX : public DXExt
{
private:
	using Super = DXExt;
public:
	MeshShaderDX() : Super() {}
	virtual ~MeshShaderDX() {}

	//virtual void CreateGeometry() override {
	//	constexpr std::array Vertices = {
	//		Vertex_PositionColor({.Position = { 0.0f, 0.5f, 0.0f }, .Color = { 1.0f, 0.0f, 0.0f, 1.0f } }),
	//		Vertex_PositionColor({.Position = { -0.5f, -0.5f, 0.0f }, .Color = { 0.0f, 1.0f, 0.0f, 1.0f } }),
	//		Vertex_PositionColor({.Position = { 0.5f, -0.5f, 0.0f }, .Color = { 0.0f, 0.0f, 1.0f, 1.0f } }),
	//	};
	//	VB.Create(COM_PTR_GET(Device), sizeof(Vertices), data(Vertices), sizeof(Vertices[0]));

	//	constexpr std::array<UINT32, 3> Indices = { 0, 1, 2 };
	//	IB.Create(COM_PTR_GET(Device), sizeof(Indices), data(Indices), sizeof(Indices[0]));
	//}
//	virtual void CreateRootSignature() override {
//		COM_PTR<ID3DBlob> Blob;
//#ifdef USE_HLSL_ROOTSIGNATRUE 
//		GetRootSignaturePartFromShader(Blob, data(GetBasePath() + TEXT(".rs.cso")));
//#else
//		constexpr std::array DRs = {
//			//!< バーテックスリソース、インデックスリソース
//			D3D12_DESCRIPTOR_RANGE({.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV, .NumDescriptors = 2, .BaseShaderRegister = 0, .RegisterSpace = 0, .OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND })
//		};
//		DX::SerializeRootSignature(Blob, {
//			D3D12_ROOT_PARAMETER({
//				.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE,
//				.DescriptorTable = D3D12_ROOT_DESCRIPTOR_TABLE({.NumDescriptorRanges = static_cast<uint32_t>(size(DRs)), .pDescriptorRanges = data(DRs) }),
//				.ShaderVisibility = D3D12_SHADER_VISIBILITY_MESH
//			}),
//		}, {}, SHADER_ROOT_ACCESS_PS);
//#endif
//		VERIFY_SUCCEEDED(Device->CreateRootSignature(0, Blob->GetBufferPointer(), Blob->GetBufferSize(), COM_PTR_UUIDOF_PUTVOID(RootSignatures.emplace_back())));
//	}
	virtual void CreatePipelineState() override;
	//virtual void CreateDescriptorHeap() override {		
	//	const D3D12_DESCRIPTOR_HEAP_DESC DHD = { .Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, .NumDescriptors = 2, .Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE, .NodeMask = 0 };
	//	VERIFY_SUCCEEDED(Device->CreateDescriptorHeap(&DHD, COM_PTR_UUIDOF_PUTVOID(CbvSrvUavDescriptorHeaps.emplace_back()))); 
	//}
	//virtual void CreateDescriptorView() override {
	//	const auto& DH = CbvSrvUavDescriptorHeaps[0];
	//	auto CDH = DH->GetCPUDescriptorHandleForHeapStart();
	//	Device->CreateShaderResourceView(COM_PTR_GET(VB.Resource), &VB.View, CDH); CDH.ptr += Device->GetDescriptorHandleIncrementSize(DH->GetDesc().Type);
	//	Device->CreateShaderResourceView(COM_PTR_GET(IB.Resource), &IB.View, CDH); CDH.ptr += Device->GetDescriptorHandleIncrementSize(DH->GetDesc().Type);
	//}

	virtual void PopulateCommandList(const size_t i) override;

	//ShaderResourceBuffer VB;
	//ShaderResourceBuffer IB;
};
#pragma endregion