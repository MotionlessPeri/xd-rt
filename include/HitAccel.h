//
// Created by Frank on 2023/9/11.
//

#ifndef XD_RT_HITACCEL_H
#define XD_RT_HITACCEL_H
#include "AABB.h"
#include "CoreTypes.h"
#include "Hitable.h"
namespace xd {
class HitAccel : public Hitable {};
class NoAccel : public HitAccel {
public:
	NoAccel(const std::vector<const Model*>& models);
	~NoAccel() = default;
	NoAccel(const NoAccel& other) = delete;
	NoAccel(NoAccel&& other) noexcept = delete;
	NoAccel& operator=(const NoAccel& other) = delete;
	NoAccel& operator=(NoAccel&& other) noexcept = delete;
	bool hit(const Ray& ray, HitRecord& rec) const override;

protected:
	std::vector<const Model*> models{};
};
class BVHNode : public HitAccel {
public:
	BVHNode() = default;
	BVHNode(const BVHNode& other) = delete;
	BVHNode(BVHNode&& other) noexcept = delete;
	BVHNode& operator=(const BVHNode& other) = delete;
	BVHNode& operator=(BVHNode&& other) noexcept = delete;
	BVHNode(std::vector<const Model*>& p_models);
	~BVHNode() override;
	bool hit(const Ray& ray, HitRecord& rec) const override;
	bool isLeaf() const;
	AABB getAABB() const { return aabb; }
	const std::vector<const Model*>& getModels() const { return models; }

protected:
	BVHNode* left = nullptr;
	BVHNode* right = nullptr;
	AABB aabb{};
	std::vector<const Model*> models{};
};
}  // namespace xd
#endif	// XD_RT_HITACCEL_H
