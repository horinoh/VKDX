#pragma once

#include "Math.h"
using namespace Math;

#include <algorithm>

#include "Shape.h"
#include "RigidBody.h"
using namespace Physics;

namespace Convex
{
	using TriangleIndices = std::tuple<int, int, int>;
	using EdgeIndices = std::pair<int, int>;

	//!< ���_���u�Ȃ�ׂ��v��܂���悤�Ȏl�ʑ̂��쐬
	static void BuildTetrahedron(const std::vector<Vec3>& Pts, std::vector<Vec3>& HullVerts, std::vector<TriangleIndices>& HullInds)
	{
		//!< ����̎�(�����ł�X)�Ɉ�ԉ����_
		std::array<Vec3, 4> P = { Pts[Distance::Farthest(Pts, Vec3::AxisX())] };
		//< �O�o�̋t�����̎����Ɉ�ԉ����_
		P[1] = Pts[Distance::Farthest(Pts, -P[0])];
		//!< �O�o�� 2 �_����Ȃ�����Ɉ�ԉ����_
		P[2] = Pts[Distance::Farthest(Pts, P[0], P[1])];
		//!< �O�o�� 3 �_����Ȃ�O�p�`�Ɉ�ԉ����_
		P[3] = Pts[Distance::Farthest(Pts, P[0], P[1], P[2])];

		//!< CCW �ɂȂ�悤�ɒ���
		if (Distance::PointTriangle(P[0], P[1], P[2], P[3]) > 0.0f) {
			std::swap(P[0], P[1]);
		}

		//!< �l�ʑ̂̒��_
		HullVerts.emplace_back(P[0]);
		HullVerts.emplace_back(P[1]);
		HullVerts.emplace_back(P[2]);
		HullVerts.emplace_back(P[3]);

		//!< �l�ʑ̂̃C���f�b�N�X
		HullInds.emplace_back(TriangleIndices({ 0, 1, 2 }));
		HullInds.emplace_back(TriangleIndices({ 0, 2, 3 }));
		HullInds.emplace_back(TriangleIndices({ 2, 1, 3 }));
		HullInds.emplace_back(TriangleIndices({ 1, 0, 3 }));
	}

	//!< �w��̓_���ʕ�̓����_���ǂ���
	[[nodiscard]] static bool IsInternal(const Vec3& Pt, const std::vector<Vec3>& HullVerts, const std::vector<TriangleIndices>& HullInds) 
	{
		for (auto& i : HullInds) {
			if (Distance::PointTriangle(Pt, HullVerts[std::get<0>(i)], HullVerts[std::get<1>(i)], HullVerts[std::get<2>(i)]) > 0.0f) {
				//!< ���Ȃ��Ƃ��P�̎O�p�`�ɑ΂��A���̑��ɂ���ΊO���_�Ȃ̂ő����I��
				return false;
			}
		}
		return true;
	}
	//!< �ʕ�̓����_���폜
	static void RemoveInternal(const std::vector<Vec3>& HullVerts, const std::vector<TriangleIndices>& HullInds, std::vector<Vec3>& Pts)
	{
		//!< �����_�����O
		{
			const auto [B, E] = std::ranges::remove_if(Pts, [&](const auto& Pt) { return IsInternal(Pt, HullVerts, HullInds); });
#ifdef _DEBUG
			//std::cout << "�����_ : " << std::distance(B, std::end(Pts)) << "�폜 (" << std::distance(std::begin(Pts), B) << ")" << std::endl;
#endif
			Pts.erase(B, E);
		}

		//!< �����_�Ƌߐڂ���_�͏��O
		{
			const auto [B, E] = std::ranges::remove_if(Pts,
				[&](const auto& Pt) {
					for (auto& i : HullVerts) {
						constexpr auto ep = (std::numeric_limits<float>::epsilon)(); //!< 0.001f;
						if ((i - Pt).LengthSq() < ep * ep) {
							return true;
						}
					}
					return false;
				});
#ifdef _DEBUG
			//std::cout << "�ߐړ_ : " << std::distance(B, std::end(Pts)) << "�폜 (" <<  << ")" << std::endl;
#endif
			Pts.erase(B, E);
		}
	}
	static void BuildConvexHull(const std::vector<Vec3>& Pts, std::vector<Vec3>& HullVerts, std::vector<TriangleIndices>& HullInds)
	{
#ifdef _DEBUG
		std::cout << "�ʕ���\�z���Ă��܂�..." << std::endl;
#endif
		//!< �܂��́u�Ȃ�ׂ��v��܂���悤�Ȏl�ʑ̂��쐬
		BuildTetrahedron(Pts, HullVerts, HullInds);

		//!< �����_�̏��O -> �O���_���c��
		auto External = Pts;
		RemoveInternal(HullVerts, HullInds, External);

		//!< �O���_�������Ȃ�܂ŌJ��Ԃ�
		while (!std::empty(External)) {
#ifdef _DEBUG
			//std::cout << "�c��̒��_=" << size(External) << std::endl;
#endif
			//!< �ŉ��_��������
			const auto FarIndexOfExternal = Distance::Farthest(External, External[0]);
			const auto& FarPointOfExternal = External[FarIndexOfExternal];

			//!< �ŉ��_�������Ă��Ȃ��O�p�` [begin(), B) �ƁA�����Ă���O�p�` [B, end()) �ɕ���
			const auto [B, E] = std::ranges::partition(HullInds,
				[&](const auto& i) {
					return Distance::PointTriangle(FarPointOfExternal, HullVerts[std::get<0>(i)], HullVerts[std::get<1>(i)], HullVerts[std::get<2>(i)]) <= 0.0f;
				});

			//!< �����Ă���O�p�`����A���j�[�N�ȕ� (���̎O�p�`�ƕӂ����L���Ă��Ȃ�) �݂̂����W
			using UniqueEdgeIndices = std::pair<EdgeIndices, bool>;
			std::vector<UniqueEdgeIndices> UniqueEdges;
			std::for_each(B, std::end(HullInds),
				[&](const auto& i) {
					//!< �O�p�`�̂R��
					const std::array Edges = {
						EdgeIndices({ std::get<0>(i), std::get<1>(i) }),
						EdgeIndices({ std::get<1>(i), std::get<2>(i) }),
						EdgeIndices({ std::get<2>(i), std::get<0>(i) }),
					};
					for (auto& i : Edges) {
						//!< ���o�̕� (�t�������e) ���ǂ����𒲂ׂ�
						const auto It = std::ranges::find_if(UniqueEdges,
							[&](const auto& rhs) {
								return (rhs.first.first == i.first && rhs.first.second == i.second) || (rhs.first.first == i.second && rhs.first.second == i.first);
							});
						if (std::end(UniqueEdges) == It) {
							//!< �V�K�̕ӂ� ���j�[�N (true) �Ƃ��Ċo���Ă���
							UniqueEdges.emplace_back(UniqueEdgeIndices({ i, true }));
						}
						else {
							//!< ���o�̕ӂ� �񃆃j�[�N (false) �Ƃ��čX�V
							It->second = false;
						}
					}
				});

			//!< �ʕ�̍X�V
			{
				//!< (���j�[�N�ȕӂ͎��W�ς݂Ȃ̂�) �����Ă���O�p�`�͍폜
				HullInds.erase(B, std::end(HullInds));

				//!< �y�o�[�e�b�N�X�z�ŉ��_�𒸓_�Ƃ��Ēǉ�����
				HullVerts.emplace_back(FarPointOfExternal);

				//!< �y�C���f�b�N�X�z�ŉ��_�ƃ��j�[�N�ӂ���Ȃ�O�p�`�Q��ǉ�
				const auto FarIndexOfHull = static_cast<int>(std::size(HullVerts) - 1);
				//!< ���j�[�N�ȕӂ������W�߂�
				const auto [B, E] = std::ranges::remove_if(UniqueEdges, [](const auto& lhs) { return lhs.second == false; });
				std::for_each(std::begin(UniqueEdges), B,
					[&](const auto& i) {
						HullInds.emplace_back(TriangleIndices({ i.first.first, i.first.second, FarIndexOfHull }));
					});
				//UniqueEdges.erase(B, std::end(UniqueEdges));
			}

			//!< �O���_�̍X�V
			{
				//!< �����܂ōς񂾂�ŉ��_�͍폜���Ă悢
				External.erase(std::next(std::begin(External), FarIndexOfExternal));

				//!< �X�V�����ʕ�ɑ΂��ē����_�̍폜
				RemoveInternal(HullVerts, HullInds, External);
			}
		}
	}

	//!< #TODO �v����
	[[nodiscard]] static Vec3 CalcCenterOfMass(const AABB& Aabb, const std::vector<Vec3>& HullVerts, const std::vector<TriangleIndices>& HullInds) {
		constexpr auto SampleCount = 100;

		auto Sampled = 0;
		auto CenterOfMass = Vec3::Zero();
		const Vec3 d = Aabb.GetExtent() / static_cast<float>(SampleCount);
		for (auto x = 0; x < SampleCount; ++x) {
			for (auto y = 0; y < SampleCount; ++y) {
				for (auto z = 0; z < SampleCount; ++z) {
					const auto Pt = Vec3(Aabb.Min.X() + d.X() * x, Aabb.Min.Y() + d.Y() * y, Aabb.Min.Z() + d.Z() * z);
					if (IsInternal(Pt, HullVerts, HullInds)) {
						CenterOfMass += Pt;
						++Sampled;
					}
				}
			}
		}
		return CenterOfMass / static_cast<float>(Sampled);
	}
	//!< #TODO �v����
	[[nodiscard]] static Mat3 CalcInertiaTensor(const AABB& Aabb, const std::vector<Vec3>& HullVerts, const std::vector<TriangleIndices>& HullInds, const Vec3& CenterOfMass) {
		constexpr auto SampleCount = 100;

		auto Sampled = 0;
		auto InertiaTensor = Mat3::Zero();
		const Vec3 d = Aabb.GetExtent() / static_cast<float>(SampleCount);
		for (auto x = 0; x < SampleCount; ++x) {
			for (auto y = 0; y < SampleCount; ++y) {
				for (auto z = 0; z < SampleCount; ++z) {
					const auto Pt = Vec3(Aabb.Min.X() + d.Z() * x, Aabb.Min.Y() + d.Y() * y, Aabb.Min.Z() + d.Z() * z) - CenterOfMass;
					if (IsInternal(Pt, HullVerts, HullInds)) {

						InertiaTensor[0][0] += Pt.Y() * Pt.Y() + Pt.Z() * Pt.Z();
						InertiaTensor[1][1] += Pt.Z() * Pt.Z() + Pt.X() * Pt.X();
						InertiaTensor[2][2] += Pt.X() * Pt.X() + Pt.Y() * Pt.Y();

						InertiaTensor[0][1] += -Pt.X() * Pt.Y();
						InertiaTensor[0][2] += -Pt.X() * Pt.Z();
						InertiaTensor[1][2] += -Pt.Y() * Pt.Z();

						InertiaTensor[1][0] += -Pt.X() * Pt.Y();
						InertiaTensor[2][0] += -Pt.X() * Pt.Z();
						InertiaTensor[2][1] += -Pt.Y() * Pt.Z();

						++Sampled;
					}
				}
			}
		}
		return InertiaTensor / static_cast<float>(Sampled);
	}
	[[nodiscard]] static Mat3 CalcInertiaTensor(const AABB& Aabb, const std::vector<Vec3>& HullVerts, const std::vector<TriangleIndices>& HullInds) {
		return CalcInertiaTensor(Aabb, HullVerts, HullInds, CalcCenterOfMass(Aabb, HullVerts, HullInds));
	}
}
