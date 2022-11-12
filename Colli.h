#pragma once

#include "Math.h"
using namespace Math;

#include <algorithm>

#include "Shape.h"
#include "RigidBody.h"
using namespace Phys;

namespace Colli
{
	namespace Distance {
		static float PointRaySq(const Vec3& Pt, const Vec3& RayPos, const Vec3& RayDir) {
			const auto ToPt = Pt - RayPos;
			return ((RayDir * ToPt.Dot(RayDir)) - ToPt).LengthSq();
		}
		static float PointRay(const Vec3& Pt, const Vec3& RayPos, const Vec3& RayDir) { return sqrtf(PointRaySq(Pt, RayPos, RayDir)); }

		static float PointTriangle(const Vec3& Pt, const Vec3& A, const Vec3& B, const Vec3& C) {
			return (Pt - A).Dot((B - A).Cross(C - A).Normalize());
		}

		static size_t Farthest(const std::vector<Vec3>& Pts, const Vec3& Dir) {
			return std::distance(std::begin(Pts), std::ranges::max_element(Pts, [&](const auto& lhs, const auto& rhs) { return Dir.Dot(lhs) < Dir.Dot(rhs); }));
		}
		static size_t Farthest(const std::vector<Vec3>& Pts, const Vec3& RayPos, const Vec3& RayDir) {
			return std::distance(std::begin(Pts), std::ranges::max_element(Pts, [&](const auto& lhs, const auto& rhs) { return PointRaySq(lhs, RayPos, RayDir) < PointRaySq(rhs, RayPos, RayDir); }));
		}
		static size_t Farthest(const std::vector<Vec3>& Pts, const Vec3& A, const Vec3& B, const Vec3& C) {
			return std::distance(std::begin(Pts), std::ranges::max_element(Pts, [&](const auto& lhs, const auto& rhs) { return PointTriangle(lhs, A, B, C) < PointTriangle(rhs, A, B, C); }));
		}
	}
	namespace Intersection {
		static bool RaySphere(const Vec3& RayPos, const Vec3& RayDir, const Vec3& SpPos, const float SpRad, float& t0, float& t1) {
			//!< 1)��	(x - SpPos)^2 = SpRad^2
			//!<		x^2 - 2 * x * SpPos + SpPos^2 - SpRad^2 = 0
			//!< 2)���C	RayPos + RayDir * t 
			//!<	1) �� x �� 2) ���� 
			//!< (RayPos + RayDir * t - SpPos)^2 = SpRad^2
			//!< (RayDir * t + M)^2 - SpRad^2 = 0 ... M = RayPos - SpPos
			//!< RayDir^2 * t^2 + 2 * M * RayDir * t + M^2 - SpRad^2 = 0
			//!< A * t^2 + B * t + C = 0 ... A = RayDir^2, B = 2 * M * RayDir, C = M^2 - SpRad^2
			const auto M = RayPos - SpPos;
			const auto A = RayDir.Dot(RayDir);
			const auto B2 = M.Dot(RayDir);
			const auto C = M.Dot(M) - SpRad * SpRad;

			const auto D4 = B2 * B2 - A * C;
			if (D4 >= 0) {
				const auto D4Sqrt = sqrt(D4);
				t0 = (-B2 - D4Sqrt) / A;
				t1 = (-B2 + D4Sqrt) / A;
				return true;
			}
			return false;
		}
	}

	struct BoundEdge
	{
		int Index;
		float Value;
		bool isMin;
	};

	struct Contact
	{
		float TimeOfImpact = 0.0f;

		RigidBody* RigidBodyA = nullptr;
		RigidBody* RigidBodyB = nullptr;

		Vec3 PointA;
		Vec3 PointB;

		Vec3 Normal;

		//float SeparationDistance = 0.0f;

		bool operator==(const Contact& rhs) const {
			return Normal == rhs.Normal && PointA == rhs.PointA && PointB == rhs.PointB && TimeOfImpact == rhs.TimeOfImpact && RigidBodyA == rhs.RigidBodyA && RigidBodyB == rhs.RigidBodyB;
		}
	};

	static bool Intersect(const RigidBody* RbA, const RigidBody* RbB, const float DeltaSec, Contact& Ct) {
		if (RbA->Shape->GetShapeTyoe() == Shape::SHAPE::SPHERE && RbB->Shape->GetShapeTyoe() == Shape::SHAPE::SPHERE) {
			const auto SpA = static_cast<const ShapeSphere*>(RbA->Shape);
			const auto SpB = static_cast<const ShapeSphere*>(RbB->Shape);

			const auto TotalRadius = SpA->Radius + SpB->Radius;
			const auto VelAB = (RbB->LinearVelocity - RbA->LinearVelocity) * DeltaSec;

			//!< �w�ǈړ����ĂȂ��ꍇ�͊��ɏd�Ȃ��Ă��邩�`�F�b�N
			if (VelAB.LengthSq() < 0.001f * 0.001f) {
				if ((RbB->Position - RbA->Position).LengthSq() <= (TotalRadius + 0.001f) * (TotalRadius + 0.001f)) {
					Ct.TimeOfImpact = 0.0f;

					//!< �@�� A -> B
					Ct.Normal = (RbB->Position - RbA->Position).Normalize();

					//!< �Փ˃��[���h�ʒu (�߂荞��ł���ꍇ������̂ŁA�K������ Ct.WorldA == Ct.WorldB �ł͂Ȃ�)
					Ct.PointA = RbA->Position + Ct.Normal * SpA->Radius;
					Ct.PointB = RbB->Position - Ct.Normal * SpB->Radius;

					//!< �Փˍ��̂��o���Ă���
					Ct.RigidBodyA = const_cast<RigidBody*>(RbA);
					Ct.RigidBodyB = const_cast<RigidBody*>(RbB);

					return true;
				}
				return false;
			}

			float t0, t1;
			//!< �ړ������m -> ���Α��x -> ���ƃJ�v�Z�� -> (�g��)���ƃ��C�Ƃ̏Փ˂ɋA��
			if (Intersection::RaySphere(RbA->Position, VelAB, RbB->Position, TotalRadius, t0, t1)) {
				if (0.0f <= t1 && t1 <= 1.0f) {
					t0 = std::max<float>(t0, 0.0f);

					//!< TOI
					Ct.TimeOfImpact = t0 * DeltaSec;

					const auto cPosA = RbA->Position + RbA->LinearVelocity * Ct.TimeOfImpact;
					const auto cPosB = RbB->Position + RbB->LinearVelocity * Ct.TimeOfImpact;

					//!< �@�� A -> B
					Ct.Normal = (cPosB - cPosA).Normalize();

					//!< �Փ˃��[���h�ʒu (�߂荞��ł���ꍇ������̂ŁA�K������ Ct.WorldA == Ct.WorldB �ł͂Ȃ�)
					Ct.PointA = cPosA + Ct.Normal * SpA->Radius;
					Ct.PointB = cPosB - Ct.Normal * SpB->Radius;

					//!< �Փˍ��̂��o���Ă���
					Ct.RigidBodyA = const_cast<RigidBody*>(RbA);
					Ct.RigidBodyB = const_cast<RigidBody*>(RbB);

					return true;
				}
			}
		}
		return false;
	}

	//!< �Փˎ��̗͐ς̓K�p
	static void Resolve(const Contact& Ct)
	{
		const auto TotalInvMass = Ct.RigidBodyA->InvMass + Ct.RigidBodyB->InvMass;
		{
			//!< ���a �d�S ->  �Փ˓_
			const auto RadA = Ct.PointA - Ct.RigidBodyA->GetWorldSpaceCenterOfMass();
			const auto RadB = Ct.PointB - Ct.RigidBodyB->GetWorldSpaceCenterOfMass();
			{
				//!< (A ���_��)���x A -> B 
				const auto VelAB = (Ct.RigidBodyB->LinearVelocity + Ct.RigidBodyB->AngularVelocity.Cross(RadB)) - (Ct.RigidBodyA->LinearVelocity + Ct.RigidBodyA->AngularVelocity.Cross(RadA));

				//!< �t�����e���\�� (���[���h�X�y�[�X)
				const auto IWIA = Ct.RigidBodyA->ToWorld(Ct.RigidBodyA->InvInertiaTensor);
				const auto IWIB = Ct.RigidBodyB->ToWorld(Ct.RigidBodyB->InvInertiaTensor);

				//!< ���x�̖@������
				const auto VelNrmAB = Ct.Normal * VelAB.Dot(Ct.Normal);

				//!< �@������ �͐�J (�^���ʕω�)
				{
					const auto AngularA = (IWIA * RadA.Cross(Ct.Normal)).Cross(RadA);
					const auto AngularB = (IWIB * RadB.Cross(Ct.Normal)).Cross(RadB);
					const auto AngularFactor = (AngularA + AngularB).Dot(Ct.Normal);

					const auto Elasticity = 1.0f + Ct.RigidBodyA->Elasticity * Ct.RigidBodyB->Elasticity;
					const auto J = VelNrmAB * Elasticity / (TotalInvMass + AngularFactor);

					Ct.RigidBodyA->ApplyImpulse(Ct.PointA, J);
					Ct.RigidBodyB->ApplyImpulse(Ct.PointB, -J);
				}

				//!< �ڐ����� �͐�J (�^���ʕω�) == ���C��
				{
					//!< ���x�̐ڐ�����
					const auto VelTanAB = VelAB - VelNrmAB;
					const auto Tangent = VelTanAB.Normalize();

					const auto AngularA = (IWIA * RadA.Cross(Tangent)).Cross(RadA);
					const auto AngularB = (IWIB * RadB.Cross(Tangent)).Cross(RadB);
					const auto AngularFactor = (AngularA + AngularB).Dot(Tangent);

					const auto Friction = Ct.RigidBodyA->Friction * Ct.RigidBodyB->Friction;
					const auto J = VelTanAB * Friction / (TotalInvMass + AngularFactor);

					Ct.RigidBodyA->ApplyImpulse(Ct.PointA, J);
					Ct.RigidBodyB->ApplyImpulse(Ct.PointB, -J);
				}
			}
		}

		//!< �߂荞�݂̒ǂ��o�� (TOI���X���C�X���Ă߂荞�܂Ȃ��悤�ɃV�~�����[�V������i�߂Ă��邽�߁A�߂荞�މ\��������̂� TOI �� 0 �̎�)
		if (0.0f == Ct.TimeOfImpact) {
			const auto AB = Ct.PointB - Ct.PointA;
			Ct.RigidBodyA->Position += AB * (Ct.RigidBodyA->InvMass / TotalInvMass);
			Ct.RigidBodyB->Position -= AB * (Ct.RigidBodyB->InvMass / TotalInvMass);
		}
	}

	using TriangleIndices = std::tuple<int, int, int>;

	static void BuildTetrahedron(const std::vector<Vec3>& Pts, std::vector<Vec3>& HullVerts, std::vector<TriangleIndices>& HullInds)
	{
		std::array<Vec3, 4> P = { Pts[Distance::Farthest(Pts, Vec3::AxisX())] };
		P[1] = Pts[Distance::Farthest(Pts, -P[0])];
		P[2] = Pts[Distance::Farthest(Pts, P[0], P[1])];
		P[3] = Pts[Distance::Farthest(Pts, P[0], P[1], P[2])];
	
		//const std::array I = { Distance::Farthest(Pts, Vec3::AxisX()), Distance::Farthest(Pts, -P[0]), Distance::Farthest(Pts, P[0], P[1]), Distance::Farthest(Pts, P[0], P[1], P[2]) };

		//!< CCW
		if (Distance::PointTriangle(P[0], P[1], P[2], P[3]) > 0.0f) {
			std::swap(P[0], P[1]);
		}

		HullVerts.emplace_back(P[0]);
		HullVerts.emplace_back(P[1]);
		HullVerts.emplace_back(P[2]);
		HullVerts.emplace_back(P[3]);

		HullInds.emplace_back(TriangleIndices({ 0, 1, 2 }));
		HullInds.emplace_back(TriangleIndices({ 0, 2, 3 }));
		HullInds.emplace_back(TriangleIndices({ 2, 1, 3 }));
		HullInds.emplace_back(TriangleIndices({ 1, 0, 3 }));
	}
	static void RemoveInternal(const std::vector<Vec3>& HullVerts, const std::vector<TriangleIndices>& HullInds, std::vector<Vec3>& Pts) 
	{
		//!< �����_�����O
		{
			const auto [B, E] = std::ranges::remove_if(Pts, [&](const Vec3& Pt) {
				for (auto& i : HullInds) {
					if (Distance::PointTriangle(Pt, HullVerts[std::get<0>(i)], HullVerts[std::get<1>(i)], HullVerts[std::get<2>(i)]) > 0.0f) {
						//!< ���Ȃ��Ƃ��P�̎O�p�`�ɑ΂��A���̑��ɂ���ΊO���_�Ȃ̂ő����I��
						return false;
					}
				}
				return true;
			});
#ifdef _DEBUG
			//std::cout << "�����_ : " << std::distance(B, std::end(Pts)) << "�폜 (" << std::distance(std::begin(Pts), B) << ")" << std::endl;
#endif
			Pts.erase(B, E);
		}
		//!< �ߐړ_�����O
		{
			const auto [B, E] = std::ranges::remove_if(Pts, [&](const Vec3& Pt) {
				for (auto& i : HullVerts) {
					constexpr auto ep = (std::numeric_limits<float>::epsilon)();
					//constexpr auto ep = 0.001f;
					if ((i - Pt).LengthSq() < ep * ep) {
						return true;
					}
				}
				return false;
			});
#ifdef _DEBUG
			//std::cout << "�ߐڌ� : " << std::distance(B, std::end(Pts)) << "�폜 (" <<  << ")" << std::endl;
#endif
			Pts.erase(B, E);
		}
	}
	using EdgeIndices = std::pair<int, int>;
	static void BuildConvexHull(const std::vector<Vec3>& Pts, std::vector<Vec3>& HullVerts, std::vector<TriangleIndices>& HullInds)
	{
#ifdef _DEBUG
		std::cout << "�ʕ���\�z���Ă��܂�..." << std::endl;
#endif
		BuildTetrahedron(Pts, HullVerts, HullInds);

		auto External = Pts;
		//!< �����_�̏��O
		RemoveInternal(HullVerts, HullInds, External);

		//!< �O���_�������Ȃ�܂ŌJ��Ԃ�
		while (!std::empty(External)) {
#ifdef _DEBUG
			//std::cout << "�c��̒��_=" << size(External) << std::endl;
#endif
			//!< �ŉ��_��������
			const auto FarIndex = Distance::Farthest(External, External[0]);
			const auto& FarPoint = External[FarIndex];

			//!< �ŉ��_�������Ă��Ȃ��O�p�` [begin(), B) �ƁA�����Ă���O�p�` [B, end()) �ɕ���
			const auto [B, E] = std::ranges::partition(HullInds, [&](const auto& i) {
				return Distance::PointTriangle(FarPoint, HullVerts[std::get<0>(i)], HullVerts[std::get<1>(i)], HullVerts[std::get<2>(i)]) <= 0.0f;
			});

			//!< �����Ă���O�p�`����A(���̎O�p�`�ƕӂ����L���Ă��Ȃ�) ���j�[�N�ȕӂ����W
			using UniqueEdgeIndices = std::pair<EdgeIndices, bool>;
			std::vector<UniqueEdgeIndices> UniqueEdges;
			std::for_each(B, std::end(HullInds), [&](const auto& i) {
				//!< �O�p�`�̂R��
				const std::array Edges = {
					EdgeIndices({ std::get<0>(i), std::get<1>(i) }),
					EdgeIndices({ std::get<1>(i), std::get<2>(i) }),
					EdgeIndices({ std::get<2>(i), std::get<0>(i) }),
				};
				for (auto& i : Edges) {
					//!< ���o�̕� (�t�������e) ���ǂ����𒲂ׂ�
					const auto It = std::ranges::find_if(UniqueEdges, [&](const auto& lhs) {
						return (lhs.first.first == i.first && lhs.first.second == i.second) || (lhs.first.first == i.second && lhs.first.second == i.first); 
					});
					if (std::end(UniqueEdges) == It) {
						//!< �V�K�̕ӂ� ���j�[�N (true) �Ƃ��Ċo���Ă���
						UniqueEdges.emplace_back(UniqueEdgeIndices({ i, true }));
					}
					else {
						//!< �����̕ӂ� �񃆃j�[�N (false) �Ƃ��ĕҏW
						It->second = false;
					}
				}
			});
			
			//!< (���j�[�N�ȕӂ͎��W�ς݂Ȃ̂�) �����Ă���O�p�`�͍폜
			//!< (�����Ă��Ȃ��O�p�`���c��)
			HullInds.erase(B, end(HullInds));

			//!< �y�o�[�e�b�N�X�z�ŉ��_�𒸓_�Ƃ��Ēǉ�����
			HullVerts.emplace_back(FarPoint);

			//!< �y�C���f�b�N�X�z�ŉ��_�ƃ��j�[�N�ӂ���Ȃ�O�p�`��ǉ�
			{
				const auto [B, E] = std::ranges::remove_if(UniqueEdges, [](const auto& lhs) { return lhs.second == false; });
				std::for_each(std::begin(UniqueEdges), B, [&](const auto& i) {
					HullInds.emplace_back(TriangleIndices({ i.first.first, i.first.second, static_cast<int>(size(HullVerts) - 1) }));
				});
			}

			//!< �����܂ōς񂾂�ŉ��_�͍폜���Ă悢
			External.erase(std::next(std::begin(External), FarIndex));

			//!< �ʕ���X�V�����̂ŁA�����_�̍폜
			RemoveInternal(HullVerts, HullInds, External);
		}
	}
}