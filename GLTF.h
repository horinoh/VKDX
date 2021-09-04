#pragma once

#include <cmath>
#include <fstream>

//!< KHR_texture_transform Šg’£
//!< https://github.com/KhronosGroup/glTF/tree/master/extensions/2.0/Khronos/KHR_texture_transform
#define USE_GLTF_EXT_TEX_TRANS

//!< MSFT_texture_dds Šg’£
//!< https://github.com/KhronosGroup/glTF/tree/master/extensions/2.0/Vendor/MSFT_texture_dds
#define USE_GLTF_EXT_TEX_DDS

static std::array<float, 3> operator+(const std::array<float, 3>& lhs, const std::array<float, 3>& rhs) { return std::array<float, 3>({ lhs[0] + rhs[0], lhs[1] + rhs[1], lhs[2] + rhs[2] }); }
static std::array<float, 3> operator*(const std::array<float, 3>& lhs, const float rhs) { return std::array<float, 3>({ lhs[0] * rhs, lhs[1] * rhs, lhs[2] * rhs }); }
static std::array<float, 3> operator*(const float rhs, const std::array<float, 3>& lhs) { return lhs * rhs; }

static std::array<float, 4> operator+(const std::array<float, 4>& lhs, const std::array<float, 4>& rhs) { return std::array<float, 4>({ lhs[0] + rhs[0], lhs[1] + rhs[1], lhs[2] + rhs[2], lhs[3] + rhs[3] }); }
static std::array<float, 4> operator*(const std::array<float, 4>& lhs, const float rhs) { return std::array<float, 4>({ lhs[0] * rhs, lhs[1] * rhs, lhs[2] * rhs, lhs[3] * rhs }); }
static std::array<float, 4> operator*(const float rhs, const std::array<float, 4>& lhs) { return lhs * rhs; }

namespace Gltf {
	static bool IsBinary(std::string_view Path) {
		std::ifstream In(data(Path), std::ios::in | std::ios::binary);
		if (!In.fail()) {
			In.seekg(0, std::ios_base::end);
			const auto Size = In.tellg();
			if (Size) {
				In.seekg(0, std::ios_base::beg);
				std::vector<std::byte> Buf(Size);
				In.read(reinterpret_cast<char*>(data(Buf)), 4);
				return std::string("glTF") == std::string(reinterpret_cast<const char*>(data(Buf)));
			}
		}
		return false;
	}
}

#ifdef USE_GLTF_FX
#include "FxGltf.h"
#endif

#ifdef USE_GLTF_TINY
#include "TinyGltf.h"
#endif