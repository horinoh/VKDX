#pragma once

#include <fx/gltf.h>
#include <cmath>

//!< KHR_texture_transform 拡張
//!< https://github.com/KhronosGroup/glTF/tree/master/extensions/2.0/Khronos/KHR_texture_transform
#define USE_GLTF_EXT_TEX_TRANS

//!< MSFT_texture_dds 拡張
//!< https://github.com/KhronosGroup/glTF/tree/master/extensions/2.0/Vendor/MSFT_texture_dds
#define USE_GLTF_EXT_TEX_DDS

static std::array<float, 3> operator+(const std::array<float, 3>& lhs, const std::array<float, 3>& rhs) { return std::array<float, 3>({ lhs[0] + rhs[0], lhs[1] + rhs[1], lhs[2] + rhs[2] }); }
static std::array<float, 3> operator*(const std::array<float, 3>& lhs, const float rhs) { return std::array<float, 3>({ lhs[0] * rhs, lhs[1] * rhs, lhs[2] * rhs }); }
static std::array<float, 3> operator*(const float rhs, const std::array<float, 3>& lhs) { return lhs * rhs; }

static std::array<float, 4> operator+(const std::array<float, 4>& lhs, const std::array<float, 4>& rhs) { return std::array<float, 4>({ lhs[0] + rhs[0], lhs[1] + rhs[1], lhs[2] + rhs[2], lhs[3] + rhs[3] }); }
static std::array<float, 4> operator*(const std::array<float, 4>& lhs, const float rhs) { return std::array<float, 4>({ lhs[0] * rhs, lhs[1] * rhs, lhs[2] * rhs, lhs[3] * rhs }); }
static std::array<float, 4> operator*(const float rhs, const std::array<float, 4>& lhs) { return lhs * rhs; }

class Gltf
{
public:
	static uint32_t GetTypeCount(const fx::gltf::Accessor::Type Type) {
		switch (Type) {
			using enum fx::gltf::Accessor::Type;
		default:
		case None: return 0;
		case Scalar: return 1;
		case Vec2: return 2;
		case Vec3: return 3;
		case Vec4:
		case Mat2: return 4;
		case Mat3: return 9;
		case Mat4: return 16;
		}		
	}
	static uint32_t GetComponentTypeSize(const fx::gltf::Accessor::ComponentType CompType) {
		switch (CompType) {
			using enum fx::gltf::Accessor::ComponentType;
		default:
		case None: return 0;
		case Byte:
		case UnsignedByte: return 1;
		case Short:
		case UnsignedShort: return 2;
		case UnsignedInt:
		case Float: return 4;
		}
	}
	static uint32_t GetTypeSize(const fx::gltf::Accessor& Acc) { return GetTypeCount(Acc.type) * GetComponentTypeSize(Acc.componentType); }

	const uint8_t* GetData(const fx::gltf::Accessor& Acc) const {
		if (-1 != Acc.bufferView) {
			const auto& Doc = GetDocument();
			const auto& BufV = Doc.bufferViews[Acc.bufferView];
			if (-1 != BufV.buffer) {
				const auto& Buf = Doc.buffers[BufV.buffer];
				return &Buf.data[BufV.byteOffset + Acc.byteOffset];
			}
		}
		return nullptr;
	}
	const uint32_t GetStride(const fx::gltf::Accessor& Acc) const {
		if (-1 != Acc.bufferView) {
			const auto& BufV = GetDocument().bufferViews[Acc.bufferView];
			if (-1 != BufV.buffer) {
				return 0 == BufV.byteStride ? GetTypeSize(Acc) : BufV.byteStride;
			}
		}
		return 0;
	}

	static bool DecomposeSemantic(const std::string_view Semantic, std::string& Name, std::string& Index) {
		if (const auto pos = Semantic.find("_");std::string::npos != pos) {
			Name = Semantic.substr(0, pos);
			Index = Semantic.substr(pos + 1);
			return true;
		}
		else {
			return false;
		}
	}

	virtual void Unload() { Documents.clear(); }
	virtual void Load(const std::string& Path) {
		if (std::string::npos != Path.rfind(".glb")) {
			Documents.emplace_back(fx::gltf::LoadFromBinary(Path, fx::gltf::ReadQuotas()));
		} else /*if(std::string::npos != Path.rfind(".gltf"))*/{
			Documents.emplace_back(fx::gltf::LoadFromText(Path, fx::gltf::ReadQuotas()));
		}

		const auto& Doc = GetDocument();
		if (!empty(Doc.extensionsUsed)) {
			std::cout << "extensionsUsed = ";
			for (const auto& i : Doc.extensionsUsed) {
				std::cout << i << ", ";
			}
			std::cout << std::endl;
		}
		if (!empty(Doc.extensionsRequired)) {
			std::cout << "extensionsRequired = ";
			for (const auto& i : Doc.extensionsRequired) {
				std::cout << i << ", ";
			}
			std::cout << std::endl;
		}

		//std::cout << "Buffers" << std::endl;
		//for (const auto& i : Doc.buffers) {
		//	if (!empty(i.name)) {
		//		std::cout << "\t" << "name = " << i.name << std::endl;
		//	}
		//	if (!empty(i.uri)) {
		//		std::cout << "\t" << "uri = " << i.uri << std::endl;
		//	}
		//	std::cout << "\t" << "IsEmbeddedResource = " << i.IsEmbeddedResource() << std::endl;
		//	
		//	std::cout << "\t" << "byteLength = " << i.byteLength << std::endl;
		//	//i.data;
		//}

		PreProcess(); {
			Process(Doc);
		} PostProcess();
	}
	virtual void PreProcess() {}
	virtual void PostProcess() {}

	virtual void Process(const fx::gltf::Document& Doc) {
		std::cout << "copyright = " << Doc.asset.copyright << std::endl;
		std::cout << "version = " << Doc.asset.version << std::endl;
		std::cout << "minVersion = " << Doc.asset.minVersion << std::endl;
		std::cout << "generator = " << Doc.asset.generator << std::endl;

		for (auto i : Doc.scenes) {
			Process(i);
		}

		for (const auto& i : Doc.animations) {
			Process(i);
		}

		//for (const auto& i : Doc.nodes) {}

		std::cout << std::endl;
		std::cout << std::endl;
	}
	virtual void Process(const fx::gltf::Scene& Scn) {
		std::cout << "Scene : " << Scn.name << std::endl;

		PushTab();
		for (auto i : Scn.nodes) {
			Process(GetDocument().nodes[i]);	
		}
		PopTab();
	}
	virtual void Process(const fx::gltf::Animation& Anim) {
		Tabs(); std::cout << "Animation : " << Anim.name << std::endl;

		PushTab();
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
			PushTab();
			if (-1 != Tag.node) {
				//Process(Document.nodes[Tag.node]);
			}
			PopTab();

			//!< サンプリング方法
			PushTab();
			if (-1 != i.sampler) {
				Process(Anim.samplers[i.sampler]);
			}
			PopTab();
		}
		PopTab();
	}

	virtual void Process(const fx::gltf::Animation::Sampler& Smp) {
		//!< 補完方法
		Tabs(); std::cout << "\t" << "interpolation = ";
		switch (Smp.interpolation)
		{
			using enum fx::gltf::Animation::Sampler::Type;
		case Linear: std::cout << "Linear"; break;
		case Step: std::cout << "Step"; break;
		case CubicSpline: std::cout << "CubicSpline"; break;
		}
		std::cout << std::endl;

		const auto& Doc = GetDocument();
		PushTab();
		//!< キーフレーム時間
		if (-1 != Smp.input) {
			Process("input", Doc.accessors[Smp.input]);
		}
		PopTab();

		PushTab();
		//!< アニメーション値 (pathにより解釈、translation, rotation,...)
		if (-1 != Smp.output) {
			Process("output", Doc.accessors[Smp.output]);
		}
		PopTab();
	}

	virtual void PushNode() { PushTab(); }
	virtual void PopNode() { PopTab(); }
	virtual void Process(const fx::gltf::Node& Nd, const uint32_t /*i*/) { Process(Nd); }
	virtual void Process(const fx::gltf::Node& Nd) {
		Tabs(); std::cout << "Node : " << Nd.name << std::endl;

		if (fx::gltf::defaults::IdentityMatrix != Nd.matrix) {
			//!< column-major
			Tabs(); std::cout << "\t" << "matrix = ";
			for (auto i : Nd.matrix) { std::cout << i << ", "; }
			std::cout << std::endl;
		}
		else {
			if (fx::gltf::defaults::NullVec3 != Nd.translation) {
				Tabs(); std::cout << "\t" << "translation = ";
				for (auto i : Nd.translation) { std::cout << i << ", "; }
				std::cout << std::endl;
			}

			if (fx::gltf::defaults::IdentityRotation != Nd.rotation) {
				Tabs(); std::cout << "\t" << "rotation = ";
				for (auto i : Nd.rotation) { std::cout << i << ", "; }
				std::cout << std::endl;
			}

			if (fx::gltf::defaults::IdentityVec3 != Nd.scale) {
				Tabs(); std::cout << "\t" << "scale = ";
				for (auto i : Nd.scale) { std::cout << i << ", "; }
				std::cout << std::endl;
			}
		}

		const auto& Doc = GetDocument();
	
		PushTab();
		if (-1 != Nd.camera) {
			Process(Doc.cameras[Nd.camera]);
		}
		PopTab();

		PushTab();
		if (-1 != Nd.skin) {
			Process(Doc.skins[Nd.skin]);
		}
		PopTab();
		
		PushTab();
		if (-1 != Nd.mesh) {
			Process(Doc.meshes[Nd.mesh]);
		}
		PopTab();

		if (!empty(Nd.children)) {
			PushNode();
			for (auto i : Nd.children) {
				Process(Doc.nodes[i], i);
			}
			PopNode();
		}
	}

	virtual void Process(const fx::gltf::Mesh& Msh) {
		Tabs(); std::cout << "Mesh : " << Msh.name << std::endl;
		
		if (!empty(Msh.weights)) {
			Tabs(); std::cout << "\t" << "Weights = ";
			for (auto i : Msh.weights) {
				std::cout << i << ", ";
			}
			std::cout << std::endl;
		}

		PushTab();
		for (const auto& i : Msh.primitives) {
			Process(i);
		}
		PopTab();
	}

	virtual void Process(const fx::gltf::Skin& Skn) {
		Tabs(); std::cout << "Skin : " << Skn.name << std::endl;

		PushTab();
		if (-1 != Skn.inverseBindMatrices) {
			//!< 各々のジョイントをローカルスペースへ変換するマトリクス
			//!< JointMatrix[i] = Inverse(GlobalTransform) * GlobalJointTransform[i] * InverseBindMatrix[i]
			Process("inverseBindMatrices", GetDocument().accessors[Skn.inverseBindMatrices]);
		}
		PopTab();

		//Push();
		//for (auto i : Skn.joints) {
		//	//Process(GetDocument().nodes[i]);
		//}
		//Pop();
	}

	virtual void Process(const fx::gltf::Camera& Cam) {
		Tabs(); std::cout << "Camera : " << Cam.name << std::endl;

		switch (Cam.type)
		{
			using enum fx::gltf::Camera::Type;
		case None:
			break;
		case Orthographic:
			Tabs(); std::cout << "\t" << "type = Orthographic" << std::endl;
			Tabs(); std::cout << "\t" << "\t" << "xmag, ymag = " << Cam.orthographic.xmag << ", " << Cam.orthographic.ymag << std::endl;
			Tabs(); std::cout << "\t" << "\t" << "znear, zfar = " << Cam.orthographic.znear << ", " << Cam.orthographic.zfar << std::endl;
			break;
		case Perspective:
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
			using enum fx::gltf::Primitive::Mode;
		case Points: std::cout << "Points"; break;
		case Lines: std::cout << "Lines"; break;
		case LineLoop: std::cout << "LineLoop"; break;
		case LineStrip: std::cout << "LineStrip"; break;
		case Triangles: std::cout << "Triangles"; break;
		case TriangleStrip: std::cout << "TriangleStrip"; break;
		case TriangleFan: std::cout << "TriangleFan"; break;
		}
		std::cout << std::endl;

		const auto& Doc = GetDocument();
		PushTab();
		for (const auto& i : Prim.attributes) {
			Process("attributes", Doc.accessors[i.second]);
		}
		PopTab();

		PushTab();
		if (-1 != Prim.indices) {
			Process("indices", Doc.accessors[Prim.indices]);
		}
		PopTab();

		PushTab();
		if (-1 != Prim.material) {
			Process(Doc.materials[Prim.material]);
		}
		PopTab();

		PushTab();
		for (const auto& i : Prim.targets) {
			for (const auto& j : Prim.attributes) {
				if (const auto it = i.find(j.first); end(i) != it) {
					Process("targets", Doc.accessors[it->second]);
				}
			}
		}
		PopTab();
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
			using enum fx::gltf::Accessor::Type;
		case None: std::cout << "None"; break;
		case Scalar: std::cout << "Scalar"; break;
		case Vec2: std::cout << "Vec2"; break;
		case Vec3: std::cout << "Vec3"; break;
		case Vec4: std::cout << "Vec4"; break;
		case Mat2: std::cout << "Mat2"; break;
		case Mat3: std::cout << "Mat3"; break;
		case Mat4: std::cout << "Mat4"; break;
		}
		std::cout << std::endl;

		Tabs(); std::cout << "\t" << "componentType = ";
		switch (Acc.componentType) {
			using enum fx::gltf::Accessor::ComponentType;
		case None: std::cout << "None"; break;
		case Byte: std::cout << "Byte"; break;
		case UnsignedByte: std::cout << "UnsignedByte"; break;
		case Short: std::cout << "Short"; break;
		case UnsignedShort: std::cout << "UnsignedShort"; break;
		case UnsignedInt: std::cout << "UnsignedInt"; break;
		case Float: std::cout << "Float"; break;
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

		const auto& Doc = GetDocument();
		PushTab();
		if (-1 != Acc.bufferView) {
			const auto& BufV = Doc.bufferViews[Acc.bufferView];
			Tabs(); std::cout << "BufferView : " << BufV.name << std::endl;
			Tabs(); std::cout << "\t" << "byteOffset = " << BufV.byteOffset << std::endl;
			Tabs(); std::cout << "\t" << "byteLength = " << BufV.byteLength << std::endl;
			Tabs(); std::cout << "\t" << "byteStrikde = " << BufV.byteStride << std::endl;

			Tabs(); std::cout << "\t" << "target = ";
			switch (BufV.target) {
				using enum fx::gltf::BufferView::TargetType;
			case None: std::cout << "None"; break;
			case ArrayBuffer: std::cout << "ArrayBuffer"; break;
			case ElementArrayBuffer: std::cout << "ElementArrayBuffer"; break;
			}
			std::cout << std::endl;

			PushTab();
			if (-1 != BufV.buffer) {
				const auto& Buf = Doc.buffers[BufV.buffer];
				Tabs(); std::cout << "Buffer : " << Buf.name << std::endl;
				Tabs(); std::cout << "\t" << "ByteLength = " << Buf.byteLength << std::endl;
				if (empty(Buf.uri)) {
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
			PopTab();
		}
		PopTab();

		//!< TODO
		Acc.sparse;
	}

	virtual void Process(const fx::gltf::Material& Mtl) {
		Tabs(); std::cout << "Material : " << Mtl.name << std::endl;

		Tabs(); std::cout << "\t" << "alphaCutoff = " << Mtl.alphaCutoff << std::endl;

		Tabs(); std::cout << "\t" << "alphaMode = ";
		switch (Mtl.alphaMode) {
			using enum fx::gltf::Material::AlphaMode;
		case Opaque: std::cout << "Opaque"; break;
		case Mask: std::cout << "Mask"; break;
		case Blend: std::cout << "Blend"; break;
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
		PushTab();
		Process(PBR.baseColorTexture);
		//!< baseColorTexture が無い場合は baseColorFactor がそのまま使われる
		Tabs(); std::cout << "\t" << "baseColorFactor";
		for (auto i : PBR.baseColorFactor) {
			std::cout << i << ", ";
		}
		std::cout << std::endl; 
		PopTab();

		PushTab();
		//!< BLUE : Metalness, GREEN : Roughness
		Process(PBR.metallicRoughnessTexture);
		//!< metallicRoughnessTexture が無い場合は metallicFactor, roughnessFactor がそのまま使われる
		Tabs(); std::cout << "\t" << "metallicFactor = " << PBR.metallicFactor << std::endl;
		Tabs(); std::cout << "\t" << "roughnessFactor = " << PBR.roughnessFactor << std::endl;
		PopTab();

		PushTab();
		Process(Mtl.normalTexture);
		PopTab();

		PushTab();
		//!< RED : Occlusion
		Process(Mtl.occlusionTexture);
		PopTab();

		PushTab();
		Process(Mtl.emissiveTexture);
		PopTab();
	}

	virtual void Process(const fx::gltf::Material::Texture& Tex) {
		Tabs(); std::cout << "texCoord = " << Tex.texCoord << std::endl;

		PushTab();
		if (-1 != Tex.index) {
			Process(GetDocument().textures[Tex.index]);
		}
		PopTab();

#ifdef USE_GLTF_EXT_TEX_TRANS
		if (const auto ItExtensions = Tex.extensionsAndExtras.find("extensions"); ItExtensions != end(Tex.extensionsAndExtras)) {
			if (const auto ItTexTransform = ItExtensions->find("KHR_texture_transform"); ItTexTransform != end(*ItExtensions)) {
				if (const auto ItOffset = ItTexTransform->find("offset"); ItOffset != end(*ItTexTransform)) {
					if (ItOffset->is_array() && 2 == size(*ItOffset)) {
						if (ItOffset->at(0).is_number_float()) {
							std::cout << ItOffset->at(0).get<float>() << ", " << ItOffset->at(1).get<float>() << std::endl;
						}
					}
				}
			}
		}
		if (const auto ItExtras = Tex.extensionsAndExtras.find("extras"); ItExtras != end(Tex.extensionsAndExtras)) {}
#endif
	}
	virtual void Process(const fx::gltf::Texture& Tex) {
		Tabs(); std::cout << "Texture : " << Tex.name << std::endl;

		const auto& Doc = GetDocument();
		PushTab();
		if (-1 != Tex.sampler) {
			Process(Doc.samplers[Tex.sampler]);
		}
		if (-1 != Tex.source) {
			Process(Doc.images[Tex.source]);
		}
		PopTab();

#ifdef USE_GLTF_EXT_TEX_DDS
		if (const auto ItExtensions = Tex.extensionsAndExtras.find("extensions"); ItExtensions != end(Tex.extensionsAndExtras)) {
			if (const auto ItTexDDS = ItExtensions->find("MSFT_texture_dds"); ItTexDDS != end(*ItExtensions)) {
				if (const auto ItSrc = ItTexDDS->find("source"); ItSrc != end(*ItTexDDS)) {
					if (ItSrc->is_number_integer()) {
						std::cout << ItSrc->get<int32_t>() << std::endl;
					}
				}
			}
		}
		if (const auto ItExtras = Tex.extensionsAndExtras.find("extras"); ItExtras != end(Tex.extensionsAndExtras)) {}
#endif
	}

	virtual void Process(const fx::gltf::Sampler& Smp) {
		Tabs(); std::cout << "Sampler : " << Smp.name << std::endl;

		Tabs(); std::cout << "\t" << "magFilter = ";
		switch (Smp.magFilter)
		{
			using enum fx::gltf::Sampler::MagFilter;
		case None: std::cout << "None"; break;
		case Nearest: std::cout << "Nearest"; break;
		case Linear: std::cout << "Linear"; break;
		}
		std::cout << std::endl;

		Tabs(); std::cout << "\t" << "minFilter = ";
		switch (Smp.minFilter)
		{
			using enum fx::gltf::Sampler::MinFilter;
		case None: std::cout << "None"; break;
		case Nearest: std::cout << "Nearest"; break;
		case Linear: std::cout << "Linear"; break;
		case NearestMipMapNearest: std::cout << "NearestMipMapNearest"; break;
		case LinearMipMapNearest: std::cout << "LinearMipMapNearest"; break;
		case NearestMipMapLinear: std::cout << "NearestMipMapLinear"; break;
		case LinearMipMapLinear: std::cout << "LinearMipMapLinear"; break;
		}
		std::cout << std::endl;

		Tabs(); std::cout << "\t" << "wrapS = ";
		switch (Smp.wrapS)
		{
			using enum fx::gltf::Sampler::WrappingMode;
		case ClampToEdge: std::cout << "ClampToEdge"; break;
		case MirroredRepeat: std::cout << "MirroredRepeat"; break;
		case Repeat: std::cout << "Repeat"; break;
		}
		std::cout << std::endl;

		Tabs(); std::cout << "\t" << "wrapT = ";
		switch (Smp.wrapT)
		{
			using enum fx::gltf::Sampler::WrappingMode;
		case ClampToEdge: std::cout << "ClampToEdge"; break;
		case MirroredRepeat: std::cout << "MirroredRepeat"; break;
		case Repeat: std::cout << "Repeat"; break;
		}
		std::cout << std::endl;
	}
	virtual void Process(const fx::gltf::Image& Img) {
		Tabs(); std::cout << "Image : " << Img.name << std::endl;

		Tabs(); std::cout << "\t" << "mimeType = " << Img.mimeType << std::endl;
		if (empty(Img.uri)) {
			if (Img.IsEmbeddedResource()) {
				Tabs(); std::cout << "\t" << "IsEmbeddedResource = " << Img.IsEmbeddedResource() << std::endl;

				Tabs(); std::cout << "\t" << "MaterializeData" << std::endl;
				std::vector<uint8_t> Data;
				Img.MaterializeData(Data);

				//const auto DataPtr = data(Data);
				//const auto DataSize = static_cast<uint32_t>(size(Data));
			}
			else {
				const auto& Doc = GetDocument();

				PushTab();
				const auto& BufV = Doc.bufferViews[Img.bufferView];
				Tabs(); std::cout << "BufferView : " << BufV.name << std::endl;
				Tabs(); std::cout << "\t" << "byteOffset = " << BufV.byteOffset << std::endl;
				Tabs(); std::cout << "\t" << "byteLength = " << BufV.byteLength << std::endl;

				PushTab();
				if (-1 != BufV.buffer) {
					const auto& Buf = Doc.buffers[BufV.buffer];
					Tabs(); std::cout << "Buffer : " << Buf.name << std::endl;
					Tabs(); std::cout << "\t" << "ByteLength = " << Buf.byteLength << std::endl;
				}
				PopTab();

				//const auto DataPtr = &Doc.buffers[BufV.buffer].data[BufV.byteOffset + 0/*ここではアクセサは無い*/];
				//const auto DataSize = BufV.byteLength;
				PopTab();
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

		PopTab();
	}

	virtual std::array<float, 3> Lerp(const std::array<float, 3>& lhs, const std::array<float, 3>& rhs, const float t) = 0;
	virtual std::array<float, 4> Lerp(const std::array<float, 4>& lhs, const std::array<float, 4>& rhs, const float t) = 0;

	virtual void UpdateAnimTranslation(const std::array<float, 3>& /*Value*/, const uint32_t /*NodeIndex*/) {}
	virtual void UpdateAnimScale(const std::array<float, 3>& /*Value*/, const uint32_t /*NodeIndex*/) {}
	virtual void UpdateAnimRotation(const std::array<float, 4>& /*Value*/, const uint32_t /*NodeIndex*/) {}
	virtual void UpdateAnimWeights(const float* /*Data*/, const uint32_t /*PrevIndex*/, const uint32_t /*NextIndex*/, const float /*t*/) {}
	virtual void UpdateAnimation(float CurrentFrame, bool bIsLoop = true) {
		const auto& Doc = GetDocument();
		for (const auto& i : Doc.animations) {
			for (const auto& j : i.channels) {
				if (-1 != j.sampler) {
					const auto& Smp = i.samplers[j.sampler];
					if (-1 != Smp.input && -1 != Smp.output) {
						const auto& InAcc = Doc.accessors[Smp.input];
						if (InAcc.type == fx::gltf::Accessor::Type::Scalar && InAcc.componentType == fx::gltf::Accessor::ComponentType::Float) {
							const auto Keyframes = reinterpret_cast<const float*>(GetData(InAcc));
							const auto MaxFrame = Keyframes[InAcc.count - 1];

							//!< アニメーションのクランプ or ループ
							if (bIsLoop) {
								while (CurrentFrame > MaxFrame) { CurrentFrame -= MaxFrame; } //!< Loop
							}
							else {
								CurrentFrame = (std::min)(CurrentFrame, MaxFrame); //!< Clamp
							}

							//!< 現在のフレームが含まれるキーフレーム範囲と、補完値tを求める
							uint32_t PrevIndex = 0, NextIndex = 0;
							for (uint32_t k = 0; k < InAcc.count; ++k) {
								if (Keyframes[k] >= CurrentFrame) {
									NextIndex = k;
									PrevIndex = NextIndex > 1 ? NextIndex - 1 : 0;
									break;
								}
							}
							const auto PrevFrame = Keyframes[PrevIndex];
							const auto NextFrame = Keyframes[NextIndex];
							std::cout << "Frame = " << CurrentFrame << " [" << PrevFrame << ", " << NextFrame << "] / " << MaxFrame << std::endl;
							const auto Delta = NextFrame - PrevFrame;

							const auto t = std::abs(Delta) <= (std::numeric_limits<float>::epsilon)() ? 0.0f : (CurrentFrame - PrevFrame) / Delta;
							const auto invt = 1.0f - t;
							std::cout << "t = " << t << std::endl;

							//!< 補完(Animation::Sampler::Type)、解釈(path:translation, sacle, rotation...)方法による処理の分岐
							const auto& OutAcc = Doc.accessors[Smp.output];
							std::cout << "\t" << j.target.path << " = ";
							switch (Smp.interpolation)
							{
								using enum fx::gltf::Animation::Sampler::Type;
							case Linear:
								if ("translation" == j.target.path) {
									const auto Data = reinterpret_cast<const std::array<float, 3>*>(GetData(OutAcc));
									//!< std::lerp() は C++20以降
									UpdateAnimTranslation(Lerp(Data[PrevIndex], Data[NextIndex], t), j.target.node);
								}
								else if ("scale" == j.target.path) {
									const auto Data = reinterpret_cast<const std::array<float, 3>*>(GetData(OutAcc));
									UpdateAnimScale(Lerp(Data[PrevIndex], Data[NextIndex], t), j.target.node);
								}
								else if ("rotation" == j.target.path) {
									const auto Data = reinterpret_cast<const std::array<float, 4>*>(GetData(OutAcc));
									UpdateAnimRotation(Lerp(Data[PrevIndex], Data[NextIndex], t), j.target.node);
								}
								else if ("weights" == j.target.path) {
									const auto Data = reinterpret_cast<const float*>(GetData(OutAcc));
									UpdateAnimWeights(Data, PrevIndex, NextIndex, t);
								}
								break;
							case Step:
								if ("translation" == j.target.path || "scale" == j.target.path) {
									const auto Data = reinterpret_cast<const std::array<float, 3>*>(GetData(OutAcc));
									UpdateAnimTranslation(Data[PrevIndex], j.target.node);
								}
								else if ("translation" == j.target.path || "scale" == j.target.path) {
									const auto Data = reinterpret_cast<const std::array<float, 3>*>(GetData(OutAcc));
									UpdateAnimScale(Data[PrevIndex], j.target.node);
								}
								else if ("rotation" == j.target.path) {
									const auto Data = reinterpret_cast<const std::array<float, 4>*>(GetData(OutAcc));
									UpdateAnimRotation(Data[PrevIndex], j.target.node);
								}
								else if ("weights" == j.target.path) {
									const auto Data = reinterpret_cast<const float*>(GetData(OutAcc));
									UpdateAnimWeights(Data, PrevIndex, (std::numeric_limits<uint32_t>::max)(), 0.0f);
								}
								break;
							case CubicSpline:
								//!< https://github.com/KhronosGroup/glTF/tree/master/specification/2.0#appendix-c-spline-interpolation
								//!< p(t) = (2 * t^3 - 3 * t^2 + 1) * p0 + (t^3 - 2 * t^2 + t) * m0 + (-2 * t^3 + 3 * t^2) * p1 + (t^3 - t^2) * m1
								//!< ここで 0<t<1、p0は開始位置(t=0)、m0は開始接線(t=0)、p1は終了位置(t=1)、m1は終了接線(t=1)
								//!< CubicSpline の場合、(InTangent, Value, OutTangent)の3つでセットになっているのでストライドは3になる
								const auto Stride = 3; //!< 0:InTangent, 1:Value, 2:OutTangent
								const auto PrevSet = PrevIndex * Stride;
								const auto NextSet = NextIndex * Stride;
								const auto t2 = t * t;
								const auto t3 = t2 * t;
								if ("translation" == j.target.path) {
									const auto Data = reinterpret_cast<const std::array<float, 3>*>(GetData(OutAcc));
									//!< Prev InTanget は使用しない
									const auto& p0 = Data[PrevSet + 1]; //!< Prev Value
									const auto& m0 = Data[PrevSet + 2]; //!< Prev OutTangent
									const auto& m1 = Data[NextSet + 0]; //!< Next InTangent
									const auto& p1 = Data[NextSet + 1]; //!< Next Value
									//!< Next OutTangent は使用しない
									UpdateAnimTranslation((2.0f * t3 - 3.0f * t2 + 1.0f) * p0 + (t3 - 2.0f * t2 + t) * Delta * m0 + (-2.0f * t3 + 3.0f * t2) * p1 + (t3 - t2) * Delta * m1, j.target.node);
								}
								else if ("scale" == j.target.path) {
									const auto Data = reinterpret_cast<const std::array<float, 3>*>(GetData(OutAcc));
									const auto& p0 = Data[PrevSet + 1];
									const auto& m0 = Data[PrevSet + 2];
									const auto& m1 = Data[NextSet + 0];
									const auto& p1 = Data[NextSet + 1];
									UpdateAnimScale((2.0f * t3 - 3.0f * t2 + 1.0f) * p0 + (t3 - 2.0f * t2 + t) * Delta * m0 + (-2.0f * t3 + 3.0f * t2) * p1 + (t3 - t2) * Delta * m1, j.target.node);
								}
								else if ("rotation" == j.target.path) {
									const auto Data = reinterpret_cast<const std::array<float, 4>*>(GetData(OutAcc));
									const auto& p0 = Data[PrevSet + 1];
									const auto& m0 = Data[PrevSet + 2];
									const auto& m1 = Data[NextSet + 0];
									const auto& p1 = Data[NextSet + 1];
									//!< クォータニオンは使用前に正規化すること
									UpdateAnimRotation((2.0f * t3 - 3.0f * t2 + 1.0f) * p0 + (t3 - 2.0f * t2 + t) * Delta * m0 + (-2.0f * t3 + 3.0f * t2) * p1 + (t3 - t2) * Delta * m1, j.target.node);
								}
								//else if ("weights" == j.target.path) {
								//	const auto Data = reinterpret_cast<const float*>(GetData(OutAcc));
								//	UpdateAnimWeights(Data, PrevIndex, NextIndex, t);
								//}
								break;
							}

							if (-1 != j.target.node) {
								auto& Nd = Doc.nodes[j.target.node];
								Nd.translation;
								Nd.rotation;
								Nd.scale;
							}
						}
					}
				}
			}
		}
	}

#ifdef _DEBUG
	void Tabs() { for (auto i = 0; i < TabDepth; ++i) { std::cout << "\t"; } }
	void PushTab() { ++TabDepth; }
	void PopTab() { --TabDepth; }
#else
	void Tabs() {}
	void PushTab() {}
	void PopTab() {}
#endif

	const fx::gltf::Document& GetDocument() const { return Documents.back(); }
protected:
	std::vector<fx::gltf::Document> Documents;
#ifdef _DEBUG
	uint8_t TabDepth = 0;
#endif
};