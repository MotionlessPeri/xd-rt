//
// Created by Frank on 2023/9/4.
//
#include "Plane.h"
using namespace xd;
Plane::Plane(const Vector3f& point, const Vector3f& normal) : point(point), normal(normal) {}
bool Plane::isPointInPlane(const Vector3f& P) const
{
	const Vector3f Pp = P - point;
	const float eps = 1e-4;
	return fabs(Pp.dot(normal)) < eps;
}
