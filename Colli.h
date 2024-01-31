#pragma once

#include "Math.h"
using namespace Math;

#include <algorithm>

#include "Shape.h"
#include "RigidBody.h"
using namespace Physics;

namespace Collision
{
	struct BoundEdge
	{
		int Index;
		float Value;
		bool IsMin;
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

	namespace Distance {
		[[nodiscard]] static float PointRaySq(const Vec3& Pt, const Vec3& RayPos, const Vec3& RayDir) {
			const auto ToPt = Pt - RayPos;
			return ((RayDir * ToPt.Dot(RayDir)) - ToPt).LengthSq();
		}
		[[nodiscard]] static float PointRay(const Vec3& Pt, const Vec3& RayPos, const Vec3& RayDir) {
			return sqrtf(PointRaySq(Pt, RayPos, RayDir));
		}

		[[nodiscard]] static float PointTriangle(const Vec3& Pt, const Vec3& A, const Vec3& B, const Vec3& C) {
			return (Pt - A).Dot((B - A).Cross(C - A).Normalize());
		}

		//!< 指定の方向に一番遠い点のインデックスを返す 
		[[nodiscard]] static size_t Farthest(const std::vector<Vec3>& Pts, const Vec3& Dir) {
			return std::distance(std::begin(Pts), std::ranges::max_element(Pts, [&](const auto& lhs, const auto& rhs) { return Dir.Dot(lhs) < Dir.Dot(rhs); }));
		}
		//!< レイから一番遠い点のインデックスを返す
		[[nodiscard]] static size_t Farthest(const std::vector<Vec3>& Pts, const Vec3& RayPos, const Vec3& RayDir) {
			return std::distance(std::begin(Pts), std::ranges::max_element(Pts, [&](const auto& lhs, const auto& rhs) { return PointRaySq(lhs, RayPos, RayDir) < PointRaySq(rhs, RayPos, RayDir); }));
		}
		//!< 三角形から一番遠い点のインデックスを返す
		[[nodiscard]] static size_t Farthest(const std::vector<Vec3>& Pts, const Vec3& A, const Vec3& B, const Vec3& C) {
			return std::distance(std::begin(Pts), std::ranges::max_element(Pts, [&](const auto& lhs, const auto& rhs) { return PointTriangle(lhs, A, B, C) < PointTriangle(rhs, A, B, C); }));
		}
	}
	namespace Intersection {
		//!< レイ vs 球
		[[nodiscard]] static bool RaySphere(const Vec3& RayPos, const Vec3& RayDir, const Vec3& SpPos, const float SpRad, float& T0, float& T1) {
			//!< 1)球	(x - SpPos)^2 = SpRad^2
			//!<		x^2 - 2 * x * SpPos + SpPos^2 - SpRad^2 = 0
			//!< 2)レイ	RayPos + RayDir * t 
			//!<	1) の x に 2) を代入 
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
				T0 = (-B2 - D4Sqrt) / A;
				T1 = (-B2 + D4Sqrt) / A;
				return true;
			}
			return false;
		}
		//!< 球 vs 球
		[[nodiscard]] static bool Spheres(const ShapeSphere* SpA, const ShapeSphere* SpB,
			const Vec3& PosA, const Vec3& PosB,
			const Vec3& VelA, const Vec3& VelB,
			const float DeltaSec, float& T)
		{
			const auto Ray = (VelB - VelA) * DeltaSec;
			const auto TotalRadius = SpA->Radius + SpB->Radius;

			float T0 = 0.0f, T1 = 0.0f;
			if (Ray.LengthSq() < 0.001f * 0.001f) {
				//!< レイが十分短い場合は既に衝突しているかどうかのチェック
				const auto PosAB = PosB - PosA;
				const auto R = TotalRadius + 0.001f;
				if (PosAB.LengthSq() > R * R) {
					return false;
				}
			}
			else if (false == Intersection::RaySphere(PosA, Ray, PosB, TotalRadius, T0, T1) || (T0 > 1.0f || T1 < 0.0f)) {
				return false;
			}

			T0 *= DeltaSec;
			//T1 *= DeltaSec;

			T = (std::max)(T0, 0.0f);

			return true;
		}

		//!< 剛体(球) vs 剛体(球)
		[[nodiscard]] static bool RigidBodySpheres(const RigidBody* RbA, const RigidBody* RbB, const float DeltaSec, Contact& Ct) {
			const auto SpA = static_cast<const ShapeSphere*>(RbA->Shape);
			const auto SpB = static_cast<const ShapeSphere*>(RbB->Shape);
			float T;
			if (Intersection::Spheres(SpA, SpB, RbA->Position, RbB->Position, RbA->LinearVelocity, RbB->LinearVelocity, DeltaSec, T)) {
				Ct.TimeOfImpact = T;

				//!< 衝突時間のオブジェクトの位置
				const auto CPosA = RbA->Position + RbA->LinearVelocity * T;
				const auto CPosB = RbB->Position + RbB->LinearVelocity * T;
				//!< 法線 A -> B
				Ct.Normal = (CPosB - CPosA).Normalize();

				//!< 衝突点
				Ct.PointA = CPosA + Ct.Normal * SpA->Radius;
				Ct.PointB = CPosB - Ct.Normal * SpB->Radius;

				//!< 衝突剛体を覚えておく
				Ct.RigidBodyA = const_cast<RigidBody*>(RbA);
				Ct.RigidBodyB = const_cast<RigidBody*>(RbB);

				return true;
			}
			return false;
		}
		//!< 剛体 vs 剛体
		[[nodiscard]] static bool RigidBodies(const RigidBody* RbA, const RigidBody* RbB, const float DeltaSec, Contact& Ct) {
			if (RbA->Shape->GetShapeTyoe() == Shape::SHAPE::SPHERE && RbB->Shape->GetShapeTyoe() == Shape::SHAPE::SPHERE) {
				return RigidBodySpheres(RbA, RbB, DeltaSec, Ct);
			}
			return false;
		}
	}

	//!< 衝突時の力積の適用
	static void Resolve(const Contact& Ct)
	{
		const auto TotalInvMass = Ct.RigidBodyA->InvMass + Ct.RigidBodyB->InvMass;
		{
			//!< 半径 (重心 -> 衝突点)
			const auto RadiusA = Ct.PointA - Ct.RigidBodyA->GetWorldSpaceCenterOfMass();
			const auto RadiusB = Ct.PointB - Ct.RigidBodyB->GetWorldSpaceCenterOfMass();
			{
				//!< 逆慣性テンソル (ワールドスペース)
				const auto WorldInvInertiaA = Ct.RigidBodyA->GetWorldSpaceInverseInertiaTensor();
				const auto WorldInvInertiaB = Ct.RigidBodyB->GetWorldSpaceInverseInertiaTensor();

				//!< (A 視点の)速度 A -> B
				const auto VelA = Ct.RigidBodyA->LinearVelocity + Ct.RigidBodyA->AngularVelocity.Cross(RadiusA);
				const auto VelB = Ct.RigidBodyB->LinearVelocity + Ct.RigidBodyB->AngularVelocity.Cross(RadiusB);
				const auto VelAB = VelB - VelA;
				//!< 速度の法線成分
				const auto NrmVelAB = Ct.Normal * VelAB.Dot(Ct.Normal);

				//!< 法線方向 力積J (運動量変化)
				{
					//!< 両者の弾性係数を掛けただけの簡易な実装とする
					const auto TotalElasticity = 1.0f + Ct.RigidBodyA->Elasticity * Ct.RigidBodyB->Elasticity;
					{
						const auto AngularJA = (WorldInvInertiaA * RadiusA.Cross(Ct.Normal)).Cross(RadiusA);
						const auto AngularJB = (WorldInvInertiaB * RadiusB.Cross(Ct.Normal)).Cross(RadiusB);
						const auto AngularFactor = (AngularJA + AngularJB).Dot(Ct.Normal);

						const auto J = NrmVelAB * TotalElasticity / (TotalInvMass + AngularFactor);

						Ct.RigidBodyA->ApplyTotalImpulse(Ct.PointA, J);
						Ct.RigidBodyB->ApplyTotalImpulse(Ct.PointB, -J);
					}
				}

				//!< 接線方向 力積J (摩擦力)
				{
					//!< 速度の接線成分
					const auto TanVelAB = VelAB - NrmVelAB;
					const auto Tangent = TanVelAB.Normalize();

					const auto TotalFriction = Ct.RigidBodyA->Friction * Ct.RigidBodyB->Friction;
					{
						const auto AngularJA = (WorldInvInertiaA * RadiusA.Cross(Tangent)).Cross(RadiusA);
						const auto AngularJB = (WorldInvInertiaB * RadiusB.Cross(Tangent)).Cross(RadiusB);
						const auto AngularFactor = (AngularJA + AngularJB).Dot(Tangent);

						const auto J = TanVelAB * TotalFriction / (TotalInvMass + AngularFactor);

						Ct.RigidBodyA->ApplyTotalImpulse(Ct.PointA, J);
						Ct.RigidBodyB->ApplyTotalImpulse(Ct.PointB, -J);
					}
				}
			}
		}

		//!< めり込みの追い出し (TOI == 0.0f の時点で衝突している場合)
		if(0.0f == Ct.TimeOfImpact) {
			//!< 質量により追い出し割合を考慮
			const auto DistAB = Ct.PointB - Ct.PointA;
			Ct.RigidBodyA->Position += DistAB * (Ct.RigidBodyA->InvMass / TotalInvMass);
			Ct.RigidBodyB->Position -= DistAB * (Ct.RigidBodyB->InvMass / TotalInvMass);
		}
	}
}

#include "Convex.h"
using namespace Convex;
#include "GJK.h"
using namespace Collision::GJK;
