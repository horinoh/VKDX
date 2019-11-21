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
		assert(false && "");
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
		assert(false && "");
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
		assert(false && "");
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
		assert(false && "");
		return DXGI_FORMAT_UNKNOWN;
	}

protected:
	virtual void LoadScene() override;
	virtual void Process(const fx::gltf::Primitive& Prim) override;
	virtual void Process(const std::string& Identifier, const fx::gltf::Accessor& Acc) override;

	virtual void OnTimer(HWND hWnd, HINSTANCE hInstance) override;

	virtual void CreateRootSignature() override {
		COM_PTR<ID3DBlob> Blob;
#ifdef USE_HLSL_ROOTSIGNATRUE
		GetRootSignaturePartFromShader(Blob, (GetBasePath() + TEXT(".rs.cso")).data());
#else
		D3D12_ROOT_PARAMETER RP = { D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS };
		RP.Constants = {};
		RP.ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
		DX::SerializeRootSignature(Blob, {}, {}, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
#endif
		RootSignatures.resize(1);
		DX::CreateRootSignature(RootSignatures[0], Blob);
		LOG_OK();
	}
	virtual void PopulateCommandList(const size_t i) override;

	UINT IndexCount = 0;
};
#pragma endregion