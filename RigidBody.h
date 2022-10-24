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

		Vec3 GetCenterOfMass() const { return Shape->GetCenterOfMass(); }; //!< Local Space
		Mat3 GetInertiaTensor() const { return Shape->GetInertiaTensor(); } //!< Local Space

		Vec3 ToLocal(const Vec3& rhs) const {
			return Rotation.Inverse().Rotate(rhs - GetCenterOfMass());
		}
		Vec3 ToWorld(const Vec3& rhs) const {
			return Position + Rotation.Rotate(rhs);
		}
		Mat3 ToWorld(const Mat3& rhs) const {
			const auto Rot3 = static_cast<const Mat3>(Rotation);
			//!< �{���� Rot3 * rhs * Rot3.Inverse() �����A�X�P�[�����O�������̂� Rot3 * rhs * Rot3.Transpose() �ŗǂ�
			return Rot3 * rhs * Rot3.Transpose();
		}

		void ApplyGravity(const float DeltaSec) {
			if (0.0f != InvMass) {
				LinearVelocity += Graivity * DeltaSec * InvMass;
			}
		}
		void ApplyLinearImpulse(const Vec3& Impulse) {
			if (0.0f != InvMass) {
				LinearVelocity += Impulse * InvMass;
			}
		}
		void ApplyAngularImpulse(const Vec3& Impulse) {
			//!< dw = I^-1 * J 
			if (0.0f != InvMass) {
				AngularVelocity += ToWorld(InvInertiaTensor) * Impulse;

#pragma region �p���x���E�l
				constexpr auto AngVelLim = 30.0f;
				if (AngularVelocity.LengthSq() > AngVelLim * AngVelLim) {
					AngularVelocity.ToNormalized();
					AngularVelocity *= AngVelLim;
				}
#pragma endregion
			}
		}

		void ApplyImpulse(const Vec3& ImpactPoint, const Vec3& Impulse) {
			if (0.0f != InvMass) {
				ApplyLinearImpulse(Impulse);
				//!< J_ang = r X J_lin
				ApplyAngularImpulse((ToWorld(GetCenterOfMass()) - ImpactPoint).Cross(Impulse));
			}
		}

		void Update(const float DeltaSec) {
			if (0.0f != InvMass) {
				//!< ���`���x����ʒu�����߂�
				Position += LinearVelocity * DeltaSec;

				{
					//!< �p���x�����]�����߂�
					const auto Rot3 = Rotation.ToMat3();
					const auto InertiaTensor = Rot3 * GetInertiaTensor() * Rot3.Transpose();
					//!< �e�����x a = I^-1 (w X (I * w))
					const auto Accel = InertiaTensor.Inverse() * (AngularVelocity.Cross(InertiaTensor * AngularVelocity));
					//!< �p���x
					AngularVelocity += Accel * DeltaSec;
					//!< q' = dq * q0
					const auto dAng = AngularVelocity * DeltaSec;
					const auto dQuat = Quat(dAng, dAng.Length());
					Rotation = (dQuat * Rotation).Normalize();

					//!< ��]�ɂ��ʒu�̍X�V
					const auto CM = ToWorld(GetCenterOfMass());
					Position = CM + dQuat.Rotate(Position - CM);
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