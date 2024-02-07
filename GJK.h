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
		//!< SignedVolue : 射影が最大となる軸や平面を見つけ、それに対し原点を射影して内部にあれば重心を返す

		[[nodiscard]] static Vec2 SignedVolume(const Vec3& A, const Vec3& B) 
		{
			const auto AB = B - A;
			const auto N = AB.Normalize();
			//!< 原点を AB 軸に射影し P とする
			const auto P = A + N * N.Dot(-A);

			//!< X, Y, Z 軸の内、絶対値が最大のものを見つける
			auto MaxIndex = 0;
			auto MaxSeg = 0.0f;
			for (auto i = 0; i < 3; ++i) {
				if (std::abs(MaxSeg) < std::abs(AB[i])) {
					MaxSeg = AB[i];
					MaxIndex = i;
				}
			}

			//!< 「P と線分」を選択した軸へ射影
			const std::array PrjSeg = { A[MaxIndex] , B[MaxIndex] };
			const auto PrjP = P[MaxIndex];

			//!< P が [A, B] の内部にある場合
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
		[[nodiscard]] static Vec3 SignedVolume(const Vec3& A, const Vec3& B, const Vec3& C) 
		{
			const auto N = (B - A).Cross(C - A).Normalize();
			const auto P = N * A.Dot(N);

			//!<  XY, YZ, ZX 平面の内、射影面積が最大のものを見つける
			auto MaxIndex = 0;
			auto MaxArea = 0.0f;
			for (auto i = 0; i < 3; ++i) {
				const auto j = (i + 1) % 3;
				const auto k = (i + 2) % 3;

				const auto a = Vec2(A[j], A[k]);
				const auto b = Vec2(B[j], B[k]);
				const auto c = Vec2(C[j], C[k]);
				const auto AB = b - a;
				const auto AC = c - a;
				const auto Area = Mat2(AB, AC).Determinant();
				//const auto Area = AB.X() * AC.Y() - AB.Y() * AC.X();

				if (std::abs(Area) > std::abs(MaxArea)) {
					MaxArea = Area;
					MaxIndex = i;
				}
			}

			//!< 「P と三角形」を選択した平面に射影 (X が選択された場合 Index1, Index2 はそれぞれ Y, Z といった具合になる)
			const auto Index1 = (MaxIndex + 1) % 3;
			const auto Index2 = (MaxIndex + 2) % 3;
			const std::array PrjTri = { Vec2(A[Index1], A[Index2]), Vec2(B[Index1], B[Index2]), Vec2(C[Index1], C[Index2]) };
			const auto PrjP = Vec2(P[Index1], P[Index2]);

			//!< 射影点と辺からなるサブ三角形の面積
			Vec3 Areas;
			for (auto i = 0; i < 3; ++i) {
				const auto j = (i + 1) % 3;
				const auto k = (i + 2) % 3;

				const auto AB = PrjTri[j] - PrjP;
				const auto AC = PrjTri[k] - PrjP;
				Areas[i] = Mat2(AB, AC).Determinant();
				//Areas[i] = AB.X() * AC.Y() - AB.Y() * AC.X();
			}
			//!< P が [A, B, C] の内部にある場合 (サブ三角形の面積の符号から分かる)
			if (Sign(MaxArea) == Sign(Areas.X()) && Sign(MaxArea) == Sign(Areas.Y()) && Sign(MaxArea) == Sign(Areas.Z())) {
				return Areas / MaxArea;
			}

			//!< 3 辺に射影して一番近いものを見つける (1-SignedVolume に帰着)
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
		[[nodiscard]] static Vec4 SignedVolume(const Vec3& A, const Vec3& B, const Vec3& C, const Vec3& D) 
		{
			//const auto Cofactor = Vec4(-Mat3(B, C, D).Determinant(), Mat3(A, C, D).Determinant(), -Mat3(A, B, D).Determinant(), Mat3(A, B, C).Determinant());
			const auto M = Mat4(Vec4(A.X(), B.X(), C.X(), D.X()), Vec4(A.Y(), B.Y(), C.Y(), D.Y()), Vec4(A.Z(), B.Z(), C.Z(), D.Z()), Vec4::One());
			const auto Cofactor = Vec4(M.Cofactor(3, 0), M.Cofactor(3, 1), M.Cofactor(3, 2), M.Cofactor(3, 3));
			const auto Det = Cofactor.X() + Cofactor.Y() + Cofactor.Z() + Cofactor.W();

			//!< 四面体内部にあれば、重心座標を返す
			if (Sign(Det) == Sign(Cofactor.X()) && Sign(Det) == Sign(Cofactor.Y()) && Sign(Det) == Sign(Cofactor.Z()) && Sign(Det) == Sign(Cofactor.W())) {
				return Cofactor / Det;
			}

			//!< 3 面に射影して一番近いものを見つける (2-SignedVolume に帰着)
			const std::array FacePts = { A, B, C, D };
			Vec4 Lambda;
			auto MinLenSq = (std::numeric_limits<float>::max)();
			for (auto i = 0; i < 4; ++i) {
				const auto j = (i + 1) % 4;
				const auto k = (i + 2) % 4;

				const auto LambdaFace = SignedVolume(FacePts[i], FacePts[j], FacePts[k]);
				const auto LenSq = (FacePts[i] * LambdaFace[0] + FacePts[j] * LambdaFace[1] + FacePts[k] * LambdaFace[2]).LengthSq();
				if (LenSq < MinLenSq) {
					Lambda.ToZero();
					Lambda[i] = LambdaFace[0];
					Lambda[j] = LambdaFace[1];
					Lambda[k] = LambdaFace[2];
					MinLenSq = LenSq;
				}
			}
			return Lambda;
		}
#ifdef _DEBUG
		static void SignedVolumeTest()
		{
			const std::array Pts = {
				Vec3(51.1996613f, 26.1989613f, 1.91339576f),
				Vec3(-51.0567360f, -26.0565681f, -0.436143428f),
				Vec3(50.8978920f, -24.1035538f, -1.04042661f),
				Vec3(-49.1021080f, 25.8964462f, -1.04042661f)
			};
			const auto Lambda = SignedVolume(Pts[0], Pts[1], Pts[2], Pts[3]);
			const auto V = Pts[0] * Lambda[0] + Pts[1] * Lambda[1] + Pts[2] * Lambda[2] + Pts[3] * Lambda[3];

			//!< 答え合わせ
			const auto CorrectLambda = Vec4(0.290f, 0.302f, 0.206f, 0.202f);
			const auto CorrectV = Vec3::Zero();
			constexpr auto Eps = 0.001f;
			if (!Lambda.NearlyEqual(CorrectLambda, Eps)) {
				__debugbreak();
			}
			if (!V.NearlyEqual(CorrectV, Eps)) {
				__debugbreak();
			}
		}
#endif

		//!< サポートポイント : 特定の方向に最も遠い点
		class SupportPoints
		{
		public:
			SupportPoints(const Vec3& A, const Vec3& B) : Data({ A, B, A - B }) { }

			const Vec3 GetA() const { return std::get<0>(Data); }
			const Vec3 GetB() const { return std::get<1>(Data); }
			const Vec3 GetC() const { return std::get<2>(Data); }

		private:
			std::tuple<Vec3, Vec3, Vec3> Data;
		};
		//!< A, B のサポートポイントの差が、C (A, B のミンコフスキー差) のサポートポイントとなる
		static [[nodiscard]] SupportPoints GetSupportPoints(const RigidBody* RbA, const RigidBody* RbB, const Vec3& NDir, const float Bias) {
			return {
				RbA->Shape->GetSupportPoint(RbA->Position, RbA->Rotation, NDir, Bias), 
				RbB->Shape->GetSupportPoint(RbB->Position, RbB->Rotation, -NDir, Bias) 
			};
		}

		static [[nodiscard]] bool SimplexSignedVolume2(const std::vector<SupportPoints>& Sps, Vec3& Dir, Vec4& OutLambda)
		{
			//constexpr auto Eps2 = (std::numeric_limits<float>::epsilon)() * (std::numeric_limits<float>::epsilon)();
			constexpr auto Eps2 = 0.0001f * 0.00001f;

			const auto Lambda = SignedVolume(Sps[0].GetC(), Sps[1].GetC());
			Dir = -1.0f * (Sps[0].GetC() * Lambda[0] + Sps[1].GetC() * Lambda[1]);
			
			OutLambda = Lambda;

			if (Dir.LengthSq() < Eps2) {
				return true;
			}

			return false;
		}
		static [[nodiscard]] bool SimplexSignedVolume3(const std::vector<SupportPoints>& Pts, Vec3& Dir, Vec4& OutLambda)
		{
			constexpr auto Eps2 = 0.0001f * 0.00001f;

			const auto Lambda = SignedVolume(Pts[0].GetC(), Pts[1].GetC(), Pts[2].GetC());

			Dir = -1.0f * (Pts[0].GetC() * Lambda[0] + Pts[1].GetC() * Lambda[1] + Pts[2].GetC() * Lambda[2]);

			OutLambda = Lambda;

			if (Dir.LengthSq() < Eps2) {
				return true;
			}

			return false;
		}
		static [[nodiscard]] bool SimplexSignedVolume4(const std::vector<SupportPoints>& Pts, Vec3& Dir, Vec4& OutLambda)
		{
			constexpr auto Eps2 = 0.0001f * 0.00001f;

			const auto Lambda = SignedVolume(Pts[0].GetC(), Pts[1].GetC(), Pts[2].GetC(), Pts[3].GetC());

			//!< Dir の更新
			Dir = -1.0f * (Pts[0].GetC() * Lambda[0] + Pts[1].GetC() * Lambda[1] + Pts[2].GetC() * Lambda[2] + Pts[3].GetC() * Lambda[3]);
			
			OutLambda = Lambda;

			if (Dir.LengthSq() < Eps2) {
				//!< 原点を含む -> 衝突
				return true;
			}

			return false;
		}
		static [[nodiscard]] bool SimplexSignedVolumes(const std::vector<SupportPoints>& Sps, Vec3& Dir, Vec4& OutLambda)
		{
			switch (size(Sps)) {
			case 2: return SimplexSignedVolume2(Sps, Dir, OutLambda);
			case 3: return SimplexSignedVolume3(Sps, Dir, OutLambda);
			case 4: return SimplexSignedVolume4(Sps, Dir, OutLambda);
			default: return false;
			}
		}

		namespace Intersection {
			//!< #TODO 要検証			
			//!< ミンコフスキー差の凸包を生成する代わりに原点を含むような単体を生成する事で代用する
			//!< A, B のミンコフスキー差 C が原点を含めば衝突となる
			//!< A, B のサポートポイントの差が C のサポートポイントとなる
			//!<	最初のサポートポイント 1 を見つける
			//!<	原点方向の次のサポートポイント 2 を見つける
			//!<	1, 2 の線分から原点方向の次のサポートポイント 3 を見つける
			//!<	1, 2, 3 がなす三角形が原点を含めば衝突、終了
			//!<	原点を向く法線方向の次のサポートポイント 4 を見つける
			//!<	1, 2, 3, 4 がなす四面体が原点を含めば衝突、終了
			//!<	一番近い三角形 (例えば 1, 2, 4) から、原点を向く法線方向の次のサポートポイント 5 を見つける
			//!<	四面体が原点を含むか、サポートポイントが無くなるまで続ける
			[[nodiscard]] static bool GJK(const RigidBody* RbA, const RigidBody* RbB)
			{
				std::vector<SupportPoints> Sps;
				Sps.reserve(4); //!< 4 枠

				//!< (1, 1, 1) 方向のサポートポイントを求める
				Sps.emplace_back(GetSupportPoints(RbA, RbB, Vec3::One().Normalize(), 0.0f));

				auto Closest = (std::numeric_limits<float>::max)();
				auto Dir = -Sps.back().GetC();
				do {
					const auto Pt = GetSupportPoints(RbA, RbB, Dir, 0.0f);

					//!< 既存の点が返るということはもう拡張できない -> 衝突無し
					if (std::end(Sps) != std::ranges::find_if(Sps, [&](const auto& rhs) {
						return Pt.GetC().NearlyEqual(rhs.GetC());
					})) {
						break;
					}

					Sps.emplace_back(Pt);

					//!< 新しい点が原点を超えていない場合、原点が内部に含まれない -> 衝突無し
					if (Dir.Dot(Pt.GetC()) < 0.0f) {
						break;
					}

					//!< 1, 2, 3-シンプレックス毎の処理 (Dir を更新、Lambda を返す)
					Vec4 Lambda;
					if (SimplexSignedVolumes(Sps, Dir, Lambda)) {
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

					//!< 有効な (Lambda が 非 0) Sps だけを残す
					const auto [Beg, End] = std::ranges::remove_if(Sps, [&](const auto& rhs) {
						return 0.0f == Lambda[static_cast<int>(IndexOf(Sps, rhs))];
					});
					Sps.erase(Beg, End);

				} while (4 != size(Sps)); //!< 四面体でここまで来たら衝突は無い (最後は四面体で決着することになる)

				return false;
			}
		}
	}
}