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
	using UniqueEdgeIndices = std::pair<EdgeIndices, bool>;

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
			//!< 少なくとも１つの三角形に対し、正の側にあれば外部点なので早期終了
			if (Distance::PointTriangle(Pt, HullVerts[std::get<0>(i)], HullVerts[std::get<1>(i)], HullVerts[std::get<2>(i)]) > 0.0f) {
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
			//!< 内部点が後方に集まる
			const auto [Beg, End] = std::ranges::remove_if(Pts, [&](const auto& Pt) {
				return IsInternal(Pt, HullVerts, HullInds);
			});
			//!< 削除実行
			Pts.erase(Beg, End);
		}

		//!< 既存点と同一とみなせる点は除外
		{
			//constexpr auto Eps2 = (std::numeric_limits<float>::epsilon)() * (std::numeric_limits<float>::epsilon)();
			constexpr auto Eps2 = 0.001f * 0.001f;
			//!< 同一とみなせる点が後方に集まる
			const auto [Beg, End] = std::ranges::remove_if(Pts, [&](const auto& Pt) {
				for (auto& i : HullVerts) {
					if ((i - Pt).LengthSq() < Eps2) {
						return true;
					}
				}
				return false;
			});
			//!< 削除実行
			Pts.erase(Beg, End);
		}
	}
	//!< ハイポリを食わせるとかなり時間がかかる上に結局ハイポリの凸包ができるだけなのでコリジョンとして現実的ではない、ローポリを食わせること
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
			std::cout << "残りの頂点=" << size(External) << std::endl;
#endif
			//!< 最遠点を見つける
			const auto FarIndexOfExternal = Distance::Farthest(External, External[0]);
			const auto& FarPointOfExternal = External[FarIndexOfExternal];

			//!< 最遠店を向いている三角形 (A とする) と、向いていない三角形 (B とする) の境界となる辺を収集します
			std::vector<UniqueEdgeIndices> UniqueEdges;
			{
				//!< B を前方 [begin(), Beg)、A を後方 [Beg, end()) に分割する
				const auto [Beg, End] = std::ranges::partition(HullInds, [&](const auto& i) {
					return Distance::PointTriangle(FarPointOfExternal, HullVerts[std::get<0>(i)], HullVerts[std::get<1>(i)], HullVerts[std::get<2>(i)]) <= 0.0f;
				});

				//!< A と B の境界となる辺を収集する (A の中から他の三角形と辺を共有しないユニークな辺のみを収集すれば良い)
				std::for_each(Beg, std::end(HullInds), [&](const auto& i) {
					const std::array Edges = {
						EdgeIndices({ std::get<0>(i), std::get<1>(i) }),
						EdgeIndices({ std::get<1>(i), std::get<2>(i) }),
						EdgeIndices({ std::get<2>(i), std::get<0>(i) }),
					};
					for (auto& j : Edges) {
						//!< 既出の辺かどうかを調べる (真逆でも良い)
						const auto It = std::ranges::find_if(UniqueEdges, [&](const auto& rhs) {
							return (rhs.first.first == j.first && rhs.first.second == j.second) || (rhs.first.first == j.second && rhs.first.second == j.first);
						});
						if (std::end(UniqueEdges) == It) {
							//!< 新規の辺なので ユニーク (true) として追加しておく
							UniqueEdges.emplace_back(UniqueEdgeIndices({ j, true }));
						}
						else {
							//!< (追加済みの辺が) 既出の辺となったら 非ユニーク (false) として情報を更新しておく
							It->second = false;
						}
					}
				});
				//!< (辺は収集済みなので) ここまで来たら A は削除してよい  
				HullInds.erase(Beg, std::end(HullInds));
			}

			//!< 凸包の更新
			{
				//!<【バーテックス】最遠点を頂点として追加する
				HullVerts.emplace_back(FarPointOfExternal);

				//!<【インデックス】最遠点とユニーク辺からなる三角形群を追加
				const auto FarIndexOfHull = static_cast<int>(std::size(HullVerts) - 1); //!< (さっき追加した) 最後の要素が最遠点のインデックス
				//!< 非ユニークな辺は後方に追いやる
				const auto [Beg, End] = std::ranges::remove_if(UniqueEdges, [](const auto& lhs) { 
					return lhs.second == false;
				});
				//!< 削除実行 (ユニークな辺が残る)
				UniqueEdges.erase(Beg, std::end(UniqueEdges));

				//!< ユニークな辺と最遠点からなる三角形を追加
				std::ranges::for_each(UniqueEdges, [&](const auto& i) {
					HullInds.emplace_back(TriangleIndices({ i.first.first, i.first.second, FarIndexOfHull }));
				});
			}

			//!< 外部点の更新
			{
				//!< ここまで済んだら最遠点は削除してよい (next は begin(External) から FarIndexOfExternal 個目のイテレータを返す)
				External.erase(std::next(std::begin(External), FarIndexOfExternal));

				//!< 更新した凸包に対して内部点を削除する
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
