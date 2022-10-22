#pragma once

#include <algorithm>

#include "Vec.h"
using namespace Math;

namespace Colli 
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
			Min = { (std::min)(Min.x(), rhs.x()), (std::min)(Min.y(), rhs.y()), (std::min)(Min.z(), rhs.z()) };
			Max = { (std::max)(Max.x(), rhs.x()), (std::max)(Max.y(), rhs.y()), (std::max)(Max.z(), rhs.z()) };
			return *this;
		}
		AABB& Expand(const AABB& rhs) {
			Expand(rhs.Min);
			Expand(rhs.Max);
			return *this;
		}

		bool Intersect(const AABB& rhs) const {
			if (Max.x() < rhs.Min.x() || rhs.Max.x() < Min.x() ||
				Max.y() < rhs.Min.y() || rhs.Max.y() < Min.y() ||
				Max.z() < rhs.Min.z() || rhs.Max.z() < Min.z()) {
				return false;
			}
			return true;
		}

		Vec3 GetCenter() const { return (Min + Max) * 0.5f; }
		Vec3 GetExtent() const { return Max - Min; }

		Vec3 Min = Vec3::Max();
		Vec3 Max = Vec3::Min();
	};
}