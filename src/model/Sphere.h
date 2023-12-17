//
// Created by Frank on 2023/12/17.
//

#ifndef XD_RT_SPHERE_H
#define XD_RT_SPHERE_H
#include <tuple>
#include "Model.h"
namespace xd {
class Sphere : public Model {
public:
	Sphere(double radius);
	bool hit(const Ray& ray, HitRecord& rec) const override;
	Vector2f generateUV(const Vector3f& point) const;
	std::tuple<Vector3f, Vector3f, Vector3f> generateDifferentials(const Vector3f& point) const;
	float getArea() const override;
	AABB getAABB() const override;
	float radius;
	float radiusInv;

protected:
	std::shared_ptr<TriangleMesh> triangulate() const override;
	/**
	 * given a point on the surface, return [theta, phi] where theta is the angle with z axis and
	 * phi is the angle with x axis
	 * @param point the point on the surface
	 * @return [theta, phi]
	 */
	std::pair<float, float> getThetaPhi(const Vector3f& point) const;
};
}  // namespace xd
#endif	// XD_RT_SPHERE_H
