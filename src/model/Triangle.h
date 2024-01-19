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
#include "MathTypes.h"
#include "Model.h"
namespace xd {
class Triangle;

// TODO:
// 1. use raw float data and Eigen::Map
// 2. store indices
class TriangleMesh : public Model {
	friend class Triangle;

public:
	TriangleMesh(const std::shared_ptr<const Model>& owner,
				 const std::vector<float>& positions,
				 const std::vector<float>& uvs,
				 const std::vector<float>& normals,
				 const std::vector<float>& tangents,
				 const std::vector<uint32_t>& indices,
				 HitAccelMethod method = HitAccelMethod::NO_ACCEL);
	TriangleMesh(const std::vector<float>& positions,
				 const std::vector<float>& uvs,
				 const std::vector<float>& normals,
				 const std::vector<float>& tangents,
				 const std::vector<uint32_t>& indices,
				 HitAccelMethod method = HitAccelMethod::NO_ACCEL);
	TriangleMesh(std::vector<float>&& positions,
				 std::vector<float>&& uvs,
				 std::vector<float>&& normals,
				 std::vector<float>&& tangents,
				 std::vector<uint32_t>&& indices,
				 HitAccelMethod method = HitAccelMethod::NO_ACCEL);
	bool hasUV() const;
	bool hasNormal() const;
	bool hasTangent() const;
	const std::vector<Triangle>& getTriangles() const { return triangles; }
	bool hit(const Ray& ray, HitRecord& rec) const override;
	const std::vector<float>& getPositions() const;
	const std::vector<float>& getUVs() const;
	const std::vector<float>& getNormals() const;
	const std::vector<float>& getTangents() const;
	const std::vector<uint32_t>& getIndices() const;
	float getArea() const override;
	AABB getAABB() const override;
	std::shared_ptr<const Model> triangulateFrom() const;
	bool isTriangulatedFromOthers() const;

protected:
	void init(HitAccelMethod method);
	void initTriangles();
	void initAccel(HitAccelMethod method);
	std::weak_ptr<const Model> owner;
	std::vector<float> rawPositions;
	Eigen::Map<const Eigen::Matrix3Xf> positionsAccessor;
	std::vector<float> rawUVs;
	Eigen::Map<const Eigen::Matrix2Xf> uvAccessor;
	std::vector<float> rawNormals;
	Eigen::Map<const Eigen::Matrix3Xf> normalAccessor;
	std::vector<float> rawTangents;
	Eigen::Map<const Eigen::Matrix3Xf> tangentAccessor;
	std::vector<uint32_t> indices;
	std::vector<Triangle> triangles;
	AABB aabb;
	std::unique_ptr<HitAccel> hitAccel;
};
class Triangle : public Model {
public:
	Triangle(const TriangleMesh* mesh, uint32_t index);
	Vector3f getBarycentricCoordUnchecked(const Vector3f& P) const;
	bool hit(const Ray& ray, HitRecord& rec) const override;
	std::array<Vector3f, 3> getPositionsUnchecked() const;
	std::array<Vector2f, 3> getUVsUnchecked() const;
	std::array<Vector3f, 3> getNormalsUnchecked() const;
	std::array<Vector3f, 3> getTangentsUnchecked() const;
	Vector3u32 getIndices() const
	{
		const auto& indices = mesh->indices;
		return {indices[3 * index], indices[3 * index + 1], indices[3 * index + 2]};
	}
	float getArea() const override;
	AABB getAABB() const override { return aabb; }
	Vector3f getGeomNormal() const;

protected:
	void calAccParams();
	template <typename ScalarType, int N>
	std::array<Eigen::Vector<ScalarType, N>, 3> getTriangleCoeffs(
		const Eigen::Map<const Eigen::Matrix<ScalarType, N, Eigen::Dynamic>>& mat) const
	{
		const auto& indices = mesh->indices;
		return {mat.col(indices[3 * index]), mat.col(indices[3 * index + 1]),
				mat.col(indices[3 * index + 2])};
	}
	const TriangleMesh* mesh;
	uint32_t index;
	Vector3f AB, AC;
	float ABSquared, ACSquared, ABDotAC;
	Vector3f N;
	Vector3f dpdu, dpdv;
	float area;
	AABB aabb;
};
};		// namespace xd
#endif	// XD_RT_TRIANGLE_H
