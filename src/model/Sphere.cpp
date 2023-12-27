//
// Created by Frank on 2023/8/19.
//
#include "Sphere.h"
#include <cassert>
#include "AABB.h"
#include "FloatWithError.h"
#include "MathUtil.h"
#include "Triangle.h"
using namespace xd;

Sphere::Sphere(double radius) : radius(radius), radiusInv(1.f / radius) {}

bool Sphere::hit(const Ray& ray, HitRecord& rec) const
{
	const FloatWithError r{radius};
	const FloatWithError ox{ray.o.x()}, oy{ray.o.y()}, oz{ray.o.z()};
	const FloatWithError dx{ray.d.x()}, dy{ray.d.y()}, dz{ray.d.z()};
	const FloatWithError a{1.f};
	// const float b = 2.f * ray.d.dot(co);
	const FloatWithError b = 2.f * (dx * ox + dy * oy + dz * oz);
	// const float c = co.squaredNorm() - radius * radius;
	const FloatWithError c = sqr(ox) + sqr(oy) + sqr(oz) - r * r;

	FloatWithError t1, t2;
	FloatWithError resT{};
	bool hit = false;
#if 1
	// specialized method for solving sphere-ray intersection
	const FloatWithError vx{ox - b / (2.f * a) * dx}, vy{oy - b / (2.f * a) * dy},
		vz{oz - b / (2.f * a) * dz};
	const FloatWithError vLen = sqrt(sqr(vx) + sqr(vy) + sqr(vz));
	const FloatWithError determine = 4.f * a * (r + vLen) * (r - vLen);
	if (determine.low < 0)
		return false;
	const FloatWithError determineSqrt = sqrt(determine);
	FloatWithError q;
	if (b < 0) {
		q = -0.5f * (b - determineSqrt);
	}
	else
		q = -0.5f * (b + determineSqrt);
	t1 = c / q;
	t2 = q / a;
	if (t2 < t1)
		std::swap(t1, t2);
	constexpr float tMin = 1e-4f;
	if (t1 > tMin && t1 < rec.tHit) {
		hit = true;
		resT = t1;
	}
	else if (t2 > tMin && t2 < rec.tHit) {
		hit = true;
		resT = t2;
	}
#else
	// const auto count = solveQuadraticReal(a, b, c, t1, t2);
	const auto count = solveQuadraticRealWithError(a, b, c, t1, t2);

	if (count == 0)
		return false;
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
#endif
	if (hit) {
		rec.geom.frame = FrameCategory::MODEL;
		rec.tHit = resT;
#if 0
		// handle rounding errors in specialized approach
		const Vector3f n = (rec.p - center).normalized();
		rec.p = n * radius + center;
		rec.pError = floatingGamma<5>() * rec.p;

		Vector3f debug{0, 0, 0};
		ray.getTPoint(resT, debug);
#else
		// handle rouding errors in common approach using Ray::getTPoint
		Vector3f pError{0, 0, 0};
		rec.geom.p = ray.getTPoint(resT, pError);
		rec.pError = pError;
#endif

		std::tie(rec.geom.derivatives.dpdu, rec.geom.derivatives.dpdv, rec.geom.derivatives.n) =
			generateDifferentials(rec.geom.p);
		rec.geom.uv = generateUV(rec.geom.p);
	}
	return hit;
}
std::tuple<Vector3f, Vector3f, Vector3f> Sphere::generateDifferentials(const Vector3f& point) const
{
	const float cosPhi = point.x() * radiusInv;
	const float sinPhi = point.y() * radiusInv;
	const float cosTheta = point.z() * radiusInv;
	const float sinTheta = std::sqrtf(1 - cosTheta * cosTheta);
	const Vector3f dpdu = Vector3f{-point.y(), point.x(), 0} * 2.f * PI;
	const Vector3f dpdv = Vector3f{point.z() * cosPhi, point.z() * sinPhi, -radius * sinTheta} * PI;
	const Vector3f n = point.normalized();
	return {dpdu, dpdv, n};
}
Vector2f Sphere::generateUV(const Vector3f& point) const
{
	const auto [theta, phi] = getThetaPhi(point);
	return {phi / TWO_PI, theta / PI};
}
std::pair<float, float> Sphere::getThetaPhi(const Vector3f& point) const
{
	float phi = std::atan2(point.y(), point.x());
	if (phi < 0)
		phi = TWO_PI + phi;
	const float theta = std::acos(point.z() / radius);
	return {theta, phi};
}
float Sphere::getArea() const
{
	return 4 * PI * radius * radius;
}
AABB Sphere::getAABB() const
{
	const Vector3f offset{radius, radius, radius};
	return {-offset, offset};
}

std::shared_ptr<TriangleMesh> Sphere::triangulate() const
{
	constexpr float phiInterval = toRadians(10.f);	  // u
	constexpr float thetaInterval = toRadians(10.f);  // v
	constexpr float eps = 1e-4f;
	const bool uOverlap = std::fmod(TWO_PI, phiInterval) < eps;
	const int uCount = (int)(TWO_PI / phiInterval) + (uOverlap ? 0 : 1);
	assert(uCount >= 2);
	const bool vOverlap = std::fmod(PI, thetaInterval) < eps;
	const int vCount = (int)(PI / thetaInterval) + (vOverlap ? 0 : 1);
	assert(vCount >= 2);
	const int pointCnt = 2 + uCount * std::max(vCount - 2, 1);
	const int triangleCnt = 2 * uCount /*sphere cap*/ + 2 * std::max(vCount - 3, 0) /*middle*/;
	std::vector<float> positions;
	std::vector<float> uvs;
	std::vector<float> normals;
	std::vector<float> tangents;
	std::vector<uint32_t> indices;
	positions.reserve(3 * pointCnt);
	normals.reserve(3 * pointCnt);
	uvs.reserve(2 * pointCnt);
	indices.reserve(3 * triangleCnt);
	const auto setVertex = [&](const Vector3f& pos, const Vector3f& normal,
							   const Vector2f& uv) -> void {
		positions.emplace_back(pos.x());
		positions.emplace_back(pos.y());
		positions.emplace_back(pos.z());
		normals.emplace_back(normal.x());
		normals.emplace_back(normal.y());
		normals.emplace_back(normal.z());
		uvs.emplace_back(uv.x());
		uvs.emplace_back(uv.y());
	};
	// generate per-vertex data
	{
		const Vector3f sphereTopDir{0, 0, 1};
		setVertex(radius * Vector3f{0, 0, 1}, sphereTopDir, getSphereUV(sphereTopDir));
	}
	for (auto theta = thetaInterval; theta < PI; theta += thetaInterval) {
		for (auto phi = 0.f; phi < TWO_PI; phi += phiInterval) {
			const auto sphereDir = getSphereDirFromThetaPhi(phi, theta);
			setVertex(radius * sphereDir, sphereDir,
					  {std::clamp(phi / TWO_PI, 0.f, 1.f), std::clamp(theta / PI, 0.f, 1.f)});
		}
	}
	{
		const Vector3f sphereBottomDir{0, 0, 1};
		setVertex(-radius * Vector3f{0, 0, 1}, sphereBottomDir, getSphereUV(sphereBottomDir));
	}
	positions.shrink_to_fit();
	uvs.shrink_to_fit();
	normals.shrink_to_fit();
	// generate indices
	// generate sphere top
	constexpr uint32_t topIndex = 0ull;
	uint32_t beginIndex = 1;
	uint32_t endIndex = beginIndex + uCount - 1;  // [beginIndex, endIndex]
	uint32_t pIndex = beginIndex;
	for (; pIndex < endIndex; ++pIndex) {
		indices.emplace_back(topIndex);
		indices.emplace_back(pIndex);
		indices.emplace_back(pIndex + 1);
	}
	indices.emplace_back(topIndex);
	indices.emplace_back(pIndex);
	indices.emplace_back(beginIndex);
	// generate sphere middle
	while (endIndex + uCount < pointCnt) {
		for (pIndex = beginIndex; pIndex < endIndex; ++pIndex) {
			const auto nextIndex = pIndex + uCount;
			indices.emplace_back(pIndex);
			indices.emplace_back(nextIndex);
			indices.emplace_back(nextIndex + 1);

			indices.emplace_back(pIndex + 1);
			indices.emplace_back(pIndex);
			indices.emplace_back(nextIndex + 1);
		}
		beginIndex += uCount;
		endIndex += uCount;
	}

	const auto bottomIndex = pointCnt - 1;
	assert(bottomIndex == endIndex + 1);
	for (pIndex = beginIndex; pIndex < endIndex; ++pIndex) {
		indices.emplace_back(pIndex);
		indices.emplace_back(pIndex + 1);
		indices.emplace_back(bottomIndex);
	}
	indices.emplace_back(pIndex);
	indices.emplace_back(beginIndex);
	indices.emplace_back(bottomIndex);
	// generate sphere bottom
	return std::make_shared<TriangleMesh>(shared_from_this(), positions, uvs, normals, tangents,
										  indices);
}
