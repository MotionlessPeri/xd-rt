//
// Created by Frank on 2023/9/19.
//
#include "Box.h"
#include <cassert>
#include "AABB.h"
#include "MathUtil.h"
#include "Triangle.h"
using namespace xd;
Box::Box(Vector3f minPoint, Vector3f maxPoint)
	: minPoint(std::move(minPoint)), maxPoint(std::move(maxPoint))
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
		rec.geom.p = ray.getTPoint(tHit.t);
		rec.geom.derivatives.n = tHit.n;
		// TODO: assign uv
		rec.geom.derivatives.dpdu = {0, 0, 0};
		rec.geom.derivatives.dpdv = {0, 0, 0};
		rec.geom.derivatives.dpdu((tHit.i + 1) % 3) = 1;
		rec.geom.derivatives.dpdv((tHit.i + 2) % 3) = 1;
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

std::shared_ptr<TriangleMesh> Box::triangulate() const
{
	std::vector<float> positions;
	std::vector<float> uvs;
	std::vector<float> normals;
	std::vector<float> tangents;
	std::vector<uint32_t> indices;
	assert(false);	// TODO: implement this
	return nullptr;
}
