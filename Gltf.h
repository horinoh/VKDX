#pragma once

#include <fx/gltf.h>

class Gltf 
{
public:
	static uint32_t GetTypeCount(const fx::gltf::Accessor::Type Type) {
		switch (Type) {
		default:
		case fx::gltf::Accessor::Type::None: return 0;
		case fx::gltf::Accessor::Type::Scalar: return 1;
		case fx::gltf::Accessor::Type::Vec2: return 2;
		case fx::gltf::Accessor::Type::Vec3: return 3;
		case fx::gltf::Accessor::Type::Vec4:
		case fx::gltf::Accessor::Type::Mat2: return 4;
		case fx::gltf::Accessor::Type::Mat3: return 9;
		case fx::gltf::Accessor::Type::Mat4: return 16;
		}		
	}
	static uint32_t GetComponentTypeSize(const fx::gltf::Accessor::ComponentType CompType) {
		switch (CompType) {
		default:
		case fx::gltf::Accessor::ComponentType::None: return 0;
		case fx::gltf::Accessor::ComponentType::Byte:
		case fx::gltf::Accessor::ComponentType::UnsignedByte: return 1;
		case fx::gltf::Accessor::ComponentType::Short:
		case fx::gltf::Accessor::ComponentType::UnsignedShort: return 2;
		case fx::gltf::Accessor::ComponentType::UnsignedInt:
		case fx::gltf::Accessor::ComponentType::Float: return 4;
		}
	}
	static uint32_t GetTypeSize(const fx::gltf::Accessor& Acc) { return GetTypeCount(Acc.type) * GetComponentTypeSize(Acc.componentType); }

	const uint8_t* GetData(const fx::gltf::Accessor& Acc) const {
		if (-1 != Acc.bufferView) {
			const auto& BufV = Document.bufferViews[Acc.bufferView];
			if (-1 != BufV.buffer) {
				const auto& Buf = Document.buffers[BufV.buffer];
				return &Buf.data[BufV.byteOffset + Acc.byteOffset];
			}
		}
		return nullptr;
	}
	const uint32_t GetStride(const fx::gltf::Accessor& Acc) const {
		if (-1 != Acc.bufferView) {
			const auto& BufV = Document.bufferViews[Acc.bufferView];
			if (-1 != BufV.buffer) {
				return 0 == BufV.byteStride ? GetTypeSize(Acc) : BufV.byteStride;
			}
		}
		return 0;
	}

	static bool DecomposeSemantic(const std::string& Semantic, std::string& Name, std::string& Index) {
		const auto pos = Semantic.find("_");
		if (std::string::npos != pos) {
			Name = Semantic.substr(0, pos);
			Index = Semantic.substr(pos + 1);
			//SemanticIndices.push_back({ i.first.substr(0, pos).c_str(), std::stoi(i.first.substr(pos + 1)) });
			return true;
		}
		else {
			//SemanticIndices.push_back({ i.first.c_str(), 0 });
			return false;
		}
	}

	virtual void Load(const std::string& Path) {
		if (std::string::npos != Path.rfind(".glb")){
			Document = fx::gltf::LoadFromBinary(Path, fx::gltf::ReadQuotas()); //!< .glb
		} else {
			Document = fx::gltf::LoadFromText(Path, fx::gltf::ReadQuotas()); //!< .gltf
		}

		if (!Document.extensionsUsed.empty()) {
			std::cout << "extensionsUsed = ";
			for (const auto& i : Document.extensionsUsed) {
				std::cout << i << ", ";
			}
			std::cout << std::endl;
		}
		if (!Document.extensionsRequired.empty()) {
			std::cout << "extensionsRequired = ";
			for (const auto& i : Document.extensionsRequired) {
				std::cout << i << ", ";
			}
			std::cout << std::endl;
		}

		std::cout << "Buffers" << std::endl;
		for (const auto& i : Document.buffers) {
			if (!i.name.empty()) {
				std::cout << "\t" << "name = " << i.name << std::endl;
			}
			if (!i.uri.empty()) {
				std::cout << "\t" << "uri = " << i.uri << std::endl;
			}
			std::cout << "\t" << "IsEmbeddedResource = " << i.IsEmbeddedResource() << std::endl;
			
			std::cout << "\t" << "byteLength = " << i.byteLength << std::endl;
			//i.data;
		}

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

		Push();
		for (auto i : Scn.nodes) {
			Process(Document.nodes[i]);	
		}
		Pop();
	}
	virtual void Process(const fx::gltf::Animation& Anim) {
		Tabs(); std::cout << "Animation : " << Anim.name << std::endl;

		Push();
		for (const auto& i : Anim.channels) {			
			const auto& Tag = i.target;

			//!< アニメーションのさせ方
			Tabs(); std::cout << "\t" << "path = " << Tag.path << std::endl;
			if ("translation" == Tag.path) {
			}
			else if ("rotation" == Tag.path) {
			}
			else if ("scale" == Tag.path) {
			}
			else if ("weights" == Tag.path) {
				//!< モーフターゲットのウエイトをアニメーションさせる場合
			}

			//!< アニメーション対象ノード
			Push();
			if (-1 != Tag.node) {
				//Process(Document.nodes[Tag.node]);
			}
			Pop();

			//!< サンプリング方法
			Push();
			if (-1 != i.sampler) {
				Process(Anim.samplers[i.sampler]);
			}
			Pop();
		}
		Pop();
	}

	virtual void Process(const fx::gltf::Animation::Sampler& Smp) {
		//!< 補完方法
		Tabs(); std::cout << "\t" << "interpolation = ";
		switch (Smp.interpolation)
		{
		case fx::gltf::Animation::Sampler::Type::Linear: std::cout << "Linear"; break;
		case fx::gltf::Animation::Sampler::Type::Step: std::cout << "Step"; break;
		case fx::gltf::Animation::Sampler::Type::CubicSpline: std::cout << "CubicSpline"; break;
		}
		std::cout << std::endl;

		Push();
		//!< キーフレーム時間
		if (-1 != Smp.input) {
			Process("input", Document.accessors[Smp.input]);
		}
		//!< アニメーション値 (pathにより解釈、translation, rotation,...)
		if (-1 != Smp.output) {
			Process("output", Document.accessors[Smp.output]);
		}
		Pop();
	}

	virtual void Process(const fx::gltf::Node& Nd) {
		Tabs(); std::cout << "Node : " << Nd.name << std::endl;

		//!< ローカルトランスフォームは以下のいずれかで表現される
		//!< Matrix (column-major)
		Tabs(); std::cout << "\t" << "matrix = ";
		for (auto i : Nd.matrix) { std::cout << i << ", "; }
		std::cout << std::endl;
		
		//!< TRS
		Tabs(); std::cout << "\t" << "translation = ";
		for (auto i : Nd.translation) { std::cout << i << ", "; }
		std::cout << std::endl; 
		
		Tabs(); std::cout << "\t" << "rotation = ";
		for (auto i : Nd.rotation) { std::cout << i << ", "; }
		std::cout << std::endl; 
		
		Tabs(); std::cout << "\t" << "scale = ";
		for (auto i : Nd.scale) { std::cout << i << ", "; }
		std::cout << std::endl; 

		Push();
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
		Pop();
	}

	virtual void Process(const fx::gltf::Mesh& Msh) {
		Tabs(); std::cout << "Mesh : " << Msh.name << std::endl;
		
		if (!Msh.weights.empty()) {
			Tabs(); std::cout << "\t" << "Weights = ";
			for (auto i : Msh.weights) {
				std::cout << i << ", ";
			}
			std::cout << std::endl;
		}

		Push();
		for (const auto& i : Msh.primitives) {
			Process(i);
		}
		Pop();
	}

	virtual void Process(fx::gltf::Skin& Skn) {
		Tabs(); std::cout << "Skin : " << Skn.name << std::endl;

		Push();
		if (-1 != Skn.inverseBindMatrices) {
			//!< 各々のジョイントをローカルスペースへ変換するマトリクス
			//!< JointMatrix[i] = Inverse(GlobalTransform) * GlobalJointTransform[i] * InverseBindMatrix[i]
			Process("inverseBindMatrices", Document.accessors[Skn.inverseBindMatrices]);
		}
		Pop();

		//Push();
		//for (auto i : Skn.joints) {
		//	//Process(Document.nodes[i]);
		//}
		//Pop();
	}

	virtual void Process(const fx::gltf::Camera& Cam) {
		Tabs(); std::cout << "Camera : " << Cam.name << std::endl;

		switch (Cam.type)
		{
		case fx::gltf::Camera::Type::None:
			break;
		case fx::gltf::Camera::Type::Orthographic:
			Tabs(); std::cout << "\t" << "type = Orthographic" << std::endl;
			Tabs(); std::cout << "\t" << "\t" << "xmag, ymag = " << Cam.orthographic.xmag << ", " << Cam.orthographic.ymag << std::endl;
			Tabs(); std::cout << "\t" << "\t" << "znear, zfar = " << Cam.orthographic.znear << ", " << Cam.orthographic.zfar << std::endl;
			break;
		case fx::gltf::Camera::Type::Perspective:
			Tabs(); std::cout << "\t" << "type = Perspective" << std::endl;
			Tabs(); std::cout << "\t" << "\t" << "yfov = " << Cam.perspective.yfov << std::endl;
			Tabs(); std::cout << "\t" << "\t" << "aspectRatio = " << Cam.perspective.aspectRatio << std::endl;
			Tabs(); std::cout << "\t" << "\t" << "znear, zfar = " << Cam.perspective.znear << ", " << Cam.perspective.zfar << std::endl;
			break;
		}
	}

	virtual void Process(const fx::gltf::Primitive& Prim) {
		Tabs(); std::cout << "mode = ";
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

		Push();
		for (const auto& i : Prim.attributes) {
			Process("attributes", Document.accessors[i.second]);
		}
		Pop();

		Push();
		if (-1 != Prim.indices) {
			Process("indices", Document.accessors[Prim.indices]);
		}
		Pop();

		Push();
		if (-1 != Prim.material) {
			Process(Document.materials[Prim.material]);
		}
		Pop();

		Push();
		for (const auto& i : Prim.targets) {
			for (const auto& j : Prim.attributes) {
				const auto it = i.find(j.first);
				if (i.end() != it) {
					Process("targets", Document.accessors[it->second]);
				}
			}
		}
		Pop();
	}
	virtual void Process(const std::string& Identifier, const fx::gltf::Accessor& Acc) {
		Tabs(); std::cout << Identifier << std::endl;
		Process(Acc);
	}
	virtual void Process(const fx::gltf::Accessor& Acc) {
		Tabs(); std::cout << "Accessor : " << Acc.name << std::endl;
		Tabs(); std::cout << "\t" << "Count = " << Acc.count << std::endl;
		Tabs(); std::cout << "\t" << "byteOffset = " << Acc.byteOffset << std::endl;

		Tabs(); std::cout << "\t" << "type = ";
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

		Tabs(); std::cout << "\t" << "componentType = ";
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
		Tabs(); std::cout << "\t" << "min = ";
		for (auto i : Acc.min) { std::cout << i << ", "; }
		std::cout << std::endl;
		
		Tabs(); std::cout << "\t" << "max = ";
		for (auto i : Acc.max) { std::cout << i << ", "; }
		std::cout << std::endl;

		Tabs(); std::cout << "\t" << "normalized = " << Acc.normalized << std::endl;

		Push();
		if (-1 != Acc.bufferView) {
			const auto& BufV = Document.bufferViews[Acc.bufferView];
			Tabs(); std::cout << "BufferView : " << BufV.name << std::endl;
			Tabs(); std::cout << "\t" << "byteOffset = " << BufV.byteOffset << std::endl;
			Tabs(); std::cout << "\t" << "byteLength = " << BufV.byteLength << std::endl;
			Tabs(); std::cout << "\t" << "byteStrikde = " << BufV.byteStride << std::endl;

			Tabs(); std::cout << "\t" << "target = ";
			switch (BufV.target) {
			case fx::gltf::BufferView::TargetType::None: std::cout << "None"; break;
			case fx::gltf::BufferView::TargetType::ArrayBuffer: std::cout << "ArrayBuffer"; break;
			case fx::gltf::BufferView::TargetType::ElementArrayBuffer: std::cout << "ElementArrayBuffer"; break;
			}
			std::cout << std::endl;

			Push();
			if (-1 != BufV.buffer) {
				const auto& Buf = Document.buffers[BufV.buffer];
				Tabs(); std::cout << "Buffer : " << Buf.name << std::endl;
				Tabs(); std::cout << "\t" << "ByteLength = " << Buf.byteLength << std::endl;
				if (Buf.uri.empty()) {
					if (Buf.IsEmbeddedResource()) {
						Tabs(); std::cout << "\t" << "IsEmbeddedResource = " << Buf.IsEmbeddedResource() << std::endl;
					} else {
						//!< [ Example ]
						//!< Buf.byteLength=10					|0|1|2|3|4|5|6|7|8|9|		
						//!< BV.byteOffset=1, BV.byteLength=7	  |1|2|3|4|5|6|7|			
						//!< Acc.byteOffset=1						|2|3|4|5|6|7|			
						//!< BV.byteStride=3						|2|3| |5|6|				
						//!< Acc.type=Vec2, Acc.conponentType=Byte	|x|y| |x|y|		

						//const auto BufVDataPtr = &Buf.data[BufV.byteOffset];
						//const auto BufVDataSize = BufV.byteLength;
						//const auto DataPtr = &Buf.data[BufV.byteOffset + Acc.byteOffset];
						//const auto DataStride = BufV.byteStride;
					}
				}
				else {
					Tabs(); std::cout << "\t" << "uri = " << Buf.uri << std::endl;
					if (Buf.IsEmbeddedResource()) {
						Tabs(); std::cout << "\t" << "IsEmbeddedResource = " << Buf.IsEmbeddedResource() << std::endl;
					}
					else {
						const auto Path = fx::gltf::detail::GetDocumentRootPath("../../") + "/" + Buf.uri;
					}
				}
			}
			Pop();
		}
		Pop();

		//!< TODO
		Acc.sparse;
	}

	virtual void Process(const fx::gltf::Material& Mtl) {
		Tabs(); std::cout << "Material : " << Mtl.name << std::endl;

		Tabs(); std::cout << "\t" << "alphaCutoff = " << Mtl.alphaCutoff << std::endl;

		Tabs(); std::cout << "\t" << "alphaMode = ";
		switch (Mtl.alphaMode) {
		case fx::gltf::Material::AlphaMode::Opaque: std::cout << "Opaque"; break;
		case fx::gltf::Material::AlphaMode::Mask: std::cout << "Mask"; break;
		case fx::gltf::Material::AlphaMode::Blend: std::cout << "Blend"; break;
		}
		std::cout << std::endl;

		Tabs(); std::cout << "\t" << "doubleSided = " << Mtl.doubleSided << std::endl;

		Tabs(); std::cout << "\t" << "emissiveFactor = ";
		for (auto i : Mtl.emissiveFactor) {
			std::cout << i << ", ";
		}
		std::cout << std::endl;

		//!< PBR (metallic-roughness model)
		const auto& PBR = Mtl.pbrMetallicRoughness;
		Push();
		Process(PBR.baseColorTexture);
		//!< baseColorTexture が無い場合は baseColorFactor がそのまま使われる
		Tabs(); std::cout << "\t" << "baseColorFactor";
		for (auto i : PBR.baseColorFactor) {
			std::cout << i << ", ";
		}
		std::cout << std::endl; 
		Pop();

		Push();
		//!< BLUE : Metalness, GREEN : Roughness
		Process(PBR.metallicRoughnessTexture);
		//!< metallicRoughnessTexture が無い場合は metallicFactor, roughnessFactor がそのまま使われる
		Tabs(); std::cout << "\t" << "metallicFactor = " << PBR.metallicFactor << std::endl;
		Tabs(); std::cout << "\t" << "roughnessFactor = " << PBR.roughnessFactor << std::endl;
		Pop();

		Push();
		Process(Mtl.normalTexture);
		Pop();

		Push();
		//!< RED : Occlusion
		Process(Mtl.occlusionTexture);
		Pop();

		Push();
		Process(Mtl.emissiveTexture);
		Pop();
	}

	virtual void Process(const fx::gltf::Material::Texture& Tex) {
		Tabs(); std::cout << "texCoord = " << Tex.texCoord << std::endl;

		Push();
		if (-1 != Tex.index) {
			Process(Document.textures[Tex.index]);
		}
		Pop();
	}
	virtual void Process(const fx::gltf::Texture& Tex) {
		Tabs(); std::cout << "Texture : " << Tex.name << std::endl;

		Push();
		if (-1 != Tex.sampler) {
			Process(Document.samplers[Tex.sampler]);
		}
		if (-1 != Tex.source) {
			Process(Document.images[Tex.source]);
		}
		Pop();
	}

	virtual void Process(const fx::gltf::Sampler& Smp) {
		Tabs(); std::cout << "Sampler : " << Smp.name << std::endl;

		Tabs(); std::cout << "\t" << "magFilter = ";
		switch (Smp.magFilter)
		{
		case fx::gltf::Sampler::MagFilter::None: std::cout << "None"; break;
		case fx::gltf::Sampler::MagFilter::Nearest: std::cout << "Nearest"; break;
		case fx::gltf::Sampler::MagFilter::Linear: std::cout << "Linear"; break;
		}
		std::cout << std::endl;

		Tabs(); std::cout << "\t" << "minFilter = ";
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

		Tabs(); std::cout << "\t" << "wrapS = ";
		switch (Smp.wrapS)
		{
		case fx::gltf::Sampler::WrappingMode::ClampToEdge: std::cout << "ClampToEdge"; break;
		case fx::gltf::Sampler::WrappingMode::MirroredRepeat: std::cout << "MirroredRepeat"; break;
		case fx::gltf::Sampler::WrappingMode::Repeat: std::cout << "Repeat"; break;
		}
		std::cout << std::endl;

		Tabs(); std::cout << "\t" << "wrapT = ";
		switch (Smp.wrapT)
		{
		case fx::gltf::Sampler::WrappingMode::ClampToEdge: std::cout << "ClampToEdge"; break;
		case fx::gltf::Sampler::WrappingMode::MirroredRepeat: std::cout << "MirroredRepeat"; break;
		case fx::gltf::Sampler::WrappingMode::Repeat: std::cout << "Repeat"; break;
		}
		std::cout << std::endl;
	}
	virtual void Process(const fx::gltf::Image& Img) {
		Tabs(); std::cout << "Image : " << Img.name << std::endl;

		Tabs(); std::cout << "\t" << "mimeType = " << Img.mimeType << std::endl;
		if (Img.uri.empty()) {
			if (Img.IsEmbeddedResource()) {
				Tabs(); std::cout << "\t" << "IsEmbeddedResource = " << Img.IsEmbeddedResource() << std::endl;

				Tabs(); std::cout << "\t" << "MaterializeData" << std::endl;
				std::vector<uint8_t> Data;
				Img.MaterializeData(Data);

				//const auto DataPtr = Data.data();
				//const auto DataSize = static_cast<uint32_t>(Data.size());
			}
			else {
				Push();
				const auto& BufV = Document.bufferViews[Img.bufferView];
				Tabs(); std::cout << "BufferView : " << BufV.name << std::endl;
				Tabs(); std::cout << "\t" << "byteOffset = " << BufV.byteOffset << std::endl;
				Tabs(); std::cout << "\t" << "byteLength = " << BufV.byteLength << std::endl;

				Push();
				if (-1 != BufV.buffer) {
					const auto& Buf = Document.buffers[BufV.buffer];
					Tabs(); std::cout << "Buffer : " << Buf.name << std::endl;
					Tabs(); std::cout << "\t" << "ByteLength = " << Buf.byteLength << std::endl;
				}
				Pop();

				//const auto DataPtr = &Document.buffers[BufV.buffer].data[BufV.byteOffset + 0/*ここではアクセサは無い*/];
				//const auto DataSize = BufV.byteLength;
				Pop();
			}
		}
		else {
			Tabs(); std::cout << "\t" << "uri = " << Img.uri << std::endl;
			if (Img.IsEmbeddedResource()) {
				Tabs(); std::cout << "\t" << "IsEmbeddedResource = " << Img.IsEmbeddedResource() << std::endl;
			}
			else {
				const auto Path = fx::gltf::detail::GetDocumentRootPath("../../") + "/" + Img.uri;
			}
		}

		Pop();
	}

#ifdef _DEBUG
	void Tabs() { for (auto i = 0; i < Hierarchy; ++i) { std::cout << "\t"; } }
	void Push() { ++Hierarchy; }
	void Pop() { --Hierarchy; }
#else
	void Tabs() {}
	void Push() {}
	void Pop() {}
#endif

protected:
	fx::gltf::Document Document;
#ifdef _DEBUG
	uint8_t Hierarchy = 0;
#endif
};