#pragma once
	
#include <fstream>

#pragma warning(push)
#pragma warning(disable:4804)
#include "draco/compression/decode.h"
#pragma warning(pop)

#include "Hierarchy.h"

static std::ostream& operator<<(std::ostream& lhs, const std::array<float, 2>& rhs) { lhs << rhs[0] << ", " << rhs[1] << std::endl; return lhs; }
static std::ostream& operator<<(std::ostream& lhs, const std::array<float, 3>& rhs) { lhs << rhs[0] << ", " << rhs[1] << ", " << rhs[2] << std::endl; return lhs; }
static std::ostream& operator<<(std::ostream& lhs, const std::array<float, 4>& rhs) { lhs << rhs[0] << ", " << rhs[1] << ", " << rhs[2] << ", " << rhs[3] << std::endl; return lhs; }

//!< 【エンコード、デコード】 対応フォーマットは .ply, .obj
//!< $draco_encoder -i XXX.ply -o YYY.drc
//!< $draco_decoder - i YYY.drc - o ZZZ.obj
class Draco : public Hierarchy
{
public:
	virtual void Process(const draco::Mesh* Mesh) {
		if (nullptr != Mesh) {
#pragma region POSITION
			const auto POSITION = Mesh->GetNamedAttribute(draco::GeometryAttribute::POSITION);
			if (nullptr != POSITION) {
				Tabs(); std::cout << "POSITION" << std::endl;
				std::array<float, 3> Value;
				for (auto i = 0; i < POSITION->size(); ++i) {
					if (i > 8) { Tabs(); std::cout << "\t" << "..." << std::endl; break; }
					if (POSITION->ConvertValue(static_cast<draco::AttributeValueIndex>(i), data(Value))) {
						Tabs(); std::cout << "\t" << Value << std::endl;
					}
				}
			}
#pragma endregion

#pragma region NORMAL
			const auto NORMAL = Mesh->GetNamedAttribute(draco::GeometryAttribute::NORMAL);
			if (nullptr != NORMAL) {
				Tabs(); std::cout << "NORMAL" << std::endl;
				std::array<float, 3> Value;
				for (auto i = 0; i < NORMAL->size(); ++i) {
					if (i > 8) { Tabs(); std::cout << "\t" << "..." << std::endl; break; }
					if (NORMAL->ConvertValue(static_cast<draco::AttributeValueIndex>(i), data(Value))) {
						Tabs(); std::cout << "\t" << Value << std::endl;
					}
				}
			}
#pragma endregion

#pragma region COLOR
			const auto COLOR = Mesh->GetNamedAttribute(draco::GeometryAttribute::COLOR);
			if (nullptr != COLOR) {
				Tabs(); std::cout << "COLOR" << std::endl;
				std::array<float, 4> Value;
				for (auto i = 0; i < COLOR->size(); ++i) {
					if (i > 8) { Tabs(); std::cout << "\t" << "..." << std::endl; break; }
					if (COLOR->ConvertValue(static_cast<draco::AttributeValueIndex>(i), data(Value))) {
						Tabs(); std::cout << "\t" << Value << std::endl;
					}
				}
			}
#pragma endregion

#pragma region TEX_COORD
			const auto TEX_COORD = Mesh->GetNamedAttribute(draco::GeometryAttribute::TEX_COORD);
			if (nullptr != TEX_COORD) {
				Tabs(); std::cout << "TEX_COORD" << std::endl;
				std::array<float, 2> Value;
				for (auto i = 0; i < TEX_COORD->size(); ++i) {
					if (i > 8) { Tabs(); std::cout << "\t" << "..." << std::endl; break; }
					if (TEX_COORD->ConvertValue(static_cast<draco::AttributeValueIndex>(i), data(Value))) {
						Tabs(); std::cout << "\t" << Value << std::endl;
					}
				}
			}
#pragma endregion

#pragma region INDEX
			Tabs(); std::cout << "Index" << std::endl;
			for (uint32_t i = 0; i < Mesh->num_faces(); ++i) {
				if (i > 8) { Tabs(); std::cout << "\t" << "..." << std::endl; break; }
				const auto Face = Mesh->face(static_cast<draco::FaceIndex>(i));
				for (auto j = 0; j < 3; ++j) {
					Tabs(); std::cout << "\t" << POSITION->mapped_index(Face[j]) << ", ";
				}
				std::cout << std::endl;
			}
#pragma endregion
		}
	}
	virtual void Load(std::string_view Path) {
		std::ifstream In(data(Path), std::ios::in | std::ios::binary);
		if (!In.fail()) {
			In.seekg(0, std::ios_base::end);
			const auto Size = In.tellg();
			if (Size) {
				In.seekg(0, std::ios_base::beg);

				std::vector<std::byte> Buf(Size);
				In.read(reinterpret_cast<char*>(data(Buf)), size(Buf));

				draco::DecoderBuffer DB;
				DB.Init(reinterpret_cast<const char*>(data(Buf)), size(Buf));
				switch (draco::Decoder::GetEncodedGeometryType(&DB).value()) {
				case draco::POINT_CLOUD:
					std::cout << "POINT_CLOUD" << std::endl;
					break;
				case draco::TRIANGULAR_MESH:
					{
						draco::Decoder Decoder;
						const auto Mesh = std::move(Decoder.DecodeMeshFromBuffer(&DB)).value();
						Process(Mesh.get());
					}
					break;
				case draco::INVALID_GEOMETRY_TYPE:
				default:
					break;
				}
			}
		}
	}
};