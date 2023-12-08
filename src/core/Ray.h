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
	// Note: don't know what kind of scenario does not need oError and dError yet. If most scenario
	// does not need this two and vast Ray objects are spawned under these scenarios, we may need to
	// move these two elsewhere.
	Vector3f oError;
	Vector3f d;
	Vector3f dError;
};

}  // namespace xd

#endif	// XD_RT_RAY_H
