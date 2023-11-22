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

constexpr inline float toRadians(const float degree)
{
	return degree / 180.f * PI;
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
	return {phi * INV_TWO_PI, theta * INV_PI};
}
inline Vector3f getSphereDirFromThetaPhi(const float phi, const float theta)
{
	const float sinTheta = std::sinf(theta);
	const float cosTheta = std::cosf(theta);
	const float sinPhi = std::sinf(phi);
	const float cosPhi = std::cosf(phi);
	return {sinTheta * cosPhi, sinTheta * sinPhi, cosTheta};
}
inline Vector3f getSphereDirFromUV(const Vector2f& uv)
{
	const float phi = uv.x() * TWO_PI;
	const float theta = uv.y() * PI;
	return getSphereDirFromThetaPhi(phi, theta);
}

inline Vector3f reflected(const Vector3f& i, const Vector3f& n)
{
	return 2 * n.dot(i) * n - i;
}
inline std::tuple<Vector3f, Vector3f> coordSystem(const Vector3f& v1)
{
	Vector3f v2{-v1.z(), 0, v1.x()};
	v2.normalize();
	Vector3f v3 = v1.cross(v2);
	return {v2, v3};
}
inline float balanceHeuristic(uint32_t numF, float pdfF, uint32_t numG, float pdfG)
{
	return (numF * pdfF) / (numF * pdfF + numG * pdfG);
}
inline float powerHeuristic(uint32_t numF, float pdfF, uint32_t numG, float pdfG)
{
	const float f = numF * pdfF, g = numG * pdfG;
	return (f * f) / (f * f + g * g);
}
inline float rgbToLuminance(const ColorRGB& color)
{
	return color.x() * 0.212671f + color.y() * 0.715160f + color.z() * 0.072169f;
}
}  // namespace xd
#endif	// XD_RT_MATHUTIL_H
