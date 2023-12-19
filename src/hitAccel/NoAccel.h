//
// Created by Frank on 2023/12/17.
//

#ifndef XD_RT_NOACCEL_H
#define XD_RT_NOACCEL_H
#include "HitAccel.h"
namespace xd {
/**
 * @brief Hit acceleration structure looping over primitives to find hit
 */
class NoAccel : public HitAccel {
public:
	explicit NoAccel(std::vector<const Model*> models);
	~NoAccel() = default;
	NoAccel(const NoAccel& other) = delete;
	NoAccel(NoAccel&& other) noexcept = delete;
	NoAccel& operator=(const NoAccel& other) = delete;
	NoAccel& operator=(NoAccel&& other) noexcept = delete;
	bool hit(const Ray& ray, HitRecord& rec) const override;
	bool hitAnything(const Ray& ray, HitRecord& rec) const override;

protected:
	std::vector<const Model*> models;
};

}  // namespace xd
#endif	// XD_RT_NOACCEL_H
