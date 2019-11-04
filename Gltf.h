#pragma once

#include <fx/gltf.h>

class Gltf 
{
public:
	virtual void Load(const std::string& Path) {
		Document = fx::gltf::LoadFromBinary(Path, fx::gltf::ReadQuotas());

		Process(Document);
	}

	virtual void Process(const fx::gltf::Document& Doc) {
		for (auto i : Doc.scenes) {
			Process(i);
		}

		for (const auto& i : Doc.animations) {
			Process(i);
		}

		std::cout << std::endl;
		std::cout << std::endl;
	}
	virtual void Process(const fx::gltf::Scene& Scn) {
		std::cout << "Scene : " << Scn.name << std::endl;
		for (auto i : Scn.nodes) {
			Process(Document.nodes[i]);	
		}
	}
	virtual void Process(const fx::gltf::Animation& Anim) {
		std::cout << "Animation : " << Anim.name << std::endl;

		for (const auto& Ch : Anim.channels) {
			const auto& Tag = Ch.target;
			std::cout << "\t" << "path = " << Tag.path << std::endl;
			if ("translation" == Tag.path) {
			}
			else if ("rotation" == Tag.path) {
			}
			else if ("scale" == Tag.path) {
			}

			if (-1 != Tag.node) {
				Process(Document.nodes[Tag.node]);
			}
			
			if (-1 != Ch.sampler) {
				Process(Anim.samplers[Ch.sampler]);
			}
		}
	}

	virtual void Process(const fx::gltf::Animation::Sampler& Smp) {
		std::cout << "\t" << "\t" << "interpolation = ";
		switch (Smp.interpolation)
		{
		case fx::gltf::Animation::Sampler::Type::Linear: std::cout << "Linear"; break;
		case fx::gltf::Animation::Sampler::Type::Step: std::cout << "Step"; break;
		case fx::gltf::Animation::Sampler::Type::CubicSpline: std::cout << "CubicSpline"; break;
		}
		std::cout << std::endl;

		if (-1 != Smp.input) {
			Process(Document.accessors[Smp.input]);
		}
		if (-1 != Smp.output) {
			Process(Document.accessors[Smp.output]);
		}
	}

	virtual void Process(const fx::gltf::Node& Nd) {
		std::cout << "Node : " << Nd.name << std::endl;

		//!< ローカルトランスフォームは以下のいずれかで表現される
		//!< Matrix (column-major)
		std::cout << "\t" << "matrix = ";
		for (auto i : Nd.matrix) { std::cout << i << ", "; }
		std::cout << std::endl;
		
		//!< TRS
		std::cout << "\t" << "translation = ";
		for (auto i : Nd.translation) { std::cout << i << ", "; }
		std::cout << std::endl; 
		std::cout << "\t" << "rotation = ";
		for (auto i : Nd.rotation) { std::cout << i << ", "; }
		std::cout << std::endl; 
		std::cout << "\t" << "scale = ";
		for (auto i : Nd.scale) { std::cout << i << ", "; }
		std::cout << std::endl; 

		if (-1 != Nd.mesh) {
			Process(Document.meshes[Nd.mesh]);
		}
		
		if (-1 != Nd.skin) {
			Process(Document.skins[Nd.skin]);
		}

		if (-1 != Nd.camera) {
			Process(Document.cameras[Nd.camera]);
		}

		for (auto i : Nd.children) {
			Process(Document.nodes[i]);
		}
	}

	virtual void Process(const fx::gltf::Mesh& Msh) {
		std::cout << "\t" << "Mesh : " << Msh.name << std::endl;
		
		if (!Msh.weights.empty()) {
			std::cout << "\t" << "\t" << "Weights = ";
			for (auto i : Msh.weights) {
				std::cout << i;
			}
			std::cout << std::endl;
		}

		for (const auto& i : Msh.primitives) {
			Process(i);
		}
	}

	virtual void Process(fx::gltf::Skin& Skn) {
		std::cout << "\t" << "Skin : " << Skn.name << std::endl;

		if (-1 != Skn.inverseBindMatrices) {
			Process(Document.accessors[Skn.inverseBindMatrices]);
		}

		for (auto i : Skn.joints) {
			Process(Document.nodes[i]);
		}
	}

	virtual void Process(const fx::gltf::Camera& Cam) {
		std::cout << "\t" << "Camera : " << Cam.name << std::endl;

		switch (Cam.type)
		{
		case fx::gltf::Camera::Type::None:
			break;
		case fx::gltf::Camera::Type::Orthographic:
			std::cout << "\t" << "\t" << "Orthographic" << std::endl;
			std::cout << "\t" << "\t" << "\t" << "xmag, ymag = " << Cam.orthographic.xmag << ", " << Cam.orthographic.ymag << std::endl;
			std::cout << "\t" << "\t" << "\t" << "znear, zfar = " << Cam.orthographic.znear << ", " << Cam.orthographic.zfar << std::endl;
			break;
		case fx::gltf::Camera::Type::Perspective:
			std::cout << "\t" << "\t" << "Perspective" << std::endl;
			std::cout << "\t" << "\t" << "\t" << "yfov = " << Cam.perspective.yfov << std::endl;
			std::cout << "\t" << "\t" << "\t" << "aspectRatio = " << Cam.perspective.aspectRatio << std::endl;
			std::cout << "\t" << "\t" << "\t" << "znear, zfar = " << Cam.perspective.znear << ", " << Cam.perspective.zfar << std::endl;
			break;
		}
	}

	virtual void Process(const fx::gltf::Primitive& Prim) {
		std::cout << "\t" << "\t" << "mode = ";
		switch (Prim.mode)
		{
		case fx::gltf::Primitive::Mode::Points: std::cout << "Points"; break;
		case fx::gltf::Primitive::Mode::Lines: std::cout << "Lines"; break;
		case fx::gltf::Primitive::Mode::LineLoop: std::cout << "LineLoop"; break;
		case fx::gltf::Primitive::Mode::LineStrip: std::cout << "LineStrip"; break;
		case fx::gltf::Primitive::Mode::Triangles: std::cout << "Triangles"; break;
		case fx::gltf::Primitive::Mode::TriangleStrip: std::cout << "TriangleStrip"; break;
		case fx::gltf::Primitive::Mode::TriangleFan: std::cout << "TriangleFan"; break;
		}
		std::cout << std::endl;

		for (auto i : Prim.attributes) {
			std::cout << "\t" << i.first << std::endl;
			Process(Document.accessors[i.second]);
		}

		if (-1 != Prim.indices) {
			std::cout << "\t" << "indices" << std::endl;
			Process(Document.accessors[Prim.indices]);
		}

		if (-1 != Prim.material) {
			Process(Document.materials[Prim.material]);
		}

		//!< Morph TODO
		for (const auto& Attr : Prim.targets) {
			for (auto i : Attr) {
				std::cout << "\t" << i.first << std::endl;
				Process(Document.accessors[i.second]);
			}
		}
	}

	virtual void Process(const fx::gltf::Accessor& Acc) {
		std::cout << "\t" << "Accessor : " << Acc.name << std::endl;
		std::cout << "\t" << "\t" << "Count = " << Acc.count << std::endl;
		std::cout << "\t" << "\t" << "byteOffset = " << Acc.byteOffset << std::endl;

		std::cout << "\t" << "\t" << "type = ";
		switch (Acc.type)
		{
		case fx::gltf::Accessor::Type::None: std::cout << "None"; break;
		case fx::gltf::Accessor::Type::Scalar: std::cout << "Scalar"; break;
		case fx::gltf::Accessor::Type::Vec2: std::cout << "Vec2"; break;
		case fx::gltf::Accessor::Type::Vec3: std::cout << "Vec3"; break;
		case fx::gltf::Accessor::Type::Vec4: std::cout << "Vec4"; break;
		case fx::gltf::Accessor::Type::Mat2: std::cout << "Mat2"; break;
		case fx::gltf::Accessor::Type::Mat3: std::cout << "Mat3"; break;
		case fx::gltf::Accessor::Type::Mat4: std::cout << "Mat4"; break;
		}
		std::cout << std::endl;

		std::cout << "\t" << "\t" << "componentType = ";
		switch (Acc.componentType) {
		case fx::gltf::Accessor::ComponentType::None: std::cout << "None"; break;
		case fx::gltf::Accessor::ComponentType::Byte: std::cout << "Byte"; break;
		case fx::gltf::Accessor::ComponentType::UnsignedByte: std::cout << "UnsignedByte"; break;
		case fx::gltf::Accessor::ComponentType::Short: std::cout << "Short"; break;
		case fx::gltf::Accessor::ComponentType::UnsignedShort: std::cout << "UnsignedShort"; break;
		case fx::gltf::Accessor::ComponentType::UnsignedInt: std::cout << "UnsignedInt"; break;
		case fx::gltf::Accessor::ComponentType::Float: std::cout << "Float"; break;
		}
		std::cout << std::endl;

		//!< コンポーネント毎の最小、最大値(バウンディングボリューム等)
		std::cout << "\t" << "\t" << "min = ";
		for (auto i : Acc.min) { std::cout << i << ", "; }
		std::cout << std::endl;
		std::cout << "\t" << "\t" << "max = ";
		for (auto i : Acc.max) { std::cout << i << ", "; }
		std::cout << std::endl;

		std::cout << "\t" << "\t" << "normalized = " << Acc.normalized << std::endl;

		if (-1 != Acc.bufferView) {
			Process(Document.bufferViews[Acc.bufferView]);
		}

		//!< TODO
		Acc.sparse;
	}

	virtual void Process(const fx::gltf::BufferView& BufV) {
		std::cout << "\t" << "\t" << "BufferView : " << BufV.name << std::endl;
		std::cout << "\t" << "\t" << "\t" << "byteOffset = " << BufV.byteOffset << std::endl;
		std::cout << "\t" << "\t" << "\t" << "byteLength = " << BufV.byteLength << std::endl;
		std::cout << "\t" << "\t" << "\t" << "byteStrikde = " << BufV.byteStride << std::endl;
		switch (BufV.target) {
		case fx::gltf::BufferView::TargetType::None:
			break;
		case fx::gltf::BufferView::TargetType::ArrayBuffer:
			std::cout << "\t" << "\t" << "\t" << "Type = " << "ArrayBuffer" << std::endl;
			break;
		case fx::gltf::BufferView::TargetType::ElementArrayBuffer:
			std::cout << "\t" << "\t" << "\t" << "Type = " << "ElementArrayBuffer" << std::endl;
			break;
		}

		if (-1 != BufV.buffer) {
			const auto& Buf = Document.buffers[BufV.buffer];
			std::cout << "\t" << "\t" << "\t" << "Buffer : " << Buf.name << std::endl;
			std::cout << "\t" << "\t" << "\t" << "\t" << "ByteLength = " << Buf.byteLength << std::endl;
			if (Buf.IsEmbeddedResource()) {}
			if (Buf.byteLength > 2) {
				std::cout << "\t" << "\t" << "\t" << "\t" << "data = " << Buf.data[0] << Buf.data[1] << Buf.data[2] << "..." << std::endl;
			}
			if (!Buf.uri.empty()) {
				std::cout << "\t" << "\t" << "\t" << "\t" << "uri = " << Buf.uri << std::endl;
			}
		}
	}

	virtual void Process(const fx::gltf::Material& Mtl) {
		std::cout << "Material : " << Mtl.name << std::endl;

		std::cout << "\t" << "alphaCutoff = " << Mtl.alphaCutoff << std::endl;

		std::cout << "\t" << "alphaMode = ";
		switch (Mtl.alphaMode) {
		case fx::gltf::Material::AlphaMode::Opaque: std::cout << "Opaque"; break;
		case fx::gltf::Material::AlphaMode::Mask: std::cout << "Mask"; break;
		case fx::gltf::Material::AlphaMode::Blend: std::cout << "Blend"; break;
		}
		std::cout << std::endl;

		std::cout << "\t" << "doubleSided = " << Mtl.doubleSided << std::endl;

		std::cout << "\t" << "emissiveFactor = ";
		for (auto i : Mtl.emissiveFactor) {
			std::cout << i << ", ";
		}
		std::cout << std::endl;

		Process(Mtl.emissiveTexture);
		Process(Mtl.normalTexture);
		Process(Mtl.occlusionTexture);

		//!< PBR (metallic-roughness model)
		const auto& PBR = Mtl.pbrMetallicRoughness;
		std::cout << "PBR" << std::endl;
		std::cout << "\t" << "metallicFactor = " << PBR.metallicFactor << std::endl;
		std::cout << "\t" << "roughnessFactor = " << PBR.roughnessFactor << std::endl;
		std::cout << "\t" << "baseColorFactor";
		for (auto i : PBR.baseColorFactor) {
			std::cout << i << ", ";
		}
		std::cout << std::endl;
		Process(PBR.baseColorTexture);
		Process(PBR.metallicRoughnessTexture);
	}

	virtual void Process(const fx::gltf::Material::Texture& Tex) {
		std::cout << "texCoord = " << Tex.texCoord << std::endl;

		if (-1 != Tex.index) {
			Process(Document.textures[Tex.index]);
		}
	}
	virtual void Process(const fx::gltf::Texture& Tex) {
		std::cout << "Texture : " << Tex.name << std::endl;

		if (-1 != Tex.sampler) {
			Process(Document.samplers[Tex.sampler]);
		}
		if (-1 != Tex.source) {
			Process(Document.images[Tex.source]);
		}
	}

	virtual void Process(const fx::gltf::Sampler& Smp) {
		std::cout << "Sampler : " << Smp.name << std::endl;

		std::cout << "\t" << "magFilter = ";
		switch (Smp.magFilter)
		{
		case fx::gltf::Sampler::MagFilter::None: std::cout << "None"; break;
		case fx::gltf::Sampler::MagFilter::Nearest: std::cout << "Nearest"; break;
		case fx::gltf::Sampler::MagFilter::Linear: std::cout << "Linear"; break;
		}
		std::cout << std::endl;

		std::cout << "\t" << "minFilter = ";
		switch (Smp.minFilter)
		{
		case fx::gltf::Sampler::MinFilter::None: std::cout << "None"; break;
		case fx::gltf::Sampler::MinFilter::Nearest: std::cout << "Nearest"; break;
		case fx::gltf::Sampler::MinFilter::Linear: std::cout << "Linear"; break;
		case fx::gltf::Sampler::MinFilter::NearestMipMapNearest: std::cout << "NearestMipMapNearest"; break;
		case fx::gltf::Sampler::MinFilter::LinearMipMapNearest: std::cout << "LinearMipMapNearest"; break;
		case fx::gltf::Sampler::MinFilter::NearestMipMapLinear: std::cout << "NearestMipMapLinear"; break;
		case fx::gltf::Sampler::MinFilter::LinearMipMapLinear: std::cout << "LinearMipMapLinear"; break;
		}
		std::cout << std::endl;

		std::cout << "\t" << "wrapS = ";
		switch (Smp.wrapS)
		{
		case fx::gltf::Sampler::WrappingMode::ClampToEdge: std::cout << "ClampToEdge"; break;
		case fx::gltf::Sampler::WrappingMode::MirroredRepeat: std::cout << "MirroredRepeat"; break;
		case fx::gltf::Sampler::WrappingMode::Repeat: std::cout << "Repeat"; break;
		}
		std::cout << std::endl;

		std::cout << "\t" << "wrapT = ";
		switch (Smp.wrapT)
		{
		case fx::gltf::Sampler::WrappingMode::ClampToEdge: std::cout << "ClampToEdge"; break;
		case fx::gltf::Sampler::WrappingMode::MirroredRepeat: std::cout << "MirroredRepeat"; break;
		case fx::gltf::Sampler::WrappingMode::Repeat: std::cout << "Repeat"; break;
		}
		std::cout << std::endl;
	}
	virtual void Process(const fx::gltf::Image& Img) {
		std::cout << "Image : " << Img.name << std::endl;

		std::cout << "\t" << "mimeType = " << Img.mimeType << std::endl;
		std::cout << "\t" << "IsEmbeddedResource = " << Img.IsEmbeddedResource() << std::endl;

		if (!Img.uri.empty()) {
			std::cout << "\t" << "uri = " << Img.uri << std::endl;

			std::vector<uint8_t> Data;
			Img.MaterializeData(Data);
		}

		Process(Document.bufferViews[Img.bufferView]);
	}

protected:
	fx::gltf::Document Document;
};