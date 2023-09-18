//
// Created by Frank on 2023/8/19.
//

#ifndef XD_RT_MATHUTIL_H
#define XD_RT_MATHUTIL_H
#include <cstdint>
#include <valarray>
#include "MathType.h"
namespace xd {
/**
 * Solve quadratic equation in real numbers.
 * If the equation has no solution, x1 x2 remains unchanged
 * If the equation has 1 solution, the solution is assign to x1
 * If the equation has 2 solutions, the smaller solution is assign to x1, the bigger is assign to x2
 * @param a coeff of quadratic term
 * @param b coeff of linear term
 * @param c coeff of constant
 * @param x1 the first solution
 * @param x2 the second solution
 * @return the count of equations
 */
inline uint32_t solveQuadraticReal(float a, float b, float c, float& x1, float& x2)
{
	const float delta = b * b - 4 * a * c;
	constexpr float eps = 1e-6;
	if (delta < -eps) {
		return 0;
	}
	else if (delta < eps) {
		x1 = -b / (2 * a);
		return 1;
	}
	else {
		x1 = (-b + std::sqrtf(delta)) / (2 * a);
		x2 = (-b - std::sqrtf(delta)) / (2 * a);
		if (x1 > x2)
			std::swap(x1, x2);
		return 2;
	}
}

template <typename T>
T interpolateWithBaryCoords(const std::array<T, 3>& inputs, const Vector3f& baryCoord)
{
	return baryCoord.x() * inputs[0] + baryCoord.y() * inputs[1] + baryCoord.z() * inputs[2];
}

/**
 * given a direction, return [theta, phi] where theta is the angle with z axis and
 * phi is the angle with x axis
 * @param dir the direction on the surface
 * @return [theta, phi]
 */
inline std::pair<float, float> getSphereThetaPhi(const Vector3f& dir)
{
	float phi = std::atan2(dir.y(), dir.x());
	if (phi < 0)
		phi = TWO_PI + phi;
	const float theta = std::acos(dir.z());
	return {theta, phi};
}
inline Vector2f getSphereUV(const Vector3f& dir)
{
	const auto [theta, phi] = getSphereThetaPhi(dir);
	return {theta * INV_PI, phi * INV_TWO_PI};
}
}  // namespace xd
#endif	// XD_RT_MATHUTIL_H
