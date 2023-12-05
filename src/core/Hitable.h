//
// Created by Frank on 2023/9/10.
//

#ifndef XD_RT_HITABLE_H
#define XD_RT_HITABLE_H
#include "HitRecord.h"
#include "Ray.h"
namespace xd {
/**
 * @brief Interface that can be hit
 *
 * Everything can be hit should inherit this interface. The hit method provides a primary hit query.
 */
class Hitable {
public:
	virtual ~Hitable() = default;
	virtual bool hit(const Ray& ray, HitRecord& rec) const = 0;
};

/**
 * @brief Interface that provides any-hit query
 *
 * Everything that multiple hit can happens should inherit this interface. The hitAnything method
 * provides an any-hit query.
 */
class HitAggregate : public Hitable {
public:
	virtual bool hitAnything(const Ray& ray, HitRecord& rec) const = 0;
};
}  // namespace xd
#endif	// XD_RT_HITABLE_H
