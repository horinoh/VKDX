#pragma once

#include "Math.h"
#include "Bound.h"

using namespace Math;
using namespace Colli;

namespace Phys 
{
	class Shape 
	{
	public:
		virtual ~Shape() {}

		enum class SHAPE
		{
			SPHERE,
		};
		virtual SHAPE GetShapeTyoe() const = 0;

		virtual Vec3 GetCenterOfMass() const = 0;
		virtual Mat3 GetInertiaTensor() const = 0;

		virtual AABB GetAABB(const Vec3& Pos, const Quat& Rot) const = 0;

		AABB Aabb;
	};

	class ShapeSphere : public Shape 
	{
	public:
		ShapeSphere() {}
		ShapeSphere(const float R) : Radius(R) {}
		virtual ~ShapeSphere() {}

		virtual SHAPE GetShapeTyoe() const override { return SHAPE::SPHERE; };

		virtual Vec3 GetCenterOfMass() const override { return Vec3::Zero(); };
		virtual Mat3 GetInertiaTensor() const override { 
			return {
				{ 2.0f * Radius * Radius / 5.0f, 0.0f, 0.0f }, 
				{ 0.0f, 2.0f * Radius * Radius / 5.0f, 0.0f },
				{ 0.0f, 0.0f, 2.0f * Radius * Radius / 5.0f }
			};
		}

		virtual AABB GetAABB(const Vec3& Pos, [[maybe_unused]] const Quat& Rot) const override {
			return { Pos - Vec3(Radius), Pos + Vec3(Radius) };
		}

		float Radius = 1.0f;
	};
}