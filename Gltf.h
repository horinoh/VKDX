#pragma once

#include <fx/gltf.h>

class Gltf 
{
public:
	virtual void Process(const fx::gltf::Document& Doc) {
		for (auto i : Doc.scenes) {
			Process(i);
		}
	}
	virtual void Process(const fx::gltf::Scene& Scn) {
		Scn.name;
		for (auto i : Scn.nodes) {
			Traverse(Document.nodes[i]);
		}
	}

	virtual void Traverse(const fx::gltf::Node& Nd) {
		Process(Nd);
		for (auto i : Nd.children) {
			Traverse(Document.nodes[i]);
		}
	}
	virtual void Process(const fx::gltf::Node& Nd) {
		Nd.name;

		//!< ローカルトランスフォームは以下のいずれかで表現される
		//!< Matrix (column-major)
		Nd.matrix;
		//!< TRS
		Nd.translation;
		Nd.rotation;
		Nd.scale;

		if (-1 != Nd.mesh) {
			Process(Document.meshes[Nd.mesh]);
		}
	}

	virtual void Process(const fx::gltf::Mesh& Msh) {
		Msh.name;

		for (const auto& i : Msh.primitives) {
			Process(i);
		}
		//for (auto i : Msh.weights) {}
	}

	virtual void Process(const fx::gltf::Primitive& Prim) {
		for (auto i : Prim.attributes) {
			i.first; //!< "POSITION", "NORMAL",...
			Process(Document.accessors[i.second]);
		}
		if (-1 != Prim.indices) {
			Process(Document.accessors[Prim.indices]);
		}
	}

	virtual void Process(const fx::gltf::Accessor& Acc) {
		Acc.name;

		if (-1 != Acc.bufferView) {
			Process(Document.bufferViews[Acc.bufferView]);
		}
		Acc.byteOffset;

		switch (Acc.componentType) {
		case fx::gltf::Accessor::ComponentType::None:
		case fx::gltf::Accessor::ComponentType::Byte:
		case fx::gltf::Accessor::ComponentType::UnsignedByte:
		case fx::gltf::Accessor::ComponentType::Short:
		case fx::gltf::Accessor::ComponentType::UnsignedShort:
		case fx::gltf::Accessor::ComponentType::UnsignedInt:
		case fx::gltf::Accessor::ComponentType::Float:
			break;
		}
		switch (Acc.type)
		{
		case fx::gltf::Accessor::Type::None:
		case fx::gltf::Accessor::Type::Scalar:
		case fx::gltf::Accessor::Type::Vec2:
		case fx::gltf::Accessor::Type::Vec3:
		case fx::gltf::Accessor::Type::Vec4:
		case fx::gltf::Accessor::Type::Mat2:
		case fx::gltf::Accessor::Type::Mat3:
		case fx::gltf::Accessor::Type::Mat4:
			break;
		}
		Acc.count; //!< 頂点数、インデックス数等
		Acc.min; Acc.max; //!< コンポーネント毎の最小、最大値 バウンディングボリューム等に使える
	}

	virtual void Process(const fx::gltf::BufferView& BufV) {
		BufV.name;

		if (-1 != BufV.buffer) {
		 	Process(Document.buffers[BufV.buffer]);
		}
		BufV.byteOffset;
		BufV.byteLength;
		BufV.byteStride;

		switch (BufV.target) {
		case fx::gltf::BufferView::TargetType::None:
		case fx::gltf::BufferView::TargetType::ArrayBuffer:
		case fx::gltf::BufferView::TargetType::ElementArrayBuffer:
			break;
		}
	}

	virtual void Process(const fx::gltf::Buffer& Buf) {
		Buf.name;

		Buf.byteLength;
		Buf.data;
		Buf.uri;
	}
	
protected:
	fx::gltf::Document Document;
};