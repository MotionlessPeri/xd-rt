//
// Created by Frank on 2023/8/16.
//

#ifndef XD_RT_MATHTYPE_H
#define XD_RT_MATHTYPE_H

#include <Eigen/Dense>

namespace xd {
using Eigen::Matrix2f;
using Eigen::Matrix3f;
using Eigen::Matrix4f;
using Eigen::Vector2f;
using Eigen::Vector3f;
using Eigen::Vector4f;
typedef Vector4f ColorRGBA;
typedef Vector3f ColorRGB;
constexpr float PI = 3.14159265f;
constexpr float TWO_PI = 2 * PI;
constexpr float INV_PI = 1.f / PI;
constexpr float INV_TWO_PI = 1.f / TWO_PI;
}  // namespace xd
#endif	// XD_RT_MATHTYPE_H
