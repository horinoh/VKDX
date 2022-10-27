#pragma once

#include "Math.h"
using namespace Math;

#include <algorithm>

#include "Shape.h"
#include "RigidBody.h"
using namespace Phys;

namespace Colli
{
	static bool RaySphere(const Vec3& RayPos, const Vec3& RayDir, const Vec3& SpPos, const float SpRad, float& t0, float& t1) 
	{
		//!< 1)��	(x - SpPos)^2 = SpRad^2 
		//!< 2)���C	RayPos + RayDir * t 
		//!<	1) �� x �� 2) ���� 
		//!< (RayPos + RayDir * t - SpPos)^2 = SpRad^2
		//!< RayDir^2 * t^2 + 2 * (RayDir.Dot(RayPos - SpPos) * t + (RayPos - SpPos)^2 - SpRad^2 = 0
		//!< A * t^2 + B * t + C = 0 �Ƃ����
		//!<	A = RayDir.Dot(RayDir), B/2 = RayDir.Dot(Tmp), C = Tmp.Dot(Tmp) - SpRad^2, ������ Tmp = SpPos - RayPos
		const auto Tmp = SpPos - RayPos;
		const auto A = RayDir.Dot(RayDir);
		const auto B2 = RayDir.Dot(Tmp);
		const auto C = Tmp.Dot(Tmp) - SpRad * SpRad;

		const auto D4 = B2 * B2 - A * C;
		if (D4 >= 0) {
			const auto D4Sqrt = sqrt(D4);
			t0 = (-B2 - D4Sqrt) / A;
			t1 = (-B2 + D4Sqrt) / A;
			return true;
		}
		return false;
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
			const auto VelAB = RbA->LinearVelocity - RbB->LinearVelocity;
			// TODO
			//const auto VelAB = (RbA->LinearVelocity - RbB->LinearVelocity) * DeltaSec;

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
			if (RaySphere(RbA->Position, VelAB, RbB->Position, TotalRadius, t0, t1)) {
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
		if(0.0f == Ct.TimeOfImpact) {
			const auto dAB = Ct.PointB - Ct.PointA;
			Ct.RigidBodyA->Position += dAB * (Ct.RigidBodyA->InvMass / TotalInvMass);
			Ct.RigidBodyA->Position -= dAB * (Ct.RigidBodyB->InvMass / TotalInvMass);
		}
	}
}