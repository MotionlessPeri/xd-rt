//
// Created by Frank on 2023/9/10.
//

#ifndef XD_RT_HITABLE_H
#define XD_RT_HITABLE_H
#include "HitRecord.h"
#include "Ray.h"
namespace xd {
class Hitable {
public:
	virtual ~Hitable() = default;
	virtual bool hit(const Ray& ray) const
	{
		HitRecord dummy;
		return hit(ray, dummy);
	}
	virtual bool hit(const Ray& ray, HitRecord& rec) const = 0;
};
}  // namespace xd
#endif	// XD_RT_HITABLE_H
