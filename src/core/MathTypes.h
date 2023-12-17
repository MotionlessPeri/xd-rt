//
// Created by Frank on 2023/8/16.
//

#ifndef XD_RT_MATHTYPES_H
#define XD_RT_MATHTYPES_H

#include <cstdint>
#include <type_traits>
#include "Eigen/Dense"
#include "Eigen/Geometry"
namespace xd {
using Eigen::Matrix2f;
using Eigen::Matrix3f;
using Eigen::Matrix4f;
using Eigen::Vector2f;
using Eigen::Vector2i;
using Eigen::Vector3f;
using Eigen::Vector4f;
using Eigen::VectorXf;
template <typename CompType, uint32_t N, typename = void>
class VecNameHelper {
public:
	using Type = Eigen::Vector<CompType, N>;
};
template <typename CompType, uint32_t N>
class VecNameHelper<CompType, N, typename std::enable_if<(N == 1)>::type> {
public:
	using Type = CompType;
};
template <uint32_t N>
using VectorNf = typename VecNameHelper<float, N>::Type;
template <uint32_t N>
using VectorNi = typename VecNameHelper<int, N>::Type;

using Transform = Eigen::Transform<float, 3, 1, 0>;
typedef Vector4f ColorRGBA;
typedef Vector3f ColorRGB;

class FloatWithError;

constexpr float PI = 3.14159265f;
constexpr float TWO_PI = 2 * PI;
constexpr float INV_PI = 1.f / PI;
constexpr float INV_TWO_PI = 1.f / TWO_PI;
}  // namespace xd
#endif	// XD_RT_MATHTYPES_H
