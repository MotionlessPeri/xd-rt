//
// Created by Frank on 2023/9/4.
//
#include "Triangle.h"
#include "MathUtil.h"
#include "hitAccel/BVHNode.h"
#include "hitAccel/NoAccel.h"
using namespace xd;

TriangleMesh::TriangleMesh(const std::shared_ptr<const Model>& owner,
						   const std::vector<float>& positions,
						   const std::vector<float>& uvs,
						   const std::vector<float>& normals,
						   const std::vector<float>& tangents,
						   const std::vector<uint32_t>& indices,
						   HitAccelMethod method)
	: TriangleMesh(positions, uvs, normals, tangents, indices, method)
{
	this->owner = owner;
}

TriangleMesh::TriangleMesh(const std::vector<float>& positions,
						   const std::vector<float>& uvs,
						   const std::vector<float>& normals,
						   const std::vector<float>& tangents,
						   const std::vector<uint32_t>& indices,
						   HitAccelMethod method)
	: rawPositions(positions),
	  positionsAccessor(rawPositions.data(), 3, rawPositions.size()),
	  rawUVs(uvs),
	  uvAccessor(rawUVs.data(), 2, rawUVs.size()),
	  rawNormals(normals),
	  normalAccessor(rawNormals.data(), 3, rawNormals.size()),
	  rawTangents(tangents),
	  tangentAccessor(rawTangents.data(), 3, rawTangents.size()),
	  indices(indices)
{
	init(method);
}

TriangleMesh::TriangleMesh(std::vector<float>&& positions,
						   std::vector<float>&& uvs,
						   std::vector<float>&& normals,
						   std::vector<float>&& tangents,
						   std::vector<uint32_t>&& indices,
						   HitAccelMethod method)
	: rawPositions(positions),
	  positionsAccessor(rawPositions.data(), 3, rawPositions.size()),
	  rawUVs(uvs),
	  uvAccessor(rawUVs.data(), 2, rawUVs.size()),
	  rawNormals(normals),
	  normalAccessor(rawNormals.data(), 3, rawNormals.size()),
	  rawTangents(tangents),
	  tangentAccessor(rawTangents.data(), 3, rawTangents.size()),
	  indices(indices)
{
	init(method);
}

bool TriangleMesh::hasUV() const
{
	return !rawUVs.empty();
}
bool TriangleMesh::hasNormal() const
{
	return !rawNormals.empty();
}
bool TriangleMesh::hasTangent() const
{
	return !rawTangents.empty();
}
bool TriangleMesh::hit(const Ray& ray, HitRecord& rec) const
{
	return hitAccel->hit(ray, rec);
}
const std::vector<float>& TriangleMesh::getPositions() const
{
	return rawPositions;
}
const std::vector<float>& TriangleMesh::getUVs() const
{
	return rawUVs;
}
const std::vector<float>& TriangleMesh::getNormals() const
{
	return rawNormals;
}
const std::vector<float>& TriangleMesh::getTangents() const
{
	return rawTangents;
}
float TriangleMesh::getArea() const
{
	float res = 0.f;
	for (const auto& triangle : triangles) {
		res += triangle.getArea();
	}
	return res;
}
AABB TriangleMesh::getAABB() const
{
	return aabb;
}

std::shared_ptr<const Model> TriangleMesh::triangulateFrom() const
{
	return owner.lock();
}
bool TriangleMesh::isTriangulatedFromOthers() const
{
	return owner.owner_before(std::weak_ptr<Model>{}) || std::weak_ptr<Model>{}.owner_before(owner);
}

void TriangleMesh::init(HitAccelMethod method)
{
	initTriangles();
	initAccel(method);
}
void TriangleMesh::initTriangles()
{
	const auto indiceCount = indices.size();
	triangles.reserve(indiceCount / 3);
	for (auto i = 0u; i < indiceCount / 3; ++i) {
		// NOTE: we may need a runtime determined fixed-capacity container for triangles.
		// Thus, we can delete both copy and move ctors of Triangle
		// And require users to get reference
		triangles.emplace_back(this, i);
		aabb.merge(triangles.back().getAABB());
	}
}
void TriangleMesh::initAccel(HitAccelMethod method)
{
	std::vector<const Model*> trianglePtrs(triangles.size());
	std::transform(triangles.begin(), triangles.end(), trianglePtrs.begin(),
				   [](const Triangle& tri) { return &tri; });
	switch (method) {
		case HitAccelMethod::NO_ACCEL:
			hitAccel = std::make_unique<NoAccel>(trianglePtrs);
			break;
		case HitAccelMethod::BVH:
			hitAccel = std::make_unique<BVHNode>(trianglePtrs);
			break;
	}
}
const std::vector<uint32_t>& TriangleMesh::getIndices() const
{
	return indices;
}

Triangle::Triangle(const TriangleMesh* mesh, uint32_t index) : mesh(mesh), index(index)
{
	calAccParams();
}

Vector3f Triangle::getBarycentricCoordUnchecked(const Vector3f& P) const
{
	// Note: code from <Real-Time Collision Detection>
	const Vector3f& v0 = AB;
	const Vector3f& v1 = AC;
	const Vector3f& v2 = P - getPositionsUnchecked()[0];
	const float d00 = ABSquared;
	const float d01 = ABDotAC;
	const float d11 = ACSquared;
	const float d20 = v2.dot(v0);
	const float d21 = v2.dot(v1);
	const float denom = d00 * d11 - d01 * d01;
	const float v = (d11 * d20 - d01 * d21) / denom;
	const float w = (d00 * d21 - d01 * d20) / denom;
	const float u = 1.f - v - w;
	return {u, v, w};
}

void Triangle::calAccParams()
{
	const auto positions = getPositionsUnchecked();
	AB = positions[1] - positions[0];
	AC = positions[2] - positions[0];
	ABSquared = AB.squaredNorm();
	ACSquared = AC.squaredNorm();
	ABDotAC = AB.dot(AC);
	N = AB.cross(AC);
	area = N.norm() / 2.f;
	for (const auto& pos : positions) {
		aabb.addPoint(pos);
	}

	Matrix2f L;
	Eigen::MatrixXf R(3, 2);
	if (mesh->hasUV()) {
		const auto uvs = getUVsUnchecked();
		L.row(0) = (uvs[1] - uvs[0]).transpose();
		L.row(1) = (uvs[2] - uvs[0]).transpose();
		R.col(0) = AB;
		R.col(1) = AC;
		const Eigen::MatrixXf partials = R * L.inverse();
		dpdu = partials.col(0);
		dpdv = partials.col(1);
	}
	else {
		dpdu = AB;
		dpdv = N.cross(AB);
	}
}
bool Triangle::hit(const Ray& ray, HitRecord& rec) const
{
	constexpr float eps = 1e-4;
	if (fabs(N.dot(ray.d)) < eps)
		return false;
	const auto positions = getPositionsUnchecked();
	const Vector3f op = positions[0] - ray.o;
	const float t = op.dot(N) / ray.d.dot(N);

	const Vector3f tPoint = ray.getTPoint(t);
	const Vector3f baryCoord = getBarycentricCoordUnchecked(tPoint);
	if (baryCoord.cwiseGreaterOrEqual(0.f).all()) {
		if (t >= rec.tHit)
			return false;
		rec.tHit = t;
		rec.p = ray.getTPoint(t);
		if (mesh->hasUV()) {
			const auto uvs = getUVsUnchecked();
			rec.uv = interpolateWithBaryCoords(uvs, baryCoord);
		}
		else {
			rec.uv = {0, 0};
		}

		if (mesh->hasNormal()) {
			const auto normals = getNormalsUnchecked();
			rec.n = interpolateWithBaryCoords(normals, baryCoord);
		}
		else {
			rec.n = N.normalized();
		}
		rec.dpdu = dpdu;
		rec.dpdv = dpdv;
		return true;
	}
	else
		return false;
}

std::array<Vector3f, 3> Triangle::getPositionsUnchecked() const
{
	return getTriangleCoeffs<float, 3>(mesh->positionsAccessor);
}
std::array<Vector2f, 3> Triangle::getUVsUnchecked() const
{
	return getTriangleCoeffs<float, 2>(mesh->uvAccessor);
}
std::array<Vector3f, 3> Triangle::getNormalsUnchecked() const
{
	return getTriangleCoeffs<float, 3>(mesh->normalAccessor);
}
std::array<Vector3f, 3> Triangle::getTangentsUnchecked() const
{
	return getTriangleCoeffs<float, 3>(mesh->tangentAccessor);
}
float Triangle::getArea() const
{
	return area;
}
