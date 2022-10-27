#pragma once

#include <vector>
#ifdef _DEBUG
#include <iostream>
#endif

#include "Math.h"
using namespace Math;

#include "Shape.h"

namespace Phys
{
	class RigidBody
	{
	public:
		virtual ~RigidBody() {
			if (nullptr != Shape) {
				delete Shape;
			}
		}

		void Init(Shape* Sh) {
			Shape = Sh;
			InvInertiaTensor = GetInertiaTensor().Inverse() * InvMass;
		}

		Vec3 GetCenterOfMass() const { return Shape->GetCenterOfMass(); };
		Vec3 GetWorldSpaceCenterOfMass() const { return Position + Rotation.Rotate(GetCenterOfMass()); }

		Mat3 GetInertiaTensor() const { return Shape->GetInertiaTensor(); }

		Vec3 ToLocal(const Vec3& rhs) const {
			return Rotation.Inverse().Rotate(rhs - GetWorldSpaceCenterOfMass());
		}
		Vec3 ToWorld(const Vec3& rhs) const {
			return GetWorldSpaceCenterOfMass() + Rotation.Rotate(rhs);
		}
		Mat3 ToWorld(const Mat3& rhs) const {
			const auto Rot3 = static_cast<const Mat3>(Rotation);
			//!< �{���� Rot3 * rhs * Rot3.Inverse() �����A�X�P�[�����O�������̂� Rot3 * rhs * Rot3.Transpose() �ŗǂ�
			return Rot3 * rhs * Rot3.Transpose();
		}

		void ApplyGravity(const float DeltaSec) {
			if (0.0f != InvMass) {
				LinearVelocity += Graivity * DeltaSec;
			}
		}
		void ApplyLinearImpulse(const Vec3& Impulse) {
			if (0.0f != InvMass) {
				LinearVelocity += Impulse * InvMass;
			}
		}
		void ApplyAngularImpulse(const Vec3& Impulse) {
			if (0.0f != InvMass) {
				//!< w = Inv(I) * AngJ 
				AngularVelocity += ToWorld(InvInertiaTensor) * Impulse;

#pragma region �p���x���E�l
				//constexpr auto AngVelLim = 30.0f;
				//if (AngularVelocity.LengthSq() > AngVelLim * AngVelLim) {
				//	AngularVelocity.ToNormalized();
				//	AngularVelocity *= AngVelLim;
				//}
#pragma endregion
			}
		}

		void ApplyImpulse(const Vec3& ImpactPoint, const Vec3& Impulse) {
			if (0.0f != InvMass) {
				ApplyLinearImpulse(Impulse);
				//!< AngJ = r x LinearJ
				ApplyAngularImpulse((ImpactPoint - GetWorldSpaceCenterOfMass()).Cross(Impulse));
			}
		}

		void Update(const float DeltaSec) {
			if (0.0f != InvMass) {
				{
					//!< �ʒu�̍X�V
					Position += LinearVelocity * DeltaSec;
				}

				{
					//!< �p�����x a = Inv(I) * (w x (I �E w))
					const auto WIT = ToWorld(GetInertiaTensor());
					const auto AngAccel = WIT.Inverse() * (AngularVelocity.Cross(WIT * AngularVelocity));
					//!< �p���x
					AngularVelocity += AngAccel * DeltaSec;

					//!< ��]�̍X�V Quat' = dQuat * Quat
					const auto DeltaAng = AngularVelocity * DeltaSec;
					const auto DeltaQuat = Quat(DeltaAng, DeltaAng.Length());
					Rotation = (DeltaQuat * Rotation).Normalize();

					//!< (��]�ɂ��) �ʒu�̍X�V
					const auto WCOM = GetWorldSpaceCenterOfMass();
					Position = WCOM + DeltaQuat.Rotate(Position - WCOM);
				}
			}
		}

		Vec3 Position = Vec3::Zero();
		Quat Rotation = Quat::Identity();

		Vec3 LinearVelocity = Vec3::Zero();
		Vec3 AngularVelocity = Vec3::Zero();

		float InvMass = 1.0f;
		Mat3 InvInertiaTensor = Mat3::Identity();

		float Elasticity = 0.5f;
		float Friction = 0.5f;

		Vec3 Graivity = Vec3(0.0f, -9.8f, 0.0f);

		Shape* Shape = nullptr;
	};
}