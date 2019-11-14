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
		return DXGI_FORMAT_UNKNOWN;
	}
	static D3D_PRIMITIVE_TOPOLOGY ToDXTopology(const fx::gltf::Primitive::Mode MD) {
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
		return D3D_PRIMITIVE_TOPOLOGY_UNDEFINED;
	}
protected:
	virtual void LoadScene() override;
	virtual void Process(const fx::gltf::Primitive& Prim) override;
	virtual void Process(const fx::gltf::Accessor& Acc) override;

	virtual void CreateRootSignature() override {
		COM_PTR<ID3DBlob> Blob;
#ifdef USE_HLSL_ROOTSIGNATRUE
		GetRootSignaturePartFromShader(Blob, (GetBasePath() + TEXT(".rs.cso")).data());
#else
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