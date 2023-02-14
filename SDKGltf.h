#pragma once

#include <GLTFSDK/GLTF.h>
#include <GLTFSDK/GLTFResourceReader.h>
#include <GLTFSDK/GLBResourceReader.h>
#include <GLTFSDK/Deserialize.h>

#include "Hierarchy.h"

static std::ostream& operator<<(std::ostream& lhs, const Microsoft::glTF::Color3& rhs) { lhs << rhs.r << ", " << rhs.g << ", " << rhs.b; return lhs; }
static std::ostream& operator<<(std::ostream& lhs, const Microsoft::glTF::Color4& rhs) { lhs << rhs.r << ", " << rhs.g << ", " << rhs.b << ", " << rhs.a; return lhs; }

namespace Gltf {
    class StreamReader : public Microsoft::glTF::IStreamReader
    {
    public:
        virtual std::shared_ptr<std::istream> GetInputStream(const std::string& FilePath) const override {
            return std::make_shared<std::ifstream>(FilePath, std::ios_base::binary);
        }
    private:
    };
	class SDK : public Hierarchy
	{
	private:
		using Super = Hierarchy;
	
	protected:
		std::unique_ptr<Microsoft::glTF::GLTFResourceReader> ResourceReader;
		Microsoft::glTF::Document Document;

	public:
		virtual void Process(const Microsoft::glTF::Sampler& Sampler) {
			Tabs(); std::cout << "Sampler [" << Sampler.id << "] : " << Sampler.name << std::endl;

			Tabs(); std::cout << "\tMagFilter = ";
			switch (Sampler.magFilter)
			{
			case Microsoft::glTF::MagFilterMode::MagFilter_NEAREST:
				std::cout << "NEAREST";
				break;
			case Microsoft::glTF::MagFilterMode::MagFilter_LINEAR:
				std::cout << "LINEAR";
				break;
			default: break;
			}
			std::cout << std::endl;

			Tabs(); std::cout << "\tMinFilter = ";
			switch (Sampler.minFilter)
			{
			case Microsoft::glTF::MinFilterMode::MinFilter_NEAREST:
				std::cout << "NEAREST";
				break;
			case Microsoft::glTF::MinFilterMode::MinFilter_LINEAR:
				std::cout << "LINEAR";
				break;
			case Microsoft::glTF::MinFilterMode::MinFilter_NEAREST_MIPMAP_NEAREST:
				std::cout << "NEAREST_MIPMAP_NEAREST";
				break;
			case Microsoft::glTF::MinFilterMode::MinFilter_LINEAR_MIPMAP_NEAREST:
				std::cout << "LINEAR_MIPMAP_NEAREST";
				break;
			case Microsoft::glTF::MinFilterMode::MinFilter_NEAREST_MIPMAP_LINEAR:
				std::cout << "NEAREST_MIPMAP_LINEAR";
				break;
			case Microsoft::glTF::MinFilterMode::MinFilter_LINEAR_MIPMAP_LINEAR:
				std::cout << "LINEAR_MIPMAP_LINEAR";
				break;
			default: break;
			}
			std::cout << std::endl;

			Tabs(); std::cout << "\tWrapS = ";
			switch (Sampler.wrapS)
			{
			case Microsoft::glTF::WrapMode::Wrap_REPEAT:
				std::cout << "REPEAT";
				break;
			case Microsoft::glTF::WrapMode::Wrap_CLAMP_TO_EDGE:
				std::cout << "CLAMP_TO_EDGE";
				break;
			case Microsoft::glTF::WrapMode::Wrap_MIRRORED_REPEAT:
				std::cout << "MIRRORED_REPEAT";
				break;
			default: break;
			}
			std::cout << std::endl;

			Tabs(); std::cout << "\tWrapT = ";
			switch (Sampler.wrapT)
			{
			case Microsoft::glTF::WrapMode::Wrap_REPEAT:
				std::cout << "REPEAT";
				break;
			case Microsoft::glTF::WrapMode::Wrap_CLAMP_TO_EDGE:
				std::cout << "CLAMP_TO_EDGE";
				break;
			case Microsoft::glTF::WrapMode::Wrap_MIRRORED_REPEAT:
				std::cout << "MIRRORED_REPEAT";
				break;
			default: break;
			}
			std::cout << std::endl;
		}
		virtual void Process(const Microsoft::glTF::Image& Image) {
			Tabs(); std::cout << "Image [" << Image.id << "] : " << Image.name << std::endl;

			if (empty(Image.uri)) {
				if (Document.bufferViews.Has(Image.bufferViewId)) {
					const auto& BufferView = Document.bufferViews.Get(Image.bufferViewId);
					if (Document.buffers.Has(BufferView.bufferId)) {
						const auto& Buffer = Document.buffers.Get(BufferView.bufferId);
						Tabs(); std::cout << "\tUri = " << Buffer.uri.substr(0, 32) << " ..." << std::endl;
					}
				}
			}
			else {
				Tabs(); std::cout << "\tUri = " << Image.uri.substr(0, 32) << " ..." << std::endl;
			}

			Tabs(); std::cout << "\tMimeType = " << Image.mimeType << " ..." << std::endl;
			
			if (Document.bufferViews.Has(Image.bufferViewId)) {
				PushTab();
				Process(Document.bufferViews.Get(Image.bufferViewId));
				PopTab();
			}

			//const auto Data = ResourceReader->ReadBinaryData(Document, Image);
		}
		virtual void Process(const Microsoft::glTF::Texture& Texture, std::string_view name = "") {
			Tabs(); std::cout << "Texture [" << Texture.id << "] : " << (empty(Texture.name) ? name : Texture.name) << std::endl;

			if (Document.samplers.Has(Texture.samplerId)) {
				PushTab();
				Process(Document.samplers.Get(Texture.samplerId));
				PopTab();
			}

			if (Document.images.Has(Texture.imageId)) {
				PushTab();
				Process(Document.images.Get(Texture.imageId));
				PopTab();
			}
		}
		virtual void Process(const Microsoft::glTF::Material::PBRMetallicRoughness& MetallicRoughness) {
			Tabs(); std::cout << "MetallicRoughness" << std::endl;

			Tabs(); std::cout << "\tBaseColorFactor = " << MetallicRoughness.baseColorFactor << std::endl;
			Tabs(); std::cout << "\tMetallicFactor = " << MetallicRoughness.metallicFactor << std::endl;;
			Tabs(); std::cout << "\tRoughnessFactor = " << MetallicRoughness.roughnessFactor << std::endl;

			if (Document.textures.Has(MetallicRoughness.baseColorTexture.textureId)) {
				PushTab();
				Process(Document.textures.Get(MetallicRoughness.baseColorTexture.textureId), "BaseColorTexture");
				Tabs(); std::cout << "\tTexCoord = " << MetallicRoughness.baseColorTexture.texCoord << std::endl;
				PopTab();
			}
			if (Document.textures.Has(MetallicRoughness.metallicRoughnessTexture.textureId)) {
				PushTab();
				Process(Document.textures.Get(MetallicRoughness.metallicRoughnessTexture.textureId), "MetallicRoughnessTexture");
				Tabs(); std::cout << "\tTexCoord = " << MetallicRoughness.metallicRoughnessTexture.texCoord << std::endl;
				PopTab();
			}
		}
		virtual void Process(const Microsoft::glTF::Material& Material) {
			Tabs(); std::cout << "Material [" << Material.id << "] : " << Material.name << std::endl;

			PushTab();
			Process(Material.metallicRoughness);
			PopTab();

			if (Document.textures.Has(Material.normalTexture.textureId)) {
				PushTab();
				Process(Document.textures.Get(Material.normalTexture.textureId), "NormalTexture");
				Tabs(); std::cout << "\tTexCoord = " << Material.normalTexture.texCoord << std::endl;
				Tabs(); std::cout << "\tScale = " << Material.normalTexture.scale << std::endl;
				PopTab();
			}

			if (Document.textures.Has(Material.occlusionTexture.textureId)) {
				PushTab();
				Process(Document.textures.Get(Material.occlusionTexture.textureId), "OcclusionTexture");
				Tabs(); std::cout << "\tTexCoord = " << Material.occlusionTexture.texCoord << std::endl;
				Tabs(); std::cout << "\tStrength = " << Material.occlusionTexture.strength << std::endl;
				PopTab();
			}

			if (Document.textures.Has(Material.emissiveTexture.textureId)) {
				PushTab();
				Process(Document.textures.Get(Material.emissiveTexture.textureId), "EmissiveTexture");
				Tabs(); std::cout << "\tTexCoord = " << Material.emissiveTexture.texCoord << std::endl;
				PopTab();
			}

			Tabs(); std::cout << "\tEmissiveFactor = " << Material.emissiveFactor << std::endl;
			Tabs(); std::cout << "\tAlphaCutoff = " << Material.alphaCutoff << std::endl;
			Tabs(); std::cout << "\tDoubleSided = " << (Material.doubleSided ? "true" : "false") << std::endl;
			Tabs(); std::cout << "\tAlphaMode = ";
			switch (Material.alphaMode) {
			case Microsoft::glTF::AlphaMode::ALPHA_UNKNOWN:
				std::cout << "UNKNOWN"; 
				break;
			case Microsoft::glTF::AlphaMode::ALPHA_OPAQUE: 
				std::cout << Microsoft::glTF::ALPHAMODE_NAME_OPAQUE; 
				break;
			case Microsoft::glTF::AlphaMode::ALPHA_BLEND:
				std::cout << Microsoft::glTF::ALPHAMODE_NAME_BLEND;
				break;
			case Microsoft::glTF::AlphaMode::ALPHA_MASK:
				std::cout << Microsoft::glTF::ALPHAMODE_NAME_MASK;
				break;
			}
			std::cout << std::endl;

			//!< マテリアル中のテクスチャをなめる場合に便利
			for (const auto& i : Material.GetTextures()) {
				switch (i.second)
				{
				case Microsoft::glTF::TextureType::BaseColor: break;
				case Microsoft::glTF::TextureType::MetallicRoughness: break;
				case Microsoft::glTF::TextureType::Normal: break;
				case Microsoft::glTF::TextureType::Occlusion: break;
				case Microsoft::glTF::TextureType::Emissive: break;
				default: break;
				}
			}
		}
		virtual void Process(const Microsoft::glTF::Buffer& Buffer) {
			Tabs(); std::cout << "Buffer [" << Buffer.id << "] : " << Buffer.name << std::endl;

			Tabs(); std::cout << "\tByteLength = " << Buffer.byteLength << std::endl;
			Tabs(); std::cout << "\tUri = " << Buffer.uri.substr(0, 32) << " ..." << std::endl;
		}
		virtual void Process(const Microsoft::glTF::BufferView& BufferView) {
			Tabs(); std::cout << "BufferView [" << BufferView.id << "] : " << BufferView.name << std::endl;

			Tabs(); std::cout << "\tBufferViewTarget = ";
			switch (BufferView.target)
			{
			case Microsoft::glTF::BufferViewTarget::UNKNOWN_BUFFER:
				std::cout << "UNKNOWN_BUFFER";
				break;
			case Microsoft::glTF::BufferViewTarget::ARRAY_BUFFER: 
				std::cout << "ARRAY_BUFFER";
				break;
			case Microsoft::glTF::BufferViewTarget::ELEMENT_ARRAY_BUFFER: 
				std::cout << "ELEMENT_ARRAY_BUFFER";
				break;
			default:
				break;
			}
			std::cout << std::endl;

			Tabs(); std::cout << "\tByteOffset = " << BufferView.byteOffset << std::endl;
			Tabs(); std::cout << "\tByteLength = " << BufferView.byteLength << std::endl;
			Tabs(); std::cout << "\tByteStride = " << BufferView.byteStride << std::endl;

			if (Document.buffers.Has(BufferView.bufferId)) {
				PushTab();
				Process(Document.buffers.Get(BufferView.bufferId));
				PopTab();
			}
		}
		virtual void Process(const Microsoft::glTF::Accessor& Accessor, std::string_view name = "") {
			Tabs(); std::cout << "Accessor [" << Accessor.id << "] : " << (empty(Accessor.name) ? name : Accessor.name) << std::endl;

			Tabs(); std::cout << "\tComponentType = " << Microsoft::glTF::Accessor::GetComponentTypeName(Accessor.componentType) << std::endl;
			switch (Accessor.componentType)
			{
			case Microsoft::glTF::ComponentType::COMPONENT_UNKNOWN: break;
			case Microsoft::glTF::ComponentType::COMPONENT_BYTE:break;
			case Microsoft::glTF::ComponentType::COMPONENT_UNSIGNED_BYTE:break;
			case Microsoft::glTF::ComponentType::COMPONENT_SHORT:break;
			case Microsoft::glTF::ComponentType::COMPONENT_UNSIGNED_SHORT:break;
			case Microsoft::glTF::ComponentType::COMPONENT_UNSIGNED_INT:break;
			case Microsoft::glTF::ComponentType::COMPONENT_FLOAT:break;
			default: break;
			}

			Tabs(); std::cout << "\tAccessorType = " << Microsoft::glTF::Accessor::GetAccessorTypeName(Accessor.type) << std::endl;
			switch (Accessor.type)
			{
			case Microsoft::glTF::AccessorType::TYPE_UNKNOWN: break;
			case Microsoft::glTF::AccessorType::TYPE_SCALAR: break;
			case Microsoft::glTF::AccessorType::TYPE_VEC2: break;
			case Microsoft::glTF::AccessorType::TYPE_VEC3: break;
			case Microsoft::glTF::AccessorType::TYPE_VEC4: break;
			case Microsoft::glTF::AccessorType::TYPE_MAT2: break;
			case Microsoft::glTF::AccessorType::TYPE_MAT3: break;
			case Microsoft::glTF::AccessorType::TYPE_MAT4: break;
			default: break;
			}

			//!< COMPONENT_FLOAT, TYPE_VEC3 の場合なら float3 の個数
			Tabs(); std::cout << "\tCount = " << Accessor.count << std::endl;
			Tabs(); std::cout << "\tByteOffset = " << Accessor.byteOffset << std::endl;
			Tabs(); std::cout << "\tNormalized = " << (Accessor.normalized ? "true" : "false") << std::endl;

			if (Document.bufferViews.Has(Accessor.bufferViewId)) {
				PushTab();
				Process(Document.bufferViews.Get(Accessor.bufferViewId));
				PopTab();
			}
		}
		virtual void Process(const Microsoft::glTF::Camera& Camera) {
			Tabs(); std::cout << "Camera [" << Camera.id << "] : " << Camera.name << std::endl;

			switch (Camera.projection->GetProjectionType()) 
			{
			case Microsoft::glTF::ProjectionType::PERSPECTIVE:
				{
					const auto& Perspective = Camera.GetPerspective();
					Tabs(); std::cout << "\tZFar = " << Perspective.zfar << std::endl;
					Tabs(); std::cout << "\tZNear = " << Perspective.znear << std::endl;
					Tabs(); std::cout << "\tAspectRatio = " << Perspective.aspectRatio << std::endl;
					Tabs(); std::cout << "\tYFov = " << Perspective.yfov << std::endl;
			}
				break;
			case Microsoft::glTF::ProjectionType::ORTHOGRAPHIC:
				{
					const auto& Orthographic =  Camera.GetOrthographic();
					Tabs(); std::cout << "\tZFar = " << Orthographic.zfar << std::endl;
					Tabs(); std::cout << "\tZNear = " << Orthographic.znear << std::endl;
					Tabs(); std::cout << "\tXMag = " << Orthographic.xmag << std::endl;
					Tabs(); std::cout << "\tYMag = " << Orthographic.ymag << std::endl;
				}
				break;
			default: break;
			} 
		}
		virtual void Process(const Microsoft::glTF::Skin& Skin) {
			Tabs(); std::cout << "Skin [" << Skin.id << "] : " << Skin.name << std::endl;
			
			if (Document.accessors.Has(Skin.inverseBindMatricesAccessorId)) {
				PushTab();
				Process(Document.accessors.Get(Skin.inverseBindMatricesAccessorId));
				PopTab();
			}

			Tabs(); std::cout << "SkeletonId = " << Skin.skeletonId << std::endl;

			Tabs(); std::cout << "jointIds = ";
			for (const auto& i : Skin.jointIds) {
				std::cout << i << ", ";
			}
			std::cout << std::endl;
		}
		virtual void Process(const Microsoft::glTF::MeshPrimitive& Primitive) {
			Tabs(); std::cout << "\tMeshMode = ";
			switch (Primitive.mode)
			{
			case Microsoft::glTF::MeshMode::MESH_POINTS:
				std::cout << "MESH_POINTS";
				break;
			case Microsoft::glTF::MeshMode::MESH_LINES:
				std::cout << "MESH_LINES";
				break;
			case Microsoft::glTF::MeshMode::MESH_LINE_LOOP:
				std::cout << "MESH_LINE_LOOP";
				break;
			case Microsoft::glTF::MeshMode::MESH_LINE_STRIP:
				std::cout << "MESH_LINE_STRIP";
				break;
			case Microsoft::glTF::MeshMode::MESH_TRIANGLES:
				std::cout << "MESH_TRIANGLES";
				break;
			case Microsoft::glTF::MeshMode::MESH_TRIANGLE_STRIP:
				std::cout << "MESH_TRIANGLE_STRIP";
				break;
			case Microsoft::glTF::MeshMode::MESH_TRIANGLE_FAN:
				std::cout << "MESH_TRIANGLE_FAN";
				break;
			default:
				break;
			}
			std::cout << std::endl;

			//!< インデックス
			if(Document.accessors.Has(Primitive.indicesAccessorId)) {
				const auto& Accessor = Document.accessors.Get(Primitive.indicesAccessorId);
				PushTab();
				Process(Accessor);
				PopTab();

				switch (Accessor.componentType)
				{
				case Microsoft::glTF::ComponentType::COMPONENT_UNSIGNED_SHORT:
					switch (Accessor.type)
					{
					case Microsoft::glTF::AccessorType::TYPE_SCALAR:
						//!< 埋め込まれていないとこの取り方はできない？
						//{
						//	const auto Data = ResourceReader->ReadBinaryData<uint16_t>(Document, Accessor);
						//	assert(Accessor.count == size(Data) && "");
						//}
						break;
					default: break;
					}
					break;
				case Microsoft::glTF::ComponentType::COMPONENT_UNSIGNED_INT:
					switch (Accessor.type)
					{
					case Microsoft::glTF::AccessorType::TYPE_SCALAR: 
						//!< 埋め込まれていないとこの取り方はできない？
						//{
						//	const auto Data = ResourceReader->ReadBinaryData<uint32_t>(Document, Accessor);
						//	assert(Accessor.count == size(Data) && "");
						//}
						break;
					default: break;
					}
					break;
				default: break;
				}

				//!< UNKNOWN_BUFFER が返る事が普通にあるので assert しない
				//if (Document.bufferViews.Has(Accessor.bufferViewId)) { assert(Microsoft::glTF::BufferViewTarget::ELEMENT_ARRAY_BUFFER == Document.bufferViews.Get(Accessor.bufferViewId).target && ""); }
			}

			//!< バーテックス
#if true
			if (size(Primitive.attributes)) {
				for (const auto& i : Primitive.attributes) {
					PushTab();
					Process(Document.accessors.Get(i.second), i.first);
					PopTab();
				}
			}
#else
			{
				std::string AccessorId;
				if (Primitive.TryGetAttributeAccessorId(Microsoft::glTF::ACCESSOR_POSITION, AccessorId))
				{
					const auto& Accessor = Document.accessors.Get(AccessorId);
					PushTab();
					Process(Accessor);
					PopTab();

					switch (Accessor.componentType)
					{
					case Microsoft::glTF::ComponentType::COMPONENT_FLOAT:
						switch (Accessor.type)
						{
						case Microsoft::glTF::AccessorType::TYPE_VEC3: 
							{
								const auto Data = ResourceReader->ReadBinaryData<float>(Document, Accessor);
								assert(Accessor.count == size(Data) / 3 && "");
						}
							break;
						default: break;
						}
						break;
					default: break;
					}

					if (Document.bufferViews.Has(Accessor.bufferViewId)) {
						assert(Microsoft::glTF::BufferViewTarget::ARRAY_BUFFER == Document.bufferViews.Get(Accessor.bufferViewId).target && "");
					}
				}
				if (Primitive.TryGetAttributeAccessorId(Microsoft::glTF::ACCESSOR_NORMAL, AccessorId))
				{
					const auto& Accessor = Document.accessors.Get(AccessorId);
					PushTab();
					Process(Accessor);
					PopTab();
				}
				if (Primitive.TryGetAttributeAccessorId(Microsoft::glTF::ACCESSOR_TANGENT, AccessorId)) 
				{
					const auto& Accessor = Document.accessors.Get(AccessorId);
					PushTab();
					Process(Accessor);
					PopTab();
				}
				if (Primitive.TryGetAttributeAccessorId(Microsoft::glTF::ACCESSOR_TEXCOORD_0, AccessorId)) 
				{
					const auto& Accessor = Document.accessors.Get(AccessorId);
					PushTab();
					Process(Accessor);
					PopTab();
				}
				if (Primitive.TryGetAttributeAccessorId(Microsoft::glTF::ACCESSOR_TEXCOORD_1, AccessorId)) 
				{
					const auto& Accessor = Document.accessors.Get(AccessorId);
					PushTab();
					Process(Accessor);
					PopTab();
				}
				if (Primitive.TryGetAttributeAccessorId(Microsoft::glTF::ACCESSOR_COLOR_0, AccessorId))
				{
					const auto& Accessor = Document.accessors.Get(AccessorId);
					PushTab();
					Process(Accessor);
					PopTab();
				}
				if (Primitive.TryGetAttributeAccessorId(Microsoft::glTF::ACCESSOR_JOINTS_0, AccessorId)) 
				{
					const auto& Accessor = Document.accessors.Get(AccessorId);
					PushTab();
					Process(Accessor);
					PopTab();
				}
				if (Primitive.TryGetAttributeAccessorId(Microsoft::glTF::ACCESSOR_WEIGHTS_0, AccessorId)) 
				{
					const auto& Accessor = Document.accessors.Get(AccessorId);
					PushTab();
					Process(Accessor);
					PopTab();
				}
			}
#endif
			if (size(Primitive.targets)) {
				Tabs(); std::cout << "\tMorphTarget" << std::endl;
				for (const auto& i : Primitive.targets) {
					if (Document.accessors.Has(i.positionsAccessorId)) {
						const auto& Accessor = Document.accessors.Get(i.positionsAccessorId);
						Tabs(); std::cout << "\t\tPositions [" << Accessor.id << "] : " << Accessor.name << std::endl;
					}
					if (Document.accessors.Has(i.normalsAccessorId)) {
						const auto& Accessor = Document.accessors.Get(i.normalsAccessorId);
						Tabs(); std::cout << "\t\tNormals [" << Accessor.id << "] : " << Accessor.name << std::endl;
					}
					if (Document.accessors.Has(i.tangentsAccessorId)) {
						const auto& Accessor = Document.accessors.Get(i.tangentsAccessorId);
						Tabs(); std::cout << "\t\tTangents [" << Accessor.id << "] : " << Accessor.name << std::endl;
					}
				}
			}

			if (Document.materials.Has(Primitive.materialId)) {
				PushTab();
				Process(Document.materials.Get(Primitive.materialId));
				PopTab();
			}
		}
		virtual void Process(const Microsoft::glTF::Mesh& Mesh) {
			Tabs(); std::cout << "Mesh [" << Mesh.id << "] : " << Mesh.name << std::endl;

			if (!empty(Mesh.weights)) {
				Tabs(); std::cout << "\tWeights = (";
				for (auto i : Mesh.weights) {
					std::cout << i << ", ";
				}
				std::cout << ")" << std::endl;
			}

			for (const auto& i : Mesh.primitives) {
				PushTab();
				Process(i);
				PopTab();
			}
		}
		virtual void Process(const Microsoft::glTF::Node& Node) {
			Tabs(); std::cout << "Node [" << Node.id << "] : " << Node.name << std::endl;

			switch (Node.GetTransformationType()) {
			case Microsoft::glTF::TRANSFORMATION_IDENTITY:
				break;
			case Microsoft::glTF::TRANSFORMATION_MATRIX:
				Tabs(); std::cout << "\tM = (" << Node.matrix.values[ 0] << ", " << Node.matrix.values[ 1] << ", " << Node.matrix.values[ 2] << ", " << Node.matrix.values[ 3] << ")" << std::endl;
				Tabs(); std::cout << "\t    (" << Node.matrix.values[ 4] << ", " << Node.matrix.values[ 5] << ", " << Node.matrix.values[ 6] << ", " << Node.matrix.values[ 7] << ")" << std::endl;
				Tabs(); std::cout << "\t    (" << Node.matrix.values[ 8] << ", " << Node.matrix.values[ 9] << ", " << Node.matrix.values[10] << ", " << Node.matrix.values[11] << ")" << std::endl;
				Tabs(); std::cout << "\t    (" << Node.matrix.values[12] << ", " << Node.matrix.values[13] << ", " << Node.matrix.values[14] << ", " << Node.matrix.values[15] << ")" << std::endl;
				break;
			case Microsoft::glTF::TRANSFORMATION_TRS:
				Tabs(); std::cout << "\tT = (" << Node.translation.x << ", " << Node.translation.y << ", " << Node.translation.z << ")" << std::endl;
				Tabs(); std::cout << "\tR = (" << Node.rotation.x << ", " << Node.rotation.y << ", " << Node.rotation.z << ", " << Node.rotation.w << ")" << std::endl;
				Tabs(); std::cout << "\tS = (" << Node.scale.x << ", " << Node.scale.y << ", " << Node.scale.z << ")" << std::endl;
				break;
			default: break;
			} 

			if (Document.meshes.Has(Node.meshId)) {
				PushTab();
				Process(Document.meshes.Get(Node.meshId));
				PopTab();
			}
			if (Document.cameras.Has(Node.cameraId)) {
				PushTab();
				Process(Document.cameras.Get(Node.cameraId));
				PopTab();
			}
			if (Document.skins.Has(Node.skinId)) {
				PushTab();
				Process(Document.skins.Get(Node.skinId));
				PopTab();
			}

			if (!empty(Node.weights)) {
				Tabs(); std::cout << "\tWeights = (";
					for (auto i : Node.weights) {
						std::cout << i << ", ";
					}
				std::cout << ")" << std::endl;
			}

			PushTab();
			for (auto i : Node.children) {
				Process(Document.nodes.Get(i));
			}
			PopTab();
		}
		virtual void Process(const Microsoft::glTF::Scene& Scene) {
			Tabs(); std::cout << "Scene [" << Scene.id << "] : " << Scene.name << std::endl;

			PushTab();
			for (auto i : Scene.nodes) {
				Process(Document.nodes.Get(i));
			}
			PopTab();
		}
		virtual void Process(const Microsoft::glTF::AnimationSampler& Sampler) {
			Tabs(); std::cout << "AnimationSampler [" << Sampler.id << "]" << std::endl;

			Tabs(); std::cout << "\tInterpolation = ";
			switch (Sampler.interpolation)
			{
			case Microsoft::glTF::InterpolationType::INTERPOLATION_UNKNOWN:
				std::cout << "UNKNOWN";
				break;
			case Microsoft::glTF::InterpolationType::INTERPOLATION_LINEAR:
				std::cout << Microsoft::glTF::INTERPOLATIONTYPE_NAME_LINEAR;
				break;
			case Microsoft::glTF::InterpolationType::INTERPOLATION_STEP:
				std::cout << Microsoft::glTF::INTERPOLATIONTYPE_NAME_STEP;
				break;
			case Microsoft::glTF::InterpolationType::INTERPOLATION_CUBICSPLINE:
				std::cout << Microsoft::glTF::INTERPOLATIONTYPE_NAME_CUBICSPLINE;
				break;
			default: break;
			}
			std::cout << std::endl;

			if (Document.accessors.Has(Sampler.inputAccessorId)) {
				PushTab();
				Process(Document.accessors.Get(Sampler.inputAccessorId), "Input");
				PopTab();
			}

			if (Document.accessors.Has(Sampler.outputAccessorId)) {
				PushTab();
				Process(Document.accessors.Get(Sampler.outputAccessorId), "Output");
				PopTab();
			}
		}
		virtual void Process(const Microsoft::glTF::Animation& Animation, const Microsoft::glTF::AnimationChannel& Channel) {
			Tabs(); std::cout << "AnimationChannel [" << Channel.id << "]" << std::endl;

			Tabs(); std::cout << "\tTarget.NodeId = [" << Channel.target.nodeId << "]" << std::endl;
			Tabs(); std::cout << "\tTarget.Path = ";
			switch (Channel.target.path)
			{
			case Microsoft::glTF::TargetPath::TARGET_UNKNOWN:
				std::cout << "UNKNOWN";
				break;
			case Microsoft::glTF::TargetPath::TARGET_TRANSLATION:
				std::cout << Microsoft::glTF::TARGETPATH_NAME_TRANSLATION;
				break;
			case Microsoft::glTF::TargetPath::TARGET_ROTATION:
				std::cout << Microsoft::glTF::TARGETPATH_NAME_ROTATION;
				break;
			case Microsoft::glTF::TargetPath::TARGET_SCALE:
				std::cout << Microsoft::glTF::TARGETPATH_NAME_SCALE;
				break;
			case Microsoft::glTF::TargetPath::TARGET_WEIGHTS:
				std::cout << Microsoft::glTF::TARGETPATH_NAME_WEIGHTS;
				break;
			}
			std::cout << std::endl;

			if (Animation.samplers.Has(Channel.samplerId)) {
				PushTab();
				Process(Animation.samplers.Get(Channel.samplerId));
				PopTab();
			}
		}
		virtual void Process(const Microsoft::glTF::Animation& Animation) {
			Tabs(); std::cout << "Animation [" << Animation.id << "] : " << Animation.name << std::endl;

			for (auto i = 0; i < Animation.channels.Size(); ++i) {
				PushTab();
				Process(Animation, Animation.channels[i]);
				PopTab();
			}

			//for (auto i = 0; i < Animation.samplers.Size();++i) {}
		}
		virtual void ProcessScene() {
			if (Document.HasDefaultScene()) {
				//!< デフォルトシーンがある場合
				Process(Document.scenes.Get(Document.GetDefaultScene().id));
			}
			else
			{
				//!< デフォルトシーンが無い場合は全シーン
				for (const auto& i : Document.scenes.Elements()) {
					Process(i);
				}
			}
		}
		virtual void ProcessAnimation() {
			for (const auto& i : Document.animations.Elements()) {
				Process(i);
			}
		}
		virtual void Process() {
			std::cout << "Version:    " << Document.asset.version << std::endl;
			std::cout << "MinVersion: " << Document.asset.minVersion << std::endl;
			std::cout << "Generator:  " << Document.asset.generator << std::endl;
			std::cout << "Copyright:  " << Document.asset.copyright << std::endl;

			std::cout << "Scene Count: " << Document.scenes.Size() << std::endl;
			if (Document.scenes.Size() > 0U)
			{
				std::cout << "Default Scene Index: " << Document.GetDefaultScene().id << std::endl;
			}

			std::cout << "Node Count:     " << Document.nodes.Size() << std::endl;
			std::cout << "Camera Count:   " << Document.cameras.Size() << std::endl;
			std::cout << "Material Count: " << Document.materials.Size() << std::endl;

			std::cout << "Mesh Count: " << Document.meshes.Size() << std::endl;
			std::cout << "Skin Count: " << Document.skins.Size() << std::endl;

			std::cout << "Image Count:   " << Document.images.Size() << std::endl;
			std::cout << "Texture Count: " << Document.textures.Size() << std::endl;
			std::cout << "Sampler Count: " << Document.samplers.Size() << std::endl;

			std::cout << "Buffer Count:     " << Document.buffers.Size() << std::endl;
			std::cout << "BufferView Count: " << Document.bufferViews.Size() << std::endl;
			std::cout << "Accessor Count:   " << Document.accessors.Size() << std::endl;

			std::cout << "Animation Count: " << Document.animations.Size() << std::endl;

			//!< シーン
			ProcessScene();

			//!< アニメーション
			ProcessAnimation();
		}

		//!< リンカエラー 4099 が出る(#pragma では回避できない)ので以下のようにしている 
		//!< Configuration Properties - Linker - CommandLine - AdditionalOptions - /ignore:4099
		void Load(const std::filesystem::path& Path) {
			auto Reader = std::make_unique<StreamReader>();
			auto In = Reader->GetInputStream(Path.string());

			if (std::string(".") + Microsoft::glTF::GLTF_EXTENSION == Path.extension()) {
				auto GLTFReader = std::make_unique<Microsoft::glTF::GLTFResourceReader>(std::move(Reader));

				//!< std::stringstream を使用して、string へ GLTF ファイルを読み込む
				std::stringstream SS;
				SS << In->rdbuf();
				Document = Microsoft::glTF::Deserialize(SS.str());

				ResourceReader = std::move(GLTFReader);
			}
			if (std::string(".") + Microsoft::glTF::GLB_EXTENSION == Path.extension()) {
				auto GLBReader = std::make_unique<Microsoft::glTF::GLBResourceReader>(std::move(Reader), std::move(In));

				Document = Microsoft::glTF::Deserialize(GLBReader->GetJson());

				ResourceReader = std::move(GLBReader);
			}

			Process();
		}
	};
}
