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
			BOX,
			CONVEX,
		};
		[[nodiscard]] virtual SHAPE GetShapeTyoe() const = 0;

		[[nodiscard]] virtual Vec3 GetCenterOfMass() const = 0;
		[[nodiscard]] virtual Mat3 GetInertiaTensor() const = 0;

		[[nodiscard]] virtual AABB GetAABB(const Vec3& Pos, const Quat& Rot) const = 0;

#pragma region GJK
		//!< 指定の方向(NormalizedDir : 正規化されていること)に一番遠い点を返す
		[[nodiscard]] virtual Vec3 GetSupportPoint(const Vec3& Pos, const Quat& Rot, const Vec3& NormalizedDir, const float Bias) const = 0;
		//virtual float GetFastestPointSpeed(const Vec3& AngVel, const Vec3& Dir) const { return 0.0f; }
#pragma endregion

	public:
		AABB Aabb;
	};

	class ShapeSphere : public Shape 
	{
	public:
		ShapeSphere() {}
		ShapeSphere(const float R) : Radius(R) {}
		virtual ~ShapeSphere() {}

		virtual SHAPE GetShapeTyoe() const override { return SHAPE::SPHERE; }

		virtual Vec3 GetCenterOfMass() const override { return Vec3::Zero(); }
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

#pragma region GJK
		virtual Vec3 GetSupportPoint(const Vec3& Pos, const Quat& Rot, const Vec3& NormalizedDir, const float Bias) const override {
			return Pos + NormalizedDir * (Radius + Bias);
		}
#pragma endregion

	public:
		float Radius = 1.0f;
	};

	class ShapeBox : public Shape
	{
	public:
		ShapeBox() {}
		virtual ~ShapeBox() {}

		virtual SHAPE GetShapeTyoe() const override { return SHAPE::BOX; }

		virtual Vec3 GetCenterOfMass() const override { return Vec3::Zero(); }
		virtual Mat3 GetInertiaTensor() const override {
			const auto W2 = Extent.X() * Extent.X(), H2 = Extent.Y() * Extent.Y(), D2 = Extent.Z() * Extent.Z();
			return {
				{ (H2 + D2) / 12.0f, 0.0f, 0.0f },
				{ 0.0f, (W2 + D2) / 12.0f, 0.0f },
				{ 0.0f, 0.0f, (W2 + D2) / 12.0f }
			};
		}

		virtual AABB GetAABB(const Vec3& Pos, [[maybe_unused]] const Quat& Rot) const override {
			const std::array Points = {
				Extent,
				Vec3(Extent.X(), Extent.Y(), -Extent.Z()),
				Vec3(Extent.X(), -Extent.Y(), Extent.Z()),
				Vec3(Extent.X(), -Extent.Y(), -Extent.Z()),
				Vec3(-Extent.X(), Extent.Y(), Extent.Z()),
				Vec3(-Extent.X(), Extent.Y(), -Extent.Z()),
				Vec3(-Extent.X(), -Extent.Y(), Extent.Z()),
				-Extent };

			AABB Aabb;
			for (auto& i : Points) {
				Aabb.Expand(Rot.Rotate(i) + Pos);
			}

			return Aabb;
		}

#pragma region GJK
		virtual Vec3 GetSupportPoint(const Vec3& Pos, const Quat& Rot, const Vec3& NormalizedDir, const float Bias) const override {
			const std::array Points = { 
				Extent,
				Vec3(Extent.X(), Extent.Y(), -Extent.Z()),
				Vec3(Extent.X(), -Extent.Y(), Extent.Z()), 
				Vec3(Extent.X(), -Extent.Y(), -Extent.Z()),
				Vec3(-Extent.X(), Extent.Y(), Extent.Z()), 
				Vec3(-Extent.X(), Extent.Y(), -Extent.Z()), 
				Vec3(-Extent.X(), -Extent.Y(), Extent.Z()),
				-Extent };

			const auto MaxPt = std::ranges::max_element(Points, [&](const auto lhs, const auto rhs) {
				return NormalizedDir.Dot(Rot.Rotate(lhs) + Pos) < NormalizedDir.Dot(Rot.Rotate(rhs) + Pos);
			});
			return *MaxPt + NormalizedDir * Bias;
		}

		//!< 指定の方向に最も速く動いている頂点の速度を返す
		float GetFastestPointSpeed(const Vec3& AngVel, const Vec3& Dir) const {
			const std::array Points = {
				Extent,
				Vec3(Extent.X(), Extent.Y(), -Extent.Z()),
				Vec3(Extent.X(), -Extent.Y(), Extent.Z()),
				Vec3(Extent.X(), -Extent.Y(), -Extent.Z()),
				Vec3(-Extent.X(), Extent.Y(), Extent.Z()),
				Vec3(-Extent.X(), Extent.Y(), -Extent.Z()),
				Vec3(-Extent.X(), -Extent.Y(), Extent.Z()),
				-Extent };

			auto MaxSpeed = 0.0f;
			for (auto& i : Points) {
				const auto Speed = Dir.Dot(AngVel.Cross(i - GetCenterOfMass()));
				if (Speed > MaxSpeed) {
					MaxSpeed = Speed;
				}
			}
			return MaxSpeed;
		}
#pragma endregion

	public:
		Vec3 Extent = Vec3::One();
	};

	class ShapeConvex : public Shape
	{
	public:
		ShapeConvex() {}
		virtual ~ShapeConvex() {}

		virtual SHAPE GetShapeTyoe() const override { return SHAPE::CONVEX; }

		virtual Vec3 GetCenterOfMass() const override { return Vec3::Zero(); }
		virtual Mat3 GetInertiaTensor() const override {
			//!< TODO
			return Mat3::Identity();
		}

		virtual AABB GetAABB(const Vec3& Pos, [[maybe_unused]] const Quat& Rot) const override {
			AABB Aabb;
			for (auto& i : Points) {
				Aabb.Expand(Rot.Rotate(i) + Pos);
			}
			return Aabb;
		}

#pragma region GJK
		virtual Vec3 GetSupportPoint(const Vec3& Pos, const Quat& Rot, const Vec3& NormalizedDir, const float Bias) const override {
			return *std::ranges::max_element(Points, [&](const auto lhs, const auto rhs) { return NormalizedDir.Dot(Rot.Rotate(lhs) + Pos) < NormalizedDir.Dot(Rot.Rotate(rhs) + Pos); }) + NormalizedDir * Bias;
		}

		//!< 指定の方向に最も速く動いている頂点の速度を返す
		float GetFastestPointSpeed(const Vec3& AngVel, const Vec3& Dir) const {
			auto MaxSpeed = 0.0f;
			for (auto& i : Points) {
				const auto Speed = Dir.Dot(AngVel.Cross(i - GetCenterOfMass()));
				if (Speed > MaxSpeed) {
					MaxSpeed = Speed;
				}
			}
			return MaxSpeed;
		}
#pragma endregion

	public:
		std::vector<Vec3> Points;
	};
}