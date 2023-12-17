//
// Created by Frank on 2023/12/17.
//

#ifndef XD_RT_BVHNODE_H
#define XD_RT_BVHNODE_H
#include "AABB.h"
#include "HitAccel.h"
namespace xd {
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
	// TODO: we may need 2 ctors: one for primitives and one for triangles. The triangles can pass
	// span in
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
}  // namespace xd
#endif	// XD_RT_BVHNODE_H
