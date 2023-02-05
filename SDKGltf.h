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
	public:
		//!< リンカエラー 4099 が出る(#pragma では回避できない)ので以下のようにしている 
		//!< Configuration Properties - Linker - CommandLine - AdditionalOptions - /ignore:4099
		virtual void Load(const std::filesystem::path& Path) {
			auto Ext = Path.extension();
			auto SReader = std::make_unique<StreamReader>();
			auto Stream = SReader->GetInputStream(Path.string());

			if (std::string(".") + Microsoft::glTF::GLTF_EXTENSION == Ext) {
				auto GLTFReader = std::make_unique<Microsoft::glTF::GLTFResourceReader>(std::move(SReader));

				//!< std::stringstream を使用して、string へ GLTF ファイルを読み込む
				std::stringstream SStream;
				SStream << Stream->rdbuf();
				auto ManifestStr = SStream.str();

				auto Document = Microsoft::glTF::Deserialize(ManifestStr);
			}
			if (std::string(".") + Microsoft::glTF::GLB_EXTENSION == Ext) {
				auto GLBReader = std::make_unique<Microsoft::glTF::GLBResourceReader>(std::move(SReader), std::move(Stream));

				auto ManifestStr = GLBReader->GetJson();

				auto Document = Microsoft::glTF::Deserialize(ManifestStr);
			}
			
			//try {
			//	auto Document = Microsoft::glTF::Deserialize(man);
			//}
			//catch (const Microsoft::glTF::GLTFException& Exception) {
			//	std::stringstream SS;
			//	SS << "Deserialize() failed";
			//	SS << Exception.what();
			//	throw std::runtime_error(SS.str());
			//}
		}
		//virtual void Process(const Microsoft::glTF::MeshPrimitive& Primitive) {
		//}
		//virtual void Process(const Microsoft::glTF::Mesh& Mesh) {
		//}
		//virtual void Process(const Microsoft::glTF::Node& Node) {
		//}
		virtual void Process(const Microsoft::glTF::Document& Document) {
			std::cout << "Version:    " << Document.asset.version << std::endl;
			std::cout << "MinVersion: " << Document.asset.minVersion << std::endl;
			std::cout << "Generator:  " << Document.asset.generator << std::endl;
			std::cout << "Copyright:  " << Document.asset.copyright << std::endl;

			std::cout << "Scene Count: " << Document.scenes.Size() << "\n";
			if (Document.scenes.Size() > 0U)
			{
				std::cout << "Default Scene Index: " << Document.GetDefaultScene().id << "\n\n";
			}

			std::cout << "Node Count:     " << Document.nodes.Size() << "\n";
			std::cout << "Camera Count:   " << Document.cameras.Size() << "\n";
			std::cout << "Material Count: " << Document.materials.Size() << "\n\n";

			std::cout << "Mesh Count: " << Document.meshes.Size() << "\n";
			std::cout << "Skin Count: " << Document.skins.Size() << "\n\n";

			std::cout << "Image Count:   " << Document.images.Size() << "\n";
			std::cout << "Texture Count: " << Document.textures.Size() << "\n";
			std::cout << "Sampler Count: " << Document.samplers.Size() << "\n\n";

			std::cout << "Buffer Count:     " << Document.buffers.Size() << "\n";
			std::cout << "BufferView Count: " << Document.bufferViews.Size() << "\n";
			std::cout << "Accessor Count:   " << Document.accessors.Size() << "\n\n";

			std::cout << "Animation Count: " << Document.animations.Size() << "\n\n";
		}
	};
}
