#pragma once

namespace Math 
{
	class Mat2 
	{
	public:
		Mat2() : Rows{ Vec2(1.0f, 0.0f), Vec2(0.0f, 1.0f) } {}
		Mat2(const Vec2& row0, const Vec2& row1) : Rows{ row0, row1 } {}
		Mat2(const Mat2& rhs) : Rows{ rhs.Rows[0], rhs.Rows[1] } {}

		inline static Mat2 Identity() { return { { 1.0f, 0.0f }, { 0.0f, 1.0f } }; }
		inline static Mat2 Zero() { return { Vec2::Zero(), Vec2::Zero() }; }

		inline bool NearlyEqual(const Mat2& rhs, const float Epsilon = (std::numeric_limits<float>::epsilon)()) const { return Rows[0].NearlyEqual(rhs.Rows[0]) && Rows[1].NearlyEqual(rhs.Rows[1]); }

		inline bool operator==(const Mat2& rhs) const { return Rows[0] == rhs.Rows[0] && Rows[1] == rhs.Rows[1]; }
		inline Mat2 operator+(const Mat2& rhs) const { return { Rows[0] + rhs.Rows[0], Rows[1] + rhs.Rows[1] }; }
		inline Mat2 operator-(const Mat2& rhs) const { return { Rows[0] - rhs.Rows[0], Rows[1] - rhs.Rows[1] }; }
		inline Mat2 operator*(const float rhs) const { return { Rows[0] * rhs, Rows[1] * rhs }; }
		inline Vec2 operator*(const Vec2& rhs) const { return { Rows[0].Dot(rhs), Rows[1].Dot(rhs) }; }
		inline Mat2 operator*(const Mat2& rhs) const {
			//!< ŒŸŽZ—p : glm ‚Ìê‡‚ÍŠ|‚¯‚é‡˜‚ª‹t‚È‚Ì‚Å’ˆÓ
			//glm::make_mat2(static_cast<const float*>(rhs)) * glm::make_mat2(static_cast<const float*>(*this));
			const auto c0 = Vec2({ rhs.Rows[0][0], rhs.Rows[1][0] });
			const auto c1 = Vec2({ rhs.Rows[0][1], rhs.Rows[1][1] });
			return { 
				{ Rows[0].Dot(c0), Rows[0].Dot(c1) }, 
				{ Rows[1].Dot(c0), Rows[1].Dot(c1) } 
			};
		}
		inline Mat2 operator/(const float rhs) const { return { Rows[0] / rhs, Rows[1] / rhs }; }

		inline const Vec2& operator[](const int i) const { return Rows[i]; }
		inline operator const float* () const { return static_cast<const float*>(Rows[0]); }

		inline Mat2 Transpose() const {
			return { 
				{ Rows[0].X(), Rows[1].X() }, 
				{ Rows[0].Y(), Rows[1].Y() } 
			};
		}
		inline float Determinant() const {
			//!< ŒŸŽZ—p
			//glm::determinant(glm::make_mat2(static_cast<const float *>(*this)));
			return Rows[0][0] * Rows[1][1] - Rows[1][0] * Rows[0][1];
		}
		inline Mat2 Inverse(const float InvDet) const {
			//!< ŒŸŽZ—p
			//glm::inverse(glm::make_mat2(static_cast<const float*>(*this)));
			return Mat2({ { Rows[1][1], -Rows[0][1] }, { -Rows[1][0], Rows[0][0] } }) * InvDet;
		}
		inline Mat2 Inverse() const { Inverse(1.0f / Determinant()); }

		inline Mat2& operator=(const Mat2& rhs) { Rows[0] = rhs.Rows[0]; Rows[1] = rhs.Rows[1]; return *this; }
		inline const Mat2& operator+=(const Mat2& rhs) { Rows[0] += rhs.Rows[0]; Rows[1] += rhs.Rows[1]; return *this; }
		inline const Mat2& operator-=(const Mat2& rhs) { Rows[0] -= rhs.Rows[0]; Rows[1] -= rhs.Rows[1]; return *this; }
		inline const Mat2& operator*=(const float rhs) { Rows[0] *= rhs; Rows[1] *= rhs; return *this; }
		inline const Mat2& operator/=(const float rhs) { Rows[0] /= rhs; Rows[1] /= rhs; return *this; }
		inline Vec2& operator[](const int i) { return Rows[i]; }

		inline Mat2& ToZero() { return (*this = Zero()); }
		inline Mat2& ToIdentity() { return (*this = Identity()); }

	private:
		std::array<Vec2, 2> Rows;
	};

	class Mat3 
	{
	public:
		Mat3() : Rows{ Vec3(1.0f, 0.0f, 0.0f), Vec3(0.0f, 1.0f, 0.0f), Vec3(0.0f, 0.0f, 1.0f) } {}
		Mat3(const Vec3& row0, const Vec3& row1, const Vec3& row2) : Rows{ row0, row1, row2 } {}
		Mat3(const Mat3& rhs) : Rows{ rhs.Rows[0], rhs.Rows[1], rhs.Rows[2] } {}

		inline static Mat3 Identity() { return { { 1.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 0.0f, 1.0f } }; }
		inline static Mat3 Zero() { return { Vec3::Zero(), Vec3::Zero(), Vec3::Zero() }; }

		inline bool NearlyEqual(const Mat3& rhs, const float Epsilon = (std::numeric_limits<float>::epsilon)()) const { return Rows[0].NearlyEqual(rhs.Rows[0]) && Rows[1].NearlyEqual(rhs.Rows[1]) && Rows[2].NearlyEqual(rhs.Rows[2]); }

		inline bool operator==(const Mat3& rhs) const { return Rows[0] == rhs.Rows[0] && Rows[1] == rhs.Rows[1] && Rows[2] == rhs.Rows[2]; }
		inline Mat3 operator+(const Mat3& rhs) const { return { Rows[0] + rhs.Rows[0], Rows[1] + rhs.Rows[1], Rows[2] + rhs.Rows[2] }; }
		inline Mat3 operator-(const Mat3& rhs) const { return { Rows[0] - rhs.Rows[0], Rows[1] - rhs.Rows[1], Rows[2] - rhs.Rows[2] }; }
		inline Mat3 operator*(const float rhs) const { return { Rows[0] * rhs, Rows[1] * rhs, Rows[2] * rhs }; }
		inline Vec3 operator*(const Vec3& rhs) const { return { Rows[0].Dot(rhs), Rows[1].Dot(rhs), Rows[2].Dot(rhs) }; }
		inline Mat3 operator*(const Mat3& rhs) const {
			//!< ŒŸŽZ—p : glm ‚Ìê‡‚ÍŠ|‚¯‚é‡˜‚ª‹t‚È‚Ì‚Å’ˆÓ
			//glm::make_mat3(static_cast<const float*>(rhs)) * glm::make_mat3(static_cast<const float*>(*this));
			const auto c0 = Vec3({ rhs.Rows[0][0],rhs.Rows[1][0], rhs.Rows[2][0] });
			const auto c1 = Vec3({ rhs.Rows[0][1],rhs.Rows[1][1], rhs.Rows[2][1] });
			const auto c2 = Vec3({ rhs.Rows[0][2],rhs.Rows[1][2], rhs.Rows[2][2] });
			return { 
				{ Rows[0].Dot(c0), Rows[0].Dot(c1), Rows[0].Dot(c2) }, 
				{ Rows[1].Dot(c0), Rows[1].Dot(c1), Rows[1].Dot(c2) }, 
				{ Rows[2].Dot(c0), Rows[2].Dot(c1), Rows[2].Dot(c2) } 
			};
		}
		inline Mat3 operator/(const float rhs) const { return { Rows[0] / rhs, Rows[1] / rhs, Rows[2] / rhs }; }

		inline const Vec3& operator[](const int i) const { return Rows[i]; }
		inline operator const float* () const { return static_cast<const float*>(Rows[0]); }

		inline Mat3 Transpose() const {
			return {
				{ Rows[0].X(), Rows[1].X(), Rows[2].X() },
				{ Rows[0].Y(), Rows[1].Y(), Rows[2].Y() },
				{ Rows[0].Z(), Rows[1].Z(), Rows[2].Z() }
			};
		}
		inline float Determinant() const {
			//!< ŒŸŽZ—p
			//glm::determinant(glm::make_mat3(static_cast<const float*>(*this)));
			return Rows[0][0] * Mat2({ { Rows[1][1], Rows[1][2] }, { Rows[2][1], Rows[2][2] } }).Determinant()
				- Rows[1][0] * Mat2({ { Rows[0][1], Rows[0][2] }, { Rows[2][1], Rows[2][2] } }).Determinant()
				+ Rows[2][0] * Mat2({ { Rows[0][1], Rows[0][2] }, { Rows[1][1], Rows[1][2] } }).Determinant();
		}
		inline Mat3 Inverse(const float InvDet) const {
			//!< ŒŸŽZ—p
			//glm::inverse(glm::make_mat3(static_cast<const float*>(*this)));
			const auto M00 = Mat2({ { Rows[1][1], Rows[1][2] }, { Rows[2][1], Rows[2][2] } }).Determinant();
			const auto M01 = Mat2({ { Rows[1][0], Rows[1][2] }, { Rows[2][0], Rows[2][2] } }).Determinant();
			const auto M02 = Mat2({ { Rows[1][0], Rows[1][1] }, { Rows[2][0], Rows[2][1] } }).Determinant();

			const auto M10 = Mat2({ { Rows[0][1], Rows[0][2] }, { Rows[2][1], Rows[2][2] } }).Determinant();
			const auto M11 = Mat2({ { Rows[0][0], Rows[0][2] }, { Rows[2][0], Rows[2][2] } }).Determinant();
			const auto M12 = Mat2({ { Rows[1][0], Rows[1][1] }, { Rows[2][0], Rows[2][1] } }).Determinant();

			const auto M20 = Mat2({ { Rows[0][1], Rows[0][2] }, { Rows[1][1], Rows[1][2] } }).Determinant();
			const auto M21 = Mat2({ { Rows[0][0], Rows[0][2] }, { Rows[1][0], Rows[1][2] } }).Determinant();
			const auto M22 = Mat2({ { Rows[0][0], Rows[0][1] }, { Rows[1][0], Rows[1][1] } }).Determinant();

			return Mat3({
				{ M00, -M10, M20 },
				{ -M01, M11, -M21 },
				{ M02, -M12, M22 },
			}) * InvDet;
		}
		inline Mat3 Inverse() const { return Inverse(1.0f / Determinant()); }

		inline Mat3& operator=(const Mat3& rhs) { Rows[0] = rhs.Rows[0]; Rows[1] = rhs.Rows[1]; Rows[2] = rhs.Rows[2]; return *this; }
		inline const Mat3& operator+=(const Mat3& rhs) { Rows[0] += rhs.Rows[0]; Rows[1] += rhs.Rows[1]; Rows[2] += rhs.Rows[2]; return *this; }
		inline const Mat3& operator-=(const Mat3& rhs) { Rows[0] -= rhs.Rows[0]; Rows[1] -= rhs.Rows[1]; Rows[2] -= rhs.Rows[2]; return *this; }
		inline const Mat3& operator*=(const float rhs) { Rows[0] *= rhs; Rows[1] *= rhs; Rows[2] *= rhs; return *this; }
		inline const Mat3& operator/=(const float rhs) { Rows[0] /= rhs; Rows[1] /= rhs; Rows[2] /= rhs; return *this; }
		inline Vec3& operator[](const int i) { return Rows[i]; }

		inline Mat3& ToZero() { return (*this = Zero()); }
		inline Mat3& ToIdentity() { return (*this = Identity()); }

	private:
		std::array<Vec3, 3> Rows;
	};

	class Mat4
	{
	public:
		Mat4() : Rows{ Vec4(1.0f, 0.0f, 0.0f, 0.0f), Vec4(0.0f, 1.0f, 0.0f, 0.0f), Vec4(0.0f, 0.0f, 1.0f, 0.0f), Vec4(0.0f, 0.0f, 0.0f, 1.0f) } {}
		Mat4(const Vec4& row0, const Vec4& row1, const Vec4& row2, const Vec4& row3) : Rows{ row0, row1, row2, row3 } {}
		Mat4(const Mat4& rhs) : Rows{ rhs.Rows[0], rhs.Rows[1], rhs.Rows[2], rhs.Rows[3] } {}

		inline static Mat4 Identity() { return { { 1.0f, 0.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 1.0f, 0.0f }, { 0.0f, 0.0f, 0.0f, 1.0f } }; }
		inline static Mat4 Zero() { return { Vec4::Zero(), Vec4::Zero(), Vec4::Zero(), Vec4::Zero() }; }

		inline bool NearlyEqual(const Mat4& rhs, const float Epsilon = (std::numeric_limits<float>::epsilon)()) const { return Rows[0].NearlyEqual(rhs.Rows[0]) && Rows[1].NearlyEqual(rhs.Rows[1]) && Rows[2].NearlyEqual(rhs.Rows[2]) && Rows[3].NearlyEqual(rhs.Rows[3]); }

		inline bool operator==(const Mat4& rhs) const { return Rows[0] == rhs.Rows[0] && Rows[1] == rhs.Rows[1] && Rows[2] == rhs.Rows[2] && Rows[3] == rhs.Rows[3]; }
		inline Mat4 operator+(const Mat4& rhs) const { return { Rows[0] + rhs.Rows[0], Rows[1] + rhs.Rows[1], Rows[2] + rhs.Rows[2], Rows[3] + rhs.Rows[3] }; }
		inline Mat4 operator-(const Mat4& rhs) const { return { Rows[0] - rhs.Rows[0], Rows[1] - rhs.Rows[1], Rows[2] - rhs.Rows[2], Rows[3] - rhs.Rows[3] }; }
		inline Mat4 operator*(const float rhs) const { return { Rows[0] * rhs, Rows[1] * rhs, Rows[2] * rhs, Rows[3] * rhs }; }
		inline Vec4 operator*(const Vec4& rhs) const { return { Rows[0].Dot(rhs), Rows[1].Dot(rhs), Rows[2].Dot(rhs), Rows[3].Dot(rhs) }; }
		inline Mat4 operator*(const Mat4& rhs) const {
			//!< ŒŸŽZ—p : glm ‚Ìê‡‚ÍŠ|‚¯‚é‡˜‚ª‹t‚È‚Ì‚Å’ˆÓ
			//glm::make_mat4(static_cast<const float*>(rhs)) * glm::make_mat4(static_cast<const float*>(*this));
			const auto c0 = Vec4({ rhs.Rows[0][0],rhs.Rows[1][0], rhs.Rows[2][0], rhs.Rows[3][0] });
			const auto c1 = Vec4({ rhs.Rows[0][1],rhs.Rows[1][1], rhs.Rows[2][1], rhs.Rows[3][1] });
			const auto c2 = Vec4({ rhs.Rows[0][2],rhs.Rows[1][2], rhs.Rows[2][2], rhs.Rows[3][2] });
			const auto c3 = Vec4({ rhs.Rows[0][3],rhs.Rows[1][3], rhs.Rows[2][3], rhs.Rows[3][3] });
			return { 
				{ Rows[0].Dot(c0), Rows[0].Dot(c1), Rows[0].Dot(c2), Rows[0].Dot(c3) }, 
				{ Rows[1].Dot(c0), Rows[1].Dot(c1), Rows[1].Dot(c2), Rows[1].Dot(c3) },
				{ Rows[2].Dot(c0), Rows[2].Dot(c1), Rows[2].Dot(c2), Rows[2].Dot(c3) }, 
				{ Rows[3].Dot(c0), Rows[3].Dot(c1), Rows[3].Dot(c2), Rows[3].Dot(c3) } 
			};
		}
		inline Mat4 operator/(const float rhs) const { return { Rows[0] / rhs, Rows[1] / rhs, Rows[2] / rhs, Rows[3] / rhs }; }

		inline const Vec4& operator[](const int i) const { return Rows[i]; }
		inline operator const float* () const { return static_cast<const float*>(Rows[0]); }

		inline Mat4 Transpose() const {
			return { 
				{ Rows[0].X(), Rows[1].X(), Rows[2].X(), Rows[3].X() }, 
				{ Rows[0].Y(), Rows[1].Y(), Rows[2].Y(), Rows[3].Y() }, 
				{ Rows[0].Z(), Rows[1].Z(), Rows[2].Z(), Rows[3].Z() }, 
				{ Rows[0].W(), Rows[1].W(), Rows[2].W(), Rows[3].W() } 
			};
		}
		inline float Determinant() const {
			//!< ŒŸŽZ—p
			//glm::determinant(glm::make_mat4(static_cast<const float *>(*this)));
			//DirectX::XMMatrixDeterminant(DirectX::XMLoadFloat4x4(*this)).m128_f32[0];
			return Rows[0][0] * Mat3({ { Rows[1][1], Rows[1][2], Rows[1][3] }, { Rows[2][1], Rows[2][2], Rows[2][3] }, { Rows[3][1], Rows[3][2], Rows[3][3] } }).Determinant()
				- Rows[1][0] * Mat3({ { Rows[0][1], Rows[0][2], Rows[0][3] }, { Rows[2][1], Rows[2][2], Rows[2][3] }, { Rows[3][1], Rows[3][2], Rows[3][3] } }).Determinant()
				+ Rows[2][0] * Mat3({ { Rows[0][1], Rows[0][2], Rows[0][3] }, { Rows[1][1], Rows[1][2], Rows[1][3] }, { Rows[3][1], Rows[3][2], Rows[3][3] } }).Determinant()
				+ Rows[3][0] * Mat3({ { Rows[0][1], Rows[0][2], Rows[0][3] }, { Rows[1][1], Rows[1][2], Rows[1][3] }, { Rows[2][1], Rows[2][2], Rows[2][3] } }).Determinant();
		}
		inline Mat4 Inverse(const float InvDet) const {
			const auto M00 = Mat3({ { Rows[1][1], Rows[1][2], Rows[1][3] }, { Rows[2][1], Rows[2][2], Rows[2][3] }, { Rows[3][1], Rows[3][2], Rows[3][3] } }).Determinant();
			const auto M01 = Mat3({ { Rows[1][0], Rows[1][2], Rows[1][3] }, { Rows[2][0], Rows[2][2], Rows[2][3] }, { Rows[3][0], Rows[3][2], Rows[3][3] } }).Determinant();
			const auto M02 = Mat3({ { Rows[1][0], Rows[1][1], Rows[1][3] }, { Rows[2][0], Rows[2][1], Rows[2][3] }, { Rows[3][0], Rows[3][1], Rows[3][3] } }).Determinant();
			const auto M03 = Mat3({ { Rows[1][0], Rows[1][1], Rows[1][2] }, { Rows[2][0], Rows[2][1], Rows[2][2] }, { Rows[3][0], Rows[3][1], Rows[3][2] } }).Determinant();

			const auto M10 = Mat3({ { Rows[0][1], Rows[0][2], Rows[0][3] }, { Rows[2][1], Rows[2][2], Rows[2][3] }, { Rows[3][1], Rows[3][2], Rows[3][3] } }).Determinant();
			const auto M11 = Mat3({ { Rows[0][0], Rows[0][2], Rows[0][3] }, { Rows[2][0], Rows[2][2], Rows[2][3] }, { Rows[3][0], Rows[3][2], Rows[3][3] } }).Determinant();
			const auto M12 = Mat3({ { Rows[0][0], Rows[0][1], Rows[0][3] }, { Rows[2][0], Rows[2][1], Rows[2][3] }, { Rows[3][0], Rows[3][1], Rows[3][3] } }).Determinant();
			const auto M13 = Mat3({ { Rows[0][0], Rows[0][1], Rows[0][2] }, { Rows[2][0], Rows[2][1], Rows[2][2] }, { Rows[3][0], Rows[3][1], Rows[3][2] } }).Determinant();

			const auto M20 = Mat3({ { Rows[0][1], Rows[0][2], Rows[0][3] }, { Rows[1][1], Rows[1][2], Rows[1][3] }, { Rows[3][1], Rows[3][2], Rows[3][3] } }).Determinant();
			const auto M21 = Mat3({ { Rows[0][0], Rows[0][2], Rows[0][3] }, { Rows[1][0], Rows[1][2], Rows[1][3] }, { Rows[3][0], Rows[3][2], Rows[3][3] } }).Determinant();
			const auto M22 = Mat3({ { Rows[0][0], Rows[0][1], Rows[0][3] }, { Rows[1][0], Rows[1][1], Rows[1][3] }, { Rows[3][0], Rows[3][1], Rows[3][3] } }).Determinant();
			const auto M23 = Mat3({ { Rows[0][0], Rows[0][1], Rows[0][2] }, { Rows[1][0], Rows[1][1], Rows[1][2] }, { Rows[3][0], Rows[3][1], Rows[3][2] } }).Determinant();

			const auto M30 = Mat3({ { Rows[0][1], Rows[0][2], Rows[0][3] }, { Rows[1][1], Rows[1][2], Rows[1][3] }, { Rows[2][1], Rows[2][2], Rows[2][3] } }).Determinant();
			const auto M31 = Mat3({ { Rows[0][0], Rows[0][2], Rows[0][3] }, { Rows[1][0], Rows[1][2], Rows[1][3] }, { Rows[2][0], Rows[2][2], Rows[2][3] } }).Determinant();
			const auto M32 = Mat3({ { Rows[0][0], Rows[0][1], Rows[0][3] }, { Rows[1][0], Rows[1][1], Rows[1][3] }, { Rows[2][0], Rows[2][1], Rows[2][3] } }).Determinant();
			const auto M33 = Mat3({ { Rows[0][0], Rows[0][1], Rows[0][2] }, { Rows[1][0], Rows[1][1], Rows[1][2] }, { Rows[2][0], Rows[2][1], Rows[2][2] } }).Determinant();

			return Mat4({
				{ M00, -M10, M20, -M30 },
				{ -M01, M11, -M21, M31 },
				{ M02, -M12, M22, -M32 },
				{ -M03, M13, -M23, M33 },
			}) * InvDet;
		}
		inline Mat4 Inverse() const 
		{
			//!< ŒŸŽZ—p
			//glm::inverse(glm::make_mat4(static_cast<const float*>(*this)));
			//DirectX::XMMatrixInverse(nullptr, DirectX::XMLoadFloat4x4(*this));
			return Inverse(1.0f / Determinant());
		}

		inline Mat4& operator=(const Mat4& rhs) { Rows[0] = rhs.Rows[0]; Rows[1] = rhs.Rows[1]; Rows[2] = rhs.Rows[2]; Rows[3] = rhs.Rows[3]; return *this; }
		inline const Mat4& operator+=(const Mat4& rhs) { Rows[0] += rhs.Rows[0]; Rows[1] += rhs.Rows[1]; Rows[2] += rhs.Rows[2]; Rows[3] += rhs.Rows[3]; return *this; }
		inline const Mat4& operator-=(const Mat4& rhs) { Rows[0] -= rhs.Rows[0]; Rows[1] -= rhs.Rows[1]; Rows[2] -= rhs.Rows[2]; Rows[3] -= rhs.Rows[3]; return *this; }
		inline const Mat4& operator*=(const float rhs) { Rows[0] *= rhs; Rows[1] *= rhs; Rows[2] *= rhs; Rows[3] *= rhs; return *this; }
		inline const Mat4& operator/=(const float rhs) { Rows[0] /= rhs; Rows[1] /= rhs; Rows[2] /= rhs; Rows[3] /= rhs; return *this; }
		inline Vec4& operator[](const int i) { return Rows[i]; }
		inline operator float* () { return static_cast<float*>(Rows[0]); }

		inline Mat4& ToZero() { return (*this = Zero()); }
		inline Mat4& ToIdentity() { return (*this = Identity()); }

	private:
		std::array<Vec4, 4> Rows;
	};
}

