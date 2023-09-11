//
// Created by Frank on 2023/8/16.
//

#ifndef XD_RT_RAY_H
#define XD_RT_RAY_H

#include "MathType.h"
namespace xd {
class Ray {
public:
	Ray(const Vector3f& o, const Vector3f& d);
	Vector3f getTPoint(float t) const { return o + t * d; }
	Vector3f o;
	Vector3f d;
};

}  // namespace xd

#endif	// XD_RT_RAY_H
