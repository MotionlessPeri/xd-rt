//
// Created by Frank on 2023/8/19.
//

#ifndef XD_RT_MATHUTIL_H
#define XD_RT_MATHUTIL_H
#include <cstdint>
#include <valarray>
#include "MathTypes.h"

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
	if (delta < 0.f) {
		return 0;
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

constexpr inline float toRadians(float degree)
{
	return degree / 180.f * PI;
}

constexpr inline double toRadians(double degree)
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

inline Vector3f coordSystem(const Vector3f& z, const Vector3f& x, bool rightHanded = true)
{
	Vector3f y = z.cross(x);
	if (!rightHanded) {
		y = -y;
	}
	return y;
}

inline Matrix3f buildFrameMatrix(const Vector3f& x, const Vector3f& y, const Vector3f& z)
{
	Matrix3f ret;
	ret << x, y, z;
	return ret;
}
inline std::tuple<Vector3f, Vector3f> coordSystem(const Vector3f& z, bool rightHanded = true)
{
	Vector3f x{-z.z(), 0, z.x()};
	return {x, coordSystem(z, x, rightHanded)};
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
inline bool fuzzyEqual(float a, float b, float eps = 1e-5f)
{
	return std::fabs(b - a) < eps;
}

constexpr float MACHINE_EPSILON = std::numeric_limits<float>::epsilon() / 2;

template <int N>
constexpr float floatingGamma()
{
	return (N * MACHINE_EPSILON) / (1 - N * MACHINE_EPSILON);
}

inline float nextFloatUp(float f)
{
	if (std::isinf(f) && f > 0.f)
		return f;
	if (f == -0.f)
		return 0.f;
	auto bits = std::bit_cast<uint32_t>(f);
	if (f >= 0)
		++bits;
	else
		--bits;
	return std::bit_cast<float>(bits);
}

inline float nextFloatDown(float f)
{
	if (std::isinf(f) && f < 0.f)
		return f;
	if (f == 0.f)
		return -0.f;
	auto bits = std::bit_cast<uint32_t>(f);
	if (f > 0)
		--bits;
	else
		++bits;
	return std::bit_cast<float>(bits);
}

inline bool isExact(const Vector3f& error)
{
	return error.x() == 0.f && error.y() == 0.f && error.z() == 0.f;
}

inline void applyTransformToPoint(const Transform& transform, Vector3f& point, Vector3f* error)
{
	if (error != nullptr) {
		// Note: Most codes from pbrt-v4
		const auto& m = transform.matrix();
		auto& x = point.x();
		auto& y = point.y();
		auto& z = point.z();
		if (isExact(*error)) {
			// Compute error for transformed exact _p_
			error->x() = floatingGamma<3>() * (std::abs(m(0, 0) * x) + std::abs(m(0, 1) * y) +
											   std::abs(m(0, 2) * z) + std::abs(m(0, 3)));
			error->y() = floatingGamma<3>() * (std::abs(m(1, 0) * x) + std::abs(m(1, 1) * y) +
											   std::abs(m(1, 2) * z) + std::abs(m(1, 3)));
			error->z() = floatingGamma<3>() * (std::abs(m(2, 0) * x) + std::abs(m(2, 1) * y) +
											   std::abs(m(2, 2) * z) + std::abs(m(2, 3)));
		}
		else {
			// Compute error for transformed approximate _p_
			const Vector3f pInError = *error;
			error->x() = (floatingGamma<3>() + 1) *
							 (std::abs(m(0, 0)) * pInError.x() + std::abs(m(0, 1)) * pInError.y() +
							  std::abs(m(0, 2)) * pInError.z()) +
						 floatingGamma<3>() * (std::abs(m(0, 0) * x) + std::abs(m(0, 1) * y) +
											   std::abs(m(0, 2) * z) + std::abs(m(0, 3)));
			error->y() = (floatingGamma<3>() + 1) *
							 (std::abs(m(1, 0)) * pInError.x() + std::abs(m(1, 1)) * pInError.y() +
							  std::abs(m(1, 2)) * pInError.z()) +
						 floatingGamma<3>() * (std::abs(m(1, 0) * x) + std::abs(m(1, 1) * y) +
											   std::abs(m(1, 2) * z) + std::abs(m(1, 3)));
			error->z() = (floatingGamma<3>() + 1) *
							 (std::abs(m(2, 0)) * pInError.x() + std::abs(m(2, 1)) * pInError.y() +
							  std::abs(m(2, 2)) * pInError.z()) +
						 floatingGamma<3>() * (std::abs(m(2, 0) * x) + std::abs(m(2, 1) * y) +
											   std::abs(m(2, 2) * z) + std::abs(m(2, 3)));
		}
	}
	point = transform * point;
}

inline void applyTransformToDirection(const Transform& transform,
									  Vector3f& dir,
									  Vector3f* error = nullptr)
{
	if (error != nullptr) {
		const auto& m = transform.matrix();
		auto &x = dir.x(), &y = dir.y(), &z = dir.z();
		if (isExact(*error)) {
			error->x() = floatingGamma<3>() *
						 (std::abs(m(0, 0) * x) + std::abs(m(0, 1) * y) + std::abs(m(0, 2) * z));
			error->y() = floatingGamma<3>() *
						 (std::abs(m(1, 0) * x) + std::abs(m(1, 1) * y) + std::abs(m(1, 2) * z));
			error->z() = floatingGamma<3>() *
						 (std::abs(m(2, 0) * x) + std::abs(m(2, 1) * y) + std::abs(m(2, 2) * z));
		}
		else {
			const Vector3f vInError = *error;
			error->x() = (floatingGamma<3>() + 1) *
							 (std::abs(m(0, 0)) * vInError.x() + std::abs(m(0, 1)) * vInError.y() +
							  std::abs(m(0, 2)) * vInError.z()) +
						 floatingGamma<3>() * (std::abs(m(0, 0) * x) + std::abs(m(0, 1) * y) +
											   std::abs(m(0, 2) * z));
			error->y() = (floatingGamma<3>() + 1) *
							 (std::abs(m(1, 0)) * vInError.x() + std::abs(m(1, 1)) * vInError.y() +
							  std::abs(m(1, 2)) * vInError.z()) +
						 floatingGamma<3>() * (std::abs(m(1, 0) * x) + std::abs(m(1, 1) * y) +
											   std::abs(m(1, 2) * z));
			error->z() = (floatingGamma<3>() + 1) *
							 (std::abs(m(2, 0)) * vInError.x() + std::abs(m(2, 1)) * vInError.y() +
							  std::abs(m(2, 2)) * vInError.z()) +
						 floatingGamma<3>() * (std::abs(m(2, 0) * x) + std::abs(m(2, 1) * y) +
											   std::abs(m(2, 2) * z));
		}
	}
	dir = (transform.linear() * dir).normalized();
}

inline bool isBlack(const ColorRGB& color, float eps = 1e-5f)
{
	return color.cwiseLess(eps).all();
}

/**
 * check if v lies in the hemisphere defined by n.
 * @param v the vector to be judged.
 * @param n the normal that determines the hemisphere.
 * @return true if v lies in the hemisphere n defines; false otherwise.
 */
inline bool sameHemisphere(const Vector3f& v, const Vector3f& n)
{
	return v.dot(n) > 0;
}
}  // namespace xd
#endif	// XD_RT_MATHUTIL_H
