//
// Created by Frank on 2023/9/11.
//
#include "BVHNode.h"
#include <numeric>
#include "AABB.h"
#include "Model.h"
using namespace xd;

BVHNode::BVHNode(std::vector<const Model*>& models)
{
	for (const auto* model : models) {
		aabb.merge(model->getAABB());
	}
	constexpr uint32_t THRESHOLD = 4u;
	if (models.size() <= THRESHOLD) {
		leafModels = models;
		return;
	}
	float totalArea = 0.f;
	for (const auto* model : models) {
		totalArea += model->getArea();
	}
	const Vector3f extent = aabb.getExtent();
	uint32_t maxIndex;
	extent.maxCoeff(&maxIndex);
	std::sort(models.begin(), models.end(), [&](const Model* lhs, const Model* rhs) {
		return lhs->getAABB().getCenter()(maxIndex) < rhs->getAABB().getCenter()(maxIndex);
	});
	std::vector<float> sumArea(models.size());
	sumArea[0] = models.front()->getArea();
	for (auto i = 1u; i < models.size(); ++i) {
		sumArea[i] = sumArea[i - 1] + models[i]->getArea();
	}
	auto pivot = std::partition_point(sumArea.begin(), sumArea.end(), [&](const float area) {
		return area < sumArea.back() / 2.f;
	});
	const uint32_t pivotIdx = std::distance(sumArea.begin(), pivot);
	std::vector<const Model*> leftModels{models.begin(), models.begin() + pivotIdx + 1};
	std::vector<const Model*> rightModels{models.begin() + pivotIdx + 1, models.end()};
	left = new BVHNode{leftModels};
	right = new BVHNode{rightModels};
}

bool BVHNode::isLeaf() const
{
	return left == nullptr && right == nullptr;
}

bool BVHNode::hit(const Ray& ray, HitRecord& rec) const
{
	bool hit = false;
	if (isLeaf()) {
		for (const auto* model : leafModels) {
			if (model->hit(ray, rec)) {
				hit = true;
			}
		}
		return hit;
	}
	auto aabbTestRec = HitRecord{rec.tHit};
	if (!aabb.hit(ray, aabbTestRec))
		return false;
	const auto leftHit = left->hit(ray, rec);
	const auto rightHit = right->hit(ray, rec);
	hit = leftHit || rightHit;
	return hit;
}
bool BVHNode::hitAnything(const Ray& ray, HitRecord& rec) const
{
	if (isLeaf()) {
		for (const auto* model : leafModels) {
			if (model->hit(ray, rec)) {
				return true;
			}
		}
		return false;
	}
	auto prevTHit = rec.tHit;
	if (!aabb.hit(ray, rec))
		return false;
	rec.tHit = prevTHit;
	return left->hit(ray, rec) || right->hit(ray, rec);
}
BVHNode::~BVHNode()
{
	delete left;
	delete right;
}
