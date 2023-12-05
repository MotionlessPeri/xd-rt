//
// Created by Frank on 2023/12/4.
//
#include "Model.h"
using namespace xd;
std::shared_ptr<TriangleMesh> Model::getTriangulatedMesh()
{
	if (!triangulatedMesh)
		triangulatedMesh = triangulate();
	return triangulatedMesh;
}

std::shared_ptr<TriangleMesh> Model::triangulate() const
{
	assert(false);
	return nullptr;
}
