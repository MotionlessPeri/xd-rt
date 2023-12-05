//
// Created by Frank on 2023/9/11.
//

#ifndef XD_RT_HITACCEL_H
#define XD_RT_HITACCEL_H
#include "AABB.h"
#include "CoreTypes.h"
#include "Hitable.h"
#include "embree4/rtcore.h"
namespace xd {
/**
 * @brief Hit acceleration structure interface
 *
 * This class represents hit accelerate structure. It inherits HitAggregate, which means any
 * HitAccel must provide both closest hit query and any hit query.
 */
class HitAccel : public HitAggregate {};

/**
 * @brief Hit acceleration structure looping over primitives to find hit
 */
class NoAccel : public HitAccel {
public:
	explicit NoAccel(const std::vector<const Model*>& models);
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

/**
 * @brief Hit acceleration structure that using bounding volume hierarchy
 *
 * This implementation uses area estimate to split bvh.
 */
class BVHNode : public HitAccel {
	// TODO: add other bvh splitting methods
public:
	BVHNode() = delete;
	BVHNode(const BVHNode& other) = delete;
	BVHNode(BVHNode&& other) noexcept = delete;
	BVHNode& operator=(const BVHNode& other) = delete;
	BVHNode& operator=(BVHNode&& other) noexcept = delete;
	explicit BVHNode(std::vector<const Model*>& models);
	~BVHNode() override;
	bool hit(const Ray& ray, HitRecord& rec) const override;
	bool hitAnything(const Ray& ray, HitRecord& rec) const override;
	bool isLeaf() const;
	AABB getAABB() const { return aabb; }
	const std::vector<const Model*>& getLeafModels() const { return leafModels; }

protected:
	BVHNode* left = nullptr;
	BVHNode* right = nullptr;
	AABB aabb{};
	std::vector<const Model*> leafModels{};
};

/**
 * @brief Hit acceleration structure using Intel's Embree library
 */
class EmbreeAccel : public HitAccel {
public:
	EmbreeAccel(RTCDevice device, const std::vector<const Primitive*>& primitives);
	bool hit(const Ray& ray, HitRecord& rec) const override;
	bool hitAnything(const Ray& ray, HitRecord& rec) const override;

protected:
	RTCDevice device;
	RTCScene scene;
	std::unordered_map<unsigned int, const Primitive*> instanceIdToPrimitiveMap;
	std::unordered_map<unsigned int, RTCGeometry> instanceIdToGeomMap;
};
}  // namespace xd
#endif	// XD_RT_HITACCEL_H
