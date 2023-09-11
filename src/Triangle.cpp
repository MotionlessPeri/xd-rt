//
// Created by Frank on 2023/9/4.
//
#include "Triangle.h"
#include "MathUtil.h"
using namespace xd;

TriangleMesh::TriangleMesh(const std::vector<Vector3f>& positions,
						   const std::vector<Vector2f>& uvs,
						   const std::vector<Vector3f>& normals,
						   const std::vector<Vector3f>& tangents,
						   const std::vector<Vector3f>& biTangents,
						   const std::vector<uint32_t>& indices)
	: positions(positions), uvs(uvs), normals(normals), tangents(tangents), biTangents(biTangents)
{
	const auto indiceCount = indices.size();
	for (auto i = 0u; i < indiceCount; i += 3) {
		// NOTE: we may need a runtime determined fixed-capacity container for triangles.
		// Thus, we can delete both copy and move ctors of Triangle
		// And require users to get reference
		triangles.emplace_back(this, indices[i], indices[i + 1], indices[i + 2]);
	}
}

TriangleMesh::TriangleMesh(std::vector<Vector3f>&& positions,
						   std::vector<Vector2f>&& uvs,
						   std::vector<Vector3f>&& normals,
						   std::vector<Vector3f>&& tangents,
						   std::vector<Vector3f>&& biTangents,
						   std::vector<uint32_t>&& indices)
	: positions(positions), uvs(uvs), normals(normals), tangents(tangents), biTangents(biTangents)
{
	const auto indiceCount = indices.size();
	for (auto i = 0u; i < indiceCount; i += 3) {
		// NOTE: we may need a runtime determined fixed-capacity container for triangles.
		// Thus, we can delete both copy and move ctors of Triangle
		// And require users to get reference
		triangles.emplace_back(this, indices[i], indices[i + 1], indices[i + 2]);
	}
}

bool TriangleMesh::hasUV() const
{
	return !uvs.empty();
}
bool TriangleMesh::hasNormal() const
{
	return !normals.empty();
}
bool TriangleMesh::hasTangent() const
{
	return !tangents.empty();
}
bool TriangleMesh::hasBiTangent() const
{
	return !biTangents.empty();
}
bool TriangleMesh::hit(const Ray& ray, HitRecord& rec) const
{
	// naive method here
	// we may need a TriangleMeshHitSolver class for robustness
	bool isHit = false;
	uint32_t index = 0;
	for (const auto& triangle : triangles) {
		if (triangle.hit(ray, rec)) {
			isHit = true;
			rec.debug = index;
		}
		++index;
	}
	return isHit;
}
const std::vector<Vector3f>& TriangleMesh::getPositions() const
{
	return positions;
}
const std::vector<Vector2f>& TriangleMesh::getUvs() const
{
	return uvs;
}
const std::vector<Vector3f>& TriangleMesh::getNormals() const
{
	return normals;
}
const std::vector<Vector3f>& TriangleMesh::getTangents() const
{
	return tangents;
}
const std::vector<Vector3f>& TriangleMesh::getBiTangents() const
{
	return biTangents;
}
float TriangleMesh::getArea() const
{
	float res = 0.f;
	for (const auto& triangle : triangles) {
		res += triangle.getArea();
	}
	return res;
}

Triangle::Triangle(const TriangleMesh* mesh, uint32_t i0, uint32_t i1, uint32_t i2)
	: mesh(mesh), indices({i0, i1, i2})
{
	calAccParams();
}

Triangle::Triangle(const TriangleMesh* mesh, const std::array<uint32_t, 3>& indices)
	: mesh(mesh), indices(indices)
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

		if (mesh->hasTangent()) {
			const auto tangents = getTangentsUnchecked();
			rec.dpdu = interpolateWithBaryCoords(tangents, baryCoord);
		}
		else {
			rec.dpdu = dpdu;
		}

		if (mesh->hasBiTangent()) {
			const auto biTangents = getBiTangentsUnchecked();
			rec.dpdv = interpolateWithBaryCoords(biTangents, baryCoord);
		}
		else {
			if (mesh->hasTangent())
				rec.dpdv = rec.n.cross(rec.dpdu);
			else
				rec.dpdv = dpdv;
		}
		return true;
	}
	else
		return false;
}

std::array<Vector3f, 3> Triangle::getPositionsUnchecked() const
{
	return getTriangleCoeffs(mesh->positions);
}
std::array<Vector2f, 3> Triangle::getUVsUnchecked() const
{
	return getTriangleCoeffs(mesh->uvs);
}
std::array<Vector3f, 3> Triangle::getNormalsUnchecked() const
{
	return getTriangleCoeffs(mesh->normals);
}
std::array<Vector3f, 3> Triangle::getTangentsUnchecked() const
{
	return getTriangleCoeffs(mesh->tangents);
}
std::array<Vector3f, 3> Triangle::getBiTangentsUnchecked() const
{
	return getTriangleCoeffs(mesh->biTangents);
}
float Triangle::getArea() const
{
	return area;
}
