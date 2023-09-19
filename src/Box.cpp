//
// Created by Frank on 2023/9/19.
//
#include "AABB.h"
#include "MathUtil.h"
#include "Model.h"
using namespace xd;
Box::Box(const Vector3f& minPoint, const Vector3f& maxPoint)
	: minPoint(minPoint), maxPoint(maxPoint)
{
}
bool Box::hit(const Ray& ray, HitRecord& rec) const
{
	struct PlaneHit {
		float t;
		Vector3f n;
		uint32_t i;
		bool operator<(const PlaneHit& rhs) const { return t < rhs.t; }
		bool operator>(const PlaneHit& rhs) const { return rhs < *this; }
		bool operator<=(const PlaneHit& rhs) const { return !(rhs < *this); }
		bool operator>=(const PlaneHit& rhs) const { return !(*this < rhs); }
	};
	PlaneHit tMin{-FLT_MAX};
	PlaneHit tMax{FLT_MAX};
	for (uint32_t i = 0; i < 3; ++i) {
		const float o = ray.o(i);
		const float d = ray.d(i);
		if (d == 0) {
			continue;
		}
		Vector3f localNormal{0, 0, 0};
		localNormal(i) = 1;
		PlaneHit localMin{(minPoint(i) - o) / d, -localNormal, i};
		PlaneHit localMax{(maxPoint(i) - o) / d, localNormal, i};
		if (localMin > localMax)
			std::swap(localMin, localMax);
		tMin = std::max(localMin, tMin);
		tMax = std::min(localMax, tMax);
	}
	bool hit = false;
	PlaneHit tHit;
	if (tMin < tMax) {
		if (tMin.t > 0 && tMin.t < rec.tHit) {
			hit = true;
			tHit = tMin;
		}
		else if (tMax.t > 0 && tMax.t < rec.tHit) {
			hit = true;
			tHit = tMax;
		}
	}
	if (hit) {
		rec.tHit = tHit.t;
		rec.n = tHit.n;
		rec.dpdu = {0, 0, 0};
		rec.dpdv = {0, 0, 0};
		rec.dpdu((tHit.i + 1) % 3) = 1;
		rec.dpdv((tHit.i + 2) % 3) = 1;
	}
	return hit;
}
float Box::getArea() const
{
	const auto extent = getExtent();
	return 2.f * (extent.x() * extent.y() + extent.y() * extent.z() + extent.z() + extent.x());
}
AABB Box::getAABB() const
{
	return {minPoint, maxPoint};
}
