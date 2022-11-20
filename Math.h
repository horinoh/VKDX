#pragma once

//#define GLM_FORCE_RADIANS
//#define GLM_FORCE_DEPTH_ZERO_TO_ONE
//#include <glm/glm.hpp>
//#include <glm/gtc/type_ptr.hpp>

//#include <DirectXMath.h>

#include "Vec.h"
#include "Mat.h"
#include "Quat.h"

template<typename T>
static [[nodiscard]] T Sign(const T rhs) { return static_cast<T>((rhs > 0) - (rhs < 0)); }

static [[nodiscard]] Math::Vec2 operator*(const float lhs, const Math::Vec2& rhs) { return rhs * lhs; }
static [[nodiscard]] Math::Vec3 operator*(const float lhs, const Math::Vec3& rhs) { return rhs * lhs; }
static [[nodiscard]] Math::Vec4 operator*(const float lhs, const Math::Vec4& rhs) { return rhs * lhs; }

static [[nodiscard]] Math::Mat2 operator*(const float lhs, const Math::Mat2& rhs) { return rhs * lhs; }
static [[nodiscard]] Math::Mat3 operator*(const float lhs, const Math::Mat3& rhs) { return rhs * lhs; }
static [[nodiscard]] Math::Mat4 operator*(const float lhs, const Math::Mat4& rhs) { return rhs * lhs; }

static [[nodiscard]] Math::Quat operator*(const float lhs, const Math::Quat& rhs) { return rhs * lhs; }

#ifdef _DEBUG
static std::ostream& operator<<(std::ostream& lhs, const Math::Vec2& rhs) { lhs << rhs.x() << ", " << rhs.y() << ", " << std::endl; return lhs; }
static std::ostream& operator<<(std::ostream& lhs, const Math::Vec3& rhs) { lhs << rhs.x() << ", " << rhs.y() << ", " << rhs.z() << ", " << std::endl; return lhs; }
static std::ostream& operator<<(std::ostream& lhs, const Math::Vec4& rhs) { lhs << rhs.x() << ", " << rhs.y() << ", " << rhs.z() << ", " << rhs.w() << ", " << std::endl; return lhs; }
static std::ostream& operator<<(std::ostream& lhs, const Math::Mat2& rhs) { lhs << rhs[0] << rhs[1]; return lhs; }
static std::ostream& operator<<(std::ostream& lhs, const Math::Mat3& rhs) { lhs << rhs[0] << rhs[1] << rhs[2];  return lhs; }
static std::ostream& operator<<(std::ostream& lhs, const Math::Mat4& rhs) { lhs << rhs[0] << rhs[1] << rhs[2] << rhs[3]; return lhs; }
static std::ostream& operator<<(std::ostream& lhs, const Math::Quat& rhs) { lhs << rhs.x() << ", " << rhs.y() << ", " << rhs.z() << ", " << rhs.w() << ", " << std::endl; return lhs; }
#endif