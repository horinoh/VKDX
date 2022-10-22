#pragma once

namespace Math 
{
	class Vec2
	{
	public:
		Vec2() : _x(0.0f), _y(0.0f) {}
		Vec2(const float x, const float y) : _x(x), _y(y) {}
		Vec2(const Vec2& rhs) : _x(rhs.x()), _y(rhs.y()) {}
		Vec2(const float rhs) : _x(rhs), _y(rhs) {}

		inline static Vec2 Zero() { return { 0.0f, 0.0f }; }
		inline static Vec2 One() { return { 1.0f, 1.0f }; }
		inline static Vec2 AxisX() { return { 1.0f, 0.0f }; }
		inline static Vec2 AxisY() { return { 0.0f, 1.0f }; }
		inline static Vec2 Epsilon() { return { (std::numeric_limits<float>::epsilon)(), (std::numeric_limits<float>::epsilon)() }; }
		inline static Vec2 Min() { return { (std::numeric_limits<float>::min)(), (std::numeric_limits<float>::min)() }; }
		inline static Vec2 Max() { return { (std::numeric_limits<float>::max)(), (std::numeric_limits<float>::max)() }; }

		inline bool operator==(const Vec2& rhs) const { return x() == rhs.x() && y() == rhs.y(); }
		inline bool operator!=(const Vec2& rhs) const { return !(*this == rhs); }
		inline Vec2 operator+(const Vec2& rhs) const { return { x() + rhs.x(), y() + rhs.y() }; }
		inline Vec2 operator-(const Vec2& rhs) const { return { x() - rhs.x(), y() - rhs.y() }; }
		inline Vec2 operator*(const float rhs) const { return { x() * rhs, y() * rhs }; }
		inline Vec2 operator/(const float rhs) const { return { x() / rhs, y() / rhs }; }
		inline Vec2 operator-() const { return { -x(), -y() }; }
		
		inline float x() const { return _x; }
		inline float y() const { return _y; }
		inline float operator[](const int i) const { return (&_x)[i]; }
		inline operator const float* () const { return &_x; }

		inline float Dot(const Vec2& rhs) const { return x() * rhs.x() + y() * rhs.y(); }
		inline float LengthSq() const { return Dot(*this); }
		inline float Length() const { return sqrtf(LengthSq()); }
		inline Vec2 Normalize() const {
			const auto Sq = LengthSq();
			if (Sq > std::numeric_limits<float>::epsilon()) {
				return *this / sqrt(Sq);
			}
			return *this;
		}

		inline Vec2& operator=(const Vec2& rhs) { _x = rhs.x(); _y = rhs.y(); return *this; }
		inline const Vec2& operator+=(const Vec2& rhs) { _x += rhs.x(); _y += rhs.y(); return *this; }
		inline const Vec2& operator-=(const Vec2& rhs) { _x -= rhs.x(); _y -= rhs.y(); return *this; }
		inline const Vec2& operator*=(const float rhs) { _x *= rhs; _y *= rhs; return *this; }
		inline const Vec2& operator/=(const float rhs) { _x /= rhs; _y /= rhs; return *this; }
		inline float& operator[](const int i) { return (&_x)[i]; }
		inline operator float* () { return &_x; }

		inline Vec2& ToZero() { return (*this = Zero()); }
		inline Vec2& ToNormalize() { return (*this = Normalize()); }
		
	private:
		float _x, _y;
		//glm::vec2;
		//XMFLOAT2;
	};

	class Vec3
	{
	public:
		Vec3() : _x(0.0f), _y(0.0f), _z(0.0f) {}
		Vec3(const float x, const float y, const float z) : _x(x), _y(y), _z(z) {}
		Vec3(const Vec3& rhs) : _x(rhs.x()), _y(rhs.y()), _z(rhs.z()) {}
		Vec3(const float rhs) : _x(rhs), _y(rhs), _z(rhs) {}

		inline static Vec3 Zero() { return { 0.0f, 0.0f, 0.0f }; }
		inline static Vec3 One() { return { 1.0f, 1.0f, 1.0f }; }
		inline static Vec3 AxisX() { return { 1.0f, 0.0f, 0.0f }; }
		inline static Vec3 AxisY() { return { 0.0f, 1.0f, 0.0f }; }
		inline static Vec3 AxisZ() { return { 0.0f, 0.0f, 1.0f }; }
		inline static Vec3 Epsilon() { return { (std::numeric_limits<float>::epsilon)(), (std::numeric_limits<float>::epsilon)(), (std::numeric_limits<float>::epsilon)() }; }
		inline static Vec3 Min() { return { (std::numeric_limits<float>::min)(), (std::numeric_limits<float>::min)(), (std::numeric_limits<float>::min)() }; }
		inline static Vec3 Max() { return { (std::numeric_limits<float>::max)(), (std::numeric_limits<float>::max)(), (std::numeric_limits<float>::max)() }; }

		inline bool operator==(const Vec3& rhs) const { return x() == rhs.x() && y() == rhs.y() && z() == rhs.z(); }
		inline bool operator!=(const Vec3& rhs) const { return !(*this == rhs); }
		inline Vec3 operator+(const Vec3& rhs) const { return { x() + rhs.x(), y() + rhs.y(), z() + rhs.z() }; }
		inline Vec3 operator-(const Vec3& rhs) const { return { x() - rhs.x(), y() - rhs.y(), z() - rhs.z() }; }
		inline Vec3 operator*(const float rhs) const { return { x() * rhs, y() * rhs, z() * rhs }; }
		inline Vec3 operator/(const float rhs) const { return { x() / rhs, y() / rhs, z() / rhs }; }
		inline Vec3 operator-() const { return { -x(), -y(), -z() }; }

		inline float x() const { return _x; }
		inline float y() const { return _y; }
		inline float z() const { return _z; }
		inline float operator[](const int i) const { return (&_x)[i]; }
		inline operator const float* () const { return &_x; }

		inline float Dot(const Vec3& rhs) const { return x() * rhs.x() + y() * rhs.y() + z() * rhs.z(); }
		inline float LengthSq() const { return Dot(*this); }
		inline float Length() const { return sqrt(LengthSq()); }
		inline Vec3 Normalize() const {
			const auto Sq = LengthSq();
			if (Sq > std::numeric_limits<float>::epsilon()) { 
				return *this / sqrt(Sq); 
			}
			return *this;
		}
		inline Vec3 Cross(const Vec3& rhs) const { return Vec3(y() * rhs.z() - rhs.y() * z(), rhs.x() * z() - x() * rhs.z(), x() * rhs.y() - rhs.x() * y()); }

		inline Vec3& operator=(const Vec3& rhs) { _x = rhs.x(); _y = rhs.y(); _z = rhs.z(); return *this; }
		inline const Vec3& operator+=(const Vec3& rhs) { _x += rhs.x(); _y += rhs.y(); _z += rhs.z(); return *this; }
		inline const Vec3& operator-=(const Vec3& rhs) { _x -= rhs.x(); _y -= rhs.y(); _z -= rhs.z(); return *this; }
		inline const Vec3& operator*=(const float rhs) { _x *= rhs; _y *= rhs; _z *= rhs; return *this; }
		inline const Vec3& operator/=(const float rhs) { _x /= rhs; _y /= rhs; _z /= rhs; return *this; }
		inline float& operator[](const int i) { return (&_x)[i]; }
		inline operator float* () { return &_x; }

		inline Vec3& ToZero() { return (*this = Zero()); }
		inline Vec3& ToNormalized() { return (*this = Normalize()); }

	private:
		float _x, _y, _z; 
		//glm::vec3;
		//XMFLOAT3;
	};

	class Vec4
	{
	public:
		Vec4() : _x(0.0f), _y(0.0f), _z(0.0f), _w(0.0f) {}
		Vec4(const float x, const float y, const float z, const float w) : _x(x), _y(y), _z(z), _w(w) {}
		Vec4(const Vec4& rhs) : _x(rhs.x()), _y(rhs.y()), _z(rhs.z()), _w(rhs.w()) {}
		Vec4(const float rhs) : _x(rhs), _y(rhs), _z(rhs), _w(rhs) {}

		inline static Vec4 Zero() { return { 0.0f, 0.0f, 0.0f, 0.0f }; }
		inline static Vec4 One() { return { 1.0f, 1.0f, 1.0f, 1.0f }; }
		inline static Vec4 AxisX() { return { 1.0f, 0.0f, 0.0f, 0.0f }; }
		inline static Vec4 AxisY() { return { 0.0f, 1.0f, 0.0f, 0.0f }; }
		inline static Vec4 AxisZ() { return { 0.0f, 0.0f, 1.0f, 0.0f }; }
		inline static Vec4 AxisW() { return { 0.0f, 0.0f, 0.0f, 1.0f }; }
		inline static Vec4 Epsilon() { return { (std::numeric_limits<float>::epsilon)(), (std::numeric_limits<float>::epsilon)(), (std::numeric_limits<float>::epsilon)(), (std::numeric_limits<float>::epsilon)() }; }
		inline static Vec4 Min() { return { (std::numeric_limits<float>::min)(), (std::numeric_limits<float>::min)(), (std::numeric_limits<float>::min)(), (std::numeric_limits<float>::min)() }; }
		inline static Vec4 Max() { return { (std::numeric_limits<float>::max)(), (std::numeric_limits<float>::max)(), (std::numeric_limits<float>::max)(), (std::numeric_limits<float>::max)() }; }

		inline bool operator==(const Vec4& rhs) const { return x() == rhs.x() && y() == rhs.y() && z() == rhs.z() && w() == rhs.w(); }
		inline bool operator!=(const Vec4& rhs) const { return !(*this == rhs); }
		inline Vec4 operator+(const Vec4& rhs) const { return { x() + rhs.x(), y() + rhs.y(), z() + rhs.z(), w() + rhs.w() }; }
		inline Vec4 operator-(const Vec4& rhs) const { return { x() - rhs.x(), y() - rhs.y(), z() - rhs.z(), w() - rhs.w() }; }
		inline Vec4 operator*(const float rhs) const { return { x() * rhs, y() * rhs, z() * rhs, w() * rhs }; }
		inline Vec4 operator/(const float rhs) const { return { x() / rhs, y() / rhs, z() / rhs, w() / rhs }; }
		inline Vec4 operator-() const { return { -x(), -y(), -z(), -w() }; }

		inline float x() const { return _x; }
		inline float y() const { return _y; }
		inline float z() const { return _z; }
		inline float w() const { return _w; }
		inline float operator[](const int i) const { return (&_x)[i]; }
		inline operator const float* () const { return &_x; }

		inline float Dot(const Vec4& rhs) const { return x() * rhs.x() + y() * rhs.y() + z() * rhs.z() + w() * rhs.w(); }
		inline float LengthSq() const { return Dot(*this); }
		inline float Length() const { return sqrtf(LengthSq()); }
		inline Vec4 Normalize() const {
			const auto Sq = LengthSq();
			if (Sq > std::numeric_limits<float>::epsilon()) {
				return *this / sqrt(Sq);
			}
			return *this;
		}

		//inline void hoge() {
		//	DirectX::XMLoadFloat(static_cast<const float*>(*this));
		//}

		inline Vec4& operator=(const Vec4& rhs) { _x = rhs.x(); _y = rhs.y(); _z = rhs.z(); _w = rhs.w(); return *this; }
		inline const Vec4& operator+=(const Vec4& rhs) { _x += rhs.x(); _y += rhs.y(); _z += rhs.z(); _w += rhs.w(); return *this; }
		inline const Vec4& operator-=(const Vec4& rhs) { _x -= rhs.x(); _y -= rhs.y(); _z -= rhs.z(); _w -= rhs.w(); return *this; }
		inline const Vec4& operator*=(const float rhs) { _x *= rhs; _y *= rhs; _z *= rhs; _w *= rhs; return *this; }
		inline const Vec4& operator/=(const float rhs) { _x /= rhs; _y /= rhs; _z /= rhs; _w /= rhs; return *this; }
		inline float& operator[](const int i) { return (&_x)[i]; }
		inline operator float* () { return &_x; }

		inline Vec4& ToZero() { return (*this = Zero()); }
		inline Vec4& ToNormalized() { return (*this = Normalize()); }

	private:
		float _x, _y, _z, _w;
		//glm::vec4;
		//XMFLOAT4;
	};
}
