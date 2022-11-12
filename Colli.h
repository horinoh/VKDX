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

			//!< 殆ど移動してない場合は既に重なっているかチェック
			if (VelAB.LengthSq() < 0.001f * 0.001f) {
				if ((RbB->Position - RbA->Position).LengthSq() <= (TotalRadius + 0.001f) * (TotalRadius + 0.001f)) {
					Ct.TimeOfImpact = 0.0f;

					//!< 法線 A -> B
					Ct.Normal = (RbB->Position - RbA->Position).Normalize();

					//!< 衝突ワールド位置 (めり込んでいる場合があるので、必ずしも Ct.WorldA == Ct.WorldB ではない)
					Ct.PointA = RbA->Position + Ct.Normal * SpA->Radius;
					Ct.PointB = RbB->Position - Ct.Normal * SpB->Radius;

					//!< 衝突剛体を覚えておく
					Ct.RigidBodyA = const_cast<RigidBody*>(RbA);
					Ct.RigidBodyB = const_cast<RigidBody*>(RbB);

					return true;
				}
				return false;
			}

			float t0, t1;
			//!< 移動球同士 -> 相対速度 -> 球とカプセル -> (拡大)球とレイとの衝突に帰着
			if (Intersection::RaySphere(RbA->Position, VelAB, RbB->Position, TotalRadius, t0, t1)) {
				if (0.0f <= t1 && t1 <= 1.0f) {
					t0 = std::max<float>(t0, 0.0f);

					//!< TOI
					Ct.TimeOfImpact = t0 * DeltaSec;

					const auto cPosA = RbA->Position + RbA->LinearVelocity * Ct.TimeOfImpact;
					const auto cPosB = RbB->Position + RbB->LinearVelocity * Ct.TimeOfImpact;

					//!< 法線 A -> B
					Ct.Normal = (cPosB - cPosA).Normalize();

					//!< 衝突ワールド位置 (めり込んでいる場合があるので、必ずしも Ct.WorldA == Ct.WorldB ではない)
					Ct.PointA = cPosA + Ct.Normal * SpA->Radius;
					Ct.PointB = cPosB - Ct.Normal * SpB->Radius;

					//!< 衝突剛体を覚えておく
					Ct.RigidBodyA = const_cast<RigidBody*>(RbA);
					Ct.RigidBodyB = const_cast<RigidBody*>(RbB);

					return true;
				}
			}
		}
		return false;
	}

	//!< 衝突時の力積の適用
	static void Resolve(const Contact& Ct)
	{
		const auto TotalInvMass = Ct.RigidBodyA->InvMass + Ct.RigidBodyB->InvMass;
		{
			//!< 半径 重心 ->  衝突点
			const auto RadA = Ct.PointA - Ct.RigidBodyA->GetWorldSpaceCenterOfMass();
			const auto RadB = Ct.PointB - Ct.RigidBodyB->GetWorldSpaceCenterOfMass();
			{
				//!< (A 視点の)速度 A -> B 
				const auto VelAB = (Ct.RigidBodyB->LinearVelocity + Ct.RigidBodyB->AngularVelocity.Cross(RadB)) - (Ct.RigidBodyA->LinearVelocity + Ct.RigidBodyA->AngularVelocity.Cross(RadA));

				//!< 逆慣性テンソル (ワールドスペース)
				const auto IWIA = Ct.RigidBodyA->ToWorld(Ct.RigidBodyA->InvInertiaTensor);
				const auto IWIB = Ct.RigidBodyB->ToWorld(Ct.RigidBodyB->InvInertiaTensor);

				//!< 速度の法線成分
				const auto VelNrmAB = Ct.Normal * VelAB.Dot(Ct.Normal);

				//!< 法線方向 力積J (運動量変化)
				{
					const auto AngularA = (IWIA * RadA.Cross(Ct.Normal)).Cross(RadA);
					const auto AngularB = (IWIB * RadB.Cross(Ct.Normal)).Cross(RadB);
					const auto AngularFactor = (AngularA + AngularB).Dot(Ct.Normal);

					const auto Elasticity = 1.0f + Ct.RigidBodyA->Elasticity * Ct.RigidBodyB->Elasticity;
					const auto J = VelNrmAB * Elasticity / (TotalInvMass + AngularFactor);

					Ct.RigidBodyA->ApplyImpulse(Ct.PointA, J);
					Ct.RigidBodyB->ApplyImpulse(Ct.PointB, -J);
				}

				//!< 接線方向 力積J (運動量変化) == 摩擦力
				{
					//!< 速度の接線成分
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

		//!< めり込みの追い出し (TOIをスライスしてめり込まないようにシミュレーションを進めているため、めり込む可能性があるのは TOI が 0 の時)
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
		//!< 内部点を除外
		{
			const auto [B, E] = std::ranges::remove_if(Pts, [&](const Vec3& Pt) {
				for (auto& i : HullInds) {
					if (Distance::PointTriangle(Pt, HullVerts[std::get<0>(i)], HullVerts[std::get<1>(i)], HullVerts[std::get<2>(i)]) > 0.0f) {
						//!< 少なくとも１つの三角形に対し、正の側にあれば外部点なので早期終了
						return false;
					}
				}
				return true;
			});
#ifdef _DEBUG
			//std::cout << "内部点 : " << std::distance(B, std::end(Pts)) << "削除 (" << std::distance(std::begin(Pts), B) << ")" << std::endl;
#endif
			Pts.erase(B, E);
		}
		//!< 近接点を除外
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
			//std::cout << "近接減 : " << std::distance(B, std::end(Pts)) << "削除 (" <<  << ")" << std::endl;
#endif
			Pts.erase(B, E);
		}
	}
	using EdgeIndices = std::pair<int, int>;
	static void BuildConvexHull(const std::vector<Vec3>& Pts, std::vector<Vec3>& HullVerts, std::vector<TriangleIndices>& HullInds)
	{
#ifdef _DEBUG
		std::cout << "凸包を構築しています..." << std::endl;
#endif
		BuildTetrahedron(Pts, HullVerts, HullInds);

		auto External = Pts;
		//!< 内部点の除外
		RemoveInternal(HullVerts, HullInds, External);

		//!< 外部点が無くなるまで繰り返す
		while (!std::empty(External)) {
#ifdef _DEBUG
			//std::cout << "残りの頂点=" << size(External) << std::endl;
#endif
			//!< 最遠点を見つける
			const auto FarIndex = Distance::Farthest(External, External[0]);
			const auto& FarPoint = External[FarIndex];

			//!< 最遠点を向いていない三角形 [begin(), B) と、向いている三角形 [B, end()) に分割
			const auto [B, E] = std::ranges::partition(HullInds, [&](const auto& i) {
				return Distance::PointTriangle(FarPoint, HullVerts[std::get<0>(i)], HullVerts[std::get<1>(i)], HullVerts[std::get<2>(i)]) <= 0.0f;
			});

			//!< 向いている三角形から、(他の三角形と辺を共有していない) ユニークな辺を収集
			using UniqueEdgeIndices = std::pair<EdgeIndices, bool>;
			std::vector<UniqueEdgeIndices> UniqueEdges;
			std::for_each(B, std::end(HullInds), [&](const auto& i) {
				//!< 三角形の３辺
				const std::array Edges = {
					EdgeIndices({ std::get<0>(i), std::get<1>(i) }),
					EdgeIndices({ std::get<1>(i), std::get<2>(i) }),
					EdgeIndices({ std::get<2>(i), std::get<0>(i) }),
				};
				for (auto& i : Edges) {
					//!< 既出の辺 (逆向き許容) かどうかを調べる
					const auto It = std::ranges::find_if(UniqueEdges, [&](const auto& lhs) {
						return (lhs.first.first == i.first && lhs.first.second == i.second) || (lhs.first.first == i.second && lhs.first.second == i.first); 
					});
					if (std::end(UniqueEdges) == It) {
						//!< 新規の辺は ユニーク (true) として覚えておく
						UniqueEdges.emplace_back(UniqueEdgeIndices({ i, true }));
					}
					else {
						//!< 既存の辺は 非ユニーク (false) として編集
						It->second = false;
					}
				}
			});
			
			//!< (ユニークな辺は収集済みなので) 向いている三角形は削除
			//!< (向いていない三角形が残る)
			HullInds.erase(B, end(HullInds));

			//!< 【バーテックス】最遠点を頂点として追加する
			HullVerts.emplace_back(FarPoint);

			//!< 【インデックス】最遠点とユニーク辺からなる三角形を追加
			{
				const auto [B, E] = std::ranges::remove_if(UniqueEdges, [](const auto& lhs) { return lhs.second == false; });
				std::for_each(std::begin(UniqueEdges), B, [&](const auto& i) {
					HullInds.emplace_back(TriangleIndices({ i.first.first, i.first.second, static_cast<int>(size(HullVerts) - 1) }));
				});
			}

			//!< ここまで済んだら最遠点は削除してよい
			External.erase(std::next(std::begin(External), FarIndex));

			//!< 凸包を更新したので、内部点の削除
			RemoveInternal(HullVerts, HullInds, External);
		}
	}
}