#pragma once

namespace Math 
{
	class Vec2
	{
	public:
		Vec2() {}
		Vec2(const float x, const float y) : Comps({x, y}) {}
		Vec2(const float rhs) : Comps({rhs, rhs}) {}

		inline static Vec2 Zero() { return Vec2(); }
		inline static Vec2 One() { return Vec2(1.0f); }
		inline static Vec2 AxisX() { return { 1.0f, 0.0f }; }
		inline static Vec2 AxisY() { return { 0.0f, 1.0f }; }
		inline static Vec2 Epsilon() { return  Vec2((std::numeric_limits<float>::epsilon)()); }
		inline static Vec2 Min() { return Vec2((std::numeric_limits<float>::min)()); }
		inline static Vec2 Max() { return Vec2((std::numeric_limits<float>::max)()); }

		inline bool NearlyEqual(const Vec2& rhs, const float Epsilon = (std::numeric_limits<float>::epsilon)()) const {
			return std::abs(X() - rhs.X()) < Epsilon && std::abs(Y() - rhs.Y()) < Epsilon; 
		}

		inline bool operator==(const Vec2& rhs) const { return X() == rhs.X() && Y() == rhs.Y(); }
		inline bool operator!=(const Vec2& rhs) const { return !(*this == rhs); }
		inline Vec2 operator+(const Vec2& rhs) const { return { X() + rhs.X(), Y() + rhs.Y() }; }
		inline Vec2 operator-(const Vec2& rhs) const { return { X() - rhs.X(), Y() - rhs.Y() }; }
		inline Vec2 operator*(const float rhs) const { return { X() * rhs, Y() * rhs }; }
		inline Vec2 operator/(const float rhs) const { return { X() / rhs, Y() / rhs }; }
		inline Vec2 operator-() const { return { -X(), -Y() }; }
		
		inline float X() const { return Comps[0]; }
		inline float Y() const { return Comps[1]; }
		inline float operator[](const int i) const { return Comps[i]; }
		inline operator const float* () const { return data(Comps); }

		inline float Dot(const Vec2& rhs) const { return X() * rhs.X() + Y() * rhs.Y(); }
		inline float LengthSq() const { return Dot(*this); }
		inline float Length() const { return sqrtf(LengthSq()); }
		inline Vec2 Normalize() const {
			const auto Sq = LengthSq();
			if (Sq > std::numeric_limits<float>::epsilon()) {
				return *this / sqrt(Sq);
			}
			return *this;
		}

		inline Vec2& operator=(const Vec2& rhs) { Comps[0] = rhs.X(); Comps[1] = rhs.Y(); return *this; }
		inline const Vec2& operator+=(const Vec2& rhs) { Comps[0] += rhs.X(); Comps[1] += rhs.Y(); return *this; }
		inline const Vec2& operator-=(const Vec2& rhs) { Comps[0] -= rhs.X(); Comps[1] -= rhs.Y(); return *this; }
		inline const Vec2& operator*=(const float rhs) { Comps[0] *= rhs; Comps[1] *= rhs; return *this; }
		inline const Vec2& operator/=(const float rhs) { Comps[0] /= rhs; Comps[1] /= rhs; return *this; }
		inline float& operator[](const int i) { return Comps[i]; }
		inline operator float* () { return data(Comps); }

		inline Vec2& ToZero() { return (*this = Zero()); }
		inline Vec2& ToNormalize() { return (*this = Normalize()); }
		
	private:
		std::array<float, 2> Comps = { 0.0f, 0.0f };
	};

	class Vec3
	{
	public:
		Vec3() {}
		Vec3(const float x, const float y, const float z) : Comps({x, y, z}) {}
		Vec3(const float rhs) : Comps({rhs, rhs, rhs}) {}

		inline static Vec3 Zero() { return Vec3(); }
		inline static Vec3 One() { return Vec3(1.0f); }
		inline static Vec3 AxisX() { return { 1.0f, 0.0f, 0.0f }; }
		inline static Vec3 AxisY() { return { 0.0f, 1.0f, 0.0f }; }
		inline static Vec3 AxisZ() { return { 0.0f, 0.0f, 1.0f }; }
		inline static Vec3 Epsilon() { return Vec3((std::numeric_limits<float>::epsilon)()); }
		inline static Vec3 Min() { return Vec3((std::numeric_limits<float>::min)()); }
		inline static Vec3 Max() { return Vec3((std::numeric_limits<float>::max)()); }

		inline bool NearlyEqual(const Vec3& rhs, const float Epsilon = (std::numeric_limits<float>::epsilon)()) const {
			return std::abs(X() - rhs.X()) < Epsilon && std::abs(Y() - rhs.Y()) < Epsilon && std::abs(Z() - rhs.Z()) < Epsilon;
		}

		inline bool operator==(const Vec3& rhs) const { return X() == rhs.X() && Y() == rhs.Y() && Z() == rhs.Z(); }
		inline bool operator!=(const Vec3& rhs) const { return !(*this == rhs); }
		inline Vec3 operator+(const Vec3& rhs) const { return { X() + rhs.X(), Y() + rhs.Y(), Z() + rhs.Z() }; }
		inline Vec3 operator-(const Vec3& rhs) const { return { X() - rhs.X(), Y() - rhs.Y(), Z() - rhs.Z() }; }
		inline Vec3 operator*(const float rhs) const { return { X() * rhs, Y() * rhs, Z() * rhs }; }
		inline Vec3 operator/(const float rhs) const { return { X() / rhs, Y() / rhs, Z() / rhs }; }
		inline Vec3 operator-() const { return { -X(), -Y(), -Z() }; }

		inline float X() const { return Comps[0]; }
		inline float Y() const { return Comps[1]; }
		inline float Z() const { return Comps[2]; }
		inline float operator[](const int i) const { return Comps[i]; }
		inline operator const float* () const { return data(Comps); }

		inline float Dot(const Vec3& rhs) const { return X() * rhs.X() + Y() * rhs.Y() + Z() * rhs.Z(); }
		inline float LengthSq() const { return Dot(*this); }
		inline float Length() const { return sqrt(LengthSq()); }
		inline Vec3 Normalize() const {
			const auto Sq = LengthSq();
			if (Sq > std::numeric_limits<float>::epsilon()) { 
				return *this / sqrt(Sq); 
			}
			return *this;
		}
		inline Vec3 Cross(const Vec3& rhs) const { return Vec3(Y() * rhs.Z() - rhs.Y() * Z(), rhs.X() * Z() - X() * rhs.Z(), X() * rhs.Y() - rhs.X() * Y()); }

		inline Vec3& operator=(const Vec2& rhs) { Comps[0] = rhs.X(); Comps[1] = rhs.Y(); return *this; }
		inline Vec3& operator=(const Vec3& rhs) { Comps[0] = rhs.X(); Comps[1] = rhs.Y(); Comps[2] = rhs.Z(); return *this; }
		inline const Vec3& operator+=(const Vec3& rhs) { Comps[0] += rhs.X(); Comps[1] += rhs.Y(); Comps[2] += rhs.Z(); return *this; }
		inline const Vec3& operator-=(const Vec3& rhs) { Comps[0] -= rhs.X(); Comps[1] -= rhs.Y(); Comps[2] -= rhs.Z(); return *this; }
		inline const Vec3& operator*=(const float rhs) { Comps[0] *= rhs; Comps[1] *= rhs; Comps[2] *= rhs; return *this; }
		inline const Vec3& operator/=(const float rhs) { Comps[0] /= rhs; Comps[1] /= rhs; Comps[2] /= rhs; return *this; }
		inline float& operator[](const int i) { return Comps[i]; }
		inline operator float* () { return data(Comps); }

		inline Vec3& ToZero() { return (*this = Zero()); }
		inline Vec3& ToNormalized() { return (*this = Normalize()); }

	private:
		std::array<float, 3> Comps = { 0.0f, 0.0f, 0.0f };
	};

	class Vec4
	{
	public:
		Vec4() {}
		Vec4(const float x, const float y, const float z, const float w) : Comps({x, y, z, w}) {}
		Vec4(const float rhs) : Comps({rhs, rhs, rhs, rhs}) {}

		inline static Vec4 Zero() { return Vec4(); }
		inline static Vec4 One() { return Vec4(1.0f); }
		inline static Vec4 AxisX() { return { 1.0f, 0.0f, 0.0f, 0.0f }; }
		inline static Vec4 AxisY() { return { 0.0f, 1.0f, 0.0f, 0.0f }; }
		inline static Vec4 AxisZ() { return { 0.0f, 0.0f, 1.0f, 0.0f }; }
		inline static Vec4 AxisW() { return { 0.0f, 0.0f, 0.0f, 1.0f }; }
		inline static Vec4 Epsilon() { return Vec4((std::numeric_limits<float>::epsilon)()); }
		inline static Vec4 Min() { return Vec4((std::numeric_limits<float>::min)()); }
		inline static Vec4 Max() { return Vec4((std::numeric_limits<float>::max)()); }

		inline bool NearlyEqual(const Vec4& rhs, const float Epsilon = (std::numeric_limits<float>::epsilon)()) const { 
			return std::abs(X() - rhs.X()) < Epsilon && std::abs(Y() - rhs.Y()) < Epsilon && std::abs(Z() - rhs.Z()) < Epsilon && std::abs(W() - rhs.W()) < Epsilon; 
		}

		inline bool operator==(const Vec4& rhs) const { return X() == rhs.X() && Y() == rhs.Y() && Z() == rhs.Z() && W() == rhs.W(); }
		inline bool operator!=(const Vec4& rhs) const { return !(*this == rhs); }
		inline Vec4 operator+(const Vec4& rhs) const { return { X() + rhs.X(), Y() + rhs.Y(), Z() + rhs.Z(), W() + rhs.W() }; }
		inline Vec4 operator-(const Vec4& rhs) const { return { X() - rhs.X(), Y() - rhs.Y(), Z() - rhs.Z(), W() - rhs.W() }; }
		inline Vec4 operator*(const float rhs) const { return { X() * rhs, Y() * rhs, Z() * rhs, W() * rhs }; }
		inline Vec4 operator/(const float rhs) const { return { X() / rhs, Y() / rhs, Z() / rhs, W() / rhs }; }
		inline Vec4 operator-() const { return { -X(), -Y(), -Z(), -W() }; }

		inline float X() const { return Comps[0]; }
		inline float Y() const { return Comps[1]; }
		inline float Z() const { return Comps[2]; }
		inline float W() const { return Comps[3]; }
		inline float operator[](const int i) const { return Comps[i]; }
		inline operator const float* () const { return data(Comps); }

		inline float Dot(const Vec4& rhs) const { return X() * rhs.X() + Y() * rhs.Y() + Z() * rhs.Z() + W() * rhs.W(); }
		inline float LengthSq() const { return Dot(*this); }
		inline float Length() const { return sqrtf(LengthSq()); }
		inline Vec4 Normalize() const {
			const auto Sq = LengthSq();
			if (Sq > std::numeric_limits<float>::epsilon()) {
				return *this / sqrt(Sq);
			}
			return *this;
		}

		inline Vec4& operator=(const Vec2& rhs) { Comps[0] = rhs.X(); Comps[1] = rhs.Y(); return *this; }
		inline Vec4& operator=(const Vec3& rhs) { Comps[0] = rhs.X(); Comps[1] = rhs.Y(); Comps[2] = rhs.Z(); return *this; }
		inline Vec4& operator=(const Vec4& rhs) { Comps[0] = rhs.X(); Comps[1] = rhs.Y(); Comps[2] = rhs.Z(); Comps[3] = rhs.W(); return *this; }
		inline const Vec4& operator+=(const Vec4& rhs) { Comps[0] += rhs.X(); Comps[1] += rhs.Y(); Comps[2] += rhs.Z(); Comps[3] += rhs.W(); return *this; }
		inline const Vec4& operator-=(const Vec4& rhs) { Comps[0] -= rhs.X(); Comps[1] -= rhs.Y(); Comps[2] -= rhs.Z(); Comps[3] -= rhs.W(); return *this; }
		inline const Vec4& operator*=(const float rhs) { Comps[0] *= rhs; Comps[1] *= rhs; Comps[2] *= rhs; Comps[3] *= rhs; return *this; }
		inline const Vec4& operator/=(const float rhs) { Comps[0] /= rhs; Comps[1] /= rhs; Comps[2] /= rhs; Comps[3] /= rhs; return *this; }
		inline float& operator[](const int i) { return Comps[i]; }
		inline operator float* () { return data(Comps); }

		inline Vec4& ToZero() { return (*this = Zero()); }
		inline Vec4& ToNormalized() { return (*this = Normalize()); }

	private:
		std::array<float, 4> Comps = { 0.0f, 0.0f, 0.0f, 0.0f };
	};

	template<size_t N>
	class Vec
	{
	public:
		Vec() { std::ranges::fill(Comps, 0.0f); }
		Vec(const float rhs) { std::ranges::fill(Comps, rhs); }

		inline static Vec Zero() { return Vec(); }
		inline static Vec One() { return Vec(1.0f); }
		inline static Vec Epsilon() { return Vec((std::numeric_limits<float>::epsilon)()); }
		inline static Vec Min() { return Vec((std::numeric_limits<float>::min)()); }
		inline static Vec Max() { return Vec((std::numeric_limits<float>::max)()); }

		inline bool NearlyEqual(const Vec& rhs, const float Epsilon = (std::numeric_limits<float>::epsilon)()) const {
			for (auto i = 0; i < size(Comps); ++i) {
				if (std::abs(Comps[i] - rhs[i]) >= Epsilon) {
					return false; 
				}
			}
			return true;
		}

		inline bool operator==(const Vec& rhs) const { return std::ranges::equal(Comps, rhs.Comps); }
		inline bool operator!=(const Vec& rhs) const { return !(*this == rhs); }
		inline Vec operator+(const Vec& rhs) const { 
			Vec V;
			std::ranges::transform(Comps, rhs.Comps, std::back_inserter(V.Comps), std::plus());
			return V;
		}
		inline Vec operator-(const Vec& rhs) const {
			Vec V;
			std::ranges::transform(Comps, rhs.Comps, std::back_inserter(V.Comps), std::minus());
			return V;
		}
		inline Vec operator*(const float rhs) const {
			Vec V;
			std::ranges::transform(Comps, rhs, std::back_inserter(V.Comps), std::multiplies());
			return V;
		}
		inline Vec operator/(const float rhs) const {
			Vec V;
			std::ranges::transform(Comps, rhs, std::back_inserter(V.Comps), std::divides());
			return V;
		}
		inline Vec operator-() const { 
			Vec V;
			std::ranges::transform(Comps, std::back_inserter(V.Comps), std::negation());
			return V;
		}

		inline float operator[](const int i) const { return Comps[i]; }
		inline operator const float* () const { return data(Comps); }

		inline float Dot(const Vec& rhs) const { 
			float Sum = 0.0f;
			for (auto i = 0; i < size(Comps); ++i) {
				Sum += Comps[i] * rhs[i];
			}
			return Sum;
		}
		inline float LengthSq() const { return Dot(*this); }
		inline float Length() const { return sqrtf(LengthSq()); }
		inline Vec Normalize() const {
			const auto Sq = LengthSq();
			if (Sq > std::numeric_limits<float>::epsilon()) {
				return *this / sqrt(Sq);
			}
			return *this;
		}

		inline const Vec& operator+=(const Vec& rhs) { 
			std::ranges::transform(Comps, rhs.Comps, std::back_inserter(Comps), std::plus());
			return *this;
		}
		inline const Vec& operator-=(const Vec& rhs) {
			std::ranges::transform(Comps, rhs.Comps, std::back_inserter(Comps), std::divides());
			return *this;
		}
		inline const Vec& operator*=(const float rhs) { 
			std::ranges::transform(Comps, rhs, std::back_inserter(Comps), std::multiplies());
			return *this; 
		}
		inline const Vec& operator/=(const float rhs) {
			std::ranges::transform(Comps, rhs, std::back_inserter(Comps), std::divides());
			return *this; 
		}
		inline float& operator[](const int i) { return Comps[i]; }
		inline operator float* () { return data(Comps); }

		inline Vec& ToZero() { return (*this = Zero()); }
		inline Vec& ToNormalized() { return (*this = Normalize()); }

	private:
		std::array<float, N> Comps;
	};
}
