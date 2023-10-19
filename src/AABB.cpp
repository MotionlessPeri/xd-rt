//
// Created by Frank on 2023/9/10.
//
#include "AABB.h"
using namespace xd;
AABB::AABB() : minPoint(FLT_MAX, FLT_MAX, FLT_MAX), maxPoint(-FLT_MAX, -FLT_MAX, -FLT_MAX) {}

AABB::AABB(const Vector3f& minPoint, const Vector3f& maxPoint)
	: minPoint(minPoint), maxPoint(maxPoint)
{
}

void AABB::merge(const AABB& rhs)
{
	minPoint = minPoint.cwiseMin(rhs.getMinPoint());
	maxPoint = maxPoint.cwiseMax(rhs.getMaxPoint());
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
void AABB::addPoint(const Vector3f& point)
{
	if (isInside(point))
		return;
	minPoint = minPoint.cwiseMin(point);
	maxPoint = maxPoint.cwiseMax(point);
}
bool AABB::isValid() const
{
	return minPoint.cwiseLessOrEqual(maxPoint).all();
}

bool AABB::hit(const Ray& ray, HitRecord& rec) const
{
	auto hitRes = hitWithParams(ray);
	const auto tHit = hitRes.tMin > 0 ? hitRes.tMin : hitRes.tMax;
	bool hit = false;
	if (hitRes.hit) {
		if (tHit < rec.tHit) {
			hit = true;
			rec.tHit = tHit;
			rec.tPoint = ray.getTPoint(tHit);
		}
	}
	return hit;
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
bool AABB::operator==(const AABB& rhs) const
{
	return minPoint.isApprox(rhs.minPoint) && maxPoint.isApprox(rhs.maxPoint);
}
bool AABB::operator!=(const AABB& rhs) const
{
	return !(rhs == *this);
}
