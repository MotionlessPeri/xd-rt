//
// Created by Frank on 2023/8/19.
//

#ifndef XD_RT_MODEL_H
#define XD_RT_MODEL_H

#include "CoreTypes.h"
#include "Hitable.h"

namespace xd {
class TriangleMesh;
class Model : public Hitable, public std::enable_shared_from_this<Model> {
public:
	Model() = default;
	virtual ~Model() = default;
	virtual float getArea() const = 0;
	virtual AABB getAABB() const = 0;
	std::shared_ptr<TriangleMesh> getTriangulatedMesh();
	// TODO: we need interfaces to deal with hit point shift due to floating-point accuracy issue
protected:
	virtual std::shared_ptr<TriangleMesh> triangulate() const;
	std::shared_ptr<TriangleMesh> triangulatedMesh = nullptr;
};

}  // namespace xd
#endif	// XD_RT_MODEL_H
