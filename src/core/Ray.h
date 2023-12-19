//
// Created by Frank on 2023/8/16.
//

#ifndef XD_RT_RAY_H
#define XD_RT_RAY_H

#include <ranges>
#include "MathTypes.h"
#include "MathUtil.h"
namespace xd {
inline Vector3f offsetRayOrigin(const Vector3f& origin,
								const Vector3f& oError,
								const Vector3f& n,
								const Vector3f& w)
{
	const auto d = n.cwiseAbs().dot(oError);
	Vector3f offset = d * n;
	if (w.dot(n) < 0) {	 // in the different hemisphere
		offset = -offset;
	}
	Vector3f res = origin + offset;
	for (auto i : std::views::iota(0u, 3u)) {
		if (offset[i] < 0)
			res[i] = nextFloatDown(res[i]);
		else if (offset[i] > 0)
			res[i] = nextFloatUp(res[i]);
	}
	return res;
}

class Ray {
public:
	Ray() = default;
	Ray(Vector3f o, Vector3f d);
	Vector3f getTPoint(float t) const { return o + t * d; }
	Vector3f getTPoint(const FloatWithError& t, Vector3f& pError) const;
	Vector3f o;
	Vector3f d;
};

inline void applyTransformToRay(const Transform& transform, Ray& ray)
{
	Vector3f oError{0, 0, 0};
	applyTransformToPoint(transform, ray.o, &oError);
	applyTransformToDirection(transform, ray.d);
	// shift ray.o to error bound's edge in direction ray.d to ensure o is in the right side
	const Vector3f oErrorHalfExtent = oError / 2.f;
	const auto dt = oErrorHalfExtent.dot(ray.d);
	ray.o += ray.d * dt;
}
}  // namespace xd

#endif	// XD_RT_RAY_H
