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
		//!< SignedVolue : �ˉe���ő�ƂȂ鎲�╽�ʂ������A����ɑ΂����_���ˉe���ē����ɂ���Ώd�S��Ԃ�

		[[nodiscard]] static Vec2 SignedVolume(const Vec3& A, const Vec3& B) 
		{
			const auto AB = B - A;
			const auto N = AB.Normalize();
			//!< ���_�� AB ���Ɏˉe�� P �Ƃ���
			const auto P = A + N * N.Dot(-A);

			//!< X, Y, Z ���̓��A��Βl���ő�̂��̂�������
			auto MaxIndex = 0;
			auto MaxSeg = 0.0f;
			for (auto i = 0; i < 3; ++i) {
				if (std::abs(MaxSeg) < std::abs(AB[i])) {
					MaxSeg = AB[i];
					MaxIndex = i;
				}
			}

			//!< �uP �Ɛ����v��I���������֎ˉe
			const std::array PrjSeg = { A[MaxIndex] , B[MaxIndex] };
			const auto PrjP = P[MaxIndex];

			//!< P �� [A, B] �̓����ɂ���ꍇ
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
		[[nodiscard]] static Vec3 SignedVolume(const Vec3& A, const Vec3& B, const Vec3& C) 
		{
			const auto N = (B - A).Cross(C - A).Normalize();
			const auto P = N * A.Dot(N);

			//!<  XY, YZ, ZX ���ʂ̓��A�ˉe�ʐς��ő�̂��̂�������
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

			//!< �uP �ƎO�p�`�v��I���������ʂɎˉe (X ���I�����ꂽ�ꍇ Index1, Index2 �͂��ꂼ�� Y, Z �Ƃ�������ɂȂ�)
			const auto Index1 = (MaxIndex + 1) % 3;
			const auto Index2 = (MaxIndex + 2) % 3;
			const std::array PrjTri = { Vec2(A[Index1], A[Index2]), Vec2(B[Index1], B[Index2]), Vec2(C[Index1], C[Index2]) };
			const auto PrjP = Vec2(P[Index1], P[Index2]);

			//!< �ˉe�_�ƕӂ���Ȃ�T�u�O�p�`�̖ʐ�
			Vec3 Areas;
			for (auto i = 0; i < 3; ++i) {
				const auto j = (i + 1) % 3;
				const auto k = (i + 2) % 3;

				const auto AB = PrjTri[j] - PrjP;
				const auto AC = PrjTri[k] - PrjP;
				Areas[i] = Mat2(AB, AC).Determinant();
				//Areas[i] = AB.X() * AC.Y() - AB.Y() * AC.X();
			}
			//!< P �� [A, B, C] �̓����ɂ���ꍇ (�T�u�O�p�`�̖ʐς̕������番����)
			if (Sign(MaxArea) == Sign(Areas.X()) && Sign(MaxArea) == Sign(Areas.Y()) && Sign(MaxArea) == Sign(Areas.Z())) {
				return Areas / MaxArea;
			}

			//!< 3 �ӂɎˉe���Ĉ�ԋ߂����̂������� (1-SignedVolume �ɋA��)
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

			//!< �l�ʑ̓����ɂ���΁A�d�S���W��Ԃ�
			if (Sign(Det) == Sign(Cofactor.X()) && Sign(Det) == Sign(Cofactor.Y()) && Sign(Det) == Sign(Cofactor.Z()) && Sign(Det) == Sign(Cofactor.W())) {
				return Cofactor / Det;
			}

			//!< 3 �ʂɎˉe���Ĉ�ԋ߂����̂������� (2-SignedVolume �ɋA��)
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

			//!< �������킹
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

		//!< �T�|�[�g�|�C���g : ����̕����ɍł������_
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
		//!< A, B �̃T�|�[�g�|�C���g�̍����AC (A, B �̃~���R�t�X�L�[��) �̃T�|�[�g�|�C���g�ƂȂ�
		static [[nodiscard]] SupportPoints GetSupportPoints(const RigidBody* RbA, const RigidBody* RbB, const Vec3& NDir, const float Bias) {
			return { RbA->Shape->GetSupportPoint(RbA->Position, RbA->Rotation, NDir, Bias), RbB->Shape->GetSupportPoint(RbB->Position, RbB->Rotation, NDir, Bias) };
		}

		static [[nodiscard]] bool SimplexSignedVolume2(std::vector<SupportPoints>& Sps, Vec3& Dir, Vec4& OutLambda)
		{
			//constexpr auto Eps2 = (std::numeric_limits<float>::epsilon)() * (std::numeric_limits<float>::epsilon)();
			constexpr auto Eps2 = 0.0001f * 0.00001f;
			constexpr auto Eps2 = 0.0001f * 0.00001f;

			const auto Lambda = SignedVolume(Sps[0].GetDiff(), Sps[1].GetDiff());
			Dir = -1.0f * (Sps[0].GetDiff() * Lambda[0] + Sps[1].GetDiff() * Lambda[1]);
			
			OutLambda[0] = Lambda[0];
			OutLambda[1] = Lambda[1];

			if (Dir.LengthSq() < Eps2) {
				return true;
			}

			const auto [Beg, End] = std::ranges::remove_if(Sps, [&](const auto& rhs) {
				return 0.0f == Lambda[static_cast<int>(IndexOf(Sps, rhs))];
			});
			Sps.erase(Beg, End);
			
			return false;
		}
		static [[nodiscard]] bool SimplexSignedVolume3(std::vector<SupportPoints>& Pts, Vec3& Dir, Vec4& OutLambda)
		{
			constexpr auto Eps2 = 0.0001f * 0.00001f;

			const auto Lambda = SignedVolume(Pts[0].GetDiff(), Pts[1].GetDiff(), Pts[2].GetDiff());

			Dir = -1.0f * (Pts[0].GetDiff() * Lambda[0] + Pts[1].GetDiff() * Lambda[1] + Pts[2].GetDiff() * Lambda[2]);

			OutLambda[0] = Lambda[0];
			OutLambda[1] = Lambda[1];
			OutLambda[2] = Lambda[2];

			if (Dir.LengthSq() < Eps2) {
				return true;
			}

			const auto [Beg, End] = std::ranges::remove_if(Pts, [&](const auto& rhs) {
				return 0.0f == Lambda[static_cast<int>(IndexOf(Pts, rhs))];
			});
			Pts.erase(Beg, End);

			return false;
		}
		static [[nodiscard]] bool SimplexSignedVolume4(std::vector<SupportPoints>& Pts, Vec3& Dir, Vec4& OutLambda)
		{
			constexpr auto Eps2 = 0.0001f * 0.00001f;

			const auto Lambda = SignedVolume(Pts[0].GetDiff(), Pts[1].GetDiff(), Pts[2].GetDiff(), Pts[3].GetDiff());

			//!< Dir �̍X�V
			Dir = -1.0f * (Pts[0].GetDiff() * Lambda[0] + Pts[1].GetDiff() * Lambda[1] + Pts[2].GetDiff() * Lambda[2] + Pts[3].GetDiff() * Lambda[3]);
			
			OutLambda[0] = Lambda[0];
			OutLambda[1] = Lambda[1];
			OutLambda[2] = Lambda[2];
			OutLambda[3] = Lambda[3];

			if (Dir.LengthSq() < Eps2) {
				//!< ���_���܂� -> �Փ�
				return true;
			}

			//!< 0.0f == Lambda[i] �ƂȂ� SimplexPoints[i] �͍폜
			const auto [Beg, End] = std::ranges::remove_if(Pts, [&](const auto& rhs) {
				return 0.0f == Lambda[static_cast<int>(IndexOf(Pts, rhs))];
			});
			Pts.erase(Beg, End);

			return false;
		}
		static [[nodiscard]] bool SimplexSignedVolumes(std::vector<SupportPoints>& Sps, Vec3& Dir, Vec4& OutLambda)
		{
			switch (size(Sps)) {
			case 2: return SimplexSignedVolume2(Sps, Dir, OutLambda);
			case 3: return SimplexSignedVolume3(Sps, Dir, OutLambda);
			case 4: return SimplexSignedVolume4(Sps, Dir, OutLambda);
			default:
				assert(false && "");
				return false;
				break;
			}
		}

		namespace Intersection {
			//!< #TODO �v����			
			//!< �~���R�t�X�L�[���̓ʕ�𐶐��������Ɍ��_���܂ނ悤�ȒP�̂𐶐����鎖�ő�p����
			//!< A, B �̃~���R�t�X�L�[�� C �����_���܂߂ΏՓ˂ƂȂ�
			//!< A, B �̃T�|�[�g�|�C���g�̍��� C �̃T�|�[�g�|�C���g�ƂȂ�
			//!<	�ŏ��̃T�|�[�g�|�C���g 1 ��������
			//!<	���_�����̎��̃T�|�[�g�|�C���g 2 ��������
			//!<	1, 2 �̐������猴�_�����̎��̃T�|�[�g�|�C���g 3 ��������
			//!<	1, 2, 3 ���Ȃ��O�p�`�����_���܂߂ΏՓˁA�I��
			//!<	���_�������@�������̎��̃T�|�[�g�|�C���g 4 ��������
			//!<	1, 2, 3, 4 ���Ȃ��l�ʑ̂����_���܂߂ΏՓˁA�I��
			//!<	��ԋ߂��O�p�` (�Ⴆ�� 1, 2, 4) ����A���_�������@�������̎��̃T�|�[�g�|�C���g 5 ��������
			//!<	�l�ʑ̂����_���܂ނ��A�T�|�[�g�|�C���g�������Ȃ�܂ő�����
			[[nodiscard]] static bool GJK(const RigidBody* RbA, const RigidBody* RbB)
			{
				std::vector<SupportPoints> Sps;
				//!< 4 �g�K�v
				Sps.reserve(4);

				//!< (1, 1, 1) �����̃T�|�[�g�|�C���g�����߂�
				Sps.emplace_back(GetSupportPoints(RbA, RbB, Vec3::One().Normalize(), 0.0f));

				auto Closest = (std::numeric_limits<float>::max)();
				auto Dir = Sps.back().GetDiff();
				do {
					const auto Pt = GetSupportPoints(RbA, RbB, Dir, 0.0f);

					//!< �����̓_���Ԃ�Ƃ������Ƃ͂����g���ł��Ȃ� -> �Փ˖���
					if (std::end(Sps) != std::ranges::find_if(Sps, [&](const auto& rhs) {
						return Pt.GetDiff().NearlyEqual(rhs.GetDiff()); 
					})) {
						break;
					}

					Sps.emplace_back(Pt);

					//!< �V�����_�����_�𒴂��Ă��Ȃ��ꍇ�A���_�������Ɋ܂܂�Ȃ� -> �Փ˖���
					if (Dir.Dot(Pt.GetDiff()) >= 0.0f) {
						break;
					}

					//!< 1, 2, 3-�V���v���b�N�X���̏���
					Vec4 Lambda;
					if (SimplexSignedVolumes(Sps, Dir, Lambda)) {
						return true;
					}

					//!< �ŒZ�������X�V�A�X�V�ł��Ȃ�ΏI��
					const auto DistSq = Dir.LengthSq();
					if (DistSq < Closest) {
						Closest = DistSq;
					}
					else {
						break;
					}

					//!< �L���� Sps (Lambda �� �� 0) �������c��
					const auto [Beg, End] = std::ranges::remove_if(Sps, [&](const auto& rhs) {
						return 0.0f == Lambda[static_cast<int>(IndexOf(Sps, rhs))];
					});
					Sps.erase(Beg, End);

				} while (4 != size(Sps)); //!< �l�ʑ̂ł����܂ŗ�����Փ˂͖��� (�Ō�͎l�ʑ̂Ō������邱�ƂɂȂ�)

				return false;
			}
		}
	}
}