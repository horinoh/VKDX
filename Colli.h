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
		[[nodiscard]] static bool RaySphere(const Vec3& RayPos, const Vec3& RayDir, const Vec3& SpPos, const float SpRad, float& t0, float& t1) {
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

	[[nodiscard]] static bool Intersect(const RigidBody* RbA, const RigidBody* RbB, const float DeltaSec, Contact& Ct) {
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

	//!< 頂点を「なるべく」包含するような四面体を作成
	static void BuildTetrahedron(const std::vector<Vec3>& Pts, std::vector<Vec3>& HullVerts, std::vector<TriangleIndices>& HullInds)
	{
		//!< 特定の軸(ここではX)に一番遠い点
		std::array<Vec3, 4> P = { Pts[Distance::Farthest(Pts, Vec3::AxisX())] };
		//< 前出の逆向きの軸軸に一番遠い点
		P[1] = Pts[Distance::Farthest(Pts, -P[0])];
		//!< 前出の 2 点からなる線分に一番遠い点
		P[2] = Pts[Distance::Farthest(Pts, P[0], P[1])];
		//!< 前出の 3 点からなる三角形に一番遠い点
		P[3] = Pts[Distance::Farthest(Pts, P[0], P[1], P[2])];

		//!< CCW になるように調整
		if (Distance::PointTriangle(P[0], P[1], P[2], P[3]) > 0.0f) {
			std::swap(P[0], P[1]);
		}

		//!< 四面体の頂点
		HullVerts.emplace_back(P[0]);
		HullVerts.emplace_back(P[1]);
		HullVerts.emplace_back(P[2]);
		HullVerts.emplace_back(P[3]);

		//!< 四面体のインデックス
		HullInds.emplace_back(TriangleIndices({ 0, 1, 2 }));
		HullInds.emplace_back(TriangleIndices({ 0, 2, 3 }));
		HullInds.emplace_back(TriangleIndices({ 2, 1, 3 }));
		HullInds.emplace_back(TriangleIndices({ 1, 0, 3 }));
	}

	//!< 指定の点が凸包の内部点かどうか
	[[nodiscard]] static bool IsInternal(const Vec3& Pt, const std::vector<Vec3>& HullVerts, const std::vector<TriangleIndices>& HullInds) {
		for (auto& i : HullInds) {
			if (Distance::PointTriangle(Pt, HullVerts[std::get<0>(i)], HullVerts[std::get<1>(i)], HullVerts[std::get<2>(i)]) > 0.0f) {
				//!< 少なくとも１つの三角形に対し、正の側にあれば外部点なので早期終了
				return false;
			}
		}
		return true;
	}
	//!< 凸包の内部点を削除
	static void RemoveInternal(const std::vector<Vec3>& HullVerts, const std::vector<TriangleIndices>& HullInds, std::vector<Vec3>& Pts) 
	{
		//!< 内部点を除外
		{
			const auto [B, E] = std::ranges::remove_if(Pts, [&](const auto& Pt) {
				return IsInternal(Pt, HullVerts, HullInds);
			});
#ifdef _DEBUG
			//std::cout << "内部点 : " << std::distance(B, std::end(Pts)) << "削除 (" << std::distance(std::begin(Pts), B) << ")" << std::endl;
#endif
			Pts.erase(B, E);
		}
		//!< 表面に非常に近い点を除外
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
		//!< 「なるべく」包含するような四面体を作成
		BuildTetrahedron(Pts, HullVerts, HullInds);

		//!< 内部点の除外 -> 外部点が残る
		auto External = Pts;
		RemoveInternal(HullVerts, HullInds, External);

		//!< 外部点が無くなるまで繰り返す
		while (!std::empty(External)) {
#ifdef _DEBUG
			//std::cout << "残りの頂点=" << size(External) << std::endl;
#endif
			//!< 最遠点を見つける
			const auto FarIndexOfExternal = Distance::Farthest(External, External[0]);
			const auto& FarPoint = External[FarIndexOfExternal];

			//!< 最遠点を向いていない三角形 [begin(), B) と、向いている三角形 [B, end()) に分割
			const auto [B, E] = std::ranges::partition(HullInds, [&](const auto& i) {
				return Distance::PointTriangle(FarPoint, HullVerts[std::get<0>(i)], HullVerts[std::get<1>(i)], HullVerts[std::get<2>(i)]) <= 0.0f;
			});

			//!< 向いている三角形から、(他の三角形と辺を共有していない) ユニークな辺のみを収集
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
					const auto It = std::ranges::find_if(UniqueEdges, [&](const auto& rhs) {
						return (rhs.first.first == i.first && rhs.first.second == i.second) || (rhs.first.first == i.second && rhs.first.second == i.first); 
					});
					if (std::end(UniqueEdges) == It) {
						//!< 新規の辺は ユニーク (true) として覚えておく
						UniqueEdges.emplace_back(UniqueEdgeIndices({ i, true }));
					}
					else {
						//!< 既出の辺は 非ユニーク (false) として更新
						It->second = false;
					}
				}
			});

			//!< 凸包の更新
			{
				//!< (ユニークな辺は収集済みなので) 向いている三角形は削除
				HullInds.erase(B, std::end(HullInds));

				//!< 【バーテックス】最遠点を頂点として追加する
				HullVerts.emplace_back(FarPoint);

				//!< 【インデックス】最遠点とユニーク辺からなる三角形群を追加
				{
					const auto FarIndexOfHull = static_cast<int>(std::size(HullVerts) - 1);
					//!< ユニークな辺だけを集める
					const auto [B, E] = std::ranges::remove_if(UniqueEdges, [](const auto& lhs) { return lhs.second == false; });
					std::for_each(std::begin(UniqueEdges), B, [&](const auto& i) {
						HullInds.emplace_back(TriangleIndices({ i.first.first, i.first.second, FarIndexOfHull }));
					});
					//UniqueEdges.erase(B, std::end(UniqueEdges));
				}
			}

			//!< 外部点の更新
			{
				//!< ここまで済んだら最遠点は削除してよい
				External.erase(std::next(std::begin(External), FarIndexOfExternal));

				//!< 更新した凸包に対して内部点の削除
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
		//!< 原点を、(選択した)軸に射影し、軸上での重心を求める
		const auto P = A + U * U.Dot(-A);

		//!< 3 軸の内、射影範囲が最大のものを見つける
		auto MaxIndex = 0;
		auto MaxSeg = 0.0f;
		for (auto i = 0; i < 3; ++i) {
			if (std::abs(MaxSeg) < std::abs(AB[i])) {
				MaxSeg = AB[i];

				MaxIndex = i;
			}
		}

		//!< 対象の軸に射影
		const std::array PrjSeg = { A[MaxIndex] , B[MaxIndex] };
		const auto PrjP = P[MaxIndex];

		//!< P が [A, B] (もしくは [B, A]) の間にある
		if ((PrjSeg[0] < PrjP && PrjP < PrjSeg[1]) || (PrjSeg[1] < PrjP && PrjP < PrjSeg[0])) {
			return Vec2(PrjSeg[1] - PrjP, PrjP - PrjSeg[0]) / MaxSeg;
		}
		//!< P が A 側の外側
		if ((PrjSeg[0] < PrjSeg[1] && PrjP <= PrjSeg[0]) || (PrjSeg[0] >= PrjSeg[1] && PrjP >= PrjSeg[0])) {
			return Vec2::AxisX();
		}
		//!< P が B 側の外側
		else {
			return Vec2::AxisY();
		}
	}
	[[nodiscard]] static Vec3 SignedVolume(const Vec3& A, const Vec3& B, const Vec3& C) {
		const auto N = (B - A).Cross(C - A).Normalize();
		const auto P = N * A.Dot(N);

		//!< 3 平面の内、射影面積が最大のものを見つける
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

		//!< 対象の平面に射影
		const auto Index1 = (MaxIndex + 1) % 3;
		const auto Index2 = (MaxIndex + 2) % 3;
		const std::array PrjPlane = { Vec2(A[Index1], A[Index2]), Vec2(B[Index1], B[Index2]), Vec2(C[Index1], C[Index2]) };
		const auto PrjP = Vec2(P[Index1], P[Index2]);

		//!< 射影点と辺からなるサブ三角形の面積
		Vec3 Areas;
		for (auto i = 0; i < 3; ++i) {
			const auto j = (i + 1) % 3;
			const auto k = (i + 2) % 3;

			const auto ab = PrjPlane[j] - PrjP;
			const auto ac = PrjPlane[k] - PrjP;
			Areas[i] = ab.x() * ac.y() - ab.y() * ac.x();
		}

		//!< 射影点が三角形の内部
		if (Sign(MaxArea) == Sign(Areas.x()) && Sign(MaxArea) == Sign(Areas.y()) && Sign(MaxArea) == Sign(Areas.z())){
			return Areas / MaxArea;
		}

		//!< 3 辺に射影して一番近いものを見つける (1D の SignedVolume に帰着)
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

		//!< 四面体内部にあれば、重心座標を返す
		if (Sign(Det) == Sign(Cofactor.x()) && Sign(Det) == Sign(Cofactor.y()) && Sign(Det) == Sign(Cofactor.z()) && Sign(Det) == Sign(Cofactor.w())) {
			return Cofactor / Det;
		}

		//!< 3 面に射影して一番近いものを見つける (2D の SignedVolume に帰着)
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

			//!< 4 枠必要
			std::vector<SupportPoints> SimplexPoints;
			SimplexPoints.reserve(4);

			SimplexPoints.emplace_back(GetSupportPoints(RbA, RbB, Vec3::One().Normalize(), 0.0f));

			auto Closest = (std::numeric_limits<float>::max)();
			auto Dir = SimplexPoints.back().GetDiff();
			do {
				const auto Pt = GetSupportPoints(RbA, RbB, Dir, 0.0f);

				//!< 既存の点が返るということはもう拡張できない -> 衝突無し
				if (std::end(SimplexPoints) != std::ranges::find_if(SimplexPoints, [&](const auto& rhs) { return Pt.GetDiff().NearlyEqual(rhs.GetDiff()); })) {
					break;
				}

				SimplexPoints.emplace_back(Pt);

				//!< 新しい点が原点を超えていない場合、原点が内部に含まれない -> 衝突無し
				if (Dir.Dot(Pt.GetDiff()) >= 0.0f) {
					break;
				}

				//!< シンプレックスポイント個数毎の処理
				const auto Count = size(SimplexPoints);
				if (4 == Count) {
					const auto Lambda = SignedVolume(SimplexPoints[0].GetDiff(), SimplexPoints[1].GetDiff(), SimplexPoints[2].GetDiff(), SimplexPoints[3].GetDiff());

					//!< Dir の更新
					Dir = SimplexPoints[0].GetDiff() * Lambda[0] + SimplexPoints[1].GetDiff() * Lambda[1] + SimplexPoints[2].GetDiff() * Lambda[2] + SimplexPoints[3].GetDiff() * Lambda[3];
					if (Dir.LengthSq() < EpsilonSq) {
						//!< 原点を含む -> 衝突
						return true;
					}

					//!< 0.0f == Lambda[i] となる SimplexPoints[i] は削除
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

				//!< 最短距離を更新、更新できなれば終了
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