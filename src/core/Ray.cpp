//
// Created by Frank on 2023/8/16.
//

#include "Ray.h"
#include <ranges>
#include "FloatWithError.h"
using namespace xd;

Ray::Ray(const Vector3f& o, const Vector3f& d) : o(o), d(d) {}
Vector3f Ray::getTPoint(const FloatWithError& t, Vector3f& pError) const
{
	const auto deltaT = t.extent() / 2.f;
	pError.x() = floatingGamma<1>() * std::fabs(o.x()) +
				 deltaT * (1 + floatingGamma<2>()) * std::fabs(d.x()) +
				 floatingGamma<2>() * std::fabs(t * d.x());
	pError.y() = floatingGamma<1>() * std::fabs(o.y()) +
				 deltaT * (1 + floatingGamma<2>()) * std::fabs(d.y()) +
				 floatingGamma<2>() * std::fabs(t * d.y());
	pError.z() = floatingGamma<1>() * std::fabs(o.z()) +
				 deltaT * (1 + floatingGamma<2>()) * std::fabs(d.z()) +
				 floatingGamma<2>() * std::fabs(t * d.z());
	return getTPoint(t);
}