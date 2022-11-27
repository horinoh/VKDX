#pragma once

namespace Math 
{
	class Mat2 
	{
	public:
		Mat2() : _rows{ Vec2(1.0f, 0.0f), Vec2(0.0f, 1.0f) } {}
		Mat2(const Vec2& row0, const Vec2& row1) : _rows{ row0, row1 } {}
		Mat2(const Mat2& rhs) : _rows{ rhs._rows[0], rhs._rows[1] } {}

		inline static Mat2 Identity() { return { { 1.0f, 0.0f }, { 0.0f, 1.0f } }; }
		inline static Mat2 Zero() { return { Vec2::Zero(), Vec2::Zero() }; }

		inline bool NearlyEqual(const Mat2& rhs, const float Epsilon = (std::numeric_limits<float>::epsilon)()) const { return _rows[0].NearlyEqual(rhs._rows[0]) && _rows[1].NearlyEqual(rhs._rows[1]); }

		inline bool operator==(const Mat2& rhs) const { return _rows[0] == rhs._rows[0] && _rows[1] == rhs._rows[1]; }
		inline Mat2 operator+(const Mat2& rhs) const { return { _rows[0] + rhs._rows[0], _rows[1] + rhs._rows[1] }; }
		inline Mat2 operator-(const Mat2& rhs) const { return { _rows[0] - rhs._rows[0], _rows[1] - rhs._rows[1] }; }
		inline Mat2 operator*(const float rhs) const { return { _rows[0] * rhs, _rows[1] * rhs }; }
		inline Vec2 operator*(const Vec2& rhs) const { return { _rows[0].Dot(rhs), _rows[1].Dot(rhs) }; }
		inline Mat2 operator*(const Mat2& rhs) const {
			//!< ŒŸŽZ—p : glm ‚Ìê‡‚ÍŠ|‚¯‚é‡˜‚ª‹t‚È‚Ì‚Å’ˆÓ
			//glm::make_mat2(static_cast<const float*>(rhs)) * glm::make_mat2(static_cast<const float*>(*this));
			const auto c0 = Vec2({ rhs._rows[0][0], rhs._rows[1][0] });
			const auto c1 = Vec2({ rhs._rows[0][1], rhs._rows[1][1] });
			return { 
				{ _rows[0].Dot(c0), _rows[0].Dot(c1) }, 
				{ _rows[1].Dot(c0), _rows[1].Dot(c1) } 
			};
		}
		inline Mat2 operator/(const float rhs) const { return { _rows[0] / rhs, _rows[1] / rhs }; }

		inline const Vec2& operator[](const int i) const { return _rows[i]; }
		inline operator const float* () const { return static_cast<const float*>(_rows[0]); }

		inline Mat2 Transpose() const {
			return { 
				{ _rows[0].x(), _rows[1].x() }, 
				{ _rows[0].y(), _rows[1].y() } 
			};
		}
		inline float Determinant() const {
			//!< ŒŸŽZ—p
			//glm::determinant(glm::make_mat2(static_cast<const float *>(*this)));
			return _rows[0][0] * _rows[1][1] - _rows[1][0] * _rows[0][1];
		}
		inline Mat2 Inverse(const float InvDet) const {
			//!< ŒŸŽZ—p
			//glm::inverse(glm::make_mat2(static_cast<const float*>(*this)));
			return Mat2({ { _rows[1][1], -_rows[0][1] }, { -_rows[1][0], _rows[0][0] } }) * InvDet;
		}
		inline Mat2 Inverse() const { Inverse(1.0f / Determinant()); }

		inline Mat2& operator=(const Mat2& rhs) { _rows[0] = rhs._rows[0]; _rows[1] = rhs._rows[1]; return *this; }
		inline const Mat2& operator+=(const Mat2& rhs) { _rows[0] += rhs._rows[0]; _rows[1] += rhs._rows[1]; return *this; }
		inline const Mat2& operator-=(const Mat2& rhs) { _rows[0] -= rhs._rows[0]; _rows[1] -= rhs._rows[1]; return *this; }
		inline const Mat2& operator*=(const float rhs) { _rows[0] *= rhs; _rows[1] *= rhs; return *this; }
		inline const Mat2& operator/=(const float rhs) { _rows[0] /= rhs; _rows[1] /= rhs; return *this; }
		inline Vec2& operator[](const int i) { return _rows[i]; }

		inline Mat2& ToZero() { return (*this = Zero()); }
		inline Mat2& ToIdentity() { return (*this = Identity()); }

	private:
		Vec2 _rows[2];
		//glm::mat2;
	};

	class Mat3 
	{
	public:
		Mat3() : _rows{ Vec3(1.0f, 0.0f, 0.0f), Vec3(0.0f, 1.0f, 0.0f), Vec3(0.0f, 0.0f, 1.0f) } {}
		Mat3(const Vec3& row0, const Vec3& row1, const Vec3& row2) : _rows{ row0, row1, row2 } {}
		Mat3(const Mat3& rhs) : _rows{ rhs._rows[0], rhs._rows[1], rhs._rows[2] } {}

		inline static Mat3 Identity() { return { { 1.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 0.0f, 1.0f } }; }
		inline static Mat3 Zero() { return { Vec3::Zero(), Vec3::Zero(), Vec3::Zero() }; }

		inline bool NearlyEqual(const Mat3& rhs, const float Epsilon = (std::numeric_limits<float>::epsilon)()) const { return _rows[0].NearlyEqual(rhs._rows[0]) && _rows[1].NearlyEqual(rhs._rows[1]) && _rows[2].NearlyEqual(rhs._rows[2]); }

		inline bool operator==(const Mat3& rhs) const { return _rows[0] == rhs._rows[0] && _rows[1] == rhs._rows[1] && _rows[2] == rhs._rows[2]; }
		inline Mat3 operator+(const Mat3& rhs) const { return { _rows[0] + rhs._rows[0], _rows[1] + rhs._rows[1], _rows[2] + rhs._rows[2] }; }
		inline Mat3 operator-(const Mat3& rhs) const { return { _rows[0] - rhs._rows[0], _rows[1] - rhs._rows[1], _rows[2] - rhs._rows[2] }; }
		inline Mat3 operator*(const float rhs) const { return { _rows[0] * rhs, _rows[1] * rhs, _rows[2] * rhs }; }
		inline Vec3 operator*(const Vec3& rhs) const { return { _rows[0].Dot(rhs), _rows[1].Dot(rhs), _rows[2].Dot(rhs) }; }
		inline Mat3 operator*(const Mat3& rhs) const {
			//!< ŒŸŽZ—p : glm ‚Ìê‡‚ÍŠ|‚¯‚é‡˜‚ª‹t‚È‚Ì‚Å’ˆÓ
			//glm::make_mat3(static_cast<const float*>(rhs)) * glm::make_mat3(static_cast<const float*>(*this));
			const auto c0 = Vec3({ rhs._rows[0][0],rhs._rows[1][0], rhs._rows[2][0] });
			const auto c1 = Vec3({ rhs._rows[0][1],rhs._rows[1][1], rhs._rows[2][1] });
			const auto c2 = Vec3({ rhs._rows[0][2],rhs._rows[1][2], rhs._rows[2][2] });
			return { 
				{ _rows[0].Dot(c0), _rows[0].Dot(c1), _rows[0].Dot(c2) }, 
				{ _rows[1].Dot(c0), _rows[1].Dot(c1), _rows[1].Dot(c2) }, 
				{ _rows[2].Dot(c0), _rows[2].Dot(c1), _rows[2].Dot(c2) } 
			};
		}
		inline Mat3 operator/(const float rhs) const { return { _rows[0] / rhs, _rows[1] / rhs, _rows[2] / rhs }; }

		inline const Vec3& operator[](const int i) const { return _rows[i]; }
		inline operator const float* () const { return static_cast<const float*>(_rows[0]); }

		inline Mat3 Transpose() const {
			return {
				{ _rows[0].x(), _rows[1].x(), _rows[2].x() },
				{ _rows[0].y(), _rows[1].y(), _rows[2].y() },
				{ _rows[0].z(), _rows[1].z(), _rows[2].z() }
			};
		}
		inline float Determinant() const {
			//!< ŒŸŽZ—p
			//glm::determinant(glm::make_mat3(static_cast<const float*>(*this)));
			return _rows[0][0] * Mat2({ { _rows[1][1], _rows[1][2] }, { _rows[2][1], _rows[2][2] } }).Determinant()
				- _rows[1][0] * Mat2({ { _rows[0][1], _rows[0][2] }, { _rows[2][1], _rows[2][2] } }).Determinant()
				+ _rows[2][0] * Mat2({ { _rows[0][1], _rows[0][2] }, { _rows[1][1], _rows[1][2] } }).Determinant();
		}
		inline Mat3 Inverse(const float InvDet) const {
			//!< ŒŸŽZ—p
			//glm::inverse(glm::make_mat3(static_cast<const float*>(*this)));
			const auto M00 = Mat2({ { _rows[1][1], _rows[1][2] }, { _rows[2][1], _rows[2][2] } }).Determinant();
			const auto M01 = Mat2({ { _rows[1][0], _rows[1][2] }, { _rows[2][0], _rows[2][2] } }).Determinant();
			const auto M02 = Mat2({ { _rows[1][0], _rows[1][1] }, { _rows[2][0], _rows[2][1] } }).Determinant();

			const auto M10 = Mat2({ { _rows[0][1], _rows[0][2] }, { _rows[2][1], _rows[2][2] } }).Determinant();
			const auto M11 = Mat2({ { _rows[0][0], _rows[0][2] }, { _rows[2][0], _rows[2][2] } }).Determinant();
			const auto M12 = Mat2({ { _rows[1][0], _rows[1][1] }, { _rows[2][0], _rows[2][1] } }).Determinant();

			const auto M20 = Mat2({ { _rows[0][1], _rows[0][2] }, { _rows[1][1], _rows[1][2] } }).Determinant();
			const auto M21 = Mat2({ { _rows[0][0], _rows[0][2] }, { _rows[1][0], _rows[1][2] } }).Determinant();
			const auto M22 = Mat2({ { _rows[0][0], _rows[0][1] }, { _rows[1][0], _rows[1][1] } }).Determinant();

			return Mat3({
				{ M00, -M10, M20 },
				{ -M01, M11, -M21 },
				{ M02, -M12, M22 },
			}) * InvDet;
		}
		inline Mat3 Inverse() const { return Inverse(1.0f / Determinant()); }

		inline Mat3& operator=(const Mat3& rhs) { _rows[0] = rhs._rows[0]; _rows[1] = rhs._rows[1]; _rows[2] = rhs._rows[2]; return *this; }
		inline const Mat3& operator+=(const Mat3& rhs) { _rows[0] += rhs._rows[0]; _rows[1] += rhs._rows[1]; _rows[2] += rhs._rows[2]; return *this; }
		inline const Mat3& operator-=(const Mat3& rhs) { _rows[0] -= rhs._rows[0]; _rows[1] -= rhs._rows[1]; _rows[2] -= rhs._rows[2]; return *this; }
		inline const Mat3& operator*=(const float rhs) { _rows[0] *= rhs; _rows[1] *= rhs; _rows[2] *= rhs; return *this; }
		inline const Mat3& operator/=(const float rhs) { _rows[0] /= rhs; _rows[1] /= rhs; _rows[2] /= rhs; return *this; }
		inline Vec3& operator[](const int i) { return _rows[i]; }

		inline Mat3& ToZero() { return (*this = Zero()); }
		inline Mat3& ToIdentity() { return (*this = Identity()); }

	//private:
		Vec3 _rows[3];
		//glm::mat3;
		//XMFLOAT3X3
	};

	class Mat4
	{
	public:
		Mat4() : _rows{ Vec4(1.0f, 0.0f, 0.0f, 0.0f), Vec4(0.0f, 1.0f, 0.0f, 0.0f), Vec4(0.0f, 0.0f, 1.0f, 0.0f), Vec4(0.0f, 0.0f, 0.0f, 1.0f) } {}
		Mat4(const Vec4& row0, const Vec4& row1, const Vec4& row2, const Vec4& row3) : _rows{ row0, row1, row2, row3 } {}
		Mat4(const Mat4& rhs) : _rows{ rhs._rows[0], rhs._rows[1], rhs._rows[2], rhs._rows[3] } {}

		inline static Mat4 Identity() { return { { 1.0f, 0.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 1.0f, 0.0f }, { 0.0f, 0.0f, 0.0f, 1.0f } }; }
		inline static Mat4 Zero() { return { Vec4::Zero(), Vec4::Zero(), Vec4::Zero(), Vec4::Zero() }; }

		inline bool NearlyEqual(const Mat4& rhs, const float Epsilon = (std::numeric_limits<float>::epsilon)()) const { return _rows[0].NearlyEqual(rhs._rows[0]) && _rows[1].NearlyEqual(rhs._rows[1]) && _rows[2].NearlyEqual(rhs._rows[2]) && _rows[3].NearlyEqual(rhs._rows[3]); }

		inline bool operator==(const Mat4& rhs) const { return _rows[0] == rhs._rows[0] && _rows[1] == rhs._rows[1] && _rows[2] == rhs._rows[2] && _rows[3] == rhs._rows[3]; }
		inline Mat4 operator+(const Mat4& rhs) const { return { _rows[0] + rhs._rows[0], _rows[1] + rhs._rows[1], _rows[2] + rhs._rows[2], _rows[3] + rhs._rows[3] }; }
		inline Mat4 operator-(const Mat4& rhs) const { return { _rows[0] - rhs._rows[0], _rows[1] - rhs._rows[1], _rows[2] - rhs._rows[2], _rows[3] - rhs._rows[3] }; }
		inline Mat4 operator*(const float rhs) const { return { _rows[0] * rhs, _rows[1] * rhs, _rows[2] * rhs, _rows[3] * rhs }; }
		inline Vec4 operator*(const Vec4& rhs) const { return { _rows[0].Dot(rhs), _rows[1].Dot(rhs), _rows[2].Dot(rhs), _rows[3].Dot(rhs) }; }
		inline Mat4 operator*(const Mat4& rhs) const {
			//!< ŒŸŽZ—p : glm ‚Ìê‡‚ÍŠ|‚¯‚é‡˜‚ª‹t‚È‚Ì‚Å’ˆÓ
			//glm::make_mat4(static_cast<const float*>(rhs)) * glm::make_mat4(static_cast<const float*>(*this));
			const auto c0 = Vec4({ rhs._rows[0][0],rhs._rows[1][0], rhs._rows[2][0], rhs._rows[3][0] });
			const auto c1 = Vec4({ rhs._rows[0][1],rhs._rows[1][1], rhs._rows[2][1], rhs._rows[3][1] });
			const auto c2 = Vec4({ rhs._rows[0][2],rhs._rows[1][2], rhs._rows[2][2], rhs._rows[3][2] });
			const auto c3 = Vec4({ rhs._rows[0][3],rhs._rows[1][3], rhs._rows[2][3], rhs._rows[3][3] });
			return { 
				{ _rows[0].Dot(c0), _rows[0].Dot(c1), _rows[0].Dot(c2), _rows[0].Dot(c3) }, 
				{ _rows[1].Dot(c0), _rows[1].Dot(c1), _rows[1].Dot(c2), _rows[1].Dot(c3) },
				{ _rows[2].Dot(c0), _rows[2].Dot(c1), _rows[2].Dot(c2), _rows[2].Dot(c3) }, 
				{ _rows[3].Dot(c0), _rows[3].Dot(c1), _rows[3].Dot(c2), _rows[3].Dot(c3) } 
			};
		}
		inline Mat4 operator/(const float rhs) const { return { _rows[0] / rhs, _rows[1] / rhs, _rows[2] / rhs, _rows[3] / rhs }; }

		inline const Vec4& operator[](const int i) const { return _rows[i]; }
		inline operator const float* () const { return static_cast<const float*>(_rows[0]); }

		inline Mat4 Transpose() const {
			return { 
				{ _rows[0].x(), _rows[1].x(), _rows[2].x(), _rows[3].x() }, 
				{ _rows[0].y(), _rows[1].y(), _rows[2].y(), _rows[3].y() }, 
				{ _rows[0].z(), _rows[1].z(), _rows[2].z(), _rows[3].z() }, 
				{ _rows[0].w(), _rows[1].w(), _rows[2].w(), _rows[3].w() } 
			};
		}
		inline float Determinant() const {
			//!< ŒŸŽZ—p
			//glm::determinant(glm::make_mat4(static_cast<const float *>(*this)));
			//DirectX::XMMatrixDeterminant(DirectX::XMLoadFloat4x4(*this)).m128_f32[0];
			return _rows[0][0] * Mat3({ { _rows[1][1], _rows[1][2], _rows[1][3] }, { _rows[2][1], _rows[2][2], _rows[2][3] }, { _rows[3][1], _rows[3][2], _rows[3][3] } }).Determinant()
				- _rows[1][0] * Mat3({ { _rows[0][1], _rows[0][2], _rows[0][3] }, { _rows[2][1], _rows[2][2], _rows[2][3] }, { _rows[3][1], _rows[3][2], _rows[3][3] } }).Determinant()
				+ _rows[2][0] * Mat3({ { _rows[0][1], _rows[0][2], _rows[0][3] }, { _rows[1][1], _rows[1][2], _rows[1][3] }, { _rows[3][1], _rows[3][2], _rows[3][3] } }).Determinant()
				+ _rows[3][0] * Mat3({ { _rows[0][1], _rows[0][2], _rows[0][3] }, { _rows[1][1], _rows[1][2], _rows[1][3] }, { _rows[2][1], _rows[2][2], _rows[2][3] } }).Determinant();
		}
		inline Mat4 Inverse(const float InvDet) const {
			const auto M00 = Mat3({ { _rows[1][1], _rows[1][2], _rows[1][3] }, { _rows[2][1], _rows[2][2], _rows[2][3] }, { _rows[3][1], _rows[3][2], _rows[3][3] } }).Determinant();
			const auto M01 = Mat3({ { _rows[1][0], _rows[1][2], _rows[1][3] }, { _rows[2][0], _rows[2][2], _rows[2][3] }, { _rows[3][0], _rows[3][2], _rows[3][3] } }).Determinant();
			const auto M02 = Mat3({ { _rows[1][0], _rows[1][1], _rows[1][3] }, { _rows[2][0], _rows[2][1], _rows[2][3] }, { _rows[3][0], _rows[3][1], _rows[3][3] } }).Determinant();
			const auto M03 = Mat3({ { _rows[1][0], _rows[1][1], _rows[1][2] }, { _rows[2][0], _rows[2][1], _rows[2][2] }, { _rows[3][0], _rows[3][1], _rows[3][2] } }).Determinant();

			const auto M10 = Mat3({ { _rows[0][1], _rows[0][2], _rows[0][3] }, { _rows[2][1], _rows[2][2], _rows[2][3] }, { _rows[3][1], _rows[3][2], _rows[3][3] } }).Determinant();
			const auto M11 = Mat3({ { _rows[0][0], _rows[0][2], _rows[0][3] }, { _rows[2][0], _rows[2][2], _rows[2][3] }, { _rows[3][0], _rows[3][2], _rows[3][3] } }).Determinant();
			const auto M12 = Mat3({ { _rows[0][0], _rows[0][1], _rows[0][3] }, { _rows[2][0], _rows[2][1], _rows[2][3] }, { _rows[3][0], _rows[3][1], _rows[3][3] } }).Determinant();
			const auto M13 = Mat3({ { _rows[0][0], _rows[0][1], _rows[0][2] }, { _rows[2][0], _rows[2][1], _rows[2][2] }, { _rows[3][0], _rows[3][1], _rows[3][2] } }).Determinant();

			const auto M20 = Mat3({ { _rows[0][1], _rows[0][2], _rows[0][3] }, { _rows[1][1], _rows[1][2], _rows[1][3] }, { _rows[3][1], _rows[3][2], _rows[3][3] } }).Determinant();
			const auto M21 = Mat3({ { _rows[0][0], _rows[0][2], _rows[0][3] }, { _rows[1][0], _rows[1][2], _rows[1][3] }, { _rows[3][0], _rows[3][2], _rows[3][3] } }).Determinant();
			const auto M22 = Mat3({ { _rows[0][0], _rows[0][1], _rows[0][3] }, { _rows[1][0], _rows[1][1], _rows[1][3] }, { _rows[3][0], _rows[3][1], _rows[3][3] } }).Determinant();
			const auto M23 = Mat3({ { _rows[0][0], _rows[0][1], _rows[0][2] }, { _rows[1][0], _rows[1][1], _rows[1][2] }, { _rows[3][0], _rows[3][1], _rows[3][2] } }).Determinant();

			const auto M30 = Mat3({ { _rows[0][1], _rows[0][2], _rows[0][3] }, { _rows[1][1], _rows[1][2], _rows[1][3] }, { _rows[2][1], _rows[2][2], _rows[2][3] } }).Determinant();
			const auto M31 = Mat3({ { _rows[0][0], _rows[0][2], _rows[0][3] }, { _rows[1][0], _rows[1][2], _rows[1][3] }, { _rows[2][0], _rows[2][2], _rows[2][3] } }).Determinant();
			const auto M32 = Mat3({ { _rows[0][0], _rows[0][1], _rows[0][3] }, { _rows[1][0], _rows[1][1], _rows[1][3] }, { _rows[2][0], _rows[2][1], _rows[2][3] } }).Determinant();
			const auto M33 = Mat3({ { _rows[0][0], _rows[0][1], _rows[0][2] }, { _rows[1][0], _rows[1][1], _rows[1][2] }, { _rows[2][0], _rows[2][1], _rows[2][2] } }).Determinant();

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

		inline Mat4& operator=(const Mat4& rhs) { _rows[0] = rhs._rows[0]; _rows[1] = rhs._rows[1]; _rows[2] = rhs._rows[2]; _rows[3] = rhs._rows[3]; return *this; }
		inline const Mat4& operator+=(const Mat4& rhs) { _rows[0] += rhs._rows[0]; _rows[1] += rhs._rows[1]; _rows[2] += rhs._rows[2]; _rows[3] += rhs._rows[3]; return *this; }
		inline const Mat4& operator-=(const Mat4& rhs) { _rows[0] -= rhs._rows[0]; _rows[1] -= rhs._rows[1]; _rows[2] -= rhs._rows[2]; _rows[3] -= rhs._rows[3]; return *this; }
		inline const Mat4& operator*=(const float rhs) { _rows[0] *= rhs; _rows[1] *= rhs; _rows[2] *= rhs; _rows[3] *= rhs; return *this; }
		inline const Mat4& operator/=(const float rhs) { _rows[0] /= rhs; _rows[1] /= rhs; _rows[2] /= rhs; _rows[3] /= rhs; return *this; }
		inline Vec4& operator[](const int i) { return _rows[i]; }
		inline operator float* () { return static_cast<float*>(_rows[0]); }

		inline Mat4& ToZero() { return (*this = Zero()); }
		inline Mat4& ToIdentity() { return (*this = Identity()); }

	private:
		Vec4 _rows[4];
		//glm::mat4;
		//XMFLOAT4X4
	};
}

