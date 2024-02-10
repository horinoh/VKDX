#pragma once

namespace Math 
{
	class Quat 
	{
	public:
		Quat() {}
		Quat(const float x, const float y, const float z, const float w) : Comps({x, y, z, w}) {}
		Quat(const Vec3& rhs) : Comps({ rhs.X(), rhs.Y(), rhs.Z(), 0.0f }) {}
		Quat(const Vec3& Axis, const float Radian) {
			const auto HalfRadian = 0.5f * Radian;
			const auto Ax = Axis.Normalize() * sinf(HalfRadian);
			Comps = { Ax.X(),Ax.Y(),Ax.Z(), cosf(HalfRadian) };
		}

		inline static Quat Identity() { return { 0.0f, 0.0f, 0.0f, 1.0f }; }

		inline bool NearlyEqual(const Quat& rhs, const float Epsilon = (std::numeric_limits<float>::epsilon)()) const {
			return std::ranges::equal(Comps, rhs.Comps, [&](const float l, const float r) { return std::abs(l - r) < Epsilon; });
		}

		inline bool operator==(const Quat& rhs) const { 
			return std::ranges::equal(Comps, rhs.Comps);
		}
		inline bool operator!=(const Quat& rhs) const { return !(*this == rhs); }
		inline Quat operator*(const float rhs) const {
			Quat r; std::ranges::transform(Comps, std::begin(r.Comps), std::bind(std::multiplies(), std::placeholders::_1, rhs)); return r;
		}
		inline Quat operator*(const Quat& rhs) const {
			return { 
				X() * rhs.W() + W() * rhs.X() + Y() * rhs.Z() - Z() * rhs.Y(), 
				Y() * rhs.W() + W() * rhs.Y() + Z() * rhs.X() - X() * rhs.Z(),
				Z() * rhs.W() + W() * rhs.Z() + X() * rhs.Y() - Y() * rhs.X(),
				W() * rhs.W() - X() * rhs.X() - Y() * rhs.Y() - Z() * rhs.Z() 
			};
		}
		inline Quat operator/(const float rhs) const { 
			Quat r; std::ranges::transform(Comps, std::begin(r.Comps), std::bind(std::divides(), std::placeholders::_1, rhs)); return r;
		}

		inline float X() const { return Comps[0]; }
		inline float Y() const { return Comps[1]; }
		inline float Z() const { return Comps[2]; }
		inline float W() const { return Comps[3]; }
		inline float operator[](const int i) const { return Comps[i]; }
		inline operator const float* () const { return data(Comps); }
		inline operator Vec3() const { return { X(), Y(), Z() }; }
		inline operator Mat3() const {
			return {
				Rotate(Vec3::AxisX()),
				Rotate(Vec3::AxisY()),
				Rotate(Vec3::AxisZ()),
			};
		}
		inline Vec3 ToVec3() const { return static_cast<Vec3>(*this); }
		inline Mat3 ToMat3() const { return static_cast<Mat3>(*this); }

		inline float Dot(const Quat& rhs) const {
			return std::inner_product(std::begin(Comps), std::end(Comps), std::begin(rhs.Comps), 0.0f);
		}
		inline float LengthSq() const { return Dot(*this); }
		inline float Length() const { return sqrtf(LengthSq()); }
		inline Quat Normalize() const {
			const auto Sq = LengthSq();
			if (Sq > std::numeric_limits<float>::epsilon()) {
				return *this / sqrt(Sq);
			}
			return *this;
		}
		inline Quat Inverse() const { return Quat({ -X(), -Y(), -Z(), W() }) / LengthSq(); }
		inline Vec3 Rotate(const Vec3& rhs) const {
			return *this * Quat(rhs) * Inverse();
		}
		inline Mat3 Rotate(const Mat3& rhs) const {
			return {
				Rotate(rhs[0]),
				Rotate(rhs[1]),
				Rotate(rhs[2])
			};
		}

		//inline Quat& operator=(const Vec2& rhs) {
		//	Comps[0] = rhs.X(); Comps[1] = rhs.Y();
		//	return *this;
		//}
		//inline Quat& operator=(const Vec3& rhs) {
		//	Comps[0] = rhs.X(); Comps[1] = rhs.Y(); Comps[2] = rhs.Z();
		//	return *this;
		//}
		//inline Quat& operator=(const Vec4& rhs) {
		//	Comps[0] = rhs.X(); Comps[1] = rhs.Y(); Comps[2] = rhs.Z(); Comps[3] = rhs.W();
		//	return *this;
		//}
		inline Quat& operator=(const Quat& rhs) { 
			std::ranges::copy(rhs.Comps, std::begin(Comps));
			return *this;
		}
		inline const Quat& operator*=(const float rhs) {
			std::ranges::transform(Comps, std::begin(Comps), std::bind(std::multiplies(), std::placeholders::_1, rhs));
			return *this; 
		}
		inline const Quat& operator/=(const float rhs) {
			std::ranges::transform(Comps, std::begin(Comps), std::bind(std::divides(), std::placeholders::_1, rhs));
			return *this; 
		}
		inline float& operator[](const int i) { return Comps[i]; }
		inline operator float* () { return data(Comps); }

		inline Quat& ToIdentity() { return (*this = Identity()); }
		inline Quat& ToNormalized() { return (*this = Normalize()); }
		
	private:
		std::array<float, 4> Comps = { 0.0f, 0.0f, 0.0f, 1.0f };
	};
}



