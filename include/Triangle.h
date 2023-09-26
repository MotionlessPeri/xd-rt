//
// Created by Frank on 2023/9/4.
//

#ifndef XD_RT_TRIANGLE_H
#define XD_RT_TRIANGLE_H
#include <array>
#include <vector>
#include "AABB.h"
#include "CoreTypes.h"
#include "Enums.h"
#include "HitAccel.h"
#include "MathType.h"
#include "Model.h"
namespace xd {
class Triangle;

// TODO:
// 1. use raw float data and Eigen::Map
// 2. store indices
class TriangleMesh : public Model {
	friend class Triangle;

public:
	TriangleMesh(const std::vector<Vector3f>& positions,
				 const std::vector<Vector2f>& uvs,
				 const std::vector<Vector3f>& normals,
				 const std::vector<Vector3f>& tangents,
				 const std::vector<Vector3f>& biTangents,
				 const std::vector<uint32_t>& indices,
				 HitAccelMethod method = HitAccelMethod::NO_ACCEL);
	TriangleMesh(std::vector<Vector3f>&& positions,
				 std::vector<Vector2f>&& uvs,
				 std::vector<Vector3f>&& normals,
				 std::vector<Vector3f>&& tangents,
				 std::vector<Vector3f>&& biTangents,
				 std::vector<uint32_t>&& indices,
				 HitAccelMethod method = HitAccelMethod::NO_ACCEL);
	bool hasUV() const;
	bool hasNormal() const;
	bool hasTangent() const;
	bool hasBiTangent() const;
	const std::vector<Triangle>& getTriangles() const { return triangles; }
	bool hit(const Ray& ray, HitRecord& rec) const override;
	const std::vector<Vector3f>& getPositions() const;
	const std::vector<Vector2f>& getUvs() const;
	const std::vector<Vector3f>& getNormals() const;
	const std::vector<Vector3f>& getTangents() const;
	const std::vector<Vector3f>& getBiTangents() const;
	float getArea() const override;
	AABB getAABB() const override;

protected:
	void init(const std::vector<uint32_t>& indices, HitAccelMethod method);
	void initTriangles(const std::vector<uint32_t>& indices);
	void initAccel(HitAccelMethod method);
	std::vector<Vector3f> positions;
	std::vector<Vector2f> uvs;
	std::vector<Vector3f> normals;
	std::vector<Vector3f> tangents;
	std::vector<Vector3f> biTangents;
	std::vector<Triangle> triangles;
	AABB aabb;
	std::unique_ptr<HitAccel> hitAccel;
};
class Triangle : public Model {
public:
	Triangle(const TriangleMesh* mesh, uint32_t i0, uint32_t i1, uint32_t i2);
	Triangle(const TriangleMesh* mesh, const std::array<uint32_t, 3>& vertices);
	Vector3f getBarycentricCoordUnchecked(const Vector3f& P) const;
	bool hit(const Ray& ray, HitRecord& rec) const override;
	std::array<Vector3f, 3> getPositionsUnchecked() const;
	std::array<Vector2f, 3> getUVsUnchecked() const;
	std::array<Vector3f, 3> getNormalsUnchecked() const;
	std::array<Vector3f, 3> getTangentsUnchecked() const;
	std::array<Vector3f, 3> getBiTangentsUnchecked() const;
	float getArea() const override;
	AABB getAABB() const override { return aabb; }
	const std::array<uint32_t, 3>& getIndices() const { return indices; }

protected:
	void calAccParams();
	template <typename T>
	std::array<T, 3> getTriangleCoeffs(const std::vector<T>& vec) const
	{
		return {vec[indices[0]], vec[indices[1]], vec[indices[2]]};
	}
	const TriangleMesh* mesh;
	std::array<uint32_t, 3> indices;
	Vector3f AB, AC;
	float ABSquared, ACSquared, ABDotAC;
	Vector3f N;
	Vector3f dpdu, dpdv;
	float area;
	AABB aabb;
};
};		// namespace xd
#endif	// XD_RT_TRIANGLE_H
