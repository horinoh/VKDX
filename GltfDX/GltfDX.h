#pragma once

#include "resource.h"

#pragma region Code
#include "../DXExt.h"
#include "../Gltf.h"

class GltfDX : public DXExt, public Gltf
{
private:
	using Super = DXExt;
public:
	GltfDX() : Super() {}
	virtual ~GltfDX() {}

	static DXGI_FORMAT ToDXFormat(const fx::gltf::Accessor::ComponentType CT) {
		switch (CT) {
		case fx::gltf::Accessor::ComponentType::UnsignedShort: return DXGI_FORMAT_R16_UINT;
		case fx::gltf::Accessor::ComponentType::UnsignedInt: return DXGI_FORMAT_R32_UINT;
		}
		DEBUG_BREAK();
		return DXGI_FORMAT_UNKNOWN;
	}
	static D3D12_PRIMITIVE_TOPOLOGY_TYPE ToDXPrimitiveTopologyType(const fx::gltf::Primitive::Mode MD) {
		switch (MD)
		{
		case fx::gltf::Primitive::Mode::Points: return D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
		case fx::gltf::Primitive::Mode::Lines: return D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
		case fx::gltf::Primitive::Mode::LineLoop:
		case fx::gltf::Primitive::Mode::LineStrip: return D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
		case fx::gltf::Primitive::Mode::Triangles:
		case fx::gltf::Primitive::Mode::TriangleStrip:
		case fx::gltf::Primitive::Mode::TriangleFan: return D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		}
		DEBUG_BREAK();
		return D3D12_PRIMITIVE_TOPOLOGY_TYPE_UNDEFINED;
	}
	static D3D12_PRIMITIVE_TOPOLOGY ToDXPrimitiveTopology(const fx::gltf::Primitive::Mode MD) {
		switch (MD)
		{
		case fx::gltf::Primitive::Mode::Points: return D3D_PRIMITIVE_TOPOLOGY_POINTLIST;
		case fx::gltf::Primitive::Mode::Lines: return D3D_PRIMITIVE_TOPOLOGY_LINELIST;
		//case fx::gltf::Primitive::Mode::LineLoop:
		case fx::gltf::Primitive::Mode::LineStrip: return D3D_PRIMITIVE_TOPOLOGY_LINESTRIP;
		case fx::gltf::Primitive::Mode::Triangles: return D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		case fx::gltf::Primitive::Mode::TriangleStrip: return D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
		//case fx::gltf::Primitive::Mode::TriangleFan:
		}
		DEBUG_BREAK();
		return D3D_PRIMITIVE_TOPOLOGY_UNDEFINED;
	}
	static DXGI_FORMAT ToDXFormat(const fx::gltf::Accessor& Acc) {
		switch (Acc.type) {
			//case fx::gltf::Accessor::Type::None:
		case fx::gltf::Accessor::Type::Scalar:
			switch (Acc.componentType) {
			//case fx::gltf::Accessor::ComponentType::None:
			case fx::gltf::Accessor::ComponentType::Byte: return DXGI_FORMAT_R8_SINT;
			case fx::gltf::Accessor::ComponentType::UnsignedByte: return DXGI_FORMAT_R8_UINT;
			case fx::gltf::Accessor::ComponentType::Short: return DXGI_FORMAT_R16_SINT;
			case fx::gltf::Accessor::ComponentType::UnsignedShort: return DXGI_FORMAT_R16_UINT;
			case fx::gltf::Accessor::ComponentType::UnsignedInt: return DXGI_FORMAT_R32_UINT;
			case fx::gltf::Accessor::ComponentType::Float: return DXGI_FORMAT_R32_FLOAT;
			}
		case fx::gltf::Accessor::Type::Vec2:
			switch (Acc.componentType) {
			//case fx::gltf::Accessor::ComponentType::None:
			case fx::gltf::Accessor::ComponentType::Byte: return DXGI_FORMAT_R8G8_SINT;
			case fx::gltf::Accessor::ComponentType::UnsignedByte: return DXGI_FORMAT_R8G8_UINT;
			case fx::gltf::Accessor::ComponentType::Short: return DXGI_FORMAT_R16G16_SINT;
			case fx::gltf::Accessor::ComponentType::UnsignedShort: return DXGI_FORMAT_R16G16_UINT;
			case fx::gltf::Accessor::ComponentType::UnsignedInt: return DXGI_FORMAT_R32G32_UINT;
			case fx::gltf::Accessor::ComponentType::Float: return DXGI_FORMAT_R32G32_FLOAT;
			}
		case fx::gltf::Accessor::Type::Vec3:
			switch (Acc.componentType) {
			//case fx::gltf::Accessor::ComponentType::None:
			//case fx::gltf::Accessor::ComponentType::Byte: return DXGI_FORMAT_R8G8B8_SINT;
			//case fx::gltf::Accessor::ComponentType::UnsignedByte: return DXGI_FORMAT_R8G8B8_UINT;
			//case fx::gltf::Accessor::ComponentType::Short: return DXGI_FORMAT_R16G16B16_SINT;
			//case fx::gltf::Accessor::ComponentType::UnsignedShort: return DXGI_FORMAT_R16G16B16_UINT;
			case fx::gltf::Accessor::ComponentType::UnsignedInt: return DXGI_FORMAT_R32G32B32_UINT;
			case fx::gltf::Accessor::ComponentType::Float: return DXGI_FORMAT_R32G32B32_FLOAT;
			}
		case fx::gltf::Accessor::Type::Vec4:
			switch (Acc.componentType) {
			//case fx::gltf::Accessor::ComponentType::None:
			case fx::gltf::Accessor::ComponentType::Byte: return DXGI_FORMAT_R8G8B8A8_SINT;
			case fx::gltf::Accessor::ComponentType::UnsignedByte: return DXGI_FORMAT_R8G8B8A8_UINT;
			case fx::gltf::Accessor::ComponentType::Short: return DXGI_FORMAT_R16G16B16A16_SINT;
			case fx::gltf::Accessor::ComponentType::UnsignedShort: return DXGI_FORMAT_R16G16B16A16_UINT;
			case fx::gltf::Accessor::ComponentType::UnsignedInt: return DXGI_FORMAT_R32G32B32A32_UINT;
			case fx::gltf::Accessor::ComponentType::Float: return DXGI_FORMAT_R32G32B32A32_FLOAT;
			}
			//case fx::gltf::Accessor::Type::Mat2:
			//case fx::gltf::Accessor::Type::Mat3:
			//case fx::gltf::Accessor::Type::Mat4:
		}
		DEBUG_BREAK();
		return DXGI_FORMAT_UNKNOWN;
	}

protected:
	virtual void LoadScene() override;
	virtual void Process(const fx::gltf::Document& Doc) override {
		NodeMatrices.assign(Doc.nodes.size(), DirectX::XMMatrixIdentity());
		Gltf::Process(Doc);
#ifdef DEBUG_STDOUT
		if (NodeMatrices.size()) {
			std::cout << "NodeMatrices[" << NodeMatrices.size() << "]" << std::endl;
			for (auto i : NodeMatrices) {
				std::cout << i;
			}
		}
#endif
	}
	virtual void PreProcess() override;
	virtual void PostProcess() override;

	virtual void PushNode() override { Gltf::PushNode(); CurrentMatrix.push_back(CurrentMatrix.back()); }
	virtual void PopNode() override { Gltf::PopNode(); CurrentMatrix.pop_back(); }
	virtual void Process(const fx::gltf::Node& Nd, const uint32_t i) override;
	virtual void Process(const fx::gltf::Camera& Cam) override;
	virtual void Process(const fx::gltf::Primitive& Prim) override;
	virtual void Process(const std::string& Identifier, const fx::gltf::Accessor& Acc) override;
	virtual void Process(const fx::gltf::Mesh& Msh) override;
	virtual void Process(const fx::gltf::Skin& Skn) override;

	virtual std::array<float, 3> Lerp(const std::array<float, 3>& lhs, const std::array<float, 3>& rhs, const float t) override { return DX::Lerp(lhs, rhs, t); }
	virtual std::array<float, 4> Lerp(const std::array<float, 4>& lhs, const std::array<float, 4>& rhs, const float t) override { return DX::Lerp(lhs, rhs, t); }

	virtual void OnTimer(HWND hWnd, HINSTANCE hInstance) override;

	virtual void UpdateAnimTranslation(const std::array<float, 3>& Value, const uint32_t NodeIndex);
	virtual void UpdateAnimScale(const std::array<float, 3>& Value, const uint32_t NodeIndex);
	virtual void UpdateAnimRotation(const std::array<float, 4>& Value, const uint32_t NodeIndex);
	virtual void UpdateAnimWeights(const float* Data, const uint32_t PrevIndex, const uint32_t NextIndex, const float t);

	virtual void CreateDepthStencil() override { DX::CreateDepthStencil(DXGI_FORMAT_D24_UNORM_S8_UINT, GetClientRectWidth(), GetClientRectHeight()); }
	virtual void CreateRootSignature() override {
		COM_PTR<ID3DBlob> Blob;
#ifdef USE_HLSL_ROOTSIGNATRUE
		GetRootSignaturePartFromShader(Blob, (GetBasePath() + TEXT(".rs.cso")).data());
#else
#if 1
		const std::array<D3D12_DESCRIPTOR_RANGE, 1> DRs = { 
			{ D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND } 
		};
#endif
		DX::SerializeRootSignature(Blob, {
#if 1
				{ D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE, { static_cast<UINT>(DRs.size()), DRs.data() }, D3D12_SHADER_VISIBILITY_VERTEX },
#endif
			}, {}, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
#endif
		RootSignatures.resize(1);
		DX::CreateRootSignature(RootSignatures[0], Blob);
		LOG_OK();
	}
	virtual void CreateCommandList() override {
		DXGI_SWAP_CHAIN_DESC1 SCD;
		SwapChain->GetDesc1(&SCD);
		for (UINT i = 0; i < SCD.BufferCount; ++i) {
			GraphicsCommandLists.push_back(COM_PTR<ID3D12GraphicsCommandList>());
			VERIFY_SUCCEEDED(Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, COM_PTR_GET(CommandAllocators[0]), nullptr, COM_PTR_UUIDOF_PUTVOID(GraphicsCommandLists.back())));
			VERIFY_SUCCEEDED(GraphicsCommandLists[i]->Close());
		}
		LOG_OK();
	}
	virtual void PopulateCommandList(const size_t i) override;

	std::vector<DirectX::XMMATRIX> CurrentMatrix = { DirectX::XMMatrixIdentity() };
	std::vector<DirectX::XMMATRIX> NodeMatrices;
	std::vector<DirectX::XMMATRIX> AnimNodeMatrices;

	FLOAT CurrentFrame = 0.0f;

	struct ProjView
	{
		DirectX::XMMATRIX Projection;
		DirectX::XMMATRIX View;
	};
	using ProjView = struct ProjView;
	ProjView PV;
	std::vector<const DirectX::XMMATRIX*> InverseBindMatrices;
	std::vector<DirectX::XMMATRIX> JointMatrices;
	std::vector<float> MorphWeights;
};
#pragma endregion