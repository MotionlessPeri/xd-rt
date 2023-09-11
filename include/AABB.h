//
// Created by Frank on 2023/9/10.
//

#ifndef XD_RT_AABB_H
#define XD_RT_AABB_H
#include "CoreTypes.h"
#include "Hitable.h"
namespace xd {
class AABB : public Hitable {
public:
	AABB(const Vector3f& minPoint, const Vector3f& maxPoint);
	AABB merge(const AABB& rhs) const;
	bool isInside(const Vector3f& point) const;
	bool isIntersected(const AABB& rhs) const;
	Vector3f getMinPoint() const { return minPoint; }
	Vector3f getMaxPoint() const { return maxPoint; }
	bool hit(const Ray& ray, HitRecord& rec) const override;
	struct HitResult {
		bool hit;
		float tMin;
		float tMax;
	};
	AABB::HitResult hitWithParams(const Ray& ray) const;

protected:
	Vector3f minPoint;
	Vector3f maxPoint;
};
}  // namespace xd
#endif	// XD_RT_AABB_H
