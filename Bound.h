#pragma once

#include <algorithm>

#include "Vec.h"
using namespace Math;

namespace Collision
{
	class AABB
	{
	public:
		AABB() { Clear(); }
		AABB(const Vec3& minVal, const Vec3& maxVal) : Min(minVal), Max(maxVal) {}
		AABB(const AABB& rhs) : Min(rhs.Min), Max(rhs.Max) {}
		AABB& operator=(const AABB& rhs) { Min = rhs.Min; Max = rhs.Max; return *this; }

		AABB& Clear() {
			Min = Vec3::Max();
			Max = Vec3::Min();
			return *this;
		}
		AABB& Expand(const Vec3& rhs) {
			Min = { (std::min)(Min.X(), rhs.X()), (std::min)(Min.Y(), rhs.Y()), (std::min)(Min.Z(), rhs.Z()) };
			Max = { (std::max)(Max.X(), rhs.X()), (std::max)(Max.Y(), rhs.Y()), (std::max)(Max.Z(), rhs.Z()) };
			return *this;
		}
		AABB& Expand(const AABB& rhs) {
			Expand(rhs.Min);
			Expand(rhs.Max);
			return *this;
		}

		[[nodiscard]] bool Intersect(const AABB& rhs) const {
			if (Max.X() < rhs.Min.X() || rhs.Max.X() < Min.X() ||
				Max.Y() < rhs.Min.Y() || rhs.Max.Y() < Min.Y() ||
				Max.Z() < rhs.Min.Z() || rhs.Max.Z() < Min.Z()) {
				return false;
			}
			return true;
		}

		[[nodiscard]] Vec3 GetCenter() const { return (Min + Max) * 0.5f; }
		[[nodiscard]] Vec3 GetExtent() const { return Max - Min; }

		Vec3 Min = Vec3::Max();
		Vec3 Max = Vec3::Min();
	};
}