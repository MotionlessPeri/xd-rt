//
// Created by Frank on 2023/9/10.
//
#include "AABB.h"
using namespace xd;
AABB::AABB(const Vector3f& minPoint, const Vector3f& maxPoint)
	: minPoint(minPoint), maxPoint(maxPoint)
{
}

AABB AABB::merge(const AABB& rhs) const
{
	return AABB(minPoint.cwiseMin(rhs.getMinPoint()), maxPoint.cwiseMin(rhs.getMaxPoint()));
}
bool AABB::isInside(const Vector3f& point) const
{
	return minPoint.cwiseLessOrEqual(point).all() && maxPoint.cwiseGreaterOrEqual(point).all();
}
bool AABB::isIntersected(const AABB& rhs) const
{
	return isInside(rhs.minPoint) || isInside(rhs.maxPoint) || rhs.isInside(minPoint) ||
		   rhs.isInside(maxPoint);
}
bool AABB::hit(const Ray& ray, HitRecord& rec) const
{
	auto hitRes = hitWithParams(ray);
	if (hitRes.hit) {
		rec.tHit = hitRes.tMin > 0 ? hitRes.tMin : hitRes.tMax;
	}
	return hitRes.hit;
}
AABB::HitResult AABB::hitWithParams(const Ray& ray) const
{
	float tMin = -FLT_MAX;
	float tMax = FLT_MAX;
	bool hit = false;
	for (int i = 0; i < 3; ++i) {
		const float o = ray.o(i);
		const float d = ray.d(i);
		if (d == 0) {
			continue;
		}
		else
			hit = true;
		float localMin = (minPoint(i) - o) / d;
		float localMax = (maxPoint(i) - o) / d;
		if (localMin > localMax)
			std::swap(localMin, localMax);
		tMin = std::max(localMin, tMin);
		tMax = std::min(localMax, tMax);
	}
	hit = hit && (tMin < tMax && tMax > 0);
	return {hit, tMin, tMax};
}
