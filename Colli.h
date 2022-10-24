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
		Vec3 Normal;
		
		Vec3 WorldA;
		Vec3 WorldB;

		float SeparationDistance = 0.0f;
		float TimeOfImpact = 0.0f;

		RigidBody* RigidBodyA = nullptr;
		RigidBody* RigidBodyB = nullptr;
	};
	
	static bool Intersect(const RigidBody* RbA, const RigidBody* RbB, const float DeltaSec, Contact& Ct) {
		if (RbA->Shape->GetShapeTyoe() == Shape::SHAPE::SPHERE && RbB->Shape->GetShapeTyoe() == Shape::SHAPE::SPHERE) {
			const auto SpA = static_cast<const ShapeSphere*>(RbA->Shape);
			const auto SpB = static_cast<const ShapeSphere*>(RbB->Shape);

			const auto TotalRadius = SpA->Radius + SpB->Radius;
			const auto relativeVelAB = RbA->LinearVelocity - RbB->LinearVelocity;
			float t0, t1;
			//!< �ړ������m -> ���Α��x -> ���ƃJ�v�Z�� -> (�g��)���ƃ��C�Ƃ̏Փ˂ɋA��
			if (RaySphere(RbA->Position, relativeVelAB, RbB->Position, TotalRadius, t0, t1)) {
				if (0.0f <= t1 && t1 <= 1.0f) {
					t0 = std::max<float>(t0, 0.0f);

					//!< TOI
					Ct.TimeOfImpact = t0 * DeltaSec;

					const auto cPosA = RbA->Position + RbA->LinearVelocity * Ct.TimeOfImpact;
					const auto cPosB = RbB->Position + RbB->LinearVelocity * Ct.TimeOfImpact;

					//!< �@�� A->B
					Ct.Normal = (cPosB - cPosA).Normalize();

					//!< �Փ˃��[���h�ʒu (�߂荞��ł���ꍇ������̂ŁA�K������ Ct.WorldA == Ct.WorldB �ł͂Ȃ�)
					Ct.WorldA = cPosA + Ct.Normal * SpA->Radius;
					Ct.WorldB = cPosB - Ct.Normal * SpB->Radius;

					//!< �Փˍ��̂��o���Ă���
					Ct.RigidBodyA = const_cast<RigidBody*>(RbA);
					Ct.RigidBodyB = const_cast<RigidBody*>(RbB);

					return true;
				}
			}
		}
		return false;
	}
	static void Resolve(const Contact& Ct) 
	{
		const auto TotalInvMass = Ct.RigidBodyA->InvMass + Ct.RigidBodyB->InvMass;
		{
			//!< �Փ˓_�Əd�S�����Ԕ��a
			const auto rA = Ct.WorldA - Ct.RigidBodyA->ToWorld(Ct.RigidBodyA->GetCenterOfMass());
			const auto rB = Ct.WorldB - Ct.RigidBodyB->ToWorld(Ct.RigidBodyB->GetCenterOfMass());
			{
				const auto velAB = (Ct.RigidBodyB->LinearVelocity + Ct.RigidBodyB->AngularVelocity.Cross(rB)) - (Ct.RigidBodyA->LinearVelocity + Ct.RigidBodyA->AngularVelocity.Cross(rA));

				//!< ���x�̖@������
				const auto velABNrm = velAB.Dot(Ct.Normal);

				const auto invWorldInertiaA = Ct.RigidBodyA->ToWorld(Ct.RigidBodyA->InvInertiaTensor);
				const auto invWorldInertiaB = Ct.RigidBodyB->ToWorld(Ct.RigidBodyB->InvInertiaTensor);

				//!< �͐� (�@������)
				{
					//!< 1) �@������
					const auto inertiaA = (invWorldInertiaA * rA.Cross(Ct.Normal)).Cross(rA);
					const auto inertiaB = (invWorldInertiaB * rB.Cross(Ct.Normal)).Cross(rB);
					const auto invInertia = (inertiaA + inertiaB).Dot(Ct.Normal);

					const auto TotalElasticity = Ct.RigidBodyA->Elasticity * Ct.RigidBodyB->Elasticity;
					//!< TODO
					const auto J = Ct.Normal * ((1.0f + TotalElasticity) * velABNrm / (TotalInvMass + invInertia));

					Ct.RigidBodyA->ApplyImpulse(Ct.WorldA, J);
					Ct.RigidBodyB->ApplyImpulse(Ct.WorldB, -J);
				}

				//!< ���C�� (�ڐ������̗͐ςƂ��Ĉ���)
				{
					//!< ���x�̐ڐ�����
					const auto velTan = velAB - Ct.Normal * velABNrm;
					const auto Tangent = velTan.Normalize();

					//!< 2) �ڐ����� ... 1) �Ɠ������A�ڐ������ɂ�������
					const auto inertiaA = (invWorldInertiaA * rA.Cross(Tangent)).Cross(rA);
					const auto inertiaB = (invWorldInertiaB * rB.Cross(Tangent)).Cross(rB);
					const auto invInertia = (inertiaA + inertiaB).Dot(Tangent);

					const auto TotalFriction = Ct.RigidBodyA->Friction * Ct.RigidBodyB->Friction;
					const auto J = velTan * (TotalFriction / (TotalInvMass + invInertia));

					Ct.RigidBodyA->ApplyImpulse(Ct.WorldA, J);
					Ct.RigidBodyB->ApplyImpulse(Ct.WorldB, -J);
				}
			}
		}

		//!< �߂荞�݂̒ǂ��o�� (TOI���X���C�X���Ă߂荞�܂Ȃ��悤�ɃV�~�����[�V������i�߂Ă��邽�߁A�߂荞�މ\��������̂� TOI �� 0 �̎�)
		if(0.0f == Ct.TimeOfImpact) {
			const auto dAB = Ct.WorldB - Ct.WorldA;
			Ct.RigidBodyA->Position += dAB * (Ct.RigidBodyA->InvMass / TotalInvMass);
			Ct.RigidBodyA->Position -= dAB * (Ct.RigidBodyB->InvMass / TotalInvMass);
		}
	}
}