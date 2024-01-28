#pragma once

#include "Math.h"
using namespace Math;

#include <algorithm>

#include "Shape.h"
#include "RigidBody.h"
using namespace Physics;

namespace Collision
{
	namespace GJK
	{
		[[nodiscard]] static Vec2 SignedVolume(const Vec3& A, const Vec3& B) 
		{
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
				const auto Area = ab.X() * ac.Y() - ab.Y() * ac.X();
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
				Areas[i] = ab.X() * ac.Y() - ab.Y() * ac.X();
			}

			//!< 射影点が三角形の内部
			if (Sign(MaxArea) == Sign(Areas.X()) && Sign(MaxArea) == Sign(Areas.Y()) && Sign(MaxArea) == Sign(Areas.Z())) {
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
			const auto Det = Cofactor.X() + Cofactor.Y() + Cofactor.Z() + Cofactor.W();

			//!< 四面体内部にあれば、重心座標を返す
			if (Sign(Det) == Sign(Cofactor.X()) && Sign(Det) == Sign(Cofactor.Y()) && Sign(Det) == Sign(Cofactor.Z()) && Sign(Det) == Sign(Cofactor.W())) {
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
			SupportPoints(const Vec3& A, const Vec3& B) : Data({ A, B, B - A }) { }

			const Vec3 GetA() const { return std::get<0>(Data); }
			const Vec3 GetB() const { return std::get<1>(Data); }
			const Vec3 GetDiff() const { return std::get<2>(Data); }

		private:
			std::tuple<Vec3, Vec3, Vec3> Data;
		};

		static [[nodiscard]] SupportPoints GetSupportPoints(const RigidBody* RbA, const RigidBody* RbB, const Vec3& NormalizedDir, const float Bias) {
			return { RbA->Shape->GetSupportPoint(RbA->Position, RbA->Rotation, NormalizedDir, Bias), RbB->Shape->GetSupportPoint(RbB->Position, RbB->Rotation, NormalizedDir, Bias) };
		}

		static [[nodiscard]] bool SimplexSignedVolume4(std::vector<SupportPoints>& SimplexPoints, Vec3& Dir)
		{
			const auto Lambda = SignedVolume(SimplexPoints[0].GetDiff(), SimplexPoints[1].GetDiff(), SimplexPoints[2].GetDiff(), SimplexPoints[3].GetDiff());

			//!< Dir の更新
			Dir = SimplexPoints[0].GetDiff() * Lambda[0] + SimplexPoints[1].GetDiff() * Lambda[1] + SimplexPoints[2].GetDiff() * Lambda[2] + SimplexPoints[3].GetDiff() * Lambda[3];
			constexpr auto EpsilonSq = (std::numeric_limits<float>::epsilon)() * (std::numeric_limits<float>::epsilon)();
			if (Dir.LengthSq() < EpsilonSq) {
				//!< 原点を含む -> 衝突
				return true;
			}

			//!< 0.0f == Lambda[i] となる SimplexPoints[i] は削除
			const auto [B, E] = std::ranges::remove_if(SimplexPoints, [&](const auto& rhs) { return 0.0f == Lambda[static_cast<int>(IndexOf(SimplexPoints, rhs))]; });
			SimplexPoints.erase(B, E);

			return false;
		}
		static [[nodiscard]] bool SimplexSignedVolume3(std::vector<SupportPoints>& SimplexPoints, Vec3& Dir)
		{
			const auto Lambda = SignedVolume(SimplexPoints[0].GetDiff(), SimplexPoints[1].GetDiff(), SimplexPoints[2].GetDiff());

			Dir = SimplexPoints[0].GetDiff() * Lambda[0] + SimplexPoints[1].GetDiff() * Lambda[1] + SimplexPoints[2].GetDiff() * Lambda[2];
			constexpr auto EpsilonSq = (std::numeric_limits<float>::epsilon)() * (std::numeric_limits<float>::epsilon)();
			if (Dir.LengthSq() < EpsilonSq) {
				return true;
			}

			const auto [B, E] = std::ranges::remove_if(SimplexPoints, [&](const auto& rhs) { return 0.0f == Lambda[static_cast<int>(IndexOf(SimplexPoints, rhs))]; });
			SimplexPoints.erase(B, E);

			return false;
		}
		static [[nodiscard]] bool SimplexSignedVolume2(std::vector<SupportPoints>& SimplexPoints, Vec3& Dir)
		{
			const auto Lambda = SignedVolume(SimplexPoints[0].GetDiff(), SimplexPoints[1].GetDiff());

			Dir = SimplexPoints[0].GetDiff() * Lambda[0] + SimplexPoints[1].GetDiff() * Lambda[1];
			constexpr auto EpsilonSq = (std::numeric_limits<float>::epsilon)() * (std::numeric_limits<float>::epsilon)();
			if (Dir.LengthSq() < EpsilonSq) {
				return true;
			}

			const auto [B, E] = std::ranges::remove_if(SimplexPoints, [&](const auto& rhs) { return 0.0f == Lambda[static_cast<int>(IndexOf(SimplexPoints, rhs))]; });
			SimplexPoints.erase(B, E);

			return false;
		}
		static [[nodiscard]] bool SimplexSignedVolume(std::vector<SupportPoints>& SimplexPoints, Vec3& Dir)
		{
			switch (size(SimplexPoints)) {
			case 2: return SimplexSignedVolume2(SimplexPoints, Dir);
			case 3: return SimplexSignedVolume3(SimplexPoints, Dir);
			case 4: return SimplexSignedVolume4(SimplexPoints, Dir);
			default:
				assert(false && "");
				return false;
				break;
			}
		}

		namespace Intersection {
			//!< #TODO 要検証
			[[nodiscard]] static bool GJK(const RigidBody* RbA, const RigidBody* RbB)
			{
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
					if (SimplexSignedVolume(SimplexPoints, Dir)) {
						//!< -> 衝突
						return true;
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
	}
}