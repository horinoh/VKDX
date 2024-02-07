#pragma once

namespace Math 
{
	class Mat2 
	{
	public:
		Mat2() {}
		Mat2(const Vec2& Row0, const Vec2& Row1) : Rows{ Row0, Row1 } {}
		Mat2(const Mat2& rhs) : Rows{ rhs.Rows[0], rhs.Rows[1] } {}

		inline static Mat2 Identity() { return Mat2(); }
		inline static Mat2 Zero() { return { Vec2::Zero(), Vec2::Zero() }; }

		inline bool NearlyEqual(const Mat2& rhs, const float Epsilon = (std::numeric_limits<float>::epsilon)()) const { 
			return std::ranges::equal(Rows, rhs.Rows, [&](const Vec2& l, const Vec2& r) { return l.NearlyEqual(r, Epsilon); });
			//return Rows[0].NearlyEqual(rhs.Rows[0], Epsilon) && Rows[1].NearlyEqual(rhs.Rows[1], Epsilon); 
		}

		inline bool operator==(const Mat2& rhs) const {
			return std::ranges::equal(Rows, rhs.Rows);
			//return Rows[0] == rhs.Rows[0] && Rows[1] == rhs.Rows[1];
		}
		inline Mat2 operator+(const Mat2& rhs) const {
			Mat2 r; std::ranges::transform(Rows, rhs.Rows, std::begin(r.Rows), std::plus()); return r;
			//return { Rows[0] + rhs.Rows[0], Rows[1] + rhs.Rows[1] };
		}
		inline Mat2 operator-(const Mat2& rhs) const { 
			Mat2 r; std::ranges::transform(Rows, rhs.Rows, std::begin(r.Rows), std::minus()); return r;
			//return { Rows[0] - rhs.Rows[0], Rows[1] - rhs.Rows[1] }; 
		}
		inline Mat2 operator*(const float rhs) const { 
			Mat2 r; std::ranges::transform(Rows, std::begin(r.Rows), std::bind(std::multiplies(), std::placeholders::_1, rhs)); return r;
			//return { Rows[0] * rhs, Rows[1] * rhs }; 
		}
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
		inline Mat2 operator/(const float rhs) const { 
			Mat2 r; std::ranges::transform(Rows, std::begin(r.Rows), std::bind(std::divides(), std::placeholders::_1, rhs)); return r;
			//return { Rows[0] / rhs, Rows[1] / rhs };
		}

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

		inline Mat2& operator=(const Mat2& rhs) { 
			std::ranges::copy(rhs.Rows, std::begin(Rows));
			//Rows[0] = rhs.Rows[0]; Rows[1] = rhs.Rows[1]; 
			return *this; 
		}
		inline const Mat2& operator+=(const Mat2& rhs) {
			std::ranges::transform(Rows, rhs.Rows, std::begin(Rows), std::plus());
			//Rows[0] += rhs.Rows[0]; Rows[1] += rhs.Rows[1];
			return *this;
		}
		inline const Mat2& operator-=(const Mat2& rhs) {
			std::ranges::transform(Rows, rhs.Rows, std::begin(Rows), std::minus());
			//Rows[0] -= rhs.Rows[0]; Rows[1] -= rhs.Rows[1]; 
			return *this;
		}
		inline const Mat2& operator*=(const float rhs) {
			std::ranges::transform(Rows, std::begin(Rows), std::bind(std::multiplies(), std::placeholders::_1, rhs));
			//Rows[0] *= rhs; Rows[1] *= rhs;
			return *this;
		}
		inline const Mat2& operator/=(const float rhs) {
			std::ranges::transform(Rows, std::begin(Rows), std::bind(std::divides(), std::placeholders::_1, rhs));
			//Rows[0] /= rhs; Rows[1] /= rhs; 
			return *this;
		}
		inline Vec2& operator[](const int i) { return Rows[i]; }

		inline Mat2& ToZero() { return (*this = Zero()); }
		inline Mat2& ToIdentity() { return (*this = Identity()); }

	private:
		std::array<Vec2, 2> Rows = { Vec2(1.0f, 0.0f), Vec2(0.0f, 1.0f) };
	};

	class Mat3 
	{
	public:
		Mat3() {}
		Mat3(const Vec3& Row0, const Vec3& Row1, const Vec3& Row2) : Rows{ Row0, Row1, Row2 } {}
		Mat3(const Mat3& rhs) : Rows{ rhs.Rows[0], rhs.Rows[1], rhs.Rows[2] } {}

		inline static Mat3 Identity() { return Mat3(); }
		inline static Mat3 Zero() { return { Vec3::Zero(), Vec3::Zero(), Vec3::Zero() }; }

		inline bool NearlyEqual(const Mat3& rhs, const float Epsilon = (std::numeric_limits<float>::epsilon)()) const { 
			return std::ranges::equal(Rows, rhs.Rows, [&](const Vec3& l, const Vec3& r) { return l.NearlyEqual(r, Epsilon); });
			//return Rows[0].NearlyEqual(rhs.Rows[0], Epsilon) && Rows[1].NearlyEqual(rhs.Rows[1], Epsilon) && Rows[2].NearlyEqual(rhs.Rows[2], Epsilon); 
		}

		inline bool operator==(const Mat3& rhs) const { 
			return std::ranges::equal(Rows, rhs.Rows);
			//return Rows[0] == rhs.Rows[0] && Rows[1] == rhs.Rows[1] && Rows[2] == rhs.Rows[2];
		}
		inline Mat3 operator+(const Mat3& rhs) const { 
			Mat3 r; std::ranges::transform(Rows, rhs.Rows, std::begin(r.Rows), std::plus()); return r;
			//return { Rows[0] + rhs.Rows[0], Rows[1] + rhs.Rows[1], Rows[2] + rhs.Rows[2] }; 
		}
		inline Mat3 operator-(const Mat3& rhs) const { 
			Mat3 r; std::ranges::transform(Rows, rhs.Rows, std::begin(r.Rows), std::minus()); return r;
			//return { Rows[0] - rhs.Rows[0], Rows[1] - rhs.Rows[1], Rows[2] - rhs.Rows[2] };
		}
		inline Mat3 operator*(const float rhs) const {
			Mat3 r; std::ranges::transform(Rows, std::begin(r.Rows), std::bind(std::multiplies(), std::placeholders::_1, rhs)); return r;
			//return { Rows[0] * rhs, Rows[1] * rhs, Rows[2] * rhs }; 
		}
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
		inline Mat3 operator/(const float rhs) const { 
			Mat3 r; std::ranges::transform(Rows, std::begin(r.Rows), std::bind(std::divides(), std::placeholders::_1, rhs)); return r;
			//return { Rows[0] / rhs, Rows[1] / rhs, Rows[2] / rhs };
		}

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
#if 0
			return Rows[0][0] * Mat2({ { Rows[1][1], Rows[1][2] }, { Rows[2][1], Rows[2][2] } }).Determinant()
				- Rows[1][0] * Mat2({ { Rows[0][1], Rows[0][2] }, { Rows[2][1], Rows[2][2] } }).Determinant()
				+ Rows[2][0] * Mat2({ { Rows[0][1], Rows[0][2] }, { Rows[1][1], Rows[1][2] } }).Determinant();
#else
			float Det = 0.0f;
			for (int j = 0; j < 3; ++j) {
				Det += Rows[0][j] * Minor(0, j).Determinant() * static_cast<float>(pow(-1, j));
			}
			return Det;
#endif
		}
		inline Mat3 Inverse(const float InvDet) const {
			//!< ŒŸŽZ—p
			//glm::inverse(glm::make_mat3(static_cast<const float*>(*this)));
#if 0
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
#else
			Mat3 M;
			for (int i = 0; i < 3; ++i) {
				for (int j = 0; j < 3; ++j) {
					M[j][i] = Cofactor(i, j);
				}
			}
			return M * InvDet;
#endif
		}
		inline Mat3 Inverse() const { return Inverse(1.0f / Determinant()); }
		//!< ¬s—ñ (s‚Ü‚½‚Í—ñ‚ðŽæ‚èœ‚¢‚Ä‚Å‚«‚é¬‚³‚¢³•ûs—ñ)
		inline Mat2 Minor(const int Column, const int Row) const {
			Mat2 M;
			int r = 0;
			for (int j = 0; j < 3; ++j) {
				if (Row == j) { continue; }
				int c = 0;
				for (int i = 0; i < 3; ++i) {
					if (Column == i) { continue; }
					M[c][r] = (*this)[i][j];
					++c;
				}
				++r;
			}
			return M;
		}
		//!< —]ˆöŽq ¬s—ñŽ®‚É -1^(i + j) ‚ðŠ|‚¯‚Ä“¾‚ç‚ê‚é
		inline float Cofactor(const int Column, const int Row) const {
			return static_cast<float>(pow(-1, Column + 1 + Row + 1)) * Minor(Column, Row).Determinant(); 
		}

		inline Mat3& operator=(const Mat2& rhs) {
			Rows[0] = rhs[0]; Rows[1] = rhs[1];
			return *this;
		}
		inline Mat3& operator=(const Mat3& rhs) { 
			std::ranges::copy(rhs.Rows, std::begin(Rows));
			//Rows[0] = rhs.Rows[0]; Rows[1] = rhs.Rows[1]; Rows[2] = rhs.Rows[2];
			return *this; 
		}
		inline const Mat3& operator+=(const Mat3& rhs) { 
			std::ranges::transform(Rows, rhs.Rows, std::begin(Rows), std::plus());
			//Rows[0] += rhs.Rows[0]; Rows[1] += rhs.Rows[1]; Rows[2] += rhs.Rows[2];
			return *this; 
		}
		inline const Mat3& operator-=(const Mat3& rhs) {
			std::ranges::transform(Rows, rhs.Rows, std::begin(Rows), std::minus());
			//Rows[0] -= rhs.Rows[0]; Rows[1] -= rhs.Rows[1]; Rows[2] -= rhs.Rows[2];
			return *this; 
		}
		inline const Mat3& operator*=(const float rhs) { 
			std::ranges::transform(Rows, std::begin(Rows), std::bind(std::multiplies(), std::placeholders::_1, rhs));
			//Rows[0] *= rhs; Rows[1] *= rhs; Rows[2] *= rhs;
			return *this; 
		}
		inline const Mat3& operator/=(const float rhs) { 
			std::ranges::transform(Rows, std::begin(Rows), std::bind(std::divides(), std::placeholders::_1, rhs));
			//Rows[0] /= rhs; Rows[1] /= rhs; Rows[2] /= rhs;
			return *this;
		}
		inline Vec3& operator[](const int i) { return Rows[i]; }

		inline Mat3& ToZero() { return (*this = Zero()); }
		inline Mat3& ToIdentity() { return (*this = Identity()); }

	private:
		std::array<Vec3, 3> Rows = { Vec3(1.0f, 0.0f, 0.0f), Vec3(0.0f, 1.0f, 0.0f), Vec3(0.0f, 0.0f, 1.0f) };
	};

	class Mat4
	{
	public:
		Mat4() {}
		Mat4(const Vec4& Row0, const Vec4& Row1, const Vec4& Row2, const Vec4& Row3) : Rows{ Row0, Row1, Row2, Row3 } {}
		Mat4(const Mat4& rhs) : Rows{ rhs.Rows[0], rhs.Rows[1], rhs.Rows[2], rhs.Rows[3] } {}

		inline static Mat4 Identity() { return Mat4(); }
		inline static Mat4 Zero() { return { Vec4::Zero(), Vec4::Zero(), Vec4::Zero(), Vec4::Zero() }; }

		inline bool NearlyEqual(const Mat4& rhs, const float Epsilon = (std::numeric_limits<float>::epsilon)()) const { 
			return std::ranges::equal(Rows, rhs.Rows, [&](const Vec4& l, const Vec4& r) { return l.NearlyEqual(r, Epsilon); });
			//return Rows[0].NearlyEqual(rhs.Rows[0], Epsilon) && Rows[1].NearlyEqual(rhs.Rows[1], Epsilon) && Rows[2].NearlyEqual(rhs.Rows[2], Epsilon) && Rows[3].NearlyEqual(rhs.Rows[3], Epsilon); 
		}

		inline bool operator==(const Mat4& rhs) const { 
			return std::ranges::equal(Rows, rhs.Rows);
			//return Rows[0] == rhs.Rows[0] && Rows[1] == rhs.Rows[1] && Rows[2] == rhs.Rows[2] && Rows[3] == rhs.Rows[3]; 
		}
		inline Mat4 operator+(const Mat4& rhs) const { 
			Mat4 r; std::ranges::transform(Rows, rhs.Rows, std::begin(r.Rows), std::plus()); return r;
			//return { Rows[0] + rhs.Rows[0], Rows[1] + rhs.Rows[1], Rows[2] + rhs.Rows[2], Rows[3] + rhs.Rows[3] }; 
		}
		inline Mat4 operator-(const Mat4& rhs) const {
			Mat4 r; std::ranges::transform(Rows, rhs.Rows, std::begin(r.Rows), std::minus()); return r;
			//return { Rows[0] - rhs.Rows[0], Rows[1] - rhs.Rows[1], Rows[2] - rhs.Rows[2], Rows[3] - rhs.Rows[3] };
		}
		inline Mat4 operator*(const float rhs) const { 
			Mat4 r; std::ranges::transform(Rows, std::begin(r.Rows), std::bind(std::multiplies(), std::placeholders::_1, rhs)); return r;
			//return { Rows[0] * rhs, Rows[1] * rhs, Rows[2] * rhs, Rows[3] * rhs }; 
		}
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
		inline Mat4 operator/(const float rhs) const { 
			Mat4 r; std::ranges::transform(Rows, std::begin(r.Rows), std::bind(std::divides(), std::placeholders::_1, rhs)); return r;
			//return { Rows[0] / rhs, Rows[1] / rhs, Rows[2] / rhs, Rows[3] / rhs }; 
		}

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
#if 0
			return Rows[0][0] * Mat3({ { Rows[1][1], Rows[1][2], Rows[1][3] }, { Rows[2][1], Rows[2][2], Rows[2][3] }, { Rows[3][1], Rows[3][2], Rows[3][3] } }).Determinant()
				- Rows[1][0] * Mat3({ { Rows[0][1], Rows[0][2], Rows[0][3] }, { Rows[2][1], Rows[2][2], Rows[2][3] }, { Rows[3][1], Rows[3][2], Rows[3][3] } }).Determinant()
				+ Rows[2][0] * Mat3({ { Rows[0][1], Rows[0][2], Rows[0][3] }, { Rows[1][1], Rows[1][2], Rows[1][3] }, { Rows[3][1], Rows[3][2], Rows[3][3] } }).Determinant()
				+ Rows[3][0] * Mat3({ { Rows[0][1], Rows[0][2], Rows[0][3] }, { Rows[1][1], Rows[1][2], Rows[1][3] }, { Rows[2][1], Rows[2][2], Rows[2][3] } }).Determinant();
#else
			float Det = 0.0f;
			for (int j = 0; j < 4; ++j) {
				Det += Rows[0][j] * Minor(0, j).Determinant() * static_cast<float>(pow(-1, j));
			}
			return Det;
#endif
		}
		inline Mat4 Inverse(const float InvDet) const {
#if 0
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
#else
			Mat4 M;
			for (int i = 0; i < 4; ++i) {
				for (int j = 0; j < 4; ++j) {
					M[j][i] = Cofactor(i, j);
				}
			}
			return M * InvDet;
#endif
		}
		inline Mat4 Inverse() const {
			//!< ŒŸŽZ—p
			//glm::inverse(glm::make_mat4(static_cast<const float*>(*this)));
			//DirectX::XMMatrixInverse(nullptr, DirectX::XMLoadFloat4x4(*this));
			return Inverse(1.0f / Determinant());
		}
		//!< ¬s—ñ (s‚Ü‚½‚Í—ñ‚ðŽæ‚èœ‚¢‚Ä‚Å‚«‚é¬‚³‚¢³•ûs—ñ)
		inline Mat3 Minor(const int Column, const int Row) const {
			Mat3 M;
			int r = 0;
			for (int j = 0; j < 4; ++j) {
				if (Row == j) { continue; }
				int c = 0;
				for (int i = 0; i < 4; ++i) {
					if (Column == i) { continue; }
					M[c][r] = (*this)[i][j];
					++c;
				}
				++r;
			}
			return M;
		}
		//!< —]ˆöŽq ¬s—ñŽ®‚É -1^(i + j) ‚ðŠ|‚¯‚Ä“¾‚ç‚ê‚é
		inline float Cofactor(const int Column, const int Row) const {
			return static_cast<float>(pow(-1, Column + 1 + Row + 1)) * Minor(Column, Row).Determinant(); 
		}

		inline Mat4& operator=(const Mat2& rhs) {
			Rows[0] = rhs[0]; Rows[1] = rhs[1];
			return *this;
		}
		inline Mat4& operator=(const Mat3& rhs) {
			Rows[0] = rhs[0]; Rows[1] = rhs[1]; Rows[2] = rhs[2];
			return *this;
		}
		inline Mat4& operator=(const Mat4& rhs) { 
			std::ranges::copy(rhs.Rows, std::begin(Rows));
			//Rows[0] = rhs.Rows[0]; Rows[1] = rhs.Rows[1]; Rows[2] = rhs.Rows[2]; Rows[3] = rhs.Rows[3];
			return *this; 
		}
		inline const Mat4& operator+=(const Mat4& rhs) { 
			std::ranges::transform(Rows, rhs.Rows, std::begin(Rows), std::plus());
			//Rows[0] += rhs.Rows[0]; Rows[1] += rhs.Rows[1]; Rows[2] += rhs.Rows[2]; Rows[3] += rhs.Rows[3];
			return *this; 
		}
		inline const Mat4& operator-=(const Mat4& rhs) { 
			std::ranges::transform(Rows, rhs.Rows, std::begin(Rows), std::minus());
			//Rows[0] -= rhs.Rows[0]; Rows[1] -= rhs.Rows[1]; Rows[2] -= rhs.Rows[2]; Rows[3] -= rhs.Rows[3];
			return *this; 
		}
		inline const Mat4& operator*=(const float rhs) {
			std::ranges::transform(Rows, std::begin(Rows), std::bind(std::multiplies(), std::placeholders::_1, rhs));
			//Rows[0] *= rhs; Rows[1] *= rhs; Rows[2] *= rhs; Rows[3] *= rhs;
			return *this; 
		}
		inline const Mat4& operator/=(const float rhs) { 
			std::ranges::transform(Rows, std::begin(Rows), std::bind(std::divides(), std::placeholders::_1, rhs));
			//Rows[0] /= rhs; Rows[1] /= rhs; Rows[2] /= rhs; Rows[3] /= rhs;
			return *this; 
		}
		inline Vec4& operator[](const int i) { return Rows[i]; }
		inline operator float* () { return static_cast<float*>(Rows[0]); }

		inline Mat4& ToZero() { return (*this = Zero()); }
		inline Mat4& ToIdentity() { return (*this = Identity()); }

	private:
		std::array<Vec4, 4> Rows = { Vec4(1.0f, 0.0f, 0.0f, 0.0f), Vec4(0.0f, 1.0f, 0.0f, 0.0f), Vec4(0.0f, 0.0f, 1.0f, 0.0f), Vec4(0.0f, 0.0f, 0.0f, 1.0f) };
	};

	template<size_t N>
	class Mat 
	{
	public:
	private:
		std::array<Vec<N>, N> Rows;
	};

	template<size_t M, size_t N>
	class _Mat
	{
	public:
	private:
		std::array<Vec<N>, M> Rows;
	};
}

