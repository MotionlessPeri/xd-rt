//
// Created by Frank on 2023/8/19.
//
#include "AABB.h"
#include "MathUtil.h"
#include "Model.h"
#include "Triangle.h"
using namespace xd;
std::shared_ptr<TriangleMesh> Model::getTriangulatedMesh()
{
	if (!triangulatedMesh)
		triangulatedMesh = triangulate();
	return triangulatedMesh;
}

std::shared_ptr<TriangleMesh> Model::triangulate() const
{
	return nullptr;
}
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

std::shared_ptr<TriangleMesh> Sphere::triangulate() const
{
	constexpr float phiInterval = toRadians(10.f); // u 
	constexpr float thetaInterval = toRadians(10.f);  // v
	constexpr float eps = 1e-4f;
	const bool uOverlap = std::fmod(TWO_PI, phiInterval) < eps;
	const int uCount = (int)(TWO_PI / phiInterval) + (uOverlap ? 0 : 1);
	assert(uCount >= 2);
	const bool vOverlap = std::fmod(PI, thetaInterval) < eps;
	const int vCount = (int)(PI / thetaInterval) + (vOverlap ? 0 : 1);
	assert(vCount >= 2);
	const int pointCnt = 2 + uCount * std::max(vCount - 2, 1);
	const int triangleCnt =
		2 * uCount /*sphere cap*/ + 2 * std::max(vCount - 3, 0) /*middle*/;
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
		setVertex(center + radius * Vector3f{0, 0, 1}, sphereTopDir, getSphereUV(sphereTopDir));
	}
	for (auto theta = thetaInterval; theta < PI; theta += thetaInterval) {
		for (auto phi = 0.f; phi < TWO_PI; phi += phiInterval) {
			const auto sphereDir = getSphereDirFromThetaPhi(phi, theta);
			setVertex(
				center + radius * sphereDir, 
				sphereDir,
					  {
					  	std::clamp(phi / TWO_PI, 0.f, 1.f),
					  	std::clamp(theta / PI, 0.f, 1.f)
					  });
		}
	}
	{
		const Vector3f sphereBottomDir{0, 0, 1};
		setVertex(center - radius * Vector3f{0, 0, 1}, sphereBottomDir,
				  getSphereUV(sphereBottomDir));
	}
	positions.shrink_to_fit();
	uvs.shrink_to_fit();
	normals.shrink_to_fit();
	// generate indices
	// generate sphere top
	constexpr uint32_t topIndex = 0ull;
	uint32_t beginIndex = 1;
	uint32_t endIndex = beginIndex + uCount - 1; // [beginIndex, endIndex]
	uint32_t pIndex = beginIndex;
	for (;pIndex < endIndex; ++pIndex) {
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
	return std::make_shared<TriangleMesh>(shared_from_this(), positions, uvs, normals,
													tangents, indices);
}
