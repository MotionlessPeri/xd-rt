//
// Created by Frank on 2023/8/19.
//
#include "AABB.h"
#include "MathUtil.h"
#include "Model.h"
using namespace xd;
Sphere::Sphere(const Vector3f& center, double radius) : center(center), radius(radius) {}
bool Sphere::hit(const Ray& ray, HitRecord& rec) const
{
	const Vector3f co = ray.o - center;
	const double a = 1;
	const double b = 2 * ray.d.dot(co);
	const double c = co.squaredNorm() - radius * radius;
	float t1, t2;
	const auto count = solveQuadraticReal(a, b, c, t1, t2);
	float resT = 0.f;
	bool hit = false;
	if (count == 0)
		return false;
	else if (count == 1) {
		if (t1 > 0 && t1 < rec.tHit) {
			hit = true;
			resT = t1;
		}
	}
	else {
		if (t1 > 0 && t1 < rec.tHit) {
			hit = true;
			resT = t1;
		}
		else if (t2 > 0 && t2 < rec.tHit) {
			hit = true;
			resT = t2;
		}
	}
	if (hit) {
		rec.tHit = resT;
		rec.tPoint = ray.getTPoint(resT);
		const Vector3f hitPoint = ray.getTPoint(rec.tHit);
		std::tie(rec.dpdu, rec.dpdv, rec.n) = generateDifferentials(hitPoint);
		rec.uv = generateUV(hitPoint);
	}
	return hit;
}
std::tuple<Vector3f, Vector3f, Vector3f> Sphere::generateDifferentials(const Vector3f& point) const
{
	const Vector3f cp = point - center;
	const auto [theta, phi] = getThetaPhi(point);
	const float cosPhi = std::cosf(phi);
	const float sinPhi = std::sinf(phi);
	const float sinTheta = std::sinf(theta);
	const Vector3f dpdu = Vector3f{-cp.y(), cp.x(), 0} * 2.f * PI;
	const Vector3f dpdv = Vector3f{cp.z() * cosPhi, cp.z() * sinPhi, -radius * sinTheta} * PI;
	const Vector3f n = cp.normalized();
	return {dpdu, dpdv, n};
}
Vector2f Sphere::generateUV(const Vector3f& point) const
{
	const auto [theta, phi] = getThetaPhi(point);
	return {theta / PI, phi / TWO_PI};
}
std::pair<float, float> Sphere::getThetaPhi(const Vector3f& point) const
{
	const Vector3f cp = point - center;
	float phi = std::atan2(cp.y(), cp.x());
	if (phi < 0)
		phi = TWO_PI + phi;
	const float theta = std::acos(cp.z() / radius);
	return {theta, phi};
}
float Sphere::getArea() const
{
	return 4 * PI * radius * radius;
}
AABB Sphere::getAABB() const
{
	const Vector3f offset{radius, radius, radius};
	return {center - offset, center + offset};
}