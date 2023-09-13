//
// Created by Frank on 2023/9/11.
//
#include <numeric>
#include "HitAccel.h"
#include "Model.h"
using namespace xd;

BVHNode::BVHNode(std::vector<const Model*>& p_models)
{
	for (const auto* model : p_models) {
		aabb = aabb.merge(model->getAABB());
	}
	constexpr uint32_t THRESHOLD = 4u;
	if (p_models.size() <= THRESHOLD) {
		models = p_models;
		return;
	}
	float totalArea = 0.f;
	for (const auto* model : p_models) {
		totalArea += model->getArea();
	}
	const Vector3f extent = aabb.getExtent();
	uint32_t maxIndex;
	extent.maxCoeff(&maxIndex);
	std::sort(p_models.begin(), p_models.end(), [&](const Model* lhs, const Model* rhs) {
		return lhs->getAABB().getCenter()(maxIndex) < rhs->getAABB().getCenter()(maxIndex);
	});
	std::vector<float> sumArea(p_models.size());
	sumArea[0] = p_models.front()->getArea();
	for (auto i = 1u; i < p_models.size(); ++i) {
		sumArea[i] = sumArea[i - 1] + p_models[i]->getArea();
	}
	auto pivot = std::partition_point(sumArea.begin(), sumArea.end(), [&](const float area) {
		return area < sumArea.back() / 2.f;
	});
	const uint32_t pivotIdx = std::distance(sumArea.begin(), pivot);
	std::vector<const Model*> leftModels{p_models.begin(), p_models.begin() + pivotIdx + 1};
	std::vector<const Model*> rightModels{p_models.begin() + pivotIdx + 1, p_models.end()};
	left = new BVHNode{leftModels};
	right = new BVHNode{rightModels};
}

bool BVHNode::isLeaf() const
{
	return left == nullptr && right == nullptr;
}

bool BVHNode::hit(const Ray& ray, HitRecord& rec) const
{
	auto prevTHit = rec.tHit;
	if (!aabb.hit(ray, rec))
		return false;
	rec.tHit = prevTHit;
	bool hit = false;
	if (isLeaf()) {
		for (const auto* model : models) {
			if (model->hit(ray, rec)) {
				hit = true;
			}
		}
		return hit;
	}
	else {
		const auto leftHit = left->hit(ray, rec);
		const auto rightHit = right->hit(ray, rec);
		hit = leftHit || rightHit;
		if (hit) {
			rec.debug <<= 1;
			if (rightHit) {
				rec.debug += 1;
			}
		}
		return hit;
	}
}

BVHNode::~BVHNode()
{
	delete left;
	delete right;
}
