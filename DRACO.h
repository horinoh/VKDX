#pragma once
	
#include <fstream>
#include <filesystem>

#pragma warning(push)
#pragma warning(disable:4804)
#pragma warning(disable:4018)
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
	virtual void GenerateNormals(draco::Mesh* Mesh) {
		const auto POSITION = Mesh->GetNamedAttribute(draco::GeometryAttribute::POSITION);

		draco::GeometryAttribute Attr;
		Attr.Init(draco::GeometryAttribute::NORMAL, nullptr, 3, draco::DT_FLOAT32, false, sizeof(float) * 3, 0);
		auto AttrId = Mesh->AddAttribute(Attr, true, (draco::AttributeValueIndex::ValueType)POSITION->size());
		std::vector<std::array<float, 3>> Normals(POSITION->size(), { 0.0f, 0.0f, 0.0f });
		for (auto i = 0; i < POSITION->size() / 3; ++i) {
			std::array<float, 3> Pos0, Pos1, Pos2;
			POSITION->ConvertValue(static_cast<draco::AttributeValueIndex>(i * 3 + 0), data(Pos0));
			POSITION->ConvertValue(static_cast<draco::AttributeValueIndex>(i * 3 + 1), data(Pos1));
			POSITION->ConvertValue(static_cast<draco::AttributeValueIndex>(i * 3 + 2), data(Pos2));
			const std::array<float, 3> V0 = { Pos1[0] - Pos0[0], Pos1[1] - Pos0[1], Pos1[2] - Pos0[2] };
			const std::array<float, 3> V1 = { Pos2[0] - Pos0[0], Pos2[1] - Pos0[1], Pos2[2] - Pos0[2] };
			const std::array<float, 3> Cross = { V0[1] * V1[2] - V0[2] * V1[1], V0[2] * V1[0] - V0[0] * V1[2], V0[0] * V1[1] - V0[1] * V1[0] };
			const auto Len = std::sqrt(Cross[0] * Cross[0] + Cross[1] * Cross[1] + Cross[2] * Cross[2]);
			const std::array<float, 3> N = { Cross[0] / Len, Cross[1] / Len, Cross[2] / Len };

			Normals[i * 3 + 0][0] += N[0]; Normals[i * 3 + 0][1] += N[1]; Normals[i * 3 + 0][2] += N[2];
			Normals[i * 3 + 1][0] += N[0]; Normals[i * 3 + 1][1] += N[1]; Normals[i * 3 + 1][2] += N[2];
			Normals[i * 3 + 2][0] += N[0]; Normals[i * 3 + 2][1] += N[1]; Normals[i * 3 + 2][2] += N[2];
		}
		for (auto i = 0; i < POSITION->size(); ++i) {
			const auto Len = std::sqrt(Normals[i][0] * Normals[i][0] + Normals[i][1] * Normals[i][1] + Normals[i][2] * Normals[i][2]);
#if 0
			const std::array<float, 3> N = { Normals[i][0] / Len, Normals[i][1] / Len, Normals[i][2] / Len };
			Mesh->attribute(AttrId)->SetAttributeValue(static_cast<draco::AttributeValueIndex>(i), data(N));
#else
			constexpr std::array N = { 0.0f, 0.0f, 1.0f };
			Mesh->attribute(AttrId)->SetAttributeValue(static_cast<draco::AttributeValueIndex>(i), data(N));
#endif
		}
	}

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
			else {
				//!< 法線を生成する場合
				GenerateNormals(const_cast<draco::Mesh*>(Mesh));
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
	virtual void Load(const std::filesystem::path& Path) {
		std::ifstream In(data(Path.string()), std::ios::in | std::ios::binary);
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