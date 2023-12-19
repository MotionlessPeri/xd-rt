//
// Created by Frank on 2023/12/17.
//

#ifndef XD_RT_BOX_H
#define XD_RT_BOX_H

#include "Model.h"
#include "ModelTypes.h"
namespace xd {
// Similar as AABB but is a model
class Box : public Model {
public:
	Box(Vector3f minPoint, Vector3f maxPoint);
	bool hit(const Ray& ray, HitRecord& rec) const override;
	float getArea() const override;
	AABB getAABB() const override;
	Vector3f getExtent() const { return maxPoint - minPoint; }

protected:
	std::shared_ptr<TriangleMesh> triangulate() const override;
	Vector3f minPoint, maxPoint;
};
}  // namespace xd
#endif	// XD_RT_BOX_H
