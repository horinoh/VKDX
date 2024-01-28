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
	[[nodiscard]] static bool IsInternal(const Vec3& Pt, const std::vector<Vec3>& HullVerts, const std::vector<TriangleIndices>& HullInds) 
	{
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
			const auto [B, E] = std::ranges::remove_if(Pts, [&](const auto& Pt) { return IsInternal(Pt, HullVerts, HullInds); });
#ifdef _DEBUG
			//std::cout << "内部点 : " << std::distance(B, std::end(Pts)) << "削除 (" << std::distance(std::begin(Pts), B) << ")" << std::endl;
#endif
			Pts.erase(B, E);
		}

		//!< 既存点と近接する点は除外
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
			//std::cout << "近接点 : " << std::distance(B, std::end(Pts)) << "削除 (" <<  << ")" << std::endl;
#endif
			Pts.erase(B, E);
		}
	}
	static void BuildConvexHull(const std::vector<Vec3>& Pts, std::vector<Vec3>& HullVerts, std::vector<TriangleIndices>& HullInds)
	{
#ifdef _DEBUG
		std::cout << "凸包を構築しています..." << std::endl;
#endif
		//!< まずは「なるべく」包含するような四面体を作成
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
			const auto& FarPointOfExternal = External[FarIndexOfExternal];

			//!< 最遠点を向いていない三角形 [begin(), B) と、向いている三角形 [B, end()) に分割
			const auto [B, E] = std::ranges::partition(HullInds,
				[&](const auto& i) {
					return Distance::PointTriangle(FarPointOfExternal, HullVerts[std::get<0>(i)], HullVerts[std::get<1>(i)], HullVerts[std::get<2>(i)]) <= 0.0f;
				});

			//!< 向いている三角形から、ユニークな辺 (他の三角形と辺を共有していない) のみを収集
			using UniqueEdgeIndices = std::pair<EdgeIndices, bool>;
			std::vector<UniqueEdgeIndices> UniqueEdges;
			std::for_each(B, std::end(HullInds),
				[&](const auto& i) {
					//!< 三角形の３辺
					const std::array Edges = {
						EdgeIndices({ std::get<0>(i), std::get<1>(i) }),
						EdgeIndices({ std::get<1>(i), std::get<2>(i) }),
						EdgeIndices({ std::get<2>(i), std::get<0>(i) }),
					};
					for (auto& i : Edges) {
						//!< 既出の辺 (逆向き許容) かどうかを調べる
						const auto It = std::ranges::find_if(UniqueEdges,
							[&](const auto& rhs) {
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
				HullVerts.emplace_back(FarPointOfExternal);

				//!< 【インデックス】最遠点とユニーク辺からなる三角形群を追加
				const auto FarIndexOfHull = static_cast<int>(std::size(HullVerts) - 1);
				//!< ユニークな辺だけを集める
				const auto [B, E] = std::ranges::remove_if(UniqueEdges, [](const auto& lhs) { return lhs.second == false; });
				std::for_each(std::begin(UniqueEdges), B,
					[&](const auto& i) {
						HullInds.emplace_back(TriangleIndices({ i.first.first, i.first.second, FarIndexOfHull }));
					});
				//UniqueEdges.erase(B, std::end(UniqueEdges));
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

	//!< #TODO 要検証
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
	//!< #TODO 要検証
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
