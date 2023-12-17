//
// Created by Frank on 2023/12/17.
//

#ifndef XD_RT_PERSPCAMERA_H
#define XD_RT_PERSPCAMERA_H
#include "Camera.h"
namespace xd {
class PerspCamera : public Camera {
public:
	PerspCamera(const Vector3f& position, const std::shared_ptr<Film>& film);
	Ray generateRay(const Vector2f& sample) const override;

protected:
	Vector3f pos;
};
}  // namespace xd

#endif	// XD_RT_PERSPCAMERA_H
