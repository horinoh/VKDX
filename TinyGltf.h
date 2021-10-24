#pragma once

#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
//#define TINYGLTF_NOEXCEPTION
//#define JSON_NOEXCEPTION
#pragma warning(push)
#pragma warning(disable:4996)
#include <tiny_gltf.h>
#pragma warning(pop)

#include "Hierarchy.h"

namespace Gltf {
	class Tiny : public Hierarchy
	{
	private:
		using Super = Hierarchy;
	public:
		virtual void Load(std::string_view Path) {
			std::string Err, Warn;
			if(IsBinary(Path)){
			//if (std::string::npos != Path.rfind(".glb")) {
				VERIFY(Loader.LoadBinaryFromFile(&Model, &Err, &Warn, data(Path)));
			}
			else /*if(std::string::npos != Path.rfind(".gltf"))*/ {
				VERIFY(Loader.LoadASCIIFromFile(&Model, &Err, &Warn, data(Path)));
			}
			if (!Warn.empty()) { std::cout << "WARN: " << Warn << std::endl; }

			for (auto i : Model.nodes) {
				Process(i);
			}
		}
		virtual void Process(const tinygltf::Primitive& Primitive) {
			//!< トポロジ
			switch (Primitive.mode) {
			case TINYGLTF_MODE_TRIANGLES: Tabs(); std::cout << "TRIANGLES" << std::endl; break;
			}

			//!< バーテックス
			for (auto i : Primitive.attributes) {
				Tabs(); std::cout << "[" << i.second << "] " << i.first << std::endl;

				const auto Acc = Model.accessors[i.second];
				Tabs(); std::cout << "\t";
				switch (Acc.componentType) {
				case TINYGLTF_COMPONENT_TYPE_FLOAT: std::cout << "FLOAT"; break;
				}
				switch (Acc.type) {
				case TINYGLTF_TYPE_VEC2: std::cout << "2" << std::endl; break;
				case TINYGLTF_TYPE_VEC3: std::cout << "3" << std::endl; break;
				case TINYGLTF_TYPE_VEC4: std::cout << "4" << std::endl; break;
				}

				const auto BufferView = Model.bufferViews[Acc.bufferView];
				const auto Stride = Acc.ByteStride(BufferView);
				const auto Buffer = Model.buffers[BufferView.buffer];
				//data(Buffer.data) + BufferView.byteOffset + Acc.byteOffset;

				Tabs(); std::cout << "\t" << "\t" << "Count = " << Acc.count << std::endl;
				Tabs(); std::cout << "\t" << "\t" << "Offset = " << Acc.byteOffset << std::endl;
				Tabs(); std::cout << "\t" << "\t" << "Stride = " << Stride << std::endl;

				Tabs(); std::cout << "\t" << "\t" << "BufferView [" << Acc.bufferView << "]" << std::endl;
				//Tabs(); std::cout << "\t" << "\t" << "\t" << "Target = " << (34962 == BufferView.target ? "ARRAY_BUFFER" : "ELEMENT_ARRAY_BUFFER") << std::endl;
				Tabs(); std::cout << "\t" << "\t" << "\t" << "Length = " << BufferView.byteLength << std::endl;
				Tabs(); std::cout << "\t" << "\t" << "\t" << "Offset = " << BufferView.byteOffset << std::endl;
				Tabs(); std::cout << "\t" << "\t" << "\t" << "Buffer [" << BufferView.buffer << "]" << std::endl;

				if (TINYGLTF_COMPONENT_TYPE_FLOAT == Acc.componentType) {
					const float* p = reinterpret_cast<const float*>(data(Buffer.data) + BufferView.byteOffset + Acc.byteOffset);
					switch (Acc.type) {
					case TINYGLTF_TYPE_VEC2: 
						for (auto j = 0; j < (std::min)(8, static_cast<int>(Acc.count)); ++j) { Tabs(); std::cout << "\t" << p[j * 2 + 0] << ", " << p[j * 2 + 1] << std::endl; }
						break;
					case TINYGLTF_TYPE_VEC3:
						for (auto j = 0; j < (std::min)(8, static_cast<int>(Acc.count)); ++j) { Tabs(); std::cout << "\t" << p[j * 3 + 0] << ", " << p[j * 3 + 1] << ", " << p[j * 3 + 2] << std::endl; }
						break;
					case TINYGLTF_TYPE_VEC4:
						for (auto j = 0; j < (std::min)(8, static_cast<int>(Acc.count)); ++j) { Tabs(); std::cout << "\t" << p[j * 4 + 0] << ", " << p[j * 4 + 1] << ", " << p[j * 4 + 2] << ", " << p[j * 4 + 3] << std::endl; }
						break;
					}
				}
			}

			//!< インデックス
			{
				Tabs(); std::cout << "INDEX" << std::endl;

				const auto Acc = Model.accessors[Primitive.indices];
				Tabs(); std::cout << "\t";
				switch (Acc.componentType) {
				case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT: std::cout << "U16" << std::endl; break;
				case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT: std::cout << "U32" << std::endl; break;
				}
				switch (Acc.type) {
				case TINYGLTF_TYPE_SCALAR: break;
				}
				
				const auto BufferView = Model.bufferViews[Acc.bufferView];
				const auto Stride = Acc.ByteStride(BufferView);
				const auto Buffer = Model.buffers[BufferView.buffer];
				//data(Buffer.data) + BufferView.byteOffset + Acc.byteOffset;

				Tabs(); std::cout << "\t" << "\t" << "Count = " << Acc.count << std::endl;
				Tabs(); std::cout << "\t" << "\t" << "Offset = " << Acc.byteOffset << std::endl;
				Tabs(); std::cout << "\t" << "\t" << "Stride = " << Stride << std::endl;

				Tabs(); std::cout << "\t" << "\t" << "BufferView [" << Acc.bufferView << "]" << std::endl;
				//Tabs(); std::cout << "\t" << "\t" << "\t" << "Target = " << (34962 == BufferView.target ? "ARRAY_BUFFER" : "ELEMENT_ARRAY_BUFFER") << std::endl;
				Tabs(); std::cout << "\t" << "\t" << "\t" << "Length = " << BufferView.byteLength << std::endl;
				Tabs(); std::cout << "\t" << "\t" << "\t" << "Offset = " << BufferView.byteOffset << std::endl;
				Tabs(); std::cout << "\t" << "\t" << "\t" << "Buffer [" << BufferView.buffer << "]" << std::endl;

				switch (Acc.componentType) {
				case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT: 
				{
					const uint16_t* p = reinterpret_cast<const uint16_t*>(data(Buffer.data) + BufferView.byteOffset + Acc.byteOffset);
					Tabs(); std::cout << "\t";
					for (auto j = 0; j < (std::min)(8, static_cast<int>(Acc.count)); ++j) { std::cout << p[j] << ", "; }
					std::cout << std::endl;
				}
					break;
				case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT: 
				{
					const uint32_t* p = reinterpret_cast<const uint32_t*>(data(Buffer.data) + BufferView.byteOffset + Acc.byteOffset);
					Tabs(); std::cout << "\t";
					for (auto j = 0; j < (std::min)(8, static_cast<int>(Acc.count)); ++j) { std::cout << p[j] << ", "; }
					std::cout << std::endl;
				}
					break;
				}
			}
		}
		virtual void Process(const tinygltf::Mesh& Mesh) {
			PushTab();
			for (auto i : Mesh.primitives) {
				Process(i);
			}
			PopTab();
		}
		virtual void Process(const tinygltf::Node& Node) {
			Node.matrix;
			Node.rotation;
			Node.scale;
			Node.translation;
			Node.weights;

			if (-1 != Node.mesh) {
				Process(Model.meshes[Node.mesh]);
			}
			PushTab();
			for (auto i : Node.children) {
				Process(Model.nodes[i]);
			}
			PopTab();
		}

		tinygltf::TinyGLTF Loader;
		tinygltf::Model Model;
	};
}

