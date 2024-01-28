#pragma once

#include <vector>
#ifdef _DEBUG
#include <iostream>
#endif

#include "Math.h"
using namespace Math;

#include "Shape.h"

namespace Physics
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

		[[nodiscard]] Vec3 GetCenterOfMass() const { return Shape->GetCenterOfMass(); };
		[[nodiscard]] Vec3 GetWorldSpaceCenterOfMass() const { return Position + Rotation.Rotate(GetCenterOfMass()); }

		[[nodiscard]] Mat3 GetInertiaTensor() const { return Shape->GetInertiaTensor(); }
		[[nodiscard]] Mat3 GetInverseInertiaTensor() const { return InvInertiaTensor; }
		[[nodiscard]] Mat3 GetWorldSpaceInverseInertiaTensor() const { return ToWorld(GetInverseInertiaTensor()); }

		[[nodiscard]] Vec3 ToLocal(const Vec3& rhs) const {
			return Rotation.Inverse().Rotate(rhs - GetWorldSpaceCenterOfMass());
		}
		[[nodiscard]] Vec3 ToWorld(const Vec3& rhs) const {
			return GetWorldSpaceCenterOfMass() + Rotation.Rotate(rhs);
		}
		[[nodiscard]] Mat3 ToWorld(const Mat3& rhs) const {
			const auto Rot3 = static_cast<const Mat3>(Rotation);
			//!< 本来は Rot3 * rhs * Rot3.Inverse() だが、スケーリングが無いので Rot3 * rhs * Rot3.Transpose() で良い
			return Rot3 * rhs * Rot3.Transpose();
		}

		void ApplyGravity(const float DeltaSec) {
#if 1
			if (0.0f != InvMass) {
				LinearVelocity += Graivity * DeltaSec;				
			}
#else
			//!< 真面目にやるとこうだが、計算量が増えるだけ
			ApplyLinearImpulse(Graivity * DeltaSec / InvMass);
#endif
		}
		void ApplyLinearImpulse(const Vec3& Impulse) {
			if (0.0f != InvMass) {
				LinearVelocity += Impulse * InvMass;
			}
		}
		void ApplyAngularImpulse(const Vec3& Impulse) {
			if (0.0f != InvMass) {
				//!< w = Inv(I) * AngularJ 
				AngularVelocity += GetWorldSpaceInverseInertiaTensor() * Impulse;

				//!< 角速度に限界値を設ける場合
				//constexpr auto AngVelLim = 30.0f;
				//if (AngularVelocity.LengthSq() > AngVelLim * AngVelLim) {
				//	AngularVelocity.ToNormalized();
				//	AngularVelocity *= AngVelLim;
				//}
			}
		}

		void ApplyTotalImpulse(const Vec3& ImpactPoint, const Vec3& Impulse) {
			if (0.0f != InvMass) {
				ApplyLinearImpulse(Impulse);
				//!< AngularJ = Radius x LinearJ
				const auto Radius = ImpactPoint - GetWorldSpaceCenterOfMass();
				ApplyAngularImpulse(Radius.Cross(Impulse));
			}
		}

		void Update(const float DeltaSec) {
			if (0.0f != InvMass) {
				//!< (速度による) 位置の更新
				{
					Position += LinearVelocity * DeltaSec;
				}

				//!< (角速度による) 位置、回転の更新
				{
					//!< 角加速度 AngAccel = Inv(I) * (w x (I ・ w))
					const auto WorldInertia = ToWorld(GetInertiaTensor());
					const auto AngAccel = WorldInertia.Inverse() * (AngularVelocity.Cross(WorldInertia * AngularVelocity));
					//!< 角速度
					AngularVelocity += AngAccel * DeltaSec;

					//!< 回転の更新 Quat' = dQuat * Quat
					const auto DeltaAng = AngularVelocity * DeltaSec;
					const auto DeltaQuat = Quat(DeltaAng, DeltaAng.Length());
					Rotation = (DeltaQuat * Rotation).Normalize();

					//!< 位置の更新
					const auto WorldCenter = GetWorldSpaceCenterOfMass();
					Position = WorldCenter + DeltaQuat.Rotate(Position - WorldCenter);
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

		inline static const Vec3 Graivity = Vec3(0.0f, -9.8f, 0.0f);

		Shape* Shape = nullptr;
	};
}