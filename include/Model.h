//
// Created by Frank on 2023/8/19.
//

#ifndef XD_RT_MODEL_H
#define XD_RT_MODEL_H

#include <tuple>
#include "HitRecord.h"
#include "Hitable.h"
#include "Ray.h"
namespace xd {

class Model : public Hitable, public std::enable_shared_from_this<Model> {
public:
	Model() = default;
	virtual ~Model() = default;
	virtual float getArea() const = 0;
	virtual AABB getAABB() const = 0;

protected:
};

class Sphere : public Model {
public:
	Sphere(const Vector3f& center, double radius);
	bool hit(const Ray& ray, HitRecord& rec) const override;
	Vector2f generateUV(const Vector3f& point) const;
	std::tuple<Vector3f, Vector3f, Vector3f> generateDifferentials(const Vector3f& point) const;
	float getArea() const override;
	AABB getAABB() const override;

protected:
	/**
	 * given a point on the surface, return [theta, phi] where theta is the angle with z axis and
	 * phi is the angle with x axis
	 * @param point the point on the surface
	 * @return [theta, phi]
	 */
	std::pair<float, float> getThetaPhi(const Vector3f& point) const;
	Vector3f center;
	float radius;
};

}  // namespace xd
#endif	// XD_RT_MODEL_H
