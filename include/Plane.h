//
// Created by Frank on 2023/9/4.
//

#ifndef XD_RT_PLANE_H
#define XD_RT_PLANE_H
#include "Model.h"
namespace xd {
class Plane : public Model {
public:
	Plane(const Vector3f& point, const Vector3f& normal);
	bool isPointInPlane(const Vector3f& P) const;

protected:
	Vector3f point;
	Vector3f normal;
};
}  // namespace xd
#endif	// XD_RT_PLANE_H
