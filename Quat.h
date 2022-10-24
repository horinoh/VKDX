#pragma once

namespace Math 
{
	class Quat 
	{
	public:
		Quat() : _x(0.0f), _y(0.0f), _z(0.0f), _w(1.0f) {}
		Quat(const float x, const float y, const float z, const float w) : _x(x), _y(y), _z(z), _w(w) {}
		Quat(const Quat& rhs) : _x(rhs.x()), _y(rhs.y()), _z(rhs.z()), _w(rhs.w()) {}
		Quat(const Vec3& rhs) : _x(rhs.x()), _y(rhs.y()), _z(rhs.z()), _w(0.0f) {}
		Quat(const Vec3& Axis, const float Radian) {
			const auto HalfRadian = 0.5f * Radian;
			const auto a = Axis.Normalize() * sinf(HalfRadian);
			_x = a.x();
			_y = a.y();
			_z = a.z();
			_w = cosf(HalfRadian);
		}

		inline static Quat Identity() { return { 0.0f, 0.0f, 0.0f, 1.0f }; }

		inline bool operator==(const Quat& rhs) const { return x() == rhs.x() && y() == rhs.y() && z() == rhs.z() && w() == rhs.w(); }
		inline bool operator!=(const Quat& rhs) const { return !(*this == rhs); }
		inline Quat operator*(const float rhs) const { return { x() * rhs, y() * rhs, z() * rhs, w() * rhs }; }
		inline Quat operator*(const Quat& rhs) const {
			return { 
				x() * rhs.w() + w() * rhs.x() + y() * rhs.z() - z() * rhs.y(), 
				y() * rhs.w() + w() * rhs.y() + z() * rhs.x() - x() * rhs.z(),
				z() * rhs.w() + w() * rhs.z() + x() * rhs.y() - y() * rhs.x(),
				w() * rhs.w() - x() * rhs.x() - y() * rhs.y() - z() * rhs.z() 
			};
		}
		inline Quat operator/(const float rhs) const { return { x() / rhs, y() / rhs, z() / rhs, w() / rhs }; }

		inline float x() const { return _x; }
		inline float y() const { return _y; }
		inline float z() const { return _z; }
		inline float w() const { return _w; }
		inline float operator[](const int i) const { return (&_x)[i]; }
		inline operator const float* () const { return &_x; }
		inline operator Vec3() const { return { x(), y(), z() }; }
		inline operator Mat3() const {
			return {
				Rotate(Vec3::AxisX()),
				Rotate(Vec3::AxisY()),
				Rotate(Vec3::AxisZ()),
			};
		}
		inline Mat3 ToMat3() const { return static_cast<Mat3>(*this); }

		inline float Dot(const Quat& rhs) const { return x() * rhs.x() + y() * rhs.y() + z() * rhs.z() + w() * rhs.w(); }
		inline float LengthSq() const { return Dot(*this); }
		inline float Length() const { return sqrtf(LengthSq()); }
		inline Quat Normalize() const {
			const auto Sq = LengthSq();
			if (Sq > std::numeric_limits<float>::epsilon()) {
				return *this / sqrt(Sq);
			}
			return *this;
		}
		inline Quat Inverse() const { return Quat({ -x(), -y(), -z(), w() }) / LengthSq(); }
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

		inline Quat& operator=(const Quat& rhs) { _x = rhs.x(); _y = rhs.y(); _z = rhs.z(); _w = rhs.w(); return *this; }
		inline const Quat& operator*=(const float rhs) { _x *= rhs; _y *= rhs; _z *= rhs; _w *= rhs; return *this; }
		inline const Quat& operator/=(const float rhs) { _x /= rhs; _y /= rhs; _z /= rhs; _w /= rhs; return *this; }
		inline float& operator[](const int i) { return (&_x)[i]; }
		inline operator float* () { return &_x; }

		inline Quat& ToIdentity() { return (*this = Identity()); }
		inline Quat& ToNormalized() { return (*this = Normalize()); }
		
	private:
		float _x, _y, _z, _w;
		//glm::quat;
		//XMFLOAT4;
	};
}



