#pragma once

#include <GLTFSDK/GLTF.h>
#include <GLTFSDK/GLTFResourceReader.h>
#include <GLTFSDK/GLBResourceReader.h>
#include <GLTFSDK/Deserialize.h>

#include "Hierarchy.h"

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
		virtual void Process(const Microsoft::glTF::Image& Image) {
			Tabs(); std::cout << "Image [" << Image.id << "] : " << Image.name << std::endl;

			Tabs(); std::cout << "\tUri = " << Image.uri.substr(0, 32) << " ..." << std::endl;
			Tabs(); std::cout << "\tMimeType = " << Image.mimeType << " ..." << std::endl;
			
			if (Document.bufferViews.Has(Image.bufferViewId)) {
				PushTab();
				Process(Document.bufferViews.Get(Image.bufferViewId));
				PopTab();
			}

			const auto Data = ResourceReader->ReadBinaryData(Document, Image);
		}
		virtual void Process(const Microsoft::glTF::Material& Material) {
			Tabs(); std::cout << "Material [" << Material.id << "] : " << Material.name << std::endl;

			Material.metallicRoughness.baseColorFactor;
			Material.metallicRoughness.baseColorTexture.textureId;
			Material.metallicRoughness.baseColorTexture.texCoord;
			Material.metallicRoughness.metallicFactor;
			Material.metallicRoughness.roughnessFactor;
			Material.metallicRoughness.metallicRoughnessTexture.textureId;
			Material.metallicRoughness.metallicRoughnessTexture.texCoord;

			Material.normalTexture.scale;
			
			Material.occlusionTexture.strength;
			
			Material.emissiveTexture.textureId;
			Material.emissiveTexture.texCoord;

			Material.emissiveFactor;
			switch (Material.alphaMode) {
			case Microsoft::glTF::AlphaMode::ALPHA_UNKNOWN: break;
			case Microsoft::glTF::AlphaMode::ALPHA_OPAQUE: 
				Microsoft::glTF::ALPHAMODE_NAME_OPAQUE;
				break;
			case Microsoft::glTF::AlphaMode::ALPHA_BLEND: 
				Microsoft::glTF::ALPHAMODE_NAME_BLEND;
				break;
			case Microsoft::glTF::AlphaMode::ALPHA_MASK: 
				Microsoft::glTF::ALPHAMODE_NAME_MASK;
				break;
			}

			Material.alphaCutoff;
			Material.doubleSided;

			const auto Texures = Material.GetTextures();
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
		virtual void Process(const Microsoft::glTF::Accessor& Accessor) {
			Tabs(); std::cout << "Accessor [" << Accessor.id << "] : " << Accessor.name << std::endl;

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
			//!< #TODO
		}
		virtual void Process(const Microsoft::glTF::Skin& Skin) {
			Tabs(); std::cout << "Skin [" << Skin.id << "] : " << Skin.name << std::endl;
			//!< #TODO
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
						{
							const auto Data = ResourceReader->ReadBinaryData<uint16_t>(Document, Accessor);
							assert(Accessor.count == size(Data) && "");
						}
						break;
					default: break;
					}
					break;
				case Microsoft::glTF::ComponentType::COMPONENT_UNSIGNED_INT:
					switch (Accessor.type)
					{
					case Microsoft::glTF::AccessorType::TYPE_SCALAR: 
						{
							const auto Data = ResourceReader->ReadBinaryData<uint32_t>(Document, Accessor);
							assert(Accessor.count == size(Data) && "");
						}
						break;
					default: break;
					}
					break;
				default: break;
				}

				if (Document.bufferViews.Has(Accessor.bufferViewId)) {
					assert(Microsoft::glTF::BufferViewTarget::ELEMENT_ARRAY_BUFFER == Document.bufferViews.Get(Accessor.bufferViewId).target && "");
				}
			}
			//!< バーテックス
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

			if (size(Primitive.attributes)) {
				Tabs(); std::cout << "\tAttributes" << std::endl;
				for (const auto& i : Primitive.attributes) {
					Tabs(); std::cout << "\t\t<" << i.first << ", " << i.second << ">" << std::endl;
				}
			}

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

			PushTab();
			for (const auto& i : Mesh.primitives) {
				Process(i);
			}
			PopTab();
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
				Process(Document.cameras.Get(Node.cameraId)); //!< #TODO
				PopTab();
			}
			if (Document.skins.Has(Node.skinId)) {
				PushTab();
				Process(Document.skins.Get(Node.skinId)); //!< #TODO
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

#if false
			//!< 各種リストアップ
			for (const auto& i : Document.images.Elements()) {
				Process(i);
			}
			for (const auto& i : Document.materials.Elements()) {
				Process(i);
			}
			for (const auto& i : Document.meshes.Elements()) {
				Process(i);
			}
			for (const auto& i : Document.nodes.Elements()) {
				Process(i);
			}
#endif
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
