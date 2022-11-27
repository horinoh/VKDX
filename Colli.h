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
		[[nodiscard]] static float PointRaySq(const Vec3& Pt, const Vec3& RayPos, const Vec3& RayDir) {
			const auto ToPt = Pt - RayPos;
			return ((RayDir * ToPt.Dot(RayDir)) - ToPt).LengthSq();
		}
		[[nodiscard]] static float PointRay(const Vec3& Pt, const Vec3& RayPos, const Vec3& RayDir) { return sqrtf(PointRaySq(Pt, RayPos, RayDir)); }

		[[nodiscard]] static float PointTriangle(const Vec3& Pt, const Vec3& A, const Vec3& B, const Vec3& C) {
			return (Pt - A).Dot((B - A).Cross(C - A).Normalize());
		}

		//!< �w��̕����Ɉ�ԉ����_�̃C���f�b�N�X��Ԃ� 
		[[nodiscard]] static size_t Farthest(const std::vector<Vec3>& Pts, const Vec3& Dir) {
			return std::distance(std::begin(Pts), std::ranges::max_element(Pts, [&](const auto& lhs, const auto& rhs) { return Dir.Dot(lhs) < Dir.Dot(rhs); }));
		}
		//!< ���C�����ԉ����_�̃C���f�b�N�X��Ԃ�
		[[nodiscard]] static size_t Farthest(const std::vector<Vec3>& Pts, const Vec3& RayPos, const Vec3& RayDir) {
			return std::distance(std::begin(Pts), std::ranges::max_element(Pts, [&](const auto& lhs, const auto& rhs) { return PointRaySq(lhs, RayPos, RayDir) < PointRaySq(rhs, RayPos, RayDir); }));
		}
		//!< �O�p�`�����ԉ����_�̃C���f�b�N�X��Ԃ�
		[[nodiscard]] static size_t Farthest(const std::vector<Vec3>& Pts, const Vec3& A, const Vec3& B, const Vec3& C) {
			return std::distance(std::begin(Pts), std::ranges::max_element(Pts, [&](const auto& lhs, const auto& rhs) { return PointTriangle(lhs, A, B, C) < PointTriangle(rhs, A, B, C); }));
		}
	}
	namespace Intersection {
		[[nodiscard]] static bool RaySphere(const Vec3& RayPos, const Vec3& RayDir, const Vec3& SpPos, const float SpRad, float& t0, float& t1) {
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

	[[nodiscard]] static bool Intersect(const RigidBody* RbA, const RigidBody* RbB, const float DeltaSec, Contact& Ct) {
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
	[[nodiscard]] static bool IsInternal(const Vec3& Pt, const std::vector<Vec3>& HullVerts, const std::vector<TriangleIndices>& HullInds) {
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
			const auto [B, E] = std::ranges::remove_if(Pts, [&](const auto& Pt) {
				return IsInternal(Pt, HullVerts, HullInds);
			});
#ifdef _DEBUG
			//std::cout << "�����_ : " << std::distance(B, std::end(Pts)) << "�폜 (" << std::distance(std::begin(Pts), B) << ")" << std::endl;
#endif
			Pts.erase(B, E);
		}
		//!< �\�ʂɔ��ɋ߂��_�����O
		{
			const auto [B, E] = std::ranges::remove_if(Pts, [&](const auto& Pt) {
				for (auto& i : HullVerts) {
					constexpr auto ep = (std::numeric_limits<float>::epsilon)(); //!< 0.001f;
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
		//!< �u�Ȃ�ׂ��v��܂���悤�Ȏl�ʑ̂��쐬
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
			const auto& FarPoint = External[FarIndexOfExternal];

			//!< �ŉ��_�������Ă��Ȃ��O�p�` [begin(), B) �ƁA�����Ă���O�p�` [B, end()) �ɕ���
			const auto [B, E] = std::ranges::partition(HullInds, [&](const auto& i) {
				return Distance::PointTriangle(FarPoint, HullVerts[std::get<0>(i)], HullVerts[std::get<1>(i)], HullVerts[std::get<2>(i)]) <= 0.0f;
			});

			//!< �����Ă���O�p�`����A(���̎O�p�`�ƕӂ����L���Ă��Ȃ�) ���j�[�N�ȕӂ݂̂����W
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
					const auto It = std::ranges::find_if(UniqueEdges, [&](const auto& rhs) {
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
				HullVerts.emplace_back(FarPoint);

				//!< �y�C���f�b�N�X�z�ŉ��_�ƃ��j�[�N�ӂ���Ȃ�O�p�`�Q��ǉ�
				{
					const auto FarIndexOfHull = static_cast<int>(std::size(HullVerts) - 1);
					//!< ���j�[�N�ȕӂ������W�߂�
					const auto [B, E] = std::ranges::remove_if(UniqueEdges, [](const auto& lhs) { return lhs.second == false; });
					std::for_each(std::begin(UniqueEdges), B, [&](const auto& i) {
						HullInds.emplace_back(TriangleIndices({ i.first.first, i.first.second, FarIndexOfHull }));
					});
					//UniqueEdges.erase(B, std::end(UniqueEdges));
				}
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
	[[nodiscard]] static Vec3 CalcCenterOfMass(const AABB& Aabb, const std::vector<Vec3>& HullVerts, const std::vector<TriangleIndices>& HullInds) {
		constexpr auto SampleCount = 100;

		auto Sampled = 0;
		auto CenterOfMass = Vec3::Zero();
		const Vec3 d = Aabb.GetExtent() / static_cast<float>(SampleCount);
		for (auto x = 0; x < SampleCount; ++x) {
			for (auto y = 0; y < SampleCount; ++y) {
				for (auto z = 0; z < SampleCount; ++z) {
					const auto Pt = Vec3(Aabb.Min.x() + d.x() * x, Aabb.Min.y() + d.y() * y, Aabb.Min.z() + d.z() * z);
					if (IsInternal(Pt, HullVerts, HullInds)) {
						CenterOfMass += Pt;
						++Sampled;
					}
				}
			}
		}
		return CenterOfMass / static_cast<float>(Sampled);
	}
	[[nodiscard]] static Mat3 CalcInertiaTensor(const AABB& Aabb, const std::vector<Vec3>& HullVerts, const std::vector<TriangleIndices>& HullInds, const Vec3& CenterOfMass) {
		constexpr auto SampleCount = 100;

		auto Sampled = 0;
		auto InertiaTensor = Mat3::Zero();
		const Vec3 d = Aabb.GetExtent() / static_cast<float>(SampleCount);
		for (auto x = 0; x < SampleCount; ++x) {
			for (auto y = 0; y < SampleCount; ++y) {
				for (auto z = 0; z < SampleCount; ++z) {
					const auto Pt = Vec3(Aabb.Min.x() + d.x() * x, Aabb.Min.y() + d.y() * y, Aabb.Min.z() + d.z() * z) - CenterOfMass;
					if (IsInternal(Pt, HullVerts, HullInds)) {

						InertiaTensor[0][0] += Pt.y() * Pt.y() + Pt.z() * Pt.z();
						InertiaTensor[1][1] += Pt.z() * Pt.z() + Pt.x() * Pt.x();
						InertiaTensor[2][2] += Pt.x() * Pt.x() + Pt.y() * Pt.y();

						InertiaTensor[0][1] += -Pt.x() * Pt.y();
						InertiaTensor[0][2] += -Pt.x() * Pt.z();
						InertiaTensor[1][2] += -Pt.y() * Pt.z();

						InertiaTensor[1][0] += -Pt.x() * Pt.y();
						InertiaTensor[2][0] += -Pt.x() * Pt.z();
						InertiaTensor[2][1] += -Pt.y() * Pt.z();

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

#pragma region GJK
	[[nodiscard]] static Vec2 SignedVolume(const Vec3& A, const Vec3& B) {
		const auto AB = B - A;
		const auto U = AB.Normalize();
		//!< ���_���A(�I������)���Ɏˉe���A����ł̏d�S�����߂�
		const auto P = A + U * U.Dot(-A);

		//!< 3 ���̓��A�ˉe�͈͂��ő�̂��̂�������
		auto MaxIndex = 0;
		auto MaxSeg = 0.0f;
		for (auto i = 0; i < 3; ++i) {
			if (std::abs(MaxSeg) < std::abs(AB[i])) {
				MaxSeg = AB[i];

				MaxIndex = i;
			}
		}

		//!< �Ώۂ̎��Ɏˉe
		const std::array PrjSeg = { A[MaxIndex] , B[MaxIndex] };
		const auto PrjP = P[MaxIndex];

		//!< P �� [A, B] (�������� [B, A]) �̊Ԃɂ���
		if ((PrjSeg[0] < PrjP && PrjP < PrjSeg[1]) || (PrjSeg[1] < PrjP && PrjP < PrjSeg[0])) {
			return Vec2(PrjSeg[1] - PrjP, PrjP - PrjSeg[0]) / MaxSeg;
		}
		//!< P �� A ���̊O��
		if ((PrjSeg[0] < PrjSeg[1] && PrjP <= PrjSeg[0]) || (PrjSeg[0] >= PrjSeg[1] && PrjP >= PrjSeg[0])) {
			return Vec2::AxisX();
		}
		//!< P �� B ���̊O��
		else {
			return Vec2::AxisY();
		}
	}
	[[nodiscard]] static Vec3 SignedVolume(const Vec3& A, const Vec3& B, const Vec3& C) {
		const auto N = (B - A).Cross(C - A).Normalize();
		const auto P = N * A.Dot(N);

		//!< 3 ���ʂ̓��A�ˉe�ʐς��ő�̂��̂�������
		auto MaxIndex = 0;
		auto MaxArea = 0.0f;
		for (auto i = 0; i < 3; ++i) {
			const auto j = (i + 1) % 3;
			const auto k = (i + 2) % 3;

			const auto a = Vec2(A[j], A[k]);
			const auto b = Vec2(B[j], B[k]);
			const auto c = Vec2(C[j], C[k]);
			const auto ab = b - a;
			const auto ac = c - a;
			const auto Area = ab.x() * ac.y() - ab.y() * ac.x();
			if (std::abs(Area) > std::abs(MaxArea)) {
				MaxArea = Area;

				MaxIndex = i;
			}
		}

		//!< �Ώۂ̕��ʂɎˉe
		const auto Index1 = (MaxIndex + 1) % 3;
		const auto Index2 = (MaxIndex + 2) % 3;
		const std::array PrjPlane = { Vec2(A[Index1], A[Index2]), Vec2(B[Index1], B[Index2]), Vec2(C[Index1], C[Index2]) };
		const auto PrjP = Vec2(P[Index1], P[Index2]);

		//!< �ˉe�_�ƕӂ���Ȃ�T�u�O�p�`�̖ʐ�
		Vec3 Areas;
		for (auto i = 0; i < 3; ++i) {
			const auto j = (i + 1) % 3;
			const auto k = (i + 2) % 3;

			const auto ab = PrjPlane[j] - PrjP;
			const auto ac = PrjPlane[k] - PrjP;
			Areas[i] = ab.x() * ac.y() - ab.y() * ac.x();
		}

		//!< �ˉe�_���O�p�`�̓���
		if (Sign(MaxArea) == Sign(Areas.x()) && Sign(MaxArea) == Sign(Areas.y()) && Sign(MaxArea) == Sign(Areas.z())){
			return Areas / MaxArea;
		}

		//!< 3 �ӂɎˉe���Ĉ�ԋ߂����̂������� (1D �� SignedVolume �ɋA��)
		const std::array EdgesPts = { A, B, C };
		Vec3 Lambda;
		auto MinLenSq = (std::numeric_limits<float>::max)();
		for (auto i = 0; i < 3; ++i) {
			const auto j = (i + 1) % 3;
			const auto k = (i + 2) % 3;

			const auto LambdaEdge = SignedVolume(EdgesPts[j], EdgesPts[k]);
			const auto LenSq = (EdgesPts[j] * LambdaEdge[0] + EdgesPts[k] * LambdaEdge[1]).LengthSq();
			if (LenSq < MinLenSq) {
				Lambda[i] = 0.0f;
				Lambda[j] = LambdaEdge[0];
				Lambda[k] = LambdaEdge[1];

				MinLenSq = LenSq;
			}
		}
		return Lambda;
	}
	[[nodiscard]] static Vec4 SignedVolume(const Vec3& A, const Vec3& B, const Vec3& C, const Vec3& D) {
		const auto Cofactor = Vec4(-Mat3(B, C, D).Determinant(), Mat3(A, C, D).Determinant(), -Mat3(A, B, D).Determinant(), Mat3(A, B, C).Determinant());
		const auto Det = Cofactor.x() + Cofactor.y() + Cofactor.z() + Cofactor.w();

		//!< �l�ʑ̓����ɂ���΁A�d�S���W��Ԃ�
		if (Sign(Det) == Sign(Cofactor.x()) && Sign(Det) == Sign(Cofactor.y()) && Sign(Det) == Sign(Cofactor.z()) && Sign(Det) == Sign(Cofactor.w())) {
			return Cofactor / Det;
		}

		//!< 3 �ʂɎˉe���Ĉ�ԋ߂����̂������� (2D �� SignedVolume �ɋA��)
		const std::array FacePts = { A, B, C, D };
		Vec4 Lambda;
		auto MinLenSq = (std::numeric_limits<float>::max)();
		for (auto i = 0; i < 4; ++i) {
			const auto j = (i + 1) % 4;
			const auto k = (i + 2) % 4;
			const auto l = (i + 3) % 4;

			const auto LambdaFace = SignedVolume(FacePts[i], FacePts[j], FacePts[k]);
			const auto LenSq = (FacePts[i] * LambdaFace[0] + FacePts[j] * LambdaFace[1] + FacePts[k] * LambdaFace[2]).LengthSq();
			if (LenSq < MinLenSq) {
				Lambda[i] = LambdaFace[0];
				Lambda[j] = LambdaFace[1];
				Lambda[k] = LambdaFace[2];
				Lambda[l] = 0.0f;

				MinLenSq = LenSq;
			}
		}
		return Lambda;
	}

	class SupportPoints
	{
	public:
		SupportPoints(const Vec3& A, const Vec3& B) : Data({A, B, B - A}) { }

		const Vec3 GetA() const { return std::get<0>(Data); }
		const Vec3 GetB() const { return std::get<1>(Data); }
		const Vec3 GetDiff() const { return std::get<2>(Data); }

		//bool operator==(const auto& rhs) const { return GetDiff().NearlyEqual(rhs.GetDiff()); }

	private:
		std::tuple<Vec3, Vec3, Vec3> Data;
	};

	static [[nodiscard]] SupportPoints GetSupportPoints(const RigidBody* RbA, const RigidBody* RbB, const Vec3& NormalizedDir, const float Bias) {
		return { RbA->Shape->GetSupportPoint(RbA->Position, RbA->Rotation, NormalizedDir, Bias), RbB->Shape->GetSupportPoint(RbB->Position, RbB->Rotation, NormalizedDir, Bias) };
	}

	namespace Intersection {
		[[nodiscard]] static bool GJK(const RigidBody* RbA, const RigidBody* RbB) {
			constexpr auto EpsilonSq = (std::numeric_limits<float>::epsilon)() * (std::numeric_limits<float>::epsilon)();

			//!< 4 �g�K�v
			std::vector<SupportPoints> SimplexPoints;
			SimplexPoints.reserve(4);

			SimplexPoints.emplace_back(GetSupportPoints(RbA, RbB, Vec3::One().Normalize(), 0.0f));

			auto Closest = (std::numeric_limits<float>::max)();
			auto Dir = SimplexPoints.back().GetDiff();
			do {
				const auto Pt = GetSupportPoints(RbA, RbB, Dir, 0.0f);

				//!< �����̓_���Ԃ�Ƃ������Ƃ͂����g���ł��Ȃ� -> �Փ˖���
				if (std::end(SimplexPoints) != std::ranges::find_if(SimplexPoints, [&](const auto& rhs) { return Pt.GetDiff().NearlyEqual(rhs.GetDiff()); })) {
					break;
				}

				SimplexPoints.emplace_back(Pt);

				//!< �V�����_�����_�𒴂��Ă��Ȃ��ꍇ�A���_�������Ɋ܂܂�Ȃ� -> �Փ˖���
				if (Dir.Dot(Pt.GetDiff()) >= 0.0f) {
					break;
				}

				//!< �V���v���b�N�X�|�C���g�����̏���
				const auto Count = size(SimplexPoints);
				if (4 == Count) {
					const auto Lambda = SignedVolume(SimplexPoints[0].GetDiff(), SimplexPoints[1].GetDiff(), SimplexPoints[2].GetDiff(), SimplexPoints[3].GetDiff());

					//!< Dir �̍X�V
					Dir = SimplexPoints[0].GetDiff() * Lambda[0] + SimplexPoints[1].GetDiff() * Lambda[1] + SimplexPoints[2].GetDiff() * Lambda[2] + SimplexPoints[3].GetDiff() * Lambda[3];
					if (Dir.LengthSq() < EpsilonSq) {
						//!< ���_���܂� -> �Փ�
						return true;
					}

					//!< 0.0f == Lambda[i] �ƂȂ� SimplexPoints[i] �͍폜
					const auto [B, E] = std::ranges::remove_if(SimplexPoints, [&](const auto& rhs) { return 0.0f == Lambda[static_cast<int>(&rhs - &*std::begin(SimplexPoints))]; });
					SimplexPoints.erase(B, E);
				}
				else if (3 == Count) {
					const auto Lambda = SignedVolume(SimplexPoints[0].GetDiff(), SimplexPoints[1].GetDiff(), SimplexPoints[2].GetDiff());

					Dir = SimplexPoints[0].GetDiff() * Lambda[0] + SimplexPoints[1].GetDiff() * Lambda[1] + SimplexPoints[2].GetDiff() * Lambda[2];
					if (Dir.LengthSq() < EpsilonSq) {
						return true;
					}

					const auto [B, E] = std::ranges::remove_if(SimplexPoints, [&](const auto& rhs) { return 0.0f == Lambda[static_cast<int>(&rhs - &*std::begin(SimplexPoints))]; });
					SimplexPoints.erase(B, E);
				}
				else {
					const auto Lambda = SignedVolume(SimplexPoints[0].GetDiff(), SimplexPoints[1].GetDiff());
					
					Dir = SimplexPoints[0].GetDiff() * Lambda[0] + SimplexPoints[1].GetDiff() * Lambda[1];
					if (Dir.LengthSq() < EpsilonSq) {
						return true;
					}

					const auto [B, E] = std::ranges::remove_if(SimplexPoints, [&](const auto& rhs) { return 0.0f == Lambda[static_cast<int>(&rhs - &*std::begin(SimplexPoints))]; });
					SimplexPoints.erase(B, E);
				}

				//!< �ŒZ�������X�V�A�X�V�ł��Ȃ�ΏI��
				const auto DistSq = Dir.LengthSq();
				if (DistSq < Closest) {
					Closest = DistSq;
				}
				else {
					break;
				}

			} while (4 != size(SimplexPoints));

			return false; 
		}
	}
#pragma endregion
}